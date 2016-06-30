#!/bin/bash

CMD="$1"

if [[ -z "${CMD}" ]]; then
    export CMD="default"
fi

export LANG=C
source tools/vars.sh

mkdir build
# this need for some outdated os
mkdir m4
autoreconf -i
cd build
if [[ "${CMD}" == "default" ]]; then
    export CC=gcc
    ../configure --enable-sanitize --enable-lto CPPFLAGS="${VARS}"
elif [[ "${CMD}" == "old" ]]; then
    ../configure CPPFLAGS="${VARS}"
elif [[ "${CMD}" == "gprof" ]]; then
    export CC=gcc
    ../configure --enable-gprof CPPFLAGS="${VARS}"
fi
make -j3
export RET=$?
cd -
exit $RET
