FROM centos:7

RUN yum -y groupinstall "Development Tools" && \
    yum install -y epel-release && \
    yum install -y \
            fftw \
            fftw-devel \
            netcdf \
            netcdf-devel \
            openmotif \
            openmotif-devel \
            openssh-server \
            perl \
            python3 \
            cmake3 \
            clang \
            git \
            freeglut \
            freeglut-devel \
            glxinfo \
            mesa-dri-drivers \
            gedit \
            evince

# note, problems with latest 6.0.0 release at this point,
# see https://github.com/dwcaress/MB-System/issues/807#issuecomment-632938328
# So, trying master just as an interim
ARG GMT_SOURCE_TAG=master

# Tried 8.2.1, but got compile error, use this stable build
ARG PROJ_SOURCE_TAG=7.2.1

# TODO in general, confirm appropriate install mechanisms for PROJ and GMT on centos
# (per .travis.yml, seems like the above tags are only defined for ubuntu.

COPY docker/centos/scripts/install-gmt.sh .
RUN GMT_SOURCE_TAG=${GMT_SOURCE_TAG} ./install-gmt.sh

COPY docker/centos/scripts/install-proj.sh .
RUN PROJ_SOURCE_TAG=${PROJ_SOURCE_TAG} ./install-proj.sh

# Cleanup
RUN rm -rf install-gmt.sh install-proj.sh && yum -y clean all


# TODO remove the following, only added to facilitate debugging
# during initial tests of the container
ENV LIBGL_DEBUG=verbose
# `glxinfo >/dev/null` will for example help spot any errors


# This location, to be mounted from some file on the host, will
# help keep the bash history upon re-starts of the container:
ENV HISTFILE=/opt/mbsystem.bash_history
# NOTE: not using the default HISTFILE to facilitate running
# the container as a different user from the one by default.

RUN mkdir -p /opt/MB-System \
             /opt/MBSWorkDir \
 && touch $HISTFILE

# NOTE now with issues not resolving GDAL/OGR stuff.
# Added the --with-gdal-config below as th sumary indicated
# no "GDAL found" (even though was there) ...  no luck.
COPY . /opt/MB-System/
RUN cd /opt/MB-System/                              \
 && ./configure --with-gdal-config=/usr/local/bin      \
                --with-proj-lib=/usr/local/lib         \
                --with-proj-include=/usr/local/include \
 && make                                            \
 && make check                                      \
 && make install

RUN  ls /usr/local/include 

WORKDIR  /opt/MBSWorkDir

# due to programs failing to load libgmt
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}/usr/local/lib64

# Permit executing MB-System commands via ssh from another container
RUN echo "export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}" >> /root/.bashrc
# For execution of MB-System gmt modules, e.g.: gmt mbgrd2obj
RUN echo "GMT_CUSTOM_LIBS = /usr/local/lib/mbsystem.so" >> /root/gmt.conf
# Generate host keys
RUN /usr/bin/ssh-keygen -A

# No explicit cmd or entry_point. Launcher script will typically run `bash`
# but also allow to run any other program in the container.
