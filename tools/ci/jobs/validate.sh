#!/bin/bash

export CC=gcc-5
export CXX=g++-5
export LOGFILE=gcc5.log

source ./tools/ci/scripts/init.sh

aptget_install git-core ca-certificates \
    make python

do_init
clone_tool
clone_servercode
cd evol-hercules

make validate
check_error $?
