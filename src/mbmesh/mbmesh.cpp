/*--------------------------------------------------------------------
 *    The MB-system:  mbmesh.cc  3/6/2026
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
 * @file mbmesh.cc
 * @brief Generate 3D Tiles from swath bathymetry data
 *
 * mbmesh reads swath sonar data files and generates OGC 3D Tiles
 * for visualization of bathymetric data with full 3D structure.
 * This preserves features like cliffs, overhangs, and caves that
 * are lost in traditional 2D gridding.
 *
 * Author:  CSUMB Capstone - Spring 2026
 * Date:    March 6, 2026
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <algorithm>


// Point cloud GLB export
#include "pointcloud_glb_writer.h"

// MB-System includes
extern "C" {
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
}

constexpr char program_name[] = "mbmesh";
constexpr char help_message[] =
    "mbmesh generates 3D point clouds from swath sonar bathymetry data.\n"
    "This tool reads swath data files and produces multiple output formats\n"
    "suitable for 3D visualization and analysis of seafloor bathymetry.";

constexpr char usage_message[] =
  "mbmesh -Idatalist [-Rwest/east/south/north] [-Ooutdir] [-html]\n"
    "       [-V -H]";

/*--------------------------------------------------------------------*/
/* SOUNDING STRUCTURE */
/*--------------------------------------------------------------------*/

/**
 * @brief A single sonar sounding (one beam return)
 *
 * This structure represents one bathymetry measurement from the
 * multibeam sonar, storing coordinates and metadata for later processing
 * and visualization.
 */
struct Sounding {
  double longitude;   // Degrees east
  double latitude;    // Degrees north
  double depth;       // Meters (negative = below sea level)
  char beamflag;      // MB-System beam quality flag
  int beam_number;    // Beam index within ping
  double time_d;      // Unix timestamp (seconds since epoch)
};

/*--------------------------------------------------------------------*/
/* GLOBAL VARIABLES */
/*--------------------------------------------------------------------*/

// Command-line options
static int verbose = 0;
static char read_datalist[MB_PATH_MAXLINE] = "datalist.mb-1";
static char output_dir[MB_PATH_MAXLINE] = "./tileset";
static bool bounds_specified = false;
static double bounds[4] = {-180.0, 180.0, -90.0, 90.0};  // west, east, south, north
static bool html_output = false;

// Statistics
static int nfile = 0;               // Number of files in datalist
static int nfile_read = 0;          // Number successfully read
static int npings = 0;              // Total pings processed
static int nbeams_total = 0;        // Total beams encountered
static int nbeams_good = 0;         // Valid beams
static int nbeams_flagged = 0;      // Flagged/rejected beams

// Storage for soundings (in-memory collection of all valid beams)
static std::vector<Sounding> all_soundings;

/*--------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES */
/*--------------------------------------------------------------------*/

static void print_help();
static int parse_options(int argc, char **argv);
static int read_datalist_file(int verbose);
static int read_swath_file(int verbose, char *file, int format, double file_weight);
static int process_ping(int verbose, int beams_bath, char *beamflag,
                       double *bath, double *bathlon, double *bathlat,
                       double time_d);
static int write_xyz_file(const char *filename);
static int write_projected_xyz_file(const char *filename);
static int write_html_file(const char *filename, const char *glb_filename);
static int launch_html_viewer_server(const char *directory, const char *html_filename);
static int ensure_directory_exists(const char *path);
static void print_statistics();
static int write_ecef_xyz_file(const char *filename);
static void geodetic_to_ecef(double lon_deg, double lat_deg, double height, double *x, double *y, double *z);

