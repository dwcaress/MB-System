#
# Gets variables from root CMakeLists
#

message("in mbedit")

add_executable(mbedit
               mbedit.c
               mbedit_creation.c
	       mbedit_bxutils.c
	       mbedit_prog.c
	       mbedit_callbacks.c)

target_include_directories(mbedit
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
			   ${NETCDF_INCLUDE_DIRS}
			   ${X11_INCLUDE_DIRS})

# link_directories($GMT_LIBRARIES)
target_link_libraries(mbedit
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
                      m
                      pthread)
                           
                           
install(TARGETS mbedit
	DESTINATION ${CMAKE_INSTALL_BINDIR})
