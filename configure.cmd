#------------------------------------------------------------------------------
# Notes on using an autotools based build system with the MB-System source
# code archive.
#------------------------------------------------------------------------------
#
# David W. Caress
# June 7, 2013
#------------------------------------------------------------------------------
#
# This build system was begun by Bob Covill in 2011, and then completed
# with a distributed, multi-continental effort by Bob Covill, Christian
# Ferreira, Hamish Bowman, Kurt Schwehr, and David Caress in May to August
# of 2013.
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
#   Installation location:
#------------------------
#    --prefix              - This is the common installation prefix for all files.
#                            If exec_prefix is defined to a different value, prefix
#                            is used only for architecture-independent files.
#                            [Default: /usr/local]
#    --exec_prefix         - The installation prefix for architecture-dependent files.
#                            By default it's the same as prefix. You should avoid
#                            installing anything directly to exec_prefix. However,
#                            the default value for directories containing
#                            architecture-dependent files should be relative to
#                            exec_prefix.
#                            [Default: ${prefix} ==> /usr/local]
#    --datarootdir         - The root of the directory tree for read-only architecture-independent data files.
#                            [Default: ${exec_prefix}/share ==> /usr/local/share]
#    --bindir              - The directory for installing executables that users run.
#                            [Default: ${exec_prefix}/bin ==> /usr/local/bin]
#    --libdir              - The directory for installing object code libraries.
#                            [Default: ${exec_prefix}/lib ==> /usr/local/lib]
#    --includedir          - The directory for installing C header files.
#                            [Default: ${exec_prefix}/include ==> /usr/local/include]
#------------------------
#   Prerequisite location:
#------------------------
#    --with-netcdf-lib     - location of NetCDF libs
#    --with-netcdf-include - location of NetCDF headers
#    --with-proj-lib       - location of PROJ libs
#    --with-proj-include   - location of PROJ headers
#    --with-gmt-lib        - location of GMT libs
#    --with-gmt-include    - location of GMT headers
#    --with-fftw-lib       - location of FFTW3 libs 
#    --with-fftw-include   - location of FFTW3 headers 
#    --with-motif-lib      - location of Motif libs 
#    --with-motif-include  - location of Motif headers 
#    --with-opengl-lib     - location of OpenGL libs 
#    --with-opengl-include - location of OpenGL headers 
#    --with-otps-dir       - location of OTPS installation 
#------------------------
#   Installation option:
#------------------------
#    --without-gsf         - build without including or supporting GSF
#                            The default is to build the bundled 
#                                gsf library as libmbgsf and link with it
#    --enable-bundledproj  - build using bundled proj package - the
#				default is to link with libproj
#                                
#------------------------------------------------------------------------------
# Configure script command line examples:
#------------------------------------------------------------------------------

# Build in place on a Mac 10.9 with prerequisites installed through Fink in /sw:
CFLAGS="-I/opt/X11/include -L/opt/X11/lib" \
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

# Build in /usr/local on a Mac 10.9 with prerequisites installed through Fink in /sw:
CFLAGS="-I/opt/X11/include -L/opt/X11/lib" \
sudo ./configure \
    --prefix=/usr/local \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-proj-include=/sw/include \
    --with-proj-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib \
    --with-otps-dir=/usr/local/tides/OTPS2

#------------------------------------------------------------------------------

# Build in ~/buildtest on a Mac 10.9 with prerequisites installed through Fink in /sw:
CFLAGS="-I/opt/X11/include -L/opt/X11/lib" \
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

# Install on Ubuntu 12.04.02LTS using only apt-get for prerequisites

# Required environment variables to be set in ~/.bashrc
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export PATH=/usr/lib/gmt/bin:$PATH

# Prerequisites
sudo apt-get install xorg-dev libmotif-dev libmotif4 libxp-dev mesa-common-dev \
    libsdl1.2-dev libsdl-image1.2-dev build-essential gfortran \
    nautilus-open-terminal libfftw3-3 libfftw3-dev \
    libnetcdf-dev netcdf-bin gdal-bin gdal1-dev gmt libgmt-dev gv

# The GMT installation does not include a pkg-config *.pc file, but it is
# simple to construct one. On Ubuntu pkg-config looks in /usr/lib/pkgconfig,
# so one can put a file there called gmt.pc with the contents:
#---------------
# <start gmt.pc>
prefix=/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/gmt
ccompiler=gcc
cppcompiler=g++

Name: gmt
Description: GMT Libraries
Version: 4.5.6
Libs: -L${libdir} -lgmt -lgmtps -lpsl
Cflags: -I${includedir}
# <end gmt.pc>
#---------------

# If the GMT installation has been augmented with a /usr/lib/pkgconfig/gmt.pc
# file so that pkg-config knows about GMT, then the configure call is just:
./configure --prefix=/usr/local

# If the GMT installation is not known to pkg-config, then the installation
# points of the libraries and the header files must be specified:
./configure --prefix=/usr/local \
    --with-gmt-include=/usr/include/gmt \
    --with-gmt-lib=/usr/lib

# Either way, once configure has been run, build and install MB-System
# into /usr/local/bin, /usr/local/lib, etc with the simple make commands:
make
sudo make install
#
#------------------------------------------------------------------------------
# To modify the build system...
#------------------------------------------------------------------------------
#
# Edit the file "configure.ac" in the top directory and "Makefile.am" in each
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
sed -i.bak s/2\.69/2\.65/ configure.ac

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
#
# Full autoconf and build sequence after modifying the build system
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

# Force configure.ac to reduce the automake version requirement from 2.69 to 2.65
sed -i.bak s/2\.69/2\.65/ configure.ac

CFLAGS="-g -Wall -I/opt/X11/include" LDFLAGS="-L/opt/X11/lib" \
./configure \
    --prefix=/Users/caress/sandbox/mbsystem \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-proj-include=/sw/include \
    --with-proj-lib=/sw/lib \
    --with-gmt-include=/sw/include \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib \
    --with-otps-dir=/usr/local/OTPS2
#    --without-gsf \
#    --enable-bundledproj

make

make install

cd src/htmlsrc ; make_mbhtml ; cd ../..

make install

#
#------------------------------------------------------------------------------
