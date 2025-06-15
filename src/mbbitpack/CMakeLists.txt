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
# MBbitpack library
#
# The library is:
#    mbbitpack
#
message("In src/mbbitpack")
#
#------------------------------------------------------------------------------
#
# build libmbbitpack

add_library(mbbitpack 
       mb_bitpack.cc)

target_include_directories(mbbitpack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#
#------------------------------------------------------------------------------
# install it all
#
install(TARGETS mbbitpack DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#------------------------------------------------------------------------------

