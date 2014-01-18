/*
/  See README file for copying and redistribution conditions.
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <string.h>
#include "types_win32.h"
#else
#include <string.h>
#include <unistd.h>
#endif
#include <math.h>


#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"
#include "pb_math.h"
#define  __SAPI__
#include "mb_sapi.h"



extern SurfDataInfo*      sapiToSurfData;
extern SurfSoundingData*  sapiToSdaBlock;



long SAPI_getXYZfromMultibeamSounding(long beam,long depthOverChartZero,
                                      double* north,double* east,double* depth)
{
 u_short soundingFlag,beamFlag;
 double posX,posY,heading,posAhead,posAstar,refPosX,refPosY;
 double myDepth,dynChartZero,chartZero;
 double cosHeading,sinHeading,xM,yM;
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if(sapiToSdaInfo == NULL) return(-1);
 if(sapiToSdaInfo->toMultiBeamDepth == NULL) return(-1);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  soundingFlag = (u_short)sapiToSdaInfo->toSoundings->soundingFlag;
  if((soundingFlag & SF_DELETED) != 0) return(-1);
  beamFlag = (u_short)sapiToSdaInfo->toMultiBeamDepth[beam].depthFlag;
  if((beamFlag & (SB_DELETED+SB_DEPTH_SUPPRESSED+SB_REDUCED_FAN)) != 0) return(-1);

  myDepth = (double)sapiToSdaInfo->toMultiBeamDepth[beam].depth;
  posAhead = (double)sapiToSdaInfo->toMultiBeamDepth[beam].beamPositionAhead;
  posAstar = (double)sapiToSdaInfo->toMultiBeamDepth[beam].beamPositionStar;
  if(depthOverChartZero != 0)
  {
   chartZero = (double)sapiToSurfData->toGlobalData->chartZero;
   dynChartZero = (double)sapiToSdaInfo->toSoundings->dynChartZero;
   myDepth = myDepth + chartZero + dynChartZero;
  }
  heading = (double)sapiToSdaInfo->toSoundings->headingWhileTransmitting;
  refPosX = sapiToSurfData->toGlobalData->referenceOfPositionX;
  refPosY = sapiToSurfData->toGlobalData->referenceOfPositionY;

  posX = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionX + refPosX;
  posY = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionY + refPosY;

  cosHeading = cos(heading);
  sinHeading = sin(heading);
  xM = (posAhead*sinHeading) + (posAstar * cosHeading);
  yM = (posAhead*cosHeading) - (posAstar * sinHeading);
  if(SAPI_posPresentationIsRad() != 0) /* rad-Presentation */
  {
   yM = M_TO_RAD_Y(yM);
   xM = M_TO_RAD_X(xM,posY);
  }
  *east  = posX + xM;
  *north  = posY + yM;
  *depth  = myDepth;
  return(0);
 }
 return(-1);
}




static long SAPI_getXYZfromSinglebeamSounding(char layer,long depthOverChartZero,
                                              double* north,double* east,double* depth)
{
 u_short soundingFlag,beamFlag;
 double posX,posY,refPosX,refPosY;
 double myDepth,dynChartZero,chartZero;
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if(sapiToSdaInfo == NULL) return(-1);
 if(sapiToSdaInfo->toSingleBeamDepth == NULL) return(-1);

 soundingFlag = (u_short)sapiToSdaInfo->toSoundings->soundingFlag;
 if((soundingFlag & SF_DELETED) != 0) return(-1);
 beamFlag = (u_short)sapiToSdaInfo->toSingleBeamDepth->depthFlag;
 if((beamFlag & (SB_DELETED+SB_DEPTH_SUPPRESSED)) != 0) return(-1);

 switch(layer)
 {
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
 if(depthOverChartZero != 0)
 {
  chartZero = (double)sapiToSurfData->toGlobalData->chartZero;
  dynChartZero = (double)sapiToSdaInfo->toSoundings->dynChartZero;
  myDepth = myDepth + chartZero + dynChartZero;
 }
 refPosX = sapiToSurfData->toGlobalData->referenceOfPositionX;
 refPosY = sapiToSurfData->toGlobalData->referenceOfPositionY;

 posX = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionX + refPosX;
 posY = (double)sapiToSdaInfo->toCenterPositions[0].centerPositionY + refPosY;

 *east  = posX;
 *north  = posY;
 *depth  = myDepth;
 return(0);
}


long SAPI_getXYZfromSinglebeamSoundingHF(long depthOverChartZero,
                                         double* north,double* east,double* depth)
{
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if(sapiToSdaInfo == NULL) return(-1);
 if(SAPI_dataHaveHighFrequencyLayer()==0) return(-1);
 return(SAPI_getXYZfromSinglebeamSounding('H',depthOverChartZero,north,east,depth));
}


long SAPI_getXYZfromSinglebeamSoundingMF(long depthOverChartZero,
                                         double* north,double* east,double* depth)
{
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if(sapiToSdaInfo == NULL) return(-1);
 if(SAPI_dataHaveMediumFrequencyLayer()==0) return(-1);
 return(SAPI_getXYZfromSinglebeamSounding('M',depthOverChartZero,north,east,depth));
}


long SAPI_getXYZfromSinglebeamSoundingLF(long depthOverChartZero,
                                         double* north,double* east,double* depth)
{
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if(sapiToSdaInfo == NULL) return(-1);
 if(SAPI_dataHaveLowFrequencyLayer()==0) return(-1);
 return(SAPI_getXYZfromSinglebeamSounding('L',depthOverChartZero,north,east,depth));
}



