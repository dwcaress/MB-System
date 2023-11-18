/*--------------------------------------------------------------------
 *    The MB-system:  mbgrd2octree.cc  8/13/2020
 *
 *    Copyright (c) 2020-2023 by
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
 * MBgrd2octree translates a topography grid to a TRN octree topography model
 * for use with the MB-System TRN (terrain relative navigation) tools.
 * The input grid is expected to be in a carteisan projected coordinate system
 * such as UTM.
 * This program derives from grdOctreeMaker, a TRN program contained within the
 * terrain-nav codebase created by Steve Rock's Precision Navigation project
 * team over many years. The original author(s) are unclear, but probably
 * include multiple Steve Rock students at Stanford such as Debbie Meduna,
 * Peter Kimball, etc.
 *
 * Author:  D. W. Caress
 * Date:  August 13, 2020
 */
/*--------------------------------------------------------------------*/


/*! grdOctreeMaker
  This octree generator takes a utm grd file and generates an octree from it.

  The input file is set with INFILE and the name to write to is OUTFILE.

  For details on how octree works including the internals of how the octree generates from the inpot
  points, go look at Octree.hpp.
*/

/*! Coordinate Systems
  Octrees used for TRN have historically been stored in the NED coordinate system.  This code can
  take a The define X_INDEX_FIRST sets the indexing order into the one dimensional array of z values.
  I have only ever worked with X_INDEX_FIRST = 1, but it should work to negate z and flip the x-y index
  ordering when set to 0. If things aren't working, look at the points or the north, east, and down
  bounds on the points and make sure they are correct for a NED coordinate system point in Monterey
  Bay (X: 4E6, Y: 5E5, Z: >0).

  The Raytrace function of the octree requires a Euclidean coordinate system to give meaningful
  results. Otherwise, the octree will work fine in any coordinate system (like LLA), and querying
  the value at a location (with an LLA query point) will work fine.
*/

/*! This code has steps:
  1) autodetect the size of the map (and resolution optionally)
  2) generate the octree object
  3) add points to the octree
  4) fill the octree cell(s) below the added points
  5) compress the octree
*/

/*! Reducing the area of the resulting octree:
  You can cut down the area of the octree with limits on the North and East coordinates. This is
  set with NORTH_BOUND, EAST_BOUND, SOUTH_BOUND, and WEST_BOUND respectively. Set any of them to
  -1 to disable that bound and go to the edge of the input grd on that dimension. Similarly,
  MAX_ACCEPTED_DEPTH and MIN_ACCEPTED_DEPTH set the range of depths which are accepted. If you
  wanted to be fancy, you could make these inputs rather than #defines.

  To use the spacing between the zeroth and first elements in X and Y as the x-y resolution of
  the octree, set RESOLUTION to -1. Otherwise, set RESOLUTION to the desired resolution of the map.

  FILL_NUMBER sets how many voxels to fill below each input point. This insures that steep hills
  will not have holes when looking sideways at them.
*/

//! zGrid could be moved to OctreeSupport (that's where I have it, but to simplify sending this, I copied it here)
/*//header entry:

  struct zGrid{
  zGrid(): zValues(NULL), numXValues(0), numYValues(0), xIndexFirst(false) {}
  zGrid(float* const z, unsigned int numX, unsigned int numY, bool xFirst): zValues(z), numXValues(numX), numYValues(numY), xIndexFirst(xFirst){}
  double getZ(unsigned int xIndex, unsigned int yIndex) const;
  void Print();

  float* zValues;
  unsigned int numXValues, numYValues;
  bool xIndexFirst;
  };

// cpp entry
double zGrid::getZ(unsigned int xIndex, unsigned int yIndex) const{
if(xIndexFirst){
return -static_cast<double>(zValues[xIndex * numYValues + yIndex]);
}
else{
return -static_cast<double>(zValues[yIndex * numXValues + xIndex]);
}
}
void zGrid::Print(){
std::cout << "X: " << numXValues << "\tY: " << numYValues << "\tXfirst: " << xIndexFirst << std::endl;
}
//end zGrid definition

*/

