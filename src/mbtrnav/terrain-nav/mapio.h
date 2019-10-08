/* 
 * File:   mapio.h
 * Author: Brian Schlining
 *
 * Created on August 30, 2007, 9:04 PM
 *
 */

#ifndef NAN
#define NAN 0
#endif
 
/*!
 * @author Copyright 2007 MBARI. All rights reserved.
 * @date 2007-09-11
 *
 * @details MAPIO is currently written to read a netCDF file (*GMT* version 2 GRD file) in UTM
 * coordinates. (Generated with the -G3 flag in MBSsystem). 
 *
 * Here's an example CDL for the GMT version 2 netcdf grids we'll be working with:
 *
 <pre>
 netcdf canyonDataUTMG3 {
 dimensions:
 x = 1022 ;
 y = 879 ;
 variables:
 double x(x) ;
 x:long_name = "Easting (meters)" ;
 x:actual_range = 597413.876506377, 598435.621004263 ;
 double y(y) ;
 y:long_name = "Northing (meters)" ;
 y:actual_range = 4071749.4760972, 4072627.48862688 ;
 float z(y, x) ;
 z:long_name = "Depth (m)" ;
 z:_FillValue = nanf ;
 z:actual_range = 374.4248f, 536.1101f ;
 
 // global attributes:
 :Conventions = "COARDS" ;
 :title = "Bathymetry Grid" ;
 :description = "\n",
 "\tProjection: UTM10N\n",
 "\tGrid created by mbgrid\n",
 "\tMB-system Version 5.1.0\n",
 "\tRun by <root> on <JacksonHole.stanford.edu> at <Thu Aug 30 00:44:23 2007>" ;
 :node_offset = 0 ;
 }
 </pre>
 * 
 * For this code, the projection is assumed to be UTM. We are ignoring the zone;
 * you'll have to be smart enough to make sure all coordinates are within the
 * same zone as the GRD file you are using.
 */


#ifndef _MAPIO_H
#define	_MAPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <netcdf.h>

/*!
 * @brief If not equal to zero then debug output will be printed.
 */
#define MAPIO_DEBUG 0

#define MAPIO_OK 1

#define MAPIO_READERROR 2

#define MAPIO_OUTOFMEMORY 4

#define MAPSRC_IS_EMPTY 0

#define MAPSRC_IS_FILLED 1

#define MAPSRC_FILL_FAILURE 2

#define MAPDATA_IS_EMPTY 0

#define MAPDATA_IS_FILLED 1

#define MAPDATA_FILL_FAILURE 2

#define MAPBOUNDS_OK 0

#define MAPBOUNDS_OUT_OF_BOUNDS 1

#define MAPBOUNDS_NEAR_EDGE 2

 
/*!
 * @struct mapdata
 * @brief Stores a submap of the netcdf GRD data
 * @param xcenter The X (Easting in meters) center coordinate provided by user
 * @param ycenter The Y (Northing in meters) center coordinate provided by user
 * @param xpts A pointer to a 1D array of the easting coordinate variable data
 * @param xdimlen The number of values in x
 * @param ypts A pointer to a 1D array of the northing coordinate variable data
 * @param ydimlen The number of values in y
 * @param z A pointer to a submap sized (ydimlen, xdimlen)
 * @param status THe status of the structure
 * @details This structure stores a submap retrieved from a GMT GRD file. 
 * Functions for working with this file are mapdata_fill and mapdata_free. Here's
 * a code snippet that shows accessing the array:
 <pre>
 // struct mapdata data
 float *z;
 float value;
 int i, j;
 
 z = data->z;
 i = data->ydimlen;
 j = data->xdimlen;
 for (i = 0; i < rows; i++) {
 for (j = 0; j < columns; j++) {
 value = z[i * columns + j];
 fprintf(stdout, "%f\t", value);
 }
 fprintf(stdout, "\n");
 }
 </pre>
*/
struct mapdata {
  double xcenter;
  double ycenter;
  double *xpts;          // Double array of x data for coordinates of z
  size_t xdimlen;     // Length of X dimension
  double *ypts;          // Double array of y data for coordinates of z
  size_t ydimlen;     // Length of Y dimension
  float *z;          
  int status;
};

