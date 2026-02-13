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
  std::cout << "  --binary, -B                  Output in binary glTF (GLB) format\n";
  std::cout << "  --output, -O DIR              Output folder path [default: input file directory]\n";
  std::cout << "  --exaggeration, -E NUM        Vertical exaggeration factor [default: 1.0]\n";
  std::cout << "  --geoorigin, -G [LON,LAT,EL]  GeoOrigin for high-precision local coordinates\n";
  std::cout << "                                  With values: use specified lon,lat,elev\n";
  std::cout << "                                  Without values: use grid center and mean altitude\n";
  std::cout << "                                  Not specified: original ECEF coordinates (default)\n";
  std::cout << "  --draco, -D                   Enable Draco mesh compression\n";
  std::cout << "  --quantize-position NUM       Draco position quantization bits (2-30) [default: 16]\n";
  std::cout << "  --quantize-normal NUM         Draco normal quantization bits (2-30) [default: 7]\n";
  std::cout << "  --quantize-texcoord NUM       Draco texcoord quantization bits (2-30) [default: 10]\n";
  std::cout << "  --quantize-color NUM          Draco color quantization bits (2-30) [default: 8]\n";
  std::cout << "                      (-Qp, -Qn, -Qt, -Qc for legacy style)\n";
  std::cout << "  --verbose, -V                 Enable verbose output\n";
  std::cout << "  --help, -H                    Print this help message\n\n";
}

bool Options::draco_quantization_valid() const {
  for (int i = 0; i < 4; i++)
    if (_draco_quantization[i] < 2 || _draco_quantization[i] > 30)
      return false;
  return true;
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
        if (opt_name != "binary" && opt_name != "draco" && opt_name != "verbose" && opt_name != "help" && opt_name != "geoorigin") {
          value_ptr = argv[++i];
        }
      }
      
      if (opt_name == "input") {
        if (!value_ptr) throw std::invalid_argument("--input requires a file argument");
        _input_filepath = value_ptr;
      }
      else if (opt_name == "output") {
        if (!value_ptr) throw std::invalid_argument("--output requires a directory argument");
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
        _is_binary_output = true;
      }
      else if (opt_name == "draco") {
        _is_draco_compressed = true;
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
      else if (opt_name == "quantize-position") {
        if (!value_ptr) throw std::invalid_argument("--quantize-position requires a numeric value");
        int val = std::atoi(value_ptr);
        if (val < 2 || val > 30) throw std::invalid_argument("Quantization value must be between 2 and 30");
        _draco_quantization[0] = val;
      }
      else if (opt_name == "quantize-normal") {
        if (!value_ptr) throw std::invalid_argument("--quantize-normal requires a numeric value");
        int val = std::atoi(value_ptr);
        if (val < 2 || val > 30) throw std::invalid_argument("Quantization value must be between 2 and 30");
        _draco_quantization[1] = val;
      }
      else if (opt_name == "quantize-texcoord") {
        if (!value_ptr) throw std::invalid_argument("--quantize-texcoord requires a numeric value");
        int val = std::atoi(value_ptr);
        if (val < 2 || val > 30) throw std::invalid_argument("Quantization value must be between 2 and 30");
        _draco_quantization[2] = val;
      }
      else if (opt_name == "quantize-color") {
        if (!value_ptr) throw std::invalid_argument("--quantize-color requires a numeric value");
        int val = std::atoi(value_ptr);
        if (val < 2 || val > 30) throw std::invalid_argument("Quantization value must be between 2 and 30");
        _draco_quantization[3] = val;
      }
      else if (opt_name == "verbose") {
        _is_verbose = true;
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
      else if (i + 1 < argc && option != 'B' && option != 'D' && option != 'V' && option != 'H' && option != 'G') {
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
            throw std::invalid_argument("Option -O requires an output folder argument");
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
          _is_draco_compressed = true;
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
          if (!_is_draco_compressed) {
            throw std::invalid_argument("Quantization options require -D (Draco compression)");
          }
          if (!value_ptr) {
            throw std::invalid_argument("Quantization option requires a value");
          }
          {
            char quant_type = arg[2];
            const char* quant_value = (arg[3] != '\0') ? &arg[3] : value_ptr;
            int quant_int = std::atoi(quant_value);
            
            if (quant_int < 2 || quant_int > 30) {
              throw std::invalid_argument("Quantization value must be between 2 and 30");
            }
            
            switch (quant_type) {
              case 'p': case 'P': _draco_quantization[0] = quant_int; break;
              case 'n': case 'N': _draco_quantization[1] = quant_int; break;
              case 't': case 'T': _draco_quantization[2] = quant_int; break;
              case 'c': case 'C': _draco_quantization[3] = quant_int; break;
              default:
                throw std::invalid_argument(std::string("Unknown quantization type: -Q") + quant_type);
            }
          }
          break;

        case 'V':
        case 'v':
          _is_verbose = true;
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
  } else {
    // User specified output folder
    auto path_info = get_path_info(_input_filepath.c_str());
    
    if (_output_filepath.back() != dir_delim)
      _output_filepath += dir_delim;
    
    _output_filepath += path_info.file_basename;
  }
}

} // namespace mbgrd2gltf
