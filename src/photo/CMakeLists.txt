find_package(OpenCV 4 REQUIRED COMPONENTS core highgui calib3d)

set(executables mbphotomosaic mbgetphotocorrection mbphotogrammetry
                mbimagecorrect mbtiff2png)

foreach(executable ${executables})
  add_executable(${executable} ${executable}.cc)
  target_link_libraries(${executable} PRIVATE mbio mbaux pthread opencv_core
                                              opencv_highgui opencv_calib3d)
endforeach()

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})
