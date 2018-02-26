#!/usr/bin/env bash

export CC=gcc-7
export CXX=g++-7
export LOGFILE=gcc7.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-7 \
    git-core \
    make autoconf automake autopoint \
    libtool libmysqlclient-dev libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
