#include <map>
#include <vector>
#include <optional>
#include <PDFxTMDLib/Common/PartonUtils.h>

namespace PDFxTMD
{

enum class AlphasType
{
    ipol
};


enum class AlphasFlavorScheme
{
    fixed,
    variable
};


struct YamlCouplingInfo
{
    AlphasType alphasOrder = AlphasType::ipol;
    OrderQCD alphsOrder = OrderQCD::NLO;
    std::vector<double> mu_vec;
    std::vector<double> alphas_vec;
    double alphasLambda3 = -1;
    double alphasLambda4 = -1;
    double alphasLambda5 = -1;
    double MZ = 91.1876;
    std::map<PartonFlavor, double> quarkThreshhold;
    std::map<PartonFlavor, double> quarkMasses;
    AlphasFlavorScheme flavorScheme = AlphasFlavorScheme::variable;
    int numFlavors = DEFULT_NUM_FLAVORS;
    AlphasType alphaCalcMethod = AlphasType::ipol;
};

std::pair<std::optional<YamlCouplingInfo>, ErrorType> YamlCouplingInfoReader(
    const std::string &yamlInfoPath);
}