#include "io/provenance.h"

#include <cmath>
#include <cstdio>
#include <ctime>

MetaValue MetaValue::from_string(const std::string &value) {
    MetaValue result;
    result.type = Type::String;
    result.text = value;
    return result;
}

MetaValue MetaValue::from_number(double value) {
    MetaValue result;
    result.type = Type::Number;
    result.number = value;
    return result;
}

MetaValue MetaValue::from_bool(bool value) {
    MetaValue result;
    result.type = Type::Bool;
    result.flag = value;
    return result;
}

MetaValue MetaValue::make_array() {
    MetaValue result;
    result.type = Type::Array;
    return result;
}

MetaValue MetaValue::make_object() {
    MetaValue result;
    result.type = Type::Object;
    return result;
}

MetaValue &MetaValue::set(const std::string &key, MetaValue value) {
    for (auto &member : members) {
        if (member.first == key) {
            member.second = std::move(value);
            return *this;
        }
    }
    members.emplace_back(key, std::move(value));
    return *this;
}

MetaValue &MetaValue::add(MetaValue value) {
    items.push_back(std::move(value));
    return *this;
}

const MetaValue *MetaValue::find(const std::string &key) const {
    for (const auto &member : members) {
        if (member.first == key) {
            return &member.second;
        }
    }
    return nullptr;
}

std::string metadata_number_text(double value) {
    char buffer[64];
    if (std::isfinite(value) && value == std::floor(value) && std::fabs(value) < 1.0e15) {
        std::snprintf(buffer, sizeof(buffer), "%.0f", value);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.12g", value);
    }
    return buffer;
}

std::vector<std::pair<std::string, std::string>> flatten_metadata(const MetaValue &value, const std::string &path) {
    std::vector<std::pair<std::string, std::string>> result;
    switch (value.type) {
    case MetaValue::Type::String:
        result.emplace_back(path, value.text);
        break;
    case MetaValue::Type::Number:
        result.emplace_back(path, metadata_number_text(value.number));
        break;
    case MetaValue::Type::Bool:
        result.emplace_back(path, value.flag ? "true" : "false");
        break;
    case MetaValue::Type::Array:
        for (std::size_t i = 0; i < value.items.size(); i++) {
            const auto inner = flatten_metadata(value.items[i], path + "[" + std::to_string(i) + "]");
            result.insert(result.end(), inner.begin(), inner.end());
        }
        break;
    case MetaValue::Type::Object:
        for (const auto &member : value.members) {
            if (!member.first.empty() && member.first[0] == '@') {
                continue;   // JSON-LD keywords link names to a vocabulary; they are not values
            }
            const auto inner = flatten_metadata(member.second, path.empty() ? member.first : path + "." + member.first);
            result.insert(result.end(), inner.begin(), inner.end());
        }
        break;
    }
    return result;
}

std::string iso_utc_time(double unix_seconds) {
    const std::time_t seconds = static_cast<std::time_t>(std::floor(unix_seconds));
    std::tm utc;
#ifdef _WIN32
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buffer;
}

