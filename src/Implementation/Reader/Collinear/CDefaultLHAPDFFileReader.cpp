#include "Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "Common/Exception.h"
#include "Common/PartonUtils.h"
#include "Common/YamlInfoReader.h"
#include <fstream>
#include <string>

namespace PDFxTMD
{

std::vector<double> CDefaultLHAPDFFileReader::getValues(PhaseSpaceComponent comp) const
{
    std::vector<double> output;
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = m_pdfShape.x_vec;
        break;
    case PhaseSpaceComponent::Q2:
        output = m_pdfShape.mu2_vec;
        break;
    default:
        throw NotSupportError("undefined Phase space component requested");
    }
    return output;
}

std::pair<double, double> CDefaultLHAPDFFileReader::getBoundaryValues(
    PhaseSpaceComponent comp) const
{
    std::pair<double, double> output;
    switch (comp)
    {
    case PhaseSpaceComponent::X:
        output = {std::exp(m_pdfShape.log_x_vec.front()), std::exp(m_pdfShape.log_x_vec.back())};
        break;
    case PhaseSpaceComponent::Q2:
        output = {std::exp(m_pdfShape.log_mu2_vec.front()),
                  std::exp(m_pdfShape.log_mu2_vec.back())};
        break;
    default:
        throw std::runtime_error("undefined Phase space component requested");
    }
    return output;
}
void CDefaultLHAPDFFileReader::read(const std::string &pdfName, int setNumber)
{
    auto filePathPair = StandardPDFSetPath(pdfName, setNumber);
    if (filePathPair.second != ErrorType::None)
    {
        throw FileLoadException("PDF set " + pdfName + " not found!");
    }

    auto infoPathPair = StandardInfoFilePath(pdfName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfName);
    }

    auto pdfStandardInfo = YamlStandardPDFInfoReader<YamlStandardPDFInfo>(*infoPathPair.first);
    if (pdfStandardInfo.second != ErrorType::None)
    {
        throw InvalidFormatException("File " + *infoPathPair.first +
                                     " is not a standard info UPDF file");
    }

    YamlStandardPDFInfo standardPDFInfo = *pdfStandardInfo.first;
    std::ifstream file(*filePathPair.first);
    if (!file.is_open())
    {
        throw PDFxTMD::FileLoadException("Unable to open file: " + *filePathPair.first);
    }

    // Pre-allocate with typical sizes
    m_pdfShape.log_x_vec.reserve(100);
    m_pdfShape.log_mu2_vec.reserve(50);
    std::string line;
    while (std::getline(file, line))
    {
        if (isComment(line) || line.empty())
            continue;

        if (isBlockSeparator(line))
        {
            m_blockNumber++;
            m_blockLine = 0;
            continue;
        }

        if (m_blockNumber == 0)
            continue;

        processDataLine(line, m_pdfShape);
        m_blockLine++;
    }
    m_pdfShape.finalizeXP2();
}

DefaultAllFlavorShape CDefaultLHAPDFFileReader::getData() const
{
    return m_pdfShape;
}

void CDefaultLHAPDFFileReader::readXKnots(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        parser >> value;
        data.x_set.emplace(value);
    }

    if (data.x_set.empty())
    {
        throw std::runtime_error("No x knots found in grid");
    }
}

void CDefaultLHAPDFFileReader::readQ2Knots(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        parser >> value;
        data.mu2_set.emplace(value * value); // Store Q²
    }

    if (data.mu2_set.empty())
    {
        throw std::runtime_error("No Q² knots found in grid");
    }
}

void CDefaultLHAPDFFileReader::readParticleIds(NumParser &parser, DefaultAllFlavorShape &data)
{
    int id;
    while (parser.hasMore())
    {
        parser >> id;
        data.flavors.emplace(static_cast<PartonFlavor>(id));
    }

    if (data.flavors.empty())
    {
        throw std::runtime_error("No particle IDs found in grid");
    }
    size_t gridSize = data.x_set.size() * data.mu2_set.size();
    for (const auto &flavor : data.flavors)
    {
        data.grids[flavor].reserve(data.grids[flavor].size() + gridSize);
    }
}

void CDefaultLHAPDFFileReader::readValues(NumParser &parser, DefaultAllFlavorShape &data)
{
    double value;
    while (parser.hasMore())
    {
        for (const auto &flavor : data.flavors)
        {
            parser >> value;
            if (parser.hasMore())
            {
                data.grids[flavor].push_back(value);
            }
            else
            {
                continue;
            }
        }
    }
}

void CDefaultLHAPDFFileReader::processDataLine(const std::string &line, DefaultAllFlavorShape &data)
{
    NumParser parser(line);

    try
    {
        switch (m_blockLine)
        {
        case 0:
            readXKnots(parser, data);
            break;
        case 1:
            readQ2Knots(parser, data);
            break;
        case 2:
            readParticleIds(parser, data);
            break;
        default:
            readValues(parser, data);
            break;
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Error in block " + std::to_string(m_blockNumber) + ", line " +
                                 std::to_string(m_blockLine) + ": " + e.what());
    }
}

} // namespace PDFxTMD