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
#    --with-netcdf-config  - location of NetCDF config script nc-config
#    --with-gdal-config    - location of GDAL config script gdal-config
#    --with-gmt-config     - location of GMT5 config script gmt-config
#    --with-proj-lib       - location of PROJ libs
#    --with-proj-include   - location of PROJ headers
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
# Setting X11 fonts used by mbgrdviz, mbeditviz, mbedit, mbnavedit, mbnavadjust
# and mbvelocitytool:
#------------------------------------------------------------------------------
# By default the graphical utilities use three fonts: Helvetica, Times New
# Roman, and Courier. This can be set in the CFLAGS environment variable by
# including options of the form:
#       -DSANS='\"helvetica\"' -DSERIF='\"times\"' -DMONO='\"courier\"'
# In the examples below, the CFLAGS environment value is set for the configure
# script by setting it on the same command line as ./configure. To set the
# fonts to Lucida, one might add:
#       -DSANS='\"lucida\"' -DSERIF='\"lucida\"' -DMONO='\"lucidatypewriter\"'
# to the CFLAGS definition
#
#------------------------------------------------------------------------------
# Mac Os X configure script command line examples:
#------------------------------------------------------------------------------
#
# Build in /usr/local on a Mac 10.9 to 10.11 with most prerequisites installed
# through Fink in /sw, and the OTPS program predict_time located
# in /usr/local/OTPS2.
# Prerequisite Fink packages include gmt, netcdf, gdal, proj, fftw3, and gv:
fink install gmt5 gmt5-dev gmt5-doc gmt5-shlibs openmotif4 \
        netcdf-bin netcdf-c7 netcdf-c7-shlibs gdal gdal-dev gdal-shlibs \
        proj proj-bin proj-shlibs fftw3 fftw3-shlibs \
        openmotif4 openmotif4-bin openmotif- libicu55-shlibs gv
#
sudo CFLAGS="-I/opt/X11/include" LDFLAGS="-L/opt/X11/lib" \
./configure \
    --prefix=/usr/local \
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
# 
# MB-System now requires GMT 5.2.1 or later (e.g. GMT 5.3.0r16365 downloaded
# directly from the GMT source code repository). The standard Ubuntu package
# manager installs previous versions of GMT for Ubuntu 12 and 14. On Ubuntu 16.04,
# GMT 5.2.1 can be installed via apt or apt-get, but the installation omits header
# files that are essential for building MB-System. Consequently as of May 2016 an
# MB-System installation on Ubuntu requires a manual installation of GMT5.
# 
# For Ubuntu Linux 12 or 14, the following use of the apt-get utility will install
# all of the MB-System prerequisites excepting GMT5 and OTPS:
# 
sudo apt-get install xorg-dev libmotif-dev libmotif4 libxp-dev mesa-common-dev \
    libsdl1.2-dev libsdl-image1.2-dev build-essential gfortran \
    nautilus-open-terminal libfftw3-3 libfftw3-dev \
    libnetcdf-dev netcdf-bin gdal-bin gdal1-dev gv
# 
# For Ubuntu Linux 16, some of the prerequisite package names have changed.The
# following use of the apt-get utility will install all of the MB-System
# prerequisites excepting GMT5 and OTPS:
# 
sudo apt-get install xorg-dev \
	libxm4:i386 libuil4:i386 libmrm4:i386 libxm4 libuil4 libmrm4 \
	libmotif-common libmotif-dev libgl1-mesa-dev libglu1-mesa-dev \
 	mesa-common-dev libfftw3-3 gfortran libfftw3-dev \
    	libnetcdf-dev netcdf-bin libproj9 libproj-dev gdal-bin libgdal-dev gmt
# 
# In order to install GMT5, follow the installation instructions in the GMT
# documentation or in the excellent GMT5 installation guide by Andrew Moody at:
#     http://andrewjmoodie.com/2014/12/installing-the-generic-mapping-tools-5-gmt-5-1-x-on-ubuntu-linux/
# 
# If the prerequisites have all been installed as shown above, and it is desired
# to install MB-System in /usr/local, then only a simple call to configure is
# required:
# 
sudo ./configure
# 
# Once the makefiles have been generated by configure, build and install using:
# 
sudo make
sudo make install
# 
# In some cases the system and/or user environment impedes the successful use of
# the GMT and/or MB-System shared libraries. In order to manually allow shared
# libraries to be found for linking or running, one can either set the CFLAGS
# environment variable during building or set the LD_LIBRARY_PATH environment
# variable at login by adding a command to the user's ~/.profile or ~/.bashrc files.
# 
# To set the CFLAGS environment variable during building include
# "-Wl,-rpath -Wl,LIBDIR" in the configure command as shown here:
# 
sudo CFLAGS="-Wl,-rpath -Wl,/usr/local/lib" ./configure
# 
# To augment the LD_LIBRARY_PATH environment variable during login add a line to
# the ~/.bashrc or ~/.profile file as shown here:
# 
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# 
#------------------------------------------------------------------------------
# CentOs configure script command line examples:
#------------------------------------------------------------------------------

# Install on CentOs 6 or 7 using yum for most or all prerequisites

# Prerequisites
sudo yum install openmotif openmotif-devel fftw fftw-devel netcdf netcdf-devel \
		proj proj-devel gdal-devel gmt gmt-devel gv

# If the prerequisites have all been installed with yum and it is desired to
# install MB-System in /usr/local, then only a simple call to configure is required:
sudo ./configure

# If NetCDF, GDAL, and GMT have been installed locally, then still only a
# simple call to configure is required:
sudo ./configure
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

# Force configure.ac to reduce the autoconf version requirement from 2.69 to 2.65
#sed -i.bak s/2\.69/2\.65/ configure.ac

CFLAGS="-g -Wall -Wmissing-prototypes -I/opt/X11/include " LDFLAGS="-L/opt/X11/lib" \
./configure \
    --prefix=/Users/caress/sandbox/mbsystem \
    --disable-static \
    --enable-shared \
    --with-proj-include=/sw/include \
    --with-proj-lib=/sw/lib \
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
