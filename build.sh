#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

readonly RAPIDJSON=$(realpath ${1:-"$(pwd)/third_party/rapidjson-f51d7c9d4b1a9313bde4a5ccf94c4af7089661a8"})

mkdir -p build && cd $_

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INCLUDE_PATH=${RAPIDJSON}/include

cmake --build .
