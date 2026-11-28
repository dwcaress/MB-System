#include "algorithms/seam_blending.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

namespace {

constexpr double pi = 3.14159265358979323846;

long lattice_index(double position, double cell_size) {
    return std::lround(position / cell_size);
}

// 1 on the plane, 0 at width from it, smooth between.
double falloff(double distance, double width) {
    if (distance >= width) {
        return 0.0;
    }
    const double c = std::cos(0.5 * pi * distance / width);
    return c * c;
}

// What has to be added to the field of the grid, on the nodes of its plane that is the plane of the neighbor, to make it the
// neighbor's field: rows of the nodes along the plane (y for the plane x = constant, x for y = constant), each a column of
// heights. A node that the neighbor's plane does not cover takes the correction of the nearest one that it does.
struct PlaneCorrection {
    std::size_t rows = 0;
    std::size_t heights = 0;
    std::vector<double> value;
    bool valid = false;

    [[nodiscard]] double at(std::size_t row, std::size_t height) const { return value[row * heights + height]; }
};

PlaneCorrection plane_correction(const ScalarGrid3D &grid, const FieldPlane &plane, int axis, long grid_plane, long row_lattice_origin, long z_lattice_origin) {
    PlaneCorrection correction;
    correction.rows = axis == 0 ? grid.ny : grid.nx;
    correction.heights = grid.nz;
    correction.value.assign(correction.rows * correction.heights, 0.0);
    std::vector<char> known(correction.value.size(), 0);

    std::vector<std::size_t> row_first(correction.rows, correction.heights);
    std::vector<std::size_t> row_last(correction.rows, 0);
    for (std::size_t row = 0; row < correction.rows; row++) {
        for (std::size_t height = 0; height < correction.heights; height++) {
            if (!plane.covers(row_lattice_origin + static_cast<long>(row), z_lattice_origin + static_cast<long>(height))) {
                continue;
            }
            const int x = axis == 0 ? static_cast<int>(grid_plane) : static_cast<int>(row);
            const int y = axis == 0 ? static_cast<int>(row) : static_cast<int>(grid_plane);
            correction.value[row * correction.heights + height] =
                plane.at(row_lattice_origin + static_cast<long>(row), z_lattice_origin + static_cast<long>(height)) - grid.at(x, y, static_cast<int>(height));
            known[row * correction.heights + height] = 1;
            row_first[row] = std::min(row_first[row], height);
            row_last[row] = std::max(row_last[row], height);
        }
    }

    // Above and below the heights of the neighbor, the correction at its highest and lowest node.
    for (std::size_t row = 0; row < correction.rows; row++) {
        if (row_first[row] >= correction.heights) {
            continue;
        }
        for (std::size_t height = 0; height < row_first[row]; height++) {
            correction.value[row * correction.heights + height] = correction.value[row * correction.heights + row_first[row]];
        }
        for (std::size_t height = row_last[row] + 1; height < correction.heights; height++) {
            correction.value[row * correction.heights + height] = correction.value[row * correction.heights + row_last[row]];
        }
    }

    // Beyond the extent of the neighbor along the plane, the row of the nearest node that it covers.
    std::vector<std::size_t> with_data;
    for (std::size_t row = 0; row < correction.rows; row++) {
        if (row_first[row] < correction.heights) {
            with_data.push_back(row);
        }
    }
    if (with_data.empty()) {
        return correction;
    }
    for (std::size_t row = 0; row < correction.rows; row++) {
        if (row_first[row] < correction.heights) {
            continue;
        }
        std::size_t nearest = with_data.front();
        for (const std::size_t candidate : with_data) {
            const std::size_t d_candidate = candidate > row ? candidate - row : row - candidate;
            const std::size_t d_nearest = nearest > row ? nearest - row : row - nearest;
            if (d_candidate < d_nearest) {
                nearest = candidate;
            }
        }
        for (std::size_t height = 0; height < correction.heights; height++) {
            correction.value[row * correction.heights + height] = correction.value[nearest * correction.heights + height];
        }
    }
    correction.valid = true;
    return correction;
}

} // namespace