/*--------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  fprintf(stderr, "\nProgram %s\n", program_name);
  fprintf(stderr, "MB-system Version %s\n", MB_VERSION);

  /* Parse command-line options */
  if (parse_options(argc, argv) != MB_SUCCESS) {
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  /* Print starting info */
  if (verbose > 0) {
    fprintf(stderr, "\nmbmesh settings:\n");
    fprintf(stderr, "  Input datalist: %s\n", read_datalist);
    fprintf(stderr, "  Output directory: %s\n", output_dir);
    if (bounds_specified) {
      fprintf(stderr, "  Geographic bounds: %.6f/%.6f/%.6f/%.6f\n",
              bounds[0], bounds[1], bounds[2], bounds[3]);
    } else {
      fprintf(stderr, "  Geographic bounds: [unbounded]\n");
    }
    fprintf(stderr, "  HTML output: %s\n", html_output ? "enabled" : "disabled");
    fprintf(stderr, "  Verbose level: %d\n", verbose);
  }

  /* Read swath data from datalist */
  fprintf(stderr, "\n=== Reading Swath Data ===\n");
  int status = read_datalist_file(verbose);

  if (status != MB_SUCCESS) {
    fprintf(stderr, "\nError reading datalist\n");
    fprintf(stderr, "Program <%s> Terminated\n", program_name);
    exit(status);
  }

  /* Print statistics */
  fprintf(stderr, "\nSwath data reading complete\n");
  print_statistics();

  if (ensure_directory_exists(output_dir) != MB_SUCCESS) {
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_FAILURE);
  }

  /* Write projected XYZ point cloud (local meters) */
  char projected_file[MB_PATH_MAXLINE];
  snprintf(projected_file, sizeof(projected_file), "%s/adjustedPointcloud.xyz", output_dir);
  write_projected_xyz_file(projected_file);

  /* Write ECEF XYZ point cloud (WGS84) */
  char ecef_file[MB_PATH_MAXLINE];
  snprintf(ecef_file, sizeof(ecef_file), "%s/ecefPointcloud.xyz", output_dir);
  write_ecef_xyz_file(ecef_file);

  fprintf(stderr, "\nProcessing complete\n");
  fprintf(stderr, "Soundings collected: %zu\n", all_soundings.size());
  fprintf(stderr, "Adjusted XYZ file written: %s\n", projected_file);

  /* Write a GLB point cloud from the adjusted XYZ file without modifying it */
  char glb_file[MB_PATH_MAXLINE];
  snprintf(glb_file, sizeof(glb_file), "%s/adjustedPointcloud.glb", output_dir);
  if (write_pointcloud_glb_file(projected_file, glb_file, verbose) == 0) {
    fprintf(stderr, "GLB point cloud file written: %s\n", glb_file);
  } else {
    fprintf(stderr, "Failed to write GLB point cloud file: %s\n", glb_file);
  }

  if (html_output) {
    char html_file[MB_PATH_MAXLINE];
    snprintf(html_file, sizeof(html_file), "%s/adjustedPointcloud.html", output_dir);
    if (write_html_file(html_file, "adjustedPointcloud.glb") == MB_SUCCESS) {
      fprintf(stderr, "HTML viewer file written: %s\n", html_file);
      if (launch_html_viewer_server(output_dir, "adjustedPointcloud.html") != MB_SUCCESS) {
        fprintf(stderr, "Warning: Failed to auto-launch Python web server/viewer.\n");
      }
    } else {
      fprintf(stderr, "Failed to write HTML viewer file: %s\n", html_file);
    }
  }

  fprintf(stderr, "\nProgram <%s> completed successfully\n", program_name);
  exit(MB_SUCCESS);
}

static int ensure_directory_exists(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      return MB_SUCCESS;
    }

    fprintf(stderr, "Error: Output path exists but is not a directory: %s\n", path);
    return MB_FAILURE;
  }

  if (mkdir(path, 0755) == 0) {
    if (verbose > 0) {
      fprintf(stderr, "Created output directory: %s\n", path);
    }
    return MB_SUCCESS;
  }

  fprintf(stderr, "Error: Cannot create output directory %s\n", path);
  return MB_FAILURE;
}

/*--------------------------------------------------------------------*/
/* GEODETIC TO ECEF CONVERSION */
/*--------------------------------------------------------------------*/
static void geodetic_to_ecef(double lon_deg, double lat_deg, double height, double *x, double *y, double *z) {
  // WGS84 ellipsoid constants
  constexpr double a = 6378137.0;         // semi-major axis (meters)
  constexpr double f = 1.0 / 298.257223563; // flattening
  constexpr double e2 = f * (2 - f);      // eccentricity squared

  double lon = lon_deg * M_PI / 180.0;
  double lat = lat_deg * M_PI / 180.0;
  double sin_lat = sin(lat);
  double cos_lat = cos(lat);
  double sin_lon = sin(lon);
  double cos_lon = cos(lon);
  double N = a / sqrt(1 - e2 * sin_lat * sin_lat);

  *x = (N + height) * cos_lat * cos_lon;
  *y = (N + height) * cos_lat * sin_lon;
  *z = (N * (1 - e2) + height) * sin_lat;
}

