#pragma once
#include <string>
#include <optional>
#include <PDFxTMDLib/Common/PartonUtils.h>

namespace PDFxTMD
{
   struct YamlErrorInfo
   {
        double ErrorConfLevel;
        std::string ErrorType;
        size_t errorSize;
        size_t size;
   };
   std::pair<std::optional<YamlErrorInfo>, ErrorType> YamlErrorInfoReader(
    const std::string &yamlInfoPath);
}