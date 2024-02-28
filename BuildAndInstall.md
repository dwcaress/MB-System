# How to Download, Build, and Install MB-System

---

David W. Caress  
November 15, 2023

---
### General Advice for Building and Installing MB-System
---

MB-System is an open source software package intended for Unix-like operating systems which 
is distributed as source code. In order to install MB-System, a source distribution must be 
downloaded, unpacked, compiled, linked, and installed locally. MB-System now includes two 
methods for building and installing the software, the first based on GNU Autotools and the 
second using the CMake package.

The GNU Autotools build system includes a script named **configure** at the top of the 
distribution structure which, when executed, generates a set of files named "Makefile" 
throughout the structure that hold compilation and linkage instructions used by the program 
**make**. These compilation and linkage instructions vary depending on the operating system 
and which versions of prerequisite software packages are installed, and in some cases 
special arguments must be included in the configure command. Once configure has been 
successfully executed, running the program **make** at the top level then actually invokes 
the local compiler and linker to build and install MB-System. Several Autotools programs, 
including autoconf, automake, and libtool, are used to generate the configure script 
included in each distribution. This build system has been the sole means by which MB-System 
could be built since 2011.

The sequence of commands by which one builds and installs a package using CMake is very 
different from Autotools. One creates a new directory to hold the package build (commonly 
named "build" by convention) in the top level of the source structure, then cd's into that 
directory, and then executes **cmake** with the argument "..", which provides **cmake** 
the path to the top level of the source directory. **Cmake** then copies the source into a 
build tree and generates Makefiles in that tree to enable compilation, linkage, and 
installation, again using the program **make**.

The CMake build system is new to MB-System with version 5.7.9, and is intended to replace 
the older Autotools approach for current and future operating systems. The old Autotools 
build system will continue to be included in MB-System distributions for the forseeable 
future to enable building and installation on legacy operating systems. The Autotools build 
system will not be maintained to enable it's use on current or future operating systems, 
and the CMake build system will not be modified to support building on legacy systems.

MB-System depends on a number of other software packages. For some operating systems, 
special arguments to the configure script are required to integrate the MB-System source 
to prerequisite software packages. 

Among the software packages that are prerequisite for MB-System are:  

* GMT (Generic Mapping Tools)   
* Proj  
* GDAL  
* netCDF  
* FFTW (Fastest Fourier Transform in the West)  
* X11  
* OpenMotif  
* OpenGL  
* OpenCV  
* PCL (Point Cloud Library) (planned, not yet in release 5.7.9)  

The sections below provide some instructions for building MB-System on a few 
common operating system distributions, including MacOs, Ubuntu Linux, 
Debian Linux, CentOs Linux, and the CygWin environment on Windows. These instructions 
include the installation of the prerequisite software packages using a 
package manager relevant to each OS and any special arguments needed for 
the cmake command or the configure script.

---
### How to download an MB-System source distribution
---

The source code for MB-System is available from a repository on Github:  
<https://github.com/dwcaress/MB-System>  

This link will open the page for the most recent stable release of MB-System. The source 
code distribution can be downloaded as zip or tar.gz archives from the "Assets" section at 
the bottom.  
    <https://github.com/dwcaress/MB-System/releases/latest>

This link will open a page listing all recent MB-System releases, with the most recent 
release at the top. The most recent release could be a beta or pre-release. The source 
code distribution can be downloaded as zip or tar.gz archives from the "Assets" section 
at the bottom.  
    <https://github.com/dwcaress/MB-System/releases/releases>

This link will download the current state of the master branch of the MB-System repository:  
    <https://github.com/dwcaress/MB-System/archive/refs/heads/master.zip>


---
### MacOS 13 Ventura (and MacOs 10 High Sierra, 11 Big Sur, and 12 Monterey)
---

The use of MacPorts to install the MB-System prerequisite packages is recommended on Apple 
Mac computers, particularly because this approach enables cleanly building with a complete 
X11 + Motif + OpenGL infrastructure separate from the libraries and header files associated 
with the XQuartz package. One usually still installs XQuartz and uses it as the X11 display 
server, but confining the MB-System compilation and linking to headers and libraries within 
the MacPorts structure avoids several issues. At present, we are not able to successfully 
run the MB-System graphical utilities when the prerequisite packages have been installed 
using the Fink or Homebrew package managers.

