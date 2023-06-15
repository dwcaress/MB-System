
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
message("In mbtrnutils")
#
#------------------------------------------------------------------------------

find_package(NetCDF REQUIRED)

#------------------------------------------------------------------------------

add_executable(mbtrnpp mbtrnpp.c)
target_include_directories(mbtrnpp PRIVATE ${NetCDF_INCLUDE_DIRS}
										${CMAKE_SOURCE_DIR}/src/mbtrnframe
                                        ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
                                        ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
										${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
										${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
										${CMAKE_SOURCE_DIR}/src/mbtrn/mb1r )
target_compile_definitions(mbtrnpp PRIVATE WITH_MBTNAV)
target_link_libraries(mbtrnpp PRIVATE mbio mbaux trnw qnx newmat tnav geolib netif r7kr mb1r qnx mbtrnframe NetCDF::NetCDF )

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
