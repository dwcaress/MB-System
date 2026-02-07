/*--------------------------------------------------------------------
 *    The MB-system:  mesh_options.h  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef MB_MESH_OPTIONS_H
#define MB_MESH_OPTIONS_H

#include <string>

namespace mbmesh {

/**
 * @brief Command line options for mb-mesh
 */
class MeshOptions {
public:
  MeshOptions(int argc, char* argv[]);

  bool is_help() const { return help_; }
  bool is_verbose() const { return verbose_; }
  
  const std::string& input_file() const { return input_file_; }
  const std::string& output_file() const { return output_file_; }
  
  // Mesh generation parameters
  double grid_spacing() const { return grid_spacing_; }
  double vertical_exaggeration() const { return vertical_exaggeration_; }
  int decimation_level() const { return decimation_level_; }
  bool use_draco_compression() const { return use_draco_; }
  
  // Mesh quality settings
  int max_triangles() const { return max_triangles_; }
  double edge_threshold() const { return edge_threshold_; }

private:
  void parse_arguments(int argc, char* argv[]);
  void print_usage();

  bool help_;
  bool verbose_;
  std::string input_file_;
  std::string output_file_;
  
  double grid_spacing_;           // Spacing between mesh vertices (meters)
  double vertical_exaggeration_;  // Vertical exaggeration factor
  int decimation_level_;          // Level of mesh decimation (0-10)
  bool use_draco_;                // Use Draco compression
  int max_triangles_;             // Maximum number of triangles
  double edge_threshold_;         // Edge collapse threshold
};

}  // namespace mbmesh

#endif  // MB_MESH_OPTIONS_H
