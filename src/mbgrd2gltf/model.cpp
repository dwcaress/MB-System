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
#include "viewpoint.h"
#include "io/cesium_writer.h"
#include "scale_axes.h"

// external libraries
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"
#include "draco_mesh.h"
#include "haxby_colors.h"

// standard library
#include <sys/stat.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace mbgrd2gltf {
namespace model {

// Validate glTF file using TinyGLTF's built-in validation
static void validate_gltf_file(const std::string& filepath, bool is_binary) {
  LOG_INFO("Validating output file...");
  tinygltf::TinyGLTF loader;
  tinygltf::Model validation_model;
  std::string err, warn;
  bool ret;
  
  if (is_binary) {
    ret = loader.LoadBinaryFromFile(&validation_model, &err, &warn, filepath);
  } else {
    ret = loader.LoadASCIIFromFile(&validation_model, &err, &warn, filepath);
  }
  
  if (!warn.empty()) {
    LOG_WARN("glTF validation warnings:", warn);
  }
  if (!err.empty()) {
    LOG_ERROR("glTF validation errors:", err);
  }
  if (ret && warn.empty() && err.empty()) {
    LOG_INFO("Validation passed - no errors or warnings");
  }
}

//New Addition: Generation in blocks
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

struct BlockBuild {
  std::vector<float> verts; // x,y,z ...
  std::vector<uint16_t> idx16;
  std::vector<uint32_t> idx32;
  bool useUint32 = false;

  int bvIndices = -1;
  int bvVerts = -1;
  int accIndices = -1;
  int accVerts = -1;

  std::vector<double> vmin, vmax;

  std::vector<uint8_t> colors; // r,g,b bytes for each vertex, if the model is colored
  int bvColors = -1;
  int accColors = -1;
};

// Remap global vertex IDs in 'tris' to local 0..k-1 and build a block-local vertex buffer.
// Uses the id_to_offset map to correctly locate vertices in the master buffer.
static BlockBuild build_block_buffers(const std::vector<float>& masterVerts,
                                    const std::vector<Triangle>& tris,
                                    const std::unordered_map<uint32_t, size_t>& id_to_offset,
                                    const std::vector<uint8_t>* masterColors = nullptr) {
  BlockBuild tb;
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
      if (masterColors) {
        tb.colors.insert(tb.colors.end(), {255, 255, 255});
      }
      return lidx;
    }
    
    const size_t off = offset_it->second;
    tb.verts.push_back(masterVerts[off + 0]);
    tb.verts.push_back(masterVerts[off + 1]);
    tb.verts.push_back(masterVerts[off + 2]);
    if (masterColors) {
      // off counts floats, three for each vertex, and the colors are three bytes for each vertex in the same order
      tb.colors.insert(tb.colors.end(), masterColors->begin() + off, masterColors->begin() + off + 3);
    }
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
		* Write the geometry to a GLTF file using the given options
		* Draco compression is used if the options specify it
		* @param geometry : The geometry to write
		* @param options : The options to use for writing the file
		*/
// The run metadata as glTF extras. Whole numbers are written as integers (1, not 1.0).
static tinygltf::Value to_gltf_value(const MetaValue& value) {
  switch (value.type) {
  case MetaValue::Type::String:
    return tinygltf::Value(value.text);
  case MetaValue::Type::Number:
    if (value.number == std::floor(value.number) && std::fabs(value.number) < 2147483647.0)
      return tinygltf::Value(static_cast<int>(value.number));
    return tinygltf::Value(value.number);
  case MetaValue::Type::Bool:
    return tinygltf::Value(value.flag);
  case MetaValue::Type::Array: {
    tinygltf::Value::Array array;
    for (const MetaValue& item : value.items)
      array.push_back(to_gltf_value(item));
    return tinygltf::Value(array);
  }
  case MetaValue::Type::Object: {
    tinygltf::Value::Object object;
    for (const auto& member : value.members)
      object[member.first] = to_gltf_value(member.second);
    return tinygltf::Value(object);
  }
  }
  return tinygltf::Value();
}

void write_gltf(const Geometry& geometry, const Options& options, GridRunInfo& run) {
  // Create vertex buffer and ID-to-offset mapping
  std::unordered_map<uint32_t, size_t> id_to_offset;
  const std::vector<float> master_vertex_buffer = get_vertex_buffer(geometry.vertices(), &id_to_offset);
  const size_t kBlockSizeDefault = 256;

  // Colors by height, with the Haxby table over the range of the grid: three bytes for each vertex, in the order of the vertex buffer. The height is the
  // grid value, not the exaggerated one, so that the colors mean the same whatever --exaggeration says.
  std::vector<uint8_t> master_colors;
  if (options.colors() && !geometry.altitudes().empty()) {
    const std::vector<float>& heights = geometry.altitudes();
    const auto range = std::minmax_element(heights.begin(), heights.end());
    const std::vector<double> bounds = mbdraco::haxby_bounds(*range.first, *range.second);
    const std::vector<std::array<double, 3>> bound_colors = mbdraco::palette_colors_for_bounds(static_cast<int>(bounds.size()));
    master_colors.reserve(master_vertex_buffer.size());
    for (const float height : heights) {
      const std::array<uint8_t, 3> color = mbdraco::haxby_color_bytes(height, bounds, bound_colors);
      master_colors.insert(master_colors.end(), color.begin(), color.end());
    }
    LOG_INFO("Colors: by height with the Haxby table of mbm_grdplot, from", bounds.front(), "to", bounds.back(),
             "m (--no-colors leaves them out)");
  } else if (!options.colors()) {
    LOG_INFO("Colors: none (--no-colors)");
  }
  const std::vector<uint8_t>* master_colors_ptr = master_colors.empty() ? nullptr : &master_colors;

  std::vector<Geometry::Block> blocks = geometry.blocks();

  if (blocks.empty()) {
    blocks = Geometry::get_triangles_blocked(geometry.vertices(), kBlockSizeDefault);
  }

  // ---- If Draco was requested, try BLOCKED DRACO first ----
  if (options.is_draco_compressed()) {
    if (!blocks.empty()) {
      LOG_INFO("Building", Logger::format_with_commas(blocks.size()), "blocks for Draco compression");
      // Build per-block buffers (localize indices & compute mins/maxes)
      std::vector<BlockBuild> built;
      built.reserve(blocks.size());
      size_t block_count = 0;
      for (const auto& block : blocks) {
        built.emplace_back(build_block_buffers(master_vertex_buffer, block.triangles, id_to_offset, master_colors_ptr));
        block_count++;
        if (block_count % 100 == 0 || block_count == blocks.size()) {
          LOG_INFO("  Built block", block_count, "of", blocks.size());
        }
      }

      // Single combined buffer holding all Draco payloads (one BufferView per block). The shared code (src/mbdraco) chooses the
      // quantization of each block from its range and --draco-precision, and writes the primitive as mbmesh does.
      std::vector<unsigned char> bin;
      tinygltf::Model model;
      tinygltf::Mesh mesh;
      mbdraco::Compression used;
      bool encode_ok = true;
      size_t encoded_count = 0;
      LOG_INFO("Encoding", Logger::format_with_commas(built.size()), "blocks with Draco compression");

      for (auto& tb : built) {
        std::vector<uint32_t> idx;
        if (tb.useUint32) {
          idx.assign(tb.idx32.begin(), tb.idx32.end());
        } else {
          idx.reserve(tb.idx16.size());
          for (auto v : tb.idx16)
            idx.push_back(static_cast<uint32_t>(v));
        }

        std::string reason;
        if (!mbdraco::add_primitive(&model, &mesh, &bin, tb.verts, nullptr, idx, options.draco(), 0, &used, &reason,
                                   tb.colors.empty() ? nullptr : &tb.colors)) {
          LOG_WARN("Draco compression failed (" + reason + ")");
          encode_ok = false;
          break;
        }

        encoded_count++;
        if (encoded_count % 100 == 0 || encoded_count == built.size()) {
          LOG_INFO("  Encoded block", encoded_count, "of", built.size());
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
        // Not the defaults of glTF, fully metallic and rough, which a viewer with no environment to reflect draws black; as in mbmesh.
        mat.pbrMetallicRoughness.metallicFactor = 0.0;
        mat.pbrMetallicRoughness.roughnessFactor = 0.65;
        model.materials.push_back(mat);

        tinygltf::Scene scene;
        tinygltf::Node node;
        node.mesh = static_cast<int>(model.meshes.size()) - 1;
        model.nodes.push_back(node);
        scene.nodes.push_back(static_cast<int>(model.nodes.size()) - 1);
        model.scenes.push_back(scene);
        model.defaultScene = 0;

        model.asset.version = "2.0";
        model.asset.generator = "MB-System mbgrd2gltf";
        run.draco = used;
        model.asset.extras = to_gltf_value(grid_run_metadata(run));

        const std::string output_filepath =
            options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");

        tinygltf::TinyGLTF gltf;
        if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true,
                                       true, options.is_binary_output())) {
          std::cerr << "Failed to write GLTF file." << std::endl;
        } else {
          validate_gltf_file(output_filepath, options.is_binary_output());
        }
        return; // done (blocked Draco)
      }

      // If any block failed to encode, fall through to uncompressed blocks.
      std::cerr << "Draco block encode failed; writing uncompressed blocks instead.\n";
    }
  }

