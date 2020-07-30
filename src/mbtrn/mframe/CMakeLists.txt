cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Libmframe VERSION 1.0)

set(MFRAME_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(UTILS_SRC_DIR ${PROJECT_SOURCE_DIR}/utils)

list(APPEND EXTRA_LIBS mframe m pthread)
list(APPEND EXTRA_INCLUDES 
"${UTILS_SRC_DIR}"
"${MFRAME_SRC_DIR}"
)

find_file( MCONFIG_C
NAME mconfig.c
PATHS
"${UTILS_SRC_DIR}"
"${MFRAME_SRC_DIR}"
)

add_compile_options( -Wall -g -O2 )
add_compile_definitions(
WITH_MSOCKET_TEST
WITH_MEDEBUG_TEST
WITH_MMDEBUG_TEST
WITH_MSTATS_TEST
WITH_MTIME_TEST
WITH_MKVCONF_TEST
)

add_library(mframe
${MFRAME_SRC_DIR}/mbbuf.c
${MFRAME_SRC_DIR}/mcbuf.c
"${MCONFIG_C}"
${MFRAME_SRC_DIR}/mdebug.c
${MFRAME_SRC_DIR}/merror.c
${MFRAME_SRC_DIR}/mfile.c
${MFRAME_SRC_DIR}/mframe.c
${MFRAME_SRC_DIR}/mkvconf.c
${MFRAME_SRC_DIR}/mlist.c
${MFRAME_SRC_DIR}/mlog.c
${MFRAME_SRC_DIR}/mmdebug.c
${MFRAME_SRC_DIR}/mmem.c
${MFRAME_SRC_DIR}/mqueue.c
${MFRAME_SRC_DIR}/mserial.c
${MFRAME_SRC_DIR}/msocket.c
${MFRAME_SRC_DIR}/mstats.c
${MFRAME_SRC_DIR}/mswap.c
${MFRAME_SRC_DIR}/mthread.c
${MFRAME_SRC_DIR}/mtime.c
${MFRAME_SRC_DIR}/mutils.c
)
set_target_properties( mframe
PROPERTIES
INCLUDE_DIRECTORIES
"${EXTRA_INCLUDES}"
)



configure_file(LibmframeConfig.h.in LibmframeConfig.h)

# add msocket-test
add_executable(msock-test ${MFRAME_SRC_DIR}/msocket-test.c)

target_link_libraries(msock-test PUBLIC ${EXTRA_LIBS})

target_include_directories(msock-test PUBLIC
"${PROJECT_BINARY_DIR}"
${EXTRA_INCLUDES}
)

# add medebug-test
add_executable(medebug-test ${MFRAME_SRC_DIR}/medebug-test.c)

target_link_libraries(medebug-test PUBLIC ${EXTRA_LIBS})

target_include_directories(medebug-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${MFRAME_SRC_DIR}"
${EXTRA_INCLUDES}
)

# add mkvconf-test
add_executable(mkvconf-test ${MFRAME_SRC_DIR}/mkvconf-test.c)

target_link_libraries(mkvconf-test PUBLIC ${EXTRA_LIBS})

target_include_directories(mkvconf-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${MFRAME_SRC_DIR}"
${EXTRA_INCLUDES}
)

# add mmdebug-test
add_executable(mmdebug-test ${MFRAME_SRC_DIR}/mmdebug-test.c)

target_link_libraries(mmdebug-test PUBLIC ${EXTRA_LIBS})

target_include_directories(mmdebug-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${MFRAME_SRC_DIR}"
${EXTRA_INCLUDES}
)

# add mstats-test
add_executable(mstats-test ${MFRAME_SRC_DIR}/mstats-test.c
${MFRAME_SRC_DIR}/mlog.c)

target_link_libraries(mstats-test PUBLIC ${EXTRA_LIBS})

target_include_directories(mstats-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${MFRAME_SRC_DIR}"
${EXTRA_INCLUDES}
)

# add mtime-test
add_executable(mtime-test ${MFRAME_SRC_DIR}/mtime-test.c)

target_link_libraries(mtime-test PUBLIC ${EXTRA_LIBS})

target_include_directories(mtime-test PUBLIC
"${PROJECT_BINARY_DIR}"
"${MFRAME_SRC_DIR}"
${EXTRA_INCLUDES}
)
