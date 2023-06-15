set(manpages
    mbio.3
    pj_init.3)

install(FILES ${manpages} DESTINATION ${CMAKE_INSTALL_MANPAGES}/man3)

message(STATUS "Installed man pages to ${CMAKE_INSTALL_MANPAGES}/man3")