/*--------------------------------------------------------------------*/
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include "mapio.h"
#include <netcdf.h>
#include "OctreeSupport.hpp"
#include "Octree.hpp"

#include "mb_define.h"
#include "mb_status.h"

/* output stream for basic stuff (stdout if verbose <= 1,
    stderr if verbose > 1) */
FILE *outfp = stdout;

/* program identifiers */
constexpr char program_name[] = "mbgrd2octree";
constexpr char help_message[] =
    "MBgrd2octree translates a topography grid to a TRN octree topography model"
    "for use with the MB-System TRN (terrain relative navigation) tools."
    "The input grid is expected to be in a carteisan projected coordinate system"
    "such as UTM.";
constexpr char usage_message[] =
    "mbgrd2octree\n"
    "\t--verbose\n"
    "\t--help\n\n"
    "\t--input=input_grid\n"
    "\t--output=output_root\n\n"
    "\t--tile-dimension=tile_dimension\n"
    "\t--tile-mode=mode\n\n"
    "\t--tile-spacing=tile_spacing\n\n";

#define X_INDEX_FIRST 1

#define RESOLUTION 1
#define FILL_NUMBER 2

#define NORTH_BOUND -1
#define SOUTH_BOUND -1
#define EAST_BOUND  -1
#define WEST_BOUND  -1/
#define MIN_ACCEPTED_DEPTH  1
#define MAX_ACCEPTED_DEPTH  3500

int setupXYZ(double** xValues, double** yValues, zGrid& zValues, char *inFile );

