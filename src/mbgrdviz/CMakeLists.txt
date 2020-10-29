#
# Gets GMT_INCLUDE_DIR from root CMakeLists
#

message("in mbgrdviz")

add_executable(mbgrdviz
               mbgrdviz_callbacks.c  mbgrdviz_creation.c  mbgrdviz_main.c)

target_include_directories(mbgrdviz
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/mbview)


target_link_libraries(mbgrdviz
                      PRIVATE libmbio.a
                      PRIVATE libmbaux.a
                      PRIVATE libmbview.a
                      PRIVATE ${CMAKE_BINARY_DIR}/src/bsio/libbsio.a
                      PRIVATE ${CMAKE_BINARY_DIR}/src/surf/libmbsapi.a
                      PRIVATE ${CMAKE_BINARY_DIR}/src/gsf/libgsf.a
                      PUBLIC gmt
                      PUBLIC Xm
                      PUBLIC Xt
                      PUBLIC X11
                      PUBLIC GL
                      PUBLIC GLU
                      PUBLIC m
                      PUBLIC netcdf
                      PUBLIC gdal
                      PUBLIC proj
                      PUBLIC pthread)
                           
                           

