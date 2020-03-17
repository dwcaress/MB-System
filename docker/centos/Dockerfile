ARG OS_TAG=7

FROM centos:${OS_TAG}

ARG GMT_SOURCE_TAG

RUN yum -y groupinstall "Development Tools" && \
    yum install -y epel-release && \
    yum install -y \
            fftw \
            fftw-devel \
            netcdf \
            netcdf-devel \
            openmotif \
            openmotif-devel \
            perl \
            python3 \
            cmake3 \
            clang \
            git \
            freeglut \
            freeglut-devel
	    
COPY scripts/install-proj.sh .
RUN GMT_SOURCE_TAG=${PROJ_SOURCE_TAG} ./install-proj.sh

COPY scripts/install-gmt.sh .
RUN GMT_SOURCE_TAG=${GMT_SOURCE_TAG} ./install-gmt.sh

# Cleanup
RUN rm -rf install-gmt.sh install-proj.sh && yum -y clean all
