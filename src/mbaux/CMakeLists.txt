find_package(GDAL REQUIRED)
find_package(GMT REQUIRED)

add_library(
  mbaux
  mb_cheb.c
  mb_delaun.c
  mb_intersectgrid.c
  mb_readwritegrd.c
  mb_surface.c
  mb_track.c
  mb_truecont.c
  mb_zgrid.c)

set_target_properties(mbaux PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbaux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbaux GMT::GMT GDAL::GDAL mbio)

if(buildGUIs)
  find_package(X11 REQUIRED)
  add_library(mbxgr mb_xgraphics.c)
  set_target_properties(mbxgr PROPERTIES VERSION "0" SOVERSION "0")
  target_link_libraries(mbxgr GMT::GMT GDAL::GDAL mbio X11::X11)

  install(TARGETS mbaux mbxgr DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
