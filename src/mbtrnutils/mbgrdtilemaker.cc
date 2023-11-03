/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdtilemaker.cc     6/11/2022
 *
 *    Copyright (c) 2022-2023 by
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
  @file
 * MBgrdtilemaker creates a set of overlapping square grids from an original
 * topography grid. The grid tiles will have 50% overlap in all directions with
 * neighboring grids.
 *
 * Author:  D. W. Caress
 * Date:  June 11, 2022
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <limits>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

/* output stream for basic stuff (stdout if verbose <= 1,
    stderr if verbose > 1) */
FILE *outfp = stdout;

/* program identifiers */
constexpr char program_name[] = "mbgrdtilemaker";
constexpr char help_message[] =
    "MBgrdtilemaker creates a set of overlapping square grids from an original "
    "topography grid. The grid tiles will have 50% overlap in all directions with "
    "neighboring grids.";
constexpr char usage_message[] =
    "mbgrdtilemaker\n"
    "\t--verbose\n"
    "\t--help\n\n"
    "\t--input=input_grid\n"
    "\t--output=output_root\n\n"
    "\t--tile-dimension=tile_dimension\n"
    "\t--tile-mode=mode\n\n"
    "\t--tile-spacing=tile_spacing\n\n";

/*--------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  int verbose = 0;
  int status = MB_SUCCESS;
  mb_path input_grid = "";
  mb_path output_root = "";
  int tile_dimension = 2001;
  int tile_dimension_spacing = ((tile_dimension - 1) / 2) + 1;
  double tile_width = 0.0;
  double tile_spacing = 0.0;
  int tile_mode = 0;

  static struct option options[] = {{"verbose", no_argument, nullptr, 0},
                                    {"help", no_argument, nullptr, 0},
                                    {"input", required_argument, nullptr, 0},
                                    {"mode", required_argument, nullptr, 0},
                                    {"output", required_argument, nullptr, 0},
                                    {"tile-dimension", required_argument, nullptr, 0},
                                    {"tile-spacing", required_argument, nullptr, 0}};

  int option_index;
  bool errflg = false;
  int c;
  bool help = false;
  while ((c = getopt_long(argc, argv, "D:d:HhI:i:M:m:O:o:S:s:Vv", options, &option_index)) != -1) {
    switch (c) {
    /*-------------------------------------------------------*/
    /* long options all return c=0 */
    case 0:
      if (strcmp("verbose", options[option_index].name) == 0) {
        verbose++;
        if (verbose >= 2)
          outfp = stderr;
      }
      else if (strcmp("help", options[option_index].name) == 0) {
        help = true;
      }
      else if (strcmp("input", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%1023s", input_grid);
        if (n != 1 || strlen(input_grid) <= 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
      else if (strcmp("output", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%1023s", output_root);
        if (n != 1 || strlen(input_grid) <= 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
      else if (strcmp("tile-dimension", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%d", &tile_dimension);
        if (n != 1 || tile_dimension <= 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
      else if (strcmp("tile-mode", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%d", &tile_mode);
        if (n != 1 || tile_mode < 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
      else if (strcmp("tile-spacing", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%lf", &tile_spacing);
        if (n != 1 || tile_spacing <= 0.0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
      break;
    /*-------------------------------------------------------*/
    /* short options */
    case 'D':
    case 'd':
      sscanf(optarg, "%d", &tile_dimension);
      break;
    case 'H':
    case 'h':
      help = true;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%1023s", input_grid);
      break;
    case 'M':
    case 'm':
      sscanf(optarg, "%d", &tile_mode);
      break;
    case 'O':
    case 'o':
      sscanf(optarg, "%1023s", output_root);
      break;
    case 'S':
    case 's':
      sscanf(optarg, "%lf", &tile_spacing);
      break;
    case 'V':
    case 'f':
      verbose++;
      if (verbose >= 2)
        outfp = stderr;
      break;
    case '?':
      errflg = true;
    }
  }

  if (errflg) {
    fprintf(outfp, "usage: %s\n", usage_message);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  if (verbose == 1 || help) {
    fprintf(outfp, "\nProgram %s\n", program_name);
    fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
  }

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
    fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(outfp, "dbg2  Control Parameters:\n");
    fprintf(outfp, "dbg2       verbose:              %d\n", verbose);
    fprintf(outfp, "dbg2       help:                 %d\n", help);
    fprintf(outfp, "dbg2       input_grid:           %s\n", input_grid);
    fprintf(outfp, "dbg2       output_root:          %s\n", output_root);
    fprintf(outfp, "dbg2       tile_mode:            %d\n", tile_mode);
    fprintf(outfp, "dbg2       tile_dimension:       %d\n", tile_dimension);
    fprintf(outfp, "dbg2       tile_spacing:         %f\n", tile_spacing);
  }

  if (help) {
    fprintf(outfp, "\n%s\n", help_message);
    fprintf(outfp, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  // Set tile_dimension if neither spacing nor dimension has been set
  if (tile_dimension <= 0 && tile_spacing <= 0.0)
    tile_dimension = 2001;

  int error = MB_ERROR_NO_ERROR;
  int memclear_error = MB_ERROR_NO_ERROR;

  int grid_projection_mode;
  mb_path grid_projection_id;
  float grid_nodatavalue;
  int grid_nxy;
  int grid_n_columns;
  int grid_n_rows;
  double grid_min;
  double grid_max;
  double grid_xmin;
  double grid_xmax;
  double grid_ymin;
  double grid_ymax;
  double grid_dx;
  double grid_dy;
  float *grid_data;

  // Read the input grid file
  status = mb_read_gmt_grd(verbose, input_grid, &grid_projection_mode, grid_projection_id,
                           &grid_nodatavalue, &grid_nxy, &grid_n_columns, &grid_n_rows,
                           &grid_min, &grid_max, &grid_xmin, &grid_xmax, &grid_ymin,
                           &grid_ymax, &grid_dx, &grid_dy, &grid_data, NULL, NULL, &error);
  if (status == MB_FAILURE) {
    fprintf(stderr, "Unable to read input grid %s\n", input_grid);
    fprintf(stderr, "Program %s terminated\n", program_name);
    exit(error);
  }

  fprintf(stdout, "\nInput grid:           %s\n", input_grid);
  fprintf(stdout, "  Projection mode:    %d\n", grid_projection_mode);
  fprintf(stdout, "  Projection id:      %s\n", grid_projection_id);
  fprintf(stdout, "  No data value:      %f\n", grid_nodatavalue);
  fprintf(stdout, "  grid_nxy:           %d\n", grid_nxy);
  fprintf(stdout, "  grid_n_columns:     %d\n", grid_n_columns);
  fprintf(stdout, "  grid_n_rows:        %d\n", grid_n_rows);
  fprintf(stdout, "  grid_min:           %f\n", grid_min);
  fprintf(stdout, "  grid_max:           %f\n", grid_max);
  fprintf(stdout, "  grid_xmin:          %f\n", grid_xmin);
  fprintf(stdout, "  grid_xmax:          %f\n", grid_xmax);
  fprintf(stdout, "  grid_ymin:          %f\n", grid_ymin);
  fprintf(stdout, "  grid_ymax:          %f\n", grid_ymax);
  fprintf(stdout, "  grid_dx:            %f\n", grid_dx);
  fprintf(stdout, "  grid_dy:            %f\n", grid_dy);

  // Set up output tile scheme
  // Each grid tile will have 50% overlap with surrounding tiles
  // Tiles are either defined in terms of the desired dimensions of the grids
  // or the tile spacing in meters. The tileset will be constructed with the
  // using the southwest corner of the input grid as the origin.
  double tile_width_x;
  double tile_width_y;
  double tile_spacing_x;
  double tile_spacing_y;
  double tileset_origin_x = grid_xmin;
  double tileset_origin_y = grid_ymin;
  if (tile_spacing > 0.0) {
    tile_dimension = 2.0 * tile_spacing / grid_dx + 1;
    tile_width_x = (tile_dimension - 1) * grid_dx;
    tile_width_y = (tile_dimension - 1) * grid_dy;
    tile_spacing_x = tile_width_x / 2.0;
    tile_spacing_y = tile_width_x / 2.0;
  } else if (tile_dimension > 0) {
    tile_width_x = (tile_dimension - 1) * grid_dx;
    tile_width_y = (tile_dimension - 1) * grid_dy;
    tile_spacing_x = tile_width_x / 2.0;
    tile_spacing_y = tile_width_x / 2.0;
  } else {
    fprintf(stderr, "Neither tile dimension nor spacing have been defined "
            "(--tile-dimension=dimension or --tile-spacing=spacing)\n"
            "Program %s terminated\n", program_name);
    exit(MB_ERROR_BAD_PARAMETER);
  }
  int num_tiles_x = (int)ceil((grid_xmax - tileset_origin_x) / tile_spacing_x);
  int num_tiles_y = (int)ceil((grid_ymax - tileset_origin_y) / tile_spacing_y);
  int num_tiles = num_tiles_x * num_tiles_y;
  double tileset_max_x = tileset_origin_x + num_tiles_x * tile_spacing_x;
  double tileset_max_y = tileset_origin_y + num_tiles_y * tile_spacing_y;
  mb_path tile_xlabel;
  mb_path tile_ylabel;
  mb_path tile_zlabel;
  mb_pathplusplus tile_title;
  strcpy(tile_xlabel, "Easting (meters)");
  strcpy(tile_ylabel, "Northing (meters)");
  strcpy(tile_zlabel, "Topography (meters)");

  fprintf(stdout, "\nOutput tileset:       %s\n", output_root);
  fprintf(stdout, "  tile_width_x:       %f\n", tile_width_x);
  fprintf(stdout, "  tile_width_y:       %f\n", tile_width_y);
  fprintf(stdout, "  tileset_origin_x:   %f\n", tileset_origin_x);
  fprintf(stdout, "  tileset_origin_y:   %f\n", tileset_origin_x);
  fprintf(stdout, "  tileset_max_x:      %f\n", tileset_max_x);
  fprintf(stdout, "  tileset_max_y:      %f\n", tileset_max_y);
  fprintf(stdout, "  num_tiles_x:        %d\n", num_tiles_x);
  fprintf(stdout, "  num_tiles_y:        %d\n", num_tiles_y);
  fprintf(stdout, "  num_tiles:          %d\n", num_tiles);
  fprintf(stdout, "  tile_xlabel:        %s\n", tile_xlabel);
  fprintf(stdout, "  tile_ylabel:        %s\n", tile_ylabel);
  fprintf(stdout, "  tile_zlabel:        %s\n", tile_zlabel);
  fprintf(stdout, "  tile_title:         %s\n", tile_title);

  mkdir(output_root, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  mb_pathplus csv_file;
  snprintf(csv_file, sizeof(csv_file), "%s/tiles.csv", output_root);
  FILE *csv_fp = fopen(csv_file, "w");
  fprintf(csv_fp, "TileName , Easting , Northing , %d\n", num_tiles);

  // Loop over all tiles
  int itile = 0;
  for (int j=0; j<num_tiles_y; j++) {
    for (int i=0; i<num_tiles_x; i++) {
      int k = i * num_tiles_y + j;
      mb_pathplus tile_name;
      mb_pathplusplus tile_grid;
      mb_pathplusplus tile_bo;
      snprintf(tile_name, sizeof(tile_name), "%s_%4.4d", output_root, itile);
      snprintf(tile_grid, sizeof(tile_grid), "%s/%s.grd", output_root, tile_name);
      snprintf(tile_bo, sizeof(tile_bo), "%s.bo", tile_name);
      snprintf(tile_title, sizeof(tile_title), "Tile %s", tile_name);
      int tile_projection_mode = grid_projection_mode;
      mb_path tile_projection_id;
      strncpy(tile_projection_id, grid_projection_id, MB_PATH_MAXLINE-1);
      float tile_nodatavalue = grid_nodatavalue;
      int tile_nxy = tile_dimension * tile_dimension;
      int tile_n_columns = tile_dimension;
      int tile_n_rows = tile_dimension;
      double tile_min = 0.0;
      double tile_max = 0.0;
      double tile_xmin = tileset_origin_x + i * tile_spacing_x - 0.5 * tile_spacing_x;
      double tile_xcen = tile_xmin + 0.5 * tile_width_x;
      double tile_xmax = tile_xmin + tile_width_x;
      double tile_ymin = tileset_origin_y + j * tile_spacing_y - 0.5 * tile_spacing_y;
      double tile_ycen = tile_ymin + 0.5 * tile_width_y;;
      double tile_ymax = tile_ymin + tile_width_y;;
      double tile_dx = grid_dx;
      double tile_dy = grid_dy;
      float *tile_data = NULL;
      status = mb_mallocd(verbose, __FILE__, __LINE__, tile_dimension * tile_dimension * sizeof(float), (void **)&tile_data, &error);
      if (error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
        fprintf(outfp, "\nMBIO Error allocating tile array:\n%s\n", message);
        fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
        mb_memory_clear(verbose, &memclear_error);
        exit(MB_ERROR_MEMORY_FAIL);
      }
      for (int kk=0; kk < tile_nxy; kk++)
        tile_data[kk] = tile_nodatavalue;

      // Get tile origin location in the input grid
      int ii0 = (int)round((tile_xmin - grid_xmin) / grid_dx);
      int jj0 = (int)round((tile_ymin - grid_ymin) / grid_dy);

      // Fill in tile data from input_grid
      bool first = true;
      for (int jj=0;jj<tile_dimension;jj++) {
        for (int ii=0;ii<tile_dimension;ii++) {
          int kk = ii * tile_dimension + jj;
          int iii = ii0 + ii;
          int jjj = jj0 + jj;
          int kkk = iii * grid_n_rows + jjj;
          if (iii >= 0 && iii < grid_n_columns && jjj >= 0 && jjj < grid_n_rows) {
            tile_data[kk] = grid_data[kkk];
            if (tile_data[kk] != tile_nodatavalue) {
              if (first) {
                tile_min = tile_data[kk];
                tile_max = tile_data[kk];
                first = false;
              } else {
                tile_min = MIN(tile_min, tile_data[kk]);
                tile_max = MAX(tile_max, tile_data[kk]);
              }
            }
          }
        }
      }

    fprintf(outfp, "\nTile %d %d: %s\n", i, j, tile_name);
    status = mb_write_gmt_grd(verbose, tile_grid, tile_data, tile_nodatavalue,
                              tile_dimension, tile_dimension,
                              tile_xmin, tile_xmax, tile_ymin, tile_ymax,
                              tile_min, tile_max, tile_dx, tile_dy,
                              tile_xlabel, tile_ylabel, tile_zlabel, tile_title,
                              tile_projection_id, argc, argv, &error);
    fprintf(csv_fp, "%s , %.2f , %.2f\n", tile_bo, tile_xcen, tile_ycen);

    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&tile_data, &error);
    itile++;
    }
  }

  status = mb_freed(verbose, __FILE__, __LINE__, (void **)&grid_data, &error);
  fclose(csv_fp);

  // Copy source grid to tiles directory
  mb_command command;
  snprintf(command, sizeof(command), "cp %s %s/source_grid.grd", input_grid, output_root);
  fprintf(outfp, "\n-----------------------------------------------------------\nExecuting: %s\n", command);
  system(command);

  // Generate octree files from the grids
  for (int itile=0; itile<num_tiles; itile++) {
    mb_pathplus tile_name;
    mb_pathplusplus tile_pathlet;
    snprintf(tile_name, sizeof(tile_name), "%s_%4.4d", output_root, itile);
    snprintf(tile_pathlet, sizeof(tile_pathlet), "%s/%s", output_root, tile_name);
    snprintf(command, sizeof(command), "mbgrd2octree --input=%s.grd --output=%s.bo",
                      tile_pathlet, tile_pathlet);
    fprintf(outfp, "\n-----------------------------------------------------------\nExecuting: %s\n", command);
    system(command);
  }

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

	exit(error);
}
/*--------------------------------------------------------------------*/
