#ifndef TerrainMapOctree_H


#include "TerrainMap.h"
#include "Octree.hpp"

#include "mapio.h"

class PlanarFitNode;
/*
TerrainMapOctree is a wrapper for the Octreeclass to make it useful for TNavFilter.

Several of these functions are DEM specific, and are included here only to standardize the interface for the two map types.
*/

class TerrainMapOctree : public TerrainMap{
	public:
		double GetRangeError(double& mapVariance, const double* const startPoint, const double* const directionVector, double expectedDistance);
		//double QueryMap(const double[3] queryPoint);
		
		
		TerrainMapOctree(const char* mapName);
		~TerrainMapOctree();
		
		//int loadSubMap(const double xcen, const double ycen, double* mapWidth) { return MAPBOUNDS_OK; }
		int loadSubMap(const double xcen, const double ycen, double* mapWidth,
			       double vehN, double vehE);

		bool initializeTiles(const char* mapName);
		bool tileLoadTest();
		
		bool withinRefMap(const double northPos, const double eastPos);
		bool withinValidMapRegion(const double north, const double east);
		bool withinSubMap(const double northPos, const double eastPos);
		
		void setLowResMap(const char* mapName){};//for Octrees, this is a no-op
		bool GetMapT(mapT& currMap);
		bool GetMapBounds(double* currMapBounds);
		
		double Getdx(void){ return OctreeMap->GetTrueResolution().x; }
		double Getdy(void){ return OctreeMap->GetTrueResolution().y; }

		
	private:
		//Octree<PlanarFitNode> OctreeMap;
		Octree<bool> *OctreeMap;
		//Octree<bool> OctreeMap1;
		//Octree<bool> OctreeMap2;
		double northingCenter_, eastEastingCenter_, westEastingCenter_;
		int numTiles_, minDistTile_, lastMinDistTile_;

		struct MapTile
		{
		   Octree<bool> *octreeMap;
		   char *mapName;
		   double northing;
		   double easting;

		   bool load()
		   {
		   	// unload unless the map is NULL
		   	if (octreeMap != NULL) unload();

		   	// Load file mapName
		   	if (mapName != NULL)
		   	{
		   		octreeMap = new Octree<bool>();
		   		return octreeMap->LoadFromFile(mapName);
		   	}
		   	else
		   		return false;
		   }

		   bool unload()
		   {
		   	// Unless the map is already NULL, delete the octree from memory
		   	if (octreeMap != NULL)
		   	{
		   		delete octreeMap;
		   		octreeMap = NULL;
		   	}

		   	return true;
		   }
		};

		MapTile *tiles_;
};

#endif
