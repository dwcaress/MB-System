#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "data_types/geometry.h"

// The pieces of the level-of-detail tiling, on meshes in the local east-north-up frame (meters).

// Smooth vertex normals from the triangles, each triangle weighted by its area; replaces mesh->normals. A vertex of no triangle gets up.
void compute_vertex_normals(Mesh *mesh);

// The spacing of a mesh that is a surface seen from above: the side of the square that one vertex has, on average, of the area of the
// triangles projected onto the horizontal plane. 0 if there is none.
[[nodiscard]] double native_spacing(const Mesh &mesh);

// A coarser mesh: the vertices in each cell of a lattice of cubes of cell_size meters, with a corner of a cube at anchor, become one vertex, the mean of
// them; the triangles that are left with two corners the same are dropped, and so are repeats. The whole mesh is clustered, and cut into tiles afterwards, so
// the tiles meet exactly along their edges. Normals are not made. Returns false, with the
// reason in *error, if the lattice has more than about two million cells along an axis.
[[nodiscard]] bool cluster_mesh(const Mesh &mesh, const Vec3 &anchor, double cell_size, Mesh *result, std::string *error);

// One tile of a mesh that has been cut apart: the square of the horizontal plane that the lattice of tile_size meters, with a corner at (x0, y0), has at
// index (i, j), and the triangles whose centers are in it. The vertices are those of the triangles, in the order they are first used, so the vertices on
// the edge of a tile are in both tiles, at the same place with the same normal.
struct MeshTile {
    int i = 0;
    int j = 0;
    Mesh mesh;
};

[[nodiscard]] std::vector<MeshTile> cut_mesh(const Mesh &mesh, double x0, double y0, double tile_size);
