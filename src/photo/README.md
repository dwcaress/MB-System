# Source Directory: src/photo/

This directory contains the C++ source files for programs used to process seafloor still photography. These programs include mbphotomosiac for generating photomosaics from single or stereo camera rigs, mbgetphotocorrection for calculating lighting correction tables based on standoff distance and look angle, and mbphotogrammetry for calculating topography from pairs of stereo photographs. All these programs use and depend on the OpenCV software package (version 4).

As of July 2020 these programs are not documented, still under very active development, and not built as part of MB-System by default. In order to build and install these programs the configure script must be rerun with the --enable-opencv argument.

These source files are copyright by David Caress and Dale Chayes and licensed using GPL3 as part of MB-System.

