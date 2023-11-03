/*--------------------------------------------------------------------
 *    The MB-system:	compression.h	5/11/2023
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

#ifndef COMPRESSION_H
#define COMPRESSION_H

// local includes
#include "matrix.h"
#include "options.h"

namespace mbgrd2gltf
{
	namespace compression
	{
		Matrix<float> compress(const Matrix<float>& altitudes, const Options& options);
	}
}

#endif
