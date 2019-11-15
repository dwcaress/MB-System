/****************************************************************************/
/* Copyright (c) 2018 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : Program to test loading & unloading of octree map tile files  */
/*            into memory hoping to detect malformed or corrupted files     */
/*            before they can affect a TRN mission.                         */
/* Filename : test_tiles.cpp                                                */
/* Author   : Henthorn                                                      */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 08/01/2018                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#include "TerrainMapOctree.h"

// Program use:
// $ test_tiles map-dir-a map-dir-b ...
// The map-dir arguments are the pathnames to directories that contain octree
// maps split into adjacent tiles. Arguments can be full or relative pathnames.
// 
int main(int argc, char **argv)
{
   // Must have at least one map-dir argument
   // 
   if (argc < 2)
   {
      printf(" usage: tile_test tile-dir-path [tile-dir-path-2 ...]\n");
      return 1;
   }

   // For each map-dir argument, create a map object. The object constructor
   // reads the list of tile filenames from the tile directory.
   // 
   for (int i = 1; i < argc; i++)
   {
      // Create the map object using the pathname
      // 
      TerrainMapOctree map(argv[i]);

      // Load and unload each tile in the tile list one at a time.
      // Only one map tile is ever in memory.
      // 
      if (!map.tileLoadTest())
      {
         printf("\n FAIL! Some tiles in %s failed to load.\n", argv[i]);
      }
      else
      {
         printf("\n %s Passed!\n", argv[i]);
      }

      // Note that the map destructor is also tested right here.
   }

   return 0;
}