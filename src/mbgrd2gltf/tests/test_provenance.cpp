/*
 * The georeference and provenance metadata of mbgrd2gltf: the blocks, the X3D
 * GeoMetadata summary and the WorldInfo and MetadataSet nodes of the page. The page
 * is written from the same tree as the glTF extras, so what is checked here is what
 * is in both.
 */

#include "provenance.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <sstream>
#include <string>

using namespace mbgrd2gltf;

namespace {

bool check(bool condition, const std::string& message) {
  if (!condition)
    std::fprintf(stderr, "FAIL: %s\n", message.c_str());
  return condition;
}

bool contains(const std::string& text, const std::string& part) {
  return text.find(part) != std::string::npos;
}

GridRunInfo example_run(bool with_center) {
  GridRunInfo run;
  run.title = "survey.grd";
  run.version = "5.8.3beta25";
  run.command_line = "mbgrd2gltf -I survey.grd -O out -B -D --geoorigin 'a \"quoted\" <name>'";
  run.working_directory = "/data/run";
  run.started_at = "2026-10-22T16:09:29Z";
  run.user = "mccann";
  run.host = "menard";
  run.grid_file = "/data/run/survey.grd";
  run.grid_columns = 2001;
  run.grid_rows = 1001;
  run.longitude_spacing = 5.0e-7;
  run.latitude_spacing = 5.0e-7;
  run.pixel_registered = true;
  run.extent_west = -122.33;
  run.extent_south = 36.368;
  run.extent_east = -122.329;
  run.extent_north = 36.3685;
  run.mean_altitude = -940.5;
  run.exaggeration = 2.0;
  run.binary = true;
  run.draco.present = true;
  run.draco.position_bits = 17;
  run.draco.position_step = 0.0009;
  if (with_center) {
    run.geoorigin_mode = "automatic";
    run.has_center = true;
    run.center_x = -2749436.658;
    run.center_y = -4344156.687;
    run.center_z = 3760606.005;
    run.center_longitude = -122.3295;
    run.center_latitude = 36.36825;
    run.center_height = -940.5;
  }
  return run;
}

std::string value_of(const MetaValue& tree, const std::string& path) {
  for (const auto& entry : flatten_metadata(tree)) {
    if (entry.first == path)
      return entry.second;
  }
  return "(missing)";
}

} // namespace

