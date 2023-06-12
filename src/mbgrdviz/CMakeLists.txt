find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(mbgrdviz mbgrdviz_callbacks.c mbgrdviz_creation.c
                        mbgrdviz_main.c)

target_link_libraries(mbgrdviz PRIVATE mbaux mbview Motif::Motif X11::Xt)

install(TARGETS mbgrdviz DESTINATION ${CMAKE_INSTALL_BINDIR})
