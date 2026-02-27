#include "geometry_3d.h"
#include <cmath>
#include <iostream>

static const double WGS84_A = 6378137.0;
static const double WGS84_B = 6356752.314245;
static const double WGS84_E2 = 0.00669437999014132;

Geometry3D::Geometry3D(const Bathymetry &bathymetry) : _bathymetry(bathymetry) {
	std::cout << "DEBUG: Generating 3D geometry from " << bathymetry.width() << " x " << bathymetry.height() << " grid"
	          << std::endl;
	calculate_geo_origin();
	generate_vertices_from_grid();
	generate_triangles();
	std::cout << "DEBUG: Generated " << _vertices_3d.size() / 3 << " vertices" << std::endl;
	std::cout << "DEBUG: Generated " << _indices.size() / 3 << " triangles" << std::endl;
	std::cout << "DEBUG: GeoOrigin: (" << _geo_origin.ecef_x << ", " << _geo_origin.ecef_y << ", " << _geo_origin.ecef_z << ")"
	          << std::endl;
}

void Geometry3D::calculate_geo_origin() {
	// Calculate center of survey in geodetic coordinates
	double center_lon = (_bathymetry.x_min() + _bathymetry.x_max()) / 2.0;
	double center_lat = (_bathymetry.y_min() + _bathymetry.y_max()) / 2.0;
	double center_elev = -420.0; // Approximate mean seafloor depth

	// Convert to ECEF to use as GeoOrigin
	geodetic_to_ecef(center_lon, center_lat, center_elev, _geo_origin.ecef_x, _geo_origin.ecef_y, _geo_origin.ecef_z);

	std::cout << "DEBUG: Survey center: lon=" << center_lon << ", lat=" << center_lat << std::endl;
}

void Geometry3D::generate_vertices_from_grid() {
	_vertices_3d.clear();
	_grid_to_vertex_index.clear();

	uint32_t vertex_index = 0;

	// Iterate through grid and create vertices only for valid (non-NaN) points
	for (size_t y = 0; y < _bathymetry.height(); y++) {
		for (size_t x = 0; x < _bathymetry.width(); x++) {
			float elevation = _bathymetry.get_elevation(x, y);

			// Skip NaN values
			if (std::isnan(elevation))
				continue;

			double longitude = _bathymetry.x_min() + x * _bathymetry.x_spacing();
			double latitude = _bathymetry.y_max() - y * _bathymetry.y_spacing();

			double ecef_x, ecef_y, ecef_z;
			geodetic_to_ecef(longitude, latitude, (double)elevation, ecef_x, ecef_y, ecef_z);

			// Subtract GeoOrigin to create local coordinates (reduces floating-point error)
			ecef_x -= _geo_origin.ecef_x;
			ecef_y -= _geo_origin.ecef_y;
			ecef_z -= _geo_origin.ecef_z;

			_vertices_3d.push_back((float)ecef_x);
			_vertices_3d.push_back((float)ecef_y);
			_vertices_3d.push_back((float)ecef_z);

			// Map grid index to vertex index
			uint32_t grid_index = y * _bathymetry.width() + x;
			_grid_to_vertex_index[grid_index] = vertex_index++;
		}
	}

	std::cout << "DEBUG: Created " << _vertices_3d.size() / 3 << " valid vertices" << std::endl;
}

void Geometry3D::generate_triangles() {
	_indices.clear();
	size_t width = _bathymetry.width();
	size_t height = _bathymetry.height();

	// Create triangles only between valid neighbors
	for (size_t y = 0; y < height - 1; y++) {
		for (size_t x = 0; x < width - 1; x++) {
			uint32_t grid_tl = y * width + x;
			uint32_t grid_tr = y * width + (x + 1);
			uint32_t grid_bl = (y + 1) * width + x;
			uint32_t grid_br = (y + 1) * width + (x + 1);

			// Check if all four corners exist in the vertex map
			auto it_tl = _grid_to_vertex_index.find(grid_tl);
			auto it_tr = _grid_to_vertex_index.find(grid_tr);
			auto it_bl = _grid_to_vertex_index.find(grid_bl);
			auto it_br = _grid_to_vertex_index.find(grid_br);

			// Create triangles only if we have valid vertices
			if (it_tl != _grid_to_vertex_index.end() && it_tr != _grid_to_vertex_index.end() &&
			    it_bl != _grid_to_vertex_index.end() && it_br != _grid_to_vertex_index.end()) {

				uint32_t v_tl = it_tl->second;
				uint32_t v_tr = it_tr->second;
				uint32_t v_bl = it_bl->second;
				uint32_t v_br = it_br->second;

				// Two triangles per cell
				_indices.push_back(v_tl);
				_indices.push_back(v_tr);
				_indices.push_back(v_bl);

				_indices.push_back(v_tr);
				_indices.push_back(v_br);
				_indices.push_back(v_bl);
			}
		}
	}

	std::cout << "DEBUG: Created " << _indices.size() / 3 << " triangles" << std::endl;
}

void Geometry3D::geodetic_to_ecef(double lon, double lat, double elevation, double &x, double &y, double &z) const {
	double lon_rad = lon * M_PI / 180.0;
	double lat_rad = lat * M_PI / 180.0;

	double sin_lat = std::sin(lat_rad);
	double cos_lat = std::cos(lat_rad);
	double N = WGS84_A / std::sqrt(1.0 - WGS84_E2 * sin_lat * sin_lat);

	x = (N + elevation) * cos_lat * std::cos(lon_rad);
	y = (N + elevation) * cos_lat * std::sin(lon_rad);
	z = (N * (1.0 - WGS84_E2) + elevation) * sin_lat;
}

std::vector<float> Geometry3D::get_vertices_3d() const { return _vertices_3d; }
std::vector<uint32_t> Geometry3D::get_indices() const { return _indices; }
