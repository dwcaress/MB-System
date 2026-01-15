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
    "right-handed coordinate system with its origin at Earth's center of mass.";
constexpr char usage_message[] =
    "mbgrd2gltf -Igrdfile [-B -Ooutputfolder -Eexaggeration\n"
    "\t-D -Qpvalue -Qnvalue -Qtvalue -Qcvalue -V -H]";

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
  std::cout << "  -I<grdfile>        Input GMT GRD format bathymetry grid file\n";
  std::cout << "  -B                 Output in binary glTF (GLB) format\n";
  std::cout << "  -O<outputfolder>   Output folder path [default: input file directory]\n";
  std::cout << "  -E<exaggeration>   Vertical exaggeration factor [default: 1.0]\n";
  std::cout << "  -D                 Enable Draco mesh compression\n";
  std::cout << "  -Qp<value>         Draco position quantization bits (2-30) [default: 16]\n";
  std::cout << "  -Qn<value>         Draco normal quantization bits (2-30) [default: 7]\n";
  std::cout << "  -Qt<value>         Draco texcoord quantization bits (2-30) [default: 10]\n";
  std::cout << "  -Qc<value>         Draco color quantization bits (2-30) [default: 8]\n";
  std::cout << "  -V                 Enable verbose output\n";
  std::cout << "  -H                 Print this help message\n\n";
}

bool Options::draco_quantization_valid() const {
  for (int i = 0; i < 4; i++)
    if (_draco_quantization[i] < 2 || _draco_quantization[i] > 30)
      return false;
  return true;
}

Options::Options(unsigned argc, const char* argv[]) {
  // Parse command line arguments using MB-System conventions
  // Options support both -I<value> and -I <value> formats
  
  if (argc < 2) {
    print_help();
    _is_help = true;
    return;
  }

  for (unsigned i = 1; i < argc; i++) {
    const char* arg = argv[i];
    
    if (arg[0] != '-') {
      throw std::invalid_argument(std::string("Unexpected argument: ") + arg + 
                                  "\nAll options must start with '-'");
    }

    char option = arg[1];
    const char* value_ptr = nullptr;
    
    // Check if value is attached to option (e.g., -Ifile.grd)
    if (arg[2] != '\0') {
      value_ptr = &arg[2];
    }
    // Check if value is in next argument (e.g., -I file.grd)
    else if (i + 1 < argc && option != 'B' && option != 'D' && option != 'V' && option != 'H') {
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

  // Check required arguments
  if (_input_filepath.empty()) {
    throw std::invalid_argument("Input grdfile is required (use -I<grdfile>)");
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
