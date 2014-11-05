#!/bin/bash

autoreconf -i
./configure
make -j3
