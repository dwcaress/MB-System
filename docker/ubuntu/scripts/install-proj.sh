#!/usr/bin/env bash
set -ex

if [ -z "$PROJ_SOURCE_TAG" ]
then
    apt-get install -y \
	    libproj-dev \
	    proj-bin \
	    libgdal-dev \
	    gdal-bin
    exit 0
fi

# Dependencies
apt-get install -y sqlite3 libsqlite3-dev curl

# Build PROJ from sources
git clone --depth=1 --branch ${PROJ_SOURCE_TAG} https://github.com/OSGeo/PROJ
# Download datum files
curl -L https://github.com/OSGeo/proj-datumgrid/releases/download/1.8/proj-datumgrid-1.8.tar.gz | tar -xzv -C PROJ/data

mkdir -p PROJ/build
pushd PROJ/build
cmake .. 

# build and install
make 
make install

popd

# GDAL dependencies
apt-get install -y \
	libcurl4-openssl-dev \
	libexpat-dev \
	libnetcdf-dev \
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

# Update ldconfig
ldconfig -v

popd

rm -rf PROJ gdal
