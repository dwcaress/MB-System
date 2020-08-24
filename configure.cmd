#-------------------------------------------------------------------------
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
# MacOs Sierra configure script command line example:
#------------------------------------------------------------------------------
#
# Install MB-System into /usr/local on a Mac running MacOS 10.12 Sierra with most
# prerequisites installed using the homebrew package manager. The other two
# prerequisites are to install the Xcode compiler suite from the Apple App Store
# and to install the X11 windowing system by downloading the XQuartz
# package from xquartz.org.
#
# The primary prerequisite homebrew packages can be installed using:
brew install gdal netcdf proj fftw gv openmotif
brew install homebrew/science/gmt
#
# The tide modeling software package OTPS from Oregon State University can be
# downloaded from: http://volkov.oce.orst.edu/tides/otps.html and installed
# manually (typically in /usr/local/OTPS2 or /usr/local/src/OTPS2.
# Optionally, a homebrew tap is available:
brew install dwcaress/mbsystem/otps --with-tpxo8
#
# These steps assume you have downloaded an MB-System distribution tar.gz file
# from the ftp site:
#       ftp://mbsystemftp@ftp.mbari.org
# The current distribution file is mbsystem-5.5.2307.tar.gz
#
# First, unpack the distribution using tar. This can be done in any location,
# but if one is working in a root-owned area such as /usr/local/src, obtaining
# root privileges using sudo is necessary for all steps:
#       sudo tar xvzf mbsystem-5.5.2307.tar.gz
# and then cd into the resulting directory:
#       cd mbsystem-5.5.2307
#
# The following MB-System configure command will work with all prerequisites
# installed via homebrew with the commands listed above:
sudo CFLAGS="-I/opt/X11/include" LDFLAGS="-L/opt/X11/lib" \
./configure \
    --prefix=/usr/local \
    --with-proj-include=/usr/local/include \
    --with-proj-lib=/usr/local/lib \
    --with-fftw-include=/usr/local/include \
    --with-fftw-lib=/usr/local/lib \
    --with-motif-include=/usr/local/include \
    --with-motif-lib=/usr/local/lib \
    --with-otps-dir=/usr/local/opt/otps
#
# Once the makefiles have been generated by configure, build and install
# MB-System using:
        sudo make
        sudo make install
#
#------------------------------------------------------------------------------
# Ubuntu Linux configure script command line example:
#------------------------------------------------------------------------------
#
# The following procedure serves to install the current version of MB-System
# (5.5.2307) on new installations of Ubuntu 14.04 and 16.04 in June 2017.
# The methodology is essentially the same for both versions of
# Ubuntu, with the exception that in 14.04LTS one must build GMT from source, but
# in 16.04 a sufficiently recent GMT release is available in the package repository.
#
# GMT:
# ----
# GMT is a key prerequisite for MB-System, which now requires GMT 5.2.1 or later.
# One can check which GMT version is available by querying the package manager in
# a terminal:
        apt-cache show gmt
#
# If the repositories you are already accessing do not include GMT, one can add the
# UbuntuGIS repo by typing:
        sudo add-apt-repository ppa:ubuntugis/ppa
        sudo apt-get update
        sudo apt-get upgrade
#
# These commands will add a GIS repository containing GMT, GDAL, GRASS,
# QGIS, etc, including MB-System:
#         https://launchpad.net/~ubuntugis/+archive/ubuntu/ppa
# Note that some packages may not be the current release, as this is the
# stable repository. As of early October 2016, the UbuntuGIS stable repository
# includes GMT 5.1.2, which is not recent enough for the current MB-System
# release.
#
# Fortunately, there is also an unstable version:
#         https://launchpad.net/~ubuntugis/archive/ubuntu/ubuntugis-unstable
# that includes more recent versions of some packages (although they have not
# all been fully tested). As of early October 2016, GMT 5.2.1 is included in
# the unstable repository.
#
# Once you have verified that GMT-5.2.1 or later is available to you, install the
# primary package and the documentation, library files, development headers, and
# hierarchical high resolution geography:
        sudo apt-get install gmt libgmt5 libgmt-dev gmt-gshhg gmt-doc
