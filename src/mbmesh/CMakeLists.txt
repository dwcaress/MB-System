#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   3/6/2026
#
#    Copyright (c) 2026 by
#    [Your Team/Institution]
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/mbmesh")

# mbmesh executable
add_executable(mbmesh mbmesh.cpp pointcloud_glb_writer.cpp)

# Link to MB-System mbio library
target_link_libraries(mbmesh PRIVATE mbio)

# Include MB-System headers and the bundled tinygltf headers
target_include_directories(mbmesh PRIVATE
  ${CMAKE_SOURCE_DIR}/src/mbio
  ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)

# Install
install(TARGETS mbmesh DESTINATION ${CMAKE_INSTALL_BINDIR})

# Unit tests for mbmesh
if(ENABLE_TESTING)
  add_executable(test_mbmesh test_mbmesh.cpp)
  
  # Link against Google Test and MB-System libraries
  target_link_libraries(test_mbmesh PRIVATE gtest gtest_main mbio)
  
  # Include MB-System headers
  target_include_directories(test_mbmesh PRIVATE ${CMAKE_SOURCE_DIR}/src/mbio)
  
  # Register test
  add_test(NAME MBMeshTests COMMAND test_mbmesh)
  
  message("Added test_mbmesh target for unit testing")
endif()