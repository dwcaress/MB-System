/*--------------------------------------------------------------------
 *    The MB-system:  mesh_options.cpp  2/6/2026
 *
 *    Copyright (c) 2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "mesh_options.h"
#include "logger.h"
#include <iostream>
#include <getopt.h>
#include <cstdlib>

namespace mbmesh {

MeshOptions::MeshOptions(int argc, char* argv[])
    : help_(false),
      verbose_(false),
      input_file_(""),
      output_file_(""),
      grid_spacing_(1.0),
      vertical_exaggeration_(1.0),
      decimation_level_(0),
      use_draco_(false),
      max_triangles_(1000000),
      edge_threshold_(0.001) {
  parse_arguments(argc, argv);
}

void MeshOptions::parse_arguments(int argc, char* argv[]) {
  int c;
  
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"spacing", required_argument, 0, 's'},
    {"exaggeration", required_argument, 0, 'e'},
    {"decimation", required_argument, 0, 'd'},
    {"draco", no_argument, 0, 'c'},
    {"max-triangles", required_argument, 0, 'm'},
    {"edge-threshold", required_argument, 0, 't'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long(argc, argv, "hvi:o:s:e:d:cm:t:", long_options, nullptr)) != -1) {
    switch (c) {
      case 'h':
        help_ = true;
        print_usage();
        break;
      case 'v':
        verbose_ = true;
        break;
      case 'i':
        input_file_ = optarg;
        break;
      case 'o':
        output_file_ = optarg;
        break;
      case 's':
        grid_spacing_ = atof(optarg);
        break;
      case 'e':
        vertical_exaggeration_ = atof(optarg);
        break;
      case 'd':
        decimation_level_ = atoi(optarg);
        break;
      case 'c':
        use_draco_ = true;
        break;
      case 'm':
        max_triangles_ = atoi(optarg);
        break;
      case 't':
        edge_threshold_ = atof(optarg);
        break;
      default:
        print_usage();
        exit(1);
    }
  }

  // Validate required arguments
  if (!help_ && (input_file_.empty() || output_file_.empty())) {
    LOG_ERROR("Input and output files are required");
    print_usage();
    exit(1);
  }
}

void MeshOptions::print_usage() {
  std::cout << "\nMB-Mesh: Generate 3D GLTF meshes from bathymetry data\n\n";
  std::cout << "Usage: mb-mesh [options]\n\n";
  std::cout << "Required options:\n";
  std::cout << "  -i, --input <file>          Input bathymetry data file\n";
  std::cout << "  -o, --output <file>         Output GLTF file\n\n";
  std::cout << "Optional parameters:\n";
  std::cout << "  -h, --help                  Show this help message\n";
  std::cout << "  -v, --verbose               Enable verbose output\n";
  std::cout << "  -s, --spacing <value>       Grid spacing in meters (default: 1.0)\n";
  std::cout << "  -e, --exaggeration <value>  Vertical exaggeration (default: 1.0)\n";
  std::cout << "  -d, --decimation <level>    Mesh decimation level 0-10 (default: 0)\n";
  std::cout << "  -c, --draco                 Enable Draco compression\n";
  std::cout << "  -m, --max-triangles <n>     Maximum triangles (default: 1000000)\n";
  std::cout << "  -t, --edge-threshold <val>  Edge collapse threshold (default: 0.001)\n\n";
  std::cout << "Example:\n";
  std::cout << "  mb-mesh -i bathymetry.txt -o output.gltf -s 2.0 -e 3.0 -v\n\n";
}

}  // namespace mbmesh
