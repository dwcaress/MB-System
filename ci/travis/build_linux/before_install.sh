#!/usr/bin/env bash
set -ex


echo "Starting docker container ${DOCKER_IMAGE}"
docker run -d --name ${BUILD_CONTAINER_NAME} -v ${TRAVIS_BUILD_DIR}:/build -w /build ${DOCKER_IMAGE} tail -f /dev/null
docker ps
