/*
 * Regression test for the vertex geometry of mbgrd2gltf.
 *
 * It writes a tiny GMT-format grid with 5 cm spacing near Monterey Bay and
 * checks Geometry against an independent double-precision ECEF calculation:
 *
 *   - GeoOrigin: vertices are accurate to millimeters. ECEF coordinates are
 *     millions of meters, where a float resolves only 0.25-0.5 m, so narrowing
 *     to float before subtracting the GeoOrigin snaps a 5 cm grid onto a
 *     coarse lattice. The GeoOrigin's own ECEF position must also be exact.
 *   - Vertex ids: every valid grid node gets a valid vertex, including the
 *     first, and the indices are 0..N-1 with no gaps or repeats.
 *   - Vertical exaggeration scales the relief about the mean altitude, so the
 *     model does not move, and the result does not depend on the GeoOrigin.
 *   - Grid node positions follow GMT: for a gridline-registered grid the N nodes
 *     span the region, so the spacing is range / (N - 1), and for a pixel-registered
 *     grid the nodes are at the cell centers. This is checked for the old GMT
 *     layout and the netCDF CF layout, with rows running either way.
 */

#include "bathymetry.h"
#include "geometry.h"
#include "options.h"

#include <netcdf.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <set>
#include <string>
#include <vector>

using namespace mbgrd2gltf;

namespace {

constexpr int nx = 24;
constexpr int ny = 20;
constexpr double spacing = 5.0e-7;          // degrees, about 5 cm
constexpr double lon_min = -122.330000;
constexpr double lat_min = 36.368000;

bool check(bool condition, const std::string& message) {
  if (!condition)
    std::fprintf(stderr, "FAIL: %s\n", message.c_str());
  return condition;
}

double node_altitude(int x, int y) {
  return -940.0 + 0.02 * x + 0.05 * y + 0.3 * std::sin(0.7 * x) * std::cos(0.5 * y);
}

bool node_is_missing(int x, int y) {
  return (x == 7 && y == 3) || (x == 8 && y == 3) || (x == nx - 1 && y == ny - 1);
}

// How a test grid is laid out in its file, and so where its nodes are.
enum class Layout { OldGmt, Cf };

struct GridSpec {
  const char* name;
  Layout layout;
  bool pixel;           // pixel registration: nodes at cell centers, region at cell edges
  bool south_to_north;  // row 0 is the southernmost row (the old GMT layout is always north to south)

  double cells_x() const { return pixel ? nx : nx - 1; }
  double cells_y() const { return pixel ? ny : ny - 1; }
  double longitude(int x) const { return lon_min + spacing * (x + (pixel ? 0.5 : 0.0)); }
  double latitude(int y) const {
    const double half = pixel ? 0.5 : 0.0;
    if (south_to_north)
      return lat_min + spacing * (half + y);
    return lat_min + spacing * cells_y() - spacing * (half + y);
  }
};

// WGS84 geodetic to ECEF, written out independently of the code under test.
struct Ecef {
  double x, y, z;
};

Ecef ecef(double lon_deg, double lat_deg, double altitude) {
  const double a = 6378137.0;
  const double f = 1.0 / 298.257223563;
  const double e2 = f * (2.0 - f);
  const double pi = 3.14159265358979323846;
  const double lon = lon_deg * pi / 180.0;
  const double lat = lat_deg * pi / 180.0;
  const double n = a / std::sqrt(1.0 - e2 * std::sin(lat) * std::sin(lat));
  return {(n + altitude) * std::cos(lat) * std::cos(lon), (n + altitude) * std::cos(lat) * std::sin(lon),
          (n * (1.0 - e2) + altitude) * std::sin(lat)};
}

std::vector<float> make_altitudes(double* z_min, double* z_max) {
  std::vector<float> altitudes(static_cast<size_t>(nx) * ny, 0.0f);
  *z_min = 1e30;
  *z_max = -1e30;
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      float value = node_is_missing(x, y) ? std::numeric_limits<float>::quiet_NaN()
                                          : static_cast<float>(node_altitude(x, y));
      altitudes[static_cast<size_t>(y) * nx + x] = value;
      if (!std::isnan(value)) {
        *z_min = std::min<double>(*z_min, value);
        *z_max = std::max<double>(*z_max, value);
      }
    }
  }
  return altitudes;
}

