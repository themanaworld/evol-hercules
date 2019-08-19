#!/usr/bin/env bash

export CC=gcc-9
export CXX=g++-9
export LOGFILE=gcc9.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-9 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
