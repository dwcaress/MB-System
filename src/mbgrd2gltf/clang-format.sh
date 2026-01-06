#!/usr/bin/env bash
# Format only the top-level C++ source files in this directory.
# Does NOT enter draco/ or tinygltf/ subdirectories.

set -euo pipefail

cd "$(dirname "$0")"

# Formatting only top-level .cpp and .h files
clang-format -i --verbose  -style=file ./*.cpp ./*.h

echo "Formatted top-level mbgrd2gltf .cpp and .h files."
