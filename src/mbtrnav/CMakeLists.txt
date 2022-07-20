# TODO : may be able to use lower versions
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Libtrnav VERSION 0.1.0)

# get absolute path to parent directory
get_filename_component(
       PARENT_DIR
       "../"
       ABSOLUTE
)

#################################
# define source directories
set(GCTP_SRC_DIR ${PROJECT_SOURCE_DIR}/gctp/source)
set(NEWMAT_SRC_DIR ${PROJECT_SOURCE_DIR}/newmat)
set(QNX_SRC_DIR ${PROJECT_SOURCE_DIR}/qnx-utils)
set(TNAV_SRC_DIR ${PROJECT_SOURCE_DIR}/terrain-nav)
set(TRNW_SRC_DIR ${PROJECT_SOURCE_DIR}/trnw)

#################################
# pull in dependencies
# - libmframe
#   - libm
#   - libpthread
# - libtrnav
# - libnetcdf
#   - libcurl [1]
#   - libhdf5 [1]
#
# [1] - required by some netcdf versions


set(DFL_PKG_PATH "${PARENT_DIR}/mbtrnframe/mframe/build/pkg")
message(" >>> CMakeLists.txt -  MFRAME_PKG_PATH  ${MFRAME_PKG_PATH}")
message(" >>> CMakeLists.txt -       PARENT_DIR  ${PARENT_DIR}")
message(" >>> CMakeLists.txt -     DFL_PKG_PATH  ${DFL_PKG_PATH}")

find_package(
Libmframe
CONFIG
PATHS
"${MFRAME_PKG_PATH}"
"${PARENT_DIR}"
"${DFL_PKG_PATH}"
PATH_SUFFIXES
lib/cmake/mframe
mframe/build/pkg/lib/cmake/mframe
REQUIRED
)

message(" >>> CMakeLists.txt -  Libmframe_INCLUDE_DIR  ${Libmframe_INCLUDE_DIR}")
message(" >>> CMakeLists.txt -    Libmframe_LIBRARIES  ${Libmframe_LIBRARIES}")
#message(FATAL_ERROR " >>>>>>>> REMOVE THIS DEBUG MESSAGE <<<<<<<<")

# include LIBNETCDF
find_library(NETCDF
NAMES
libnetcdf
libnetcdf.so
libnetcdf.dylib
libnetcdf.dll.a
PATHS
"/usr/lib64"
"/opt/local/lib"
)

find_path(NETCDF_HEADER_PATH
NAMES netcdf.h
PATHS
"/opt/local/include"
)

# include LIBCURL [for some versions of netcdf]
find_library(CURL
NAMES
libcurl
libcurl.so
PATHS "/usr/lib64"
)

# include LIBHDF5 [for some versions of netcdf]
find_library(HDF5
NAMES
libhdf5_hl_cpp
libhdf5_cpp
libhdf5hl_fortran
libhdf5_fortran
libhdf5_hl
libhdf5
libhdf5_hl_cpp.so
libhdf5_cpp.so
libhdf5hl_fortran.so
libhdf5_fortran.so
libhdf5_hl.so
libhdf5.so
PATHS
"/usr/lib64"
"/opt/local/lib"
)

#################################
# define dependency libs

# libm, libpthread, libnetcdf
list(APPEND EXTRA_LIBS
Libmframe::mframe
m
pthread
"${NETCDF}"
)

# add libcurl [as needed]
if(CURL)
list(APPEND EXTRA_LIBS "${CURL}")
endif()

# add libhdf5 [as needed]
if(HDF5)
list(APPEND EXTRA_LIBS "${HDF5}")
endif()

#################################
# define include paths

# mframe, libtrnav, netcdf
list(APPEND EXTRA_INCLUDES
"${Libmframe_INCLUDE_DIR}"
"${MFRAME_INCL_DIR}"
"${TRNW_SRC_DIR}"
"${GCTP_SRC_DIR}"
"${NEWMAT_SRC_DIR}"
"${QNX_SRC_DIR}"
"${TNAV_SRC_DIR}"
"${TRNW_SRC_DIR}"
"${NETCDF_HEADER_PATH}"
)

#################################
# specify build options

# global compiler options
add_compile_options( -Wall -g -O2 )

