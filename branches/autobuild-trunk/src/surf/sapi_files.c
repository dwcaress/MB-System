/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


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
#include "sapi.h"

#define SAPI_VERSION "SAPI V3.1.4"

#ifdef _WIN32
#include <io.h>
#define access _access
#endif

extern XdrSurf mem_convertOneSdaBlock2(XDR* xdrs,SdaInfo* sdaInfo,short versLess2);
extern size_t initializeSdaInfo(SurfDataInfo* toSurfDataInfo,SdaInfo* toSdaInfo);
extern void setPointersInSdaInfo(void* toSdaBlock,SdaInfo* toSdaInfo);
long SAPI_openFile(char* surfDir,char* surfFile,long errorprint);


SurfDataInfo*      sapiToSurfData;
SurfSoundingData*  sapiToSdaBlock;
Boolean            loadIntoMemory=False;


void SAPI_printAPIandSURFversion(void)
{
 fprintf(stderr,"Version: %s\n         %s\n",SAPI_VERSION,SURF_VERSION);
}


static void freeControlData(void)
{
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL)
 {
  sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if(sapiToSurfData->xdrs != NULL)
  {
   free((char*)sapiToSurfData->xdrs);
   sapiToSurfData->xdrs=NULL;
  }
  if(sapiToSurfData->fp != NULL)fclose(sapiToSurfData->fp);
  free((char*)sapiToSurfData);
  sapiToSurfData=NULL;
 }
 if(sapiToSdaInfo != NULL && loadIntoMemory != True )
     free((char*)sapiToSdaInfo);
 if(sapiToSdaBlock != NULL) free((char*)sapiToSdaBlock);
 sapiToSurfData = NULL;
 sapiToSdaInfo = NULL;
 sapiToSdaBlock = NULL;
}



long SAPI_openIntoMemory(char* surfDir,char* surfFile,long errorprint)
{
 loadIntoMemory=True;
 return(SAPI_openFile(surfDir,surfFile,errorprint));
}

long SAPI_open(char* surfDir,char* surfFile,long errorprint)
{
 loadIntoMemory=False;
 return(SAPI_openFile(surfDir,surfFile,errorprint));
}




long SAPI_openFile(char* surfDir,char* surfFile,long errorprint)
{
 SdaInfo* sapiToSdaInfo = NULL;
 char filesix[300];
 char filesda[300];
 XdrSurf ret;
 size_t  sizeOfSdaBlock;


 if(access(surfDir,0) != 0)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't access path: '%s' !\n",surfDir);
  return((long)-1);
 }

 freeControlData();

 sapiToSurfData = (SurfDataInfo*)calloc(1,sizeof(SurfDataInfo));
 if (sapiToSurfData == NULL)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  return((long)-1);
 }

 strncpy(filesix,surfDir,250);
 strcat(filesix,"/");
 strcat(filesix,surfFile);
 strcat(filesix,".six");

 strncpy(filesda,surfDir,250);
 strcat(filesda,"/");
 strcat(filesda,surfFile);
 strcat(filesda,".sda");

 if(access(filesix,4) !=0)
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't access file: '%s' !\n",filesix);
  return(-1);
 }
 if(access(filesda,4) !=0)
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't access file: '%s' !\n",filesda);
  return(-1);
 }

 ret = mem_ReadSixStructure(filesix,sapiToSurfData);
 if(ret != SURF_SUCCESS)
 {
  mem_destroyAWholeSurfStructure(sapiToSurfData);
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't read file: '%s' !\n",filesix);
  return((long)-1);
 }

 /* special mode for rewrite read the whole surfFile into memory */

 if(loadIntoMemory==True)
 {
  ret = mem_ReadSdaStructure(filesda,sapiToSurfData);
  if(ret != SURF_SUCCESS)
  {
   mem_destroyAWholeSurfStructure(sapiToSurfData);
   freeControlData();
   if(errorprint != 0)
     fprintf(stderr,"SAPI-Error: Can't read file: '%s' !\n",filesda);
   return((long)-1);
  }
  surf_moveInSdaThread(sapiToSurfData,ABS_POSITION,0);
  return((long)0);
 }

 /* Allocate the necessary memory for a SDA-structure and read
    the first SDA-Block from file; */

 if(sapiToSurfData->nrOfSoundings <=0)
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: nr of soundings = %d!\n",(int)(sapiToSurfData->nrOfSoundings));
  return((long)-1);
 }

 /* allocate structure for xdr-conversion and SdaInfo and Sda-Thread */

 sapiToSdaInfo = (SdaInfo*)calloc(1,sizeof(SdaInfo));
 sapiToSurfData->toSdaInfo = sapiToSdaInfo;

 sapiToSurfData->xdrs = (XDR*)calloc(1,sizeof(XDR));  /* ????? */

 if((sapiToSurfData->xdrs == NULL) || (sapiToSdaInfo == NULL))
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  return((long)-1);
 }

 sizeOfSdaBlock = initializeSdaInfo(sapiToSurfData,sapiToSdaInfo);
 sapiToSdaBlock = (SurfSoundingData*)calloc(1,sizeOfSdaBlock);

 if((sapiToSurfData->xdrs == NULL) || (sapiToSdaInfo == NULL))
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  return((long)-1);
 }

 setPointersInSdaInfo(sapiToSdaBlock,sapiToSdaInfo);

 /* open file Read and read first Block */

 sapiToSurfData->fp = xdrSurfOpenRead(sapiToSurfData->xdrs,(const char*)filesda);
 if(sapiToSurfData->fp == NULL)
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't open file: '%s' !\n",filesda);
  return((long)-1);
 }

 ret = mem_convertOneSdaBlock2(sapiToSurfData->xdrs,sapiToSdaInfo,sapiToSurfData->sourceVersionLess2);
 if(ret != SURF_SUCCESS)
 {
  freeControlData();
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't read file: '%s' !\n",filesda);
  return((long)-1);
 }

 return((long)0);
}



