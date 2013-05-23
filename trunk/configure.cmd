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
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN" \
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
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN" \
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
CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN" \
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
