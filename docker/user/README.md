# Using the dockerized MB-System

Notes for the end user about using the dockerized MB-System.

**Status**: WIP. Feedback welcome.

## Requirements

The core requirements on your host system are:

- Docker engine
- X11 server

Additionally:

- on MacOS:

    - [socat](http://www.dest-unreach.org/socat/)

- on Windows 10:
    - (TODO)


The dockerized MB-system has been tested on CentOS 7 and MacOS.


## The MB-System docker image

The MB-System docker image is available at
https://hub.docker.com/r/mbari/mbsystem.

Note that proper releases of the image are indicated with tags having an
`x.y.z` prefix, for example, `5.7.6beta32`.
Typically, you will be using the most recent of such available tags.

The complete image designation has the form `mbari/mbsystem:<tag>`,
for example, `mbari/mbsystem:5.7.6beta32`.
In the following we will assume that such image designation is captured
in the `$MBSYSTEM_IMAGE` environment variable:

    $ export MBSYSTEM_IMAGE=mbari/mbsystem:5.7.6beta32

### Getting the image

The launcher script (see below) will download the image if not already
available locally, but you can get it beforehand with an explicit
`git pull` command:

    $ docker pull $MBSYSTEM_IMAGE
    5.7.6beta32: Pulling from mbari/mbsystem
    ab5ef0e58194: Already exists
    a145630667c7: Pull complete
    785c505a04fc: Pull complete
    71848de57f0d: Pull complete
    6d6760671aa6: Pull complete
    ded91f515ba2: Pull complete
    a3267f0a1277: Pull complete
    11aeaa2f215b: Pull complete
    e570f695f2c5: Pull complete
    914373a96266: Pull complete
    2913a20436b7: Pull complete
    Digest: sha256:2f9b6314edff2b11ceb9f63962a0db20e43f30a61d751593d93f053276dd9a81
    Status: Downloaded newer image for mbari/mbsystem:5.7.6beta32
    docker.io/mbari/mbsystem:5.7.6beta32

### The launcher script

The included `mbsystem.sh` script is basically a wrapper around the
`docker` command on your host to help run the MB-System.
This script is to be used on Linux and MacOS hosts.

In the following we assume that `mbsystem.sh` is available through
your `$PATH` and that it has execution permission.

The script requires the environment variable `MBSYSTEM_IMAGE` to be
defined as explained above.
The script can accept some arguments
(run `mbsystem.sh -h` to see a help message),
but you will typically run it with no arguments
to simply launch the container with `bash` running in it.
As an example, starting on your host:

    $ pwd
    /tmp
    
    $ mbsystem.sh

    bash-4.2$ pwd
    /opt/MBSWorkDir
    
    bash-4.2$ mbedit -h
    
    Program MBedit
    MB-system Version ...
    
    MBedit is an interactive editor used to ...

Note that `/opt/MBSWorkDir` is the initial working directory in the container.
This location is mapped to the current directory on your host when you
launch the system, `/tmp` in the example.

Note also that `$HOME/.mbsystem.bash_history`, a file created by the script
on your host and mounted into a file in the container, is used to help
preserve your command history upon re-starts of the container.
