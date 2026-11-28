#pragma once

#include <cstddef>
#include <vector>

#include "../data_types/geometry.h"

/*******************************************************************************
 * Chunks that agree where they meet.
 *
 * The screened Poisson fields of two neighboring chunks are solved apart, from
 * their own points, and differ where the chunks meet: by an additive offset
 * (each field is shifted so that its own samples sit on the iso-value) and by
 * the shape of the solution. The two surfaces then have a step between them
 * and the triangles that each keeps do not share vertices, so the seam shows.
 *
 * The grids of all the chunks have their nodes on one lattice, and a chunk's
 * edges are on lattice planes. If two chunks have the same field values on the
 * nodes of the plane they share, marching cubes makes the same vertices along
 * it, so the two meshes meet exactly, with no step and no gap.
 *
 * So a chunk takes the final field of the chunks that were finished before it
 * (the one to its west and the one to its south) on the planes it shares with
 * them, and adds to its own field a correction that is the difference there and
 * dies away smoothly, over a width, into the chunk. The field of the chunk is
 * left alone farther from the planes than that. The chunk then publishes its
 * own final field on its east and north planes, for the chunks that follow it.
 ******************************************************************************/

// The values of a field on the nodes of one plane of the lattice, the plane x = constant (axis 0, with u the y index) or
// y = constant (axis 1, with u the x index), between two u indices and over a range of z indices. Lattice indices are
// the position over the cell size, so that the planes of two grids are compared whatever their origins.
struct FieldPlane {
    int axis = 0;
    long plane_index = 0;
    long first_u = 0;
    long count_u = 0;
    long first_z = 0;
    long count_z = 0;
    std::vector<double> values;   // count_z rows of count_u values

    [[nodiscard]] bool empty() const { return values.empty(); }
    [[nodiscard]] bool covers(long u, long z) const {
        return u >= first_u && u < first_u + count_u && z >= first_z && z < first_z + count_z;
    }
    [[nodiscard]] double at(long u, long z) const {
        return values[static_cast<std::size_t>(z - first_z) * static_cast<std::size_t>(count_u) + static_cast<std::size_t>(u - first_u)];
    }
};

// The nodes of grid on the plane (axis, plane_index) with u from first_u to last_u, both included, at every height the
// grid has. Empty if the plane is not in the grid.
[[nodiscard]] FieldPlane extract_field_plane(const ScalarGrid3D &grid, int axis, long plane_index, long first_u, long last_u);

struct SeamBlendResult {
    std::size_t nodes_changed = 0;
    double largest_correction = 0.0;   // in field units
};

// Changes the field of grid so that on the planes west (the plane x = constant of the chunk to the west, which is the
// west edge of this chunk) and south (the plane y = constant, the south edge) it has the values of those planes, and
// dies away to the field it was over width meters, on both sides of each plane. Either plane may be null or empty, for
// no neighbor there. Where a plane does not cover a node (the grid is taller, or wider, than the neighbor's) the
// correction of the nearest node it does cover is used, so that the field has no step there. In the corner where the
// two corrections overlap they are combined so that each plane still has exactly its own values.
SeamBlendResult blend_field_to_planes(ScalarGrid3D *grid, const FieldPlane *west, const FieldPlane *south, double width);

// For a solve with the neighbors' fields as boundary values: puts the values of the planes west and south (as for
// blend_field_to_planes, either may be null) into the nodes of grid that are on them, and marks those nodes in fixed (one
// byte per node of the grid, made if it is empty), so that the solver leaves them alone. Where a plane does not cover a
// node (the grid is taller, or wider, than the neighbor's) the nearest node it does cover gives the value. A node on both planes
// takes the value of the west one. Returns the number of nodes fixed.
std::size_t fix_field_planes(ScalarGrid3D *grid, const FieldPlane *west, const FieldPlane *south, std::vector<unsigned char> *fixed);

// The meshes of two chunks that were matched meet at vertices that are the same points, to rounding (they are made from the
// same field values on a shared plane, in grids with different origins). This merges the vertices of a mesh, such as the
// chunks put together, that are within tolerance of each other, so that the seams are not edges of the mesh, drops the
// triangles that are left with two corners the same, and makes the normals again from the merged triangles, so that
// they are smooth across a seam. Returns the number of vertices that were merged into another.
std::size_t weld_coincident_vertices(Mesh *mesh, double tolerance);
