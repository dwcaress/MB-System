#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "../data_types/geometry.h"
#include "draco_mesh.h"
#include "provenance.h"

// The georeferencing and provenance metadata of the run (see provenance.h and
// GEOREFERENCE.md). Once set, every GLB written gets it as asset.extras, with a
// "MB-System.content" block saying what that file holds. Set it once, before
// writing; it is not meant to change while files are being written. Without it,
// GLBs are written without metadata, as before.
void set_glb_run_metadata(const MetaValue &run_metadata);
void clear_glb_run_metadata();

// Draco compression (KHR_draco_mesh_compression) of the triangle meshes that write_mesh_glb_file() writes: mesh.glb, raw_mesh.glb,
// the chunks and the tiles of a tileset, as mbgrd2gltf compresses its mesh (src/mbdraco is the code they share). Point clouds,
// lines and the track are not compressed, since the extension is for triangles. The positions are quantized over the largest range
// of the three axes of the mesh, so the number of bits is chosen for each file from the range, to be no coarser than
// settings.precision meters; the normals are quantized in settings.normal_bits bits.
// Until set, the meshes are not compressed. A build without Draco (mbdraco::available() false) writes them uncompressed.
void set_glb_compression(const mbdraco::Settings &settings);

// The primitive that each triangle of a mesh goes in, for write_mesh_glb_file: the triangles whose centers are in the same square
// of a lattice of block_size meters (anchored at the origin of the mesh's frame, so the squares are the chunks when block_size is the
// chunk size) are one group. The mesh must be in the local east-north-up frame, in meters. Empty if block_size is not more than zero.
[[nodiscard]] std::vector<std::uint32_t> triangle_blocks(const Mesh &mesh, double block_size);

// Writes the mesh as one mesh of one primitive for each group of triangles (one primitive of everything if there are no groups), as
// mbgrd2gltf writes the blocks of a grid: each primitive has the vertices its triangles use, numbered from zero, and is a Draco stream
// by itself when Draco is on, so no part of a viewer has to hold the whole surface in one array. The vertices on the edge of a group are
// in both. triangle_groups holds the group of each triangle, one for each, numbered from zero without gaps.
[[nodiscard]] bool write_mesh_glb_file(
    const std::filesystem::path &path,
    const Mesh &mesh,
    std::string *error,
    const GlbContent &content = GlbContent(),
    const std::vector<std::uint32_t> *triangle_groups = nullptr);

// Colors the vertices of a mesh by height, for tiles: with the Haxby table, the color changing linearly between the bounds (11
// heights, from haxby_height_bounds), at a height of the vertex's z plus height_offset. The bounds come from the heights of the whole
// survey, so that every tile and every level has the same colors. write_mesh_glb_file then writes them as COLOR_0 (red, green, blue
// bytes, and Draco-compressed with the rest), with a white base color so that they show as they are.
[[nodiscard]] std::vector<double> haxby_height_bounds(double minimum, double maximum);
void color_mesh_by_height(Mesh *mesh, double height_offset, const std::vector<double> &bounds);

// How the points of a point cloud are colored by height: with the Haxby color table that mbm_grdplot uses by default, 11
// colors from dark blue to white, changing linearly between 11 heights. HaxbyLinear is "mbm_grdplot -G5": the heights
// are equally spaced, at rounded values chosen from the range. HaxbyEqualized is "mbm_grdplot -G5 -S": the heights
// divide the points into cells of equal numbers. None is one orange.
enum class PointColorMap { None, HaxbyLinear, HaxbyEqualized };

// With heights, one value for each point, and a colormap other than None, the points get a COLOR_0 attribute that
// colors them by it (see colors_for_values in glb_writer.cpp), and the heights the color changes between are added to
// the content description. Without heights, or with None, the points are one orange.
// The names --pointcloud-colormap takes, "linear, equalized, none", and the parsing of one.
[[nodiscard]] const char *point_color_map_names();
[[nodiscard]] const char *point_color_map_name(PointColorMap colormap);
[[nodiscard]] bool parse_point_color_map(const std::string &name, PointColorMap *colormap);

[[nodiscard]] bool write_pointcloud_glb_file(
    const std::filesystem::path &path,
    const PointCloud &pointcloud,
    std::string *error,
    const GlbContent &content = GlbContent(),
    const std::vector<double> *heights = nullptr,
    PointColorMap colormap = PointColorMap::HaxbyLinear);

[[nodiscard]] bool write_normal_lines_glb_file(
    const std::filesystem::path &path,
    const OrientedPointCloud &pointcloud,
    double line_length,
    std::string *error,
    const GlbContent &content = GlbContent());

// Draws polylines, for example the sensor track, as line segments. Positions are
// written as given, so they must already be in the output frame.
[[nodiscard]] bool write_track_glb_file(
    const std::filesystem::path &path,
    const std::vector<std::vector<Vec3>> &polylines,
    std::string *error,
    const GlbContent &content = GlbContent());

[[nodiscard]] bool write_origin_ray_lines_glb_file(
    const std::filesystem::path &path,
    const CollectedPointCloud &pointcloud,
    double line_length,
    std::string *error,
    const GlbContent &content = GlbContent());
