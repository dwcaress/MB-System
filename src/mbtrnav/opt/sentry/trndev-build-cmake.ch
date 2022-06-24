#!/bin/bash

# Build the trndev packages as described in README-trndev-build.md

if [ ! -d mframe ]
then
echo " missing mframe directory"
exit -1
fi

if [ ! -d libtrnav ]
then
echo " missing libtrnav directory"
exit -1
fi

CMAKE=$(which cmake)

TRNDEV_TOP=$PWD

TRNDEV_INSTALL=${TRNDEV_INSTALL:-${TRNDEV_TOP}/install}

build_cmake_pkg(){
    mkdir build
    cd build
    ${CMAKE} ..
    ${CMAKE} --build .
    ${CMAKE} --install . --prefix `pwd`/pkg
    if [ ! -d ${TRNDEV_INSTALL} ]
    then
        mkdir -p ${TRNDEV_INSTALL}
    fi
    echo "installing to ${TRNDEV_INSTALL}"
    ${CMAKE} --install . --prefix ${TRNDEV_INSTALL}
}

cd ${TRNDEV_TOP}/mframe
build_cmake_pkg

cd ${TRNDEV_TOP}/libtrnav
build_cmake_pkg

