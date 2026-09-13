#include "dpd_native_runner.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if __has_include(<dnnl.hpp>)
#include <dnnl.hpp>
#else
#include <oneapi/dnnl/dnnl.hpp>
#endif

#ifdef DPD_HAS_OPENMP
#include <omp.h>
#endif

namespace PDFxTMD::DPDHybrid {
namespace {

constexpr std::array<char, 8> kMagic = {'D','P','D','N','A','T','1','\0'};
constexpr std::uint32_t kFormatVersion = 1;
constexpr std::uint32_t kAsinhFlag = 1;
constexpr std::uint32_t kOpLinear = 1;
constexpr std::uint32_t kOpSilu = 2;
constexpr std::uint32_t kOpGelu = 3;
constexpr std::uint32_t kOpRelu = 4;
constexpr std::uint32_t kOpTanh = 5;

template <typename T>
T read_scalar(std::ifstream& input, const char* label) {
    T value{};
    input.read(reinterpret_cast<char*>(&value), sizeof(T));
    if (!input) {
        throw std::runtime_error(std::string("truncated native model while reading ") + label);
    }
    return value;
}

template <typename T>
std::vector<T> read_vector(std::ifstream& input, std::size_t count, const char* label) {
    std::vector<T> values(count);
    if (count != 0) {
        input.read(reinterpret_cast<char*>(values.data()), static_cast<std::streamsize>(count * sizeof(T)));
        if (!input) {
            throw std::runtime_error(std::string("truncated native model while reading ") + label);
        }
    }
    return values;
}

std::string bytes_to_hex(const std::array<std::uint8_t, 32>& bytes) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string result(64, '0');
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        result[2 * i] = digits[bytes[i] >> 4U];
        result[2 * i + 1] = digits[bytes[i] & 0x0FU];
    }
    return result;
}

inline double clamp_double(double value, double low, double high) {
    return std::max(low, std::min(high, value));
}

inline double scale_log10(double value, double low, double high) {
    value = std::max(value, low);
    const double lv = std::log10(value);
    const double llo = std::log10(std::max(low, 1.0e-300));
    const double lhi = std::log10(std::max(high, low * 1.0001));
    return 2.0 * (lv - llo) / std::max(lhi - llo, 1.0e-12) - 1.0;
}

bool env_enabled(const char* name, bool default_value = false) {
    const char* value = std::getenv(name);
    if (value == nullptr || *value == '\0') {
        return default_value;
    }
    const std::string text(value);
    return text != "0" && text != "false" && text != "FALSE"
        && text != "off" && text != "OFF";
}

std::string env_text(const char* name, const char* default_value) {
    const char* value = std::getenv(name);
    return value == nullptr || *value == '\0' ? std::string(default_value) : std::string(value);
}

}  // namespace

struct NativeRunner::Impl {
    static constexpr std::size_t kSmallBatchThreshold = 32;

    struct Tensor {
        std::size_t rows = 0;
        std::size_t cols = 0;
        std::vector<float> values;

        void resize(std::size_t new_rows, std::size_t new_cols) {
            rows = new_rows;
            cols = new_cols;
            values.resize(rows * cols);
        }

        float* data() noexcept { return values.data(); }
        const float* data() const noexcept { return values.data(); }

        float& operator()(std::size_t row, std::size_t col) {
            return values[row * cols + col];
        }

        const float& operator()(std::size_t row, std::size_t col) const {
            return values[row * cols + col];
        }
    };

    struct Operation {
        std::uint32_t type = 0;
        std::uint32_t input_dim = 0;
        std::uint32_t output_dim = 0;
        std::vector<float> weight;
        std::vector<float> transposed_weight;
        std::vector<float> bias;
    };

    struct DnnlExecutable {
        dnnl::primitive primitive;
        std::unordered_map<int, dnnl::memory> args;

        void execute(dnnl::stream& stream) const {
            primitive.execute(stream, args);
        }
    };

    struct DnnlPlan {
        struct DeviceTensor {
            std::size_t rows = 0;
            std::size_t cols = 0;
            dnnl::memory memory;
        };

        std::size_t batch = 0;
        bool uses_gpu = false;
        Tensor features;
        DeviceTensor device_features;
        std::vector<DeviceTensor> stages;
        DeviceTensor device_output;
        Tensor output;
        std::vector<DnnlExecutable> operations;

