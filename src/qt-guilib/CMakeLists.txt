find_package(
  Qt5
  COMPONENTS Gui Quick
  REQUIRED)
find_package(GMT REQUIRED)
find_package(VTK REQUIRED)

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
    Surface.cpp
    SwathGridData.cpp
    MBGui.cpp
    SurfaceRenderer.cpp
    PickerInteractorStyle.cpp
    Utilities.cpp
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
