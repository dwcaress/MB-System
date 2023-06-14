add_library(r7kr r7kr/r7kc.c r7kr/r7k-reader.c)
target_link_libraries(r7kr mframe)

install(TARGETS r7kr DESTINATION ${CMAKE_INSTALL_LIBDIR})
