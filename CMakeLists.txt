cmake_minimum_required(VERSION 3.16)
project(PDF_PROCESSOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 6 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

add_executable(PDF_PROCESSOR WIN32
    src/main.cpp
    src/MainWindow.cpp
    include/MainWindow.h
    src/StringConverter.cpp
    src/FileModel.cpp
    src/PdfProcessor.cpp
)

target_include_directories(PDF_PROCESSOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(PDF_PROCESSOR PRIVATE cxx_std_17)

if (MSVC)
    target_compile_options(PDF_PROCESSOR PRIVATE /W4)
else()
    target_compile_options(PDF_PROCESSOR PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(PDF_PROCESSOR PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)

set_target_properties(PDF_PROCESSOR PROPERTIES OUTPUT_NAME "PDF_PROCESSOR")

