//  See README file for copying and redistribution conditions.

#include <math.h>
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

extern SurfDataInfo *sapiToSurfData;
extern SurfSoundingData *sapiToSdaBlock;

long SAPI_getXYZfromMultibeamSounding(
    long beam, long depthOverChartZero,
    double *north, double *east, double *depth) {
  SdaInfo *sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return -1;
  if (sapiToSdaInfo->toMultiBeamDepth == NULL) return -1;
  if ((beam >= 0) && (beam<SAPI_getNrBeams())) {
    double dynChartZero, chartZero;
    const u_short soundingFlag = (u_short)sapiToSdaInfo->toSoundings->soundingFlag;
    if ((soundingFlag & SF_DELETED) != 0) return -1;
    const u_short beamFlag = (u_short)sapiToSdaInfo->toMultiBeamDepth[beam].depthFlag;
    if ((beamFlag & (SB_DELETED+SB_DEPTH_SUPPRESSED+SB_REDUCED_FAN)) != 0) return -1;

    double myDepth = (double)sapiToSdaInfo->toMultiBeamDepth[beam].depth;
    const double posAhead = (double)sapiToSdaInfo->toMultiBeamDepth[beam].beamPositionAhead;
    const double posAstar = (double)sapiToSdaInfo->toMultiBeamDepth[beam].beamPositionStar;
    if (depthOverChartZero != 0) {
      chartZero = (double)sapiToSurfData->toGlobalData->chartZero;
      dynChartZero = (double)sapiToSdaInfo->toSoundings->dynChartZero;
      myDepth = myDepth + chartZero + dynChartZero;
    }
    const double heading = (double)sapiToSdaInfo->toSoundings->headingWhileTransmitting;
    const double refPosX = sapiToSurfData->toGlobalData->referenceOfPositionX;
    const double refPosY = sapiToSurfData->toGlobalData->referenceOfPositionY;

    const double posX = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionX + refPosX;
    const double posY = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionY + refPosY;

    const double cosHeading = cos(heading);
    const double sinHeading = sin(heading);
    double xM = (posAhead*sinHeading) + (posAstar * cosHeading);
    double yM = (posAhead*cosHeading) - (posAstar * sinHeading);
    // rad presentation
    if (SAPI_posPresentationIsRad() != 0) {
      yM = M_TO_RAD_Y(yM);
      xM = M_TO_RAD_X(xM, posY);
    }
    *east = posX + xM;
    *north = posY + yM;
    *depth = myDepth;
    return 0;
  }

  return -1;
}

static long SAPI_getXYZfromSinglebeamSounding(
    char layer, long depthOverChartZero,
    double *north, double *east, double *depth) {
  SdaInfo *sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return -1;
  if (sapiToSdaInfo->toSingleBeamDepth == NULL) return -1;

  const u_short soundingFlag = (u_short)sapiToSdaInfo->toSoundings->soundingFlag;
  if ((soundingFlag & SF_DELETED) != 0) return -1;
  const u_short beamFlag = (u_short)sapiToSdaInfo->toSingleBeamDepth->depthFlag;
  if ((beamFlag & (SB_DELETED+SB_DEPTH_SUPPRESSED)) != 0) return -1;

  double myDepth;
  switch(layer) {
  case 'H':
        myDepth = (double)sapiToSdaInfo->toSingleBeamDepth->depthHFreq;
        break;
  case 'M':
        myDepth = (double)sapiToSdaInfo->toSingleBeamDepth->depthMFreq;
        break;
  case 'L':
        myDepth = (double)sapiToSdaInfo->toSingleBeamDepth->depthLFreq;
        break;
  default:
        myDepth = 0.0;
        break;
  }

  if (depthOverChartZero != 0) {
    double chartZero = (double)sapiToSurfData->toGlobalData->chartZero;
    double dynChartZero = (double)sapiToSdaInfo->toSoundings->dynChartZero;
    myDepth = myDepth + chartZero + dynChartZero;
  }

  const double refPosX = sapiToSurfData->toGlobalData->referenceOfPositionX;
  const double refPosY = sapiToSurfData->toGlobalData->referenceOfPositionY;

  const double posX = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionX + refPosX;
  const double posY = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionY + refPosY;

  *east = posX;
  *north = posY;
  *depth = myDepth;

  return 0;
}

long SAPI_getXYZfromSinglebeamSoundingHF(
    long depthOverChartZero, double *north, double *east, double *depth) {
  SdaInfo *sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return -1;
  if (SAPI_dataHaveHighFrequencyLayer()==0) return -1;
  return SAPI_getXYZfromSinglebeamSounding('H', depthOverChartZero,north,east,depth);
}

long SAPI_getXYZfromSinglebeamSoundingMF(
    long depthOverChartZero, double *north, double *east, double *depth) {
  SdaInfo *sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return -1;
  if (SAPI_dataHaveMediumFrequencyLayer()==0) return -1;
  return SAPI_getXYZfromSinglebeamSounding('M', depthOverChartZero,north,east,depth);
}

long SAPI_getXYZfromSinglebeamSoundingLF(
    long depthOverChartZero, double *north, double *east, double *depth)
{
  SdaInfo *sapiToSdaInfo = NULL;

  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return -1;
  if (SAPI_dataHaveLowFrequencyLayer()==0) return -1;
  return SAPI_getXYZfromSinglebeamSounding('L', depthOverChartZero,north,east,depth);
}



