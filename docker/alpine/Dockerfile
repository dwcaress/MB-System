# Alpine is smaller than Ubuntu
FROM alpine:latest AS builder

ARG BUILD_ONLY="openssh cmake \
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

ARG LCM_DEPS="glib-dev \
    fontconfig \
    ttf-droid \
    ttf-liberation \
    openjdk11 "

ARG MBSYS_DEPS="proj-dev \
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

ARG MBSYS_BRANCH="master"

ARG COMMON_DEPS="libgcc libstdc++ bash mesa-dev mesa-dri-gallium"

ARG BUILD_DEPS="${COMMON_DEPS} ${LCM_DEPS} ${MBSYS_DEPS}"

ARG LCM_BUILD_OPTS="CMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DLCM_ENABLE_GO=OFF \
    -DLCM_ENABLE_PYTHON=OFF \
    -DLCM_ENABLE_LUA=OFF \
    -DLCM_ENABLE_EXAMPLES=ON"

ARG MB_PAT="ATCTT3xFfGN0GimkqhsikY9nY7oTXez74wobYHNcOuIBMG4DI6JCdD25P9urnWzR83d6v9q0QxCVyxEvN2Mk9oKQKWdA_gW53SAwr4-kTC-sCCVmVe233R_E6rdRd9fJzSG_ypeGPmLqlEnCbWZxe1x8cz0vzRITyPuPNBYkphywmwTsAr9sTuM=012AA319"
ARG MF_PAT="ATCTT3xFfGN0ndfNYTNTfVPQLzeYe3SD_AHrJxvqExcPKPRTvOhlwf0KUdLvtxhkCe2uvpRKC3EvcsV5vATFyNeQH1ZyG6t_DX2EI5BsP_6x1sYyV2nPIDPi7rmrCMLhCYVTg4TAIXON6gRnlOD83lsz4viO4VGMadhjGOAxHmqYmO_XgPYF0JY=7A143707"

ARG BUILD_DIR=/tmp/build

ENV CLASSPATH="/usr/local/share/java/jchart2d-3.2.2.jar:\
/usr/local/share/java/lcm.jar:\
/usr/local/share/java/lcmtypes_senlcm.jar:\
/usr/local/share/java/lcmtypes_trn.jar:\
/usr/local/share/java/jide-oss-2.9.7.jar:\
/usr/local/share/java/lcmtypes_geolcm.jar:\
/usr/local/share/java/lcmtypes_stdlcm.jar:\
/usr/local/share/java/xmlgraphics-commons-1.3.1.jar"

RUN apk -U upgrade --no-cache && \
    apk add --no-cache ${BUILD_ONLY} ${BUILD_DEPS} && \
    mkdir -p -m 775 /.gmt/server && \
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
    echo "!!! Building MBSYS_BRANCH ${MBSYS_BRANCH}" && \
    git checkout ${MBSYS_BRANCH} && \
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
    cd ${BUILD_DIR} && \
    git clone https://x-token-auth:${MB_PAT}@bitbucket.org/mbari/mb1stream.git && \
    cd mb1stream && \
    git clone https://x-token-auth:${MF_PAT}@bitbucket.org/mbari/mframe.git && \
    cd mframe && \
    make && \
    cd .. && \
    make MFRAME_BUILD=mframe && \
    make install && \
    apk del ${BUILD_ONLY}  && \
    apk -v cache clean && \
    rm -rf ${BUILD_DIR} && \
    ldconfig / && \
    ldconfig /usr/local/lib && ldconfig /usr/local/lib/gmt5 

# EXPOSE ports
#EXPOSE 80/tcp
#EXPOSE 443/tcp
#EXPOSE 28000/tcp
#EXPOSE 28000/udp
#EXPOSE 27000/tcp
#EXPOSE 27000/udp
#EXPOSE 27027/tcp
#EXPOSE 27027/udp
#EXPOSE 8000/tcp
#EXPOSE 8000/udp
#EXPOSE 8001/tcp
#EXPOSE 8001/udp
#EXPOSE 7000/tcp
#EXPOSE 7000/udp
#EXPOSE 7001/tcp
#EXPOSE 7001/udp
#EXPOSE 7667/tcp
#EXPOSE 7667/udp

# Set the entrypoint which can be overwritten by docker-compose
#ENTRYPOINT ["/opt/mbsys/mbtrn/start.sh"]

