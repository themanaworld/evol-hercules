#!/usr/bin/env bash

CMD="$1"

if [[ -z "${CMD}" ]]; then
    export CMD="default"
fi

export LANG=C
source tools/vars.sh

if [[ "$(uname)" == "FreeBSD" ]]; then
    export CORES=$(sysctl hw.ncpu | awk '{print $2}')
else
    export CORES=$(cat /proc/cpuinfo|grep processor|wc -l)
fi

mkdir build
# this need for some outdated os
mkdir m4
# for some os, libtoolize should be launch
libtoolize -i
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
make -j${CORES}
export RET=$?
cd -
exit $RET
