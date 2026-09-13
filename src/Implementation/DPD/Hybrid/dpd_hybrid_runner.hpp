#pragma once

#include "dpd_native_runner.hpp"
#include "hybrid_artifact.hpp"
#include "pdfxtmd_hybrid_interpolator.hpp"
#include "pdfxtmd_baseline.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace PDFxTMD::DPDHybrid {

class HybridRunner {
public:
    static constexpr std::size_t kPairCount = 121;
    static constexpr std::size_t kDefaultNodeCacheNodes = 50000;

    HybridRunner(
        const std::string& network_model_path,
        const std::string& hybrid_artifact_path,
        std::optional<std::size_t> node_cache_capacity = std::nullopt,
        std::optional<int> thread_count = std::nullopt
    );

    std::vector<double> predict(
        double x1,
        double x2,
        double mu1_2,
        double mu2_2
    );

    // Vectorized query path. All points in the call share one node-planning
    // pass and one neural evaluation for the union of missing stencil nodes.
    // This is the preferred path for non-local/random workloads.
    std::vector<double> predict_batch(
        const std::vector<double>& x1,
        const std::vector<double>& x2,
        const std::vector<double>& mu1_2,
        const std::vector<double>& mu2_2
    );

    const HybridArtifact& artifact() const noexcept { return artifact_; }
    std::size_t node_cache_capacity() const noexcept { return node_cache_capacity_; }
    std::size_t node_cache_size() const noexcept { return persistent_node_cache_.size(); }
    int thread_count() const noexcept { return thread_count_; }

    // Benchmark/diagnostic helpers. These do not change model parameters or artifacts.
    void clear_node_cache() noexcept;

private:
    struct NodeCache {
        std::vector<NodeIndex> nodes;
        std::vector<const double*> rows;
        // Used only when persistent caching is disabled.
        std::vector<double> owned_values;

        std::size_t row_index(const NodeIndex& node) const;
        const double* row_values(std::size_t row) const;
    };

    struct PersistentNodeEntry {
        std::array<double, kPairCount> values{};
        std::list<std::uint64_t>::iterator lru_position;
    };

    static std::uint64_t node_key(const NodeIndex& node) noexcept;
    void touch_cache_entry(
        std::unordered_map<std::uint64_t, PersistentNodeEntry>::iterator entry
    );
    void insert_cache_entry(const NodeIndex& node, const double* values);
    std::string network_metadata_value(const char* key) const;
    std::vector<double> network_predict(
        const std::vector<double>& x1,
        const std::vector<double>& x2,
        const std::vector<double>& mu1_2,
        const std::vector<double>& mu2_2,
        const std::vector<double>& spdf_baseline
    );
    void precompute_baseline_grid();
    const PDFxTMDBaseline::LegValues& baseline_leg(std::uint16_t ix, std::uint16_t imu) const;
    std::vector<double> evaluate_nodes(const std::vector<NodeIndex>& nodes);
    NodeCache build_node_cache(std::vector<NodeIndex> nodes);
    void maybe_log_query(double x1, double x2, double mu1_2, double mu2_2);
    bool heavy_active(
        double mu1_2,
        double mu2_2,
        std::size_t public_pair_index
    ) const;

    HybridArtifact artifact_;
    std::size_t node_cache_capacity_ = kDefaultNodeCacheNodes;
    int thread_count_ = 0;
    std::ofstream query_log_;
    std::string query_log_path_;
    std::size_t query_log_max_ = 0;
    std::size_t query_log_count_ = 0;
    std::size_t query_log_flush_every_ = 10000;
    std::list<std::uint64_t> node_cache_lru_;
    std::unordered_map<std::uint64_t, PersistentNodeEntry> persistent_node_cache_;
    NativeRunner native_network_;
    PDFxTMDBaseline baseline_;
    std::vector<double> mu2_axis_;
    std::vector<PDFxTMDBaseline::LegValues> baseline_leg_grid_;
    std::vector<std::size_t> public_channels_;
};

}  // namespace PDFxTMD::DPDHybrid
