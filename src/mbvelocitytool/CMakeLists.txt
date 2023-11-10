message("In src/mbvelocitytool")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(
  mbvelocitytool mbvelocity.c mbvelocity_bxutils.c mbvelocity_callbacks.c
                 mbvelocity_creation.c mbvelocity_prog.c)

target_link_libraries(mbvelocitytool PRIVATE mbaux mbxgr 
                      ${MOTIF_LIBRARIES}
		      ${X11_LIBRARIES}
		      ${X11_Xt_LIB})

install(TARGETS mbvelocitytool DESTINATION ${CMAKE_INSTALL_BINDIR})
