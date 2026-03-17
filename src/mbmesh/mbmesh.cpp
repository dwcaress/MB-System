/*--------------------------------------------------------------------
 *    The MB-system:  mbmesh.cc  3/6/2026
 *
 *    Copyright (c) 2026 by
 *    [Your Team/Institution]
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

/*--------------------------------------------------------------------
 * PHASE 1 SCOPE:
 * This initial implementation focuses ONLY on:
 * 1. Reading datalist files
 * 2. Opening swath files with MB-System API
 * 3. Extracting bathymetry soundings
 * 4. Storing soundings in memory
 * 5. Printing statistics
 *
 * NOT INCLUDED YET:
 * - Spatial indexing (quadtree/octree)
 * - Mesh generation
 * - 3D Tiles output
 * - ECEF coordinate conversion
 *--------------------------------------------------------------------*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>
#include <vector>
#include <algorithm>


// MB-System includes
extern "C" {
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
}

constexpr char program_name[] = "mbmesh";
constexpr char help_message[] =
    "mbmesh generates 3D Tiles from swath sonar bathymetry data.\n"
    "This tool reads swath data files and produces OGC 3D Tiles format\n"
    "output suitable for web-based 3D visualization.\n\n"
    "Phase 1 implementation reads and validates swath data input.";

constexpr char usage_message[] =
    "mbmesh -Idatalist [-Rwest/east/south/north] [-Ooutdir]\n"
    "       [-V -H]";

/*--------------------------------------------------------------------*/
/* SOUNDING STRUCTURE */
/*--------------------------------------------------------------------*/

/**
 * @brief A single sonar sounding (one beam return)
 *
 * This structure represents one bathymetry measurement from the
 * multibeam sonar. For Phase 1, we just store the basic information
 * needed to validate the data input pipeline.
 */
struct Sounding {
  double longitude;   // Degrees east
  double latitude;    // Degrees north
  double depth;       // Meters (negative = below sea level)
  char beamflag;      // MB-System beam quality flag
  int beam_number;    // Beam index within ping
  double time_d;      // Unix timestamp (seconds since epoch)

  // TODO Phase 2: Add ECEF coordinates (x, y, z)
  // TODO Phase 2: Add amplitude, acrosstrack distance, etc.
};

/*--------------------------------------------------------------------*/
/* GLOBAL VARIABLES */
/*--------------------------------------------------------------------*/

// Command-line options
static int verbose = 0;
static char read_datalist[MB_PATH_MAXLINE] = "datalist.mb-1";
static char output_dir[MB_PATH_MAXLINE] = "./tileset";
static bool bounds_specified = false;
static double bounds[4] = {-360.0, 360.0, -90.0, 90.0};  // west, east, south, north

// Statistics
static int nfile = 0;               // Number of files in datalist
static int nfile_read = 0;          // Number successfully read
static int npings = 0;              // Total pings processed
static int nbeams_total = 0;        // Total beams encountered
static int nbeams_good = 0;         // Valid beams
static int nbeams_flagged = 0;      // Flagged/rejected beams

