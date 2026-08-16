cmake_minimum_required(VERSION 3.16)
project(PDFMerger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PDFMerger WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/pdf_merger.cpp
)

target_include_directories(PDFMerger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(PDFMerger PRIVATE cxx_std_17)

if (MSVC)
    target_compile_options(PDFMerger PRIVATE /W4)
else()
    target_compile_options(PDFMerger PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(PDFMerger PRIVATE comdlg32 shell32 user32 gdi32 comctl32)

set_target_properties(PDFMerger PROPERTIES OUTPUT_NAME "PDFMerger")
