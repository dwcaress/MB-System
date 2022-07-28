#[[
Find prerequesite packages and build Makefiles

For Qt apps we currently rely on qmake and .pro files to determine Qt header
and library location
]]

cmake_minimum_required(VERSION 3.18)
project(MB-System)

# Verify that OS is supported
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS ${CMAKE_SYSTEM_NAME}")
endif()

# Basics
option(buildGUIs "build graphical tools" ON)
option(buildOpenCV "build OpenCV tools" ON)

option(buildPCL "build point-cloud-library" OFF)
option(buildTRN "build MBTRN tools" OFF)
option(buildTNav "build MBTNav tools" OFF)
option(buildQt "build Qt tools" OFF)
option(buildGSF "build GSF library" OFF)
option(buildTest "build test tools" OFF)


# Set list of required packages - DO NOT CHANGE NAMES!
set(packages GMT PROJ netCDF GDAL FFTW OpenGL X11 Motif)

if (buildQt)
  # Add Qt, VTK to packages
  message("Build Qt/VTK apps")  
  list(APPEND packages "Qt" "VTK")
else()
  message("Do not build Qt/VTK apps")
endif()

if (buildPCL)
  list(APPEND packages "PCL")
endif()



if (help)

  message("specify OTPS install location with -DotpsDir=directory")
  message("set install directory with -DinstallDir=directory")  
  message("force package include directory with -Dpackage_include=path")
  message("force package libraries with -Dpackage_lib=file")  
  message("where package is one of the following:")
  message("${packages} ${packages}")
  message("enable debugging with -DCMAKE_BUILD_TYPE=Debug")
  message("build Qt-based GUIs with -DbuildQt=1")
  message("build TRN software with -DbuildTRN=1")
  message("build shared/static libraries with -DBUILD_SHARED_LIBS=ON/OFF")
  message(FATAL_ERROR "")
endif()

option(BUILD_SHARED_LIBS "Build and link with shared libraries" ON)

if (APPLE)

  # Assume MacPorts default install location in /opt/local
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /opt/local/include)
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

  # Must set PROJ_ROOT to point at directory containing proj-versionX
  # so that find_package() works
  # MacPorts puts projX files in /opt/local/lib/projX
  set(PROJ_ROOT /opt/local/lib/proj9)
  set(GMT_ROOT /opt/local/lib/gmt6) # Required for FindGMT()


  #[[ find_package(X11) appears reliable on MacOS

  if (NOT X11_include)
    message("assume X11_include in /opt/X11/include")
    set(X11_include /opt/X11/include)
  endif()

  if (NOT X11_lib)
    message("assume X11_lib in /opt/X11/lib")
    set(X11_lib /opt/X11/lib/libXt.dylib;/opt/X11/lib/libX11.dylib)
  endif()
  ]]

  # OSX OpenGL support is not very reliable or consistent.
  # By default specific /opt/local for OpenGL includes and libs,
  # unless user specifies location on command line

  if (NOT OpenGL_include)
    message("assume OpenGL_include in /opt/local/include")  
    set(OpenGL_include /opt/local/include)  
  endif()
  
  if (NOT OpenGL_lib)
    message("assume OpenGL_lib in /opt/local/lib")
    set(OpenGL_lib /opt/local/lib/libGL.dylib;/opt/local/lib/libGLU.dylib)
  endif()

  set(otpsDir /usr/local/opt/otps)

else()
  # Not MacOS
  set(otpsDir /local/opt/otps)
endif()

#[[
NOTE: VTK_lib is specified as a space-delimited list of libraries, and 
so those spaces must be quoted when specified with cmake -D command line
option. E.g. suppose $VTK_LIBS is a bash shell variable. Then specify 
VTK_lib to cmake like this:
   % cmake -DVTK_lib="$VTK_LIBS" ..

Note the double-quotes around VTK_LIBS, and note that -D options must
precede the final argument that specifies location of the CMakeLists.txt
file, which is .. in this case.
]]

# Add globally-needed compiler flags
add_compile_options(-DUSE_DEFAULT_FILE_FUNCTIONS)

