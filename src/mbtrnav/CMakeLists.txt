#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes 
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#      
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#
# MBtrnav libraries and test programs
#
# This group of libraries derives for an MBARI software repository that supports
# Terrain Relative Navigation (TRN) software. Aside from this CMakeLists.txt file
# and the parallel Autotools build system files Makefile.am and Makefile.in, 
# all three  written to enable building these libraries as part of MB-System, the
# directories and source files below are synchronized with the MBARI libtrnav
# repository.
#
# Build targets include:
# Libraries:
#   geolib newmat qnx tnav
#   geocon trnw netif mb1
#   trnucli trncli trnwcli
#
# Applications/Tests:
#   trn-server mmcpub mmcsub mcpub mcsub mb1rs
#   trn-player replay mb1log-player trnlog-player csvlog-player
#   trnu-cli trnu-svr trn-client trn-cli
#   readlog writelog log-to-matlab log-to-csv
#   otree bodump octree2patches grd-octree-maker csv-octree-maker transoct
#   test-trnif test-trnlog test-tile test-geocon test-trnattr
#
message("In src/mbtrnav")
#
#------------------------------------------------------------------------------
# set build option defaults
if(NOT DEFINED buildVerbose)
option(buildVerbose "build with verbose output" ON)
endif()
if(NOT DEFINED buildTRNOptional)
option(buildTRNOptional "build targets in buildTRNOptional" OFF)
endif()

if(buildTrnNoRand)
    # add TRN_NORAND to disable TRN random seed.
    add_compile_definitions(TRN_NORAND)
    message("TRN_NORAND set : TRN random seed DISABLED")
else()
    message("TRN_NORAND unset : TRN random seed ENABLED")
endif()

find_package(NetCDF REQUIRED)
find_package(LibPROJ REQUIRED)

add_compile_definitions(
HAVE_CONFIG_H
_GNU_SOURCE
#TRN_NORAND
WITH_TESTS
WITH_MMDEBUG
WITH_MST_MSTATS
MST_STATS_EN
#WITH_ALT_DELTAT_VALIDATION
#WITH_CEP_2022
WITH_TRNLOG
WITH_TRNLOG_EST_OUT
)

