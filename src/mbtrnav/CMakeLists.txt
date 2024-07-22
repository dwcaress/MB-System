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

find_package(NetCDF REQUIRED)
find_package(LibPROJ REQUIRED)

add_compile_definitions(
HAVE_CONFIG_H
_GNU_SOURCE
#TRN_NORAND
WITH_TESTS
WITH_MMDEBUG
DWITH_MST_MSTATS
MST_STATS_EN
#WITH_ALT_DELTAT_VALIDATION
WITH_TNAVPF_CEP_CORRECTION
WITH_TRNLOG
WITH_TRNLOG_EST_OUT
WITH_TNAVPF_CEP_CORRECTION)

#------------------------------------------------------------------------------
#
# build libgeolib (gctp)

add_library(geolib 
       gctp/source/gctp.c
       gctp/source/alberfor.c
       gctp/source/alberinv.c
       gctp/source/alconfor.c
       gctp/source/alconinv.c
       gctp/source/azimfor.c
       gctp/source/aziminv.c
       gctp/source/eqconfor.c
       gctp/source/eqconinv.c
       gctp/source/equifor.c
       gctp/source/equiinv.c
       gctp/source/gnomfor.c
       gctp/source/gnominv.c
       gctp/source/goodfor.c
       gctp/source/goodinv.c
       gctp/source/gvnspfor.c
       gctp/source/gvnspinv.c
       gctp/source/hamfor.c
       gctp/source/haminv.c
       gctp/source/imolwfor.c
       gctp/source/imolwinv.c
       gctp/source/lamazfor.c
       gctp/source/lamazinv.c
       gctp/source/merfor.c
       gctp/source/merinv.c
       gctp/source/millfor.c
       gctp/source/millinv.c
       gctp/source/molwfor.c
       gctp/source/molwinv.c
       gctp/source/obleqfor.c
       gctp/source/obleqinv.c
       gctp/source/omerfor.c
       gctp/source/omerinv.c
       gctp/source/orthfor.c
       gctp/source/orthinv.c
       gctp/source/polyfor.c
       gctp/source/polyinv.c
       gctp/source/psfor.c
       gctp/source/psinv.c
       gctp/source/robfor.c
       gctp/source/robinv.c
       gctp/source/sinfor.c
       gctp/source/sininv.c
       gctp/source/somfor.c
       gctp/source/sominv.c
       gctp/source/sterfor.c
       gctp/source/sterinv.c
       gctp/source/stplnfor.c
       gctp/source/stplninv.c
       gctp/source/tmfor.c
       gctp/source/tminv.c
       gctp/source/utmfor.c
       gctp/source/utminv.c
       gctp/source/vandgfor.c
       gctp/source/vandginv.c
       gctp/source/wivfor.c
       gctp/source/wivinv.c
       gctp/source/wviifor.c
       gctp/source/wviiinv.c
       gctp/source/for_init.c
       gctp/source/inv_init.c
       gctp/source/cproj.c
       gctp/source/report.c
       gctp/source/lamccfor.c
       gctp/source/lamccinv.c
       gctp/source/paksz.c
       gctp/source/untfz.c
       gctp/source/sphdz.c
       gctp/source/br_gctp.c
)
#
#------------------------------------------------------------------------------
#
# build libnewmat (matrix math)

add_library(newmat 
       newmat/newmat1.cpp
       newmat/newmat2.cpp
       newmat/newmat3.cpp
       newmat/newmat4.cpp
       newmat/newmat5.cpp
       newmat/newmat6.cpp
       newmat/newmat7.cpp
       newmat/newmat8.cpp
       newmat/newmatex.cpp
       newmat/bandmat.cpp
       newmat/submat.cpp
       newmat/myexcept.cpp
       newmat/cholesky.cpp
       newmat/evalue.cpp
       newmat/fft.cpp
       newmat/hholder.cpp
       newmat/jacobi.cpp
       newmat/newfft.cpp
       newmat/sort.cpp
       newmat/svd.cpp
       newmat/newmatrm.cpp
       newmat/newmat9.cpp
)
#
#------------------------------------------------------------------------------
#
# build libqnx (utils)

add_library(qnx 
       qnx-utils/AsciiFile.cc
       qnx-utils/AngleData.cc
       qnx-utils/CharData.cc
       qnx-utils/BinaryFile.cc
       qnx-utils/DataLog.cc
       qnx-utils/DataLogReader.cc
       qnx-utils/DataLogWriter.cc
       qnx-utils/FileData.cc
       qnx-utils/StringConverter.cc
       qnx-utils/StringData.cc
       qnx-utils/TimeP.cc
       qnx-utils/TimeTag.cc
       qnx-utils/ShortData.cc
       qnx-utils/Math.cc
       qnx-utils/NavUtils.cc
       qnx-utils/IntegerData.cc
       qnx-utils/FloatData.cc
       qnx-utils/DoubleData.cc
       qnx-utils/DataField.cc
       qnx-utils/DataFieldFactory.cc
       qnx-utils/LogFile.cc
)
target_link_libraries(qnx PRIVATE geolib)
#
#------------------------------------------------------------------------------
#
# build tnav (TRN engine core)

# specify library
add_library(tnav 
       terrain-nav/TerrainNav.cpp
       terrain-nav/TNavConfig.cpp
       terrain-nav/TNavFilter.cpp
       terrain-nav/TNavPointMassFilter.cpp
       terrain-nav/TNavParticleFilter.cpp
       terrain-nav/TNavBankFilter.cpp
       terrain-nav/TNavPFLog.cpp
       terrain-nav/TerrainMapOctree.cpp
       terrain-nav/PositionLog.cpp
       terrain-nav/TerrainNavLog.cpp
       terrain-nav/mapio.cpp
       terrain-nav/structDefs.cpp
       terrain-nav/TrnLog.cpp
       terrain-nav/trn_log.cpp
       terrain-nav/myOutput.cpp
       terrain-nav/matrixArrayCalcs.cpp
       terrain-nav/TerrainMapDEM.cpp
       terrain-nav/OctreeSupport.cpp
       terrain-nav/Octree.cpp
       terrain-nav/OctreeNode.cpp
       terrain-nav/TRNUtils.cpp
)
target_include_directories(tnav PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
                                          ${NetCDF_INCLUDE_DIRS})
