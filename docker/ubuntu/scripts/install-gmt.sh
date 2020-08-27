#!/usr/bin/env bash
set -ex
# GSSHG and DCW can be installed easiest from repos.  They'll be used
# regardless of which GMT we install
apt-get install -y gmt-gshhg gmt-dcw

if [ -z "$GMT_SOURCE_TAG" ]
then
    apt-get install -y libgmt-dev gmt
    exit 0
fi

# Build GMT from sources
git clone --depth=1 --branch ${GMT_SOURCE_TAG} https://github.com/GenericMappingTools/gmt
mkdir -p gmt/build
cd gmt/build

cmake -DGSHHG_ROOT=/usr/share/gmt-gshhg -DDCW_ROOT=/usr/share/gmt-dcw ..
make
make install

# Update ldconfig
ldconfig -v

cd ../..
rm -rf gmt
