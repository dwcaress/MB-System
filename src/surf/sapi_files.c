// See README file for copying and redistribution conditions.

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

#define SAPI_VERSION "SAPI V3.1.4"

#ifdef _WIN32
#include <io.h>
#define access _access
#endif

extern XdrSurf mem_convertOneSdaBlock2(XDR *xdrs, SdaInfo* sdaInfo, short versLess2);
extern size_t initializeSdaInfo(SurfDataInfo *toSurfDataInfo, SdaInfo *toSdaInfo);
extern void setPointersInSdaInfo(void *toSdaBlock, SdaInfo *toSdaInfo);
long SAPI_openFile(char *surfDir, char *surfFile, long errorprint);

SurfDataInfo *sapiToSurfData;
SurfSoundingData *sapiToSdaBlock;
bool loadIntoMemory = false;

static void freeControlData(void) {
  SdaInfo* sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) {
    sapiToSdaInfo = sapiToSurfData->toSdaInfo;
    if (sapiToSurfData->xdrs != NULL) {
      free((char*)sapiToSurfData->xdrs);
      sapiToSurfData->xdrs=NULL;
    }
    if (sapiToSurfData->fp != NULL) fclose(sapiToSurfData->fp);
    free((char*)sapiToSurfData);
    sapiToSurfData=NULL;
  }
  if (sapiToSdaInfo != NULL && !loadIntoMemory)
    free((char*)sapiToSdaInfo);
  if (sapiToSdaBlock != NULL) free((char*)sapiToSdaBlock);
  sapiToSurfData = NULL;
  sapiToSdaInfo = NULL;
  sapiToSdaBlock = NULL;
}

long SAPI_open(char* surfDir,char* surfFile,long errorprint) {
  loadIntoMemory = false;
  return SAPI_openFile(surfDir, surfFile, errorprint);
}

long SAPI_openFile(char* surfDir, char* surfFile, long errorprint) {
  if (access(surfDir, 0) != 0) {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't access path: '%s' !\n", surfDir);
    return -1l;
  }

  freeControlData();

  sapiToSurfData = (SurfDataInfo*)calloc(1, sizeof(SurfDataInfo));
  if (sapiToSurfData == NULL) {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't allocate sufficient memory' !\n");
    return -1l;
  }

  char filesix[300];
  strncpy(filesix, surfDir, 250);
  strcat(filesix, "/");
  strcat(filesix, surfFile);
  strcat(filesix, ".six");

  char filesda[300];
  strncpy(filesda, surfDir, 250);
  strcat(filesda, "/");
  strcat(filesda, surfFile);
  strcat(filesda, ".sda");

  if (access(filesix, 4) !=0) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't access file: '%s' !\n", filesix);
    return -1;
  }
  if (access(filesda, 4) !=0) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't access file: '%s' !\n", filesda);
    return -1;
  }

  XdrSurf ret = mem_ReadSixStructure(filesix, sapiToSurfData);
  if (ret != SURF_SUCCESS) {
    mem_destroyAWholeSurfStructure(sapiToSurfData);
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't read file: '%s' !\n", filesix);
    return -1l;
  }

  // Special mode for rewrite read the whole surfFile into memory

  if (loadIntoMemory) {
    ret = mem_ReadSdaStructure(filesda, sapiToSurfData);
    if (ret != SURF_SUCCESS) {
      mem_destroyAWholeSurfStructure(sapiToSurfData);
      freeControlData();
      if (errorprint != 0)
        fprintf(stderr, "SAPI-Error: Can't read file: '%s' !\n", filesda);
      return -1l;
    }
    surf_moveInSdaThread(sapiToSurfData, ABS_POSITION, 0);
    return 0l;
  }

  // Allocate the necessary memory for a SDA-structure and read
  // the first SDA-Block from file

  if (sapiToSurfData->nrOfSoundings <=0 ) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: nr of soundings = %d!\n", (int)(sapiToSurfData->nrOfSoundings));
    return -1l;
  }

  // Allocate structure for xdr-conversion and SdaInfo and Sda-Thread

  SdaInfo* sapiToSdaInfo = (SdaInfo*)calloc(1, sizeof(SdaInfo));
  sapiToSurfData->toSdaInfo = sapiToSdaInfo;

  sapiToSurfData->xdrs = (XDR*)calloc(1, sizeof(XDR));  /* ????? */

  if ((sapiToSurfData->xdrs == NULL) || (sapiToSdaInfo == NULL)) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't allocate sufficient memory' !\n");
    return -1l;
  }

  const size_t sizeOfSdaBlock = initializeSdaInfo(sapiToSurfData, sapiToSdaInfo);
  sapiToSdaBlock = (SurfSoundingData*)calloc(1, sizeOfSdaBlock);

  if ((sapiToSurfData->xdrs == NULL) || (sapiToSdaInfo == NULL)) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't allocate sufficient memory' !\n");
    return -1l;
  }

  setPointersInSdaInfo(sapiToSdaBlock, sapiToSdaInfo);

  // Open file Read and read first Block

  sapiToSurfData->fp = xdrSurfOpenRead(sapiToSurfData->xdrs, (const char*)filesda);
  if (sapiToSurfData->fp == NULL) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't open file: '%s' !\n", filesda);
    return -1l;
  }

  ret = mem_convertOneSdaBlock2(sapiToSurfData->xdrs, sapiToSdaInfo, sapiToSurfData->sourceVersionLess2);
  if (ret != SURF_SUCCESS) {
    freeControlData();
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't read file: '%s' !\n", filesda);
    return -1l;
  }

  return 0l;
}

