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
  COMPONENTS Gui Quick Graphs
  REQUIRED)
  
find_package(GMT REQUIRED)

# If VTK 9.5 is from Homebrew it was built against external NetCDF and
# its exported targets reference NetCDF::NetCDF. Make that target exist before
# find_package(VTK) so VTK-targets.cmake doesn't error.
find_package(netCDF CONFIG QUIET)
if(netCDF_FOUND AND NOT TARGET NetCDF::NetCDF)
  add_library(NetCDF::NetCDF INTERFACE IMPORTED)
  target_link_libraries(NetCDF::NetCDF INTERFACE netCDF::netcdf)
endif()


message("Find minimum VTK_VERSION ${MIN_VTK_VERSION}")
find_package(VTK ${MIN_VTK_VERSION} COMPONENTS GUISupportQtQuick RenderingLabel)
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
    DataSelectInteractorStyle.cpp    
    DisplayProperties.cpp
    DrawInteractorStyle.cpp
    Emitter.cpp
    GmtGridData.cpp
    GmtGridReader.cpp
    GmtGridSurface.cpp
    LightingInteractorStyle.cpp
    MBGui.cpp
    MBQuickItem.cpp
    MyRubberBandStyle.cpp
    PickInteractorStyle.cpp
    PixmapDrawer.cpp    
    PixmapImage.cpp
    PointsSelectInteractorStyle.cpp
    RadioButtonGroup.cpp
    RestrictCameraStyle.cpp
    Route.cpp
    SharedConstants.cpp
    SlopeShader.cpp
    Surface.cpp
    SurfaceRenderer.cpp
    SwathData.cpp
    TopoColorMap.cpp
    TopoData.cpp
    TopoDataItem.cpp
    TopoDataItemSettings.cpp    
    TopoDataReader.cpp
    TopoProfileItem.cpp    
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
  PRIVATE Qt6::Core Qt6::Quick Qt6::Graphs mbio
  PUBLIC mbeditvizlib
  PUBLIC VTK::CommonColor
  VTK::CommonSystem
  VTK::ChartsCore
  VTK::pugixml
  VTK::CommonComputationalGeometry
  VTK::CommonExecutionModel
  VTK::DICOMParser
  VTK::FiltersExtraction
  VTK::FiltersProgrammable
  VTK::FiltersGeometry
  VTK::FiltersHybrid
  VTK::FiltersHyperTree
  VTK::FiltersModeling
  VTK::FiltersSources
  VTK::FiltersStatistics
  VTK::FiltersTexture
  VTK::FiltersVerdict
  VTK::GUISupportQt
  VTK::GUISupportQtQuick	 
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
  VTK::RenderingContext2D
  VTK::RenderingCore
  VTK::RenderingLabel
  VTK::RenderingFreeType
  VTK::RenderingOpenGL2
  VTK::RenderingVolume	 
  VTK::RenderingUI
  VTK::ViewsContext2D
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
   VTK::netcdf
   GMT::GMT
    LibPROJ::LibPROJ)

vtk_module_autoinit(
  TARGETS MBGui
  MODULES VTK::RenderingLabel
)

install(TARGETS MBGui DESTINATION ${CMAKE_INSTALL_LIBDIR})
