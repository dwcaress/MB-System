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

		/*
		* Encode the geometry using Draco
		* @param vertex_buffer : The vertex buffer to encode
		* @param index_buffer : The index buffer to encode
		* @param options : The options to use for the encoding
		* @param out : The output buffer to store the encoded data
		* @return bool : True if the encoding was successful, false otherwise
		*/
		bool dracoEncodeGeometry(const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer, const Options& options,
			std::vector<unsigned char>& out) {
			if (vertex_buffer.empty() || index_buffer.empty())
				return false;

			draco::Mesh mesh;
			const size_t num_vertices = vertex_buffer.size() / 3;
			mesh.set_num_points(num_vertices);

			draco::PointCloud& pc = mesh;
			draco::GeometryAttribute pos_att;

			// Initialize the attribute with the correct type and number of components
			// The attribute is of type POSITION and has 3 components (x, y, z)
			pos_att.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, sizeof(float) * 3, 0);

			// Add the attribute to the mesh
			const uint32_t pos_att_id = pc.AddAttribute(pos_att, true, num_vertices);

			// Check if the attribute was added successfully
			if (pos_att_id == -1) {
				std::cerr << "Failed adding position attribute to the mesh." << std::endl;
				return false;
			}
			auto* attribute = pc.attribute(pos_att_id);
			// Add the vertices to the Draco mesh
			for (size_t i = 0; i < vertex_buffer.size(); i += 3) {
				const float pos[3] = {vertex_buffer[i], vertex_buffer[i + 1], vertex_buffer[i + 2]};
				attribute->SetAttributeValue(draco::AttributeValueIndex(i / 3), pos);
			}

			for (size_t i = 0; i < index_buffer.size(); i += 3) {
				const draco::Mesh::Face face = {
					draco::PointIndex(index_buffer[i]),
					draco::PointIndex(index_buffer[i + 1]),
					draco::PointIndex(index_buffer[i + 2])
				};
				mesh.SetFace(draco::FaceIndex(i / 3), face);
			}
			// Encode the mesh using Draco, return false if the encoding fails
			draco::Encoder encoder;
			encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, options.draco_quantization());
			draco::EncoderBuffer buffer;
			if (!encoder.EncodeMeshToBuffer(mesh, &buffer).ok()) {
				std::cerr << "Encoding failed." << std::endl;
				return false;
			}

			out.assign(buffer.data(), buffer.data() + buffer.size());
			return true;
		}

		/*
		* Compress the geometry using Draco and add the compressed data to the model
		* @param model : The model to add the compressed data to
		* @param geometry : The geometry to compress
		* @param options : The options to use for the compression
		* @param vertex_buffer : The vertex buffer to compress
		* @param index_buffer : The index buffer to compress
		* @return bool : True if the compression was successful, false otherwise
		*/
		bool dracoCompressed(tinygltf::Model& model, const Geometry& geometry, const Options& options, const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer) {
			// Check if the geometry is empty or if the Draco compression is disabled or invalid (quantization value)
			if (!options.is_draco_compressed() || vertex_buffer.empty() || index_buffer.empty() || options.draco_quantization() <= 0 || options.draco_quantization() > 30)
				return false;

			// Encode the geometry using Draco && add the compressed data to the model buffer.
			tinygltf::Buffer buffer;
			std::vector<unsigned char> compressed_data;
			if (!dracoEncodeGeometry(vertex_buffer, index_buffer, options, compressed_data) || compressed_data.empty()) {
				std::cerr << "Failed to encode geometry using Draco. Falling back to regular GLTF format." << std::endl;
				return false;
			}

			buffer.data.assign(compressed_data.begin(), compressed_data.end());
			model.buffers.push_back(std::move(buffer));
			// Prepare the buffer view for the compressed data
			tinygltf::BufferView bufferView;
			bufferView.buffer = 0;  // Reference to the first buffer
			bufferView.byteOffset = 0;
			bufferView.byteLength = static_cast<size_t>(compressed_data.size());
			model.bufferViews.push_back(std::move(bufferView));
			// Prepare the index and vertex accessors
			tinygltf::Accessor indexAccessor;
			indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT; // 5125
			indexAccessor.count = index_buffer.size(); // Count of indices
			indexAccessor.type = TINYGLTF_TYPE_SCALAR; // SCALAR
			model.accessors.push_back(std::move(indexAccessor));

			tinygltf::Accessor vertexAccessor;
			vertexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // 5126
			vertexAccessor.count = vertex_buffer.size() / 3; // Count of side
			vertexAccessor.type = TINYGLTF_TYPE_VEC3; // VEC3
			vertexAccessor.maxValues = get_vertex_maxes(vertex_buffer);
			vertexAccessor.minValues = get_vertex_mins(vertex_buffer);
			model.accessors.push_back(std::move(vertexAccessor));

			// Prepare the primitive for the model
			tinygltf::Primitive primitive;
			primitive.mode = TINYGLTF_MODE_TRIANGLES;
			primitive.attributes["POSITION"] = static_cast<int>(model.accessors.size()) - 1;
			primitive.indices = static_cast<int>(model.accessors.size()) - 2;
			primitive.material = 0;

			// Initialize the primitive extensions map for Draco compression
			std::map<std::string, tinygltf::Value> dracoExtension;
			int bufferViewIndex = static_cast<int>(model.bufferViews.size()) - 1;
			dracoExtension["bufferView"] = tinygltf::Value(bufferViewIndex);

			// Map to hold attribute indices
			std::map<std::string, tinygltf::Value> attributes;
			attributes["POSITION"] = tinygltf::Value(0);
			dracoExtension["attributes"] = tinygltf::Value(attributes);

			// Add the Draco extension to the primitive's extensions map
			primitive.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(dracoExtension);
			// Add the prepared primitive to the model
			tinygltf::Mesh newMesh;
			newMesh.primitives.push_back(std::move(primitive));
			model.meshes.push_back(std::move(newMesh));
			// Add the required and used extensions to the model
			model.extensionsUsed.push_back("KHR_draco_mesh_compression");
			model.extensionsRequired.push_back("KHR_draco_mesh_compression");

			return true;
		}

		/*
		* Write the geometry to a GLTF file using the given options
		* Draco compression is used if the options specify it
		* @param geometry : The geometry to write
		* @param options : The options to use for writing the file
		*/
		void write_gltf(const Geometry& geometry, const Options& options) {
			const std::vector<float> vertex_buffer = get_vertex_buffer(geometry.vertices());
			const std::vector<uint32_t> index_buffer = get_index_buffer(geometry.triangles());
			std::string output_filepath = options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");
			tinygltf::Model model;

			if (!dracoCompressed(model, geometry, options, vertex_buffer, index_buffer)) {
				tinygltf::Buffer buffer = get_buffer(vertex_buffer, index_buffer);
				model.buffers.push_back(std::move(buffer));
				model.bufferViews.push_back(get_index_buffer_view(index_buffer));
				model.bufferViews.push_back(get_vertex_buffer_view(vertex_buffer, index_buffer));
				model.accessors.push_back(get_index_accessor(index_buffer, vertex_buffer.size() / 3));
				model.accessors.push_back(get_vertex_accessor(vertex_buffer));
				tinygltf::Mesh mesh;
				mesh.primitives.push_back(get_primitive());
				model.meshes.push_back(std::move(mesh));
			}

			// Set up the material
			tinygltf::Material material;
			material.doubleSided = true;
			model.materials.push_back(material);

			// Set up the scene and node
			tinygltf::Scene scene;
			tinygltf::Node node;
			node.mesh = static_cast<int>(model.meshes.size()) - 1;
			model.nodes.push_back(node);
			scene.nodes.push_back(static_cast<int>(model.nodes.size()) - 1);
			model.scenes.push_back(scene);
			model.defaultScene = 0;
			model.asset.version = "2.0";
			model.asset.generator = "tinygltf";

			tinygltf::TinyGLTF gltf;
			if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true, true, options.is_binary_output())) {
				std::cerr << "Failed to write GLTF file." << std::endl;
			}
		}

	}
}
