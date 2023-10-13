#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2023 by
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
#    See README file for copying and redistribution conditions.
#------------------------------------------------------------------------------
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

set(PROJECT_DATE "27 June 2023")

message("----------------------------------------------------------")
message("MB-System")
message("CMake Build System")
message("Johannes Schauer (University of Weurzburg")
message("Tom O'Reilly (Monterey Bay Aquarium Research Institute")
message("David Caress (Monterey Bay Aquarium Research Institute")
message("${PROJECT_DATE}")
message ("CMake version: ${CMAKE_VERSION}")
message("The default build mode is release (optimized for performance).")
message("  -To obtain a debug-able build invoke cmake with -DCMAKE_BUILD_TYPE=debug")
message("Specify OTPS install location with -DotpsDir=directory")
message("Specify OTPS install location with -DotpsDir=directory")
message("Set install directory with -DCMAKE_INSTALL_PREFIX=directory")
message("Enable debugging with -DCMAKE_BUILD_TYPE=Debug")
message("Build shared/static libraries with -DBUILD_SHARED_LIBS=ON/OFF")
message("Disable building TRN software with -DbuildTRN=OFF")
message("Disable building photomosaicing software with -DbuildOpenCV=OFF")
message("Build Qt-based GUIs with -DbuildQt=ON")
message("Build unit tests with -DbuildTests=ON")
message("Print out all CMake variables with -Dverbose=1")
message("----------------------------------------------------------")

# Define minimum CMake version required
cmake_minimum_required(VERSION 3.20)
project(MB-System 
        VERSION 5.7.9
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
  set(BYTESWAPPED 1)
else()
  set(BYTESWAPPED 0)
endif()

option(buildGSF "build GSF library" ON)
option(buildGUIs "build graphical tools" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
option(BUILD_SHARED_LIBS "Build and link with shared libraries" ON)

option(buildGSF "build GSF library" ON)
option(buildGUIs "build graphical tools" ON)
option(buildOpenCV "build OpenCV tools" ON)
option(buildTRN "build MBTRN tools" ON)
option(buildPCL "build point-cloud-library" OFF)
option(buildQt "build Qt tools" ON)
option(buildTests "build unit tests exercised by make check" OFF)

if(APPLE)
  set(CMAKE_INSTALL_MANPAGES /usr/local/share/man) 
  set(CMAKE_INSTALL_DOC /usr/local/share/doc) 
  set(CMAKE_INSTALL_SHARE /usr/local/share) 
else()
  set(CMAKE_INSTALL_MANPAGES /usr/local/share/man) 
  set(CMAKE_INSTALL_DOC /usr/local/share/doc) 
  set(CMAKE_INSTALL_SHARE /usr/local/share) 
endif()
if(APPLE)
  set(otpsDir /usr/local/opt/otps) 
else()
  set(otpsDir /local/opt/otps)
endif()
set(levitusDir ${CMAKE_INSTALL_PREFIX}/share/mbsystem)

message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
message("CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message("BYTESWAPPED: ${BYTESWAPPED}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


include(GNUInstallDirs)
include(CTest)

# uninstall target
if(NOT TARGET uninstall)
  add_custom_target(uninstall COMMAND xargs rm -vf < install_manifest.txt)
endif()

# Utilities and libraries
if(buildGSF)
  add_subdirectory(src/gsf)
endif()

add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

if(buildTRN)
  add_subdirectory(src/mbtrnframe)
  add_subdirectory(src/mbtrn)
  add_subdirectory(src/mbtrnav)
  add_subdirectory(src/mbtrnutils)
endif()

# Executables
if(buildGUIs)
  add_subdirectory(src/mbview)
  add_subdirectory(src/mbedit)
  add_subdirectory(src/mbgrdviz)
  add_subdirectory(src/mbeditviz)
  add_subdirectory(src/mbnavadjust)
  add_subdirectory(src/mbnavedit)
  add_subdirectory(src/mbvelocitytool)
endif()

add_subdirectory(src/utilities)
add_subdirectory(src/deprecated)
add_subdirectory(src/macros)

if(buildQt)
  add_subdirectory(src/qt-guilib)
  add_subdirectory(src/qt-mbgrdviz)
  add_subdirectory(src/qt-mbgrdviz-3)
endif()

if(buildOpenCV)
  add_subdirectory(src/photo)
endif()

if(buildTests)
  add_subdirectory(third_party)
  add_subdirectory(test/mbio)
  add_subdirectory(test/utilities)
  add_subdirectory(test/deprecated)
endif()

add_subdirectory(src/man)
add_subdirectory(src/html)
add_subdirectory(src/ps)
add_subdirectory(src/share)

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