        static dnnl::memory::desc matrix_desc(std::size_t rows, std::size_t cols) {
            return dnnl::memory::desc(
                {static_cast<dnnl::memory::dim>(rows), static_cast<dnnl::memory::dim>(cols)},
                dnnl::memory::data_type::f32,
                dnnl::memory::format_tag::ab
            );
        }

        static DeviceTensor make_device_tensor(
            dnnl::engine& engine,
            std::size_t rows,
            std::size_t cols,
            float* host_handle = nullptr
        ) {
            DeviceTensor tensor;
            tensor.rows = rows;
            tensor.cols = cols;
            const auto description = matrix_desc(rows, cols);
            tensor.memory = host_handle == nullptr
                ? dnnl::memory(description, engine)
                : dnnl::memory(description, engine, host_handle);
            return tensor;
        }

        static void write_memory(
            const std::vector<float>& source, const dnnl::memory& destination
        ) {
            const std::size_t bytes = source.size() * sizeof(float);
            if (bytes != destination.get_desc().get_size()) {
                throw std::runtime_error("oneDNN upload size does not match its memory descriptor");
            }
            float* mapped = destination.map_data<float>();
            std::copy(source.begin(), source.end(), mapped);
            destination.unmap_data(mapped);
        }

        static void read_memory(const dnnl::memory& source, std::vector<float>& destination) {
            const std::size_t bytes = destination.size() * sizeof(float);
            if (bytes != source.get_desc().get_size()) {
                throw std::runtime_error("oneDNN download size does not match its memory descriptor");
            }
            float* mapped = source.map_data<float>();
            std::copy(mapped, mapped + destination.size(), destination.begin());
            source.unmap_data(mapped);
        }

        static DnnlExecutable make_linear(
            dnnl::engine& engine,
            const DeviceTensor& source,
            DeviceTensor& destination,
            const std::vector<float>& transposed_weight,
            const std::vector<float>& bias
        ) {
            const auto src_desc = matrix_desc(source.rows, source.cols);
            const auto weight_desc = matrix_desc(source.cols, destination.cols);
            const auto bias_desc = matrix_desc(1, destination.cols);
            const auto dst_desc = matrix_desc(destination.rows, destination.cols);

#if DNNL_VERSION_MAJOR >= 3
            const auto pd = dnnl::matmul::primitive_desc(
                engine, src_desc, weight_desc, bias_desc, dst_desc
            );
#else
            const auto description = dnnl::matmul::desc(
                src_desc, weight_desc, bias_desc, dst_desc
            );
            const auto pd = dnnl::matmul::primitive_desc(description, engine);
#endif
            DnnlExecutable executable;
            executable.primitive = dnnl::matmul(pd);
            executable.args.emplace(DNNL_ARG_SRC, source.memory);
            executable.args.emplace(DNNL_ARG_WEIGHTS, dnnl::memory(weight_desc, engine));
            executable.args.emplace(DNNL_ARG_BIAS, dnnl::memory(bias_desc, engine));
            executable.args.emplace(DNNL_ARG_DST, destination.memory);
            write_memory(transposed_weight, executable.args.at(DNNL_ARG_WEIGHTS));
            write_memory(bias, executable.args.at(DNNL_ARG_BIAS));
            return executable;
        }

        static dnnl::algorithm activation_algorithm(std::uint32_t type) {
            switch (type) {
            case kOpSilu: return dnnl::algorithm::eltwise_swish;
            case kOpGelu: return dnnl::algorithm::eltwise_gelu_erf;
            case kOpRelu: return dnnl::algorithm::eltwise_relu;
            case kOpTanh: return dnnl::algorithm::eltwise_tanh;
            default: throw std::runtime_error("unsupported native activation operation");
            }
        }

        static DnnlExecutable make_activation(
            dnnl::engine& engine,
            const DeviceTensor& source,
            DeviceTensor& destination,
            std::uint32_t type
        ) {
            const auto data_desc = matrix_desc(source.rows, source.cols);
            const auto algorithm = activation_algorithm(type);
            const float alpha = type == kOpSilu ? 1.0f : 0.0f;
            constexpr float beta = 0.0f;
#if DNNL_VERSION_MAJOR >= 3
            const auto pd = dnnl::eltwise_forward::primitive_desc(
                engine,
                dnnl::prop_kind::forward_inference,
                algorithm,
                data_desc,
                data_desc,
                alpha,
                beta
            );
#else
            const auto description = dnnl::eltwise_forward::desc(
                dnnl::prop_kind::forward_inference,
                algorithm,
                data_desc,
                alpha,
                beta
            );
            const auto pd = dnnl::eltwise_forward::primitive_desc(description, engine);
#endif
            DnnlExecutable executable;
            executable.primitive = dnnl::eltwise_forward(pd);
            executable.args.emplace(DNNL_ARG_SRC, source.memory);
            executable.args.emplace(DNNL_ARG_DST, destination.memory);
            return executable;
        }

