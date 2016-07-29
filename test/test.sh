#!/usr/bin/env bash

set -eu

# compile
if [ ${1:-""} = "build" ]; then
    bash $(dirname $0)/../build.sh
fi

function cleanup() {
    rm -rf $tmpfile
}

trap cleanup EXIT

validate="$(dirname $0)/../build/taginfo-validate"
objectType="nodes"

tmpfile=$(mktemp -p /tmp tmp.${objectType}.XXXXXXX)

$validate \
    --osm $(dirname $0)/nodes.xml \
    --taginfo $(dirname $0)/nodes-taginfo.json > $tmpfile

if [ $? -eq 0 ]; then
    echo "Wrote validation output to $tmpfile"
fi

diff $tmpfile $(dirname $0)/fixtures/nodes.expected.txt -q

if [ $? -eq 0 ]; then
    echo "Returned expected values"
fi

