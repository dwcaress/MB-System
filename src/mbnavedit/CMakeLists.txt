#
# Gets variables from root CMakeLists
#

message("in mbnavedit")

add_executable(mbnavedit
	       mbnavedit_callbacks.c
               mbnavedit_creation.c
	       mbnavedit_prog.c
	       mbnavedit_util.c)

target_include_directories(mbnavedit
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
			   ${NETCDF_INCLUDE_DIR}
			   ${X11_INCLUDE_DIR})
			   
# link_directories($GMT_LIBRARIES)
target_link_libraries(mbnavedit
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
                           
                           

