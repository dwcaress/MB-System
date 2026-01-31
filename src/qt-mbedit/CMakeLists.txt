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
  Qt6
  COMPONENTS Gui Widgets Quick OpenGL
  REQUIRED)

find_package(VTK ${MIN_VTK_VERSION} COMPONENTS GUISupportQtQuick)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qt-mbedit Backend.cpp GuiNames.cpp 
	       main.cpp qml.qrc)

target_include_directories(qt-mbedit PUBLIC ${CMAKE_SOURCE_DIR}/src/mbaux)
				 

target_link_libraries(qt-mbedit PRIVATE MBGui Qt6::Core Qt6::Quick
                      VTK::GUISupportQtQuick
                      mbeditLib mbio)

install(TARGETS qt-mbedit DESTINATION ${CMAKE_INSTALL_BINDIR})
