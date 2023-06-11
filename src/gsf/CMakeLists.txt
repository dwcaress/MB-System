add_library(
  mbgsf
  dump_gsf.c
  gsf_compress.c
  gsf_enc.c
  gsf_indx.c
  gsf.c
  gsf_dec.c
  gsf_geo.c
  gsf_info.c)
target_compile_definitions(mbgsf PRIVATE USE_DEFAULT_FILE_FUNCTIONS=1)
target_include_directories(mbgsf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS mbgsf DESTINATION ${CMAKE_INSTALL_LIBDIR})
