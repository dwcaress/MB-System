#!/usr/bin/env bash
set -e

bold=$(tput bold)
endbold=$(tput sgr0)

echo "${bold}Compiler Info:${endbold}"
docker exec -e CC=${CC} -e CXX=${CXX} -t ${BUILD_CONTAINER_NAME} ${CC} -v
docker exec -e CC=${CC} -e CXX=${CXX} -t ${BUILD_CONTAINER_NAME} ${CXX} -v

echo "${bold}PROJ info:${endbold}"
docker exec -e CC=${CC} -e CXX=${CXX} -t ${BUILD_CONTAINER_NAME} proj

echo "${bold}GDAL info:${endbold}"
docker exec -e CC=${CC} -e CXX=${CXX} -t ${BUILD_CONTAINER_NAME} gdal-config --version

echo "${bold}GMT info:${endbold}"
docker exec -e CC=${CC} -e CXX=${CXX} -t ${BUILD_CONTAINER_NAME} gmt-config --version
