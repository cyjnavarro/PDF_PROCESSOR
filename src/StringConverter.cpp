#include "StringConverter.h"

#include <codecvt>
#include <locale>

std::wstring StringConverter::toWide(const std::string& utf8String) {
    if (utf8String.empty()) {
        return L"";
    }
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8String);
}

std::string StringConverter::toUtf8(const std::wstring& wideString) {
    if (wideString.empty()) {
        return "";
    }
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wideString);
}
