/* FILENAME      : TerrainMap.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 01/01/08
 *
 * LAST MODIFIED : 07/17/09
 * MODIFIED BY   : Debbie Meduna
 * ----------------------------------------------------------------------------
 * Modification History:
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include "TerrainMap.h"

#ifdef use_namespace
using namespace std;
#endif

TerrainMap::TerrainMap(char *mapName)
{
   this->refMap = new refMapT;

   setRefMap(mapName);  
}

TerrainMap::~TerrainMap()
{
   if(refMap!=NULL)
      delete refMap;
}

int TerrainMap::extractSubMap(const double north, const double east, 
			      double* mapParams)
{
   int statusCode;

   //check that there is a reference map loaded to extract data from
   if(this->refMap->src == NULL)
   {
      logs(TL_OMASK(TL_TERRAIN_MAP, TL_LOG),"Attempted to extract map data with no reference map defined!!");
      return MAPBOUNDS_OUT_OF_BOUNDS;
   }

   //if map is already defined, clear memory
   if(this->map.xpts != NULL || this->map.ypts != NULL)
      this->map.clean();

   //load data from reference map
   mapdata* data = (mapdata*) malloc(sizeof(struct mapdata));
   statusCode = mapdata_fill(this->refMap->src, data, east, north, mapParams[1]
                             , mapParams[0]);

   //check status of loaded map data to ensure it worked properly
   if(statusCode != MAPBOUNDS_OUT_OF_BOUNDS)
   {
      convertMapdataToMapT(data);  

      //load variance map data
      extractVarMap(north, east, mapParams);
   }         
   
   mapdata_free(data, 1);
   return statusCode;
}

bool TerrainMap::withinRefMap(const double northPos, const double eastPos)
{
   int withinBounds = mapbounds_contains(this->refMap->bounds, northPos, 
                                         eastPos);
   if(withinBounds == MAPBOUNDS_OK)
      return true;

   return false;
}

bool TerrainMap::withinValidMapRegion(const double northPos, const double eastPos)
{
   if(withinRefMap(northPos,eastPos))
   {
      double mapValue = mapsrc_find(this->refMap->src, eastPos, northPos);
      //if(!isnan(mapValue))   // If initialized, return true
      if(!ISNIN(mapValue))
         return true;
   }

   return false;
}


bool TerrainMap::withinSubMap(const double northPos, const double eastPos)
{
  //Check to make sure a sub-map has been loaded
  if(this->map.xpts == NULL)
    return false;

  //Check if the point is within the loaded sub-map
  if((northPos > this->map.xpts[0]) &&
     (northPos < this->map.xpts[this->map.numX-1]) &&
     (eastPos > this->map.ypts[0]) && 
     (eastPos < this->map.ypts[this->map.numY-1])){
    return true;
  }
  else{
    return false;
  }
}

void TerrainMap::setLowResMap(char *mapName)
{
   if(this->refMap->lowResSrc == NULL)
   {
      this->refMap->lowResSrc = mapsrc_init();
      mapsrc_fill(mapName, this->refMap->lowResSrc);
   }

   if(this->refMap->lowResSrc->status != MAPSRC_IS_FILLED)
   {
      logs(TL_OMASK(TL_TERRAIN_MAP, TL_LOG),"Error loading in low resolution map file...\n");
      throw Exception("TerrainMap::setLowResMap() - Error loading map file");
   }
}


double TerrainMap::getNearestLowResMapPoint(const double north, 
                                            const double east,
                                            double &nearestNorth,
                                            double &nearestEast)
{
   double zi;

   if(this->refMap->lowResSrc == NULL)
      return 0.0;

   zi = mapsrc_find(this->refMap->lowResSrc, east, north);
   nearestNorth = refMap->lowResSrc->y
      [closestPtUniformArray(north, refMap->lowResSrc->y[0], 
                             refMap->lowResSrc->y
                             [refMap->lowResSrc->ydimlen-1],
                             refMap->lowResSrc->ydimlen)];
   nearestEast = refMap->lowResSrc->x
      [closestPtUniformArray(east, refMap->lowResSrc->x[0], 
                             refMap->lowResSrc->x
                             [refMap->lowResSrc->xdimlen-1],
                             refMap->lowResSrc->xdimlen)];

   return zi;
}


void TerrainMap::setRefMap(char *mapName)
{
   mapbounds* tempBounds;
   char mapPrefix[1024];
   char mapVarName[1040];

   //clear memory for any currently stored ref maps
   this->refMap->clean();
   
   //set map source to new reference map
   this->refMap->src = mapsrc_init();
   mapsrc_fill(mapName, this->refMap->src);
 
   //check that map source fill was successful
   if(this->refMap->src->status != MAPSRC_IS_FILLED)
   {
      logs(TL_OMASK(TL_TERRAIN_MAP, TL_LOG),"Error loading in map file...\n");
      throw Exception("TerrainMap::setRefMap() - Error loading map file");
   }
   
   //define variance map file name
   strcpy(mapPrefix, mapName);
   strtok(mapPrefix, ".");
   snprintf(mapVarName, 1040, "%s%s", mapPrefix, "_sd.grd");
   
   //set variance map source if provided
   this->refMap->varSrc = mapsrc_init();
   mapsrc_fill(mapVarName, this->refMap->varSrc);
   
   if(this->refMap->varSrc->status != MAPSRC_IS_FILLED)
   {
      mapsrc_free(this->refMap->varSrc);
      this->refMap->varSrc = NULL;
   }
 
   //set map bounds structure for new reference map
   this->refMap->bounds = mapbounds_init();
   tempBounds = mapbounds_init();
    if( (int check_error_code = mapbounds_fill1(this->refMap->src, tempBounds))!=MAPIO_OK){
        logs(TL_OMASK(TL_TERRAIN_MAP, TL_LOG),"WARN - mapbounds_fill1 ret[%d]\n",check_error_code);
    }

   //change labels to keep with a right-handed coordinate system
   this->refMap->bounds->xmin = tempBounds->ymin;
   this->refMap->bounds->xmax = tempBounds->ymax;
   this->refMap->bounds->ymin = tempBounds->xmin;
   this->refMap->bounds->ymax = tempBounds->xmax;
   this->refMap->bounds->dx = tempBounds->dy;
   this->refMap->bounds->dy = tempBounds->dx;
   
   free(tempBounds);
   tempBounds = NULL;
 
   //display reference map boundary information to screen
   char* outputString = mapbounds_tostring(this->refMap->bounds);
   logs(TL_OMASK(TL_TERRAIN_MAP, TL_LOG),outputString);
   free(outputString);
   
   return;
}

void TerrainMap::convertMapdataToMapT(mapdata* currMapStruct)
{
   int i;
  
   //define parameters in mapT structure based on parameters in currMapStruct
   this->map.numX = int(currMapStruct->ydimlen);
   this->map.numY = int(currMapStruct->xdimlen);
   this->map.xcen = currMapStruct->ycenter;
   this->map.ycen = currMapStruct->xcenter;

   //define map xpts and ypts vectors
   this->map.xpts = new double[this->map.numX];
   this->map.ypts = new double[this->map.numY];
  
   //Map is stored in E,N,U frame. Convert to N,E,D frame
   for(i = 0; i < this->map.numX; i++)
      this->map.xpts[i] = currMapStruct->ypts[i];
  
   for(i = 0; i < this->map.numY; i++)
      this->map.ypts[i] = currMapStruct->xpts[i];

   //define map parameters
   this->map.dx = this->refMap->bounds->dx;
   this->map.dy = this->refMap->bounds->dy;


   //convert zpts to Matrix of depths, positive downward
   Matrix temp(this->map.numX, this->map.numY);
  
   for(int row = 1; row <= this->map.numX; row++)
   {
      for(int col = 1; col <=this->map.numY; col++)
      {
         float* z = currMapStruct->z;   
         float value = z[(row - 1) * this->map.numY + (col - 1)];
         temp(row, col) = fabs(value);
      }
   }
  
   this->map.depths = temp;

}


int TerrainMap::extractVarMap(const double north, const double east, 
                              double* mapParams)
{
   int statusCode;

   //check that there is a variance map loaded to extract data from
   if(this->refMap->varSrc == NULL)
   {
      this->map.depthVariance.ReSize(this->map.numX, this->map.numY);
      this->map.depthVariance = fabs(this->map.dx);

      //check for valid variance value
      // if(isnan(this->map.depthVariance(1,1)) || // Using ISNIN here
      if( ISNIN(this->map.depthVariance(1,1)) ||
                this->map.depthVariance(1,1) == 0 )
      {
         map.depthVariance = fabs(map.dx);       
      }

      statusCode = MAPBOUNDS_OK;
   }
   else
   {
      mapdata* data = (mapdata*) malloc(sizeof(struct mapdata));
      statusCode = mapdata_fill(this->refMap->varSrc, data, east, north, 
                                mapParams[1], mapParams[0]);

      //check status of loaded map data to ensure it worked properly
      if(statusCode != MAPBOUNDS_OUT_OF_BOUNDS)
      {
          //convert zpts to Matrix of depths
         Matrix temp(this->map.numX, this->map.numY);
         
         for(int row = 1; row <= this->map.numX; row++)
         {
            for(int col = 1; col <=this->map.numY; col++)
            {
               float* z = data->z;   
               float value = z[(row - 1) * this->map.numY + (col - 1)];

               //estimate variance by stored std. dev. values plus variogram 
               //variation at the given map resolution
               temp(row, col) = value*value + 1.0 + 
                  this->mapVariogram.evalVariogram(this->map.dx);

               //check for valid variance values
               // if(isnan(temp(row,col)) || value == 0)  Using ISNIN here
               if(ISNIN(temp(row,col)) || value == 0) 
                  temp(row,col) = fabs(this->map.dx);               
            }
         }

         this->map.depthVariance = temp;

      }  
      
      mapdata_free(data, 1);
   }
   
   return statusCode; 
}
