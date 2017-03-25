#!/usr/bin/env bash

export CC=gcc-5
export CXX=g++-5
export LOGFILE=gcc5.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-5 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
