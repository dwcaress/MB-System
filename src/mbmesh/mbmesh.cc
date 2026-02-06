/*--------------------------------------------------------------------
 *    The MB-system:  mbmesh.cc  2/6/2026
 *
 *    Copyright (c) 2026-2026 by
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
 * @file
 * mbmesh is a utility used to create 3D mesh representations directly from
 * swath sonar data. The program takes input similar to mbgrid but bypasses
 * the 2D gridding operation to enable accurate representation of bathymetric
 * features such as cliffs, spires, overhangs, and caves.
 * 
 * The output format is OGC 3D Tiles 1.1, whose fundamental tile format is
 * .glb (glTF binary). The initial implementation creates a single root tile
 * that can be visualized in web browsers using X3DOM or other glTF viewers.
 *
 * Author:  D. W. Caress and others
 * Date:    February 6, 2026
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>
#include <vector>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

/* output stream for basic stuff */
FILE *outfp = stdout;

/* program identifiers */
constexpr char program_name[] = "mbmesh";
constexpr char help_message[] =
    "mbmesh is a utility used to create 3D mesh representations directly from\n"
    "swath sonar data. The program bypasses 2D gridding to enable accurate\n"
    "representation of bathymetric features such as cliffs, spires, overhangs,\n"
    "and caves. Output is in OGC 3D Tiles 1.1 format (glTF/glb files).";
constexpr char usage_message[] =
    "mbmesh -Ifilelist -Oroot [-Rwest/east/south/north -V -H]";

/*--------------------------------------------------------------------*/