        DnnlPlan(
            dnnl::engine& engine,
            std::size_t batch_size,
            std::size_t feature_count,
            std::size_t output_count,
            const std::vector<Operation>& main_ops,
            bool use_gpu
        ) : batch(batch_size), uses_gpu(use_gpu) {
            if (main_ops.empty()) {
                throw std::runtime_error("native DPD network has no operations");
            }
            features.resize(batch, feature_count);
            device_features = make_device_tensor(
                engine, batch, feature_count, uses_gpu ? nullptr : features.data()
            );
            stages.reserve(main_ops.size());
            operations.reserve(main_ops.size());

            const DeviceTensor* source = &device_features;
            for (std::size_t index = 0; index < main_ops.size(); ++index) {
                const Operation& operation = main_ops[index];
                const std::size_t destination_cols = operation.type == kOpLinear
                    ? operation.output_dim
                    : source->cols;
                const bool direct_output = index + 1 == main_ops.size();
                DeviceTensor* destination = nullptr;
                if (direct_output) {
                    if (destination_cols != output_count) {
                        throw std::runtime_error(
                            "native DPD network final operation has the wrong output width"
                        );
                    }
                    output.resize(batch, destination_cols);
                    device_output = make_device_tensor(
                        engine,
                        batch,
                        destination_cols,
                        uses_gpu ? nullptr : output.data()
                    );
                    destination = &device_output;
                } else {
                    stages.push_back(make_device_tensor(engine, batch, destination_cols));
                    destination = &stages.back();
                }
                if (operation.type == kOpLinear) {
                    operations.push_back(make_linear(
                        engine,
                        *source,
                        *destination,
                        operation.transposed_weight,
                        operation.bias
                    ));
                } else {
                    operations.push_back(make_activation(
                        engine, *source, *destination, operation.type
                    ));
                }
                source = destination;
            }
        }

        const Tensor& run(dnnl::stream& stream) {
            if (uses_gpu) {
                write_memory(features.values, device_features.memory);
            }
            for (const DnnlExecutable& operation : operations) {
                operation.execute(stream);
            }
            stream.wait();
            if (uses_gpu) {
                read_memory(device_output.memory, output.values);
            }
            return output;
        }
    };

    std::uint32_t input_dim = 0;
    std::uint32_t output_dim = 0;
    std::uint32_t pair_count = 0;
    std::array<double, 6> domain{};
    double charm_mass = 1.40;
    double bottom_mass = 4.75;
    std::array<std::uint8_t, 32> checkpoint_sha256{};
    std::vector<double> residual_epsilon;
    std::vector<std::pair<std::int32_t, std::int32_t>> pid_pairs;
    std::vector<Operation> main_ops;
    std::array<std::uint8_t, NativeRunner::kPairCount> pair_threshold_mask{};
    bool gpu_detected = false;
    bool using_gpu = false;
    bool gpu_required = false;
    dnnl::engine engine;
    dnnl::stream stream;
    std::unordered_map<std::size_t, std::unique_ptr<DnnlPlan>> plans;

    void select_engine() {
        const std::string preference = env_text("PDFXTMD_DNNL_DEVICE", "auto");
        if (preference != "auto" && preference != "cpu" && preference != "gpu") {
            throw std::runtime_error(
                "PDFXTMD_DNNL_DEVICE must be one of: auto, cpu, gpu"
            );
        }
        gpu_required = preference == "gpu";
#ifdef PDFXTMD_DNNL_GPU_ENABLED
        try {
            gpu_detected = dnnl::engine::get_count(dnnl::engine::kind::gpu) > 0;
        } catch (const dnnl::error&) {
            gpu_detected = false;
        }
        if (gpu_required && !gpu_detected) {
            throw std::runtime_error(
                "PDFXTMD_DNNL_DEVICE=gpu was requested, but oneDNN found no GPU engine"
            );
        }
        using_gpu = preference != "cpu" && gpu_detected;
#else
        if (gpu_required) {
            throw std::runtime_error(
                "PDFXTMD_DNNL_DEVICE=gpu was requested, but ENABLE_DPD_GPU is OFF"
            );
        }
#endif
        engine = dnnl::engine(
            using_gpu ? dnnl::engine::kind::gpu : dnnl::engine::kind::cpu, 0
        );
        stream = dnnl::stream(engine);
    }

