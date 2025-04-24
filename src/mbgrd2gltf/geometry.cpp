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

  // standard library
#include <cmath>
#include <iostream>

#define EARTH_RADIUS_M 6371000.0
#define WGS_84_SEMI_MAJOR_AXIS 6378137.0
#define WGS_84_INVERSE_FLATTENING 298.257223563

namespace mbgrd2gltf {
	Geometry::Geometry(const Bathymetry& bathymetry, const Options& options) :
		_vertices(get_vertices(bathymetry, options.exaggeration())),
		_triangles(get_triangles(_vertices)) {
	}

	double Geometry::to_radians(double degrees) {
		return degrees * (3.1415926535 / 180.0);
	}

	double Geometry::get_longitude(const Bathymetry& bathymetry, size_t x) {
		return bathymetry.longitude_min() + bathymetry.longitude_spacing() * (double)x;
	}

	double Geometry::get_latitude(const Bathymetry& bathymetry, size_t y) {
		//std::cerr << std::fixed;
		//std::cerr << "bathymetry.latitude_min(): " << bathymetry.latitude_min() << '\n';
		//std::cerr << "bathymetry.latitude_max(): " << bathymetry.latitude_max() << '\n';
		//return bathymetry.latitude_min() + bathymetry.latitude_spacing() * (double)y;
		return bathymetry.latitude_max() - bathymetry.latitude_spacing() * (double)y;
	}

	Vertex Geometry::get_earth_centered_vertex(double longitude, double latitude, double altitude, uint32_t id) {
		// Test with:
		// longitude, latitude, altitude: -122.503094 37.058946 -1020
		// x, y, z:                       -2737902.2652    -4297133.60787    3822000.89563
		//longitude = -122.503094;
		//latitude = 37.058946;
		//altitude = -1020;

		//std::cerr << std::fixed;
		//std::cerr << "longitude, latitude, altitude: " << longitude << ' ' << latitude << ' ' << altitude << '\n';
		double phi = to_radians(latitude);
		double theta = to_radians(longitude);

		double cos_phi = cos(phi);
		double cos_theta = cos(theta);
		double sin_phi = sin(phi);
		double sin_theta = sin(theta);
		double rho = EARTH_RADIUS_M + altitude;

		// this assumes z is up
		double x = rho * cos_phi * cos_theta;
		double y = rho * cos_phi * sin_theta;
		double z = rho * sin_phi;

		//std::cerr << "x, y, z: " << x << ' ' << y << ' ' << z << '\n';

		// Mimic https://github.com/GenericMappingTools/gmt/blob/be890649579be45e94269632786d08890a26dfea/src/gmt_map.c#L9094-L9108
		// and   https://github.com/x3dom/x3dom/blob/3ace18318cd192e424546569932abfe3e1e2346a/src/nodes/Geospatial/GeoCoordinate.js#L380-L443
		double F = 1.0 / WGS_84_INVERSE_FLATTENING;
		double e_squared = F * (2.0 - F);

		double sin_lon = sin(to_radians(longitude));
		double cos_lon = cos(to_radians(longitude));
		double sin_lat = sin(to_radians(latitude));
		double cos_lat = cos(to_radians(latitude));

		double N = WGS_84_SEMI_MAJOR_AXIS / sqrt(1.0 - e_squared * sin_lat * sin_lat);
		double tmp = (N + altitude) * cos_lat;
		x = tmp * cos_lon;
		y = tmp * sin_lon;
		z = (N * (1 - e_squared) + altitude) * sin_lat;

		//std::cerr << "WGS-84 x, y, z: " << x << ' ' << y << ' ' << z << '\n';
		// [vagrant@localhost build]$ ./grd-to-gltf Monterey25.grd -e 10 -b
		// longitude, latitude, altitude: -122.503094 36.440848 -1020.000000
		// x, y, z: -2753604.329500 -4321778.000725 3783720.828683
		// WGS-84 x, y, z: -2759951.394247 -4331739.709638 3767050.208608
		// [vagrant@localhost build]$ echo "-122.503094 36.440848 -1020.000000" | gmt mapproject -E
		// -2759951.41997    -4331739.72409    3767050.17337
		//exit(0);


		// gltf assumes y is up
		// With "return Vertex(x, z, y, id)" terrain ends up south of Australia
		return Vertex(x, y, z, id);
	}

	Matrix<Vertex> Geometry::get_vertices(const Bathymetry& bathymetry, double vertical_exaggeration) {
		Matrix<Vertex> out(bathymetry.size_x(), bathymetry.size_y());
		const auto& altitudes = bathymetry.altitudes();
		uint32_t vertex_id = 1;

		for (size_t y = 0; y < altitudes.size_y(); ++y) {
			for (size_t x = 0; x < altitudes.size_x(); ++x) {
				float altitude = altitudes.at(x, y);

				if (!std::isnan(altitude)) {
					double longitude = get_longitude(bathymetry, x);
					double latitude = get_latitude(bathymetry, y);
					double adjusted_altitude = (double)altitude * vertical_exaggeration;
					out.at(x, y) = get_earth_centered_vertex(longitude, latitude, adjusted_altitude, vertex_id++);
				}
			}
		}

		return out;
	}

	std::vector<Triangle> Geometry::get_triangles(const Matrix<Vertex>& vertices) {
		size_t end_y = vertices.size_y() - 1;
		size_t end_x = vertices.size_x() - 1;
		size_t max_triangle_count = 2 * end_x * end_y;

		std::vector<Triangle> out;

		out.reserve(max_triangle_count);

		for (size_t y = 0; y < end_y; ++y) {
			for (size_t x = 0; x < end_x; ++x) {
				const auto& bottom_left = vertices.at(x, y);
				const auto& bottom_right = vertices.at(x + 1, y);
				const auto& top_left = vertices.at(x, y + 1);
				const auto& top_right = vertices.at(x + 1, y + 1);

				if (bottom_left.is_valid() && top_right.is_valid()) {
					if (top_left.is_valid())
						out.emplace_back(Triangle{
							bottom_left.index(),
							top_left.index(),
							top_right.index()
							});

					if (bottom_right.is_valid())
						out.emplace_back(Triangle{
							bottom_left.index(),
							top_right.index(),
							bottom_right.index()
							});
				}
				else if (bottom_right.is_valid() && top_left.is_valid()) {
					if (bottom_left.is_valid())
						out.emplace_back(Triangle{
							bottom_right.index(),
							bottom_left.index(),
							top_left.index()
							});

					if (top_right.is_valid())
						out.emplace_back(Triangle{
							bottom_right.index(),
							top_left.index(),
							top_right.index()
							});
				}
			}
		}

		return out;
	}

}
