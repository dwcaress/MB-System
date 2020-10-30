#
#

message("in surf")

# Static library
add_library(mbsapi STATIC
            mem_surf.c pb_math.c sapi_data.c sapi_files.c
            sapi_globals.c sapi_simple.c util_surf.c xdr_surf.c)

target_compile_definitions(mbsapi PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


target_include_directories(mbsapi
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


                           

# Shared library
add_library(mbsapiShared SHARED
            mem_surf.c pb_math.c sapi_data.c sapi_files.c
            sapi_globals.c sapi_simple.c util_surf.c xdr_surf.c)

target_compile_definitions(mbsapiShared PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


target_include_directories(mbsapiShared
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


                           

