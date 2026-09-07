# Note: CMake support is community-based. The maintainers do not use CMake
# internally.

cmake_minimum_required(VERSION 3.16)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif (POLICY CMP0069)

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif (POLICY CMP0077)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.18.0)

# GoogleTest 1.15+ requires C++17. Scope this to the googletest/googlemock
# subdirectories only (set() here does not propagate back up to the rest of
# the MB-System build, which otherwise targets C++11).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CYGWIN AND NOT MSYS AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL QNX)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
# MB-System vendors GoogleTest/GoogleMock purely as a private test dependency;
# it must not be installed system-wide (doing so previously left a stale
# GTest install under CMAKE_INSTALL_PREFIX that shadowed this vendored copy
# during compilation on systems where that prefix's include dir is on the
# compiler's implicit search path).
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
option(GTEST_HAS_ABSL "Use Abseil and RE2. Requires Abseil and RE2 to be separately added to the build." OFF)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
else()
  add_subdirectory( googletest )
endif()
