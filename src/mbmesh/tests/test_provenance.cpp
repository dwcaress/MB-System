// The georeference and provenance metadata must say the same thing in every place it
// is written: the extras of each GLB, and the GeoMetadata and WorldInfo of the page.

#include "io/glb_writer.h"
#include "io/provenance.h"
#include "io/x3dom_writer.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

bool contains(const std::string &text, const std::string &part) {
    return text.find(part) != std::string::npos;
}

// The same flattening as flatten_metadata(), of a value read back from a GLB.
void flatten_value(const tinygltf::Value &value, const std::string &path, std::map<std::string, std::string> *out) {
    if (value.IsObject()) {
        for (const std::string &key : value.Keys()) {
            if (key[0] == '@') {
                continue;   // JSON-LD keywords are not values
            }
            flatten_value(value.Get(key), path.empty() ? key : path + "." + key, out);
        }
    } else if (value.IsArray()) {
        for (std::size_t i = 0; i < value.ArrayLen(); i++) {
            flatten_value(value.Get(static_cast<int>(i)), path + "[" + std::to_string(i) + "]", out);
        }
    } else if (value.IsString()) {
        (*out)[path] = value.Get<std::string>();
    } else if (value.IsBool()) {
        (*out)[path] = value.Get<bool>() ? "true" : "false";
    } else if (value.IsNumber()) {
        (*out)[path] = metadata_number_text(value.GetNumberAsDouble());
    }
}

RunInfo example_run(bool with_center) {
    RunInfo run;
    run.title = "OceanImaging2018/20180923d1/multibeam";
    run.version = "5.8.3beta25";
    run.command_line = "mbmesh -I datalist.mb-1 -L 0.5 --geoorigin=-122.4,36.6,-10 --html 'a \"quoted\" <name>'";
    run.working_directory = "/data/run";
    run.started_at = "2026-10-22T16:09:29Z";
    run.user = "mccann";
    run.host = "menard";
    run.datalist = "/data/run/datalist.mb-1";
    run.formats = {88, 89};
    run.file_count = 2;
    run.files = {"/data/run/a.mb89", "/data/run/b.mb88"};
    run.pings = 100;
    run.soundings_read = 5000;
    run.soundings_accepted = 4000;
    run.soundings_flagged = 900;
    run.soundings_out_of_bounds = 100;
    run.first_time = "2018-09-23T15:31:41Z";
    run.last_time = "2018-09-23T23:59:12Z";
    run.extent_west = -122.39995;
    run.extent_south = 36.63694;
    run.extent_east = -122.39939;
    run.extent_north = 36.63724;
    run.level_of_detail = 0.5;
    run.poisson_cell_size = 0.125;
    run.normal_search_radius = 0.3;
    run.point_spacing = 0.07;
    run.chunked = true;
    run.chunk_size = 100;
    run.halo = 2;
    run.threads = 4;
    if (with_center) {
        run.geoorigin_mode = "specified";
        run.has_center = true;
        run.center_x = -2749436.658;
        run.center_y = -4344156.687;
        run.center_z = 3760606.005;
        run.center_longitude = -122.399674;
        run.center_latitude = 36.63706;
        run.center_height = -2228.12;
    }
    return run;
}

bool write_and_read(const std::filesystem::path &path, const GlbContent &content, tinygltf::Model *model) {
    Mesh mesh;
    mesh.vertices = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0)};
    mesh.normals = {Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)};
    mesh.indices = {0, 1, 2};
    std::string error;
    if (!write_mesh_glb_file(path, mesh, &error, content)) {
        std::fprintf(stderr, "FAIL: writing %s: %s\n", path.string().c_str(), error.c_str());
        return false;
    }
    tinygltf::TinyGLTF loader;
    std::string load_error;
    std::string warning;
    if (!loader.LoadBinaryFromFile(model, &load_error, &warning, path.string())) {
        std::fprintf(stderr, "FAIL: reading %s: %s\n", path.string().c_str(), load_error.c_str());
        return false;
    }
    return true;
}

X3DomModel page_model() {
    X3DomModel model;
    model.uri = "mbmesh_output/mesh.glb";
    model.label = "mesh";
    model.group = "mbmesh_output";
    model.visible = true;
    return model;
}

std::string read_file(const std::filesystem::path &path) {
    std::ifstream input(path);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

} // namespace

