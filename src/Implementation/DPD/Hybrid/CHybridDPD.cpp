#include "PDFxTMDLib/Implementation/DPD/Hybrid/CHybridDPD.h"

#include "PDFxTMDLib/Common/Exception.h"
#ifdef PDFXTMD_HAS_DPD_HYBRID
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include "dpd_hybrid_runner.hpp"

#include <array>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <utility>
#endif

namespace PDFxTMD
{

#ifdef PDFXTMD_HAS_DPD_HYBRID
namespace
{

constexpr std::array<int, DPDHybrid::HybridRunner::kPairCount / 11> kPids = {
    -5, -4, -3, -2, -1, 21, 1, 2, 3, 4, 5};

int canonicalPid(PartonFlavor flavor)
{
    const int pid = static_cast<int>(flavor);
    return pid == 0 ? 21 : pid;
}

int pidIndex(PartonFlavor flavor)
{
    const int pid = canonicalPid(flavor);
    for (std::size_t index = 0; index < kPids.size(); ++index)
    {
        if (kPids[index] == pid)
            return static_cast<int>(index);
    }
    return -1;
}

std::filesystem::path nativeModelPath(const std::filesystem::path &artifactPath)
{
    std::filesystem::path result = artifactPath;
    result.replace_extension(".native");
    return result;
}

} // namespace

struct CHybridDPD::State
{
    State(const std::string &nativePath,
          const std::string &artifactPath,
          std::optional<std::size_t> cacheCapacity,
          std::optional<int> threadCount)
        : runner(nativePath, artifactPath, cacheCapacity, threadCount)
    {
    }

    std::mutex mutex;
    DPDHybrid::HybridRunner runner;
    bool hasLastQuery = false;
    double lastX1 = 0.0;
    double lastMu1_2 = 0.0;
    double lastX2 = 0.0;
    double lastMu2_2 = 0.0;
    std::vector<double> lastValues;

    const std::vector<double> &evaluate(double x1,
                                        double mu1_2,
                                        double x2,
                                        double mu2_2)
    {
        if (!hasLastQuery || x1 != lastX1 || mu1_2 != lastMu1_2 ||
            x2 != lastX2 || mu2_2 != lastMu2_2)
        {
            lastValues = runner.predict(x1, x2, mu1_2, mu2_2);
            lastX1 = x1;
            lastMu1_2 = mu1_2;
            lastX2 = x2;
            lastMu2_2 = mu2_2;
            hasLastQuery = true;
        }
        return lastValues;
    }
};

CHybridDPD::CHybridDPD(const std::string &pdfSetName, int setMember)
{
    const auto infoPath = StandardInfoFilePath(pdfSetName);
    if (infoPath.second != ErrorType::None || !infoPath.first.has_value())
        throw FileLoadException("PDF set metadata " + pdfSetName + " not found!");
    const auto info = YamlStandardPDFInfoReader(*infoPath.first);
    if (info.second != ErrorType::None || !info.first.has_value())
        throw FileLoadException("Unable to read metadata for PDF set " + pdfSetName);
    if (info.first->HybridThreads.has_value() && *info.first->HybridThreads <= 0)
        throw FileLoadException("HybridThreads must be a positive integer");

    const auto filePath = StandardPDFSetPath(pdfSetName, setMember);
    if (filePath.second != ErrorType::None || !filePath.first.has_value())
        throw FileLoadException("PDF set " + pdfSetName + " not found!");

    const std::filesystem::path artifactPath(*filePath.first);
    const std::filesystem::path nativePath = nativeModelPath(artifactPath);
    if (!std::filesystem::is_regular_file(nativePath))
    {
        throw FileLoadException(
            "Hybrid DPD neural model not found: " + nativePath.string());
    }

    state_ = std::make_shared<State>(
        nativePath.string(),
        artifactPath.string(),
        info.first->HybridCacheCapacity,
        info.first->HybridThreads);
}

CHybridDPD::~CHybridDPD() = default;

double CHybridDPD::dpd(PartonFlavor flavor1,
                         PartonFlavor flavor2,
                         double x1,
                         double mu1_2,
                         double x2,
                         double mu2_2)
{
    const int first = pidIndex(flavor1);
    const int second = pidIndex(flavor2);
    if (first < 0 || second < 0)
        return 0.0;

    std::lock_guard<std::mutex> lock(state_->mutex);
    const std::vector<double> &values =
        state_->evaluate(x1, mu1_2, x2, mu2_2);
    return values[static_cast<std::size_t>(first) * kPids.size() +
                  static_cast<std::size_t>(second)];
}
#else
struct CHybridDPD::State
{
};

CHybridDPD::CHybridDPD(const std::string &, int)
{
    throw NotSupportError(
        "PDFxTMD-DPDH1 requires a PDFxTMD build with hybrid DPD support");
}

CHybridDPD::~CHybridDPD() = default;

double CHybridDPD::dpd(PartonFlavor,
                         PartonFlavor,
                         double,
                         double,
                         double,
                         double)
{
    throw NotSupportError(
        "PDFxTMD-DPDH1 requires a PDFxTMD build with hybrid DPD support");
}
#endif

} // namespace PDFxTMD
