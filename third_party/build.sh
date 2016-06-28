#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

RAPIDJSON_URL='https://github.com/miloyip/rapidjson/archive/f51d7c9d4b1a9313bde4a5ccf94c4af7089661a8.zip'
RAPIDJSON_FILE='rapidjson-f51d7c9d4b1a9313bde4a5ccf94c4af7089661a8.tar.gz'

LIBOSMIUM_URL='https://github.com/osmcode/libosmium/archive/v2.7.2.tar.gz'
LIBOSMIUM_FILE='libosmium-v2.7.2.tar.gz'

wget --quiet --continue --no-check-certificate "${RAPIDJSON_URL}" -O "${RAPIDJSON_FILE}" &
wget --quiet --continue --no-check-certificate "${LIBOSMIUM_URL}" -O "${LIBOSMIUM_FILE}" &
wait

sha256sum --quiet --check << EOF
f874b86e39542f81058541a0af65a2b5aa46906cbabfd2eceb5a90167d7395c5  libosmium-v2.7.2.tar.gz
47d5a72d583fa27ed50cd848e41a51a841dc9779779464caac5a11032e2eda9d  rapidjson-f51d7c9d4b1a9313bde4a5ccf94c4af7089661a8.tar.gz
EOF

cmake -E tar xf "${RAPIDJSON_FILE}"
cmake -E tar xf "${LIBOSMIUM_FILE}"
