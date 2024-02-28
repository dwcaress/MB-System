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

message("In src/gsf")

add_library(
  mbgsf
  dump_gsf.c
  gsf_compress.c
  gsf_enc.c
  gsf_indx.c
  gsf.c
  gsf_dec.c
  gsf_geo.c
  gsf_info.c)
target_compile_definitions(mbgsf PRIVATE USE_DEFAULT_FILE_FUNCTIONS=1)
target_include_directories(mbgsf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS mbgsf DESTINATION ${CMAKE_INSTALL_LIBDIR})