target_link_libraries(tnav PRIVATE newmat qnx NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build gcon

# specify library
add_library(gcon
       trnw/GeoCon.cpp
)
target_include_directories(gcon PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
                                          ${LibPROJ_INCLUDE_DIRS})
target_link_libraries(gcon PRIVATE newmat qnx LibPROJ::LibPROJ )
#
#------------------------------------------------------------------------------
#
# build trnw (TRN C wrapper)

# specify library
add_library(trnw 
       trnw/trnw.cpp
       trnw/mb1_msg.c
       trnw/trnif_msg.c
)
target_include_directories(trnw PRIVATE ${CMAKE_SOURCE_DIR}/src/mbio 
                                          ${LibPROJ_INCLUDE_DIRS}
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav 
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat 
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
                                          ${NetCDF_INCLUDE_DIRS})
target_link_libraries(trnw PRIVATE tnav newmat qnx gcon LibPROJ::LibPROJ NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build libnetif (TRNx server network interfaces)

# specify library
add_library(netif 
       trnw/netif.c
       trnw/trnif_proto.c
)
target_include_directories(netif PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
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
       trnw/mb1_msg.c
)
#
#------------------------------------------------------------------------------
#
# build trnu_cli component library

# specify library
add_library(trnucli 
       trnw/trnu_cli.c
)
target_include_directories(trnucli PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${LibPROJ_INCLUDE_DIRS}
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
target_link_libraries(trnucli PRIVATE mbtrnframe netif)
#
#------------------------------------------------------------------------------
#
# build trncli component library

# specify library
add_library(trncli
       utils/TerrainNavClient.cpp
       utils/TrnClient.cpp
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
# build trnucli-test (TRNU client app)

# specify build target
add_executable(trnucli-test
       trnw/trnucli_test.c
)
target_include_directories(trnucli-test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${LibPROJ_INCLUDE_DIRS}
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
target_link_libraries(trnucli-test PRIVATE mbtrnframe trnucli netif tnav trnw newmat qnx geolib LibPROJ::LibPROJ NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build trnusvr-test (stand-alone TRN server (no TRN instance))

# specify build target
add_executable(trnusvr-test
       trnw/trnusvr_test.c
)
target_include_directories(trnusvr-test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${LibPROJ_INCLUDE_DIRS}
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
target_link_libraries(trnusvr-test PRIVATE mbtrnframe netif tnav trnw newmat qnx geolib NetCDF::NetCDF)


##################################
# extra TRN clients

#
#------------------------------------------------------------------------------
#
#  build libtrnwcli

add_library(trnwcli STATIC
trnw/trn_cli.c
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
#  build trncli-test

add_executable(trncli-test
trnw/trncli_test.c
trnw/mb1_msg.c
)

target_link_libraries(trncli-test PRIVATE mbtrnframe trnwcli newmat geolib trnw qnx mb1 tnav gcon LibPROJ::LibPROJ NetCDF::NetCDF m)

target_include_directories(trncli-test PRIVATE ${CMAKE_SOURCE_DIR} 
								                ${LibPROJ_INCLUDE_DIRS}
												${CMAKE_SOURCE_DIR}/src/mbtrnframe trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mmcpub

add_executable(mmcpub
trnw/mmcpub.c
)

target_link_libraries(mmcpub PRIVATE mbtrnframe)
target_include_directories(mmcpub PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/mbtrnframe trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mmcsub

add_executable(mmcsub
trnw/mmcsub.c
)

target_link_libraries(mmcsub PRIVATE mbtrnframe)
target_include_directories(mmcsub PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/mbtrnframe trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mcpub

add_executable(mcpub
trnw/mcpub.c
)

target_include_directories(mcpub PRIVATE ${CMAKE_SOURCE_DIR} trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mcsub

# specify build target
add_executable(mcsub
trnw/mcsub.c
)

target_include_directories(mcsub PRIVATE ${CMAKE_SOURCE_DIR} trnw terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build trnif-test

# specify build target
add_executable(trnif-test
trnw/trnif_test.c
trnw/trnif_proto.c
trnw/trnif_msg.c
)

target_link_libraries(trnif-test PRIVATE mbtrnframe netif newmat geolib trnw qnx mb1 tnav NetCDF::NetCDF)

target_include_directories(trnif-test PRIVATE
${CMAKE_SOURCE_DIR}
${LibPROJ_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/src/mbtrnframe
trnw
terrain-nav)
#
#------------------------------------------------------------------------------
#
#  build mb1rs

add_executable(mb1rs
trnw/mb1_msg.c
trnw/mb1rs.c
trnw/mb1rs-app.c
)

target_link_libraries(mb1rs PRIVATE mbtrnframe m)
target_include_directories(mb1rs PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/mbtrnframe trnw terrain-nav)

#------------------------------------------------------------------------------
# install it all
#
install(TARGETS geolib newmat tnav qnx trnw netif mb1 trnucli trnwcli DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS trnucli-test trnusvr-test trncli-test mmcpub mmcsub mcpub mcsub trnif-test mb1rs DESTINATION ${CMAKE_INSTALL_BINDIR})
#
#------------------------------------------------------------------------------

