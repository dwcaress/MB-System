#include "synthetic_data.h"
#include "io/glb_writer.h"
#include "algorithms/marching_cubes.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

int main() {
    constexpr int grid_size = 36;
    constexpr double cell_size = 0.08;
    const std::filesystem::path output_dir = "src/mbmesh/output";
    const std::filesystem::path glb_path = output_dir / "mc-test-mesh.glb";
    const std::filesystem::path html_path = output_dir / "mc-test-mesh.html";

    std::filesystem::create_directories(output_dir);

    const ScalarGrid3D grid = sample_sdf_grid(sphere_sdf, grid_size, cell_size);

    MarchingCubesOptions options;
    options.iso_value = 0.0;
    const Mesh mesh = marching_cubes(grid, options);

    std::printf("Sphere SDF marching cubes: %zu vertices, %zu triangles\n",
                mesh.vertices.size(),
                mesh.indices.size() / 3);

    if (mesh.vertices.empty()) {
        std::fprintf(stderr, "Expected non-empty sphere mesh\n");
        return 1;
    }

    std::string error;

    if (!write_mesh_glb_file(glb_path, mesh, &error)) {
        std::fprintf(stderr, "Failed to write GLB: %s\n", error.c_str());
        return 1;
    }

    return 0;
}

/*
g++ -std=c++17 -I src/mbmesh/include -I src/mbgrd2gltf \
  src/mbmesh/tests/test_marching_cubes.cpp \
  src/mbmesh/src/algorithms/marching_cubes.cpp \
  src/mbmesh/src/io/glb_writer.cpp \
  -o /tmp/mbmesh_test_marching_cubes

/tmp/mbmesh_test_marching_cubes
*/
