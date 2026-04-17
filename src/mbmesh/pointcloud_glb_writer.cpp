/*--------------------------------------------------------------------
 *    The MB-system:  pointcloud_glb_writer.cpp  4/16/2026
 *--------------------------------------------------------------------*/

#include "pointcloud_glb_writer.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

static bool read_xyz_points(const char *xyz_file,
                            std::vector<float> *positions,
                            std::array<double, 3> *min_values,
                            std::array<double, 3> *max_values) {
  if (xyz_file == nullptr || positions == nullptr || min_values == nullptr || max_values == nullptr) {
    return false;
  }

  FILE *fp = fopen(xyz_file, "r");
  if (!fp) {
    return false;
  }

  positions->clear();
  min_values->fill(std::numeric_limits<double>::max());
  max_values->fill(std::numeric_limits<double>::lowest());

  char line[4096];
  while (fgets(line, sizeof(line), fp) != nullptr) {
    if (line[0] == '#') {
      continue;
    }

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if (sscanf(line, "%lf %lf %lf", &x, &y, &z) == 3) {
      positions->push_back(static_cast<float>(x));
      positions->push_back(static_cast<float>(y));
      positions->push_back(static_cast<float>(z));

      (*min_values)[0] = std::min((*min_values)[0], x);
      (*min_values)[1] = std::min((*min_values)[1], y);
      (*min_values)[2] = std::min((*min_values)[2], z);
      (*max_values)[0] = std::max((*max_values)[0], x);
      (*max_values)[1] = std::max((*max_values)[1], y);
      (*max_values)[2] = std::max((*max_values)[2], z);
    }
  }

  fclose(fp);
  return !positions->empty();
}

int write_pointcloud_glb_file(const char *xyz_file, const char *glb_file, int verbose) {
  std::vector<float> positions;
  std::array<double, 3> min_values;
  std::array<double, 3> max_values;

  if (!read_xyz_points(xyz_file, &positions, &min_values, &max_values)) {
    fprintf(stderr, "Error: unable to read point cloud XYZ file: %s\n", xyz_file);
    return 1;
  }

  tinygltf::Model model;
  model.asset.version = "2.0";
  model.asset.generator = "MB-System mbmesh";

  tinygltf::Buffer buffer;
  buffer.data.resize(positions.size() * sizeof(float));
  if (!buffer.data.empty()) {
    std::memcpy(buffer.data.data(), positions.data(), buffer.data.size());
  }
  model.buffers.push_back(std::move(buffer));

  tinygltf::BufferView buffer_view;
  buffer_view.buffer = 0;
  buffer_view.byteOffset = 0;
  buffer_view.byteLength = model.buffers[0].data.size();
  buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
  model.bufferViews.push_back(std::move(buffer_view));

  tinygltf::Accessor accessor;
  accessor.bufferView = 0;
  accessor.byteOffset = 0;
  accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
  accessor.count = positions.size() / 3;
  accessor.type = TINYGLTF_TYPE_VEC3;
  accessor.minValues = {min_values[0], min_values[1], min_values[2]};
  accessor.maxValues = {max_values[0], max_values[1], max_values[2]};
  model.accessors.push_back(std::move(accessor));

  tinygltf::Primitive primitive;
  primitive.mode = TINYGLTF_MODE_POINTS;
  primitive.attributes["POSITION"] = 0;
  primitive.material = 0;

  tinygltf::Mesh mesh;
  mesh.name = "point_cloud";
  mesh.primitives.push_back(std::move(primitive));
  model.meshes.push_back(std::move(mesh));

  tinygltf::Material material;
  material.doubleSided = true;
  material.pbrMetallicRoughness.baseColorFactor = {1.0, 1.0, 1.0, 1.0};
  model.materials.push_back(std::move(material));

  tinygltf::Node node;
  node.mesh = 0;
  model.nodes.push_back(std::move(node));

  tinygltf::Scene scene;
  scene.nodes.push_back(0);
  model.scenes.push_back(std::move(scene));
  model.defaultScene = 0;

  tinygltf::TinyGLTF gltf;
  if (!gltf.WriteGltfSceneToFile(&model, glb_file, true, true, true, true)) {
    fprintf(stderr, "Error: failed to write point cloud GLB: %s\n", glb_file);
    return 1;
  }

  if (verbose > 0) {
    fprintf(stderr, "Point cloud GLB written successfully: %s\n", glb_file);
  }

  return 0;
}