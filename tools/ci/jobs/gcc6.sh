#!/usr/bin/env bash

export CC=gcc-6
export CXX=g++-6
export LOGFILE=gcc6.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-6 \
    git-core \
    make autoconf automake autopoint \
    libtool mariadb-client libmariadbclient-dev-compat libz-dev libpcre3-dev

do_init
build_init

run_configure $*
run_make
