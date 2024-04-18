/*--------------------------------------------------------------------
 *    The MB-system:	geometry.h	5/11/2023
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
 *    The program MBgrd2gltf, including this source file, was created
 *    by a Capstone Project team at the California State University
 *    Monterey Bay (CSUMB) including Kyle Dowling, Julian Fortin,
 *    Jesse Benavides, Nicolas Porras Falconio. This team was mentored by:
 *    Mike McCann
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

 // local includes
#include "vertex.h"
#include "triangle.h"
#include "matrix.h"
#include "bathymetry.h"


// standard library
#include <vector>

namespace mbgrd2gltf {
	class Geometry {
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
