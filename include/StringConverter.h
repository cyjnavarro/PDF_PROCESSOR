#pragma once

#include <string>

class StringConverter {
public:
    static std::wstring toWide(const std::string& utf8String);
    static std::string toUtf8(const std::wstring& wideString);
};
