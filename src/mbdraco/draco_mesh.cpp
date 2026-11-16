#include "draco_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <memory>

namespace mbdraco {

int position_bits(double range, double precision) {
    if (!(range > 0.0) || !(precision > 0.0) || !std::isfinite(range) || !std::isfinite(precision)) {
        return 8;
    }
    const int bits = static_cast<int>(std::ceil(std::log2(range / precision + 1.0)));
    return std::max(8, std::min(30, bits));
}

bool parse_precision(const char *text, double *precision, std::string *error) {
    char *end = nullptr;
    const double value = text != nullptr ? std::strtod(text, &end) : 0.0;
    if (text == nullptr || end == text || *end != '\0' || !std::isfinite(value) || value <= 0.0) {
        if (error != nullptr) {
            *error = std::string("'") + (text != nullptr ? text : "") + "' is not a size in meters, more than zero";
        }
        return false;
    }
    *precision = value;
    return true;
}

bool parse_normal_bits(const char *text, int *bits, std::string *error) {
    char *end = nullptr;
    const long value = text != nullptr ? std::strtol(text, &end, 10) : 0;
    if (text == nullptr || end == text || *end != '\0' || value < 2 || value > 30) {
        if (error != nullptr) {
            *error = std::string("'") + (text != nullptr ? text : "") + "' is not a number of bits, 2 to 30";
        }
        return false;
    }
    *bits = static_cast<int>(value);
    return true;
}

} // namespace mbdraco

#ifdef MBDRACO_WITH_DRACO

#include "draco/compression/decode.h"
#include "draco/compression/encode.h"
#include "tinygltf/tiny_gltf.h"

namespace mbdraco {

bool available() {
    return true;
}

bool encode(const std::vector<float> &positions, const std::vector<float> *normals, const std::vector<std::uint32_t> &indices,
                       int position_bits, int normal_bits, Encoded *result, std::string *error, const std::vector<std::uint8_t> *colors) {
    const auto fail = [&](const std::string &message) {
        if (error != nullptr) {
            *error = message;
        }
        return false;
    };
    if (result == nullptr || positions.empty() || positions.size() % 3 != 0 || indices.empty() || indices.size() % 3 != 0) {
        return fail("Draco: no mesh to encode");
    }
    if (position_bits < 2 || position_bits > 30 || normal_bits < 2 || normal_bits > 30) {
        return fail("Draco: the quantization bits must be 2 to 30");
    }
    const std::size_t point_count = positions.size() / 3;
    if (normals != nullptr && normals->size() != positions.size()) {
        return fail("Draco: the normals and the positions differ in number");
    }

    if (colors != nullptr && colors->size() != positions.size()) {
        return fail("Draco: the colors and the positions differ in number");
    }

    draco::Mesh mesh;
    mesh.set_num_points(static_cast<std::uint32_t>(point_count));
    draco::GeometryAttribute position_attribute;
    position_attribute.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
    const int position_attribute_id = mesh.AddAttribute(position_attribute, true, static_cast<std::uint32_t>(point_count));
    if (position_attribute_id < 0) {
        return fail("Draco: the position attribute could not be added");
    }
    int normal_attribute_id = -1;
    if (normals != nullptr) {
        draco::GeometryAttribute normal_attribute;
        normal_attribute.Init(draco::GeometryAttribute::NORMAL, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
        normal_attribute_id = mesh.AddAttribute(normal_attribute, true, static_cast<std::uint32_t>(point_count));
        if (normal_attribute_id < 0) {
            return fail("Draco: the normal attribute could not be added");
        }
    }
    int color_attribute_id = -1;
    if (colors != nullptr) {
        draco::GeometryAttribute color_attribute;
        // Floats, not bytes: CesiumJS decodes a Draco color of bytes wrongly (noise of red, green and blue), and a float color is right in it and in X3DOM.
        color_attribute.Init(draco::GeometryAttribute::COLOR, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
        color_attribute_id = mesh.AddAttribute(color_attribute, true, static_cast<std::uint32_t>(point_count));
        if (color_attribute_id < 0) {
            return fail("Draco: the color attribute could not be added");
        }
    }
    for (std::size_t n = 0; n < point_count; n++) {
        if (color_attribute_id >= 0) {
            const float rgb[3] = {(*colors)[3 * n] / 255.0f, (*colors)[3 * n + 1] / 255.0f, (*colors)[3 * n + 2] / 255.0f};
            mesh.attribute(color_attribute_id)->SetAttributeValue(draco::AttributeValueIndex(static_cast<std::uint32_t>(n)), rgb);
        }
        mesh.attribute(position_attribute_id)->SetAttributeValue(draco::AttributeValueIndex(static_cast<std::uint32_t>(n)), &positions[3 * n]);
        if (normal_attribute_id >= 0) {
            mesh.attribute(normal_attribute_id)->SetAttributeValue(draco::AttributeValueIndex(static_cast<std::uint32_t>(n)), &(*normals)[3 * n]);
        }
    }
    for (std::size_t t = 0; t + 2 < indices.size(); t += 3) {
        mesh.SetFace(draco::FaceIndex(static_cast<std::uint32_t>(t / 3)),
                     {draco::PointIndex(indices[t]), draco::PointIndex(indices[t + 1]), draco::PointIndex(indices[t + 2])});
    }

    draco::Encoder encoder;
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, position_bits);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, normal_bits);
    if (colors != nullptr) {
        encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, 10);   // a step of 1/1023 at most, finer than the bytes they come from
    }
    draco::EncoderBuffer buffer;
    const draco::Status status = encoder.EncodeMeshToBuffer(mesh, &buffer);
    if (!status.ok()) {
        return fail(std::string("Draco: encoding failed: ") + status.error_msg());
    }

