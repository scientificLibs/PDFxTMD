#include "hybrid_artifact.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace PDFxTMD::DPDHybrid {
namespace {

constexpr std::array<unsigned char, 16> kMagic = {
    'D', 'P', 'D', '-', 'H', 'Y', 'B', 'R',
    'I', 'D', '-', 'B', '1', 0, 0, 0
};

void read_exact(std::istream& stream, void* target, std::size_t size) {
    stream.read(static_cast<char*>(target), static_cast<std::streamsize>(size));
    if (!stream) {
        throw std::runtime_error("truncated C++ hybrid artifact");
    }
}

std::uint16_t read_u16(std::istream& stream) {
    std::array<unsigned char, 2> bytes{};
    read_exact(stream, bytes.data(), bytes.size());
    return static_cast<std::uint16_t>(bytes[0])
        | (static_cast<std::uint16_t>(bytes[1]) << 8u);
}

std::uint32_t read_u32(std::istream& stream) {
    std::array<unsigned char, 4> bytes{};
    read_exact(stream, bytes.data(), bytes.size());
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<std::uint32_t>(bytes[i]) << (8u * i);
    }
    return value;
}

std::uint64_t read_u64(std::istream& stream) {
    std::array<unsigned char, 8> bytes{};
    read_exact(stream, bytes.data(), bytes.size());
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<std::uint64_t>(bytes[i]) << (8u * i);
    }
    return value;
}

std::int32_t read_i32(std::istream& stream) {
    return static_cast<std::int32_t>(read_u32(stream));
}

double read_f64(std::istream& stream) {
    const std::uint64_t bits = read_u64(stream);
    double value = 0.0;
    static_assert(sizeof(value) == sizeof(bits), "binary64 is required");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::string read_string(std::istream& stream, std::uint32_t size) {
    if (size > 1024u * 1024u) {
        throw std::runtime_error("unreasonable string length in hybrid artifact");
    }
    std::string value(size, '\0');
    if (size != 0) {
        read_exact(stream, value.data(), size);
    }
    return value;
}

void validate_axis(const std::vector<double>& axis, const char* name) {
    if (axis.size() < 2) {
        throw std::runtime_error(std::string(name) + " axis is too short");
    }
    for (std::size_t index = 0; index < axis.size(); ++index) {
        if (!std::isfinite(axis[index])) {
            throw std::runtime_error(std::string(name) + " axis is non-finite");
        }
        if (index != 0 && !(axis[index] > axis[index - 1])) {
            throw std::runtime_error(std::string(name) + " axis is not increasing");
        }
    }
}

}  // namespace

bool NodeIndex::operator<(const NodeIndex& other) const noexcept {
    return std::tie(ix1, ix2, imu1, imu2)
        < std::tie(other.ix1, other.ix2, other.imu1, other.imu2);
}

bool NodeIndex::operator==(const NodeIndex& other) const noexcept {
    return ix1 == other.ix1 && ix2 == other.ix2
        && imu1 == other.imu1 && imu2 == other.imu2;
}

std::uint64_t HybridArtifact::packed_key(
    const NodeIndex& node,
    std::size_t pair_index
) const {
    if (node.ix1 >= nx() || node.ix2 >= nx()
        || node.imu1 >= nmu() || node.imu2 >= nmu()
        || pair_index >= pid_pairs.size()) {
        throw std::out_of_range("hybrid node/channel key is out of range");
    }
    return (((
        (static_cast<std::uint64_t>(node.ix1) * nx() + node.ix2) * nmu()
        + node.imu1) * nmu() + node.imu2) * pid_pairs.size()) + pair_index;
}

void HybridArtifact::apply_overrides(
    const NodeIndex& node,
    std::vector<double>& core
) const {
    apply_overrides(node, core.data(), core.size());
}

void HybridArtifact::apply_overrides(
    const NodeIndex& node,
    double* core,
    std::size_t core_size
) const {
    if (core == nullptr || core_size != pid_pairs.size()) {
        throw std::invalid_argument("node core must contain all 121 PID pairs");
    }
    const std::uint64_t first = packed_key(node, 0);
    const std::uint64_t last = first + pid_pairs.size();
    auto begin = std::lower_bound(
        overrides.begin(), overrides.end(), first,
        [](const OverrideRecord& record, std::uint64_t value) {
            return record.packed < value;
        }
    );
    for (auto it = begin; it != overrides.end() && it->packed < last; ++it) {
        core[it->pair_index] = it->value;
    }
}

