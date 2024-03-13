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

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the directory where the Draco source files are located
set(DRACO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/draco")

# Manually set the include directories for Draco and other components
include_directories(
    ${NetCDF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf
    ${DRACO_DIR}
)

# Recursively find all Draco .cc files and .h files
file(GLOB_RECURSE DRACO_SOURCES "${DRACO_DIR}/*.cc" "${DRACO_DIR}/*.h")
# Recursively filter out all test files
list(FILTER DRACO_SOURCES EXCLUDE REGEX ".*/.*test.*")

message(STATUS "Draco sources: ${DRACO_SOURCES}")

add_executable(mbgrd2gltf
	bathymetry.cpp compression.cpp geometry.cpp main.cpp model.cpp options.cpp ${DRACO_SOURCES} )


target_include_directories(mbgrd2gltf
    PRIVATE ${NetCDF_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf/tinygltf
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf
	${CMAKE_SOURCE_DIR}/draco)


target_link_libraries(mbgrd2gltf
	PRIVATE mbsystem
	PRIVATE ${NetCDF_LIBRARIES}
	PRIVATE ${CMAKE_DL_LIBS})



install(TARGETS mbgrd2gltf
	DESTINATION ${CMAKE_INSTALL_BINDIR})
