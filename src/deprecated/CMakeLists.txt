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

message("In src/deprecated")

set(executables
    mb7k2ss
    mb7kpreprocess
    mbauvnavusbl
    mbhsdump
    mbhysweeppreprocess
    mbinsreprocess
    mbkongsbergpreprocess
    mbneptune2esf
    mbrollbias
    mbrphsbias
    mbstripnan
    mbswplspreprocess)

foreach(executable ${executables})
  add_executable(${executable} ${executable}.cc)
  target_link_libraries(${executable} mbio mbaux)
endforeach()

# mbotpsold is a deprecated C program (not C++, and needs no mbaux) that
# needs the OTPS installation location compiled in, same as mbotps
message("-- Setting otpsDir to ${otpsDir}")
string(REPLACE "\\" "\\\\" otpsDir_escaped "${otpsDir}")
file(WRITE otps.h
	"const char *otps_location = \"${otpsDir_escaped}\";")

add_executable(mbotpsold mbotpsold.c otps.h)
target_link_libraries(mbotpsold PRIVATE mbio)

install(TARGETS ${executables} mbotpsold DESTINATION ${CMAKE_INSTALL_BINDIR})

set(macros
    mbm_fmtvel
    mbm_grd2geovrml
    mbm_grid)

install(PROGRAMS ${macros} DESTINATION ${CMAKE_INSTALL_BINDIR})