long SAPI_nextSounding(long errorprint) {
  SdaInfo* sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSurfData == NULL || sapiToSdaInfo == NULL) {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: No SURF-data open !\n");
    return -1l;
  }

  // Special mode for rewrite read the whole surfFile into memory

  if (loadIntoMemory) {
    if (surf_moveInSdaThread(sapiToSurfData, FORE_ONE_STEP, 0) == END_OF_THREAD) {
      if (errorprint != 0)
        fprintf(stderr, "SAPI-Error: End of file !\n");
      return -1l;
    }
    return 0l;
  }

  const XdrSurf ret = mem_convertOneSdaBlock2(
      sapiToSurfData->xdrs, sapiToSdaInfo, sapiToSurfData->sourceVersionLess2);
  if (ret != SURF_SUCCESS) {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't read file or EOF !\n");
    return -1l;
  }
  return 0l;
}

long SAPI_rewind(long errorprint) {
  SdaInfo* sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;

  // Special mode for rewrite read the whole surfFile into memory
  if (loadIntoMemory) {
    if (sapiToSurfData == NULL || sapiToSdaInfo == NULL) {
      return -1l;
    }
    surf_moveInSdaThread(sapiToSurfData, TO_START, 0);
    return 0l;
  }

  if (sapiToSurfData == NULL || sapiToSurfData->xdrs == NULL ||
      sapiToSdaInfo == NULL || sapiToSurfData->fp == NULL) {
    return -1l;
  }

  rewind(sapiToSurfData->fp);
  return SAPI_nextSounding(errorprint);
}

void SAPI_close(void) {
  if (sapiToSurfData != NULL) {
    if (sapiToSurfData->fp != NULL) fclose(sapiToSurfData->fp);
    sapiToSurfData->fp = NULL;
    mem_destroyAWholeSurfStructure(sapiToSurfData);
    sapiToSurfData = NULL;
  }
#ifndef _WIN32
  freeControlData();
#endif
}

