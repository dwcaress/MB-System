/*--------------------------------------------------------------------
 *    The MB-system:	provenance.h	10/22/2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The georeference and provenance metadata that mbgrd2gltf writes in the
 * extras of its glTF file and in the GeoMetadata and WorldInfo nodes of its
 * X3D page. One tree feeds both, so they cannot disagree.
 *
 * The names are those proposed in src/mbmesh/GEOREFERENCE.md, which mbmesh
 * uses too: "georeference" (where the model is), "provenance" (how it was
 * made) and "MB-System" (details particular to this program).
 */

#ifndef PROVENANCE_H
#define PROVENANCE_H

#include "draco_mesh.h"
#include <cstddef>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace mbgrd2gltf {

// A JSON-like value that keeps the order its members were added in.
struct MetaValue {
  enum class Type { String, Number, Bool, Array, Object };
  Type type = Type::Object;
  std::string text;
  double number = 0.0;
  bool flag = false;
  std::vector<MetaValue> items;                           // Array
  std::vector<std::pair<std::string, MetaValue>> members; // Object

  static MetaValue from_string(const std::string& value);
  static MetaValue from_number(double value);
  static MetaValue from_bool(bool value);
  static MetaValue make_array();
  static MetaValue make_object();
  MetaValue& set(const std::string& key, MetaValue value);
  MetaValue& add(MetaValue value);
  const MetaValue* find(const std::string& key) const;
};

// A number as text: integers as integers, anything else with 12 significant digits.
std::string metadata_number_text(double value);

// Every leaf as a dotted path (arrays as name[i]) and its text.
std::vector<std::pair<std::string, std::string>> flatten_metadata(const MetaValue& value,
                                                                 const std::string& path = "");

// A time in seconds since 1970 as ISO 8601 UTC, "2026-10-22T16:09:29Z".
std::string iso_utc_time(double unix_seconds);

struct GridRunInfo {
  std::string title;

  // The software and the execution.
  std::string program = "mbgrd2gltf";
  std::string version;
  std::string command_line;
  std::string working_directory;
  std::string started_at; // ISO 8601 UTC
  std::string user;
  std::string host;

  // The input grid.
  std::string grid_file; // absolute path
  size_t grid_columns = 0;
  size_t grid_rows = 0;
  double longitude_spacing = 0.0; // degrees
  double latitude_spacing = 0.0;
  bool pixel_registered = false;

  // Where the model is.
  std::string geoorigin_mode = "none"; // "none", "automatic" or "specified"
  bool has_center = false;
  double center_x = 0.0, center_y = 0.0, center_z = 0.0; // ECEF, meters
  double center_longitude = 0.0, center_latitude = 0.0, center_height = 0.0;
  double extent_west = 0.0, extent_south = 0.0, extent_east = 0.0, extent_north = 0.0;
  double mean_altitude = 0.0;

  // How it was made.
  double exaggeration = 1.0;
  bool binary = false;
  // What the mesh was compressed with (KHR_draco_mesh_compression), filled in by write_gltf(): for the many blocks of the file, the
  // most bits and the coarsest step. Not present when the file is written as arrays.
  mbdraco::Compression draco;
};

// The three blocks of asset.extras: "georeference", "provenance" and "MB-System".
MetaValue grid_run_metadata(const GridRunInfo& run);

// The keyword and value strings of the X3D GeoMetadata summary field, which takes
// only the keywords the X3D specification lists. There is always an even number.
std::vector<std::string> geo_metadata_summary(const GridRunInfo& run);

// Writes the WorldInfo and GeoMetadata nodes for the page's scene, at the given indent.
void write_x3d_metadata(std::ostream& out, const std::string& title, const MetaValue& metadata,
                        const std::vector<std::string>& summary, const std::string& indent);

} // namespace mbgrd2gltf

#endif
