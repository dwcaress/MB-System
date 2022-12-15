#
# Gets variables from root CMakeLists
#

message("in qt-mbgrdviz-3")


# Run moc when needed
set(CMAKE_AUTOMOC ON)

# Run rcc when needed
set(CMAKE_AUTORCC ON)

# Run uic automatically when needed.
set(CMAKE_AUTOUIC ON)

# NOTE: add_executable() should not include auto-generated files
add_executable(qt-mbgrdviz-3
               main.cpp qml.qrc)


target_include_directories(qt-mbgrdviz-3
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIRS}
                           ${GDAL_INCLUDE_DIRS}
                           ${CMAKE_SOURCE_DIR}/src/qt-guilib)

target_link_libraries(qt-mbgrdviz-3
                      PRIVATE
                      MBGui
                      mbev
                      mbio
                      mbaux
		      mbxgr
                      mbview
                      mbbsio
                      mbsapi                                            
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
                           
                           

