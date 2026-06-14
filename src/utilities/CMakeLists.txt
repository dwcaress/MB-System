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
    mb7k2jstar
    mbabsorption
    mbareaclean
    mbauvloglist
    mbbackangle
    mbclean
    mbcopy
    mbctdlist
    mbdatalist
    mbdefaults
    mbdumpesf
    mbextractsegy
    mbfilter
    mbfnv2navlab
    mbformat
    mbgetesf
    mbgpstide
    mbgrid
    mbhistogram
    mbinfo
    mblist
    mbnavlab2fnv
    mbmakedatalist
    mbmakeplatform
    mbmapscale
    mbminirovnav
    mbmosaic
    mbnavlist
    mbprocess
    mbrolltimelag
    mbroutetime
    mbsegygrid
    mbsegyinfo
    mbsegylist
    mbsegypsd
    mbset
    mbsslayout
    mbsvplist
    mbsvpselect
    mbtime
    mbvoxelclean)

foreach(executable ${executables})
  add_executable(${executable} ${executable}.cc)
  target_link_libraries(${executable} PRIVATE mbio mbbsio mbaux mbsapi mbgsf ${MB_SYSLIBS})
  # MSVC default stack is 1 MB; mbfilter / mbauvloglist crash with stack
  # overflow when stdin is a pipe. Bump reserve to 8 MB.
  if(MSVC)
    target_link_options(${executable} PRIVATE /STACK:8388608)
  endif()
endforeach()

add_executable(mbpreprocess mbpreprocess.cc)
target_link_libraries(mbpreprocess PRIVATE mbio mbbsio mbaux mbsapi mbgsf ${MB_SYSLIBS})
if(MSVC)
  target_link_options(mbpreprocess PRIVATE /STACK:8388608)
endif()

add_executable(mblevitus mblevitus.cc levitus.h)
target_link_libraries(mblevitus PRIVATE mbio mbbsio mbaux mbsapi mbgsf ${MB_SYSLIBS})

add_executable(mbconfig mbconfig.cc levitus.h)
target_link_libraries(mbconfig PRIVATE mbio mbbsio mbaux mbsapi mbgsf ${MB_SYSLIBS})

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

install(TARGETS ${executables} mbpreprocess mbconfig mblevitus DESTINATION ${CMAKE_INSTALL_BINDIR})
