#!/bin/bash

RENDER=${1:-vulkan}
WINDOW=${2:-glfw}

rm -rf build
cmake -B build -DENGINE_RENDERING=$RENDER -DENGINE_WINDOWING=$WINDOW
cmake --build build -j$(sysctl -n hw.ncpu) -v