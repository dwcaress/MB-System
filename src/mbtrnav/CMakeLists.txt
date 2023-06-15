#------------------------------------------------------------------------------
#
# MBtrnav libraries
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
#
message("In mbtrnav")
#
#------------------------------------------------------------------------------

find_package(NetCDF REQUIRED)

#------------------------------------------------------------------------------
#
# build libgeolib (gctp)

add_library(geolib STATIC
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

add_library(newmat STATIC
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

add_library(qnx STATIC
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
#
#------------------------------------------------------------------------------
#
# build tnav (TRN engine core)

# specify library
add_library(tnav STATIC
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
#
#------------------------------------------------------------------------------
#
# build trnw (TRN C wrapper)

# specify library
add_library(trnw STATIC
       trnw/trnw.cpp
       trnw/mb1_msg.c
       trnw/trnif_msg.c
)
target_include_directories(trnw PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav 
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat 
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
                                          ${NetCDF_INCLUDE_DIRS})
#
#------------------------------------------------------------------------------
#
# build libnetif (TRNx server network interfaces)

# specify library
add_library(netif STATIC
       trnw/netif.c
       trnw/trnif_proto.c
)
target_include_directories(netif PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
#
#------------------------------------------------------------------------------
#
# build libmb1

# specify library
add_library(mb1 STATIC
       trnw/mb1_msg.c
)
#
#------------------------------------------------------------------------------
#
# build trnu_cli component library

# specify library
add_library(trnucli STATIC
       trnw/trnu_cli.c
)
target_include_directories(trnucli PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
#
#------------------------------------------------------------------------------
#
# build trnucli-test (TRNU client app)

# specify build target
add_executable(trnucli-test
       trnw/trnucli_test.c
)
target_include_directories(trnucli-test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
target_link_libraries(trnucli-test PRIVATE mbtrnframe trnucli netif tnav trnw newmat qnx geolib NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build trnusvr-test (stand-alone TRN server (no TRN instance))

# specify build target
add_executable(trnusvr-test
       trnw/trnusvr_test.c
)
target_include_directories(trnusvr-test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                          ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
target_link_libraries(trnusvr-test PRIVATE mbtrnframe netif tnav trnw newmat qnx geolib NetCDF::NetCDF)
#------------------------------------------------------------------------------
# install it all
#
install(TARGETS mbio DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#------------------------------------------------------------------------------

