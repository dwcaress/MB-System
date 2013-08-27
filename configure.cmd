#------------------------------------------------------------------------------
# Notes on using an autotools based build system with the MB-System source
# code archive.
#------------------------------------------------------------------------------
#
# David W. Caress
# June 7, 2013
#------------------------------------------------------------------------------
#
# This build system was begun by Bob Covill in 2012, and then completed
# with a distributed, multi-continental effort by Bob Covill, Christian
# Ferreira, Hamish Bowman, Kurt Schwehr, and David Caress in May and June
# of 2013.
#
#------------------------------------------------------------------------------
# To modify the build system...
#------------------------------------------------------------------------------
#
# Edit the file "configure.in" in the top directory and "Makefile.am" in each
# directory and then run the following sequence of commands:

# Build libtool files for AM_PROG_LIBTOOL
libtoolize --force --copy
aclocal

# Build custom header for configure
autoheader
automake --add-missing --include-deps
autoconf

# To update configure files use the following:
autoupdate
autoreconf --force --install --warnings=all

# Reset the autotools version to 2.65 to accomodate some Linux distributions
sed -i.bak s/2\.69/2\.65/ configure.in

# When you run ./configure, a number of configure options are saved  to a
# header file:
#     ./src/mbio/mb_config.h
# This file has a template:
#     ./src/mbio/mb_config.h.in
# This file is conditionally included by:
#     ./src/mbio/mb_define.h
# which is in turn included by essentially every MB-System C source file.
# The condition under which mb_config.h is used is simply the use of the
# configure script to generate the makefiles. If one uses the alternate
# "install_makefiles" build system, then an alternate header file named
#     ./src/mbio/mb_config2.h
# is used instead.
#
#------------------------------------------------------------------------------
# To use the build system...
#------------------------------------------------------------------------------
# To generate the makefiles needed to build MB-System, run ./configure
# with the options appropriate for your situation. Some examples are
# given below.
#
# After configure you can run the make utility in the usual fashion
make
make install
#
# Some other useful make commands include:
make clean (to clean up compiled code)
make distclean (to clean up compiled code and configure output)
make uninstall (to remove a previously installed version)

#------------------------------------------------------------------------------
# Configure script command line options:
#------------------------------------------------------------------------------
# --prefix=install      - location for mbsystem (/usr/local/mbsystem)
# --with-netcdf-lib     - location of NetCDF libs
# --with-netcdf-include - location of NetCDF headers
# --with-gmt-lib        - location of GMT libs
# --with-gmt-include    - location of GMT headers
# --with-fftw-lib       - location of FFTW3 libs (optional)
# --with-fftw-include   - location of FFTW3 headers (optional)
# --with-motif-lib      - location of Motif libs (optional)
# --with-motif-include  - location of Motif headers (optional)
# --with-opengl-lib     - location of OpenGL libs (optional)
# --with-opengl-include - location of OpenGL headers (optional)
# --with-otps-dir       - location of OTPS installation (optional)
# --without-gsf         - build without including or supporting GSF
# --enable-static       - build using static libraries - shared libraries are default

#------------------------------------------------------------------------------
# Configure script command line examples:
#------------------------------------------------------------------------------

# Build in place:
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib" \
./configure \
    --prefix=/Users/caress/sandbox/mbsystem \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib

#------------------------------------------------------------------------------

# Build in /usr/local:
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib" \
sudo ./configure \
    --prefix=/usr/local \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib

#------------------------------------------------------------------------------

# Build in ~/buildtest
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib" \
./configure \
    --prefix=/Users/caress/buildtest \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib

#------------------------------------------------------------------------------
#
# Full autoconf and build sequence
#   - Do this in the development tree prior to a commit to the source archive or
#     prior to making a source distribution
#
# First clean up old installation and build
make uninstall
make clean

# Reconstruct the build system, and then use it to build in place
# in my personal development tree
libtoolize --force --copy
aclocal
autoheader
automake --add-missing --include-deps
autoconf
autoupdate

autoreconf --force --install --warnings=all

# Force configure.in to reduce the automake version requirement from 2.69 to 2.65
sed -i.bak s/2\.69/2\.65/ configure.in

CFLAGS="-g -I/usr/X11R6/include" LDFLAGS="-L/usr/X11R6/lib" \
./configure \
    --prefix=/Users/caress/sandbox/mbsystem \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib \
    --with-otps-dir=/usr/local/OTPS2
#--without-gsf

make

cd src/htmlsrc ; make_mbhtml ; cd ../..

make install

#------------------------------------------------------------------------------

# Install on Ubuntu 12.04.02LTS using only apt-get for prerequisites

# Required environment variables to be set in ~/.bashrc
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export PATH=/usr/lib/gmt/bin:$PATH

# Prerequisites
sudo apt-get install xorg-dev libmotif-dev libmotif4 libxp-dev mesa-common-dev \
    libsdl1.2-dev libsdl-image1.2-dev build-essential gfortran \
    nautilus-open-terminal libfftw3-3 libfftw3-dev \
    libnetcdf-dev netcdf-bin gdal-bin gdal1-dev gmt libgmt-dev gv

# configure call
CFLAGS="-g" LDFLAGS="-lpsl" \
./configure \
    --prefix=/usr/local \
    --with-gmt-include=/usr/include/gmt \
    --with-gmt-lib=/usr/lib

# build and install into /usr/local/bin, /usr/local/lib, etc
make
sudo make install

#------------------------------------------------------------------------------
