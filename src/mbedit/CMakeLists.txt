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
			   ${NETCDF_INCLUDE_DIR}
			   ${X11_INCLUDE_DIR})
			   
# link_directories($GMT_LIBRARIES)
target_link_libraries(mbedit
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
                      m
                      pthread)
                           
                           

