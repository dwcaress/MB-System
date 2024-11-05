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
# MBtrn libraries and test programs
#
# This group of libraries and programs are MB-System only utilities supporting
# Terrain Relative Navigation (TRN) software. 
# The libraries are:
#    r7kr
#    mb1r
#
# The test programs are:
#    udps
#.   udpc
#.   mbtnav_cli
#.   stream7k
#.   frames7k
#.   emu7k
#.   r7kr-test
#.   trnc
#.   tbinx
#.   mb1conv
#.   mb12csv
#.   mb1r_test
#.   mb1-cli
#
message("In src/mbtrn")

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

add_compile_definitions(
HAVE_CONFIG_H
_GNU_SOURCE
WITH_TESTS
WITH_R7KR_TEST
WITH_MB1R_TEST
WITH_MST_MSTATS
MST_STATS_EN
WITH_MMDEBUG)
#
#------------------------------------------------------------------------------
#
add_library(r7kr 
	utils/mb71_msg.c
	r7kr/r7kc.c 
	r7kr/r7k-reader.c
)
target_include_directories(r7kr PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(r7kr PRIVATE mbtrnframe trnw)
#
#------------------------------------------------------------------------------
#
add_library(mb1r 
	utils/mb71_msg.c
	mb1r/mb1-reader.c 
)
target_include_directories(mb1r PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
                              ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw 
                              ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(mb1r PRIVATE mbtrnframe trnw)

install(TARGETS r7kr mb1r DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#------------------------------------------------------------------------------
#
add_executable(udps utils/udps.c)
target_include_directories(udps PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe)
target_link_libraries(udps PRIVATE mbtrnframe m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(udpc utils/udpc.c)
target_include_directories(udpc PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe)
target_link_libraries(udpc PRIVATE mbtrnframe m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(mbtnav_cli utils/mbtnav_cli.c)
target_include_directories(mbtnav_cli PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
							${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(mbtnav_cli PRIVATE mbtrnframe m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(stream7k utils/stream7k.c)
target_include_directories(stream7k PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                            ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
                            ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(stream7k PRIVATE mbtrnframe r7kr)
#
#------------------------------------------------------------------------------
#
add_executable(frames7k utils/frames7k.c)
target_include_directories(frames7k PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                            ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
                            ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(frames7k PRIVATE mbtrnframe r7kr)
#
#------------------------------------------------------------------------------
#
add_executable(emu7k utils/emu7k.c)
target_include_directories(emu7k PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
							${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(emu7k PRIVATE mbtrnframe r7kr m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(r7kr_test r7kr/r7kr-test.c)
target_include_directories(r7kr_test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr)
target_link_libraries(r7kr_test PRIVATE r7kr)
#
#------------------------------------------------------------------------------
#
add_executable(trnc utils/trnc.c)
target_include_directories(trnc PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw 
                            ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(trnc PRIVATE mbtrnframe mb1r)
#
#------------------------------------------------------------------------------
#
add_executable(tbinx utils/tbinx.c)
target_include_directories(tbinx PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw 
                            ${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(tbinx PRIVATE mbtrnframe mb1r r7kr m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(mb1conv utils/mb1conv.c)
target_include_directories(mb1conv PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
target_link_libraries(mb1conv PRIVATE mbtrnframe mb1r mb1 m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(mb12csv utils/mb12csv.c)
target_include_directories(mb12csv PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe 
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
target_link_libraries(mb12csv PRIVATE mbtrnframe mb1r mb1 m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(mb1r_test mb1r/mb1r-test.c)
target_include_directories(mb1r_test PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
							${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
target_link_libraries(mb1r_test PRIVATE mb1r m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(mb1-cli mb1r/mb1cli-app.c)
target_include_directories(mb1-cli PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                            ${LibPROJ_INCLUDE_DIRS}
                            ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
target_link_libraries(mb1-cli PRIVATE mbtrnframe mb1r mb1 m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(emallpub utils/emallpub.c)
target_include_directories(emallpub PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                            ${CMAKE_SOURCE_DIR}/src/mbio
							${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(emallpub PRIVATE TIRPC::TIRPC mbtrnframe m pthread)
#
#------------------------------------------------------------------------------
#
add_executable(emserpub utils/emserpub.c)
target_include_directories(emserpub PRIVATE ${CMAKE_SOURCE_DIR}/src/mbtrnframe
                            ${CMAKE_SOURCE_DIR}/src/mbio
							${CMAKE_SOURCE_DIR}/src/mbtrnutils)
target_link_libraries(emserpub PRIVATE TIRPC::TIRPC mbtrnframe m pthread)
#
#------------------------------------------------------------------------------
#
install(TARGETS r7kr mb1r DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
install(TARGETS udps udpc mbtnav_cli stream7k frames7k emu7k r7kr_test trnc tbinx mb1conv mb12csv mb1r_test mb1-cli emallpub emserpub DESTINATION ${CMAKE_INSTALL_BINDIR})
#
#------------------------------------------------------------------------------
#
