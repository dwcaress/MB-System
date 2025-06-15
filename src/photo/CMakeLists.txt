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

message("In src/photo")

set(executables mbphotomosaic mbgetphotocorrection mbphotogrammetry
                  mbimagecorrect mbtiff2png mbimagelist)
set(scripts mbm_makeimagelist)

find_package(OpenCV 4 REQUIRED COMPONENTS core highgui calib3d)

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS ${scripts} DESTINATION ${CMAKE_INSTALL_BINDIR})

 