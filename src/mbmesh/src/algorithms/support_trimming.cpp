#include "algorithms/support_trimming.h"

#include "math/kdtree.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace {

constexpr std::size_t maximum_spacing_samples = 2048;
constexpr double automatic_support_radius_scale = 2.5;
constexpr double automatic_normal_offset_scale = 1.5;

enum class VertexSupportResult {
    supported,
    insufficient_neighbors,
    excessive_normal_offset,
    insufficient_normal_alignment,
};

double compact_quadratic_weight(double distance_squared, double radius_squared) {
    if (radius_squared <= 0.0 || distance_squared >= radius_squared) {
        return 0.0;
    }

    const double normalized_distance_squared = distance_squared / radius_squared;
    const double falloff = 1.0 - normalized_distance_squared;
    return falloff * falloff;
}

std::vector<Vec3> sample_positions(const OrientedPointCloud &oriented_points) {
    std::vector<Vec3> positions;
    positions.reserve(oriented_points.size());
    for (const OrientedPoint &oriented_point : oriented_points) {
        positions.push_back(oriented_point.point);
    }
    return positions;
}

double estimate_point_spacing(const std::vector<Vec3> &points,
                              const KDTree &tree,
                              std::size_t neighbor_rank) {
    if (points.size() < 2) {
        return 0.0;
    }

    neighbor_rank = std::max<std::size_t>(1, neighbor_rank);
    const std::size_t spacing_sample_count =
        std::min(points.size(), maximum_spacing_samples);

    std::vector<double> spacing_samples;
    spacing_samples.reserve(spacing_sample_count);

    for (std::size_t sample = 0; sample < spacing_sample_count; sample++) {
        const std::size_t point_index = spacing_sample_count == 1 ? 0 : sample * (points.size() - 1) / (spacing_sample_count - 1);

        const std::vector<KDTree::Neighbor> neighbors = tree.k_nearest(points[point_index], neighbor_rank, point_index);
        if (neighbors.empty()) {
            continue;
        }

        const double distance_squared = neighbors.back().distance_squared;
        if (distance_squared > vec3_epsilon * vec3_epsilon) {
            spacing_samples.push_back(std::sqrt(distance_squared));
        }
    }

    if (spacing_samples.empty()) {
        return 0.0;
    }

    const std::size_t median_index = spacing_samples.size() / 2;
    std::nth_element(spacing_samples.begin(),
                     spacing_samples.begin() + median_index,
                     spacing_samples.end());
    return spacing_samples[median_index];
}

std::vector<Vec3> calculate_vertex_normals(const Mesh &mesh) {
    std::vector<Vec3> normals(mesh.vertices.size(), Vec3(0.0));

    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const unsigned int a = mesh.indices[i];
        const unsigned int b = mesh.indices[i + 1];
        const unsigned int c = mesh.indices[i + 2];
        if (a >= mesh.vertices.size() ||
            b >= mesh.vertices.size() ||
            c >= mesh.vertices.size()) {
            continue;
        }

        const Vec3 face_normal = cross(mesh.vertices[b] - mesh.vertices[a], mesh.vertices[c] - mesh.vertices[a]);
        if (face_normal.length_squared() <= vec3_epsilon * vec3_epsilon) {
            continue;
        }

        normals[a] += face_normal;
        normals[b] += face_normal;
        normals[c] += face_normal;
    }

    for (Vec3 &normal : normals) {
        if (normal.length_squared() > vec3_epsilon * vec3_epsilon) {
            normal = normalize(normal);
        } else {
            normal = Vec3(0.0, 0.0, 1.0);
        }
    }

    return normals;
}

