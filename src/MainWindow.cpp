#include "MainWindow.h"

#include "pdf_merger.h"

#include <algorithm>
#include <commctrl.h>
#include <commdlg.h>
#include <codecvt>
#include <locale>
#include <shellapi.h>
#include <vector>

namespace {

bool isPdfPath(const std::wstring& path) {
    if (path.size() < 4) {
        return false;
    }

    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });

    return lower.substr(lower.size() - 4) == L".pdf";
}

std::vector<std::wstring> expandMultiSelectPaths(const std::wstring& directory, const std::wstring& rawSelection) {
    std::vector<std::wstring> results;
    std::wstring path = directory;
    if (!path.empty() && path.back() != L'\\' && path.back() != L'/') {
        path += L"\\";
    }

    std::size_t index = 0;
    while (index < rawSelection.size()) {
        const std::size_t end = rawSelection.find(L'\0', index);
        if (end == std::wstring::npos) {
            break;
        }

        const std::wstring item = rawSelection.substr(index, end - index);
        if (!item.empty()) {
            results.push_back(path + item);
        }
        index = end + 1;
    }

    if (results.empty() && !rawSelection.empty()) {
        results.push_back(rawSelection);
    }

    return results;
}

std::wstring toWindowsWide(const std::string& value) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(value);
}

std::string toUtf8Internal(const std::wstring& value) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(value);
}

} // namespace

MainWindow::MainWindow() = default;

std::wstring MainWindow::toWide(const std::string& value) {
    return toWindowsWide(value);
}

std::string MainWindow::toUtf8(const std::wstring& value) {
    return toUtf8Internal(value);
}

