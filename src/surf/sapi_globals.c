// See README file for copying and redistribution conditions.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#ifdef _WIN32
#include "types_win32.h"
#else
#include <unistd.h>
#endif

#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"
#include "pb_math.h"
#define  __SAPI__
#include "mb_sapi.h"

extern SurfDataInfo *sapiToSurfData;
extern SurfSoundingData *sapiToSdaBlock;

long SAPI_getNrSoundings(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrOfSoundings;
}

long SAPI_getNrBeams(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrBeams;
}

long SAPI_posPresentationIsRad(void) {
  return
      sapiToSurfData != NULL &&
      sapiToSurfData->toGlobalData->presentationOfPosition == EASTING_NORTHING
      ? 1l : 0l;
}

char *SAPI_getTypeOfSounder(void) {
  static char sounderType[2] = "?";

  if(sapiToSurfData != NULL) sounderType[0] = sapiToSurfData->toGlobalData->typeOfSounder;
  return sounderType;
}

char *SAPI_getNameOfSounder(void) {
  static char nameOfSounder[20] = "?";

  if (sapiToSurfData != NULL) {
    strncpy(nameOfSounder, sapiToSurfData->toGlobalData->nameOfSounder, STRING_SIZE);
  }
  return nameOfSounder;
}

char *SAPI_getNameOfShip(void) {
  static char nameOfShip[20] = "?";

  if(sapiToSurfData != NULL) {
    strncpy(nameOfShip, sapiToSurfData->toGlobalData->shipsName, STRING_SIZE);
  }
  return nameOfShip;
}

long SAPI_getNrSoundvelocityProfiles(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrCProfiles;
}

long SAPI_getNrEvents(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrEvents;
}

long SAPI_getNrPolygonElements(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrPolyElements;
}

long SAPI_getNrPositionsensors(void) {
  return sapiToSurfData == NULL ? 0l : sapiToSurfData->nrPosiSensors;
}

long SAPI_dataHaveHighFrequencyLayer(void) {
  long layer = 0;
  if(sapiToSurfData != NULL) {
    if(sapiToSurfData->toGlobalData->highFrequency > 0.0) layer=1;
  }
  return layer;
}

long SAPI_dataHaveMediumFrequencyLayer(void) {
  long layer = 0;
  if(sapiToSurfData != NULL) {
    if(sapiToSurfData->toGlobalData->mediumFrequency > 0.0) layer = 1;
  }
  return layer;
}

long SAPI_dataHaveLowFrequencyLayer(void)
{
  long layer = 0;
  if(sapiToSurfData != NULL) {
    if(sapiToSurfData->toGlobalData->lowFrequency > 0.0) layer = 1;
  }
  return layer;
}

SurfGlobalData *SAPI_getGlobalData(void) {
  return sapiToSurfData == NULL ? NULL : sapiToSurfData->toGlobalData;
}

SurfStatistics *SAPI_getStatistics(void) {
  return sapiToSurfData == NULL ? NULL : sapiToSurfData->toStatistics;
}

SurfPositionAnySensor *SAPI_getPositionSensor(long nrSensor) {
  if(sapiToSurfData == NULL) return NULL;
  const long maxNrPosSens = SAPI_getNrPositionsensors();
  if (nrSensor >= 0 && nrSensor < maxNrPosSens) {
    SurfPositionSensorArray *toSensor =
        (SurfPositionSensorArray *)sapiToSurfData->toPosiSensors[nrSensor].label;
    return (SurfPositionAnySensor *)toSensor;
  }

  return NULL;
}

SurfEventValues *SAPI_getEvent(long nrEvent) {
  if(sapiToSurfData == NULL) return NULL;
  long maxNrEvents = SAPI_getNrEvents();
  if (nrEvent >= 0 && nrEvent < maxNrEvents) {
    return &(sapiToSurfData->toEvents->values[nrEvent]);
  }
  return NULL;
}

SurfPolygons *SAPI_getPolygons(void) {
  return sapiToSurfData == NULL ? NULL : sapiToSurfData->toPolygons;
}

double SAPI_getAbsoluteStartTimeOfProfile(void) {
  if(sapiToSurfData == NULL) return 0.0;

  SurfTm sTm;
  surf_timeSizetoSurfTm(sapiToSurfData->toGlobalData->startTimeOfProfile, &sTm);
  return surf_timeAbsoluteFromSurfTm(&sTm);
}


