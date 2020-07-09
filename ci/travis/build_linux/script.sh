#!/usr/bin/env bash
set -ex

if [ -z "${MBSYSTEM_CONFIGURE_ARGS}" ]; then
    export MBSYSTEM_CONFIGURE_ARGS="--enable-mbtrn --enable-mbtnav"
fi

echo "Building MB-System..."
# DOCKER_IMAGE="zberkowitz/mbsystem-deps:${OS}-${OS_TAG}"
docker exec -e CC="${CC}" -e CXX="${CXX}" -e CFLAGS="${CFLAGS}" -t ${BUILD_CONTAINER_NAME} bash -c "./configure ${MBSYSTEM_CONFIGURE_ARGS} --enable-test && make && make V=1 VERBOSE=1 check"
