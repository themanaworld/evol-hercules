#!/usr/bin/env bash

export CC=clang-6.0
export CXX=clang++-6.0
export LOGFILE=clang6.log

source ./tools/ci/scripts/init.sh

aptget_install clang-6.0 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