# debug
if( WITH_PDEBUG )
# add WITH_PDEBUG to enable extra debug output, e.g. PTRACE, PDPRINT
# compile using cmake .. -DWITH_PDEBUG=1
message("compiling with WITH_PDEBUG")
add_compile_definitions(WITH_PDEBUG)
endif()

# built-in netif module debug
if( WITH_NETIF_MMD_EXT )
# TODO: get external netif mmdebug channel defs
else()
# add NETIF_MMDEBUG to use netif default MMDEBUG macros
# instead of values defined in a custom mconfig.h
# [may need for Cygwin build]
# compile using cmake .. -DNETIF_MMDEBUG=1
message("compiling with NETIF_MMDEBUG")
add_compile_definitions(NETIF_MMDEBUG)
endif()

# configuration/feature header
configure_file(LibtrnavConfig.h.in LibtrnavConfig.h)

#################################
# emit some build info
message("WITH_PDEBUG=${WITH_PDEBUG}")
message("WITH_NETIF_MMD_EXT=${WITH_NETIF_MMD_EXT}")
message("PARENT_DIR=${PARENT_DIR}")
message("MFRAME_PKG_PATH=${MFRAME_PKG_PATH}")
message("EXTRA_INCLUDES=${EXTRA_INCLUDES}")
message("EXTRA_LIBS`=${EXTRA_LIBS}")

#################################
# build trnu_cli component library

# specify library
add_library(trnucli STATIC
${TRNW_SRC_DIR}/trnu_cli.c
)

# specify include directories
set_target_properties( trnucli
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
)

#################################
# build libgeolib (gctp)

# specify library
add_library(geolib STATIC
 ${GCTP_SRC_DIR}/gctp.c
 ${GCTP_SRC_DIR}/alberfor.c
 ${GCTP_SRC_DIR}/alberinv.c
 ${GCTP_SRC_DIR}/alconfor.c
 ${GCTP_SRC_DIR}/alconinv.c
 ${GCTP_SRC_DIR}/azimfor.c
 ${GCTP_SRC_DIR}/aziminv.c
 ${GCTP_SRC_DIR}/eqconfor.c
 ${GCTP_SRC_DIR}/eqconinv.c
 ${GCTP_SRC_DIR}/equifor.c
 ${GCTP_SRC_DIR}/equiinv.c
 ${GCTP_SRC_DIR}/gnomfor.c
 ${GCTP_SRC_DIR}/gnominv.c
 ${GCTP_SRC_DIR}/goodfor.c
 ${GCTP_SRC_DIR}/goodinv.c
 ${GCTP_SRC_DIR}/gvnspfor.c
 ${GCTP_SRC_DIR}/gvnspinv.c
 ${GCTP_SRC_DIR}/hamfor.c
 ${GCTP_SRC_DIR}/haminv.c
 ${GCTP_SRC_DIR}/imolwfor.c
 ${GCTP_SRC_DIR}/imolwinv.c
 ${GCTP_SRC_DIR}/lamazfor.c
 ${GCTP_SRC_DIR}/lamazinv.c
 ${GCTP_SRC_DIR}/merfor.c
 ${GCTP_SRC_DIR}/merinv.c
 ${GCTP_SRC_DIR}/millfor.c
 ${GCTP_SRC_DIR}/millinv.c
 ${GCTP_SRC_DIR}/molwfor.c
 ${GCTP_SRC_DIR}/molwinv.c
 ${GCTP_SRC_DIR}/obleqfor.c
 ${GCTP_SRC_DIR}/obleqinv.c
 ${GCTP_SRC_DIR}/omerfor.c
 ${GCTP_SRC_DIR}/omerinv.c
 ${GCTP_SRC_DIR}/orthfor.c
 ${GCTP_SRC_DIR}/orthinv.c
 ${GCTP_SRC_DIR}/polyfor.c
 ${GCTP_SRC_DIR}/polyinv.c
 ${GCTP_SRC_DIR}/psfor.c
 ${GCTP_SRC_DIR}/psinv.c
 ${GCTP_SRC_DIR}/robfor.c
 ${GCTP_SRC_DIR}/robinv.c
 ${GCTP_SRC_DIR}/sinfor.c
 ${GCTP_SRC_DIR}/sininv.c
 ${GCTP_SRC_DIR}/somfor.c
 ${GCTP_SRC_DIR}/sominv.c
 ${GCTP_SRC_DIR}/sterfor.c
 ${GCTP_SRC_DIR}/sterinv.c
 ${GCTP_SRC_DIR}/stplnfor.c
 ${GCTP_SRC_DIR}/stplninv.c
 ${GCTP_SRC_DIR}/tmfor.c
 ${GCTP_SRC_DIR}/tminv.c
 ${GCTP_SRC_DIR}/utmfor.c
 ${GCTP_SRC_DIR}/utminv.c
 ${GCTP_SRC_DIR}/vandgfor.c
 ${GCTP_SRC_DIR}/vandginv.c
 ${GCTP_SRC_DIR}/wivfor.c
 ${GCTP_SRC_DIR}/wivinv.c
 ${GCTP_SRC_DIR}/wviifor.c
 ${GCTP_SRC_DIR}/wviiinv.c
 ${GCTP_SRC_DIR}/for_init.c
 ${GCTP_SRC_DIR}/inv_init.c
 ${GCTP_SRC_DIR}/cproj.c
 ${GCTP_SRC_DIR}/report.c
 ${GCTP_SRC_DIR}/lamccfor.c
 ${GCTP_SRC_DIR}/lamccinv.c
 ${GCTP_SRC_DIR}/paksz.c
 ${GCTP_SRC_DIR}/untfz.c
 ${GCTP_SRC_DIR}/sphdz.c
 ${GCTP_SRC_DIR}/br_gctp.c
)

