/*--------------------------------------------------------------------
 *    The MB-system:	model.cpp	5/11/2023
 *
 *    Copyright (c) 2023-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
 /*
  *    The program MBgrd2gltf, including this source file, was created
  *    by a Capstone Project team at the California State University
  *    Monterey Bay (CSUMB) including Kyle Dowling, Julian Fortin,
  *    Jesse Benavides, Nicolas Porras Falconio. This team was mentored by:
  *    Mike McCann
  *      Monterey Bay Aquarium Research Institute
  *      Moss Landing, California, USA
  *--------------------------------------------------------------------*/

#include "model.h"


  // external libraries
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"

namespace mbgrd2gltf {
	namespace model {
		std::vector<float> get_vertex_buffer(const Matrix<Vertex>& vertices) {
			std::vector<float> out;

			out.reserve(vertices.count() * 3);

			for (size_t i = 0; i < vertices.count(); ++i) {
				const auto& vertex = vertices[i];

				if (vertex.is_valid()) {
					size_t index = out.size();

					out.resize(index + 3);
					out[index] = vertex.x();
					out[index + 1] = vertex.y();
					out[index + 2] = vertex.z();
				}
			}

			return out;
		}

		std::vector<uint32_t> get_index_buffer(const std::vector<Triangle>& triangles) {
			size_t index = 0;
			std::vector<uint32_t> out(triangles.size() * 3);

			for (const Triangle& triangle : triangles) {
				out[index] = triangle.a();
				out[index + 1] = triangle.b();
				out[index + 2] = triangle.c();
				index += 3;
			}

			return out;
		}

		tinygltf::Buffer get_buffer(const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer) {
			tinygltf::Buffer out;

			size_t buffer_size = index_buffer.size() * 4 + vertex_buffer.size() * 4;

			out.data.resize(buffer_size);

			auto& data = out.data;

			uint32_t* indices_ptr = (uint32_t*)&data[0];

			for (size_t i = 0; i < index_buffer.size(); ++i)
				indices_ptr[i] = index_buffer[i];

			float* vertices_ptr = (float*)&data[index_buffer.size() * 4];

			for (size_t i = 0; i < vertex_buffer.size(); ++i)
				vertices_ptr[i] = vertex_buffer[i];

			return out;
		}

		tinygltf::BufferView get_vertex_buffer_view(const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer) {
			tinygltf::BufferView out;

			out.buffer = 0;
			out.byteOffset = index_buffer.size() * 4;
			out.byteLength = vertex_buffer.size() * 4;
			out.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			return out;
		}

		tinygltf::BufferView get_index_buffer_view(const std::vector<uint32_t>& index_buffer) {
			tinygltf::BufferView out;

			out.buffer = 0;
			out.byteOffset = 0;
			out.byteLength = index_buffer.size() * 4;
			out.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

			return out;
		}

		std::vector<double> get_vertex_mins(const std::vector<float>& vertex_buffer) {
			float x_min = vertex_buffer[0];
			float y_min = vertex_buffer[1];
			float z_min = vertex_buffer[2];

			for (size_t i = 3; i < vertex_buffer.size(); i += 3) {
				if (vertex_buffer[i] < x_min)
					x_min = vertex_buffer[i];

				if (vertex_buffer[i + 1] < y_min)
					y_min = vertex_buffer[i + 1];

				if (vertex_buffer[i + 2] < z_min)
					z_min = vertex_buffer[i + 2];
			}

			return {x_min, y_min, z_min};
		}

		std::vector<double> get_vertex_maxes(const std::vector<float>& vertex_buffer) {
			float x_max = vertex_buffer[0];
			float y_max = vertex_buffer[1];
			float z_max = vertex_buffer[2];

			for (size_t i = 3; i < vertex_buffer.size(); i += 3) {
				if (vertex_buffer[i] > x_max)
					x_max = vertex_buffer[i];

				if (vertex_buffer[i + 1] > y_max)
					y_max = vertex_buffer[i + 1];

				if (vertex_buffer[i + 2] > z_max)
					z_max = vertex_buffer[i + 2];
			}

			return {x_max, y_max, z_max};
		}

		tinygltf::Accessor get_vertex_accessor(const std::vector<float>& vertex_buffer) {
			tinygltf::Accessor out;

			out.bufferView = 1;
			out.byteOffset = 0;
			out.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			out.count = vertex_buffer.size() / 3;
			out.type = TINYGLTF_TYPE_VEC3;
			out.maxValues = get_vertex_maxes(vertex_buffer);
			out.minValues = get_vertex_mins(vertex_buffer);

			return out;
		}

		tinygltf::Accessor get_index_accessor(const std::vector<uint32_t> index_buffer, size_t vertex_count) {
			tinygltf::Accessor out;

			out.bufferView = 0;
			out.byteOffset = 0;
			out.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
			out.count = index_buffer.size();
			out.type = TINYGLTF_TYPE_SCALAR;
			out.maxValues = {(double)(vertex_count - 1)};
			out.minValues = {0.0};

			return out;
		}

		tinygltf::Primitive get_primitive() {
			tinygltf::Primitive out;

			out.indices = 0;
			out.attributes["POSITION"] = 1;
			out.material = 0;
			out.mode = TINYGLTF_MODE_TRIANGLES;

			return out;
		}


