/*--------------------------------------------------------------------
 *    The MB-system:	triangle.h	5/11/2023
 *
 *    Copyright (c) 2023-2023 by
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

#ifndef TRIANGLE_H
#define TRIANGLE_H

// standard library
#include <cstdint>

namespace mbgrd2gltf
{
	class Triangle
	{
	private: // members

		uint32_t _a;
		uint32_t _b;
		uint32_t _c;

	public: // methods

		Triangle() :
		_a(0),
		_b(0),
		_c(0)
		{}

		Triangle(uint32_t a, uint32_t b, uint32_t c) :
		_a(a),
		_b(b),
		_c(c)
		{}

		inline uint32_t a() const { return _a; }
		inline uint32_t b() const { return _b; }
		inline uint32_t c() const { return _c; }
	};
}

#endif