  // ---- Blocked, UNCOMPRESSED path (fixes typed-array overflow) ----
  // Build per-block buffers (remap global IDs to local)
  LOG_INFO("Building", Logger::format_with_commas(blocks.size()), "blocks for uncompressed output");
  std::vector<BlockBuild> built;
  built.reserve(blocks.size());
  size_t block_count = 0;
  for (const auto& block : blocks) {
    built.emplace_back(build_block_buffers(master_vertex_buffer, block.triangles, id_to_offset, master_colors_ptr));
    block_count++;
    if (block_count % 100 == 0 || block_count == blocks.size()) {
      LOG_INFO("  Built block", block_count, "of", blocks.size());
    }
  }

  // One combined binary buffer for all blocks, with 4-byte alignment
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

  auto pushAccessorVerts = [&](const BlockBuild& tb, int bvIndex) -> int {
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

  auto pushAccessorIndex = [&](const BlockBuild& tb, int bvIndex) -> int {
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

  // Pack: [indices][verts] per block → repeat
  LOG_INFO("Packing", Logger::format_with_commas(built.size()), "blocks into buffer");
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

    // colors, as floats from 0 to 1 (three bytes would not be aligned to four)
    if (!tb.colors.empty()) {
      std::vector<float> color_floats;
      color_floats.reserve(tb.colors.size());
      for (const uint8_t c : tb.colors) {
        color_floats.push_back(static_cast<float>(c) / 255.0f);
      }
      start = align4(bin.size());
      if (start != bin.size())
        bin.resize(start);
      append_bytes(bin, color_floats);
      tb.bvColors = pushBufferView(start, color_floats.size() * sizeof(float), /*isIndex*/ false);
      tinygltf::Accessor acc;
      acc.bufferView = tb.bvColors;
      acc.byteOffset = 0;
      acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
      acc.count = tb.colors.size() / 3;
      acc.type = TINYGLTF_TYPE_VEC3;
      model.accessors.push_back(std::move(acc));
      tb.accColors = static_cast<int>(model.accessors.size()) - 1;
    }

    // accessors
    tb.accIndices = pushAccessorIndex(tb, tb.bvIndices);
    tb.accVerts = pushAccessorVerts(tb, tb.bvVerts);

    // primitive
    tinygltf::Primitive prim;
    prim.mode = TINYGLTF_MODE_TRIANGLES;
    prim.indices = tb.accIndices;
    prim.attributes["POSITION"] = tb.accVerts;
    if (tb.accColors >= 0) {
      prim.attributes["COLOR_0"] = tb.accColors;
    }
    prim.material = 0;
    mesh.primitives.push_back(std::move(prim));
    
    packed_count++;
    if (packed_count % 100 == 0 || packed_count == built.size()) {
      LOG_INFO("  Packed block", packed_count, "of", built.size());
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
  mat.pbrMetallicRoughness.metallicFactor = 0.0;   // see the Draco path above: the glTF defaults are black without an environment
  mat.pbrMetallicRoughness.roughnessFactor = 0.65;
  model.materials.push_back(mat);

  tinygltf::Scene scene;
  tinygltf::Node node;
  node.mesh = static_cast<int>(model.meshes.size()) - 1;
  model.nodes.push_back(node);
  scene.nodes.push_back(static_cast<int>(model.nodes.size()) - 1);
  model.scenes.push_back(scene);
  model.defaultScene = 0;
  model.asset.version = "2.0";
  model.asset.generator = "MB-System mbgrd2gltf";
  model.asset.extras = to_gltf_value(grid_run_metadata(run));

  std::string output_filepath =
      options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");
  tinygltf::TinyGLTF gltf;
  if (!gltf.WriteGltfSceneToFile(&model, output_filepath, options.is_binary_output(), true, true,
                                 options.is_binary_output())) {
    std::cerr << "Failed to write GLTF file." << std::endl;
  } else {
    validate_gltf_file(output_filepath, options.is_binary_output());
  }
}

// The check box that turns the colors of the model off. The vertices have a color each (COLOR_0) and the material a white base color; with the colors off
// the vertex colors are not used and the base color is the gray that MeshLab shows a mesh in, as in the pages of mbmesh. The model is an Inline, whose nodes X3DOM
// makes when the file has arrived, so they are reached through its internal nodes, and done again from time to time while the colors are off.
static const char* height_colors_script = R"JS(
      var heightColorsOn = true;
      var heightColorsGray = 0.75;   // 192 of 255, the gray that MeshLab shows a mesh in

      function collectShapes(node, shapes, seen) {
        if (!node || seen.indexOf(node) >= 0) {
          return;
        }
        seen.push(node);
        if (node.typeName && node.typeName() === 'Shape') {
          shapes.push(node);
          return;
        }
        var children = node._childNodes || [];
        for (var i = 0; i < children.length; i++) {
          collectShapes(children[i], shapes, seen);
        }
        var fields = node._cf || {};
        for (var name in fields) {
          if (fields[name] && fields[name].node) {
            collectShapes(fields[name].node, shapes, seen);
          }
        }
      }

      function applyHeightColors() {
        var inline = document.getElementById('inline');
        if (!inline || !inline._x3domNode) {
          return;
        }
        var shapes = [], changed = false;
        collectShapes(inline._x3domNode, shapes, []);
        for (var s = 0; s < shapes.length; s++) {
          var shape = shapes[s], material = shape._cf && shape._cf.appearance && shape._cf.appearance.node &&
                                            shape._cf.appearance.node._cf.material && shape._cf.appearance.node._cf.material.node;
          var geometry = shape._cf && shape._cf.geometry && shape._cf.geometry.node;
          if (!material || !geometry || !material._vf || shape._colorsOn === heightColorsOn) {
            continue;
          }
          if (shape._colorsOn === undefined && geometry._hasColor !== true) {
            continue;   // a block without vertex colors
          }
          var gray = heightColorsOn ? 1 : heightColorsGray;
          material._vf.baseColorFactor = new x3dom.fields.SFColorRGBA(gray, gray, gray, 1);
          material.fieldChanged('baseColorFactor');
          geometry._hasColor = heightColorsOn;
          shape._dirty.shader = true;
          shape._dirty.colors = true;
          geometry.invalidateVolume();
          shape._colorsOn = heightColorsOn;
          changed = true;
        }
        if (changed) {
          var runtime = document.getElementById('x3dElement').runtime;
          if (runtime && runtime.canvas && runtime.canvas.doc) {
            runtime.canvas.doc.needRender = true;
          }
        }
      }

      function setHeightColors(on) {
        heightColorsOn = on;
        applyHeightColors();
      }

      // Blocks arrive as the file is read: the ones that arrive while the colors are off are made gray.
      setInterval(function () {
        if (heightColorsOn === false) {
          try { applyHeightColors(); } catch (e) { }
        }
      }, 500);
)JS";

// The script of the Copy GeoViewpoint button: the --geoviewpoint option for the current view, geodetic like a
// GeoViewpoint, so that it does not depend on the GeoOrigin or the grid. The page defines modelOrigin, the
// WGS 84 ECEF position of the GeoOrigin that the model is relative to (zeros if it is absolute).
static const char* copy_viewpoint_script = R"JS(
      // WGS 84 ECEF meters to [longitude, latitude, elevation], in degrees and meters.
      function ecefToGeodetic(x, y, z) {
        var f = 1 / 298.257223563, a = 6378137, e2 = f * (2 - f);
        var longitude = Math.atan2(y, x), r = Math.sqrt(x * x + y * y), latitude = Math.atan2(z, r * (1 - e2)), height = 0;
        for (var i = 0; i < 10; i++) {
          var n = a / Math.sqrt(1 - e2 * Math.sin(latitude) * Math.sin(latitude));
          height = r / Math.cos(latitude) - n;
          latitude = Math.atan2(z, r * (1 - e2 * n / (n + height)));
        }
        return [longitude * 180 / Math.PI, latitude * 180 / Math.PI, height];
      }

      // Says for a few seconds, under the button, what the Copy GeoViewpoint button did.
      var copyStatusTimer = null;
      function showCopyStatus(message) {
        var status = document.getElementById('copy-status');
        if (!status) {
          return;
        }
        status.textContent = message;
        status.style.visibility = 'visible';
        clearTimeout(copyStatusTimer);
        copyStatusTimer = setTimeout(function () { status.style.visibility = 'hidden'; }, 5000);
      }

      // Puts the --geoviewpoint option for the current view on the clipboard and in the X3DOM log. It is
      // geodetic, like a GeoViewpoint: longitude, latitude and elevation of the camera, its orientation
      // relative to the local frame there (as a GeoViewpoint takes it), and the center of rotation. It does
      // not depend on the GeoOrigin, the area or the grid, so it can be used in another run.
      function copyGeoViewpoint() {
        var runtime = document.getElementById('x3dElement').runtime;
        var matrix = runtime.viewMatrix().inverse();
        var axes = [matrix.e0(), matrix.e1(), matrix.e2()];
        var camera = matrix.e3();
        var center = null;
        try {
          center = runtime.canvas.doc._scene.getViewpoint().getCenterOfRotation();
        } catch (e) {
          center = null;
        }
        if (!center) {
          var box = runtime.getSceneBBox();
          center = { x: (box.min.x + box.max.x) / 2, y: (box.min.y + box.max.y) / 2, z: (box.min.z + box.max.z) / 2 };
        }
        var position = ecefToGeodetic(camera.x + modelOrigin[0], camera.y + modelOrigin[1], camera.z + modelOrigin[2]);
        var target = ecefToGeodetic(center.x + modelOrigin[0], center.y + modelOrigin[1], center.z + modelOrigin[2]);

        // The local frame at the camera in the form X3DOM uses for a GeoViewpoint: X east, Y up, Z south.
        var phi = position[1] * Math.PI / 180, lambda = position[0] * Math.PI / 180;
        var east = [-Math.sin(lambda), Math.cos(lambda), 0];
        var north = [-Math.sin(phi) * Math.cos(lambda), -Math.sin(phi) * Math.sin(lambda), Math.cos(phi)];
        var up = [Math.cos(phi) * Math.cos(lambda), Math.cos(phi) * Math.sin(lambda), Math.sin(phi)];
        var frame = [east, up, [-north[0], -north[1], -north[2]]];
        var camera_axes = [[axes[0].x, axes[0].y, axes[0].z], [axes[1].x, axes[1].y, axes[1].z], [axes[2].x, axes[2].y, axes[2].z]];

        // The orientation is the frame's transpose times the camera's rotation, as an axis and an angle.
        var o = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];
        for (var i = 0; i < 3; i++) {
          for (var j = 0; j < 3; j++) {
            for (var k = 0; k < 3; k++) {
              o[i][j] += frame[i][k] * camera_axes[j][k];
            }
          }
        }
        var cosine = Math.max(-1, Math.min(1, (o[0][0] + o[1][1] + o[2][2] - 1) / 2));
        var angle = Math.acos(cosine);
        var axis = [o[2][1] - o[1][2], o[0][2] - o[2][0], o[1][0] - o[0][1]];
        var length = Math.sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
        if (length < 1e-9) {
          axis = [0, 0, 1];
          angle = 0;
        } else {
          axis = [axis[0] / length, axis[1] / length, axis[2] / length];
        }

        var numbers = [position[0], position[1], position[2], axis[0], axis[1], axis[2], angle, target[0], target[1], target[2]];
        var digits = [7, 7, 3, 8, 8, 8, 8, 7, 7, 3];
        var text = '--geoviewpoint=' + numbers.map(function (value, index) { return value.toFixed(digits[index]); }).join(',');
        x3dom.debug.logInfo(text);
        var failed = function () {
          showCopyStatus('The clipboard is not available here; the option is in the X3DOM log');
          window.prompt('Copy the option:', text);
        };
        if (navigator.clipboard && window.isSecureContext) {
          navigator.clipboard.writeText(text).then(function () {
            showCopyStatus('The --geoviewpoint option was copied to the clipboard');
          }, failed);
        } else {
          failed();
        }
      })JS";

