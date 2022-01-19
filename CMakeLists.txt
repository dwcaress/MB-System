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

# Set list of required modules (i.e. packages with find modules)
set(modules GMT Proj OpenGL X11 Motif GDAL NetCDF FFTW)

if (help)
  message("specify OPTS install location with -DoptsDir=directory")
  message("set install prefix with -Dprefix=directory")  
  message("force package include directory with -Dpackage_include=path")
  message("force package libraries with -Dpackage_lib=file")  
  message("where package is one of the following:")
  message("${modules} ${packages}")
  message("enable debugging with -DCMAKE_BUILD_TYPE=Debug")
  message("built Qt-based GUIs with -DbuildQt=1")
  message("built TRN software with -DbuildTrn=1")  
  message("you MUST specify -DVTK_include=<dir> -DVTK_lib=<libs>")
  message(FATAL_ERROR "")
endif()


if (APPLE)

  # On OSX we assume X11 and OpenGL are provided by XQuartz in /opt/X11,
  # unless user specifies location on command line
  message("MACOS: assume X11 and OpenGL are in /opt/X11")

  if (NOT X11_include)
    message("assume X11_include in /opt/X11/include")
    set(X11_include /opt/X11/include)
  endif()

  if (NOT X11_lib)
    message("assume X11_lib in /opt/X11/lib")
    set(X11_lib /opt/X11/lib/libXt.dylib;/opt/X11/lib/libX11.dylib)
  endif()

  if (NOT OpenGL_include)
    message("assume OpenGL_include in /opt/X11/include")  
    set(OpenGL_include /opt/X11/include)  
  endif()
  
  if (NOT OpenGL_lib)
    message("assume OpenGL_lib in /opt/X11/lib")
    set(OpenGL_lib /opt/X11/lib/libGL.dylib;/opt/X11/lib/libGLU.dylib)
  endif()

  set(otpsDir /usr/local/opt/otps)

else()
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


if (prefix)
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

# Find required modules
message("find modules: ${modules}")
foreach(module ${modules})
  # No need to search, if module's include and lib are specified as
  # command line '-D' arguments
  if (${module}_include AND ${module}_lib)
    message("${module} include and lib are pre-specified")
    continue()
  endif()

  if (NOT ${module}_include OR  NOT ${module}_lib)
    # Try to find module
    message("include Find${MODULE}")
    include(Find${module})
    string(TOUPPER ${module} MODULE)
    if (${module}_FOUND OR ${MODULE}_FOUND)
      if (${MODULE}_LIBRARY)
        message("rename ${MODULE}_LIBRARY")
        set(${MODULE}_LIBRARIES ${${MODULE}_LIBRARY})
      endif()
    else()
      message(FATAL_ERROR
              "Module ${module} not found\n"
              "Specify -D${module}_include=dir and -D${module}_lib=file "
              "on cmake command line")

    endif()
  endif()
  
endforeach()

if (buildQt)
  # Look for packages that don't have modules
  if (NOT Qt5_include OR NOT Qt5_lib)
    # This will need to change; see
    # https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html#manual:cmake-qt(7)
    find_package(Qt5 COMPONENTS Gui Quick REQUIRED)
    if (Qt5_FOUND)
      message("Found Qt5 package")
      set(QT5_INCLUDE_DIR ${Qt5Gui_INCLUDE_DIRS})
      list(APPEND QT5_INCLUDE_DIR ${Qt5Quick_INCLUDE_DIRS})
      set(QT5_LIBRARIES ${Qt5Gui_LIBRARIES})
      list(APPEND QT5_LIBRARIES ${Qt5Quick_LIBRARIES})
    else()
      message(FATAL_ERROR "Qt5 package not found")
    endif()
  endif()

  if (VTK_include)
    set(VTK_INCLUDE_DIR ${VTK_include})
  endif()

  if (VTK_lib)
    set(VTK_LIBRARIES ${VTK_lib})
  endif()

  if (NOT VTK_include OR NOT VTK_lib)
    find_package(VTK REQUIRED)
    if (VTK_FOUND)
      message("Found VTK package")
    else()
      message("VTK package not found")
    endif()
  endif()

