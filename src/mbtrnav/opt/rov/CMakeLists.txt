
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
#
message("In src/mbtrnav/opt/rov")
#
#------------------------------------------------------------------------------
#

#################################
# find dependencies
find_package(NetCDF REQUIRED)

find_package(LibPROJ REQUIRED)

find_package(lcm REQUIRED)
if(lcm_FOUND)
include(${LCM_USE_FILE})
message(" LCM found (please ignore any warnings about CMake version deprecation...)")
else()
message(" LCM NOT found")
endif()


find_package(Java)
if(JAVA_FOUND)
    include(UseJava)
    set(java_args JAVA_SOURCES java_sources)
    message("-- Java found!")
endif()

add_compile_definitions(
HAVE_CONFIG_H
_GNU_SOURCE
)

# define source directories
set(GCTP_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/gctp/source)
set(NEWMAT_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/newmat)
set(QNX_SRC_DIR    ${CMAKE_SOURCE_DIR}/src/mbtrnav/qnx-utils)
set(TNAV_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/terrain-nav)
set(TRNW_SRC_DIR   ${CMAKE_SOURCE_DIR}/src/mbtrnav/trnw)
set(UTILS_SRC_DIR  ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils)
set(DORADO_SRC_DIR ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/dorado)
set(ROV_SRC_DIR    ${CMAKE_SOURCE_DIR}/src/mbtrnav/opt/rov)
set(ROV_BIN_DIR    ${CMAKE_BINARY_DIR}/src/mbtrnav/opt/rov

# add libproj [as needed]
if(LibPROJ_LIBRARY)
list(APPEND TRN_COMMON_LIBS "${LibPROJ_LIBRARY}")
list(PREPEND LIBTRNAV_INCLUDES "${LibPROJ_INCLUDE_DIR}")
else()
set(LibPROJ_LIBRARY "")
list(PREPEND LIBTRNAV_INCLUDES "${GCTP_SRC_DIR}")
endif()

# define include paths
# mframe, libtrnav, netcdf
# use proj (default) or gctp, not both (set above)
list(PREPEND LIBTRNAV_INCLUDES
#"${LibPROJ_INCLUDE_DIR}"
#"${GCTP_SRC_DIR}"
"${Libmframe_INCLUDE_DIRS}"
"${NEWMAT_SRC_DIR}"
"${QNX_SRC_DIR}"
"${TNAV_SRC_DIR}"
"${TRNW_SRC_DIR}"
"${UTILS_SRC_DIR}"
"${ROV_SRC_DIR}"
"${ROV_BIN_DIR}"
"${NetCDF_INCLUDE_DIRS}"
)

#
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

add_compile_options(-std=c++11)
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

# specify include paths and libraries
target_link_libraries(trnxplug PRIVATE newmat geolib geocon trnw qnx mb1 tnav udpms trncli NetCDF::NetCDF lcm)

target_include_directories(trnxplug PRIVATE
    ${LIBTRNAV_INCLUDES}
)

#
#------------------------------------------------------------------------------
#
# build trnxpp (TRN LCM pre-processor)

# specify build target
add_executable(trnxpp
    trnxpp_app.cpp
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils/TrnClient.cpp
    ${CMAKE_SOURCE_DIR}/src/mbtrnav/utils/TerrainNavClient.cpp
)

# specify include paths and libraries
target_link_libraries(trnxpp PRIVATE lcm stdc++ mb1 trncli tnav newmat qnx m geolib udpms trnxplug NetCDF::NetCDF)

target_include_directories(trnxpp PRIVATE
    ${LIBTRNAV_INCLUDES}
)
#
#------------------------------------------------------------------------------
# Build optional targets wnen buildTRNOptional=ON
# Must update TRN_OPTIONAL list at end of block if
# targets may be moved in/out of buildTRNOptional block
if(buildTRNOptional)
#
#------------------------------------------------------------------------------
#
# build trnlcm-test (TRN LCM test)

# specify build target
add_executable(trnlcm-test
    trnlcm_test.cpp
)

# specify include paths and libraries
# may need rt (librt) in Linux
target_link_libraries(trnlcm-test PRIVATE lcm pthread)

target_include_directories(trnlcm-test PRIVATE
    ${LIBTRNAV_INCLUDES}
    /usr/local/include
)

add_compile_options(-std=c++11)
#
#------------------------------------------------------------------------------
#
# build fv-test (flag variable test)

# specify build target
add_executable(fv-test
    flag_var_test.cpp
)

# specify include paths and libraries
# may need rt (librt) in Linux
target_link_libraries(fv-test PRIVATE stdc++)

target_include_directories(fv-test PRIVATE
    ${LIBTRNAV_INCLUDES}
)

# update this list when targets are moved
# in/out of buildTRNOptional block
list(APPEND TRN_OPTIONAL trnlcm-test fv-test)

endif() # buildTRNOptional

if(buildVerbose)
message(" TRN_OPTIONAL                ${TRN_OPTIONAL}")
endif()

#
#------------------------------------------------------------------------------
# install it all
install(TARGETS trnlcm trnlcm-cpp udpms trnxplug

DESTINATION
${CMAKE_INSTALL_LIBDIR}
COMPONENT
Devel
)

install(TARGETS
trnxpp
${TRN_OPTIONAL}

DESTINATION
${CMAKE_INSTALL_BINDIR}
COMPONENT
Devel
)

if(JAVA_FOUND)
    install_jar(trnlcm-jar share/java)
endif()
#------------------------------------------------------------------------------
