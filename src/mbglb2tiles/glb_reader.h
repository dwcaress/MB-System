#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

#include "data_types/geometry.h"

// Reads the triangle mesh of a GLB that mbmesh or mbgrd2gltf wrote: every primitive of every mesh, Draco-compressed (KHR_draco_mesh_compression) or
// plain, joined into one mesh, with the vertices as the file holds them. What the file says about where they are comes from the "georeference" block of
// asset.extras: with vertexCoordinates "relativeToCenter" the position on the Earth is the vertex plus center (WGS 84 ECEF, meters), and with "absolute" it
// is the vertex. The mesh is not welded: the primitives each have their own copies of the vertices on their edges.
struct GlbMesh {
    Mesh mesh;                       // vertices as stored; no normals
    bool has_center = false;
    Vec3 center;                     // ECEF, when has_center
    double position_step = 0.0;      // the step of the Draco quantization, in meters, that the file says; 0 if it does not say
    std::size_t primitives = 0;
    std::size_t draco_primitives = 0;
    std::string generator;
    std::string description;
};

// The Draco primitives are decoded on up to threads threads at once, and the progress is logged. Returns false, with the reason in *error, if the file
// cannot be read, has no georeference block, or holds anything but triangles.
[[nodiscard]] bool read_mesh_glb(const std::filesystem::path &path, GlbMesh *result, std::string *error, int threads = 1);
