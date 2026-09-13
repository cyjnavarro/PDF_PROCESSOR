#pragma once

#include <string>
#include <vector>
#include <functional>

using MergeProgressCallback = std::function<void(size_t current, size_t total, const std::string& fileName)>;

class PdfProcessor {
public:
    PdfProcessor() = default;
    
    bool validateFiles(const std::vector<std::string>& filePaths, std::string& errorMessage) const;
    
    bool mergeFiles(const std::vector<std::string>& filePaths,
                    const std::string& outputPath,
                    const MergeProgressCallback& progressCallback = MergeProgressCallback());

private:
    static bool hasPdfExtension(const std::string& path);
    static bool isPdfSignature(const std::string& path);
    static std::string readFileBytes(const std::string& path);
    static std::string trimTrailingEof(const std::string& content);
};
