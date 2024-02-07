#------------------------------------------------------------------------------
# Commands to reconstruct the Autotools based build system for MB-System
#------------------------------------------------------------------------------
#
# David W. Caress
# November 16, 2023
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
# MacPorts, and the OpenCV based photomosaicing and Terrain relative
# Navigation are all enabled
# Add --enable-deprecated to also build the deprecated programs
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

#
#------------------------------------------------------------------------------
