cmake_minimum_required(VERSION 3.18)
project(MB-System 
        VERSION 5.7.9
        DESCRIPTION "Open Source software for the processing and display of seafloor mapping data"
        HOMEPAGE_URL https://www.mbari.org/technology/mb-system/ )
set(PROJECT_DATE "9 June 2023")

add_compile_definitions(CMAKE_BUILD_SYSTEM=1)

option(buildGSF "build GSF library" ON)
option(buildGUIs "build graphical tools" ON)

option(buildOpenCV "build OpenCV tools" OFF)
option(buildPCL "build point-cloud-library" OFF)
option(buildTRN "build MBTRN tools" OFF)
option(buildTNav "build MBTNav tools" OFF)
option(buildQt "build Qt tools" OFF)
option(BUILD_SHARED_LIBS "Build and link with shared libraries" ON)

if(help)
  message("specify OTPS install location with -DotpsDir=directory")
  message("set install directory with -DCMAKE_INSTALL_PREFIX=directory")
  message("enable debugging with -DCMAKE_BUILD_TYPE=Debug")
  message("build Qt-based GUIs with -DbuildQt=1")
  message("build TRN software with -DbuildTRN=1")
  message("build shared/static libraries with -DBUILD_SHARED_LIBS=ON/OFF")
  message("disable building tests with -DBUILD_TESTING=OFF")
  message(FATAL_ERROR "")
endif()

if(APPLE)
  set(otpsDir /usr/local/opt/otps)
else()
  set(otpsDir /local/opt/otps)
endif()
set(levitusDir ${CMAKE_INSTALL_PREFIX}/share/mbsystem)

message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message(
  "CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")

include(GNUInstallDirs)
include(CTest)

# Utilities and libraries
if(buildGSF)
  add_subdirectory(src/gsf)
endif()

add_subdirectory(src/mbaux)
add_subdirectory(src/mbio)
add_subdirectory(src/bsio)
add_subdirectory(src/surf)

if(buildTRN)
  add_subdirectory(src/mbtrnframe)
  add_subdirectory(src/mbtrn)
endif()

# Executables
if(buildGUIs)
  add_subdirectory(src/mbview)
  add_subdirectory(src/mbedit)
  add_subdirectory(src/mbgrdviz)
  add_subdirectory(src/mbeditviz)
  add_subdirectory(src/mbnavadjust)
  add_subdirectory(src/mbnavedit)
  add_subdirectory(src/mbvelocitytool)
endif()

add_subdirectory(src/utilities)
add_subdirectory(src/deprecated)
add_subdirectory(src/macros)

if(buildQt)
  add_subdirectory(src/qt-guilib)
  add_subdirectory(src/qt-mbgrdviz)
  add_subdirectory(src/qt-mbgrdviz-3)
endif()

if(buildOpenCV)
  add_subdirectory(src/photo)
endif()

if(BUILD_TESTING)
  add_subdirectory(third_party)
  add_subdirectory(test/mbio)
  add_subdirectory(test/utilities)
  add_subdirectory(test/deprecated)
endif()

#add_subdirectory(src/man)
#add_subdirectory(src/html)
#add_subdirectory(src/share)