HybridArtifact HybridArtifact::load(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        throw std::runtime_error("could not open hybrid artifact: " + path);
    }
    std::array<unsigned char, 16> magic{};
    read_exact(stream, magic.data(), magic.size());
    if (magic != kMagic) {
        throw std::runtime_error("not a DPD-HYBRID-B1 artifact");
    }
    if (read_u32(stream) != kFormatVersion) {
        throw std::runtime_error("unsupported C++ hybrid artifact version");
    }

    const std::uint32_t nx = read_u32(stream);
    const std::uint32_t nmu = read_u32(stream);
    const std::uint32_t n_pairs = read_u32(stream);
    const std::uint32_t n_pids = read_u32(stream);
    const std::int32_t nmcor = read_i32(stream);
    const std::uint64_t override_count = read_u64(stream);
    if (nx < 2 || nmu < 2 || n_pairs != kPairCount || n_pids != kPidCount) {
        throw std::runtime_error("incompatible hybrid dimensions");
    }
    if (override_count > 1000000000ull) {
        throw std::runtime_error("unreasonable override count");
    }

    HybridArtifact result;
    result.nmcor = nmcor;
    result.xmax = read_f64(stream);
    result.charm_mass = read_f64(stream);
    result.bottom_mass = read_f64(stream);
    result.spdf_member = read_i32(stream);
    const std::uint32_t spdf_set_length = read_u32(stream);
    const std::uint32_t dpd_set_length = read_u32(stream);
    read_exact(stream, result.checkpoint_sha256.data(), result.checkpoint_sha256.size());
    read_exact(stream, result.grid_sha256.data(), result.grid_sha256.size());
    read_exact(
        stream,
        result.source_override_sha256.data(),
        result.source_override_sha256.size()
    );

    result.pids.resize(n_pids);
    for (int& pid : result.pids) {
        pid = read_i32(stream);
    }
    result.pid_pairs.reserve(n_pairs);
    for (std::uint32_t index = 0; index < n_pairs; ++index) {
        // Stream reads must be sequenced explicitly. Function-argument
        // evaluation order is not a portable serialization contract.
        const int first = read_i32(stream);
        const int second = read_i32(stream);
        result.pid_pairs.emplace_back(first, second);
    }
    result.xs.resize(nx);
    result.mus.resize(nmu);
    for (double& value : result.xs) {
        value = read_f64(stream);
    }
    for (double& value : result.mus) {
        value = read_f64(stream);
    }
    result.spdf_set = read_string(stream, spdf_set_length);
    result.dpd_set = read_string(stream, dpd_set_length);

    result.overrides.reserve(static_cast<std::size_t>(override_count));
    for (std::uint64_t index = 0; index < override_count; ++index) {
        OverrideRecord record;
        record.node.ix1 = read_u16(stream);
        record.node.ix2 = read_u16(stream);
        record.node.imu1 = read_u16(stream);
        record.node.imu2 = read_u16(stream);
        record.pair_index = read_u16(stream);
        record.value = read_f64(stream);
        if (!std::isfinite(record.value)) {
            throw std::runtime_error("hybrid override is non-finite");
        }
        record.packed = result.packed_key(record.node, record.pair_index);
        if (!result.overrides.empty()
            && record.packed <= result.overrides.back().packed) {
            throw std::runtime_error("hybrid override keys are not sorted and unique");
        }
        result.overrides.push_back(record);
    }
    if (stream.peek() != std::char_traits<char>::eof()) {
        throw std::runtime_error("unexpected trailing bytes in hybrid artifact");
    }

    validate_axis(result.xs, "x");
    validate_axis(result.mus, "log(mu^2)");
    if (result.xmax != result.xs.back()) {
        throw std::runtime_error("xmax does not equal the final x node");
    }
    if (result.nmcor < 2 || result.nmcor > static_cast<int>(result.nx())) {
        throw std::runtime_error("nmcor is incompatible with the x axis");
    }
    if (!(result.charm_mass > 0.0) || !(result.bottom_mass > 0.0)) {
        throw std::runtime_error("invalid heavy-flavour masses");
    }
    const std::array<int, kPidCount> expected_pids = {
        -5, -4, -3, -2, -1, 21, 1, 2, 3, 4, 5
    };
    if (!std::equal(result.pids.begin(), result.pids.end(), expected_pids.begin())) {
        throw std::runtime_error("PID order is incompatible with the neural node provider");
    }
    std::size_t pair_index = 0;
    for (int first : expected_pids) {
        for (int second : expected_pids) {
            if (result.pid_pairs.at(pair_index++) != std::make_pair(first, second)) {
                throw std::runtime_error("PID-pair order is not the expected Cartesian order");
            }
        }
    }
    if (result.spdf_set.empty() || result.spdf_member < 0) {
        throw std::runtime_error("invalid SPDF set/member in hybrid artifact");
    }
    return result;
}

}  // namespace PDFxTMD::DPDHybrid