    bool fall_back_to_cpu() {
        if (!using_gpu || gpu_required) {
            return false;
        }
        plans.clear();
        using_gpu = false;
        engine = dnnl::engine(dnnl::engine::kind::cpu, 0);
        stream = dnnl::stream(engine);
        return true;
    }

    static Operation read_operation(std::ifstream& input) {
        Operation operation;
        operation.type = read_scalar<std::uint32_t>(input, "operation type");
        operation.input_dim = read_scalar<std::uint32_t>(input, "operation input dimension");
        operation.output_dim = read_scalar<std::uint32_t>(input, "operation output dimension");
        (void)read_scalar<std::uint32_t>(input, "operation reserved field");
        if (operation.type == kOpLinear) {
            const std::size_t weight_count =
                static_cast<std::size_t>(operation.input_dim) * operation.output_dim;
            operation.weight = read_vector<float>(input, weight_count, "linear weights");
            operation.bias = read_vector<float>(input, operation.output_dim, "linear bias");
            operation.transposed_weight.resize(weight_count);
            for (std::size_t output = 0; output < operation.output_dim; ++output) {
                for (std::size_t input_index = 0; input_index < operation.input_dim; ++input_index) {
                    operation.transposed_weight[input_index * operation.output_dim + output] =
                        operation.weight[output * operation.input_dim + input_index];
                }
            }
        } else if (operation.type < kOpSilu || operation.type > kOpTanh) {
            throw std::runtime_error("native model contains an unsupported operation code");
        }
        return operation;
    }

    explicit Impl(const std::string& model_path) {
        std::ifstream input(model_path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("cannot open native DPD model: " + model_path);
        }
        std::array<char, 8> magic{};
        input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
        if (!input || magic != kMagic) {
            throw std::runtime_error("invalid native DPD model magic");
        }
        const std::uint32_t format_version = read_scalar<std::uint32_t>(
            input, "format version"
        );
        if (format_version != kFormatVersion) {
            throw std::runtime_error(
                "unsupported native DPD model format version; re-export the asinh v1 model"
            );
        }
        input_dim = read_scalar<std::uint32_t>(input, "input dimension");
        output_dim = read_scalar<std::uint32_t>(input, "output dimension");
        pair_count = read_scalar<std::uint32_t>(input, "pair count");
        const std::uint32_t main_op_count = read_scalar<std::uint32_t>(input, "main op count");
        const std::uint32_t flags = read_scalar<std::uint32_t>(input, "flags");
        const std::uint32_t reserved = read_scalar<std::uint32_t>(input, "reserved field");
        if (flags != kAsinhFlag || reserved != 0) {
            throw std::runtime_error("native DPD model is not the asinh v1 schema");
        }

        for (double& value : domain) {
            value = read_scalar<double>(input, "domain value");
        }
        charm_mass = read_scalar<double>(input, "charm mass");
        bottom_mass = read_scalar<double>(input, "bottom mass");
        input.read(
            reinterpret_cast<char*>(checkpoint_sha256.data()),
            static_cast<std::streamsize>(checkpoint_sha256.size())
        );
        if (!input) {
            throw std::runtime_error("truncated native model checkpoint hash");
        }

        if (output_dim != NativeRunner::kPairCount || pair_count != NativeRunner::kPairCount) {
            throw std::runtime_error("native DPD model must contain exactly 121 outputs/pairs");
        }
        residual_epsilon = read_vector<double>(input, output_dim, "residual epsilon");
        for (const double epsilon : residual_epsilon) {
            if (!std::isfinite(epsilon) || epsilon <= 0.0) {
                throw std::runtime_error("residual epsilon must be finite and positive");
            }
        }
        pid_pairs.reserve(pair_count);
        for (std::uint32_t i = 0; i < pair_count; ++i) {
            const auto first = read_scalar<std::int32_t>(input, "first PID");
            const auto second = read_scalar<std::int32_t>(input, "second PID");
            pid_pairs.emplace_back(first, second);
        }

        main_ops.reserve(main_op_count);
        for (std::uint32_t i = 0; i < main_op_count; ++i) {
            main_ops.push_back(read_operation(input));
        }
        std::uint32_t dimension = input_dim;
        for (const Operation& operation : main_ops) {
            if (operation.input_dim != dimension) {
                throw std::runtime_error("native DPD operation dimensions are not contiguous");
            }
            if (operation.type == kOpLinear) {
                dimension = operation.output_dim;
            } else if (operation.output_dim != dimension) {
                throw std::runtime_error("native DPD activation changed matrix width");
            }
        }
        if (dimension != output_dim) {
            throw std::runtime_error("native DPD network does not end at 121 outputs");
        }

        if (input.peek() != std::ifstream::traits_type::eof()) {
            throw std::runtime_error("native DPD model contains trailing data");
        }

        for (std::size_t pair = 0; pair < pid_pairs.size(); ++pair) {
            const int first_abs = std::abs(pid_pairs[pair].first);
            const int second_abs = std::abs(pid_pairs[pair].second);
            std::uint8_t mask = 0;
            if (first_abs == 4) mask |= 1U << 0U;
            if (second_abs == 4) mask |= 1U << 1U;
            if (first_abs == 5) mask |= 1U << 2U;
            if (second_abs == 5) mask |= 1U << 3U;
            pair_threshold_mask[pair] = mask;
        }
        select_engine();
    }