// The old GMT netCDF layout: side/xysize, one 1D z array, the region in x_range
// and y_range, the increments in spacing, and the registration on z.
bool write_old_gmt_grid(const std::string& path, const GridSpec& spec, const std::vector<float>& altitudes,
                        double z_min, double z_max) {
  int ncid = 0, side = 0, xysize = 0;
  int x_range = 0, y_range = 0, z_range = 0, spacing_var = 0, dimension = 0, z = 0;
  if (nc_create(path.c_str(), NC_CLOBBER, &ncid) != NC_NOERR)
    return false;
  nc_def_dim(ncid, "side", 2, &side);
  nc_def_dim(ncid, "xysize", altitudes.size(), &xysize);
  nc_def_var(ncid, "x_range", NC_DOUBLE, 1, &side, &x_range);
  nc_def_var(ncid, "y_range", NC_DOUBLE, 1, &side, &y_range);
  nc_def_var(ncid, "z_range", NC_DOUBLE, 1, &side, &z_range);
  nc_def_var(ncid, "spacing", NC_DOUBLE, 1, &side, &spacing_var);
  nc_def_var(ncid, "dimension", NC_INT, 1, &side, &dimension);
  nc_def_var(ncid, "z", NC_FLOAT, 1, &xysize, &z);
  const int node_offset = spec.pixel ? 1 : 0;
  nc_put_att_int(ncid, z, "node_offset", NC_INT, 1, &node_offset);
  nc_enddef(ncid);

  const double x_values[2] = {lon_min, lon_min + spacing * spec.cells_x()};
  const double y_values[2] = {lat_min, lat_min + spacing * spec.cells_y()};
  const double z_values[2] = {z_min, z_max};
  const double spacings[2] = {spacing, spacing};
  const int dimensions[2] = {nx, ny};
  nc_put_var_double(ncid, x_range, x_values);
  nc_put_var_double(ncid, y_range, y_values);
  nc_put_var_double(ncid, z_range, z_values);
  nc_put_var_double(ncid, spacing_var, spacings);
  nc_put_var_int(ncid, dimension, dimensions);
  nc_put_var_float(ncid, z, altitudes.data());
  return nc_close(ncid) == NC_NOERR;
}

// The netCDF CF layout GMT writes by default: x and y coordinate variables that
// hold the node positions, z(y, x), actual_range attributes holding the region,
// and a global node_offset of 1 for a pixel-registered grid. There is no spacing.
bool write_cf_grid(const std::string& path, const GridSpec& spec, const std::vector<float>& altitudes,
                   double z_min, double z_max) {
  int ncid = 0, x_dim = 0, y_dim = 0, x_var = 0, y_var = 0, z_var = 0;
  if (nc_create(path.c_str(), NC_CLOBBER, &ncid) != NC_NOERR)
    return false;
  nc_def_dim(ncid, "x", nx, &x_dim);
  nc_def_dim(ncid, "y", ny, &y_dim);
  nc_def_var(ncid, "x", NC_DOUBLE, 1, &x_dim, &x_var);
  nc_def_var(ncid, "y", NC_DOUBLE, 1, &y_dim, &y_var);
  const int z_dims[2] = {y_dim, x_dim};
  nc_def_var(ncid, "z", NC_FLOAT, 2, z_dims, &z_var);

  const double x_region[2] = {lon_min, lon_min + spacing * spec.cells_x()};
  const double y_region[2] = {lat_min, lat_min + spacing * spec.cells_y()};
  const double z_region[2] = {z_min, z_max};
  nc_put_att_double(ncid, x_var, "actual_range", NC_DOUBLE, 2, x_region);
  nc_put_att_double(ncid, y_var, "actual_range", NC_DOUBLE, 2, y_region);
  nc_put_att_double(ncid, z_var, "actual_range", NC_DOUBLE, 2, z_region);
  if (spec.pixel) {
    const int node_offset = 1;
    nc_put_att_int(ncid, NC_GLOBAL, "node_offset", NC_INT, 1, &node_offset);
  }
  nc_enddef(ncid);

  std::vector<double> x_nodes(nx), y_nodes(ny);
  for (int x = 0; x < nx; x++)
    x_nodes[x] = spec.longitude(x);
  for (int y = 0; y < ny; y++)
    y_nodes[y] = spec.latitude(y);
  nc_put_var_double(ncid, x_var, x_nodes.data());
  nc_put_var_double(ncid, y_var, y_nodes.data());
  nc_put_var_float(ncid, z_var, altitudes.data());
  return nc_close(ncid) == NC_NOERR;
}

bool write_grid(const std::string& path, const GridSpec& spec, std::vector<float>* altitudes) {
  double z_min = 0.0, z_max = 0.0;
  *altitudes = make_altitudes(&z_min, &z_max);
  return spec.layout == Layout::OldGmt ? write_old_gmt_grid(path, spec, *altitudes, z_min, z_max)
                                       : write_cf_grid(path, spec, *altitudes, z_min, z_max);
}

