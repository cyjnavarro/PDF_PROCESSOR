# PDF Merger (C++ / MinGW)

This project replaces the original Python-only implementation with a C++ codebase that can be built using MinGW and CMake.

## Build with MinGW

From the project root:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Then run:

```powershell
./build/pdf_merger.exe merged.pdf input1.pdf input2.pdf
```

## Notes

- The app validates that each input file exists and looks like a PDF.
- It merges each file sequentially into a single output PDF.
- This is a lightweight C++ foundation for the original PDF Merger workflow and is ready to expand with a GUI layer or a dedicated PDF library later.
