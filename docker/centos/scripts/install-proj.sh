#!/usr/bin/env bash
set -ex

if [ -z "$PROJ_SOURCE_TAG" ]
then
    yum install -y \
	    proj-devel \
	    proj \
	    gdal-devel \
	    gdal
    exit 0
fi

# Dependencies
yum install -y sqlite-devel sqlite curl

# Build PROJ from sources
git clone --depth=1 --branch ${PROJ_SOURCE_TAG} https://github.com/OSGeo/PROJ
# Download datum files
curl -L https://github.com/OSGeo/proj-datumgrid/releases/download/1.8/proj-datumgrid-1.8.tar.gz | tar -xzv -C PROJ/data

mkdir -p PROJ/build
pushd PROJ/build
cmake3 .. -DPROJ_TESTS=OFF

# build and install
make 
make install

popd

# GDAL dependencies
yum install -y \
	libcurl-devel \
	expat-devel \
	automake \
	autoconf \
	libtool
	
# Build GDAL
git clone --depth=1 --branch v3.0.3 https://github.com/OSGeo/gdal
pushd gdal/gdal
autoreconf -fvi
./autogen.sh
./configure
make -j3 
make install

popd

rm -rf PROJ gdal
