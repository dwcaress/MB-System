find_package(
  Qt5
  COMPONENTS Gui Widgets Quick
  REQUIRED)
find_package(VTK REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(topoGridViewer TopoGridViewer.cpp)

target_link_libraries(topoGridViewer PRIVATE 
                                          mbeditvizlib)

install(TARGETS topoGridViewer DESTINATION ${CMAKE_INSTALL_BINDIR})
