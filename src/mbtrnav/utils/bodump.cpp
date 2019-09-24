#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
//
// So that the Linux Makefile over in the G2TerrainNav directory can use this
// source to (also) compile in Linux.  (Note the -I. in the BODUMP build in
// the Linux Makefile.)
#ifdef _QNX
  #include "Octree.hpp"
  #include "OctreeSupport.hpp"
#else
  #include <Octree.hpp>
  #include <OctreeSupport.hpp>
#endif

// cc OctreeSupport.cpp bodump.cc -O2 -WC,-ei -D_QNX -o bodump

int main(int argc, char **argv)
{
   char *mapName = argv[1];

   clock_t startTime, stopTime;
   //
   // Read in the real map and print out "meta" data.
   Octree<bool> map;
   bool success;

   if( argc<2 )
   {
      printf("Please supply a map name.\n");
      exit(-1);
   }

   printf("Reading map from %s.\n", mapName);
   startTime = clock();
   success = map.LoadFromFile(mapName);
   stopTime = clock();

   if( success )  
   {
      printf("Map successfully read in %5.2e seconds.\n", 
	     ((double)(stopTime-startTime))/CLOCKS_PER_SEC );
#ifdef _QNX
      char *cmd = "sin -P bodump";
      // char *cmd = "sin";
      system(cmd);
#endif
      
   }
   else
   {
      printf("Error reading the map.\n");
      return -1;
   }

   map.Print();

   return 0;
}
