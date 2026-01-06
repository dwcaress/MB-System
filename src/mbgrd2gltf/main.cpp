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
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "logger.h"
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

    // Configure logger based on verbose flag
    if (options.is_verbose()) {
      Logger::set_level(LogLevel::DEBUG);
    } else {
      Logger::set_level(LogLevel::INFO);
    }

    // Log the command line
    std::string command_line;
    for (int i = 0; i < argc; ++i) {
      if (i > 0) command_line += " ";
      command_line += argv[i];
    }
    LOG_INFO("Command:", command_line);

    // Get input file size
    struct stat input_st;
    double input_size_mb = 0.0;
    if (stat(options.input_filepath().c_str(), &input_st) == 0) {
      input_size_mb = input_st.st_size / (1024.0 * 1024.0);
    }
    char input_size_str[32];
    snprintf(input_size_str, sizeof(input_size_str), "%.3f", input_size_mb);

    LOG_INFO("Starting mbgrd2gltf processing for", options.input_filepath(),
             "(" + std::string(input_size_str) + " MB)");
    LOG_INFO("Binary output:", options.is_binary_output() ? "enabled," : "disabled,",
             "Draco compression:", options.is_draco_compressed() ? "enabled" : "disabled");

    Bathymetry bathymetry(options);
    LOG_INFO("Generating 3D geometry from 2D bathymetric grid data");
    LOG_INFO("Vertical exaggeration:", options.exaggeration());
    Geometry geometry(bathymetry, options);

    std::string output_file =
        options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");

    // Get absolute path
    char abs_path[PATH_MAX];
    std::string abs_output_file = output_file;
    if (realpath(output_file.c_str(), abs_path) != nullptr) {
      abs_output_file = abs_path;
    } else {
      // If file doesn't exist yet, construct absolute path manually
      char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        if (output_file[0] != '/') {
          abs_output_file = std::string(cwd) + "/" + output_file;
        }
      }
    }

    model::write_gltf(geometry, options);
    
    // Get output file size
    struct stat output_st;
    double output_size_mb = 0.0;
    if (stat(output_file.c_str(), &output_st) == 0) {
      output_size_mb = output_st.st_size / (1024.0 * 1024.0);
    }
    char output_size_str[32];
    snprintf(output_size_str, sizeof(output_size_str), "%.3f", output_size_mb);
    
    LOG_INFO("Successfully wrote glTF file to", abs_output_file, 
             "(" + std::string(output_size_str) + " MB)");
  } catch (const std::invalid_argument& e) {
    std::cerr << "Invalid argument error: " << e.what() << std::endl;
    return 1;
  } catch (const std::out_of_range& e) {
    std::cerr << "Out of range error: " << e.what() << std::endl;
    return 1;
  } catch (const std::bad_alloc& e) {
    std::cerr << "Memory allocation error: " << e.what() << std::endl;
    return 1;
  } catch (const std::runtime_error& e) {
    std::cerr << "Runtime error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred." << std::endl;
    return 1;
  }
  return 0;
}
