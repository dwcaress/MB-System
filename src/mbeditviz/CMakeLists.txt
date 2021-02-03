#
# Gets variables from root CMakeLists
#

message("in mbeditviz")

add_executable(mbeditviz
	       mbeditviz_main
               mbeditviz_creation
	       mbeditviz_prog
	       mbeditviz_callbacks)

target_include_directories(mbeditviz
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
			   ${NETCDF_INCLUDE_DIR}
			   ${X11_INCLUDE_DIR})
			   
# link_directories($GMT_LIBRARIES)
target_link_libraries(mbeditviz
                      PRIVATE
                      mbio
                      mbaux
		      mbxgr
                      mbview
                      mbbsio
                      mbsapi
                      mbgsf
		      ${GMT_LIBRARIES}
		      ${NETCDF_LIBRARIES}
		      ${MOTIF_LIBRARIES}
		      ${X11_LIBRARIES}
		      ${X11_XT_LIBRARIES}		      
		      ${GDAL_LIBRARIES}
		      ${PROJ_LIBRARIES}
		      ${OPENGL_LIBRARIES}
                      m
                      pthread)
                           
                           

