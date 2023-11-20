
#------------------------------------------------------------------------------
#
# This group of libraries derives for an MBARI software repository that supports
# Terrain Relative Navigation (TRN) software. Aside from this CMakeLists.txt file
# and the parallel Autotools build system files Makefile.am and Makefile.in,
# all three  written to enable building these libraries as part of MB-System, the
# directories and source files below are synchronized with the MBARI libtrnav
# repository.
# The libraries are:
#   udpms
#   trnxplug
#   trnlcm
#   udpms
# The application(s) are:
#   trnxpp
message("In mbtrnav/opt/rov")
#------------------------------------------------------------------------------

find_package(NetCDF REQUIRED)
find_package(lcm REQUIRED)
include(${LCM_USE_FILE})

find_package(Java)
if(JAVA_FOUND)
    include(UseJava)
    set(java_args JAVA_SOURCES java_sources)
endif()
add_compile_definitions(
HAVE_CONFIG_H
_GNU_SOURCE
)
#------------------------------------------------------------------------------
# generate LCM source

lcm_wrap_types(
    C_SOURCES c_sources
    C_HEADERS c_install_headers
    CPP_HEADERS cpp_install_headers
    ${java_args}
    lcm_types/signal_t.lcm
    lcm_types/string_t.lcm
    lcm_types/nav_solution_t.lcm
    lcm_types/message_t.lcm
    lcm_types/analog_t.lcm
    lcm_types/digital_t.lcm
    lcm_types/dvl_stat_t.lcm
    lcm_types/pcomms_t.lcm
    lcm_types/parosci_stat_t.lcm
    lcm_types/ctd_t.lcm
    lcm_types/idt_t.lcm
    lcm_types/lcm_header_t.lcm
    lcm_types/rdi_pd4_t.lcm
    lcm_types/kearfott_t.lcm
    lcm_types/octans_t.lcm
    lcm_types/double_t.lcm
    lcm_types/mb1_beam_t.lcm
    lcm_types/trnu_estimate_t.lcm
    lcm_types/trn_stat_t.lcm
    lcm_types/trn_meas_t.lcm
    lcm_types/trn_pose_t.lcm
    lcm_types/trnupub_t.lcm
    lcm_types/trn_mb1_t.lcm
)

# C Static libs
#lcm_add_library(trnlcm C STATIC
#${c_sources}
#${c_install_headers}
#)
#target_include_directories(trnlcm INTERFACE
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# C shared libs
lcm_add_library(trnlcm C ${c_sources} ${c_install_headers})
target_include_directories(trnlcm INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# C++ libs
lcm_add_library(trnlcm-cpp CPP ${cpp_install_headers})
target_include_directories(trnlcm-cpp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

lcm_install_headers(DESTINATION include
${c_install_headers}
${cpp_install_headers}
)

# TRN LCM types jar
if(JAVA_FOUND)
  add_jar(trnlcm-jar
    OUTPUT_NAME lcm_rovtrn
    INCLUDE_JARS lcm-java
    SOURCES ${java_sources}
  )
endif()
#
#------------------------------------------------------------------------------
#
# build udpms

# specify library
add_library(udpms
       udpm_sub.c)
#
#------------------------------------------------------------------------------
#
# build trnxplug

# specify library
    add_library(trnxplug
    plug-common.cpp
    plug-idtlass.cpp
    plug-oisledx.cpp
    plug-dvl.cpp
    plug-idt.cpp
    plug-oisled.cpp
    plug-oisled2.cpp
    plug-lassmb.cpp
    plug-xmb1.cpp
)

target_include_directories(trnxplug PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
    ${NetCDF_INCLUDE_DIRS}
)

target_link_libraries(trnxplug PRIVATE
    newmat
    geolib
    trnw
    qnx
    mb1
    tnav
    udpms
    trncli
    NetCDF::NetCDF
    lcm
)
#
#------------------------------------------------------------------------------
#
# build trnxpp (TRN LCM pre-processor)

# specify build target
add_executable(trnxpp
    trnxpp_app.cpp
)
target_include_directories(trnxpp PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
    ${NetCDF_INCLUDE_DIRS}
)

target_link_libraries(trnxpp PRIVATE lcm stdc++ mb1 trncli tnav newmat qnx m geolib udpms trnxplug NetCDF::NetCDF)
#
#------------------------------------------------------------------------------
#
# build trnlcm-test (TRN LCM test)

# specify build target
#add_executable(trnlcm-test
#    trnlcm_test.cpp
#)
#
#target_include_directories(trnlcm-test PRIVATE
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
#    ${NetCDF_INCLUDE_DIRS}
#)

# may need rt (librt) in Linux
#target_link_libraries(trnlcm-test PRIVATE lcm stdc++ pthread)
#
#------------------------------------------------------------------------------
#
# build fv-test (flag variable test)

# specify build target
#add_executable(fv-test
#    flag_var_test.cpp
#)
#
#target_include_directories(fv-test PRIVATE
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw
#    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov
#    ${NetCDF_INCLUDE_DIRS}
#)

# may need rt (librt) in Linux
#target_link_libraries(fv-test PRIVATE stdc++)
#------------------------------------------------------------------------------
# install it all
#
install(TARGETS
    trnlcm
    trnlcm-cpp
    udpms
    trnxplug
    trnxpp
#    trnlcm-test
#    fv-test
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    INCLUDES DESTINATION include
)

if(JAVA_FOUND)
    install_jar(trnlcm-jar share/java)
endif()
