#ifndef TerrainMapDEM_H

#include "TerrainMap.h"
#include "mapio.h"

#include "structDefs.h"

struct refMapT{
	mapbounds* bounds;
	mapsrc* src;
	mapsrc* varSrc;
	mapsrc* lowResSrc;

	refMapT(){
		src = NULL;
		varSrc = NULL;
		lowResSrc = NULL;
		bounds = NULL;
	}
	
	~refMapT() { clean(); }
	
	void clean(){
		if(src!=NULL){
			mapsrc_free(src);
			src = NULL;
		}

		if(varSrc!=NULL){
			mapsrc_free(varSrc);
			varSrc = NULL;
		}

		if(lowResSrc!=NULL){
			mapsrc_free(lowResSrc);
			lowResSrc = NULL;
		}

		if(bounds != NULL){
			free(bounds);
			bounds = NULL;
		}
	}
};


/*
TerrainMapDEM pulls together the functionality written for DEM maps into a single class and makes 
it work through the same interface as the Octree version of TerrainMap.

Documentation form TNavFilter and ParticleFilter is grouped together at the bottom of the file.
*/

class TerrainMapDEM : public TerrainMap{
	public:
		double GetRangeError(double& mapVariance, const double* const startPoint, const double* const directionVector, double expectedDistance);
		//double QueryMap(double const * const queryPoint);
		
		int loadSubMap(const double xcen, const double ycen, double* mapWidth,
			       double vehN, double vehE);
		
		TerrainMapDEM(const char* mapName);
		~TerrainMapDEM();
		
		//functionality moved from TNavFilter or TNavParticleFilter
		bool withinRefMap(const double northPos, const double eastPos);
		bool withinValidMapRegion(const double north, const double east);
		bool withinSubMap(const double northPos, const double eastPos);
		
		void setLowResMap(const char* mapName);
		bool GetMapT(mapT& currMap);
		bool GetMapBounds(double* currMapBounds);
		
		double Getdx(void){return refMap->bounds->dx;}
		double Getdy(void){return refMap->bounds->dy;}
		
	private:
		bool computeMapRayIntersection(const double* position, double *u, double& r, double &var);   
		void interpolateDepth(double xi, double yi, double &zi, double &var);
		double getNearestLowResMapPoint(const double north, const double east, double& nearestNorth, double& nearestEast);
		double computeInterpDepthVariance(int* xIndices, int* yIndices, ColumnVector Weights);
		
		void setRefMap(const char *mapName);
		int extractSubMap(const double north, const double east, double* mapParams);
		void convertMapdataToMapT(mapdata* currMapStruct);
		int extractVarMap(const double north, const double east, double* mapParams);
	
	private:
		//were public
		mapT map;
		refMapT* refMap;
		
		
		
	public:
		void interpolateGradient(double xi, double yi, Matrix& gradient);
		void computeInterpTerrainGradient(int* xIndices, int* yIndices, double xi, double yi, Matrix& gradient);
		void interpolateDepthMat(double* xi, double* yi, Matrix& zi, Matrix& var);
		
		
		
		
		
	
};



#endif




