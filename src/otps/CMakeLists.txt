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

message("In src/otps")

add_executable(mbotps mbotps.c otps.h)

message("-- Setting otpsDir to ${otpsDir}")
file(WRITE otps.h
	"const char *otps_location = \"${otpsDir}\";")
 
target_link_libraries(mbotps PRIVATE mbio)

install(TARGETS mbotps
	DESTINATION ${CMAKE_INSTALL_BINDIR})
