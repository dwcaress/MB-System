#include <libgen.h>
#include <cmath>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>


#include "TerrainMapOctree.h"
#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include "genFilterDefs.h"
#include "structDefs.h"
#include "trn_log.h"

double
TerrainMapOctree::
GetRangeError(double& mapVariance, const double* const startPoint, const double* const directionVector, double expectedDistance) {
	Vector octreeVectorStartPoint (startPoint[0], startPoint[1], startPoint[2]);
	Vector octreeVectorDirectionVector (directionVector[0], directionVector[1], directionVector[2]);
	
	//TODO work out variance properly 
	mapVariance = OctreeMap->GetTrueResolution().Norm()/1.0;//1.73;//3.4641 is 2*sqrt(3)
	
	
	double predictedDistance = OctreeMap->RayTrace(octreeVectorStartPoint, octreeVectorDirectionVector);
	if(predictedDistance == -1){
		//missed the map
		return NAN;
	}
	return expectedDistance - predictedDistance;
}
/*
double
TerrainMapOctree::
QueryMap(const double* const queryPoint) {
	Vector queryPoint = 
	//return static_cast<double>(OctreeMap->Query(queryPoint));
	return OctreeMap->InterpolatingQuery(queryPoint);
}
*/

// Tile loading test utility. Call after successful call
// to initializeTiles(const char *mapname). Returns false
// if any of the tiles fails to load. Otherwise returns true.
// All tiles in the array are tested.
// 
bool
TerrainMapOctree::
tileLoadTest()
{
   // load and unload here to catch a possible corrupted or
   // malformed map file. This can be used in a offline utility
   // to ensure that all tiles will load (not corrupted).
   // It can take quite some time to execute, depending on the
   // number of tiles, so it is not recommended for use in
   // mission applications.
   // 

   bool value = true;
   for( int i = 0; i < numTiles_; i++ )
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::pre-load of tile %s ...\n",
	         tiles_[i].mapName);
      if (tiles_[i].load())
      {
	 tiles_[i].octreeMap->Print();
	 tiles_[i].unload();
      }
      else
      {
	      logs(TL_LOG|TL_SERR,"TerrainMapOctree::pre-load of tile %s failed\n",
	         tiles_[i].mapName);
	      value = false;
      }
   }
   return value;
}

bool
TerrainMapOctree::
initializeTiles(const char* mapName)
{
   // Initialize number of tiles
   // 
   numTiles_ = 0;

   // Caller must provide a map name
   // 
   if (NULL == mapName)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::no mapName given\n");
      return false;
   }

   // Does mapName exist?
   // 
   if( !open( mapName, O_EXCL ) )
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::mapName = %s doesn't exist.\n", mapName);
      return false;
   }
   
   // If mapname is a directory load the tiles in the "tiles.csv" file,
   // Otherwise create one tile with just the mapname and then return "true".  
   //
   if( !opendir( mapName ) )
   {
   	// mapName is not a directory - load one tile, one tile only
   	// 
      numTiles_ = 1;
      tiles_ = new struct MapTile;
      tiles_[0].mapName = STRDUPNULL(mapName);
      tiles_[0].octreeMap = NULL;
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::Using a single (non-tiled) map.\n");
      return true;
   }

   
   int charLen = 512;
   char tileDataName[charLen];
   char tileName[charLen];
   char record[charLen];
   char header[128];
   char comma;
   strncpy( tileDataName, mapName, charLen );
   strcat(  tileDataName, "/tiles.csv" );
   logs(TL_LOG|TL_SERR,"TerrainMapOctree::tileDataName = %s\n", tileDataName);

   // Open tiles.csv containing the tile info
   // 
   if( !open( tileDataName, O_EXCL ) )
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::tileDataName = %s doesn't exist.\n", tileDataName);
      return false;
   }


	// Read the header line including the number of tiles in the list
	//    
   fstream f(tileDataName);
   f >> header >> comma  >> header >> comma >> header >> comma >> numTiles_;
   logs(TL_LOG|TL_SERR,"TerrainMapOctree::numTiles_ = %d.\n", numTiles_);

   // Got to have at least one tile in the list
   // 
   if( numTiles_ < 1 )
   {
      return false;
   }

   // Each line in the file has the following format:
   // relativeTileFilename , center-easting , center-northing 
   //  
   tiles_ = new struct MapTile[numTiles_];
   for( int i = 0; i < numTiles_; i++ )
   {
      f >> tileName >> comma  >> tiles_[i].easting >> comma >> tiles_[i].northing;
      sprintf( record, "%s/%s", mapName, tileName );

      tiles_[i].mapName = STRDUPNULL(record);
      tiles_[i].octreeMap = NULL;
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::read tile line: %s %.1f %.1f .\n",
			   tiles_[i].mapName, tiles_[i].northing, tiles_[i].easting);

   }

   f.close();
   lastMinDistTile_ = 0;
   return true;
}



