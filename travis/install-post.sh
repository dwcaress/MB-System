#!/bin/bash
#
# This is a post-install script common to all Docker images.
#
cat /etc/lsb-release

clang++ --version
gmt-config --version
