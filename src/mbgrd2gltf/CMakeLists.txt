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

# Attempt to find an installed version of Draco
find_package(Draco)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set the draco directory
set(DRACO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/draco")

# Set the sources for the executable
set(SOURCES bathymetry.cpp compression.cpp geometry.cpp main.cpp model.cpp options.cpp)

# Manually set the include directories for Draco and other components
include_directories(
    ${NetCDF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf
    ${DRACO_DIR}
)

if(NOT TARGET Draco::Draco)
    if(NOT Draco_FOUND)
        message("Using local Draco source files from the project directory")

        # Recursively find all Draco .cc files and .h files
        file(GLOB_RECURSE DRACO_SOURCES "${DRACO_DIR}/*.cc" "${DRACO_DIR}/*.h")
        # Recursively filter out all test files
        list(FILTER DRACO_SOURCES EXCLUDE REGEX ".*/.*test.*")

		# Set the include directories and sources conditionally.
        set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/draco)
		set(SOURCES ${SOURCES} ${DRACO_SOURCES})
    else()
		message("Using installed Draco, but Draco target not found, checking for libraries...")
		find_library(DRACO_LIBRARY NAMES draco PATHS /usr/local/lib)
		if(DRACO_LIBRARY)
			set(DRACO_LIBRARIES ${DRACO_LIBRARY})
			find_path(DRACO_INCLUDE_DIR draco/draco_features.h /usr/local/include)
			include_directories(${DRACO_INCLUDE_DIR})
		else()
			message(FATAL_ERROR "Draco library not found.")
		endif()
    endif()
else()
    message("Using installed Draco target: Draco::Draco")
    set(DRACO_LIBRARIES Draco::Draco)
endif()


# Specify sources for the executable
add_executable(mbgrd2gltf ${SOURCES})

# Set and link the include directories
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf/tinygltf)
target_include_directories(mbgrd2gltf
    PRIVATE ${NetCDF_INCLUDE_DIRS}
    PRIVATE ${INCLUDE_DIRS}
)

target_link_libraries(mbgrd2gltf
    PRIVATE mbsystem
    PRIVATE ${NetCDF_LIBRARIES}
    PRIVATE ${DRACO_LIBRARIES}
    PRIVATE ${CMAKE_DL_LIBS}
)

install(TARGETS mbgrd2gltf
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