/*--------------------------------------------------------------------*/
/* CREATE ECEF .XYZ FILE (WGS84) */
/*--------------------------------------------------------------------*/
static int write_ecef_xyz_file(const char *filename) {
  if (all_soundings.empty()) {
    fprintf(stderr, "Warning: No soundings to write ECEF XYZ file\n");
    return MB_FAILURE;
  }

  /* TODO: FIXME - GeoOrigin offset loses precision detail. Consider:
     1. Making GeoOrigin an optional command-line parameter (--geo-origin)
     2. Writing global ECEF coordinates directly (no offset subtraction)
     3. Or increasing output precision to compensate for subtraction artifacts */

  /* Compute centroid (GeoOrigin, same as projected) */
  double sum_lon = 0.0, sum_lat = 0.0, sum_depth = 0.0;
  for (const auto &s : all_soundings) {
    sum_lon += s.longitude;
    sum_lat += s.latitude;
    sum_depth += s.depth;
  }
  double ref_lon = sum_lon / all_soundings.size();
  double ref_lat = sum_lat / all_soundings.size();
  double ref_depth = sum_depth / all_soundings.size();

  // Compute GeoOrigin ECEF offset
  double x0, y0, z0;
  geodetic_to_ecef(ref_lon, ref_lat, -ref_depth, &x0, &y0, &z0);

  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: Cannot create ECEF XYZ file: %s\n", filename);
    return MB_FAILURE;
  }

  // User-facing output, matching other point cloud writers
  fprintf(stderr, "\nWriting ECEF XYZ point cloud: %s\n", filename);
  fprintf(stderr, "  Points: %zu\n", all_soundings.size());

  /* Write header */
  fprintf(fp, "# X(m) Y(m) Z(m) - ECEF coordinates (WGS84, GeoOrigin offset)\n");
  fprintf(fp, "# Reference (GeoOrigin): lon=%.8f lat=%.8f depth=%.3f\n",
          ref_lon, ref_lat, ref_depth);
  fprintf(fp, "# Offset: x0=%.3f y0=%.3f z0=%.3f\n", x0, y0, z0);

  /* Write ECEF points with GeoOrigin offset */
  for (const auto &s : all_soundings) {
    double x, y, z;
    // Note: depth is positive down, so height = -depth
    geodetic_to_ecef(s.longitude, s.latitude, -s.depth, &x, &y, &z);
    fprintf(fp, "%.3f %.3f %.3f\n", x - x0, y - y0, z - z0);
  }

  fclose(fp);
  fprintf(stderr, "  ECEF XYZ file written successfully\n");
  fprintf(stderr, "  Location: %s\n", filename);
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* PARSE COMMAND-LINE OPTIONS */
/*--------------------------------------------------------------------*/

static int parse_options(int argc, char **argv) {
  int option_index;
  int errflg = 0;
  int c;
  bool help = false;

  static struct option long_options[] = {
      {"verbose", no_argument, nullptr, 0},
      {"help", no_argument, nullptr, 0},
      {"input", required_argument, nullptr, 0},
      {"html", no_argument, nullptr, 0},
      {nullptr, 0, nullptr, 0}};

  // Support legacy single-dash long form requested by users: -html
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-html") == 0) {
      argv[i] = (char *)"--html";
    }
  }

  /* Process command line options */
  while ((c = getopt_long(argc, argv, "I:O:R:VvHh", long_options, &option_index)) != -1) {
    switch (c) {
    case 0:
      /* Handle long options */
      if (strcmp(long_options[option_index].name, "html") == 0) {
        html_output = true;
      }
      break;

    case 'I':
      sscanf(optarg, "%s", read_datalist);
      break;

    case 'O':
      sscanf(optarg, "%s", output_dir);
      break;

    case 'R':
      /* Parse bounds: west/east/south/north */
      {
        int n = sscanf(optarg, "%lf/%lf/%lf/%lf",
                      &bounds[0], &bounds[1], &bounds[2], &bounds[3]);
        if (n == 4) {
          bounds_specified = true;
        } else {
          fprintf(stderr, "Error parsing -R option: %s\n", optarg);
          fprintf(stderr, "Expected format: -Rwest/east/south/north\n");
          errflg++;
        }
      }
      break;

    case 'V':
    case 'v':
      verbose++;
      break;

    case 'H':
    case 'h':
      help = true;
      break;

    case '?':
      errflg++;
      break;
    }
  }

  if (errflg || help) {
    print_help();
    return MB_FAILURE;
  }

  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* PRINT HELP MESSAGE */