#################################
# define source directories
set(GCTP_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/gctp/source)
set(NEWMAT_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat)
set(QNX_SRC_DIR    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils)
set(TNAV_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
set(TRNW_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
set(UTILS_SRC_DIR  ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils)
set(DORADO_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/dorado)
set(OPTROV_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov)
set(TRNPLAYER_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/examples/trn-player)
set(MFRAME_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnframe)

#################################
# define include paths
# mframe, libtrnav, netcdf, proj
# use proj (default) or gctp, not both
list(PREPEND LIBTRNAV_INCLUDES
"${Libmframe_INCLUDE_DIRS}"
"${LibPROJ_INCLUDE_DIR}"
#"${GCTP_SRC_DIR}"
"${MFRAME_SRC_DIR}"
"${NEWMAT_SRC_DIR}"
"${QNX_SRC_DIR}"
"${TNAV_SRC_DIR}"
"${TRNW_SRC_DIR}"
"${UTILS_SRC_DIR}"
"${NetCDF_INCLUDE_DIRS}"
)

#################################
# emit some build info
if(buildVerbose)
get_directory_property( DirDefs DIRECTORY ${CMAKE_SOURCE_DIR}/src/mbtrnav COMPILE_DEFINITIONS )
message("mbtrnav ----------------------------------------------------")
message(" buildVerbose                ${buildVerbose}")
message(" buildTRNOptional            ${buildTRNOptional}")
message(" buildUseProj                ${buildUseProj}")
message(" buildTRNLCM                 ${buildTRNLCM}")
message(" buildTrnNoRand              ${buildTrnNoRand}")
message(" COMPILE_DEFINITIONS         ${DirDefs}")
message(" Libmframe_FOUND             ${Libmframe_FOUND}")
message(" Libmframe_INCLUDE_DIRS      ${Libmframe_INCLUDE_DIRS}")
message(" Libmframe_LIBRARIES         ${Libmframe_LIBRARIES}")
message(" mbtrnframe_FOUND            ${mbtrnframe_FOUND}")
message(" mbtrnframe_INCLUDE_DIRS     ${mbtrnframe_INCLUDE_DIRS}")
message(" mbtrnframe_LIBRARIES        ${mbtrnframe_LIBRARIES}")
message(" LibPROJ_FOUND               ${LibPROJ_FOUND}")
message(" LibPROJ_LIBRARIES           ${LibPROJ_LIBRARIES}")
message(" LibPROJ_INCLUDE_DIRS        ${LibPROJ_INCLUDE_DIRS}")
message(" LibPROJ_INCLUDE_DIR         ${LibPROJ_INCLUDE_DIR}")
message(" NetCDF_FOUND                ${NetCDF_FOUND}")
message(" NetCDF_INCLUDE_DIRS         ${NetCDF_INCLUUDE_DIRS}")
message(" NetCDF_LIBRARIES            ${NetCDF_LIBRARIES}")
message(" NetCDF_LIBRARY              ${NetCDF_LIBRARY}")
message(" NetCDF_VERSION              ${NetCDF_VERSION}")
message(" netCDF_INSTALL_PREFIX       ${netCDF_INSTALL_PREFIX}")
message(" netCDF_INCLUDE_DIR          ${netCDF_INCLUDE_DIR}")
message(" netCDF_LIB_DIR              ${netCDF_LIB_DIR}")
message(" GCTP_SRC_DIR                ${GCTP_SRC_DIR}")
message(" NEWMAT_SRC_DIR              ${NEWMAT_SRC_DIR}")
message(" QNX_SRC_DIR                 ${QNX_SRC_DIR}")
message(" TNAV_SRC_DIR                ${TNAV_SRC_DIR}")
message(" TRNW_SRC_DIR                ${TRNW_SRC_DIR}")
message(" UTILS_SRC_DIR               ${UTILS_SRC_DIR}")
message(" NetCDF_INCLUDE_DIRS         ${NetCDF_INCLUDE_DIRS}")
message(" LIBTRNAV_INCLUDES           ${LIBTRNAV_INCLUDES}")
message("----------------------------------------------------------")
endif()

#------------------------------------------------------------------------------
#
# build libgeolib (gctp)

add_library(geolib 
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

target_compile_options(geolib PRIVATE
"-Wno-deprecated-non-prototype"
"-Wno-unused-result"
"-Wno-unused-variable"
"-Wno-unused-but-set-variable"
"-Wno-uninitialized"
"-Wno-sometimes-uninitialized"
)

#
#------------------------------------------------------------------------------
#
# build libnewmat (matrix math)

add_library(newmat 
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
#
#------------------------------------------------------------------------------
#
# build libqnx (utils)

add_library(qnx 
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

# specify include paths and libraries
target_link_libraries(qnx PRIVATE geolib)
#
#------------------------------------------------------------------------------
#
# build tnav (TRN engine core)

# specify library
add_library(tnav 
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
   ${TNAV_SRC_DIR}/TrnLog.cpp
)

# specify include paths and libraries
target_link_libraries(tnav PRIVATE newmat qnx NetCDF::NetCDF)

target_include_directories(tnav PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build geocon

# specify library
add_library(geocon
    ${TRNW_SRC_DIR}/GeoCon.cpp
)

# specify include paths and libraries
target_link_libraries(geocon PRIVATE qnx LibPROJ::LibPROJ )

target_include_directories(geocon PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)

target_compile_options(geocon PRIVATE
"-std=c++11"
)

#
#------------------------------------------------------------------------------
#
# build trnw (TRN C wrapper backend)

# specify library
add_library(trnw
    ${TRNW_SRC_DIR}/trnw.cpp
    ${TRNW_SRC_DIR}/mb1_msg.c
    ${TRNW_SRC_DIR}/trnif_msg.c
)

# specify include paths and libraries
target_link_libraries(trnw PRIVATE tnav qnx newmat geocon m pthread)

target_include_directories(trnw PRIVATE BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbio
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build libnetif (TRNx server network interfaces)

# specify library
add_library(netif 
    ${TRNW_SRC_DIR}/netif.c
    ${TRNW_SRC_DIR}/trnif_proto.c
)

# specify include paths and libraries
target_link_libraries(netif PRIVATE mbtrnframe tnav trnw)

target_include_directories(netif PRIVATE BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnutils
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build libmb1

# specify library
add_library(mb1 
    ${TRNW_SRC_DIR}/mb1_msg.c
)
#
#------------------------------------------------------------------------------
#
# build trnu_cli component library

# specify library
add_library(trnucli 
    ${TRNW_SRC_DIR}/trnu_cli.c
)

# specify include paths and libraries
target_link_libraries(trnucli PRIVATE mbtrnframe netif)

target_include_directories(trnucli PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build trncli component library

# specify library
add_library(trncli
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify include paths and libraries
target_link_libraries(trncli PRIVATE newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)

target_include_directories(trncli PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build libtrnwcli

add_library(trnwcli
    ${TRNW_SRC_DIR}/trn_cli.c
)

# specify include paths and libraries
target_link_libraries(trnwcli PRIVATE mbtrnframe newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)

target_include_directories(trnwcli PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build trn-server (stand-alone TRN server app)

# specify build target
add_executable(trn-server
    ${UTILS_SRC_DIR}/trn_server.cpp
)

# specify include paths and libraries
target_link_libraries(trn-server PRIVATE tnav newmat qnx geolib)

target_include_directories(trn-server PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build mmcpub

add_executable(mmcpub
    ${TRNW_SRC_DIR}/mmcpub.c
)

# specify include paths and libraries
target_link_libraries(mmcpub PRIVATE mbtrnframe)

target_include_directories(mmcpub PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build mmcsub

add_executable(mmcsub
    ${TRNW_SRC_DIR}/mmcsub.c
)

# specify include paths and libraries
target_link_libraries(mmcsub PRIVATE mbtrnframe)

target_include_directories(mmcsub PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build mcpub

add_executable(mcpub
    ${TRNW_SRC_DIR}/mcpub.c
)

# specify include paths and libraries
target_include_directories(mcpub PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build mcsub

# specify build target
add_executable(mcsub
    ${TRNW_SRC_DIR}/mcsub.c
)

# specify include paths and libraries
target_include_directories(mcsub PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build mb1rs

add_executable(mb1rs
    ${TRNW_SRC_DIR}/mb1_msg.c
    ${TRNW_SRC_DIR}/mb1rs.c
    ${TRNW_SRC_DIR}/mb1rs-app.c
)

# specify include paths and libraries
target_link_libraries(mb1rs PRIVATE mbtrnframe m)

target_include_directories(mb1rs PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build trn-player

# specify build target
add_executable(trn-player
    ${TRNPLAYER_SRC_DIR}/TrnPlayer.hpp
    ${TRNPLAYER_SRC_DIR}/TrnPlayer.cpp
    ${TRNPLAYER_SRC_DIR}/TrnPlayerCtx.hpp
    ${TRNPLAYER_SRC_DIR}/trn-player.cpp
    ${TNAV_SRC_DIR}/TerrainNav.cpp
)

# specify include paths and libraries
target_link_libraries(trn-player PRIVATE tnav qnx newmat NetCDF::NetCDF)

target_include_directories(trn-player PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)

target_compile_options(trn-player PRIVATE
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build replay

# specify build target
add_executable(replay
    ${DORADO_SRC_DIR}/Replay.cpp
    ${DORADO_SRC_DIR}/trn_replay.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
)

# specify include paths and libraries
target_link_libraries(replay PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

target_include_directories(replay PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
    ${DORADO_SRC_DIR}
)

target_compile_options(replay PRIVATE
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build mb1log-player

# specify build target
add_executable(mb1log-player
    ${UTILS_SRC_DIR}/mb1log_player.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
)

# specify include paths and libraries
target_link_libraries(mb1log-player PRIVATE trnw trncli netif tnav qnx newmat geolib)

target_include_directories(mb1log-player PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
    ${OPTROV_SRC_DIR}
)

target_compile_options(mb1log-player PRIVATE
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build trnlog-player

# specify build target
add_executable(trnlog-player
    ${UTILS_SRC_DIR}/trnlog_player.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify include paths and libraries
target_link_libraries(trnlog-player PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

target_include_directories(trnlog-player PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
    ${OPTROV_SRC_DIR}
)

target_compile_options(trnlog-player PRIVATE
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build csvlog-player

# specify build target
add_executable(csvlog-player
    ${UTILS_SRC_DIR}/csvlog_player.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify include paths and libraries
target_link_libraries(csvlog-player PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

target_include_directories(csvlog-player PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
    ${OPTROV_SRC_DIR}
)

target_compile_options(csvlog-player PRIVATE
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build trnu-cli (TRNU client app)

# specify build target
add_executable(trnu-cli
    ${TRNW_SRC_DIR}/trnu-cli.c
)

# specify include paths and libraries
target_link_libraries(trnu-cli PRIVATE mbtrnframe trnucli netif tnav trnw newmat qnx geolib LibPROJ::LibPROJ NetCDF::NetCDF)

target_include_directories(trnu-cli PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build trnu-svr (stand-alone TRN server (no TRN instance))

# specify build target
add_executable(trnu-svr
    ${TRNW_SRC_DIR}/trnu-svr.c
)

# specify include paths and libraries
target_link_libraries(trnu-svr PRIVATE mbtrnframe netif tnav trnw newmat qnx geolib NetCDF::NetCDF)

target_include_directories(trnu-svr PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build trn-client

# specify build target
add_executable(trn-client
    ${UTILS_SRC_DIR}/trn-client.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify include paths and libraries
target_link_libraries(trn-client PRIVATE tnav newmat qnx NetCDF::NetCDF)

target_include_directories(trn-client PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build trn-cli

add_executable(trn-cli
    ${TRNW_SRC_DIR}/trn-cli.c
    ${TRNW_SRC_DIR}/mb1_msg.c
)

# specify include paths and libraries
target_link_libraries(trn-cli PRIVATE mbtrnframe trnw trnwcli qnx newmat geocon m)

target_include_directories(trn-cli PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)


# Build optional targets wnen buildTRNOptional=ON
# Must update TRN_OPTIONAL list at end of block if
# targets may be moved in/out of buildTRNOptional block

if(buildTRNOptional)
#
#------------------------------------------------------------------------------
#
# build readlog

# specify build target
add_executable(readlog
    ${QNX_SRC_DIR}/readlog.cc
)

# specify include paths and libraries
target_link_libraries(readlog PRIVATE qnx)

target_include_directories(readlog PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build writelog

# specify build target
add_executable(writelog
    ${QNX_SRC_DIR}/writelog.cc
)

# specify include paths and libraries
target_link_libraries(writelog PRIVATE qnx)

target_include_directories(writelog PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build log-to-matlab

# specify build target
add_executable(log-to-matlab
    ${QNX_SRC_DIR}/logToMatlab.cc
)

# specify include paths and libraries
target_link_libraries(log-to-matlab PRIVATE qnx)

target_include_directories(log-to-matlab PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build log-to-csv

# specify build target
add_executable(log-to-csv
    ${QNX_SRC_DIR}/logToCsv.cc
)

# specify include paths and libraries
target_link_libraries(log-to-csv  PRIVATE qnx)

target_include_directories(log-to-csv PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build otree

# specify build target
add_executable(otree
    ${UTILS_SRC_DIR}/OctreeTest.cpp
)

# specify include paths and libraries
target_link_libraries(otree PRIVATE tnav newmat qnx)

target_include_directories(otree PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build bodump (binary octree dump)

# specify build target
add_executable(bodump
    ${UTILS_SRC_DIR}/bodump.cpp
)

# specify include paths and libraries
target_link_libraries(bodump PRIVATE tnav newmat qnx)

target_include_directories(bodump PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build octree2patches

# specify build target
add_executable(octree2patches
    ${UTILS_SRC_DIR}/octree2Patches.cpp
)

# specify include paths and libraries
target_link_libraries(octree2patches PRIVATE tnav newmat qnx)

target_include_directories(octree2patches PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build grd-octree-maker

# specify build target
add_executable(grd-octree-maker
    ${UTILS_SRC_DIR}/grdOctreeMaker.cpp
)

# specify include paths and libraries
target_link_libraries(grd-octree-maker PRIVATE tnav newmat qnx NetCDF::NetCDF)

target_include_directories(grd-octree-maker PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build csv-octree-maker

# specify build target
add_executable(csv-octree-maker
    ${UTILS_SRC_DIR}/csvOctreeMaker.cpp
)

# specify include paths and libraries
target_link_libraries(csv-octree-maker PRIVATE tnav newmat qnx NetCDF::NetCDF
)

target_include_directories(csv-octree-maker PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#------------------------------------------------------------------------------
#
# build transoct

# specify build target
add_executable(transoct
    ${UTILS_SRC_DIR}/translateOctree.cpp
)

# specify include paths and libraries
target_link_libraries(transoct PRIVATE tnav newmat qnx)

target_include_directories(transoct PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build test-trnif

# specify build target
add_executable(test-trnif
    ${TRNW_SRC_DIR}/trnif_test.c
    ${TRNW_SRC_DIR}/trnif_proto.c
    ${TRNW_SRC_DIR}/trnif_msg.c
)

# specify include paths and libraries
target_link_libraries(test-trnif PRIVATE mbtrnframe netif newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)

target_include_directories(test-trnif PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build test-trnlog

# specify build target
add_executable(test-trnlog
    ${UTILS_SRC_DIR}/trnlog_test.cpp
)

# specify include paths and libraries
target_link_libraries(test-trnlog PRIVATE tnav newmat qnx geolib NetCDF::NetCDF)

target_include_directories(test-trnlog PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
# build test-tile

# specify build target
add_executable(test-tile
    ${UTILS_SRC_DIR}/tile_test.cpp
)

# specify include paths and libraries
target_link_libraries(test-tile PRIVATE tnav newmat qnx)

target_include_directories(test-tile PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
#
#  build test-geocon

add_executable(test-geocon
    ${TRNW_SRC_DIR}/geocon_test.cpp
)

# specify include paths and libraries
target_link_libraries(test-geocon PRIVATE geocon qnx)

target_include_directories(test-geocon PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
)

target_compile_options(test-geocon PRIVATE
"-std=c++11"
)

#
#------------------------------------------------------------------------------
#
# build test-trnattr

# specify build target
add_executable(test-trnattr
    ${UTILS_SRC_DIR}/trnattr_test.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify include paths and libraries
target_link_libraries(test-trnattr PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

target_include_directories(test-trnattr PRIVATE BEFORE
    ${LIBTRNAV_INCLUDES}
    ${DORADO_SRC_DIR}
)

target_compile_options(test-trnattr PRIVATE
"-std=c++11"
)

# update this list when targets are moved
# in/out of buildTRNOptional block
list(APPEND TRN_OPTIONAL
readlog writelog log-to-matlab log-to-csv
otree bodump octree2patches grd-octree-maker csv-octree-maker transoct
test-trnif test-trnlog test-tile test-geocon test-trnattr
)
endif() # buildTRNOptional

if(buildVerbose)
message(" TRN_OPTIONAL                ${TRN_OPTIONAL}")
endif()

#
#------------------------------------------------------------------------------
# install it all
#
install(TARGETS geolib newmat qnx tnav geocon trnw netif mb1 trnucli trncli trnwcli
DESTINATION
${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS
trn-server mmcpub mmcsub mcpub mcsub mb1rs
trn-player replay mb1log-player trnlog-player csvlog-player
trnu-cli trnu-svr trn-client trn-cli
${TRN_OPTIONAL}

DESTINATION
${CMAKE_INSTALL_BINDIR}
)

