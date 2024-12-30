#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
namespace PDFxTMD
{
class FileDownloader
{
  public:
    FileDownloader(const std::string &baseUrl) : baseUrl(baseUrl)
    {
    }

    bool download(const std::string &pdfSetName)
    {
        std::string downloadUrl = baseUrl + pdfSetName;
        CURL *curl;
        CURLcode res;
        std::ofstream file(pdfSetName, std::ios::binary);

        if (!file.is_open())
        {
            std::cerr << "Failed to open file for writing: " << pdfSetName << std::endl;
            return false;
        }

        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, downloadUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file.rdbuf());
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }

        file.close();

        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        return true;
    }

  private:
    std::string baseUrl;
};

// Example usage
int main()
{
    FileDownloader downloader("http://lhapdfsets.web.cern.ch/lhapdfsets/current/");
    std::string pdfSetName = "cteq61.tar.gz"; // Specify the PDF set name here
    if (downloader.download(pdfSetName))
    {
        std::cout << "Download completed successfully." << std::endl;
    }
    else
    {
        std::cout << "Download failed." << std::endl;
    }
    return 0;
}

} // namespace PDFxTMD