namespace {

constexpr const char *vertical_reference =
    "A sounding's height is the MBIO depth below the sea surface, negated and used as a height above the "
    "WGS 84 ellipsoid. No geoid correction is applied.";

constexpr const char *vertex_convention_relative =
    "Vertices are WGS 84 ECEF positions in meters minus the center (the GeoOrigin ECEF position). "
    "The axes are those of ECEF, not a local east-north-up frame.";

constexpr const char *vertex_convention_absolute = "Vertices are absolute WGS 84 ECEF positions in meters.";

constexpr const char *vertex_convention_local_frame =
    "Vertices are in meters in the east-north-up frame at the center, written Y-up as glTF requires: X is east, "
    "Y is up and Z is south. absolute = center + east * x + north * -z + up * y, in WGS 84 ECEF. The transform of "
    "the tileset places them.";

MetaValue georeference_block(const RunInfo &run) {
    MetaValue block = MetaValue::make_object();
    block.set("schemaVersion", MetaValue::from_number(1));
    block.set("crs", MetaValue::from_string("EPSG:4978"));
    block.set("vertexCoordinates", MetaValue::from_string(run.local_frame ? "localFrame" : (run.has_center ? "relativeToCenter" : "absolute")));
    if (run.has_center) {
        MetaValue center = MetaValue::make_array();
        center.add(MetaValue::from_number(run.center_x));
        center.add(MetaValue::from_number(run.center_y));
        center.add(MetaValue::from_number(run.center_z));
        block.set("center", center);

        MetaValue geodetic = MetaValue::make_object();
        geodetic.set("crs", MetaValue::from_string("EPSG:4979"));
        geodetic.set("longitude", MetaValue::from_number(run.center_longitude));
        geodetic.set("latitude", MetaValue::from_number(run.center_latitude));
        geodetic.set("height", MetaValue::from_number(run.center_height));
        block.set("geodeticCenter", geodetic);
    }
    block.set("heightReference", MetaValue::from_string("ellipsoid"));

    MetaValue extent = MetaValue::make_object();
    extent.set("west", MetaValue::from_number(run.extent_west));
    extent.set("south", MetaValue::from_number(run.extent_south));
    extent.set("east", MetaValue::from_number(run.extent_east));
    extent.set("north", MetaValue::from_number(run.extent_north));
    block.set("extent", extent);
    return block;
}

// The JSON-LD context that links the names of the provenance block to W3C PROV-O
// (http://www.w3.org/ns/prov#), so that JSON-LD and RDF software can read the block as PROV
// statements. Software that does not know JSON-LD sees ordinary members.
MetaValue provenance_context() {
    MetaValue context = MetaValue::make_object();
    context.set("prov", MetaValue::from_string("http://www.w3.org/ns/prov#"));
    context.set("xsd", MetaValue::from_string("http://www.w3.org/2001/XMLSchema#"));

    MetaValue started = MetaValue::make_object();
    started.set("@id", MetaValue::from_string("prov:startedAtTime"));
    started.set("@type", MetaValue::from_string("xsd:dateTime"));
    context.set("startedAtTime", started);

    MetaValue software = MetaValue::make_object();
    software.set("@id", MetaValue::from_string("prov:wasAssociatedWith"));
    context.set("software", software);

    MetaValue inputs = MetaValue::make_object();
    inputs.set("@id", MetaValue::from_string("prov:used"));
    context.set("inputs", inputs);
    return context;
}

MetaValue provenance_block(const RunInfo &run) {
    MetaValue block = MetaValue::make_object();
    block.set("@context", provenance_context());
    block.set("@type", MetaValue::from_string("prov:Activity"));
    block.set("schemaVersion", MetaValue::from_number(1));

    MetaValue software = MetaValue::make_object();
    software.set("@type", MetaValue::from_string("prov:SoftwareAgent"));
    software.set("name", MetaValue::from_string(run.program));
    software.set("version", MetaValue::from_string(run.version));
    block.set("software", software);

    block.set("startedAtTime", MetaValue::from_string(run.started_at));
    block.set("commandLine", MetaValue::from_string(run.command_line));
    block.set("workingDirectory", MetaValue::from_string(run.working_directory));
    block.set("user", MetaValue::from_string(run.user));
    block.set("host", MetaValue::from_string(run.host));

    MetaValue inputs = MetaValue::make_object();
    inputs.set("@type", MetaValue::from_string("prov:Entity"));
    inputs.set("datalist", MetaValue::from_string(run.datalist));
    MetaValue formats = MetaValue::make_array();
    for (const int format : run.formats) {
        formats.add(MetaValue::from_number(format));
    }
    inputs.set("formats", formats);
    inputs.set("fileCount", MetaValue::from_number(static_cast<double>(run.file_count)));
    MetaValue files = MetaValue::make_array();
    for (const std::string &file : run.files) {
        files.add(MetaValue::from_string(file));
    }
    inputs.set("files", files);
    if (!run.first_time.empty()) {
        inputs.set("firstTime", MetaValue::from_string(run.first_time));
        inputs.set("lastTime", MetaValue::from_string(run.last_time));
    }
    block.set("inputs", inputs);
    return block;
}

MetaValue program_block(const RunInfo &run) {
    MetaValue block = MetaValue::make_object();
    block.set("schemaVersion", MetaValue::from_number(1));
    block.set("verticalReference", MetaValue::from_string(vertical_reference));
    block.set("vertexConvention", MetaValue::from_string(run.local_frame ? vertex_convention_local_frame
                                                         : (run.has_center ? vertex_convention_relative : vertex_convention_absolute)));

    MetaValue geoorigin = MetaValue::make_object();
    geoorigin.set("mode", MetaValue::from_string(run.geoorigin_mode));
    block.set("geoOrigin", geoorigin);

    MetaValue soundings = MetaValue::make_object();
    soundings.set("pings", MetaValue::from_number(static_cast<double>(run.pings)));
    soundings.set("read", MetaValue::from_number(static_cast<double>(run.soundings_read)));
    soundings.set("accepted", MetaValue::from_number(static_cast<double>(run.soundings_accepted)));
    soundings.set("flagged", MetaValue::from_number(static_cast<double>(run.soundings_flagged)));
    soundings.set("outOfBounds", MetaValue::from_number(static_cast<double>(run.soundings_out_of_bounds)));
    block.set("soundings", soundings);

    MetaValue area = MetaValue::make_object();
    area.set("applied", MetaValue::from_bool(run.bounds_applied));
    if (run.bounds_applied) {
        area.set("west", MetaValue::from_number(run.bounds_west));
        area.set("south", MetaValue::from_number(run.bounds_south));
        area.set("east", MetaValue::from_number(run.bounds_east));
        area.set("north", MetaValue::from_number(run.bounds_north));
    }
    block.set("area", area);

    MetaValue reconstruction = MetaValue::make_object();
    reconstruction.set("levelOfDetail", MetaValue::from_number(run.level_of_detail));
    reconstruction.set("poissonCellSize", MetaValue::from_number(run.poisson_cell_size));
    if (!run.poisson_solver.empty()) {
        reconstruction.set("poissonSolver", MetaValue::from_string(run.poisson_solver));
        reconstruction.set("poissonIterations",
                           run.poisson_iterations > 0
                               ? MetaValue::from_number(run.poisson_iterations)
                               : MetaValue::from_string("chosen from each grid: one per cell across its longest edge, at least 50"));
    }
    reconstruction.set("normalSearchRadius", MetaValue::from_number(run.normal_search_radius));
    reconstruction.set("pointSpacing", MetaValue::from_number(run.point_spacing));
    reconstruction.set("mode", MetaValue::from_string(run.chunked ? "chunked" : "singlePass"));
    if (run.chunked) {
        reconstruction.set("chunkSize", MetaValue::from_number(run.chunk_size));
        reconstruction.set("halo", MetaValue::from_number(run.halo));
        reconstruction.set("seamBlend", MetaValue::from_number(run.seam_blend));
        reconstruction.set("threads", MetaValue::from_number(static_cast<double>(run.threads)));
    }
    block.set("reconstruction", reconstruction);
    return block;
}

} // namespace

