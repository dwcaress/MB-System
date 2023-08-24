# Alpine is smaller than Ubuntu
FROM alpine:latest AS builder
ENV BUILD_ONLY "openssh cmake \
    build-base \
    autoconf \
    automake \
    libtool \
    net-tools \
    git \
    wget \
    curl \
    gpg \
    curl"

ENV LCM_DEPS "glib-dev \
    fontconfig \
    ttf-droid \
    ttf-liberation \
    openjdk11 "

ENV MBSYS_DEPS "proj-dev \
    gdal-dev \
    netcdf-dev \
    fftw-dev  \
    gnuplot \
    ghostscript \
    libx11-dev \
    libxt-dev \
    libtirpc-dev \
    glu-dev \
    motif-dev"

ENV COMMON_DEPS "libgcc libstdc++ mesa-dev"

ENV BUILD_DEPS "${COMMON_DEPS} ${LCM_DEPS} ${MBSYS_DEPS}"

ENV LCM_BUILD_OPTS "CMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DLCM_ENABLE_GO=OFF \
    -DLCM_ENABLE_PYTHON=OFF \
    -DLCM_ENABLE_LUA=OFF \
    -DLCM_ENABLE_EXAMPLES=ON"

ENV BUILD_DIR /tmp/build

RUN apk -U upgrade --no-cache && \
    apk add --no-cache ${BUILD_ONLY} ${BUILD_DEPS} && \
    mkdir -p ${BUILD_DIR} && \
    cd ${BUILD_DIR} && \
    wget https://github.com/GenericMappingTools/gmt/releases/download/6.1.1/gmt-6.1.1-src.tar.xz && \
    tar -xvf gmt-6.1.1-src.tar.xz && \
    cd gmt-6.1.1 && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && make && make install && \
    git clone https://github.com/lcm-proj/lcm.git && \
    cd lcm && [ -d build ] || mkdir build && \
    cd build && cmake ${LCM_BUILD_OPTS} .. && make && make install && \
    cd ${BUILD_DIR} && \
    git clone https://github.com/dwcaress/MB-System.git && \
    cd MB-System && \
    libtoolize --force --copy && \
    aclocal && \
    autoheader && \
    automake --add-missing --include-deps && \
    autoconf && \
    autoupdate && \
    autoreconf --force --install --warnings=all && \
    echo "!!! Fixing \${bits} refrences in MB-System configure script" && \
    cp configure configure.auto && \
    cat configure | sed -e "s/\${bits}/64/g" > configure.new && \
    mv configure.new configure && \
    chmod 775 configure && \
    ./configure --enable-mbtrn --enable-mbtnav && \
    make CFLAGS="-I/usr/local/include/gmt" LDFLAGS="-L/usr/local/lib/gmt5/lib -lgmt" && make install && \
    mkdir -p /opt/mbsys && \
    cp -r ${BUILD_DIR}/MB-System/src/mbtrn/tools/* /opt/mbsys/ && \
    apk del ${BUILD_ONLY}  && \
    apk -v cache clean && \
    rm -rf ${BUILD_DIR} && \
    ldconfig / && \
    ldconfig /usr/local/lib && ldconfig /usr/local/lib/gmt5 

# EXPOSE ports
#EXPOSE 80 443 27000 27027 8001 7000 7001

# Set the entrypoint which can be overwritten by docker-compose
#ENTRYPOINT ["/opt/mbsys/mbtrn/start.sh"]

