#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

RAPIDJSON_URL='https://github.com/miloyip/rapidjson/archive/v1.0.2.tar.gz'
RAPIDJSON_FILE='rapidjson-v1.0.2.tar.gz'

LIBOSMIUM_URL='https://github.com/osmcode/libosmium/archive/v2.7.2.tar.gz'
LIBOSMIUM_FILE='libosmium-v2.7.2.tar.gz'

wget --quiet ${RAPIDJSON_URL} -O ${RAPIDJSON_FILE}
cmake -E tar xf ${RAPIDJSON_FILE}

wget --quiet ${LIBOSMIUM_URL} -O ${LIBOSMIUM_FILE}
cmake -E tar xf ${LIBOSMIUM_FILE}
