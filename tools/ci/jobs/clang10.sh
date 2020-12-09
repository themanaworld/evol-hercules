#!/usr/bin/env bash

export CC=clang-10
export CXX=clang++-10
export LOGFILE=clang10.log

source ./tools/ci/scripts/init.sh

aptget_install clang-10 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
