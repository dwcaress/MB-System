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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qt-mbnavedit-2 Backend.cpp 
	       main.cpp qml.qrc)

target_include_directories(qt-mbnavedit-2 PUBLIC ${CMAKE_SOURCE_DIR}/src/mbaux)

target_link_options(qt-mbnavedit-2 PUBLIC "-Wunused-variable")

target_link_libraries(qt-mbnavedit-2 PRIVATE MBGui Qt5::Core Qt5::Quick
                      mbio)

install(TARGETS qt-mbedit DESTINATION ${CMAKE_INSTALL_BINDIR})