FieldPlane extract_field_plane(const ScalarGrid3D &grid, int axis, long plane_index, long first_u, long last_u) {
    FieldPlane plane;
    if (grid.nx < 2 || grid.ny < 2 || grid.nz < 2 || grid.cell_size <= 0.0 || last_u < first_u) {
        return plane;
    }
    const double cell = grid.cell_size;
    const long x0 = lattice_index(grid.origin.x, cell);
    const long y0 = lattice_index(grid.origin.y, cell);
    const long z0 = lattice_index(grid.origin.z, cell);
    const long along_origin = axis == 0 ? x0 : y0;   // the index of the plane in the grid
    const long across_origin = axis == 0 ? y0 : x0;
    const long along_count = static_cast<long>(axis == 0 ? grid.nx : grid.ny);
    const long across_count = static_cast<long>(axis == 0 ? grid.ny : grid.nx);

    const long plane_node = plane_index - along_origin;
    if (plane_node < 0 || plane_node >= along_count) {
        return plane;
    }
    const long u_first = std::max(first_u, across_origin);
    const long u_last = std::min(last_u, across_origin + across_count - 1);
    if (u_last < u_first) {
        return plane;
    }

    plane.axis = axis;
    plane.plane_index = plane_index;
    plane.first_u = u_first;
    plane.count_u = u_last - u_first + 1;
    plane.first_z = z0;
    plane.count_z = static_cast<long>(grid.nz);
    plane.values.resize(static_cast<std::size_t>(plane.count_u) * static_cast<std::size_t>(plane.count_z));
    for (long z = 0; z < plane.count_z; z++) {
        for (long u = 0; u < plane.count_u; u++) {
            const long across = u_first + u - across_origin;
            const int x = static_cast<int>(axis == 0 ? plane_node : across);
            const int y = static_cast<int>(axis == 0 ? across : plane_node);
            plane.values[static_cast<std::size_t>(z) * static_cast<std::size_t>(plane.count_u) + static_cast<std::size_t>(u)] = grid.at(x, y, static_cast<int>(z));
        }
    }
    return plane;
}

SeamBlendResult blend_field_to_planes(ScalarGrid3D *grid, const FieldPlane *west, const FieldPlane *south, double width) {
    SeamBlendResult result;
    if (grid == nullptr || grid->nx < 2 || grid->ny < 2 || grid->nz < 2 || grid->cell_size <= 0.0 || !(width > 0.0)) {
        return result;
    }
    const double cell = grid->cell_size;
    const long x0 = lattice_index(grid->origin.x, cell);
    const long y0 = lattice_index(grid->origin.y, cell);
    const long z0 = lattice_index(grid->origin.z, cell);

    // The nodes of the planes in the grid, and what each needs added.
    long west_node = 0;
    long south_node = 0;
    PlaneCorrection west_correction;
    PlaneCorrection south_correction;
    if (west != nullptr && !west->empty() && west->axis == 0) {
        west_node = west->plane_index - x0;
        if (west_node >= 0 && west_node < static_cast<long>(grid->nx)) {
            west_correction = plane_correction(*grid, *west, 0, west_node, y0, z0);
        }
    }
    if (south != nullptr && !south->empty() && south->axis == 1) {
        south_node = south->plane_index - y0;
        if (south_node >= 0 && south_node < static_cast<long>(grid->ny)) {
            south_correction = plane_correction(*grid, *south, 1, south_node, x0, z0);
        }
    }
    const bool use_west = west_correction.valid;
    const bool use_south = south_correction.valid;
    if (!use_west && !use_south) {
        return result;
    }

    // The correction is the sum of the two, each falling off from its own plane, and where both are not zero the
    // correction of the south plane has the value that the west plane has at their meeting taken off, so that each plane
    // keeps exactly its own values: on the west plane the west correction has weight one and the south term is zero.
    for (long z = 0; z < static_cast<long>(grid->nz); z++) {
        for (long y = 0; y < static_cast<long>(grid->ny); y++) {
            const double south_weight = use_south ? falloff(std::fabs(static_cast<double>(y - south_node)) * cell, width) : 0.0;
            for (long x = 0; x < static_cast<long>(grid->nx); x++) {
                const double west_weight = use_west ? falloff(std::fabs(static_cast<double>(x - west_node)) * cell, width) : 0.0;
                if (west_weight == 0.0 && south_weight == 0.0) {
                    continue;
                }
                double delta = 0.0;
                if (west_weight > 0.0) {
                    delta += west_weight * west_correction.at(static_cast<std::size_t>(y), static_cast<std::size_t>(z));
                }
                if (south_weight > 0.0) {
                    double south_term = south_correction.at(static_cast<std::size_t>(x), static_cast<std::size_t>(z));
                    if (west_weight > 0.0) {
                        south_term -= west_weight * south_correction.at(static_cast<std::size_t>(west_node), static_cast<std::size_t>(z));
                    }
                    delta += south_weight * south_term;
                }
                grid->values[grid->index(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z))] += delta;
                result.nodes_changed++;
                result.largest_correction = std::max(result.largest_correction, std::fabs(delta));
            }
        }
    }
    return result;
}

