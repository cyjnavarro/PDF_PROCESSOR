#pragma once

#include <string>
#include <vector>
#include <windows.h>

class MainWindow {
public:
    MainWindow();
    int run(HINSTANCE instance, int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void createControls();
    void addPdfFiles();
    void clearFiles();
    void mergeFiles();
    void moveSelectedUp();
    void moveSelectedDown();
    void handleDroppedFiles(HDROP dropHandle);
    void updateFileList();
    void setStatus(const std::wstring& status);
    void applySelectionState();

    static std::wstring toWide(const std::string& value);
    static std::string toUtf8(const std::wstring& value);

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    HWND statusLabel_ = nullptr;
    HWND fileList_ = nullptr;
    HWND addButton_ = nullptr;
    HWND clearButton_ = nullptr;
    HWND mergeButton_ = nullptr;
    HWND upButton_ = nullptr;
    HWND downButton_ = nullptr;
    std::vector<std::wstring> pdfFiles_;
};