int main( int argc, char **argv )
{
  int verbose = 0;
  int status = MB_SUCCESS;
  mb_path inFile;
  mb_path outFile;
  double bounds[4];
  bool bounds_set = false;

  static struct option options[] = {{"verbose", no_argument, nullptr, 0},
                                    {"help", no_argument, nullptr, 0},
                                    {"bounds", required_argument, nullptr, 0},
                                    {"input", required_argument, nullptr, 0},
                                    {"output", required_argument, nullptr, 0}};

  int option_index;
  bool errflg = false;
  int c;
  bool help = false;
  while ((c = getopt_long(argc, argv, "HhI:i:O:o:R:r:Vv", options, &option_index)) != -1) {
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
        const int n = sscanf(optarg, "%1023s", inFile);
        if (n != 1 || strlen(inFile) <= 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
        if (strlen(inFile) < 5 || strncmp(".grd", &inFile[strlen(inFile)-4], 4) != 0) {
          if (strlen(inFile) < MB_PATH_MAXLINE - 5)
            strcat(inFile, ".grd");
        }
      }
      else if (strcmp("output", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%1023s", outFile);
        if (n != 1 || strlen(outFile) <= 0) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        }
        if (strlen(outFile) < 5 || strncmp(".bo", &outFile[strlen(outFile)-3], 3) != 0) {
          if (strlen(outFile) < MB_PATH_MAXLINE - 4)
            strcat(outFile, ".bo");
        }
      }
      else if (strcmp("bounds", options[option_index].name) == 0) {
        const int n = sscanf(optarg, "%lf/%lf/%lf/%lf",
                              &bounds[0], &bounds[1], &bounds[2], &bounds[3]);
        if (n != 4 || (bounds[0] >= bounds[1]) || (bounds[2] >= bounds[3])) {
          fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                  options[option_index].name, optarg, program_name);
          exit(MB_ERROR_BAD_PARAMETER);
        } else {
          bounds_set = true;
        }
      }
      break;
    /*-------------------------------------------------------*/
    /* short options */
    case 'H':
    case 'h':
      help = true;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%1023s", inFile);
      break;
    case 'O':
    case 'o':
      sscanf(optarg, "%1023s", outFile);
      break;
    case 'R':
    case 'r':
      {
      int n = sscanf(optarg, "%lf/%lf/%lf/%lf",
                            &bounds[0], &bounds[1], &bounds[2], &bounds[3]);
      if (n != 4 || (bounds[0] >= bounds[1]) || (bounds[2] >= bounds[3])) {
        fprintf(stderr, "Failed to parse argument: %s=%s\nProgram %s terminated\n",
                options[option_index].name, optarg, program_name);
        exit(MB_ERROR_BAD_PARAMETER);
      }
      }
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
    fprintf(outfp, "dbg2       inFile:               %s\n", inFile);
    fprintf(outfp, "dbg2       outFile:              %s\n", outFile);
    fprintf(outfp, "dbg2       bounds_set:           %d\n", bounds_set);
    if (bounds_set) {
      fprintf(outfp, "dbg2       bounds[0]:            %f\n", bounds[0]);
      fprintf(outfp, "dbg2       bounds[1]:            %f\n", bounds[1]);
      fprintf(outfp, "dbg2       bounds[2]:            %f\n", bounds[2]);
      fprintf(outfp, "dbg2       bounds[3]:            %f\n", bounds[3]);
    }
  }

  if (help) {
    fprintf(outfp, "\n%s\n", help_message);
    fprintf(outfp, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  long north_bound = -1;
  long south_bound = -1;
  long  east_bound = -1;
  long  west_bound = -1;
  if(bounds_set) {
    east_bound  = bounds[0];
    west_bound  = bounds[1];
    south_bound = bounds[2];
    north_bound = bounds[3];
  }

  // Read input data
  if( open( inFile, O_EXCL) == 0 ) {
    printf("\nInput grid %s not opened.\n", inFile);
    fprintf(outfp, "Program <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  //Yes I mean '=' not '==' DO NOT CHANGE IT.
  zGrid zValues;
  double *xValues = NULL, *yValues = NULL;
  if(int temp = setupXYZ(&xValues, &yValues, zValues, inFile)){return temp;}

  //Auto detect X and Y resolution or use input
  Vector DesiredResolution(RESOLUTION,RESOLUTION,RESOLUTION);
  if(RESOLUTION == -1){
    DesiredResolution.x = xValues[1] - xValues[0];
    DesiredResolution.y = yValues[1] - yValues[0];
    DesiredResolution.z = 1;
  }

  Vector Lowermost;
  Vector Uppermost;
  bool uninitialized = true;

  //autodetect size
  Vector point;
  for(unsigned int xIndex = 0; xIndex < zValues.numXValues; xIndex++){
    for(unsigned int yIndex = 0; yIndex < zValues.numYValues; yIndex++){
      if(zValues.getZ(xIndex, yIndex) != 99999 && !isnan(zValues.getZ(xIndex, yIndex))){
        //test bounds
        if((north_bound != -1) && (xValues[xIndex] > north_bound)){continue;}
        if((south_bound != -1) && (xValues[xIndex] < south_bound)){continue;}
        if((east_bound != -1) && (yValues[yIndex] > east_bound)){continue;}
        if((west_bound != -1) && (yValues[yIndex] < west_bound)){continue;}
        if((MAX_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) > MAX_ACCEPTED_DEPTH)){continue;}
        if((MIN_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) < MIN_ACCEPTED_DEPTH)){continue;}


        if(uninitialized){
           Lowermost = Vector(xValues[xIndex], yValues[yIndex], zValues.getZ(xIndex, yIndex));
           Uppermost = Vector(xValues[xIndex], yValues[yIndex], zValues.getZ(xIndex, yIndex));
           uninitialized = false;
           Lowermost.Print();
        }

        point = Vector( xValues[xIndex], yValues[yIndex], zValues.getZ(xIndex, yIndex));
        if(Lowermost.x > point.x){ Lowermost.x = point.x;}
        if(Lowermost.y > point.y){ Lowermost.y = point.y;}
        if(Lowermost.z > point.z){ Lowermost.z = point.z;}
        if(Uppermost.x < point.x){ Uppermost.x = point.x;}
        if(Uppermost.y < point.y){ Uppermost.y = point.y;}
        if(Uppermost.z < point.z){ Uppermost.z = point.z;}


      }
    }
  }

  Vector PointCloudSize = Uppermost - Lowermost + Vector(1.0, 1.0, 1.0);
  Vector OctreeSize = DesiredResolution;
  while(!OctreeSize.StrictlyGreaterOrEqualTo(PointCloudSize)){
    OctreeSize *= 2.0;
  }
  Vector LowerBounds = Lowermost - DesiredResolution * 0.5;
  Vector UpperBounds = LowerBounds + OctreeSize;

  //initialize octree
  //printf("about to build Octree\n");
  Octree<bool> newOctreeMap(DesiredResolution + Vector(0.001,0.001,0.001),
                            LowerBounds, UpperBounds, OctreeType::BinaryOccupancy);

  // Add points
  int countPointsAdded = 0;
  for(unsigned int xIndex = 0; xIndex < zValues.numXValues; xIndex++){
    for(unsigned int yIndex = 0; yIndex < zValues.numYValues; yIndex++){
      if(zValues.getZ(xIndex, yIndex) != 99999 && !isnan(zValues.getZ(xIndex, yIndex))){
        if(zValues.getZ(xIndex, yIndex) <= 4000){
          point = Vector( xValues[xIndex], yValues[yIndex], zValues.getZ(xIndex, yIndex));

          //test bounds
          if((north_bound != -1) && (xValues[xIndex] > north_bound)){continue;}
          if((south_bound != -1) && (xValues[xIndex] < south_bound)){continue;}
          if((east_bound != -1) && (yValues[yIndex] > east_bound)){continue;}
          if((west_bound != -1) && (yValues[yIndex] < west_bound)){continue;}
          if((MAX_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) > MAX_ACCEPTED_DEPTH)){continue;}
          if((MIN_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) < MIN_ACCEPTED_DEPTH)){continue;}

          countPointsAdded ++;
          newOctreeMap.AddPoint(point);
        }
      }
    }
  }
  Vector TrueResolution = newOctreeMap.GetTrueResolution();

  //fill below input points
  for(unsigned int xIndex = 0; xIndex < zValues.numXValues; xIndex++){
    for(unsigned int yIndex = 0; yIndex < zValues.numYValues; yIndex++){
      if((zValues.getZ(xIndex, yIndex) != 99999) && (!isnan(zValues.getZ(xIndex, yIndex))) && (zValues.getZ(xIndex, yIndex) < 3000)) {
        //test bounds
        if((north_bound != -1) && (xValues[xIndex] > north_bound)){continue;}
        if((south_bound != -1) && (xValues[xIndex] < south_bound)){continue;}
        if((east_bound != -1) && (yValues[yIndex] > east_bound)){continue;}
        if((west_bound != -1) && (yValues[yIndex] < west_bound)){continue;}
        if((MAX_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) > MAX_ACCEPTED_DEPTH)){continue;}
        if((MIN_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) < MIN_ACCEPTED_DEPTH)){continue;}

        point = Vector( xValues[xIndex], yValues[yIndex], zValues.getZ(xIndex, yIndex));

        //fill
        double zToFill = point.z + TrueResolution.z;
        for(int jj = 0; jj < FILL_NUMBER; jj++){
           newOctreeMap.FillSmallestResolutionLeafAtPointIfEmpty(Vector(point.x, point.y, zToFill),true);

           zToFill += TrueResolution.z;
        }
      }
    }
  }

  //compress
  newOctreeMap.Collapse();

  newOctreeMap.SaveToFile(outFile);

  fprintf(outfp, "\nCompleted octree %s:\n", outFile);
  newOctreeMap.Print();

  delete[] xValues;
  delete[] yValues;
  delete[] zValues.zValues;

  return 0;
}

