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
#include <cstdlib>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <limits.h>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "logger.h"
#include "options.h"
#include "bathymetry.h"
#include "geometry.h"
#include "model.h"
#include "provenance.h"
#include "mb_define.h"

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

    // Start capturing log messages for provenance in HTML output
    Logger::start_capture();

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

    if (options.is_draco_compressed()) {
      LOG_INFO("Draco: positions kept to", options.draco().precision, "m in each block (--draco-precision), normals in",
               options.draco().normal_bits, "bits (--draco-normal-bits)");
    }

    Bathymetry bathymetry(options);
    LOG_INFO("Generating 3D geometry from 2D bathymetric grid data");
    LOG_INFO("Vertical exaggeration:", options.exaggeration());
    Geometry geometry(bathymetry, options);

    std::string output_file =
        options.output_filepath() + (options.is_binary_output() ? ".glb" : ".gltf");

    // Get absolute path
    char abs_path[PATH_MAX];
    std::string abs_output_file = output_file;
#ifdef _WIN32
    // _fullpath does not require the target file to already exist.
    if (_fullpath(abs_path, output_file.c_str(), PATH_MAX) != nullptr) {
      abs_output_file = abs_path;
    }
#else
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
#endif

    // What is recorded in the glTF file and in the page about where the model is and
    // how it was made. One tree, so that the two cannot disagree.
    GridRunInfo run;
    run.program = "mbgrd2gltf";
    run.version = MB_VERSION;
    run.command_line = command_line;
    run.started_at = iso_utc_time(static_cast<double>(std::time(nullptr)));
    {
      char cwd[PATH_MAX];
#ifndef _WIN32
      if (getcwd(cwd, sizeof(cwd)) != nullptr)
        run.working_directory = cwd;
      if (const char* user = std::getenv("USER"))
        run.user = user;
      else if (const char* logname = std::getenv("LOGNAME"))
        run.user = logname;
      char host[256] = "";
      if (gethostname(host, sizeof(host) - 1) == 0)
        run.host = host;
#else
      if (const char* user = std::getenv("USERNAME"))
        run.user = user;
      if (const char* host = std::getenv("COMPUTERNAME"))
        run.host = host;
      (void)cwd;
#endif
    }
    {
      std::string grid_file = options.input_filepath();
#ifndef _WIN32
      char grid_path[PATH_MAX];
      if (realpath(grid_file.c_str(), grid_path) != nullptr)
        grid_file = grid_path;
#endif
      run.grid_file = grid_file;
      const size_t slash = grid_file.find_last_of("/\\");
      run.title = slash == std::string::npos ? grid_file : grid_file.substr(slash + 1);
    }
    run.grid_columns = bathymetry.size_x();
    run.grid_rows = bathymetry.size_y();
    run.longitude_spacing = bathymetry.longitude_spacing();
    run.latitude_spacing = bathymetry.latitude_spacing();
    run.pixel_registered = bathymetry.pixel_registered();
    run.extent_west = bathymetry.longitude_min();
    run.extent_east = bathymetry.longitude_max();
    run.extent_south = bathymetry.latitude_min();
    run.extent_north = bathymetry.latitude_max();
    run.mean_altitude = geometry.reference_altitude();
    if (options.is_geoorigin_auto() || options.is_geoorigin_set()) {
      run.geoorigin_mode = options.is_geoorigin_set() ? "specified" : "automatic";
      run.has_center = true;
      run.center_x = geometry.geoorigin_x();
      run.center_y = geometry.geoorigin_y();
      run.center_z = geometry.geoorigin_z();
      if (options.is_geoorigin_set()) {
        run.center_longitude = options.geoorigin_lon();
        run.center_latitude = options.geoorigin_lat();
        run.center_height = options.geoorigin_elev();
      } else {
        run.center_longitude = (bathymetry.longitude_min() + bathymetry.longitude_max()) / 2.0;
        run.center_latitude = (bathymetry.latitude_min() + bathymetry.latitude_max()) / 2.0;
        run.center_height = geometry.reference_altitude();
      }
    }
    run.exaggeration = options.exaggeration();
    run.binary = options.is_binary_output();

    model::write_gltf(geometry, options, run);
    
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
    
    // Generate the viewer pages if requested
    if (options.is_html_output() || options.is_cesium_html_output()) {
      // The pages load the model over HTTP, which browsers require for
      // local files. Log the command to run rather than starting a server, so
      // no process is left running and no port is held.
      std::string html_dir = ".";
      size_t abs_slash = abs_output_file.find_last_of("/\\");
      if (abs_slash != std::string::npos && abs_slash > 0) {
        html_dir = abs_output_file.substr(0, abs_slash);
      }

      // Get just the page filenames
      std::string html_filename = options.output_filepath();
      size_t last_slash = html_filename.find_last_of("/\\");
      if (last_slash != std::string::npos) {
        html_filename = html_filename.substr(last_slash + 1);
      }
      std::vector<std::string> page_names;

      // Get captured log messages for provenance
      const std::vector<std::string>& log_messages = Logger::get_captured_logs();

      if (options.is_html_output()) {
        // Get current timestamp in ISO 8601 format
        std::time_t now = std::time(nullptr);
        std::tm* gmt = std::gmtime(&now);
        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(gmt, "%Y-%m-%dT%H:%M:%SZ");
        std::string timestamp = timestamp_stream.str();

        model::write_html(bathymetry, geometry, options, command_line, timestamp, log_messages, run);
        page_names.push_back(html_filename + ".html");
      }
      if (options.is_cesium_html_output()) {
        model::write_cesium_html(geometry, options, log_messages, run);
        page_names.push_back(html_filename + "_cesium.html");
      }

      // Quote the directory for the shell
      std::string quoted_dir = "'";
      for (char c : html_dir) {
        if (c == '\'') {
          quoted_dir += "'\\''";
        } else {
          quoted_dir += c;
        }
      }
      quoted_dir += "'";

      LOG_INFO("To view", std::string(page_names.size() > 1 ? "the pages" : page_names.front()) + ", run this command in a terminal (Ctrl-C stops it), then open the URL" +
                              (page_names.size() > 1 ? "s" : "") + ":");
      LOG_INFO("  python3 -m http.server 8000 --bind 127.0.0.1 --directory", quoted_dir);
      for (const std::string& page_name : page_names) {
        LOG_INFO("  http://127.0.0.1:8000/" + page_name);
      }
    }
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
