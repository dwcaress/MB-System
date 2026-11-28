#include "glb_reader.h"
#include "mesh_levels.h"

#include "algorithms/seam_blending.h"
#include "io/coordinate_frame.h"
#include "io/glb_writer.h"
#include "io/provenance.h"
#include "io/tileset_writer.h"

#include "draco_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::printf("FAILED: %s\n", what);
        failures++;
    }
}

// A square surface of n x n vertices h meters apart, flat on the left half and a smooth hill on the right, triangulated as a grid.
Mesh make_surface(int n, double h) {
    Mesh mesh;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            const double x = h * i, y = h * j;
            const double hill = x > 0.5 * h * n ? 0.5 * std::sin(0.3 * (x - 0.5 * h * n)) * std::cos(0.25 * y) : 0.0;
            mesh.vertices.push_back(Vec3(x, y, hill));
        }
    }
    for (int j = 0; j + 1 < n; j++) {
        for (int i = 0; i + 1 < n; i++) {
            const unsigned int a = j * n + i, b = a + 1, c = a + n, d = c + 1;
            for (const unsigned int k : {a, b, d, a, d, c}) {
                mesh.indices.push_back(k);
            }
        }
    }
    return mesh;
}

} // namespace

int main() {
    const int n = 129;
    const double h = 0.1;
    Mesh mesh = make_surface(n, h);

    check(std::fabs(native_spacing(mesh) - h) < 0.002, "the spacing of a grid is its step");
    compute_vertex_normals(&mesh);
    check(mesh.normals.size() == mesh.vertices.size() && std::fabs(mesh.normals[10 * n + 10].z - 1.0) < 1e-12, "the normal of a flat part points up");
    check(mesh.normals[10 * n + 100].z < 1.0 && mesh.normals[10 * n + 100].z > 0.5, "and that of a slope leans");

    // Coarsening by twice the step takes about a quarter of the vertices and keeps the flat part flat, with no degenerate or repeated triangle.
    Mesh coarse;
    std::string error;
    check(cluster_mesh(mesh, Vec3(0.0, 0.0, 0.0), 2.0 * h, &coarse, &error), "clustering works");
    check(coarse.vertices.size() < mesh.vertices.size() / 3 && coarse.vertices.size() > mesh.vertices.size() / 6, "twice the cell is about a quarter of the vertices");
    check(coarse.indices.size() < mesh.indices.size() / 3 && coarse.indices.size() % 3 == 0, "and of the triangles");
    bool flat = true, valid = true;
    std::set<std::tuple<unsigned, unsigned, unsigned>> seen;
    for (std::size_t t = 0; t + 2 < coarse.indices.size(); t += 3) {
        const unsigned a = coarse.indices[t], b = coarse.indices[t + 1], c = coarse.indices[t + 2];
        valid = valid && a != b && b != c && c != a;
        std::vector<unsigned> sorted = {a, b, c};
        std::sort(sorted.begin(), sorted.end());
        valid = valid && seen.insert({sorted[0], sorted[1], sorted[2]}).second;
    }
    for (const Vec3 &vertex : coarse.vertices) {
        if (vertex.x < 0.4 * h * n) {
            flat = flat && std::fabs(vertex.z) < 1e-12;
        }
    }
    check(valid, "no triangle has two corners the same and none is repeated");
    check(flat, "the flat part stays flat");

    // The tiles of a mesh meet exactly: cutting the clustered mesh gives vertices on a shared edge that are the same points with the same normals,
    // and together the tiles hold every triangle once.
    compute_vertex_normals(&coarse);
    const std::vector<MeshTile> tiles = cut_mesh(coarse, 0.0, 0.0, 3.2);
    check(tiles.size() == 16, "a 12.8 m square cut at 3.2 m is 16 tiles");
    std::size_t triangles = 0;
    std::set<std::tuple<double, double, double>> positions;
    std::size_t vertex_copies = 0;
    bool normals_agree = true;
    std::vector<std::pair<std::tuple<double, double, double>, Vec3>> first_normal;
    for (const MeshTile &tile : tiles) {
        triangles += tile.mesh.indices.size() / 3;
        vertex_copies += tile.mesh.vertices.size();
        for (std::size_t k = 0; k < tile.mesh.vertices.size(); k++) {
            const Vec3 &v = tile.mesh.vertices[k];
            positions.insert({v.x, v.y, v.z});
            first_normal.push_back({{v.x, v.y, v.z}, tile.mesh.normals[k]});
        }
    }
    std::sort(first_normal.begin(), first_normal.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    for (std::size_t k = 1; k < first_normal.size(); k++) {
        if (first_normal[k].first == first_normal[k - 1].first) {
            normals_agree = normals_agree && std::fabs(first_normal[k].second.z - first_normal[k - 1].second.z) < 1e-12;
        }
    }
    check(triangles == coarse.indices.size() / 3, "the tiles hold every triangle once");
    check(positions.size() <= coarse.vertices.size() && vertex_copies > positions.size(), "the vertices on the edges are in two tiles, at the same points");
    check(normals_agree, "with the same normals");

    // A GLB of mbmesh's writer, as mbglb2tiles reads it: many primitives, relative to a center, Draco-compressed.
    const std::filesystem::path directory = std::filesystem::temp_directory_path() / "mbglb2tiles_test";
    std::filesystem::create_directories(directory);
    const CoordinateFrame frame = coordinate_frame_from_origin(GeodeticPosition{-122.3, 36.39, -850.0});
    RunInfo run;
    run.program = "mbmesh";
    run.has_center = true;
    run.center_x = frame.origin_ecef.x;
    run.center_y = frame.origin_ecef.y;
    run.center_z = frame.origin_ecef.z;
    run.center_longitude = frame.origin.longitude;
    run.center_latitude = frame.origin.latitude;
    run.center_height = frame.origin.elevation;
    set_glb_run_metadata(run_metadata(run));
    set_glb_compression(mbdraco::Settings());
    Mesh ecef_offsets;
    for (const Vec3 &vertex : mesh.vertices) {
        ecef_offsets.vertices.push_back(local_to_ecef(vertex, frame) - frame.origin_ecef);
    }
    ecef_offsets.indices = mesh.indices;
    const std::vector<std::uint32_t> groups = triangle_blocks(mesh, 3.2);
    check(write_mesh_glb_file(directory / "in.glb", ecef_offsets, &error, GlbContent{"mesh", "test"}, &groups), "writes a test GLB");
    GlbMesh read;
    check(read_mesh_glb(directory / "in.glb", &read, &error), "reads it");
    check(read.has_center && std::fabs(read.center.x - frame.origin_ecef.x) < 1e-6 && read.primitives == 16 && read.draco_primitives == (mbdraco::available() ? 16 : 0),
          "a center, and 16 primitives, Draco ones if this build has Draco");
    check(read.mesh.indices.size() == mesh.indices.size() && read.mesh.vertices.size() > mesh.vertices.size(), "all the triangles, with the vertices on the edges twice");
    check(mbdraco::available() ? (read.position_step > 0.0 && read.position_step < 0.002) : read.position_step == 0.0, "and the quantization step of the file, none without Draco");
    // The vertices come back to within the step of the originals once put in the local frame again, and welding makes the surface one again.
    for (Vec3 &vertex : read.mesh.vertices) {
        vertex = ecef_to_local(vertex + read.center, frame);
    }
    const std::size_t before = read.mesh.vertices.size();
    const std::size_t welded = weld_coincident_vertices(&read.mesh, read.position_step > 0.0 ? 2.0 * read.position_step : 0.001);
    check(welded > 0 && read.mesh.vertices.size() < before && read.mesh.vertices.size() == mesh.vertices.size(), "welding gives back the vertices of the surface");

    // A GLB that is not from the programs is refused, and so is a missing one.
    set_glb_run_metadata(MetaValue());
    clear_glb_run_metadata();
    check(write_mesh_glb_file(directory / "plain.glb", ecef_offsets, &error), "writes a GLB without extras");
    check(!read_mesh_glb(directory / "plain.glb", &read, &error) && error.find("georeference") != std::string::npos, "which has no georeference, so is refused");
    check(!read_mesh_glb(directory / "missing.glb", &read, &error), "a missing file is an error");

    // The tree of a tileset: transform on the root, REPLACE, children nested.
    TilesetNode root;
    root.uri = "L1/tile_0_0.glb";
    root.minimum[0] = root.minimum[1] = root.minimum[2] = -10.0;
    root.maximum[0] = root.maximum[1] = root.maximum[2] = 10.0;
    root.geometric_error = 0.8;
    TilesetNode child;
    child.uri = "L0/tile_0_0.glb";
    child.minimum[0] = child.minimum[1] = child.minimum[2] = -10.0;
    child.maximum[0] = child.maximum[1] = child.maximum[2] = 0.0;
    root.children.push_back(child);
    check(write_tileset_tree_json(directory / "tileset.json", frame, root, MetaValue(), "test", &error), "writes a tileset tree");
    std::ifstream json_file(directory / "tileset.json");
    const std::string json((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
    check(json.find("\"refine\": \"REPLACE\"") != std::string::npos && json.find("\"transform\"") != std::string::npos && json.find("\"children\"") != std::string::npos &&
              json.find("L0/tile_0_0.glb") != std::string::npos && json.find("\"version\": \"1.1\"") != std::string::npos,
          "the tree has the transform, REPLACE, and the child");

    std::filesystem::remove_all(directory);
    std::printf("levels: %zu vertices, %zu coarse, %zu tiles\n", mesh.vertices.size(), coarse.vertices.size(), tiles.size());
    return failures == 0 ? 0 : 1;
}
