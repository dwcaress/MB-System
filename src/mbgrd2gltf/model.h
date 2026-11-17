/*--------------------------------------------------------------------
 *    The MB-system:	model.h	5/11/2023
 *
 *    Copyright (c) 2023-2025 by
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

#ifndef MODEL_H
#define MODEL_H

// local includes
#include "geometry.h"
#include "options.h"
#include "bathymetry.h"
#include "provenance.h"

#include <vector>
#include <string>

namespace mbgrd2gltf {
namespace model {
// Writes the model. run.draco is filled in with what the mesh was compressed with, for the page that write_html() writes after.
void write_gltf(const Geometry& geometry, const Options& options, GridRunInfo& run);
void write_html(const Bathymetry& bathymetry, const Geometry& geometry, const Options& options, 
                const std::string& command_line, const std::string& timestamp,
                const std::vector<std::string>& log_messages, const GridRunInfo& run);
// Writes the page for CesiumJS, <output>_cesium.html beside the model, and the log that it links to (<output>.log), which write_html() writes too.
void write_cesium_html(const Geometry& geometry, const Options& options, const std::vector<std::string>& log_messages, const GridRunInfo& run);
}
} // namespace mbgrd2gltf

#endif
