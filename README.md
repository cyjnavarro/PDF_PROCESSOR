# PDF_PROCESSOR (C++ / MinGW)

This project is a Qt 6 desktop PDF workspace built with C++, MinGW, and CMake.

## Build with MinGW

From the project root:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

The merge engine uses QPDF. With MSYS2 UCRT64, install it with:

```powershell
pacman -S mingw-w64-ucrt-x86_64-qpdf
```

Then run:

```powershell
./build/PDF_PROCESSOR.exe
```

## Notes

- The app validates that each input file exists and looks like a PDF.
- The landing page provides PDF merge, image conversion, and PDF splitting.
- The splitter opens the PDF in an in-app viewer with page navigation, then accepts ranges such as `1-3, 4-6` and creates one output PDF for each range.
- The merge screen uses QPDF to copy pages into a structurally valid output PDF.
- Each selected merge file has an `X` button for removing only that file.

