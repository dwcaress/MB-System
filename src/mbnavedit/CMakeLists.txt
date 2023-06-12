find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(mbnavedit mbnavedit.c mbnavedit_callbacks.c mbnavedit_creation.c
                         mbnavedit_prog.c mbnavedit_util.c)

target_link_libraries(mbnavedit PRIVATE mbaux mbxgr Motif::Motif X11::Xt)

install(TARGETS mbnavedit DESTINATION ${CMAKE_INSTALL_BINDIR})