    void fill_features(
        Tensor& features,
        const std::vector<double>& x1,
        const std::vector<double>& x2,
        const std::vector<double>& mu1_2,
        const std::vector<double>& mu2_2
    ) const {
        const std::size_t batch = x1.size();
        features.resize(batch, input_dim);
        const double x_min = domain[0];
        const double x_max = domain[1];
        const double mu_min = domain[4];
        const double mu_max = domain[5];
        constexpr std::size_t expected = 8U + 4U;
        if (expected != input_dim) {
            throw std::runtime_error("native DPD metadata implies a different feature dimension");
        }

        for (std::size_t row = 0; row < batch; ++row) {
            const double xsum = clamp_double(x1[row] + x2[row], 0.0, 1.0);
            const double remainder = clamp_double(1.0 - xsum, x_min, 1.0);
            const double lx1 = scale_log10(x1[row], x_min, x_max);
            const double lx2 = scale_log10(x2[row], x_min, x_max);
            const double lrem = scale_log10(remainder, x_min, 1.0);
            const double lm1 = scale_log10(mu1_2[row], mu_min, mu_max);
            const double lm2 = scale_log10(mu2_2[row], mu_min, mu_max);
            const double asym_x = (x1[row] - x2[row]) / std::max(x1[row] + x2[row], 1.0e-30);
            const double asym_mu = 0.5 * (lm1 - lm2);
            const double xsum_feature = 2.0 * xsum - 1.0;
            const std::array<double, 8> base = {
                lx1, lx2, lrem, lm1, lm2, asym_x, asym_mu, xsum_feature
            };

            std::size_t column = 0;
            for (double value : base) {
                features(row, column++) = static_cast<float>(value);
            }
            const double charm2 = charm_mass * charm_mass;
            const double bottom2 = bottom_mass * bottom_mass;
            const std::array<double, 4> distances = {
                std::log10(std::max(mu1_2[row], 1.0e-300) / charm2),
                std::log10(std::max(mu2_2[row], 1.0e-300) / charm2),
                std::log10(std::max(mu1_2[row], 1.0e-300) / bottom2),
                std::log10(std::max(mu2_2[row], 1.0e-300) / bottom2),
            };
            for (double value : distances) {
                features(row, column++) = static_cast<float>(clamp_double(value, -1.0, 1.0));
            }

            if (column != input_dim) {
                throw std::runtime_error("native DPD feature writer produced an invalid column count");
            }
        }
    }

    DnnlPlan& plan_for(std::size_t batch) {
        auto found = plans.find(batch);
        if (found != plans.end()) {
            return *found->second;
        }
        auto plan = std::make_unique<DnnlPlan>(
            engine,
            batch,
            input_dim,
            output_dim,
            main_ops,
            using_gpu
        );
        DnnlPlan& result = *plan;
        plans.emplace(batch, std::move(plan));
        return result;
    }
};

NativeRunner::NativeRunner(const std::string& model_path)
    : impl_(std::make_unique<Impl>(model_path)) {}

