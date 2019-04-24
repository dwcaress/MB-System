#!/bin/bash

./configure --enable-test
make -j 3
make check
