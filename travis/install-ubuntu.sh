#!/bin/bash

set -ex
# apt-get update
apt-get install -y locales
locale-gen en_US.UTF-8

PACKAGES=(build-essential clang)
PACKAGES+=(gmt gmt-gshhg libgmt-dev)
PACKAGES+=(libfftw3-dev)
PACKAGES+=(libproj-dev)
PACKAGES+=(gdal-bin libgdal-dev)
PACKAGES+=(libnetcdf-dev netcdf-bin)
PACKAGES+=(python3)

# TODO(schwehr): Be able to build without x11 stuff.
PACKAGES+=(libmotif-dev libglu1-mesa-dev mesa-common-dev)

apt-get install -y "${PACKAGES[@]}"
