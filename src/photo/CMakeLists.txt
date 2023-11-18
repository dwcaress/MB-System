message("In src/photo")

set(executables mbphotomosaic mbgetphotocorrection mbphotogrammetry
                  mbimagecorrect mbtiff2png)

find_package(OpenCV 4 REQUIRED COMPONENTS core highgui calib3d)

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})

 