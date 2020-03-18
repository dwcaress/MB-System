#!/usr/bin/env bash
set -e

bold=$(tput bold)
endbold=$(tput sgr0)

echo "${bold}Compiler Info:${endbold}"
${CC} -v
${CXX} -v

echo "${bold}PROJ info:${endbold}"
proj

echo "${bold}GDAL info:${endbold}"
gdal-config --version

echo "${bold}GMT info:${endbold}"
gmt --version
