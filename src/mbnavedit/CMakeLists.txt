message("In src/mbnavedit")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(mbnavedit mbnavedit.c mbnavedit_callbacks.c mbnavedit_creation.c
                         mbnavedit_prog.c mbnavedit_util.c)

target_link_libraries(mbnavedit PRIVATE mbaux mbxgr
                            ${MOTIF_LIBRARIES}
			    ${X11_LIBRARIES}
			    ${X11_Xt_LIB})



install(TARGETS mbnavedit DESTINATION ${CMAKE_INSTALL_BINDIR})
