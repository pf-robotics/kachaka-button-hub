#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

SOURCE_DIRS=(
    ./button_hub
    ./proto
)

set -eux

for dir in "${SOURCE_DIRS[@]}"; do
    find "${dir}" -type f -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" -o -name "*.ino" -o -name "*.proto" \
        | grep -v '/src/nghttp2/' \
        | grep -v '/src/sh2lib/' \
        | xargs clang-format --verbose -i
done
