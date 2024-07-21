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
#
# MB-System Terrain Relative Navigation (TRN) Tools (mbsystem/src/mbtrnutils)
#
# This directory includes the MB-System TRN program mbtrnpp, which is used
# to use new swath bathymetry data to localize a platform relative to a
# preexisting topography model, in real time or offline. Also included are two
# supporting programs having to do with obtaining the preexisting topographic
# model in the octree form used by TRN. The program mbgrd2octree converts a
# GMT grid into an octree, and mbgrdtilemaker generates tiled octrees needed
# for handling very large models.
#
message("In src/mbtrnutils")
#
#------------------------------------------------------------------------------

find_package(NetCDF REQUIRED)
find_package(LibPROJ REQUIRED)

#------------------------------------------------------------------------------

add_executable(mbtrnpp mbtrnpp.c)
target_include_directories(mbtrnpp PRIVATE ${NetCDF_INCLUDE_DIRS} ${libPROJ_INCLUDE_DIRS}
										${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                        ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
                                        ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
										${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
										${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
										${CMAKE_SOURCE_DIR}/src/mbtrn/mb1r )
target_compile_definitions(mbtrnpp PRIVATE WITH_MBTNAV)
target_link_libraries(mbtrnpp PRIVATE mbio mbaux trnw qnx newmat tnav geolib netif r7kr mb1r qnx mbtrnframe gcon NetCDF::NetCDF LibPROJ::LibPROJ )

#------------------------------------------------------------------------------

add_executable(mbgrd2octree mbgrd2octree.cc)
target_include_directories(mbgrd2octree PRIVATE ${NetCDF_INCLUDE_DIRS}
                                        ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav )
target_link_libraries(mbgrd2octree PRIVATE mbio mbaux tnav NetCDF::NetCDF )

#------------------------------------------------------------------------------

add_executable(mbgrdtilemaker mbgrdtilemaker.cc)
target_link_libraries(mbgrdtilemaker PRIVATE mbio mbaux NetCDF::NetCDF )

#------------------------------------------------------------------------------
# install it all
#
install(TARGETS mbtrnpp mbgrd2octree mbgrdtilemaker DESTINATION ${CMAKE_INSTALL_BINDIR})
#
#------------------------------------------------------------------------------
