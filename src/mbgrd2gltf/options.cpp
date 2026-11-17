/*--------------------------------------------------------------------
 *    The MB-system:	options.cpp	5/11/2023
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

#include "options.h"
#include "viewpoint.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#if defined(_WIN32) || defined(WIN32) || defined(WIN64)
#define OS_IS_WINDOWS true
#else
#define OS_IS_WINDOWS false
#endif

#if OS_IS_WINDOWS
const char dir_delim = '\\';
#else
const char dir_delim = '/';
#endif

constexpr char program_name[] = "MBgrd2gltf";
constexpr char help_message[] =
    "MBgrd2gltf converts a GMT GRD format bathymetry grid file into\n"
    "a glTF (GL Transmission Format) 3D model file. The program generates\n"
    "a 3D mesh representation of the bathymetry with optional Draco\n"
    "compression, vertical exaggeration, and binary output format.\n"
    "\n"
    "The output mesh vertices are positioned in an Earth-Centered, Earth-Fixed\n"
    "(ECEF) Cartesian coordinate system with units in meters. ECEF is a 3D\n"
    "right-handed coordinate system with its origin at Earth's center of mass.\n"
    "A GeoOrigin can be specified to improve rendering precision for localized areas.";
constexpr char usage_message[] =
    "mbgrd2gltf --input FILE [OPTIONS]\n"
    "       mbgrd2gltf -I FILE [OPTIONS]  (legacy style)";

namespace mbgrd2gltf {

struct PathInfo {
  std::string folder;
  std::string file_basename;
};

PathInfo get_path_info(const char* filepath) {
  const char* start_of_filename = filepath;

  for (const char* iter = filepath; *iter; ++iter) {
#if OS_IS_WINDOWS
    bool is_dir_char = *iter == '/' || *iter == '\\';
#else
    bool is_dir_char = *iter == '/';
#endif

    if (is_dir_char)
      start_of_filename = iter + 1;
  }

  const char* start_of_extension = start_of_filename;

  for (const char* iter = start_of_filename; *iter; ++iter) {
    if (*iter == '.')
      break;

    start_of_extension += 1;
  }

  std::string file_basename =
      std::string(start_of_filename, start_of_extension - start_of_filename);
  std::string folder = std::string(filepath, start_of_filename - filepath);

  if (file_basename.empty())
    file_basename = "output";

  return {folder, file_basename};
}

void print_help() {
  std::cout << "\n" << program_name << "\n\n";
  std::cout << help_message << "\n\n";
  std::cout << "usage: " << usage_message << "\n\n";
  std::cout << "Options:\n";
  std::cout << "  --input, -I FILE              Input GMT GRD format bathymetry grid file (required)\n";
  std::cout << "  --no-binary                   Write glTF (.gltf, JSON with the buffers embedded) in place of the binary glTF (.glb)\n";
  std::cout << "                                that is written by default; --binary, -B are accepted and do nothing\n";
  std::cout << "  --output, -O ROOT             Output file root [default: input file basename]\n";
  std::cout << "  --exaggeration, -E NUM        Vertical exaggeration factor, applied about the grid's mean altitude [default: 1.0]\n";
  std::cout << "  --geoorigin[=LON,LAT,EL]      GeoOrigin for high-precision local coordinates\n";
  std::cout << "  -G[LON,LAT,EL]                Short form; the value follows -G with no space\n";
  std::cout << "                                  With values: use specified lon,lat,elev\n";
  std::cout << "                                  Without values: use grid center and mean altitude\n";
  std::cout << "                                  Not specified: original ECEF coordinates (default)\n";
  std::cout << "                                The value must be attached to the option, with an equals sign and\n";
  std::cout << "                                no spaces: --geoorigin=-122.0,36.8,-1000 or -G-122.0,36.8,-1000.\n";
  std::cout << "                                A value in a separate argument is not accepted.\n";
  std::cout << "                                GLB positions are 32-bit floats, so absolute ECEF (magnitudes of about\n";
  std::cout << "                                6.4 million meters) resolves only about 0.5 m; use a GeoOrigin to\n";
  std::cout << "                                preserve sub-meter detail\n";
  std::cout << "  --no-draco                    Write the mesh as arrays. By default the mesh is compressed with Draco\n";
  std::cout << "                                (KHR_draco_mesh_compression), as mbmesh does; --draco, -D are accepted and do nothing\n";
  std::cout << "  --draco-precision NUM         With Draco, keep positions to this many meters [default: 0.001]: each block (a primitive of the file) gets the\n";
  std::cout << "                                bits that make the step of its quantization no coarser, between 8 and 30\n";
  std::cout << "  --draco-normal-bits NUM       With Draco, the bits of each normal, 2-30 [default: 10]; the mesh has no normals\n";
  std::cout << "                                today, so this changes nothing\n";
  std::cout << "  --geoviewpoint=VALUES         With --html or --cesium-html, add a GeoViewpoint to the page; can be repeated. The values are\n";
  std::cout << "                                geodetic, in the order of --geoorigin, so they do not depend on the GeoOrigin\n";
  std::cout << "                                or the grid and can be used again: lon,lat,elev,clon,clat,celev, the camera\n";
  std::cout << "                                position and the point it looks at, or lon,lat,elev,ax,ay,az,angle,clon,clat,celev,\n";
  std::cout << "                                with the orientation as an axis and an angle, as a GeoViewpoint takes it. Heights are\n";
  std::cout << "                                above the ellipsoid. The Copy GeoViewpoint button of the page gives the ten-number form.\n";
  std::cout << "                                The value must be attached: --geoviewpoint=-122.0,36.8,-900,...\n";
  std::cout << "  --no-colors                   Do not color the vertices by height. By default every vertex has a color (COLOR_0) from the Haxby table that\n";
  std::cout << "                                mbm_grdplot uses, over the range of the grid, and the pages have a \"color by height\" check box that turns\n";
  std::cout << "                                the colors off, which shows the model in one gray, as mbmesh does\n";
  std::cout << "  --html, -W                    Generate HTML viewer with inline glTF/GLB model and print the command to view it\n";
  std::cout << "  --cesium-html                 Also write a viewer page for CesiumJS, <output>_cesium.html beside the model. The globe of Cesium would hide\n";
  std::cout << "                                data below the ellipsoid, so the page starts with it hidden and has a check box that shows it, translucent\n";
  std::cout << "  --cesium-url URL              The directory of the CesiumJS build of the --cesium-html page, with Cesium.js and Widgets/widgets.css in it\n";
  std::cout << "                                [https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium]\n";
  std::cout << "  --verbose, -V                 Enable verbose output\n";
  std::cout << "  --help, -H                    Print this help message\n\n";
}

// Helper to check if argument starts with prefix
static bool starts_with(const char* str, const char* prefix) {
  return std::strncmp(str, prefix, std::strlen(prefix)) == 0;
}

// Helper to extract value from --option=value format
static const char* get_option_value(const char* arg) {
  const char* eq = std::strchr(arg, '=');
  return eq ? eq + 1 : nullptr;
}

Options::Options(unsigned argc, const char* argv[]) {
  // Parse command line arguments supporting both modern (--option) and legacy (-O) styles
  
  if (argc < 2) {
    print_help();
    _is_help = true;
    return;
  }

  for (unsigned i = 1; i < argc; i++) {
    const char* arg = argv[i];
    
    if (arg[0] != '-') {
      throw std::invalid_argument(std::string("Unexpected argument: ") + arg + 
                                  "\nAll options must start with '-' or '--'");
    }

    // Modern double-dash long options
    if (arg[1] == '-') {
      const char* option = arg + 2;
      const char* value_ptr = get_option_value(arg);
      
      // Extract option name (before '=' if present)
      std::string opt_name;
      if (value_ptr) {
        opt_name = std::string(option, value_ptr - option - 1);
      } else {
        opt_name = option;
      }
      
      // Get value from next arg if not in --option=value format
      if (!value_ptr && i + 1 < argc && argv[i + 1][0] != '-') {
        // Don't consume next arg for flags
        if (opt_name != "binary" && opt_name != "no-binary" && opt_name != "draco" && opt_name != "no-draco" && opt_name != "verbose" && opt_name != "help" && opt_name != "geoorigin" && opt_name != "html" && opt_name != "cesium-html" && opt_name != "no-colors") {
          value_ptr = argv[++i];
        }
      }
      
      if (opt_name == "input") {
        if (!value_ptr) throw std::invalid_argument("--input requires a file argument");
        _input_filepath = value_ptr;
      }
      else if (opt_name == "output") {
        if (!value_ptr) throw std::invalid_argument("--output requires an output file root argument");
        _output_filepath = value_ptr;
        _is_output_folder_set = true;
      }
      else if (opt_name == "exaggeration") {
        if (!value_ptr) throw std::invalid_argument("--exaggeration requires a numeric value");
        _exaggeration = std::atof(value_ptr);
        if (_exaggeration <= 0.0) throw std::invalid_argument("Exaggeration must be > 0");
        _is_exaggeration_set = true;
      }
      else if (opt_name == "binary") {
        _is_binary_output = true;   // the default; accepted for the scripts that give it
      }
      else if (opt_name == "no-binary") {
        _is_binary_output = false;
      }
      else if (opt_name == "draco") {
        _draco.enabled = true;   // the default; accepted for the scripts that give it
      }
      else if (opt_name == "no-draco") {
        _draco.enabled = false;
      }
      else if (opt_name == "draco-precision") {
        if (!value_ptr) throw std::invalid_argument("--draco-precision requires a size in meters");
        std::string reason;
        if (!mbdraco::parse_precision(value_ptr, &_draco.precision, &reason)) throw std::invalid_argument("Invalid --draco-precision: " + reason);
      }
      else if (opt_name == "draco-normal-bits") {
        if (!value_ptr) throw std::invalid_argument("--draco-normal-bits requires a number of bits");
        std::string reason;
        if (!mbdraco::parse_normal_bits(value_ptr, &_draco.normal_bits, &reason)) throw std::invalid_argument("Invalid --draco-normal-bits: " + reason);
      }
      else if (opt_name == "geoorigin") {
        if (!value_ptr) {
          _is_geoorigin_auto = true;
        } else {
          std::string geoorigin_str(value_ptr);
          size_t comma1 = geoorigin_str.find(',');
          size_t comma2 = geoorigin_str.rfind(',');
          if (comma1 == std::string::npos || comma2 == std::string::npos || comma1 == comma2) {
            throw std::invalid_argument("GeoOrigin format must be lon,lat,elev");
          }
          _geoorigin_lon = std::atof(geoorigin_str.substr(0, comma1).c_str());
          _geoorigin_lat = std::atof(geoorigin_str.substr(comma1 + 1, comma2 - comma1 - 1).c_str());
          _geoorigin_elev = std::atof(geoorigin_str.substr(comma2 + 1).c_str());
          _is_geoorigin_set = true;
        }
      }
      else if (opt_name.compare(0, 9, "quantize-") == 0) {
        throw std::invalid_argument("--" + opt_name + " is replaced by --draco-precision (the step of the positions in meters, for "
                                    "each block) and --draco-normal-bits, which mbmesh takes as well");
      }
      else if (opt_name == "geoviewpoint") {
        if (!value_ptr) throw std::invalid_argument("--geoviewpoint requires a value, attached: --geoviewpoint=lon,lat,elev,clon,clat,celev");
        UserViewpoint viewpoint;
        std::string viewpoint_error;
        if (!parse_user_viewpoint(value_ptr, &viewpoint, &viewpoint_error))
          throw std::invalid_argument("Invalid --geoviewpoint: " + viewpoint_error);
        _geoviewpoints.push_back(value_ptr);
      }
      else if (opt_name == "verbose") {
        _is_verbose = true;
      }
      else if (opt_name == "html") {
        _is_html_output = true;
      }
      else if (opt_name == "no-colors") {
        _colors = false;
      }
      else if (opt_name == "cesium-html") {
        _is_cesium_html_output = true;
      }
      else if (opt_name == "cesium-url") {
        if (!value_ptr) throw std::invalid_argument("--cesium-url requires the directory of a CesiumJS build");
        _cesium_url = value_ptr;
      }
      else if (opt_name == "help") {
        print_help();
        _is_help = true;
        return;
      }
      else {
        throw std::invalid_argument(std::string("Unknown option: --") + opt_name);
      }
    }
    // Legacy single-dash short options
    else {
      char option = arg[1];
      const char* value_ptr = nullptr;
      
      // Check if value is attached to option (e.g., -Ifile.grd)
      if (arg[2] != '\0') {
        value_ptr = &arg[2];
      }
      // Check if value is in next argument (e.g., -I file.grd)
      else if (i + 1 < argc && option != 'B' && option != 'D' && option != 'V' && option != 'H' && option != 'G' && option != 'W') {
        value_ptr = argv[++i];
      }

      switch (option) {
        case 'I':
        case 'i':
          if (!value_ptr) {
            throw std::invalid_argument("Option -I requires a grdfile argument");
          }
          _input_filepath = value_ptr;
          break;

        case 'O':
        case 'o':
          if (!value_ptr) {
            throw std::invalid_argument("Option -O requires an output file root argument");
          }
          _output_filepath = value_ptr;
          _is_output_folder_set = true;
          break;

        case 'E':
        case 'e':
          if (!value_ptr) {
            throw std::invalid_argument("Option -E requires an exaggeration value");
          }
          _exaggeration = std::atof(value_ptr);
          if (_exaggeration <= 0.0) {
            throw std::invalid_argument("Exaggeration must be > 0");
          }
          _is_exaggeration_set = true;
          break;

        case 'B':
        case 'b':
          _is_binary_output = true;
          break;

        case 'D':
        case 'd':
          _draco.enabled = true;   // the default; accepted for the scripts that give it
          break;

        case 'G':
        case 'g':
          if (!value_ptr) {
            // -G without arguments: use automatic GeoOrigin (grid center)
            _is_geoorigin_auto = true;
          } else {
            // -G with arguments: parse lon,lat,elev
            std::string geoorigin_str(value_ptr);
            size_t comma1 = geoorigin_str.find(',');
            size_t comma2 = geoorigin_str.rfind(',');
            
            if (comma1 == std::string::npos || comma2 == std::string::npos || comma1 == comma2) {
              throw std::invalid_argument("GeoOrigin format must be lon,lat,elev");
            }
            
            _geoorigin_lon = std::atof(geoorigin_str.substr(0, comma1).c_str());
            _geoorigin_lat = std::atof(geoorigin_str.substr(comma1 + 1, comma2 - comma1 - 1).c_str());
            _geoorigin_elev = std::atof(geoorigin_str.substr(comma2 + 1).c_str());
            _is_geoorigin_set = true;
          }
          break;

        case 'Q':
        case 'q':
          throw std::invalid_argument("-Q is replaced by --draco-precision (the step of the positions in meters, for each block) "
                                      "and --draco-normal-bits, which mbmesh takes as well");

        case 'V':
        case 'v':
          _is_verbose = true;
          break;

        case 'W':
        case 'w':
          _is_html_output = true;
          break;

        case 'H':
        case 'h':
          print_help();
          _is_help = true;
          return;

        default:
          throw std::invalid_argument(std::string("Unknown option: -") + option);
      }
    }
  }

  if (!_geoviewpoints.empty() && !_is_html_output && !_is_cesium_html_output) {
    throw std::invalid_argument("--geoviewpoint adds a viewpoint to the page that --html or --cesium-html writes; use one of them too");
  }

  // Check required arguments
  if (_input_filepath.empty()) {
    throw std::invalid_argument("Input grdfile is required (use --input FILE or -I FILE)");
  }

  // Set output path if not specified
  if (!_is_output_folder_set) {
    auto path_info = get_path_info(_input_filepath.c_str());
    _output_filepath = path_info.folder;
    
    if (_output_filepath.empty()) {
      _output_filepath = ".";
      _output_filepath += dir_delim;
    } else {
      if (_output_filepath.back() != dir_delim)
        _output_filepath += dir_delim;
    }
    
    _output_filepath += path_info.file_basename;
  }
  // If user specified output root, use it directly (no modification needed)
}

} // namespace mbgrd2gltf
