#include "dpd_hybrid_runner.hpp"

#include "PDFxTMDLib/Common/EnvUtils.h"
#include "pdfxtmd_hybrid_interpolator.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>
#ifdef DPD_HAS_OPENMP
#include <omp.h>
#endif

namespace PDFxTMD::DPDHybrid
{
namespace
{

std::string hex_digest(const std::array<std::uint8_t, 32> &digest)
{
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (std::uint8_t byte : digest)
    {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

} // namespace

HybridRunner::HybridRunner(const std::string &network_model_path,
                           const std::string &hybrid_artifact_path,
                           std::optional<std::size_t> node_cache_capacity,
                           std::optional<int> thread_count)
    : artifact_(HybridArtifact::load(hybrid_artifact_path)), native_network_(network_model_path),
      baseline_(artifact_.spdf_set, artifact_.spdf_member, artifact_.charm_mass,
                artifact_.bottom_mass)
{
    if (artifact_.pid_pairs.size() != kPairCount)
    {
        throw std::runtime_error("hybrid artifact does not contain 121 PID pairs");
    }
    if (network_metadata_value("dpd_source_checkpoint_sha256") !=
        hex_digest(artifact_.checkpoint_sha256))
    {
        throw std::runtime_error(
            "neural node provider and hybrid artifact use different checkpoints");
    }
    public_channels_.resize(kPairCount);
    for (std::size_t pair = 0; pair < kPairCount; ++pair)
    {
        public_channels_[pair] = pair;
    }

    // The NN is queried only on exact hybrid-grid nodes. Precompute the tiny
    // single-PDF leg table once so random workloads never call PDFxTMD from
    // the hot node-generation path.
    precompute_baseline_grid();

    node_cache_capacity_ = env_enabled("DPD_HYBRID_DISABLE_CACHE")
                               ? 0
                               : node_cache_capacity.value_or(env_size_allow_zero(
                                     "DPD_HYBRID_NODE_CACHE_NODES", kDefaultNodeCacheNodes));
    if (node_cache_capacity_ > 0 && node_cache_capacity_ < 16)
    {
        node_cache_capacity_ = 16;
    }
    if (node_cache_capacity_ > 0)
    {
        persistent_node_cache_.reserve(node_cache_capacity_);
    }
    thread_count_ =
        thread_count.value_or(static_cast<int>(env_size_allow_zero("DPD_HYBRID_THREADS", 0)));
    if (thread_count_ < 0)
    {
        throw std::runtime_error("hybrid thread count must be non-negative");
    }

    query_log_path_ = env_string("DPD_HYBRID_QUERY_LOG");
    query_log_max_ = env_size_allow_zero("DPD_HYBRID_QUERY_LOG_MAX", 1000000);
    query_log_flush_every_ = env_positive_size("DPD_HYBRID_QUERY_LOG_FLUSH_EVERY", 10000);
    if (!query_log_path_.empty())
    {
        const std::filesystem::path log_path(query_log_path_);
        if (!log_path.parent_path().empty())
        {
            std::filesystem::create_directories(log_path.parent_path());
        }
        query_log_.open(log_path, std::ios::out | std::ios::trunc);
        if (!query_log_)
        {
            throw std::runtime_error("cannot open DPD hybrid query log: " + query_log_path_);
        }
        query_log_ << "x1,x2,mu1_2,mu2_2\n";
        query_log_ << std::setprecision(17);
        std::cerr << "[hybrid-query-log] path=" << query_log_path_;
        if (query_log_max_ == 0)
        {
            std::cerr << ", max=unlimited";
        }
        else
        {
            std::cerr << ", max=" << query_log_max_;
        }
        std::cerr << ", flush_every=" << query_log_flush_every_ << "\n";
    }

}

std::string HybridRunner::network_metadata_value(const char *key) const
{
    return native_network_.metadata_value(key);
}

std::vector<double> HybridRunner::network_predict(const std::vector<double> &x1,
                                                  const std::vector<double> &x2,
                                                  const std::vector<double> &mu1_2,
                                                  const std::vector<double> &mu2_2,
                                                  const std::vector<double> &spdf_baseline)
{
#ifdef DPD_HAS_OPENMP
    if (thread_count_ > 0)
    {
        omp_set_num_threads(thread_count_);
    }
#endif
    return native_network_.predict(x1, x2, mu1_2, mu2_2, spdf_baseline);
}

void HybridRunner::maybe_log_query(double x1, double x2, double mu1_2, double mu2_2)
{
    if (!query_log_.is_open())
    {
        return;
    }
    if (query_log_max_ != 0 && query_log_count_ >= query_log_max_)
    {
        return;
    }

    query_log_ << x1 << ',' << x2 << ',' << mu1_2 << ',' << mu2_2 << '\n';
    if (!query_log_)
    {
        throw std::runtime_error("failed while writing DPD hybrid query log: " + query_log_path_);
    }
    ++query_log_count_;
    if (query_log_flush_every_ > 0 && query_log_count_ % query_log_flush_every_ == 0)
    {
        query_log_.flush();
    }
}

std::size_t HybridRunner::NodeCache::row_index(const NodeIndex &node) const
{
    const auto found = std::lower_bound(nodes.begin(), nodes.end(), node);
    if (found == nodes.end() || !(*found == node))
    {
        throw std::runtime_error("hybrid node planner missed a required node");
    }
    return static_cast<std::size_t>(found - nodes.begin());
}

const double *HybridRunner::NodeCache::row_values(std::size_t row) const
{
    if (row >= rows.size())
    {
        throw std::out_of_range("hybrid node-cache row is out of range");
    }
    return rows[row];
}

std::uint64_t HybridRunner::node_key(const NodeIndex &node) noexcept
{
    return static_cast<std::uint64_t>(node.ix1) | (static_cast<std::uint64_t>(node.ix2) << 16U) |
           (static_cast<std::uint64_t>(node.imu1) << 32U) |
           (static_cast<std::uint64_t>(node.imu2) << 48U);
}

void HybridRunner::touch_cache_entry(
    std::unordered_map<std::uint64_t, PersistentNodeEntry>::iterator entry)
{
    node_cache_lru_.splice(node_cache_lru_.end(), node_cache_lru_, entry->second.lru_position);
}

void HybridRunner::clear_node_cache() noexcept
{
    persistent_node_cache_.clear();
    node_cache_lru_.clear();
}

void HybridRunner::insert_cache_entry(const NodeIndex &node, const double *values)
{
    const std::uint64_t key = node_key(node);
    auto found = persistent_node_cache_.find(key);
    if (found != persistent_node_cache_.end())
    {
        std::copy(values, values + kPairCount, found->second.values.begin());
        touch_cache_entry(found);
        return;
    }

    node_cache_lru_.push_back(key);
    PersistentNodeEntry entry;
    std::copy(values, values + kPairCount, entry.values.begin());
    entry.lru_position = std::prev(node_cache_lru_.end());
    persistent_node_cache_.emplace(key, std::move(entry));

    while (persistent_node_cache_.size() > node_cache_capacity_)
    {
        const std::uint64_t evicted_key = node_cache_lru_.front();
        node_cache_lru_.pop_front();
        persistent_node_cache_.erase(evicted_key);
    }
}

void HybridRunner::precompute_baseline_grid()
{
    const std::size_t nx = artifact_.xs.size();
    const std::size_t nmu = artifact_.mus.size();
    if (nx == 0 || nmu == 0)
    {
        throw std::runtime_error("hybrid artifact has empty interpolation axes");
    }
    mu2_axis_.resize(nmu);
    for (std::size_t imu = 0; imu < nmu; ++imu)
    {
        mu2_axis_[imu] = std::exp(artifact_.mus[imu]);
    }

    baseline_leg_grid_.resize(nx * nmu);
    for (std::size_t imu = 0; imu < nmu; ++imu)
    {
        for (std::size_t ix = 0; ix < nx; ++ix)
        {
            baseline_leg_grid_[imu * nx + ix] =
                baseline_.evaluate_leg(artifact_.xs[ix], mu2_axis_[imu]);
        }
    }
}

const PDFxTMDBaseline::LegValues &HybridRunner::baseline_leg(std::uint16_t ix,
                                                             std::uint16_t imu) const
{
    const std::size_t nx = artifact_.xs.size();
    const std::size_t index = static_cast<std::size_t>(imu) * nx + ix;
    if (index >= baseline_leg_grid_.size())
    {
        throw std::out_of_range("hybrid baseline leg index is out of range");
    }
    return baseline_leg_grid_[index];
}

std::vector<double> HybridRunner::evaluate_nodes(const std::vector<NodeIndex> &nodes)
{
    if (nodes.empty())
    {
        return {};
    }

    std::vector<double> x1(nodes.size());
    std::vector<double> x2(nodes.size());
    std::vector<double> mu1_2(nodes.size());
    std::vector<double> mu2_2(nodes.size());
    for (std::size_t row = 0; row < nodes.size(); ++row)
    {
        const NodeIndex node = nodes[row];
        x1[row] = artifact_.xs.at(node.ix1);
        x2[row] = artifact_.xs.at(node.ix2);
        mu1_2[row] = mu2_axis_.at(node.imu1);
        mu2_2[row] = mu2_axis_.at(node.imu2);
    }
    std::vector<double> baseline(nodes.size() * kPairCount, 0.0);
    for (std::size_t row = 0; row < nodes.size(); ++row)
    {
        if (x1[row] + x2[row] >= 1.0)
        {
            continue;
        }
        const NodeIndex node = nodes[row];
        const auto &leg1 = baseline_leg(node.ix1, node.imu1);
        const auto &leg2 = baseline_leg(node.ix2, node.imu2);
        double *const baseline_row = baseline.data() + row * kPairCount;
        PDFxTMDBaseline::phase_space_pairs(leg1, leg2, x1[row], x2[row], baseline_row);
    }
    std::vector<double> values = network_predict(x1, x2, mu1_2, mu2_2, baseline);

    if (values.size() != nodes.size() * kPairCount)
    {
        throw std::runtime_error("neural node provider returned an invalid shape");
    }

    for (std::size_t row = 0; row < nodes.size(); ++row)
    {
        const double weight = x1[row] * x2[row];
        if (!(weight > 0.0))
        {
            throw std::runtime_error("native hybrid node has a non-positive x weight");
        }
        double *const core = values.data() + row * kPairCount;
        for (std::size_t pair = 0; pair < kPairCount; ++pair)
        {
            core[pair] /= weight;
        }
        if (x1[row] + x2[row] > 1.0 - kPDFxTMDEps8)
        {
            std::fill(core, core + kPairCount, 0.0);
        }
        artifact_.apply_overrides(nodes[row], core, kPairCount);
    }
    return values;
}

HybridRunner::NodeCache HybridRunner::build_node_cache(std::vector<NodeIndex> nodes)
{
    NodeCache cache;
    cache.nodes = std::move(nodes);
    cache.rows.resize(cache.nodes.size(), nullptr);
    if (cache.nodes.empty())
    {
        return cache;
    }

    // A zero capacity is useful as a benchmark/control mode. Also bypass the
    // persistent LRU for a batch whose union of required nodes is larger than
    // the cache itself; otherwise early nodes could be evicted before the
    // batch interpolation stage consumes them.
    if (node_cache_capacity_ == 0 || cache.nodes.size() > node_cache_capacity_)
    {
        cache.owned_values = evaluate_nodes(cache.nodes);
        for (std::size_t row = 0; row < cache.nodes.size(); ++row)
        {
            cache.rows[row] = cache.owned_values.data() + row * kPairCount;
        }
        return cache;
    }

    std::vector<NodeIndex> missing;
    missing.reserve(cache.nodes.size());
    for (const NodeIndex &node : cache.nodes)
    {
        auto found = persistent_node_cache_.find(node_key(node));
        if (found == persistent_node_cache_.end())
        {
            missing.push_back(node);
            continue;
        }
        touch_cache_entry(found);
    }
    if (!missing.empty())
    {
        const std::vector<double> missing_values = evaluate_nodes(missing);
        for (std::size_t row = 0; row < missing.size(); ++row)
        {
            insert_cache_entry(missing[row], missing_values.data() + row * kPairCount);
        }
    }

    // Resolve pointers only after all insertions/evictions have completed.
    // References to unordered_map values survive rehashing, but this second
    // lookup also makes the lifetime/eviction rule explicit and simple.
    for (std::size_t row = 0; row < cache.nodes.size(); ++row)
    {
        auto found = persistent_node_cache_.find(node_key(cache.nodes[row]));
        if (found == persistent_node_cache_.end())
        {
            throw std::runtime_error("persistent hybrid node cache lost a required node");
        }
        touch_cache_entry(found);
        cache.rows[row] = found->second.values.data();
    }

    return cache;
}

bool HybridRunner::heavy_active(double mu1_2, double mu2_2, std::size_t public_pair_index) const
{
    const auto pair = artifact_.pid_pairs.at(public_pair_index);
    const double charm2 = artifact_.charm_mass * artifact_.charm_mass;
    const double bottom2 = artifact_.bottom_mass * artifact_.bottom_mass;
    const auto active_leg = [&](int pid, double scale) {
        const int absolute = std::abs(pid);
        return (absolute != 4 || scale >= charm2) && (absolute != 5 || scale >= bottom2);
    };
    return active_leg(pair.first, mu1_2) && active_leg(pair.second, mu2_2);
}

std::vector<double> HybridRunner::predict_batch(const std::vector<double> &x1,
                                                const std::vector<double> &x2,
                                                const std::vector<double> &mu1_2,
                                                const std::vector<double> &mu2_2)
{
    const std::size_t batch = x1.size();
    if (x2.size() != batch || mu1_2.size() != batch || mu2_2.size() != batch)
    {
        throw std::invalid_argument("All coordinate arrays must have equal length");
    }
    if (batch == 0)
    {
        return {};
    }

    std::vector<double> output(batch * kPairCount, 0.0);
    std::vector<InterpolationStencil> stencils(batch);
    std::vector<unsigned char> active(batch, 0);
    std::vector<NodeIndex> planned;
    planned.reserve(batch * 16);

    for (std::size_t point = 0; point < batch; ++point)
    {
        if (!(mu1_2[point] > 0.0) || !(mu2_2[point] > 0.0))
        {
            throw std::invalid_argument("DPD interpolation requires positive mu^2");
        }
        const double first_x = std::max(x1[point], kPDFxTMDEps6);
        const double second_x = std::max(x2[point], kPDFxTMDEps6);
        if (first_x + second_x > 1.0 - kPDFxTMDEps8)
        {
            continue;
        }
        maybe_log_query(first_x, second_x, mu1_2[point], mu2_2[point]);
        const double first_t =
            std::clamp(std::log(mu1_2[point]), artifact_.mus.front(), artifact_.mus.back());
        const double second_t =
            std::clamp(std::log(mu2_2[point]), artifact_.mus.front(), artifact_.mus.back());
        stencils[point] =
            make_interpolation_stencil(artifact_, first_x, second_x, first_t, second_t);
        active[point] = 1;
        const auto &stencil = stencils[point];
        planned.insert(planned.end(), stencil.nodes.begin(),
                       stencil.nodes.begin() + stencil.node_count);
    }
    std::sort(planned.begin(), planned.end());
    planned.erase(std::unique(planned.begin(), planned.end()), planned.end());

    const NodeCache cache = build_node_cache(std::move(planned));

    std::vector<StencilNodeRows> resolved(batch);
    for (std::size_t point = 0; point < batch; ++point)
    {
        if (!active[point])
        {
            continue;
        }
        const auto &stencil = stencils[point];
        for (std::size_t slot = 0; slot < stencil.node_count; ++slot)
        {
            resolved[point][slot] = cache.row_values(cache.row_index(stencil.nodes[slot]));
        }
    }
    std::vector<ChannelValues> interpolated(batch);
    for (std::size_t point = 0; point < batch; ++point)
    {
        if (!active[point])
        {
            continue;
        }
        interpolate_core_channels(artifact_, stencils[point], resolved[point], public_channels_,
                                  interpolated[point]);
    }
    for (std::size_t point = 0; point < batch; ++point)
    {
        if (!active[point])
        {
            continue;
        }
        const double weight = stencils[point].x1 * stencils[point].x2;
        double *const output_row = output.data() + point * kPairCount;
        for (std::size_t pair = 0; pair < kPairCount; ++pair)
        {
            if (!heavy_active(mu1_2[point], mu2_2[point], pair))
            {
                continue;
            }
            output_row[pair] = weight * interpolated[point][pair];
        }
    }
    return output;
}

std::vector<double> HybridRunner::predict(double x1, double x2, double mu1_2, double mu2_2)
{
    if (!(mu1_2 > 0.0) || !(mu2_2 > 0.0))
    {
        throw std::invalid_argument("DPD interpolation requires positive mu^2");
    }

    std::vector<double> output(kPairCount, 0.0);
    const double query_first_x = std::max(x1, kPDFxTMDEps6);
    const double query_second_x = std::max(x2, kPDFxTMDEps6);
    if (query_first_x + query_second_x > 1.0 - kPDFxTMDEps8)
    {
        return output;
    }

    // Optional real-workload capture. Record only interpolation-eligible calls
    // and write the same clamped x coordinates used by the runner. This keeps
    // the resulting CSV directly replayable by --workload file.
    maybe_log_query(query_first_x, query_second_x, mu1_2, mu2_2);

    const double first_t = std::clamp(std::log(mu1_2), artifact_.mus.front(), artifact_.mus.back());
    const double second_t =
        std::clamp(std::log(mu2_2), artifact_.mus.front(), artifact_.mus.back());
    const InterpolationStencil direct_stencil =
        make_interpolation_stencil(artifact_, query_first_x, query_second_x, first_t, second_t);
    std::vector<NodeIndex> planned;
    planned.reserve(direct_stencil.node_count);
    planned.insert(planned.end(), direct_stencil.nodes.begin(),
                   direct_stencil.nodes.begin() + direct_stencil.node_count);
    std::sort(planned.begin(), planned.end());
    planned.erase(std::unique(planned.begin(), planned.end()), planned.end());

    const NodeCache cache = build_node_cache(std::move(planned));

    const auto resolve_rows = [&](const InterpolationStencil &stencil) {
        StencilNodeRows rows{};
        for (std::size_t slot = 0; slot < stencil.node_count; ++slot)
        {
            rows[slot] = cache.row_values(cache.row_index(stencil.nodes[slot]));
        }
        return rows;
    };
    const StencilNodeRows direct_rows = resolve_rows(direct_stencil);
    ChannelValues direct_values{};
    interpolate_core_channels(artifact_, direct_stencil, direct_rows, public_channels_,
                              direct_values);

    const double weight = direct_stencil.x1 * direct_stencil.x2;
    for (std::size_t pair = 0; pair < kPairCount; ++pair)
    {
        if (!heavy_active(mu1_2, mu2_2, pair))
        {
            continue;
        }
        output[pair] = weight * direct_values[pair];
    }
    return output;
}

} // namespace PDFxTMD::DPDHybrid
