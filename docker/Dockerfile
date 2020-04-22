# See README.md
FROM zberkowitz/mbsystem-deps:centos-7

RUN mkdir -p /opt/MB-System \
             /opt/MBSWorkDir

COPY . /opt/MB-System/
RUN  cd /opt/MB-System/         && \
     ./configure                && \
     make                       && \
     make check                 && \
     make install

WORKDIR  /opt/MBSWorkDir

# No explicit cmd or entry_point; any MB-System program (and arguments)
# can be executed as arguments passed at the end of the `docker run ...` command.