/*--------------------------------------------------------------------*/

static void print_help() {
  fprintf(stderr, "\n%s\n", help_message);
  fprintf(stderr, "\nusage: %s\n", usage_message);
  fprintf(stderr, "\nRequired:\n");
  fprintf(stderr, "  -I<datalist>       Input datalist file [datalist.mb-1]\n");
  fprintf(stderr, "\nOptional:\n");
  fprintf(stderr, "  -O<outputdir>      Output directory [./tileset]\n");
  fprintf(stderr, "  -R<w/e/s/n>        Geographic bounds (degrees)\n");
  fprintf(stderr, "  -html              Also write adjustedPointcloud.html viewer file\n");
  fprintf(stderr, "  -V                 Increase verbosity (can repeat: -V -V)\n");
  fprintf(stderr, "  -H                 Print this help message\n");
  fprintf(stderr, "\nExample:\n");
  fprintf(stderr, "  mbmesh -Idatalist.mb-1 -R-122.5/-121.8/36.5/37.2 -V\n\n");
}

/*--------------------------------------------------------------------*/
/* READ DATALIST FILE */
/*--------------------------------------------------------------------*/

/**
 * @brief Read datalist and process all swath files
 *
 * This function follows the pattern from mbgrid.cc lines 1800-2100.
 * It opens the datalist, iterates through each file entry, and
 * calls read_swath_file() for each valid swath file.
 *
 * @param verbose Verbosity level
 * @return MB_SUCCESS or error code
 */
static int read_datalist_file(int verbose) {
  void *datalist = nullptr;
  int look_processed = MB_DATALIST_LOOK_UNSET;
  int error = MB_ERROR_NO_ERROR;

  /* Open datalist */
  int status = mb_datalist_open(verbose, &datalist, read_datalist,
                                look_processed, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "\nUnable to open datalist file: %s\n", read_datalist);
    fprintf(stderr, "Error: %d\n", error);
    return status;
  }

  if (verbose > 0) {
    fprintf(stderr, "Datalist opened: %s\n", read_datalist);
  }

  
  /* Variables for mb_datalist_read3() */
  int pstatus = MB_PROCESSED_NONE; // Indicates whether to use raw or processed file
  int astatus = MB_ALTNAV_NONE; // Indicates whether alternative navigation is available
  char path[MB_PATH_MAXLINE] = ""; // Raw file path
  char ppath[MB_PATH_MAXLINE] = ""; // Processed file path (if available)
  char apath[MB_PATH_MAXLINE] = ""; // Alternative navigation file path (if available)
  char dpath[MB_PATH_MAXLINE] = ""; // Optional data file path (not used in this project)
  int format = 0; // MB-System format code
  double file_weight = 1.0; // Weight for this file (usually 1.0)

  // int mb_datalist_read3(int verbose, void *datalist,
  //                       int *pstatus, char *path, char *ppath,
  //                       int *astatus, char *apath, char *dpath,
  //                       int *format, double *file_weight, int *error);

  while (mb_datalist_read3(verbose, datalist,
                           &pstatus, path, ppath,
                           &astatus, apath, dpath,
                           &format, &file_weight, &error) == MB_SUCCESS) {
    // Skip non-swath files (format <= 0)
    if (format <= 0) continue;

    // Skip comment lines
    if (path[0] == '#') continue;

    // Choose raw or processed file
    char *file_to_read = (pstatus == MB_PROCESSED_USE) ? ppath : path;

    // Count files
    nfile++;

    // Read this file
    if (verbose > 0) {
      fprintf(stderr, "\nProcessing file %d: %s\n", nfile, file_to_read);
    }

    status = read_swath_file(verbose, file_to_read, format, file_weight);

    if (status != MB_SUCCESS) {
      fprintf(stderr, "Warning: Failed to read file: %s\n", file_to_read);
      // Continue with next file
    }
  }

  /* Close datalist */
  mb_datalist_close(verbose, &datalist, &error);

  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* READ SINGLE SWATH FILE */
