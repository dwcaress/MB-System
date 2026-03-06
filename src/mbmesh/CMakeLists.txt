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
add_executable(mbmesh mbmesh.cpp)

# Link to MB-System mbio library
target_link_libraries(mbmesh PRIVATE mbio)

# Include MB-System headers
target_include_directories(mbmesh PRIVATE ${CMAKE_SOURCE_DIR}/src/mbio)

# Install
install(TARGETS mbmesh DESTINATION ${CMAKE_INSTALL_BINDIR})