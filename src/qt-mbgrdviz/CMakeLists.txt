#
# Gets variables from root CMakeLists
#

message("in qt-mbgrdviz")

add_executable(qt-mbgrdviz
               BackEnd.cpp main.cpp moc_BackEnd.cpp qrc_qml.cpp)


target_include_directories(qt-mbgrdviz
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/qt-guilib)

target_link_libraries(qt-mbgrdviz
                      PRIVATE
                      MBGui
		      ${GMT_LIBRARIES}
		      ${NETCDF_LIBRARIES}
		      ${MOTIF_LIBRARIES}
		      ${X11_LIBRARIES}
		      ${X11_XT_LIBRARIES}		      
		      ${OPENGL_LIBRARIES}
		      ${GDAL_LIBRARIES}
		      ${PROJ_LIBRARIES}
                      ${QT5_LIBRARIES}
                      ${VTK_LIBRARIES}
                      m
                      pthread)
                           
                           

