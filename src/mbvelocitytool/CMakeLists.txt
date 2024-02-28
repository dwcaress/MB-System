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

message("In src/mbvelocitytool")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(
  mbvelocitytool mbvelocity.c mbvelocity_bxutils.c mbvelocity_callbacks.c
                 mbvelocity_creation.c mbvelocity_prog.c)

target_link_libraries(mbvelocitytool PRIVATE mbaux mbxgr 
                      ${MOTIF_LIBRARIES}
		      ${X11_LIBRARIES}
		      ${X11_Xt_LIB})

install(TARGETS mbvelocitytool DESTINATION ${CMAKE_INSTALL_BINDIR})
