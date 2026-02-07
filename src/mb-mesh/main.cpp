/*--------------------------------------------------------------------
 *    The MB-system:  main.cpp  2/6/2026
 *
 *    Copyright (c) 2026 by
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
/**
 * @file main.cpp
 * @brief Main entry point for mb-mesh - 3D mesh generation from bathymetry data
 *
 * mb-mesh generates 3D GLTF mesh files directly from bathymetry data,
 * similar to how mbgrid generates 2D maps. This tool creates optimized
 * 3D meshes suitable for visualization and analysis.
 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include "mesh_options.h"
#include "mesh_generator.h"
#include "logger.h"

using namespace mbmesh;

int main(int argc, char* argv[]) {
  try {
    // Parse command line options
    MeshOptions options(argc, argv);
    
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
    LOG_INFO("Command: " + command_line);

    LOG_INFO("MB-Mesh: Generating 3D mesh from bathymetry data");
    LOG_INFO("Input file: " + options.input_file());
    LOG_INFO("Output file: " + options.output_file());

    // Create mesh generator
    MeshGenerator generator(options);

    // Load bathymetry data
    LOG_INFO("Loading bathymetry data...");
    if (!generator.load_data()) {
      LOG_ERROR("Failed to load bathymetry data");
      return 1;
    }

    // Generate 3D mesh
    LOG_INFO("Generating 3D mesh...");
    if (!generator.generate_mesh()) {
      LOG_ERROR("Failed to generate mesh");
      return 1;
    }

    // Write GLTF output
    LOG_INFO("Writing GLTF file...");
    if (!generator.write_gltf()) {
      LOG_ERROR("Failed to write GLTF file");
      return 1;
    }

    LOG_INFO("3D mesh generation completed successfully");
    return 0;

  } catch (const std::exception& e) {
    LOG_ERROR(std::string("Error: ") + e.what());
    return 1;
  } catch (...) {
    LOG_ERROR("Unknown error occurred");
    return 1;
  }
}
