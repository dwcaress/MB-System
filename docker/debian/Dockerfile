# Debian base
FROM debian:12.5 AS builder

ARG BUILD_ONLY="openssh-server \
    cmake \
    build-essential \
    autoconf \
    automake \
    libtool \
    net-tools \
    git \
    wget \
    curl \
    gpg \
    curl"

ARG COMMON_DEPS="libgcc-11-dev \
    libglib2.0-dev \
    x11-apps \
    mesa-utils \
    bash \
    vim"

ARG LCM_DEPS="fontconfig \
    fonts-droid-fallback \
    fonts-liberation \
    openjdk-17-jdk"

ARG MBSYS_BRANCH="master"

ARG MBSYS_DEPS="libproj-dev \
    libgdal-dev \
    gmt-common \
    libgmt6 \
    libgmt-dev \
    gdal-bin \
    libgdal-dev \
    libhdf5-dev \
    libnetcdf-dev \
    libhdf5-dev \
    libfftw3-dev  \
    graphicsmagick \
    ffmpeg \
    libxdg-basedir-dev \
    gnuplot \
    ghostscript \
    ghostscript-x \
    x11-common \
    x11-apps \
    libxt-dev \
    libtirpc-dev \
    libglu1-mesa-dev \
    libmotif-dev \
    libxml2-dev \
    libopencv-dev"

ARG BUILD_DEPS="${COMMON_DEPS} ${LCM_DEPS} ${MBSYS_DEPS}"

ARG LCM_BUILD_OPTS="CMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DLCM_ENABLE_GO=OFF \
    -DLCM_ENABLE_PYTHON=OFF \
    -DLCM_ENABLE_LUA=OFF \
    -DLCM_ENABLE_EXAMPLES=ON"

ARG BUILD_DIR=/tmp/build

ENV JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"

ENV CLASSPATH="/usr/local/share/java/jchart2d-3.2.2.jar:\
/usr/local/share/java/lcm.jar:\
/usr/local/share/java/lcmtypes_senlcm.jar:\
/usr/local/share/java/lcmtypes_trn.jar:\
/usr/local/share/java/jide-oss-2.9.7.jar:\
/usr/local/share/java/lcmtypes_geolcm.jar:\
/usr/local/share/java/lcmtypes_stdlcm.jar:\
/usr/local/share/java/xmlgraphics-commons-1.3.1.jar"

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y ${BUILD_ONLY} ${BUILD_DEPS} && \
    mkdir -p ${BUILD_DIR} && \
    mkdir -p -m 775 /.gmt/server && \
    cd ${BUILD_DIR} && \
    git clone https://github.com/lcm-proj/lcm.git && \
    cd lcm && [ -d build ] || mkdir build && \
    cd build && cmake ${LCM_BUILD_OPTS} .. && make && make install && \
    cd ${BUILD_DIR} && \
    git clone https://github.com/dwcaress/MB-System.git && \
    cd MB-System && \
    git checkout ${MBSYS_BRANCH} && \
    echo "!!! Building MBSYS_BRANCH ${MBSYS_BRANCH}" && sleep 3 && \
    mkdir build && cd build && \
    cmake -DbuildOpenCV=ON -DCMAKE_BUILD_TYPE=Debug .. && make && make install && \
    mkdir -p /opt/mbsys && \
    cp -r ${BUILD_DIR}/MB-System/src/mbtrn/tools/* /opt/mbsys/ && \
    rm -rf /var/lib/apt/lists/* && \
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

