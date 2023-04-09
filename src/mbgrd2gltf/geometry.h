#ifndef STOQS_GEOMETRY_H
#define STOQS_GEOMETRY_H

// local includes
#include <stoqs/vertex.h>
#include <stoqs/triangle.h>
#include <stoqs/matrix.h>
#include <stoqs/bathymetry.h>

// standard library
#include <vector>

namespace stoqs
{
	class Geometry
	{
	private: // members

		Matrix<Vertex> _vertices;
		std::vector<Triangle> _triangles;

	private: // methods

		static double to_radians(double degrees);
		static double get_longitude(const Bathymetry& bathymetry, size_t x);
		static double get_latitude(const Bathymetry& bathymetry, size_t y);
		static Vertex get_earth_centered_vertex(double longitude, double latitude, double altitude, uint32_t id);
		static Matrix<Vertex> get_vertices(const Bathymetry& bathymetry, double vertical_exaggeration);
		static std::vector<Triangle> get_triangles(const Matrix<Vertex>& vertices);

	public: // methods

		Geometry(const Bathymetry& bathymetry, const Options& options);

		const Matrix<Vertex>& vertices() const { return _vertices; }
		const std::vector<Triangle>& triangles() const { return _triangles; }
	};
}

#endif
