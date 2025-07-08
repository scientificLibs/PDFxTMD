#pragma once

#include <fstream>
#include <map>
#include <optional>
#include <sstream> // Required for stringstream, though we change its usage
#include <string>
#include <variant>
#include <vector>

#include "PDFxTMDLib/Common/PartonUtils.h" // Assuming ErrorType is defined here

#include "ryml.hpp"
#include "ryml_std.hpp"
#include <iostream>

#include <vector>
#include <type_traits>

// General case: T is not a vector
template <typename T>
struct is_vector : std::false_type {};

// Specialization: T is std::vector<U> for some U
template <typename U>
struct is_vector<std::vector<U>> : std::true_type {};

namespace PDFxTMD
{
class ConfigWrapper
{
  public:
    enum class Format
    {
        YAML
    };

    // Constructor initializes an empty YAML map
    ConfigWrapper()
    {
        initializeEmptyYAML();
    }

    bool loadFromFile(const std::filesystem::path &filepath, Format format);

    bool loadFromString(const std::string &data_string, Format format);

template <typename T>
std::pair<std::optional<T>, ErrorType> get(const std::string &key) const {
    if (data.format == Format::YAML) {
        ryml::ConstNodeRef root = data.tree.rootref();
        if (!root.is_map()) {
            return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
        }
        ryml::csubstr ckey(key.data(), key.size());
        if (!root.has_child(ckey)) {
            return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
        }

        ryml::ConstNodeRef node = root[ckey];
        try {
            if constexpr (is_vector<T>::value) {
                // T is a std::vector<U>
                if (node.is_seq()) {
                    using U = typename T::value_type; // U is the element type (e.g., int, std::string)
                    T values;
                    for (const auto& child : node) {
                        U val;
                        child >> val; // Extract each sequence element as U
                        values.push_back(val);
                    }
                    return {values, ErrorType::None};
                } else {
                    return {std::nullopt, ErrorType::CONFIG_ConversionFailed};
                }
            } else {
                // T is not a vector (e.g., int, std::string)
                T value;
                node >> value; // Direct streaming for non-vector types
                return {value, ErrorType::None};
            }
        } catch (const std::exception &) {
            return {std::nullopt, ErrorType::CONFIG_ConversionFailed};
        }
    }
    return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
}
    template <typename T> bool set(const std::string &key, const T &value)
    {
        if (data.format == Format::YAML)
        {
            if (!data.tree.rootref().is_map())
            {
                return false;
            }
            try
            {
                ryml::csubstr ckey(key.data(), key.size());

                // BUG FIX 2: Stream directly to the node.
                // This avoids creating a temporary string and the resulting
                // dangling pointer. ryml will manage the memory.
                data.tree[ckey] << value;
            }
            catch (const std::exception &e)
            {
                // It's generally better to let exceptions propagate or handle them more gracefully
                // than re-throwing as a different type, but we'll keep the original logic.
                throw std::runtime_error(e.what());
            }
            return true;
        }
        return false;
    }

    bool saveToFile(const std::string &filename) const;

    void initializeEmptyYAML()
    {
        data.tree.clear();
        data.tree.rootref() |= ryml::MAP; // Set the root to be a map
        data.format = Format::YAML;
    }

  private:
    struct Data
    {
        Format format;
        ryml::Tree tree;
    } data;
};
} // namespace PDFxTMD