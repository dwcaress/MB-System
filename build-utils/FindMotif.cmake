
if (APPLE)
	if(MACOS_USE_MACPORTS)
		find_path(MOTIF_INCLUDE_DIR Xm/Xm.h /opt/local/include)
		find_library(MOTIF_LIBRARY Xm /opt/local/lib)
	else() # assume Homebrew
		find_path(MOTIF_INCLUDE_DIR Xm/Xm.h /opt/homebrew/include)
		find_library(MOTIF_LIBRARY Xm /opt/homebrew/lib)
	endif()
else()
	find_path(MOTIF_INCLUDE_DIR Xm/Xm.h)
	find_library(MOTIF_LIBRARY Xm)
endif()

mark_as_advanced(MOTIF_FOUND MOTIF_INCLUDE_DIR MOTIF_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Motif REQUIRED_VARS MOTIF_INCLUDE_DIR
                                                      MOTIF_LIBRARY)

if (MOTIF_FOUND)
  set(MOTIF_LIBRARIES ${MOTIF_LIBRARY})
  set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
  if(NOT TARGET Motif::Motif)
    add_library(Motif::Motif INTERFACE IMPORTED)
    set_target_properties(
      Motif::Motif
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MOTIF_INCLUDE_DIR}"
                 INTERFACE_LINK_LIBRARIES "${MOTIF_LIBRARY}")
  endif()
  message("-- Motif found!")
else()
  message("-- Motif NOT found...")
endif()
