#
#

message("in gsf")

# Static library
add_library(gsf
            STATIC
            dump_gsf.c  gsf_compress.c  gsf_enc.c  gsf_indx.c
            gsf.c       gsf_dec.c       gsf_geo.c  gsf_info.c)


target_compile_definitions(gsf PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(gsf
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


# Shared library
add_library(gsfShared
            SHARED
            dump_gsf.c  gsf_compress.c  gsf_enc.c  gsf_indx.c
            gsf.c       gsf_dec.c       gsf_geo.c  gsf_info.c)


target_compile_definitions(gsfShared PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(gsfShared
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


