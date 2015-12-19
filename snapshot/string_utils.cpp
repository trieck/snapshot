#include "stdafx.h"
#include "string_utils.h"

namespace string_utils {
    using Codecvt = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<Codecvt, wchar_t> converter;

    std::string toUTF8(const std::wstring& input)
    {
        return converter.to_bytes(input);
    }

    std::string toUTF8(const wchar_t* input)
    {
        return converter.to_bytes(input);
    }
}