std::size_t fix_field_planes(ScalarGrid3D *grid, const FieldPlane *west, const FieldPlane *south, std::vector<unsigned char> *fixed) {
    if (grid == nullptr || fixed == nullptr || grid->nx < 2 || grid->ny < 2 || grid->nz < 2 || grid->cell_size <= 0.0) {
        return 0;
    }
    const double cell = grid->cell_size;
    const long x0 = lattice_index(grid->origin.x, cell);
    const long y0 = lattice_index(grid->origin.y, cell);
    const long z0 = lattice_index(grid->origin.z, cell);
    if (fixed->size() != grid->values.size()) {
        fixed->assign(grid->values.size(), 0);
    }
    std::size_t count = 0;
    const auto apply = [&](const FieldPlane *plane, int axis, long origin_along, long origin_across, std::size_t along_count) {
        if (plane == nullptr || plane->empty() || plane->axis != axis) {
            return;
        }
        const long node = plane->plane_index - origin_along;
        if (node < 0 || node >= static_cast<long>(along_count)) {
            return;
        }
        const PlaneCorrection correction = plane_correction(*grid, *plane, axis, node, origin_across, z0);
        if (!correction.valid) {
            return;
        }
        for (std::size_t height = 0; height < correction.heights; height++) {
            for (std::size_t row = 0; row < correction.rows; row++) {
                const int x = static_cast<int>(axis == 0 ? node : static_cast<long>(row));
                const int y = static_cast<int>(axis == 0 ? static_cast<long>(row) : node);
                const std::size_t at = grid->index(x, y, static_cast<int>(height));
                if ((*fixed)[at] != 0) {
                    continue;
                }
                grid->values[at] += correction.at(row, height);   // the correction is the plane's value minus the grid's
                (*fixed)[at] = 1;
                count++;
            }
        }
    };
    apply(west, 0, x0, y0, grid->nx);
    apply(south, 1, y0, x0, grid->ny);
    return count;
}

namespace {

struct WeldCell {
    long long x, y, z;
    bool operator==(const WeldCell &other) const { return x == other.x && y == other.y && z == other.z; }
};

struct WeldCellHash {
    std::size_t operator()(const WeldCell &cell) const {
        std::size_t h = std::hash<long long>()(cell.x);
        h = h * 1000003u ^ std::hash<long long>()(cell.y);
        h = h * 1000003u ^ std::hash<long long>()(cell.z);
        return h;
    }
};

} // namespace

std::size_t weld_coincident_vertices(Mesh *mesh, double tolerance) {
    if (mesh == nullptr || mesh->vertices.empty() || !(tolerance > 0.0)) {
        return 0;
    }
    const std::size_t count = mesh->vertices.size();
    const bool had_normals = mesh->normals.size() == count;
    const double tolerance_squared = tolerance * tolerance;

    // The first vertex found in each cell of the size of the tolerance, and, for each vertex, the one it is merged into.
    std::unordered_map<WeldCell, std::vector<unsigned int>, WeldCellHash> cells;
    cells.reserve(count);
    std::vector<unsigned int> merged_into(count);
    std::vector<unsigned int> new_index(count);
    std::vector<Vec3> vertices;
    vertices.reserve(count);
    std::size_t merged = 0;
    for (std::size_t n = 0; n < count; n++) {
        const Vec3 &v = mesh->vertices[n];
        const WeldCell cell{static_cast<long long>(std::floor(v.x / tolerance)), static_cast<long long>(std::floor(v.y / tolerance)),
                            static_cast<long long>(std::floor(v.z / tolerance))};
        bool found = false;
        for (long long dx = -1; dx <= 1 && !found; dx++) {
            for (long long dy = -1; dy <= 1 && !found; dy++) {
                for (long long dz = -1; dz <= 1 && !found; dz++) {
                    const auto near = cells.find(WeldCell{cell.x + dx, cell.y + dy, cell.z + dz});
                    if (near == cells.end()) {
                        continue;
                    }
                    for (const unsigned int other : near->second) {
                        const Vec3 &o = mesh->vertices[other];
                        const double ex = v.x - o.x, ey = v.y - o.y, ez = v.z - o.z;
                        if (ex * ex + ey * ey + ez * ez <= tolerance_squared) {
                            merged_into[n] = other;
                            new_index[n] = new_index[other];
                            found = true;
                            merged++;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            merged_into[n] = static_cast<unsigned int>(n);
            new_index[n] = static_cast<unsigned int>(vertices.size());
            vertices.push_back(v);
            cells[cell].push_back(static_cast<unsigned int>(n));
        }
    }
    if (merged == 0) {
        return 0;
    }

    std::vector<unsigned int> indices;
    indices.reserve(mesh->indices.size());
    for (std::size_t t = 0; t + 2 < mesh->indices.size(); t += 3) {
        const unsigned int a = new_index[mesh->indices[t]];
        const unsigned int b = new_index[mesh->indices[t + 1]];
        const unsigned int c = new_index[mesh->indices[t + 2]];
        if (a == b || b == c || c == a) {
            continue;
        }
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }
    mesh->vertices = std::move(vertices);
    mesh->indices = std::move(indices);
    mesh->normals.clear();
    if (had_normals) {
        mesh->normals.assign(mesh->vertices.size(), Vec3(0.0));
        for (std::size_t t = 0; t + 2 < mesh->indices.size(); t += 3) {
            const unsigned int a = mesh->indices[t], b = mesh->indices[t + 1], c = mesh->indices[t + 2];
            const Vec3 face = cross(mesh->vertices[b] - mesh->vertices[a], mesh->vertices[c] - mesh->vertices[a]);
            mesh->normals[a] += face;
            mesh->normals[b] += face;
            mesh->normals[c] += face;
        }
        for (Vec3 &normal : mesh->normals) {
            normal = normal.length_squared() > 1.0e-20 ? normalize(normal) : Vec3(0.0, 0.0, 1.0);
        }
    }
    return merged;
}