int main() {
  bool ok = true;

  for (const bool with_center : {true, false}) {
    const std::string label = with_center ? "with GeoOrigin: " : "without GeoOrigin: ";
    const GridRunInfo run = example_run(with_center);
    const MetaValue tree = grid_run_metadata(run);

    ok &= check(tree.find("georeference") && tree.find("provenance") && tree.find("MB-System"),
                label + "the three blocks");
    ok &= check(value_of(tree, "georeference.crs") == "EPSG:4978", label + "crs");
    ok &= check(value_of(tree, "georeference.heightReference") == "ellipsoid", label + "height reference");
    ok &= check(value_of(tree, "georeference.extent.west") == "-122.33", label + "extent west");
    ok &= check(value_of(tree, "georeference.extent.north") == "36.3685", label + "extent north");
    ok &= check(value_of(tree, "provenance.commandLine") == run.command_line, label + "command line exactly");
    ok &= check(value_of(tree, "provenance.startedAtTime") == "2026-10-22T16:09:29Z", label + "time of execution");
    ok &= check(value_of(tree, "provenance.software.name") == "mbgrd2gltf", label + "program name");
    ok &= check(value_of(tree, "provenance.inputs.gridFile") == "/data/run/survey.grd", label + "grid file");
    ok &= check(value_of(tree, "MB-System.grid.columns") == "2001", label + "grid columns");
    ok &= check(value_of(tree, "MB-System.grid.registration") == "pixel", label + "registration");
    ok &= check(value_of(tree, "MB-System.model.verticalExaggeration") == "2", label + "exaggeration");
    ok &= check(value_of(tree, "MB-System.content.compression.extension") == "KHR_draco_mesh_compression", label + "draco extension");
    ok &= check(value_of(tree, "MB-System.content.compression.positionBits") == "17", label + "draco position bits");
    ok &= check(value_of(tree, "MB-System.content.compression.normalBits") == "(missing)", label + "no normal bits for a mesh without normals");
    ok &= check(value_of(tree, "MB-System.verticalReference") != "(missing)", label + "vertical reference stated");

    if (with_center) {
      ok &= check(value_of(tree, "georeference.vertexCoordinates") == "relativeToCenter", "relative vertices");
      ok &= check(std::fabs(std::stod(value_of(tree, "georeference.center[0]")) - run.center_x) < 1e-6, "center x");
      ok &= check(std::fabs(std::stod(value_of(tree, "georeference.center[2]")) - run.center_z) < 1e-6, "center z");
      ok &= check(value_of(tree, "georeference.geodeticCenter.crs") == "EPSG:4979", "geodetic center crs");
      ok &= check(value_of(tree, "georeference.geodeticCenter.height") == "-940.5", "geodetic center height");
      ok &= check(value_of(tree, "MB-System.geoOrigin.mode") == "automatic", "mode");
    } else {
      ok &= check(value_of(tree, "georeference.vertexCoordinates") == "absolute", "absolute vertices");
      ok &= check(value_of(tree, "georeference.center[0]") == "(missing)", "no center without GeoOrigin");
      ok &= check(value_of(tree, "georeference.geodeticCenter.longitude") == "(missing)", "no geodetic center");
      ok &= check(value_of(tree, "MB-System.geoOrigin.mode") == "none", "mode none");
    }

    // The provenance block links its names to W3C PROV-O with JSON-LD.
    {
      const MetaValue* provenance = tree.find("provenance");
      const MetaValue* context = provenance ? provenance->find("@context") : nullptr;
      ok &= check(context != nullptr, label + "provenance has a JSON-LD context");
      if (context) {
        const auto mapped = [&](const std::string& name) {
          const MetaValue* entry = context->find(name);
          const MetaValue* id = entry ? entry->find("@id") : nullptr;
          return id ? id->text : std::string("(missing)");
        };
        ok &= check(context->find("prov") && context->find("prov")->text == "http://www.w3.org/ns/prov#", label + "PROV-O namespace");
        ok &= check(mapped("startedAtTime") == "prov:startedAtTime", label + "startedAtTime is prov:startedAtTime");
        ok &= check(mapped("software") == "prov:wasAssociatedWith", label + "software is prov:wasAssociatedWith");
        ok &= check(mapped("inputs") == "prov:used", label + "inputs is prov:used");
        for (const std::string name : {"startedAtTime", "software", "inputs"})
          ok &= check(provenance->find(name) != nullptr, label + "context names '" + name + "' which the block has");
        ok &= check(provenance->find("@type") && provenance->find("@type")->text == "prov:Activity", label + "the run is a prov:Activity");
        ok &= check(provenance->find("software")->find("@type")->text == "prov:SoftwareAgent", label + "the program is a prov:SoftwareAgent");
        ok &= check(provenance->find("inputs")->find("@type")->text == "prov:Entity", label + "the inputs are prov:Entity");
      }
      for (const auto& entry : flatten_metadata(tree))
        ok &= check(entry.first.find('@') == std::string::npos, label + "JSON-LD keywords are not flattened: " + entry.first);
    }

    // The GeoMetadata summary: keyword and value pairs, only keywords X3D defines.
    const std::vector<std::string> summary = geo_metadata_summary(run);
    ok &= check(!summary.empty() && summary.size() % 2 == 0, label + "summary has keyword/value pairs");
    const std::set<std::string> allowed = {"title", "description", "coordinateSystem", "horizontalDatum", "ellipsoid",
                                           "verticalDatum", "resolution", "originator", "date", "dataUrl", "dataFormat"};
    for (size_t i = 0; i + 1 < summary.size(); i += 2) {
      ok &= check(allowed.count(summary[i]) == 1, label + "summary keyword '" + summary[i] + "' is defined by X3D");
      ok &= check(!summary[i + 1].empty(), label + "summary value for '" + summary[i] + "' is not empty");
    }

    // The page's nodes.
    std::ostringstream out;
    write_x3d_metadata(out, run.title, tree, summary, "        ");
    const std::string page = out.str();
    ok &= check(!contains(page, "@context") && !contains(page, "@type"), label + "JSON-LD keywords are not in the page");
    ok &= check(contains(page, "<WorldInfo title='survey.grd'"), label + "WorldInfo");
    ok &= check(contains(page, "<GeoMetadata"), label + "GeoMetadata");
    ok &= check(contains(page, "<MetadataSet name='extras' containerField='metadata'"), label + "MetadataSet tree");
    ok &= check(contains(page, "GEOREFERENCE.md"), label + "cites the vocabulary document");
    ok &= check(contains(page, "name='georeference'") && contains(page, "name='provenance'") && contains(page, "name='MB-System'"),
                label + "the three blocks in the page");
    ok &= check(contains(page, "name='crs' value='&quot;EPSG:4978&quot;'"), label + "crs in the page");
    ok &= check(with_center == contains(page, "name='center' value='" + metadata_number_text(run.center_x)),
                label + "center in the page exactly when there is a GeoOrigin");
    ok &= check(contains(page, "&lt;name&gt;") && !contains(page, "<name>"), label + "command line is escaped in the page");
    ok &= check(contains(page, "coordinateSystem&quot; &quot;GC&quot;"), label + "geocentric in the summary");
    for (const auto& entry : flatten_metadata(tree)) {
      if (entry.second.find_first_of("\"'<>&") != std::string::npos)
        continue;
      ok &= check(contains(page, entry.first + ": " + entry.second), label + "WorldInfo has " + entry.first);
    }
  }

  // No metadata, no nodes.
  {
    std::ostringstream out;
    write_x3d_metadata(out, "none", MetaValue::make_object(), {}, "");
    ok &= check(out.str().empty(), "no nodes without metadata");
  }

  if (ok)
    std::printf("OK: provenance\n");
  return ok ? 0 : 1;
}