To state it again succinctly, if you want to build and install MB-System in MacOs, use 
MacPorts, and **do not** use Fink or HomeBrew.

This example is relevant for MacOs 10.13 High Sierra to the current MacOs 13 Ventura on 
both Intel and ARM (Apple Silicon) architecture computers.

* From the Apple App Store, install Xcode developer tools, which includes the LLVM compiler suite.

* Install the XQuartz X11 server from  
  <https://www.xquartz.org>  
  XQuartz 2.8.5 or later is required for all MB-System installations.

* Install MacPorts using the appropriate downloadable installer package from:  
  <https://www.macports.org/install.php>

* After MacPorts installation, first make sure the default port packages are 
  current by running selfupdate and then install the MB-System prerequisites.  

        sudo port -v selfupdate    
        sudo port install gmt6 fftw-3 mesa libGLU openmotif opencv4  

* Also make sure that a current version of Python3 is available. First list  
  the available Python3 versions, install the most recent, and then set port  
  to link that version to python3:  
  
        port select --list python
           
  Versions of Python available through MacPorts (November 6, 2023):
	* python27
	* python310
	* python311
	* python38
            
  The most recent version is python311, so install it and make it the active installation:
  
        sudo port install python311  
        sudo port select --set python python311  
        sudo port select --set python3 python311  
       
  Also install the most recent Python imaging library Pillow:  
  
        sudo port install py311-Pillow

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  We recommend using Chrome for the download rather than Safari,
  because Chrome does not prematurely unpack the tarball.
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations):

        mv MB-System-5.7.9.tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Option 1: **CMake Build System** (recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), create a working directory named 
  "build", **cd** into "build", and then execute **cmake** at the location
  /usr/local/src/MB-System-5.7.9/build/. This command should successfully enable building the entire 
  current MB-System (5.7.9 or later) on any Mac computer with the prerequisites installed 
  through MacPorts. This has been tested with computers running Ventura.
  
	    mkdir build
	    cd build
	    cmake ..
  