/*--------------------------------------------------------------------*/

/**
 * @brief Read bathymetry data from a single swath file
 *
 * This function follows the pattern from mbgrid.cc lines 2600-2800.
 * It initializes MB-System I/O, reads pings in a loop, and extracts
 * bathymetry soundings from each ping.
 *
 * @param verbose Verbosity level
 * @param file Path to swath file
 * @param format MB-System format code
 * @param file_weight Weight for this file (usually 1.0)
 * @return MB_SUCCESS or error code
 */
static int read_swath_file(int verbose, char *file, int format,
                          double file_weight) {
  (void)file_weight;
  if (verbose > 0) {
    fprintf(stderr, "  Opening file (format %d)...\n", format);
  }

  /* MB-System I/O variables */
  void *mbio_ptr = nullptr;
  void *store_ptr = nullptr;
  int error = MB_ERROR_NO_ERROR;

  /* Data arrays (will be allocated after mb_read_init) */
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE] = "";
  //int kind, time_i[7];
  //double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;

  /* Ping data variables */
  int kind;
  int time_i[7];
  double time_d;
  double navlon, navlat;
  double speed, heading;
  double distance, altitude, sensordepth;
  int beams_bath, beams_amp, pixels_ss;

  // Initializing MB-System I/O and reading pings will be implemented in the next steps.
  //REFERENCE: See mbgrid.cc lines 2630-2650 for example

  // Time limits: full range = no filtering,]
  // full range of valid times referenced from mb_time.cc lines 72-77
  int btime_i[7] = {1930, 1, 1, 0, 0, 0, 0};
  int etime_i[7] = {3000, 1, 1, 0, 0, 0, 0};
  double btime_d, etime_d;

  int status = mb_read_init(verbose, file, format, 1, 0, bounds,
                            btime_i, etime_i, 0.0, 1.0,
                            &mbio_ptr, &btime_d, &etime_d,
                            &beams_bath, &beams_amp, &pixels_ss,
                            &error);

  // if mb_read_init fails, print error message and return failure
  if (status != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, error, &message);
    fprintf(stderr, "  Error initializing file: %s\n", message);
    return MB_FAILURE;
  }
  // if mb_read_init succeeds, print number of beams and pixels
  if (verbose > 0) {
    fprintf(stderr, "  File opened: %d beams, %d amp, %d ss\n",
            beams_bath, beams_amp, pixels_ss);
  }

  // Register arrays with MB-System I/O system
  // This is CRITICAL - arrays must be registered before mb_read/mb_get_all calls
  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                            (void **)&beamflag, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering beamflag array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                            (void **)&bath, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering bath array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                            (void **)&bathacrosstrack, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering bathacrosstrack array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                            (void **)&bathalongtrack, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering bathalongtrack array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
                            (void **)&amp, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering amp array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                            (void **)&ss, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering ss array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                            (void **)&ssacrosstrack, &error);
  if (status !=  MB_SUCCESS) {
    fprintf(stderr, "Error registering ssacrosstrack array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                            (void **)&ssalongtrack, &error);
  if (status != MB_SUCCESS) {
    fprintf(stderr, "Error registering ssalongtrack array\n");
    mb_close(verbose, &mbio_ptr, &error);
    return MB_FAILURE;
  }

  // For now, don't pre-allocate arrays - let mb_get_all handle it
  // Just initialize pointers to NULL

/* Read pings in loop */
  int pings = 0; // Local counter for this file
  int total_records = 0;
  int data_records = 0;

  fprintf(stderr, "  [DEBUG] About to start mb_read loop, mbio_ptr=%p\n", (void*)mbio_ptr);
  while ((status = mb_read(
      verbose, mbio_ptr, &kind, &pings,
      time_i, &time_d,
      &navlon, &navlat, &speed, &heading,
      &distance, &altitude, &sensordepth,
      &beams_bath, &beams_amp, &pixels_ss,
      beamflag, bath, amp, bathacrosstrack, bathalongtrack,
      ss, ssacrosstrack, ssalongtrack,
      comment, &error)) == MB_SUCCESS) {
    
    total_records++;
    
    /* Only process survey data */
    if (kind == MB_DATA_DATA) {
      data_records++;
      /* Process this ping */
      process_ping(verbose, beams_bath, beamflag,
                  bath, bathacrosstrack, bathalongtrack, time_d);

      /* Update global counter */
      npings++;
      pings++;

      /* Progress report */
      if (verbose > 1 && pings % 100 == 0) {
        fprintf(stderr, "    Processed %d pings...\r", pings);
        fflush(stderr);
      }
    }
  }

  fprintf(stderr, "  [DEBUG] mb_read loop exited: status=%d\n", status);

  if (verbose > 0) {
    fprintf(stderr, "  File complete: %d pings processed\n", pings);
  }

  // Cleanup and close file
  if (ssalongtrack != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ssalongtrack, &error);
  if (ssacrosstrack != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ssacrosstrack, &error);
  if (ss != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, &error);
  if (bathalongtrack != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bathalongtrack, &error);
  if (bathacrosstrack != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bathacrosstrack, &error);
  if (bath != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
  if (amp != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
  if (beamflag != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);

  // Reset arrays to NULL for next file
  ssalongtrack = nullptr;
  ssacrosstrack = nullptr;
  ss = nullptr;
  bathalongtrack = nullptr;
  bathacrosstrack = nullptr;
  bath = nullptr;
  amp = nullptr;
  beamflag = nullptr;

  status = mb_close(verbose, &mbio_ptr, &error);
  if (status != MB_SUCCESS && verbose > 0) {
    fprintf(stderr, "  Warning: Error closing file\n");
  }

  nfile_read++;
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* PROCESS SINGLE PING */
/*--------------------------------------------------------------------*/

/**
 * @brief Process bathymetry beams from one ping
 *
 * This function extracts valid soundings from a ping and stores
 * them in the global all_soundings vector for later processing.
 *
 * @param verbose Verbosity level
 * @param beams_bath Number of bathymetry beams in ping
 * @param beamflag Quality flag array [beams_bath]
 * @param bath Depth array [beams_bath] (meters)
 * @param bathacrosstrack Across-track distance array [beams_bath] (meters)
 * @param bathalongtrack Along-track distance array [beams_bath] (meters)
 * @param time_d Timestamp (Unix seconds)
 * @return MB_SUCCESS
 */
static int process_ping(int verbose, int beams_bath, char *beamflag,
                       double *bath, double *bathacrosstrack, double *bathalongtrack,
                       double time_d) {

  // Process each beam in the ping
   
   // Loop through all beams and extract valid soundings.
   // Calculate longitude and latitude from acrosstrack and alongtrack distances.
   
    for (int i = 0; i < beams_bath; i++) {
      // Count total beams
      nbeams_total++;
   
      // Check beam quality
      if (!mb_beam_ok(beamflag[i])) {
        nbeams_flagged++;
        continue;  // Skip bad beam
      }
   
      // Create sounding
      Sounding s;
      s.longitude = bathacrosstrack[i];
      s.latitude = bathalongtrack[i];
      s.depth = bath[i];
      s.beamflag = beamflag[i];
      s.beam_number = i;
      s.time_d = time_d;
   
      // Filter by geographic bounds if specified
      if (bounds_specified) {
        if (s.longitude < bounds[0] || s.longitude > bounds[1] ||
            s.latitude < bounds[2] || s.latitude > bounds[3]) {
          continue;  // Outside bounds, skip
        }
      }
   
      // Add to collection
      all_soundings.push_back(s);
      nbeams_good++;
    }
   
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* CREATE .XYZ FILE */
/*--------------------------------------------------------------------*/

/**
 * @brief Write soundings to XYZ point cloud file
 * @param filename Output file path
 * @return MB_SUCCESS or error code
 */
static int write_xyz_file(const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: Cannot create XYZ file: %s\n", filename);
    return MB_FAILURE;
  }

  fprintf(stderr, "\nWriting XYZ point cloud: %s\n", filename);
  fprintf(stderr, "  Points: %zu\n", all_soundings.size());

  // Write header (optional)
  fprintf(fp, "# X(lon) Y(lat) Z(depth)\n");

  // Write points
  for (const auto &s : all_soundings) {
    fprintf(fp, "%.8f %.8f %.3f\n", s.longitude, s.latitude, s.depth);
  }

  fclose(fp);
  fprintf(stderr, "  XYZ file written successfully\n");
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* CREATE PROJECTED .XYZ FILE (LOCAL METERS) */
/*--------------------------------------------------------------------*/

/**
 * @brief Write soundings to XYZ file in local projected coordinates (meters)
 *
 * Converts lon/lat to local meters relative to the data centroid and
 * centers depth around the mean. This makes the file viewable in
 * generic 3D point cloud viewers without axis scale mismatch.
 *
 * @param filename Output file path
 * @return MB_SUCCESS or error code
 */
static int write_projected_xyz_file(const char *filename) {
  if (all_soundings.empty()) {
    fprintf(stderr, "Warning: No soundings to write projected XYZ file\n");
    return MB_FAILURE;
  }

  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: Cannot create projected XYZ file: %s\n", filename);
    return MB_FAILURE;
  }

  /* Compute centroid */
  double sum_lon = 0.0, sum_lat = 0.0, sum_depth = 0.0;
  for (const auto &s : all_soundings) {
    sum_lon += s.longitude;
    sum_lat += s.latitude;
    sum_depth += s.depth;
  }
  double ref_lon = sum_lon / all_soundings.size();
  double ref_lat = sum_lat / all_soundings.size();
  double ref_depth = sum_depth / all_soundings.size();

  /* Meters per degree at the reference latitude */
  constexpr double m_per_deg_lat = 111132.0;
  double m_per_deg_lon = 111132.0 * cos(ref_lat * M_PI / 180.0);

  fprintf(stderr, "\nWriting projected XYZ point cloud: %s\n", filename);
  fprintf(stderr, "  Reference point: lon=%.6f lat=%.6f depth=%.1f\n",
          ref_lon, ref_lat, ref_depth);
  fprintf(stderr, "  Scale: 1 deg lon = %.1f m, 1 deg lat = %.1f m\n",
          m_per_deg_lon, m_per_deg_lat);
  fprintf(stderr, "  Points: %zu\n", all_soundings.size());

  /* Write header */
  fprintf(fp, "# X(m) Y(m) Z(m) - local projected coordinates\n");
  fprintf(fp, "# Reference: lon=%.8f lat=%.8f depth=%.3f\n",
          ref_lon, ref_lat, ref_depth);

  /* Write projected points */
  for (const auto &s : all_soundings) {
    double x = (s.longitude - ref_lon) * m_per_deg_lon;
    double y = (s.latitude - ref_lat) * m_per_deg_lat;
    double z = s.depth - ref_depth;
    fprintf(fp, "%.3f %.3f %.3f\n", x, y, z);
  }

  fclose(fp);
  fprintf(stderr, "  Projected XYZ file written successfully\n");
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* CREATE HTML VIEWER FILE */
/*--------------------------------------------------------------------*/

static int write_html_file(const char *filename, const char *glb_filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: Cannot create HTML file: %s\n", filename);
    return MB_FAILURE;
  }

  fprintf(fp, "<html>\n");
  fprintf(fp, "    <head>\n");
  fprintf(fp, "        <title>MB-System Adjusted Point Cloud Viewer</title>\n");
  fprintf(fp, "        <script type='text/javascript' src='http://www.x3dom.org/download/x3dom.js'></script>\n");
  fprintf(fp, "        <link rel='stylesheet' type='text/css' href='http://www.x3dom.org/download/x3dom.css'></link>\n");
  fprintf(fp, "    </head>\n");
  fprintf(fp, "    <body>\n");
  fprintf(fp, "        <h1>MB-System Adjusted Point Cloud Viewer</h1>\n");
  fprintf(fp, "        <p>\n");
  fprintf(fp, "            Viewing adjustedPointcloud.glb exported by mbmesh from swath sonar data.\n");
  fprintf(fp, "        </p>\n");
  fprintf(fp, "        <x3d>\n");
  fprintf(fp, "            <scene>\n");
  fprintf(fp, "                <transform>\n");
  fprintf(fp, "                    <inline url=\"%s\"></inline>\n", glb_filename);
  fprintf(fp, "                </transform>\n");
  fprintf(fp, "            </scene>\n");
  fprintf(fp, "        </x3d>\n");
  fprintf(fp, "    </body>\n");
  fprintf(fp, "</html>\n");

  fclose(fp);
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* LAUNCH HTML VIEWER */
/*--------------------------------------------------------------------*/

static int launch_html_viewer_server(const char *directory, const char *html_filename) {
  if (directory == nullptr || html_filename == nullptr) {
    return MB_FAILURE;
  }

  const int port = 8000;

  char server_command[MB_PATH_MAXLINE * 2];
  snprintf(server_command, sizeof(server_command),
           "python3 -m http.server %d --bind 127.0.0.1 --directory \"%s\" >/tmp/mbmesh_http.log 2>&1 &",
           port, directory);

  int server_status = system(server_command);
  if (server_status != 0) {
    return MB_FAILURE;
  }

  char open_command[MB_PATH_MAXLINE * 2];
  snprintf(open_command, sizeof(open_command),
           "python3 -c \"import webbrowser; webbrowser.open('http://127.0.0.1:%d/%s')\"",
           port, html_filename);

  int open_status = system(open_command);
  if (open_status != 0) {
    return MB_FAILURE;
  }

  fprintf(stderr, "Started Python web server at http://127.0.0.1:%d/%s\n", port, html_filename);
  fprintf(stderr, "Server logs: /tmp/mbmesh_http.log\n");
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* PRINT STATISTICS */
/*--------------------------------------------------------------------*/

/**
 * @brief Print statistics about data reading
 */
static void print_statistics() {
  fprintf(stderr, "\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "    Swath Data Reading Statistics\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "Files in datalist:       %d\n", nfile);
  fprintf(stderr, "Files successfully read: %d\n", nfile_read);
  fprintf(stderr, "Pings processed:         %d\n", npings);
  fprintf(stderr, "Beams total:             %d\n", nbeams_total);
  fprintf(stderr, "Beams good:              %d\n", nbeams_good);
  fprintf(stderr, "Beams flagged:           %d\n", nbeams_flagged);
  fprintf(stderr, "Soundings stored:        %zu\n", all_soundings.size());

  if (nbeams_total > 0) {
    double percent_good = 100.0 * nbeams_good / nbeams_total;
    fprintf(stderr, "Acceptance rate:         %.1f%%\n", percent_good);
  }

  fprintf(stderr, "========================================\n");

  /* Print sample soundings if verbose */
  if (verbose > 1 && !all_soundings.empty()) {
    fprintf(stderr, "\nSample soundings (first 10):\n");
    fprintf(stderr, "  %-13s %-13s %-11s %-6s\n",
            "Longitude", "Latitude", "Depth", "Flag");
    fprintf(stderr, "  %-13s %-13s %-11s %-6s\n",
            "-------------", "-------------", "-----------", "------");

    int nsamples = std::min(10, (int)all_soundings.size());
    for (int i = 0; i < nsamples; i++) {
      const Sounding &s = all_soundings[i];
      fprintf(stderr, "  %13.7f %13.7f %11.2f %6d\n",
              s.longitude, s.latitude, s.depth, (int)s.beamflag);
    }
  }

  /* Compute geographic bounds */
  if (!all_soundings.empty()) {
    double min_lon = 999.0, max_lon = -999.0;
    double min_lat = 999.0, max_lat = -999.0;
    double min_depth = 99999.0, max_depth = -99999.0;

    for (const auto &s : all_soundings) {
      min_lon = std::min(min_lon, s.longitude);
      max_lon = std::max(max_lon, s.longitude);
      min_lat = std::min(min_lat, s.latitude);
      max_lat = std::max(max_lat, s.latitude);
      min_depth = std::min(min_depth, s.depth);
      max_depth = std::max(max_depth, s.depth);
    }

    fprintf(stderr, "\nData bounds:\n");
    fprintf(stderr, "  Longitude: %11.6f to %11.6f\n", min_lon, max_lon);
    fprintf(stderr, "  Latitude:  %11.6f to %11.6f\n", min_lat, max_lat);
    fprintf(stderr, "  Depth:     %11.2f to %11.2f meters\n", min_depth, max_depth);
  }

  fprintf(stderr, "\n");
}
