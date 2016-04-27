#!/bin/bash

export CC=gcc-5
export CXX=g++-5
export LOGFILE=gcc5.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-5 g++-5 \
    git-core \
    make autoconf automake autopoint gettext libphysfs-dev \
    libxml2-dev libcurl4-gnutls-dev libpng-dev \
    libsdl-gfx1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-net1.2-dev libsdl-ttf2.0-dev

do_init
clone_servercode
mkdir server-code/src/evol
cp -r evol-hercules/* server-code/src/evol/
check_error $?
cd server-code/src/evol
check_error $?

ls -la
ls -la ..
run_configure $*
run_make
