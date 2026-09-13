#include "FileDialog.h"

#include <commdlg.h>
#include <algorithm>
#include <cctype>

std::vector<std::wstring> FileDialog::showOpenPdfDialog(HWND parentWindow) {
    std::vector<std::wstring> result;
    
    wchar_t fileBuffer[32768] = {0};
    OPENFILENAMEW openFile = {};
    openFile.lStructSize = sizeof(OPENFILENAMEW);
    openFile.hwndOwner = parentWindow;
    openFile.lpstrFile = fileBuffer;
    openFile.nMaxFile = sizeof(fileBuffer) / sizeof(fileBuffer[0]);
    openFile.lpstrFilter = L"PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0\0";
    openFile.nFilterIndex = 1;
    openFile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST;
    openFile.lpstrTitle = L"Select PDFs to merge";

    if (!GetOpenFileNameW(&openFile)) {
        return result;
    }

    std::wstring selection(fileBuffer);

    // Check if multiple files were selected
    if (selection.find(L'\0') == std::wstring::npos) {
        // Single file selected
        if (isPdfPath(selection)) {
            result.push_back(selection);
        }
    } else {
        // Multiple files selected
        const std::wstring directory = selection.substr(0, selection.find(L'\0'));
        const std::wstring rawSelection = selection.substr(directory.size() + 1);
        result = expandMultiSelectPaths(directory, rawSelection);
    }

    return result;
}

std::wstring FileDialog::showSavePdfDialog(HWND parentWindow) {
    wchar_t outputPath[32768] = {0};
    OPENFILENAMEW saveDialog = {};
    saveDialog.lStructSize = sizeof(OPENFILENAMEW);
    saveDialog.hwndOwner = parentWindow;
    saveDialog.lpstrFile = outputPath;
    saveDialog.nMaxFile = sizeof(outputPath) / sizeof(outputPath[0]);
    saveDialog.lpstrFilter = L"PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0\0";
    saveDialog.nFilterIndex = 1;
    saveDialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    saveDialog.lpstrTitle = L"Save merged PDF as";

    if (!GetSaveFileNameW(&saveDialog)) {
        return L"";
    }

    return std::wstring(outputPath);
}

std::vector<std::wstring> FileDialog::expandMultiSelectPaths(
    const std::wstring& directory,
    const std::wstring& rawSelection) {
    
    std::vector<std::wstring> results;
    std::wstring path = directory;
    
    if (!path.empty() && path.back() != L'\\' && path.back() != L'/') {
        path += L"\\";
    }

    size_t index = 0;
    while (index < rawSelection.size()) {
        const size_t end = rawSelection.find(L'\0', index);
        if (end == std::wstring::npos) {
            break;
        }

        const std::wstring item = rawSelection.substr(index, end - index);
        if (!item.empty()) {
            std::wstring fullPath = path + item;
            if (isPdfPath(fullPath)) {
                results.push_back(fullPath);
            }
        }
        index = end + 1;
    }

    if (results.empty() && !rawSelection.empty() && isPdfPath(directory + L"\\" + rawSelection)) {
        results.push_back(directory + L"\\" + rawSelection);
    }

    return results;
}

bool FileDialog::isPdfPath(const std::wstring& path) {
    if (path.size() < 4) {
        return false;
    }

    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](wchar_t ch) { return static_cast<wchar_t>(std::tolower(static_cast<unsigned char>(ch))); });

    return lower.substr(lower.size() - 4) == L".pdf";
}
