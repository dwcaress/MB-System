#-------------------------------------------------------------------------
# Notes on using the MB-System autotools based build system
#------------------------------------------------------------------------------
#
# David W. Caress
# May 12, 2022
#------------------------------------------------------------------------------
#
# This Autoconf build system was begun by Bob Covill in 2011, and then completed
# with a distributed, multi-continental effort by Bob Covill, Christian
# Ferreira, Hamish Bowman, Kurt Schwehr, and David Caress in May to August
# of 2013.
#
#------------------------------------------------------------------------------
# To use the build system...
#------------------------------------------------------------------------------
#
# Obtain the MB-System source tree by either downloading and unpacking an
# MB-System source distribution tarball (e.g. mbsystem-5.7.9.tar.gz) from
# the Github repository:
#   https://github.com/dwcaress/MB-System/releases
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
make check (build and execute a limited set of unit tests)
make clean (to delete compiled object files in the source code tree)
make uninstall (to fully uninstall the installed libraries, headers, and programs)
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
#   MB-System configure options:
#------------------------
#    --prefix=install      - location for mbsystem (/usr/local/mbsystem) (optional)"])
#    --enable-hardening    - Enable compiler and linker options to frustrate memory corruption exploits  (e.g. -fPIE and -pie) (optional)"])
#    --enable-test         - Enable building unit tests in test/ and third-party/"])
#    --with-netcdf-config  - location of NetCDF config script nc-config (optional)"])
#    --with-gdal-config    - location of GDAL config script gdal-config (optional)"])
#    --with-gmt-config     - location of GMT config script gmt-config (optional)"])
#    --with-proj-lib       - location of PROJ libs (optional)"])
#    --with-proj-include   - location of PROJ headers (optional)"])
#    --with-fftw-lib       - location of FFTW3 libs (optional)"])
#    --with-fftw-include   - location of FFTW3 headers (optional)"])
#    --with-motif-lib      - location of Motif libs (optional)"])
#    --with-motif-include  - location of Motif headers (optional)"])
#    --with-opengl-lib     - location of OpenGL libs (optional)"])
#    --with-opengl-include - location of OpenGL headers (optional)"])
#    --with-otps-dir       - location of OTPS installation (optional)"])
#    --with-opencv-lib     - location of OpenCV libs (optional)"])
#    --with-opencv-include - location of OpenCV headers (optional)"])
#    --enable-opencv       - enable building tools using OpenCV (optional)"])
#    --enable-mbtrn        - enable building terrain relative navigation (TRN) tools (optional)"])
#    --enable-mbtnav       - enable building terrain relative navigation (TRN) tools (optional)"])
#    --disable-mbtools     - disable building graphical tools (use with --enable-mbtrn and --enable-mbtnav)"])
#    --enable-qt           - Enable building graphical tools using the Qt5 framework"])
#    --with-vtk-include    - location of VTK8.2+ headers (required if qt enabled)"])
#    --with-vtk-lib        - location of VTK8.2+ libraries (required if qt enabled)"])
#    --with-debug          - Set compiler flags to allow full debugging"])
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
# MacOs Example Using MacPorts to Install Prerequisite packages:
#------------------------------------------------------------------------------
#
# The use of MacPorts to install the MB-System prerequisite packages is
# recommended on Apple Mac computers, particularly because this approach enables
# cleanly building with a complete X11 + Motif + OpenGL infrastructure separate
# from the libraries and header files associated with the XQuartz package.
# One usually still installs XQuartz and uses it as the X11 display server, but
# confining the MB-System compiliation and linking to headers and libraries
# within the MacPorts structure avoids several issues.
#
# This example is relevant for MacOs 10.13 High Sierra to the current MacOs 12
# Monterey on both Intel and ARM (Apple Silicon) architecture computers.
#
# Install Xcode and the Xcode command line tools, which includes the LLVM
# compiler suite.
#
# Install the XQuartz X11 server from
#   https://www.xquartz.org
# XQuartz 2.8.2 or later is required for all MB-System installations.
#
# Install MacPorts using the appropriate downloadable installer package from:
#   https://www.macports.org/install.php
#
# After MacPorts installation, first make sure the default port packages are
# current by running selfupdate and then install the MB-System prerequisites.
  sudo port -v selfupdate
  sudo port install gmt6 proj6 fftw-3 xorg mesa libGLU openmotif
#
# Also make sure that a current version of Python3 is available. First list the
# available Python3 versions, install the most recent, and then set port to link
# that version to python3:
  port select --list python
  sudo port install python310
  sudo port select --set python3 python310
# Also install the most recent Python imaging library Pillow
  sudo port install py310-Pillow
