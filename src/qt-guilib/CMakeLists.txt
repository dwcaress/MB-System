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
  COMPONENTS Gui Quick
  REQUIRED)
find_package(GMT REQUIRED)
find_package(VTK REQUIRED)
message("Found VTK_VERSION ${VTK_VERSION}")

find_package(LibPROJ REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(src
    TopoColorMap.cpp
    TopoGridReader.cpp
    Camera.cpp
    MBQuickItem.cpp
    ColorMap.cpp
    DisplayProperties.cpp
    MBQuickView.cpp
    QVtkItem.cpp
    GmtGridReader.cpp
    TopoGridData.cpp
    GmtGridData.cpp
    MBQuickItem.cpp
    QVtkRenderer.cpp
    GmtGridSurface.cpp
    DataPointsOverlay.cpp
    Route.cpp
    Surface.cpp
    SwathGridData.cpp
    MBGui.cpp
    SurfaceRenderer.cpp
    PickerInteractorStyle.cpp
    Utilities.cpp
    ClickableLabel.cpp
    resources.qrc)

add_library(MBGui ${src})
set_target_properties(MBGui PROPERTIES VERSION "0" SOVERSION "0")

if (APPLE)
  set_target_properties(MBGui PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
endif (APPLE)

target_include_directories(MBGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  MBGui
  PRIVATE Qt5::Core Qt5::Quick mbio mbeditvizlib
  PUBLIC VTK::CommonColor
  	 VTK::CommonSystem
	 VTK::pugixml
	 VTK::CommonComputationalGeometry
         VTK::CommonExecutionModel
	 VTK::DICOMParser
	 VTK::FiltersExtraction
	 VTK::FiltersGeometry
	 VTK::FiltersHybrid
	 VTK::FiltersHyperTree
	 VTK::FiltersModeling
	 VTK::FiltersSources
	 VTK::FiltersStatistics
	 VTK::FiltersTexture
	 VTK::FiltersVerdict
	 VTK::GUISupportQt
	 VTK::ImagingColor
	 VTK::ImagingGeneral
	 VTK::ImagingHybrid
	 VTK::ImagingSources
	 VTK::IOImage
         VTK::IOGeometry
         VTK::IOXML
         VTK::InteractionStyle
	 VTK::ParallelDIY
         VTK::RenderingAnnotation
         VTK::RenderingCore
         VTK::RenderingFreeType
         VTK::RenderingOpenGL2
         VTK::RenderingVolume	 
         VTK::RenderingUI
	 VTK::loguru
	 VTK::fmt
	 VTK::verdict
	 VTK::doubleconversion
	 VTK::expat
	 VTK::lz4
	 VTK::lzma
	 VTK::metaio
	 VTK::jpeg
	 VTK::png
	 VTK::tiff
         GMT::GMT
	 LibPROJ::LibPROJ)

install(TARGETS MBGui DESTINATION cmake-example/lib)