Options make_options(std::vector<std::string> arguments) {
  arguments.insert(arguments.begin(), "mbgrd2gltf");
  std::vector<const char*> argv;
  for (const std::string& argument : arguments)
    argv.push_back(argument.c_str());
  return Options(static_cast<unsigned>(argv.size()), argv.data());
}

double mean_of_valid(const std::vector<float>& altitudes) {
  double sum = 0.0;
  size_t count = 0;
  for (const float value : altitudes) {
    if (!std::isnan(value)) {
      sum += value;
      count++;
    }
  }
  return sum / static_cast<double>(count);
}

// Largest distance, in meters, between each vertex (plus the GeoOrigin offset)
// and the ECEF position computed independently from the grid node.
double worst_position_error(const Geometry& geometry, const GridSpec& spec, const std::vector<float>& altitudes,
                            double exaggeration, double reference) {
  double worst = 0.0;
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      if (node_is_missing(x, y))
        continue;
      const Vertex& vertex = geometry.vertices().at(x, y);
      double altitude = altitudes[static_cast<size_t>(y) * nx + x];
      altitude = reference + (altitude - reference) * exaggeration;
      const Ecef expected = ecef(spec.longitude(x), spec.latitude(y), altitude);
      const double dx = vertex.x() + geometry.geoorigin_x() - expected.x;
      const double dy = vertex.y() + geometry.geoorigin_y() - expected.y;
      const double dz = vertex.z() + geometry.geoorigin_z() - expected.z;
      worst = std::max(worst, std::sqrt(dx * dx + dy * dy + dz * dz));
    }
  }
  return worst;
}

} // namespace