endif(buildQt)  

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
set(X11_XT_INCLUDE_DIR ${X11_Xt_INCLUDE_PATH})
set(X11_XT_LIBRARIES ${X11_Xt_LIB})

# Make NetCDF variable names consistent
set(NETCDF_INCLUDE_DIR ${NetCDF_INCLUDE_DIRS})
set(NETCDF_LIBRARIES ${NetCDF_LIBRARIES})

# Override header and library locations if specified on command line
# with -D<package>_include or -D<package>_lib.
message("override header and library locations if specified on command line")
message("APPENDED modules to packages, now: ${packages}")
# Set variables ${MODULE}_INCLUDE_DIR an ${MODULE}_LIBRARIES for each required
# MODULE
foreach(module ${modules})
  if (${module}_include)
    message("override ${module} include directory with ${${module}_include}")
    string(TOUPPER ${module} MODULE)
    set(${MODULE}_INCLUDE_DIR ${${module}_include})
  endif()
  if (${module}_lib)
    message("override ${module} library with ${${module}_lib}")
    string(TOUPPER ${module} MODULE)
    set(${MODULE}_LIBRARIES ${${module}_lib})
endif()
endforeach()



message("GMT_INCLUDE_DIR: ${GMT_INCLUDE_DIR}")
message("FFTW_INCLUDE_DIR: ${FFTW_INCLUDE_DIR}")
message("PROJ_INCLUDE_DIR: ${PROJ_INCLUDE_DIR}")
message("GDAL_INCLUDE_DIR: ${GDAL_INCLUDE_DIR}")
message("OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message("X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message("MOTIF_INCLUDE_DIR: ${MOTIF_INCLUDE_DIR}")
message("NETCDF_INCLUDE_DIR: ${NETCDF_INCLUDE_DIR}")
message("X11_XT_INCLUDE_DIR: ${X11_XT_INCLUDE_DIR}")
message("VTK_INCLUDE_DIR: ${VTK_INCLUDE_DIR}")
message("QT5_INCLUDE_DIR: ${QT5_INCLUDE_DIR}")

message("GMT_LIBRARIES: ${GMT_LIBRARIES}")
message("FFTW_LIBRARIES: ${FFTW_LIBRARIES}")
message("PROJ_LIBRARIES: ${PROJ_LIBRARIES}")
message("GDAL_LIBRARIES: ${GDAL_LIBRARIES}")
message("OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message("X11_LIBRARIES: ${X11_LIBRARIES}")
message("MOTIF_LIBRARIES: ${MOTIF_LIBRARIES}")
message("NETCDF_LIBRARIES: ${NETCDF_LIBRARIES}")
message("X11_XT_LIBRARIES: ${X11_XT_LIBRARIES}")
message("VTK_LIBRARIES: ${VTK_LIBRARIES}")
message("QT5_LIBRARIES: ${QT5_LIBRARIES}")

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
  set(EXTRA_LIBRARIES "")
endif()

# Use standard cache variables for installation
include(GNUInstallDirs)

# Utilities and libraries
add_subdirectory(src/gsf)
add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/mbview)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)
if (buildTrn)
   add_subdirectory(src/mbtrn)
endif(buildTrn)

# Executables
add_subdirectory(src/mbgrdviz)
add_subdirectory(src/mbedit)
add_subdirectory(src/mbeditviz)
add_subdirectory(src/mbnavadjust)
add_subdirectory(src/mbnavedit)
add_subdirectory(src/mbvelocitytool)
add_subdirectory(src/utilities)

if (buildQt)
  add_subdirectory(src/qt-guilib)
  add_subdirectory(src/qt-mbgrdviz)
  add_subdirectory(src/qvtkTest)  
endif(buildQt)

