#!/bin/bash

class_dir="./Classes/"
build_dir="./build/"

mkdir -p "$build_dir"

files=(*.cpp)
class_files=("$class_dir"*.cpp)

all_files=("${files[@]}" "${class_files[@]}")

for file in "${all_files[@]}"; do
    echo "Compiling: $file"
    basefile=$(basename "${file%.cpp}")
    g++ -c "$file" -o "${build_dir}${basefile}.o"
done

obj_files=("${build_dir}"*.o)

echo "Object files to be linked:"
for file in "${obj_files[@]}"; do
    echo "$file"
done

echo "Linking..."
g++ "${obj_files[@]}" -o "${build_dir}output"
echo "Build complete."
