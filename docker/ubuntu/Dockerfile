ARG OS_TAG=bionic

FROM ubuntu:${OS_TAG}

ARG GMT_SOURCE_TAG
ARG PROJ_SOURCE_TAG
ARG DEBIAN_FRONTEND=noninteractive

# Install all dependencies except for proj and gmt from default repos
RUN apt-get update && \
    apt-get install -y locales && \
    locale-gen en_US.UTF-8
    
RUN apt-get install -y \
    	    build-essential \
	    clang \
	    git \
	    cmake \
	    libfftw3-dev \
	    netcdf-bin \
	    libnetcdf-dev \
	    python3 \
	    libmotif-dev \
	    libglu1-mesa-dev \
	    mesa-common-dev

COPY scripts/install-proj.sh .
RUN GMT_SOURCE_TAG=${PROJ_SOURCE_TAG} ./install-proj.sh

COPY scripts/install-gmt.sh .
RUN GMT_SOURCE_TAG=${GMT_SOURCE_TAG} ./install-gmt.sh

# Cleanup
RUN rm -rf install-gmt.sh install-proj.sh /var/lib/apt/lists/*