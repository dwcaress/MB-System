#
#

message("in surf")

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(mbsapi
            mem_surf.c pb_math.c sapi_data.c sapi_files.c
            sapi_globals.c sapi_simple.c util_surf.c xdr_surf.c)

target_compile_definitions(mbsapi PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


target_include_directories(mbsapi
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio)


                           
install(TARGETS mbsapi 
	DESTINATION ${CMAKE_INSTALL_LIBDIR})
