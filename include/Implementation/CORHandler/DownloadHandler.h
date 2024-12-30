#include <cstdio>
#include <curl/curl.h>
#include <iostream>
namespace PDFxTMD
{
class DownloadHandler
{
  public:
    void handle(Context &context)
    {
        if (!context.contains("writable_path"))
        {
            std::cerr << "Writable path not found in context." << std::endl;
            context.set("error", "Writable path missing");
            return;
        }
        std::string writablePath = *context.get("writable_path");
        std::string filePath = writablePath + "/file.tar.gz";
        if (downloadFile("https://example.com/file.tar.gz", filePath))
        {
            std::cout << "File downloaded to: " << filePath << std::endl;
            context.set("file_path", filePath);
            if (nextHandler)
            {
                nextHandler->handle(context);
            }
        }
        else
        {
            std::cerr << "File download failed." << std::endl;
            context.set("error", "File download failed");
        }
    }

  private:
    static bool downloadFile(const std::string &url, const std::string &outputPath)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            std::cerr << "Failed to initialize CURL." << std::endl;
            return false;
        }

        FILE *file = fopen(outputPath.c_str(), "wb");
        if (!file)
        {
            std::cerr << "Failed to open file for writing: " << outputPath << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);

        CURLcode res = curl_easy_perform(curl);
        fclose(file);
        curl_easy_cleanup(curl);

        return res == CURLE_OK;
    }
};
} // namespace PDFxTMD