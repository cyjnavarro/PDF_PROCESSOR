#pragma once

#include <functional>
#include <string>
#include <vector>

using ProgressCallback = std::function<void(std::size_t current, std::size_t total, const std::string& file_name)>;

namespace pdf_merger {

bool validate_pdf_files(const std::vector<std::string>& pdf_paths, std::string& error_message);
bool merge_pdfs(const std::vector<std::string>& pdf_paths,
                const std::string& output_path,
                const ProgressCallback& progress_callback = ProgressCallback());

} // namespace pdf_merger
