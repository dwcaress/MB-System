#include "gltf_writer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

void GLTFWriter::write_gltf(const std::string &output_path, const std::vector<float> &vertices,
                            const std::vector<uint32_t> &indices, const GeoOrigin &geo_origin) {
	std::cout << "DEBUG: Writing glTF file to " << output_path << std::endl;

	std::vector<uint8_t> buffer_data;
	size_t vertex_offset, index_offset;
	encode_buffers(vertices, indices, buffer_data, vertex_offset, index_offset);

	size_t vertex_bytes = vertices.size() * sizeof(float);
	size_t index_bytes = indices.size() * sizeof(uint32_t);
	std::string json =
	    create_gltf_json(vertices.size() / 3, indices.size(), vertex_bytes, index_bytes, vertex_offset, index_offset, geo_origin);

	// Pad JSON to 4-byte boundary with spaces
	size_t json_padding = (4 - (json.size() % 4)) % 4;
	for (size_t i = 0; i < json_padding; i++) {
		json += " ";
	}

	std::ofstream file(output_path, std::ios::binary);
	if (!file)
		throw std::runtime_error("Failed to open output file: " + output_path);

	uint32_t magic = 0x46546C67;
	uint32_t version = 2;
	uint32_t file_size = 12 + 8 + json.size() + 8 + buffer_data.size();

	file.write(reinterpret_cast<const char *>(&magic), 4);
	file.write(reinterpret_cast<const char *>(&version), 4);
	file.write(reinterpret_cast<const char *>(&file_size), 4);

	uint32_t json_length = json.size();
	uint32_t json_type = 0x4E4F534A;
	file.write(reinterpret_cast<const char *>(&json_length), 4);
	file.write(reinterpret_cast<const char *>(&json_type), 4);
	file.write(json.c_str(), json.size());

	uint32_t bin_length = buffer_data.size();
	uint32_t bin_type = 0x004E4942;
	file.write(reinterpret_cast<const char *>(&bin_length), 4);
	file.write(reinterpret_cast<const char *>(&bin_type), 4);
	file.write(reinterpret_cast<const char *>(buffer_data.data()), buffer_data.size());

	file.close();
	std::cout << "DEBUG: Successfully wrote glTF file (" << file_size << " bytes)" << std::endl;
}

std::string GLTFWriter::create_gltf_json(size_t vertex_count, size_t index_count, size_t vertex_buffer_size,
                                         size_t index_buffer_size, size_t vertex_offset, size_t index_offset,
                                         const GeoOrigin &geo_origin) {
	std::ostringstream json;
	json << std::fixed << std::setprecision(3);

	json << "{\n"
	     << "  \"asset\": { \"version\": \"2.0\" },\n"
	     << "  \"extensions\": {\n"
	     << "    \"EXT_structural_metadata\": {\n"
	     << "      \"GeoOrigin\": {\n"
	     << "        \"cartesian\": [" << geo_origin.ecef_x << ", " << geo_origin.ecef_y << ", " << geo_origin.ecef_z << "]\n"
	     << "      }\n"
	     << "    }\n"
	     << "  },\n"
	     << "  \"extensionsUsed\": [\"EXT_structural_metadata\"],\n"
	     << "  \"scene\": 0,\n"
	     << "  \"scenes\": [{ \"nodes\": [0] }],\n"
	     << "  \"nodes\": [{ \"mesh\": 0 }],\n"
	     << "  \"meshes\": [{\n"
	     << "    \"primitives\": [{\n"
	     << "      \"attributes\": { \"POSITION\": 0 },\n"
	     << "      \"indices\": 1,\n"
	     << "      \"mode\": 4\n"
	     << "    }]\n"
	     << "  }],\n"
	     << "  \"accessors\": [\n"
	     << "    {\n"
	     << "      \"bufferView\": 0,\n"
	     << "      \"componentType\": 5126,\n"
	     << "      \"count\": " << vertex_count << ",\n"
	     << "      \"type\": \"VEC3\"\n"
	     << "    },\n"
	     << "    {\n"
	     << "      \"bufferView\": 1,\n"
	     << "      \"componentType\": 5125,\n"
	     << "      \"count\": " << index_count << ",\n"
	     << "      \"type\": \"SCALAR\"\n"
	     << "    }\n"
	     << "  ],\n"
	     << "  \"bufferViews\": [\n"
	     << "    { \"buffer\": 0, \"byteOffset\": " << vertex_offset << ", \"byteLength\": " << vertex_buffer_size << " },\n"
	     << "    { \"buffer\": 0, \"byteOffset\": " << index_offset << ", \"byteLength\": " << index_buffer_size << " }\n"
	     << "  ],\n"
	     << "  \"buffers\": [{ \"byteLength\": " << (index_offset + index_buffer_size) << " }]\n"
	     << "}\n";

	return json.str();
}

void GLTFWriter::encode_buffers(const std::vector<float> &vertices, const std::vector<uint32_t> &indices,
                                std::vector<uint8_t> &buffer_data, size_t &vertex_offset, size_t &index_offset) {
	size_t vertex_bytes = vertices.size() * sizeof(float);
	size_t index_bytes = indices.size() * sizeof(uint32_t);

	// Vertex data starts at offset 0
	vertex_offset = 0;

	// Calculate index offset with 4-byte alignment
	size_t after_vertices = vertex_bytes;
	size_t padding_needed = (4 - (after_vertices % 4)) % 4;
	index_offset = after_vertices + padding_needed;

	// Allocate buffer with padding
	buffer_data.resize(index_offset + index_bytes);

	// Copy vertex data
	std::memcpy(buffer_data.data(), vertices.data(), vertex_bytes);

	// Padding is already zero-initialized by resize

	// Copy index data at aligned offset
	std::memcpy(buffer_data.data() + index_offset, indices.data(), index_bytes);
}
