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

add_library(mbgmtmodules 
    mbcontour.c 
    mbgrd2obj.c 
    #mbgrdtiff.c NOTE: new version doesn't work, still using pre-GMT6 version
    mbgrdtifforg.c 
    mbswath.c)

set_target_properties(mbgmtmodules PROPERTIES VERSION "0" SOVERSION "0")
target_compile_definitions(mbgmtmodules PUBLIC MB_PACKAGE_VERSION="${PROJECT_VERSION}"
                                       MB_PACKAGE_DATE="${PROJECT_DATE}")
target_include_directories(mbgmtmodules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbgmtmodules GMT::GMT GMT::PSL mbio mbaux)

install(TARGETS mbgmtmodules DESTINATION ${CMAKE_INSTALL_LIBDIR})
