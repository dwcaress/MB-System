#include "draco_mesh.h"
#include "io/glb_writer.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::printf("FAILED: %s\n", what);
        failures++;
    }
}

std::vector<unsigned char> read_file(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::uint32_t word(const std::vector<unsigned char> &bytes, std::size_t at) {
    return static_cast<std::uint32_t>(bytes[at]) | (static_cast<std::uint32_t>(bytes[at + 1]) << 8) | (static_cast<std::uint32_t>(bytes[at + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
}

} // namespace

int main() {
    // The bits for a precision: a step of at most a millimeter over 3 m takes 12 bits, over 120 m 17; never fewer than 8 or more than 30.
    check(mbdraco::position_bits(3.0, 0.001) == 12, "3 m at a millimeter takes 12 bits");
    check(mbdraco::position_bits(120.0, 0.001) == 17, "120 m at a millimeter takes 17 bits");
    check(mbdraco::position_bits(0.0001, 0.001) == 8 && mbdraco::position_bits(1.0e9, 0.001) == 30, "the bits are kept to 8 to 30");
    check(mbdraco::position_bits(0.0, 0.001) == 8 && mbdraco::position_bits(5.0, 0.0) == 8, "no range or no precision gives the least");

    // A hilly grid of 40 x 40 vertices, 10 cm apart, with normals pointing mostly up.
    constexpr int n = 40;
    Mesh mesh;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            const double x = 0.1 * i, y = 0.1 * j;
            mesh.vertices.push_back(Vec3(x, y, 0.3 * std::sin(1.7 * x) * std::cos(1.3 * y)));
            const double dx = -0.3 * 1.7 * std::cos(1.7 * x) * std::cos(1.3 * y), dy = 0.3 * 1.3 * std::sin(1.7 * x) * std::sin(1.3 * y);
            const double length = std::sqrt(dx * dx + dy * dy + 1.0);
            mesh.normals.push_back(Vec3(dx / length, dy / length, 1.0 / length));
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

    const std::filesystem::path directory = std::filesystem::temp_directory_path() / "mbmesh_test_draco";
    std::filesystem::create_directories(directory);
    std::string error;
    {
        mbdraco::Settings off;
        off.enabled = false;
        set_glb_compression(off);
    }
    check(write_mesh_glb_file(directory / "plain.glb", mesh, &error), "writes without compression");

    // Split into blocks of 1 m: the 3.9 m square is 4 x 4 blocks, one primitive each, with the vertices that its triangles use.
    const std::vector<std::uint32_t> groups = triangle_blocks(mesh, 1.0);
    check(groups.size() == mesh.indices.size() / 3 && *std::max_element(groups.begin(), groups.end()) == 15, "triangle_blocks makes 16 groups for 16 blocks");
    check(triangle_blocks(mesh, 0.0).empty(), "and none for a block size of zero");
    check(write_mesh_glb_file(directory / "split_plain.glb", mesh, &error, GlbContent(), &groups), "writes the blocks without compression");
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string load_error, warning;
        check(loader.LoadBinaryFromFile(&model, &load_error, &warning, (directory / "split_plain.glb").string()), "the split file loads");
        check(model.meshes.size() == 1 && model.meshes[0].primitives.size() == 16, "as one mesh of 16 primitives");
        std::size_t index_count = 0, vertex_count = 0;
        bool local = true;
        for (const tinygltf::Primitive &primitive : model.meshes[0].primitives) {
            const tinygltf::Accessor &position = model.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::Accessor &index = model.accessors[primitive.indices];
            index_count += index.count;
            vertex_count += position.count;
            local = local && index.maxValues[0] + 1 == static_cast<double>(position.count);
        }
        check(index_count == mesh.indices.size(), "which hold all the triangles");
        check(local && vertex_count > mesh.vertices.size() && vertex_count < 2 * mesh.vertices.size(), "each with its own vertices, those on the edges in two");
    }

    // Colored by height: the Haxby table over the heights of the surface, COLOR_0 on every primitive and a white base color.
    {
        const std::vector<double> bounds = haxby_height_bounds(-0.3, 0.3);
        check(bounds.size() == 11, "the Haxby bounds are 11 heights");
        Mesh colored = mesh;
        color_mesh_by_height(&colored, 0.0, bounds);
        check(colored.colors.size() == 3 * mesh.vertices.size(), "color_mesh_by_height gives three bytes to a vertex");
        check(write_mesh_glb_file(directory / "colored_plain.glb", colored, &error, GlbContent(), &groups), "writes colored blocks without compression");
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string load_error, warning;
        check(loader.LoadBinaryFromFile(&model, &load_error, &warning, (directory / "colored_plain.glb").string()), "the colored file loads");
        bool all = !model.meshes.empty();
        for (const tinygltf::Primitive &primitive : model.meshes[0].primitives) {
            const auto found = primitive.attributes.find("COLOR_0");
            all = all && found != primitive.attributes.end() && model.accessors[found->second].count == model.accessors[primitive.attributes.at("POSITION")].count &&
                  model.accessors[found->second].componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && model.accessors[found->second].normalized;
        }
        check(all, "every primitive has COLOR_0 for each vertex, as normalized bytes");
        check(model.materials[0].pbrMetallicRoughness.baseColorFactor == std::vector<double>({1.0, 1.0, 1.0, 1.0}), "the base color is white");
        if (mbdraco::available()) {
            set_glb_compression(mbdraco::Settings());
            check(write_mesh_glb_file(directory / "colored_draco.glb", colored, &error, GlbContent(), &groups), "writes colored blocks with compression");
            mbdraco::Settings off;
            off.enabled = false;
            set_glb_compression(off);
            tinygltf::Model packed_model;
            check(loader.LoadBinaryFromFile(&packed_model, &load_error, &warning, (directory / "colored_draco.glb").string()), "the colored Draco file loads");
            bool draco_all = !packed_model.meshes.empty();
            for (const tinygltf::Primitive &primitive : packed_model.meshes[0].primitives) {
                const auto extension = primitive.extensions.find("KHR_draco_mesh_compression");
                draco_all = draco_all && extension != primitive.extensions.end() && extension->second.Get("attributes").Has("COLOR_0") && primitive.attributes.count("COLOR_0") == 1;
            }
            check(draco_all, "every compressed primitive has COLOR_0 in its Draco stream");
        }
    }

    if (!mbdraco::available()) {
        std::printf("this build has no Draco: the compressed case is not tested\n");
        return failures == 0 ? 0 : 1;
    }

    set_glb_compression(mbdraco::Settings());   // the defaults: on, a millimeter, 10 bits
    GlbContent content;
    content.role = "mesh";
    check(write_mesh_glb_file(directory / "draco.glb", mesh, &error, content), "writes with compression");
    check(write_mesh_glb_file(directory / "split_draco.glb", mesh, &error, content, &groups), "writes the blocks with compression");
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string load_error, warning;
        loader.LoadBinaryFromFile(&model, &load_error, &warning, (directory / "split_draco.glb").string());
        check(model.meshes.size() == 1 && model.meshes[0].primitives.size() == 16, "the compressed blocks are one mesh of 16 primitives");
        std::size_t faces = 0;
        bool all_decode = true;
        for (const tinygltf::Primitive &primitive : model.meshes[0].primitives) {
            const auto found = primitive.extensions.find("KHR_draco_mesh_compression");
            if (found == primitive.extensions.end()) {
                all_decode = false;
                continue;
            }
            const tinygltf::BufferView &view = model.bufferViews[found->second.Get("bufferView").GetNumberAsInt()];
            const unsigned char *start = model.buffers[view.buffer].data.data() + view.byteOffset;
            std::vector<float> block_positions;
            std::vector<std::uint32_t> block_indices;
            all_decode = mbdraco::decode(std::vector<unsigned char>(start, start + view.byteLength), &block_positions, &block_indices, &error) && all_decode;
            faces += block_indices.size() / 3;
        }
        check(all_decode, "each block's stream decodes by itself");
        check(faces == mesh.indices.size() / 3, "to all the triangles");
    }
    {
        mbdraco::Settings off;
        off.enabled = false;
        set_glb_compression(off);
    }

    const std::vector<unsigned char> plain = read_file(directory / "plain.glb");
    const std::vector<unsigned char> packed = read_file(directory / "draco.glb");
    check(!packed.empty() && packed.size() * 3 < plain.size(), "the compressed file is less than a third of the size");
    check(packed.size() > 28 && word(packed, 0) == 0x46546C67, "it is a GLB");

    // The JSON says what it is, and the binary chunk is the Draco stream that decodes to the vertices, to the step.
    const std::uint32_t json_length = word(packed, 12);
    const std::string json(reinterpret_cast<const char *>(packed.data()) + 20, json_length);
    check(json.find("KHR_draco_mesh_compression") != std::string::npos, "the extension is named");
    check(json.find("\"extensionsRequired\"") != std::string::npos, "and required");
    const std::size_t binary_start = 20 + json_length + 8;
    const std::uint32_t binary_length = word(packed, 20 + json_length);
    check(binary_start + binary_length <= packed.size(), "the binary chunk is in the file");
    std::vector<unsigned char> stream(packed.begin() + static_cast<std::ptrdiff_t>(binary_start), packed.begin() + static_cast<std::ptrdiff_t>(binary_start + binary_length));
    std::vector<float> positions;
    std::vector<std::uint32_t> indices;
    check(mbdraco::decode(stream, &positions, &indices, &error), "the stream decodes");
    check(positions.size() == 3 * mesh.vertices.size() && indices.size() == mesh.indices.size(), "to as many points and triangles as went in");

    // Every vertex has a decoded point within the step of the quantization, which is the range (3.9 m) over 2^12 - 1.
    const double step = 3.9 / (std::pow(2.0, mbdraco::position_bits(3.9, 0.001)) - 1.0);
    double worst = 0.0;
    for (const Vec3 &vertex : mesh.vertices) {
        double nearest = 1.0e9;
        for (std::size_t p = 0; p + 2 < positions.size(); p += 3) {
            nearest = std::min(nearest, std::max({std::fabs(vertex.x - positions[p]), std::fabs(vertex.y - positions[p + 1]), std::fabs(vertex.z - positions[p + 2])}));
        }
        worst = std::max(worst, nearest);
    }
    check(worst <= step, "every vertex is within the quantization step of a decoded point");

    // The metadata of the file says how it was compressed.
    check(json.find("\"positionBits\"") == std::string::npos, "without run metadata the file has no extras to say it in");

    std::filesystem::remove_all(directory);
    std::printf("Draco: %zu bytes against %zu uncompressed, worst error %.6f m (step %.6f)\n", packed.size(), plain.size(), worst, step);
    return failures == 0 ? 0 : 1;
}
