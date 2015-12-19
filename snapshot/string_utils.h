#pragma once

namespace string_utils {
    std::string toUTF8(const std::wstring& input);
    std::string toUTF8(const wchar_t* input);
}
