# Dockerized MB-System

See https://github.com/dwcaress/MB-System/issues/807

## Status

- Basic setup functional
- Some GUI basic tests OK on MacOS
- This README is still preliminary as well

## Automatic image build and publication

- To be automatically triggered on Docker Hub upon a push to the master branch.

    TODO: confirm master is the branch to be used.

- Image tag: `mbari/mbsystem:latest`

    TODO: also generate and publish explicit version.

## Manual image build and publication

Example with `5.7.6beta23` for the tag:

    $ cd ..  ## i.e., root of the MB-System codebase
    $ docker build -f docker/Dockerfile -t "mbari/mbsystem:5.7.6beta23" .
    $ docker push mbari/mbsystem:5.7.6beta23

## Basic tests

With:

    $ MBSYSTEM_IMAGE="mbari/mbsystem:latest"

Some command-line program:

    $ docker run -it --rm $MBSYSTEM_IMAGE mbabsorption -h

GUI program:

    $ ip=$(ifconfig en0 | grep "inet " | sed 's/.*inet \([0-9\.]*\).*/\1/g')
    $ xhost + ${ip}
    $ export DISPLAY=${i}:0
    $ docker run -it --rm -e DISPLAY=${ip}:0 $MBSYSTEM_IMAGE mbgrdviz
