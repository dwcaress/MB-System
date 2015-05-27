#-------------------------------------------------------------------------
# Version: $Id$
#------------------------------------------------------------------------------
# Notes on using the MB-System autotools based build system 
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
#
# Obtain the MB-System source tree by either downloading and unpacking an
# MB-System source distribution tarball (e.g. mbsystem-5.5.2245.tar.gz) from:
#   ftp://ftp.ldeo.columbia.edu/pub/mbsystem/
# or by downloading directly from the source code archive at:
#   http://svn.mb-system.org/listing.php?repname=MB-System
#
# To generate the makefiles needed to build MB-System, in a shell cd to the
# top of the MB-System source tree and run ./configure with the options
# appropriate for your situation. Some examples are given below.
#
# After configure you can run the make utility in the usual fashion
make
make install
#
# Some other useful make commands include:
make clean (to clean up compiled code)
make distclean (to clean up compiled code and configure output)
make uninstall (to remove a previously installed version)
#
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
#    --datarootdir         - The root of the directory tree for read-only architecture
#                            -independent data files.
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
#    --enable-bundledproj  - build using bundled proj package - the
#				default is to link with libproj
#                                
#------------------------------------------------------------------------------
# Mac Os X configure script command line examples:
#------------------------------------------------------------------------------
#
# Build in /usr/local on a Mac 10.9 or 10.10 with most prerequisites installed
# through Fink in /sw, and the OTPS program predict_time located
# in /usr/local/OTPS2.
# Prerequisite Fink packages include gmt, netcdf, proj, fftw3, and gv:
#
sudo CFLAGS="-I/opt/X11/include" LDFLAGS="-L/opt/X11/lib" \
./configure \
    --prefix=/usr/local \
    --with-netcdf-include=/sw/include \
    --with-netcdf-lib=/sw/lib \
    --with-gmt-include=/sw/include/gmt \
    --with-gmt-lib=/sw/lib \
    --with-proj-include=/sw/include \
    --with-proj-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib \
    --with-otps-dir=/usr/local/OTPS2
#
#------------------------------------------------------------------------------
# Ubuntu Linux configure script command line examples:
#------------------------------------------------------------------------------

# Install on Ubuntu (12.04.02LTS or 14.04LTS) using only apt-get for all
# prerequisites including GMT 5.1.2

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
sudo ./configure --prefix=/usr/local

# If the GMT installation is not known to pkg-config, then the installation
# points of the libraries and the header files must be specified:
sudo ./configure --prefix=/usr/local \
    --with-gmt-include=/usr/include/gmt \
    --with-gmt-lib=/usr/lib

# Either way, once configure has been run, build and install MB-System
# into /usr/local/bin, /usr/local/lib, etc with the simple make commands:
sudo make
sudo make install

#------------------------------------------------------------------------------

# Install on Ubuntu (12.04.02LTS or 14.04LTS) using apt-get for all
# prerequisites except GMT 5.1.2, which is built locally from source and
# installed into /usr/local

# Prerequisites excluding GMT
sudo apt-get install xorg-dev libmotif-dev libmotif4 libxp-dev mesa-common-dev \
    libsdl1.2-dev libsdl-image1.2-dev build-essential gfortran \
    nautilus-open-terminal libfftw3-3 libfftw3-dev \
    libnetcdf-dev netcdf-bin gdal-bin gdal1-dev gv
    
# GMT 5.1.2 is built from source using cmake and by default installs into
# /usr/local in Ubuntu. Follow the directions in the GMT Wiki.
# The specific installation points of the libraries and the header files will
# vary according to the architecture. On a machine with a 32-bit processor,
# the following configure options are required to build MB-System:
sudo ./configure --prefix=/usr/local \
    --with-gmt-include=/usr/local/include/gmt \
    --with-gmt-lib=/usr/local/lib/i386-linux-gnu

# Once configure has been run, build and install MB-System into /usr/local/bin,
# /usr/local/lib, etc with the simple make commands:
sudo make
sudo make install

#------------------------------------------------------------------------------
# CentOs configure script command line examples:
#------------------------------------------------------------------------------

# Install on CentOs 6 or 7 using only yum for prerequisites

# Prerequisites
sudo yum install openmotif openmotif-devel fftw fftw-devel netcdf netcdf-devel \
		proj proj-devel gdal-devel gmt gmt-devel gv

# If the prerequisites have all been installed with yum and it is desired to
# install MB-System in /usr/local, then only a simple call to configure is required:
sudo ./configure
#
#------------------------------------------------------------------------------

# Install on CentOs 6 or 7 using yum for prerequisites other than GMT
#
# Prerequisites
sudo yum install openmotif openmotif-devel fftw fftw-devel netcdf netcdf-devel \
		proj proj-devel gdal-devel gv

# If GMT 5.1.2 has been installed from source into /usr/local, and it is desired to
# install MB-System in /usr/local, then only the call to configure must specify
# the location of the GMT libraries and header files:
sudo ./configure --prefix=/usr/local \
    --with-gmt-include=/usr/local/include/gmt \
    --with-gmt-lib=/usr/local/lib64
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

# When you run ./configure, a number of configure options are saved to a
# header file:
#     ./src/mbio/mb_config.h
# This file has a template:
#     ./src/mbio/mb_config.h.in
# This file is conditionally included by:
#     ./src/mbio/mb_define.h
# which is in turn included by essentially every MB-System C source file.
#
#------------------------------------------------------------------------------
#
# Full autoconf and build sequence after modifying the build system
#   - Do this in the development tree prior to a commit to the source archive or
#     prior to making a source distribution
#
# First clean up old installation and build
make -j uninstall
make -j clean

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
    --with-gmt-include=/sw/include/gmt \
    --with-gmt-lib=/sw/lib \
    --with-fftw-include=/sw/include \
    --with-fftw-lib=/sw/lib \
    --with-motif-include=/sw/include \
    --with-motif-lib=/sw/lib \
    --with-otps-dir=/usr/local/OTPS2

make -j

make -j install

cd src/htmlsrc ; make_mbhtml ; cd ../..

make -j install

#
#------------------------------------------------------------------------------