#
# Download the MB-System source package from the repository at GitHub:
#   https://github.com/dwcaress/MB-System
# There are often beta releases that are more recent than the current stable
# release. For instance, to download 5.7.9.beta42 go to:
    https://github.com/dwcaress/MB-System/archive/refs/tags/5.7.9beta42.tar.gz
#
# Unpack the MB-System distribution tarball, and then cd into the top directory
# of the resulting structure. This will typically be named something like
# "MB-System-5.7.9". At that location, execute the configure script, named
# "configure", with the options necessary for your context. The XCode compiler
# tools do not look for header files or libraries in the locations used by
# MacPorts, and so it is necessary to specify these locations for several of
the prerequisite packages.
#
# This command should successfully enable building the current MB-System
# (5.7.9 or later) on any Mac computer with the prerequisites installed through
# MacPorts. This has been tested with computers running High Sierra, Big Sur,
# and Monterey.
  ./configure \
      --prefix=/usr/local \
      --disable-static \
      --enable-shared \
      --enable-hardening \
      --enable-test \
      --with-proj-lib=/opt/local/lib/proj8/lib \
      --with-proj-include=/opt/local/lib/proj8/include \
      --with-gmt-config=/opt/local/lib/gmt6/bin \
      --with-fftw-lib=/opt/local/lib \
      --with-fftw-include=/opt/local/include \
      --with-x11-lib=/opt/local/lib \
      --with-x11-include=/opt/local/include \
      --with-motif-lib=/opt/local/lib \
      --with-motif-include=/opt/local/include \
      --with-opengl-include=/opt/local/include \
      --with-opengl-lib=/opt/local/lib \
      --with-otps-dir=/usr/local/src/otps
#
# Once the makefiles have been generated by configure, build and install
# MB-System using:
  make
  make check
  sudo make install
#
# The MB-System codebase includes some experimental components, such as OpenCV
# based photomosaicing (enabled with --enable-opencv) and  a realtime Terrain
# Relative Navigation infrastructure and toolset (--enable-mtrn and --enable-mbtnav).
# An additional prerequisite for the photomosaicing is OpenCV, which can be
# installed by:
  sudo port install gmt6 proj6 fftw-3 xorg mesa libGLU openmotif opencv4
# This configure command should enable building the entire MB-System
# package, including these experimental tools.
  ./configure \
      --prefix=/usr/local \
      --disable-static \
      --enable-shared \
      --enable-hardening \
      --enable-test \
      --with-proj-lib=/opt/local/lib/proj6/lib \
      --with-proj-include=/opt/local/lib/proj6/include \
      --with-gmt-config=/opt/local/lib/gmt6/bin \
      --with-fftw-lib=/opt/local/lib \
      --with-fftw-include=/opt/local/include \
      --with-x11-lib=/opt/local/lib \
      --with-x11-include=/opt/local/include \
      --with-motif-lib=/opt/local/lib \
      --with-motif-include=/opt/local/include \
      --with-opengl-include=/opt/local/include \
      --with-opengl-lib=/opt/local/lib \
      --enable-mbtrn \
      --enable-mbtnav \
      --enable-opencv \
      --with-opencv-include=/opt/local/include/opencv4 \
      --with-opencv-lib=/opt/local/lib/opencv4 \
      --with-otps-dir=/usr/local/src/otps
#
#------------------------------------------------------------------------------
# MacOs Example Using Homebrew to Install Prerequisite packages:
#------------------------------------------------------------------------------
#
# The use of MacPorts to install the MB-System prerequisite packages is
# recommended, but for Intel architecture computers and pre-Monterey OS versions
# the use of Homebrew is still a viable option.
#
# This example is relevant for MacOs Big Sur (11.6) on both Intel and ARM
# architecture computers when the prerequisite software packages have been
# installed using the Homebrew package manager. These commands will also
# succeed in building MB-System on Macs running MacOs Monterey (11.7), but the
# graphical programs will not execute properly on ARM Macs.
#
# Install Xcode and the Xcode command line tools, which includes the LLVM
# compiler suite.
#
# Install the XQuartz X11 server from
#   https://www.xquartz.org (2.8 or later required for Monterey OS and/or ARM architecture)
#
# Install the Homebrew package manager from https://brew.sh and then use brew
# to install all of the the MB-System prerequisite packages:
#   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
#   brew upgrade
#   brew install proj gdal netcdf fftw gmt openmotif
#
# Unpack the MB-System distribution tarball, and then cd into the top directory
# of the resulting structure. This will typically be named something like
# MB-System-5.7.9/. At that location, execute the configure script, named "configure",
# with the options necessary for your context. The XCode compiler tools do not
# look for header files or libraries in the locations used by Homebrew, and so it
# is necessary to specify these locations for several of the prerequisite packages.
# On Intel architecture Macs, Homebrew installs packages into /usr/local/, while
# on ARM architecture (Apple Silicon) Macs, Homebrew installs packages into
# /opt/local/
#
# In Intel Macs running Big Sur, the following command should succeed:
./configure \
    --prefix=/usr/local \
    --disable-static \
    --enable-shared \
    --enable-hardening \
    --enable-test \
    --with-x11-lib=/opt/X11/lib \
    --with-x11-include=/opt/X11/include \
    --with-motif-lib=/usr/local/opt/openmotif/lib \
    --with-motif-include=/usr/local/opt/openmotif/include \
    --with-opengl-include=/opt/X11/include \
    --with-opengl-lib=/opt/X11/lib \
    --with-otps-dir=/usr/local/src/otps
