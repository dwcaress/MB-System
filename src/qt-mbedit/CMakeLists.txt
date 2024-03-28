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

find_package(
  Qt5
  COMPONENTS Gui Widgets Quick
  REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

message("In CMakeLists.txt!!! CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")

add_executable(qt-mbedit main.cpp mainwindow.cpp mainwindow.h mainwindow.ui)

# If header directories specified with ${CMAKE_SOURCE_DIR}, qtcreator cannot
# find them!
target_include_directories(qt-mbedit PUBLIC /home/oreilly/MB-System/src/mbio/
                                     /home/oreilly/MB-System/src/mbaux
                                     /home/oreilly/MB-System/src/mbedit)

target_link_libraries(qt-mbedit PRIVATE MBGui Qt5::Core Qt5::Widgets Qt5::Quick
                               	           mbeditLib mbio
					   PUBLIC TIRPC::TIRPC)

install(TARGETS qt-mbedit DESTINATION ${CMAKE_INSTALL_BINDIR})