#################################
# build libnewmat (matrix math)

# specify library
add_library(newmat STATIC
${NEWMAT_SRC_DIR}/newmat1.cpp
${NEWMAT_SRC_DIR}/newmat2.cpp
${NEWMAT_SRC_DIR}/newmat3.cpp
${NEWMAT_SRC_DIR}/newmat4.cpp
${NEWMAT_SRC_DIR}/newmat5.cpp
${NEWMAT_SRC_DIR}/newmat6.cpp
${NEWMAT_SRC_DIR}/newmat7.cpp
${NEWMAT_SRC_DIR}/newmat8.cpp
${NEWMAT_SRC_DIR}/newmatex.cpp
${NEWMAT_SRC_DIR}/bandmat.cpp
${NEWMAT_SRC_DIR}/submat.cpp
${NEWMAT_SRC_DIR}/myexcept.cpp
${NEWMAT_SRC_DIR}/cholesky.cpp
${NEWMAT_SRC_DIR}/evalue.cpp
${NEWMAT_SRC_DIR}/fft.cpp
${NEWMAT_SRC_DIR}/hholder.cpp
${NEWMAT_SRC_DIR}/jacobi.cpp
${NEWMAT_SRC_DIR}/newfft.cpp
${NEWMAT_SRC_DIR}/sort.cpp
${NEWMAT_SRC_DIR}/svd.cpp
${NEWMAT_SRC_DIR}/newmatrm.cpp
${NEWMAT_SRC_DIR}/newmat9.cpp
)

#################################
# build libqnx (utils)

# specify library
add_library(qnx STATIC
${QNX_SRC_DIR}/AsciiFile.cc
${QNX_SRC_DIR}/AngleData.cc
${QNX_SRC_DIR}/CharData.cc
${QNX_SRC_DIR}/BinaryFile.cc
${QNX_SRC_DIR}/DataLog.cc
${QNX_SRC_DIR}/DataLogReader.cc
${QNX_SRC_DIR}/DataLogWriter.cc
${QNX_SRC_DIR}/FileData.cc
${QNX_SRC_DIR}/StringConverter.cc
${QNX_SRC_DIR}/StringData.cc
${QNX_SRC_DIR}/TimeP.cc
${QNX_SRC_DIR}/TimeTag.cc
${QNX_SRC_DIR}/ShortData.cc
${QNX_SRC_DIR}/Math.cc
${QNX_SRC_DIR}/NavUtils.cc
${QNX_SRC_DIR}/IntegerData.cc
${QNX_SRC_DIR}/FloatData.cc
${QNX_SRC_DIR}/DoubleData.cc
${QNX_SRC_DIR}/DataField.cc
${QNX_SRC_DIR}/DataFieldFactory.cc
${QNX_SRC_DIR}/LogFile.cc
)

#################################
# build tnav (TRN engine core)

