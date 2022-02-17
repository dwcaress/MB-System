/*****************************************************************************
 * Copyright (c) 2002-2020 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    TerrainMapOctree.cpp
 * @authors MBARI/Stanford TRN team
 * @date    mm/dd/yyyy
 * @brief   Octree-specific TerrainMap class
 *
 * Project: TRN
 * Summary: See @brief
 *****************************************************************************/

/******************************
 * Headers Imports Requires Uses
 ******************************/

#include <libgen.h>
#include <cmath>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include "TerrainMapOctree.h"
#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include "genFilterDefs.h"
#include "structDefs.h"
#include "trn_log.h"

/******************************
 * Macros
 ******************************/

#define  TILEPATHNAMELEN  512
#define  TILEFILENAMELEN  100
#define  TILEHEADERLEN    128
#define  TILESFILENAME   "tiles.csv"


TerrainMapOctree::TerrainMapOctree(const char* mapName)
:
OctreeMap(NULL),
numTiles_(0),
minDistTile_(0),
lastMinDistTile_(0),
tiles_(NULL)
{
   //OctreeMap = Octree<PlanarFitNode>();
   //OctreeMap1 = Octree<bool>();
   //OctreeMap = &OctreeMap1;
    interpMapMethod=0;

   if (NULL == mapName)
   {
      logs(TL_LOG|TL_SERR,
         "TerrainMapOctree::TerrainMapOctree - NULL ptr to mapName");
      throw Exception("TerrainMapOctree - Error loading tile file");
   }

   // Set up the list of tiles. If mapName refers to just a regular
   // octree map file, one tile will be created. Otherwise, an array
   // of tile is created and used.
   if (!initializeTiles(mapName))
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree Load Failed for tile %s.", mapName);
      throw Exception("TerrainMapOctree - Error loading tile file");
   }

   // Load the first one.
   clock_t begin = clock();
   bool loaded = tiles_[0].load();
   clock_t end = clock();
   double duration = (double)(end - begin)/CLOCKS_PER_SEC;

   if (!loaded)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::Octree Load Failed for %s.",
         tiles_[0].mapName);
      throw Exception("TerrainMapOctree - Error loading map file");
   }

   logs(TL_LOG,"TerrainMapOctree::Octree tile load %s took %f seconds.",
      tiles_[0].mapName, duration);

   OctreeMap = tiles_[0].octreeMap;
   OctreeMap->Print();

}


TerrainMapOctree::~TerrainMapOctree()
{
   if (tiles_)
   {
      for (int i = 0; i < numTiles_; i++)
      {
         if (tiles_[i].octreeMap) tiles_[i].unload();
          if (tiles_[i].mapName) free(tiles_[i].mapName); //delete tiles_[i].mapName;
      }
      delete [] tiles_;
      tiles_ = NULL;
   }
   numTiles_ = 0;
}


double TerrainMapOctree::GetRangeError(double& mapVariance,
   const double* const startPoint, const double* const directionVector,
   double expectedDistance)
{
   // check for null parameters
   if (NULL == startPoint || NULL == directionVector)
   {
      logs(TL_LOG|TL_SERR,
         "TerrainMapOctree::GetRangeError - NULL param: startPoint(%x) directionVector(%x)",
         startPoint, directionVector);
      return NAN;
   }

   Vector octreeVectorStartPoint (startPoint[0], startPoint[1], startPoint[2]);
   Vector octreeVectorDirectionVector (directionVector[0], directionVector[1], directionVector[2]);

   //TODO work out variance properly
   mapVariance = OctreeMap->GetTrueResolution().Norm()/1.0;//1.73;//3.4641 is 2*sqrt(3)

   double predictedDistance = OctreeMap->RayTrace(octreeVectorStartPoint, octreeVectorDirectionVector);
   if (predictedDistance == -1)
   {
      //missed the map
      return NAN;
   }
   return expectedDistance - predictedDistance;
}

#ifdef WITH_QUERYMAP
double TerrainMapOctree::QueryMap(const double* const queryPoint)
{
   //return static_cast<double>(OctreeMap->Query(queryPoint));
   return OctreeMap->InterpolatingQuery(queryPoint);
}
#endif


// Tile loading test utility. Call after successful call
// to initializeTiles(const char *mapname). Returns false
// if any of the tiles fails to load. Otherwise returns true.
// All tiles in the array are tested.
bool TerrainMapOctree::tileLoadTest()
{
   // load and unload here to catch a possible corrupted or
   // malformed map file. This can be used in a offline utility
   // to ensure that all tiles will load (not corrupted).
   // It can take quite some time to execute, depending on the
   // number of tiles, so it is not recommended for use in
   // mission applications.

   bool value = true;
   for (int i = 0; i < numTiles_; i++)
   {
      logs(TL_LOG,"TerrainMapOctree::pre-load of tile %s ...",
            tiles_[i].mapName);
      if (tiles_[i].load())
      {
         tiles_[i].octreeMap->Print();
         if (!tiles_[i].unload())
            logs(TL_LOG|TL_SERR,"TerrainMapOctree::unload of tile %s failed",
               tiles_[i].mapName);
      }
      else
      {
         logs(TL_LOG|TL_SERR,"TerrainMapOctree::pre-load of tile %s failed",
            tiles_[i].mapName);
            value = false;
      }
   }
   return value;
}