#if 0
void recalculateData(void) {
  if (sapiToSurfData == NULL) {
    return;
  }

  const double minDef = 999999999.0;  // TODO(schwehr): Use DBL_MAX
  const double maxDef = -999999999.0;

  SurfGlobalData *toGlobalData = sapiToSurfData->toGlobalData;
  SurfStatistics *toStatistics = sapiToSurfData->toStatistics;

  short nrBeams = (short) sapiToSurfData->nrBeams;
  long nrSoundings = sapiToSurfData->nrOfSoundings;

  const bool posIsMeter = toGlobalData->presentationOfPosition == 'X';

  bool isPitchcompensated = true;
  if (strncmp(toGlobalData->nameOfSounder, "MD", 2) == 0)
    isPitchcompensated = false;
  if (strncmp(toGlobalData->nameOfSounder, "FS", 2) == 0)
    isPitchcompensated = false;

  double minBeamPositionStar;
  double maxBeamPositionStar;
  double minBeamPositionAhead;
  double maxBeamPositionAhead;
  if (toGlobalData->typeOfSounder != 'F') {
    minBeamPositionStar = 0.0;
    maxBeamPositionStar = 0.0;
    minBeamPositionAhead = 0.0;
    maxBeamPositionAhead = 0.0;
  } else {
    minBeamPositionStar = minDef;
    maxBeamPositionStar = maxDef;
    minBeamPositionAhead = minDef;
    maxBeamPositionAhead = maxDef;
  }
  double minDepth = minDef;
  double maxDepth = maxDef;
  double minX = minDef;
  double minY = minDef;
  double maxX = maxDef;
  double maxY = maxDef;

  double minSpeed = minDef;
  double maxSpeed = maxDef;
  double minRoll = minDef;
  double maxRoll = maxDef;
  double minPitch = minDef;
  double maxPitch = maxDef;
  double minHeave = minDef;
  double maxHeave = maxDef;

  const double refX = toGlobalData->referenceOfPositionX;
  const double refY = toGlobalData->referenceOfPositionY;
  double posX = 0.0;
  double posY = 0.0;
  double relWay = 0.0;
  double relTime = 0.0;

  FanParam fanParam;
  memset(&fanParam, 0, sizeof(fanParam));

  bool depthStatisticsFound = false;
  short indexToAngle;
  short indexToTransducer;
  u_short depthFlag;
  u_short soundingFlag;
  double lastX;
  double lastY;

  long firstSounding = -1;
  for (long ii = 0; ii < nrSoundings; ii++) {
    surf_moveInSdaThread(sapiToSurfData, ABS_POSITION, ii);
    soundingFlag = sapiToSurfData->toSdaInfo->toSoundings->soundingFlag;

    if ((soundingFlag & (SF_DELETED | SF_ALL_BEAMS_DELETED)) == 0) {
      firstSounding++;
      relTime = (double) sapiToSurfData->toSdaInfo->toSoundings->relTime;
      posX = (double)
             (sapiToSurfData->toSdaInfo->toActCenterPosition->centerPositionX) + refX;
      posY = (double)
             (sapiToSurfData->toSdaInfo->toActCenterPosition->centerPositionY) + refY;
      const double speed = (double)
                           (sapiToSurfData->toSdaInfo->toActCenterPosition->speed);
      if (firstSounding == 0) {
        lastX = posX;
        lastY = posY;
      }
      if (posX > maxX) maxX = posX;
      if (posX < minX) minX = posX;
      if (posY > maxY) maxY = posY;
      if (posY < minY) minY = posY;
      if (speed > maxSpeed) maxSpeed = speed;
      if (speed < minSpeed) minSpeed = speed;

      double deltaX;
      double deltaY;
      if (posIsMeter) {
        deltaX = posX - lastX;
        deltaY = posY - lastY;
      } else {
        deltaX = setToPlusMinusPI(posX - lastX);
        deltaY = setToPlusMinusPI(posY - lastY);
        deltaY = RAD_TO_METER_Y(deltaY);
        deltaX = RAD_TO_METER_X(deltaX, lastY);
      }
      relWay = relWay + sqrt((deltaX*deltaX) + (deltaY*deltaY));
      sapiToSurfData->toSdaInfo->toSoundings->relWay = (float)relWay;
      if (firstSounding == 0) {
        toGlobalData->modifiedTrackStartX = (float)(posX - refX);
        toGlobalData->modifiedTrackStartY = (float)(posY - refY);
      }
      lastX = posX;
      lastY = posY;

      const double tide = (double)sapiToSurfData->toSdaInfo->toSoundings->tide;

      const double roll =
          (double)sapiToSurfData->toSdaInfo->toSoundings->rollWhileTransmitting;
      fanParam.pitchTx =
          (double)sapiToSurfData->toSdaInfo->toSoundings->pitchWhileTransmitting
          + (double)sapiToSurfData->toGlobalData->offsetPitchFore;
      fanParam.heaveTx = (double)
                         sapiToSurfData->toSdaInfo->toSoundings->heaveWhileTransmitting;
      fanParam.ckeel   = (double)
                         sapiToSurfData->toSdaInfo->toSoundings->cKeel;
      fanParam.cmean   = (double)
                         sapiToSurfData->toSdaInfo->toSoundings->cMean;

      if (roll > maxRoll) maxRoll = roll;
      if (roll < minRoll) minRoll = roll;
      if (fanParam.pitchTx > maxPitch) maxPitch = fanParam.pitchTx;
      if (fanParam.pitchTx < minPitch) minPitch = fanParam.pitchTx;
      if (fanParam.heaveTx > maxHeave) maxHeave = fanParam.heaveTx;
      if (fanParam.heaveTx < minHeave) minHeave = fanParam.heaveTx;

      if (toGlobalData->typeOfSounder == 'F') {
        indexToAngle =
            (short)sapiToSurfData->toSdaInfo->toSoundings->indexToAngle;
        SurfMultiBeamAngleTable *toAngles = getSurfAngleTable(sapiToSurfData->toAngleTables,
                                     nrBeams, indexToAngle);

        bool allBeamsDeleted = true;
        for (short beam = 0; beam < nrBeams; beam++) {
          depthFlag = sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag;
          if ((depthFlag & SB_DELETED) == 0) {
            allBeamsDeleted = false;
            fanParam.angle = toAngles->beamAngle[beam];
            indexToTransducer =
                (short)sapiToSurfData->toSdaInfo->toSoundings->indexToTransducer;
            if ((sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag & SB_TRANSDUCER_PLUS1) != 0)
              indexToTransducer++;
            fanParam.draught =
                (double)sapiToSurfData->toTransducers[indexToTransducer].transducerDepth;
            fanParam.transducerOffsetAhead =
                (double)sapiToSurfData->toTransducers[indexToTransducer].transducerPositionAhead;
            fanParam.transducerOffsetStar =
                (double)sapiToSurfData->toTransducers[indexToTransducer].transducerPositionStar;
            if (sapiToSurfData->toSdaInfo->toMultiBeamRec != NULL)
              fanParam.heaveRx =
                  (double)sapiToSurfData->toSdaInfo->toMultiBeamRec[beam].heaveWhileReceiving;
            else
              fanParam.heaveRx = 0.0;
            fanParam.travelTime =
                (double)sapiToSurfData->toSdaInfo->toMultiBeamTT[beam].travelTimeOfRay;

            if (depthFromTT(&fanParam, isPitchcompensated)) {
              const double depth = fanParam.depth - tide;
              sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depth = (float)depth;
              sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].beamPositionAhead =
                  (float)fanParam.posAhead;
              sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].beamPositionStar  =
                  (float)fanParam.posStar;
              if (depth < minDepth)
                minDepth = depth;
              if (depth > maxDepth)
                maxDepth = depth;
              if (fanParam.posStar < minBeamPositionStar)
                minBeamPositionStar = fanParam.posStar;
              if (fanParam.posStar > maxBeamPositionStar)
                maxBeamPositionStar = fanParam.posStar;
              if (fanParam.posAhead < minBeamPositionAhead)
                minBeamPositionAhead = fanParam.posAhead;
              if (fanParam.posAhead > maxBeamPositionAhead)
                maxBeamPositionAhead = fanParam.posAhead;
              depthStatisticsFound = true;
            } else {
              sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag =
                  depthFlag | SB_DELETED;
            }
          }
        } /*for(beam = 0;beam < nrBeams;beam++)*/
        if (allBeamsDeleted) {
          sapiToSurfData->toSdaInfo->toSoundings->soundingFlag =
              soundingFlag | SF_DELETED | SF_ALL_BEAMS_DELETED;
        }
        // if (toGlobalData->typeOfSounder == 'F')
      } else {
        const double cmean = (double)sapiToSurfData->toSdaInfo->toSoundings->cMean;
        sapiToSurfData->toSdaInfo->toSoundings->cKeel = (float)cmean;
        if ((sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthFlag & SB_DELETED) == 0) {
          double depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthLFreq;
          if (depth != 0.0) {
            if (depth < minDepth)
              minDepth = depth;
            if (depth > maxDepth)
              maxDepth = depth;
            depthStatisticsFound = true;
          }
          depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthMFreq;
          if (depth != 0.0) {
            if (depth < minDepth)
              minDepth = depth;
            if (depth > maxDepth)
              maxDepth = depth;
            depthStatisticsFound = true;
          }
          depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthHFreq;
          if (depth != 0.0) {
            if (depth < minDepth)
              minDepth = depth;
            if (depth > maxDepth)
              maxDepth = depth;
            depthStatisticsFound = true;
          }
        }
      }
    }  // if ((soundingFlag & SF_DELETED) == 0)
  }  // for(ii=0;ii<nrSoundings;ii++)

  if (!depthStatisticsFound) {
    minDepth = 0.0;
    maxDepth = 0.0;
    minBeamPositionStar = 0.0;
    maxBeamPositionStar = 0.0;
    minBeamPositionAhead = 0.0;
    maxBeamPositionAhead = 0.0;
    minX = 0.0;
    maxX = 0.0;
    minY = 0.0;
    maxY = 0.0;
  }

  toStatistics->minDepth = (float)minDepth;
  toStatistics->maxDepth = (float)maxDepth;
  toStatistics->minBeamPositionStar = (float)minBeamPositionStar;
  toStatistics->maxBeamPositionStar = (float)maxBeamPositionStar;
  toStatistics->minBeamPositionAhead = (float)minBeamPositionAhead;
  toStatistics->maxBeamPositionAhead = (float)maxBeamPositionAhead;

  toStatistics->minEasting = minX;
  toStatistics->maxEasting = maxX;
  toStatistics->minNorthing = minY;
  toStatistics->maxNorthing = maxY;

  toStatistics->minSpeed = (float)minSpeed;
  toStatistics->maxSpeed = (float)maxSpeed;
  toStatistics->minRoll = (float)minRoll;
  toStatistics->maxRoll = (float)maxRoll;
  toStatistics->minPitch = (float)minPitch;
  toStatistics->maxPitch = (float)maxPitch;
  toStatistics->minHeave = (float)minHeave;
  toStatistics->maxHeave = (float)maxHeave;

  toGlobalData->modifiedTrackStopX = (float)(posX - refX);
  toGlobalData->modifiedTrackStopY = (float)(posY - refY);
  toGlobalData->modifiedStartStopDistance = (float)(relWay);
  toGlobalData->originalTrackStartX = toGlobalData->modifiedTrackStartX;
  toGlobalData->originalTrackStartY = toGlobalData->modifiedTrackStartY;
  toGlobalData->originalTrackStopX = toGlobalData->modifiedTrackStopX;
  toGlobalData->originalTrackStopY = toGlobalData->modifiedTrackStopY;
  toGlobalData->originalStartStopDistance = (float)(relWay);
  toGlobalData->originalStartStopTime = relTime;
}
#endif