int main() {
    bool ok = true;
    const std::filesystem::path directory = std::filesystem::temp_directory_path() / "mbmesh_test_provenance";
    std::filesystem::create_directories(directory);

    // ---- a point cloud GLB is colored by the heights it is given: COLOR_0, low dark, high yellow ----
    {
        PointCloud points;
        std::vector<double> heights;
        for (int i = 0; i < 100; i++) {
            points.push_back(Vec3(i, 0.0, 0.0));
            heights.push_back(-100.0 + i);
        }
        const std::filesystem::path path = directory / "points.glb";
        std::string error;
        ok &= check(write_pointcloud_glb_file(path, points, &error, GlbContent{"pointcloud", "points"}, &heights, PointColorMap::HaxbyEqualized),
                    "colored point cloud is written");
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string load_error, warning;
        ok &= check(loader.LoadBinaryFromFile(&model, &load_error, &warning, path.string()), "colored point cloud is read back");
        const tinygltf::Primitive &primitive = model.meshes.at(0).primitives.at(0);
        ok &= check(primitive.mode == TINYGLTF_MODE_POINTS && primitive.attributes.count("COLOR_0") == 1, "the points have COLOR_0");
        if (primitive.attributes.count("COLOR_0") == 1) {
            const tinygltf::Accessor &accessor = model.accessors.at(primitive.attributes.at("COLOR_0"));
            ok &= check(accessor.count == 100 && accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT,
                        "COLOR_0 is a float RGB color for each point");
            const tinygltf::BufferView &view = model.bufferViews.at(accessor.bufferView);
            const float *rgb = reinterpret_cast<const float *>(model.buffers.at(view.buffer).data.data() + view.byteOffset + accessor.byteOffset);
            const auto near = [](float value, double expected) { return std::fabs(value - expected / 255.0) < 1e-3; };
            ok &= check(rgb[0] < 60.0 / 255.0 && rgb[2] > 170.0 / 255.0 && rgb[1] < 130.0 / 255.0, "the lowest point is dark blue");
            // The cell bounds are the 100 sorted heights at 100 * k / 11 (less one), so point 26 is the fourth bound, with the fourth Haxby color.
            ok &= check(near(rgb[26 * 3], 106.0) && near(rgb[26 * 3 + 1], 235.0) && near(rgb[26 * 3 + 2], 255.0), "a point at the fourth bound has the fourth Haxby color");
            ok &= check(near(rgb[99 * 3], 255.0) && near(rgb[99 * 3 + 1], 255.0) && near(rgb[99 * 3 + 2], 255.0), "a point above the last bound is white");
        }
        ok &= check(model.materials.at(0).pbrMetallicRoughness.baseColorFactor.at(0) == 1.0 && model.materials.at(0).pbrMetallicRoughness.baseColorFactor.at(2) == 1.0,
                    "the base color is white, so that the colors are the points' own");

        // The linear map, the default: 11 equal steps between rounded heights. For 100 heights from -100 to -1 the step is 11.25 from -101.25,
        // so the highest point, -1, is 0.911 of the way from the ninth Haxby color (255,161,68) to the tenth (255,186,133).
        tinygltf::Model plain;
        {
            set_glb_run_metadata(run_metadata(example_run(true)));
            ok &= check(write_pointcloud_glb_file(path, points, &error, GlbContent{"pointcloud", "points"}, &heights), "linear point cloud is written");
            clear_glb_run_metadata();
            tinygltf::Model linear;
            ok &= check(loader.LoadBinaryFromFile(&linear, &load_error, &warning, path.string()), "linear point cloud is read back");
            const tinygltf::Primitive &linear_primitive = linear.meshes.at(0).primitives.at(0);
            ok &= check(linear_primitive.attributes.count("COLOR_0") == 1, "the linear points have COLOR_0");
            if (linear_primitive.attributes.count("COLOR_0") == 1) {
                const tinygltf::Accessor &accessor = linear.accessors.at(linear_primitive.attributes.at("COLOR_0"));
                const tinygltf::BufferView &view = linear.bufferViews.at(accessor.bufferView);
                const float *rgb = reinterpret_cast<const float *>(linear.buffers.at(view.buffer).data.data() + view.byteOffset + accessor.byteOffset);
                ok &= check(std::fabs(rgb[99 * 3] - 1.0f) < 1e-3 && std::fabs(rgb[99 * 3 + 1] - (161.0 + 0.9111 * 25.0) / 255.0) < 2e-3 &&
                                std::fabs(rgb[99 * 3 + 2] - (68.0 + 0.9111 * 65.0) / 255.0) < 2e-3,
                            "the linear map colors the highest point between the ninth and tenth Haxby colors");
            }
            const std::string description = linear.asset.extras.Get("MB-System").Get("content").Get("description").Get<std::string>();
            ok &= check(description.find("-101.25") != std::string::npos && description.find("equally spaced") != std::string::npos,
                        "the linear map lists its bounds in the description");
        }
        ok &= check(write_pointcloud_glb_file(path, points, &error, GlbContent{"pointcloud", "points"}, &heights, PointColorMap::None) &&
                        loader.LoadBinaryFromFile(&plain, &load_error, &warning, path.string()) &&
                        plain.meshes.at(0).primitives.at(0).attributes.count("COLOR_0") == 0,
                    "the colormap none gives no COLOR_0");
        PointColorMap parsed = PointColorMap::None;
        ok &= check(parse_point_color_map("equalized", &parsed) && parsed == PointColorMap::HaxbyEqualized && parse_point_color_map("linear", &parsed) &&
                        parsed == PointColorMap::HaxbyLinear && parse_point_color_map("none", &parsed) && parsed == PointColorMap::None &&
                        !parse_point_color_map("rainbow", &parsed),
                    "the colormap names parse");

        ok &= check(write_pointcloud_glb_file(path, points, &error, GlbContent{"pointcloud", "points"}), "plain point cloud is written");
        ok &= check(loader.LoadBinaryFromFile(&plain, &load_error, &warning, path.string()) &&
                        plain.meshes.at(0).primitives.at(0).attributes.count("COLOR_0") == 0,
                    "without heights the points have no COLOR_0");
    }

    // ---- GLB extras equal the metadata, and carry the content of the file ----
    for (const bool with_center : {true, false}) {
        const std::string label = with_center ? "with GeoOrigin: " : "without GeoOrigin: ";
        const RunInfo run = example_run(with_center);
        const MetaValue metadata = run_metadata(run);
        set_glb_run_metadata(metadata);

        tinygltf::Model model;
        GlbContent content;
        content.role = "chunk";
        content.description = "one chunk";
        content.has_index = true;
        content.index_i = 3;
        content.index_j = -2;
        content.chunk_label.present = true;
        content.chunk_label.corners[1][0] = -122.2999;
        content.chunk_label.corners[2][1] = 36.4001;
        content.chunk_label.center[2] = -900.5;
        content.chunk_label.rise = 1.5;
        content.chunk_label.text_size = 0.3;
        content.primitives = 1;   // the writer says how many primitives the mesh is in; this one is one
        ok &= check(write_and_read(directory / "a.glb", content, &model), label + "round trip");

        const tinygltf::Value &extras = model.asset.extras;
        ok &= check(extras.IsObject(), label + "asset.extras is an object");

        std::map<std::string, std::string> found;
        flatten_value(extras, "", &found);
        std::map<std::string, std::string> expected;
        for (const auto &entry : flatten_metadata(metadata)) {
            expected[entry.first] = entry.second;
        }
        for (const auto &entry : flatten_metadata(content_metadata(content), "MB-System.content")) {
            expected[entry.first] = entry.second;
        }
        ok &= check(found == expected, label + "GLB extras equal the run metadata plus the content");
        if (found != expected) {
            for (const auto &entry : expected) {
                if (!found.count(entry.first) || found[entry.first] != entry.second) {
                    std::fprintf(stderr, "  differs: %s expected '%s' found '%s'\n", entry.first.c_str(), entry.second.c_str(),
                                 found.count(entry.first) ? found[entry.first].c_str() : "(missing)");
                }
            }
        }

        // The provenance block links its names to W3C PROV-O with JSON-LD.
        {
            const tinygltf::Value &provenance = extras.Get("provenance");
            const tinygltf::Value &context = provenance.Get("@context");
            ok &= check(context.Get("prov").Get<std::string>() == "http://www.w3.org/ns/prov#", label + "PROV-O namespace in the context");
            ok &= check(context.Get("startedAtTime").Get("@id").Get<std::string>() == "prov:startedAtTime", label + "startedAtTime is prov:startedAtTime");
            ok &= check(context.Get("startedAtTime").Get("@type").Get<std::string>() == "xsd:dateTime", label + "startedAtTime is a dateTime");
            ok &= check(context.Get("software").Get("@id").Get<std::string>() == "prov:wasAssociatedWith", label + "software is prov:wasAssociatedWith");
            ok &= check(context.Get("inputs").Get("@id").Get<std::string>() == "prov:used", label + "inputs is prov:used");
            ok &= check(provenance.Get("@type").Get<std::string>() == "prov:Activity", label + "the run is a prov:Activity");
            ok &= check(provenance.Get("software").Get("@type").Get<std::string>() == "prov:SoftwareAgent", label + "the program is a prov:SoftwareAgent");
            ok &= check(provenance.Get("inputs").Get("@type").Get<std::string>() == "prov:Entity", label + "the inputs are prov:Entity");
            // Every name the context maps is a name the block has.
            for (const std::string &name : {"startedAtTime", "software", "inputs"}) {
                ok &= check(provenance.Has(name), label + "context names '" + name + "' which the block has");
            }
        }

        ok &= check(found["georeference.crs"] == "EPSG:4978", label + "crs");
        ok &= check(found["MB-System.content.role"] == "chunk", label + "content role");
        ok &= check(found["MB-System.content.chunk[0]"] == "3" && found["MB-System.content.chunk[1]"] == "-2", label + "chunk indices");
        ok &= check(found["MB-System.content.chunkLabel.corners[1][0]"].rfind("-122.2999", 0) == 0 && found["MB-System.content.chunkLabel.corners[2][1]"].rfind("36.4001", 0) == 0 &&
                        found["MB-System.content.chunkLabel.center[2]"].rfind("-900.5", 0) == 0 && found["MB-System.content.chunkLabel.rise"].rfind("1.5", 0) == 0 &&
                        found["MB-System.content.chunkLabel.textSize"].rfind("0.3", 0) == 0,
                    label + "the place of the chunk for the survey page");
        ok &= check(found.count("MB-System.verticalReference") == 1, label + "vertical reference stated");
        ok &= check(found["provenance.commandLine"] == run.command_line, label + "command line survives exactly");

        if (with_center) {
            ok &= check(found["georeference.vertexCoordinates"] == "relativeToCenter", "relative vertices");
            ok &= check(std::fabs(std::stod(found["georeference.center[0]"]) - run.center_x) < 1e-6, "center x");
            ok &= check(std::fabs(std::stod(found["georeference.center[1]"]) - run.center_y) < 1e-6, "center y");
            ok &= check(std::fabs(std::stod(found["georeference.center[2]"]) - run.center_z) < 1e-6, "center z");
            ok &= check(std::fabs(std::stod(found["georeference.geodeticCenter.longitude"]) - run.center_longitude) < 1e-9, "center longitude");
            ok &= check(found["georeference.geodeticCenter.crs"] == "EPSG:4979", "geodetic center crs");
            ok &= check(found["provenance.inputs.fileCount"] == "2", "file count");
            ok &= check(found["provenance.inputs.files[1]"] == "/data/run/b.mb88", "second file listed");
        } else {
            ok &= check(found["georeference.vertexCoordinates"] == "absolute", "absolute vertices");
            ok &= check(found.count("georeference.center[0]") == 0, "no center without GeoOrigin");
            ok &= check(found.count("georeference.geodeticCenter.longitude") == 0, "no geodetic center without GeoOrigin");
        }

        // ---- the page says the same ----
        X3DomWriterOptions options;
        options.title = run.title;
        options.models.push_back(page_model());
        options.metadata = metadata;
        options.geo_summary = geo_metadata_summary(run);
        const std::filesystem::path page_path = directory / "page.html";
        std::string error;
        ok &= check(write_glb_x3dom_file(page_path, options, &error), label + "page written: " + error);
        const std::string page = read_file(page_path);

        ok &= check(!contains(page, "@context") && !contains(page, "@type"), label + "JSON-LD keywords are not in the page");
        ok &= check(contains(page, "<WorldInfo"), label + "WorldInfo present");
        ok &= check(contains(page, "<GeoMetadata"), label + "GeoMetadata present");
        ok &= check(contains(page, "<MetadataSet name='extras' containerField='metadata'"), label + "MetadataSet tree present");
        ok &= check(contains(page, "GEOREFERENCE.md"), label + "MetadataSet cites the vocabulary document");
        ok &= check(contains(page, "name='georeference'") && contains(page, "name='provenance'") && contains(page, "name='MB-System'"),
                    label + "the three blocks are in the page");
        ok &= check(contains(page, "name='crs' value='&quot;EPSG:4978&quot;'"), label + "crs in the page");
        ok &= check(with_center == contains(page, "name='center' value='" + metadata_number_text(run.center_x)),
                    label + "center in the page exactly when there is a GeoOrigin");
        // Quotes and angle brackets in the command line must not break the attribute.
        ok &= check(contains(page, "&lt;name&gt;") && !contains(page, "<name>"), label + "command line is escaped in the page");

        // Every entry of the metadata is a WorldInfo line.
        for (const auto &entry : flatten_metadata(metadata)) {
            if (entry.first.rfind("provenance.inputs.files", 0) == 0 || entry.second.find_first_of("\"'<>&") != std::string::npos) {
                continue;
            }
            ok &= check(contains(page, entry.first + ": " + entry.second), label + "WorldInfo has " + entry.first);
        }

        // The GeoMetadata summary: keyword and value pairs, only keywords X3D defines.
        ok &= check(!options.geo_summary.empty() && options.geo_summary.size() % 2 == 0, label + "summary has keyword/value pairs");
        const std::set<std::string> allowed = {"title", "description", "coordinateSystem", "horizontalDatum", "ellipsoid", "verticalDatum",
                                               "resolution", "originator", "date", "dataUrl", "dataFormat"};
        for (std::size_t i = 0; i + 1 < options.geo_summary.size(); i += 2) {
            ok &= check(allowed.count(options.geo_summary[i]) == 1, label + "summary keyword '" + options.geo_summary[i] + "' is defined by X3D");
            ok &= check(!options.geo_summary[i + 1].empty(), label + "summary value for '" + options.geo_summary[i] + "' is not empty");
        }
        ok &= check(contains(page, "coordinateSystem&quot; &quot;GC&quot;"), label + "geocentric coordinate system in the summary");
    }

    // ---- a file whose georeference differs, such as a tileset's content, has its own metadata ----
    {
        RunInfo ecef_run = example_run(true);
        set_glb_run_metadata(run_metadata(ecef_run));

        RunInfo frame_run = example_run(true);
        frame_run.local_frame = true;
        frame_run.geoorigin_mode = "none";
        const MetaValue frame_metadata = run_metadata(frame_run);

        GlbContent content;
        content.role = "tileset_tile";
        content.description = "one chunk";
        content.metadata = &frame_metadata;
        tinygltf::Model model;
        ok &= check(write_and_read(directory / "c.glb", content, &model), "round trip with its own metadata");
        std::map<std::string, std::string> found;
        flatten_value(model.asset.extras, "", &found);
        ok &= check(found["georeference.vertexCoordinates"] == "localFrame", "the file's own georeference is written, not the run's");
        ok &= check(found.count("georeference.center[0]") == 1 && found["MB-System.content.role"] == "tileset_tile", "with its center and its content");
        ok &= check(found["MB-System.vertexConvention"].find("Y is up") != std::string::npos && found["MB-System.geoOrigin.mode"] == "none",
                    "and a convention that says how the vertices are placed");

        // The run's own metadata is not changed by it.
        tinygltf::Model other;
        ok &= check(write_and_read(directory / "d.glb", GlbContent(), &other), "round trip without");
        std::map<std::string, std::string> unchanged;
        flatten_value(other.asset.extras, "", &unchanged);
        ok &= check(unchanged["georeference.vertexCoordinates"] == "relativeToCenter", "a file without its own metadata still has the run's");
    }

    // ---- no run metadata: no extras, and a page without the nodes ----
    {
        clear_glb_run_metadata();
        tinygltf::Model model;
        ok &= check(write_and_read(directory / "b.glb", GlbContent(), &model), "plain round trip");
        ok &= check(!model.asset.extras.IsObject() || model.asset.extras.Keys().empty(), "no run metadata means no extras");

        X3DomWriterOptions options;
        options.title = "none";
        options.models.push_back(page_model());
        const std::filesystem::path page_path = directory / "plain.html";
        std::string error;
        ok &= check(write_glb_x3dom_file(page_path, options, &error), "plain page written");
        ok &= check(!contains(read_file(page_path), "<GeoMetadata"), "no GeoMetadata without metadata");
    }

    std::filesystem::remove_all(directory);
    if (ok) {
        std::printf("OK: provenance\n");
    }
    return ok ? 0 : 1;
}
