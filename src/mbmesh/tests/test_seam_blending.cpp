#include "algorithms/marching_cubes.h"
#include "algorithms/seam_blending.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::printf("FAILED: %s\n", what);
        failures++;
    }
}

constexpr double cell = 0.1;

// A grid whose first node is at lattice index (x0, y0, z0), holding field(x, y, z).
ScalarGrid3D make_grid(int x0, int y0, int z0, int nx, int ny, int nz, const std::function<double(double, double, double)> &field) {
    ScalarGrid3D grid(Vec3(x0 * cell, y0 * cell, z0 * cell), cell, nx, ny, nz);
    for (int z = 0; z < nz; z++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                grid.values[grid.index(x, y, z)] = field((x0 + x) * cell, (y0 + y) * cell, (z0 + z) * cell);
            }
        }
    }
    return grid;
}

// The surface z = height(x, y) as a field that is positive above it.
double surface(double x, double y) {
    return 0.3 * std::sin(0.9 * x) + 0.2 * std::cos(0.7 * y);
}

} // namespace

int main() {
    // Two chunks of 3 m, side by side along x: A owns x in [0, 3], B x in [3, 6]; each grid reaches 0.7 m past its chunk.
    // A's field is the height above the surface, B's the same with an offset of 2 cm and a ripple along the seam.
    const auto field_a = [](double x, double y, double z) { return z - surface(x, y); };
    const auto field_b = [](double x, double y, double z) { return z - surface(x, y) + 0.02 + 0.01 * std::cos(3.0 * y); };
    const int seam = 30;   // lattice index of x = 3
    const ScalarGrid3D a = make_grid(-7, -7, -10, 30 + 7 + 8, 30 + 14 + 1, 21, field_a);
    ScalarGrid3D b = make_grid(seam - 7, -7, -10, 30 + 14 + 1, 30 + 14 + 1, 21, field_b);
    const ScalarGrid3D b_before = b;

    const FieldPlane west = extract_field_plane(a, 0, seam, 0, 30);
    check(!west.empty() && west.count_u == 31 && west.count_z == 21 && west.first_u == 0, "the plane has the nodes asked for, at every height");
    check(west.at(10, -5) == a.values[a.index(seam + 7, 10 + 7, -5 + 10)], "a node of the plane has the value of the grid");
    check(extract_field_plane(a, 0, 1000, 0, 30).empty(), "a plane that is not in the grid is empty");

    const SeamBlendResult result = blend_field_to_planes(&b, &west, nullptr, 1.0);
    check(result.nodes_changed > 0 && result.largest_correction > 0.015 && result.largest_correction < 0.04, "the field is corrected by about the offset");

    // On the plane the field is A's, to the last bit; beyond the width it is B's own; between, it is in between.
    double largest_difference = 0.0;
    for (int z = 0; z < 21; z++) {
        for (int y = 0; y <= 30; y++) {
            largest_difference = std::max(largest_difference, std::fabs(b.at(7, y + 7, z) - west.at(y, z - 10)));
        }
    }
    check(largest_difference < 1.0e-12, "the corrected field has the neighbor's values on the plane");
    bool untouched = true;
    for (int z = 0; z < 21; z++) {
        for (int y = 0; y < 45; y++) {
            for (int x = 7 + 10; x < 45; x++) {   // 1 m (10 cells) from the plane, and beyond
                untouched = untouched && b.at(x, y, z) == b_before.at(x, y, z);
            }
        }
    }
    check(untouched, "the field is not changed 1 m or more from the plane");
    const double half_way = b.at(7 + 5, 20, 10) - b_before.at(7 + 5, 20, 10);
    const double full = b.at(7, 20, 10) - b_before.at(7, 20, 10);
    check(std::fabs(half_way) > 0.0 && std::fabs(half_way) < std::fabs(full) && half_way * full > 0.0, "the correction falls off smoothly with the distance");
    check(b.at(7 - 3, 20, 10) != b_before.at(7 - 3, 20, 10), "the field is also corrected on the other side of the plane, so that it has no jump there");

    // Marching cubes then makes the same vertices along the plane from both grids.
    const Mesh mesh_a = marching_cubes(a, MarchingCubesOptions());
    const Mesh mesh_b = marching_cubes(b, MarchingCubesOptions());
    std::vector<Vec3> on_a, on_b;
    for (const Vec3 &v : mesh_a.vertices) {
        if (std::fabs(v.x - 3.0) < 1.0e-9 && v.y > 0.3 && v.y < 2.7) {
            on_a.push_back(v);
        }
    }
    for (const Vec3 &v : mesh_b.vertices) {
        if (std::fabs(v.x - 3.0) < 1.0e-9 && v.y > 0.3 && v.y < 2.7) {
            on_b.push_back(v);
        }
    }
    double worst = 0.0;
    for (const Vec3 &p : on_a) {
        double nearest = 1.0e9;
        for (const Vec3 &q : on_b) {
            nearest = std::min(nearest, std::sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y) + (p.z - q.z) * (p.z - q.z)));
        }
        worst = std::max(worst, nearest);
    }
    check(on_a.size() > 20 && on_a.size() == on_b.size(), "both meshes have the same number of vertices on the plane");
    check(worst < 1.0e-9, "the vertices on the plane are the same points");
    // Without the correction the two surfaces are 2 cm apart there.
    const Mesh uncorrected = marching_cubes(b_before, MarchingCubesOptions());
    double apart = 0.0;
    for (const Vec3 &v : uncorrected.vertices) {
        if (std::fabs(v.x - 3.0) < 1.0e-9 && v.y > 0.3 && v.y < 2.7) {
            double nearest = 1.0e9;
            for (const Vec3 &q : on_a) {
                nearest = std::min(nearest, std::sqrt((v.x - q.x) * (v.x - q.x) + (v.y - q.y) * (v.y - q.y) + (v.z - q.z) * (v.z - q.z)));
            }
            apart = std::max(apart, nearest);
        }
    }
    check(apart > 0.005, "without the correction the vertices on the plane are apart");

    // The corner: a chunk with a neighbor to the west and one to the south, whose fields agree where they meet.
    // The west neighbor has its offset on the plane x = 3 and the south neighbor on the plane y = 3; both are the same
    // function of x, y, z near the corner column, as the final fields of two chunks that meet there are.
    {
        const auto neighbor_field = [](double x, double y, double z) { return z - surface(x, y) + 0.01 * (x < 3.0 ? -1.0 : 1.0) * 0.0 + 0.015 + 0.004 * std::sin(2.0 * x + 3.0 * y); };
        const ScalarGrid3D west_grid = make_grid(-7, seam, -10, 30 + 14 + 1, 30 + 14 + 1, 21, neighbor_field);    // y in [3, 6]: wrong side; only its plane x = 3 is used
        const ScalarGrid3D south_grid = make_grid(seam, -7, -10, 30 + 14 + 1, 30 + 14 + 1, 21, neighbor_field);
        const FieldPlane west_plane = extract_field_plane(west_grid, 0, seam, seam, seam + 30);
        const FieldPlane south_plane = extract_field_plane(south_grid, 1, seam, seam, seam + 30);
        ScalarGrid3D c = make_grid(seam - 7, seam - 7, -10, 30 + 14 + 1, 30 + 14 + 1, 21, [](double x, double y, double z) { return z - surface(x, y) - 0.01; });
        const ScalarGrid3D c_before = c;
        blend_field_to_planes(&c, &west_plane, &south_plane, 1.0);
        double on_west = 0.0, on_south = 0.0;
        for (int z = 0; z < 21; z++) {
            for (int n = 0; n <= 30; n++) {
                on_west = std::max(on_west, std::fabs(c.at(7, 7 + n, z) - west_plane.at(seam + n, z - 10)));
                on_south = std::max(on_south, std::fabs(c.at(7 + n, 7, z) - south_plane.at(seam + n, z - 10)));
            }
        }
        check(on_west < 1.0e-12 && on_south < 1.0e-12, "at a corner each plane has exactly its neighbor's values, the corner column too");
        bool far = true;
        for (int z = 0; z < 21; z++) {
            for (int y = 0; y < 45; y++) {
                for (int x = 0; x < 45; x++) {
                    if (std::abs(x - 7) >= 10 && std::abs(y - 7) >= 10) {
                        far = far && c.at(x, y, z) == c_before.at(x, y, z);
                    }
                }
            }
        }
        check(far, "at a corner the field is not changed 1 m or more from both planes");
    }

    // Beyond the heights of the neighbor the correction is that of its highest and lowest node, so there is no step.
    {
        const ScalarGrid3D tall = make_grid(-7, -7, -10, 44, 45, 21, [](double x, double y, double z) { return z - surface(x, y); });
        const ScalarGrid3D low = make_grid(seam - 7, -7, -3, 45, 45, 7, [](double x, double y, double z) { return z - surface(x, y) + 0.02; });
        const FieldPlane short_plane = extract_field_plane(low, 0, seam, 0, 30);
        ScalarGrid3D taller = tall;
        blend_field_to_planes(&taller, &short_plane, nullptr, 1.0);
        const double at_edge = taller.at(7, 20, 10 - 3 + 6) - tall.at(7, 20, 10 - 3 + 6);   // the neighbor's highest node, z = 0.3
        const double above = taller.at(7, 20, 20) - tall.at(7, 20, 20);                     // far above it, z = 1.0
        check(std::fabs(above - at_edge) < 1.0e-12, "above the neighbor's grid the correction is that of its highest node");
    }

    // The neighbor's plane as boundary values: its nodes take its values and are marked, and no others.
    {
        ScalarGrid3D g = b_before;
        std::vector<unsigned char> fixed;
        const std::size_t count = fix_field_planes(&g, &west, nullptr, &fixed);
        std::size_t marked = 0;
        bool on_plane = true;
        bool values = true;
        for (int z = 0; z < static_cast<int>(g.nz); z++) {
            for (int y = 0; y < static_cast<int>(g.ny); y++) {
                for (int x = 0; x < static_cast<int>(g.nx); x++) {
                    const std::size_t at = g.index(x, y, z);
                    if (fixed[at] == 0) {
                        values = values && g.values[at] == b_before.values[at];
                        continue;
                    }
                    marked++;
                    on_plane = on_plane && x == 7;
                    if (west.covers(-7 + y, -10 + z)) {
                        values = values && g.values[at] == west.at(-7 + y, -10 + z);
                    }
                }
            }
        }
        check(count > 0 && count == marked, "the nodes of the plane are fixed, and counted");
        check(on_plane, "only nodes on the plane are fixed");
        check(values, "fixed nodes have the neighbor's values and the others are left alone");
        std::vector<unsigned char> none;
        ScalarGrid3D h = b_before;
        check(fix_field_planes(&h, nullptr, nullptr, &none) == 0 && h.values == b_before.values, "without planes nothing is fixed");
    }

    // No neighbors, and no width: nothing changes.
    {
        ScalarGrid3D untouched_grid = a;
        check(blend_field_to_planes(&untouched_grid, nullptr, nullptr, 1.0).nodes_changed == 0 && untouched_grid.values == a.values, "without planes nothing changes");
        check(blend_field_to_planes(&untouched_grid, &west, nullptr, 0.0).nodes_changed == 0, "with no width nothing changes");
    }

    // Welding: two meshes that meet on an edge, whose vertices there differ by rounding, become one surface.
    {
        Mesh mesh;
        mesh.vertices = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(1 + 2.0e-9, 1.0e-9, 0), Vec3(2, 0, 0), Vec3(0, 1, 3.0e-9), Vec3(5, 5, 5)};
        mesh.indices = {0, 1, 2, 3, 4, 5};
        mesh.normals.assign(mesh.vertices.size(), Vec3(0, 0, 1));
        const std::size_t merged = weld_coincident_vertices(&mesh, 1.0e-6);
        check(merged == 2 && mesh.vertices.size() == 5, "vertices that are within the tolerance of each other are merged");
        check(mesh.indices.size() == 6 && mesh.indices[3] == mesh.indices[1], "the triangles use the merged vertex");
        check(mesh.normals.size() == mesh.vertices.size() && std::fabs(mesh.normals[0].z) > 0.99, "the normals are made again for the merged vertices");

        Mesh far;
        far.vertices = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(1.0 + 1.0e-3, 0, 0), Vec3(2, 0, 0), Vec3(1, 1, 0)};
        far.indices = {0, 1, 2, 3, 4, 5};
        check(weld_coincident_vertices(&far, 1.0e-6) == 0 && far.vertices.size() == 6, "vertices a millimeter apart are not merged at a micrometer");

        Mesh collapsed;
        collapsed.vertices = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1.0 + 1.0e-9, 0, 0), Vec3(0, 1, 0)};
        collapsed.indices = {0, 1, 3, 1, 2, 3};   // the second triangle has two corners that become one
        weld_coincident_vertices(&collapsed, 1.0e-6);
        check(collapsed.indices.size() == 3, "a triangle with two corners that were merged is dropped");

        // Across a cell boundary of the tolerance grid.
        Mesh edge;
        edge.vertices = {Vec3(0.9999999e-6, 0, 0), Vec3(1.0000001e-6, 0, 0), Vec3(0, 1, 0)};
        edge.indices = {0, 1, 2};
        check(weld_coincident_vertices(&edge, 1.0e-6) == 1, "vertices on either side of a cell boundary are merged");
    }

    if (failures == 0) {
        std::printf("seam blending tests passed\n");
    }
    return failures == 0 ? 0 : 1;
}
