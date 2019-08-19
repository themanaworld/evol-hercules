#!/usr/bin/env bash

export CC=gcc-8
export CXX=g++-8
export LOGFILE=gcc8.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-8 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
