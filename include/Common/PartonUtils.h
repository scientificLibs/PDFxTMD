#pragma once
#include <exception>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__)
#define FOLDER_SEP "/"
#elif defined(_WIN32)
#define FOLDER_SEP "\\"
#endif
#define STD_PDF_INFO_EXTENSION ".info"
#define STD_PDF_DATA_EXTENSION ".dat"
#define ENV_PATH_VARIABLE "PDFxTMD_PATH"

namespace PDFxTMD
{
// Enum to define policies for handling out of range values
enum class OffBoundaryPolicy
{
    Zero,   // Return 0 for out of range
    Clamp,  // Clamp the value to the nearest limit
    Ignore, // Ignore and keep the value unchanged
    Throw   // Throw an exception for out of range
};
enum class PhaseSpaceComponent
{
    X,
    Kt2,
    Q2
};
struct OffBoundaryPolicies
{
    OffBoundaryPolicy xValPolicy = OffBoundaryPolicy::Throw;
    OffBoundaryPolicy pValPolicy = OffBoundaryPolicy::Throw;
};

struct OffBoundaryPoliciesUPDF : OffBoundaryPolicies
{
    OffBoundaryPolicy ktValPolicy = OffBoundaryPolicy::Throw;
};
enum class ErrorType
{
    None,                    // No error
    CONFIG_KeyNotFound,      // The key does not exist
    CONFIG_ConversionFailed, // Conversion to the desired type
                             // failed,
    FILE_NOT_FOUND = 10,
};
enum class OrderQCD
{
    LO,
    NLO,
    NNLO,
    N3LO
};
enum PartonFlavor
{
    tbar = -6,
    bbar = -5,
    cbar = -4,
    sbar = -3,
    ubar = -2,
    dbar = -1,
    gNS = 0, // non-standard g
    d = 1,
    u = 2,
    s = 3,
    c = 4,
    b = 5,
    t = 6,
    g = 21,
    photon = 22,
    // Non stadard ew partciles
    z0 = 100,
    wplus,
    wminus,
    higgs
};

std::vector<std::string> splitPaths(const std::string &paths);
bool hasWriteAccess(const std::string &path);
double _extrapolateLinear(double x, double xl, double xh, double yl, double yh);
std::string StandardPDFNaming(const std::string &pdfName, int set);
std::vector<std::string> GetEnviormentalVariablePaths(
    const std::string &envVariable = ENV_PATH_VARIABLE, char delimiter = ':');
std::vector<std::string> split(const std::string &str, char delimiter);
/// Format an integer @a val as a zero-padded string of length
/// @a nchars
std::string to_str_zeropad(int val, size_t nchars = 4);
std::pair<std::optional<std::string>, ErrorType> StandardInfoFilePath(
    const std::string &pdfSetName);
std::pair<std::optional<std::string>, ErrorType> StandardPDFSetPath(const std::string &pdfSetName,
                                                                    int set);
} // namespace PDFxTMD
