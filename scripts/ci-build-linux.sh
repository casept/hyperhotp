#!/usr/bin/env bash
set -eo pipefail

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON ..
make -j"$(nproc)"
