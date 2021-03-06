#!/usr/bin/env bash

export CC=gcc-4.6
export CXX=g++-4.6
export LOGFILE=gcc4.6.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-4.6 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
