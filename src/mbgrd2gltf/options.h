/*--------------------------------------------------------------------
 *    The MB-system:	options.h	5/11/2023
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

#ifndef OPTIONS_H
#define OPTIONS_H

// standard library
#include <string>
#include <vector>

#include "draco_mesh.h"

namespace mbgrd2gltf {
class Options {
private: // members
  std::string _input_filepath;
  std::string _output_filepath;
  double _exaggeration = 1.0;
  double _geoorigin_lon = 0.0;
  double _geoorigin_lat = 0.0;
  double _geoorigin_elev = 0.0;
  bool _is_binary_output = true;   // GLB unless --no-binary
  bool _is_help = false;
  bool _is_verbose = false;
  bool _is_exaggeration_set = false;
  bool _is_output_folder_set = false;
  bool _is_geoorigin_set = false;
  bool _is_geoorigin_auto = false;
  bool _is_html_output = false;
  bool _colors = true;                            // COLOR_0 by height, unless --no-colors
  bool _is_cesium_html_output = false;            // --cesium-html: also a page for CesiumJS
  std::string _cesium_url;                        // --cesium-url: the CesiumJS build of that page; empty for the default
  mbdraco::Settings _draco;                       // on by default; see src/mbdraco/draco_mesh.h
  std::vector<std::string> _geoviewpoints;        // --geoviewpoint values, as given

public: // members
  Options(unsigned argc, const char** argv);

  const std::string& input_filepath() const { return _input_filepath; }
  const std::string& output_filepath() const { return _output_filepath; }
  double exaggeration() const { return _exaggeration; }
  double geoorigin_lon() const { return _geoorigin_lon; }
  double geoorigin_lat() const { return _geoorigin_lat; }
  double geoorigin_elev() const { return _geoorigin_elev; }
  bool is_binary_output() const { return _is_binary_output; }
  bool is_help() const { return _is_help; }
  bool is_verbose() const { return _is_verbose; }
  bool is_exaggeration_set() const { return _is_exaggeration_set; }
  bool is_output_folder_set() const { return _is_output_folder_set; }
  bool is_draco_compressed() const { return _draco.enabled; }
  bool is_geoorigin_set() const { return _is_geoorigin_set; }
  bool is_geoorigin_auto() const { return _is_geoorigin_auto; }
  bool is_html_output() const { return _is_html_output; }
  bool is_cesium_html_output() const { return _is_cesium_html_output; }
  bool colors() const { return _colors; }
  const std::string& cesium_url() const { return _cesium_url; }
  const mbdraco::Settings& draco() const { return _draco; }
  const std::vector<std::string>& geoviewpoints() const { return _geoviewpoints; }
};
} // namespace mbgrd2gltf

#endif
