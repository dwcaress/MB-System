/*--------------------------------------------------------------------
 *    The MB-system:	geometry.cpp	5/11/2023
 *
 *    Copyright (c) 2023-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
  *    The program MBgrd2gltf, including this source file, was created
  *    by a Capstone Project team at the California State University
  *    Monterey Bay (CSUMB) including Kyle Dowling, Julian Fortin,
  *    Jesse Benavides, Nicolas Porras Falconio. This team was mentored by:
  *    Mike McCann
  *      Monterey Bay Aquarium Research Institute
  *      Moss Landing, California, USA
  *--------------------------------------------------------------------*/

#include "geometry.h"
#include "wgs84.h"
#include "logger.h"

// standard library
#include <cmath>
#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>

namespace mbgrd2gltf {

// Log an ECEF coordinate with millimeter resolution; the default stream
// precision shows only six digits, which hides the quantization this code avoids.
static std::string ecef_text(double value) {
  std::ostringstream text;
  text << std::fixed << std::setprecision(3) << value;
  return text.str();
}

// Mean of the valid (non-NaN) altitudes of the grid, in meters.
static double mean_altitude(const Bathymetry& bathymetry) {
  const auto& altitudes = bathymetry.altitudes();
  double altitude_sum = 0.0;
  size_t altitude_count = 0;
  for (size_t y = 0; y < altitudes.size_y(); ++y) {
    for (size_t x = 0; x < altitudes.size_x(); ++x) {
      float altitude = altitudes.at(x, y);
      if (!std::isnan(altitude)) {
        altitude_sum += altitude;
        altitude_count++;
      }
    }
  }
  return (altitude_count > 0) ? (altitude_sum / altitude_count) : 0.0;
}

Geometry::Geometry(const Bathymetry& bathymetry, const Options& options) {
  // Vertical exaggeration scales the relief about the mean altitude of the grid,
  // so the model stays where it is. Scaling the altitude itself would also move
  // the whole model by (exaggeration - 1) times its depth, away from the GeoOrigin
  // and from the viewpoint, which is set from the unexaggerated mean altitude.
  const double reference_altitude = mean_altitude(bathymetry);
  _reference_altitude = reference_altitude;

  // Apply GeoOrigin offset
  if (options.is_geoorigin_auto()) {
    // Automatic GeoOrigin: use geographic center of grid
    double geoorigin_lon = (bathymetry.longitude_min() + bathymetry.longitude_max()) / 2.0;
    double geoorigin_lat = (bathymetry.latitude_min() + bathymetry.latitude_max()) / 2.0;
    
    // The automatic GeoOrigin elevation is the mean altitude of the valid grid values
    double geoorigin_elev = reference_altitude;
    
    LOG_INFO("Using automatic GeoOrigin (grid center):", geoorigin_lon, ",", geoorigin_lat, ",", geoorigin_elev);
    
    // Compute GeoOrigin ECEF offset
    const EcefPosition geoorigin_ecef = get_ecef_position(geoorigin_lon, geoorigin_lat, geoorigin_elev);
    _geoorigin_x = geoorigin_ecef.x;
    _geoorigin_y = geoorigin_ecef.y;
    _geoorigin_z = geoorigin_ecef.z;
    
    LOG_INFO("GeoOrigin ECEF offset:", ecef_text(_geoorigin_x), ",", ecef_text(_geoorigin_y), ",", ecef_text(_geoorigin_z));
  } else if (options.is_geoorigin_set()) {
    double geoorigin_lon = options.geoorigin_lon();
    double geoorigin_lat = options.geoorigin_lat();
    double geoorigin_elev = options.geoorigin_elev();
    LOG_INFO("Using user-specified GeoOrigin:", geoorigin_lon, ",", geoorigin_lat, ",", geoorigin_elev);
    
    // Compute GeoOrigin ECEF offset
    const EcefPosition geoorigin_ecef = get_ecef_position(geoorigin_lon, geoorigin_lat, geoorigin_elev);
    _geoorigin_x = geoorigin_ecef.x;
    _geoorigin_y = geoorigin_ecef.y;
    _geoorigin_z = geoorigin_ecef.z;
    
    LOG_INFO("GeoOrigin ECEF offset:", ecef_text(_geoorigin_x), ",", ecef_text(_geoorigin_y), ",", ecef_text(_geoorigin_z));
  } else {
    // No GeoOrigin - use original ECEF coordinates (no offset)
    _geoorigin_x = 0.0;
    _geoorigin_y = 0.0;
    _geoorigin_z = 0.0;
    LOG_INFO("Using original ECEF coordinates (no GeoOrigin offset)");
  }
  
  // Generate vertices with GeoOrigin offset applied (or no offset if not set)
  _vertices = get_vertices(bathymetry, options.exaggeration(), reference_altitude,
                           _geoorigin_x, _geoorigin_y, _geoorigin_z, &_altitudes);
  
  size_t valid_vertices = 0;
  for (const auto& v : _vertices) {
    if (v.is_valid())
      valid_vertices++;
  }
  LOG_INFO("Created", Logger::format_with_commas(valid_vertices), "vertices");
}

double Geometry::to_radians(double degrees) { return degrees * (pi / 180.0); }

double Geometry::get_longitude(const Bathymetry& bathymetry, size_t x) {
  return bathymetry.longitude(x);
}

double Geometry::get_latitude(const Bathymetry& bathymetry, size_t y) {
  return bathymetry.latitude(y);
}

Geometry::EcefPosition Geometry::get_ecef_position(double longitude, double latitude, double altitude) {
  double ecef[3];
  wgs84_geodetic_to_ecef(longitude, latitude, altitude, ecef);
  return EcefPosition{ecef[0], ecef[1], ecef[2]};
}

Vertex Geometry::get_earth_centered_vertex(double longitude, double latitude, double altitude,
                                           uint32_t id) {
  const EcefPosition ecef = get_ecef_position(longitude, latitude, altitude);

  // gltf assumes y is up
  // With "return Vertex(x, z, y, id)" terrain ends up south of Australia
  //
  // This narrows to float. It is only right when no GeoOrigin is in use, where
  // the coordinates are absolute ECEF and a float is all glTF can hold anyway.
  return Vertex(ecef.x, ecef.y, ecef.z, id);
}

Matrix<Vertex> Geometry::get_vertices(const Bathymetry& bathymetry, double vertical_exaggeration,
                                       double reference_altitude,
                                       double geoorigin_x, double geoorigin_y, double geoorigin_z,
                                       std::vector<float>* vertex_altitudes) {
  Matrix<Vertex> out(bathymetry.size_x(), bathymetry.size_y());
  const auto& altitudes = bathymetry.altitudes();
  uint32_t vertex_id = 1;
  bool printed_first = false;
  size_t valid_count = 0;

  for (size_t y = 0; y < altitudes.size_y(); ++y) {
    for (size_t x = 0; x < altitudes.size_x(); ++x) {
      float altitude = altitudes.at(x, y);

      if (!std::isnan(altitude)) {
        double longitude = get_longitude(bathymetry, x);
        double latitude = get_latitude(bathymetry, y);
        double adjusted_altitude = (double)altitude;
        if (vertical_exaggeration != 1.0)
          adjusted_altitude = reference_altitude + (adjusted_altitude - reference_altitude) * vertical_exaggeration;

        // Keep the ECEF position in double and subtract the GeoOrigin in double.
        // Only the small remainder, within a few kilometers of the GeoOrigin, is
        // narrowed to float. Narrowing the absolute ECEF position first would
        // snap every vertex to a 0.25-0.5 m lattice, whatever the grid spacing.
        const EcefPosition ecef = get_ecef_position(longitude, latitude, adjusted_altitude);
        const uint32_t this_id = vertex_id++;
        vertex_altitudes->push_back(altitude);   // index() is the id less one, so this is where the vertex's value is
        
        // Print first few valid vertices for debugging
        if (!printed_first) {
          if (geoorigin_x != 0.0 || geoorigin_y != 0.0 || geoorigin_z != 0.0) {
            LOG_INFO("First vertex [", x, ",", y, "] before offset: x=", ecef_text(ecef.x), 
                     "y=", ecef_text(ecef.y), "z=", ecef_text(ecef.z), 
                     "lon=", longitude, "lat=", latitude, "alt=", altitude);
          }
        }
        
        // Apply GeoOrigin offset
        out.at(x, y) = Vertex(static_cast<float>(ecef.x - geoorigin_x), 
                              static_cast<float>(ecef.y - geoorigin_y), 
                              static_cast<float>(ecef.z - geoorigin_z), 
                              this_id);  // ids start at 1; 0 marks "no vertex" (see Vertex::is_valid)
        
        if (!printed_first) {
          if (geoorigin_x != 0.0 || geoorigin_y != 0.0 || geoorigin_z != 0.0) {
            LOG_INFO("First vertex [", x, ",", y, "] after offset:  x=", out.at(x, y).x(), 
                     "y=", out.at(x, y).y(), "z=", out.at(x, y).z());
          }
          printed_first = true;
        }
        
        valid_count++;
        // Print a sample from the middle
        if (valid_count == 1000) {
          if (geoorigin_x != 0.0 || geoorigin_y != 0.0 || geoorigin_z != 0.0) {
            LOG_INFO("Sample vertex #1000 [", x, ",", y, "] before offset: x=", ecef_text(ecef.x), 
                     "y=", ecef_text(ecef.y), "z=", ecef_text(ecef.z));
            LOG_INFO("Sample vertex #1000 [", x, ",", y, "] after offset:  x=", out.at(x, y).x(), 
                     "y=", out.at(x, y).y(), "z=", out.at(x, y).z());
          }
        }
      }
    }
  }

  return out;
}

//This function when called will grab the geometry in chunks instead of all in one
std::vector<Geometry::Block> Geometry::get_triangles_blocked(const Matrix<Vertex>& vertices,
                                                          size_t blockSize) {
  std::vector<Geometry::Block> blocks;

  // Number of cells. Triangles are generated over cells.
  const size_t cellsY = (vertices.size_y() > 0) ? vertices.size_y() - 1 : 0;
  const size_t cellsX = (vertices.size_x() > 0) ? vertices.size_x() - 1 : 0;
  if (cellsX == 0 || cellsY == 0)
    return blocks;

  for (size_t ty = 0; ty < cellsY; ty += blockSize) {
    for (size_t tx = 0; tx < cellsX; tx += blockSize) {
      const size_t endY = std::min(ty + blockSize, cellsY);
      const size_t endX = std::min(tx + blockSize, cellsX);

      Geometry::Block block;
      block.x0 = tx;
      block.y0 = ty;
      block.x1 = endX;
      block.y1 = endY;
      block.triangles.reserve(2ull * (endX - tx) * (endY - ty));

      for (size_t y = ty; y < endY; ++y) {
        for (size_t x = tx; x < endX; ++x) {
          const auto& bottom_left = vertices.at(x, y);
          const auto& bottom_right = vertices.at(x + 1, y);
          const auto& top_left = vertices.at(x, y + 1);
          const auto& top_right = vertices.at(x + 1, y + 1);

          if (bottom_left.is_valid() && top_right.is_valid()) {
            if (top_left.is_valid())
              block.triangles.emplace_back(
                  Triangle{bottom_left.index(), top_left.index(), top_right.index()});

            if (bottom_right.is_valid())
              block.triangles.emplace_back(
                  Triangle{bottom_left.index(), top_right.index(), bottom_right.index()});
          } else if (bottom_right.is_valid() && top_left.is_valid()) {
            if (bottom_left.is_valid())
              block.triangles.emplace_back(
                  Triangle{bottom_right.index(), bottom_left.index(), top_left.index()});

            if (top_right.is_valid())
              block.triangles.emplace_back(
                  Triangle{bottom_right.index(), top_left.index(), top_right.index()});
          }
        }
      }

      if (!block.triangles.empty())
        blocks.push_back(std::move(block));
    }
  }

  return blocks;
}
} // namespace mbgrd2gltf
