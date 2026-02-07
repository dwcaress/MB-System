# ------------------------------------------------------------------------------
# The MB-system:  CMakeLists.txt   2/6/2026
#
# Copyright (c) 2026 by
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

message("In src/mb-mesh")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the sources for the executable
set(SOURCES 
    main.cpp 
    mesh_generator.cpp 
    mesh_options.cpp 
    logger.cpp
)

# Create the executable
add_executable(mb-mesh ${SOURCES})

# Set include directories
target_include_directories(mb-mesh
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(mb-mesh
    PRIVATE ${CMAKE_DL_LIBS}
)

# Install target
install(TARGETS mb-mesh DESTINATION bin)
