#!/usr/bin/env bash
set -ex

# The netCDF4 C++ api needs base netcdf4 installed from source
curl -L ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-c-4.7.3.tar.gz | tar zxv
cd netcdf-c-4.7.3
./configure
make
make install
cd ..

# See https://github.com/Unidata/netcdf-cxx4
git clone https://github.com/Unidata/netcdf-cxx4.git
cd netcdf-cxx4
mkdir build
cd build
export netCDFCxx_DIR=/usr/local/lib64/cmake/netCDF/
cmake3 ..
make
make install
cd ../..
