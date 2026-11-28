# ------------------------------------------------------------------------------
# The MB-system:  CMakeLists.txt   27 June 2023
#
# Copyright (c) 2023-2024 by
# David W. Caress (caress@mbari.org)
# Monterey Bay Aquarium Research Institute
# Moss Landing, California, USA
# Dale N. Chayes
# Center for Coastal and Ocean Mapping
# University of New Hampshire
# Durham, New Hampshire, USA
# Christian dos Santos Ferreira
# MARUM
# University of Bremen
# Bremen Germany
#
# MB-System was created by Caress and Chayes in 1992 at the
# Lamont-Doherty Earth Observatory
# Columbia University
# Palisades, NY 10964
#
# See README.md file for copying and redistribution conditions.
# ------------------------------------------------------------------------------

message("In src/mbgrd2gltf")

find_package(NetCDF REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the sources for the executable
# The page for CesiumJS (--cesium-html) is written by the source of mbmesh and mbglb2tiles, which uses only the standard library.
set(SOURCES bathymetry.cpp geometry.cpp logger.cpp main.cpp model.cpp options.cpp provenance.cpp viewpoint.cpp
	${CMAKE_SOURCE_DIR}/src/mbmesh/src/io/cesium_writer.cpp)

# Include directories (Draco is in the shared mbdraco library, src/mbdraco)
include_directories(
	${NetCDF_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/tinygltf
)

# Specify sources for the executable
add_executable(mbgrd2gltf ${SOURCES})

# Set and link the include directories
target_include_directories(mbgrd2gltf
	PRIVATE ${NetCDF_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf
	PRIVATE ${CMAKE_SOURCE_DIR}/src/mbmesh/include
)

target_link_libraries(mbgrd2gltf
	PRIVATE mbsystem
	PRIVATE ${NetCDF_LIBRARIES}
	PRIVATE mbdraco
	PRIVATE ${CMAKE_DL_LIBS}
)

# Regression test for the vertex geometry (no Draco or glTF code is needed).
if(buildTests)
	add_executable(mbgrd2gltf_test_geometry
		tests/test_geometry.cpp bathymetry.cpp geometry.cpp logger.cpp options.cpp viewpoint.cpp)
	target_include_directories(mbgrd2gltf_test_geometry
		PRIVATE ${NetCDF_INCLUDE_DIRS}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	)
	target_link_libraries(mbgrd2gltf_test_geometry PRIVATE ${NetCDF_LIBRARIES} mbdraco)
	add_test(NAME mbgrd2gltf_geometry COMMAND mbgrd2gltf_test_geometry)

	add_executable(mbgrd2gltf_test_provenance tests/test_provenance.cpp provenance.cpp)
	target_link_libraries(mbgrd2gltf_test_provenance PRIVATE mbdraco)
	target_include_directories(mbgrd2gltf_test_provenance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	add_test(NAME mbgrd2gltf_provenance COMMAND mbgrd2gltf_test_provenance)

	add_executable(mbgrd2gltf_test_viewpoint tests/test_viewpoint.cpp viewpoint.cpp)
	target_include_directories(mbgrd2gltf_test_viewpoint PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	add_test(NAME mbgrd2gltf_viewpoint COMMAND mbgrd2gltf_test_viewpoint)

	# The scale object of the pages, held in scale_axes.h for mbmesh and mbglb2tiles as well.
	add_executable(mbgrd2gltf_test_scale_axes tests/test_scale_axes.cpp)
	target_include_directories(mbgrd2gltf_test_scale_axes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	add_test(NAME mbgrd2gltf_scale_axes COMMAND mbgrd2gltf_test_scale_axes)
endif()

install(TARGETS mbgrd2gltf
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
