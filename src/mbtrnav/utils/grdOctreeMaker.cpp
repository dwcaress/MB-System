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


#include <netcdf.h>
#include "mapio.h"
#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define X_INDEX_FIRST 1
//#define RESOLUTION -1
#define RESOLUTION 1
#define FILL_NUMBER 2

#define NORTH_BOUND -1//5094336
#define SOUTH_BOUND -1//5084336
#define EAST_BOUND  -1//591526
#define WEST_BOUND  -1//587430
#define MIN_ACCEPTED_DEPTH  1//1000
#define MAX_ACCEPTED_DEPTH  5000

#define OUTFILE "temp.bo"
//#define INFILE  "/home/david/ARL/SharedWithVM/mapsfromRock/Topo_UpperCanyonTesting_2m_Wall_UTM_STEVE2.grd"
//#define INFILE  "/home/david/ARL/SharedWithVM/mapsfromRock/Topo_UpperCanyonTesting_2m_UTM.grd"
//#define INFILE "/home/david/ARL/mbariMaps/PortugueseLedge/PortugueseLedge20080424TopoUTM_NoNan.grd"
//#define INFILE "/media/sf_SharedWithVM/planer-fit/Map/AxialEM_post2015_AUV2015Bounds_TopoUTM5m.grd"
#define INFILE "/media/sf_SharedWithVM/planer-fit/Map/IcebergTestWall/IcebergTestWall_Topo1mUTM.grd"

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

int setupXYZ(double** xValues, double** yValues, zGrid& zValues, char *inFile );