TerrainMapOctree::
TerrainMapOctree(const char* mapName) {
   //OctreeMap = Octree<PlanarFitNode>();
   //OctreeMap1 = Octree<bool>();
   //OctreeMap = &OctreeMap1;

   // Set up the list of tiles. If mapName refers to just a regular
   // octree map file, one tile will be created. Otherwise, an array
   // of tile is created and used.
   // 
   if( !initializeTiles(mapName) )
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree Load Failed for tile %s.\n", mapName);
      throw Exception("TerrainMapOctree - Error loading tile file");
   }

   logs(TL_LOG|TL_SERR,"TerrainMapOctree::Now loading %s.\n", tiles_[0].mapName);
   //
   // Load the first one.
   //
   clock_t begin = clock();
   bool loaded = tiles_[0].load();
   clock_t end = clock();
   double duration = (double)(end - begin)/CLOCKS_PER_SEC;
   logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree tile load took %f seconds.\n", duration);
   
   if(!loaded)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree Load Failed for %s.\n", tiles_[0].mapName);
      //fprintf(stderr,"\nOctree Load Failed\n");
      throw Exception("TerrainMapOctree - Error loading map file");
   }

   OctreeMap = tiles_[0].octreeMap;
   OctreeMap->Print();

}

int
TerrainMapOctree::
loadSubMap(const double xcen, const double ycen, double* mapWidth, double vehN, double vehE)
{
	// Gotta have at least two tiles to even bother with this stuff.
	// Return now unless there are 2 or more tiles.
	// 
   if( numTiles_ < 2)
   {
   	return MAPBOUNDS_OK;
   }

   logs(TL_LOG|TL_SERR,"TerrainMapOctree:   (vehN, vehE )  =  (%.2f, %.2f).\n", vehN, vehE);

   
   int i;
   double minDist = 1e8;
   double distance;

   // Compute distance from the vehicle to each of the tile map centers.  Select the smallest.
   for( i=0; i<numTiles_; i++ )
   {
     distance =  sqrt( pow( (vehN - tiles_[i].northing), 2 ) + pow( (vehE - tiles_[i].easting), 2 ) );
     //logs(TL_LOG|TL_SERR,"TerrainMapOctree:   Distance to tile%d = %.2f.\n", i+1, distance);

     if( distance < minDist )
     {
       minDistTile_ = i;
       minDist = distance;
     }
   }
   logs(TL_LOG|TL_SERR,"TerrainMapOctree:  Min Distance = %.2f.\n", minDist);
   logs(TL_LOG|TL_SERR,"TerrainMapOctree:  Using tile %d.\n", minDistTile_ + 1);

   // When the closest center location is in another tile, make the switch
   // 
   if( lastMinDistTile_ != minDistTile_ )
   {

		//
		// First unload the last tile:
		tiles_[lastMinDistTile_].unload();

		logs(TL_LOG|TL_SERR,"TerrainMapOctree:  Switching to tile %d.\n", minDistTile_ + 1);

		//
		// Now load the new tile:
		logs(TL_LOG|TL_SERR,"TerrainMapOctree::Now loading %s.\n",
		        tiles_[minDistTile_].mapName);

		clock_t begin = clock();
		bool loaded = tiles_[minDistTile_].load();
		clock_t end = clock();
		double duration = (double)(end - begin)/CLOCKS_PER_SEC;
		logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree tile load took %f seconds.\n", duration);

		if(!loaded)
		{
			// We're kind of screwed if the map doesn't load, so throw
			// an exception here. Another option is to reload the old file,
			// assuming just the new file is corrupted.
			// 
			logs(TL_LOG|TL_SERR,"TerrainMapOctree:  Octree Load Failed for %s.\n",
                    tiles_[minDistTile_].mapName);
			throw Exception("TerrainMapOctree - Error loading map file.");
		}

		//
		// Switch the pointer and we're ready to use
		//
		lastMinDistTile_ = minDistTile_;
		OctreeMap = tiles_[minDistTile_].octreeMap;
		OctreeMap->Print();
       
   }

   return MAPBOUNDS_OK;
}


TerrainMapOctree::
~TerrainMapOctree() {
	if (tiles_)
	{
		for (int i = 0; i < numTiles_; i++)
		{
         if (tiles_[i].octreeMap) tiles_[i].unload();
			if (tiles_[i].mapName) delete tiles_[i].mapName;
		}
		delete [] tiles_;
		tiles_ = NULL;
	}
	numTiles_ = 0;
}

bool
TerrainMapOctree::
withinRefMap(const double northPos, const double eastPos){
	Vector LowerBounds = OctreeMap->GetLowerBounds();
	Vector UpperBounds = OctreeMap->GetUpperBounds();
	
	return ((northPos < UpperBounds.x)
		&& (northPos > LowerBounds.x)
		&& (eastPos < UpperBounds.y)
		&& (eastPos > LowerBounds.y));
}

bool 
TerrainMapOctree::
withinValidMapRegion(const double northPos, const double eastPos){
	return withinRefMap(northPos, eastPos);
}

bool 
TerrainMapOctree::
withinSubMap(const double northPos, const double eastPos){
	return withinRefMap(northPos, eastPos);
}

bool 
TerrainMapOctree::
GetMapT(mapT& currMap){
	fprintf(stderr,"\n\nGetMapT not implemented\n\n");
	return false;
}

bool 
TerrainMapOctree::
GetMapBounds(double* currMapBounds){
	fprintf(stderr,"\n\nGetMapBounds not implemented\n\n");
	return false;
}


