message("In src/deprecated")

set(executables
    mb7k2jstar
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

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})
