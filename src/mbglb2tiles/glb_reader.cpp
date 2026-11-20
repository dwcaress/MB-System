#include "glb_reader.h"

#include "draco_mesh.h"
#include "util/logger.h"

#include <cmath>
#include <cstdint>
#include <atomic>
#include <cstring>
#include <thread>
#include <vector>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

namespace {

void set_error(std::string *error, const std::string &message) {
    if (error != nullptr) {
        *error = message;
    }
}

// The bytes of an accessor: where element 0 is, and the distance between elements.
[[nodiscard]] bool accessor_bytes(const tinygltf::Model &model, const tinygltf::Accessor &accessor, const unsigned char **start, std::size_t *stride, std::string *error) {
    if (accessor.bufferView < 0 || static_cast<std::size_t>(accessor.bufferView) >= model.bufferViews.size()) {
        set_error(error, "an accessor has no buffer view");
        return false;
    }
    const tinygltf::BufferView &view = model.bufferViews[static_cast<std::size_t>(accessor.bufferView)];
    if (view.buffer < 0 || static_cast<std::size_t>(view.buffer) >= model.buffers.size()) {
        set_error(error, "a buffer view has no buffer");
        return false;
    }
    const int byte_stride = accessor.ByteStride(view);
    if (byte_stride <= 0) {
        set_error(error, "an accessor has an invalid stride");
        return false;
    }
    const std::vector<unsigned char> &data = model.buffers[static_cast<std::size_t>(view.buffer)].data;
    const std::size_t first = view.byteOffset + accessor.byteOffset;
    if (accessor.count > 0 && first + static_cast<std::size_t>(byte_stride) * (accessor.count - 1) + 1 > data.size()) {
        set_error(error, "an accessor reaches beyond its buffer");
        return false;
    }
    *start = data.data() + first;
    *stride = static_cast<std::size_t>(byte_stride);
    return true;
}

[[nodiscard]] bool node_is_identity(const tinygltf::Node &node) {
    const auto near = [](double a, double b) { return std::fabs(a - b) < 1.0e-12; };
    if (!node.matrix.empty()) {
        for (std::size_t k = 0; k < node.matrix.size(); k++) {
            if (!near(node.matrix[k], (k % 5 == 0) ? 1.0 : 0.0)) {
                return false;
            }
        }
    }
    for (std::size_t k = 0; k < node.translation.size(); k++) {
        if (!near(node.translation[k], 0.0)) {
            return false;
        }
    }
    for (std::size_t k = 0; k < node.rotation.size(); k++) {
        if (!near(node.rotation[k], k == 3 ? 1.0 : 0.0)) {
            return false;
        }
    }
    for (std::size_t k = 0; k < node.scale.size(); k++) {
        if (!near(node.scale[k], 1.0)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] double number_of(const tinygltf::Value &value) {
    return value.IsNumber() ? value.GetNumberAsDouble() : 0.0;
}

} // namespace

bool read_mesh_glb(const std::filesystem::path &path, GlbMesh *result, std::string *error, int threads) {
    *result = GlbMesh();
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string load_error, warning;
    if (!loader.LoadBinaryFromFile(&model, &load_error, &warning, path.string())) {
        set_error(error, "cannot read " + path.string() + ": " + load_error);
        return false;
    }
    result->generator = model.asset.generator;

    // Where the vertices are: the georeference block that mbmesh and mbgrd2gltf write.
    const tinygltf::Value &extras = model.asset.extras;
    if (!extras.IsObject() || !extras.Has("MB-System") || !extras.Has("georeference")) {
        set_error(error, path.string() + " has no georeference in its extras, so it was not written by mbmesh or mbgrd2gltf, and where it is on the Earth is not known");
        return false;
    }
    const tinygltf::Value &georeference = extras.Get("georeference");
    const std::string convention = georeference.Has("vertexCoordinates") ? georeference.Get("vertexCoordinates").Get<std::string>() : "";
    if (convention == "relativeToCenter") {
        const tinygltf::Value &center = georeference.Get("center");
        if (!center.IsArray() || center.ArrayLen() != 3) {
            set_error(error, "the georeference has vertexCoordinates relativeToCenter, without a center of three numbers");
            return false;
        }
        result->has_center = true;
        result->center = Vec3(number_of(center.Get(0)), number_of(center.Get(1)), number_of(center.Get(2)));
    } else if (convention != "absolute") {
        set_error(error, "the georeference has vertexCoordinates '" + convention + "', not absolute or relativeToCenter");
        return false;
    }
    const tinygltf::Value &content = extras.Get("MB-System").Get("content");
    if (content.IsObject()) {
        if (content.Has("compression") && content.Get("compression").Has("positionStep")) {
            result->position_step = number_of(content.Get("compression").Get("positionStep"));
        }
        if (content.Has("description") && content.Get("description").IsString()) {
            result->description = content.Get("description").Get<std::string>();
        }
    }

    for (const tinygltf::Node &node : model.nodes) {
        if (node.mesh >= 0 && !node_is_identity(node)) {
            set_error(error, "a node of the file has a transform, which is not handled: the vertices of mbmesh and mbgrd2gltf are in the file's frame");
            return false;
        }
    }

    // The pieces, in the order of the file: a Draco primitive is a stream to decode, a plain one is read at once.
    struct Piece {
        bool draco = false;
        const unsigned char *stream = nullptr;
        std::size_t length = 0;
        std::vector<float> positions;
        std::vector<std::uint32_t> indices;
        std::string problem;
    };
    std::vector<Piece> pieces;
    for (const tinygltf::Mesh &gltf_mesh : model.meshes) {
        for (const tinygltf::Primitive &primitive : gltf_mesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                set_error(error, "the file has a primitive that is not triangles");
                return false;
            }
            Piece piece;
            const auto draco = primitive.extensions.find("KHR_draco_mesh_compression");
            if (draco != primitive.extensions.end()) {
                const int view_index = draco->second.Get("bufferView").GetNumberAsInt();
                if (view_index < 0 || static_cast<std::size_t>(view_index) >= model.bufferViews.size()) {
                    set_error(error, "a Draco primitive names a buffer view that is not there");
                    return false;
                }
                const tinygltf::BufferView &view = model.bufferViews[static_cast<std::size_t>(view_index)];
                piece.draco = true;
                piece.stream = model.buffers.at(static_cast<std::size_t>(view.buffer)).data.data() + view.byteOffset;
                piece.length = view.byteLength;
                result->draco_primitives++;
            } else {
                const auto found = primitive.attributes.find("POSITION");
                if (found == primitive.attributes.end() || primitive.indices < 0) {
                    set_error(error, "a primitive has no POSITION or no indices");
                    return false;
                }
                const tinygltf::Accessor &position = model.accessors.at(static_cast<std::size_t>(found->second));
                if (position.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || position.type != TINYGLTF_TYPE_VEC3) {
                    set_error(error, "POSITION is not three floats");
                    return false;
                }
                const unsigned char *start = nullptr;
                std::size_t stride = 0;
                if (!accessor_bytes(model, position, &start, &stride, error)) {
                    return false;
                }
                piece.positions.resize(3 * position.count);
                for (std::size_t k = 0; k < position.count; k++) {
                    std::memcpy(&piece.positions[3 * k], start + k * stride, 3 * sizeof(float));
                }
                const tinygltf::Accessor &index = model.accessors.at(static_cast<std::size_t>(primitive.indices));
                if (!accessor_bytes(model, index, &start, &stride, error)) {
                    return false;
                }
                for (std::size_t k = 0; k < index.count; k++) {
                    std::uint32_t value = 0;
                    switch (index.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        value = start[k * stride];
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        std::uint16_t small;
                        std::memcpy(&small, start + k * stride, sizeof(small));
                        value = small;
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        std::memcpy(&value, start + k * stride, sizeof(value));
                        break;
                    default:
                        set_error(error, "the indices are not unsigned integers");
                        return false;
                    }
                    piece.indices.push_back(value);
                }
            }
            pieces.push_back(std::move(piece));
        }
    }
    result->primitives = pieces.size();

    // Decode the Draco streams, each by itself, on several threads.
    if (result->draco_primitives > 0) {
        const int thread_count = static_cast<int>(std::max<std::size_t>(1, std::min<std::size_t>(static_cast<std::size_t>(std::max(1, threads)), result->draco_primitives)));
        LOG_INFO("[input]", "decoding " + std::to_string(result->draco_primitives) + " Draco primitives on " + std::to_string(thread_count) + " threads");
        std::atomic<std::size_t> next(0), done(0);
        const std::size_t step = result->draco_primitives < 20 ? result->draco_primitives + 1 : result->draco_primitives / 10;
        const auto work = [&]() {
            for (;;) {
                const std::size_t n = next.fetch_add(1);
                if (n >= pieces.size()) {
                    return;
                }
                Piece &piece = pieces[n];
                if (!piece.draco) {
                    continue;
                }
                if (!mbdraco::decode(std::vector<unsigned char>(piece.stream, piece.stream + piece.length), &piece.positions, &piece.indices, &piece.problem)) {
                    if (piece.problem.empty()) {
                        piece.problem = "failed";
                    }
                }
                const std::size_t finished = done.fetch_add(1) + 1;
                if (finished % step == 0 && finished < result->draco_primitives) {
                    LOG_INFO("[input]", "decoded " + std::to_string(finished) + " of " + std::to_string(result->draco_primitives) + " Draco primitives");
                }
            }
        };
        std::vector<std::thread> helpers;
        for (int t = 1; t < thread_count; t++) {
            helpers.emplace_back(work);
        }
        work();
        for (std::thread &helper : helpers) {
            helper.join();
        }
    }

    Mesh &mesh = result->mesh;
    std::size_t vertex_total = 0, index_total = 0;
    for (const Piece &piece : pieces) {
        vertex_total += piece.positions.size() / 3;
        index_total += piece.indices.size();
    }
    mesh.vertices.reserve(vertex_total);
    mesh.indices.reserve(index_total);
    for (Piece &piece : pieces) {
        if (!piece.problem.empty()) {
            set_error(error, "cannot decode a Draco primitive: " + piece.problem);
            return false;
        }
        const std::size_t base = mesh.vertices.size();
        for (std::size_t k = 0; k + 2 < piece.positions.size(); k += 3) {
            mesh.vertices.push_back(Vec3(piece.positions[k], piece.positions[k + 1], piece.positions[k + 2]));
        }
        for (const std::uint32_t index : piece.indices) {
            mesh.indices.push_back(static_cast<unsigned int>(base + index));
        }
        piece = Piece();
    }
    if (mesh.vertices.empty() || mesh.indices.empty() || mesh.indices.size() % 3 != 0) {
        set_error(error, path.string() + " holds no triangles");
        return false;
    }
    for (const unsigned int index : mesh.indices) {
        if (index >= mesh.vertices.size()) {
            set_error(error, path.string() + " has an index beyond its vertices");
            return false;
        }
    }
    return true;
}