int main( int argc, char **argv ) 
{
   zGrid zValues;
   double *xValues = NULL, *yValues = NULL;

   long north_bound = -1;
   long south_bound = -1;
   long  east_bound = -1;
   long  west_bound = -1;

   // Read input data

   if( argc < 2 )
   {
      printf("Please supply a file name, without the suffix.\n");
      return 1;
   }
	
   char inFile[128], outFile[128];
   snprintf(inFile, 128, "%s.grd", argv[1]);
   snprintf(outFile, 128, "%s.bo", argv[1]);
   
   //struct stat buf;
   //if( stat(inFile, &buf) == 0 )
   if( open( inFile, O_EXCL) == 0 )
   {
      printf("File %s not found.\n", inFile);
      return 1;
   }

   if( argc > 2 )
   {
      for (int i = 1; i < argc; i++)
      {
	 if( !strcmp(argv[i], "-R") )
	 {
	    if( i < (argc - 4) )
	    {
	       east_bound  = atol(argv[i+1]);
	       west_bound  = atol(argv[i+2]);
	       south_bound = atol(argv[i+3]);
	       north_bound = atol(argv[i+4]);
	    }
	    else
	    {
	       printf("Error:  You must supply four arguments after -R, with a space after -R.\n");
	       return 1;
	    }
	 }
	 else if( !strcmp(argv[i], "-G") )
	 {
	    if( i < (argc - 1) )
	    {
	       snprintf(outFile, 128, "%s.bo", argv[i+1]);
	    }
	    else
	    {
	       printf("Error:  You must supply a file name after -G.\n");
	       return 1;
	    }
	 }
      }
   }

   //Yes I mean '=' not '==' DO NOT CHANGE IT.	
   if(int temp = setupXYZ(&xValues, &yValues, zValues, inFile)){return temp;}
	
   std::cout << "X[0]: " << xValues[0] << std::endl;
   std::cout << "Y[0]: " << yValues[0] << std::endl;
   std::cout << "Z dimensions and order:\n";
   zValues.Print();
	
   //Auto detect X and Y resolution or use input
	
   Vector DesiredResolution(RESOLUTION,RESOLUTION,RESOLUTION);
	
   if(RESOLUTION == -1){
      DesiredResolution.x = xValues[1] - xValues[0];
      DesiredResolution.y = yValues[1] - yValues[0];
      DesiredResolution.z = 1;
   }
	
   std::cout << "Detecting point cloud size:\n";
	
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
   std::cout << uninitialized << "\n\n";
   std::cout << "Lowermost: ";
   Lowermost.Print();
   std::cout << "Uppermost: ";
   Uppermost.Print();
	
   Vector PointCloudSize = Uppermost - Lowermost + Vector(1.0, 1.0, 1.0);
   Vector OctreeSize = DesiredResolution;
   printf("PointCloudSize\t");
   PointCloudSize.Print();
   while(!OctreeSize.StrictlyGreaterOrEqualTo(PointCloudSize)){
      OctreeSize *= 2.0;
      printf("OctreeSize\t");
      OctreeSize.Print();
   }
   Vector LowerBounds = Lowermost - DesiredResolution *0.5;
   Vector UpperBounds = LowerBounds + OctreeSize;
	
   //initialize octree
   printf("about to build Octree\n");
   Octree<bool> newOctreeMap(DesiredResolution + Vector(0.001,0.001,0.001), LowerBounds, UpperBounds,
			     OctreeType::BinaryOccupancy);
	
   newOctreeMap.Print();
   Vector tempPoint;
   // Add points
   std::cout << "adding points\nrow\t# added\tLast Point Tested\n";
   int countPointsAdded = 0;
   for(unsigned int xIndex = 0; xIndex < zValues.numXValues; xIndex++){
      for(unsigned int yIndex = 0; yIndex < zValues.numYValues; yIndex++){
	 if(zValues.getZ(xIndex, yIndex) != 99999 && !isnan(zValues.getZ(xIndex, yIndex))){
	    if(zValues.getZ(xIndex, yIndex) <= MAX_ACCEPTED_DEPTH){
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
					
	       /*
	       //if((4072252 > point.x) & (4072250 < point.x ) & (588730 < point.y) & (588732 > point.y)){
	       //if((xIndex > 799.9) & (xIndex < 800.1) & (yIndex > 799.9) & (yIndex < 800.1)){
	       if((xIndex == 800) & (yIndex == 800)){
	       //if(countPointsAdded == 21744847){
	       std::cout << "TEST POINT: \n";
	       std::cout << "xindex: " << xIndex << "\tyIndex: " << yIndex << "\n";
	       std::cout << "xInt: " << (int)point.x << "\n";
	       point.Print();
	       tempPoint = point;
	       }
	       */
					
	    }
	 }
      }
      //report progress
      if(xIndex %100 == 0){std::cout << xIndex << "\t" << countPointsAdded << "\t";point.Print();}
   }
   Vector TrueResolution = newOctreeMap.GetTrueResolution();
	
   //fill below input points
   std::cout << "about to fill Octree\n";
   int count = 0;//for progress reporting
	
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
		
	    //report progress
	    if(count %100000 == 0){
	       std::cout << count << std::endl;
	    }
	    count ++;
				
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
   printf("about to collapse\n");
   newOctreeMap.Collapse();
	
   //put any test ray traces you want here. 
   std::cout << "\nTest Ray Traces\n";
   std::cout << newOctreeMap.RayTrace(Vector(south_bound + 2, east_bound - 2,0), Vector(0,0,1)) << "\t";
   std::cout << newOctreeMap.RayTrace(Vector(south_bound + 2, east_bound + 2,0), Vector(0,0,1)) << "\n";
   std::cout << newOctreeMap.RayTrace(Vector(south_bound - 2, east_bound - 2,0), Vector(0,0,1)) << "\t";
   std::cout << newOctreeMap.RayTrace(Vector(south_bound - 2, east_bound + 2,0), Vector(0,0,1)) << "\n";
	
	
	
   std::cout << "\nDone building octree\n";
   //newOctreeMap.SaveToFile(OUTFILE);
   newOctreeMap.SaveToFile(outFile);
   std::cout << "Done\n\n";
	
   /*
   //save corner to corner trace from octree to compare to DEM
   FILE* f = fopen("tempTestTraceOfLatestOctree.txt", "w");
   fprintf(f, "LowerX = %f\nLowerY = %f\nUpperX = %f\nUpperY = %f\n\nZ = [", Lowermost.x, Lowermost.y, Uppermost.x, Uppermost.y);
   for(double i = 0.0; i< 1000.0; i++){
   fprintf(f, "%f, ", newOctreeMap.RayTrace(Vector((Lowermost.x * i/1000.0) + (Uppermost.x * (1000.0 - i) / 1000.0),
   (Lowermost.y * i/1000.0) + (Uppermost.y * (1000.0 - i) / 1000.0),
   0.0),
   Vector(0,0,1)));
   }
   fprintf(f, "];\n");
   fclose(f);
	
   */
	
	
	
   double num = 1000.0;
   FILE* f = fopen("tempTestTraceOfLatestOctree.txt", "w");
   //fprintf(f, "LowerX = %f\nLowerY = %f\nUpperX = %f\nUpperY = %f\n\nZ = [", Lowermost.x, Lowermost.y, Uppermost.x, Uppermost.y);
	
   //Look Down
   //Vector dir(0,0,1);
   //double depth = 0.0;
	
	
	
   //for looking sideways
   Lowermost = Vector(4070168, 589629, 860);
   Uppermost = Vector(4070306, 588843, 860);
   Vector dir(-1,0,0);
   double depth = 890;
	
   for(double i = 0.0; i< num; i++){
      fprintf(f, "%f, ", newOctreeMap.RayTrace(Vector((Lowermost.x * i/num) + (Uppermost.x * (num - i) / num),
						      (Lowermost.y * i/num) + (Uppermost.y * (num - i) / num),
						      depth),
					       dir));
   }
   fprintf(f, "];\n");
   fclose(f);
	
	
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
	
   std::cout << "loading grd\n";
	
   err = nc_open(inFile, NC_NOWRITE, &(src->ncid));
   if(NC_NOERR != err) {
      std::cout << "failed load: " << err << "\n";
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
      //std::cout << *xValues << "\t" << *yValues << "\t" << (*xValues)[0] << "\n";
      double* temp = *xValues;
      (*xValues) = *yValues;
      (*yValues) = temp;
      //std::cout << *xValues << "\t" << *yValues << "\t" << (*xValues)[0] << "\n";
   }else{
      zValues = zGrid(array, src->xdimlen, src->ydimlen, false);
   }
	
   return 0;
}
	
