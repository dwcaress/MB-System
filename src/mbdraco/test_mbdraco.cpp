// Test of the shared Draco code: the quantization bits, the option values, the encoding of a mesh and the primitive that it adds
// to a glTF, with two meshes in one buffer as mbgrd2gltf's tiles are.
#include "draco_mesh.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
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

// A grid of n x n vertices, step meters apart, with a gentle relief, and its triangles.
void grid(int n, double step, double x0, std::vector<float> *positions, std::vector<float> *normals, std::vector<std::uint32_t> *indices) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            const double x = x0 + step * i, y = step * j;
            positions->insert(positions->end(), {static_cast<float>(x), static_cast<float>(y), static_cast<float>(0.2 * std::sin(0.9 * x) * std::cos(0.7 * y))});
            if (normals != nullptr) {
                normals->insert(normals->end(), {0.0f, 0.0f, 1.0f});
            }
        }
    }
    for (int j = 0; j + 1 < n; j++) {
        for (int i = 0; i + 1 < n; i++) {
            const std::uint32_t a = j * n + i, b = a + 1, c = a + n, d = c + 1;
            indices->insert(indices->end(), {a, b, d, a, d, c});
        }
    }
}

} // namespace

int main() {
    // The bits for a precision.
    check(mbdraco::position_bits(3.0, 0.001) == 12, "3 m at a millimeter takes 12 bits");
    check(mbdraco::position_bits(120.0, 0.001) == 17, "120 m at a millimeter takes 17 bits");
    check(mbdraco::position_bits(0.0001, 0.001) == 8 && mbdraco::position_bits(1.0e9, 0.001) == 30, "the bits are kept to 8 to 30");
    check(mbdraco::position_bits(0.0, 0.001) == 8 && mbdraco::position_bits(5.0, 0.0) == 8, "no range or no precision gives the least");

    // The defaults, which both programs share.
    const mbdraco::Settings defaults;
    check(defaults.enabled && defaults.precision == 0.001 && defaults.normal_bits == 10, "Draco is on, at a millimeter and 10 bits of normal");

    // The option values.
    double precision = 0.0;
    int bits = 0;
    std::string reason;
    check(mbdraco::parse_precision("0.01", &precision, &reason) && precision == 0.01, "a precision parses");
    for (const char *bad : {"", "abc", "0", "-1", "1x", "nan"}) {
        check(!mbdraco::parse_precision(bad, &precision, &reason) && reason.find("not a size in meters") != std::string::npos, "a bad precision is refused");
    }
    check(mbdraco::parse_normal_bits("12", &bits, &reason) && bits == 12, "bits parse");
    for (const char *bad : {"", "abc", "1", "31", "8.5"}) {
        check(!mbdraco::parse_normal_bits(bad, &bits, &reason) && reason.find("not a number of bits") != std::string::npos, "bad bits are refused");
    }

    if (!mbdraco::available()) {
        std::printf("this build has no Draco: the encoding is not tested\n");
        return failures == 0 ? 0 : 1;
    }

    // Two meshes of different sizes in one glTF, one with normals and one without.
    std::vector<float> positions_a, normals_a, positions_b;
    std::vector<std::uint32_t> indices_a, indices_b;
    grid(30, 0.1, 0.0, &positions_a, &normals_a, &indices_a);
    grid(60, 2.0, 500.0, &positions_b, nullptr, &indices_b);

    tinygltf::Model model;
    tinygltf::Mesh mesh;
    std::vector<unsigned char> buffer;
    mbdraco::Compression used;
    check(mbdraco::add_primitive(&model, &mesh, &buffer, positions_a, &normals_a, indices_a, defaults, 0, &used, &reason), "adds a mesh with normals");
    const int first_bits = used.position_bits;
    check(mbdraco::add_primitive(&model, &mesh, &buffer, positions_b, nullptr, indices_b, defaults, 0, &used, &reason), "adds a mesh without normals");
    check(mesh.primitives.size() == 2 && model.bufferViews.size() == 2, "a primitive and a buffer view for each");
    check(model.bufferViews[1].byteOffset % 4 == 0 && model.bufferViews[1].byteOffset >= model.bufferViews[0].byteLength, "the second stream is aligned and after the first");
    check(model.bufferViews[1].byteOffset + model.bufferViews[1].byteLength == buffer.size(), "the streams fill the buffer");
    check(mesh.primitives[0].attributes.count("NORMAL") == 1 && mesh.primitives[1].attributes.count("NORMAL") == 0, "only the first has normals");
    check(model.extensionsUsed.size() == 1 && model.extensionsRequired.size() == 1, "the extension is listed once, used and required");
    check(mesh.primitives[0].extensions.count("KHR_draco_mesh_compression") == 1, "the primitive names the extension");
    check(used.present && used.position_bits > first_bits, "the larger mesh gets more bits, and the file records the most");
    check(used.normal_bits == 10, "and the bits of the normals");

    // Each stream decodes to as many points and triangles as went in, and the accessors say so.
    std::vector<float> decoded;
    std::vector<std::uint32_t> decoded_indices;
    const std::size_t points[2] = {positions_a.size() / 3, positions_b.size() / 3};
    const std::size_t faces[2] = {indices_a.size() / 3, indices_b.size() / 3};
    for (int k = 0; k < 2; k++) {
        const tinygltf::BufferView &view = model.bufferViews[k];
        const std::vector<unsigned char> stream(buffer.begin() + static_cast<std::ptrdiff_t>(view.byteOffset),
                                                buffer.begin() + static_cast<std::ptrdiff_t>(view.byteOffset + view.byteLength));
        check(mbdraco::decode(stream, &decoded, &decoded_indices, &reason), "a stream decodes");
        check(decoded.size() / 3 == points[k] && decoded_indices.size() / 3 == faces[k], "to as many points and triangles as went in");
        const tinygltf::Primitive &primitive = mesh.primitives[k];
        check(model.accessors[primitive.attributes.at("POSITION")].count == points[k] && model.accessors[primitive.indices].count == 3 * faces[k], "as the accessors say");
        check(model.accessors[primitive.attributes.at("POSITION")].bufferView == -1, "which have no buffer views");
    }

    // The error of the positions is within the step that the file records for the mesh with the coarsest.
    const double step_b = 118.0 / (std::pow(2.0, mbdraco::position_bits(118.0, 0.001)) - 1.0);
    check(used.position_step >= step_b * 0.99 && used.position_step <= 0.001, "the step recorded is the coarsest, no more than the precision");

    // A mesh that cannot be encoded says why.
    std::vector<std::uint32_t> none;
    check(!mbdraco::add_primitive(&model, &mesh, &buffer, positions_a, nullptr, none, defaults, 0, &used, &reason) && !reason.empty(), "no triangles is refused, with a reason");

    std::printf("mbdraco: bits %d to %d, step %.6f m\n", first_bits, used.position_bits, used.position_step);
    return failures == 0 ? 0 : 1;
}
