#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# emscripten-resolve-target.sh — extract CMake target name from executable path
#
# Usage: emscripten-resolve-target.sh <path|target>
#
# Expected path format: .../CONFIG/TARGET/EXE
#   Example: /path/build_emscripten/examples/Debug/01_example/index.js
#            → extracts "01_example"
#
# If input has no '/', treats it as a target name and returns it as-is.
#-------------------------------------------------------------------------------
set -euo pipefail

INPUT="${1:?usage: emscripten-resolve-target.sh <path|target>}"

# If no '/', it's already a target name
if [[ "$INPUT" != */* ]]; then
    echo "$INPUT"
    exit 0
fi

# Path format: .../CONFIG/TARGET/EXE
target=$(basename "$(dirname "$INPUT")")

echo "$target"