if (CMAKE_COMPILER_IS_GNUCC)
  message("Using GNUCC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
else()
  message("Not using GNUCC")
  # Setting these flags results in compile error S_IFDIR undeclared symbol,
  # in gsf/gsf_indx.c
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED ON)
  set(CMAKE_C_EXTENSIONS OFF)
endif()

message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

if (installDir)
  set(CMAKE_INSTALL_PREFIX ${prefix})
endif()

# Get directory of this CMakeLists.txt file
set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})
message("MB_HOME: ${MB_HOME}")

message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")


# Find required packages; start with CONFIG package search, fall-back
# to MODULE search
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

message("find packages: ${packages}")
foreach(package ${packages})
  message("look for ${package}")
  # No need to search, if package's include and lib are specified as
  # command line '-D' arguments

  if (${package}_include AND ${package}_lib)
    message("${package} include and lib are pre-specified")
    continue()
  endif()

  # Try to find package
  set(packageFound FALSE)

  if (${package} STREQUAL "Qt")
    # Qt handled a bit differently
    # This will need to change; see
    # https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html#manual:cmake-qt(7)
    find_package(Qt5 COMPONENTS Gui Quick REQUIRED)

    if (Qt5_FOUND)
      message("Found Qt5 package in ${Qt5_DIR}")
      set(QT5_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
      list(APPEND QT5_INCLUDE_DIRS ${Qt5Quick_INCLUDE_DIRS})
      set(QT5_LIBRARIES ${Qt5Gui_LIBRARIES})
      list(APPEND QT5_LIBRARIES ${Qt5Quick_LIBRARIES})
    else()
       message(FATAL_ERROR "Qt5 package not found")
    endif()
    
    continue()

  endif()

  if (${package} STREQUAL "VTK")
    message("TRY find_package(VTK 8.2.0)")
    find_package(VTK "8.2.0" EXACT REQUIRED)
    find_package(VTK REQUIRED)
    continue()    
  endif()

  # Most packages handled by the following
  message("TRY find_package(${package})")
  find_package(${package})
  message("BACK FROM find_package(${package})")
  
  if (${package}_FOUND)
    message("Found ${package}")
    set(packageFound TRUE)
    message("${package}_VERSION: ${${package}_VERSION}")    
    ### message("${package}_INCLUDE_DIRS: ${${package}_INCLUDE_DIRS}")
    ### message("${package}_LIBRARIES: ${${package}_LIBRARIES})")

    if (${package} STREQUAL "netCDF")
       # Make NetCDF variable names all upper case
       set(NETCDF_INCLUDE_DIRS ${${package}_INCLUDE_DIRS})
       set(NETCDF_LIBRARIES ${${package}_LIBRARIES})
       set(NETCDF_VERSION ${${package}_VERSION})       

    elseif (${package} STREQUAL "OpenGL")
      # find_package(OpenGL) returns variables in all upper case
      set(OPENGL_INCLUDE_DIRS ${${package}_INCLUDE_DIR})
      
    elseif (${package} STREQUAL "X11")
      # find_package(X11) returns X11_INCLUDE_DIR
      set(X11_INCLUDE_DIRS ${${package}_INCLUDE_DIR})

    elseif (${package} STREQUAL "Motif")
      # find_package(Motif) returns all upper case, MOTIF_INCLUDE_DIR
      message("package Motif: ${package}_INCLUDE_DIR: ${${package}_INCLUDE_DIR}")
      set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
            
    endif()


  endif()


  if (NOT ${packageFound})
    message(FATAL_ERROR
            "Couldn't find ${package}\n"
            "Specify -D${package}_include=dir and -D${package}_lib=file "
            "on cmake command line")
  endif()


endforeach()


### Print all accessible variables
### get_cmake_property(_variableNames VARIABLES)
### list (SORT _variableNames)
### foreach (_variableName ${_variableNames})
###    message(STATUS "${_variableName}=${${_variableName}}")
### endforeach()

message("Done finding packages")
# Not all the Find packages we use conform to the variable-naming rules
# described at
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
# Make names consistent...
set(X11_XT_INCLUDE_DIRS ${X11_Xt_INCLUDE_PATH})
set(X11_XT_LIBRARIES ${X11_Xt_LIB})



# Override header and library locations if specified on command line
# with -D<package>_include or -D<package>_lib.
message("override header and library locations if specified on command line")
message("APPENDED packages to packages, now: ${packages}")
# Set variables ${PACKAGE}_INCLUDE_DIRS and ${PACKAGE}_LIBRARIES for each required
# PACKAGE
foreach(package ${packages})
  if (${package}_include)
    message("override ${package} include directory with ${${package}_include}")
    string(TOUPPER ${package} PACKAGE)
    set(${PACKAGE}_INCLUDE_DIRS ${${package}_include} CACHE FILE "${package} header directory")
  endif()
  if (${package}_lib)
    message("override ${package} library with ${${package}_lib}")
    string(TOUPPER ${package} PACKAGE)
    set(${PACKAGE}_LIBRARIES ${${package}_lib} CACHE FILE "${package} library(ies)")
endif()
endforeach()

# Print package variables
message("package variables:")
foreach(package ${packages})

  set(varName ${package}_VERSION)
  string(TOUPPER ${varName} VARNAME)
  message("${VARNAME}: ${${VARNAME}}")
  
  set(varName ${package}_INCLUDE_DIRS)
  string(TOUPPER ${varName} VARNAME)
  message("${VARNAME}: ${${VARNAME}}")

  set(varName ${package}_LIBRARIES)
  string(TOUPPER ${varName} VARNAME)
  message("${VARNAME}: ${${VARNAME}}")

  set(varName ${package}_DIR)
  string(TOUPPER ${varName} VARNAME)
  message("${VARNAME}: ${${VARNAME}}")
  message("${varName}: ${${varName}}")
  
endforeach()

message("Assume OPTS is installed in ${otpsDir}; override with -DotpsDir=dir")

# Build a few needed header files
message("call buildHeaders(${otpsDir} ${CMAKE_INSTALL_PREFIX}/share/mbsystem)")
include(buildHeaders)
buildHeaders(${otpsDir} ${CMAKE_INSTALL_PREFIX}/share/mbsystem)

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
  list(APPEND EXTRA_LIBRARIES ${QT5_LIBRARIES})
  list(APPEND EXTRA_LIBRARIES ${VTK_LIBRARIES})
  
  message("EXTRA_LIBRARIES: ${EXTRA_LIBRARIES}")
else()

endif()

# Use standard cache variables for installation
include(GNUInstallDirs)

# Utilities and libraries
if (buildGSF)
   message("building GSF")
   add_subdirectory(src/gsf)
   set(GSF_LIB mbgsf)
   set(MBIO_GSF_SRC "mbsys_gsf.c mbr_gsfgenmb.c")
else()
   message("NOT building GSF")
   set(GSF_LIB "")
   set(MBIO_GSF_SRC "")
endif(buildGSF)

add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/mbview)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)


if (buildTRN)
   add_subdirectory(src/mbtrn)
   
   # Some executables get linked with TRN libraries in this case
   set(R7KR_LIB r7kr)
   set(MFRAME_LIB mframe)
else()
   # Not building TRN libraries
   set(R7KR_LIB "")
   set(MFRAME_LIB "")   
endif(buildTRN)


# Executables
if (buildGUIs)
   add_subdirectory(src/mbedit)
   add_subdirectory(src/mbgrdviz)
   add_subdirectory(src/mbeditviz)
   add_subdirectory(src/mbnavadjust)
   add_subdirectory(src/mbnavedit)
   add_subdirectory(src/mbvelocitytool)
endif(buildGUIs)

add_subdirectory(src/utilities)

if (buildQt)
  add_subdirectory(src/qt-guilib)
  add_subdirectory(src/qt-mbgrdviz)
endif(buildQt)

# TESTING
include(buildConfigHeader)

buildConfigHeader(${buildGSF} ${buildTRN} ${buildTNav}
                              ${buildGUIs} ${buildTest})

message("DONE")
