#!/bin/bash

pwd
mkdir logs

export dir=$(pwd)
export ERRFILE=${dir}/logs/${LOGFILE}

cat /etc/os-release

rm ${ERRFILE}

function do_init {
    cd ..
    ln -s evol-hercules server-plugin
}

function update_repos {
    if [ "$CI_SERVER" == "" ];
    then
        return
    fi

    export DATA=$(cat /etc/resolv.conf|grep "nameserver 1.10.100.101")
    if [ "$DATA" != "" ];
    then
        echo "Detected local runner"
        sed -i 's!http://httpredir.debian.org/debian!http://1.10.100.103/debian!' /etc/apt/sources.list
    else
        echo "Detected non local runner"
    fi
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

function gitclone1 {
    echo git clone $2 $3
    git clone $2 $3
    if [ "$?" != 0 ]; then
        echo git clone $1 $3
        git clone $1 $3
        return $?
    fi
    return $?
}

function gitclone {
    export name1=$1/$2
    export name2=${CI_BUILD_REPO##*@}
    export name2=https://${name2%/*}/$2

    gitclone1 "$name1" "$name2" $3
    if [ "$?" != 0 ]; then
        sleep 1s
        gitclone1 "$name1" "$name2" $3
        if [ "$?" != 0 ]; then
            sleep 3s
            gitclone1 "$name1" "$name2" $3
            if [ "$?" != 0 ]; then
                sleep 5s
                gitclone1 "$name1" "$name2" $3
            fi
        fi
    fi
    check_error $?
}

function check_error {
    if [ "$1" != 0 ]; then
        echo "error $1"
        exit $1
    fi
}

function run_configure_simple {
    echo "autoreconf -i"
    mkdir m4
    autoreconf -i
    check_error $?
    mkdir build
    cd build
    echo "../configure $*"
    ../configure $*
    check_error $?
}

function run_configure {
    run_configure_simple $* CPPFLAGS="${VARS}"
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

function clone_tool {
    rm -rf tools
    gitclone https://gitlab.com/evol evol-tools.git tools
}

function clone_servercode {
    rm -rf server-code
    gitclone https://gitlab.com/evol hercules.git server-code
}

function build_init {
    clone_servercode
    mkdir server-code/src/evol
    mkdir -p server-data/plugins
    cp -r evol-hercules/* server-code/src/evol/
    check_error $?
    cd server-code/src/evol
    source tools/vars.sh
    check_error $?
}

update_repos
aptget_update
