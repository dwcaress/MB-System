# Using the dockerized MB-System

Notes for the end user about using the dockerized MB-System.

**Status**: Functional and tested, mainly on CentOS 7 and macOS,
and, more recently, though minimally, also on Windows 11.

## Requirements

The core requirements on your host system are:

- Docker engine
- X11 server

Additionally:

- on macOS:

    - [socat](http://www.dest-unreach.org/socat/)

- on Windows 11:
    - More details [here](README-win11.md). 



## The MB-System docker image

The MB-System docker image is available at
https://hub.docker.com/r/mbari/mbsystem.

Note that proper releases of the image are indicated with tags having an
`x.y.z` prefix, for example, `5.7.6beta37`.
Typically, you will be using the most recent of such available tags.

The complete image designation has the form `mbari/mbsystem:<tag>`,
for example, `mbari/mbsystem:5.7.6beta37`.
The `MBSYSTEM_IMAGE` variable in the launcher script (described below)
is used to capture this image designation.

### The launcher script

The included `mbsystem.sh` script is basically a wrapper around the
`docker` command on your host to help run the MB-System.
This script is to be used on Linux and MacOS hosts.

In the following we assume that `mbsystem.sh` is available through
your `$PATH` and that it has execution permission.

First, edit the script to set the particular docker image to be used
(variable `MBSYSTEM_IMAGE`), as well as the running user and the host
directory to mount as `/opt/MBSWorkDir` in the container.

> The launcher script will automatically download the image if not already
> available locally, but you can get it beforehand with an explicit
> `docker pull` command, for example:
>
>       $ docker pull mbari/mbsystem:5.7.6beta37
>       5.7.6beta37: Pulling from mbari/mbsystem
>       ab5ef0e58194: Already exists
>       37cd1160c2ff: Pull complete
>       418bb9b64c52: Pull complete
>       cee9b14da639: Pull complete
>       cb70efeb0bb1: Pull complete
>       ced5f26bd0fc: Pull complete
>       6306c31894e1: Pull complete
>       05defd1ec313: Pull complete
>       9616a48b6670: Pull complete
>       618c4d986e70: Pull complete
>       Digest: sha256:924b85c95c53ed70716e7016fdbf822b69219fb3da3f99e0929f841b894d60c2
>       Status: Downloaded newer image for mbari/mbsystem:5.7.6beta37
>       docker.io/mbari/mbsystem:5.7.6beta37


The script can accept some arguments
(run `mbsystem.sh -h` to see a help message),
but it will typically be run with no arguments
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
By default, depending on your defined settings in the script, this location
is mapped to the current directory on your host when you launch the system,
`/tmp` in the example above.

Note also that `$HOME/.mbsystem.bash_history`, a file created by the script
on your host and mounted into a file in the container, is used to help
preserve your command history upon re-starts of the container.
