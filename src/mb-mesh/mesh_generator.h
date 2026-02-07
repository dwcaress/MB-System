/*--------------------------------------------------------------------
 *    The MB-system:  mesh_generator.h  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef MB_MESH_GENERATOR_H
#define MB_MESH_GENERATOR_H

#include <vector>
#include <string>
#include "mesh_options.h"

namespace mbmesh {

/**
 * @brief Structure to hold a 3D vertex
 */
struct Vertex {
  double x, y, z;
  Vertex(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
};

/**
 * @brief Structure to hold a triangle (3 vertex indices)
 */
struct Triangle {
  int v0, v1, v2;
  Triangle(int i0, int i1, int i2) : v0(i0), v1(i1), v2(i2) {}
};

/**
 * @brief Bathymetry data point
 */
struct BathymetryPoint {
  double lon, lat, depth;
  BathymetryPoint(double lon_, double lat_, double depth_) 
    : lon(lon_), lat(lat_), depth(depth_) {}
};

/**
 * @brief Main mesh generator class
 */
class MeshGenerator {
public:
  explicit MeshGenerator(const MeshOptions& options);

  // Main processing pipeline
  bool load_data();
  bool generate_mesh();
  bool write_gltf();

private:
  // Internal processing methods
  void create_regular_grid();
  void triangulate();
  void apply_decimation();
  void compute_normals();
  
  // Helper methods
  double interpolate_depth(double x, double y) const;
  void write_gltf_ascii();
  void write_gltf_binary();

  const MeshOptions& options_;
  
  // Data storage
  std::vector<BathymetryPoint> bathymetry_data_;
  std::vector<Vertex> vertices_;
  std::vector<Triangle> triangles_;
  std::vector<Vertex> normals_;
  
  // Bounds
  double min_lon_, max_lon_;
  double min_lat_, max_lat_;
  double min_depth_, max_depth_;
};

}  // namespace mbmesh

#endif  // MB_MESH_GENERATOR_H
