#include "algorithms/coverage_footprint.h"
#include "algorithms/support_trimming.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::printf("FAILED: %s\n", what);
        failures++;
    }
}

// Soundings on a 0.05 m grid over [0, 20] x [0, 10], except a hole of 2 m around (10, 5) and the strip x > 15.
bool has_sounding(double x, double y) {
    const double dx = x - 10.0;
    const double dy = y - 5.0;
    return !(dx * dx + dy * dy < 1.0) && !(x > 15.0 && y > 5.0);
}

} // namespace

int main() {
    // ---------------------------------------------------------------------------------------------- footprint
    CoverageFootprint footprint(0.0, 0.0, 20.0, 10.0, 0.1);
    for (double x = 0.0; x <= 20.0; x += 0.05) {
        for (double y = 0.0; y <= 10.0; y += 0.05) {
            if (has_sounding(x, y)) {
                footprint.add(x, y, 0.0);
            }
        }
    }

    check(footprint.has_sounding_near(3.0, 3.0, 0.0, 0.2), "a sounding is near a point in the surveyed area");
    check(footprint.has_sounding_near(10.0, 5.0 + 1.1, 0.0, 0.2), "a sounding is near a point just outside a void");
    check(!footprint.has_sounding_near(10.0, 5.0, 0.0, 0.2), "no sounding is near the middle of a void of 2 m across");
    check(footprint.has_sounding_near(10.0, 5.0, 0.0, 1.2), "a sounding is within 1.2 m of the middle of the void");
    check(!footprint.has_sounding_near(-1.0, 5.0, 0.0, 0.2), "no sounding is near a point beyond the edge of the survey");
    check(!footprint.has_sounding_near(-1000.0, 5.0, 0.0, 0.2), "no sounding is near a point far outside the raster");
    check(footprint.has_sounding_near(3.0, 3.0, 0.15, 0.2), "a point a little above the soundings is near them");
    check(!footprint.has_sounding_near(3.0, 3.0, -1.2, 0.5), "a point 1.2 m under the soundings is not near them");
    check(!footprint.has_sounding_near(3.0, 3.0, 1.2, 0.5), "a point 1.2 m over the soundings is not near them");
    check(footprint.cell_size() >= 0.1 && footprint.cell_count() > 0, "the raster has cells of the size asked for");

    // A raster that would have more cells than the limit has larger cells.
    CoverageFootprint coarse(0.0, 0.0, 20.0, 10.0, 0.01, 10000);
    check(coarse.cell_count() <= 10000 && coarse.cell_size() > 0.01, "the cells grow to keep the raster within the limit");

    // ----------------------------------------------------------------------------------- trimming with the footprint
    // A flat sheet of mesh over [0, 20] x [0, 10], with samples where has_sounding() says so, plus 3 m of mesh beyond the
    // edge at x = 20 that has no samples. The vertices of the hole and of the margin are both short of samples.
    Mesh mesh;
    const double step = 0.1;
    const int nx = 231;
    const int ny = 101;
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            mesh.vertices.push_back(Vec3(i * step, j * step, 0.0));
        }
    }
    for (int j = 0; j + 1 < ny; j++) {
        for (int i = 0; i + 1 < nx; i++) {
            const unsigned int a = j * nx + i;
            mesh.indices.insert(mesh.indices.end(), {a, a + 1, a + nx, a + 1, a + nx + 1, a + nx});
        }
    }
    OrientedPointCloud samples;
    for (double x = 0.0; x <= 20.0; x += 0.04) {
        for (double y = 0.0; y <= 10.0; y += 0.04) {
            if (x <= 15.0 || y <= 5.0 ? has_sounding(x, y) : false) {
                samples.emplace_back(Vec3(x, y, 0.0), Vec3(0.0, 0.0, 1.0));
            }
        }
    }

    SupportTrimmingOptions options;
    options.support_radius = 0.3;
    options.max_normal_offset = 0.1;
    options.minimum_neighbors = 3;

    SupportTrimmingDiagnostics without;
    const Mesh trimmed = support_trimming(mesh, samples, options, &without);

    options.footprint = &footprint;
    options.max_void_distance = 1.25;
    SupportTrimmingDiagnostics with;
    const Mesh filled = support_trimming(mesh, samples, options, &with);

    check(with.void_vertices_restored > 0, "the hole's vertices are restored");
    check(filled.indices.size() > trimmed.indices.size(), "the filled mesh has more triangles");
    check(with.void_distance_maximum <= 1.25 + 1.0e-9, "no restored vertex is farther than the limit from a sample");

    auto covers = [](const Mesh &m, double x, double y) {
        for (std::size_t t = 0; t + 2 < m.indices.size(); t += 3) {
            const Vec3 &a = m.vertices[m.indices[t]];
            const Vec3 &b = m.vertices[m.indices[t + 1]];
            const Vec3 &c = m.vertices[m.indices[t + 2]];
            const double d = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
            const double l1 = ((b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y)) / d;
            const double l2 = ((c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y)) / d;
            if (l1 >= 0.0 && l2 >= 0.0 && 1.0 - l1 - l2 >= 0.0) {
                return true;
            }
        }
        return false;
    };
    check(!covers(trimmed, 10.0, 5.0), "without the footprint the hole is a hole");
    check(covers(filled, 10.0, 5.0), "with the footprint the hole is filled");
    check(!covers(filled, 22.0, 5.0), "the mesh beyond the edge of the survey stays trimmed");

    // A sheet of mesh 1.2 m under the surveyed plane, inside the footprint seen from above and within the distance
    // limit of the samples above it, is not a void: the samples are along its normal, not beside it (away from the hole, where the
    // samples at its rim are beside it).
    Mesh two = mesh;
    const unsigned int base = static_cast<unsigned int>(two.vertices.size());
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            two.vertices.push_back(Vec3(i * step, j * step, -1.2));
        }
    }
    for (std::size_t t = 0; t < mesh.indices.size(); t++) {
        two.indices.push_back(mesh.indices[t] + base);
    }
    SupportTrimmingOptions sheet_options = options;
    sheet_options.footprint = &footprint;
    sheet_options.max_void_distance = 1.25;
    sheet_options.max_hole_area = 0.0;
    const Mesh with_sheet = support_trimming(two, samples, sheet_options, nullptr);
    std::size_t sheet_triangles = 0;
    for (std::size_t t = 0; t + 2 < with_sheet.indices.size(); t += 3) {
        if (with_sheet.vertices[with_sheet.indices[t]].z < -0.4 && with_sheet.vertices[with_sheet.indices[t]].x < 7.0) {
            sheet_triangles++;
        }
    }
    check(sheet_triangles == 0, "a sheet under the surveyed surface is not put back");
    check(covers(filled, 3.0, 3.0), "the surveyed surface is kept");

    // Holes inside the surface are put back without the footprint, if they are small: the hole of 2 m across (area
    // 3.1 m^2) is filled with a limit of 4 m^2 and not with one of 1 m^2, and the margin beyond the survey, which touches
    // the open edge of the mesh, is not.
    options.footprint = nullptr;
    options.max_void_distance = 0.0;
    options.max_hole_area = 4.0;
    SupportTrimmingDiagnostics holes;
    const Mesh closed = support_trimming(mesh, samples, options, &holes);
    check(holes.holes_restored > 0, "the hole is restored as a hole");
    check(covers(closed, 10.0, 5.0), "a hole inside the surface is filled");
    check(!covers(closed, 22.0, 5.0), "the margin at the open edge stays trimmed when holes are filled");
    options.max_hole_area = 1.0;
    const Mesh too_small = support_trimming(mesh, samples, options, nullptr);
    check(!covers(too_small, 10.0, 5.0), "a hole larger than the limit is not filled");


    // The near-data step: with only max_void_distance set (no footprint, no holes), a vertex that is short of neighbors
    // but has a sample within three support radii (0.9 m) is put back, and one with no sample that near is not. The
    // mesh 2 m beyond the edge of the samples at x = 20 stays trimmed.
    options.footprint = nullptr;
    options.max_hole_area = 0.0;
    options.max_void_distance = 1.25;
    SupportTrimmingDiagnostics near;
    const Mesh near_mesh = support_trimming(mesh, samples, options, &near);
    check(near.near_vertices_restored > 0, "vertices with a sample close by are put back");
    check(near_mesh.indices.size() > trimmed.indices.size(), "the near-data step adds triangles");
    check(!covers(near_mesh, 22.0, 5.0), "the mesh 2 m beyond the samples stays trimmed in the near-data step");
    options.max_void_distance = 0.0;
    SupportTrimmingDiagnostics no_step;
    const Mesh no_step_mesh = support_trimming(mesh, samples, options, &no_step);
    check(no_step.near_vertices_restored == 0 && no_step_mesh.indices.size() == trimmed.indices.size(), "without a distance limit there is no near-data step");

    // Island removal: a mesh of 4 m x 4 m with samples only in its middle 1 m x 1 m leaves the middle as a piece of
    // about 1 m^2, surrounded by trimmed mesh and away from the open edge of the mesh. A limit of 2 m^2 takes it out
    // and one of 0.1 m^2 keeps it.
    Mesh square;
    OrientedPointCloud middle;
    const int side = 41;
    for (int j = 0; j < side; j++) {
        for (int i = 0; i < side; i++) {
            square.vertices.push_back(Vec3(i * 0.1, j * 0.1, 0.0));
        }
    }
    for (int j = 0; j + 1 < side; j++) {
        for (int i = 0; i + 1 < side; i++) {
            const unsigned int a = j * side + i;
            square.indices.insert(square.indices.end(), {a, a + 1, a + side, a + 1, a + side + 1, a + side});
        }
    }
    for (double x = 1.5; x <= 2.5; x += 0.02) {
        for (double y = 1.5; y <= 2.5; y += 0.02) {
            middle.emplace_back(Vec3(x, y, 0.0), Vec3(0.0, 0.0, 1.0));
        }
    }
    SupportTrimmingOptions island_options;
    island_options.support_radius = 0.1;
    island_options.max_normal_offset = 0.05;
    island_options.minimum_neighbors = 3;
    SupportTrimmingDiagnostics keep_island;
    const Mesh with_island = support_trimming(square, middle, island_options, &keep_island);
    check(keep_island.islands_removed == 0 && covers(with_island, 2.0, 2.0), "without a limit the piece is kept");
    island_options.min_island_area = 0.1;
    SupportTrimmingDiagnostics smaller;
    const Mesh kept_island = support_trimming(square, middle, island_options, &smaller);
    check(smaller.islands_removed == 0 && covers(kept_island, 2.0, 2.0), "a piece larger than the limit is kept");
    island_options.min_island_area = 2.0;
    SupportTrimmingDiagnostics larger;
    const Mesh removed_island = support_trimming(square, middle, island_options, &larger);
    check(larger.islands_removed == 1 && removed_island.indices.empty(), "a piece smaller than the limit is taken out");
    island_options.threads = 4;
    SupportTrimmingDiagnostics threaded;
    const Mesh removed_threaded = support_trimming(square, middle, island_options, &threaded);
    check(threaded.islands_removed == larger.islands_removed && removed_threaded.indices == removed_island.indices &&
              removed_threaded.vertices.size() == removed_island.vertices.size(),
          "the threads do not change the result");

    if (failures == 0) {
        std::printf("coverage footprint tests passed\n");
    }
    // Pieces of a mesh not joined to the rest: one with soundings near it stays, one in the air goes, a large one stays.
    {
        CoverageFootprint ground(-10.0, -10.0, 10.0, 10.0, 0.1);
        for (double x = -9.0; x <= 9.0; x += 0.05) {
            for (double y = -9.0; y <= 9.0; y += 0.05) {
                ground.add(x, y, 0.0);
            }
        }
        const auto add_square = [](Mesh *mesh, double x, double y, double z, double side) {
            const unsigned int base = static_cast<unsigned int>(mesh->vertices.size());
            mesh->vertices.push_back(Vec3(x, y, z));
            mesh->vertices.push_back(Vec3(x + side, y, z));
            mesh->vertices.push_back(Vec3(x + side, y + side, z));
            mesh->vertices.push_back(Vec3(x, y + side, z));
            for (const unsigned int k : {0u, 1u, 2u, 0u, 2u, 3u}) {
                mesh->indices.push_back(base + k);
            }
        };
        Mesh pieces;
        add_square(&pieces, -8.0, -8.0, 0.0, 1.0);    // on the soundings
        add_square(&pieces, 0.0, 0.0, 2.0, 1.0);      // two meters above them
        add_square(&pieces, 3.0, 3.0, 2.0, 4.0);      // above them too, but larger than the limit
        PieceRemoval removed;
        remove_unsupported_pieces(&pieces, ground, 0.3, 2.0, &removed);
        check(removed.pieces == 1 && removed.triangles == 2 && removed.vertices == 4, "the small piece in the air is removed");
        check(pieces.indices.size() == 12 && pieces.vertices.size() == 8 && pieces.normals.size() == 8, "the supported and the large pieces stay");
    }

    return failures == 0 ? 0 : 1;
}
