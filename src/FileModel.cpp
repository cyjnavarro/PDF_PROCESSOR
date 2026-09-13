#include "FileModel.h"
#include "StringConverter.h"

#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

// ===== PdfFile Implementation =====

PdfFile::PdfFile(const std::string& path) : path_(path) {
    filename_ = fs::path(path).filename().string();
}

bool PdfFile::validate(std::string& errorMessage) const {
    if (path_.empty()) {
        errorMessage = "File path is empty.";
        return false;
    }

    if (!fs::exists(path_)) {
        errorMessage = "File not found: " + path_;
        return false;
    }

    if (!fs::is_regular_file(path_)) {
        errorMessage = "Path is not a file: " + path_;
        return false;
    }

    if (!hasPdfExtension(path_)) {
        errorMessage = "File is not a PDF: " + path_;
        return false;
    }

    if (!isPdfSignature(path_)) {
        errorMessage = "Invalid PDF file (missing signature): " + path_;
        return false;
    }

    return true;
}

bool PdfFile::hasPdfExtension(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext == ".pdf";
}

bool PdfFile::isPdfSignature(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    char header[5] = {0};
    file.read(header, 5);
    return std::string(header, 5) == "%PDF-";
}

// ===== FileCollection Implementation =====

void FileCollection::add(const PdfFile& file) {
    // Avoid duplicates
    for (const auto& existing : files_) {
        if (existing.getPath() == file.getPath()) {
            return;
        }
    }
    files_.push_back(file);
}

void FileCollection::remove(size_t index) {
    if (index < files_.size()) {
        files_.erase(files_.begin() + index);
    }
}

void FileCollection::clear() {
    files_.clear();
}

void FileCollection::moveUp(size_t index) {
    if (index > 0 && index < files_.size()) {
        std::swap(files_[index], files_[index - 1]);
    }
}

void FileCollection::moveDown(size_t index) {
    if (index < files_.size() - 1) {
        std::swap(files_[index], files_[index + 1]);
    }
}

std::vector<std::string> FileCollection::toStringPaths() const {
    std::vector<std::string> paths;
    paths.reserve(files_.size());
    for (const auto& file : files_) {
        paths.push_back(file.getPath());
    }
    return paths;
}
