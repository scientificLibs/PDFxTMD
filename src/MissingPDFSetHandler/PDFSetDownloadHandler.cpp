#include "MissingPDFSetHandler/PDFSetDownloadHandler.h"
#include "Common/EnvUtils.h"
#include "Common/FileUtils.h"
#include "Common/PartonUtils.h"
#include "Common/StringUtils.h"
#include "MissingPDFSetHandler/ArchiveExtractorCommand.h"
#include "MissingPDFSetHandler/DownloadUrlCommand.h"
#include "MissingPDFSetHandler/PathSelectionCommand.h"
#include "MissingPDFSetHandler/RepoSelectionCommand.h"
#include <iostream>
#include <regex>
#include <variant>

namespace PDFxTMD
{
void failureMessage()
{
    std::cerr << "Error occurred in the current state." << std::endl;
    std::cerr << "Quit (Q) or press any other key." << std::endl;
}
PDFSetDownloadHandler::PDFSetDownloadHandler()
{
    m_commands = {std::make_shared<pathSelectionCommnand>(),
                  std::make_shared<RepoSelectionCommand>(), std::make_shared<DownloadUrlCommand>(),
                  std::make_shared<ArchiveExtractorCommand>()};
}
bool PDFSetDownloadHandler::Start(const std::string &pdfName)
{
    if (StandardInfoFilePath(pdfName).second == ErrorType::None)
    {
        return true;
    }
    m_context["PDFSet"] = pdfName;
    for (auto &&command_ : m_commands)
    {
        do
        {
            if (!command_->execute(m_context))
            {
                if (m_context.find("Error") != m_context.end())
                {
                    std::string errVal = std::get<std::string>(m_context["Error"]);
                    if (errVal == "Q")
                    {
                        std::cout << "Quit..." << std::endl;
                        return false;
                    }
                    std::string description = "";
                    if (std::holds_alternative<std::string>(m_context["Error"]))
                    {
                        description = std::get<std::string>(m_context["Error"]);
                    }
                    std::cerr << "Failed: " << description << std::endl;
                }

                failureMessage();
                std::string response;
                std::getline(std::cin, response);
                if (response == "Q")
                {
                    std::cout << "Quit..." << std::endl;
                    return false;
                }

                continue;
            }
            break;
        } while (true);
    }
    return true;
}
} // namespace PDFxTMD
