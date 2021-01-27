# Accepts the following _INCLUDE_DIR and _LIBRARY "-D" options on the command
# line:
# GMT_
# PROJ_
# OPENGL_
# MOTIF_
# FFT_
# OTPS_
# OPENCV_
# GDAL
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

message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")

# Include my functions (these are not macros)
include(findLibraryDir)
include(findHeaderPath)
include(FindNetCDF)
include(FindGMT)
include(FindProj)
include(FindOpenCV)

# Find required packages
# Not all the Find modules we use conform to the variable-naming rules
# described at
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html

# FindNetCDF.cmake sets output NetCDF_INCLUDE_DIRS (plural name),
# doesn't set NetCDF_INCLUDE_DIR or NETCDF_INCLUDE_DIR
find_package("NetCDF")
if (NetCDF_FOUND)
  message("NetCDF_INCLUDE_DIRS: ${NetCDF_INCLUDE_DIRS}")
  message("NetCDF_LIBRARIES: ${NetCDF_LIBRARIES}")
else()
  message("warning: NetCDF not found")
endif()

# FindOpenCV doesn't set OPENCV_INCLUDE_DIR

# These Find modules all casefold package name to uppercase in output variable
# names, so we process them as a list
set(packages GMT Proj OpenGL X11 MOTIF GDAL)
message("process these: ${packages}")
foreach(package ${packages})
  message("Process package ${package}")
  find_package(${package})
  # find_package() output variables typically uppercase
  string(TOUPPER ${package} prefix)  
  if (${prefix}_FOUND)
    message("found ${package} - ${prefix}_INCLUDE_DIR: ${${prefix}_INCLUDE_DIR}")
    message("found ${package} - ${prefix}_LIBRARY: ${${prefix}_LIBRARY}")  

    # Make library variable name consistent
    if (${prefix}_LIBRARY)
      set(${prefix}_LIBRARIES ${${prefix}_LIBRARY})
    endif()
  else()
    message("${package} not found")
  endif()
  
endforeach()

message("Done finding packages")

# Make NetCDF variable names consistent
set(NETCDF_INCLUDE_DIR ${NetCDF_INCLUDE_DIRS})
set(NETCDF_LIBRARY ${NetCDF_LIBRARIES})

# Verify that OS is supported
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS")
endif()

message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("PROJ_INCLUDE_DIR: ${PROJ_INCLUDE_DIR}")
message("GDAL_INCLUDE_DIR: ${GDAL_INCLUDE_DIR}")
message("OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message("X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message("MOTIF_INCLUDE_DIR: ${MOTIF_INCLUDE_DIR}")
message("NETCDF_INCLUDE_DIR: ${NETCDF_INCLUDE_DIR}")

message("GMT_LIBRARIES: ${GMT_LIBRARIES}")
message("PROJ_LIBRARIES: ${PROJ_LIBRARIES}")
message("GDAL_LIBRARIES: ${GDAL_LIBRARIES}")
message("OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message("X11_LIBRARIES: ${X11_LIBRARIES}")
message("MOTIF_LIBRARIES: ${MOTIF_LIBRARIES}")
message("NETCDF_LIBRARIES: ${NETCDF_LIBRARIES}")


# Utilities and libraries
add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/mbview)
add_subdirectory(src/gsf)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

# Executables
add_subdirectory(src/mbgrdviz)
