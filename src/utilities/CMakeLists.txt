#
# Gets variables from root CMakeLists
#

message("in utilities")

add_executable(mbabsorption
               mbabsorption.cc)

target_include_directories(mbabsorption
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${NETCDF_INCLUDE_DIR}                           
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux
                           ${CMAKE_SOURCE_DIR}/src/gsf)                        

target_link_libraries(mbabsorption
                      PRIVATE
                      mbio
                      mbaux
                      gsf
		      ${GMT_LIBRARIES}
		      ${NETCDF_LIBRARIES}
                      m
                      pthread)
                           
                           

