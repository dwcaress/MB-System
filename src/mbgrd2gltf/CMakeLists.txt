message("In src/mbgrd2gltf")

find_package(NetCDF REQUIRED)

add_executable(mbgrd2gltf
	bathymetry.cpp compression.cpp geometry.cpp main.cpp model.cpp options.cpp)

target_include_directories(mbgrd2gltf
	PRIVATE ${NetCDF_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src/mbgrd2gltf/tinygltf)

target_link_libraries(mbgrd2gltf
	PRIVATE NetCDF::NetCDF)

install(TARGETS mbgrd2gltf
	DESTINATION ${CMAKE_INSTALL_BINDIR})
