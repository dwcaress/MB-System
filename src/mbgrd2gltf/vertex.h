/*--------------------------------------------------------------------
 *    The MB-system:	vertex.h	5/11/2023
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

#ifndef POINT_H
#define POINT_H

// standard library
#include <cstdint>

namespace mbgrd2gltf
{
	class Vertex
	{
	private: // members

		float _x;
		float _y;
		float _z;
		uint32_t _id;

	public: // methods

		Vertex() :
		_x(0),
		_y(0),
		_z(0),
		_id(0)
		{}

		Vertex(float x, float y, float z, uint32_t id) :
		_x(x),
		_y(y),
		_z(z),
		_id(id)
		{}

		Vertex(Vertex&&) = default;
		Vertex(const Vertex&) = default;

		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float z() const { return _z; }
		inline uint32_t index() const { return _id - 1; }
		inline bool is_valid() const { return _id > 0; }

		Vertex& operator=(Vertex&&) = default;
		Vertex& operator=(const Vertex&) = default;
	};
}

#endif
