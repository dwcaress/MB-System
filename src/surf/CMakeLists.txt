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

message("In src/surf")

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

add_library(
  mbsapi
  mem_surf.c
  pb_math.c
  sapi_data.c
  sapi_files.c
  sapi_globals.c
  sapi_simple.c
  util_surf.c
  xdr_surf.c)

set_target_properties(mbsapi PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbbsio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbsapi PUBLIC TIRPC::TIRPC)

install(TARGETS mbsapi DESTINATION ${CMAKE_INSTALL_LIBDIR})
