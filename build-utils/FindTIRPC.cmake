find_package(PkgConfig)
pkg_check_modules(TIRPC libtirpc)

set(TIRPC_VERSION ${TIRPC_VERSION})

find_path(
  TIRPC_INCLUDE_DIR
  NAMES rpc/rpc.h
  PATH_SUFFIXES tirpc
  HINTS ${TIRPC_INCLUDE_DIRS} ${TIRPC_INCLUDEDIR})

find_library(
  TIRPC_LIBRARY
  NAMES tirpc
  HINTS ${TIRPC_LIBRARY_DIRS} ${TIRPC_LIBDIR})

set(TIRPC_VERSION ${PC_RapidJSON_VERSION})

mark_as_advanced(TIRPC_FOUND TIRPC_INCLUDE_DIR TIRPC_VERSION TIRPC_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  TIRPC
  REQUIRED_VARS TIRPC_INCLUDE_DIR TIRPC_LIBRARY
  VERSION_VAR TIRPC_VERSION)

if(TIRPC_FOUND)
  set(TIRPC_LIBRARIES ${TIRPC_LIBRARY})
  set(TIRPC_INCLUDE_DIRS ${TIRPC_INCLUDE_DIR})
  if(NOT TARGET TIRPC::TIRPC)
    add_library(TIRPC::TIRPC INTERFACE IMPORTED)
    set_target_properties(
      TIRPC::TIRPC
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TIRPC_INCLUDE_DIR}"
                 INTERFACE_LINK_LIBRARIES "${TIRPC_LIBRARY}")
  endif()
  message("-- TIRPC found!")
else()
  message("-- TIRPC NOT found...")
endif()