//TODO: The method used here is similar to Newton Raphson and could run
  //      indefinitely and also may miss the FIRST point of intersection.
  //      Currently only used in TNavParticleFilter
	//			Put this in TerrainMap
  /* Helper Function:computeMapRayIntersection
   * Usage: computeMapRayIntersection(position, beamsMF, r, var)
   * -------------------------------------------------------------------------*/
  /*! This function computes the range associated with the intersection of the 
   * direction vector, u, emanating from the 3D location position, with the
   * current extracted map, terrainMap->map.  In addition to the range, the 
   * function fills in the variable var which specifies the variance of the 
   * intersected map location. The function returns a boolean indicating if 
   * the map-ray intersection was successful, i.e. if a valid map value was 
   * found for the intersection point.
   */
  //bool computeMapRayIntersection(const double* position, double *u, double &r,
	//			 double &var);   

	//TODO: This should go to TerrainMap
  /* Helper Function: interpolateDepth
   * Usage: interpolateDepth(xi, yi, zi, variance)
   * -------------------------------------------------------------------------*/
  /*! This function is used to interpolate a depth value, zi, from the current
   * extracted map terrainMap->map.  In addition to zi, the function fills in 
   * the variable var (passed by reference) which specificies the variance of 
   * zi. The interpolation is performed according to the method specified in
   * the private variable interpMapMethod. 0: nearest-neighbor, 1: bilinear,
   * 2: bicubic, 3: spline. 
   */
  //void interpolateDepth(double xi, double yi, double &zi, double &var);



	//TODO: This is only used in TNavPointMassFilter, can probably be axed if
  //      that is changed
	//			This should be in TerrainMap	
  /* Usage: interpolateDepthMat(xi, yi, zi, variance)
   * -------------------------------------------------------------------------*/
  /*! This function is used to interpolate a matrix of depth values, zi, 
   * from the current extracted map terrainMap->map.  The corresponding location
   * of each entry in zi is given by the pairs (xi, yi).
   * In addition to zi, the function fills in the matrix variable var 
   * (passed by reference) which specificies the variance of each point in
   * zi. The interpolation is performed according to the method specified in
   * the private variable interpMapMethod. 0: nearest-neighbor, 1: bilinear,
   * 2: bicubic, 3: spline. 
   */
  //void interpolateDepthMat(double* xi, double* yi, Matrix &zi, Matrix &var);


//TODO:	This should be in TerrainMap	
  /* Usage: var = computeInterpDepthVariance(xi, yi, zi, xInd, yInd, W)
   * -------------------------------------------------------------------------*/
  /*! This function is used to assess the variance of a terrain depth value, zi,
   * computed using an interpolation method.  xInd and yInd are Nx1 integer 
   * arrays of indices into the current extracted map, terrainMap->map, 
   * indicating the map depths used in the interpolation.  Weights is an Nx1 
   * matrix indicating the interpolation weights associated with each of the
   * map points specified in xInd, yInd.  It returns a single variance.
   */
  //double computeInterpDepthVariance(int* xIndices, int* yIndices,
	//			    ColumnVector Weights);


	//TODO:	This should be in TerrainMap	
  /* Usage: interpolateGradient(xi, yi, gradient)
   * -------------------------------------------------------------------------*/
  /*! This function is used to calculate the interpolated local terrain gradient
   * at the (north,east) point (xi,yi), using the current extracted map, 
   * terrainMap->map.  The computed gradient is returned in the pass by 
   * reference variable, which should be a matrix of size 1x2.
   */
  //void interpolateGradient(double xi, double yi, Matrix &gradient);


	//TODO:	This should be in TerrainMap	
	//			Why have this and the two preceding functions??
  /* Usage: interpolateDepthAndGradient(xi, yi, zi, var, gradient)
   * -------------------------------------------------------------------------*/
  /*! This function is used to calculate the interpolated depth along with the 
   * local terrain gradient at the (north,east) point (xi,yi), using the current
   * extracted map, terrainMap->map.  The interpolated depth, zi, and the 
   * computed gradient are returned in the pass by reference variables. The 
   * gradient matrix should have size 1x2.
   */
  //void interpolateDepthAndGradient(double xi, double yi, double &zi,
	//			   double &var, Matrix &gradient);


	//TODO:	This should be in TerrainMap	
	//			Is this redundant?	
  /* Usage: computeInterpTerrainGradient(xIndices, yIndices, xi, yi, gradient)
   * -------------------------------------------------------------------------*/
  /*! This function is used to calculate the local terrain gradient at the 
   * (north,east) point (xi,yi), using the current extracted map, 
   * terrainMap->map.  The terrain gradient is computed based on the 
   * interpolation method.  The inputs xIndices and yIndices are Nx1 integer 
   * arrays into the extracted map, indicating the map depths used in the 
   * interpolation.  Currently, this function only differentiates bilinear 
   * interpolation.  If the interpolation method is not bilinear, the 
   * gradients are computed using forward/backward/or central differencing.  
   * The gradients are returned in the 1x2 matrix gradient, which is passed by 
   * reference.  
   */
  //void computeInterpTerrainGradient(int* xIndices, int* yIndices, 
	//			    double xi, double yi,
		//		    Matrix &gradient);

