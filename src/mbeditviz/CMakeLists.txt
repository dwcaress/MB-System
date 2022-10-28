#
# Gets variables from root CMakeLists
#

message("in mbeditviz")

add_executable(mbeditviz
	       mbeditviz_main.c
               mbeditviz_creation.c
	       mbeditviz_prog.c
	       mbeditviz_callbacks.c)


# Build library just containing non-GUI functions
add_library(mbev
            mbeditviz_prog.c)


target_include_directories(mbeditviz
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
			   ${NETCDF_INCLUDE_DIRS}
			   ${X11_INCLUDE_DIRS})

target_include_directories(mbev
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf
                           ${CMAKE_SOURCE_DIR}/src/mbview
			   ${NETCDF_INCLUDE_DIRS}
			   ${X11_INCLUDE_DIRS})

# link_directories($GMT_LIBRARIES)
target_link_libraries(mbeditviz
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
                           
                           


install(TARGETS mbeditviz
	DESTINATION ${CMAKE_INSTALL_BINDIR})
