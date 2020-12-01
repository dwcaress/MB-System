# FindRapidJSON.cmake
#
# Finds the library with LIBNAME
#
# The following variables should be defined before running this
# script:
#
#  LIBNAME - library name (without prefix or suffix)
#  HEADERNAME - header file name
#  HEADERSUBDIR - subdirectory containing header
#
# This will define the following variables
#
#    ${LIBNAME}_FOUND
#    ${LIBNAME}_INCLUDE_DIRS
#
# and the following imported targets
#
#     ${LIBNAME}::${LIBNAME}
#
# Author: Pablo Arias - pabloariasal@gmail.com

if (CMAKE_SCRIPT_MODE_FILE)
  set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
endif ()

message("findLibrary.cmake: CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")

find_package(PkgConfig)
message("findLibrary.cmake: pkg_check_modules prefix: PC_${LIBNAME}")
pkg_check_modules(PC_${LIBNAME} QUIET ${LIBNAME})

# TEST TEST TEST
### set(PC_${LIBNAME}_INCLUDE_DIRS /usr/include;/usr/local/include)

message("findLibrary.cmake: call find_path() for ${HEADERNAME}, look in ${PC_${LIBNAME}_INCLUDE_DIRS}")
unset(${LIBNAME}_INCLUDE_DIR CACHE)
find_path(${LIBNAME}_INCLUDE_DIR
    NAMES ${HEADERNAME}
    PATHS ${PC_${LIBNAME}_INCLUDE_DIRS}
    PATH_SUFFIXES ${HEADERSUBDIR}
)

if (NOT ${LIBNAME}_INCLUDE_DIR)
  message("findLibrary.cmake: Could not find ${HEADERNAME}, dammit")
  unset(${LIBNAME}_FOUND)
  return()
else()
  message("findLibrary.cmake: found ${${LIBNAME}_INCLUDE_DIR}/${HEADERNAME}")
endif()

set(${LIBNAME}_VERSION ${PC_${LIBNAME}_VERSION})

## mark_as_advanced(${LIBNAME}_FOUND ${LIBNAME}_INCLUDE_DIR ${LIBNAME}_VERSION)

# include functions to be used by find_package functions, including
# setting <packageName>_FOUND variable
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${LIBNAME}
    REQUIRED_VARS ${LIBNAME}_INCLUDE_DIR
    VERSION_VAR ${LIBNAME}_VERSION)

if(${LIBNAME}_FOUND)
    message("findLibrary.cmake: Found ${LIBNAME}!!!")
    set(${LIBNAME}_INCLUDE_DIRS ${${LIBNAME}_INCLUDE_DIR})
else ()
    message("findLibrary.cmake: Aw, ${LIBNAME} not found")
endif()

if(${LIBNAME}_FOUND AND NOT TARGET ${LIBNAME}::${LIBNAME})
    if (NOT CMAKE_SCRIPT_MODE_FILE)
      add_library(${LIBNAME}::${LIBNAME} INTERFACE IMPORTED)
      set_target_properties(${LIBNAME}::${LIBNAME} PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${${LIBNAME}_INCLUDE_DIR}")
    endif()
endif()
