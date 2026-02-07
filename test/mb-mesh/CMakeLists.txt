# ------------------------------------------------------------------------------
# The MB-system:  CMakeLists.txt   2/6/2026
#
# Copyright (c) 2026 by
# David W. Caress (caress@mbari.org)
# Monterey Bay Aquarium Research Institute
# Moss Landing, California, USA
#
# See README.md file for copying and redistribution conditions.
# ------------------------------------------------------------------------------

# Add mb-mesh test
add_test(NAME mb-mesh_test 
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/mb-mesh_test.py"
         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Set test properties
set_tests_properties(mb-mesh_test PROPERTIES
    TIMEOUT 120
    LABELS "mb-mesh;mesh;gltf"
)