VertexSupportResult vertex_support_result(
    const Vec3 &vertex,
    const Vec3 &mesh_normal,
    const OrientedPointCloud &oriented_points,
    const KDTree &tree,
    double support_radius,
    double max_normal_offset,
    std::size_t minimum_neighbors,
    double minimum_normal_alignment) {
    const double support_radius_squared = support_radius * support_radius;
    const std::vector<KDTree::Neighbor> neighbors = tree.radius_search_squared(vertex, support_radius_squared);

    Vec3 weighted_normal(0.0);
    double weighted_normal_offset = 0.0;
    double accumulated_weight = 0.0;
    std::size_t contributing_neighbors = 0;

    for (const KDTree::Neighbor &neighbor : neighbors) {
        const OrientedPoint &sample = oriented_points[neighbor.index];
        if (sample.normal.length_squared() <= vec3_epsilon * vec3_epsilon) {
            continue;
        }

        const double weight = compact_quadratic_weight(neighbor.distance_squared, support_radius_squared);
        if (weight <= vec3_epsilon) {
            continue;
        }

        const Vec3 sample_normal = normalize(sample.normal);
        weighted_normal += sample_normal * weight;
        weighted_normal_offset += std::fabs(dot(vertex - sample.point, sample_normal)) * weight;
        accumulated_weight += weight;
        contributing_neighbors++;
    }

    if (contributing_neighbors < minimum_neighbors ||
        accumulated_weight <= vec3_epsilon) {
        return VertexSupportResult::insufficient_neighbors;
    }

    const double mean_normal_offset = weighted_normal_offset / accumulated_weight;
    if (mean_normal_offset > max_normal_offset) {
        return VertexSupportResult::excessive_normal_offset;
    }

    if (minimum_normal_alignment > 0.0) {
        if (weighted_normal.length_squared() <= vec3_epsilon * vec3_epsilon ||
            mesh_normal.length_squared() <= vec3_epsilon * vec3_epsilon) {
            return VertexSupportResult::insufficient_normal_alignment;
        }

        const double alignment = std::fabs(dot(normalize(mesh_normal), normalize(weighted_normal)));
        if (alignment < minimum_normal_alignment) {
            return VertexSupportResult::insufficient_normal_alignment;
        }
    }

    return VertexSupportResult::supported;
}

Mesh compact_supported_triangles(const Mesh &raw_mesh, const std::vector<bool> &supported_vertices) {
    Mesh trimmed_mesh;
    trimmed_mesh.indices.reserve(raw_mesh.indices.size());

    const unsigned int unmapped = std::numeric_limits<unsigned int>::max();
    std::vector<unsigned int> vertex_map(raw_mesh.vertices.size(), unmapped);

    for (std::size_t i = 0; i + 2 < raw_mesh.indices.size(); i += 3) {
        const unsigned int old_indices[3] = {
            raw_mesh.indices[i],
            raw_mesh.indices[i + 1],
            raw_mesh.indices[i + 2],
        };

        bool valid_triangle = true;
        for (const unsigned int old_index : old_indices) {
            if (old_index >= raw_mesh.vertices.size() || !supported_vertices[old_index]) {
                valid_triangle = false;
                break;
            }
        }
        if (!valid_triangle ||
            old_indices[0] == old_indices[1] ||
            old_indices[1] == old_indices[2] ||
            old_indices[2] == old_indices[0]) {
            continue;
        }

        const Vec3 face_normal = cross(raw_mesh.vertices[old_indices[1]] - raw_mesh.vertices[old_indices[0]],
                                       raw_mesh.vertices[old_indices[2]] - raw_mesh.vertices[old_indices[0]]);
        if (face_normal.length_squared() <= vec3_epsilon * vec3_epsilon) {
            continue;
        }

        for (const unsigned int old_index : old_indices) {
            if (vertex_map[old_index] == unmapped) {
                vertex_map[old_index] = static_cast<unsigned int>(trimmed_mesh.vertices.size());
                trimmed_mesh.vertices.push_back(raw_mesh.vertices[old_index]);
            }
            trimmed_mesh.indices.push_back(vertex_map[old_index]);
        }
    }

    trimmed_mesh.normals = calculate_vertex_normals(trimmed_mesh);
    return trimmed_mesh;
}

} // namespace

