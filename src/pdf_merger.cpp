#include "pdf_merger.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool has_pdf_extension(const std::string& path) {
    const std::string ext = fs::path(path).extension().string();
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lower == ".pdf";
}

std::string read_file_bytes(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string trim_trailing_eof(const std::string& content) {
    const std::string eof_marker = "%%EOF";
    const std::size_t eof_pos = content.rfind(eof_marker);
    if (eof_pos == std::string::npos) {
        return content;
    }

    const std::size_t end_of_pdf = eof_pos + eof_marker.size();
    if (end_of_pdf >= content.size()) {
        return content.substr(0, eof_pos);
    }

    const std::string tail = content.substr(end_of_pdf);
    if (tail.find_first_not_of(" \r\n\t") == std::string::npos) {
        return content.substr(0, eof_pos);
    }

    return content;
}

} // namespace

bool pdf_merger::validate_pdf_files(const std::vector<std::string>& pdf_paths, std::string& error_message) {
    if (pdf_paths.empty()) {
        error_message = "No PDF files selected.";
        return false;
    }

    for (const auto& pdf_path : pdf_paths) {
        if (pdf_path.empty()) {
            error_message = "One of the selected file paths is empty.";
            return false;
        }

        if (!fs::exists(pdf_path)) {
            error_message = "File not found: " + pdf_path;
            return false;
        }

        if (!fs::is_regular_file(pdf_path)) {
            error_message = "Path is not a file: " + pdf_path;
            return false;
        }

        if (!has_pdf_extension(pdf_path)) {
            error_message = "File is not a PDF: " + pdf_path;
            return false;
        }

        std::ifstream input(pdf_path, std::ios::binary);
        if (!input) {
            error_message = "Unable to open PDF for validation: " + pdf_path;
            return false;
        }

        char header[5] = {0};
        input.read(header, 5);
        if (std::string(header, 5) != "%PDF-") {
            error_message = "Invalid or corrupted PDF file: " + pdf_path;
            return false;
        }
    }

    error_message.clear();
    return true;
}

bool pdf_merger::merge_pdfs(const std::vector<std::string>& pdf_paths,
                           const std::string& output_path,
                           const ProgressCallback& progress_callback) {
    std::string error_message;
    if (!validate_pdf_files(pdf_paths, error_message)) {
        std::cerr << error_message << '\n';
        return false;
    }

    const fs::path output_dir = fs::path(output_path).parent_path();
    if (!output_dir.empty() && !fs::exists(output_dir)) {
        std::cerr << "Output directory does not exist: " << output_dir << '\n';
        return false;
    }

    std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
    if (!output) {
        std::cerr << "Unable to create output file: " << output_path << '\n';
        return false;
    }

    const std::size_t total_files = pdf_paths.size();
    for (std::size_t index = 0; index < total_files; ++index) {
        const std::string& file_path = pdf_paths[index];
        if (progress_callback) {
            progress_callback(index + 1, total_files, fs::path(file_path).filename().string());
        }

        std::string content = read_file_bytes(file_path);
        if (content.empty()) {
            std::cerr << "Unable to read PDF file: " << file_path << '\n';
            return false;
        }

        if (index > 0) {
            content = trim_trailing_eof(content);
        }

        output.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!output) {
            std::cerr << "Error while writing merged PDF: " << output_path << '\n';
            return false;
        }
    }

    output.close();
    if (!output) {
        std::cerr << "Error while closing output PDF: " << output_path << '\n';
        return false;
    }

    return true;
}
