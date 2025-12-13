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
# The libraries are:
#    geolib (GCTP)
#    newmat
#    qnx
#    tnav
#    trnw
#.   netif
#.   qnx
#.   trnucli
#
# The test programs are:
#    trnucli-test
#.   trnusvr-test
#
message("In src/mbtrnav")
#
#------------------------------------------------------------------------------
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
#WITH_TNAVPF_CEP_CORRECTION
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
set(TRNPLAYER_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/examples/trn-player)

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
       ${TNAV_SRC_DIR}/TrnLog.cpp
       ${TNAV_SRC_DIR}/trn_log.cpp
       ${TNAV_SRC_DIR}/myOutput.cpp
       ${TNAV_SRC_DIR}/matrixArrayCalcs.cpp
       ${TNAV_SRC_DIR}/TerrainMapDEM.cpp
       ${TNAV_SRC_DIR}/OctreeSupport.cpp
       ${TNAV_SRC_DIR}/Octree.cpp
       ${TNAV_SRC_DIR}/OctreeNode.cpp
       ${TNAV_SRC_DIR}/TRNUtils.cpp
)
target_include_directories(tnav PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS})
target_link_libraries(tnav PRIVATE newmat qnx NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build geocon

# specify library
add_library(geocon
    ${TRNW_SRC_DIR}/GeoCon.cpp
)
target_include_directories(geocon PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${LibPROJ_INCLUDE_DIRS})
target_link_libraries(geocon PRIVATE newmat qnx LibPROJ::LibPROJ )
#
#------------------------------------------------------------------------------
#
# build trnwc (TRN C wrapper)
add_library(trnwc STATIC
    ${TRNW_SRC_DIR}/mb1_msg.c
    ${TRNW_SRC_DIR}/trnif_msg.c
)

target_include_directories(trnwc PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbio
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)
#
#------------------------------------------------------------------------------
#
# build trnw (TRN C wrapper)

# specify library
add_library(trnw 
       ${TRNW_SRC_DIR}/trnw.cpp
       ${TRNW_SRC_DIR}/mb1_msg.c
       ${TRNW_SRC_DIR}/trnif_msg.c
)

target_include_directories(trnw PRIVATE ${CMAKE_SOURCE_DIR}/src/mbio
  ${LibPROJ_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
  ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
  ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
  ${NetCDF_INCLUDE_DIRS})

target_link_libraries(trnw PRIVATE tnav newmat qnx geocon LibPROJ::LibPROJ NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build libnetif (TRNx server network interfaces)

# specify library
add_library(netif 
    ${TRNW_SRC_DIR}/netif.c
    ${TRNW_SRC_DIR}/trnif_proto.c
)

target_include_directories(netif PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnutils)

target_link_libraries(netif PRIVATE mbtrnframe tnav trnw)
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

target_include_directories(trnucli PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)

target_link_libraries(trnucli PRIVATE mbtrnframe netif)
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

target_include_directories(trncli PRIVATE
${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)

target_link_libraries(trncli PRIVATE newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
#  build libtrnwcli

add_library(trnwcli STATIC
    ${TRNW_SRC_DIR}/trn_cli.c
)

target_include_directories(trnwcli PRIVATE
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)

target_link_libraries(trnwcli PRIVATE newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build trn-server (stand-alone TRN server app)

# specify build target
add_executable(trn-server
    ${UTILS_SRC_DIR}/trn_server.cpp
)

# specify dependency libs
target_link_libraries(trn-server PRIVATE tnav newmat qnx geolib)

# specify include paths
target_include_directories(trn-server PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${NetCDF_INCLUDE_DIRS}
)
#
#------------------------------------------------------------------------------
#
#  build mmcpub

add_executable(mmcpub
    ${TRNW_SRC_DIR}/mmcpub.c
)

target_link_libraries(mmcpub PRIVATE mbtrnframe)

target_include_directories(mmcpub PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    trnw terrain-nav
)
#
#------------------------------------------------------------------------------
#
#  build mmcsub

add_executable(mmcsub
    ${TRNW_SRC_DIR}/mmcsub.c
)

target_link_libraries(mmcsub PRIVATE mbtrnframe)

target_include_directories(mmcsub PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    trnw
    terrain-nav
)
#
#------------------------------------------------------------------------------
#
#  build mcpub

add_executable(mcpub
    ${TRNW_SRC_DIR}/mcpub.c
)

target_include_directories(mcpub PRIVATE ${CMAKE_SOURCE_DIR} trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mcsub

# specify build target
add_executable(mcsub
    ${TRNW_SRC_DIR}/mcsub.c
)

target_include_directories(mcsub PRIVATE
    ${CMAKE_SOURCE_DIR}
    trnw
    terrain-nav
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

target_link_libraries(mb1rs PRIVATE mbtrnframe m)

target_include_directories(mb1rs PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    trnw
    terrain-nav
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

# specify dependency libs
target_link_libraries(trn-player PRIVATE tnav qnx newmat NetCDF::NetCDF)

# specify include paths
target_include_directories(trn-player PUBLIC
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)

target_compile_options(trn-player PUBLIC
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
#${DORADO_SRC_DIR}/replay_app.cpp
${UTILS_SRC_DIR}/TerrainNavClient.cpp
)

# specify dependency libs
target_link_libraries(replay PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

target_include_directories(replay PUBLIC
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/dorado
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)

target_compile_options(replay PUBLIC
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

# specify dependency libs
target_link_libraries(mb1log-player PRIVATE trnw trncli netif tnav qnx newmat geolib)

# specify include paths
target_include_directories(mb1log-player PUBLIC
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)
target_compile_options(mb1log-player PUBLIC
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

# specify dependency libs
target_link_libraries(trnlog-player PRIVATE
    trnw
    trncli
    netif
    tnav
    qnx
    newmat
    geolib
    NetCDF::NetCDF
)

# specify include paths
target_include_directories(trnlog-player PUBLIC
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
    ${NetCDF_INCLUDE_DIRS}
)

target_compile_options(trnlog-player PUBLIC
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

# specify dependency libs
target_link_libraries(csvlog-player PRIVATE
    trnw
    trncli
    netif
    tnav
    qnx
    newmat
    geolib
    NetCDF::NetCDF
)

# specify include paths
target_include_directories(csvlog-player PUBLIC
BEFORE
${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
${NetCDF_INCLUDE_DIRS}
)

target_compile_options(csvlog-player PUBLIC
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build readlog

# specify build target
add_executable(readlog
    ${QNX_SRC_DIR}/readlog.cc
)

# specify dependency libs
target_link_libraries(readlog PRIVATE qnx)

# specify include paths
target_include_directories(readlog PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
)
#
#------------------------------------------------------------------------------
#
# build writelog

# specify build target
add_executable(writelog
    ${QNX_SRC_DIR}/writelog.cc
)

# specify dependency libs
target_link_libraries(writelog PRIVATE qnx)

# specify include paths
target_include_directories(writelog PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
)
#
#------------------------------------------------------------------------------
#
# build log-to-matlab

# specify build target
add_executable(log-to-matlab
    ${QNX_SRC_DIR}/logToMatlab.cc
)

# specify dependency libs
target_link_libraries(log-to-matlab PRIVATE qnx)

# specify include paths
target_include_directories(log-to-matlab PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
)
#
#------------------------------------------------------------------------------
#
# build log-to-csv

# specify build target
add_executable(log-to-csv
    ${QNX_SRC_DIR}/logToCsv.cc
)

# specify dependency libs
target_link_libraries(log-to-csv  PRIVATE qnx)

# specify include paths
target_include_directories(log-to-csv PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
)
#------------------------------------------------------------------------------
#
# build otree

# specify build target
add_executable(otree
    ${UTILS_SRC_DIR}/OctreeTest.cpp
)

# specify dependency libs
target_link_libraries(otree PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(otree PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
)
#------------------------------------------------------------------------------
#
# build bodump (binary octree dump)

# specify build target
add_executable(bodump
    ${UTILS_SRC_DIR}/bodump.cpp
)

# specify dependency libs
target_link_libraries(bodump PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(bodump PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
)
#------------------------------------------------------------------------------
#
# build octree2patches

# specify build target
add_executable(octree2patches
    ${UTILS_SRC_DIR}/octree2Patches.cpp
)

# specify dependency libs
target_link_libraries(octree2patches PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(octree2patches PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${NetCDF_INCLUDE_DIRS}
)
#------------------------------------------------------------------------------
#
# build grd-octree-maker

# specify build target
add_executable(grd-octree-maker
    ${UTILS_SRC_DIR}/grdOctreeMaker.cpp
)

# specify dependency libs
target_link_libraries(grd-octree-maker PRIVATE tnav newmat qnx NetCDF::NetCDF)

# specify include paths
target_include_directories(grd-octree-maker PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${NetCDF_INCLUDE_DIRS}
)
#------------------------------------------------------------------------------
#
# build csv-octree-maker

# specify build target
add_executable(csv-octree-maker
    ${UTILS_SRC_DIR}/csvOctreeMaker.cpp
)

# specify dependency libs
target_link_libraries(csv-octree-maker PRIVATE tnav newmat qnx NetCDF::NetCDF
)

# specify include paths
target_include_directories(csv-octree-maker PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${NetCDF_INCLUDE_DIRS}
)
#------------------------------------------------------------------------------
#
# build transoct

# specify build target
add_executable(transoct
    ${UTILS_SRC_DIR}/translateOctree.cpp
)

# specify dependency libs
target_link_libraries(transoct PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(transoct PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${NetCDF_INCLUDE_DIRS}
)
#
#------------------------------------------------------------------------------
#
# build trnucli-test (TRNU client app)

# specify build target
add_executable(trnucli-test
    ${TRNW_SRC_DIR}/trnucli_test.c
)

target_link_libraries(trnucli-test PRIVATE mbtrnframe trnucli netif tnav trnw newmat qnx geolib LibPROJ::LibPROJ NetCDF::NetCDF)

target_include_directories(trnucli-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
)
#
#------------------------------------------------------------------------------
#
# build trnusvr-test (stand-alone TRN server (no TRN instance))

# specify build target
add_executable(trnusvr-test
    ${TRNW_SRC_DIR}/trnusvr_test.c
)

target_link_libraries(trnusvr-test PRIVATE mbtrnframe netif tnav trnw newmat qnx geolib NetCDF::NetCDF)

target_include_directories(trnusvr-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
)
#
#------------------------------------------------------------------------------
#
# build trnclient-test

# specify build target
add_executable(trnclient-test
    ${UTILS_SRC_DIR}/trnclient_test.cpp
    ${UTILS_SRC_DIR}/TrnClient.cpp
    ${UTILS_SRC_DIR}/TerrainNavClient.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify dependency libs
target_link_libraries(trnclient-test PRIVATE tnav newmat qnx  NetCDF::NetCDF)

# specify include paths
target_include_directories(trnclient-test PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${NetCDF_INCLUDE_DIRS}
)
#
#------------------------------------------------------------------------------
#
#  build trncli-test

add_executable(trncli-test
    ${TRNW_SRC_DIR}/trncli_test.c
    ${TRNW_SRC_DIR}/mb1_msg.c
)

target_link_libraries(trncli-test PRIVATE mbtrnframe trnwcli newmat geolib trnw qnx mb1 tnav geocon LibPROJ::LibPROJ NetCDF::NetCDF m)

target_include_directories(trncli-test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe trnw terrain-nav
)
#
#------------------------------------------------------------------------------
#
#  build trnif-test

# specify build target
add_executable(trnif-test
    ${TRNW_SRC_DIR}/trnif_test.c
    ${TRNW_SRC_DIR}/trnif_proto.c
    ${TRNW_SRC_DIR}/trnif_msg.c
)

target_link_libraries(trnif-test PRIVATE mbtrnframe netif newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)

target_include_directories(trnif-test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${LibPROJ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnframe
)
#
#------------------------------------------------------------------------------
#
# build trnlog-test

# specify build target
add_executable(trnlog-test
    ${UTILS_SRC_DIR}/test_trn_log.cpp
)

target_link_libraries(trnlog-test PRIVATE tnav newmat qnx geolib NetCDF::NetCDF)

target_include_directories(trnlog-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
)
#
#------------------------------------------------------------------------------
#
# build tile-test

# specify build target
add_executable(tile-test
    ${UTILS_SRC_DIR}/tile_test.cpp
)

# specify dependency libs
target_link_libraries(tile-test PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(tile-test PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${NetCDF_INCLUDE_DIRS}
)
#
#------------------------------------------------------------------------------
#
#  build geocon-test

add_executable(geocon-test
    ${TRNW_SRC_DIR}/geocon_test.cpp
)

target_link_libraries(geocon-test PRIVATE geocon qnx)

target_include_directories(geocon-test PRIVATE
${CMAKE_SOURCE_DIR}
${LibPROJ_INCLUDE_DIRS}
trnw qnx-utils
)
#
#------------------------------------------------------------------------------
#
# build test-trnattr

# specify build target
add_executable(test-trnattr
    ${UTILS_SRC_DIR}/test_trnattr.cpp
    ${UTILS_SRC_DIR}/TrnAttr.cpp
)

# specify dependency libs
target_link_libraries(test-trnattr PRIVATE trnw trncli netif tnav qnx newmat geolib NetCDF::NetCDF)

# specify include paths
target_include_directories(test-trnattr PUBLIC
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/dorado
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${NetCDF_INCLUDE_DIRS}
)

target_compile_options(test-trnattr PUBLIC
"-std=c++11"
)
#
#------------------------------------------------------------------------------
#
# build test-trn-log

# specify build target
add_executable(test-trn-log
    ${UTILS_SRC_DIR}/test_trn_log.cpp
)

# specify dependency libs
target_link_libraries(test-trn-log PRIVATE tnav newmat qnx)

# specify include paths
target_include_directories(test-trn-log PRIVATE
BEFORE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
)
#------------------------------------------------------------------------------
# install it all
#
install(TARGETS geolib newmat qnx tnav geocon trnwc trnw netif mb1 trnucli trncli trnwcli
DESTINATION
${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS
trn-server mmcpub mmcsub mcpub mcsub mb1rs
trn-player replay mb1log-player trnlog-player csvlog-player
readlog writelog log-to-matlab log-to-csv
otree bodump octree2patches grd-octree-maker csv-octree-maker transoct
trnucli-test trnusvr-test trnclient-test trncli-test trnif-test
trnlog-test tile-test geocon-test test-trnattr test-trn-log
DESTINATION
${CMAKE_INSTALL_BINDIR}
)

