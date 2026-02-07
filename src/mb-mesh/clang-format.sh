#!/bin/bash
# Format all C++ source files in mb-mesh directory

# Find all .cpp and .h files and format them
find . -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} \;

echo "Code formatting complete!"
