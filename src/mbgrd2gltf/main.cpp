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

#include <iostream>
#include <exception>
#include <typeinfo>
#include <stdexcept>
#include "options.h"
#include "bathymetry.h"
#include "geometry.h"
#include "model.h"

using namespace mbgrd2gltf;

int main(int argc, char* argv[]) {
	try {
		Options options((unsigned)argc, (const char**)argv);
		if (options.is_help()) {
			std::cout << "Help requested, exiting..." << std::endl;
			return 0;
		}
		Bathymetry bathymetry(options);
		Geometry geometry(bathymetry, options);
		model::write_gltf(geometry, options);
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "Invalid argument error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::out_of_range& e) {
		std::cerr << "Out of range error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::bad_alloc& e) {
		std::cerr << "Memory allocation error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "General error: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown error occurred." << std::endl;
		return 1;
	}
	return 0;
}
