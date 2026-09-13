#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace PDFxTMD::DPDHybrid {

struct NodeIndex {
    std::uint16_t ix1{};
    std::uint16_t ix2{};
    std::uint16_t imu1{};
    std::uint16_t imu2{};

    bool operator<(const NodeIndex& other) const noexcept;
    bool operator==(const NodeIndex& other) const noexcept;
};

struct OverrideRecord {
    NodeIndex node;
    std::uint16_t pair_index{};
    double value{};
    std::uint64_t packed{};
};

class HybridArtifact {
public:
    static constexpr std::uint32_t kFormatVersion = 1;
    static constexpr std::size_t kPidCount = 11;
    static constexpr std::size_t kPairCount = 121;

    static HybridArtifact load(const std::string& path);

    std::uint64_t packed_key(const NodeIndex& node, std::size_t pair_index) const;
    void apply_overrides(const NodeIndex& node, std::vector<double>& core) const;
    void apply_overrides(
        const NodeIndex& node,
        double* core,
        std::size_t core_size
    ) const;

    std::vector<double> xs;
    std::vector<double> mus;
    double xmax{};
    int nmcor{};
    double charm_mass{};
    double bottom_mass{};
    std::string spdf_set;
    int spdf_member{};
    std::string dpd_set;
    std::vector<int> pids;
    std::vector<std::pair<int, int>> pid_pairs;
    std::array<std::uint8_t, 32> checkpoint_sha256{};
    std::array<std::uint8_t, 32> grid_sha256{};
    std::array<std::uint8_t, 32> source_override_sha256{};
    std::vector<OverrideRecord> overrides;

    std::size_t nx() const noexcept { return xs.size(); }
    std::size_t nmu() const noexcept { return mus.size(); }
};

}  // namespace PDFxTMD::DPDHybrid
