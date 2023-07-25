#
# Gets variables from root CMakeLists
#

message("in deprecated")

set(executables
    mb7k2jstar mb7k2ss mb7kpreprocess mbauvnavusbl
    mbhsdump mbhysweeppreprocess mbinsreprocess
    mbkongsbergpreprocess mbneptune2esf
    mbrollbias mbrphsbias mbstripnan
    mbswplspreprocess
    )

foreach(executable ${executables})
  add_executable(${executable}
                 ${executable}.cc)

  target_include_directories(${executable}
                             PUBLIC
                             .
                             ${GMT_INCLUDE_DIRS}
                             ${NETCDF_INCLUDE_DIRS}                           
                             ${CMAKE_SOURCE_DIR}/src/mbio
                             ${CMAKE_SOURCE_DIR}/src/mbaux
                             ${CMAKE_SOURCE_DIR}/src/gsf)


  target_link_libraries(${executable}
                        PRIVATE
                        mbio
                        mbbsio
                        mbaux
                        mbsapi
                        ${GSF_LIB}
			${R7KR_LIB}
			${MFRAME_LIB}
	                ${GMT_LIBRARIES}
                        ${NETCDF_LIBRARIES}
                        ${GDAL_LIBRARIES}
                        ${PROJ_LIBRARIES}
                        ${FFTW_LIBRARIES}
                        m
                        pthread)

endforeach()
                           
                           
install(TARGETS ${executables}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
	
