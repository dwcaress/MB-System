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
                           ${CMAKE_SOURCE_DIR}/src/mbview
                           ${CMAKE_SOURCE_DIR}/src/bsio
                           ${CMAKE_SOURCE_DIR}/src/surf
                           ${CMAKE_SOURCE_DIR}/src/gsf)                           
# Why do libbsio.a, libmbsapi.a and libgsf.a require full path else
# they aren't found by linker?
target_link_libraries(mbgrdviz
                      PRIVATE
                      mbio
                      mbaux
                      mbview
                      bsio
                      mbsapi
                      gsf
                      gmt
                      Xm
                      Xt
                      X11
                      GL
                      GLU
                      m
                      netcdf
                      gdal
                      proj
                      pthread)
                           
                           

