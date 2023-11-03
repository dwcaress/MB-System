#-------------------------------------------------------------------------
How to Download and Install MB-System
#------------------------------------------------------------------------------

David W. Caress
November 2, 2023

#------------------------------------------------------------------------------
General Advice for Building and Installing MB-System
#------------------------------------------------------------------------------
#
MB-System is an open source software package intended for Unix-like operating systems, including a build system based on the GNU Autotools incorporating Autoconf, Automake, Libtool, and Make. In order to build and install MB-System, one first runs a script named "configure" included at the top level directory of the MB-System source code that generates files named "Makefile" throughout the source code structure. In turn, these Make files determine how the libraries and programs are compiled, linked, and installed using the Make utility. 

MB-System depends on a number of other software packages. For some operating systems, special arguments to the configure script are required to integrate the MB-System source to prerequisite software packages. 

Among the software packages that are prerequisite for MB-System are:
  GMT (Generic Mapping Tools)
  Proj
  GDAL
  netCDF
  FFTW (Fastest Fourier Transform in the West)
  X11
  OpenMotif
  OpenGL
  OpenCV
  PCL (Point Cloud Library) (planned, not yet in release 5.7.9)

The sections below provide some instructions for building MB-System on a few 
common operating system distributions, including MacOs, Ubuntu Linux, 
Debian Linux, and the CygWin environment on Windows. These instructions 
include the installation of the prerequisite software packages using a 
package manager relevant to each OS and the special arguments needed for 
the configure script.

#------------------------------------------------------------------------------
How to download an MB-System source distribution
#------------------------------------------------------------------------------

The source code for MB-System is available from a repository on Github:
    https://github.com/dwcaress/MB-System

This link will open the page for the most recent stable release of MB-System. The source code distribution can be downloaded as zip or tar.gz archives from the "Assets" section at the bottom.
    https://github.com/dwcaress/MB-System/releases/latest

This link will open a page listing all recent MB-System releases, with the most recent release at the top. The most recent release could be a beta or pre-release. The source code distribution can be downloaded as zip or tar.gz archives from the "Assets" section at the bottom.
    https://github.com/dwcaress/MB-System/releases/releases
This link will download the current state of the master branch of the MB-System repository:
    https://github.com/dwcaress/MB-System/archive/refs/heads/master.zip


#------------------------------------------------------------------------------
MacOS 13 Ventura (and MacOs 10 High Sierra, 11 Big Sur, and 12 Monterey)
#------------------------------------------------------------------------------

The use of MacPorts to install the MB-System prerequisite packages is recommended on Apple Mac computers, particularly because this approach enables cleanly building with a complete X11 + Motif + OpenGL infrastructure separate from the libraries and header files associated with the XQuartz package. One usually still installs XQuartz and uses it as the X11 display server, but confining the MB-System compilation and linking to headers and libraries within the MacPorts structure avoids several issues.

This example is relevant for MacOs 10.13 High Sierra to the current MacOs 13 Ventura on both Intel and ARM (Apple Silicon) architecture computers.

Install Xcode and the Xcode command line tools, which includes the LLVM compiler suite.

Install the XQuartz X11 server from
  https://www.xquartz.org
XQuartz 2.8.5 or later is required for all MB-System installations.

Install MacPorts using the appropriate downloadable installer package from:
  https://www.macports.org/install.php

After MacPorts installation, first make sure the default port packages are current by running selfupdate and then install the MB-System prerequisites.
  sudo port -v selfupdate
  sudo port install gmt6 fftw-3 mesa libGLU openmotif

Also make sure that a current version of Python3 is available. First list the available Python3 versions, install the most recent, and then set port to link that version to python3:
  port select --list python

Result as of July 18, 2023:
  Available versions for python:
   none
   python27
   python30
   python311 (active)
   python38

The most recent version is python311, so install it:
  sudo port install python311
  sudo port select --set python python311
  sudo port select --set python3 python311

Also install the most recent Python imaging library Pillow
  sudo port install py311-Pillow

Download the MB-System source package from the repository at GitHub:
  https://github.com/dwcaress/MB-System
There are often beta releases that are more recent than the current stable release. For instance, to download 5.7.9.beta42 go to:
    https://github.com/dwcaress/MB-System/archive/refs/tags/5.7.9beta58.tar.gz

Recommend using Chrome for the above, not Safari, and placing the tarball in /usr/local/src.

Unpack the MB-System distribution tarball, and then cd into the top directory of the resulting structure. This will typically be named something like "MB-System-5.7.9". At that location, execute the configure script, named "configure", with the options necessary for your context. The XCode compiler tools do not look for header files or libraries in the locations used by MacPorts, and so it is necessary to specify these locations for several of the prerequisite packages.
 
