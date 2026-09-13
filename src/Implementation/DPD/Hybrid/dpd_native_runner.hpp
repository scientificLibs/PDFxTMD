#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace PDFxTMD::DPDHybrid {

class NativeRunner {
public:
    static constexpr std::size_t kPairCount = 121;

    explicit NativeRunner(const std::string& model_path);
    ~NativeRunner();
    NativeRunner(NativeRunner&&) noexcept;
    NativeRunner& operator=(NativeRunner&&) noexcept;
    NativeRunner(const NativeRunner&) = delete;
    NativeRunner& operator=(const NativeRunner&) = delete;

    std::string metadata_value(const char* key) const;

    std::vector<double> predict(
        const std::vector<double>& x1,
        const std::vector<double>& x2,
        const std::vector<double>& mu1_2,
        const std::vector<double>& mu2_2,
        const std::vector<double>& spdf_baseline
    );

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace PDFxTMD::DPDHybrid