#
# In ARM Macs running Big Sur, the following command should succeed:
./configure \
    --prefix=/usr/local \
    --disable-static \
    --enable-shared \
    --enable-hardening \
    --enable-test \
    --with-x11-lib=/opt/X11/lib \
    --with-x11-include=/opt/X11/include \
    --with-opengl-lib=/opt/X11/lib \
    --with-opengl-include=/opt/X11/include \
    --with-motif-lib=/opt/homebrew/lib \
    --with-motif-include=/opt/homebrew/include \
    --with-otps-dir=/usr/local/src/otps
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
# The following procedure serves to install MB-System on Ubuntu Focal 20.04.
#
# GMT:
# ----
# GMT is a key prerequisite for MB-System, which now requires GMT 6.1 or later.
# One can check which GMT version is available by querying the package manager in
# a terminal:
        apt-cache show gmt
# For Ubuntu 20.04 the available GMT version is 6.0.0. Therefore, it is necessary
# to install GMT 6.1+ from a personal package archive (PPA) or by compiling,
# linking, and installing the GMT source distribution manually.
#
# One can enable installation from the UbuntuGIS PPA by entering the following
# commands:
        sudo add-apt-repository ppa:ubuntugis/ppa
        sudo apt-get update
        sudo apt-get upgrade
#
# Once GMT-6.1.0 or later is available to you, install the primary package and
# the documentation, library files, development headers, and hierarchical high
# resolution geography:
        sudo apt-get install gmt libgmt5 libgmt-dev gmt-gshhg gmt-doc
#
# If you need (or want) to build GMT from source, follow the installation
# instructions at:
#        https://www.generic-mapping-tools.org
#
# Motif:
# ------
# MB-System depends on X11, OpenMotif, and OpenGL. These can be installed using:
        sudo apt-get install libx11-dev xorg-dev libmotif-dev libmotif4 \
            libxp-dev mesa-common-dev libsdl1.2-dev libsdl-image1.2-dev
#
# Other dependencies:
# -------------------
# The many other MB-System dependencies can be installed via:
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
# from the GitHub repository:
#       https://github.com/dwcaress/MB-System/releases
#
# First, unpack the distribution using tar. This can be done in any location,
# but if one is working in a root-owned area such as /usr/local/src, obtaining
# root privileges using sudo is necessary for all steps:
#       sudo tar xvzf MB-System-5.7.9.tar.gz
# and then cd into the resulting directory:
#       cd MB-System-5.7.9.tar.gz
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
#
#------------------------------------------------------------------------------
# Completing the build and dealing with user environment issues
#------------------------------------------------------------------------------
#
# On all systems there are user environment setting that are required to use some
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
#   - The example here is for a MacOs environment in which the prerequisite
#     packages have been installed with MacPorts.
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

# Run the configure script - here the prerequisites have been installed with
# MacPorts, and the experimental OpenCV based photomosaicing and Terrain relative
# Navigation are all enabled
./configure \
  --prefix=/usr/local \
  --disable-static \
  --enable-shared \
  --enable-hardening \
  --enable-test \
  --with-proj-lib=/opt/local/lib/proj9/lib \
  --with-proj-include=/opt/local/lib/proj9/include \
  --with-gmt-config=/opt/local/lib/gmt6/bin \
  --with-fftw-lib=/opt/local/lib \
  --with-fftw-include=/opt/local/include \
  --with-x11-lib=/opt/local/lib \
  --with-x11-include=/opt/local/include \
  --with-motif-lib=/opt/local/lib \
  --with-motif-include=/opt/local/include \
  --with-opengl-include=/opt/local/include \
  --with-opengl-lib=/opt/local/lib \
  --enable-mbtrn \
  --enable-mbtnav \
  --enable-opencv \
  --with-opencv-include=/opt/local/include/opencv4 \
  --with-opencv-lib=/opt/local/lib/opencv4 \
  --with-otps-dir=/usr/local/src/otps

make
make check
make install

cd src/htmlsrc ; ./make_mbhtml ; cd ../..

make -j install

#
#------------------------------------------------------------------------------
