#!/bin/bash

export CC=gcc-4.8
export CXX=g++-4.8
export LOGFILE=gcc4.8.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-4.8 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make