#if 0
long SAPI_writeBackFromMemory(char* surfDir, char* surfFile, long errorprint) {
  if (sapiToSurfData == NULL) {
    if (errorprint != 0)
      fprintf(stderr,
              "SAPI-Error: There is no open SURF-file for writing back !\n");
    return -1l;
  }

  if (!loadIntoMemory) {
    if (errorprint != 0)
      fprintf(stderr,
              "SAPI-Error: For writing back you have to open\n     the file with SAPI_openIntoMemory(..)!\n");
    return -1l;
  }

  if (access(surfDir, 0) != 0) {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't access path: '%s' !\n", surfDir);
    return -1l;
  }

  recalculateData();

  if (sapiToSurfData->toFreeText != NULL) free(sapiToSurfData->toFreeText);
  sapiToSurfData->toFreeText =
      (SurfFreeText*)calloc(1, SIZE_OF_FREE_TEXT_ARRAY(20));
  if (sapiToSurfData->toFreeText != NULL) {
    sapiToSurfData->nrFreeTextUnits=20;
    sprintf(sapiToSurfData->toFreeText->label, SURF_FREE_TEXT_LABEL);
    sprintf(sapiToSurfData->toFreeText->blocks[0].text, "%s%s%s",
            "@(", "#)", "This SURF-Dataset was NOT generated by STN-Atlas !");
  }

  char filesix[300];
  strncpy(filesix, surfDir, 250);
  strcat(filesix, "/");
  strcat(filesix, surfFile);
  strcat(filesix, ".six");

  char filesda[300];
  strncpy(filesda, surfDir, 250);
  strcat(filesda, "/");
  strcat(filesda, surfFile);
  strcat(filesda, ".sda");

  XdrSurf ret = mem_WriteSdaStructure(filesda, sapiToSurfData);
  if (ret == SURF_SUCCESS) {
    ret = mem_WriteSixStructure(filesix, sapiToSurfData);
    if (ret != SURF_SUCCESS) {
      if (errorprint != 0)
        fprintf(stderr, "SAPI-Error: Can't write back file: '%s' !\n", filesix);
      return -1;
    }
  } else {
    if (errorprint != 0)
      fprintf(stderr, "SAPI-Error: Can't write back file: '%s' !\n", filesda);
    return -1;
  }

  return 0l;
}
#endif // #if 0 SAPI_writeBackFromMemory
