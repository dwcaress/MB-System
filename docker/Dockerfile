FROM centos:7

RUN yum -y groupinstall "Development Tools"

RUN yum install -y epel-release

# Per https://www.mbari.org/products/research-software/mb-system/how-to-download-and-install-mb-system/#toggle-id-10
RUN yum -y install \
            fftw \
            fftw-devel \
            gdal-devel \
            gmt \
            gmt-devel \
            netcdf \
            netcdf-devel \
            openmotif \
            openmotif-devel \
            perl \
            proj \
            proj-devel

# Added freeglut to resolve OpenGL stuff:
RUN yum -y install \
            freeglut \
            freeglut-devel

# -------------------------
# MB-System:

RUN  mkdir -p /opt/MB-System /opt/MBSWorkDir

COPY . /opt/MB-System/
RUN  cd /opt/MB-System/         && \
     ./configure                && \
     make                       && \
     make install

WORKDIR    /opt/MBSWorkDir

# No explicit cmd or entry_point; any MB-System program (and arguments)
# can be executed as arguments passed at the end of `docker run ...` command.
