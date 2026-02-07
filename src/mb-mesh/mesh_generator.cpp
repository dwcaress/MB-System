/*--------------------------------------------------------------------
 *    The MB-system:  mesh_generator.cpp  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "mesh_generator.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>

namespace mbmesh {

MeshGenerator::MeshGenerator(const MeshOptions& options)
    : options_(options),
      min_lon_(std::numeric_limits<double>::max()),
      max_lon_(std::numeric_limits<double>::lowest()),
      min_lat_(std::numeric_limits<double>::max()),
      max_lat_(std::numeric_limits<double>::lowest()),
      min_depth_(std::numeric_limits<double>::max()),
      max_depth_(std::numeric_limits<double>::lowest()) {
}

bool MeshGenerator::load_data() {
  std::ifstream file(options_.input_file());
  if (!file.is_open()) {
    LOG_ERROR("Cannot open input file: " + options_.input_file());
    return false;
  }

  std::string line;
  int line_count = 0;
  
  // Skip header if present
  if (std::getline(file, line)) {
    // Check if first line is a header (contains non-numeric characters)
    if (line.find("lon") != std::string::npos || 
        line.find("lat") != std::string::npos) {
      LOG_DEBUG("Skipping header line");
    } else {
      // First line is data, process it
      file.seekg(0);
    }
  }

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    double lon, lat, depth;
    
    if (iss >> lon >> lat >> depth) {
      bathymetry_data_.emplace_back(lon, lat, depth);
      
      // Update bounds
      min_lon_ = std::min(min_lon_, lon);
      max_lon_ = std::max(max_lon_, lon);
      min_lat_ = std::min(min_lat_, lat);
      max_lat_ = std::max(max_lat_, lat);
      min_depth_ = std::min(min_depth_, depth);
      max_depth_ = std::max(max_depth_, depth);
      
      line_count++;
    }
  }

  file.close();

  LOG_INFO("Loaded " + std::to_string(line_count) + " bathymetry points");
  LOG_DEBUG("Longitude range: [" + std::to_string(min_lon_) + ", " + std::to_string(max_lon_) + "]");
  LOG_DEBUG("Latitude range: [" + std::to_string(min_lat_) + ", " + std::to_string(max_lat_) + "]");
  LOG_DEBUG("Depth range: [" + std::to_string(min_depth_) + ", " + std::to_string(max_depth_) + "]");

  return line_count > 0;
}

bool MeshGenerator::generate_mesh() {
  LOG_INFO("Creating regular grid...");
  create_regular_grid();
  
  LOG_INFO("Triangulating mesh...");
  triangulate();
  
  if (options_.decimation_level() > 0) {
    LOG_INFO("Applying mesh decimation...");
    apply_decimation();
  }
  
  LOG_INFO("Computing normals...");
  compute_normals();
  
  LOG_INFO("Generated mesh with " + std::to_string(vertices_.size()) + 
           " vertices and " + std::to_string(triangles_.size()) + " triangles");
  
  return vertices_.size() > 0 && triangles_.size() > 0;
}

void MeshGenerator::create_regular_grid() {
  double spacing = options_.grid_spacing();
  double vert_exag = options_.vertical_exaggeration();
  
  // Calculate grid dimensions
  int nx = static_cast<int>((max_lon_ - min_lon_) / spacing) + 1;
  int ny = static_cast<int>((max_lat_ - min_lat_) / spacing) + 1;
  
  LOG_DEBUG("Grid dimensions: " + std::to_string(nx) + " x " + std::to_string(ny));
  
  // Generate grid vertices
  for (int j = 0; j < ny; ++j) {
    for (int i = 0; i < nx; ++i) {
      double x = min_lon_ + i * spacing;
      double y = min_lat_ + j * spacing;
      double z = interpolate_depth(x, y) * vert_exag;
      
      vertices_.emplace_back(x, y, z);
    }
  }
}

void MeshGenerator::triangulate() {
  double spacing = options_.grid_spacing();
  int nx = static_cast<int>((max_lon_ - min_lon_) / spacing) + 1;
  int ny = static_cast<int>((max_lat_ - min_lat_) / spacing) + 1;
  
  // Create triangles from grid (two triangles per grid cell)
  for (int j = 0; j < ny - 1; ++j) {
    for (int i = 0; i < nx - 1; ++i) {
      int idx = j * nx + i;
      
      // First triangle
      triangles_.emplace_back(idx, idx + 1, idx + nx);
      
      // Second triangle
      triangles_.emplace_back(idx + 1, idx + nx + 1, idx + nx);
    }
  }
}

void MeshGenerator::apply_decimation() {
  // Simple decimation: keep every Nth triangle based on decimation level
  int keep_factor = options_.decimation_level() + 1;
  
  if (keep_factor <= 1) return;
  
  std::vector<Triangle> decimated;
  for (size_t i = 0; i < triangles_.size(); i += keep_factor) {
    decimated.push_back(triangles_[i]);
  }
  
  triangles_ = std::move(decimated);
  LOG_DEBUG("Decimated to " + std::to_string(triangles_.size()) + " triangles");
}

void MeshGenerator::compute_normals() {
  // Initialize normals with zero vectors
  normals_.resize(vertices_.size(), Vertex(0, 0, 0));
  
  // Accumulate face normals
  for (const auto& tri : triangles_) {
    const Vertex& v0 = vertices_[tri.v0];
    const Vertex& v1 = vertices_[tri.v1];
    const Vertex& v2 = vertices_[tri.v2];
    
    // Calculate edges
    double e1x = v1.x - v0.x, e1y = v1.y - v0.y, e1z = v1.z - v0.z;
    double e2x = v2.x - v0.x, e2y = v2.y - v0.y, e2z = v2.z - v0.z;
    
    // Calculate normal (cross product)
    double nx = e1y * e2z - e1z * e2y;
    double ny = e1z * e2x - e1x * e2z;
    double nz = e1x * e2y - e1y * e2x;
    
    // Add to vertex normals
    normals_[tri.v0].x += nx; normals_[tri.v0].y += ny; normals_[tri.v0].z += nz;
    normals_[tri.v1].x += nx; normals_[tri.v1].y += ny; normals_[tri.v1].z += nz;
    normals_[tri.v2].x += nx; normals_[tri.v2].y += ny; normals_[tri.v2].z += nz;
  }
  
  // Normalize
  for (auto& n : normals_) {
    double len = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    if (len > 0.0) {
      n.x /= len;
      n.y /= len;
      n.z /= len;
    }
  }
}

double MeshGenerator::interpolate_depth(double x, double y) const {
  // Simple inverse distance weighted interpolation
  double sum_weights = 0.0;
  double sum_values = 0.0;
  const double power = 2.0;
  const double epsilon = 1e-10;
  
  for (const auto& point : bathymetry_data_) {
    double dx = x - point.lon;
    double dy = y - point.lat;
    double dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist < epsilon) {
      // Point is exactly on a data point
      return point.depth;
    }
    
    double weight = 1.0 / std::pow(dist, power);
    sum_weights += weight;
    sum_values += weight * point.depth;
  }
  
  if (sum_weights > 0.0) {
    return sum_values / sum_weights;
  }
  
  return 0.0;  // Default depth if no data available
}

bool MeshGenerator::write_gltf() {
  write_gltf_ascii();
  return true;
}

void MeshGenerator::write_gltf_ascii() {
  std::ofstream file(options_.output_file());
  if (!file.is_open()) {
    LOG_ERROR("Cannot open output file: " + options_.output_file());
    return;
  }

  // Write minimal GLTF 2.0 JSON
  file << "{\n";
  file << "  \"asset\": {\n";
  file << "    \"version\": \"2.0\",\n";
  file << "    \"generator\": \"MB-System mb-mesh\"\n";
  file << "  },\n";
  file << "  \"scene\": 0,\n";
  file << "  \"scenes\": [\n";
  file << "    {\n";
  file << "      \"nodes\": [0]\n";
  file << "    }\n";
  file << "  ],\n";
  file << "  \"nodes\": [\n";
  file << "    {\n";
  file << "      \"mesh\": 0\n";
  file << "    }\n";
  file << "  ],\n";
  file << "  \"meshes\": [\n";
  file << "    {\n";
  file << "      \"primitives\": [\n";
  file << "        {\n";
  file << "          \"attributes\": {\n";
  file << "            \"POSITION\": 0,\n";
  file << "            \"NORMAL\": 1\n";
  file << "          },\n";
  file << "          \"indices\": 2\n";
  file << "        }\n";
  file << "      ]\n";
  file << "    }\n";
  file << "  ],\n";
  file << "  \"accessors\": [\n";
  file << "    {\n";
  file << "      \"bufferView\": 0,\n";
  file << "      \"componentType\": 5126,\n";
  file << "      \"count\": " << vertices_.size() << ",\n";
  file << "      \"type\": \"VEC3\",\n";
  file << "      \"min\": [" << min_lon_ << ", " << min_lat_ << ", " << min_depth_ * options_.vertical_exaggeration() << "],\n";
  file << "      \"max\": [" << max_lon_ << ", " << max_lat_ << ", " << max_depth_ * options_.vertical_exaggeration() << "]\n";
  file << "    },\n";
  file << "    {\n";
  file << "      \"bufferView\": 1,\n";
  file << "      \"componentType\": 5126,\n";
  file << "      \"count\": " << normals_.size() << ",\n";
  file << "      \"type\": \"VEC3\"\n";
  file << "    },\n";
  file << "    {\n";
  file << "      \"bufferView\": 2,\n";
  file << "      \"componentType\": 5125,\n";
  file << "      \"count\": " << triangles_.size() * 3 << ",\n";
  file << "      \"type\": \"SCALAR\"\n";
  file << "    }\n";
  file << "  ],\n";
  file << "  \"bufferViews\": [\n";
  file << "    {\n";
  file << "      \"buffer\": 0,\n";
  file << "      \"byteOffset\": 0,\n";
  file << "      \"byteLength\": " << vertices_.size() * 12 << "\n";
  file << "    },\n";
  file << "    {\n";
  file << "      \"buffer\": 0,\n";
  file << "      \"byteOffset\": " << vertices_.size() * 12 << ",\n";
  file << "      \"byteLength\": " << normals_.size() * 12 << "\n";
  file << "    },\n";
  file << "    {\n";
  file << "      \"buffer\": 0,\n";
  file << "      \"byteOffset\": " << (vertices_.size() + normals_.size()) * 12 << ",\n";
  file << "      \"byteLength\": " << triangles_.size() * 12 << "\n";
  file << "    }\n";
  file << "  ],\n";
  file << "  \"buffers\": [\n";
  file << "    {\n";
  file << "      \"byteLength\": " << (vertices_.size() + normals_.size() + triangles_.size() * 3) * 4 << "\n";
  file << "    }\n";
  file << "  ]\n";
  file << "}\n";

  file.close();
  
  LOG_INFO("GLTF file written successfully");
}

}  // namespace mbmesh