/*
 * Write an HTML viewer page with inline glTF/GLB model using X3DOM
 * @param bathymetry : The bathymetry data (for grid bounds)
 * @param geometry : The geometry used (for provenance metadata)
 * @param options : The options used (for provenance metadata)
 * @param command_line : The complete command line string
 * @param timestamp : ISO 8601 timestamp of processing
 * @param log_messages : Captured log messages for provenance
 */
// The log of the run, in a file of its own beside the model, which the pages link to.
static void write_log_file(const std::string& log_filepath, const std::vector<std::string>& log_messages) {
  std::ofstream log_file(log_filepath);
  if (!log_file.is_open()) {
    throw std::runtime_error("Failed to open log output file: " + log_filepath);
  }
  for (const auto& log_msg : log_messages) {
    log_file << log_msg << "\n";
  }
}

void write_html(const Bathymetry& bathymetry, const Geometry& geometry, const Options& options,
                const std::string& command_line, const std::string& timestamp,
                const std::vector<std::string>& log_messages, const GridRunInfo& run) {
  
  // Determine the glTF/GLB filename (same as output root + extension)
  std::string model_filename = options.output_filepath();
  size_t last_slash = model_filename.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    model_filename = model_filename.substr(last_slash + 1);
  }
  model_filename += (options.is_binary_output() ? ".glb" : ".gltf");
  
  // HTML output file path
  std::string html_filepath = options.output_filepath() + ".html";
  
  // Open output file
  std::ofstream html_file(html_filepath);
  if (!html_file.is_open()) {
    throw std::runtime_error("Failed to open HTML output file: " + html_filepath);
  }
  
  // The log goes in a file of its own beside the page, which the page loads when the log is first shown, so
  // that a long log does not make the page large and hard to edit by hand.
  std::string log_url = html_filepath.substr(html_filepath.find_last_of("/\\") == std::string::npos ? 0 : html_filepath.find_last_of("/\\") + 1);
  log_url = log_url.substr(0, log_url.size() - 5) + ".log";
  write_log_file(options.output_filepath() + ".log", log_messages);

  // Get grid bounds from bathymetry
  double lon_min = bathymetry.longitude_min();
  double lon_max = bathymetry.longitude_max();
  double lat_min = bathymetry.latitude_min();
  double lat_max = bathymetry.latitude_max();
  
  // Calculate center point for GeoViewpoint
  double center_lon = (lon_min + lon_max) / 2.0;
  double center_lat = (lat_min + lat_max) / 2.0;
  
  // Calculate mean depth from bathymetry altitude data
  const auto& altitudes = bathymetry.altitudes();
  double altitude_sum = 0.0;
  size_t altitude_count = 0;
  for (size_t y = 0; y < altitudes.size_y(); ++y) {
    for (size_t x = 0; x < altitudes.size_x(); ++x) {
      float altitude = altitudes.at(x, y);
      if (!std::isnan(altitude)) {
        altitude_sum += altitude;
        altitude_count++;
      }
    }
  }
  double mean_depth = (altitude_count > 0) ? (altitude_sum / altitude_count) : 0.0;
  
  // Get mean elevation for auto geoorigin 
  double mean_elevation = 0.0;
  if (options.is_geoorigin_auto()) {
    mean_elevation = mean_depth;
  }
  
  // Calculate viewing altitude based on grid extent
  // Approximate degrees to meters at this latitude: 1 degree ≈ 111km
  double lat_extent_deg = lat_max - lat_min;
  double lon_extent_deg = lon_max - lon_min;
  double cos_lat = std::cos(center_lat * M_PI / 180.0);
  
  // Convert to approximate meters
  double lat_extent_m = lat_extent_deg * 111000.0;
  double lon_extent_m = lon_extent_deg * 111000.0 * cos_lat;
  double max_extent_m = std::max(lat_extent_m, lon_extent_m);
  
  // Calculate altitude based on field of view to fill the window
  // fieldOfView = 0.785398 radians (45 degrees)
  // distance = (extent / 2) / tan(fov / 2)
  const double fov = 0.785398;  // 45 degrees in radians
  double view_altitude = ((max_extent_m / 2.0) / std::tan(fov / 2.0));
  
  // Format GeoViewpoint position and orientation (X3D Geo order: lat, lon, elevation)
  // Position elevation is absolute, so add mean_depth to the relative view_altitude
  std::ostringstream geoposition_str, orientation_str;
  geoposition_str << std::fixed << std::setprecision(6) 
                  << center_lat << " " << center_lon << " " << std::setprecision(1) << (mean_depth + view_altitude);
  
  // Orientation: rotate -90 degrees around X axis to look straight down (north up)
  orientation_str << "1 0 0 -1.5708";

  // The same view as a plain Viewpoint, in the coordinates of the model: ECEF minus the GeoOrigin's
  // ECEF position (the vertices are written that way), or plain ECEF without a GeoOrigin. Above the
  // center of the grid by view_altitude, looking straight down with north at the top: the rotation
  // whose columns are the local east, north and up axes, as an axis and an angle.
  // The ECEF position of the GeoOrigin, which the vertices are relative to (zeros without one).
  const double origin_ecef[3] = {geometry.geoorigin_x(), geometry.geoorigin_y(), geometry.geoorigin_z()};
  double local_position[3], local_center[3], local_axis[3] = {0.0, 0.0, 1.0}, local_angle = 0.0;
  {
    const double phi = center_lat * M_PI / 180.0;
    const double lambda = center_lon * M_PI / 180.0;
    const double east[3] = {-std::sin(lambda), std::cos(lambda), 0.0};
    const double north[3] = {-std::sin(phi) * std::cos(lambda), -std::sin(phi) * std::sin(lambda), std::cos(phi)};
    const double up[3] = {std::cos(phi) * std::cos(lambda), std::cos(phi) * std::sin(lambda), std::sin(phi)};
    const Geometry::EcefPosition middle = Geometry::get_ecef_position(center_lon, center_lat, mean_depth);
    const double origin[3] = {geometry.geoorigin_x(), geometry.geoorigin_y(), geometry.geoorigin_z()};
    const double middle_ecef[3] = {middle.x, middle.y, middle.z};
    for (int i = 0; i < 3; i++) {
      local_center[i] = middle_ecef[i] - origin[i];
      local_position[i] = local_center[i] + up[i] * view_altitude;
    }
    const double trace = east[0] + north[1] + up[2];
    local_angle = std::acos(std::max(-1.0, std::min(1.0, (trace - 1.0) / 2.0)));
    double axis[3] = {north[2] - up[1], up[0] - east[2], east[1] - north[0]};
    const double length = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (length > 1.0e-9) {
      for (int i = 0; i < 3; i++)
        local_axis[i] = axis[i] / length;
    } else {
      local_angle = 0.0;
    }
  }
  
  // Write HTML content
  html_file << "<!DOCTYPE html>\n";
  html_file << "<html style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n";
  html_file << "  <head>\n";
  html_file << "    <meta http-equiv='X-UA-Compatible' content='chrome=1'></meta>\n";
  html_file << "    <meta http-equiv='Content-Type' content='text/html;charset=utf-8'></meta>\n";
  html_file << "    <link type='text/css' href='https://www.x3dom.org/x3dom/dist/x3dom.css' rel='stylesheet'>\n";
  html_file << "    <script type='text/javascript' src='https://www.x3dom.org/x3dom/dist/x3dom-full.js'></script>\n";
  html_file << "    <style>\n";
  // The scene fills the window. X3DOM sizes its canvas from the CSS size of the canvas, and sets the width
  // and height attributes of the canvas, which give it an aspect ratio, from that. With a height that is a
  // percentage of a parent, the canvas can end up 2:1 whatever the window is, and in Safari the size can
  // grow until the drawing buffer is too large to allocate. A size in viewport units, which must win over
  // the style that X3DOM writes, keeps it the size of the window.
  html_file << "      html, body { overflow: hidden; }\n";
  html_file << "      x3d { display: block; width: 100vw !important; height: 100vh !important; }\n";
  html_file << "      x3d canvas { display: block; width: 100% !important; height: 100% !important; }\n";
  html_file << "      .x3dom-logContainer { bottom: 0px; position: absolute; }\n";
  html_file << "      #log-panel {\n";
  html_file << "        position: fixed;\n";
  html_file << "        bottom: 0;\n";
  html_file << "        left: 0;\n";
  html_file << "        right: 0;\n";
  html_file << "        max-height: 30vh;\n";
  html_file << "        background-color: rgba(240,240,240,0.95);\n";
  html_file << "        border-top: 2px solid #666;\n";
  html_file << "        display: none;\n";
  html_file << "        flex-direction: column;\n";
  html_file << "        z-index: 2000;\n";
  html_file << "      }\n";
  html_file << "      #log-panel.visible { display: flex; }\n";
  html_file << "      #log-header {\n";
  html_file << "        background-color: #666;\n";
  html_file << "        color: white;\n";
  html_file << "        padding: 8px 12px;\n";
  html_file << "        font-weight: bold;\n";
  html_file << "        display: flex;\n";
  html_file << "        justify-content: space-between;\n";
  html_file << "        align-items: center;\n";
  html_file << "        cursor: pointer;\n";
  html_file << "      }\n";
  html_file << "      #log-content {\n";
  html_file << "        flex: 1;\n";
  html_file << "        overflow-y: auto;\n";
  html_file << "        padding: 8px;\n";
  html_file << "        font-family: 'Courier New', monospace;\n";
  html_file << "        font-size: 12px;\n";
  html_file << "      }\n";
  html_file << "      .log-line {\n";
  html_file << "        padding: 2px 0;\n";
  html_file << "        white-space: pre-wrap;\n";
  html_file << "        word-wrap: break-word;\n";
  html_file << "      }\n";
  html_file << "      #toggle-log-btn {\n";
  html_file << "        position: fixed;\n";
  html_file << "        bottom: 10px;\n";
  html_file << "        left: 10px;\n";
  html_file << "        background-color: rgba(102,102,102,0.9);\n";
  html_file << "        color: white;\n";
  html_file << "        border: none;\n";
  html_file << "        padding: 10px 15px;\n";
  html_file << "        border-radius: 4px;\n";
  html_file << "        cursor: pointer;\n";
  html_file << "        z-index: 1500;\n";
  html_file << "        font-weight: bold;\n";
  html_file << "      }\n";
  html_file << "      #toggle-log-btn:hover { background-color: rgba(85,85,85,0.9); }\n";
  html_file << "      #model-title {\n";
  html_file << "        position: absolute;\n";
  html_file << "        top: 0px;\n";
  html_file << "        left: 50%;\n";
  html_file << "        transform: translateX(-50%);\n";
  html_file << "        background-color: rgba(199,202,204,.7);\n";
  html_file << "        padding: 4px 20px;\n";
  html_file << "        margin: 2px;\n";
  html_file << "        margin-top: 2px;\n";
  html_file << "        z-index: 1000;\n";
  html_file << "        font-size: 16px;\n";
  html_file << "        font-weight: bold;\n";
  html_file << "        white-space: nowrap;\n";
  html_file << "      }\n";
  html_file << "    </style>\n";
  html_file << "  </head>\n";
  html_file << "  <body style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n";
  html_file << "    <div id='HUDs_Div'>\n";
  html_file << "      <div id='X3DOM_MBSystem_Links' class='group' style='margin:2px; margin-top:26px; padding:4px; background-color:rgba(199,202,204,.7); position:absolute; float:center; z-index:1000;'>\n";
  html_file << "        <a href='https://www.x3dom.org'>X3DOM</a> output created with\n";
  html_file << "        <a href='https://www.mbari.org/products/research-software/mb-system/'>MB-System</a> mbgrd2gltf.\n";
  html_file << "      </div>\n";
  html_file << "      <div id='model-title'>" << model_filename << "</div>\n";
  html_file << "      <div id='Interaction_Toolbox' style='margin:2px; padding:4px; padding-right:150px; background-color:rgba(199,202,204,.7);position:absolute; z-index:1000; right:0px; top:0px;'>\n";
  html_file << "        <table>\n";
  html_file << "          <tr>\n";
  html_file << "            <td>Navigation Mode:\n";
  html_file << "            </td>\n";
  html_file << "            <td align='right'>\n";
  html_file << "              <select style='float:right;' onchange='if (this.selectedIndex !== undefined) { var e = document.getElementById(&apos;x3dElement&apos;); if (this.options[this.selectedIndex].value === &apos;examine&apos;) { e.runtime.examine(); } else if (this.options[this.selectedIndex].value === &apos;lookat&apos;) { e.runtime.lookAt(); } else if (this.options[this.selectedIndex].value === &apos;walk&apos;) { e.runtime.walk(); } else if (this.options[this.selectedIndex].value === &apos;fly&apos;) { e.runtime.fly(); } else if (this.options[this.selectedIndex].value === &apos;helicopter&apos;) { e.runtime.helicopter(); } else if (this.options[this.selectedIndex].value === &apos;none&apos;) { e.runtime.noNav(); } }'>\n";
  html_file << "                <option value='examine'>Examine\n";
  html_file << "                </option>\n";
  html_file << "                <option value='lookat'>LookAt\n";
  html_file << "                </option>\n";
  html_file << "                <option value='walk'>Walk\n";
  html_file << "                </option>\n";
  html_file << "                <option value='fly'>Fly\n";
  html_file << "                </option>\n";
  html_file << "                <option value='helicopter'>Helicopter\n";
  html_file << "                </option>\n";
  html_file << "                <option value='none'>None\n";
  html_file << "                </option>\n";
  html_file << "              </select>\n";
  html_file << "            </td>\n";
  html_file << "          </tr>\n";
  html_file << "          <tr>\n";
  html_file << "            <td>Viewpoint:\n";
  html_file << "            </td>\n";
  html_file << "            <td align='right'>\n";
  html_file << "              <select style='float:right;' onchange='bindViewpoint(this.value)'>\n";
  html_file << "                <option value='OVERVIEW_LOCAL'>Viewpoint (model coordinates)\n";
  html_file << "                </option>\n";
  html_file << "                <option value='OVERVIEW_GEO'>GeoViewpoint (with GeoOrigin)\n";
  html_file << "                </option>\n";
  for (size_t i = 0; i < options.geoviewpoints().size(); ++i) {
    html_file << "                <option value='USER_GEOVIEW_" << (i + 1) << "'>User GeoViewpoint " << (i + 1) << "\n";
    html_file << "                </option>\n";
  }
  html_file << "              </select>\n";
  html_file << "            </td>\n";
  html_file << "          </tr>\n";
  html_file << "          <tr>\n";
  html_file << "            <td>Debug Display:\n";
  html_file << "            </td>\n";
  html_file << "            <td align='right'>\n";
  html_file << "              <input type='checkbox' onclick='if (this.checked) { document.getElementById(&apos;x3dElement&apos;).runtime.statistics(true); } else { document.getElementById(&apos;x3dElement&apos;).runtime.statistics(false); }'>\n";
  html_file << "              </input> Stats\n";
  html_file << "              <input type='checkbox' onclick='document.getElementById(&apos;x3dElement&apos;).runtime.debug();'>\n";
  html_file << "              </input> Log\n";
  html_file << "            </td>\n";
  html_file << "          </tr>\n";
  // The three buttons are in one row, in one cell so that the table keeps its two columns, and the scale axes check box is under them.
  html_file << "          <tr>\n";
  html_file << "            <td colspan='2'>\n";
  html_file << "              <button onclick='document.getElementById(&apos;x3dElement&apos;).runtime.showAll();'> Show Everything\n";
  html_file << "              </button>\n";
  html_file << "              <button onclick='document.getElementById(&apos;x3dElement&apos;).runtime.resetView();'> Reset View\n";
  html_file << "              </button>\n";
  html_file << "              <button onclick='copyGeoViewpoint()' title='Puts the option that gives the current view on the clipboard and in the X3DOM log'> Copy GeoViewpoint\n";
  html_file << "              </button>\n";
  html_file << "              <div id='copy-status' style='visibility:hidden; font-size:smaller; width:0; min-width:100%; min-height:1.2em;'>&nbsp;</div>\n";
  html_file << "            </td>\n";
  html_file << "          </tr>\n";
  if (options.colors()) {
    html_file << "          <tr>\n";
    html_file << "            <td colspan='2'>\n";
    html_file << "              <input id='height-colors-checkbox' type='checkbox' checked onclick='setHeightColors(this.checked);'>\n";
    html_file << "              </input> color by height\n";
    html_file << "            </td>\n";
    html_file << "          </tr>\n";
  }
  scale_axes::write_checkbox_row(html_file);
  html_file << "        </table>\n";
  html_file << "      </div>\n";
  html_file << "    </div>\n";
  html_file << "    <x3d id='x3dElement' showStat='false' showLog='false' style='width:100%; height:100%; border:0; margin:0; padding:0;'>\n";
  html_file << "      <scene DEF='scene'>\n";
  
  // Two overview viewpoints, which are meant to show the same view, so that each can be checked
  // against the other. The first is bound when the page opens. The Viewpoint is in the coordinates
  // of the model. The GeoViewpoint gives the same place as a latitude, longitude and elevation, and
  // the GeoOrigin it is relative to is a child node: geoOrigin is an SFNode field, so X3DOM takes it
  // from a child element and ignores an attribute that names a DEF, in which case the GeoViewpoint
  // is placed in absolute ECEF, millions of meters from a model that is relative to the GeoOrigin.
  // Page Down steps from one viewpoint to the other.
  html_file << std::fixed << std::setprecision(3)
            << "        <Viewpoint DEF='OVERVIEW_LOCAL' id='OVERVIEW_LOCAL' description='Overview, Viewpoint in model coordinates' position='"
            << local_position[0] << " " << local_position[1] << " " << local_position[2] << "' orientation='" << std::setprecision(8)
            << local_axis[0] << " " << local_axis[1] << " " << local_axis[2] << " " << local_angle
            << "' fieldOfView='0.785398' centerOfRotation='" << std::setprecision(3) << local_center[0] << " " << local_center[1] << " "
            << local_center[2] << "'" << scale_axes::viewpoint_hook() << "></Viewpoint>\n";
  html_file << "        <GeoViewpoint DEF='OVERVIEW_GEO' id='OVERVIEW_GEO' description='Overview, GeoViewpoint with geoOrigin' position='" << geoposition_str.str() 
            << "' orientation='" << orientation_str.str() 
            << "' fieldOfView='0.785398' centerOfRotation='" 
            << std::fixed << std::setprecision(6) << center_lat << " " << center_lon << " " 
            << std::setprecision(2) << mean_depth << "'" << scale_axes::viewpoint_hook();
  
  html_file << " geoSystem='\"GD\" \"WE\"'>";

  // The GeoOrigin, as the child node of the GeoViewpoint that the field takes.
  if (options.is_geoorigin_auto() || options.is_geoorigin_set()) {
    html_file << "\n          <GeoOrigin DEF='GRID_ORIGIN' geoSystem='\"GD\" \"WE\"' geoCoords='";
    if (options.is_geoorigin_set()) {
      html_file << std::fixed << std::setprecision(6)
                << options.geoorigin_lat() << " " << options.geoorigin_lon() << " "
                << std::setprecision(2) << options.geoorigin_elev();
    } else {
      // Use grid center and mean elevation for auto mode
      html_file << std::fixed << std::setprecision(6)
                << center_lat << " " << center_lon << " " << std::setprecision(2) << mean_elevation;
    }
    html_file << "'></GeoOrigin>\n        ";
  }
  html_file << "</GeoViewpoint>\n";

  // GeoViewpoints the user supplied. The GeoOrigin is the one defined in OVERVIEW_GEO, used again.
  for (size_t i = 0; i < options.geoviewpoints().size(); ++i) {
    UserViewpoint user;
    std::string viewpoint_error;
    double axis[3];
    double angle = 0.0;
    if (!parse_user_viewpoint(options.geoviewpoints()[i], &user, &viewpoint_error) ||
        !geoviewpoint_orientation(user, axis, &angle, &viewpoint_error)) {
      throw std::runtime_error("GeoViewpoint " + std::to_string(i + 1) + ": " + viewpoint_error);
    }
    html_file << std::fixed << "        <GeoViewpoint DEF='USER_GEOVIEW_" << (i + 1) << "' id='USER_GEOVIEW_" << (i + 1)
              << "' description='User GeoViewpoint " << (i + 1) << "' position='" << std::setprecision(7) << user.latitude << " "
              << user.longitude << " " << std::setprecision(3) << user.elevation << "' orientation='" << std::setprecision(8) << axis[0] << " "
              << axis[1] << " " << axis[2] << " " << angle << "' fieldOfView='0.785398' centerOfRotation='" << std::setprecision(7)
              << user.center_latitude << " " << user.center_longitude << " " << std::setprecision(3) << user.center_elevation
              << "'" << scale_axes::viewpoint_hook() << " geoSystem='\"GD\" \"WE\"'>";
    if (options.is_geoorigin_auto() || options.is_geoorigin_set())
      html_file << "\n          <GeoOrigin USE='GRID_ORIGIN'></GeoOrigin>\n        ";
    html_file << "</GeoViewpoint>\n";
  }

  // Documentation of the scene, from the same metadata that is in the glTF extras.
  write_x3d_metadata(html_file, run.title.empty() ? model_filename : run.title, grid_run_metadata(run),
                     geo_metadata_summary(run), "        ");
  html_file << "        <transform>\n";
  html_file << "          <Inline id='inline' url='" << model_filename << "' nameSpaceName='gltf' mapDEFToID='true'></Inline>\n";
  html_file << "        </transform>\n";
  scale_axes::write_scene(html_file);
  html_file << "      </scene>\n";
  html_file << "    </x3d>\n";
  html_file << "    \n";
  html_file << "    <button id='toggle-log-btn' onclick='toggleLogPanel()'>Show Processing Log</button>\n";
  html_file << "    \n";
  html_file << "    <div id='log-panel'>\n";
  html_file << "      <div id='log-header' onclick='toggleLogPanel()'>\n";
  html_file << "        <span id='log-title'>Processing Log</span>\n";
  html_file << "        <span id='close-log'>✕</span>\n";
  html_file << "      </div>\n";
  html_file << "      <div id='log-content'>\n";

  html_file << "      </div>\n";
  html_file << "    </div>\n";
  html_file << "    \n";
  html_file << "    <script>\n";
  html_file << std::fixed << std::setprecision(3)
            << "      // The WGS 84 ECEF position of the GeoOrigin that the model is relative to (zeros if it is absolute).\n"
            << "      var modelOrigin = [" << origin_ecef[0] << ", " << origin_ecef[1] << ", " << origin_ecef[2] << "];\n";
  {
    // The log file's name as a JavaScript string, with the characters that would end it escaped.
    std::string escaped_url;
    for (char c : log_url) {
      if (c == '\'' || c == '\\') {
        escaped_url += '\\';
      }
      if (c == '<') {
        escaped_url += "\\x3c";
      } else {
        escaped_url += c;
      }
    }
    html_file << "      var logUrl = '" << escaped_url << "';\n";
  }
  html_file << copy_viewpoint_script << "\n";
  if (options.colors()) {
    html_file << height_colors_script << "\n";
  }
  html_file << "      // Makes one of the page's viewpoints the one in use.\n";
  html_file << "      function bindViewpoint(id) {\n";
  html_file << "        var viewpoint = document.getElementById(id);\n";
  html_file << "        if (viewpoint) {\n";
  html_file << "          viewpoint.setAttribute('set_bind', 'true');\n";
  html_file << "        }\n";
  html_file << "      }\n\n";
  html_file << "      // Loads the log file the first time the panel is shown.\n";
  html_file << "      var logLoaded = false;\n";
  html_file << "      function loadLog() {\n";
  html_file << "        if (logLoaded) {\n";
  html_file << "          return;\n";
  html_file << "        }\n";
  html_file << "        logLoaded = true;\n";
  html_file << "        var content = document.getElementById('log-content');\n";
  html_file << "        var show = function (lines) {\n";
  html_file << "          lines.forEach(function (line) {\n";
  html_file << "            var div = document.createElement('div');\n";
  html_file << "            div.className = 'log-line';\n";
  html_file << "            div.textContent = line;\n";
  html_file << "            content.appendChild(div);\n";
  html_file << "          });\n";
  html_file << "          document.getElementById('log-title').textContent = 'Processing Log (' + lines.length + ' lines, from ' + logUrl + ')';\n";
  html_file << "        };\n";
  html_file << "        fetch(logUrl).then(function (response) {\n";
  html_file << "          if (!response.ok) {\n";
  html_file << "            throw new Error(response.status + ' ' + response.statusText);\n";
  html_file << "          }\n";
  html_file << "          return response.text();\n";
  html_file << "        }).then(function (text) {\n";
  html_file << "          show(text.split('\\n').filter(function (line) { return line.length > 0; }));\n";
  html_file << "        }).catch(function (error) {\n";
  html_file << "          logLoaded = false;\n";
  html_file << "          show(['Could not load ' + logUrl + ': ' + error.message + ' (the page must be served over HTTP)']);\n";
  html_file << "        });\n";
  html_file << "      }\n\n";
  html_file << "      function toggleLogPanel() {\n";
  html_file << "        var panel = document.getElementById('log-panel');\n";
  html_file << "        var btn = document.getElementById('toggle-log-btn');\n";
  html_file << "        if (panel.classList.contains('visible')) {\n";
  html_file << "          panel.classList.remove('visible');\n";
  html_file << "          btn.textContent = 'Show Processing Log';\n";
  html_file << "        } else {\n";
  html_file << "          panel.classList.add('visible');\n";
  html_file << "          btn.textContent = 'Hide Processing Log';\n";
  html_file << "          loadLog();\n";
  html_file << "        }\n";
  html_file << "      }\n";
  html_file << scale_axes::script();
  html_file << "      scaleAxesExaggeration = " << std::fixed << std::setprecision(6) << options.exaggeration() << ";\n";
  html_file << "    </script>\n";
  html_file << "  </body>\n";
  html_file << "</html>\n";
  
  html_file.close();
  
  LOG_INFO("Successfully wrote HTML viewer to", html_filepath);
}

