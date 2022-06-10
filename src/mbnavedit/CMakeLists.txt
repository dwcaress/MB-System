#
# Gets variables from root CMakeLists
#

message("in mbnavedit")

add_executable(mbnavedit
               mbnavedit.c
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
			   ${NETCDF_INCLUDE_DIRS}
			   ${X11_INCLUDE_DIRS})
			   
# link_directories($GMT_LIBRARIES)
target_link_libraries(mbnavedit
                      PRIVATE
                      mbio
                      mbaux
		      mbxgr
                      mbview
                      mbbsio
                      mbsapi
                      ${GSF_LIB}
		      ${R7KR_LIB}
		      ${MFRAME_LIB}
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
                           
                           
install(TARGETS mbnavedit
	DESTINATION ${CMAKE_INSTALL_BINDIR})
	
