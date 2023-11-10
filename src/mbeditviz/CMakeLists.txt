message("In src/mbeditviz")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_library(mbeditvizlib mbeditviz_creation.c mbeditviz_prog.c
                         mbeditviz_callbacks.c)
target_include_directories(mbeditvizlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbeditvizlib PRIVATE mbaux mbxgr mbview
  			    ${MOTIF_LIBRARIES}
			    ${X11_LIBRARIES}
			    ${X11_Xt_LIB})

add_executable(mbeditviz mbeditviz_main.c mbeditviz_creation.c mbeditviz_prog.c
                         mbeditviz_callbacks.c)
target_include_directories(mbeditviz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbeditviz PRIVATE mbaux mbxgr mbview
  			    ${MOTIF_LIBRARIES}
			    ${X11_LIBRARIES}
			    ${X11_Xt_LIB})

install(TARGETS mbeditviz DESTINATION ${CMAKE_INSTALL_BINDIR})
