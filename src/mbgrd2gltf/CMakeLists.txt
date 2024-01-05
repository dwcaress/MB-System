#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes 
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#      
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

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
