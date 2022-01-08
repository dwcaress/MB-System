#
# Gets variables from root CMakeLists
#

message("in mbvelocitytool")

add_executable(mbvelocitytool
               mbvelocity.c
               mbvelocity_bxutils.c
	       mbvelocity_callbacks.c
               mbvelocity_creation.c
	       mbvelocity_prog.c)
	       
	      
target_include_directories(mbvelocitytool
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
			   ${NETCDF_INCLUDE_DIR}
			   ${X11_INCLUDE_DIR})
			   
# link_directories($GMT_LIBRARIES)
target_link_libraries(mbvelocitytool
                      PRIVATE
                      mbio
                      mbaux
		      mbxgr
                      mbview
                      mbbsio
                      mbsapi
                      mbgsf
		      r7kr
		      mframe
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
                           
                           

