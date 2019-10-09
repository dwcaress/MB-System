#ifndef TerrainMap_H
#define TerrainMap_H

// Use NINVAL instead of NAN
// NINVAL defined in matrixArrayCalcs.h
// 
//#ifndef NAN
//#define NAN 90000 //this is from matrixArrayCalcs.h definition of isnan(a)
//#endif


#include <cmath>
#include "structDefs.h"

#define VARIOGRAM_FRACTAL_DIM 2.234
#define VARIOGRAM_ALPHA 0.0066
inline double evalVariogram(const double s){return VARIOGRAM_ALPHA * pow(s, 2.0 * (3.0 - VARIOGRAM_FRACTAL_DIM));}


/*
TerrainMap is a an abstract parrent for TerrainMapDEM and TerrainMapOctree.  It's primary purpose is to define a 
common interface for the two map types.  TerrainMapOctree is a wrapper for the Octree class which is documented in Octree.hpp.
TerrainMapDEM has functions from TnavFilter, TnavParticleFilter, and the old TerrainMap.  It pulls together the DEM functionality 
into one class.  Many of the function comments from tNavParticleFilter and TnavFilter are collected at the bottom of TerrainMapDEM.

NOTE: several of the functions listed below are no-ops for the Octree since they are specific to the functionality of DEMs.  
*/


class TerrainMap{
	public:
		virtual ~TerrainMap(void){}
		
		virtual double GetRangeError(double& mapVariance, const double* const startPoint, const double* const directionVector, double expectedDistance) = 0;
		//virtual double QueryMap(double const * const queryPoint) = 0;
		
		virtual int loadSubMap(const double xcen, const double ycen, double* mapWidth,
				       double vehN = -1, double vehE = -1) = 0;
		
		virtual bool withinRefMap(const double northPos, const double eastPos) = 0;
		virtual bool withinValidMapRegion(const double north, const double east) = 0;
		virtual bool withinSubMap(const double northPos, const double eastPos) = 0;
		
		virtual void setLowResMap(const char* mapName) = 0;
		virtual bool GetMapT(mapT& currMap) = 0;
		virtual bool GetMapBounds(double* currMapBounds) = 0;
		
		virtual double Getdx(void) = 0;
		virtual double Getdy(void) = 0;
		
		void setMapInterpMethod(const int &type){this->interpMapMethod = type;}
		int GetInterpMethod(void){ return interpMapMethod; }
		
	protected:
		int interpMapMethod;
};


#endif
