find_package(
  Qt5
  COMPONENTS Gui Quick
  REQUIRED)
find_package(GMT REQUIRED)
find_package(VTK REQUIRED)

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
target_include_directories(MBGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  MBGui
  PRIVATE Qt5::Core Qt5::Quick mbio mbeditvizlib
  PUBLIC VTK::CommonCore
         VTK::CommonExecutionModel
         VTK::IOGeometry
         VTK::InteractionStyle
         VTK::RenderingAnnotation
         VTK::RenderingCore
         VTK::RenderingFreeType
         VTK::RenderingOpenGL2
         VTK::RenderingUI
         GMT::GMT)

install(TARGETS MBGui DESTINATION cmake-example/lib)
