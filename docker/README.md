# Dockerized MB-System

See https://github.com/dwcaress/MB-System/issues/807

## Status

- Basic setup functional
- Some basic GUI tests OK on MacOS and CentOS 7

## Dockerfile

The [`Dockerfile`](Dockerfile) here for the MB-System image build leverages 
[zberkowitz/mbsystem-deps:centos-7](https://hub.docker.com/r/zberkowitz/mbsystem-deps/tags)
as base image for the build of the Mb-System itself.
See [`../.travis.yml`](../.travis.yml) and [`centos/Dockerfile`](centos/Dockerfile). 

**TODO**: Determine whether the build of the dependencies base image above
should be incorporated as part of the overall MB-System image build.

## Automatic image build and publication

- Automatically triggered on Docker Hub upon a push to the master branch
  at Github.
  In this case the updated image gets the "latest" tag: `mbari/mbsystem:latest`.
  
    **NOTE**: This is currently enabled for the 
    [mbari-org/MB-System](https://github.com/mbari-org/MB-System) fork.

- **TODO**: Also generate versioned image upon a push to a tag.

- **TODO**(who?): Enable the above at Docker Hub for the 
  [`dwcaress/MB-System`](https://github.com/dwcaress/MB-System)
  repo.

## Manual image build and publication

Inspect [`ChangeLog.md`](../ChangeLog.md) to determine the version to be
reflected in the docker image.

Example:

    $ MBSYSTEM_IMAGE=mbari/mbsystem:5.7.6beta32
    $ cd ..  ## i.e., root of the MB-System codebase
    $ docker build -f docker/Dockerfile -t "$MBSYSTEM_IMAGE" .
    
Do some tests (see below) and then:

    $ docker push $MBSYSTEM_IMAGE

## Basic tests

### Command-line program example

    $ docker run -it --rm $MBSYSTEM_IMAGE mbabsorption -h

    Program MBabsorption
    MB-system Version 5.7.6beta32
    
    MBabsorption calculates the absorption of sound in sea water
    in dB/km as a function of frequency, temperature, salinity,
    sound speed, pH, and depth.
    
    usage: mbabsorption [-Csoundspeed -Ddepth -Ffrequency -Pph -Ssalinity -Ttemperature -V -H]

### GUI programs

Status: preliminary (some notes [here](gui/README.md)).

    $ socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\" &
    $ ip=$(ifconfig en0 | grep "inet " | sed 's/.*inet \([0-9\.]*\).*/\1/g')
    $ xhost + ${ip}
    $ export DISPLAY=${i}:0
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbgrdviz
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbeditviz
    
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbnavadjust

----

**DC & CR meeting (2020-01-13)**

- Ability to access shares like titan
- Use current host directory mapped as "home" directory
- List file with "pointers" to locations
- Windows