/*! The following is mostly riped from the TRN code to load a DEM */

/*
//struct mapsrc
int ncid;           // NetCDF file id
double *x;          // Double array of ALL the x axis data
int xid;            // NetCDF variable id to X variable
int xdimid;         // NetCDF dimension id to X dimension
size_t xdimlen;     // Length of X dimension
double *y;          // Double array of ALL the X axis data
int yid;            // NetCDF variable id to Y variable
int ydimid;         // NetCDF dimension id to Y dimension
size_t ydimlen;     // Length of Y dimension
int zid;            // NetCDF variable id to Z variable
int status;         // Error status (see MAPIO_* values)
*/
int setupXYZ(double** xValues, double** yValues, zGrid& zValues, char *inFile )
{
   mapsrc* src = mapsrc_init();

   int err, i;
   double range[2];
   double delta;


   err = nc_open(inFile, NC_NOWRITE, &(src->ncid));
   if(NC_NOERR != err) {
      std::cout << "Failed to load input grid: " << err << "\n";
      std::cout << "check input filename\n";
      return 1;
   }

   /* get x variable */
   err = nc_inq_dimid(src->ncid, "x", &(src->xdimid));
   if(NC_NOERR != err) {
      err = nc_inq_dimid(src->ncid,"lon", &(src->ydimid));
      if(NC_NOERR != err){
   return 2;
      }
      std::cout << "Warning: Using \"lon\" for x\n";
   }
   err = nc_inq_dimlen(src->ncid, src->xdimid, &(src->xdimlen));
   if(NC_NOERR != err) {
      return 3;
   }

   *xValues = new double[src->xdimlen];

   err = nc_inq_varid(src->ncid, "x", &(src->xid));
   if(NC_NOERR != err) {
      err = nc_inq_varid(src->ncid,"lon", &(src->xid));
      if(NC_NOERR != err){
   return 4;
      }
      std::cout << "Warning: Using \"lon\" for x\n";
   }
   err = nc_get_att_double(src->ncid, src->xid, "actual_range", range);
   if(NC_NOERR != err) {
      return 5;
   }

   //fill in xvec based on range boundaries.
   delta = (range[1] - range[0]) / (src->xdimlen - 1);
   for(i = 0; i < int(src->xdimlen); i++) {
      (*xValues)[i] = range[0] + delta * i;
   }

   /* get y variable */
   err = nc_inq_dimid(src->ncid, "y", &(src->ydimid));
   if(NC_NOERR != err) {
      err = nc_inq_dimid(src->ncid,"lat", &(src->ydimid));
      if(NC_NOERR != err){
   return 6;
      }
      std::cout << "Warning: Using \"lat\" for y\n";
   }
   err = nc_inq_dimlen(src->ncid, src->ydimid, &(src->ydimlen));
   if(NC_NOERR != err) {
      return 7;
   }

   *yValues = new double[src->ydimlen];

   err = nc_inq_varid(src->ncid, "y", &(src->yid));
   if(NC_NOERR != err) {
      err = nc_inq_varid(src->ncid,"lat", &(src->yid));
      if(NC_NOERR != err){
   return 8;
      }
      std::cout << "Warning: Using \"lat\" for y\n";
   }

   err = nc_get_att_double(src->ncid, src->yid, "actual_range", range);
   if(NC_NOERR != err) {
      return 9;
   }
   //fill in yvec based on range boundaries.
   delta = (range[1] - range[0]) / (src->ydimlen - 1);
   for(i = 0; i < int(src->ydimlen); i++) {
      (*yValues)[i] = range[0] + delta * i;
   }

   /* get z variable */
   err = nc_inq_varid(src->ncid, "z", &(src->zid));
   if(NC_NOERR != err) {
      return 10;
   }

   // start = { x0, y0}, count = {xdimlen, ydimlen}
   size_t start[2];        // For NetCDF access -> {x0, y0}
   size_t count[2];        // For NetCDF access -> {xdimlen, ydimlen}

   int XI = 1, YI = 0;
   float* array;

   // Get the indices into the map array for the corners of our submap
   start[XI] = 0;//nearest(xmin, src->x, src->xdimlen);
   start[YI] = 0;
   count[XI] = src->xdimlen;//nearest(xmax, src->x, src->xdimlen) - start[XI] + 1;
   count[YI] = src->ydimlen;
   std::cout << start[XI] << "\t" << start[YI] << "\t" << count[XI] << "\t" << count[YI] << "\n";

   // Allocate memory for the arrays
   array = new float[count[YI] * count[XI]];
   if(array == NULL) {
      exit(19);
   }

   // Extract the data from the netcdf source file.
   err = nc_get_vara_float(src->ncid, src->zid, start, count, array);


   if(X_INDEX_FIRST){
      zValues = zGrid(array, src->ydimlen, src->xdimlen, true);
      double* temp = *xValues;
      (*xValues) = *yValues;
      (*yValues) = temp;
   }else{
      zValues = zGrid(array, src->xdimlen, src->ydimlen, false);
   }

   return 0;
}
