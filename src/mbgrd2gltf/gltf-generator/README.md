# GLTF Generator

This is a program made to replace the current 3d geometry generation pipeline used in STOQS. This project was done for CSUMB capstone in fall 2021 to solve the following issue: https://github.com/stoqs/stoqs/issues/1093

# Team

* Isaac Hirzel
* Oliver Stringer
* Zachary Abbett
# Dependencies

* netCDF4 C libary
	* For linux, this can be gotten from the system package manager. On ubuntu, the package is **libnetcdf-dev**
	* For Windows, this can be downloaded from https://www.unidata.ucar.edu/downloads/netcdf/
* TinyGLTF
	* This is in the project files as a single-header library. No futher action is needed for this dependency.

# How to Build

This project uses CMake as the build-system generator, and running cmake with the CMakeLists.txt file in the gltf-generator folder should be enough to generate the build files.

More specifically, in a MB-System Docker container shell one can:
```
	cd /opt/MB-System/src/mbgrd2gltf/gltf-generator/build
    export netCDFCxx_DIR=/usr/local/lib64/cmake/netCDF/
	cmake3 ..	
	make
    ./grd-to-gltf -h
```

As long as all the required packages/dependencies are installed, this should work.
