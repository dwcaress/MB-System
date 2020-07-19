#!/usr/bin/env sh
set -ex
# GSSHG and DCW can be installed easiest from repos.  They'll be used
# regardless of which GMT we install
yum install -y gshhg-gmt-nc4 dcw-gmt

if [ -z "$GMT_SOURCE_TAG" ]
then
    yum install -y gmt gmt-devel
    exit 0
fi

# Build GMT from sources
git clone --depth=1 --branch ${GMT_SOURCE_TAG} https://github.com/GenericMappingTools/gmt
mkdir -p gmt/build
cd gmt/build

cmake3 -DGSHHG_ROOT=/usr/share/gshhg-gmt -DDCW_ROOT=/usr/share/dcw-gmt ..
make
make install

# Update ldconfig
ldconfig -v

cd ../..
rm -rf gmt
