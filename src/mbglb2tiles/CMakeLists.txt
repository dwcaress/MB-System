#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   mbglb2tiles
#
#    OGC 3D Tiles of levels of detail from a GLB mesh of mbmesh or mbgrd2gltf.
#    The tileset writer, the coordinate frame and the GLB writer are those of mbmesh.
#------------------------------------------------------------------------------

message("In src/mbglb2tiles")

set(MBMESH_DIR ${CMAKE_SOURCE_DIR}/src/mbmesh)

set(MBGLB2TILES_SHARED_SOURCES
  ${MBMESH_DIR}/src/io/coordinate_frame.cpp
  ${MBMESH_DIR}/src/io/provenance.cpp
  ${MBMESH_DIR}/src/io/glb_writer.cpp
  ${MBMESH_DIR}/src/io/tileset_writer.cpp
  ${MBMESH_DIR}/src/io/x3dom_writer.cpp
  ${MBMESH_DIR}/src/io/cesium_writer.cpp
  ${MBMESH_DIR}/src/algorithms/seam_blending.cpp
  ${MBMESH_DIR}/src/algorithms/marching_cubes.cpp)

find_package(Threads REQUIRED)

add_executable(mbglb2tiles
  mbglb2tiles.cpp
  glb_reader.cpp
  mesh_levels.cpp
  ${MBGLB2TILES_SHARED_SOURCES})
target_compile_features(mbglb2tiles PRIVATE cxx_std_17)
target_include_directories(mbglb2tiles PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MBMESH_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/mbio
  ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
target_link_libraries(mbglb2tiles PRIVATE mbio mbdraco Threads::Threads)

install(TARGETS mbglb2tiles DESTINATION ${CMAKE_INSTALL_BINDIR})

if(buildTests)
  add_executable(mbglb2tiles_test_levels
    tests/test_levels.cpp
    mesh_levels.cpp
    glb_reader.cpp
    ${MBGLB2TILES_SHARED_SOURCES})
  target_compile_features(mbglb2tiles_test_levels PRIVATE cxx_std_17)
  target_include_directories(mbglb2tiles_test_levels PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MBMESH_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/mbio
    ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
  target_link_libraries(mbglb2tiles_test_levels PRIVATE mbdraco Threads::Threads)
  add_test(NAME mbglb2tiles_levels COMMAND mbglb2tiles_test_levels)
endif()
