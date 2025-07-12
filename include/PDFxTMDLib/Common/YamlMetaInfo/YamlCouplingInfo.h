#pragma once
#include <PDFxTMDLib/Common/PartonUtils.h>
#include <map>
#include <optional>
#include <vector>

namespace PDFxTMD
{

enum class AlphasType
{
    None,
    ipol,
    analytic,
    ode
};

enum class AlphasFlavorScheme
{
    fixed,
    variable
};

struct YamlCouplingInfo
{
    OrderQCD alphasOrder = OrderQCD::NLO;
    std::vector<double> mu_vec;
    std::vector<double> alphas_vec;
    double alphasLambda3 = -1;
    double alphasLambda4 = -1;
    double alphasLambda5 = -1;
    double MZ = 91.1876;
    double AlphaS_MZ = -1;
    double AlphaS_Reference = -1;
    double AlphaS_MassReference = -1;
    std::map<PartonFlavor, double> quarkThreshhold;
    std::map<PartonFlavor, double> quarkMasses;
    AlphasFlavorScheme flavorScheme = AlphasFlavorScheme::variable;
    int AlphaS_NumFlavors = DEFULT_NUM_FLAVORS;
    AlphasType alphaCalcMethod = AlphasType::None;
};

// Calculate the number of active quark flavours at energy scale Q2
std::pair<std::optional<YamlCouplingInfo>, ErrorType> YamlCouplingInfoReader(
    const std::string &yamlInfoPath);
int numFlavorsQ2(double q2, const YamlCouplingInfo &couplingInfo, int nfMin = -1, int nfMax = -1);
// Calculate a beta function given the number of active flavours
double beta(OrderQCD qcdOrder, int nf);

// Calculate beta functions given the number of active flavours
std::vector<double> betas(int nf);
} // namespace PDFxTMD