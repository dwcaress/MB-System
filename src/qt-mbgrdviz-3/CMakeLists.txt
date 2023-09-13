find_package(
  Qt5
  COMPONENTS Gui Widgets Quick
  REQUIRED)
find_package(VTK REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qt-mbgrdviz-3 main.cpp qml.qrc)
target_link_libraries(qt-mbgrdviz-3 PRIVATE MBGui Qt5::Core Qt5::Quick)