* Option 2: **Autotools Build System** (not recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, 
  named **configure**, with the options 
  necessary for your context. The XCode compiler tools do not look for header files or 
  libraries in the locations used by MacPorts, and so it is  necessary to specify these 
  locations for several of the prerequisite packages. This command should successfully 
  enable building the current core MB-System (5.7.9 or later) on any Mac computer with 
  the prerequisites installed through MacPorts. This has been tested with computers 
  running Ventura and Monterey.
  
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
  
  The MB-System codebase includes some components that are optional when using 
  the Autotools build system, such as OpenCV based 
  photomosaicing (enabled with --enable-opencv) and  a realtime Terrain Relative 
  Navigation infrastructure and toolset (--enable-mtrn and --enable-mbtnav). 
  This configure command should enable building the entire MB-System package, 
  including these optional tools:
	
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

* Once the makefiles have been generated by cmake or configure, build and install 
  MB-System using:

	    make
	    sudo make install

---
### Ubuntu Jammy Jellyfish 22.04 and Focal Fossa 20.04
---

This example is relevant for both Ubuntu 20.04 and 22.04 because the prerequisite
package names are the same.

Ubuntu and Debian Linux distributions include multiple tools for managing software
distributions. We recommend using the command line program apt to install the prerequisite 
software for MB-System.

* Install Ubuntu 20 or 22 from an ISO image of the release (https://ubuntu.com/download/desktop).

* Update the starting packages:

	    sudo apt upgrade

* Install compilers and other essential software:

	    sudo apt install build-essential

* Install MB-System prerequisites:

	    sudo apt install netcdf-bin libnetcdf-dev libgdal-dev \
	      gmt libgmt6 libgmt-dev libproj-dev \
	      libfftw3-3 libfftw3-dev libmotif-dev \
	      xfonts-100dpi libglu1-mesa-dev \
	      libopencv-dev cmake gfortran

  MB-System also requires Python3 and the Pillow library for Python3. Both of these packages
  are installed by default in Ubuntu 20 and 22.

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  We recommend using Chrome for the download rather than Safari,
  because Chrome does not prematurely unpack the tarball.
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src/ (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations). 
  If you do not have write privilege in /usr/local/src/ then you will need to execute the
  following commands using sudo:

        mv MB-System-5.7.9-tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Option 1: **CMake Build System** (recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), create a working directory named 
  "build", **cd** into "build", and then  execute **cmake** at the location 
  /usr/local/src/MB-System-5.7.9/build/). This command should successfully enable building 
  the entire current MB-System (5.7.9 or later) on either Ubuntu 20 or 22 as long as the 
  prerequisite  software packages have been installed.
  
	    mkdir build
	    cd build
	    cmake ..
  
* Option 2: **Autotools Build System** (not recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, named 
  **configure**, with the following options to build the entire MB-System package. 

	    ./configure \
          --enable-mbtrn --enable-mbtnav --enable-opencv \
          --with-opencv-include=/usr/include/opencv4 \
          --with-opencv-lib=/lib/x86_64-linux-gnu

  The MB-System libraries will be placed in /usr/local/lib/, but the runtime dynamic linker 
  does not look for shared libraries in this directory by default. Users can add 
  /usr/local/lib to the directories searched for shared libraries by adding 
  “/usr/local/lib” to the environment variable LD_LIBRARY_PATH. This is accomplished by 
  placing the command 
  
	    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	    
  in an environment file in the user’s home directory named .zprofile (if using the zsh 
  shell) or .profile (if using the bash shell). Other user environment files can be used,
  such as .zshrc for zsh or .bashrc for bash. The preferred alternative to using 
  LD_LIBRARY_PATH is to embed the shared library location in the compiled libraries and
  executables by passing an rpath command to the linker by setting the LD_FLAGS 
  environment variable available to the configure command:
  
	    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
	      ./configure \
 	       --enable-mbtrn --enable-mbtnav --enable-opencv \
 	       --with-opencv-include=/usr/include/opencv4 \
 	       --with-opencv-lib=/lib/x86_64-linux-gnu
  
  Note that the CMake build system builds the binaries such that the runtim libraries are
  reliably found without any additional commands or argument.

* Once the makefiles have been generated by cmake or configure, build and install 
  MB-System using:

	    make
	    sudo make install

---
### Ubuntu Bionic Beaver 18.04
---

Ubuntu 18 has a version of CMake that is too old to work with MB-System's CMake build system.
Therefore MB-System can only be built and installed on Ubuntu 18 using the old Autotools
based build  system. Ubuntu 18 also has a version of OpenCV that is too old to allow 
building the MB-System photomosaicing tools.

Ubuntu and Debian Linux distributions include multiple tools for managing software
distributions. We recommend using the command line program apt to install the prerequisite 
software for MB-System.

* Install Ubuntu 18 from an ISO image of the release (https://ubuntu.com/download/desktop).

* Update the starting packages:

	    sudo apt upgrade

* Install compilers and other essential software:

	    sudo apt install build-essential

* Install MB-System prerequisites:

        sudo apt install gmt libgmt5 libgmt-dev gmt-common \
          proj-bin proj-data libproj-dev libgdal-dev \
          libfftw3-3 libfftw3-dev libmotif-dev \
          xfonts-100dpi libglu1-mesa-dev gfortran


  MB-System also requires Python3 and the Pillow library for Python3. Both of these packages
  are installed by default in Ubuntu 18.

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  We recommend using Chrome for the download rather than Safari,
  because Chrome does not prematurely unpack the tarball.
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src/ (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations). 
  If you do not have write privilege in /usr/local/src/ then you will need to execute the
  following commands using sudo:

        mv MB-System-5.7.9-tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Only option: **Autotools Build System**  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, named 
  **configure**, with the following options to build the entire MB-System package. 

	    ./configure \
          --enable-mbtrn --enable-mbtnav --enable-opencv \
          --with-opencv-include=/usr/include/opencv4 \
          --with-opencv-lib=/lib/x86_64-linux-gnu

  The MB-System libraries will be placed in /usr/local/lib/, but the runtime dynamic linker 
  does not look for shared libraries in this directory by default. Users can add 
  /usr/local/lib to the directories searched for shared libraries by adding 
  “/usr/local/lib” to the environment variable LD_LIBRARY_PATH. This is accomplished by 
  placing the command 
  
	    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	    
  in an environment file in the user’s home directory named .zprofile (if using the zsh 
  shell) or .profile (if using the bash shell). Other user environment files can be used,
  such as .zshrc for zsh or .bashrc for bash. The preferred alternative to using 
  LD_LIBRARY_PATH is to embed the shared library location in the compiled libraries and
  executables by passing an rpath command to the linker by setting the LD_FLAGS 
  environment variable available to the configure command:
  
	    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
	      ./configure \
 	       --enable-mbtrn --enable-mbtnav --enable-opencv \
 	       --with-opencv-include=/usr/include/opencv4 \
 	       --with-opencv-lib=/lib/x86_64-linux-gnu
  
  Note that the CMake build system builds the binaries such that the runtim libraries are
  reliably found without any additional commands or argument.

* Once the makefiles have been generated by cmake or configure, build and install 
  MB-System using:

	    make
	    sudo make install

---
### Debian 11 and 12
---
This example is relevant for both Debian 11 and 12 because the prerequisite
package names are the same.

Ubuntu and Debian Linux distributions include multiple tools for managing software
distributions. We recommend using the command line program apt to install the prerequisite 
software for MB-System.

* Install Debian 11 or 12 from an ISO image of the release downloaded from 
  [https://www.debian.org](https://www.debian.org).

* Update the starting packages:

	    sudo apt upgrade

* Install compilers and other essential software:

	    sudo apt install build-essential

* Install MB-System prerequisites:

        sudo apt install netcdf-bin libnetcdf-dev libgdal-dev \
	      gmt libgmt6 libgmt-dev libproj-dev \
 	      libfftw3-3 libfftw3-dev libmotif-dev \
	      xfonts-100dpi libglu1-mesa-dev \
	      libopencv-dev gfortran

  MB-System also requires Python3 and the Pillow library for Python3. Both of these packages
  are installed by default in Debian 11 and 12.

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  We recommend using Chrome for the download rather than Safari,
  because Chrome does not prematurely unpack the tarball.
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src/ (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations). 
  If you do not have write privilege in /usr/local/src/ then you will need to execute the
  following commands using sudo:

        mv MB-System-5.7.9-tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Option 1: **CMake Build System** (recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), create a working directory named 
  "build", **cd** into "build", and then  execute **cmake** at the location 
  /usr/local/src/MB-System-5.7.9/build/). This command should successfully enable building 
  the entire current MB-System (5.7.9 or later) on either Debian 11 or 12 as long as the 
  prerequisite  software packages have been installed.
  
	    mkdir build
	    cd build
	    cmake ..
  
* Option 2: **Autotools Build System** (not recommended)  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, named 
  **configure**, with the following options to build the entire MB-System package. 

	    ./configure \
          --enable-mbtrn --enable-mbtnav --enable-opencv \
          --with-opencv-include=/usr/include/opencv4 \
          --with-opencv-lib=/lib/x86_64-linux-gnu

  The MB-System libraries will be placed in /usr/local/lib/, but the runtime dynamic linker 
  does not look for shared libraries in this directory by default. Users can add 
  /usr/local/lib to the directories searched for shared libraries by adding 
  “/usr/local/lib” to the environment variable LD_LIBRARY_PATH. This is accomplished by 
  placing the command 
  
	    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	    
  in an environment file in the user’s home directory named .zprofile (if using the zsh 
  shell) or .profile (if using the bash shell). Other user environment files can be used,
  such as .zshrc for zsh or .bashrc for bash. The preferred alternative to using 
  LD_LIBRARY_PATH is to embed the shared library location in the compiled libraries and
  executables by passing an rpath command to the linker by setting the LD_FLAGS 
  environment variable available to the configure command:
  
	    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
	      ./configure \
 	       --enable-mbtrn --enable-mbtnav --enable-opencv \
 	       --with-opencv-include=/usr/include/opencv4 \
 	       --with-opencv-lib=/lib/x86_64-linux-gnu
  
  Note that the CMake build system builds the binaries such that the runtime libraries are
  reliably found without any additional commands or argument.

* Once the makefiles have been generated by cmake or configure, build and install 
  MB-System using:

	    make
	    sudo make install

---
### CentOs 7
---
This example is relevant for CentOs 7, and includes building the core MB-System programs
plus the Terrain Relative Navigation tools, but not the photomosaicing tools (which require
a more recent version of OpenCV than included in CentOs 7).

CentOs and Red Hat Linux distributions include 
multiple tools for managing software distributions. We recommend using the command line 
program yum to install the prerequisite  software for MB-System.

* Install CentOs 7 from an ISO image of the release downloaded from 
  [https://www.centos.org/download/](https://www.centos.org/download/).

* Update the starting packages:

	    sudo yum upgrade

* Install EPEL repo:
    
        sudo yum install epel-release

* Install MB-System prerequisites:

        sudo yum install openmotif openmotif-devel \
          fftw fftw-devel netcdf netcdf-devel \
          proj proj-devel gdal-devel gmt gmt-devel gv \
          mesa-libGL mesa-libGL-devel mesa-libGLU mesa-libGLU-devel

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  We recommend using Chrome for the download rather than Safari,
  because Chrome does not prematurely unpack the tarball.
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src/ (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations). 
  If you do not have write privilege in /usr/local/src/ then you will need to execute the
  following commands using sudo:

        mv MB-System-5.7.9-tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Only Option: **Autotools Build System**  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, named 
  **configure**, with the following options to build the entire MB-System package. 

	    ./configure --enable-mbtrn --enable-mbtnav

  The MB-System libraries will be placed in /usr/local/lib/, but the runtime dynamic linker 
  does not look for shared libraries in this directory by default. Users can add 
  /usr/local/lib to the directories searched for shared libraries by adding 
  “/usr/local/lib” to the environment variable LD_LIBRARY_PATH. This is accomplished by 
  placing the command 
  
	    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
	    
  in an environment file in the user’s home directory named .zprofile (if using the zsh 
  shell) or .profile (if using the bash shell). Other user environment files can be used,
  such as .zshrc for zsh or .bashrc for bash. The preferred alternative to using 
  LD_LIBRARY_PATH is to embed the shared library location in the compiled libraries and
  executables by passing an rpath command to the linker by setting the LD_FLAGS 
  environment variable available to the configure command:
  
	    LDFLAGS="-Wl,-rpath -Wl,/usr/local/lib" \
	      ./configure --enable-mbtrn --enable-mbtnav
  
* Once the makefiles have been generated by configure, build and install 
  MB-System using:

	    make
	    sudo make install

---
### CygWin
---

CygWin is a collection of software tools that augment Windows computers with a Unix-style 
environment within which one can build, install, and run Unix-y packages like MB-System.

If Cygwin has been installed on a Windows system, then one must install a number of 
additional prerequisite packages before building MB-System. The package installation is 
done using the Cygwin setup program **setyo0x86_64,exe** rather than through Linux-style 
package managers like **apt** or **yum**. This example installs the core command line
programs of MB-System without any of the graphical tools. For this purpose the prerequisite 
packages include:  

* gcc
* g++
* rpc-devel
* gambas3-devel
* libproj-devel
* libproj12  
* libnetcdf-devel  
* libnetcdf  
* libgdal-devl  
* libgdal19  
* libfftw3-devel  
* libfftw3  
* cmake  
* make  
* fftw  
* fftw-devel  
* ghostscript  
* gv  
* libcurl-devel  
* libnpcr0  
* libnpcr0-devel  
* libpcre  
* pcre-devel  
* openssh  
* subversion  
* xinit  
* zlib  
* zlib-devel  
* liblapack-devel

Once Cygwin and the prerequisite packages are installed, proceed as follows:  

* Download the MB-System source package from the repository at GitHub:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System  
  There are often beta releases that are more recent than the current stable 
  release. The most recent beta release will be at the top of this page:  
  &nbsp;&nbsp;&nbsp;&nbsp;https://github.com/dwcaress/MB-System/releases  
  Although MB-System can be installed from a source tree located anywhere, we
  generally place the tarball in /usr/local/src and then unpack and build
  MB-System there.
  
* Unpack the MB-System distribution tarball in /usr/local/src/ (or somewhere else, if you
  prefer - this is our convention for consistency when managing many installations). 
  If you do not have write privilege in /usr/local/src/ then you will need to execute the
  following commands using sudo:

        mv MB-System-5.7.9-tar.gz /usr/local/src
        cd /usr/local/src
        tar xvzf MB-System-5.7.9.tar.gz  
      
  and then cd into the top directory of the resulting structure:
  
        cd MB-System-5.7.9
  
* Only option: **Autotools Build System**  
  At that location (/usr/local/src/MB-System-5.7.9/), execute the configure script, named 
  **configure**, with the following options to build MB-System without any graphical tools:: 

       ./configure \
         --enable-mbtrn --enable-mbtnav  \
         --disable-dependency-tracking --disable-mbtools

* Once the makefiles have been generated by cmake or configure, build and install 
  MB-System using:

	    make
	    sudo make install


---
### Additional Setup
---

* PATH environment variable  
  Unless one specifies otherwise, the MB-System executable programs, libraries, and header 
  files will be installed in directories named /usr/local/bin /usr/local/lib and 
  /usr/local/include respectively. In order for the MB-System programs to execute from a 
  command line, the /usr/local/bin directory must be included in a user’s \$PATH environment 
  variable. This is typically accomplished by including a statement like:  
  
         export PATH=/usr/local/bin:$PATH
       
  in an environment file in the user’s home directory named .zshenv  (if using the zsh 
  shell), or .profile (if using the bash shell). Other user environment files can be 
  used, such as .zshrc for zsh or .bashrc for bash.
  
* Enable MB-System GMT Modules  
  A key prerequisite for MB-System is the software package Generic Mapping Tools, or GMT.
  The programs mbcontour, mbswath, mbgrdtiff, and mbgrd2obj are actually GMT plug-in 
  modules invoked as commands of the program gmt:

	    gmt mbcontour (….various arguments…..)  
	    gmt mbswath (….various arguments…..)  
	    gmt mbgrdtiff (….various arguments…..)  

  These four modules are contained in a library named mbsystem.so on Linux computers and
  mbsystem.dylib on MacOs. In either case, this library is installed to /usr/local/lib.
  In order for GMT to successfully execute the MB-System modules, the location of the shared 
  library containing the dynamically loaded modules must be known to GMT. GMT looks for the 
  modules using an internal variable GMT_CUSTOM_LIBS loaded from a configuration file named 
  gmt.conf. The man page for gmt.conf is at:  
  &nbsp;&nbsp;&nbsp;&nbsp;[https://docs.generic-mapping-tools.org/dev/gmt.conf.html](https://docs.generic-mapping-tools.org/dev/gmt.conf.html)

  One usually enables use of the MB-System GMT modules by creating a gmt.conf file in the 
  user’s home directory, and altering the gmt.conf file either by manual editing or through 
  use of the gmtset module such as:

	    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.so
	    
  on Linux computers and
  
	    gmt gmtset GMT_CUSTOM_LIBS /usr/local/lib/mbsystem.dylib
	    
  on MacOs computers.

  A complexity not mentioned in the gmt.conf man page is that GMT looks for gmt.conf first 
  in the current working directory, then if not found it looks in the users home directory, 
  and if not found then uses the system defaults. A number  of programs and plotting 
  scripts using GMT create a gmt.conf file in the local directory, alter some of the 
  settings in that file, use it, and then delete it before exiting. If one of these crashes 
  or is interruped before the temporary gmt.conf file is deleted, then an orphan gmt.conf 
  file is left that will become the default then next time GMT is run in that directory.
  
* Perl ForkManager module  
  Several MB-System programs are written in the scripting language Perl. Three of these 
  (mbm_multiprocess, mbm_multicopy, mbm_multidatalist) utilize a Perl library called
  ForkManager that enables parallel processing, and thus require that this library be
  installed. The managagement of Perl libraries is done with a tool named cpan rather than
  with the package managers used to install other MB-System prerequisites. Perl and cpan 
  are available on all of the relevant operating systems. In all cases, execute:
  
	    cpan Parallel:ForkManager
	    

---
### Updating an MB-System Installation
---

When one updates to a new MB-System version, we recommend uninstalling the previous 
version before installing the next. In the directory for the previous version (e.g., 
/usr/local/src/MB-System5.7.9beta52 if using Autotools or 
/usr/local/src/MB-System5.7.9beta52/build if using CMake), use the following command 
whether using the Autotools or the CMake build system:

	    sudo make uninstall
	    sudo make clean
  
Then follow the appropriate installation instructions starting with downloading the next 
version from GitHub.

---
### Docker Container with MB-System
---

An updated MB-System Docker Image is generated each time that a new release is created in the MB-System Github repository. This Docker is based on CentOs 7, and can be run on MacOs, Linux, and Windows computers. Data present on the host computer's filesystems can be processed using the MB-System programs in the Docker container.

The MB-System docker image is available at 
    https://hub.docker.com/r/mbari/mbsystem

Documentation is available at:
    https://github.com/dwcaress/MB-System/tree/master/docker/user
    https://github.com/dwcaress/MB-System/blob/master/docker/user/README-win11.md


---
### Packaged MB-System Distributions
---

There currently are no packaged distributions of MB-System. We are working on establishing them again.


---

