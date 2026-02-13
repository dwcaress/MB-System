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
#include "logger.h"

// standard library
#include <cmath>
#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>

#define WGS_84_SEMI_MAJOR_AXIS 6378137.0
#define WGS_84_INVERSE_FLATTENING 298.257223563

namespace mbgrd2gltf {

Geometry::Geometry(const Bathymetry& bathymetry, const Options& options) {
  // Apply GeoOrigin offset
  if (options.is_geoorigin_auto()) {
    // Automatic GeoOrigin: use geographic center of grid
    double geoorigin_lon = (bathymetry.longitude_min() + bathymetry.longitude_max()) / 2.0;
    double geoorigin_lat = (bathymetry.latitude_min() + bathymetry.latitude_max()) / 2.0;
    
    // Compute mean altitude from valid altitude values
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
    double geoorigin_elev = (altitude_count > 0) ? (altitude_sum / altitude_count) : 0.0;
    
    LOG_INFO("Using automatic GeoOrigin (grid center):", geoorigin_lon, ",", geoorigin_lat, ",", geoorigin_elev);
    
    // Compute GeoOrigin ECEF offset
    Vertex geoorigin_vertex = get_earth_centered_vertex(geoorigin_lon, geoorigin_lat, geoorigin_elev, 0);
    _geoorigin_x = geoorigin_vertex.x();
    _geoorigin_y = geoorigin_vertex.y();
    _geoorigin_z = geoorigin_vertex.z();
    
    LOG_INFO("GeoOrigin ECEF offset:", _geoorigin_x, ",", _geoorigin_y, ",", _geoorigin_z);
  } else if (options.is_geoorigin_set()) {
    double geoorigin_lon = options.geoorigin_lon();
    double geoorigin_lat = options.geoorigin_lat();
    double geoorigin_elev = options.geoorigin_elev();
    LOG_INFO("Using user-specified GeoOrigin:", geoorigin_lon, ",", geoorigin_lat, ",", geoorigin_elev);
    
    // Compute GeoOrigin ECEF offset
    Vertex geoorigin_vertex = get_earth_centered_vertex(geoorigin_lon, geoorigin_lat, geoorigin_elev, 0);
    _geoorigin_x = geoorigin_vertex.x();
    _geoorigin_y = geoorigin_vertex.y();
    _geoorigin_z = geoorigin_vertex.z();
    
    LOG_INFO("GeoOrigin ECEF offset:", _geoorigin_x, ",", _geoorigin_y, ",", _geoorigin_z);
  } else {
    // No GeoOrigin - use original ECEF coordinates (no offset)
    _geoorigin_x = 0.0;
    _geoorigin_y = 0.0;
    _geoorigin_z = 0.0;
    LOG_INFO("Using original ECEF coordinates (no GeoOrigin offset)");
  }
  
  // Generate vertices with GeoOrigin offset applied (or no offset if not set)
  _vertices = get_vertices(bathymetry, options.exaggeration(), 
                           _geoorigin_x, _geoorigin_y, _geoorigin_z);
  
  size_t valid_vertices = 0;
  for (const auto& v : _vertices) {
    if (v.is_valid())
      valid_vertices++;
  }
  LOG_INFO("Created", Logger::format_with_commas(valid_vertices), "vertices");
}

double Geometry::to_radians(double degrees) { return degrees * (3.1415926535 / 180.0); }

double Geometry::get_longitude(const Bathymetry& bathymetry, size_t x) {
  return bathymetry.longitude_min() + bathymetry.longitude_spacing() * (double)x;
}

double Geometry::get_latitude(const Bathymetry& bathymetry, size_t y) {
  return bathymetry.latitude_max() - bathymetry.latitude_spacing() * (double)y;
}

Vertex Geometry::get_earth_centered_vertex(double longitude, double latitude, double altitude,
                                           uint32_t id) {
  // WGS-84 ellipsoid calculations
  double F = 1.0 / WGS_84_INVERSE_FLATTENING;
  double e_squared = F * (2.0 - F);

  double sin_lon = sin(to_radians(longitude));
  double cos_lon = cos(to_radians(longitude));
  double sin_lat = sin(to_radians(latitude));
  double cos_lat = cos(to_radians(latitude));

  double N = WGS_84_SEMI_MAJOR_AXIS / sqrt(1.0 - e_squared * sin_lat * sin_lat);
  double tmp = (N + altitude) * cos_lat;
  double x = tmp * cos_lon;
  double y = tmp * sin_lon;
  double z = (N * (1 - e_squared) + altitude) * sin_lat;

  // gltf assumes y is up
  // With "return Vertex(x, z, y, id)" terrain ends up south of Australia
  return Vertex(x, y, z, id);
}

Matrix<Vertex> Geometry::get_vertices(const Bathymetry& bathymetry, double vertical_exaggeration,
                                       double geoorigin_x, double geoorigin_y, double geoorigin_z) {
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
        double adjusted_altitude = (double)altitude * vertical_exaggeration;
        Vertex vertex = get_earth_centered_vertex(longitude, latitude, adjusted_altitude, vertex_id++);
        
        // Print first few valid vertices for debugging
        if (!printed_first) {
          if (geoorigin_x != 0.0 || geoorigin_y != 0.0 || geoorigin_z != 0.0) {
            LOG_INFO("First vertex [", x, ",", y, "] before offset: x=", vertex.x(), 
                     "y=", vertex.y(), "z=", vertex.z(), 
                     "lon=", longitude, "lat=", latitude, "alt=", altitude);
          }
        }
        
        // Apply GeoOrigin offset
        out.at(x, y) = Vertex(vertex.x() - geoorigin_x, 
                              vertex.y() - geoorigin_y, 
                              vertex.z() - geoorigin_z, 
                              vertex.index());
        
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
            LOG_INFO("Sample vertex #1000 [", x, ",", y, "] before offset: x=", vertex.x(), 
                     "y=", vertex.y(), "z=", vertex.z());
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
std::vector<Geometry::Tile> Geometry::get_triangles_tiled(const Matrix<Vertex>& vertices,
                                                          size_t tileSize) {
  std::vector<Geometry::Tile> tiles;

  // Number of cells. Triangles are generated over cells.
  const size_t cellsY = (vertices.size_y() > 0) ? vertices.size_y() - 1 : 0;
  const size_t cellsX = (vertices.size_x() > 0) ? vertices.size_x() - 1 : 0;
  if (cellsX == 0 || cellsY == 0)
    return tiles;

  for (size_t ty = 0; ty < cellsY; ty += tileSize) {
    for (size_t tx = 0; tx < cellsX; tx += tileSize) {
      const size_t endY = std::min(ty + tileSize, cellsY);
      const size_t endX = std::min(tx + tileSize, cellsX);

      Geometry::Tile tile;
      tile.x0 = tx;
      tile.y0 = ty;
      tile.x1 = endX;
      tile.y1 = endY;
      tile.triangles.reserve(2ull * (endX - tx) * (endY - ty));

      for (size_t y = ty; y < endY; ++y) {
        for (size_t x = tx; x < endX; ++x) {
          const auto& bottom_left = vertices.at(x, y);
          const auto& bottom_right = vertices.at(x + 1, y);
          const auto& top_left = vertices.at(x, y + 1);
          const auto& top_right = vertices.at(x + 1, y + 1);

          if (bottom_left.is_valid() && top_right.is_valid()) {
            if (top_left.is_valid())
              tile.triangles.emplace_back(
                  Triangle{bottom_left.index(), top_left.index(), top_right.index()});

            if (bottom_right.is_valid())
              tile.triangles.emplace_back(
                  Triangle{bottom_left.index(), top_right.index(), bottom_right.index()});
          } else if (bottom_right.is_valid() && top_left.is_valid()) {
            if (bottom_left.is_valid())
              tile.triangles.emplace_back(
                  Triangle{bottom_right.index(), bottom_left.index(), top_left.index()});

            if (top_right.is_valid())
              tile.triangles.emplace_back(
                  Triangle{bottom_right.index(), top_left.index(), top_right.index()});
          }
        }
      }

      if (!tile.triangles.empty())
        tiles.push_back(std::move(tile));
    }
  }

  return tiles;
}
} // namespace mbgrd2gltf
