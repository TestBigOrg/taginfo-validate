#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

readonly RAPIDJSON=$(realpath ${1:-"$(pwd)/third_party/rapidjson-1.0.2"})

mkdir -p build && cd $_

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INCLUDE_PATH=${RAPIDJSON}/include

cmake --build .
