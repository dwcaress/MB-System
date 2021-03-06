cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Libmframe VERSION 0.1.0)

#################################
# define source directories
# utils only used for mbsystem builds
# which have a custom mconfig.h

set(MFRAME_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(UTILS_SRC_DIR ${PROJECT_SOURCE_DIR}/utils)

#################################
# pull in dependencies
# - libm
# - libpthread

find_library(MATH_LIB
NAMES
libm
libm.so
libm.dylib
libm.dll.a
libm.dll
PATHS
"/usr/lib64"
REQUIRED
)

find_library(PTHREAD_LIB
NAMES
libpthread
libpthread.so
libpthread.dylib
libpthread.dll.a
libpthread.dll
PATHS
"/usr/lib64"
REQUIRED
)

# define common libraries and include paths
list(APPEND EXTRA_LIBS
${MATH_LIB}
${PTHREAD_LIB}
)

include_directories(
"${UTILS_SRC_DIR}"
"${MFRAME_SRC_DIR}"
)

#################################
# resolve custom/default mconfig.c,h
# (utils is for a specific app)

find_file( MCONFIG_C
NAME mconfig.c
PATHS
"${UTILS_SRC_DIR}"
"${MFRAME_SRC_DIR}"
)

find_file( MCONFIG_H
NAME mconfig.h
PATHS
"${UTILS_SRC_DIR}"
"${MFRAME_SRC_DIR}"
)

#################################
# specify build options

add_compile_options( -Wall -g -O2 )
# [WITH_PDEBUG to provide debug output, e.g. PTRACE]
add_compile_definitions(
WITH_MSOCKET_TEST
WITH_MEDEBUG_TEST
WITH_MMDEBUG_TEST
WITH_MSTATS_TEST
WITH_MTIME_TEST
WITH_MKVCONF_TEST
)

# configuration/feature header
configure_file(LibmframeConfig.h.in LibmframeConfig.h)

#################################
# build libmframe

# specify build target
add_library(mframe
${MFRAME_SRC_DIR}/mbbuf.c
${MFRAME_SRC_DIR}/mcbuf.c
"${MCONFIG_C}"
${MFRAME_SRC_DIR}/mdebug.c
${MFRAME_SRC_DIR}/merror.c
${MFRAME_SRC_DIR}/mfile.c
${MFRAME_SRC_DIR}/mframe.c
${MFRAME_SRC_DIR}/mkvconf.c
${MFRAME_SRC_DIR}/mlist.c
${MFRAME_SRC_DIR}/mlog.c
${MFRAME_SRC_DIR}/mmdebug.c
${MFRAME_SRC_DIR}/mmem.c
${MFRAME_SRC_DIR}/mqueue.c
${MFRAME_SRC_DIR}/mserial.c
${MFRAME_SRC_DIR}/msocket.c
${MFRAME_SRC_DIR}/mstats.c
${MFRAME_SRC_DIR}/mswap.c
${MFRAME_SRC_DIR}/mthread.c
${MFRAME_SRC_DIR}/mtime.c
${MFRAME_SRC_DIR}/mutils.c
)

#################################
# build msocket-test

# specify build target
add_executable(msock-test ${MFRAME_SRC_DIR}/msocket-test.c)

# specify dependency libs
target_link_libraries(msock-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# build medebug-test

# specify build target
add_executable(medebug-test ${MFRAME_SRC_DIR}/medebug-test.c)

# specify dependency libs
target_link_libraries(medebug-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# build mkvconf-test

# specify build target
add_executable(mkvconf-test ${MFRAME_SRC_DIR}/mkvconf-test.c)

# specify dependency libs
target_link_libraries(mkvconf-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# build mmdebug-test

# specify build target
add_executable(mmdebug-test ${MFRAME_SRC_DIR}/mmdebug-test.c)

# specify dependency libs
target_link_libraries(mmdebug-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# build mstats-test

# specify build target
add_executable(mstats-test ${MFRAME_SRC_DIR}/mstats-test.c
${MFRAME_SRC_DIR}/mlog.c)

# specify dependency libs
target_link_libraries(mstats-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# build mtime-test

# specify build target
add_executable(mtime-test ${MFRAME_SRC_DIR}/mtime-test.c)

# specify dependency libs
target_link_libraries(mtime-test
PUBLIC
mframe
${EXTRA_LIBS}
)

#################################
# Export package

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# generate export header
include(GenerateExportHeader)
generate_export_header(mframe)
set_property(TARGET mframe PROPERTY VERSION ${Libmframe_VERSION})
set_property(TARGET mframe PROPERTY SOVERSION 1)
set_property(TARGET mframe PROPERTY INTERFACE_mframe_MAJOR_VERSION 0)
set_property(TARGET mframe APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING mframe_MAJOR_VERSION
)

# install package targets, define export group
# (destination(s) relative to package root)
install(TARGETS
mframe
EXPORT LibmframeTargets
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib
RUNTIME DESTINATION bin
INCLUDES DESTINATION include
)

# install execuables
# (destination(s) relative to package root)
install(PROGRAMS
"${CMAKE_CURRENT_BINARY_DIR}/msock-test"
"${CMAKE_CURRENT_BINARY_DIR}/medebug-test"
"${CMAKE_CURRENT_BINARY_DIR}/mkvconf-test"
"${CMAKE_CURRENT_BINARY_DIR}/mmdebug-test"
"${CMAKE_CURRENT_BINARY_DIR}/mstats-test"
"${CMAKE_CURRENT_BINARY_DIR}/mtime-test"
COMPONENT runtime
DESTINATION
bin/mframe-${Libmframe_VERSION}
)

# install package files
# (destination(s) relative to package root)
install( FILES
${MFRAME_SRC_DIR}/mbbuf.h
${MFRAME_SRC_DIR}/mcbuf.h
"${MCONFIG_H}"
${MFRAME_SRC_DIR}/mdebug.h
${MFRAME_SRC_DIR}/medebug.h
${MFRAME_SRC_DIR}/merror.h
${MFRAME_SRC_DIR}/mfile.h
${MFRAME_SRC_DIR}/mframe.h
${MFRAME_SRC_DIR}/mkvconf.h
${MFRAME_SRC_DIR}/mlist.h
${MFRAME_SRC_DIR}/mlog.h
${MFRAME_SRC_DIR}/mmdebug.h
${MFRAME_SRC_DIR}/mmem.h
${MFRAME_SRC_DIR}/mqueue.h
${MFRAME_SRC_DIR}/mserial.h
${MFRAME_SRC_DIR}/msocket.h
${MFRAME_SRC_DIR}/mstats.h
${MFRAME_SRC_DIR}/mswap.h
${MFRAME_SRC_DIR}/mthread.h
${MFRAME_SRC_DIR}/mtime.h
${MFRAME_SRC_DIR}/mutils.h
"${CMAKE_CURRENT_BINARY_DIR}/mframe_export.h"
COMPONENT devel
DESTINATION
include/mframe-${Libmframe_VERSION}
)

# set environment (used by cmake.in)
set(INCLUDE_INSTALL_DIR include/mframe-${CMAKE_PROJECT_VERSION}
CACHE PATH "libmframe include install path")

set(LIB_INSTALL_DIR lib/mframe-${CMAKE_PROJECT_VERSION}
CACHE PATH "libmframe lib install path" )

# load cmake package config module
include(CMakePackageConfigHelpers)

# set package path
# (relative to package root)
set(ConfigPackageLocation lib/cmake/mframe)

# generate/stage package config file (Config.cmake)
# using cmake.in
configure_package_config_file(
cmake/Libmframe.cmake.in
${CMAKE_CURRENT_BINARY_DIR}/mframe-pkg/LibmframeConfig.cmake
INSTALL_DESTINATION ${ConfigPackageLocation}
PATH_VARS
INCLUDE_INSTALL_DIR
)

# generate/stage package version vile (ConfigVersion.make)
write_basic_package_version_file(
${CMAKE_CURRENT_BINARY_DIR}/mframe-pkg/LibmframeConfigVersion.cmake
VERSION ${Libmframe_VERSION}
COMPATIBILITY AnyNewerVersion
#ARCH_INDEPENDENT
)

# install package config and version files
install(FILES
${CMAKE_CURRENT_BINARY_DIR}/mframe-pkg/LibmframeConfig.cmake
${CMAKE_CURRENT_BINARY_DIR}/mframe-pkg/LibmframeConfigVersion.cmake
DESTINATION ${ConfigPackageLocation}
)

# generate/stage Targets.cmake
export(EXPORT LibmframeTargets
FILE "${CMAKE_CURRENT_BINARY_DIR}/mframe-pkg/LibmframeTargets.cmake"
NAMESPACE Libmframe::
)

# install export group, Targets.cmake
install(EXPORT LibmframeTargets
FILE
LibmframeTargets.cmake
NAMESPACE
Libmframe::
DESTINATION
${ConfigPackageLocation}
)