// Load tile info into array of MapTile structures.
// If all tile data loads OK, return true. Otherwise false.
//
bool TerrainMapOctree::initializeTiles(const char* mapName)
{
   // Initialize number of tiles
   numTiles_ = 0;
   lastMinDistTile_ = 0;
   int tilesLoaded = 0;

   // First, check gross errors and abort.
   // Caller must provide a map name
   if (NULL == mapName)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree::no mapName given");
      return false;
   }

   // File must exist
   struct stat map_stat;
   if (0 != stat(mapName, &map_stat))
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree:: map file %s not found: %s",
         mapName, strerror(errno));
      return false;
   }

   // The file in mapName exists, attempt to load the map.
   // Is file a directory? Load the tiles in the "tiles.csv" file.
   // Otherwise create a single tile with the mapName.
   if (S_ISREG(map_stat.st_mode))
   {
      // mapName is a regular file - load one tile, one tile only
      numTiles_ = 1;
      tiles_ = new struct MapTile;
      tiles_[0].mapName = STRDUPNULL(mapName);
      tiles_[0].octreeMap = NULL;
      tilesLoaded = 1;
      logs(TL_LOG,"TerrainMapOctree::Using a single (non-tiled) map.");
   }
   else if (S_ISDIR(map_stat.st_mode))
   {
      // mapName is a directory - load the tiles data in tiles.csv
      char tileDataName[TILEPATHNAMELEN];
      char tileName[TILEFILENAMELEN];
      char record[TILEPATHNAMELEN+TILEFILENAMELEN];
      char header[TILEHEADERLEN];
      header[0] = record[0] = tileName[0] = tileDataName[0] = '\0';
      snprintf(tileDataName, TILEPATHNAMELEN, "%s/%s", mapName, TILESFILENAME);
      logs(TL_LOG,"TerrainMapOctree::tileDataName = %s", tileDataName);

      // Open and process tiles.csv containing the tile info
      fstream tf(tileDataName);
      if (tf.is_open())
      {
         char comma = 0;
         // Read the header line including the number of tiles in the list
         // Expected header format is "TileName , Easting , Northing , 9"
         tf >> header >> comma  >> header >> comma >> header >> comma >> numTiles_;
         if (tf.good())
         {
            logs(TL_LOG,"TerrainMapOctree::numTiles_ = %d.", numTiles_);

            // Each line in the file has the following format:
            // relativeTileFilename , center-easting , center-northing
            // Eg, "MapTile4.bo , 594250 , 4061750"
            tiles_ = new struct MapTile[numTiles_];
         }
         else
         {
            logs(TL_LOG,"TerrainMapOctree::numTiles_ = %d.", numTiles_);
            numTiles_ = 0;
         }

         // track the number of actual tiles
         int tn = 0;
         for (int i = 0; i < numTiles_; i++)
         {
            double north = 0.;
            double east  = 0.;
            tileName[0] = '\0';
            tf >> tileName >> comma  >> east >> comma  >> north;

            // Check for premature eof, etc.
            if (!tf.good())
            {
               logs(TL_LOG|TL_SERR,"TerrainMapOctree:: read error: %s",
                     tileDataName);
               break;
            }

            snprintf(record, TILEPATHNAMELEN+TILEFILENAMELEN, "%s/%s",
               mapName, tileName);

            // Problem with tile?
            // skip nameless tiles and - may indicate num tiles error
            struct stat tile_stat;
            if (strlen(tileName) < 1)
            {
               logs(TL_LOG|TL_SERR,"TerrainMapOctree::nameless tile %d.",
                     i+1);
               continue;
            }

            // skip tiles not found or not a regular file
            if (0 != stat(record, &tile_stat) || !(S_ISREG(tile_stat.st_mode)))
            {
               logs(TL_LOG|TL_SERR,"TerrainMapOctree::no access to tile %d: %s.",
                     i, record);
               continue;
            }

            // Tile is OK
            tiles_[tn].mapName  = STRDUPNULL(record);
            tiles_[tn].easting  = east;
            tiles_[tn].northing = north;
            tiles_[tn].octreeMap= NULL;
            logs(TL_LOG,"TerrainMapOctree::read tile line: %s %.1f %.1f .",
                  tiles_[tn].mapName, tiles_[tn].northing, tiles_[tn].easting);
            tn++;
         }

         tilesLoaded = tn;
         logs(TL_LOG,"TerrainMapOctree::loaded %d useable tiles from %s.",
               tilesLoaded, tileDataName);

         tf.close();
      }
      else
      {
         // failed to open tiles.csv
         logs(TL_LOG|TL_SERR,"TerrainMapOctree:: cannot open %s: %s",
            tileDataName, strerror(errno));
         numTiles_ = 0;
      }
   }
   else
   {
      // failed S_ISDIR and S_ISREG tests
      logs(TL_LOG|TL_SERR,
         "TerrainMapOctree::%s is not a regular file or a directory", mapName);
      numTiles_ = 0;
   }

   // Got to have at least one tile in the list.
   if(numTiles_ < 1)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree:: no tile data loaded");
      return false;
   }
   else if(numTiles_ != tilesLoaded)
   {
      logs(TL_LOG|TL_SERR,"TerrainMapOctree:: %d tiles failed to load",
         (numTiles_ - tilesLoaded));
      return false;
   }
   else
   {
      return true;
   }
}


