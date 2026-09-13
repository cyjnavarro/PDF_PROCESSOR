#pragma once

#include <string>
#include <vector>
#include <windows.h>

class FileDialog {
public:
    static std::vector<std::wstring> showOpenPdfDialog(HWND parentWindow);
    static std::wstring showSavePdfDialog(HWND parentWindow);

private:
    static std::vector<std::wstring> expandMultiSelectPaths(
        const std::wstring& directory, 
        const std::wstring& rawSelection);
    
    static bool isPdfPath(const std::wstring& path);
};
