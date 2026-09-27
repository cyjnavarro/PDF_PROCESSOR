cmake_minimum_required(VERSION 3.16)
project(PDF_PROCESSOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 6 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport Pdf PdfWidgets REQUIRED)

find_path(QPDF_INCLUDE_DIR qpdf/QPDF.hh REQUIRED)
find_library(QPDF_LIBRARY NAMES qpdf REQUIRED)
find_file(QPDF_RUNTIME_LIBRARY NAMES libqpdf30.dll PATHS C:/msys64/ucrt64/bin REQUIRED)
find_file(QT_PDF_RUNTIME_LIBRARY NAMES Qt6Pdf.dll PATHS C:/msys64/ucrt64/bin REQUIRED)
find_file(QT_PDF_WIDGETS_RUNTIME_LIBRARY NAMES Qt6PdfWidgets.dll PATHS C:/msys64/ucrt64/bin REQUIRED)
find_path(QT_IMAGEFORMATS_DIR NAMES qjpeg.dll PATHS C:/msys64/ucrt64/share/qt6/plugins/imageformats REQUIRED)

add_executable(PDF_PROCESSOR WIN32
    src/main.cpp
    src/MainWindow.cpp
    include/MainWindow.h
    app_icon.rc
    resources.qrc
    src/StringConverter.cpp
    src/FileModel.cpp
    src/PdfProcessor.cpp
)

target_include_directories(PDF_PROCESSOR PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${QPDF_INCLUDE_DIR}
)

target_compile_features(PDF_PROCESSOR PRIVATE cxx_std_17)

if (MSVC)
    target_compile_options(PDF_PROCESSOR PRIVATE /W4)
else()
    target_compile_options(PDF_PROCESSOR PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(PDF_PROCESSOR PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Pdf
    Qt6::PdfWidgets
    ${QPDF_LIBRARY}
)

add_custom_command(TARGET PDF_PROCESSOR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QPDF_RUNTIME_LIBRARY}
        $<TARGET_FILE_DIR:PDF_PROCESSOR>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_PDF_RUNTIME_LIBRARY}
        $<TARGET_FILE_DIR:PDF_PROCESSOR>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_PDF_WIDGETS_RUNTIME_LIBRARY}
        $<TARGET_FILE_DIR:PDF_PROCESSOR>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${QT_IMAGEFORMATS_DIR}
        $<TARGET_FILE_DIR:PDF_PROCESSOR>/imageformats
)

set_target_properties(PDF_PROCESSOR PROPERTIES OUTPUT_NAME "PDF_PROCESSOR")

