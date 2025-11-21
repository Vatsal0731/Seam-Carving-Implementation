# Seam-Carving-Implementation

This project implements a content-aware image resizing algorithm based on Seam Carving, enabling dynamic adjustment of image dimensions while preserving key visual elements.

## Compile
g++ seam.cpp -o seam `pkg-config --cflags --libs opencv4`

## Run
./seam <image_path> <new_width> <new_height>
