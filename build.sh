#!/bin/bash

mkdir build
# this need for some outdated os
mkdir m4
autoreconf -i
cd build
../configure --enable-sanitize
make -j3
cd -
exit $?
