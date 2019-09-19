#!/bin/bash

CFLAGS='-O2 -Wall' ./configure --enable-mbtrn --enable-test
make -j 3
make check
