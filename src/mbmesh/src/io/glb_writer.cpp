#include "io/glb_writer.h"

#include "draco_mesh.h"
#include "haxby_colors.h"
#include "util/logger.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

namespace {

void set_error(std::string *error, const std::string &message) {
    if (error != nullptr) {
        *error = message;
    }
}

[[nodiscard]] bool is_finite_vec3(const Vec3 &value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

[[nodiscard]] bool fits_float(double value) {
    constexpr double max_float = static_cast<double>(std::numeric_limits<float>::max());
    return std::isfinite(value) && value >= -max_float && value <= max_float;
}

[[nodiscard]] bool fits_float_vec3(const Vec3 &value) {
    return fits_float(value.x) && fits_float(value.y) && fits_float(value.z);
}

[[nodiscard]] bool validate_mesh(const Mesh &mesh, std::string *error) {
    if (mesh.vertices.empty()) {
        set_error(error, "Mesh has no vertices");
        return false;
    }

    if (mesh.indices.empty()) {
        set_error(error, "Mesh has no triangle indices");
        return false;
    }

    if (mesh.indices.size() % 3 != 0) {
        set_error(error, "Mesh index count is not divisible by 3");
        return false;
    }

    if (!mesh.normals.empty() && mesh.normals.size() != mesh.vertices.size()) {
        set_error(error, "Mesh normal count does not match vertex count");
        return false;
    }

    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        if (!is_finite_vec3(mesh.vertices[i])) {
            set_error(error, "Mesh contains non-finite vertex at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(mesh.vertices[i])) {
            set_error(error, "Mesh vertex is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    for (std::size_t i = 0; i < mesh.normals.size(); ++i) {
        if (!is_finite_vec3(mesh.normals[i])) {
            set_error(error, "Mesh contains non-finite normal at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(mesh.normals[i])) {
            set_error(error, "Mesh normal is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    for (const unsigned int index : mesh.indices) {
        if (index >= mesh.vertices.size()) {
            set_error(error, "Mesh contains an out-of-range vertex index");
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool validate_pointcloud(const PointCloud &pointcloud, std::string *error) {
    if (pointcloud.empty()) {
        set_error(error, "Point cloud has no points");
        return false;
    }

    for (std::size_t i = 0; i < pointcloud.size(); ++i) {
        if (!is_finite_vec3(pointcloud[i])) {
            set_error(error, "Point cloud contains non-finite point at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(pointcloud[i])) {
            set_error(error, "Point cloud point is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool validate_collected_pointcloud(const CollectedPointCloud &pointcloud, std::string *error) {
    if (pointcloud.empty()) {
        set_error(error, "Collected point cloud has no points");
        return false;
    }

    for (std::size_t i = 0; i < pointcloud.size(); ++i) {
        if (!is_finite_vec3(pointcloud[i].point)) {
            set_error(error, "Collected point cloud contains non-finite point at index " + std::to_string(i));
            return false;
        }
        if (!is_finite_vec3(pointcloud[i].origin)) {
            set_error(error, "Collected point cloud contains non-finite origin at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(pointcloud[i].point)) {
            set_error(error, "Collected point cloud point is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(pointcloud[i].origin)) {
            set_error(error, "Collected point cloud origin is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool validate_oriented_pointcloud(const OrientedPointCloud &pointcloud, std::string *error) {
    if (pointcloud.empty()) {
        set_error(error, "Oriented point cloud has no points");
        return false;
    }

    for (std::size_t i = 0; i < pointcloud.size(); ++i) {
        if (!is_finite_vec3(pointcloud[i].point)) {
            set_error(error, "Oriented point cloud contains non-finite point at index " + std::to_string(i));
            return false;
        }
        if (!is_finite_vec3(pointcloud[i].normal)) {
            set_error(error, "Oriented point cloud contains non-finite normal at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(pointcloud[i].point)) {
            set_error(error, "Oriented point cloud point is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
        if (!fits_float_vec3(pointcloud[i].normal)) {
            set_error(error, "Oriented point cloud normal is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    return true;
}

[[nodiscard]] std::array<double, 3> vec3_min_values(const std::vector<Vec3> &values) {
    std::array<double, 3> result{ std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };

    for (const Vec3 &value : values) {
        result[0] = std::min(result[0], value.x);
        result[1] = std::min(result[1], value.y);
        result[2] = std::min(result[2], value.z);
    }

    return result;
}

[[nodiscard]] std::array<double, 3> vec3_max_values(const std::vector<Vec3> &values) {
    std::array<double, 3> result{ std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };

    for (const Vec3 &value : values) {
        result[0] = std::max(result[0], value.x);
        result[1] = std::max(result[1], value.y);
        result[2] = std::max(result[2], value.z);
    }

    return result;
}

[[nodiscard]] std::array<double, 1> index_min_value(const std::vector<std::uint32_t> &indices) {
    std::uint32_t min_value = std::numeric_limits<std::uint32_t>::max();
    for (const std::uint32_t index : indices) {
        min_value = std::min(min_value, index);
    }
    return {static_cast<double>(min_value)};
}

[[nodiscard]] std::array<double, 1> index_max_value(const std::vector<std::uint32_t> &indices) {
    std::uint32_t max_value = 0;
    for (const std::uint32_t index : indices) {
        max_value = std::max(max_value, index);
    }
    return {static_cast<double>(max_value)};
}

void append_padding(std::vector<unsigned char> *buffer) {
    while (buffer->size() % 4 != 0) {
        buffer->push_back(0);
    }
}

template <typename T>
std::size_t append_vector_bytes(std::vector<unsigned char> *buffer, const std::vector<T> &values) {
    append_padding(buffer);
    const std::size_t offset = buffer->size();
    const std::size_t byte_count = values.size() * sizeof(T);
    buffer->resize(offset + byte_count);
    if (byte_count > 0) {
        std::memcpy(buffer->data() + offset, values.data(), byte_count);
    }
    return offset;
}

[[nodiscard]] std::vector<float> flatten_vec3_values(const std::vector<Vec3> &values) {
    std::vector<float> flattened;
    flattened.reserve(values.size() * 3);
    for (const Vec3 &value : values) {
        flattened.push_back(static_cast<float>(value.x));
        flattened.push_back(static_cast<float>(value.y));
        flattened.push_back(static_cast<float>(value.z));
    }
    return flattened;
}

int add_buffer_view(tinygltf::Model *model, int target, std::size_t byte_offset, std::size_t byte_length) {
    tinygltf::BufferView buffer_view;
    buffer_view.buffer = 0;
    buffer_view.byteOffset = byte_offset;
    buffer_view.byteLength = byte_length;
    buffer_view.target = target;
    model->bufferViews.push_back(std::move(buffer_view));
    return static_cast<int>(model->bufferViews.size()) - 1;
}

int add_vec3_accessor(tinygltf::Model *model, int buffer_view_index, std::size_t count, const std::array<double, 3> &min_values, const std::array<double, 3> &max_values) {
    tinygltf::Accessor accessor;
    accessor.bufferView = buffer_view_index;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = count;
    accessor.type = TINYGLTF_TYPE_VEC3;
    accessor.minValues = {min_values[0], min_values[1], min_values[2]};
    accessor.maxValues = {max_values[0], max_values[1], max_values[2]};
    model->accessors.push_back(std::move(accessor));
    return static_cast<int>(model->accessors.size()) - 1;
}

int add_index_accessor(tinygltf::Model *model, int buffer_view_index, std::size_t count, const std::array<double, 1> &min_values, const std::array<double, 1> &max_values) {
    tinygltf::Accessor accessor;
    accessor.bufferView = buffer_view_index;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    accessor.count = count;
    accessor.type = TINYGLTF_TYPE_SCALAR;
    accessor.minValues = {min_values[0]};
    accessor.maxValues = {max_values[0]};
    model->accessors.push_back(std::move(accessor));
    return static_cast<int>(model->accessors.size()) - 1;
}

// The Haxby color table that mbm_grdplot uses, and the bounds and colors by height, are shared with mbgrd2gltf (src/mbdraco/haxby_colors.h).
using mbdraco::color_for_value;
using mbdraco::haxby_color_count;
using mbdraco::haxby_palette_color;
using mbdraco::linear_color_bounds;
using mbdraco::palette_colors_for_bounds;

// The colors for the heights, as the color table that mbm_grdplot -G5 makes for a grid with the Haxby colors, with a color at each of
// 11 bounds and the color changing linearly from one to the next; the bounds are returned in bounds. For HaxbyLinear they
// are those of linear_color_bounds. For HaxbyEqualized (-S) the data are divided into 11 cells of equal numbers of values
// (what "gmt grdhisteq -C11" does to a Cartesian grid), and the bounds are each cell's lower bound, 0.01 of the range
// below the minimum for the first. A value above the last bound has the last color, white. If the values are not spread
// enough to have 11 different bounds, the palette is interpolated over fewer, as mbm_grdplot does.
[[nodiscard]] std::vector<float> colors_for_values(const std::vector<double> &values, PointColorMap colormap, std::vector<double> *bounds) {
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t count = sorted.size();
    const double range = sorted.back() - sorted.front();

    if (colormap == PointColorMap::HaxbyLinear) {
        linear_color_bounds(sorted.front(), sorted.back(), bounds);
    } else {
        bounds->clear();
        bounds->push_back(sorted.front() - 0.01 * range);
        for (int k = 1; k < haxby_color_count; k++) {
            // The first value of cell k, which is the last of cell k - 1, as gmt grdhisteq reports it.
            const std::size_t last_of_previous = static_cast<std::size_t>(k) * count / (haxby_color_count);
            const double bound = sorted[last_of_previous > 0 ? last_of_previous - 1 : 0];
            if (bound > bounds->back()) {
                bounds->push_back(bound);
            }
        }
    }

    const int bound_count = static_cast<int>(bounds->size());
    const std::vector<std::array<double, 3>> bound_colors = palette_colors_for_bounds(bound_count);

    std::vector<float> colors;
    colors.reserve(count * 3);
    for (const double value : values) {
        const std::array<double, 3> color = color_for_value(value, *bounds, bound_colors);
        for (int c = 0; c < 3; c++) {
            colors.push_back(static_cast<float>(color[c] / 255.0));
        }
    }
    return colors;
}

[[nodiscard]] tinygltf::Material make_material(const std::array<double, 4> &base_color) {
    tinygltf::Material material;
    material.doubleSided = true;
    material.pbrMetallicRoughness.baseColorFactor = {base_color[0], base_color[1], base_color[2], base_color[3]};
    material.pbrMetallicRoughness.metallicFactor = 0.0;
    material.pbrMetallicRoughness.roughnessFactor = 0.65;
    return material;
}

mbdraco::Settings &compression_storage() {
    static mbdraco::Settings settings = [] {
        mbdraco::Settings off;
        off.enabled = false;   // not compressed until set_glb_compression() says so
        return off;
    }();
    return settings;
}

MetaValue &run_metadata_storage() {
    static MetaValue metadata;
    return metadata;
}

bool &has_run_metadata() {
    static bool has = false;
    return has;
}

[[nodiscard]] tinygltf::Value to_gltf_value(const MetaValue &value) {
    switch (value.type) {
    case MetaValue::Type::String:
        return tinygltf::Value(value.text);
    case MetaValue::Type::Number:
        // Whole numbers are written as integers (1, not 1.0), so counts read as counts.
        if (value.number == std::floor(value.number) && std::fabs(value.number) < 2147483647.0) {
            return tinygltf::Value(static_cast<int>(value.number));
        }
        return tinygltf::Value(value.number);
    case MetaValue::Type::Bool:
        return tinygltf::Value(value.flag);
    case MetaValue::Type::Array: {
        tinygltf::Value::Array array;
        for (const MetaValue &item : value.items) {
            array.push_back(to_gltf_value(item));
        }
        return tinygltf::Value(array);
    }
    case MetaValue::Type::Object: {
        tinygltf::Value::Object object;
        for (const auto &member : value.members) {
            object[member.first] = to_gltf_value(member.second);
        }
        return tinygltf::Value(object);
    }
    }
    return tinygltf::Value();
}

// Puts the run's georeferencing and provenance, and what this file holds, in asset.extras.
void apply_asset_extras(tinygltf::Model *model, const GlbContent &content) {
    if (content.metadata == nullptr && !has_run_metadata()) {
        return;
    }

    MetaValue extras = content.metadata != nullptr ? *content.metadata : run_metadata_storage();
    MetaValue *program = nullptr;
    for (auto &member : extras.members) {
        if (member.first == "MB-System") {
            program = &member.second;
        }
    }
    if (program != nullptr && !content.role.empty()) {
        program->set("content", content_metadata(content));
    }
    model->asset.extras = to_gltf_value(extras);
}

[[nodiscard]] bool write_single_primitive_glb_file(
    const std::filesystem::path &path,
    const std::vector<Vec3> &vertices,
    int primitive_mode,
    const std::string &mesh_name,
    const std::array<double, 4> &base_color,
    const std::string &error_context,
    std::string *error,
    const GlbContent &content,
    const std::vector<float> *colors = nullptr) {
    if (path.empty()) {
        set_error(error, "GLB output path is empty");
        return false;
    }

    if (!validate_pointcloud(vertices, error)) {
        return false;
    }

    std::vector<unsigned char> binary_buffer;

    const std::vector<float> positions = flatten_vec3_values(vertices);
    const std::size_t position_offset = append_vector_bytes(&binary_buffer, positions);
    const std::size_t position_byte_length = positions.size() * sizeof(float);
    std::size_t color_offset = 0;
    if (colors != nullptr) {
        color_offset = append_vector_bytes(&binary_buffer, *colors);
    }
    append_padding(&binary_buffer);

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "MB-System mbmesh";
    apply_asset_extras(&model, content);

    tinygltf::Buffer buffer;
    buffer.data = std::move(binary_buffer);
    model.buffers.push_back(std::move(buffer));

    const int position_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, position_offset, position_byte_length);
    const int position_accessor = add_vec3_accessor(&model, position_buffer_view, vertices.size(), vec3_min_values(vertices), vec3_max_values(vertices));

    tinygltf::Primitive primitive;
    primitive.mode = primitive_mode;
    primitive.attributes["POSITION"] = position_accessor;
    primitive.material = 0;
    if (colors != nullptr) {
        // COLOR_0 as RGB floats, which a viewer multiplies by the material's base color.
        const int color_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, color_offset, colors->size() * sizeof(float));
        tinygltf::Accessor color_accessor;
        color_accessor.bufferView = color_buffer_view;
        color_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        color_accessor.count = vertices.size();
        color_accessor.type = TINYGLTF_TYPE_VEC3;
        model.accessors.push_back(std::move(color_accessor));
        primitive.attributes["COLOR_0"] = static_cast<int>(model.accessors.size()) - 1;
    }

    tinygltf::Mesh gltf_mesh;
    gltf_mesh.name = mesh_name;
    gltf_mesh.primitives.push_back(std::move(primitive));
    model.meshes.push_back(std::move(gltf_mesh));

    model.materials.push_back(make_material(base_color));

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(std::move(node));

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(std::move(scene));
    model.defaultScene = 0;

    const std::string output_path = path.string();
    tinygltf::TinyGLTF gltf;
    if (!gltf.WriteGltfSceneToFile(&model, output_path, true, true, true, true)) {
        set_error(error, "Failed to write " + error_context + " GLB file: " + output_path);
        return false;
    }

    return true;
}

} // namespace

std::vector<double> haxby_height_bounds(double minimum, double maximum) {
    std::vector<double> bounds;
    linear_color_bounds(minimum, maximum, &bounds);
    return bounds;
}

void color_mesh_by_height(Mesh *mesh, double height_offset, const std::vector<double> &bounds) {
    mesh->colors.clear();
    if (bounds.empty()) {
        return;
    }
    const std::vector<std::array<double, 3>> bound_colors = palette_colors_for_bounds(static_cast<int>(bounds.size()));
    mesh->colors.reserve(mesh->vertices.size() * 3);
    for (const Vec3 &vertex : mesh->vertices) {
        const std::array<double, 3> color = color_for_value(vertex.z + height_offset, bounds, bound_colors);
        for (int c = 0; c < 3; c++) {
            mesh->colors.push_back(static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, color[c])))));
        }
    }
}

void set_glb_run_metadata(const MetaValue &run_metadata) {
    run_metadata_storage() = run_metadata;
    has_run_metadata() = true;
}

void clear_glb_run_metadata() {
    run_metadata_storage() = MetaValue();
    has_run_metadata() = false;
}

void set_glb_compression(const mbdraco::Settings &settings) {
    compression_storage() = settings;
}

std::vector<std::uint32_t> triangle_blocks(const Mesh &mesh, double block_size) {
    std::vector<std::uint32_t> groups;
    if (!(block_size > 0.0)) {
        return groups;
    }
    const std::size_t triangle_count = mesh.indices.size() / 3;
    groups.resize(triangle_count);
    std::unordered_map<std::uint64_t, std::uint32_t> ids;
    std::uint64_t last_key = 0;
    std::uint32_t last_id = 0;
    bool have_last = false;
    for (std::size_t t = 0; t < triangle_count; t++) {
        const Vec3 &a = mesh.vertices[mesh.indices[3 * t]];
        const Vec3 &b = mesh.vertices[mesh.indices[3 * t + 1]];
        const Vec3 &c = mesh.vertices[mesh.indices[3 * t + 2]];
        const auto bi = static_cast<std::int64_t>(std::floor((a.x + b.x + c.x) / 3.0 / block_size));
        const auto bj = static_cast<std::int64_t>(std::floor((a.y + b.y + c.y) / 3.0 / block_size));
        const std::uint64_t key = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(bi)) << 32) | static_cast<std::uint32_t>(bj);
        if (!have_last || key != last_key) {
            auto found = ids.find(key);
            if (found == ids.end()) {
                found = ids.emplace(key, static_cast<std::uint32_t>(ids.size())).first;
            }
            last_key = key;
            last_id = found->second;
            have_last = true;
        }
        groups[t] = last_id;
    }
    return groups;
}

bool write_mesh_glb_file(const std::filesystem::path &path, const Mesh &mesh, std::string *error, const GlbContent &content_in,
                         const std::vector<std::uint32_t> *triangle_groups) {
    if (error != nullptr) {
        error->clear();
    }
    GlbContent content = content_in;

    if (path.empty()) {
        set_error(error, "GLB output path is empty");
        return false;
    }

    if (!validate_mesh(mesh, error)) {
        return false;
    }

    const std::size_t triangle_count = mesh.indices.size() / 3;
    if (triangle_groups != nullptr && !triangle_groups->empty() && triangle_groups->size() != triangle_count) {
        set_error(error, "the triangle groups do not match the triangles of the mesh");
        return false;
    }

    // The triangles of each group, as lists of triangle numbers (counting sort). One group, the whole mesh, when there are no groups.
    std::uint32_t group_count = 1;
    std::vector<std::size_t> group_start;
    std::vector<std::uint32_t> group_triangles;
    const bool split = triangle_groups != nullptr && !triangle_groups->empty();
    if (split) {
        group_count = *std::max_element(triangle_groups->begin(), triangle_groups->end()) + 1;
        group_start.assign(group_count + 1, 0);
        for (const std::uint32_t group : *triangle_groups) {
            group_start[group + 1]++;
        }
        for (std::uint32_t g = 0; g < group_count; g++) {
            group_start[g + 1] += group_start[g];
        }
        group_triangles.resize(triangle_count);
        std::vector<std::size_t> fill(group_start.begin(), group_start.end() - 1);
        for (std::size_t t = 0; t < triangle_count; t++) {
            group_triangles[fill[(*triangle_groups)[t]]++] = static_cast<std::uint32_t>(t);
        }
    }

    // Each part holds the vertices that its triangles use, numbered from zero in the order they are first used, as mbgrd2gltf's primitives do,
    // so a primitive decodes by itself and its indices are small.
    struct Part {
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<std::uint8_t> colors;
        std::vector<std::uint32_t> indices;
    };
    std::vector<std::uint32_t> local(split ? mesh.vertices.size() : 0, std::numeric_limits<std::uint32_t>::max());
    std::vector<std::uint32_t> touched;
    const auto make_part = [&](std::uint32_t group, Part *part) {
        *part = Part();
        if (!split) {
            part->positions = flatten_vec3_values(mesh.vertices);
            if (!mesh.normals.empty()) {
                part->normals = flatten_vec3_values(mesh.normals);
            }
            part->colors = mesh.colors;
            part->indices.assign(mesh.indices.begin(), mesh.indices.end());
            return;
        }
        touched.clear();
        part->indices.reserve((group_start[group + 1] - group_start[group]) * 3);
        for (std::size_t k = group_start[group]; k < group_start[group + 1]; k++) {
            const std::size_t t = group_triangles[k];
            for (int corner = 0; corner < 3; corner++) {
                const std::uint32_t vertex = mesh.indices[3 * t + corner];
                if (local[vertex] == std::numeric_limits<std::uint32_t>::max()) {
                    local[vertex] = static_cast<std::uint32_t>(touched.size());
                    touched.push_back(vertex);
                }
                part->indices.push_back(local[vertex]);
            }
        }
        part->positions.reserve(touched.size() * 3);
        for (const std::uint32_t vertex : touched) {
            part->positions.push_back(static_cast<float>(mesh.vertices[vertex].x));
            part->positions.push_back(static_cast<float>(mesh.vertices[vertex].y));
            part->positions.push_back(static_cast<float>(mesh.vertices[vertex].z));
        }
        if (!mesh.normals.empty()) {
            part->normals.reserve(touched.size() * 3);
            for (const std::uint32_t vertex : touched) {
                part->normals.push_back(static_cast<float>(mesh.normals[vertex].x));
                part->normals.push_back(static_cast<float>(mesh.normals[vertex].y));
                part->normals.push_back(static_cast<float>(mesh.normals[vertex].z));
            }
        }
        if (!mesh.colors.empty()) {
            part->colors.reserve(touched.size() * 3);
            for (const std::uint32_t vertex : touched) {
                part->colors.insert(part->colors.end(), mesh.colors.begin() + 3 * vertex, mesh.colors.begin() + 3 * vertex + 3);
            }
        }
        for (const std::uint32_t vertex : touched) {
            local[vertex] = std::numeric_limits<std::uint32_t>::max();
        }
    };

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "MB-System mbmesh";
    tinygltf::Mesh gltf_mesh;
    gltf_mesh.name = "mesh";
    std::vector<unsigned char> binary_buffer;
    bool compressed = false;

    // Compressed by Draco if it is on and there is a Draco: one stream for each part, in one buffer. If one fails, the arrays are written.
    if (compression_storage().enabled && mbdraco::available()) {
        compressed = true;
        std::string reason;
        mbdraco::Compression used;
        Part part;
        for (std::uint32_t group = 0; group < group_count && compressed; group++) {
            make_part(group, &part);
            compressed = mbdraco::add_primitive(&model, &gltf_mesh, &binary_buffer, part.positions, part.normals.empty() ? nullptr : &part.normals,
                                                part.indices, compression_storage(), 0, &used, &reason,
                                                part.colors.empty() ? nullptr : &part.colors);
        }
        if (compressed) {
            content.compression = used;
        } else {
            LOG_WARN("Draco compression of " + path.filename().string() + " failed (" + reason + "); written without compression");
            content.compression = mbdraco::Compression();
            model = tinygltf::Model();
            model.asset.version = "2.0";
            model.asset.generator = "MB-System mbmesh";
            gltf_mesh = tinygltf::Mesh();
            gltf_mesh.name = "mesh";
            binary_buffer.clear();
        }
    }

    if (!compressed) {
        Part part;
        for (std::uint32_t group = 0; group < group_count; group++) {
            make_part(group, &part);
            const std::size_t position_offset = append_vector_bytes(&binary_buffer, part.positions);
            const std::size_t normal_offset = part.normals.empty() ? 0 : append_vector_bytes(&binary_buffer, part.normals);
            const std::size_t color_offset = part.colors.empty() ? 0 : append_vector_bytes(&binary_buffer, part.colors);
            append_padding(&binary_buffer);
            const std::size_t index_offset = append_vector_bytes(&binary_buffer, part.indices);
            append_padding(&binary_buffer);

            std::array<double, 3> minimum = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
            std::array<double, 3> maximum = {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
            for (std::size_t k = 0; k < part.positions.size(); k++) {
                minimum[k % 3] = std::min(minimum[k % 3], static_cast<double>(part.positions[k]));
                maximum[k % 3] = std::max(maximum[k % 3], static_cast<double>(part.positions[k]));
            }
            tinygltf::Primitive primitive;
            primitive.mode = TINYGLTF_MODE_TRIANGLES;
            const std::size_t vertex_count = part.positions.size() / 3;
            primitive.attributes["POSITION"] =
                add_vec3_accessor(&model, add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, position_offset, part.positions.size() * sizeof(float)),
                                  vertex_count, minimum, maximum);
            if (!part.normals.empty()) {
                std::array<double, 3> normal_minimum = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
                std::array<double, 3> normal_maximum = {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
                for (std::size_t k = 0; k < part.normals.size(); k++) {
                    normal_minimum[k % 3] = std::min(normal_minimum[k % 3], static_cast<double>(part.normals[k]));
                    normal_maximum[k % 3] = std::max(normal_maximum[k % 3], static_cast<double>(part.normals[k]));
                }
                primitive.attributes["NORMAL"] =
                    add_vec3_accessor(&model, add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, normal_offset, part.normals.size() * sizeof(float)),
                                      vertex_count, normal_minimum, normal_maximum);
            }
            if (!part.colors.empty()) {
                tinygltf::Accessor color_accessor;
                color_accessor.bufferView = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, color_offset, part.colors.size());
                color_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
                color_accessor.normalized = true;
                color_accessor.count = vertex_count;
                color_accessor.type = TINYGLTF_TYPE_VEC3;
                model.accessors.push_back(std::move(color_accessor));
                primitive.attributes["COLOR_0"] = static_cast<int>(model.accessors.size()) - 1;
            }
            primitive.indices = add_index_accessor(
                &model, add_buffer_view(&model, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER, index_offset, part.indices.size() * sizeof(std::uint32_t)),
                part.indices.size(), index_min_value(part.indices), index_max_value(part.indices));
            primitive.material = 0;
            gltf_mesh.primitives.push_back(std::move(primitive));
        }
    }

    content.primitives = static_cast<int>(group_count);
    apply_asset_extras(&model, content);

    append_padding(&binary_buffer);
    tinygltf::Buffer buffer;
    buffer.data = std::move(binary_buffer);
    model.buffers.push_back(std::move(buffer));
    model.meshes.push_back(std::move(gltf_mesh));

    tinygltf::Material material;
    material.doubleSided = true;
    // COLOR_0 is multiplied by the base color, so colored vertices get white, which leaves them as they are.
    material.pbrMetallicRoughness.baseColorFactor = mesh.colors.empty() ? std::vector<double>{0.35, 0.55, 0.72, 1.0} : std::vector<double>{1.0, 1.0, 1.0, 1.0};
    material.pbrMetallicRoughness.metallicFactor = 0.0;
    material.pbrMetallicRoughness.roughnessFactor = 0.65;
    model.materials.push_back(std::move(material));

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(std::move(node));

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(std::move(scene));
    model.defaultScene = 0;

    tinygltf::TinyGLTF gltf;
    if (!gltf.WriteGltfSceneToFile(&model, path.string(), true, true, true, true)) {
        set_error(error, "Failed to write mesh GLB file: " + path.string());
        return false;
    }
    return true;
}

const char *point_color_map_names() {
    return "linear, equalized, none";
}

const char *point_color_map_name(PointColorMap colormap) {
    switch (colormap) {
    case PointColorMap::None:
        return "none";
    case PointColorMap::HaxbyLinear:
        return "linear";
    case PointColorMap::HaxbyEqualized:
        return "equalized";
    }
    return "unknown";
}

bool parse_point_color_map(const std::string &name, PointColorMap *colormap) {
    for (const PointColorMap candidate : {PointColorMap::None, PointColorMap::HaxbyLinear, PointColorMap::HaxbyEqualized}) {
        if (name == point_color_map_name(candidate)) {
            *colormap = candidate;
            return true;
        }
    }
    return false;
}

bool write_pointcloud_glb_file(
    const std::filesystem::path &path,
    const PointCloud &pointcloud,
    std::string *error,
    const GlbContent &content,
    const std::vector<double> *heights,
    PointColorMap colormap) {
    if (error != nullptr) {
        error->clear();
    }

    if (heights != nullptr && colormap != PointColorMap::None && heights->size() == pointcloud.size() && !pointcloud.empty()) {
        std::vector<double> bounds;
        const std::vector<float> colors = colors_for_values(*heights, colormap, &bounds);
        GlbContent colored = content;
        std::ostringstream description;
        description << std::fixed << std::setprecision(2) << ", colored by height in the local frame (tangent to the ellipsoid) with the Haxby color table "
                    << "that mbm_grdplot uses, as \"mbm_grdplot -G5" << (colormap == PointColorMap::HaxbyEqualized ? " -S" : "")
                    << "\" does: the color changes linearly between these heights, in meters, "
                    << (colormap == PointColorMap::HaxbyEqualized ? "which divide the points into cells of equal numbers, " : "which are equally spaced, ")
                    << "from dark blue at the first to white at the last:";
        for (const double bound : bounds) {
            description << " " << bound;
        }
        colored.description += description.str();
        // The colors give the points their hue, so the base color is white.
        return write_single_primitive_glb_file(path, pointcloud, TINYGLTF_MODE_POINTS, "point_cloud", {1.0, 1.0, 1.0, 1.0}, "point cloud", error,
                                               colored, &colors);
    }

    return write_single_primitive_glb_file(
        path,
        pointcloud,
        TINYGLTF_MODE_POINTS,
        "point_cloud",
        {1.0, 0.45, 0.0, 1.0},
        "point cloud",
        error,
        content);
}

bool write_normal_lines_glb_file(
    const std::filesystem::path &path,
    const OrientedPointCloud &pointcloud,
    double line_length,
    std::string *error,
    const GlbContent &content) {
    if (error != nullptr) {
        error->clear();
    }

    if (line_length <= 0.0 || !std::isfinite(line_length)) {
        set_error(error, "Normal line length must be positive and finite");
        return false;
    }

    if (!validate_oriented_pointcloud(pointcloud, error)) {
        return false;
    }

    std::vector<Vec3> line_vertices;
    line_vertices.reserve(pointcloud.size() * 2);
    for (const OrientedPoint &oriented_point : pointcloud) {
        const Vec3 normal = normalize(oriented_point.normal);
        line_vertices.push_back(oriented_point.point);
        line_vertices.push_back(oriented_point.point + normal * line_length);
    }

    return write_single_primitive_glb_file(
        path,
        line_vertices,
        TINYGLTF_MODE_LINE,
        "normal_lines",
        {0.0, 0.75, 1.0, 1.0},
        "normal lines",
        error,
        content);
}

bool write_origin_ray_lines_glb_file(
    const std::filesystem::path &path,
    const CollectedPointCloud &pointcloud,
    double line_length,
    std::string *error,
    const GlbContent &content) {
    if (error != nullptr) {
        error->clear();
    }

    if (line_length <= 0.0 || !std::isfinite(line_length)) {
        set_error(error, "Origin-ray line length must be positive and finite");
        return false;
    }

    if (!validate_collected_pointcloud(pointcloud, error)) {
        return false;
    }

    std::vector<Vec3> line_vertices;
    line_vertices.reserve(pointcloud.size() * 2);
    for (const CollectedPoint &collected_point : pointcloud) {
        const Vec3 direction_to_origin = normalize(collected_point.origin - collected_point.point);
        line_vertices.push_back(collected_point.point);
        line_vertices.push_back(collected_point.point + direction_to_origin * line_length);
    }

    return write_single_primitive_glb_file(
        path,
        line_vertices,
        TINYGLTF_MODE_LINE,
        "origin_ray_lines",
        {1.0, 0.0, 0.85, 1.0},
        "origin-ray lines",
        error,
        content);
}

bool write_track_glb_file(
    const std::filesystem::path &path,
    const std::vector<std::vector<Vec3>> &polylines,
    std::string *error,
    const GlbContent &content) {
    if (error != nullptr) {
        error->clear();
    }

    // Each polyline becomes a run of line segments (pairs of vertices).
    std::vector<Vec3> line_vertices;
    for (const std::vector<Vec3> &polyline : polylines) {
        for (std::size_t i = 0; i + 1 < polyline.size(); i++) {
            line_vertices.push_back(polyline[i]);
            line_vertices.push_back(polyline[i + 1]);
        }
    }

    if (line_vertices.empty()) {
        set_error(error, "The track has no segments to draw");
        return false;
    }

    for (std::size_t i = 0; i < line_vertices.size(); i++) {
        if (!fits_float_vec3(line_vertices[i])) {
            set_error(error, "Track position is outside the GLB float range at index " + std::to_string(i));
            return false;
        }
    }

    return write_single_primitive_glb_file(
        path,
        line_vertices,
        TINYGLTF_MODE_LINE,
        "sensor_track",
        {0.0, 0.0, 0.0, 1.0},
        "sensor track",
        error,
        content);
}
