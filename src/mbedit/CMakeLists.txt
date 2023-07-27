find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(mbedit mbedit.c mbedit_creation.c mbedit_bxutils.c mbedit_prog.c
                      mbedit_callbacks.c)

target_link_libraries(mbedit PRIVATE mbaux mbxgr
		            ${MOTIF_LIBRARIES}
			    ${X11_LIBRARIES}
			    ${X11_Xt_LIB})



install(TARGETS mbedit DESTINATION ${CMAKE_INSTALL_BINDIR})
