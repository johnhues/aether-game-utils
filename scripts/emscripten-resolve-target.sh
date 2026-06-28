#!/usr/bin/env bash
#-------------------------------------------------------------------------------
# emscripten-resolve-target.sh — extract CMake target name from executable path
#
# Usage: emscripten-resolve-target.sh <path|target>
#
# Expected path format: .../TARGET/CONFIG/EXE
#   Example: /path/build_em/examples/01_example/Debug/index.js
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

# Path format: .../TARGET/CONFIG/EXE
# Remove filename to get CONFIG dir
config_dir=$(dirname "$INPUT")

# Remove CONFIG dir to get TARGET
target=$(basename "$(dirname "$config_dir")")

echo "$target"