MetaValue run_metadata(const RunInfo &run) {
    MetaValue extras = MetaValue::make_object();
    extras.set("georeference", georeference_block(run));
    extras.set("provenance", provenance_block(run));
    extras.set("MB-System", program_block(run));
    return extras;
}

std::vector<std::string> geo_metadata_summary(const RunInfo &run) {
    std::vector<std::string> summary;
    const auto add = [&summary](const std::string &keyword, const std::string &value) {
        summary.push_back(keyword);
        summary.push_back(value);
    };

    if (!run.title.empty()) {
        add("title", run.title);
    }
    add("description",
        "Seafloor from swath bathymetry, made by MB-System " + run.program + " " + run.version +
            (run.local_frame ? "; vertices are meters in the east-north-up frame at the center, written Y-up, as the content of a 3D Tiles tileset"
                             : (run.has_center ? "; vertices are WGS 84 ECEF meters minus the GeoOrigin ECEF position"
                                               : "; vertices are absolute WGS 84 ECEF meters")));
    add("coordinateSystem", "GC");      // geocentric
    add("horizontalDatum", "W84");
    add("ellipsoid", "WE");
    add("verticalDatum", "WGS 84 ellipsoid; depth below the sea surface is used as height above the ellipsoid, with no geoid correction");
    add("resolution", metadata_number_text(run.poisson_cell_size));
    add("originator", "MB-System " + run.program + " " + run.version + ", run by " + run.user + " on " + run.host);
    if (!run.first_time.empty()) {
        add("date", run.first_time + " - " + run.last_time);
    }
    add("dataUrl", "file://" + run.datalist);
    std::string formats;
    for (const int format : run.formats) {
        formats += (formats.empty() ? "" : ", ") + std::to_string(format);
    }
    add("dataFormat", formats.empty() ? std::string("GLB triangle mesh of MB-System mbmesh or mbgrd2gltf") : "MB-System datalist of swath files in MBIO format " + formats);
    return summary;
}

MetaValue content_metadata(const GlbContent &content) {
    MetaValue block = MetaValue::make_object();
    block.set("role", MetaValue::from_string(content.role));
    block.set("description", MetaValue::from_string(content.description));
    if (content.has_index) {
        // The name in the file is the word of the thing: a tileset's files hold tiles, the others chunks.
        MetaValue index = MetaValue::make_array();
        index.add(MetaValue::from_number(content.index_i));
        index.add(MetaValue::from_number(content.index_j));
        block.set(content.role == "tileset_tile" ? "tile" : "chunk", index);
    }
    if (content.primitives > 0) {
        block.set("primitives", MetaValue::from_number(content.primitives));
    }
    if (content.compression.present) {
        MetaValue compression = MetaValue::make_object();
        compression.set("extension", MetaValue::from_string("KHR_draco_mesh_compression"));
        compression.set("positionBits", MetaValue::from_number(content.compression.position_bits));
        compression.set("positionStep", MetaValue::from_number(content.compression.position_step));
        if (content.compression.normal_bits > 0) {
            compression.set("normalBits", MetaValue::from_number(content.compression.normal_bits));
        }
        block.set("compression", compression);
    }
    if (content.chunk_label.present) {
        const auto position = [](const double *at) {
            MetaValue value = MetaValue::make_array();
            for (int k = 0; k < 3; k++) {
                value.add(MetaValue::from_number(at[k]));
            }
            return value;
        };
        MetaValue label = MetaValue::make_object();
        MetaValue corners = MetaValue::make_array();
        for (int corner = 0; corner < 4; corner++) {
            corners.add(position(content.chunk_label.corners[corner]));
        }
        label.set("corners", corners);
        label.set("center", position(content.chunk_label.center));
        label.set("rise", MetaValue::from_number(content.chunk_label.rise));
        label.set("textSize", MetaValue::from_number(content.chunk_label.text_size));
        block.set("chunkLabel", label);
    }
    return block;
}
