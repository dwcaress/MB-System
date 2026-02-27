#ifndef GLTF_WRITER_H
#define GLTF_WRITER_H

#include <string>
#include <vector>
#include <cstdint>

struct GeoOrigin {
	double ecef_x, ecef_y, ecef_z;
};

class GLTFWriter {
  public:
	void write_gltf(const std::string &output_path, const std::vector<float> &vertices, const std::vector<uint32_t> &indices,
	                const GeoOrigin &geo_origin);

  private:
	std::string create_gltf_json(size_t vertex_count, size_t index_count, size_t vertex_buffer_size, size_t index_buffer_size,
	                             size_t vertex_offset, size_t index_offset, const GeoOrigin &geo_origin);
	void encode_buffers(const std::vector<float> &vertices, const std::vector<uint32_t> &indices,
	                    std::vector<uint8_t> &buffer_data, size_t &vertex_offset, size_t &index_offset);
};

#endif
