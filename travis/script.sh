#!/bin/bash

CFLAGS=-O2 ./configure --enable-test
make -j 3
make check
