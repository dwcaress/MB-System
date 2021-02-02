# Find prerequesite packages and build Makefiles
#
# 
# For Qt apps we currently rely on qmake and .pro files to determine Qt header
# and library location

cmake_minimum_required(VERSION 3.18)
project(MB-System)

# Set list of required packages
set(packages GMT Proj OpenGL X11 Xt Motif GDAL NetCDF FFTW)

if (help)
  message("force package include directory with -Dpackage_include=path")
  message("force package libraries with -Dpackage_lib=file")  
  message("where package is one of the following:")
  message("${packages}")
  message(FATAL_ERROR "")
endif()

# Add globally-needed compiler flags
add_compile_options(-DUSE_DEFAULT_FILE_FUNCTIONS -fPIC -DPIC -MD -MP -MF  -fno-common -g)

# C-only flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

# C++-only flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


# Get directory of this CMakeLists.txt file
set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})
message("MB_HOME: ${MB_HOME}")

message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")

# Find required packages
message("ok, find packages ${packages}")
foreach(package ${packages})
  include(Find${package})
  string(TOUPPER ${package} PACKAGE)
  if (${package}_FOUND OR ${PACKAGE}_FOUND)
    if (${PACKAGE}_LIBRARY)
      message("rename ${PACKAGE}_LIBRARY (val=${${PACKAGE}_LIBRARY})")
      set(${PACKAGE}_LIBRARIES ${${PACKAGE}_LIBRARY})
    endif()
  else()
    # Check to see if specified on command line
    if (${package}_include AND ${package}_lib)
      continue()
    endif()
    message(FATAL_ERROR
            "Package ${package} not found\n"
            "Specify -D${package}_include=dir and -D${package}_lib=file on cmake command line")

  endif()
endforeach()

message("Done finding packages")
# Not all the Find modules we use conform to the variable-naming rules
# described at
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
# Make names consistent...
set(X11_XT_INCLUDE_DIR ${X11_Xt_INCLUDE_PATH})
set(X11_XT_LIBRARIES ${X11_Xt_LIB})

# Make NetCDF variable names consistent
set(NETCDF_INCLUDE_DIR ${NetCDF_INCLUDE_DIRS})
set(NETCDF_LIBRARIES ${NetCDF_LIBRARIES})

# Override header and library locations if specified on command line
# with -D<package>_include or -D<package>_lib.
message("override header and library locations if specified on command line")
foreach(package ${packages})
  if (${package}_include)
    message("override ${package} include directory with ${${package}_include}")
    string(TOUPPER ${package} PACKAGE)
    set(${PACKAGE}_INCLUDE_DIR ${${package}_include})
  endif()
  if (${package}_lib)
    message("override ${package} library with ${${package}_lib}")
    string(TOUPPER ${package} PACKAGE)
    set(${PACKAGE}_LIBRARIES ${${package}_lib})
endif()
endforeach()

# Verify that OS is supported
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS")
endif()


message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("FFTW_INCLUDE_DIR: ${FFTW_INCLUDE_DIR}")
message("PROJ_INCLUDE_DIR: ${PROJ_INCLUDE_DIR}")
message("GDAL_INCLUDE_DIR: ${GDAL_INCLUDE_DIR}")
message("OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message("X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message("MOTIF_INCLUDE_DIR: ${MOTIF_INCLUDE_DIR}")
message("NETCDF_INCLUDE_DIR: ${NETCDF_INCLUDE_DIR}")
message("X11_XT_INCLUDE_DIR: ${X11_XT_INCLUDE_DIR}")

message("GMT_LIBRARIES: ${GMT_LIBRARIES}")
message("FFTW_LIBRARIES: ${FFTW_LIBRARIES}")
message("PROJ_LIBRARIES: ${PROJ_LIBRARIES}")
message("GDAL_LIBRARIES: ${GDAL_LIBRARIES}")
message("OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message("X11_LIBRARIES: ${X11_LIBRARIES}")
message("MOTIF_LIBRARIES: ${MOTIF_LIBRARIES}")
message("NETCDF_LIBRARIES: ${NETCDF_LIBRARIES}")
message("X11_XT_LIBRARIES: ${X11_XT_LIBRARIES}")

# MacOS requires all referred library symbols be specified when
# building shared dylib
if (APPLE)
  list(APPEND EXTRA_LIBRARIES ${GMT_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${FFTW_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${PROJ_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${GDAL_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${OPENGL_LIBRARIES})  
  list(APPEND EXTRA_LIBRARIES ${MOTIF_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${NETCDF_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${X11_LIBRARIES})      
  list(APPEND EXTRA_LIBRARIES ${X11_XT_LIBRARIES})
  message("EXTRA_LIBRARIES: ${EXTRA_LIBRARIES}")
else()
  set(EXTRA_LIBRARIES "")
endif()


# Utilities and libraries
add_subdirectory(src/gsf)
add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/mbview)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

# Executables
add_subdirectory(src/mbgrdviz)
add_subdirectory(src/utilities)
