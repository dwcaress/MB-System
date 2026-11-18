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
  src/io/coordinate_frame.cpp
  src/io/datalist_reader.cpp
  src/io/point_spill.cpp
  src/io/xyz_writer.cpp
  src/io/track.cpp
  src/io/provenance.cpp
  src/io/glb_writer.cpp
  src/io/tileset_writer.cpp
  src/io/x3dom_writer.cpp
  src/io/cesium_writer.cpp
  src/algorithms/point_decimation.cpp
  src/algorithms/normal_estimation.cpp
  src/algorithms/screened_poisson.cpp
  src/algorithms/marching_cubes.cpp
  src/algorithms/support_trimming.cpp
  src/algorithms/seam_blending.cpp
  src/algorithms/reconstruction.cpp)

find_package(Threads REQUIRED)

# The shared Draco compression (src/mbdraco): glb_writer.h and provenance.h name its types.
include_directories(${CMAKE_SOURCE_DIR}/src/mbdraco)

add_executable(mbmesh ${MBMESH_SOURCES})
target_compile_features(mbmesh PRIVATE cxx_std_17)

# Link to MB-System mbio library
target_link_libraries(mbmesh PRIVATE mbio Threads::Threads mbdraco)

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
    src/io/provenance.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_support_trimming PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_support_trimming PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_support_trimming COMMAND mbmesh_test_support_trimming)

  add_executable(mbmesh_test_settings
    tests/test_settings.cpp
    settings.cpp
    src/io/coordinate_frame.cpp
    src/io/datalist_reader.cpp
    src/io/point_spill.cpp
    src/algorithms/point_decimation.cpp
    src/algorithms/normal_estimation.cpp
    src/algorithms/screened_poisson.cpp
    src/algorithms/marching_cubes.cpp
    src/algorithms/support_trimming.cpp
    src/algorithms/seam_blending.cpp
    src/algorithms/reconstruction.cpp)
  target_compile_features(mbmesh_test_settings PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_settings PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbio
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  target_link_libraries(mbmesh_test_settings PRIVATE mbio Threads::Threads)
  add_test(NAME mbmesh_settings COMMAND mbmesh_test_settings)

  add_executable(mbmesh_test_coverage_footprint
    tests/test_coverage_footprint.cpp
    src/algorithms/support_trimming.cpp)
  target_compile_features(mbmesh_test_coverage_footprint PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_coverage_footprint PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_coverage_footprint COMMAND mbmesh_test_coverage_footprint)

  add_executable(mbmesh_test_seam_blending
    tests/test_seam_blending.cpp
    src/algorithms/seam_blending.cpp
    src/algorithms/marching_cubes.cpp)
  target_compile_features(mbmesh_test_seam_blending PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_seam_blending PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_seam_blending COMMAND mbmesh_test_seam_blending)

  add_executable(mbmesh_test_point_spill
    tests/test_point_spill.cpp
    src/io/point_spill.cpp)
  target_compile_features(mbmesh_test_point_spill PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_point_spill PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_point_spill COMMAND mbmesh_test_point_spill)

  add_executable(mbmesh_test_coordinate_frame
    tests/test_coordinate_frame.cpp
    src/io/coordinate_frame.cpp)
  target_compile_features(mbmesh_test_coordinate_frame PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_coordinate_frame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_coordinate_frame COMMAND mbmesh_test_coordinate_frame)

  add_executable(mbmesh_test_chunking
    tests/test_chunking.cpp
    src/io/point_spill.cpp
    src/algorithms/reconstruction.cpp
    src/algorithms/seam_blending.cpp
    src/algorithms/normal_estimation.cpp
    src/algorithms/screened_poisson.cpp
    src/algorithms/marching_cubes.cpp
    src/algorithms/support_trimming.cpp)
  target_compile_features(mbmesh_test_chunking PRIVATE cxx_std_17)
  target_link_libraries(mbmesh_test_chunking PRIVATE Threads::Threads)
  target_include_directories(mbmesh_test_chunking PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_chunking COMMAND mbmesh_test_chunking)

  add_executable(mbmesh_test_track
    tests/test_track.cpp
    src/io/track.cpp
    src/io/provenance.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_track PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_track PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_track COMMAND mbmesh_test_track)

  add_executable(mbmesh_test_x3dom_writer
    tests/test_x3dom_writer.cpp
    src/io/coordinate_frame.cpp
    src/io/provenance.cpp
    src/io/x3dom_writer.cpp)
  target_compile_features(mbmesh_test_x3dom_writer PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_x3dom_writer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_x3dom_writer COMMAND mbmesh_test_x3dom_writer)

  add_executable(mbmesh_test_cesium_writer
    tests/test_cesium_writer.cpp
    src/io/cesium_writer.cpp)
  target_compile_features(mbmesh_test_cesium_writer PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_cesium_writer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  add_test(NAME mbmesh_cesium_writer COMMAND mbmesh_test_cesium_writer)

  add_executable(mbmesh_test_tileset
    tests/test_tileset.cpp
    src/io/coordinate_frame.cpp
    src/io/provenance.cpp
    src/io/tileset_writer.cpp)
  target_compile_features(mbmesh_test_tileset PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_tileset PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_tileset COMMAND mbmesh_test_tileset)

  add_executable(mbmesh_test_provenance
    tests/test_provenance.cpp
    src/io/coordinate_frame.cpp
    src/io/provenance.cpp
    src/io/glb_writer.cpp
    src/io/x3dom_writer.cpp)
  target_compile_features(mbmesh_test_provenance PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_provenance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_provenance COMMAND mbmesh_test_provenance)

  add_executable(mbmesh_test_marching_cubes
    tests/test_marching_cubes.cpp
    src/algorithms/marching_cubes.cpp
    src/io/provenance.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_marching_cubes PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_marching_cubes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_marching_cubes COMMAND mbmesh_test_marching_cubes)

  add_executable(mbmesh_test_normal_estimation
    tests/test_normal_estimation.cpp
    src/algorithms/normal_estimation.cpp
    src/io/provenance.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_normal_estimation PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_normal_estimation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_normal_estimation COMMAND mbmesh_test_normal_estimation)

  add_executable(mbmesh_test_draco_mesh
    tests/test_draco_mesh.cpp
    src/io/provenance.cpp
    src/io/glb_writer.cpp)
  target_compile_features(mbmesh_test_draco_mesh PRIVATE cxx_std_17)
  target_include_directories(mbmesh_test_draco_mesh PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  add_test(NAME mbmesh_draco_mesh COMMAND mbmesh_test_draco_mesh)

  # glb_writer.cpp compresses the meshes with the shared mbdraco library, so every test that holds it links the library.
  get_property(mbmesh_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  foreach(mbmesh_target ${mbmesh_targets})
    get_target_property(mbmesh_target_sources ${mbmesh_target} SOURCES)
    if(mbmesh_target_sources MATCHES "glb_writer.cpp" AND NOT mbmesh_target STREQUAL "mbmesh")
      target_link_libraries(${mbmesh_target} PRIVATE mbdraco)
    endif()
  endforeach()

  add_test(NAME mbmesh_test COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests/mbmesh_test.py")
endif()
