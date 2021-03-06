#!/usr/bin/env bash

export CC=clang-7
export CXX=clang++-7
export LOGFILE=clang7.log

source ./tools/ci/scripts/init.sh

aptget_install clang-7 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