long SAPI_nextSounding(long errorprint)
{
 XdrSurf ret;
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
 if((sapiToSurfData == NULL) || (sapiToSdaInfo == NULL))
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: No SURF-data open !\n");
  return((long)-1);
 }

 /* special mode for rewrite read the whole surfFile into memory */

 if(loadIntoMemory==True)
 {
  if(surf_moveInSdaThread(sapiToSurfData,FORE_ONE_STEP,0) == END_OF_THREAD)
  {
   if(errorprint != 0)
     fprintf(stderr,"SAPI-Error: End of file !\n");
   return((long)-1);
  }
  return((long)0);
 }

 ret = mem_convertOneSdaBlock2(sapiToSurfData->xdrs,sapiToSdaInfo,sapiToSurfData->sourceVersionLess2);
 if(ret != SURF_SUCCESS)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't read file or EOF !\n");
  return((long)-1);
 }
 return((long)0);
}



long SAPI_rewind(long errorprint)
{
 SdaInfo* sapiToSdaInfo = NULL;

 if(sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;

 /* special mode for rewrite read the whole surfFile into memory */
 if(loadIntoMemory==True)
 {
  if((sapiToSurfData == NULL) || (sapiToSdaInfo == NULL))
  {
   return((long)-1);
  }
  surf_moveInSdaThread(sapiToSurfData,TO_START,0);
  return((long)0);
 }

 if((sapiToSurfData == NULL) || (sapiToSurfData->xdrs == NULL)
  || (sapiToSdaInfo == NULL) || (sapiToSurfData->fp == NULL))
 {
  return((long)-1);
 }

 rewind(sapiToSurfData->fp);
 return(SAPI_nextSounding(errorprint));
}


void SAPI_close(void)
{
 if(sapiToSurfData != NULL)
 {
  if(sapiToSurfData->fp != NULL)fclose(sapiToSurfData->fp);
  sapiToSurfData->fp = NULL;
  mem_destroyAWholeSurfStructure(sapiToSurfData);
  sapiToSurfData = NULL;
 }
#ifndef _WIN32
 freeControlData();
#endif
}





void recalculateData(void)
{
 SurfGlobalData* toGlobalData;
 SurfStatistics* toStatistics;
 SurfMultiBeamAngleTable* toAngles;
 Boolean isPitchcompensated,allBeamsDeleted;
 Boolean depthStatisticsFound = False;
 Boolean posIsMeter = False;
 FanParam fanParam;
 short indexToAngle,indexToTransducer;
 u_short depthFlag,soundingFlag;
 long ii,nrSoundings,firstSounding;
 short beam,nrBeams;
 double tide,depth;
 double minDef= 999999999.0;
 double maxDef=-999999999.0;
 double minBeamPositionStar,maxBeamPositionStar;
 double minBeamPositionAhead,maxBeamPositionAhead;
 double minDepth,maxDepth;
 double minX,maxX,minY,maxY,refX,refY,relWay,relTime;
 double posX,posY,deltaX,deltaY,lastX,lastY;
 double speed,minSpeed,maxSpeed;
 double roll,minRoll,maxRoll;
 double minPitch,maxPitch;
 double minHeave,maxHeave;
 double cmean;

 if(sapiToSurfData!=NULL)
 {
  toGlobalData = sapiToSurfData->toGlobalData;
  toStatistics = sapiToSurfData->toStatistics;

  nrBeams = (short) sapiToSurfData->nrBeams;
  nrSoundings = sapiToSurfData->nrOfSoundings;

  if(toGlobalData->presentationOfPosition=='X')
       posIsMeter=True;

  isPitchcompensated = True;
  if(strncmp(toGlobalData->nameOfSounder,"MD",2) == 0)
       isPitchcompensated = False;
  if(strncmp(toGlobalData->nameOfSounder,"FS",2) == 0)
       isPitchcompensated = False;


  if(toGlobalData->typeOfSounder != 'F')
  {
   minBeamPositionStar  = 0.0;
   maxBeamPositionStar  = 0.0;
   minBeamPositionAhead = 0.0;
   maxBeamPositionAhead = 0.0;
  }
  else
  {
   minBeamPositionStar  = minDef;
   maxBeamPositionStar  = maxDef;
   minBeamPositionAhead = minDef;
   maxBeamPositionAhead = maxDef;
  }
  minDepth = minDef;
  maxDepth = maxDef;
  minX = minDef;
  minY = minDef;
  maxX = maxDef;
  maxY = maxDef;

  minSpeed=minRoll=minPitch=minHeave=minDef;
  maxSpeed=maxRoll=maxPitch=maxHeave=maxDef;

  refX  = toGlobalData->referenceOfPositionX;
  refY  = toGlobalData->referenceOfPositionY;
  posX = posY = 0.0;
  relWay = 0.0;
  relTime=0.0;

  firstSounding = -1;
  for(ii=0;ii<nrSoundings;ii++)
  {
   surf_moveInSdaThread(sapiToSurfData,ABS_POSITION,ii);
   soundingFlag = sapiToSurfData->toSdaInfo->toSoundings->soundingFlag;

   if((soundingFlag & (SF_DELETED | SF_ALL_BEAMS_DELETED)) == 0)
   {
    firstSounding ++;
    relTime = (double) sapiToSurfData->toSdaInfo->toSoundings->relTime;
    posX = (double)
         (sapiToSurfData->toSdaInfo->toActCenterPosition->centerPositionX) + refX;
    posY = (double)
         (sapiToSurfData->toSdaInfo->toActCenterPosition->centerPositionY) + refY;
    speed = (double)
         (sapiToSurfData->toSdaInfo->toActCenterPosition->speed);
    if(firstSounding == 0)
    {
     lastX = posX;
     lastY = posY;
    }
    if(posX > maxX) maxX = posX;
    if(posX < minX) minX = posX;
    if(posY > maxY) maxY = posY;
    if(posY < minY) minY = posY;
    if(speed > maxSpeed) maxSpeed = speed;
    if(speed < minSpeed) minSpeed = speed;

    if(posIsMeter==True)
    {
     deltaX = posX - lastX;
     deltaY = posY - lastY;
    }
    else
    {
     deltaX = setToPlusMinusPI(posX - lastX);
     deltaY = setToPlusMinusPI(posY - lastY);
     deltaY = RAD_TO_METER_Y(deltaY);
     deltaX = RAD_TO_METER_X(deltaX,lastY);
    }
    relWay = relWay + sqrt((deltaX*deltaX) + (deltaY*deltaY));
    sapiToSurfData->toSdaInfo->toSoundings->relWay = (float)relWay;
    if(firstSounding == 0)
    {
     toGlobalData->modifiedTrackStartX = (float)(posX - refX);
     toGlobalData->modifiedTrackStartY = (float)(posY - refY);
    }
    lastX = posX;
    lastY = posY;


    tide = (double)sapiToSurfData->toSdaInfo->toSoundings->tide;

    roll =
        (double)sapiToSurfData->toSdaInfo->toSoundings->rollWhileTransmitting;
    fanParam.pitchTx =
        (double)sapiToSurfData->toSdaInfo->toSoundings->pitchWhileTransmitting
       +(double)sapiToSurfData->toGlobalData->offsetPitchFore;
    fanParam.heaveTx = (double)
        sapiToSurfData->toSdaInfo->toSoundings->heaveWhileTransmitting;
    fanParam.ckeel   = (double)
        sapiToSurfData->toSdaInfo->toSoundings->cKeel;
    fanParam.cmean   = (double)
        sapiToSurfData->toSdaInfo->toSoundings->cMean;

    if(roll > maxRoll) maxRoll = roll;
    if(roll < minRoll) minRoll = roll;
    if(fanParam.pitchTx > maxPitch) maxPitch = fanParam.pitchTx;
    if(fanParam.pitchTx < minPitch) minPitch = fanParam.pitchTx;
    if(fanParam.heaveTx > maxHeave) maxHeave = fanParam.heaveTx;
    if(fanParam.heaveTx < minHeave) minHeave = fanParam.heaveTx;

    if(toGlobalData->typeOfSounder == 'F')
    {
     indexToAngle =
             (short)sapiToSurfData->toSdaInfo->toSoundings->indexToAngle;
     toAngles = getSurfAngleTable(sapiToSurfData->toAngleTables,
                                                   nrBeams,indexToAngle);

     allBeamsDeleted=True;
     for(beam = 0;beam < nrBeams;beam++)
     {
      depthFlag = sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag;
      if((depthFlag & SB_DELETED) == 0)
      {
       allBeamsDeleted=False;
       fanParam.angle = toAngles->beamAngle[beam];
       indexToTransducer =
             (short)sapiToSurfData->toSdaInfo->toSoundings->indexToTransducer;
       if((sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag & SB_TRANSDUCER_PLUS1) != 0)
             indexToTransducer++;
       fanParam.draught = (double)
         sapiToSurfData->toTransducers[indexToTransducer].transducerDepth;
       fanParam.transducerOffsetAhead = (double)
         sapiToSurfData->toTransducers[indexToTransducer].transducerPositionAhead;
       fanParam.transducerOffsetStar  = (double)
         sapiToSurfData->toTransducers[indexToTransducer].transducerPositionStar;
       if(sapiToSurfData->toSdaInfo->toMultiBeamRec != NULL)
         fanParam.heaveRx = (double)
               sapiToSurfData->toSdaInfo->toMultiBeamRec[beam].heaveWhileReceiving;
       else
         fanParam.heaveRx = 0.0;
       fanParam.travelTime = (double)
         sapiToSurfData->toSdaInfo->toMultiBeamTT[beam].travelTimeOfRay;

       if(depthFromTT(&fanParam,isPitchcompensated) == True)
       {
        depth=fanParam.depth - tide;
        sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depth = (float)depth;
        sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].beamPositionAhead =
                                                 (float)fanParam.posAhead;
        sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].beamPositionStar  =
                                                 (float)fanParam.posStar;
        if(depth < minDepth)
          minDepth = depth;
        if(depth > maxDepth)
          maxDepth = depth;
        if(fanParam.posStar < minBeamPositionStar)
          minBeamPositionStar = fanParam.posStar;
        if(fanParam.posStar > maxBeamPositionStar)
          maxBeamPositionStar = fanParam.posStar;
        if(fanParam.posAhead < minBeamPositionAhead)
          minBeamPositionAhead = fanParam.posAhead;
        if(fanParam.posAhead > maxBeamPositionAhead)
          maxBeamPositionAhead = fanParam.posAhead;
        depthStatisticsFound=True;
       }
       else
       {
        sapiToSurfData->toSdaInfo->toMultiBeamDepth[beam].depthFlag =
                                                     depthFlag | SB_DELETED;
       }
      }
     } /*for(beam = 0;beam < nrBeams;beam++)*/
     if(allBeamsDeleted==True)
     {
      sapiToSurfData->toSdaInfo->toSoundings->soundingFlag =
               soundingFlag | SF_DELETED | SF_ALL_BEAMS_DELETED;
     }
    } /*if(toGlobalData->typeOfSounder == 'F')*/
    else
    {
     cmean = (double)sapiToSurfData->toSdaInfo->toSoundings->cMean;
     sapiToSurfData->toSdaInfo->toSoundings->cKeel = (float)cmean;
     if((sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthFlag & SB_DELETED) == 0)
     {
      depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthLFreq;
      if(depth != 0.0)
      {
       if(depth < minDepth)
          minDepth = depth;
       if(depth > maxDepth)
          maxDepth = depth;
       depthStatisticsFound=True;
      }
      depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthMFreq;
      if(depth != 0.0)
      {
       if(depth < minDepth)
          minDepth = depth;
       if(depth > maxDepth)
          maxDepth = depth;
       depthStatisticsFound=True;
      }
      depth = (double)sapiToSurfData->toSdaInfo->toSingleBeamDepth->depthHFreq;
      if(depth != 0.0)
      {
       if(depth < minDepth)
          minDepth = depth;
       if(depth > maxDepth)
          maxDepth = depth;
       depthStatisticsFound=True;
      }
     }
    }
   } /*if((soundingFlag & SF_DELETED) == 0)*/
  } /*for(ii=0;ii<nrSoundings;ii++)*/

  if(depthStatisticsFound==False)
  {
   minDepth = maxDepth = 0.0;
   minBeamPositionStar = maxBeamPositionStar = 0.0;
   minBeamPositionAhead = maxBeamPositionAhead = 0.0;
   minX = maxX = minY = maxY = 0.0;
  }

  toStatistics->minDepth  = (float)minDepth;
  toStatistics->maxDepth  = (float)maxDepth;
  toStatistics->minBeamPositionStar  = (float)minBeamPositionStar;
  toStatistics->maxBeamPositionStar  = (float)maxBeamPositionStar;
  toStatistics->minBeamPositionAhead  = (float)minBeamPositionAhead;
  toStatistics->maxBeamPositionAhead  = (float)maxBeamPositionAhead;

  toStatistics->minEasting  = minX;
  toStatistics->maxEasting  = maxX;
  toStatistics->minNorthing = minY;
  toStatistics->maxNorthing = maxY;

  toStatistics->minSpeed=(float)minSpeed;
  toStatistics->maxSpeed=(float)maxSpeed;
  toStatistics->minRoll=(float)minRoll;
  toStatistics->maxRoll=(float)maxRoll;
  toStatistics->minPitch=(float)minPitch;
  toStatistics->maxPitch=(float)maxPitch;
  toStatistics->minHeave=(float)minHeave;
  toStatistics->maxHeave=(float)maxHeave;

  toGlobalData->modifiedTrackStopX  = (float)(posX - refX);
  toGlobalData->modifiedTrackStopY  = (float)(posY - refY);
  toGlobalData->modifiedStartStopDistance = (float)(relWay);
  toGlobalData->originalTrackStartX = toGlobalData->modifiedTrackStartX;
  toGlobalData->originalTrackStartY = toGlobalData->modifiedTrackStartY;
  toGlobalData->originalTrackStopX  = toGlobalData->modifiedTrackStopX;
  toGlobalData->originalTrackStopY  = toGlobalData->modifiedTrackStopY;
  toGlobalData->originalStartStopDistance = (float)(relWay);
  toGlobalData->originalStartStopTime     = relTime;
 }
}





