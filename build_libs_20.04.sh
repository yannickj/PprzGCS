#!/bin/bash

tinymxl2_src=$GITHUB_WORKSPACE/ext/tinyxml2
tinymxl2_build=$GITHUB_WORKSPACE/build/ext/tinymxl2

pprzlink_src=$GITHUB_WORKSPACE/ext/pprzlink/lib/v2.0/C++
pprzlink_build=$GITHUB_WORKSPACE/build/ext/pprzlink

local_install=$GITHUB_WORKSPACE/build/local_install

mkdir -p $tinymxl2_build
mkdir -p $pprzlink_build
mkdir -p $local_install

cmake -S $tinymxl2_src -B $tinymxl2_build -DCMAKE_INSTALL_PREFIX="$local_install"
cmake --build $tinymxl2_build -j$(nproc)
cmake --build $tinymxl2_build --target install


cmake -S $pprzlink_src -B $pprzlink_build -DCMAKE_PREFIX_PATH="$local_install" -DCMAKE_INSTALL_PREFIX="$local_install"
cmake --build $pprzlink_build -j$(nproc)
cmake --build $pprzlink_build --target install

