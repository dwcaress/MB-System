#!/bin/bash

CFLAGS="-O2"
# TODO(schwehr): Enable more checks.
# CFLAGS="$CFLAGS -Wall"
# CFLAGS="$CFLAGS -Wextra"
CFLAGS="$CFLAGS" ./configure --enable-mbtrn --enable-mbtnav --enable-test
make -j 3
make check
