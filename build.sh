#!/bin/bash

CMD="$1"

if [[ -z "${CMD}" ]]; then
    export CMD="default"
fi

mkdir build
# this need for some outdated os
mkdir m4
autoreconf -i
cd build
if [[ "${CMD}" == "default" ]]; then
    ../configure --enable-sanitize
elif [[ "${CMD}" == "old" ]]; then
    ../configure
fi
make -j3
export RET=$?
cd -
exit $RET
