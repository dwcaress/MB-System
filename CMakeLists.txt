# Accepts the following _INCLUDE and _LIBS "-D" options on the command line:
# GMT_
# PROJ_
# OPENGL_
# QT_
# VTK_

cmake_minimum_required(VERSION 3.18)
project(MB-Sytstem)

# Find C include directory with specified name
function(findCIncludeDir directory var)
  message("***findCIncludeDir(): look for ${directory} in ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
  find_file(tmp ${directory}
            PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
            NO_DEFAULT_PATH
            REQUIRED)

message("***Found ${directory}: ${tmp}")
set(${var} ${tmp} PARENT_SCOPE)
  
endfunction()


# Find C++ include directory with specified name
function(findCXXIncludeDir directory var)
  message("***findCXXIncludeDir(): look for ${directory}")
  find_file(tmp ${directory}
            PATHS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
            NO_DEFAULT_PATH
            REQUIRED)

  message("***Found ${directory}: ${tmp}")  
  set(${var} ${tmp} PARENT_SCOPE)
  
endfunction()


# Look for specified library, set var to corresponding directory
function(findLibraryDir library var)
  message("findLibraryDir() - look for ${library}")
  find_library(LIBFILE ${library} REQUIRED)
  message("findLibraryDir(); set var ${var}")  
  get_filename_component(tmp ${LIBFILE} DIRECTORY)
  set(${var} ${tmp} PARENT_SCOPE)
endfunction()

set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})

message("MB_HOME: ${MB_HOME}")

# Add build-utils to module path
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")

# Use functions in cmakeUils.cmake
###include(cmakeUtils.cmake)


message("CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
message("CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")

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

if (NOT GMT_LIB_DIR)
  # Look for library in usual places
  ### find_library(GMT_LIB_DIR gmt REQUIRED)
  findLibraryDir(gmt GMT_LIB_DIR)
  message("returned from findLibraryDir() - GMT_LIB_DIR: ${GMT_LIB_DIR}")  
endif()

if (NOT VTK_INCLUDE_DIR)
  # Look for includes in usual places
  findCXXIncludeDir(vtk-8.2 VTK_INCLUDE_DIR)
endif()

#[[
if (NOT VTK_LIBS)
  # Look for library in usual places
  find_library(VTK_LIBS vtk REQUIRED)  
endif()
]]

message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("GMT_LIB_DIR: ${GMT_LIB_DIR}")
message("VTK_INCLUDE_DIR: ${VTK_INCLUDE_DIR}")


add_subdirectory(src/mbaux)
