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
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/mbview
                           ${CMAKE_SOURCE_DIR}/src/bsio
                           ${CMAKE_SOURCE_DIR}/src/surf
                           ${CMAKE_SOURCE_DIR}/src/gsf)                        

link_directories($GMT_LIBRARY)
   
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
                           
                           

