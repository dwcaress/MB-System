
set(executables mbphotomosaic mbgetphotocorrection mbphotogrammetry
                  mbimagecorrect mbtiff2png)

if (APPLE)

  # Set up use of pkg-config - the `pkg_check_modules` function is created with this call
  find_package(PkgConfig REQUIRED) 

  set(ENV{PKG_CONFIG_PATH} "/opt/local/lib/opencv4/pkgconfig")
  pkg_search_module(OPENCV4 REQUIRED IMPORTED_TARGET opencv4)

  foreach(executable ${executables})
    add_executable(${executable} ${executable}.cc)
    target_link_libraries(${executable} PRIVATE mbio mbaux pthread PkgConfig::OPENCV4)
  endforeach()

else()
  # Avoid problem in FindOpenCV.cmake by trying config mode first
  set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
  
  find_package(OpenCV 4 REQUIRED COMPONENTS core highgui calib3d)

  if(OpenCV_FOUND)
    message(STATUS "Found OpenCV version ${OpenCV_VERSION}")
    message(STATUS "OpenCV directories: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "OpenCV not found, please read the README.md")
  endif(OpenCV_FOUND)

  foreach(executable ${executables})
    add_executable(${executable} ${executable}.cc)
    target_link_libraries(${executable} PRIVATE mbio mbaux pthread opencv_core
                                                opencv_highgui opencv_calib3d)
  endforeach()
endif()

install(TARGETS ${executables} DESTINATION ${CMAKE_INSTALL_BINDIR})

 