#
# If you need (or want) to build GMT5 from source, follow the installation instructions in the GMT
# online wiki:
#        http://gmt.soest.hawaii.edu/projects/gmt/wiki/BuildingGMT#Building-GMT-from-source
# or alternatively, there is an excellent GMT5 installation guide by Andrew Moody at:
#        http://andrewjmoodie.com/2014/12/installing-the-generic-mapping-tools-5-gmt-5-1-x-on-ubuntu-linux/
#
# Motif:
# ------
# Another MB-System dependency that can be apparently unavailable is libmotif4. The
# Motif widget set is used by all of the MB-System interactive graphical tools. Do
# not use the old LessTif package - this served as a functional close of Motif long
# ago, but no longer duplicates all of the needed Motif functionality. Instead,
# install some version of Motif4. If
        apt-cache show libmotif4
# indicates that libmotif4 is unavailable, then add the ubuntu extras repo using
# the commands:
        sudo su
        echo 'deb http://cz.archive.ubuntu.com/ubuntu trusty main universe' \
            >> /etc/apt/sources.list.d/extra.list
        apt-get install update
# You will need to re-login or open a new terminal in order for this to take effect.
#
# Install X11 dependencies of Motif as well as libmotif4:
        sudo apt-get install libx11-dev xorg-dev libmotif-dev libmotif4 \
            libxp-dev mesa-common-dev libsdl1.2-dev libsdl-image1.2-dev
#
# Other dependencies:
# -------------------
# The many other MB-System dependencies are available for both 14.04 and 16.04, and
# can be installed via:
        sudo apt-get install  build-essential gfortran nautilus-open-terminal \
            libfftw3-3 libfftw3-dev libnetcdf-dev netcdf-bin \
            libgdal-bin gdal-dev gv csh libgmt-dev libproj-dev
#
# Everything at once:
# -------------------
# In fact, it should be possible to install all MB-System dependencies in a single
# apt-get command:
        sudo apt-get install gmt libgmt5 libgmt-dev gmt-gshhg gmt-doc \
            libx11-dev xorg-dev libmotif-dev libmotif4 \
            libxp-dev mesa-common-dev libsdl1.2-dev libsdl-image1.2-dev \
            build-essential gfortran nautilus-open-terminal \
            libfftw3-3 libfftw3-dev libnetcdf-dev netcdf-bin \
            libgdal-bin gdal-dev gv csh libgmt-dev libproj-dev
#
# MB-System:
# ----------
# These steps assume you have downloaded an MB-System distribution tar.gz file
# from the ftp site:
#       ftp://mbsystemftp@ftp.mbari.org
# The current distribution file is mbsystem-5.5.2307.tar.gz
#
# First, unpack the distribution using tar. This can be done in any location,
# but if one is working in a root-owned area such as /usr/local/src, obtaining
# root privileges using sudo is necessary for all steps:
#       sudo tar xvzf mbsystem-5.5.2307.tar.gz
# and then cd into the resulting directory:
#       cd mbsystem-5.5.2307
#
# If the prerequisites have all been installed as shown above, and it is desired
# to install MB-System in /usr/local, then only a simple call to configure is
# required:
       sudo ./configure
#
# Once the makefiles have been generated by configure, build and install using:
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
#       sudo CFLAGS="-Wl,-rpath -Wl,/usr/local/lib" ./configure
#
# To augment the LD_LIBRARY_PATH environment variable during login add a line to
# the ~/.bashrc or ~/.profile file as shown here:
#
#       export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#
#------------------------------------------------------------------------------
# CentOs configure script command line example:
#------------------------------------------------------------------------------
#
# The following commands install MB-System on CentOs 6 or 7
#
# Install the prerequisites using yum:
sudo yum install openmotif openmotif-devel fftw fftw-devel netcdf netcdf-devel \
		proj proj-devel gdal-devel gmt gmt-devel gv