    // Decoded again, to check it and to count what it holds.
    draco::DecoderBuffer decoder_buffer;
    decoder_buffer.Init(buffer.data(), buffer.size());
    draco::Decoder decoder;
    auto decoded = decoder.DecodeMeshFromBuffer(&decoder_buffer);
    if (!decoded.ok()) {
        return fail(std::string("Draco: the encoded mesh does not decode: ") + decoded.status().error_msg());
    }
    const std::unique_ptr<draco::Mesh> check = std::move(decoded).value();
    const draco::PointAttribute *decoded_position = check->GetNamedAttribute(draco::GeometryAttribute::POSITION);
    if (decoded_position == nullptr) {
        return fail("Draco: the encoded mesh has no positions");
    }
    result->data.assign(buffer.data(), buffer.data() + buffer.size());
    result->position_id = decoded_position->unique_id();
    const draco::PointAttribute *decoded_normal = check->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
    result->normal_id = decoded_normal != nullptr ? static_cast<int>(decoded_normal->unique_id()) : -1;
    const draco::PointAttribute *decoded_color = check->GetNamedAttribute(draco::GeometryAttribute::COLOR);
    result->color_id = decoded_color != nullptr ? static_cast<int>(decoded_color->unique_id()) : -1;
    result->point_count = check->num_points();
    result->face_count = check->num_faces();
    return true;
}

bool decode(const std::vector<unsigned char> &data, std::vector<float> *positions, std::vector<std::uint32_t> *indices, std::string *error) {
    draco::DecoderBuffer buffer;
    buffer.Init(reinterpret_cast<const char *>(data.data()), data.size());
    draco::Decoder decoder;
    auto decoded = decoder.DecodeMeshFromBuffer(&buffer);
    if (!decoded.ok()) {
        if (error != nullptr) {
            *error = decoded.status().error_msg();
        }
        return false;
    }
    const std::unique_ptr<draco::Mesh> mesh = std::move(decoded).value();
    const draco::PointAttribute *attribute = mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
    if (attribute == nullptr) {
        return false;
    }
    positions->clear();
    for (draco::PointIndex point(0); point < mesh->num_points(); point++) {
        float value[3];
        attribute->GetMappedValue(point, value);
        positions->insert(positions->end(), value, value + 3);
    }
    indices->clear();
    for (draco::FaceIndex face(0); face < mesh->num_faces(); face++) {
        for (int k = 0; k < 3; k++) {
            indices->push_back(mesh->face(face)[k].value());
        }
    }
    return true;
}

