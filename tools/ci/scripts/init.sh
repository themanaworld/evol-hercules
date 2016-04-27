#!/bin/bash

pwd
mkdir logs

export dir=$(pwd)
export ERRFILE=${dir}/logs/${LOGFILE}

cat /etc/os-release

rm ${ERRFILE}

function do_init {
    $CC --version
    $CXX --version
}

function aptget_update {
    echo "apt-get update"
    apt-get update
    if [ "$?" != 0 ]; then
        sleep 1s
        apt-get update
        if [ "$?" != 0 ]; then
            sleep 2s
            apt-get update
            if [ "$?" != 0 ]; then
                sleep 5s
                apt-get update
            fi
        fi
    fi
}

function aptget_install {
    echo "apt-get -y -qq install $*"
    apt-get -y -qq install $*
    if [ "$?" != 0 ]; then
        sleep 1s
        apt-get -y -qq install $*
        if [ "$?" != 0 ]; then
            sleep 2s
            if [ "$?" != 0 ]; then
                sleep 5s
                apt-get -y -qq install $*
            fi
            apt-get -y -qq install $*
        fi
    fi
}

function check_error {
    if [ "$1" != 0 ]; then
        echo "error $1"
        exit $result
    fi
}

function run_configure_simple {
    echo "autoreconf -i"
    autoreconf -i
    check_error $?
    mkdir build
    cd build
    echo "../configure $*"
    ../configure $*
    check_error $?
}

function run_configure {
    run_configure_simple $*
}

function run_make {
    echo "make -j2 V=0 $*"
    make -j2 V=0 $*
    check_error $?
}

function run_check_warnings {
    DATA=$(cat $ERRFILE)
    if [ "$DATA" != "" ];
    then
        cat $ERRFILE
        exit 1
    fi
}

function run_h {
    rm $ERRFILE
    echo "$CC -c -x c++ $* $includes */*/*/*.h */*/*.h */*.h *.h"
    $CC -c -x c++ $* $includes */*/*/*.h */*/*.h */*.h *.h 2>$ERRFILE
    DATA=$(cat $ERRFILE)
    if [ "$DATA" != "" ];
    then
        cat $ERRFILE
        exit 1
    fi
}

function run_tarball {
    rm $ERRFILE
    echo "make dist-xz"
    make dist-xz 2>$ERRFILE
    check_error $?

    mkdir $1
    cd $1
    echo "tar xf ../*.tar.xz"
    tar xf ../*.tar.xz
    cd manaplus*
}

function run_mplint {
    rm $ERRFILE
    echo "mplint/src/mplint $*"
    mplint/src/mplint $* >$ERRFILE
    check_error $?
    run_check_warnings
}

aptget_update
