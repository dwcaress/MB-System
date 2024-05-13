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

message("In src/utilities")

find_package(FFTW REQUIRED)
find_package(LibPROJ REQUIRED)

set(executables
    mbabsorption
    mbdatalist
    mbgrid
    mbnavlist
    mbsegypsd
    mbareaclean
    mbdefaults
    mbhistogram
    mbpreprocess
    mbset
    mbauvloglist
    mbdumpesf
    mbinfo
    mbprocess
    mbsslayout
    mbbackangle
    mbextractsegy
    mbrolltimelag
    mbsvplist
    mbclean
    mbfilter
    mblist
    mbroutetime
    mbsvpselect
    mbformat
    mbmakeplatform
    mbsegygrid
    mbtime
    mbcopy
    mbgetesf
    mbminirovnav
    mbsegyinfo
    mbvoxelclean
    mbctdlist
    mbgpstide
    mbmosaic
    mbsegylist)

foreach(executable ${executables})
  add_executable(${executable} ${executable}.cc)
  target_link_libraries(${executable} PRIVATE mbio mbbsio mbaux mbsapi mbgsf m pthread)
endforeach()

add_executable(mblevitus mblevitus.cc levitus.h)
target_link_libraries(mblevitus PRIVATE mbio mbbsio mbaux mbsapi mbgsf m pthread)

add_executable(mbconfig mbconfig.cc levitus.h)
target_link_libraries(mbconfig PRIVATE mbio mbbsio mbaux mbsapi mbgsf m pthread)

file(WRITE levitus.h
  "const char *levitusfile = \"${CMAKE_INSTALL_PREFIX}/${levitusDir}/LevitusAnnual82.dat\";\nconst char *otps_location = \"${otpsDir}\";\n")

target_link_libraries(mbsvpselect PRIVATE LibPROJ::LibPROJ)
target_link_libraries(mbgrid PRIVATE mbaux)
target_link_libraries(mbsegypsd PRIVATE FFTW::Double)
target_compile_definitions(
  mbconfig
  PRIVATE MBSYSTEM_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
          ENV_OTPSDIR="${otpsDir}"
          ENV_LEVITUSANNUAL82="${levitusDir}/LevitusAnnual82.dat")
target_compile_definitions(
  mblevitus PRIVATE ENV_LEVITUSANNUAL82="${CMAKE_INSTALL_PREFIX}/${levitusDir}/LevitusAnnual82.dat")

install(TARGETS ${executables} mbconfig mblevitus DESTINATION ${CMAKE_INSTALL_BINDIR})
