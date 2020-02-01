#!/usr/bin/env bash
set -ex

if [ -z "${MBSYSTEM_CONFIGURE_ARGS}" ]; then
    export MBSYSTEM_CONFIGURE_ARGS="--enable-mbtrn --enable-mbtnav"
fi

echo "Building MB-System..."
./configure ${MBSYSTEM_CONFIGURE_ARGS} --enable-test --with-proj-include=/usr/local/include --with-proj-lib=/usr/local/lib --with-fftw-include=/usr/local/include --with-fftw-lib=/usr/local/lib --with-motif-include=/usr/local/include --with-motif-lib=/usr/local/lib

make && make check
