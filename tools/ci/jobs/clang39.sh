#!/usr/bin/env bash

export CC=clang-3.9
export CXX=clang++-3.9
export LOGFILE=gcc6.log

source ./tools/ci/scripts/init.sh

aptget_install clang-3.9 \
    git-core \
    make autoconf automake autopoint \
    libtool mariadb-client libmariadbclient-dev-compat libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