int TerrainMapOctree::loadSubMap(const double xcen, const double ycen,
   double* mapWidth, double vehN, double vehE)
{
   // Gotta have at least two tiles to even bother with this stuff.
   // Return now unless there are 2 or more tiles.
   if (numTiles_ < 2)
   {
      return MAPBOUNDS_OK;
   }

//   logs(TL_LOG|TL_SERR,"TerrainMapOctree:   (vehN, vehE )  =  (%.2f, %.2f).",
//      vehN, vehE);
   logs(TL_LOG, "TerrainMapOctree:   (vehN, vehE )  =  (%.2f, %.2f).",
      vehN, vehE);

   double minDist = 1e8;
   // Compute distance from the vehicle to each of the tile map centers.  Select the smallest.
   for (int i = 0; i < numTiles_; i++)
   {
      double distance = sqrt (pow ((vehN - tiles_[i].northing), 2) + pow ((vehE - tiles_[i].easting), 2));
      //logs(TL_LOG|TL_LOG,"TerrainMapOctree:   Distance to tile%d = %.2f.\n", i+1, distance);

      if (distance < minDist)
      {
         minDistTile_ = i;
         minDist = distance;
      }
   }
   logs(TL_LOG,"TerrainMapOctree:  Min Distance = %.2f.", minDist);
   logs(TL_LOG,"TerrainMapOctree:  Using tile %d.", minDistTile_ + 1);

   // When the closest center location is in another tile, make the switch
   if (lastMinDistTile_ != minDistTile_)
   {
      // First unload the last tile:
      tiles_[lastMinDistTile_].unload();

      logs(TL_LOG,"TerrainMapOctree:  Switching to tile %d.",
         minDistTile_ + 1);

      // Now load the new tile:
      clock_t begin = clock();
      bool loaded = tiles_[minDistTile_].load();
      clock_t end = clock();
      double duration = (double)(end - begin)/CLOCKS_PER_SEC;

      if (!loaded)
      {
         // We're kind of screwed if the map doesn't load, so throw
         // an exception here. Another option is to reload the old file,
         // assuming just the new file is corrupted.
         logs(TL_LOG|TL_SERR,"TerrainMapOctree:  Octree Load Failed for %s.",
            tiles_[minDistTile_].mapName);
         throw Exception("TerrainMapOctree - Error loading map file.");
      }

      logs(TL_LOG,"TerrainMapOctree::Octree tile load %s took %f seconds.",
         tiles_[minDistTile_].mapName, duration);

      // Switch the pointer and we're ready to use
      lastMinDistTile_ = minDistTile_;
      OctreeMap = tiles_[minDistTile_].octreeMap;
      OctreeMap->Print();

   }

   return MAPBOUNDS_OK;
}

bool TerrainMapOctree::withinRefMap(const double northPos, const double eastPos)
{
   Vector LowerBounds = OctreeMap->GetLowerBounds();
   Vector UpperBounds = OctreeMap->GetUpperBounds();

   return ((northPos < UpperBounds.x)
      && (northPos > LowerBounds.x)
      && (eastPos < UpperBounds.y)
      && (eastPos > LowerBounds.y));
}

bool TerrainMapOctree::withinValidMapRegion(const double northPos,
   const double eastPos)
{
   return withinRefMap(northPos, eastPos);
}

bool TerrainMapOctree::withinSubMap(const double northPos, const double eastPos)
{
   return withinRefMap(northPos, eastPos);
}

bool TerrainMapOctree::GetMapT(mapT& currMap)
{
   logs(TL_LOG|TL_SERR, "\n\tGetMapT not implemented");
   return false;
}

bool TerrainMapOctree::GetMapBounds(double* currMapBounds)
{
   logs(TL_LOG|TL_SERR, "\n\tGetMapBounds not implemented");
   return false;
}
