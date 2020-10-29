# Accepts the following _INCLUDE_DIR and _LIB_DIR "-D" options on the command line:
# GMT_
# PROJ_
# OPENGL_
# VTK_
# MOTIF_
# FFT_
# OTPS_
# OPENCV_
# 
# For Qt we currently rely on qmake and .pro files to determine Qt header
# and library location

cmake_minimum_required(VERSION 3.18)
project(MB-System)


# Add globally-needed compiler flags
add_compile_options(-fPIC -DPIC -g)

# Get directory of this CMakeLists.txt file
set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})
message("MB_HOME: ${MB_HOME}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")

# Run script
set(LIBNAME PROJ)
set(HEADERNAME proj.h)
set(HEADERSUBDIR proj)
include(findLibrary)
if (NOT PROJ_FOUND)
  message("PROJ not found")
else()
  message("Found PROJ")
endif()


# Include my functions
include(foo)
include(findLibraryDir)
include(findCIncludeDir)
include(findCXXIncludeDir)

message("executing foo()")
foo()
message("returned from foo()")

# Set search paths for includes, libs based on host OS
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS")
endif()

if (NOT GMT_INCLUDE_DIR)
  # Look for includes in usual places
  findCIncludeDir(gmt GMT_INCLUDE_DIR)
endif()


if (NOT VTK_INCLUDE_DIR)
  # Look for includes in usual places
  findCXXIncludeDir(vtk-8.2 VTK_INCLUDE_DIR)
endif()

if (NOT PROJ_INCLUDE_DIR)
  # Look for includes in usual places
  findCXXIncludeDir(proj PROJ_INCLUDE_DIR)
endif()

if (NOT OPENGL_INCLUDE_DIR)
  # Look for includes in usual places
  findCIncludeDir(GL OPENGL_INCLUDE_DIR)
endif()


if (NOT VTK_LIB_DIR)
  # Look for library in usual places
  findLibraryDir(vtkIOCore-8.2 VTK_LIB_DIR)
endif()

if (NOT GMT_LIB_DIR)
  # Look for library in usual places
  findLibraryDir(gmt GMT_LIB_DIR)
endif()


message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("VTK_INCLUDE_DIR: ${VTK_INCLUDE_DIR}")
message("PROJ_INCLUDE_DIR: ${PROJ_INCLUDE_DIR}")
message("OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message("GMT_LIB_DIR: ${GMT_LIB_DIR}")
message("VTK_LIB_DIR: ${VTK_LIB_DIR}")


# Libraries
add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/mbview)
add_subdirectory(src/gsf)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

# Executables
add_subdirectory(src/mbgrdviz)
