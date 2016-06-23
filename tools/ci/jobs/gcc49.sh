#!/bin/bash

export CC=gcc-4.9
export CXX=g++-4.9
export LOGFILE=gcc4.9.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-4.9 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
