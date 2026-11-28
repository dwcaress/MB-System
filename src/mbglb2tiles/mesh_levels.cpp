#include "mesh_levels.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>

void compute_vertex_normals(Mesh *mesh) {
    std::vector<Vec3> sums(mesh->vertices.size(), Vec3(0.0, 0.0, 0.0));
    for (std::size_t t = 0; t + 2 < mesh->indices.size(); t += 3) {
        const Vec3 &a = mesh->vertices[mesh->indices[t]];
        const Vec3 &b = mesh->vertices[mesh->indices[t + 1]];
        const Vec3 &c = mesh->vertices[mesh->indices[t + 2]];
        const double ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        const double vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        // The cross product has the length of twice the area, which is the weight.
        const Vec3 normal(uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx);
        for (int corner = 0; corner < 3; corner++) {
            Vec3 &sum = sums[mesh->indices[t + corner]];
            sum.x += normal.x;
            sum.y += normal.y;
            sum.z += normal.z;
        }
    }
    mesh->normals.resize(sums.size());
    for (std::size_t k = 0; k < sums.size(); k++) {
        const double length = std::sqrt(sums[k].x * sums[k].x + sums[k].y * sums[k].y + sums[k].z * sums[k].z);
        mesh->normals[k] = length > 0.0 ? Vec3(sums[k].x / length, sums[k].y / length, sums[k].z / length) : Vec3(0.0, 0.0, 1.0);
    }
}

double native_spacing(const Mesh &mesh) {
    double area = 0.0;
    for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
        const Vec3 &a = mesh.vertices[mesh.indices[t]];
        const Vec3 &b = mesh.vertices[mesh.indices[t + 1]];
        const Vec3 &c = mesh.vertices[mesh.indices[t + 2]];
        area += 0.5 * std::fabs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
    }
    if (mesh.vertices.empty() || !(area > 0.0)) {
        return 0.0;
    }
    return std::sqrt(area / static_cast<double>(mesh.vertices.size()));
}

bool cluster_mesh(const Mesh &mesh, const Vec3 &anchor, double cell_size, Mesh *result, std::string *error) {
    if (!(cell_size > 0.0)) {
        if (error != nullptr) {
            *error = "the cell size of the clustering is not more than zero";
        }
        return false;
    }
    constexpr std::int64_t bias = std::int64_t(1) << 20;
    constexpr std::int64_t limit = std::int64_t(1) << 21;
    const std::size_t count = mesh.vertices.size();
    std::vector<std::pair<std::uint64_t, std::uint32_t>> keyed(count);
    for (std::size_t n = 0; n < count; n++) {
        const Vec3 &v = mesh.vertices[n];
        const std::int64_t cell[3] = {static_cast<std::int64_t>(std::floor((v.x - anchor.x) / cell_size)) + bias,
                                      static_cast<std::int64_t>(std::floor((v.y - anchor.y) / cell_size)) + bias,
                                      static_cast<std::int64_t>(std::floor((v.z - anchor.z) / cell_size)) + bias};
        if (cell[0] < 0 || cell[0] >= limit || cell[1] < 0 || cell[1] >= limit || cell[2] < 0 || cell[2] >= limit) {
            if (error != nullptr) {
                *error = "the mesh has more than two million cells of " + std::to_string(cell_size) + " m along an axis";
            }
            return false;
        }
        keyed[n] = {(static_cast<std::uint64_t>(cell[0]) << 42) | (static_cast<std::uint64_t>(cell[1]) << 21) | static_cast<std::uint64_t>(cell[2]),
                    static_cast<std::uint32_t>(n)};
    }
    std::sort(keyed.begin(), keyed.end());

    std::vector<std::uint32_t> new_index(count);
    std::vector<Vec3> vertices;
    for (std::size_t k = 0; k < count;) {
        std::size_t end = k;
        double sx = 0.0, sy = 0.0, sz = 0.0;
        while (end < count && keyed[end].first == keyed[k].first) {
            const Vec3 &v = mesh.vertices[keyed[end].second];
            sx += v.x;
            sy += v.y;
            sz += v.z;
            new_index[keyed[end].second] = static_cast<std::uint32_t>(vertices.size());
            end++;
        }
        const double members = static_cast<double>(end - k);
        vertices.push_back(Vec3(sx / members, sy / members, sz / members));
        k = end;
    }
    keyed.clear();
    keyed.shrink_to_fit();

    // The triangles that keep three corners, each once: a repeat has the same three vertices, in whatever order.
    struct Corners {
        std::uint32_t sorted[3];
        std::uint32_t original[3];
    };
    std::vector<Corners> triangles;
    for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
        const std::uint32_t a = new_index[mesh.indices[t]], b = new_index[mesh.indices[t + 1]], c = new_index[mesh.indices[t + 2]];
        if (a == b || b == c || c == a) {
            continue;
        }
        Corners corners;
        corners.original[0] = a;
        corners.original[1] = b;
        corners.original[2] = c;
        std::copy(corners.original, corners.original + 3, corners.sorted);
        std::sort(corners.sorted, corners.sorted + 3);
        triangles.push_back(corners);
    }
    std::stable_sort(triangles.begin(), triangles.end(), [](const Corners &x, const Corners &y) {
        return std::lexicographical_compare(x.sorted, x.sorted + 3, y.sorted, y.sorted + 3);
    });
    result->vertices = std::move(vertices);
    result->normals.clear();
    result->indices.clear();
    result->indices.reserve(triangles.size() * 3);
    for (std::size_t t = 0; t < triangles.size(); t++) {
        if (t > 0 && std::equal(triangles[t].sorted, triangles[t].sorted + 3, triangles[t - 1].sorted)) {
            continue;
        }
        for (int corner = 0; corner < 3; corner++) {
            result->indices.push_back(triangles[t].original[corner]);
        }
    }
    return true;
}

