#pragma once

#include <string>
#include <vector>
#include <functional>

using MergeProgressCallback = std::function<void(size_t current, size_t total, const std::string& fileName)>;
using SplitProgressCallback = std::function<void(size_t current, size_t total, const std::string& fileName)>;

class PdfProcessor {
public:
    PdfProcessor() = default;
    
    bool validateFiles(const std::vector<std::string>& filePaths, std::string& errorMessage) const;
    
    bool mergeFiles(const std::vector<std::string>& filePaths,
                    const std::string& outputPath,
                    const MergeProgressCallback& progressCallback = MergeProgressCallback());

    size_t getPageCount(const std::string& filePath) const;

    bool splitFile(const std::string& filePath,
                   const std::vector<std::pair<size_t, size_t>>& pageRanges,
                   const std::string& outputDirectory,
                   const std::string& baseName,
                   const SplitProgressCallback& progressCallback = SplitProgressCallback());

private:
    static bool hasPdfExtension(const std::string& path);
    static bool isPdfSignature(const std::string& path);
    static std::string readFileBytes(const std::string& path);
    static std::string trimTrailingEof(const std::string& content);
};
