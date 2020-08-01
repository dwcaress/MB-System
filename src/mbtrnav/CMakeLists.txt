# TODO : may be able to use lower versions
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Libtrnav VERSION 1.0)

# library source paths
# TODO : find/build mframe package
set(MFRAME_DIR ../mframe)
set(GCTP_SRC_DIR ${PROJECT_SOURCE_DIR}/gctp/source)
set(NEWMAT_SRC_DIR ${PROJECT_SOURCE_DIR}/newmat)
set(QNX_SRC_DIR ${PROJECT_SOURCE_DIR}/qnx-utils)
set(TNAV_SRC_DIR ${PROJECT_SOURCE_DIR}/terrain-nav)
set(TRNW_SRC_DIR ${PROJECT_SOURCE_DIR}/trnw)

# mframe library
# TODO : build package
find_library(MFRAMELIB
NAMES libmframe.a
PATHS "${MFRAME_DIR}/build"
)

# mframe header path
find_path(MFRAME_HEADER_PATH
NAMES mframe.h
PATHS
"${MFRAME_DIR}/src"
)

# mconfig header/source path
# may be custom (utils) or default (src)
find_path(MFCONF_CUSTOM_PATH
NAMES mconfig.h
PATHS
"${MFRAME_DIR}/utils"
)
find_path(MFCONF_DEFAULT_PATH
NAMES mconfig.h
PATHS
"${MFRAME_DIR}/src"
)

if(MFCONF_CUSTOM_PATH)
set(MFCONF_HEADER_PATH "${MFCONF_CUSTOM_PATH}")
else()
set(MFCONF_HEADER_PATH "${MFCONF_DEFAULT_PATH}")
add_compile_options("SHELL:-D NETIF_MMDEBUG")
endif()

# libnetcdf
find_library(NETCDF
NAMES
libnetcdf
libnetcdf.so
libnetcdf.dylib
PATHS
"/usr/lib64"
"/opt/local/lib"
)

find_path(NETCDF_HEADER_PATH
NAMES netcdf.h
PATHS
"/opt/local/include"
)

# libcurl
find_library(CURL
NAMES
libcurl
libcurl.so
PATHS "/usr/lib64"
)

# libhdf5
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

# library paths
list(APPEND EXTRA_LIBS
"${MFRAMELIB}"
m
pthread
"${NETCDF}"
)

if(CURL)
list(APPEND EXTRA_LIBS "${CURL}")
endif()

if(HDF5)
list(APPEND EXTRA_LIBS "${HDF5}")
endif()

# include paths
list(APPEND EXTRA_INCLUDES
"${MFCONF_HEADER_PATH}"
"${MFRAME_HEADER_PATH}"
"${MFRAME_SRC_DIR}"
"${TRNW_SRC_DIR}"
"${GCTP_SRC_DIR}"
"${NEWMAT_SRC_DIR}"
"${QNX_SRC_DIR}"
"${TNAV_SRC_DIR}"
"${TRNW_SRC_DIR}"
"${NETCDF_HEADER_PATH}"
)

# global compiler options
add_compile_options( -Wall -g -O2 )

configure_file(LibtrnavConfig.h.in LibtrnavConfig.h)

message("MFRAME_LIB=${MFRAMELIB}")
message("MFRAME_HEADER_PATH=${MFRAME_HEADER_PATH}")
message("MFCONF_HEADER_PATH=${MFCONF_HEADER_PATH}")
message("EXTRA_INCLUDES=${EXTRA_INCLUDES}")

# build trnu_cli component library
add_library(trnucli STATIC
${TRNW_SRC_DIR}/trnu_cli.c
)
set_target_properties( trnucli
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
)

# libgeolib (gctp)
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

# libnewmat (matrix math)
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

# libqnx (utils)
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

# TRN core
set_target_properties( tnav
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
)

# TRN C wrapper
add_library(trnw STATIC
${TRNW_SRC_DIR}/trnw.cpp
${TRNW_SRC_DIR}/mb1_msg.c
${TRNW_SRC_DIR}/trnif_msg.c
)

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

# libnetif (TRNx server network interfaces)
add_library(netif STATIC
${TRNW_SRC_DIR}/netif.c
${TRNW_SRC_DIR}/trnif_proto.c
)

set_target_properties( netif
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
LINK_LIBRARIES
"libtrnw.a"
"m"
"pthread"
)

# trnucli-test (TRNU client app)
add_executable(trnucli-test
${PROJECT_SOURCE_DIR}/trnw/trnucli_test.c
)
# link with trnucli, mframe libs
target_link_libraries(trnucli-test
trnucli
${EXTRA_LIBS}
)

target_include_directories(trnucli-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/trnw"
"${EXTRA_INCLUDES}"
)


# trnusvr-test  (stand-alone TRN server (no TRN instance))
add_executable(trnusvr-test
${PROJECT_SOURCE_DIR}/trnw/trnusvr_test.c
)
# link with trnusvr, mframe libs
target_link_libraries(trnusvr-test
"trnw"
"tnav"
"qnx"
"newmat"
"netif"
"geolib"
"${EXTRA_LIBS}"
)

target_include_directories(trnusvr-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/trnw"
"${EXTRA_INCLUDES}"
)
