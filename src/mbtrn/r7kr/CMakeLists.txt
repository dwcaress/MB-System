#
# Variables are passed from top-level CMakeLists.txt

message("in mbtrn/r7kr")

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(r7kr r7kc.c r7k-reader.c)


target_compile_definitions(r7kr PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(r7kr
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr)


install(TARGETS r7kr r7kr
	DESTINATION ${CMAKE_INSTALL_LIBDIR})