void write_cesium_html(const Geometry& geometry, const Options& options, const std::vector<std::string>& log_messages, const GridRunInfo& run) {
  std::string root = options.output_filepath();
  const size_t last_slash = root.find_last_of("/\\");
  const std::string name = last_slash == std::string::npos ? root : root.substr(last_slash + 1);
  write_log_file(root + ".log", log_messages);

  CesiumPageOptions page;
  page.title = run.title.empty() ? name : run.title;
  page.cesium_url = options.cesium_url();
  page.vertical_exaggeration = options.exaggeration();
  page.log_uri = name + ".log";
  page.log_label = "mbgrd2gltf log";
  // The vertices are ECEF offsets from the GeoOrigin if there is one, absolute ECEF otherwise.
  if (options.is_geoorigin_auto() || options.is_geoorigin_set()) {
    page.model_origin[0] = geometry.geoorigin_x();
    page.model_origin[1] = geometry.geoorigin_y();
    page.model_origin[2] = geometry.geoorigin_z();
  }
  CesiumModel model;
  model.uri = name + (options.is_binary_output() ? ".glb" : ".gltf");
  model.label = model.uri;
  model.visible = true;
  model.shade_without_normals = true;   // the mesh has no normals
  model.height_colors = options.colors();
  page.models.push_back(model);
  for (size_t i = 0; i < options.geoviewpoints().size(); ++i) {
    UserViewpoint user;
    double axis[3];
    double angle = 0.0;
    std::string viewpoint_error;
    if (!parse_user_viewpoint(options.geoviewpoints()[i], &user, &viewpoint_error) ||
        !geoviewpoint_orientation(user, axis, &angle, &viewpoint_error)) {
      throw std::runtime_error("GeoViewpoint " + std::to_string(i + 1) + ": " + viewpoint_error);
    }
    CesiumUserViewpoint viewpoint;
    viewpoint.longitude = user.longitude;
    viewpoint.latitude = user.latitude;
    viewpoint.height = user.elevation;
    for (int k = 0; k < 3; ++k) {
      viewpoint.axis[k] = axis[k];
    }
    viewpoint.angle = angle;
    page.user_viewpoints.push_back(viewpoint);
  }
  const std::string html_filepath = root + "_cesium.html";
  std::string error;
  if (!write_cesium_page(html_filepath, page, &error)) {
    throw std::runtime_error(error);
  }
  LOG_INFO("Wrote Cesium page", html_filepath, "with the globe hidden; the check box in the page shows it");
}

} // namespace model
} // namespace mbgrd2gltf
