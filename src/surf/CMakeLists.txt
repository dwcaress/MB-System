message("In src/surf")

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

add_library(
  mbsapi
  mem_surf.c
  pb_math.c
  sapi_data.c
  sapi_files.c
  sapi_globals.c
  sapi_simple.c
  util_surf.c
  xdr_surf.c)

set_target_properties(mbsapi PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbbsio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbsapi PUBLIC TIRPC::TIRPC)

install(TARGETS mbsapi DESTINATION ${CMAKE_INSTALL_LIBDIR})
