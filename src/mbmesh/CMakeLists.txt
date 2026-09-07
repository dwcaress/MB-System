#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   3/6/2026
#
#    Copyright (c) 2026 by
#    [Your Team/Institution]
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/mbmesh")

set(MBMESH_SOURCES
  mbmesh.cpp
  settings.cpp
  src/io/datalist_reader.cpp
  src/io/xyz_writer.cpp
  src/io/glb_writer.cpp
  src/io/x3dom_writer.cpp
  src/algorithms/point_decimation.cpp
  src/algorithms/normal_estimation.cpp
  src/algorithms/screened_poisson.cpp
  src/algorithms/marching_cubes.cpp
  src/algorithms/support_trimming.cpp)

add_executable(mbmesh ${MBMESH_SOURCES})
target_compile_features(mbmesh PRIVATE cxx_std_17)

# Link to MB-System mbio library
target_link_libraries(mbmesh PRIVATE mbio)

target_include_directories(mbmesh PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/mbio
  ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)

install(TARGETS mbmesh DESTINATION ${CMAKE_INSTALL_BINDIR})

if(buildTests)
  add_executable(mbmesh_test_support_trimming
    tests/test_support_trimming.cpp
    src/algorithms/support_trimming.cpp
    src/algorithms/screened_poisson.cpp
    src/algorithms/marching_cubes.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_support_trimming PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_support_trimming PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_support_trimming COMMAND mbmesh_test_support_trimming)

  add_executable(mbmesh_test_settings
    tests/test_settings.cpp
    settings.cpp
    src/io/datalist_reader.cpp
    src/algorithms/point_decimation.cpp
    src/algorithms/normal_estimation.cpp
    src/algorithms/screened_poisson.cpp
    src/algorithms/marching_cubes.cpp
    src/algorithms/support_trimming.cpp)
  target_compile_features(mbmesh_test_settings PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_settings PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbio
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  target_link_libraries(mbmesh_test_settings PRIVATE mbio)
  add_test(NAME mbmesh_settings COMMAND mbmesh_test_settings)
endif()
