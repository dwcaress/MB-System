/*--------------------------------------------------------------------
 *    The MB-system:	main.cpp	5/11/2023
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

// local includes
#include "bathymetry.h"
#include "geometry.h"
#include "model.h"
#include "options.h"

// standard library
#include <iostream>

// external libraries
#include <netcdf.h>

using namespace mbgrd2gltf;

int main(int argc, char *argv[])
{
	try
	{
		Options options((unsigned)argc, (const char**)argv);

		if (options.is_help())
			return 0;

		Bathymetry bathymetry(options);
		Geometry geometry(bathymetry, options);
		model::write_gltf(geometry, options);
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what() << std::endl;

		return 1;
	}

	return 0;
}