NativeRunner::~NativeRunner() = default;
NativeRunner::NativeRunner(NativeRunner&&) noexcept = default;
NativeRunner& NativeRunner::operator=(NativeRunner&&) noexcept = default;

std::string NativeRunner::metadata_value(const char* key) const {
    if (std::strcmp(key, "dpd_source_checkpoint_sha256") == 0) {
        return bytes_to_hex(impl_->checkpoint_sha256);
    }
    if (std::strcmp(key, "dpd_dnnl_fast16_available") == 0) {
        return "true";
    }
    if (std::strcmp(key, "dpd_dnnl_device") == 0) {
        return impl_->using_gpu ? "gpu" : "cpu";
    }
    if (std::strcmp(key, "dpd_dnnl_gpu_available") == 0) {
        return impl_->gpu_detected ? "true" : "false";
    }
    return {};
}

std::vector<double> NativeRunner::predict(
    const std::vector<double>& x1,
    const std::vector<double>& x2,
    const std::vector<double>& mu1_2,
    const std::vector<double>& mu2_2,
    const std::vector<double>& spdf_baseline
) {
    const std::size_t batch = x1.size();
    if (batch == 0) {
        return {};
    }
    if (x2.size() != batch || mu1_2.size() != batch || mu2_2.size() != batch) {
        throw std::invalid_argument("All coordinate arrays must have equal length");
    }
    if (spdf_baseline.size() != batch * kPairCount) {
        throw std::invalid_argument("SPDF baseline must contain batch*121 values");
    }

    Impl::DnnlPlan* plan = nullptr;
    try {
        plan = &impl_->plan_for(batch);
        impl_->fill_features(plan->features, x1, x2, mu1_2, mu2_2);
    } catch (const dnnl::error&) {
        if (!impl_->fall_back_to_cpu()) {
            throw;
        }
        plan = &impl_->plan_for(batch);
        impl_->fill_features(plan->features, x1, x2, mu1_2, mu2_2);
    }
    const Impl::Tensor* correction_result = nullptr;
    try {
        correction_result = &plan->run(impl_->stream);
    } catch (const dnnl::error&) {
        if (!impl_->fall_back_to_cpu()) {
            throw;
        }
        plan = &impl_->plan_for(batch);
        impl_->fill_features(plan->features, x1, x2, mu1_2, mu2_2);
        correction_result = &plan->run(impl_->stream);
    }
    const Impl::Tensor& correction = *correction_result;
    if (correction.rows != batch || correction.cols != kPairCount) {
        throw std::runtime_error("native DPD network returned an invalid shape");
    }

    std::vector<double> result(batch * kPairCount, 0.0);
    const double charm2 = impl_->charm_mass * impl_->charm_mass;
    const double bottom2 = impl_->bottom_mass * impl_->bottom_mass;

#ifdef DPD_HAS_OPENMP
    #pragma omp parallel for if(batch == 16 && omp_get_max_threads() > 1) schedule(static)
#endif
    for (std::ptrdiff_t row_index = 0;
         row_index < static_cast<std::ptrdiff_t>(batch); ++row_index) {
        const std::size_t row = static_cast<std::size_t>(row_index);
        if (!(x1[row] > 0.0 && x2[row] > 0.0 && x1[row] + x2[row] < 1.0)) {
            continue;
        }

        std::uint8_t inactive_mask = 0;
        if (mu1_2[row] < charm2) inactive_mask |= 1U << 0U;
        if (mu2_2[row] < charm2) inactive_mask |= 1U << 1U;
        if (mu1_2[row] < bottom2) inactive_mask |= 1U << 2U;
        if (mu2_2[row] < bottom2) inactive_mask |= 1U << 3U;

        const double* const baseline_row = spdf_baseline.data() + row * kPairCount;
        const float* const correction_row = correction.data() + row * kPairCount;
        double* const result_row = result.data() + row * kPairCount;
        for (std::size_t pair = 0; pair < kPairCount; ++pair) {
            if ((impl_->pair_threshold_mask[pair] & inactive_mask) != 0) {
                continue;
            }
            const double baseline = baseline_row[pair];
            result_row[pair] = baseline + (baseline + impl_->residual_epsilon[pair])
                * std::sinh(static_cast<double>(correction_row[pair]));
        }
    }
    return result;
}

}  // namespace PDFxTMD::DPDHybrid
