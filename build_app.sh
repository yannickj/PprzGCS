#!/bin/bash

local_install=$GITHUB_WORKSPACE/build/local_install

cmake -S . -B build -DCMAKE_PREFIX_PATH="$local_install" -DBUILD_FOR_DEB=ON -DPROJ_ON_SYSTEM=ON
cmake --build build -j$(nproc)
echo "Build directory is $(pwd)/build"
ls -l $(pwd)/build

