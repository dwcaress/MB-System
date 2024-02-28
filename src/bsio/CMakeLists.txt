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

message("In src/bsio")

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

add_library(
  mbbsio
  mbbs_io.c
  mbbs_iomrv1.c
  mbbs_iomrv2.c
  mbbs_memalloc.c
  mbbs_misc.c
  mbbs_swapbytes.c
  mbbs_tm.c
  mbbs_wrhdrfields.c
  mbbs_wrsampflags.c)

set_target_properties(mbbsio PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbbsio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbbsio PRIVATE TIRPC::TIRPC)

install(TARGETS mbbsio DESTINATION ${CMAKE_INSTALL_LIBDIR})
