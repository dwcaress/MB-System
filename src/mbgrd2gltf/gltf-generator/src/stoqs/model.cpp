#include <stoqs/model.h>

// external libraries
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

namespace stoqs
{
	namespace model
	{
		std::vector<float> get_vertex_buffer(const Matrix<Vertex>& vertices)
		{
			std::vector<float> out;

			out.reserve(vertices.count() * 3);
			
			for (size_t i = 0; i < vertices.count(); ++i)
			{
				const auto& vertex = vertices[i];

				if (vertex.is_valid())
				{
					size_t index = out.size();

					out.resize(index + 3);
					out[index] = vertex.x();
					out[index + 1] = vertex.y();
					out[index + 2] = vertex.z();
				}
			}

			return out;
		}

		std::vector<uint32_t> get_index_buffer(const std::vector<Triangle>& triangles)
		{
			size_t index = 0;
			std::vector<uint32_t> out(triangles.size() * 3);

			for (const Triangle& triangle : triangles)
			{
				out[index] = triangle.a();
				out[index + 1] = triangle.b();
				out[index + 2] = triangle.c();
				index  += 3;
			}

			return out;
		}

		tinygltf::Buffer get_buffer(const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer)
		{
			tinygltf::Buffer out;
			
			size_t buffer_size = index_buffer.size() * 4 + vertex_buffer.size() * 4; 

			out.data.resize(buffer_size);

			auto& data = out.data;

			uint32_t *indices_ptr = (uint32_t*)&data[0];

			for (size_t i = 0; i < index_buffer.size(); ++i)
				indices_ptr[i] = index_buffer[i];

			float *vertices_ptr = (float*)&data[index_buffer.size() * 4];

			for (size_t i = 0; i < vertex_buffer.size(); ++i)
				vertices_ptr[i] = vertex_buffer[i];

			return out;
		}

		tinygltf::BufferView get_vertex_buffer_view(const std::vector<float>& vertex_buffer, const std::vector<uint32_t>& index_buffer)
		{
			tinygltf::BufferView out;

			out.buffer = 0;
			out.byteOffset = index_buffer.size() * 4;
			out.byteLength = vertex_buffer.size() * 4;
			out.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			return out;
		}

		tinygltf::BufferView get_index_buffer_view(const std::vector<uint32_t>& index_buffer)
		{
			tinygltf::BufferView out;

			out.buffer = 0;
			out.byteOffset = 0;
			out.byteLength = index_buffer.size() * 4;
			out.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

			return out;
		}

		std::vector<double> get_vertex_mins(const std::vector<float>& vertex_buffer)
		{
			float x_min = vertex_buffer[0];
			float y_min = vertex_buffer[1];
			float z_min = vertex_buffer[2];
			
			for (size_t i = 3; i < vertex_buffer.size(); i += 3)
			{
				if (vertex_buffer[i] < x_min)
					x_min = vertex_buffer[i];

				if (vertex_buffer[i + 1] < y_min)
					y_min = vertex_buffer[i + 1];

				if (vertex_buffer[i + 2] < z_min)
					z_min = vertex_buffer[i + 2];
			}

			return { x_min, y_min, z_min };
		}

		std::vector<double> get_vertex_maxes(const std::vector<float>& vertex_buffer)
		{
			float x_max = vertex_buffer[0];
			float y_max = vertex_buffer[1];
			float z_max = vertex_buffer[2];

			for (size_t i = 3; i < vertex_buffer.size(); i += 3)
			{
				if (vertex_buffer[i] > x_max)
					x_max = vertex_buffer[i];

				if (vertex_buffer[i + 1] > y_max)
					y_max = vertex_buffer[i + 1];

				if (vertex_buffer[i + 2] > z_max)
					z_max = vertex_buffer[i + 2];
			}

			return { x_max, y_max, z_max };
		}

		tinygltf::Accessor get_vertex_accessor(const std::vector<float>& vertex_buffer)
		{
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

		tinygltf::Accessor get_index_accessor(const std::vector<uint32_t> index_buffer, size_t vertex_count)
		{
			tinygltf::Accessor out;
			
			out.bufferView = 0;
			out.byteOffset = 0;
			out.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
			out.count = index_buffer.size();
			out.type = TINYGLTF_TYPE_SCALAR;
			out.maxValues = { (double)(vertex_count - 1) };
			out.minValues = { 0.0 };

			return out;
		}

		tinygltf::Primitive get_primitive()
		{
			tinygltf::Primitive out;

			out.indices = 0;
			out.attributes["POSITION"] = 1;
			out.material = 0;
			out.mode = TINYGLTF_MODE_TRIANGLES;

			return out;
		}

		void write_gltf(const Geometry& geometry, const Options& options)
		{
			std::string output_filepath = options.output_filepath()
				+ (options.is_binary_output() ? ".glb" : ".gltf");

			tinygltf::Mesh mesh;
			mesh.primitives =
			{
				get_primitive()
			};

			tinygltf::Node node;
			node.mesh = 0;

			tinygltf::Scene scene;
			scene.nodes = { 0 };
		
			std::vector<float> vertex_buffer = get_vertex_buffer(geometry.vertices());
			std::vector<uint32_t> index_buffer = get_index_buffer(geometry.triangles());

			tinygltf::Model model;
			model.scenes = { scene };
			model.meshes = { mesh };
			model.nodes = { node };
			model.buffers =
			{
				get_buffer(vertex_buffer, index_buffer)
			};

			model.bufferViews =
			{
				get_index_buffer_view(index_buffer),
				get_vertex_buffer_view(vertex_buffer, index_buffer)
			};

			model.accessors =
			{
				get_index_accessor(index_buffer, vertex_buffer.size() / 3),
				get_vertex_accessor(vertex_buffer)
			};

			model.asset.version = "2.0";
			model.asset.generator = "tinygltf";
			model.materials =
			{
				tinygltf::Material()
			};
			
			tinygltf::TinyGLTF gltf;

			gltf.WriteGltfSceneToFile(&model, output_filepath, false, true, true, options.is_binary_output());
		}
	}
}
