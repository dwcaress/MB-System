#!/usr/bin/env bash
set -ex

echo "Building MB-System Build Dependencies Docker image for ${OS} ${OS_TAG}"
DOCKER_BUILD_ARGS="--build-arg OS_TAG=${OS_TAG} \
                   --build-arg GMT_SOURCE_TAG=${GMT_SOURCE_TAG} \
		   --build-arg PROJ_SOURCE_TAG=${PROJ_SOURCE_TAG}"

# Docker doesn't seem to respect build-args unless the dockerfile is in the current directory root...
pushd "${TRAVIS_BUILD_DIR}/docker/${OS}"
docker build ${DOCKER_BUILD_ARGS} -t ${DOCKER_IMAGE} .

echo "Pushing build-deps docker image to docker hub"
docker login -u="${DOCKER_USER}" -p="${DOCKER_PASSWORD}"
docker push "${DOCKER_IMAGE}"
