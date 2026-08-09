#include "io/glb_writer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
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

[[nodiscard]] tinygltf::Material make_material(const std::array<double, 4> &base_color) {
    tinygltf::Material material;
    material.doubleSided = true;
    material.pbrMetallicRoughness.baseColorFactor = {base_color[0], base_color[1], base_color[2], base_color[3]};
    material.pbrMetallicRoughness.metallicFactor = 0.0;
    material.pbrMetallicRoughness.roughnessFactor = 0.65;
    return material;
}

[[nodiscard]] bool write_single_primitive_glb_file(
    const std::filesystem::path &path,
    const std::vector<Vec3> &vertices,
    int primitive_mode,
    const std::string &mesh_name,
    const std::array<double, 4> &base_color,
    const std::string &error_context,
    std::string *error) {
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
    append_padding(&binary_buffer);

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "MB-System mbmesh";

    tinygltf::Buffer buffer;
    buffer.data = std::move(binary_buffer);
    model.buffers.push_back(std::move(buffer));

    const int position_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, position_offset, position_byte_length);
    const int position_accessor = add_vec3_accessor(&model, position_buffer_view, vertices.size(), vec3_min_values(vertices), vec3_max_values(vertices));

    tinygltf::Primitive primitive;
    primitive.mode = primitive_mode;
    primitive.attributes["POSITION"] = position_accessor;
    primitive.material = 0;

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

bool write_mesh_glb_file(const std::filesystem::path &path, const Mesh &mesh, std::string *error) {
    if (error != nullptr) {
        error->clear();
    }

    if (path.empty()) {
        set_error(error, "GLB output path is empty");
        return false;
    }

    if (!validate_mesh(mesh, error)) {
        return false;
    }

    std::vector<unsigned char> binary_buffer;

    const std::vector<float> positions = flatten_vec3_values(mesh.vertices);
    const std::size_t position_offset = append_vector_bytes(&binary_buffer, positions);
    const std::size_t position_byte_length = positions.size() * sizeof(float);

    std::size_t normal_offset = 0;
    std::size_t normal_byte_length = 0;
    if (!mesh.normals.empty()) {
        const std::vector<float> normals = flatten_vec3_values(mesh.normals);
        normal_offset = append_vector_bytes(&binary_buffer, normals);
        normal_byte_length = normals.size() * sizeof(float);
    }

    std::vector<std::uint32_t> indices;
    indices.reserve(mesh.indices.size());
    for (const unsigned int index : mesh.indices) {
        indices.push_back(static_cast<std::uint32_t>(index));
    }

    const std::size_t index_offset = append_vector_bytes(&binary_buffer, indices);
    const std::size_t index_byte_length = indices.size() * sizeof(std::uint32_t);
    append_padding(&binary_buffer);

    tinygltf::Model model;
    model.asset.version = "2.0";
    model.asset.generator = "MB-System mbmesh";

    tinygltf::Buffer buffer;
    buffer.data = std::move(binary_buffer);
    model.buffers.push_back(std::move(buffer));

    const int position_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, position_offset, position_byte_length);
    const int position_accessor = add_vec3_accessor(&model, position_buffer_view, mesh.vertices.size(), vec3_min_values(mesh.vertices), vec3_max_values(mesh.vertices));

    int normal_accessor = -1;
    if (!mesh.normals.empty()) {
        const int normal_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ARRAY_BUFFER, normal_offset, normal_byte_length);
        normal_accessor = add_vec3_accessor(&model, normal_buffer_view, mesh.normals.size(), vec3_min_values(mesh.normals), vec3_max_values(mesh.normals));
    }

    const int index_buffer_view = add_buffer_view(&model, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER, index_offset, index_byte_length);
    const int index_accessor = add_index_accessor(&model, index_buffer_view, indices.size(), index_min_value(indices), index_max_value(indices));

    tinygltf::Primitive primitive;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    primitive.attributes["POSITION"] = position_accessor;
    if (normal_accessor >= 0) {
        primitive.attributes["NORMAL"] = normal_accessor;
    }
    primitive.indices = index_accessor;
    primitive.material = 0;

    tinygltf::Mesh gltf_mesh;
    gltf_mesh.name = "mesh";
    gltf_mesh.primitives.push_back(std::move(primitive));
    model.meshes.push_back(std::move(gltf_mesh));

    tinygltf::Material material;
    material.doubleSided = true;
    material.pbrMetallicRoughness.baseColorFactor = {0.35, 0.55, 0.72, 1.0};
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

    const std::string output_path = path.string();
    tinygltf::TinyGLTF gltf;
    if (!gltf.WriteGltfSceneToFile(&model, output_path, true, true, true, true)) {
        set_error(error, "Failed to write mesh GLB file: " + output_path);
        return false;
    }

    return true;
}

bool write_pointcloud_glb_file(const std::filesystem::path &path, const PointCloud &pointcloud, std::string *error) {
    if (error != nullptr) {
        error->clear();
    }

    return write_single_primitive_glb_file(
        path,
        pointcloud,
        TINYGLTF_MODE_POINTS,
        "point_cloud",
        {1.0, 0.45, 0.0, 1.0},
        "point cloud",
        error);
}

bool write_normal_lines_glb_file(
    const std::filesystem::path &path,
    const OrientedPointCloud &pointcloud,
    double line_length,
    std::string *error) {
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
        error);
}

bool write_origin_ray_lines_glb_file(
    const std::filesystem::path &path,
    const CollectedPointCloud &pointcloud,
    double line_length,
    std::string *error) {
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
        error);
}
