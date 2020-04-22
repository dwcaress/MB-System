# Using the dockerized MB-System

Notes for the end user about using the dockerized MB-System.

**Status**: WIP, preliminary notes.  Feedback most welcome.

## Requirements

The core requirements on your host system are:

- Docker engine
- X11 server

Additionally:

- on MacOS:
    - [socat](http://www.dest-unreach.org/socat/)

- on Windows 10:
    - (TODO)


The dockerized MB-system has been tested on ...


## The MB-System docker image

The MB-System docker image is available at
https://hub.docker.com/repository/docker/mbari/mbsystem.

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
    a145630667c7: Already exists
    785c505a04fc: Already exists
    71848de57f0d: Already exists
    6d6760671aa6: Already exists
    ded91f515ba2: Already exists
    a3267f0a1277: Already exists
    77cffdf7b54a: Pull complete
    48a2a54358a4: Pull complete
    5b675a116cbe: Pull complete
    Digest: sha256:548ce9f5980d9b8a25efc415a2e20882647b2ad63d805fd48797375818cb33ce
    Status: Downloaded newer image for mbari/mbsystem:5.7.6beta32
    docker.io/mbari/mbsystem:5.7.6beta32

### The launcher script

The included `mbsystem.sh` script is basically a wrapper around the
`docker` command to help run any of the MB-System programs.
At this point, the script has a focus on Linux and MacOS hosts.

TODO: Windows 10.

Relying on the `MBSYSTEM_IMAGE` environment variable as defined above,
you just need to indicate the desired program and any arguments
to `mbsystem.sh`, for example:

    $ ./mbsystem.sh mbabsorption -h

In this case, this is equivalent to:

    $ docker run -it --rm $MBSYSTEM_IMAGE mbabsorption -h
    
The launcher, however, is also intended to help set up the host environment
prior to running the MB-System programs, in particular those with a GUI.
Example:

    $ ./mbsystem.sh -gui mbnavadjust

> NOTE: the `-gui` option is temporary.

