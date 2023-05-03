#
# Gets variables from root CMakeLists
#

message("in mbgrd2gltf")

# list(APPEND CMAKE_PREFIX_PATH "/home/vagrant/Downloads/netcdf-cxx4/build")

add_executable(mbgrd2gltf
	bathymetry.cpp compression.cpp geometry.cpp main.cpp model.cpp options.cpp)

target_include_directories(mbgrd2gltf
	PUBLIC
	.
	${CMAKE_SOURCE_DIR}/src/${PROJECT_NAME}/tinygltf
	${NETCDF_INCLUDE_DIRS})

target_link_libraries(mbgrd2gltf
	PRIVATE
	${NETCDF_LIBRARIES})
	
# Getting tests
# file(GLOB TEST_SRCS "test/*.cpp")

# Creating executables for tests
# foreach(TEST ${TEST_SRCS})
#	get_filename_component(TEST_TARGET_NAME ${TEST} NAME_WE)
#	add_executable(${TEST_TARGET_NAME} ${TEST} $<TARGET_OBJECTS:common_objects>)
#	list(APPEND TARGETS ${TEST_TARGET_NAME})
# endforeach()

install(TARGETS mbgrd2gltf
	DESTINATION ${CMAKE_INSTALL_BINDIR})
