// The OGC 3D Tiles output: tile content in the local east-north-up frame written Y-up, and tileset.json,
// whose root transform places the tiles on the Earth. The chain that a viewer follows, glTF Y-up to
// Z-up (a quarter turn about X, as X3DOM and every 3D Tiles loader do) and then the transform, must give
// back the ECEF position of each vertex.

#include "io/coordinate_frame.h"
#include "io/tileset_writer.h"

#include "tinygltf/json.hpp"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

bool near(double a, double b, double tolerance) {
    return std::fabs(a - b) <= tolerance;
}

} // namespace

int main() {
    bool ok = true;

    GeodeticPosition origin;
    origin.longitude = -122.4;
    origin.latitude = 36.6;
    origin.elevation = 0.0;
    const CoordinateFrame frame = coordinate_frame_from_origin(origin);

    // ---- the content of a tile: east-north-up to glTF Y-up, a rotation
    Mesh local;
    local.vertices = {Vec3(10.0, 20.0, -2000.0), Vec3(11.0, 20.0, -2000.0), Vec3(10.0, 21.0, -1999.0)};
    local.normals = {Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)};
    local.indices = {0, 1, 2};
    const Mesh content = tileset_content_mesh(local);
    ok &= check(content.vertices.size() == 3 && content.indices == local.indices, "the content keeps its vertices and indices");
    ok &= check(near(content.vertices[0].x, 10.0, 0) && near(content.vertices[0].y, -2000.0, 0) && near(content.vertices[0].z, -20.0, 0),
                "east stays X, up becomes Y and north becomes -Z");
    ok &= check(near(content.normals[0].x, 0.0, 0) && near(content.normals[0].y, 1.0, 0) && near(content.normals[0].z, 0.0, 0), "an up normal is +Y");

    // The face normal, by the winding, is the original one rotated the same way, so the winding is kept.
    const Vec3 before = cross(local.vertices[1] - local.vertices[0], local.vertices[2] - local.vertices[0]);
    const Vec3 after = cross(content.vertices[1] - content.vertices[0], content.vertices[2] - content.vertices[0]);
    ok &= check(near(after.x, before.x, 1e-12) && near(after.y, before.z, 1e-12) && near(after.z, -before.y, 1e-12), "the winding is kept");

    // ---- bounds
    double minimum[3], maximum[3];
    ok &= check(mesh_bounds(local, minimum, maximum) && minimum[0] == 10.0 && maximum[0] == 11.0 && minimum[1] == 20.0 && maximum[1] == 21.0 &&
                    minimum[2] == -2000.0 && maximum[2] == -1999.0,
                "the bounds of a mesh, in east, north, up");
    ok &= check(!mesh_bounds(Mesh(), minimum, maximum), "an empty mesh has no bounds");

    // ---- the transform: the east-north-up frame in ECEF, column-major
    const std::vector<double> transform = tileset_transform(frame);
    ok &= check(transform.size() == 16 && transform[3] == 0.0 && transform[7] == 0.0 && transform[11] == 0.0 && transform[15] == 1.0,
                "a 4x4 transform with a last row of 0 0 0 1");
    ok &= check(near(transform[12], frame.origin_ecef.x, 1e-9) && near(transform[13], frame.origin_ecef.y, 1e-9) && near(transform[14], frame.origin_ecef.z, 1e-9),
                "the last column is the origin in ECEF");

    // The chain a viewer follows: Y-up to Z-up, (x, y, z) to (x, -z, y), then the transform.
    double worst = 0.0;
    for (const Vec3 &vertex : content.vertices) {
        const double enu[3] = {vertex.x, -vertex.z, vertex.y};
        double ecef[3];
        for (int i = 0; i < 3; i++) {
            ecef[i] = transform[12 + i] + transform[0 + i] * enu[0] + transform[4 + i] * enu[1] + transform[8 + i] * enu[2];
        }
        // against the ECEF position of the original local vertex
        const Vec3 expected = local_to_ecef(local.vertices[static_cast<std::size_t>(&vertex - &content.vertices[0])], frame);
        worst = std::fmax(worst, std::fmax(std::fabs(ecef[0] - expected.x), std::fmax(std::fabs(ecef[1] - expected.y), std::fabs(ecef[2] - expected.z))));
    }
    ok &= check(worst < 1e-6, "Y-up to Z-up and the transform give back the ECEF position of each vertex");

    // ---- tileset.json
    const std::filesystem::path directory = std::filesystem::temp_directory_path() / "mbmesh_test_tileset";
    std::filesystem::create_directories(directory);
    TilesetTile first, second;
    first.uri = "tile_0_0.glb";
    first.minimum[0] = 0;   first.minimum[1] = 0;   first.minimum[2] = -2010;
    first.maximum[0] = 40;  first.maximum[1] = 30;  first.maximum[2] = -1990;
    second.uri = "tile 1_0.glb";   // needs escaping to be a path in JSON? no, but keep a space to check strings
    second.minimum[0] = 40; second.minimum[1] = 0;  second.minimum[2] = -2005;
    second.maximum[0] = 80; second.maximum[1] = 30; second.maximum[2] = -1995;

    MetaValue extras = MetaValue::make_object();
    extras.set("note", MetaValue::from_string("a \"quoted\" note\nwith a line"));
    extras.set("count", MetaValue::from_number(2));
    MetaValue list = MetaValue::make_array();
    list.add(MetaValue::from_number(1.5));
    list.add(MetaValue::from_bool(true));
    extras.set("list", list);

    std::string error;
    const std::filesystem::path path = directory / "tileset.json";
    ok &= check(write_tileset_json(path, frame, {first, second}, extras, "test", &error), "tileset.json is written: " + error);

    std::ifstream input(path);
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(text);
    } catch (const std::exception &exception) {
        ok &= check(false, std::string("tileset.json is valid JSON: ") + exception.what());
        return 1;
    }

    ok &= check(json["asset"]["version"] == "1.1" && json["asset"]["generator"] == "test", "the tileset says it is 3D Tiles 1.1");
    const nlohmann::json &root = json["root"];
    ok &= check(root["transform"].size() == 16 && near(root["transform"][12].get<double>(), frame.origin_ecef.x, 1e-9), "the root has the transform");
    ok &= check(!root.contains("content"), "the root has no content, only children");
    ok &= check(root["refine"] == "ADD" && root["children"].size() == 2, "the root adds its two children");

    // The root box covers both tiles: center (40, 15, -2000), half axes (40, 15, 10).
    const nlohmann::json &box = root["boundingVolume"]["box"];
    ok &= check(box.size() == 12 && near(box[0].get<double>(), 40, 1e-9) && near(box[1].get<double>(), 15, 1e-9) && near(box[2].get<double>(), -2000, 1e-9) &&
                    near(box[3].get<double>(), 40, 1e-9) && near(box[7].get<double>(), 15, 1e-9) && near(box[11].get<double>(), 10, 1e-9) &&
                    box[4] == 0 && box[5] == 0 && box[6] == 0 && box[8] == 0 && box[9] == 0 && box[10] == 0,
                "the root box is the center and half axes of both tiles, along east, north and up");
    const double diagonal = std::sqrt(80.0 * 80.0 + 30.0 * 30.0 + 20.0 * 20.0);
    ok &= check(near(root["geometricError"].get<double>(), diagonal, 1e-9) && near(json["geometricError"].get<double>(), diagonal, 1e-9),
                "the geometric error of the tileset and of the root is the diagonal, so a viewer refines to the tiles");

    const nlohmann::json &child = root["children"][0];
    ok &= check(child["content"]["uri"] == "tile_0_0.glb" && child["geometricError"] == 0 && !child.contains("children"), "a tile is a leaf with no error and its content");
    ok &= check(near(child["boundingVolume"]["box"][0].get<double>(), 20, 1e-9) && near(child["boundingVolume"]["box"][3].get<double>(), 20, 1e-9) &&
                    near(child["boundingVolume"]["box"][11].get<double>(), 10, 1e-9),
                "the box of a tile");
    ok &= check(root["children"][1]["content"]["uri"] == "tile 1_0.glb", "a uri is kept as given");

    ok &= check(json["extras"]["note"] == "a \"quoted\" note\nwith a line" && json["extras"]["count"] == 2 && json["extras"]["list"][0] == 1.5 &&
                    json["extras"]["list"][1] == true,
                "the extras are written, with strings escaped");

    // No extras, no member.
    ok &= check(write_tileset_json(directory / "plain.json", frame, {first}, MetaValue(), "test", &error), "a tileset without extras is written");
    std::ifstream plain_input(directory / "plain.json");
    const nlohmann::json plain = nlohmann::json::parse(std::string((std::istreambuf_iterator<char>(plain_input)), std::istreambuf_iterator<char>()));
    ok &= check(!plain.contains("extras") && plain["root"]["children"].size() == 1, "without extras there is no extras member");

    // Nothing to write.
    ok &= check(!write_tileset_json(directory / "none.json", frame, {}, MetaValue(), "test", &error) && !error.empty() && !std::filesystem::exists(directory / "none.json"),
                "a tileset with no tiles is refused");

    std::filesystem::remove_all(directory);
    if (ok) {
        std::printf("OK: tileset\n");
    }
    return ok ? 0 : 1;
}