# specify library
add_library(tnav STATIC
${TNAV_SRC_DIR}/TerrainNav.cpp
${TNAV_SRC_DIR}/TNavConfig.cpp
${TNAV_SRC_DIR}/TNavFilter.cpp
${TNAV_SRC_DIR}/TNavPointMassFilter.cpp
${TNAV_SRC_DIR}/TNavParticleFilter.cpp
${TNAV_SRC_DIR}/TNavBankFilter.cpp
${TNAV_SRC_DIR}/TNavPFLog.cpp
${TNAV_SRC_DIR}/TerrainMapOctree.cpp
${TNAV_SRC_DIR}/PositionLog.cpp
${TNAV_SRC_DIR}/TerrainNavLog.cpp
${TNAV_SRC_DIR}/mapio.cpp
${TNAV_SRC_DIR}/structDefs.cpp
${TNAV_SRC_DIR}/trn_log.cpp
${TNAV_SRC_DIR}/myOutput.cpp
${TNAV_SRC_DIR}/matrixArrayCalcs.cpp
${TNAV_SRC_DIR}/TerrainMapDEM.cpp
${TNAV_SRC_DIR}/OctreeSupport.cpp
${TNAV_SRC_DIR}/Octree.cpp
${TNAV_SRC_DIR}/OctreeNode.cpp
${TNAV_SRC_DIR}/TRNUtils.cpp
)

# specify include paths and libraries
set_target_properties( tnav
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
)

#################################
# build trnw (TRN C wrapper)

# specify library
add_library(trnw STATIC
${TRNW_SRC_DIR}/trnw.cpp
${TRNW_SRC_DIR}/mb1_msg.c
${TRNW_SRC_DIR}/trnif_msg.c
)

# specify include paths and libraries
set_target_properties( trnw
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
LINK_LIBRARIES
"libtnav.a"
"libqnx.a"
"libnewmat.a"
"m"
"pthread"
)

#################################
# build libnetif (TRNx server network interfaces)

# specify library
add_library(netif STATIC
${TRNW_SRC_DIR}/netif.c
${TRNW_SRC_DIR}/trnif_proto.c
)

# specify include paths and libraries
set_target_properties( netif
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
LINK_LIBRARIES
"libtrnw.a"
"m"
"pthread"
)

##################################
# build trnucli-test (TRNU client app)

# specify build target
add_executable(trnucli-test
${PROJECT_SOURCE_DIR}/trnw/trnucli_test.c
)

# specify dependency libs
target_link_libraries(trnucli-test
trnucli
Libmframe::mframe
"${EXTRA_LIBS}"
)

# specify include paths
target_include_directories(trnucli-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/trnw"
"${EXTRA_INCLUDES}"
)

##################################
# build trnusvr-test (stand-alone TRN server (no TRN instance))

# specify build target
add_executable(trnusvr-test
${PROJECT_SOURCE_DIR}/trnw/trnusvr_test.c
)

# specify dependency libs
target_link_libraries(trnusvr-test
"netif"
"trnw"
"tnav"
"qnx"
"newmat"
"geolib"
"${EXTRA_LIBS}"
)

# specify include paths
target_include_directories(trnusvr-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/trnw"
"${EXTRA_INCLUDES}"
)

#################################
# Export package

# set environment (used by cmake.in)
set(INCLUDE_INSTALL_DIR include/libtrnav-${CMAKE_PROJECT_VERSION}
CACHE PATH "libtrnav include install path")

set(LIB_INSTALL_DIR lib/libtrnav-${CMAKE_PROJECT_VERSION}
CACHE PATH "libtrnav lib install path" )

# load cmake package config module
include(CMakePackageConfigHelpers)

# set package path
# (relative to package root)
set(ConfigPackageLocation lib/cmake/libtrnav)

# generate/stage package config file (Config.cmake)
# using cmake.in
configure_package_config_file(
cmake/Libtrnav.cmake.in
${CMAKE_CURRENT_BINARY_DIR}/libtrnav-pkg/LibtrnavConfig.cmake
INSTALL_DESTINATION ${ConfigPackageLocation}
PATH_VARS
INCLUDE_INSTALL_DIR
)

# generate/stage package version vile (ConfigVersion.make)
write_basic_package_version_file(
${CMAKE_CURRENT_BINARY_DIR}/libtrnav-pkg/LibtrnavConfigVersion.cmake
VERSION ${Libtrnav_VERSION}
COMPATIBILITY AnyNewerVersion
#ARCH_INDEPENDENT
)


