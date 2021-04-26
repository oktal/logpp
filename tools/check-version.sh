#!/bin/bash

set -eu

SEMVER_REGEX='^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$'

TARGET_VERSION=$1
VERSION=`make version | grep -E ${SEMVER_REGEX}`

if [ "$TARGET_VERSION" != "$VERSION" ]
then
    echo FAILED
    exit 1
fi

echo OK
exit
