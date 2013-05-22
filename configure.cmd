

# One option:
export CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN" ; \
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

export CFLAGS="-I/usr/X11R6/include -L/usr/X11R6/lib -DDARWIN" ; \
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