##################################
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# get cmake export module
include(GenerateExportHeader)

# generate export headers
generate_export_header(geolib)
set_property(TARGET geolib PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET geolib PROPERTY SOVERSION 0)
set_property(TARGET geolib PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET geolib APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING geolib_MAJOR_VERSION
)

generate_export_header(newmat)
set_property(TARGET newmat PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET newmat PROPERTY SOVERSION 0)
set_property(TARGET newmat PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET newmat APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING newmat_MAJOR_VERSION
)

generate_export_header(qnx)
set_property(TARGET qnx PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET qnx PROPERTY SOVERSION 0)
set_property(TARGET qnx PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET qnx APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING qnx_MAJOR_VERSION
)

generate_export_header(tnav)
set_property(TARGET tnav PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET tnav PROPERTY SOVERSION 0)
set_property(TARGET tnav PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET tnav APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING tnav_MAJOR_VERSION
)

generate_export_header(trnw)
set_property(TARGET trnw PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET trnw PROPERTY SOVERSION 0)
set_property(TARGET trnw PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET trnw APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING trnw_MAJOR_VERSION
)

generate_export_header(netif)
set_property(TARGET netif PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET netif PROPERTY SOVERSION 0)
set_property(TARGET netif PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET netif APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING netif_MAJOR_VERSION
)

generate_export_header(trnucli)
set_property(TARGET trnucli PROPERTY VERSION ${Libtrnav_VERSION})
set_property(TARGET trnucli PROPERTY SOVERSION 0)
set_property(TARGET trnucli PROPERTY INTERFACE_libtrnav_MAJOR_VERSION 0)
set_property(TARGET trnucli APPEND PROPERTY
COMPATIBLE_INTERFACE_STRING trnucli_MAJOR_VERSION
)

# specify export layout
install(TARGETS
geolib
newmat
qnx
tnav
trnw
netif
trnucli
EXPORT LibtrnavTargets
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib
RUNTIME DESTINATION bin
INCLUDES DESTINATION include
)

# specify package files
install(
FILES
${TNAV_SRC_DIR}/trn_common.h
${TRNW_SRC_DIR}/trnw.h
${TRNW_SRC_DIR}/netif.h
${TRNW_SRC_DIR}/trnu_cli.h
${TRNW_SRC_DIR}/trn_cli.h
${TRNW_SRC_DIR}/trn_msg.h
${TRNW_SRC_DIR}/mb1_msg.h
${TRNW_SRC_DIR}/trnif_msg.h
${TRNW_SRC_DIR}/trnif_proto.h
"${CMAKE_CURRENT_BINARY_DIR}/geolib_export.h"
"${CMAKE_CURRENT_BINARY_DIR}/newmat_export.h"
"${CMAKE_CURRENT_BINARY_DIR}/qnx_export.h"
"${CMAKE_CURRENT_BINARY_DIR}/tnav_export.h"
"${CMAKE_CURRENT_BINARY_DIR}/trnw_export.h"
"${CMAKE_CURRENT_BINARY_DIR}/trnucli_export.h"
DESTINATION
include/libtrnav-${Libtrnav_VERSION}
COMPONENT
Devel
)

install(
PROGRAMS
"${CMAKE_CURRENT_BINARY_DIR}/trnucli-test"
"${CMAKE_CURRENT_BINARY_DIR}/trnusvr-test"
DESTINATION
bin/libtrnav-${Libtrnav_VERSION}
COMPONENT
runtime
)

# install package config and version files
install(FILES
${CMAKE_CURRENT_BINARY_DIR}/libtrnav-pkg/LibtrnavConfig.cmake
${CMAKE_CURRENT_BINARY_DIR}/libtrnav-pkg/LibtrnavConfigVersion.cmake
DESTINATION ${ConfigPackageLocation}
)

# generate/stage Targets.cmake
export(EXPORT LibtrnavTargets
FILE "${CMAKE_CURRENT_BINARY_DIR}/libtrnav-pkg/LibtrnavTargets.cmake"
NAMESPACE Libtrnav::
)

# install export group, Targets.cmake
install(EXPORT LibtrnavTargets
FILE
LibtrnavTargets.cmake
NAMESPACE
Libtrnav::
DESTINATION
${ConfigPackageLocation}
)
