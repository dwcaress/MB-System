#
# Variables are passed from top-level CMakeLists.txt

message("in gsf")

# Static library
add_library(mbgsf
            STATIC
            dump_gsf.c  gsf_compress.c  gsf_enc.c  gsf_indx.c
            gsf.c       gsf_dec.c       gsf_geo.c  gsf_info.c)


target_compile_definitions(mbgsf PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(mbgsf
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


# Shared library
add_library(mbgsfShared
            SHARED
            dump_gsf.c  gsf_compress.c  gsf_enc.c  gsf_indx.c
            gsf.c       gsf_dec.c       gsf_geo.c  gsf_info.c)

set_target_properties(mbgsfShared PROPERTIES OUTPUT_NAME mbgsf)

target_compile_definitions(mbgsfShared PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(mbgsfShared
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


install(TARGETS mbgsf mbgsfShared
	DESTINATION ${CMAKE_INSTALL_LIBDIR})
