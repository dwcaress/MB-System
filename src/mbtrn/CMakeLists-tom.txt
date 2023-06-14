#
# Variables are passed from top-level CMakeLists.txt

message("in mbtrn/")

# Build r7kr and mframe libraries from here, since we should not
# 'interfere' with third-party CMakeLists.txt already in those
# subdirectories

# Build r7kr library
set(R7KR_DIR r7kr/)

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(r7kr ${R7KR_DIR}/r7kc.c ${R7KR_DIR}/r7k-reader.c)


target_compile_definitions(r7kr PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)                  

target_include_directories(r7kr
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/r7kr
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/utils
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/mframe/src)                           


install(TARGETS r7kr r7kr
	DESTINATION ${CMAKE_INSTALL_LIBDIR})


# Build mframe library
set(MFRAME_DIR mframe/src)

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(mframe
            ${MFRAME_DIR}/mbbuf.c
            ${MFRAME_DIR}/mcbuf.c
            ${MFRAME_DIR}/mconfig.c
            ${MFRAME_DIR}/merror.c
            ${MFRAME_DIR}/mfile.c
            ${MFRAME_DIR}/mframe.c
            ${MFRAME_DIR}/mkvconf.c
            ${MFRAME_DIR}/mkvconf-test.c
            ${MFRAME_DIR}/mlist.c
            ${MFRAME_DIR}/mlog.c
            ${MFRAME_DIR}/mmdebug.c
            ${MFRAME_DIR}/mmem.c
            ${MFRAME_DIR}/mqueue.c
            ${MFRAME_DIR}/mserial.c
            ${MFRAME_DIR}/msocket.c
            ${MFRAME_DIR}/mstats.c
            ${MFRAME_DIR}/mswap.c
            ${MFRAME_DIR}/mthread.c
            ${MFRAME_DIR}/mtime.c
            ${MFRAME_DIR}/mutils.c)

target_compile_definitions(mframe PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,GNU>:USE_DEFAULT_FILE_FUNCTIONS>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:USE_DEFAULT_FILE_FUNCTIONS>)
         
target_include_directories(mframe
                           PUBLIC
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/utils
                           ${CMAKE_SOURCE_DIR}/src/mbtrn/mframe/src)

install(TARGETS mframe mframe
	DESTINATION ${CMAKE_INSTALL_LIBDIR})
