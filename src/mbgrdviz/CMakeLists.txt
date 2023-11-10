message("In src/mbgrdviz")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

add_executable(mbgrdviz mbgrdviz_callbacks.c mbgrdviz_creation.c
                        mbgrdviz_main.c)

		      target_link_libraries(mbgrdviz PRIVATE mbaux mbview
			${MOTIF_LIBRARIES}
			${X11_LIBRARIES}
			${X11_Xt_LIB}
		      )

install(TARGETS mbgrdviz DESTINATION ${CMAKE_INSTALL_BINDIR})