// Storage for soundings (in-memory for Phase 1)
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
static void print_statistics();

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
    fprintf(stderr, "  Verbose level: %d\n", verbose);
  }

  /* Read swath data from datalist */
  fprintf(stderr, "\n=== Phase 1: Reading Swath Data ===\n");
  int status = read_datalist_file(verbose);

  if (status != MB_SUCCESS) {
    fprintf(stderr, "\nError reading datalist\n");
    fprintf(stderr, "Program <%s> Terminated\n", program_name);
    exit(status);
  }

  /* Print statistics */
  fprintf(stderr, "\nSwath data reading complete\n");
  print_statistics();

  /* TODO Phase 2: Build spatial index from all_soundings */
  /* TODO Phase 3: Generate meshes */
  /* TODO Phase 4: Write 3D Tiles output */

  fprintf(stderr, "\n=== Phase 1 Complete ===\n");
  fprintf(stderr, "Soundings collected: %zu\n", all_soundings.size());
  fprintf(stderr, "Ready for Phase 2 (spatial indexing)\n");
  fprintf(stderr, "\nProgram <%s> completed successfully\n", program_name);
  exit(MB_SUCCESS);
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
      {nullptr, 0, nullptr, 0}};

  /* Process command line options */
  while ((c = getopt_long(argc, argv, "I:O:R:VvHh", long_options, &option_index)) != -1) {
    switch (c) {
    case 0:
      /* Handle long options */
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
  fprintf(stderr, "  -V                 Increase verbosity (can repeat: -V -V)\n");
  fprintf(stderr, "  -H                 Print this help message\n");
  fprintf(stderr, "\nPhase 1 Implementation:\n");
  fprintf(stderr, "  This version reads swath data and validates input only.\n");
  fprintf(stderr, "  Spatial indexing and 3D Tiles output coming in Phase 2.\n");
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
  if (verbose > 0) {
    fprintf(stderr, "  Opening file (format %d)...\n", format);
  }

  /* MB-System I/O variables */
  void *mbio_ptr = nullptr;
  int error = MB_ERROR_NO_ERROR;

  /* Data arrays (will be allocated after mb_read_init) */
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathlon = nullptr;
  double *bathlat = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *sslon = nullptr;
  double *sslat = nullptr;
  char comment[MB_COMMENT_MAXLINE] = "";
  int kind, time_i[7];
  double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;

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

  // Return early for stub
  // return MB_SUCCESS;

  /* TODO #3: Allocate data arrays
   *
   * After mb_read_init() succeeds, allocate arrays to hold ping data.
   * Use mb_mallocd() for MB-System memory tracking.
   *
   * PSEUDOCODE:
   *
   * // Allocate beamflag array
   * status = mb_mallocd(verbose, __FILE__, __LINE__,
   *                     beams_bath * sizeof(char),
   *                     (void **)&beamflag, &error);
   * if (status != MB_SUCCESS) {
   *   fprintf(stderr, "Error allocating beamflag array\n");
   *   return MB_FAILURE;
   * }
   *
   * // Allocate bath array
   * status = mb_mallocd(verbose, __FILE__, __LINE__,
   *                     beams_bath * sizeof(double),
   *                     (void **)&bath, &error);
   * if (status != MB_SUCCESS) {
   *   fprintf(stderr, "Error allocating bath array\n");
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
   *   return MB_FAILURE;
   * }
   *
   * // Repeat for: bathlon, bathlat, amp
   * // (amp uses beams_amp size)
   *
   * // Skip sidescan for Phase 1
   *
   * REFERENCE: See mbgrid.cc lines 2660-2680 for example
   */

  // STUB: Your implementation here


  // status = mb_mallocd(verbose, __FILE__, __LINE__,
  //                     beams_bath * sizeof(char),
  //                     (void **)&beamflag, &error);
  // if (status != MB_SUCCESS) {
  //   fprintf(stderr, "Error allocating beamflag array\n");
  //   return MB_FAILURE;
  // }

  // fprintf(stderr, "TODO #3: Allocate arrays with mb_mallocd()\n");
  // fprintf(stderr, "  Location: mbmesh.cc line %d\n", __LINE__ - 5);
  // fprintf(stderr, "\n");

  /* TODO #3: Allocate + Register Arrays */
  status = mb_mallocd(verbose, __FILE__, __LINE__, 
                      beams_bath * sizeof(char), (void**)&beamflag, &error);
  if (status == MB_SUCCESS)
    mb_register_array(verbose, MB_MEM_TYPE_BATHY, sizeof(char), (void**)&beamflag, &error);
  
  status = mb_mallocd(verbose, __FILE__, __LINE__, 
                      beams_bath * sizeof(double), (void**)&bath, &error);
  if (status == MB_SUCCESS)
    mb_register_array(verbose, MB_MEM_TYPE_BATHY, sizeof(double), (void**)&bath, &error);
  
  status = mb_mallocd(verbose, __FILE__, __LINE__, 
                      beams_bath * sizeof(double), (void**)&bathlon, &error);
  if (status == MB_SUCCESS)
    mb_register_array(verbose, MB_MEM_TYPE_BATHY_POS, sizeof(double), (void**)&bathlon, &error);
  
  status = mb_mallocd(verbose, __FILE__, __LINE__, 
                      beams_bath * sizeof(double), (void**)&bathlat, &error);
  if (status == MB_SUCCESS)
    mb_register_array(verbose, MB_MEM_TYPE_BATHY_POS, sizeof(double), (void**)&bathlat, &error);
  
  status = mb_mallocd(verbose, __FILE__, __LINE__, 
                      beams_amp * sizeof(double), (void**)&amp, &error);
  if (status == MB_SUCCESS)
    mb_register_array(verbose, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void**)&amp, &error);

  if (status != MB_SUCCESS) {
    fprintf(stderr, "  Error allocating arrays\n");
    goto cleanup; 
  }
  if (verbose > 1) fprintf(stderr, "  Arrays allocated OK\n");

  /* TODO #4: Read pings in loop
   *
   * Use mb_get() in a while loop to read ping-by-ping data.
   *
   * PSEUDOCODE:
   *
   * int pings = 0; // Local counter for this file
   *
   * while (true) {
   *   status = mb_get(
   *     verbose, mbio_ptr, &kind, &pings,
   *     time_i, &time_d,
   *     &navlon, &navlat, &speed, &heading,
   *     &distance, &altitude, &sensordepth,
   *     &beams_bath, &beams_amp, &pixels_ss,
   *     beamflag, bath, amp, bathlon, bathlat,
   *     ss, sslon, sslat,  // Pass nullptr for ss arrays if not allocated
   *     comment, &error
   *   );
   *
   *   // Check for end of file or error
   *   if (error > MB_ERROR_NO_ERROR) {
   *     break;
   *   }
   *
   *   // Only process survey data
   *   if (kind == MB_DATA_DATA) {
   *     // Process this ping
   *     process_ping(verbose, beams_bath, beamflag,
   *                 bath, bathlon, bathlat, time_d);
   *
   *     // Update global counter
   *     npings++;
   *     pings++; // Local counter
   *
   *     // Progress report
   *     if (verbose > 1 && pings % 100 == 0) {
   *       fprintf(stderr, "    Processed %d pings...\r", pings);
   *     }
   *   }
   * }
   *
   * if (verbose > 0) {
   *   fprintf(stderr, "  File complete: %d pings processed\n", pings);
   * }
   *
   * REFERENCE: See mbgrid.cc lines 2700-2750 for example
   */

  // STUB: Your implementation here
  fprintf(stderr, "TODO #4: Implement mb_get() reading loop\n");
  fprintf(stderr, "  Location: mbmesh.cc line %d\n", __LINE__ - 5);
  fprintf(stderr, "\n");

  /* TODO #5: Cleanup and close file
   *
   * Free allocated arrays and close MB-System I/O.
   *
   * PSEUDOCODE:
   *
   * // Free arrays in reverse order of allocation
   * if (amp != nullptr)
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
   * if (bathlat != nullptr)
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlat, &error);
   * if (bathlon != nullptr)
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlon, &error);
   * if (bath != nullptr)
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
   * if (beamflag != nullptr)
   *   mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
   *
   * // Close MB-System I/O
   * status = mb_close(verbose, &mbio_ptr, &error);
   * if (status != MB_SUCCESS) {
   *   fprintf(stderr, "  Warning: Error closing file\n");
   * }
   *
   * REFERENCE: See mbgrid.cc lines 2780-2800 for example
   */

  // STUB: Your implementation here
  fprintf(stderr, "TODO #5: Free arrays and call mb_close()\n");
  fprintf(stderr, "  Location: mbmesh.cc line %d\n", __LINE__ - 5);
  fprintf(stderr, "\n");

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
 * @param bathlon Longitude array [beams_bath] (degrees)
 * @param bathlat Latitude array [beams_bath] (degrees)
 * @param time_d Timestamp (Unix seconds)
 * @return MB_SUCCESS
 */
static int process_ping(int verbose, int beams_bath, char *beamflag,
                       double *bath, double *bathlon, double *bathlat,
                       double time_d) {

  /* TODO #6: Process each beam in the ping
   *
   * Loop through all beams and extract valid soundings.
   *
   * PSEUDOCODE:
   *
   * for (int i = 0; i < beams_bath; i++) {
   *   // Count total beams
   *   nbeams_total++;
   *
   *   // Check beam quality
   *   if (!mb_beam_ok(beamflag[i])) {
   *     nbeams_flagged++;
   *     continue;  // Skip bad beam
   *   }
   *
   *   // Create sounding
   *   Sounding s;
   *   s.longitude = bathlon[i];
   *   s.latitude = bathlat[i];
   *   s.depth = bath[i];
   *   s.beamflag = beamflag[i];
   *   s.beam_number = i;
   *   s.time_d = time_d;
   *
   *   // Filter by geographic bounds if specified
   *   if (bounds_specified) {
   *     if (s.longitude < bounds[0] || s.longitude > bounds[1] ||
   *         s.latitude < bounds[2] || s.latitude > bounds[3]) {
   *       continue;  // Outside bounds, skip
   *     }
   *   }
   *
   *   // Add to collection
   *   all_soundings.push_back(s);
   *   nbeams_good++;
   * }
   *
   * REFERENCE: See mbgrid.cc lines 2900-2950 for example
   */

  // STUB: Your implementation here
  for (int i = 0; i < beams_bath; i++) {
    nbeams_total++;
    // TODO: Implement beam processing
  }

  if (verbose > 2 && nbeams_total % 10000 == 0) {
    fprintf(stderr, "TODO #6: Implement beam processing\n");
    fprintf(stderr, "  Location: mbmesh.cc line %d\n", __LINE__ - 10);
    fprintf(stderr, "  Beams processed so far: %d\n", nbeams_total);
  }

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