// Structure to hold swath data points for 3D mesh generation
struct SwathPoint {
  double longitude;
  double latitude;
  double depth;
  double time;
  int beam_number;
};

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  /* MBIO status variables */
  int status = MB_SUCCESS;
  int verbose = 0;
  int error = MB_ERROR_NO_ERROR;
  char *message = nullptr;

  /* MBIO read control parameters */
  int pings = 1;
  int lonflip = 0;
  double bounds[4] = {-360.0, 360.0, -90.0, 90.0};
  int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
  int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
  double speedmin = 0.0;
  double timegap = 1000000000.0;

  /* mbmesh control variables */
  char filelist[MB_PATH_MAXLINE] = "";
  char fileroot[MB_PATH_MAXLINE] = "mbmesh";
  char output_file[MB_PATH_MAXLINE] = "";
  bool bounds_set = false;
  bool help = false;
  bool errflg = false;
  
  /* output settings */
  bool binary_output = true; // glb by default
  
  /* statistics */
  int total_pings = 0;
  int total_beams = 0;
  int nfiles = 0;
  
  /* parse the command-line options */
  int option_index;
  int c;
  
  static struct option options[] = {
    {"help", no_argument, nullptr, 0},
    {"input", required_argument, nullptr, 0},
    {"output", required_argument, nullptr, 0},
    {"verbose", no_argument, nullptr, 0},
    {nullptr, 0, nullptr, 0}
  };
  
  while ((c = getopt_long(argc, argv, "HhI:i:O:o:R:r:Vv", options, &option_index)) != -1) {
    switch (c) {
      case 0:
        // long options
        break;
      case 'H':
      case 'h':
        help = true;
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", filelist);
        break;
      case 'O':
      case 'o':
        sscanf(optarg, "%1023s", fileroot);
        break;
      case 'R':
      case 'r':
        mb_get_bounds(optarg, bounds);
        bounds_set = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        if (verbose >= 2)
          outfp = stderr;
        break;
      case '?':
        errflg = true;
    }
  }

  /* if error or help flag set then print usage and exit */
  if (errflg) {
    fprintf(outfp, "usage: %s\n", usage_message);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  if (help) {
    fprintf(outfp, "\n%s\n", help_message);
    fprintf(outfp, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  /* check for required input file */
  if (strlen(filelist) == 0) {
    fprintf(outfp, "\nNo input file specified!\n");
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  /* verbose output */
  if (verbose > 0) {
    fprintf(outfp, "\nProgram <%s>\n", program_name);
    fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
  }

  if (verbose > 0) {
    fprintf(outfp, "\nControl Parameters:\n");
    fprintf(outfp, "  Input file:      %s\n", filelist);
    fprintf(outfp, "  Output root:     %s\n", fileroot);
    fprintf(outfp, "  Verbose:         %d\n", verbose);
    if (bounds_set) {
      fprintf(outfp, "  Bounds:          %f %f %f %f\n", 
              bounds[0], bounds[1], bounds[2], bounds[3]);
    }
  }

  /* set output filename */
  if (binary_output) {
    snprintf(output_file, sizeof(output_file), "%s.glb", fileroot);
  } else {
    snprintf(output_file, sizeof(output_file), "%s.gltf", fileroot);
  }

  if (verbose > 0) {
    fprintf(outfp, "  Output file:     %s\n", output_file);
  }

  /* storage for swath data points */
  std::vector<SwathPoint> swath_points;

  /* initialize datalist reading */
  void *datalist = nullptr;
  int format;
  double file_weight;
  int pstatus;
  int astatus = MB_ALTNAV_NONE;
  char path[MB_PATH_MAXLINE] = "";
  char ppath[MB_PATH_MAXLINE] = "";
  char apath[MB_PATH_MAXLINE] = "";
  char dpath[MB_PATH_MAXLINE] = "";
  char file[MB_PATH_MAXLINE] = "";
  int rformat;
  char rfile[MB_PATH_MAXLINE] = "";

  fprintf(outfp, "\nReading swath data from datalist...\n");

  /* open datalist */
  const int look_processed = MB_DATALIST_LOOK_UNSET;
  if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
    error = MB_ERROR_OPEN_FAIL;
    fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_OPEN_FAIL);
  }

  /* loop over files in datalist */
  while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, 
                           &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
    
    /* skip comment lines and non-swath data */
    if (format <= 0 || path[0] == '#')
      continue;

    /* apply pstatus to get actual file to read */
    if (pstatus == MB_PROCESSED_USE)
      strcpy(file, ppath);
    else
      strcpy(file, path);

    /* check for mbinfo file to get file bounds */
    rformat = format;
    strcpy(rfile, file);
    bool file_in_bounds = true;
    status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
    if (status == MB_FAILURE) {
      file_in_bounds = true;
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }

    /* skip file if out of bounds */
    if (!file_in_bounds) {
      if (verbose > 0)
        fprintf(outfp, "  Skipping out-of-bounds file: %s\n", rfile);
      continue;
    }

    nfiles++;
    if (verbose > 0)
      fprintf(outfp, "\n  Processing file %d: %s (format %d)\n", nfiles, rfile, rformat);

    /* initialize reading the swath file */
    void *mbio_ptr = nullptr;
    double btime_d, etime_d;
    int beams_bath, beams_amp, pixels_ss;

    /* check for "fast bathymetry" or "fbt" file */
    mb_get_fbt(verbose, rfile, &rformat, &error);

    /* initialize reading */
    if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, 
                           btime_i, etime_i, speedmin, timegap, astatus, apath,
                           &mbio_ptr, &btime_d, &etime_d, &beams_bath, 
                           &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
      fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* allocate memory for reading data arrays */
    char *beamflag = nullptr;
    double *bath = nullptr;
    double *amp = nullptr;
    double *bathlon = nullptr;
    double *bathlat = nullptr;
    double *ss = nullptr;
    double *sslon = nullptr;
    double *sslat = nullptr;

    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 
                                sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
                                sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                sizeof(double), (void **)&bathlon, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                sizeof(double), (void **)&bathlat, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                sizeof(double), (void **)&sslon, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                sizeof(double), (void **)&sslat, &error);

    if (error != MB_ERROR_NO_ERROR) {
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* read and store swath data */
    int rpings, kind;
    int time_i[7];
    double time_d;
    double navlon, navlat;
    double speed, heading;
    double distance, altitude, sensordepth;
    char comment[MB_COMMENT_MAXLINE];
    int file_pings = 0;
    int file_beams = 0;

    /* loop over reading data */
    while (error <= MB_ERROR_NO_ERROR) {
      status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d,
                      &navlon, &navlat, &speed, &heading, &distance, &altitude,
                      &sensordepth, &beams_bath, &beams_amp, &pixels_ss,
                      beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat,
                      comment, &error);

      /* handle time gaps */
      if (error == MB_ERROR_TIME_GAP) {
        error = MB_ERROR_NO_ERROR;
        status = MB_SUCCESS;
      }

      /* process bathymetry data */
      if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
        file_pings++;
        
        for (int ib = 0; ib < beams_bath; ib++) {
          if (mb_beam_ok(beamflag[ib])) {
            SwathPoint point;
            point.longitude = bathlon[ib];
            point.latitude = bathlat[ib];
            point.depth = bath[ib];
            point.time = time_d;
            point.beam_number = ib;
            swath_points.push_back(point);
            file_beams++;
          }
        }
      }

      if (verbose >= 2) {
        fprintf(outfp, "    Ping read: kind=%d beams_bath=%d error=%d status=%d\n",
                kind, beams_bath, error, status);
      }
    }

    /* close the swath file */
    status = mb_close(verbose, &mbio_ptr, &error);

    if (verbose > 0) {
      fprintf(outfp, "    File statistics: %d pings, %d valid beams\n", 
              file_pings, file_beams);
    }

    total_pings += file_pings;
    total_beams += file_beams;
  }

  /* close datalist */
  mb_datalist_close(verbose, &datalist, &error);

  /* print summary statistics */
  fprintf(outfp, "\nSwath Data Reading Complete:\n");
  fprintf(outfp, "  Files processed:    %d\n", nfiles);
  fprintf(outfp, "  Total pings:        %d\n", total_pings);
  fprintf(outfp, "  Total valid beams:  %d\n", total_beams);
  fprintf(outfp, "  Points collected:   %zu\n", swath_points.size());

  /* placeholder for 3D mesh generation and glTF output */
  fprintf(outfp, "\n3D Mesh Generation:\n");
  fprintf(outfp, "  [To be implemented: Convert swath points to 3D mesh]\n");
  fprintf(outfp, "  [To be implemented: Generate OGC 3D Tiles]\n");
  fprintf(outfp, "  [To be implemented: Write glTF/glb output to %s]\n", output_file);

  /* some basic statistics about the collected points */
  if (!swath_points.empty()) {
    double min_lon = swath_points[0].longitude;
    double max_lon = swath_points[0].longitude;
    double min_lat = swath_points[0].latitude;
    double max_lat = swath_points[0].latitude;
    double min_depth = swath_points[0].depth;
    double max_depth = swath_points[0].depth;

    for (const auto& point : swath_points) {
      min_lon = std::min(min_lon, point.longitude);
      max_lon = std::max(max_lon, point.longitude);
      min_lat = std::min(min_lat, point.latitude);
      max_lat = std::max(max_lat, point.latitude);
      min_depth = std::min(min_depth, point.depth);
      max_depth = std::max(max_depth, point.depth);
    }

    fprintf(outfp, "\nData Bounds:\n");
    fprintf(outfp, "  Longitude: %.6f to %.6f\n", min_lon, max_lon);
    fprintf(outfp, "  Latitude:  %.6f to %.6f\n", min_lat, max_lat);
    fprintf(outfp, "  Depth:     %.2f to %.2f meters\n", min_depth, max_depth);
  }

  fprintf(outfp, "\nProgram <%s> completed\n", program_name);
  
  return MB_SUCCESS;
}
