#!/usr/bin/env bash

export CC=clang-8
export CXX=clang++-8
export LOGFILE=clang8.log

source ./tools/ci/scripts/init.sh

aptget_install clang-8 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
