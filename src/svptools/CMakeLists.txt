#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   3 November 2026
#
#    Copyright (c) 2026-2026 by
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

message("In src/svptools")

find_package(NetCDF REQUIRED)

add_library(mbsvptools STATIC
    mbsvp_location.c
    mbsvp_profile.c
    mbsvp_score.c
    mbsvp_tools.h
    mbsvp_util.c
    mbsvp_woa.c)
target_include_directories(mbsvptools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbsvptools PUBLIC mbio NetCDF::NetCDF)

add_executable(mbsvpbest mbsvpbest.c)
target_link_libraries(mbsvpbest PRIVATE mbsvptools ${MB_SYSLIBS})

install(TARGETS mbsvpbest DESTINATION ${CMAKE_INSTALL_BINDIR})