std::vector<MeshTile> cut_mesh(const Mesh &mesh, double x0, double y0, double tile_size) {
    std::map<std::pair<int, int>, std::vector<std::uint32_t>> groups;
    for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
        const Vec3 &a = mesh.vertices[mesh.indices[t]];
        const Vec3 &b = mesh.vertices[mesh.indices[t + 1]];
        const Vec3 &c = mesh.vertices[mesh.indices[t + 2]];
        const int i = static_cast<int>(std::floor(((a.x + b.x + c.x) / 3.0 - x0) / tile_size));
        const int j = static_cast<int>(std::floor(((a.y + b.y + c.y) / 3.0 - y0) / tile_size));
        groups[{i, j}].push_back(static_cast<std::uint32_t>(t / 3));
    }
    const bool has_normals = mesh.normals.size() == mesh.vertices.size();
    std::vector<std::uint32_t> local(mesh.vertices.size(), std::numeric_limits<std::uint32_t>::max());
    std::vector<std::uint32_t> touched;
    std::vector<MeshTile> tiles;
    tiles.reserve(groups.size());
    for (const auto &group : groups) {
        MeshTile tile;
        tile.i = group.first.first;
        tile.j = group.first.second;
        touched.clear();
        for (const std::uint32_t triangle : group.second) {
            for (int corner = 0; corner < 3; corner++) {
                const std::uint32_t vertex = mesh.indices[3 * static_cast<std::size_t>(triangle) + corner];
                if (local[vertex] == std::numeric_limits<std::uint32_t>::max()) {
                    local[vertex] = static_cast<std::uint32_t>(touched.size());
                    touched.push_back(vertex);
                }
                tile.mesh.indices.push_back(local[vertex]);
            }
        }
        for (const std::uint32_t vertex : touched) {
            tile.mesh.vertices.push_back(mesh.vertices[vertex]);
            if (has_normals) {
                tile.mesh.normals.push_back(mesh.normals[vertex]);
            }
            local[vertex] = std::numeric_limits<std::uint32_t>::max();
        }
        tiles.push_back(std::move(tile));
    }
    return tiles;
}
