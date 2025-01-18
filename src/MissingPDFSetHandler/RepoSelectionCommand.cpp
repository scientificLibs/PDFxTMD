#include "MissingPDFSetHandler/RepoSelectionCommand.h"
#include "Common/StringUtils.h"
#include <curl/curl.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

namespace PDFxTMD
{
bool CheckUrl(const std::string &url, StandardTypeMap &context)
{
    using namespace std::literals;
    CURL *curl;
    CURLcode res;
    long httpCode = 0;

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl)
    {
        context["Error"] = "Failed to initialize curl";
        return false;
    }

    // Set libcurl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);               // Perform HEAD request
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);       // Follow redirects
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0"); // Set User-Agent string

    // Perform the request
    res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        // Get the HTTP response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        if (httpCode == 200)
        {
            curl_easy_cleanup(curl);
            return true;
        }
        else
        {
            context["Error"] = "HTTP error code: " + std::to_string(httpCode) + " for URL: " + url;
            return false;
        }
    }
    else
    {
        context["Error"] = "curl_easy_perform() failed: "s + curl_easy_strerror(res);
        return false;
    }

    // Cleanup libcurl
    curl_easy_cleanup(curl);
    return false;
}

// Function to normalize PDFSet names
std::string normalizePDFSetName(const std::string &pdfSet)
{
    std::ostringstream normalized;
    for (char c : pdfSet)
    {
        if (c == '+' || c == '=')
        {
            normalized << '%' << std::uppercase << std::hex << static_cast<int>(c);
        }
        else
        {
            normalized << c;
        }
    }
    return normalized.str();
}

bool RepoSelectionCommand::execute(StandardTypeMap &context)
{
    std::string PDFSet;
    if (std::holds_alternative<std::string>(context["PDFSet"]))
    {
        PDFSet = std::get<std::string>(context["PDFSet"]);
    }
    else
    {
        context["Error"] = "PDFSet key not found in the context";
        return false;
    }
    std::cout << "\nPlease select a PDF repository:\n" << std::endl;

    std::cout << "1. LHAPDF (enter: 1)" << std::endl;
    std::cout << "2. TMDLib (enter: 2)" << std::endl;
    std::cout << "3. Custom repository (enter URL)" << std::endl;
    std::cout << "   Example: https://www.example.com/pdfsetName.tgz\n" << std::endl;

    std::cout << "4. Exit (Q)" << std::endl;

    std::string inputLine;
    std::getline(std::cin, inputLine);
    trim(inputLine);

    if (inputLine == "Q")
    {
        context["Error"] = "Q";
        return false;
    }
    if (inputLine == "1")
    {
        std::string normalizedPDFSet = normalizePDFSetName(PDFSet);
        std::string url =
            "http://lhapdfsets.web.cern.ch/lhapdfsets/current/" + normalizedPDFSet + ".tar.gz";
        if (CheckUrl(url, context))
        {
            context["URL"] = url;
            return true;
        }
        return false;
    }
    else if (inputLine == "2")
    {
        std::string normalizedPDFSet = normalizePDFSetName(PDFSet);
        std::string url = "https://syncandshare.desy.de/index.php/s/GjjcwKQC93M979e/"
                          "download?path=%2FTMD%20grid%20files&files=" +
                          normalizedPDFSet + ".tgz";
        if (CheckUrl(url, context))
        {
            context["URL"] = url;
            return true;
        }
        return false;
    }

    // Regex pattern for basic URL validation
    std::regex urlPattern(R"(^https?://[\w\-.]+(:\d+)?(/[\w\-./]*)?$)");
    if (std::regex_match(inputLine, urlPattern))
    {
        if (CheckUrl(inputLine, context))
        {
            context["URL"] = inputLine;
            return true;
        }
        return false;
    }
    else
    {
        KeyValueStore error;
        context["Error"] = "Invalid URL format";
        return false;
    }
}
} // namespace PDFxTMD