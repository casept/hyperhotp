#!/usr/bin/env bash
set -eo pipefail

mkdir build && cd build
cmake ..
make -j"$(nproc)"