This command should successfully enable building the current MB-System (5.7.9 or later) on any Mac computer with the prerequisites installed through MacPorts. This has been tested with computers running Ventura and Monterey.
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
      --with-otps-dir=/usr/local/src/otps

Once the makefiles have been generated by configure, build and install MB-System using:
  make
  make check
  sudo make install

The MB-System codebase includes some optional components, such as OpenCV based photomosaicing (enabled with --enable-opencv) and  a realtime Terrain Relative Navigation infrastructure and toolset (--enable-mtrn and --enable-mbtnav). An additional prerequisite for the photomosaicing is OpenCV4, which can be installed by:
  sudo port install gmt6 fftw-3 libGLU openmotif opencv4
This configure command should enable building the entire MB-System package, including these optional tools.
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

#------------------------------------------------------------------------------
Ubuntu Jammy Jellyfish 22.04
#------------------------------------------------------------------------------

Install Ubuntu 20 from ISO, and then update the starting packages:
    sudo apt upgrade

Install compilers:
sudo apt install build-essential

Install MB-System prerequisites:
    sudo apt install libnetcdf-bin libnetcdf-dev libgdal-dev \
gmt libgmt6 libgmt-dev libproj-dev \
libfftw3-3 libfftw3-dev libmotif-dev \
xfonts-100dpi libglu1-mesa-dev \
libopencv-dev gfortran

Run configure to build all of MB-System:
    ./configure \
        --enable-mbtrn --enable-mbtnav --enable-opencv \
        --with-opencv-include=/usr/include/opencv4 \
        --with-opencv-lib=/lib/x86_64-linux-gnu

The MB-System libraries will be placed in /usr/local/lib, but the
runtime dynamic linker does not look for shared libraries in this
directory by default. Users can add /usr/local/lib to the directories
searched for shared libraries by adding "/usr/local/lib" to the
environment variable LD_LIBRARY_PATH. This is accomplished by 
placing the command 

   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

in in an environment file in the user’s home 
directory named .zprofile (if using the zsh shell) or .profile 
(if using the bash shell). Other user environment files can be used, 
such as .zshrc for zsh or .bashrc for bash.

The preferred alternative to using LD_LIBRARY_PATH is to embed the 
shared library location in the compiled executables
by passing an rpath command to the linker by setting the LD_FLAGS
environment variable available to the configure command:
 
    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
        ./configure \
        --enable-mbtrn --enable-mbtnav --enable-opencv \
        --with-opencv-include=/usr/include/opencv4 \
        --with-opencv-lib=/lib/x86_64-linux-gnu

Build MB-System:
    make
    make check
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so


#------------------------------------------------------------------------------
Ubuntu Focal Fossa 20.04
#------------------------------------------------------------------------------

Install Ubuntu 22 from ISO, and then update the starting packages:
    sudo apt upgrade

Install compilers:
sudo apt install build-essential

Install MB-System prerequisites:
    sudo apt install libgdal-dev \
        gmt libgmt6 libgmt-dev libproj-dev \
        libfftw3-3 libfftw3-dev libmotif-dev \
        xfonts-100dpi libglu1-mesa-dev libopencv-dev \
        gfortran

Run configure to build all of MB-System:
    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
        ./configure \
        --enable-mbtrn --enable-mbtnav --enable-opencv \
        --with-opencv-include=/usr/include/opencv4 \
        --with-opencv-lib=/lib/x86_64-linux-gnu

Build MB-System:
    make
    make check
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so


#------------------------------------------------------------------------------
Ubuntu Bionic Beaver 18.04
#------------------------------------------------------------------------------

Note that the photomosaicing tools utilising OpenCV cannot be built under Ubuntu 18.

Install Ubuntu 22 from ISO, and then update the starting packages:
    sudo apt upgrade

Install compilers:
sudo apt install build-essential

Install MB-System prerequisites:
    sudo apt install libgdal-dev \
        gmt libgmt5 libgmt-dev gmt-common proj-bin proj-data libproj-dev \
        libfftw3-3 libfftw3-dev libmotif-dev \
        xfonts-100dpi libglu1-mesa-dev \
        gfortran

Run configure to build core MB-System plus the Terrain Relative Navigation tools:
    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
        ./configure \
        --enable-mbtrn --enable-mbtnav

Build MB-System:
    make
    make check
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so


#------------------------------------------------------------------------------
Debian 12
#------------------------------------------------------------------------------

Install Debian 12 from ISO, and then update the starting packages:
    sudo apt upgrade

Install compilers:
sudo apt install build-essential

Install MB-System prerequisites:
    sudo apt install netcdf-bin libnetcdf-dev libgdal-dev \