int main() {
  bool ok = true;

  const std::string path = (std::filesystem::temp_directory_path() / "mbgrd2gltf_test_geometry.grd").string();
  std::vector<float> altitudes;
  const GridSpec primary{"old GMT layout, gridline", Layout::OldGmt, false, false};
  if (!check(write_grid(path, primary, &altitudes), "write the test grid"))
    return 1;

  const double origin_lon = -122.329995;
  const double origin_lat = 36.368005;
  const double origin_elevation = -940.5;
  const std::string origin_argument = "--geoorigin=-122.329995,36.368005,-940.5";
  const double mean = mean_of_valid(altitudes);

  try {
    // ---- GeoOrigin: positions and the origin itself must be accurate
    {
      const Options options = make_options({"--input", path, origin_argument});
      const Bathymetry bathymetry(options);
      const Geometry geometry(bathymetry, options);

      const Ecef origin = ecef(origin_lon, origin_lat, origin_elevation);
      const double origin_error = std::sqrt(std::pow(geometry.geoorigin_x() - origin.x, 2) +
                                            std::pow(geometry.geoorigin_y() - origin.y, 2) +
                                            std::pow(geometry.geoorigin_z() - origin.z, 2));
      ok &= check(origin_error < 1e-6, "GeoOrigin ECEF position is exact, got an error of " + std::to_string(origin_error) + " m");

      const double worst = worst_position_error(geometry, primary, altitudes, 1.0, mean);
      std::printf("GeoOrigin: worst vertex position error %.2e m\n", worst);
      ok &= check(worst < 1e-3, "vertices with a GeoOrigin are accurate to a millimeter, worst " + std::to_string(worst) + " m");

      // 5 cm spacing must not collapse onto a coarse lattice
      std::set<float> distinct_x;
      size_t valid = 0;
      for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
          if (!node_is_missing(x, y)) {
            distinct_x.insert(geometry.vertices().at(x, y).x());
            valid++;
          }
        }
      }
      ok &= check(distinct_x.size() > valid / 2, "vertex coordinates are not snapped to a lattice: " +
                                                      std::to_string(distinct_x.size()) + " distinct x values for " +
                                                      std::to_string(valid) + " vertices");

      // ---- vertex ids: every valid node has a valid vertex, indices are 0..N-1
      std::set<uint32_t> indices;
      bool validity_matches = true;
      for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
          const Vertex& vertex = geometry.vertices().at(x, y);
          validity_matches = validity_matches && (vertex.is_valid() == !node_is_missing(x, y));
          if (vertex.is_valid())
            indices.insert(vertex.index());
        }
      }
      ok &= check(validity_matches, "a vertex is valid exactly where the grid has a value, including the first node");
      ok &= check(geometry.vertices().at(0, 0).is_valid() && geometry.vertices().at(0, 0).index() == 0,
                  "the first valid vertex has index 0");
      ok &= check(indices.size() == valid && *indices.begin() == 0 && *indices.rbegin() == valid - 1,
                  "vertex indices are 0..N-1 with no gaps or repeats");
    }

    // ---- exaggeration scales the relief about the mean altitude
    {
      const Options options = make_options({"--input", path, origin_argument, "--exaggeration", "3"});
      const Bathymetry bathymetry(options);
      const Geometry geometry(bathymetry, options);
      const double worst = worst_position_error(geometry, primary, altitudes, 3.0, mean);
      std::printf("Exaggeration x3: worst vertex position error %.2e m\n", worst);
      ok &= check(worst < 1e-3, "exaggeration scales altitudes about their mean, worst error " + std::to_string(worst) + " m");

      // The colors go by the grid value of each vertex, in the order of Vertex::index(), whatever the exaggeration
      bool altitudes_match = true;
      size_t checked = 0;
      for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
          if (node_is_missing(x, y))
            continue;
          const Vertex& vertex = geometry.vertices().at(x, y);
          altitudes_match = altitudes_match && vertex.index() < geometry.altitudes().size() &&
                            geometry.altitudes()[vertex.index()] == altitudes[static_cast<size_t>(y) * nx + x];
          checked++;
        }
      }
      ok &= check(altitudes_match && checked == geometry.altitudes().size(),
                  "the true height of each vertex is kept, by index, under a vertical exaggeration of 3");
    }

    // ---- the model does not depend on which GeoOrigin is used, with or without exaggeration
    for (const char* exaggeration : {"1", "4"}) {
      const Options specified = make_options({"--input", path, origin_argument, "--exaggeration", exaggeration});
      const Options automatic = make_options({"--input", path, "--geoorigin", "--exaggeration", exaggeration});
      const Bathymetry bathymetry_specified(specified);
      const Bathymetry bathymetry_automatic(automatic);
      const Geometry geometry_specified(bathymetry_specified, specified);
      const Geometry geometry_automatic(bathymetry_automatic, automatic);

      double worst = 0.0;
      for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
          if (node_is_missing(x, y))
            continue;
          const Vertex& a = geometry_specified.vertices().at(x, y);
          const Vertex& b = geometry_automatic.vertices().at(x, y);
          const double dx = a.x() + geometry_specified.geoorigin_x() - (b.x() + geometry_automatic.geoorigin_x());
          const double dy = a.y() + geometry_specified.geoorigin_y() - (b.y() + geometry_automatic.geoorigin_y());
          const double dz = a.z() + geometry_specified.geoorigin_z() - (b.z() + geometry_automatic.geoorigin_z());
          worst = std::max(worst, std::sqrt(dx * dx + dy * dy + dz * dz));
        }
      }
      ok &= check(worst < 1e-3, std::string("geometry is independent of the GeoOrigin at exaggeration ") + exaggeration +
                                    ", worst difference " + std::to_string(worst) + " m");
    }
    // ---- node positions follow GMT, for every layout, registration and row order
    {
      const GridSpec specs[] = {
          {"old GMT layout, gridline", Layout::OldGmt, false, false},
          {"old GMT layout, pixel", Layout::OldGmt, true, false},
          {"CF layout, gridline, rows south to north", Layout::Cf, false, true},
          {"CF layout, gridline, rows north to south", Layout::Cf, false, false},
          {"CF layout, pixel, rows south to north", Layout::Cf, true, true},
          {"CF layout, pixel, rows north to south", Layout::Cf, true, false},
      };
      for (const GridSpec& spec : specs) {
        std::vector<float> spec_altitudes;
        if (!check(write_grid(path, spec, &spec_altitudes), std::string("write the grid: ") + spec.name))
          continue;
        const Options options = make_options({"--input", path, origin_argument});
        const Bathymetry bathymetry(options);
        const Geometry geometry(bathymetry, options);
        const double worst = worst_position_error(geometry, spec, spec_altitudes, 1.0, mean_of_valid(spec_altitudes));
        std::printf("%-44s worst node position error %.2e m\n", spec.name, worst);
        ok &= check(worst < 1e-3, std::string("node positions match GMT for ") + spec.name + ", worst error " +
                                      std::to_string(worst) + " m");
      }
    }
  } catch (const std::exception& error) {
    std::fprintf(stderr, "FAIL: unexpected exception: %s\n", error.what());
    ok = false;
  }

  std::filesystem::remove(path);
  std::printf(ok ? "mbgrd2gltf geometry tests passed\n" : "mbgrd2gltf geometry tests FAILED\n");
  return ok ? 0 : 1;
}
