message(STATUS "In src/mbaux: CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(GDAL CONFIG)
if (NOT GDAL_FOUND)
  message(STATUS "Package GDAL config not found, looking for FindGDAL.cmake file...")
  find_package(GDAL REQUIRED)
endif (NOT GDAL_FOUND)

find_package(GMT CONFIG)
if (NOT GMT_FOUND)
  message(STATUS "Package GMT config not found, looking for FindGMT.cmake file...")
  find_package(GMT REQUIRED)
endif (NOT GMT_FOUND)

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

install(TARGETS mbaux DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(buildGUIs)
  find_package(X11 REQUIRED)
  add_library(mbxgr mb_xgraphics.c)
  set_target_properties(mbxgr PROPERTIES VERSION "0" SOVERSION "0")
  target_link_libraries(mbxgr GMT::GMT GDAL::GDAL mbio ${X11_LIBRARIES})

  install(TARGETS mbxgr DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