int MainWindow::run(HINSTANCE instance, int nCmdShow) {
    instance_ = instance;

    const wchar_t CLASS_NAME[] = L"PDFMergerWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassW(&wc);

    hwnd_ = CreateWindowExW(
        WS_EX_ACCEPTFILES,
        CLASS_NAME,
        L"PDF Merger",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        720,
        560,
        nullptr,
        nullptr,
        instance,
        this);

    if (!hwnd_) {
        return 1;
    }

    DragAcceptFiles(hwnd_, TRUE);
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* window = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = static_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!window && message != WM_DESTROY) {
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    switch (message) {
    case WM_CREATE:
        window->createControls();
        return 0;
    case WM_DESTROY:
        DragAcceptFiles(hwnd, FALSE);
        PostQuitMessage(0);
        return 0;
    case WM_DROPFILES:
        window->handleDroppedFiles(reinterpret_cast<HDROP>(wParam));
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1001) {
            window->addPdfFiles();
        } else if (LOWORD(wParam) == 1002) {
            window->clearFiles();
        } else if (LOWORD(wParam) == 1003) {
            window->mergeFiles();
        } else if (LOWORD(wParam) == 1004) {
            window->moveSelectedUp();
        } else if (LOWORD(wParam) == 1005) {
            window->moveSelectedDown();
        }
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

void MainWindow::applySelectionState() {
    if (!fileList_) {
        return;
    }

    const int selected = static_cast<int>(SendMessageW(fileList_, LB_GETCURSEL, 0, 0));
    const bool hasSelection = selected != LB_ERR;
    EnableWindow(upButton_, hasSelection && selected > 0);
    EnableWindow(downButton_, hasSelection && selected >= 0 && selected < static_cast<int>(pdfFiles_.size()) - 1);
}

void MainWindow::createControls() {
    SetWindowTextW(hwnd_, L"PDF Merger");

    CreateWindowExW(0, L"STATIC", L"PDF Merger",
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    20, 18, 220, 30,
                    hwnd_, nullptr, instance_, nullptr);

    fileList_ = CreateWindowExW(0, L"LISTBOX", L"",
                                WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
                                20, 60, 660, 280,
                                hwnd_, reinterpret_cast<HMENU>(2001), instance_, nullptr);

    statusLabel_ = CreateWindowExW(0, L"STATIC", L"Ready",
                                   WS_VISIBLE | WS_CHILD | SS_LEFT,
                                   20, 355, 660, 24,
                                   hwnd_, nullptr, instance_, nullptr);

    addButton_ = CreateWindowExW(0, L"BUTTON", L"Add PDFs",
                                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 20, 390, 150, 34,
                                 hwnd_, reinterpret_cast<HMENU>(1001), instance_, nullptr);

    clearButton_ = CreateWindowExW(0, L"BUTTON", L"Clear",
                                   WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                   185, 390, 120, 34,
                                   hwnd_, reinterpret_cast<HMENU>(1002), instance_, nullptr);

    upButton_ = CreateWindowExW(0, L"BUTTON", L"Move Up",
                                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                320, 390, 120, 34,
                                hwnd_, reinterpret_cast<HMENU>(1004), instance_, nullptr);

    downButton_ = CreateWindowExW(0, L"BUTTON", L"Move Down",
                                  WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  455, 390, 120, 34,
                                  hwnd_, reinterpret_cast<HMENU>(1005), instance_, nullptr);

    mergeButton_ = CreateWindowExW(0, L"BUTTON", L"Merge & Save As",
                                   WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                   590, 390, 90, 34,
                                   hwnd_, reinterpret_cast<HMENU>(1003), instance_, nullptr);

    updateFileList();
}

void MainWindow::updateFileList() {
    SendMessageW(fileList_, LB_RESETCONTENT, 0, 0);
    for (const auto& file : pdfFiles_) {
        SendMessageW(fileList_, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(file.c_str()));
    }

    EnableWindow(mergeButton_, pdfFiles_.size() > 0);
    EnableWindow(clearButton_, pdfFiles_.size() > 0);
    applySelectionState();
    if (pdfFiles_.empty()) {
        setStatus(L"Ready");
    }
}

void MainWindow::setStatus(const std::wstring& status) {
    SetWindowTextW(statusLabel_, status.c_str());
}

void MainWindow::handleDroppedFiles(HDROP dropHandle) {
    UINT fileCount = DragQueryFileW(dropHandle, UINT_MAX, nullptr, 0);
    bool added = false;

    for (UINT i = 0; i < fileCount; ++i) {
        const UINT bufSize = 32768;
        wchar_t path[bufSize] = {0};
        const UINT copied = DragQueryFileW(dropHandle, i, path, bufSize);
        if (copied == 0) {
            continue;
        }

        std::wstring filePath(path);
        if (!isPdfPath(filePath)) {
            continue;
        }

        if (std::find(pdfFiles_.begin(), pdfFiles_.end(), filePath) == pdfFiles_.end()) {
            pdfFiles_.push_back(filePath);
            added = true;
        }
    }

    DragFinish(dropHandle);

    if (added) {
        setStatus(L"Files added via drag-and-drop.");
    }

    updateFileList();
}

void MainWindow::moveSelectedUp() {
    const int selected = static_cast<int>(SendMessageW(fileList_, LB_GETCURSEL, 0, 0));
    if (selected <= 0 || selected >= static_cast<int>(pdfFiles_.size())) {
        return;
    }

    auto it = pdfFiles_.begin() + selected;
    std::iter_swap(it, it - 1);
    updateFileList();
    SendMessageW(fileList_, LB_SETCURSEL, selected - 1, 0);
    applySelectionState();
}

void MainWindow::moveSelectedDown() {
    const int selected = static_cast<int>(SendMessageW(fileList_, LB_GETCURSEL, 0, 0));
    if (selected < 0 || selected >= static_cast<int>(pdfFiles_.size()) - 1) {
        return;
    }

    auto it = pdfFiles_.begin() + selected;
    std::iter_swap(it, it + 1);
    updateFileList();
    SendMessageW(fileList_, LB_SETCURSEL, selected + 1, 0);
    applySelectionState();
}

void MainWindow::addPdfFiles() {
    wchar_t fileBuffer[32768] = {0};
    OPENFILENAMEW openFile = {};
    openFile.lStructSize = sizeof(OPENFILENAMEW);
    openFile.hwndOwner = hwnd_;
    openFile.lpstrFile = fileBuffer;
    openFile.nMaxFile = sizeof(fileBuffer) / sizeof(fileBuffer[0]);
    openFile.lpstrFilter = L"PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0\0";
    openFile.nFilterIndex = 1;
    openFile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST;
    openFile.lpstrTitle = L"Select PDFs to merge";

    if (!GetOpenFileNameW(&openFile)) {
        return;
    }

    std::wstring selection(fileBuffer);
    std::vector<std::wstring> selected;

    if (selection.find(L'\0') == std::wstring::npos) {
        selected.push_back(selection);
    } else {
        const std::wstring directory = selection.substr(0, selection.find(L'\0'));
        const std::wstring rawSelection = selection.substr(directory.size() + 1);
        selected = expandMultiSelectPaths(directory, rawSelection);
    }

    bool added = false;
    for (const auto& path : selected) {
        if (isPdfPath(path) && std::find(pdfFiles_.begin(), pdfFiles_.end(), path) == pdfFiles_.end()) {
            pdfFiles_.push_back(path);
            added = true;
        }
    }

    if (added) {
        setStatus(L"Files added to merge list.");
    }

    updateFileList();
}

void MainWindow::clearFiles() {
    pdfFiles_.clear();
    updateFileList();
    setStatus(L"Ready");
}

void MainWindow::mergeFiles() {
    if (pdfFiles_.empty()) {
        MessageBoxW(hwnd_, L"Please select at least one PDF file first.", L"No Files", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t outputPath[32768] = {0};
    OPENFILENAMEW saveDialog = {};
    saveDialog.lStructSize = sizeof(OPENFILENAMEW);
    saveDialog.hwndOwner = hwnd_;
    saveDialog.lpstrFile = outputPath;
    saveDialog.nMaxFile = sizeof(outputPath) / sizeof(outputPath[0]);
    saveDialog.lpstrFilter = L"PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0\0";
    saveDialog.nFilterIndex = 1;
    saveDialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    saveDialog.lpstrTitle = L"Save merged PDF as";

    if (!GetSaveFileNameW(&saveDialog)) {
        return;
    }

    std::vector<std::string> narrowPaths;
    narrowPaths.reserve(pdfFiles_.size());
    for (const auto& path : pdfFiles_) {
        narrowPaths.push_back(toUtf8(path));
    }

    const std::string output = toUtf8(std::wstring(outputPath));

    setStatus(L"Merging files...");

    bool ok = pdf_merger::merge_pdfs(narrowPaths, output,
        [this](std::size_t current, std::size_t total, const std::string& fileName) {
            std::wstring text = L"Processing " + std::to_wstring(current) + L"/" + std::to_wstring(total) + L": " + toWide(fileName);
            setStatus(text);
        });

    if (!ok) {
        MessageBoxW(hwnd_, L"The merge operation failed. Check the selected files and try again.", L"Merge Failed", MB_OK | MB_ICONERROR);
        setStatus(L"Merge failed.");
        return;
    }

    MessageBoxW(hwnd_, L"PDF files merged successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
    setStatus(L"Merge complete.");
    pdfFiles_.clear();
    updateFileList();
}