long SAPI_writeBackFromMemory(char* surfDir,char* surfFile,long errorprint)
{
 char filesix[300];
 char filesda[300];
 XdrSurf ret;

 if(sapiToSurfData==NULL)
 {
  if(errorprint != 0)
   fprintf(stderr,
       "SAPI-Error: There is no open SURF-file for writing back !\n");
  return((long)-1);
 }

 if(loadIntoMemory==False)
 {
  if(errorprint != 0)
   fprintf(stderr,
    "SAPI-Error: For writing back you have to open\n     the file with SAPI_openIntoMemory(..)!\n");
  return((long)-1);
 }

 if(access(surfDir,0) != 0)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't access path: '%s' !\n",surfDir);
  return((long)-1);
 }

 recalculateData();

 if(sapiToSurfData->toFreeText != NULL) free(sapiToSurfData->toFreeText);
 sapiToSurfData->toFreeText =
        (SurfFreeText*)calloc(1,SIZE_OF_FREE_TEXT_ARRAY(20));
 if(sapiToSurfData->toFreeText != NULL)
 {
  sapiToSurfData->nrFreeTextUnits=20;
  sprintf(sapiToSurfData->toFreeText->label,SURF_FREE_TEXT_LABEL);
  sprintf(sapiToSurfData->toFreeText->blocks[0].text,"%s%s%s",
                 "@(","#)","This SURF-Dataset was NOT generated by STN-Atlas !");
 }

 strncpy(filesix,surfDir,250);
 strcat(filesix,"/");
 strcat(filesix,surfFile);
 strcat(filesix,".six");

 strncpy(filesda,surfDir,250);
 strcat(filesda,"/");
 strcat(filesda,surfFile);
 strcat(filesda,".sda");

 ret = mem_WriteSdaStructure(filesda,sapiToSurfData);
 if(ret == SURF_SUCCESS)
 {
  ret = mem_WriteSixStructure(filesix,sapiToSurfData);
  if(ret != SURF_SUCCESS)
  {
   if(errorprint != 0)
     fprintf(stderr,"SAPI-Error: Can't write back file: '%s' !\n",filesix);
   return(-1);
  }
 }
 else
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't write back file: '%s' !\n",filesda);
  return(-1);
 }

 return((long)0);
}


