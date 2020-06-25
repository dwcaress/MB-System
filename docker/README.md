# Dockerized MB-System

See https://github.com/dwcaress/MB-System/issues/807

**NOTE**: User-oriented instructions: [user/README.md](user/README.md).

> Proper location for such instructions TBD.

## Status

- Basic setup functional
- GUI tests OK on CentOS 7 and MacOS
- OpenGL-related issues on MacOS

## Dockerfile

The [`Dockerfile`](Dockerfile) here for the MB-System image uses `centos:7`
as base image. On top of this, dependencies are installed according to
[general instructions for CentOS](
https://www.mbari.org/products/research-software/mb-system/how-to-download-and-install-mb-system/#toggle-id-10)
on the project website, as well as relevant CI scripts in the repo, and as
needed while doing testing of the system, for example:

- `mesa-dri-drivers`
- `gedit`
- `evince`


## Automatic image build and publication

Image build and publication to be automatically triggered at Docker Hub:

  - image `mbari/mbsystem:latest` upon a push to the *master* branch
  - image `mbari/mbsystem:<tag>`  upon pushing tag `<tag>`

(This is currently enabled for the
[mbari-org/MB-System](https://github.com/mbari-org/MB-System) fork.)

**TODO**(who?): Enable this for [`dwcaress/MB-System`](https://github.com/dwcaress/MB-System)

## Manual image build and publication

Inspect [`ChangeLog.md`](../ChangeLog.md) to determine the version to be
reflected in the docker image.

Example:

    $ MBSYSTEM_IMAGE=mbari/mbsystem:5.7.6beta37
    $ cd ..  ## i.e., root of the MB-System codebase
    $ docker build -f docker/Dockerfile -t "$MBSYSTEM_IMAGE" .
    
Do some tests (see below) and then:

    $ docker push $MBSYSTEM_IMAGE

## Basic tests

Note: very basic tests just for minimal functionality, in particular,
no volume mappings are set below.

### Command-line program example

    $ docker run -it --rm $MBSYSTEM_IMAGE mbabsorption -h

    Program MBabsorption
    MB-system Version 5.7.6beta37
    
    MBabsorption calculates the absorption of sound in sea water
    in dB/km as a function of frequency, temperature, salinity,
    sound speed, pH, and depth.
    
    usage: mbabsorption [-Csoundspeed -Ddepth -Ffrequency -Pph -Ssalinity -Ttemperature -V -H]

### GUI programs

(some notes [here](gui/README.md))

    $ socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\" &
    $ ip=$(ifconfig en0 | grep "inet " | sed 's/.*inet \([0-9\.]*\).*/\1/g')
    $ xhost + ${ip}
    $ export DISPLAY=${i}:0
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbgrdviz
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbeditviz
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbnavadjust

----

**DC & CR meeting (2020-01-13)**

- [x] Use current host directory mapped as "home" directory.
  Done, see [user/README.md](user/README.md).

- [x] Ability to access shares like titan.
  This is already in place, noting that some image customization is
  required regarding with user/group ownership.

- List file with "pointers" to locations

- Windows
