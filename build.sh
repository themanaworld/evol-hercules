#!/bin/bash

mkdir build
autoreconf -i
cd build
../configure
make -j3
cd -