bool add_primitive(tinygltf::Model *model, tinygltf::Mesh *mesh, std::vector<unsigned char> *buffer, const std::vector<float> &positions,
                   const std::vector<float> *normals, const std::vector<std::uint32_t> &indices, const Settings &settings, int material,
                   Compression *used, std::string *reason, const std::vector<std::uint8_t> *colors) {
    if (model == nullptr || mesh == nullptr || buffer == nullptr || positions.empty() || positions.size() % 3 != 0) {
        if (reason != nullptr) {
            *reason = "Draco: no mesh to add";
        }
        return false;
    }
    double minimum[3] = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    double maximum[3] = {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
    for (std::size_t n = 0; n + 2 < positions.size(); n += 3) {
        for (int k = 0; k < 3; k++) {
            minimum[k] = std::min(minimum[k], static_cast<double>(positions[n + k]));
            maximum[k] = std::max(maximum[k], static_cast<double>(positions[n + k]));
        }
    }
    const double range = std::max({maximum[0] - minimum[0], maximum[1] - minimum[1], maximum[2] - minimum[2]});
    const int bits = position_bits(range, settings.precision);
    Encoded encoded;
    if (!encode(positions, normals, indices, bits, settings.normal_bits, &encoded, reason, colors)) {
        return false;
    }

    // The stream goes to the shared buffer, at a multiple of four bytes; the caller puts the buffer in the model once the last
    // mesh is added.
    while (buffer->size() % 4 != 0) {
        buffer->push_back(0);
    }
    tinygltf::BufferView view;
    view.buffer = 0;
    view.byteOffset = buffer->size();
    view.byteLength = encoded.data.size();
    buffer->insert(buffer->end(), encoded.data.begin(), encoded.data.end());
    model->bufferViews.push_back(std::move(view));
    const int view_index = static_cast<int>(model->bufferViews.size()) - 1;

    // The accessors have no buffer views: a viewer that does not know the extension finds nothing, and a viewer that does
    // takes the counts and the types from them.
    tinygltf::Accessor position_accessor;
    position_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    position_accessor.count = encoded.point_count;
    position_accessor.type = TINYGLTF_TYPE_VEC3;
    position_accessor.minValues = {minimum[0], minimum[1], minimum[2]};
    position_accessor.maxValues = {maximum[0], maximum[1], maximum[2]};
    model->accessors.push_back(std::move(position_accessor));
    const int position_index = static_cast<int>(model->accessors.size()) - 1;

    int normal_index = -1;
    if (encoded.normal_id >= 0) {
        tinygltf::Accessor normal_accessor;
        normal_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        normal_accessor.count = encoded.point_count;
        normal_accessor.type = TINYGLTF_TYPE_VEC3;
        model->accessors.push_back(std::move(normal_accessor));
        normal_index = static_cast<int>(model->accessors.size()) - 1;
    }

    int color_index = -1;
    if (encoded.color_id >= 0) {
        tinygltf::Accessor color_accessor;
        color_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        color_accessor.normalized = false;
        color_accessor.count = encoded.point_count;
        color_accessor.type = TINYGLTF_TYPE_VEC3;
        model->accessors.push_back(std::move(color_accessor));
        color_index = static_cast<int>(model->accessors.size()) - 1;
    }

    tinygltf::Accessor index_accessor;
    index_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    index_accessor.count = 3 * encoded.face_count;
    index_accessor.type = TINYGLTF_TYPE_SCALAR;
    model->accessors.push_back(std::move(index_accessor));
    const int index_index = static_cast<int>(model->accessors.size()) - 1;

    tinygltf::Primitive primitive;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    primitive.attributes["POSITION"] = position_index;
    if (normal_index >= 0) {
        primitive.attributes["NORMAL"] = normal_index;
    }
    if (color_index >= 0) {
        primitive.attributes["COLOR_0"] = color_index;
    }
    primitive.indices = index_index;
    primitive.material = material;

    std::map<std::string, tinygltf::Value> attributes;
    attributes["POSITION"] = tinygltf::Value(encoded.position_id);
    if (encoded.normal_id >= 0) {
        attributes["NORMAL"] = tinygltf::Value(encoded.normal_id);
    }
    if (encoded.color_id >= 0) {
        attributes["COLOR_0"] = tinygltf::Value(encoded.color_id);
    }
    std::map<std::string, tinygltf::Value> extension;
    extension["bufferView"] = tinygltf::Value(view_index);
    extension["attributes"] = tinygltf::Value(attributes);
    primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(extension);
    mesh->primitives.push_back(std::move(primitive));

    for (std::vector<std::string> *list : {&model->extensionsUsed, &model->extensionsRequired}) {
        if (std::find(list->begin(), list->end(), "KHR_draco_mesh_compression") == list->end()) {
            list->push_back("KHR_draco_mesh_compression");
        }
    }

    if (used != nullptr) {
        const double step = range / (std::pow(2.0, bits) - 1.0);
        used->present = true;
        used->position_bits = std::max(used->position_bits, bits);
        used->position_step = std::max(used->position_step, step);
        used->normal_bits = encoded.normal_id >= 0 ? settings.normal_bits : used->normal_bits;
    }
    return true;
}

} // namespace mbdraco

#else

namespace mbdraco {

bool available() {
    return false;
}

bool encode(const std::vector<float> &, const std::vector<float> *, const std::vector<std::uint32_t> &, int, int, Encoded *, std::string *error,
            const std::vector<std::uint8_t> *) {
    if (error != nullptr) {
        *error = "this build does not include Draco";
    }
    return false;
}

bool decode(const std::vector<unsigned char> &, std::vector<float> *, std::vector<std::uint32_t> *, std::string *error) {
    if (error != nullptr) {
        *error = "this build does not include Draco";
    }
    return false;
}

bool add_primitive(tinygltf::Model *, tinygltf::Mesh *, std::vector<unsigned char> *, const std::vector<float> &, const std::vector<float> *,
                   const std::vector<std::uint32_t> &, const Settings &, int, Compression *, std::string *reason, const std::vector<std::uint8_t> *) {
    if (reason != nullptr) {
        *reason = "this build does not include Draco";
    }
    return false;
}

} // namespace mbdraco

#endif
