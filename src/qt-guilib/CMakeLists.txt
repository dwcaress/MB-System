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
  COMPONENTS Gui Quick
  REQUIRED)
find_package(GMT REQUIRED)
find_package(VTK 9.3 REQUIRED)
message("Found VTK_VERSION ${VTK_VERSION}")

find_package(LibPROJ REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(src
    Camera.cpp
    ClickableLabel.cpp
    ColorMap.cpp
    CPixmapDrawer.cpp
    DataPointsOverlay.cpp
    DisplayProperties.cpp
    Emitter.cpp
    GmtGridData.cpp
    GmtGridReader.cpp
    GmtGridSurface.cpp
    MBGui.cpp
    MBQuickItem.cpp
    PickerInteractorStyle.cpp
    PixmapDrawer.cpp    
    PixmapImage.cpp
    QVtkItem.cpp
    QVtkRenderer.cpp
    Route.cpp
    SharedConstants.cpp
    SlopeFilter.cpp
    Surface.cpp
    SurfaceRenderer.cpp
    SwathData.cpp
    TopoColorMap.cpp
    TopoData.cpp
    TopoDataItem.cpp
    TopoDataPickerInteractorStyle.cpp
    TopoDataReader.cpp
    Utilities.cpp
    resources.qrc)

add_library(MBGui ${src})
set_target_properties(MBGui PROPERTIES VERSION "0" SOVERSION "0")

if (APPLE)
  set_target_properties(MBGui PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
endif (APPLE)

target_include_directories(MBGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
				 ${CMAKE_SOURCE_DIR}/src/mbaux
				 ${CMAKE_SOURCE_DIR}/src/mbeditviz
				 )
target_link_libraries(
  MBGui
  PRIVATE Qt6::Core Qt6::Quick mbio
  PUBLIC mbeditvizlib
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
