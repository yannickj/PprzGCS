#!/bin/bash

local_install=$GITHUB_WORKSPACE/build/local_install

cmake -S . -B build -DCMAKE_PREFIX_PATH="$local_install"
cmake --build build -j$(nproc)
echo "Build directory is $(pwd)/build"
ls -l $(pwd)/build

