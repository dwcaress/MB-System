/*
/  See README file for copying and redistribution conditions.
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <string.h>
#include "types_win32.h"
#else
#include <strings.h>
#include <unistd.h>
#endif
#include <math.h>

#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"
#include "pb_math.h"
#define  __SAPI__
#include "sapi.h"



extern SurfDataInfo*      sapiToSurfData;
extern SdaInfo*           sapiToSdaInfo;
extern SurfSoundingData*  sapiToSdaBlock;

static char sccsid[50] = {"@(#)libsapi.a  Version 3.1 17.10.2001"};


/* There are C++ - Compilers, which omit unreferenced statics */
char* forCCsapi(void)
{
 return(sccsid);
}


long SAPI_getNrSoundings(void)
{
 long nrSoundings = 0;

 if(sapiToSurfData != NULL) nrSoundings = sapiToSurfData->nrOfSoundings;
 return(nrSoundings);
}


long SAPI_getNrBeams(void)
{
 long nrBeams = 0;

 if(sapiToSurfData != NULL) nrBeams = sapiToSurfData->nrBeams;
 return(nrBeams);
}


long SAPI_posPresentationIsRad(void)
{
 if(sapiToSurfData != NULL)
 {
  if(sapiToSurfData->toGlobalData->presentationOfPosition == EASTING_NORTHING)
    return((long)1);
 } 
 return((long)0);
}


char* SAPI_getTypeOfSounder(void)
{
 static char sounderType[2] = {"?"};

 if(sapiToSurfData != NULL) sounderType[0] = sapiToSurfData->toGlobalData->typeOfSounder;
 return(sounderType);
}


char* SAPI_getNameOfSounder(void)
{
 static char nameOfSounder[20] = {"?"};
 
 if(sapiToSurfData != NULL)
 {
  strncpy(nameOfSounder,sapiToSurfData->toGlobalData->nameOfSounder,STRING_SIZE);
 } 
 return(nameOfSounder);
}


char* SAPI_getNameOfShip(void)
{
 static char nameOfShip[20] = {"?"};
 
 if(sapiToSurfData != NULL)
 {
  strncpy(nameOfShip,sapiToSurfData->toGlobalData->shipsName,STRING_SIZE);
 } 
 return(nameOfShip);
}


long SAPI_getNrSoundvelocityProfiles(void)
{
 long nrCProfiles = 0;

 if(sapiToSurfData != NULL) nrCProfiles = sapiToSurfData->nrCProfiles;
 return(nrCProfiles);
}


long SAPI_getNrEvents(void)
{
 long nrEvents = 0;

 if(sapiToSurfData != NULL) nrEvents = sapiToSurfData->nrEvents;
 return(nrEvents);
}


long SAPI_getNrPolygonElements(void)
{
 long nrPolyElements = 0;

 if(sapiToSurfData != NULL) nrPolyElements = sapiToSurfData->nrPolyElements;
 return(nrPolyElements);
}


long SAPI_getNrPositionsensors(void)
{
 long nrPosiSensors = 0;

 if(sapiToSurfData != NULL) nrPosiSensors = sapiToSurfData->nrPosiSensors;
 return(nrPosiSensors);
}


long SAPI_dataHaveHighFrequencyLayer(void)
{
 long layer=0;
 if(sapiToSurfData != NULL)
 {
  if(sapiToSurfData->toGlobalData->highFrequency > 0.0) layer=1;
 } 
 return(layer);    
}


long SAPI_dataHaveMediumFrequencyLayer(void)
{
 long layer=0;
 if(sapiToSurfData != NULL)
 {
  if(sapiToSurfData->toGlobalData->mediumFrequency > 0.0) layer=1;
 } 
 return(layer);    
}


long SAPI_dataHaveLowFrequencyLayer(void)
{
 long layer=0;
 if(sapiToSurfData != NULL)
 {
  if(sapiToSurfData->toGlobalData->lowFrequency > 0.0) layer=1;
 } 
 return(layer);    
}


SurfGlobalData* SAPI_getGlobalData(void)
{
 if(sapiToSurfData == NULL) return(NULL);
 return(sapiToSurfData->toGlobalData);
}


SurfStatistics* SAPI_getStatistics(void)
{
 if(sapiToSurfData == NULL) return(NULL);
 return(sapiToSurfData->toStatistics);
}


SurfPositionAnySensor* SAPI_getPositionSensor(long nrSensor)
{
 SurfPositionSensorArray* toSensor;
 long maxNrPosSens = 0;
 
 if(sapiToSurfData == NULL) return(NULL);
 maxNrPosSens = SAPI_getNrPositionsensors();
 if((nrSensor >= 0) && (nrSensor < maxNrPosSens))
 {
  toSensor = (SurfPositionSensorArray*)sapiToSurfData->toPosiSensors[nrSensor].label;
  return((SurfPositionAnySensor*)toSensor);
 }
 return(0);
}


SurfEventValues* SAPI_getEvent(long nrEvent)
{
 long maxNrEvents = 0;
 
 if(sapiToSurfData == NULL) return(NULL);
 maxNrEvents = SAPI_getNrEvents();
 if((nrEvent >= 0) && (nrEvent < maxNrEvents))
 {
  return(&(sapiToSurfData->toEvents->values[nrEvent]));
 }
 return(NULL);
}


SurfPolygons* SAPI_getPolygons(void)
{
 if(sapiToSurfData == NULL) return(NULL);
 return(sapiToSurfData->toPolygons);
}



double SAPI_getAbsoluteStartTimeOfProfile(void)
{
 SurfTm sTm;
 
 if(sapiToSurfData == NULL) return(0.0);
 surf_timeSizetoSurfTm(sapiToSurfData->toGlobalData->startTimeOfProfile,&sTm);
 return(surf_timeAbsoluteFromSurfTm (&sTm));
}


