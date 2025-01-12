#include "MissingPDFSetHandler/DownloadUrlCommand.h"
#include "curl/curl.h"
#include <fstream>
#include <iostream>

namespace PDFxTMD
{
// Callback function to write received data into the file
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::ofstream *file = static_cast<std::ofstream *>(userdata);
    file->write(static_cast<const char *>(ptr), size * nmemb);
    return size * nmemb;
}

bool DownloadUrlCommand::execute(StandardTypeMap &context)
{
    using namespace std::literals;
    std::string url;
    if (std::holds_alternative<std::string>(context["URL"]))
    {
        url = std::get<std::string>(context["URL"]);
    }
    else
    {
        context["Error"] = "URL key is not found in context";
        return false;
    }
    std::string PDFSet;
    if (std::holds_alternative<std::string>(context["PDFSet"]))
    {
        PDFSet = std::get<std::string>(context["PDFSet"]);
    }
    else
    {
        context["Error"] = "PDFSet key is not found in context";
        return false;
    }
    std::string SelectedPath;
    if (std::holds_alternative<std::string>(context["SelectedPath"]))
    {
        SelectedPath = std::get<std::string>(context["SelectedPath"]);
    }
    else
    {
        context["Error"] = "SelectedPath key is not found in context";
        return false;
    }
    std::string downloadUrl = url;
    CURL *curl;
    CURLcode res;

    // Open the file for writing
    std::string filePath = SelectedPath + "/" + PDFSet + ".tgz";
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        context["Error"] = "Failed to open file for writing: " + filePath;
        return false;
    }

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl)
    {
        context["Error"] = "Failed to initialize curl";
        return false;
    }

    // Set libcurl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);       // Follow redirects
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0"); // Set a User-Agent string
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL verification (if necessary)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // Disable host verification (if necessary)

    // Perform the request
    res = curl_easy_perform(curl);

    // Cleanup libcurl
    curl_easy_cleanup(curl);

    // Close the file
    file.close();

    // Check for errors
    if (res != CURLE_OK)
    {
        context["Error"] = "curl_easy_perform() failed: "s + curl_easy_strerror(res);
        return false;
    }
    context["PDFSetAddress"] = filePath;
    return true;
}

} // namespace PDFxTMD