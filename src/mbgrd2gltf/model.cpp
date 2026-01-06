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
#include "logger.h"

// external libraries
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"
#include "draco/compression/encode.h"

// standard library
#include <sys/stat.h>

namespace mbgrd2gltf {
namespace model {
//New Addition: Generation in tiles
std::vector<double> get_vertex_mins(const std::vector<float>& vertex_buffer);
std::vector<double> get_vertex_maxes(const std::vector<float>& vertex_buffer);

// ---- helpers for packing & alignment ----
static inline size_t align4(size_t n) { return (n + 3) & ~size_t(3); }
template <typename T>

static void append_bytes(std::vector<unsigned char>& bin, const std::vector<T>& data) {
  if (data.empty())
    return;
  const size_t old = bin.size();
  bin.resize(old + data.size() * sizeof(T));
  std::memcpy(bin.data() + old, data.data(), data.size() * sizeof(T));
}

struct TileBuild {
  std::vector<float> verts; // x,y,z ...
  std::vector<uint16_t> idx16;
  std::vector<uint32_t> idx32;
  bool useUint32 = false;

  int bvIndices = -1;
  int bvVerts = -1;
  int accIndices = -1;
  int accVerts = -1;

  std::vector<double> vmin, vmax;
};

// Remap global vertex IDs in 'tris' to local 0..k-1 and build a tile-local vertex buffer.
// Uses the id_to_offset map to correctly locate vertices in the master buffer.
static TileBuild build_tile_buffers(const std::vector<float>& masterVerts,
                                    const std::vector<Triangle>& tris,
                                    const std::unordered_map<uint32_t, size_t>& id_to_offset) {
  TileBuild tb;
  if (tris.empty())
    return tb;

  std::unordered_map<uint32_t, uint32_t> remap;
  remap.reserve(tris.size() * 3);

  auto addVertex = [&](uint32_t gidx) -> uint32_t {
    auto it = remap.find(gidx);
    if (it != remap.end())
      return it->second;

    uint32_t lidx = static_cast<uint32_t>(tb.verts.size() / 3);
    remap[gidx] = lidx;

    // Look up the actual buffer offset for this vertex ID
    auto offset_it = id_to_offset.find(gidx);
    if (offset_it == id_to_offset.end()) {
      // This should not happen if triangles are correctly formed
      LOG_ERROR("Vertex ID", gidx, "not found in buffer map");
      tb.verts.push_back(0.0f);
      tb.verts.push_back(0.0f);
      tb.verts.push_back(0.0f);
      return lidx;
    }
    
    const size_t off = offset_it->second;
    tb.verts.push_back(masterVerts[off + 0]);
    tb.verts.push_back(masterVerts[off + 1]);
    tb.verts.push_back(masterVerts[off + 2]);
    return lidx;
  };

  // First pass discovers unique verts
  for (const auto& t : tris) {
    addVertex(t.a());
    addVertex(t.b());
    addVertex(t.c());
  }

  const uint32_t uniqueVerts = static_cast<uint32_t>(tb.verts.size() / 3);
  tb.useUint32 = (uniqueVerts > 65535u);

  // Second pass writes indices in chosen type
  if (tb.useUint32)
    tb.idx32.reserve(tris.size() * 3);
  else
    tb.idx16.reserve(tris.size() * 3);

  auto getLocal = [&](uint32_t gidx) -> uint32_t {
    auto it = remap.find(gidx);
    return (it != remap.end()) ? it->second : 0u;
  };

  for (const auto& t : tris) {
    uint32_t a = getLocal(t.a());
    uint32_t b = getLocal(t.b());
    uint32_t c = getLocal(t.c());
    if (tb.useUint32) {
      tb.idx32.push_back(a);
      tb.idx32.push_back(b);
      tb.idx32.push_back(c);
    } else {
      tb.idx16.push_back(static_cast<uint16_t>(a));
      tb.idx16.push_back(static_cast<uint16_t>(b));
      tb.idx16.push_back(static_cast<uint16_t>(c));
    }
  }

  tb.vmin = get_vertex_mins(tb.verts);
  tb.vmax = get_vertex_maxes(tb.verts);
  return tb;
}

/*
		 * Create a buffer of vertex positions from a matrix of vertices.
		 * Also creates a mapping from vertex ID to buffer position offset.
		 * @param vertices : The matrix of vertices.
		 * @param id_to_offset : Output map from vertex ID to buffer offset (in floats, divide by 3 for vertex index)
		 * @return std::vector<float> : The buffer of vertex positions.
		 */
std::vector<float> get_vertex_buffer(const Matrix<Vertex>& vertices, 
                                     std::unordered_map<uint32_t, size_t>* id_to_offset = nullptr) {
  std::vector<float> out;
  out.reserve(vertices.count() * 3); // Reserve space for 3 floats per vertex

  for (const auto& vertex : vertices) {
    if (vertex.is_valid()) {
      if (id_to_offset) {
        // Map vertex ID to its position in the buffer (offset in floats)
        (*id_to_offset)[vertex.index()] = out.size();
      }
      out.push_back(vertex.x());
      out.push_back(vertex.y());
      out.push_back(vertex.z());
    }
  }

  return out;
}

/*
		 * Create a buffer of indices from a vector of triangles.
		 * @param triangles : The vector of triangles.
		 * @return std::vector<uint32_t> : The buffer of indices.
		 */
std::vector<uint32_t> get_index_buffer(const std::vector<Triangle>& triangles) {
  std::vector<uint32_t> out(triangles.size() * 3); // Reserve space for 3 indices per triangle

  size_t index = 0;
  for (const Triangle& triangle : triangles) {
    out[index++] = triangle.a();
    out[index++] = triangle.b();
    out[index++] = triangle.c();
  }

  return out;
}

/*
		 * Combine vertex and index buffers into a single GLTF buffer.
		 * @param vertex_buffer : The buffer of vertex positions.
		 * @param index_buffer : The buffer of indices.
		 * @return tinygltf::Buffer : The combined GLTF buffer.
		 */
tinygltf::Buffer get_buffer(const std::vector<float>& vertex_buffer,
                            const std::vector<uint32_t>& index_buffer) {
  tinygltf::Buffer out;
  size_t buffer_size =
      index_buffer.size() * sizeof(uint32_t) + vertex_buffer.size() * sizeof(float);
  out.data.resize(buffer_size);

  uint32_t* indices_ptr = reinterpret_cast<uint32_t*>(out.data.data());
  std::copy(index_buffer.begin(), index_buffer.end(), indices_ptr);

  float* vertices_ptr =
      reinterpret_cast<float*>(out.data.data() + index_buffer.size() * sizeof(uint32_t));
  std::copy(vertex_buffer.begin(), vertex_buffer.end(), vertices_ptr);

  return out;
}

/*
		 * Create a buffer view for the vertex buffer in GLTF.
		 * @param vertex_buffer : The buffer of vertex positions.
		 * @param index_buffer : The buffer of indices.
		 * @return tinygltf::BufferView : The buffer view for the vertex buffer.
		 */
tinygltf::BufferView get_vertex_buffer_view(const std::vector<float>& vertex_buffer,
                                            const std::vector<uint32_t>& index_buffer) {
  tinygltf::BufferView out;
  out.buffer = 0;
  out.byteOffset = index_buffer.size() * sizeof(uint32_t);
  out.byteLength = vertex_buffer.size() * sizeof(float);
  out.target = TINYGLTF_TARGET_ARRAY_BUFFER;
  return out;
}

/*
		 * Create a buffer view for the index buffer in GLTF.
		 * @param index_buffer : The buffer of indices.
		 * @return tinygltf::BufferView : The buffer view for the index buffer.
		 */
tinygltf::BufferView get_index_buffer_view(const std::vector<uint32_t>& index_buffer) {
  tinygltf::BufferView out;
  out.buffer = 0;
  out.byteOffset = 0;
  out.byteLength = index_buffer.size() * sizeof(uint32_t);
  out.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
  return out;
}

/*
		 * Get the minimum values for each coordinate (x, y, z) in the vertex buffer.
		 * @param vertex_buffer : The buffer of vertex positions.
		 * @return std::vector<double> : The minimum values for x, y, and z.
		 */
std::vector<double> get_vertex_mins(const std::vector<float>& vertex_buffer) {
  float x_min = std::numeric_limits<float>::max();
  float y_min = std::numeric_limits<float>::max();
  float z_min = std::numeric_limits<float>::max();

  for (size_t i = 0; i < vertex_buffer.size(); i += 3) {
    x_min = std::min(x_min, vertex_buffer[i]);
    y_min = std::min(y_min, vertex_buffer[i + 1]);
    z_min = std::min(z_min, vertex_buffer[i + 2]);
  }

  return {x_min, y_min, z_min};
}

/*
		 * Get the maximum values for each coordinate (x, y, z) in the vertex buffer.
		 * @param vertex_buffer : The buffer of vertex positions.
		 * @return std::vector<double> : The maximum values for x, y, and z.
		 */
std::vector<double> get_vertex_maxes(const std::vector<float>& vertex_buffer) {
  float x_max = std::numeric_limits<float>::lowest();
  float y_max = std::numeric_limits<float>::lowest();
  float z_max = std::numeric_limits<float>::lowest();

  for (size_t i = 0; i < vertex_buffer.size(); i += 3) {
    x_max = std::max(x_max, vertex_buffer[i]);
    y_max = std::max(y_max, vertex_buffer[i + 1]);
    z_max = std::max(z_max, vertex_buffer[i + 2]);
  }

  return {x_max, y_max, z_max};
}

/*
		 * Create a GLTF accessor for the vertex buffer.
		 * @param vertex_buffer : The buffer of vertex positions.
		 * @return tinygltf::Accessor : The accessor for the vertex buffer.
		 */
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

/*
		 * Create a GLTF accessor for the index buffer.
		 * @param index_buffer : The buffer of indices.
		 * @param vertex_count : The number of vertices.
		 * @return tinygltf::Accessor : The accessor for the index buffer.
		 */
tinygltf::Accessor get_index_accessor(const std::vector<uint32_t>& index_buffer,
                                      size_t vertex_count) {
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

/*
		 * Create a GLTF primitive for the mesh.
		 * @return tinygltf::Primitive : The primitive for the mesh.
		 */
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
bool dracoEncodeGeometry(const std::vector<float>& vertex_buffer,
                         const std::vector<uint32_t>& index_buffer, const Options& options,
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
  pos_att.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
               static_cast<int64_t>(12), static_cast<int64_t>(0));

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
    const draco::Mesh::Face face = {draco::PointIndex(index_buffer[i]),
                                    draco::PointIndex(index_buffer[i + 1]),
                                    draco::PointIndex(index_buffer[i + 2])};
    mesh.SetFace(draco::FaceIndex(i / 3), face);
  }

