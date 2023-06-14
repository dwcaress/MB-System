if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

add_library(
  mbbsio
  mbbs_io.c
  mbbs_iomrv1.c
  mbbs_iomrv2.c
  mbbs_memalloc.c
  mbbs_misc.c
  mbbs_swapbytes.c
  mbbs_tm.c
  mbbs_wrhdrfields.c
  mbbs_wrsampflags.c)

set_target_properties(mbbsio PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbbsio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbbsio PRIVATE TIRPC::TIRPC)

install(TARGETS mbbsio mbbsio DESTINATION ${CMAKE_INSTALL_LIBDIR})
