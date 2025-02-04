#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes 
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#      
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------
#
#
# To build MB-System from source using CMake do (example):
#
# mkdir build
# cd build
# cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#
# CMAKE_BUILD_TYPE can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel
#
#------------------------------------------------------------------------------

message("----------------------------------------------------------")
message("MB-System")
message("CMake Build System")
message("David Caress (Monterey Bay Aquarium Research Institute)")
message("Johannes Schauer (University of Weurzburg)")
message("Tom O'Reilly (Monterey Bay Aquarium Research Institute)")
message("----------------------------------------------------------")
message("CMake version: ${CMAKE_VERSION}")
message("Set install directory using -DCMAKE_INSTALL_PREFIX=directory (default is /usr/local)")
message("The default build mode is release (optimized for performance, not debuggable).")
message("Enable debugging with -DCMAKE_BUILD_TYPE=Debug")
message("Build shared/static libraries with -DBUILD_SHARED_LIBS=ON/OFF")
message("Disable building TRN software with -DbuildTRN=OFF")
message("Disable building photomosaicing software with -DbuildOpenCV=OFF")
message("Disable supporting GSF format data with -DbuildGSF=OFF")
message("Build Qt-based GUIs with -DbuildQt=ON")
message("Build deprecated programs with -DbuildDeprecated=ON")
message("Disable unit tests with -DbuildTests=OFF (run unit tests with make test)")
message("Specify OTPS install location with -DotpsDir=directory")
message("Print out all CMake variables with -Dverbose=1")
message("----------------------------------------------------------")

# Define minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Define project
project(MB-System 
        DESCRIPTION "Open Source software for the processing and display of seafloor mapping data"
        HOMEPAGE_URL https://www.mbari.org/technology/mb-system/ 
        LANGUAGES C CXX )

# Disallow in-source builds
get_filename_component (srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component (bindir "${CMAKE_BINARY_DIR}" REALPATH)
if (${srcdir} STREQUAL ${bindir})
  message(FATAL_ERROR "In-source builds are not allowed. "
  "Please create a directory and run cmake from there, passing the path "
  "to this source directory as the last argument. This process created "
  "the file `CMakeCache.txt' and the directory `CMakeFiles' in ${srcdir}. "
  "Please remove them.")
endif (${srcdir} STREQUAL ${bindir})

add_compile_definitions(CMAKE_BUILD_SYSTEM=1)
if (CMAKE_C_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBYTESWAPPED")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBYTESWAPPED")
endif()

option(buildGSF "build GSF library" ON)
option(buildGUIs "build graphical tools" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
option(BUILD_SHARED_LIBS "Build and link with shared libraries" ON)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_MACOSX_RPATH ON)

option(buildGSF "build GSF library" ON)
option(buildGUIs "build graphical tools" ON)
option(buildOpenCV "build OpenCV tools" ON)
option(buildTRN "build MBTRN tools" ON)
option(buildTRNLCM "build MBTRN LCM tools" OFF)
option(buildPCL "build point-cloud-library" OFF)
option(buildQt "build Qt tools" OFF)
option(buildTests "build unit tests exercised by make test" ON)
if (DEFINED CACHE{otpsDir})
  set(otpsDir $CACHE{otpsDir})
else()
  set(otpsDir "/usr/local/src/otps")
endif()

set(CMAKE_INSTALL_SHARE share) 
set(CMAKE_INSTALL_MANPAGES share/man)
set(CMAKE_INSTALL_DOC share/doc)
set(levitusDir "share/mbsystem")

include(GNUInstallDirs)
include(CTest)

# uninstall target
if(NOT TARGET uninstall)
  add_custom_target(uninstall COMMAND xargs rm -vf < install_manifest.txt)
endif()

# Libraries
if(buildGSF)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_GSF")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_GSF")
  add_subdirectory(src/gsf)
endif()
add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

# GMT modules
add_subdirectory(src/gmt)

# Executables
add_subdirectory(src/utilities)
add_subdirectory(src/macros)
add_subdirectory(src/mbgrd2gltf)
add_subdirectory(src/otps)
if(buildGUIs)
  add_subdirectory(src/mbview)
  add_subdirectory(src/mbedit)
  add_subdirectory(src/mbgrdviz)
  add_subdirectory(src/mbeditviz)
  add_subdirectory(src/mbnavadjust)
  add_subdirectory(src/mbnavedit)
  add_subdirectory(src/mbvelocitytool)
endif()
if(buildOpenCV)
  add_subdirectory(src/photo)
endif()
if(buildTRN)
  add_subdirectory(src/mbtrnframe)
  add_subdirectory(src/mbtrn)
  add_subdirectory(src/mbtrnav)
  add_subdirectory(src/mbtrnutils)
  if(buildTRNLCM)
    add_subdirectory(src/mbtrnav/opt/rov)
  endif()
endif()
if(buildQt)
  add_subdirectory(src/qt-guilib)
  add_subdirectory(src/qt-mbgrdviz)
  add_subdirectory(src/qt-mbgrdviz-3)  
  add_subdirectory(src/qt-mbedit)
  add_subdirectory(src/qt-mbedit-widgets)
  add_subdirectory(src/qt-mbnavedit)
  ### add_subdirectory(src/qt-test)
  ### add_subdirectory(src/qt-qqvtkRenderItem)
  ### add_subdirectory(src/qt-qqvtkItem)
  ### add_subdirectory(src/qt-mbeditviz)
  
endif()
if(buildDeprecated)
  add_subdirectory(src/deprecated)
endif()

if(buildTests)
  add_subdirectory(third_party)
  add_subdirectory(test/mbio)
  add_subdirectory(test/utilities)
  if(buildDeprecated)
    add_subdirectory(test/deprecated)
  endif()
endif()

add_subdirectory(src/man)
add_subdirectory(src/html)
add_subdirectory(src/share)


message("----------------------------------------------------------")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_INSTALL_FULL_LIBDIR: ${CMAKE_INSTALL_FULL_LIBDIR}")
message("CMAKE_INSTALL_FULL_INCLUDEDIR: ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message("CMAKE_INSTALL_FULL_BINDIR: ${CMAKE_INSTALL_FULL_BINDIR}")
message("CMAKE_INSTALL_FULL_DOCDIR: ${CMAKE_INSTALL_FULL_DOCDIR}")
message("CMAKE_INSTALL_FULL_DATADIR: ${CMAKE_INSTALL_FULL_DATADIR}")
message("CMAKE_INSTALL_FULL_DATAROOTDIR: ${CMAKE_INSTALL_FULL_DATAROOTDIR}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
message("CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message("BYTESWAPPED: ${BYTESWAPPED}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")
message("CMAKE_MACOSX_RPATH: ${CMAKE_MACOSX_RPATH}")

# For debugging: print all set variables
message("----------------------------------------------------------")

if (verbose)
  message("All CMake variables:")
  message(" ")
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
  message("----------------------------------------------------------")
endif()
