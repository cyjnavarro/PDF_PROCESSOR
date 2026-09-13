#pragma once

#include <string>
#include <vector>
#include <functional>

class PdfFile {
public:
    explicit PdfFile(const std::string& path);
    
    const std::string& getPath() const { return path_; }
    const std::string& getFilename() const { return filename_; }
    bool validate(std::string& errorMessage) const;

private:
    std::string path_;
    std::string filename_;
    
    static bool hasPdfExtension(const std::string& path);
    static bool isPdfSignature(const std::string& path);
};

class FileCollection {
public:
    void add(const PdfFile& file);
    void remove(size_t index);
    void clear();
    void moveUp(size_t index);
    void moveDown(size_t index);
    
    const std::vector<PdfFile>& getFiles() const { return files_; }
    size_t getCount() const { return files_.size(); }
    bool isEmpty() const { return files_.empty(); }
    
    std::vector<std::string> toStringPaths() const;

private:
    std::vector<PdfFile> files_;
};
