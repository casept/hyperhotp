#!/usr/bin/env bash
set -eo pipefail

mkdir build && cd build
cmake -DBUILD_GUI=ON ..
make -j"$(nproc)"
