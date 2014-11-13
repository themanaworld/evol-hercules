#!/bin/bash

mkdir build
autoreconf -i
cd build
../configure --enable-sanitize
make -j3
cd -
exit $?
