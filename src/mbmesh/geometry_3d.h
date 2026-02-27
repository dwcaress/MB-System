#ifndef GEOMETRY_3D_H
#define GEOMETRY_3D_H

#include <vector>
#include <cstdint>
#include <unordered_map>
#include "bathymetry.h"
#include "gltf_writer.h"

class Geometry3D {
  public:
	Geometry3D(const Bathymetry &bathymetry);
	std::vector<float> get_vertices_3d() const;
	std::vector<uint32_t> get_indices() const;
	GeoOrigin get_geo_origin() const { return _geo_origin; }

  private:
	const Bathymetry &_bathymetry;
	std::vector<float> _vertices_3d;
	std::vector<uint32_t> _indices;
	std::unordered_map<uint32_t, uint32_t> _grid_to_vertex_index;
	GeoOrigin _geo_origin;

	void generate_vertices_from_grid();
	void generate_triangles();
	void geodetic_to_ecef(double lon, double lat, double elevation, double &x, double &y, double &z) const;
	void calculate_geo_origin();
};

#endif
