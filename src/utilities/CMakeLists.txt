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
    mblevitus
    mbrolltimelag
    mbsvplist
    mbclean
    mbfilter
    mblist
    mbroutetime
    mbsvpselect
    mbconfig
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
  target_link_libraries(${executable} PRIVATE mbio mbbsio mbaux mbsapi m
                                              pthread)
endforeach()

target_link_libraries(mbsvpselect PRIVATE LibPROJ::LibPROJ)
target_link_libraries(mbgrid PRIVATE mbaux)
target_link_libraries(mbsegypsd PRIVATE FFTW::Double)
target_compile_definitions(
  mbconfig
  PRIVATE MBSYSTEM_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
          otpsDir="${otpsDir}"
          LevitusAnnual82="${levitusDir}/LevitusAnnual82.dat")
target_compile_definitions(
  mblevitus PRIVATE LevitusAnnual82="${levitusDir}/LevitusAnnual82.dat")

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})
