#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes 
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#      
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/mbnavadjust")

# libmbnavadjust: GUI-free project-file I/O and core logic shared by
# mbnavadjust (GUI), mbnavadjustmerge (CLI), and mbnavadjustfine. Built
# unconditionally (no Motif/X11 dependency) so mbnavadjustmerge does not
# require a GUI build.
add_library(
  mbnavadjust_core
  mbnavadjust_io.c
  mbnavadjust_autopick.c
  mbnavadjust_invertnav.c)

set(HEADERS
    mbnavadjust_io.h
    mbnavadjust_core.h)

set_target_properties(mbnavadjust_core PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbnavadjust_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbnavadjust_core PUBLIC mbaux)

install(TARGETS mbnavadjust_core DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable(mbnavadjustmerge mbnavadjustmerge.c)
target_link_libraries(mbnavadjustmerge PRIVATE mbnavadjust_core mbaux)

install(TARGETS mbnavadjustmerge DESTINATION ${CMAKE_INSTALL_BINDIR})

if(buildGUIs)
  find_package(X11 REQUIRED)
  find_package(Motif REQUIRED)

  add_executable(mbnavadjust
                  mbnavadjust.c
                  mbnavadjust_callbacks.c
                  mbnavadjust_creation.c
                  mbnavadjust_prog.c
                  mbnavadjust_util.c)

  target_link_libraries(mbnavadjust PRIVATE mbnavadjust_core mbview mbaux mbxgr
		 	${MOTIF_LIBRARIES}
			${X11_LIBRARIES}
			${X11_Xt_LIB})

  install(TARGETS mbnavadjust DESTINATION ${CMAKE_INSTALL_BINDIR})

  if(buildPCL)
    find_package(PCL REQUIRED)
    find_package(LibPROJ REQUIRED)

    if(LibPROJ_VERSION VERSION_LESS 5)
      message("mbnavadjustfine requires modern PROJ4")
    else()
      add_executable(mbnavadjustfine mbnavadjust_fine.cc mbnavadjust_icp.cc)
      target_link_libraries(mbnavadjustfine PRIVATE mbnavadjust_core mbaux LibPROJ::LibPROJ
                                                    pthread)

      target_link_directories(mbnavadjustfine PRIVATE ${PCL_LIBRARY_DIRS})
      target_include_directories(mbnavadjustfine PRIVATE ${PCL_INCLUDE_DIRS})
      target_compile_definitions(mbnavadjustfine PRIVATE ${PCL_DEFINITIONS})
      target_link_libraries(mbnavadjustfine PRIVATE ${PCL_LIBRARIES})

      install(TARGETS mbnavadjustfine DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  endif()
endif()
