#!/bin/bash
set -xeuo pipefail
cd "$(cd "$(dirname "$0")" && pwd)"

cmake -G Ninja -S . -B build_clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
cmake --build build_clang
build_clang/test/test

cmake -G Ninja -S . -B build_gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
cmake --build build_gcc
build_gcc/test/test

cmake -G Ninja -S . -B build_mingw -DCMAKE_TOOLCHAIN_FILE=.github/toolchains/mingw-w64.cmake
cmake --build build_mingw
wine build_mingw/test/test.exe

# emcmake cmake -G Ninja -B build_emscripten .
# cmake --build build_emscripten
# node build_emscripten/test/test.js

act -W .github/workflows/ubuntu_clang.yml --container-architecture linux/amd64
act -W .github/workflows/ubuntu_gcc.yml --container-architecture linux/amd64
act -W .github/workflows/ubuntu_mingw.yml --container-architecture linux/amd64