#
# These steps assume you have downloaded an MB-System distribution tar.gz file
# from the ftp site:
#       ftp://mbsystemftp@ftp.mbari.org
# The current distribution file is mbsystem-5.5.2307.tar.gz
#
# First, unpack the distribution using tar. This can be done in any location,
# but if one is working in a root-owned area such as /usr/local/src, obtaining
# root privileges using sudo is necessary for all steps:
#       sudo tar xvzf mbsystem-5.5.2307.tar.gz
# and then cd into the resulting directory:
#       cd mbsystem-5.5.2307
#
# If the prerequisites have all been installed with yum and it is desired to
# install MB-System in /usr/local, then only a simple call to configure is required:
sudo ./configure
#
#------------------------------------------------------------------------------
# Completing the build and dealing with user environment issues
#------------------------------------------------------------------------------
#
# On all systems there are user enviroment setting that are required to use some
# aspects of MB-System. First, three MB-System tools (mbswath, mbcontour, and
# mbgrdtiff) are actually GMT modules built into a shared library named
# "mbsystem.so". GMT only knows about these external modules through a user's
# configuration, which is defined by a file called "gmt.conf" that is in
# the user's home directory. To create this file, go to the home directory
# and direct the output of the gmtdefaults module to a file named "gmt.conf":
        cd ~
        gmt gmtdefaults > gmt.conf
#
# Next, use a text editor of your choice to edit the line of gmt.conf that
# sets the "GMT_CUSTOM_LIBS" parameter so that this parameter is the full
# path to the "mbsystem.so" shared library. If you have installed MB-System
# in /usr/local, then the path should be "/usr/local/lib/mbsystem.so".
#
# The MB-System plotting macros mbm_grdplot, mbm_plot, etc all generate
# shellscripts that in turn execute a combination of GMT, MB-System, and other
# programs to generate and then display postscript plots or images.
# The MB-System program mbdefaults sets the programs you want to use to display
# postscript files and images on the screen. If, for instance, you want to
# use "gv" to display postscript and "feh" to display images, then run:
        mbdefaults -Dgv -Ifeh -V
# These defaults are stored in a hidden file called ".mbio_defaults" in the user's
# home directory.

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

# Reconstruct the build system, including the Makefile.in files and the configure script
glibtoolize --force --copy
aclocal
autoheader
automake --add-missing --include-deps
autoconf
autoupdate
autoreconf --force --install

# Run the configure script - here all possible optional targets are enabled
# include the experimental and prototype tools (mbtrn, mbtnav, qt tools, opencv tools)
LDFLAGS="-L/opt/X11/lib" \
CFLAGS="-g -I/opt/X11/include" \
./configure \
    --prefix=/usr/local \
    --with-proj-include=/usr/local/include \
    --with-proj-lib=/usr/local/lib \
    --with-fftw-include=/usr/local/include \
    --with-fftw-lib=/usr/local/lib \
    --with-motif-include=/usr/local/include \
    --with-motif-lib=/usr/local/lib \
    --with-opengl-include=/opt/X11/include \
    --with-opengl-lib=/opt/X11/lib \
    --with-otps-dir=/usr/local/opt/otps \
    --enable-hardening \
    --enable-test \
    --enable-mbtrn \
    --enable-mbtnav \
    --enable-opencv \
    --with-opencv-include=/usr/local/include/opencv4 \
    --with-opencv-lib=/usr/local/lib
#    --enable-qt \
#    --enable-pcltools

make
make check
make install

cd src/htmlsrc ; ./make_mbhtml ; cd ../..

make -j install

#
#------------------------------------------------------------------------------
