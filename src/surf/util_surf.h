// DATEINAME        : util_surf.h
// ERSTELLUNGSDATUM : 09.08.93
// COPYRIGHT (C) 1993: ATLAS ELEKTRONIK GMBH, 28305 BREMEN
//
// See README file for copying and redistribution conditions.

#ifndef SURF_UTIL_SURF_H_
#define SURF_UTIL_SURF_H_

#include <time.h>

// Return vals of moveInSdaThread
typedef enum {
  STEP_DONE,
  END_OF_THREAD
} MoveInSdaThread;

// Operating mode of stepInSdaThread
typedef enum {
  FORE_ONE_STEP,
  BACK_ONE_STEP,
  FORE_X_STEPS,
  BACK_X_STEPS,
  ABS_POSITION,
  HALF_WAY_ABS,
  BACK_HALF_WAY_REL,
  FORE_HALF_WAY_REL,
  TO_START,
  TO_END
} ModeMoveInSdaThread;

// Operating mode of surf_insertNewSdaBlockAtActualPosition
typedef enum {
  INSERT_AFTER_ACT_POS,
  INSERT_BEFOR_ACT_POS
} SDAinsertMode;

/************************************************************
*  relative Zeiten werden im Format 'SurfTime' in           *
*          Sekunden dargestellt                             *
************************************************************/
typedef double SurfTime;

typedef struct {
  char date[10];
  char time[10];
} SurfTimeDate;

typedef struct {
  struct tm tmTime;
  int fractionalSeconds; // 1/100 sec.
} SurfTm;

MoveInSdaThread surf_moveInSdaThread(
    SurfDataInfo* toSurfDataInfo, ModeMoveInSdaThread mode,
    u_long nrOfSteps);
XdrSurf surf_backupSdaBlock(SurfDataInfo* toSurfDataInfo);
void surf_restoreSdaBlock(SurfDataInfo* toSurfDataInfo);
XdrSurf surf_insertNewSdaBlockAtActualPosition(
    SurfDataInfo* toSurfDataInfo, SDAinsertMode where);

// Time functions
void surf_timeSizetoTimeDate(char* timeSize, SurfTimeDate* timeDate);
void surf_timeSizetoSurfTm(char* timeSize, SurfTm* surfTm);
void surf_surfTmToTimeSize(char* timeSize, SurfTm* surfTm);
SurfTime surf_timeOfTheDayFromTimeSize (char* timeSize);
SurfTime surf_timeOfTheDayFromSurfTm (SurfTm* surfTm);
SurfTime surf_timeAbsoluteFromSurfTm (SurfTm* surfTm);
SurfTime surf_difftime (SurfTm* later,SurfTm* earlier);
void surf_putJulianDayIntoTm(SurfTm* surfTm);
long surf_timeSizeToInt(char* timeSize);
void surf_setVendorText(SurfDataInfo* toSurfData);

#endif  // SURF_UTIL_SURF_H_
