#include "PdfProcessor.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

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

    try {
        QPDF mergedPdf;
        mergedPdf.emptyPDF();
        QPDFPageDocumentHelper mergedPages(mergedPdf);

        const size_t totalFiles = filePaths.size();
        for (size_t i = 0; i < totalFiles; ++i) {
            const auto& filePath = filePaths[i];
            QPDF sourcePdf;
            sourcePdf.processFile(filePath.c_str());
            QPDFPageDocumentHelper sourcePages(sourcePdf);

            for (const auto& page : sourcePages.getAllPages()) {
                mergedPages.addPage(page, false);
            }

            if (progressCallback) {
                progressCallback(i + 1, totalFiles, fs::path(filePath).filename().string());
            }
        }

        QPDFWriter writer(mergedPdf, outputPath.c_str());
        writer.write();
        return true;
    } catch (const std::exception& error) {
        std::cerr << "Unable to merge PDFs: " << error.what() << '\n';
        return false;
    }
}

size_t PdfProcessor::getPageCount(const std::string& filePath) const {
    try {
        QPDF pdf;
        pdf.processFile(filePath.c_str());
        return QPDFPageDocumentHelper(pdf).getAllPages().size();
    } catch (const std::exception& error) {
        std::cerr << "Unable to read PDF page count: " << error.what() << '\n';
        return 0;
    }
}

bool PdfProcessor::splitFile(const std::string& filePath,
                             const std::vector<std::pair<size_t, size_t>>& pageRanges,
                             const std::string& outputDirectory,
                             const std::string& baseName,
                             const SplitProgressCallback& progressCallback) {
    if (pageRanges.empty() || !fs::exists(outputDirectory)) {
        return false;
    }

    try {
        QPDF sourcePdf;
        sourcePdf.processFile(filePath.c_str());
        const auto pages = QPDFPageDocumentHelper(sourcePdf).getAllPages();

        for (size_t rangeIndex = 0; rangeIndex < pageRanges.size(); ++rangeIndex) {
            const auto [firstPage, lastPage] = pageRanges[rangeIndex];
            if (firstPage >= pages.size() || lastPage >= pages.size() || firstPage > lastPage) {
                return false;
            }

            QPDF partPdf;
            partPdf.emptyPDF();
            QPDFPageDocumentHelper partPages(partPdf);
            for (size_t pageIndex = firstPage; pageIndex <= lastPage; ++pageIndex) {
                partPages.addPage(pages[pageIndex], false);
            }

            std::string partStem = baseName;
            if (pageRanges.size() > 1) {
                partStem += "_" + std::to_string(firstPage + 1) + "-" +
                    std::to_string(lastPage + 1);
            }

            fs::path outputPath = fs::path(outputDirectory) / (partStem + ".pdf");
            size_t collisionIndex = 2;
            while (fs::exists(outputPath)) {
                outputPath = fs::path(outputDirectory) /
                    (partStem + "_" + std::to_string(collisionIndex++) + ".pdf");
            }

            const std::string partName = outputPath.filename().string();
            QPDFWriter writer(partPdf, outputPath.string().c_str());
            writer.write();

            if (progressCallback) {
                progressCallback(rangeIndex + 1, pageRanges.size(), partName);
            }
        }

        return true;
    } catch (const std::exception& error) {
        std::cerr << "Unable to split PDF: " << error.what() << '\n';
        return false;
    }
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
