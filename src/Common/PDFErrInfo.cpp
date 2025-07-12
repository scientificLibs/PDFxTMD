#include <PDFxTMDLib/Common/PDFErrInfo.h>

namespace PDFxTMD
{
std::vector<std::string> PDFErrInfo::qpartNames() const
{
    std::vector<std::string> rtn;
    rtn.reserve(qparts.size());
    for (size_t iq = 0; iq < qparts.size(); ++iq)
        rtn.emplace_back(qpartName(iq));
    return rtn;
}

size_t PDFErrInfo::nmemCore() const
{
    return qparts[0][0].second;
}
size_t PDFErrInfo::nmemPar() const
{
    size_t rtn = 0;
    for (size_t i = 1; i < qparts.size(); ++i)
    {
        for (size_t j = 1; j < qparts[i].size(); ++j)
        {
            rtn += qparts[i][j].second;
        }
    }
    return rtn;
}
std::string PDFErrInfo::qpartName(size_t iq) const
{
    const EnvParts &eparts = qparts[iq];
    if (eparts.size() == 1)
        return eparts[0].first;
    std::string qname = "[";
    for (size_t ie = 0; ie < eparts.size(); ++ie)
    {
        qname += (ie == 0 ? "" : ",") + eparts[ie].first;
    }
    qname += "]";
    return qname;
}
PDFErrInfo PDFErrInfo::CalculateErrorInfo(const YamlErrorInfo &yamlErrInfo)
{
    PDFErrInfo::QuadParts qparts;

    // Loop over the quadrature parts, separated by +  signs, after extracting the core part
    std::vector<std::string> quadstrs = split(yamlErrInfo.ErrorType, '+');
    qparts.reserve(quadstrs.size());
    qparts.push_back({{quadstrs[0], 0}});
    size_t nmempar = 0;

    for (size_t iq = 1; iq < quadstrs.size(); ++iq)
    {
        PDFErrInfo::EnvParts eparts;

        // Loop over any envelope components, separated by * signs
        const std::string &qstr = quadstrs[iq];
        std::vector<std::string> envstrs = split(qstr, '*');
        for (const std::string &estr : envstrs)
        {
            // Determine if a pair or singleton variation
            size_t esize = (StartsWith(estr, "#") || StartsWith(estr, "$")) ? 1 : 2;
            std::string ename = estr;
            eparts.push_back({ename, esize});
            nmempar += esize;
        }

        qparts.push_back(eparts);
    }

    // Finally, compute and set the size of the core errors
    qparts[0][0].second = yamlErrInfo.errorSize - nmempar;

    return PDFErrInfo(qparts, yamlErrInfo.ErrorConfLevel, yamlErrInfo.ErrorType);
}
} // namespace PDFxTMD
