find_package(
  Qt5
  COMPONENTS Gui Widgets Quick
  REQUIRED)
find_package(VTK REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(topoGridViewer TopoGridViewer.cpp)

target_include_directories(topoGridViewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(topoGridViewer PRIVATE
                      MBGui
                      PUBLIC
                      ${VTK_LIBRARIES}
                      ) 

install(TARGETS topoGridViewer DESTINATION ${CMAKE_INSTALL_BINDIR})