		void EncodeGeometryWithDraco(const std::vector<float>& vertices, const std::vector<int>& faces,
			std::vector<char>& compressed_data) {
			// Step 1: Create a Draco mesh
			draco::Mesh mesh;

			// Step 2: Prepare and add vertices to the Draco mesh
			const int num_vertices = vertices.size() / 3;
			draco::PointCloud& pc = mesh;
			draco::GeometryAttribute pos_att;
			/*
			// The attribute id is set to 0, which is the default attribute id for position attribute.
			// The attribute id is used to identify the attribute in the mesh.
			// The number of components in the attribute is 3, which is the number of components in the position attribute (x, y, z).
			// The data type of the attribute is set to DT_FLOAT32, which is the data type of the position attribute.
			// The attribute is not quantized, so the normalized flag is set to false.
			// The byte stride is set to 0, which is the default value for the byte stride.
			// The byte offset is set to 0, which is the default value for the byte offset.
			*/
			pos_att.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, 0, 0);
			const uint32_t pos_att_id = pc.AddAttribute(pos_att, true, num_vertices);

			for (int i = 0; i < num_vertices; ++i) {
				const float* const vertex = &vertices[i * 3];
				pc.attribute(pos_att_id)->SetAttributeValue(draco::AttributeValueIndex(i), vertex);
			}

			// Step 3: Add faces to the Draco mesh
			const int num_faces = faces.size() / 3;
			for (int i = 0; i < num_faces; ++i) {
				mesh.AddFace({draco::PointIndex(faces[i * 3]), draco::PointIndex(faces[i * 3 + 1]), draco::PointIndex(faces[i * 3 + 2])});
			}

			// Step 4: Create a Draco encoder
			draco::Encoder encoder;

			// Step 5: Set the desired encoding quantization
			// The quantization bit is the number of bits used to store the quantized value.
			// The quantization bit can be set to 14, 16, 18, 20, 22, 24, 26, 28, 30, or 32.
			// The default value for the quantization bit is 14.
			encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);

			// Step 6: Encode the geometry data
			// The encoded data will be stored in the buffer
			draco::EncoderBuffer buffer;
			const draco::Status status = encoder.EncodeMeshToBuffer(mesh, &buffer);
			// Check if the encoding was successful
			if (!status.ok()) {
				std::cerr << "Encoding failed: " << status.error_msg() << std::endl;
				return;
			}
			// Step 7: Copy the encoded data to the output buffer 
			// The encoded data is stored in the buffer.data() and buffer.size() is the size of the encoded data
			compressed_data.resize(buffer.size());
			std::memcpy(compressed_data.data(), buffer.data(), buffer.size());
		}

		void write_gltf(const Geometry& geometry, const Options& options) {
			std::string output_filepath = options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");

			tinygltf::Model model;
			tinygltf::Mesh mesh;
			tinygltf::Node node;
			node.mesh = 0;

			tinygltf::Scene scene;
			scene.nodes = {0};

			if (options.is_draco_compressed()) {
				// Encode geometry with Draco if the option is enabled
				std::vector<char> draco_compressed_data;
				EncodeGeometryWithDraco(geometry.ConvertMatrixToVector(geometry.vertices()), geometry.ConvertTrianglesToVector(geometry.triangles()), draco_compressed_data);

				tinygltf::Buffer buffer;
				buffer.data.assign(draco_compressed_data.begin(), draco_compressed_data.end());
				buffer.name = "Draco_Compressed";

				tinygltf::BufferView bufferView;
				bufferView.buffer = 0;
				bufferView.byteOffset = 0;
				bufferView.byteLength = buffer.data.size();
				bufferView.target = 0;

				tinygltf::Accessor accessor;
				accessor.bufferView = 0;
				accessor.byteOffset = 0;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
				accessor.count = buffer.data.size();
				accessor.type = TINYGLTF_TYPE_SCALAR;

				tinygltf::Primitive primitive;
				primitive.indices = 0;
				primitive.attributes = {};
				// Assuming you have a bufferViewIndex that you want to assign to the extension
				int bufferViewIndex = 0; // Example index

				// Create the extension as a map (similar to a JSON object)
				tinygltf::Value::Object dracoExtension;

				// Assign the bufferView index to the extension
				// Here, we're directly working with tinygltf::Value::Object, which is essentially a map
				dracoExtension["bufferView"] = tinygltf::Value(bufferViewIndex);

				// Now, create a tinygltf::Value from the extension map and assign it to the primitive's extensions
				primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(dracoExtension);

				mesh.primitives = {primitive};
				model.buffers = {buffer};
				model.bufferViews = {bufferView};
				model.accessors = {accessor};
			}
			else {
				// Proceed with standard GLTF creation if Draco compression is not enabled
				std::vector<float> vertex_buffer = get_vertex_buffer(geometry.vertices());
				std::vector<uint32_t> index_buffer = get_index_buffer(geometry.triangles());

				mesh.primitives = {get_primitive()};

				model.buffers = {get_buffer(vertex_buffer, index_buffer)};
				model.bufferViews = {get_index_buffer_view(index_buffer), get_vertex_buffer_view(vertex_buffer, index_buffer)};
				model.accessors = {get_index_accessor(index_buffer, vertex_buffer.size() / 3), get_vertex_accessor(vertex_buffer)};
			}

			model.scenes = {scene};
			model.meshes = {mesh};
			model.nodes = {node};
			model.asset.version = "2.0";
			model.asset.generator = "tinygltf";
			model.materials = {tinygltf::Material()};

			tinygltf::TinyGLTF gltf;
			gltf.WriteGltfSceneToFile(&model, output_filepath, false, true, true, options.is_binary_output());
		}
	}
}
