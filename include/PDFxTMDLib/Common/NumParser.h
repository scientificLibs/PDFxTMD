#pragma once
#include <charconv>
#include <cstring>
#include <string_view>

namespace PDFxTMD
{

class NumParser
{
  public:
    explicit NumParser(std::string_view input) noexcept
        : _current(input.data()), _end(input.data() + input.size())
    {
    }

    void reset(std::string_view input) noexcept
    {
        _current = input.data();
        _end = input.data() + input.size();
    }

    template <typename T> bool operator>>(T &value) noexcept
    {
        skipSpaces();
        auto result = parseNumber<T>(value);
        return result;
    }

    [[nodiscard]] bool hasMore() const noexcept
    {
        return _current < _end;
    }

  private:
    const char *_current;
    const char *_end;

    void skipSpaces() noexcept
    {
        while (_current < _end && *_current == ' ')
        {
            ++_current;
        }
    }

    template <typename T> bool parseNumber(T &value) noexcept
    {
        auto [ptr, ec] = std::from_chars(_current, _end, value);
        if (ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range)
        {
            return false;
        }
        _current = ptr;
        return true;
    }
};

} // namespace PDFxTMD
