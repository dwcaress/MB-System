# Accepts the following _INCLUDE_DIR and _LIB_DIR "-D" options on the command
# line:
# GMT_
# PROJ_
# OPENGL_
# VTK_
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

### TEST TEST TEST
set(packages GMT PROJ OpenGL VTK MOTIF OPENCV GDAL)
message("process these: ${packages}")
foreach(package ${packages})
  message("Process package ${package}")
endforeach()
message("Done processing packages")

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

# Verify that OS is supported
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS")
endif()


# Look for gmt 'C' headers in implicit C include directories
message("Look for gmt headers")
findHeaderPath(gmt GMT_INCLUDE_DIR)
message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")

# Look for vtk C++ headers in implicit C++ include directories
findHeaderPath(vtk-9.0 VTK_INCLUDE_DIR)

# Look for proj C++ headers in implicit C++ include directories
findHeaderPath(proj PROJ_INCLUDE_DIR)

# Look for gdal C++ headers in implicit C++ include directories
findHeaderPath(gdal.h GDAL_INCLUDE_DIR)
message("findHeaderPath() found ${GDAL_INCLUDE_DIR}")
find_package(GDAL)
if (GDAL_FOUND)
  message("find_package() found GDAL; headers at ${GDAL_INCLUDE_DIR}")
else()
  message("find_package() did not find GDAL")
endif()

# Look for OpenGL 'C' headers in implicit C include directories  
findHeaderPath(GL OPENGL_INCLUDE_DIR)

# Look for X11 'C' headers in implicit C include directories  
findHeaderPath(X11 X11_INCLUDE_DIR)
message("findHeaderPath() found ${X11_INCLUDE_DIR}")
find_package(X11)
if (X11_FOUND)
  message("find_package() found X11; headers at ${X11_INCLUDE_DIR}")
else()
  message("find_package() did not find X11")
endif()

# Look for Motif 'C' headers in implicit C include directories  
findHeaderPath(Xm MOTIF_INCLUDE_DIR)

# Look for netcdf 'C' headers in implicit C include directories  
findHeaderPath(netcdf.h NETCDF_INCLUDE_DIR)
find_package(NetCDF)
if (NetCDF_FOUND)
  message("find_package() found NetCDF; headers at ${NetCDF_INCLUDE_DIRS}")
else()
  message("find_package() did not find NetCDF")
endif()



# Look for library in usual places
findLibraryDir(vtkIOCore-9.0 VTK_LIB_DIR)

# Look for library in usual places
findLibraryDir(gmt GMT_LIB_DIR)

message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("VTK_INCLUDE_DIR: ${VTK_INCLUDE_DIR}")
message("PROJ_INCLUDE_DIR: ${PROJ_INCLUDE_DIR}")
message("GDAL_INCLUDE_DIR: ${GDAL_INCLUDE_DIR}")
message("OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message("X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message("MOTIF_INCLUDE_DIR: ${MOTIF_INCLUDE_DIR}")
message("NETCDF_INCLUDE_DIR: ${NETCDF_INCLUDE_DIR}")
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