/*!
 * @struct mapsrc
 * @brief Stores info needed for accessing GMT GRD file
 * @param ncid The NetCDF file id
 * @param x Double Array of ALL the easting (x-axis) data
 * @param xid The NetCDF variable id to the easting variable
 * @param xdimid The NetCDF dimension id for the easting variable
 * @param xdimlen The number of values in x
 * @param y Double array of all the northing (y-axis) data
 * @param yid The NetCDF variable id to the northing variable
 * @param ydimid The NetCDF dimension id for the northing variable
 * @param ydimlen The number of values in y
 * @param zid The NetCDF variable id to the height/depth variable
 * @param status
 */
struct mapsrc {
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
};

/*!
 * @struct mapbounds
 * @brief Stores info about the boundaries of a GMT GRD file. This
 * information can be used to determine if a location is within the 
 * bounds of an existing NetCDF GRD file.
 * @param ncid The NetCDF file id
 * @param xmin Minimum value in NetCDF X variable
 * @param xmax Maximum value in NetCDF X variable
 * @param dx Nominal pixel size, in meters, of Y axis
 * @param ymin Minimum value in NetCDF Y variable
 * @param ymax Maximum value in NetCDF Y variable
 * @param dy Nominal pixel size, in meters, of Y axis
 */
struct mapbounds {
  int ncid;           // NetCDF file id
  double xmin;        // Minimum value in NetCDF X variable
  double xmax;        // Maximum value in NetCDF X variable
  double dx;          // Nominal pixle size in meters of X variable
  double ymin;        // Minimum value in NetCDF Y variable
  double ymax;        // Minimum value in NetCDFR Y variable
  double dy;          // Nominal pixel size in meters of Y variable
};

int check_error(int status, struct mapsrc *src);

/*!
 * function: mapsrc_fill
 * @brief Opens a GMT GRD file and read coordinate variables
 * @details Opens a GMT version 2 GRD file (netcdf). It stores the 
 *     needed information, such as ids to the netcdf file, variables, and the
 *     data for the x and y coordinate variables. Note that 'struct mapsrc'
 *     contains references to  arrays which must be released. If an error occurs
 *     on fill a flag will be set in the mapsrc.status field.
 * @param file The path of the file to read.
 * @param src The mapsrc structure for storing the information about the netcdf
 *     data file.
 */
void mapsrc_fill(const char *file, struct mapsrc *src);

/*!
 * function: mapsrc_free
 * @brief Frees the memory used by a mapsrc structure
 * @details A mapsrc structure references to arrays. This function will
 *  free those arrays as well as the parent structure.
 * @param src Is the mapsrc structure to be freed
 */
void mapsrc_free(struct mapsrc *src);

/*!
 * function: mapsrc_find
 * @brief Find a z value at a given point
 * @details Find a z-value in a map at a given x, y location
 * @param src The mapsrc structure pointing to the map you want to read
 * @param x The x coordinate in meters UTM (use same zone as netCDF file).
 * @param y The y coordinate in meters UTM (use same zone as netCDF file).
 * @return The z value in meters of the point nearest to the x, y location
 *      NAN is returned if the x,y coordinate falls outside the
 *      boundaries of the map.
 *
 */
 
float mapsrc_find(struct mapsrc *src, double x, double y);

/*!
 * function: mapsrc_init
 * @brief Initializes a mapsrc structure
 * @details Allocates memory for a mapsrc structure and sets the status flags.
 * You will need to call 'mapsrc_free' on the structure to release it's memory 
 * when you are finished with it.
 * @result A mapsrc structure. 
 */
struct mapsrc *mapsrc_init(void);

/*!
 * function: mapsrc_tostring
 * @brief create a string of a mapsrc structure
 * @param src The mapsrc structure
 * @return A string. Don't forget to free it when you're done.
 */
char *mapsrc_tostring(struct mapsrc *src);

