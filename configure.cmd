#------------------------------------------------------------------------------
# From Bob Covill
# Updated March 16, 2013
#------------------------------------------------------------------------------
# To modify build system...
# Edit configure.in and Makefile.am (in each folder)
# and then run the following:
#------------------------------------------------------------------------------

# Build libtool files for AM_PROG_LIBTOOL
libtoolize --force --copy
aclocal

# Build custom header for configure
autoheader
automake --add-missing --include-deps
autoconf

# To update configure files use the following:
autoupdate
autoreconf --force --install --symlink --warnings=all

#------------------------------------------------------------------------------
# More notes from Bob Covill
# May 28, 2013
#------------------------------------------------------------------------------
#
# 1. The actual ./configure script is controlled by a template file
# configure.in . Any changes to configure should be added there.
#
# 2. A number of configure options are saved when you run ./configure to a
# header file ./src/mbsystem_config.h . Again this has a
# template ./src/mbsystem_config.h.in .
#
# 3. Each folder has a Makefile template Makefile.am.
#
# 4. Any changes to the configure system should be done in the above
# templates. After any of the files are changed the configure system has
# to be rebuilt with the autoreconf command ...
autoreconf --force --install --warnings=all
# This will generate a new configure script and update the Makefile.in
# files.
#
# 5. Once you run a successful ./configure the final Makefiles are
# generated.
#
# 6. After configure you can run the usual ...
make
make install
make clean (to clean up compiled code)
make distclean (to clean up compiled code and configure output)
make uninstall (to remove a previously installed version)

#------------------------------------------------------------------------------
# Configure script command line options:
#------------------------------------------------------------------------------
# --prefix=install location for mbsystem (/usr/local/mbsystem)
# --with-netcdf-lib location of NetCDF libs
# --with-netcdf-include location of NetCDF headers
# --with-gmt-lib location of GMT libs
# --with-gmt-include location of GMT headers
# --with-fftw-lib location of FFTW3 libs (optional)
# --with-fftw-include location of FFTW3 headers (optional)
# --with-motif-lib location of Motif libs (optional)
# --with-motif-include location of Motif headers (optional)
# --with-opengl-lib location of OpenGL libs (optional)
# --with-opengl-include location of OpenGL headers (optional)
#------------------------------------------------------------------------------

# Build in place:
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN -DBYTESWAPPED" \
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
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN -DBYTESWAPPED" \
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
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DBYTESWAPPED" \
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
