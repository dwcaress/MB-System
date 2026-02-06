# ------------------------------------------------------------------------------
# The MB-system:  CMakeLists.txt   6 February 2026
#
# Copyright (c) 2026-2026 by
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

message("In src/mbmesh")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the sources for the executable
set(SOURCES mbmesh.cc)

# Create the executable
add_executable(mbmesh ${SOURCES})

# Link libraries
target_link_libraries(mbmesh
  PRIVATE mbsystem
  PRIVATE mbaux
  PRIVATE mbio
)

# Install the executable
install(TARGETS mbmesh DESTINATION ${CMAKE_INSTALL_BINDIR})