/*!
 * function: mapdata_fill
 * @brief Read a submap from a netcdf UTM GRD file into a mapdata structure
 * @details Reads in a rectangular block of bathymetry data from a file (defined by src). The
 * size of the rectangle is based on the params but will NOT be exaclty as specified by xwidth and ywidth.
 * @param src The mapsrc structure definining where we are reading data from
 * @param data A mapdata structure for storing the submap
 * @param x The easting position of the vehicle. In the same UTM coordinate system as the src map.
 * @param y The northing position of the vehicle. In the same UTM coordinate system as the src map.
 * @param xwidth The width of the submap in meters to retrive. 
 * @param ywidth The width of the submap in meters to retrieve
 * @result An integer code of MAPBOUNDS_OK if all is OK, MAPBOUNDS_OUT_OF_BOUNDS if the
 *      x or y coordinate is outside the boundaries fo the map reference by src, or
 *      MAPBOUNDS_NEAR_EDGE if you x,y position is near the edge (resulting in a truncated
 *      submap)
 */
int mapdata_fill(struct mapsrc *src, struct mapdata *data, double x, double y, double xwidth, double ywidth);

/*!
 * function: mapdata_free
 * @brief Free's the memory held by a mapdata structure
 * @param data The mapdata structure who's memory we want to free
 * @param free_all If equal to 0 then the structure itself will not be freed, only
 *      the internal pointers. All structure values will be set to 0. if not
 *      equal to 1 then the data structure's memory will be release.
 */
void mapdata_free(struct mapdata *data, int free_all);

/*!
 * function: mapdata_tostring
 * @brief create a string of a mapdata structure
 * @param data The mapdata structure
 * @return A string. Don't forget to free it when you're done.
 */
char *mapdata_tostring(struct mapdata *data);

/*!
 * function: nearest
 * @brief Find the index of the nearest value
 * @details Finds the index of the nearest value in a double array to a key value.
 *      this is a naive implementation but should get us going.
 * @param key The key value
 * @param base The double array that you're searcng to find the nearest value in. This 
 *      implementation of nearest assumes the arrays are sorted ascending.
 * @param nmemb The number of elements in the base array
 */
int nearest(double key, const double *base, size_t nmemb);
void z_print(const float *z, int rows, int columns);

float getZ(const float *z, int row, int column, int columns);

/*!
 * function: mapdata_check
 * @brief Returns a code indicating the status of the mapdata
 * @details TODO
 * @param data THe mapdata structure
 * @return An integer indicating the status of the data structure
 * 
 */
int mapdata_check(struct mapdata *data, struct mapsrc *src, double xcenter, double ycenter, double xwidth, double ywidth);

/*!
 * function: mapbounds_init
 * @brief Create a mapbounds struct
 * @details Allocates memory for a mapbounds struct as sets all values to 0.
 *      The structure's memory will need to be freed when done with the structure.
 * @return A mapbounds structure
 */
struct mapbounds *mapbounds_init();

/*!
 * function: mapbounds_fill1
 * @brief Fills a mapbounds structure from a mapsrc object
 * @details Fills in a mapbounds structure based on info in the src stucture
 * @param src The mapsrc data structure
 * @param bounds The mapbounds data structure to populate
 * @return TODO...return a meaninful error code
 */
int mapbounds_fill1(struct mapsrc *src, struct mapbounds *bounds);

/*!
 * function: mapbounds_fill2
 * @brief Fill in a mapbounds structure from a netcdf file
 * @details Fills in a maounds structure based on data in the netcdf file
 */
int mapbounds_fill2(const char *file, struct mapbounds *bounds);

/*!
 * function: mapbounds_tostring
 * @brief Return a string representation of a mapbounds struct
 * @details String rep of a mapbounds struct
 * @param bounds A mapbounds struct
 * @return A char*. Don't forget to free it when you're done with it
 */
char *mapbounds_tostring(struct mapbounds *bounds);

int mapbounds_contains(struct mapbounds *bounds, const double x, const double y);
 
int mapdata_checksize(struct mapbounds *bounds, struct mapdata *data, const double xwidth, const double ywidth);

#ifdef	__cplusplus
extern "C" {
#endif
  
  
  
  
#ifdef	__cplusplus
}
#endif

#endif	/* _MAPIO_H */

