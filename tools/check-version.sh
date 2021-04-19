#!/bin/bash

set -eu

SEMVER_REGEX='^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$'

TAG_PREFIX=${2:-v}
TAG=$1

TAG_VERSION=${TAG/#$TAG_PREFIX}

VERSION=`make version | grep -E ${SEMVER_REGEX}`

if [ "$TAG_VERSION" != "$VERSION" ]
then
    echo FAILED
    exit 1
fi

echo OK
exit
