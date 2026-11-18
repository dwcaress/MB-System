#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../data_types/geometry.h"
#include "../data_types/swathfile.h"
#include "provenance.h"

/*
 * OGC 3D Tiles output: the chunks of a chunked reconstruction as the tiles of a tileset that a viewer can load a part at
 * a time, instead of one file of the whole surface.
 *
 * A tile's content is a GLB in the local east-north-up frame of the reconstruction, written Y-up as glTF
 * requires and as 3D Tiles expects (X east, Y up, Z south). Loaders rotate it to Z-up (the frame of the
 * tileset, east-north-up), and the transform of the root tile, which is the east-north-up frame at the
 * frame origin expressed in WGS 84 ECEF, places the tileset on the Earth. Bounding volumes are boxes in
 * the east-north-up frame, which is the tileset's coordinate system before the transform.
 */

// One tile of the tileset: its content and where it is, in the local east-north-up frame (X east, Y north, Z up).
struct TilesetTile {
    std::string uri;           // relative to tileset.json
    double minimum[3] = {0.0, 0.0, 0.0};
    double maximum[3] = {0.0, 0.0, 0.0};
};

// The mesh in the form tileset content has: local east-north-up (x east, y north, z up) to glTF Y-up
// (x east, y up, z south), a rotation, so the winding is kept.
[[nodiscard]] Mesh tileset_content_mesh(const Mesh &local_mesh);

// The bounds of a mesh in local east-north-up coordinates. Returns false for an empty mesh.
[[nodiscard]] bool mesh_bounds(const Mesh &mesh, double minimum[3], double maximum[3]);

// The east-north-up frame at the origin of the frame, in WGS 84 ECEF, as the 16 numbers of a 3D Tiles
// transform: a 4x4 matrix in column-major order, whose columns are east, north, up and the origin.
[[nodiscard]] std::vector<double> tileset_transform(const CoordinateFrame &frame);

// X3DOM's GeoTileset takes the height of the camera as at least 1 m above the ellipsoid, so tiles of data below the ellipsoid, as seafloor
// is, never get near enough to refine. A tileset is therefore lifted: the frame is put higher along its vertical, in the transform of
// tileset.json. The default lift puts the lowest point 10 m above the ellipsoid, to a meter, if it is lower (zero if not).
[[nodiscard]] double default_tileset_lift(double lowest_height);

// The frame with its origin lift meters higher along its up axis, for the transform of a lifted tileset.
[[nodiscard]] CoordinateFrame lifted_frame(const CoordinateFrame &frame, double lift);

// Records the lift in the run metadata that is the tileset's extras, as georeference.liftMeters.
// Records in the metadata that the tiles are colored by height, with the heights (ellipsoid height = bounds + height_offset) that the
// color changes between, for the page to say so and to offer the color off.
void set_metadata_height_colors(MetaValue *metadata, double height_offset, const std::vector<double> &bounds);

void set_metadata_lift(MetaValue *metadata, double lift);

// mbmesh --tileset and mbglb2tiles write the same layout into the same directory, tileset/ beside mesh.glb: tileset.json and a directory
// L<level> of tiles tile_I_J.glb for each level, L0 the finest. A directory that has a tileset.json already holds a tileset of an earlier
// run, of either program. This removes what such a tileset has that the next one may not overwrite: the tileset.json, the L<level> directories
// and the tile_I_J.glb files left directly in it by older versions. It returns how many files it removed, and removes nothing from a
// directory without a tileset.json, whatever else is in it.
std::size_t remove_previous_tileset(const std::filesystem::path &directory);

// Writes tileset.json: a root tile with the transform and no content, whose children are the tiles, each a
// leaf. The geometric error of the root is the diagonal of its bounding box, so a viewer always refines to
// the tiles, and that of each tile is 0. If extras is not empty it is written as the tileset's extras.
[[nodiscard]] bool write_tileset_json(
    const std::filesystem::path &path,
    const CoordinateFrame &frame,
    const std::vector<TilesetTile> &tiles,
    const MetaValue &extras,
    const std::string &generator,
    std::string *error);

// A tileset of levels of detail, as mbglb2tiles writes it: each tile is a coarser version of what its children hold together, and a
// viewer replaces it with them as the camera comes near.
struct TilesetNode {
    std::string uri;                      // relative to tileset.json; empty for a tile without content
    double minimum[3] = {0.0, 0.0, 0.0};  // the box of the content and of all the children, in the east-north-up frame
    double maximum[3] = {0.0, 0.0, 0.0};
    double geometric_error = 0.0;         // meters; 0 for the finest tiles
    std::vector<TilesetNode> children;
};

// Writes tileset.json for the tree of tiles under root: the transform is on the root, the refinement is REPLACE for all, and the
// geometric error of the tileset is the diagonal of the box of the root. If extras is not empty it is written as the tileset's extras.
[[nodiscard]] bool write_tileset_tree_json(
    const std::filesystem::path &path,
    const CoordinateFrame &frame,
    const TilesetNode &root,
    const MetaValue &extras,
    const std::string &generator,
    std::string *error);
