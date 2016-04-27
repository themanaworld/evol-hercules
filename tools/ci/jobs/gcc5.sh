#!/bin/bash

export CC=gcc-5
export CXX=g++-5
export LOGFILE=gcc5.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-5 g++-5 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
clone_servercode
mkdir server-code/src/evol
mkdir -p server-data/plugins
cp -r evol-hercules/* server-code/src/evol/
check_error $?
cd server-code/src/evol
check_error $?

ls -la
ls -la ..
run_configure $*
run_make
