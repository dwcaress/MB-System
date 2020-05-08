# See README.md
FROM zberkowitz/mbsystem-deps:centos-7

RUN yum install -y \
        glxinfo \
        mesa-dri-drivers

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

COPY . /opt/MB-System/
RUN  cd /opt/MB-System/         && \
     ./configure                && \
     make                       && \
     make check                 && \
     make install

WORKDIR  /opt/MBSWorkDir

# No explicit cmd or entry_point. Launcher script will typically run `bash`
# but also allow to run any other program in the container.