  // Encode the mesh using Draco, return false if the encoding fails
  draco::Encoder encoder;
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION,
                                   options.draco_quantization(0));
  encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, options.draco_quantization(1));
  encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD,
                                   options.draco_quantization(2));
  encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, options.draco_quantization(3));

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
bool dracoCompressed(tinygltf::Model& model, const Geometry& geometry, const Options& options,
                     const std::vector<float>& vertex_buffer,
                     const std::vector<uint32_t>& index_buffer) {
  // Check if the geometry is empty or if the Draco compression is disabled or invalid (quantization value)
  if (!options.is_draco_compressed() || vertex_buffer.empty() || index_buffer.empty() ||
      !options.draco_quantization_valid())
    return false;

  // Log Draco compression with quantization settings
  LOG_INFO("Applying Draco compression with quantization: position=", options.draco_quantization(0),
           "normal=", options.draco_quantization(1), "texcoord=", options.draco_quantization(2),
           "color=", options.draco_quantization(3));

  // Encode the geometry using Draco && add the compressed data to the model buffer.
  tinygltf::Buffer buffer;
  std::vector<unsigned char> compressed_data;
  if (!dracoEncodeGeometry(vertex_buffer, index_buffer, options, compressed_data) ||
      compressed_data.empty()) {
    std::cerr << "Failed to encode geometry using Draco. Falling back to regular GLTF format."
              << std::endl;
    return false;
  }

  buffer.data.assign(compressed_data.begin(), compressed_data.end());
  model.buffers.push_back(std::move(buffer));
  // Prepare the buffer view for the compressed data
  tinygltf::BufferView bufferView;
  bufferView.buffer = 0; // Reference to the first buffer
  bufferView.byteOffset = 0;
  bufferView.byteLength = static_cast<size_t>(compressed_data.size());
  model.bufferViews.push_back(std::move(bufferView));
  // Prepare the index and vertex accessors
  tinygltf::Accessor indexAccessor;
  indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT; // 5125
  indexAccessor.count = index_buffer.size();                          // Count of indices
  indexAccessor.type = TINYGLTF_TYPE_SCALAR;                          // SCALAR
  model.accessors.push_back(std::move(indexAccessor));

  tinygltf::Accessor vertexAccessor;
  vertexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // 5126
  vertexAccessor.count = vertex_buffer.size() / 3;              // Count of side
  vertexAccessor.type = TINYGLTF_TYPE_VEC3;                     // VEC3
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
  // Create vertex buffer and ID-to-offset mapping
  std::unordered_map<uint32_t, size_t> id_to_offset;
  const std::vector<float> master_vertex_buffer = get_vertex_buffer(geometry.vertices(), &id_to_offset);
  const size_t kTileSizeDefault = 256;

  std::vector<Geometry::Tile> tiles = geometry.tiles();

  if (tiles.empty()) {
    tiles = Geometry::get_triangles_tiled(geometry.vertices(), kTileSizeDefault);
  }

  // ---- If Draco was requested, try TILED DRACO first ----
  if (options.is_draco_compressed()) {
    if (!tiles.empty()) {
      LOG_INFO("Building", Logger::format_with_commas(tiles.size()), "tiles for Draco compression");
      // Build per-tile buffers (localize indices & compute mins/maxes)
      std::vector<TileBuild> built;
      built.reserve(tiles.size());
      size_t tile_count = 0;
      for (const auto& tile : tiles) {
        built.emplace_back(build_tile_buffers(master_vertex_buffer, tile.triangles, id_to_offset));
        tile_count++;
        if (tile_count % 100 == 0 || tile_count == tiles.size()) {
          LOG_INFO("  Built tile", tile_count, "of", tiles.size());
        }
      }

      // Single combined buffer holding all Draco payloads (one BufferView per tile)
      std::vector<unsigned char> bin;
      tinygltf::Model model;

      auto pushBufferView = [&](size_t byteOffset, size_t byteLength) -> int {
        tinygltf::BufferView bv;
        bv.buffer = 0;
        bv.byteOffset = byteOffset;
        bv.byteLength = byteLength;
        model.bufferViews.push_back(std::move(bv));
        return static_cast<int>(model.bufferViews.size()) - 1;
      };

      tinygltf::Mesh mesh;
      bool encode_ok = true;
      size_t encoded_count = 0;
      LOG_INFO("Encoding", Logger::format_with_commas(built.size()), "tiles with Draco compression");

      for (auto& tb : built) {
        std::vector<uint32_t> idx;
        if (tb.useUint32) {
          idx.assign(tb.idx32.begin(), tb.idx32.end());
        } else {
          idx.reserve(tb.idx16.size());
          for (auto v : tb.idx16)
            idx.push_back(static_cast<uint32_t>(v));
        }

        // Draco encode this tile
        std::vector<unsigned char> cbytes;
        if (!dracoEncodeGeometry(tb.verts, idx, options, cbytes)) {
          encode_ok = false;
          break;
        }

        // Append compressed bytes to combined buffer
        size_t start = align4(bin.size());
        if (start != bin.size())
          bin.resize(start);
        append_bytes(bin, cbytes);
        const int bv = pushBufferView(start, cbytes.size());

        // Minimal fallback accessors
        tinygltf::Accessor iAcc;
        iAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
        iAcc.count = idx.size();
        iAcc.type = TINYGLTF_TYPE_SCALAR;
        model.accessors.push_back(std::move(iAcc));

        tinygltf::Accessor vAcc;
        vAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        vAcc.count = tb.verts.size() / 3;
        vAcc.type = TINYGLTF_TYPE_VEC3;
        vAcc.minValues = tb.vmin;
        vAcc.maxValues = tb.vmax;
        model.accessors.push_back(std::move(vAcc));

        const int ia = static_cast<int>(model.accessors.size()) - 2;
        const int va = ia + 1;

        // Primitive with KHR_draco_mesh_compression
        tinygltf::Primitive prim;
        prim.mode = TINYGLTF_MODE_TRIANGLES;

        // Fallback attributes (some viewers use them if they ignore extensions)
        prim.indices = ia;
        prim.attributes["POSITION"] = va;
        prim.material = 0;

        // Draco extension points at the compressed payload
        std::map<std::string, tinygltf::Value> dracoExt;
        dracoExt["bufferView"] = tinygltf::Value(bv);
        std::map<std::string, tinygltf::Value> attrMap;
        attrMap["POSITION"] = tinygltf::Value(0); // POSITION attribute id inside Draco payload
        dracoExt["attributes"] = tinygltf::Value(attrMap);

        prim.extensions["KHR_draco_mesh_compression"] = tinygltf::Value(dracoExt);
        mesh.primitives.push_back(std::move(prim));
        
        encoded_count++;
        if (encoded_count % 100 == 0 || encoded_count == built.size()) {
          LOG_INFO("  Encoded tile", encoded_count, "of", built.size());
        }
      }

      if (encode_ok) {
        // Back the bufferViews
        tinygltf::Buffer buffer;
        buffer.data = std::move(bin);
        model.buffers.push_back(std::move(buffer));

        // Mesh + material + node + scene
        model.meshes.push_back(std::move(mesh));
        tinygltf::Material mat;
        mat.doubleSided = true;
        model.materials.push_back(mat);

        tinygltf::Scene scene;
        tinygltf::Node node;
        node.mesh = static_cast<int>(model.meshes.size()) - 1;
        model.nodes.push_back(node);
        scene.nodes.push_back(static_cast<int>(model.nodes.size()) - 1);
        model.scenes.push_back(scene);
        model.defaultScene = 0;

        model.asset.version = "2.0";
        model.asset.generator = "tinygltf";
        model.extensionsUsed.push_back("KHR_draco_mesh_compression");
        model.extensionsRequired.push_back("KHR_draco_mesh_compression");

        const std::string output_filepath =
            options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");

        tinygltf::TinyGLTF gltf;
        if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true,
                                       true, options.is_binary_output())) {
          std::cerr << "Failed to write GLTF file." << std::endl;
        }
        return; // done (tiled Draco)
      }

      // If any tile failed to encode, fall through to uncompressed tiling.
      std::cerr << "Draco tile encode failed; writing uncompressed tiles instead.\n";
    } else {
      // No tiles somehow; keep your original single-primitive Draco path
      const std::vector<uint32_t> index_buffer = get_index_buffer(geometry.triangles());
      const std::string output_filepath =
          options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");
      tinygltf::Model model;
      if (dracoCompressed(model, geometry, options, master_vertex_buffer, index_buffer)) {
        tinygltf::Material mat;
        mat.doubleSided = true;
        model.materials.push_back(mat);
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
        if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true,
                                       true, options.is_binary_output())) {
          std::cerr << "Failed to write GLTF file." << std::endl;
        }
        return; // done (single-primitive Draco)
      }
      // If even that fails, fall through to uncompressed tiling below.
    }
  }

  // ---- Tiled, UNCOMPRESSED path (fixes typed-array overflow) ----
  if (tiles.empty()) {
    // Safety fallback to old single-primitive uncompressed path
    const std::vector<uint32_t> index_buffer = get_index_buffer(geometry.triangles());
    const std::string output_filepath =
        options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");
    tinygltf::Model model;

    tinygltf::Buffer buffer = get_buffer(master_vertex_buffer, index_buffer);
    model.buffers.push_back(std::move(buffer));
    model.bufferViews.push_back(get_index_buffer_view(index_buffer));
    model.bufferViews.push_back(get_vertex_buffer_view(master_vertex_buffer, index_buffer));
    model.accessors.push_back(get_index_accessor(index_buffer, master_vertex_buffer.size() / 3));
    model.accessors.push_back(get_vertex_accessor(master_vertex_buffer));

    tinygltf::Mesh mesh;
    mesh.primitives.push_back(get_primitive());
    model.meshes.push_back(std::move(mesh));

    tinygltf::Material mat;
    mat.doubleSided = true;
    model.materials.push_back(mat);
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
    if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true, true,
                                   options.is_binary_output())) {
      std::cerr << "Failed to write GLTF file." << std::endl;
    }
    return;
  }

  // Build per-tile buffers (remap global IDs to local)
  LOG_INFO("Building", Logger::format_with_commas(tiles.size()), "tiles for uncompressed output");
  std::vector<TileBuild> built;
  built.reserve(tiles.size());
  size_t tile_count = 0;
  for (const auto& tile : tiles) {
    built.emplace_back(build_tile_buffers(master_vertex_buffer, tile.triangles, id_to_offset));
    tile_count++;
    if (tile_count % 100 == 0 || tile_count == tiles.size()) {
      LOG_INFO("  Built tile", tile_count, "of", tiles.size());
    }
  }

  // One combined binary buffer for all tiles, with 4-byte alignment
  std::vector<unsigned char> bin;
  tinygltf::Model model;

  auto pushBufferView = [&](size_t byteOffset, size_t byteLength, bool isIndex) -> int {
    tinygltf::BufferView bv;
    bv.buffer = 0;
    bv.byteOffset = byteOffset;
    bv.byteLength = byteLength;
    bv.target = isIndex ? TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER : TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.push_back(std::move(bv));
    return static_cast<int>(model.bufferViews.size()) - 1;
  };

  auto pushAccessorVerts = [&](const TileBuild& tb, int bvIndex) -> int {
    tinygltf::Accessor acc;
    acc.bufferView = bvIndex;
    acc.byteOffset = 0;
    acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // 5126
    acc.count = tb.verts.size() / 3;
    acc.type = TINYGLTF_TYPE_VEC3;
    acc.minValues = tb.vmin;
    acc.maxValues = tb.vmax;
    model.accessors.push_back(std::move(acc));
    return static_cast<int>(model.accessors.size()) - 1;
  };

  auto pushAccessorIndex = [&](const TileBuild& tb, int bvIndex) -> int {
    tinygltf::Accessor acc;
    acc.bufferView = bvIndex;
    acc.byteOffset = 0;
    acc.componentType = tb.useUint32 ? TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
                                     : TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    acc.count = tb.useUint32 ? tb.idx32.size() : tb.idx16.size();
    acc.type = TINYGLTF_TYPE_SCALAR;
    acc.minValues = {0.0};
    acc.maxValues = {double((tb.verts.size() / 3) ? (tb.verts.size() / 3 - 1) : 0)};
    model.accessors.push_back(std::move(acc));
    return static_cast<int>(model.accessors.size()) - 1;
  };

  tinygltf::Mesh mesh;

  // Pack: [indices][verts] per tile → repeat
  LOG_INFO("Packing", Logger::format_with_commas(built.size()), "tiles into buffer");
  size_t packed_count = 0;
  for (auto& tb : built) {
    // indices
    size_t start = align4(bin.size());
    if (start != bin.size())
      bin.resize(start);
    if (tb.useUint32)
      append_bytes(bin, tb.idx32);
    else
      append_bytes(bin, tb.idx16);
    const size_t idxBytes =
        tb.useUint32 ? tb.idx32.size() * sizeof(uint32_t) : tb.idx16.size() * sizeof(uint16_t);
    tb.bvIndices = pushBufferView(start, idxBytes, /*isIndex*/ true);

    // vertices
    start = align4(bin.size());
    if (start != bin.size())
      bin.resize(start);
    append_bytes(bin, tb.verts);
    tb.bvVerts = pushBufferView(start, tb.verts.size() * sizeof(float), /*isIndex*/ false);

    // accessors
    tb.accIndices = pushAccessorIndex(tb, tb.bvIndices);
    tb.accVerts = pushAccessorVerts(tb, tb.bvVerts);

    // primitive
    tinygltf::Primitive prim;
    prim.mode = TINYGLTF_MODE_TRIANGLES;
    prim.indices = tb.accIndices;
    prim.attributes["POSITION"] = tb.accVerts;
    prim.material = 0;
    mesh.primitives.push_back(std::move(prim));
    
    packed_count++;
    if (packed_count % 100 == 0 || packed_count == built.size()) {
      LOG_INFO("  Packed tile", packed_count, "of", built.size());
    }
  }

  // Single buffer that backs all bufferViews
  tinygltf::Buffer buffer;
  buffer.data = std::move(bin);
  model.buffers.push_back(std::move(buffer));

  // Mesh + material + node + scene
  model.meshes.push_back(std::move(mesh));
  tinygltf::Material mat;
  mat.doubleSided = true;
  model.materials.push_back(mat);

  tinygltf::Scene scene;
  tinygltf::Node node;
  node.mesh = static_cast<int>(model.meshes.size()) - 1;
  model.nodes.push_back(node);
  scene.nodes.push_back(static_cast<int>(model.nodes.size()) - 1);
  model.scenes.push_back(scene);
  model.defaultScene = 0;
  model.asset.version = "2.0";
  model.asset.generator = "tinygltf";

  std::string output_filepath =
      options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");
  tinygltf::TinyGLTF gltf;
  if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true, true,
                                 options.is_binary_output())) {
    std::cerr << "Failed to write GLTF file." << std::endl;
  }
}

} // namespace model
} // namespace mbgrd2gltf