gmt libgmt6 libgmt-dev libproj-dev \
libfftw3-3 libfftw3-dev libmotif-dev \
xfonts-100dpi libglu1-mesa-dev \
libopencv-dev gfortran

Run configure to build all of MB-System:
    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
       ./configure \
       --enable-mbtrn --enable-mbtnav --enable-opencv \
       --with-opencv-include=/usr/include/opencv4 \
       --with-opencv-lib=/lib/x86_64-linux-gnu

Build MB-System:
    make
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so


#------------------------------------------------------------------------------
CentOs 7 and 8
#------------------------------------------------------------------------------

Note that the photomosaicing tools utilising OpenCV cannot be built under CentOs 7.

Install CentOs 7 or 8 from ISO, and then update the starting packages:
    sudo yum upgrade

Install EPEL repo:
    sudo yum install epel-release

Install MB-System prerequisites:
    sudo yum install openmotif openmotif-devel \
        fftw fftw-devel netcdf netcdf-devel \
        proj proj-devel gdal-devel gmt gmt-devel gv \
        mesa-libGL mesa-libGL-devel mesa-libGLU mesa-libGLU-devel 

Run configure to build the core MB-System plus the Terrain Relative Navigation tools:
    ./configure \
        --enable-mbtrn --enable-mbtnav

Build MB-System:
    make
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so


#------------------------------------------------------------------------------
CygWin
#------------------------------------------------------------------------------

CygWin is a collection of software tools that augment Windows computers with a Unix-style environment within which one can build, install, and run Unix-y packages like MB-System.

If Cygwin is installed, then one must install a number of prerequisite packages before building MB-System. These include:

    gcc, g++, rpc-devel, gambas3-devel, libproj-devel, libproj12, libnetcdf-devel, libnetcdf, libgdal-devl,libgdal19, libfftw3-devel, libfftw3, cmake, make, fftw, fftw-devel, ghostscript, gv, libcurl-devel,  libnpcr0, libnpcr0-devel (libpcre, pcre-devel), openssh, subversion, xinit, zlib, zlib-devel, liblapack-devel

Run configure to build MB-System without any graphical tools::
    ./configure --enable-mbtrn --enable-mbtnav  --disable-dependency-tracking --disable-mbtools
    
Build MB-System:
    make
    sudo make install

Post-Installation Actions:
    cpan Parallel::ForkManager
    gmt gmtset IO_NC4_CHUNK_SIZE classic
    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so

#------------------------------------------------------------------------------
Docker Container with MB-System
#------------------------------------------------------------------------------

An updated MB-System Docker Image is generated each time that a new release is created in the MB-System Github repository. This Docker is based on CentOs 7, and can be run on MacOs, Linux, and Windows computers. Data present on the host computer's filesystems can be processed using the MB-System programs in the Docker container.

The MB-System docker image is available at 
    https://hub.docker.com/r/mbari/mbsystem

Documentation is available at:
    https://github.com/dwcaress/MB-System/tree/master/docker/user
    https://github.com/dwcaress/MB-System/blob/master/docker/user/README-win11.md


#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
To modify the build system...
#------------------------------------------------------------------------------
#
Edit the file "configure.ac" in the top directory and "Makefile.am" in each
directory and then run the following sequence of commands:

Build libtool files for AM_PROG_LIBTOOL
    libtoolize --force --copy
    aclocal

Build custom header for configure
    autoheader
    automake --add-missing --include-deps
    autoconf

To update configure files use the following:
    autoupdate
    autoreconf --force --install --warnings=all

Reset the autotools version to 2.65 to accomodate some Linux distributions
    sed -i.bak s/2\.69/2\.65/ configure.ac

When you run ./configure, a number of configure options are saved to a
header file:
    ./src/mbio/mb_config.h
This file has a template:
    ./src/mbio/mb_config.h.in
This file is conditionally included by:
    ./src/mbio/mb_define.h
which is in turn included by essentially every MB-System C source file.
#
#------------------------------------------------------------------------------
#
Full autoconf and build sequence after modifying the build system
  - Do this in the development tree prior to a commit to the source archive or
    prior to making a source distribution
  - The example here is for a MacOs environment in which the prerequisite
    packages have been installed with MacPorts.
#
First clean up old installation and build
    make -j uninstall
    make -j clean

Reconstruct the build system, including the Makefile.in files and the configure script
    glibtoolize --force --copy
    aclocal
    autoheader
    automake --add-missing --include-deps
    autoconf
    autoupdate
    autoreconf --force --install

Run the configure script - here the prerequisites have been installed with
MacPorts, and the experimental OpenCV based photomosaicing and Terrain relative
Navigation are all enabled
    CFLAGS="-g -Wall -Wextra" CPPFLAGS="-g" ./configure \
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
