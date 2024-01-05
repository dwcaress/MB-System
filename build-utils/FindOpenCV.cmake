#--------------------------------------------------------------------
#    The MB-system:    build-utils/FindOpenCV.cmake    11/3/2023
#
#    Copyright (c) 2023-2023 by
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
#--------------------------------------------------------------------*/
# 
# Find OpenCV Library (version 4) (https://github.com/opencv/opencv)
#
set (OPENCV_FOUND FALSE)

# First try find_package() in config mode
if (NOT OPENCV_FOUND)

  find_package(OpenCV 4 COMPONENTS core highgui calib3d CONFIG)

  if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    foreach(executable ${executables})
      add_executable(${executable} ${executable}.cc)
      target_link_libraries(${executable} PRIVATE 
                  mbio mbaux pthread opencv_core
                  opencv_highgui opencv_calib3d)
    endforeach()
  endif(OpenCV_FOUND)

endif()

#
# Else if pkg-config exists then look for opencv4
if (NOT OPENCV_FOUND)

  find_package(PkgConfig)
  if (PkgConfig_FOUND)
    if (APPLE)
      set(ENV{PKG_CONFIG_PATH} "/opt/local/lib/opencv4/pkgconfig")
    endif(APPLE)
    pkg_search_module(OPENCV4 IMPORTED_TARGET opencv4)

    if (OPENCV4_FOUND)
      foreach(executable ${executables})
        add_executable(${executable} ${executable}.cc)
        target_link_libraries(${executable} PRIVATE mbio mbaux pthread PkgConfig::OPENCV4)
      endforeach()
      set(OPENCV_FOUND TRUE)
    endif(OPENCV4_FOUND)

  endif(PkgConfig_FOUND)
endif(NOT OPENCV_FOUND)

if (OPENCV_FOUND)
  message("-- OpenCV found!")
else()
  message ("-- OpenCV NOT found...")
endif ()
