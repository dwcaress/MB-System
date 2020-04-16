# Dockerized MB-System

See https://github.com/dwcaress/MB-System/issues/807

## Status

- Basic setup functional
- Some basic GUI tests OK on MacOS
- This README is still preliminary as well

## Dockerfile

`Dockerfile` here leverages 
[zberkowitz/mbsystem-deps:centos-7](https://hub.docker.com/r/zberkowitz/mbsystem-deps/tags)
as base image for the build of the Mb-System itself.
See [`../.travis.yml`](../.travis.yml) and [`centos/Dockerfile`](centos/Dockerfile). 

TODO: determine whether the build of the dependencies base image should be
incorporated as part of the overall MB-System build.

## Automatic image build and publication

- Automatically triggered on Docker Hub upon a push to the master branch
  at Github.  In this case the updated image is `mbari/mbsystem:latest`.

- TODO: also generate versioned image upon a (final) push to a tag.

## Manual image build and publication

Inspect `ChangeLog.md` to determine the version to be reflected in the docker image.

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

Status: preliminary (some notes [here](https://github.com/carueda/mbsystem-docker/blob/master/notes/gui.md)).

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
- keep using separate repo for now
