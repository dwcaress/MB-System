# ------------------------------------------------------------------------------
# The MB-system:  CMakeLists.txt   mbdraco
#
# Draco compression of triangle meshes, shared by mbgrd2gltf and mbmesh: the Draco
# encoder and decoder source in draco/, and draco_mesh.h/.cpp, which choose the
# quantization of a mesh and write the KHR_draco_mesh_compression primitive of a
# glTF. It is a static library, mbdraco, that the programs link.
# ------------------------------------------------------------------------------

message("In src/mbdraco")

set(MBDRACO_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# The sources are held as paths relative to this directory (draco/...), so that the filter of the test files below does not
# depend on where the source tree is located: a path such as /home/test/MB-System has "test" in it.
file(GLOB_RECURSE MBDRACO_DRACO_SOURCES RELATIVE "${MBDRACO_DIR}" "${MBDRACO_DIR}/draco/*.cc")
list(FILTER MBDRACO_DRACO_SOURCES EXCLUDE REGEX ".*/.*test.*")

add_library(mbdraco STATIC ${MBDRACO_DRACO_SOURCES} draco_mesh.cpp)
target_compile_features(mbdraco PRIVATE cxx_std_17)
# The programs include "draco_mesh.h" and "draco/...", and tinygltf is held beside the glTF writers of mbgrd2gltf.
target_include_directories(mbdraco PUBLIC ${MBDRACO_DIR} ${CMAKE_SOURCE_DIR}/src/mbgrd2gltf)
target_compile_definitions(mbdraco PRIVATE MBDRACO_WITH_DRACO)
# The vendored Draco source is not ours to make quiet.
set_source_files_properties(${MBDRACO_DRACO_SOURCES} PROPERTIES COMPILE_OPTIONS -w)
set_target_properties(mbdraco PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(buildTests)
  add_executable(mbdraco_test_draco_mesh test_mbdraco.cpp)
  target_compile_features(mbdraco_test_draco_mesh PRIVATE cxx_std_17)
  target_link_libraries(mbdraco_test_draco_mesh PRIVATE mbdraco)
  add_test(NAME mbdraco_draco_mesh COMMAND mbdraco_test_draco_mesh)
endif()
