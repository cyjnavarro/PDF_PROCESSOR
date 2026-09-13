#include "PdfProcessor.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

bool PdfProcessor::validateFiles(const std::vector<std::string>& filePaths, std::string& errorMessage) const {
    if (filePaths.empty()) {
        errorMessage = "No PDF files selected.";
        return false;
    }

    for (const auto& filePath : filePaths) {
        if (filePath.empty()) {
            errorMessage = "One of the selected file paths is empty.";
            return false;
        }

        if (!fs::exists(filePath)) {
            errorMessage = "File not found: " + filePath;
            return false;
        }

        if (!fs::is_regular_file(filePath)) {
            errorMessage = "Path is not a file: " + filePath;
            return false;
        }

        if (!hasPdfExtension(filePath)) {
            errorMessage = "File is not a PDF: " + filePath;
            return false;
        }

        if (!isPdfSignature(filePath)) {
            errorMessage = "Invalid or corrupted PDF file: " + filePath;
            return false;
        }
    }

    return true;
}

bool PdfProcessor::mergeFiles(const std::vector<std::string>& filePaths,
                              const std::string& outputPath,
                              const MergeProgressCallback& progressCallback) {
    std::string errorMessage;
    if (!validateFiles(filePaths, errorMessage)) {
        std::cerr << errorMessage << '\n';
        return false;
    }

    const fs::path outputDir = fs::path(outputPath).parent_path();
    if (!outputDir.empty() && !fs::exists(outputDir)) {
        std::cerr << "Output directory does not exist: " << outputDir << '\n';
        return false;
    }

    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        std::cerr << "Unable to create output file: " << outputPath << '\n';
        return false;
    }

    const size_t totalFiles = filePaths.size();
    for (size_t i = 0; i < totalFiles; ++i) {
        const auto& filePath = filePaths[i];
        
        if (progressCallback) {
            progressCallback(i + 1, totalFiles, fs::path(filePath).filename().string());
        }

        std::string content = readFileBytes(filePath);
        if (content.empty()) {
            std::cerr << "Unable to read PDF file: " << filePath << '\n';
            return false;
        }

        // Trim EOF marker from all files except the first
        if (i > 0) {
            content = trimTrailingEof(content);
        }

        output.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!output) {
            std::cerr << "Error while writing merged PDF: " << outputPath << '\n';
            return false;
        }
    }

    output.close();
    if (!output) {
        std::cerr << "Error while closing output PDF: " << outputPath << '\n';
        return false;
    }

    return true;
}

bool PdfProcessor::hasPdfExtension(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext == ".pdf";
}

bool PdfProcessor::isPdfSignature(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    char header[5] = {0};
    file.read(header, 5);
    return std::string(header, 5) == "%PDF-";
}

std::string PdfProcessor::readFileBytes(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string PdfProcessor::trimTrailingEof(const std::string& content) {
    const std::string eofMarker = "%%EOF";
    const size_t eofPos = content.rfind(eofMarker);
    
    if (eofPos == std::string::npos) {
        return content;
    }

    const size_t endOfPdf = eofPos + eofMarker.size();
    if (endOfPdf >= content.size()) {
        return content.substr(0, eofPos);
    }

    const std::string tail = content.substr(endOfPdf);
    if (tail.find_first_not_of(" \r\n\t") == std::string::npos) {
        return content.substr(0, eofPos);
    }

    return content;
}
