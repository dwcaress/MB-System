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

message("In src/gmt")

find_package(GDAL CONFIG)
if (NOT GDAL_FOUND)
  message(STATUS "GDAL package config file GDALConfig.cmake not found, looking for FindGDAL.cmake file...")
  find_package(GDAL REQUIRED)
  if (NOT GDAL_FOUND)
     message(STATUS "GDAL still not found")
  else ()
     message(STATUS "FOUND GDAL")
  endif (NOT GDAL_FOUND)
  
endif (NOT GDAL_FOUND)

find_package(GMT CONFIG)
if (NOT GMT_FOUND)
  message(STATUS "GMT package config file GMTConfig.cmake not found, looking for FindGMT.cmake file...")
  find_package(GMT REQUIRED)
endif (NOT GMT_FOUND)

add_library(mbsystem 
    mbcontour.c 
    mbgrd2obj.c 
    #mbgrdtiff.c NOTE: new version doesn't work, still using pre-GMT6 version
    mbgrdtifforg.c 
    mbswath.c)

set_target_properties(mbsystem PROPERTIES VERSION "0" SOVERSION "0" PREFIX "" OUTPUT_NAME mbsystem)
target_compile_definitions(mbsystem PUBLIC MB_PACKAGE_VERSION="${PROJECT_VERSION}"
                                       MB_PACKAGE_DATE="${PROJECT_DATE}")
target_include_directories(mbsystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbsystem GMT::GMT GMT::PSL mbio mbaux)

install(TARGETS mbsystem DESTINATION ${CMAKE_INSTALL_LIBDIR})
