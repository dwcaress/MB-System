# Accepts the following _INCLUDE and _LIBS "-D" options on the command line:
# GMT_
# PROJ_
# OPENGL_
# QT_
# VTK_

cmake_minimum_required(VERSION 3.18)
project(MB-Sytstem)

### TEST FUNCTION
function(findIncludeDir file var)
  message("this is findIncludeDir()")
endfunction()

function(findLibInStdLoc file var)
  message("this is findLibInStdLoc()")
endfunction()


set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})

message("MB_HOME: ${MB_HOME}")

# Add build-utils to module path
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/build-utils/)
message("CMAKE_MODULE_PATH now: ${CMAKE_MODULE_PATH}")

# Use functions in cmakeUils.cmake
###include(cmakeUtils.cmake)


message("CMAKE_SYSTEM: ${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")
message("CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message("CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
message("CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")

# Set search paths for includes, libs based on host OS
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message("Building on LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message("Building on Darwin")
else()
  message(FATAL_ERROR "Building on unsupported OS")
endif()

if (NOT GMT_INCLUDE)
  # Look for includes in usual places
  find_file(GMT_HEADER "gmt"
            PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
            REQUIRED)
  message("GMT_HEADER: ${GMT_HEADER}")
  get_filename_component(GMT_INCLUDE ${GMT_HEADER} DIRECTORY)
  message("GMT_INCLUDE: ${GMT_INCLUDE}")  

endif()

if (NOT GMT_LIBS)
  # Look for library in usual places
  find_library(GMT_LIBS NAMES libgmt.so libgmt.dylib libgmt.a)
  message("GMT_LIBS: ${GMT_LIBS}")
  if (NOT GMT_LIBS)
    message(FATAL_ERROR "GMT_LIBS not found")
  endif()
endif()

message("GMT_LIBS: ${GMT_LIBS}")


add_subdirectory(src/mbaux)
