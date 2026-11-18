/*--------------------------------------------------------------------
 *    The MB-system:	provenance.cpp	10/22/2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "provenance.h"

#include <cmath>
#include <cstdio>
#include <ctime>

namespace mbgrd2gltf {

MetaValue MetaValue::from_string(const std::string& value) {
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

MetaValue& MetaValue::set(const std::string& key, MetaValue value) {
  for (auto& member : members) {
    if (member.first == key) {
      member.second = std::move(value);
      return *this;
    }
  }
  members.emplace_back(key, std::move(value));
  return *this;
}

MetaValue& MetaValue::add(MetaValue value) {
  items.push_back(std::move(value));
  return *this;
}

const MetaValue* MetaValue::find(const std::string& key) const {
  for (const auto& member : members) {
    if (member.first == key)
      return &member.second;
  }
  return nullptr;
}

std::string metadata_number_text(double value) {
  char buffer[64];
  if (std::isfinite(value) && value == std::floor(value) && std::fabs(value) < 1.0e15)
    std::snprintf(buffer, sizeof(buffer), "%.0f", value);
  else
    std::snprintf(buffer, sizeof(buffer), "%.12g", value);
  return buffer;
}

std::vector<std::pair<std::string, std::string>> flatten_metadata(const MetaValue& value,
                                                                 const std::string& path) {
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
    for (size_t i = 0; i < value.items.size(); i++) {
      const auto inner = flatten_metadata(value.items[i], path + "[" + std::to_string(i) + "]");
      result.insert(result.end(), inner.begin(), inner.end());
    }
    break;
  case MetaValue::Type::Object:
    for (const auto& member : value.members) {
      if (!member.first.empty() && member.first[0] == '@')
        continue; // JSON-LD keywords link names to a vocabulary; they are not values
      const auto inner =
          flatten_metadata(member.second, path.empty() ? member.first : path + "." + member.first);
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

constexpr const char* vertical_reference =
    "A grid node's height is its grid value (MB-System grids hold depth as a negative elevation), "
    "used as a height above the WGS 84 ellipsoid. No geoid correction is applied.";

constexpr const char* vertex_convention_relative =
    "Vertices are WGS 84 ECEF positions in meters minus the center (the GeoOrigin ECEF position). "
    "The axes are those of ECEF, not a local east-north-up frame.";

constexpr const char* vertex_convention_absolute =
    "Vertices are absolute WGS 84 ECEF positions in meters.";

MetaValue number(double value) { return MetaValue::from_number(value); }
MetaValue text(const std::string& value) { return MetaValue::from_string(value); }

MetaValue georeference_block(const GridRunInfo& run) {
  MetaValue block = MetaValue::make_object();
  block.set("schemaVersion", number(1));
  block.set("crs", text("EPSG:4978"));
  block.set("vertexCoordinates", text(run.has_center ? "relativeToCenter" : "absolute"));
  if (run.has_center) {
    MetaValue center = MetaValue::make_array();
    center.add(number(run.center_x));
    center.add(number(run.center_y));
    center.add(number(run.center_z));
    block.set("center", center);

    MetaValue geodetic = MetaValue::make_object();
    geodetic.set("crs", text("EPSG:4979"));
    geodetic.set("longitude", number(run.center_longitude));
    geodetic.set("latitude", number(run.center_latitude));
    geodetic.set("height", number(run.center_height));
    block.set("geodeticCenter", geodetic);
  }
  block.set("heightReference", text("ellipsoid"));

  MetaValue extent = MetaValue::make_object();
  extent.set("west", number(run.extent_west));
  extent.set("south", number(run.extent_south));
  extent.set("east", number(run.extent_east));
  extent.set("north", number(run.extent_north));
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

MetaValue provenance_block(const GridRunInfo& run) {
  MetaValue block = MetaValue::make_object();
  block.set("@context", provenance_context());
  block.set("@type", text("prov:Activity"));
  block.set("schemaVersion", number(1));

  MetaValue software = MetaValue::make_object();
  software.set("@type", text("prov:SoftwareAgent"));
  software.set("name", text(run.program));
  software.set("version", text(run.version));
  block.set("software", software);

  block.set("startedAtTime", text(run.started_at));
  block.set("commandLine", text(run.command_line));
  block.set("workingDirectory", text(run.working_directory));
  block.set("user", text(run.user));
  block.set("host", text(run.host));

  MetaValue inputs = MetaValue::make_object();
  inputs.set("@type", text("prov:Entity"));
  inputs.set("gridFile", text(run.grid_file));
  inputs.set("format", text("GMT netCDF grid"));
  block.set("inputs", inputs);
  return block;
}

MetaValue program_block(const GridRunInfo& run) {
  MetaValue block = MetaValue::make_object();
  block.set("schemaVersion", number(1));
  block.set("verticalReference", text(vertical_reference));
  block.set("vertexConvention",
            text(run.has_center ? vertex_convention_relative : vertex_convention_absolute));

  MetaValue geoorigin = MetaValue::make_object();
  geoorigin.set("mode", text(run.geoorigin_mode));
  block.set("geoOrigin", geoorigin);

  MetaValue grid = MetaValue::make_object();
  grid.set("columns", number(static_cast<double>(run.grid_columns)));
  grid.set("rows", number(static_cast<double>(run.grid_rows)));
  grid.set("longitudeSpacing", number(run.longitude_spacing));
  grid.set("latitudeSpacing", number(run.latitude_spacing));
  grid.set("registration", text(run.pixel_registered ? "pixel" : "gridline"));
  block.set("grid", grid);

  MetaValue model = MetaValue::make_object();
  model.set("verticalExaggeration", number(run.exaggeration));
  model.set("meanAltitude", number(run.mean_altitude));
  model.set("binary", MetaValue::from_bool(run.binary));
  block.set("model", model);

  MetaValue content = MetaValue::make_object();
  content.set("role", text("mesh"));
  content.set("description", text("The gridded seafloor surface"));
  if (run.draco.present) {
    // As mbmesh writes it, under the same names (GEOREFERENCE.md).
    MetaValue compression = MetaValue::make_object();
    compression.set("extension", text("KHR_draco_mesh_compression"));
    compression.set("positionBits", number(run.draco.position_bits));
    compression.set("positionStep", number(run.draco.position_step));
    if (run.draco.normal_bits > 0)
      compression.set("normalBits", number(run.draco.normal_bits));
    content.set("compression", compression);
  }
  block.set("content", content);
  return block;
}

// An X3D MFString: each string in double quotes, with \ and " escaped.
std::string mf_string(const std::vector<std::string>& strings) {
  std::string result;
  for (const std::string& value : strings) {
    std::string quoted = "\"";
    for (const char character : value) {
      if (character == '"' || character == '\\')
        quoted += '\\';
      quoted += character;
    }
    quoted += '"';
    result += (result.empty() ? "" : " ") + quoted;
  }
  return result;
}

std::string escape_html(const std::string& value) {
  std::string escaped;
  for (const char character : value) {
    switch (character) {
    case '&': escaped += "&amp;"; break;
    case '<': escaped += "&lt;"; break;
    case '>': escaped += "&gt;"; break;
    case '"': escaped += "&quot;"; break;
    case '\'': escaped += "&#39;"; break;
    default: escaped += character;
    }
  }
  return escaped;
}

bool all_of_type(const MetaValue& array, MetaValue::Type type) {
  for (const MetaValue& item : array.items) {
    if (item.type != type)
      return false;
  }
  return !array.items.empty();
}

// A value as X3D metadata nodes: an object as a MetadataSet, a string as a
// MetadataString, a number as a MetadataDouble, a boolean as a MetadataBoolean,
// and an array of numbers or of strings as one node holding all of them.
void write_metadata_node(std::ostream& out, const MetaValue& value, const std::string& name,
                         int depth, const std::string& extra_attributes = "") {
  const std::string indent(static_cast<size_t>(depth) * 2, ' ');
  const std::string head = "name='" + escape_html(name) + "'" + extra_attributes;

  switch (value.type) {
  case MetaValue::Type::String:
    out << indent << "<MetadataString " << head << " value='"
        << escape_html(mf_string({value.text})) << "'></MetadataString>\n";
    break;
  case MetaValue::Type::Number:
    out << indent << "<MetadataDouble " << head << " value='" << metadata_number_text(value.number)
        << "'></MetadataDouble>\n";
    break;
  case MetaValue::Type::Bool:
    out << indent << "<MetadataBoolean " << head << " value='" << (value.flag ? "true" : "false")
        << "'></MetadataBoolean>\n";
    break;
  case MetaValue::Type::Array:
    if (all_of_type(value, MetaValue::Type::Number)) {
      std::string numbers;
      for (const MetaValue& item : value.items)
        numbers += (numbers.empty() ? "" : " ") + metadata_number_text(item.number);
      out << indent << "<MetadataDouble " << head << " value='" << numbers << "'></MetadataDouble>\n";
    } else if (all_of_type(value, MetaValue::Type::String)) {
      std::vector<std::string> strings;
      for (const MetaValue& item : value.items)
        strings.push_back(item.text);
      out << indent << "<MetadataString " << head << " value='" << escape_html(mf_string(strings))
          << "'></MetadataString>\n";
    } else {
      out << indent << "<MetadataSet " << head << ">\n";
      for (size_t i = 0; i < value.items.size(); i++)
        write_metadata_node(out, value.items[i], std::to_string(i), depth + 1);
      out << indent << "</MetadataSet>\n";
    }
    break;
  case MetaValue::Type::Object:
    out << indent << "<MetadataSet " << head << ">\n";
    for (const auto& member : value.members) {
      if (!member.first.empty() && member.first[0] == '@')
        continue; // JSON-LD keywords are for the glTF file; the page shows the values
      write_metadata_node(out, member.second, member.first, depth + 1);
    }
    out << indent << "</MetadataSet>\n";
    break;
  }
}

constexpr const char* metadata_reference =
    "https://github.com/dwcaress/MB-System/blob/master/src/mbmesh/GEOREFERENCE.md";

} // namespace

MetaValue grid_run_metadata(const GridRunInfo& run) {
  MetaValue extras = MetaValue::make_object();
  extras.set("georeference", georeference_block(run));
  extras.set("provenance", provenance_block(run));
  extras.set("MB-System", program_block(run));
  return extras;
}

std::vector<std::string> geo_metadata_summary(const GridRunInfo& run) {
  std::vector<std::string> summary;
  const auto add = [&summary](const std::string& keyword, const std::string& value) {
    summary.push_back(keyword);
    summary.push_back(value);
  };

  if (!run.title.empty())
    add("title", run.title);
  add("description",
      "Seafloor from a bathymetry grid, made by MB-System " + run.program + " " + run.version +
          (run.has_center ? "; vertices are WGS 84 ECEF meters minus the GeoOrigin ECEF position"
                          : "; vertices are absolute WGS 84 ECEF meters"));
  add("coordinateSystem", "GC"); // geocentric
  add("horizontalDatum", "W84");
  add("ellipsoid", "WE");
  add("verticalDatum",
      "WGS 84 ellipsoid; the grid value is used as height above the ellipsoid, with no geoid correction");
  // The grid spacing in meters, at about 111 km per degree of latitude.
  add("resolution", metadata_number_text(run.latitude_spacing * 111320.0));
  add("originator", "MB-System " + run.program + " " + run.version + ", run by " + run.user + " on " + run.host);
  add("dataUrl", "file://" + run.grid_file);
  add("dataFormat", "GMT netCDF grid");
  return summary;
}

void write_x3d_metadata(std::ostream& out, const std::string& title, const MetaValue& metadata,
                        const std::vector<std::string>& summary, const std::string& indent) {
  if (metadata.members.empty())
    return;

  std::vector<std::string> lines;
  for (const auto& entry : flatten_metadata(metadata))
    lines.push_back(entry.first + ": " + entry.second);

  out << indent << "<WorldInfo title='" << escape_html(title) << "' info='"
      << escape_html(mf_string(lines)) << "'></WorldInfo>\n"
      << indent << "<GeoMetadata DEF='GEO_METADATA' summary='" << escape_html(mf_string(summary))
      << "'>\n";
  // Written at depth 5 + indent so that nested nodes stay readable.
  write_metadata_node(out, metadata, "extras", 5,
                      " containerField='metadata' reference='" + std::string(metadata_reference) + "'");
  out << indent << "</GeoMetadata>\n";
}

} // namespace mbgrd2gltf