Mesh support_trimming(const Mesh &raw_mesh, const OrientedPointCloud &oriented_points, SupportTrimmingOptions options, SupportTrimmingDiagnostics *diagnostics) {
    if (diagnostics != nullptr) {
        *diagnostics = SupportTrimmingDiagnostics();
        diagnostics->input_vertices = raw_mesh.vertices.size();
        diagnostics->input_triangles = raw_mesh.indices.size() / 3;
    }

    if (!options.enabled) {
        if (diagnostics != nullptr) {
            diagnostics->supported_vertices = raw_mesh.vertices.size();
            diagnostics->output_vertices = raw_mesh.vertices.size();
            diagnostics->output_triangles = raw_mesh.indices.size() / 3;
        }
        return raw_mesh;
    }
    if (raw_mesh.vertices.empty() || raw_mesh.indices.empty() || oriented_points.empty()) {
        return Mesh();
    }

    const std::vector<Vec3> points = sample_positions(oriented_points);
    const KDTree tree(points);

    const std::size_t minimum_neighbors = static_cast<std::size_t>(std::max(1, options.minimum_neighbors));
    const bool needs_automatic_support_radius = !std::isfinite(options.support_radius) || options.support_radius <= 0.0;
    const bool needs_automatic_normal_offset = !std::isfinite(options.max_normal_offset) || options.max_normal_offset <= 0.0;

    double point_spacing = 0.0;
    if (needs_automatic_support_radius || needs_automatic_normal_offset) {
        point_spacing = estimate_point_spacing(points, tree, minimum_neighbors);
    }
    if (diagnostics != nullptr) {
        diagnostics->estimated_point_spacing = point_spacing;
    }

    const double support_radius = needs_automatic_support_radius ? automatic_support_radius_scale * point_spacing : options.support_radius;
    const double max_normal_offset = needs_automatic_normal_offset ? (point_spacing > vec3_epsilon ? automatic_normal_offset_scale * point_spacing : 0.5 * support_radius) : options.max_normal_offset;

    if (!std::isfinite(support_radius) || !std::isfinite(max_normal_offset) || support_radius <= vec3_epsilon || max_normal_offset <= 0.0) {
        return Mesh();
    }
    if (diagnostics != nullptr) {
        diagnostics->resolved_support_radius = support_radius;
        diagnostics->resolved_max_normal_offset = max_normal_offset;
    }

    const double minimum_normal_alignment = std::clamp(options.minimum_normal_alignment, 0.0, 1.0);
    const std::vector<Vec3> mesh_normals = raw_mesh.normals.size() == raw_mesh.vertices.size() ? raw_mesh.normals : calculate_vertex_normals(raw_mesh);

    std::vector<bool> supported_vertices(raw_mesh.vertices.size(), false);
    for (std::size_t i = 0; i < raw_mesh.vertices.size(); i++) {
        const VertexSupportResult result =
            vertex_support_result(raw_mesh.vertices[i],
                                  mesh_normals[i],
                                  oriented_points,
                                  tree,
                                  support_radius,
                                  max_normal_offset,
                                  minimum_neighbors,
                                  minimum_normal_alignment);
        supported_vertices[i] = result == VertexSupportResult::supported;

        if (diagnostics == nullptr) {
            continue;
        }

        switch (result) {
        case VertexSupportResult::supported:
            diagnostics->supported_vertices++;
            break;
        case VertexSupportResult::insufficient_neighbors:
            diagnostics->rejected_for_neighbors++;
            break;
        case VertexSupportResult::excessive_normal_offset:
            diagnostics->rejected_for_normal_offset++;
            break;
        case VertexSupportResult::insufficient_normal_alignment:
            diagnostics->rejected_for_normal_alignment++;
            break;
        }
    }

    Mesh trimmed_mesh = compact_supported_triangles(raw_mesh, supported_vertices);
    if (diagnostics != nullptr) {
        diagnostics->output_vertices = trimmed_mesh.vertices.size();
        diagnostics->output_triangles = trimmed_mesh.indices.size() / 3;
    }
    return trimmed_mesh;
}
