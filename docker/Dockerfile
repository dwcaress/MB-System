# See README.md
FROM zberkowitz/mbsystem-deps:centos-7

RUN yum install -y \
        glxinfo \
        mesa-dri-drivers

# TODO remove the following, only added to facilitate debugging
# during initial tests of the container
ENV LIBGL_DEBUG=verbose
# `glxinfo >/dev/null` will for example help spot any errors


RUN mkdir -p /opt/MB-System \
             /opt/MBSWorkDir

COPY . /opt/MB-System/
RUN  cd /opt/MB-System/         && \
     ./configure                && \
     make                       && \
     make check                 && \
     make install

WORKDIR  /opt/MBSWorkDir

# No explicit cmd or entry_point. Launcher script will typically run `bash`
# but also allow to run any other program in the container.
