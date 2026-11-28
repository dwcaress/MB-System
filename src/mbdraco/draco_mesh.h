#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tinygltf {
class Model;
struct Mesh;
} // namespace tinygltf

// Draco compression of triangle meshes for the KHR_draco_mesh_compression extension of glTF, shared by mbmesh and mbgrd2gltf so
// that both choose the quantization the same way, write the same primitives and are given the same options:
//
//   --no-draco              write the arrays, without compression (the default is to compress)
//   --draco-precision NUM   the coarsest step of the quantization of the positions, in meters (default 0.001)
//   --draco-normal-bits NUM the bits of each component of a normal, 2 to 30 (default 10), for the programs that write normals
//
// The positions are quantized over the largest of the ranges of the three axes of each mesh, with the number of bits that makes
// the step no coarser than the precision, so a small mesh gets few bits and a large one more, not a fixed number.
//
// The code is the Draco source in src/mbdraco/draco, which both the CMake and the autotools builds compile. A build that does not
// include it (MBDRACO_WITH_DRACO not defined) has available() false, and the programs write meshes without compression.
namespace mbdraco {

constexpr double kDefaultPrecision = 0.001; // meters
constexpr int kDefaultNormalBits = 10;

struct Settings {
    bool enabled = true;
    double precision = kDefaultPrecision;
    int normal_bits = kDefaultNormalBits;
};

// What a file was compressed with, for the metadata that it carries. For a file of many meshes, the most bits and the coarsest step.
struct Compression {
    bool present = false;
    int position_bits = 0;
    double position_step = 0.0; // meters
    int normal_bits = 0;        // 0 when there are no normals
};

struct Encoded {
    std::vector<unsigned char> data;   // the compressed mesh, to be the content of a buffer view
    int position_id = 0;               // the unique ids of the attributes in the compressed mesh, as the extension names them
    int normal_id = -1;                // -1 for a mesh without normals
    int color_id = -1;                 // -1 for a mesh without colors
    std::size_t point_count = 0;       // what the compressed mesh decodes to, which can differ from what was encoded
    std::size_t face_count = 0;        // (a vertex that touches the surface twice is made two, and a degenerate triangle goes)
};

[[nodiscard]] bool available();

// The bits of position that make the step of the quantization no more than precision meters over a range of range meters, at
// least 8 and at most 30 (Draco's limit).
[[nodiscard]] int position_bits(double range, double precision);

// The values of --draco-precision and --draco-normal-bits, checked the same way by both programs: false, with the reason in *error,
// if the text is not a size in meters of more than zero, or a number of bits from 2 to 30.
[[nodiscard]] bool parse_precision(const char *text, double *precision, std::string *error);
[[nodiscard]] bool parse_normal_bits(const char *text, int *bits, std::string *error);

// Encodes the mesh: positions (x, y, z for each vertex), normals (the same, or null), and three indices for each triangle, with
// the positions in position_bits bits (2 to 30) over the largest of the ranges of the three axes, and the normals in
// normal_bits bits each. The result is decoded again to check it, and to count what it holds. Returns false and says why in
// *error if it cannot.
// colors, if not null, are three bytes (red, green, blue) for each vertex. Draco gets them as floats from 0 to 1, quantized to 10 bits, which is finer than the bytes: CesiumJS decodes
// a Draco color of bytes wrongly. The glTF accessor of COLOR_0 is then FLOAT. Without Draco the colors are bytes.
[[nodiscard]] bool encode(const std::vector<float> &positions, const std::vector<float> *normals, const std::vector<std::uint32_t> &indices,
                          int position_bits, int normal_bits, Encoded *result, std::string *error,
                          const std::vector<std::uint8_t> *colors = nullptr);

// The positions and faces of a compressed mesh, for tests: three floats for each point, three indices for each face.
[[nodiscard]] bool decode(const std::vector<unsigned char> &data, std::vector<float> *positions, std::vector<std::uint32_t> *indices, std::string *error);

// Compresses a mesh into a primitive of a glTF mesh: the quantization is chosen from the range of the positions and the settings,
// the stream is appended to *buffer (at a multiple of four bytes) and a buffer view of the model is made for it, and the primitive
// is added to mesh->primitives with its accessors (which have no buffer views) and the extension, and the extension is listed as
// used and required. A mesh can have many primitives, as in the files of both programs, each a stream of its own: the caller puts *buffer into model->buffers[0] after
// the last one, and the model's other parts. *used is made to hold what the file was compressed with. Returns false, with the
// reason in *reason, if it cannot, and the caller writes the arrays.
[[nodiscard]] bool add_primitive(tinygltf::Model *model, tinygltf::Mesh *mesh, std::vector<unsigned char> *buffer, const std::vector<float> &positions,
                                 const std::vector<float> *normals, const std::vector<std::uint32_t> &indices, const Settings &settings, int material,
                                 Compression *used, std::string *reason, const std::vector<std::uint8_t> *colors = nullptr);

} // namespace mbdraco
