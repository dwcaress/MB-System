/*-----------------------------------------------------------------------
/ P R O G R A M M K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : xdr_surf.c Version 3.0
/  ERSTELLUNGSDATUM : 28.07.93
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ COPYRIGHT (C) 1993: ATLAS ELEKTRONIK GMBH, 28305 BREMEN
/ ------------------------------------------------------------------------
/
/  See README file for copying and redistribution conditions.
/
/
/ HIER/SACHN: P: RP ____ _ ___ __
/ BENENNUNG :
/ ERSTELLER : Peter Block    : SAS3
/ FREIGABE  : __.__.__  GS__
/ AEND/STAND: __.__.__  __
/ PRUEFVERM.:
*/

/* Changed long values to int and xdr_long() calls to xdr_int() so that
	the code works with existing data on 64 bit compilers
	David W. Caress
	February 2, 2010 */

#define _XDR_SURF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xdr_surf.h"



/* ***********************************************************************
*                                                                        *
*  ENDE DES DEKLARATIONSTEILS                                            *
*                                                                        *
*********************************************************************** */





/***************************************/
/*                                     */
/* SURF-stringconversions              */
/*                                     */
/***************************************/



XdrSurf xdr_SurfString(XDR *xdrs,char *gp)
{
  u_int sizeS;

  sizeS=STRING_SIZE; 
  return(xdr_bytes(xdrs,&gp,&sizeS,sizeS));
}

 
XdrSurf xdr_SurfText(XDR *xdrs,char *gp)
{
  u_int sizeS;

  sizeS=TEXT_SIZE; 
  return(xdr_bytes(xdrs,&gp,&sizeS,sizeS));
}

 
XdrSurf xdr_SurfTime(XDR *xdrs,char *gp)
{
  u_int sizeT;

  sizeT=TIME_SIZE; 
  return(xdr_bytes(xdrs,&gp,&sizeT,sizeT));
}






/***************************************/
/*                                     */
/* SURF-filehandles                    */
/*                                     */
/***************************************/



/* handle XDR-formatted files for READ */

FILE* xdrSurfOpenRead(XDR *xdrs,const char* filename)
{
  FILE* fp;

  fp = fopen(filename,"rb+");
  if(fp != NULL)
  {
    xdrstdio_create(xdrs,fp,XDR_DECODE);
  } 
  return(fp);
}




/* handle XDR-formatted files for WRITE */

FILE* xdrSurfOpenWrite(XDR *xdrs,const char* filename)
{
  FILE* fp;

  fp = fopen(filename,"wb+");
  if(fp != NULL)
  {
    xdrstdio_create(xdrs,fp,XDR_ENCODE);
  } 
  return(fp);
}



/***************************************/
/*                                     */
/* SURF-conversions for     SIX-files  */
/*                                     */
/***************************************/


short getSurfVersion(char* version)
{
 if(strlen(version) != 9) return(0);
 if(strncmp(version,"SURF ",5) != 0) return(0);
 return(((short)(version[6]))*256 + ((short)(version[8])));
}


 
XdrSurf xdr_SurfCheckVersion(XDR *xdrs,SurfDescriptor *gp,char* label,
                                  short* newVersion,short* oldVersion)
{
  u_int sizeL;
  char* toLabel;

  /* Get the label and check the consistency of the stream */

  toLabel = (char *)gp->label;
  sizeL=LABEL_SIZE; 
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  *oldVersion = getSurfVersion(gp->label);
  *newVersion = getSurfVersion(SURF_VERSION);
  if(*oldVersion == 0) return(SURF_WRONG_VERSION);
  if(*oldVersion > *newVersion) return(SURF_WRONG_VERSION);
  strcpy((char *)gp->label,SURF_VERSION);
  return(SURF_SUCCESS);
}


void insertDefaultSixDescriptor(short typ,SurfSixDescriptor *gp)
{
 gp->typ = typ;
 gp->nr  = 0l;
}
    
    
void insertDefaultSdaDescriptor(short typ,SurfSdaDescriptor *gp)
{
 gp->typ = typ;
 gp->nr  = 0l;
}
    
    
void insertDefaultNrOfDescriptor(short typ,SurfNrofDescriptor *gp)
{
 gp->typ = typ;
 gp->nr  = 0l;
}
    
 
XdrSurf xdr_SurfSixDescriptor(XDR *xdrs,SurfSixDescriptor *gp)
{
  return( 
          xdr_short(xdrs,&gp->typ)                         &&
          xdr_u_int(xdrs,&gp->nr)                         );
}

 
XdrSurf xdr_SurfSdaDescriptor(XDR *xdrs,SurfSdaDescriptor *gp)
{
  return(
          xdr_short(xdrs,&gp->typ)                         &&
          xdr_u_int(xdrs,&gp->nr)                         );
}

 
XdrSurf xdr_SurfNrofDescriptor(XDR *xdrs,SurfNrofDescriptor *gp)
{
  return(
          xdr_short(xdrs,&gp->typ)                         &&
          xdr_u_int(xdrs,&gp->nr)                         );
}

 
XdrSurf xdr_SurfDescriptor(XDR *xdrs,SurfDescriptor *gp,
                                  short* newVersion,short* oldVersion)
{
  XdrSurf status;
  short newVers,oldVers,vers20,vers30;

  /* Get the first Label and check the Version of the stream */

  vers20 = getSurfVersion(SURF_VERS2_0);
  vers30 = getSurfVersion(SURF_VERS3_0);

  status = xdr_SurfCheckVersion(xdrs,gp,SURF_VERSION,&newVers,&oldVers);
  if (status != SURF_SUCCESS) return(status);

  *newVersion = newVers;
  *oldVersion = oldVers;

  status =
        ( xdr_short                 (xdrs,&gp->six)                    &&
          xdr_SurfSixDescriptor     (xdrs,&gp->descriptor)             &&
          xdr_SurfSixDescriptor     (xdrs,&gp->globalData)             &&
          xdr_SurfSixDescriptor     (xdrs,&gp->statistics)             &&
          xdr_SurfSixDescriptor     (xdrs,&gp->positionSensor)         &&
          xdr_SurfSixDescriptor     (xdrs,&gp->transducer)             &&
          xdr_SurfSixDescriptor     (xdrs,&gp->angleTab)               &&
          xdr_SurfSixDescriptor     (xdrs,&gp->cProfile)               &&
          xdr_SurfSixDescriptor     (xdrs,&gp->polygon)                &&
          xdr_SurfSixDescriptor     (xdrs,&gp->events)                 &&
          xdr_SurfSixDescriptor     (xdrs,&gp->freeText)               );

  if(oldVers < vers30)
  {
   insertDefaultSixDescriptor(ADDSTATISTICS  ,&gp->addStatistics  );
   insertDefaultSixDescriptor(TPESTATICS     ,&gp->tpeStatics     );
   insertDefaultSixDescriptor(CPROFTPES      ,&gp->cprofTpes      );
   insertDefaultSixDescriptor(FREESIXDESCR   ,&gp->freeSixDescr   );
   insertDefaultSixDescriptor(FREESNDGDESCR  ,&gp->freeSndgDescr  );
   insertDefaultSixDescriptor(FREEBEAMDESCR  ,&gp->freeBeamDescr  );
   insertDefaultSixDescriptor(SIXATTDATA     ,&gp->freeSixAttData );
   insertDefaultSixDescriptor(VENDORTEXT     ,&gp->vendorText     );
  }
  else
  {
   status = status                                                     &&
        ( xdr_SurfSixDescriptor     (xdrs,&gp->addStatistics)          &&
          xdr_SurfSixDescriptor     (xdrs,&gp->tpeStatics)             &&
          xdr_SurfSixDescriptor     (xdrs,&gp->cprofTpes)              &&
          xdr_SurfSixDescriptor     (xdrs,&gp->freeSixDescr)           &&
          xdr_SurfSixDescriptor     (xdrs,&gp->freeSndgDescr)          &&
          xdr_SurfSixDescriptor     (xdrs,&gp->freeBeamDescr)          &&
          xdr_SurfSixDescriptor     (xdrs,&gp->freeSixAttData)         &&
          xdr_SurfSixDescriptor     (xdrs,&gp->vendorText)              );
  }

  status = status                                                      &&
        ( xdr_short                 (xdrs,&gp->sda)                    &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->soundings)              &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->centerPositions)        &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->singleBeamDepth)        &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->multiBeamDepth)         &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->multiBeamTT)            &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->multiBeamRecv)          &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->signalParams)           &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->signalAmplitudes)       );

  if(oldVers < vers20)
  {
   insertDefaultSdaDescriptor(BEAMAMPLITUDES,&gp->beamAmplitudes);
   insertDefaultSdaDescriptor(EXTBEAMAMPLI  ,&gp->extendBeamAmplitudes);
   insertDefaultSdaDescriptor(SIDESCANDATA  ,&gp->sidescanData);
   insertDefaultSdaDescriptor(TXPARMS       ,&gp->txParams);
  }
  else
  {
   status = status                                                     &&
         (xdr_SurfSdaDescriptor     (xdrs,&gp->beamAmplitudes)         &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->extendBeamAmplitudes)   &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->sidescanData)           &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->txParams)               );
  }

  if(oldVers < vers30)
  {
   insertDefaultSdaDescriptor(POSITIONCEP,&gp->positionCpes);
   insertDefaultSdaDescriptor(MULTITPES  ,&gp->multiTpeParams);
   insertDefaultSdaDescriptor(SINGLETPES ,&gp->singleTpeParams);
   insertDefaultSdaDescriptor(SNDGATTDATA,&gp->sndgAttData);
   insertDefaultSdaDescriptor(BEAMATTDATA,&gp->beamAttData);
  }
  else
  {
   status = status                                                     &&
         (xdr_SurfSdaDescriptor     (xdrs,&gp->positionCpes)           &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->multiTpeParams)         &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->singleTpeParams)        &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->sndgAttData)            &&
          xdr_SurfSdaDescriptor     (xdrs,&gp->beamAttData)            );
  }

  status = status &&       
        ( xdr_short                 (xdrs,&gp->nrof)                   &&
          xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfBeams)           &&
          xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfCProfileElements)&&
          xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfPolygonElements) &&
          xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfEvents)          &&
          xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfFreeTextBlocks)  );

  if(oldVers < vers20)
  {
   insertDefaultNrOfDescriptor(MAX_NROF_SIDESCAN_DATA,
                                               &gp->maxNrOfSidescanData);
   insertDefaultNrOfDescriptor(NROF_RX_TVG_SETS,&gp->nrOfRxTvgSets);
   insertDefaultNrOfDescriptor(NROF_TX_TVG_SETS,&gp->nrOfTxTvgSets);
  }
  else
  {
   status = status &&
       ( xdr_SurfNrofDescriptor    (xdrs,&gp->maxNrOfSidescanData)    &&
         xdr_SurfNrofDescriptor    (xdrs,&gp->nrOfRxTvgSets)          &&
         xdr_SurfNrofDescriptor    (xdrs,&gp->nrOfTxTvgSets)          );
  }

  status = status &&       
        ( xdr_short                 (xdrs,&gp->eod)                    );
  return(status);        
}




 
 
 
XdrSurf xdr_SurfGlobalData(XDR *xdrs,SurfGlobalData *gp)
{
  char buffer[LABEL_SIZE];
  u_int sizeL;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_GLOBAL_DATA_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  

    
  return(
    xdr_SurfString(xdrs,(char*)&gp->shipsName)                             &&
    xdr_SurfTime(xdrs,(char*)&gp->startTimeOfProfile)                      &&
    xdr_SurfString(xdrs,(char*)&gp->regionOfProfile)                       &&
    xdr_SurfString(xdrs,(char*)&gp->numberOfProfile)                       &&
    xdr_float(xdrs,&gp->chartZero)                                         &&
    xdr_float(xdrs,&gp->tideZero)                                          &&
    xdr_u_int(xdrs,&gp->numberOfMeasuredSoundings)                        &&
    xdr_u_int(xdrs,&gp->actualNumberOfSoundingSets)                       &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfTideModification)              &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfDepthModification)             &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfPosiModification)              &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfParaModification)              &&
    xdr_u_int(xdrs,&gp->correctedParameterFlags)                          &&
    xdr_float(xdrs,&gp->offsetHeave)                                       &&
    xdr_float(xdrs,&gp->offsetRollPort)                                    &&
    xdr_float(xdrs,&gp->offsetRollStar)                                    &&
    xdr_float(xdrs,&gp->offsetPitchFore)                                   &&
    xdr_float(xdrs,&gp->offsetPitchAft)                                    &&
    xdr_SurfString(xdrs,(char*)&gp->nameOfSounder)                         &&
    xdr_char(xdrs,&gp->typeOfSounder)                                      &&
    xdr_float(xdrs,&gp->highFrequency)                                     &&
    xdr_float(xdrs,&gp->mediumFrequency)                                   &&
    xdr_float(xdrs,&gp->lowFrequency)                                      &&
    xdr_SurfString(xdrs,(char*)&gp->nameOfEllipsoid)                       &&
    xdr_double(xdrs,&gp->semiMajorAxis)                                    &&
    xdr_double(xdrs,&gp->flattening)                                       &&
    xdr_SurfString(xdrs,(char*)&gp->projection)                            &&
    xdr_char(xdrs,&gp->presentationOfPosition)                             &&
    xdr_double(xdrs,&gp->referenceMeridian)                                &&
    xdr_double(xdrs,&gp->falseEasting)                                     &&
    xdr_double(xdrs,&gp->falseNorthing)                                    &&
    xdr_double(xdrs,&gp->referenceOfPositionX)                             &&
    xdr_double(xdrs,&gp->referenceOfPositionY)                             &&
    xdr_char(xdrs,&gp->presentationOfRelWay)                               &&
    xdr_float(xdrs,&gp->planedTrackStartX)                                 &&
    xdr_float(xdrs,&gp->planedTrackStartY)                                 &&
    xdr_float(xdrs,&gp->planedTrackStopX)                                  &&
    xdr_float(xdrs,&gp->planedTrackStopY)                                  &&
    xdr_float(xdrs,&gp->originalTrackStartX)                               &&
    xdr_float(xdrs,&gp->originalTrackStartY)                               &&
    xdr_float(xdrs,&gp->originalTrackStopX)                                &&
    xdr_float(xdrs,&gp->originalTrackStopY)                                &&
    xdr_float(xdrs,&gp->originalStartStopDistance)                         &&
    xdr_double(xdrs,&gp->originalStartStopTime)                            &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfTrackModification)             &&
    xdr_float(xdrs,&gp->modifiedTrackStartX)                               &&
    xdr_float(xdrs,&gp->modifiedTrackStartY)                               &&
    xdr_float(xdrs,&gp->modifiedTrackStopX)                                &&
    xdr_float(xdrs,&gp->modifiedTrackStopY)                                &&
    xdr_float(xdrs,&gp->modifiedStartStopDistance)                         );
}



 
XdrSurf xdr_SurfStatistics(XDR *xdrs,SurfStatistics *gp)
{
  char buffer[LABEL_SIZE];
  u_int sizeL;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_STATISTICS_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  
    
  return(
            xdr_double(xdrs,&gp->minNorthing)                             &&
            xdr_double(xdrs,&gp->maxNorthing)                             &&
            xdr_double(xdrs,&gp->minEasting)                              &&
            xdr_double(xdrs,&gp->maxEasting)                              &&
            xdr_float(xdrs,&gp->   minSpeed)                              &&
            xdr_float(xdrs,&gp->   maxSpeed)                              &&
            xdr_float(xdrs,&gp->minRoll)                                  &&
            xdr_float(xdrs,&gp->maxRoll)                                  &&
            xdr_float(xdrs,&gp->minPitch)                                 &&
            xdr_float(xdrs,&gp->maxPitch)                                 &&
            xdr_float(xdrs,&gp->minHeave)                                 &&
            xdr_float(xdrs,&gp->maxHeave)                                 &&
            xdr_float(xdrs,&gp->minBeamPositionStar)                      &&
            xdr_float(xdrs,&gp->maxBeamPositionStar)                      &&
            xdr_float(xdrs,&gp->minBeamPositionAhead)                     &&
            xdr_float(xdrs,&gp->maxBeamPositionAhead)                     &&
            xdr_float(xdrs,&gp->minDepth)                                 &&
            xdr_float(xdrs,&gp->maxDepth)                                 );
}



 
XdrSurf xdr_SurfPositionPolarfix(XDR *xdrs,SurfPositionPolarfix *gp)
{
    
  return(
     xdr_float(xdrs,&gp->polarfixLocationX)                             &&
     xdr_float(xdrs,&gp->polarfixLocationY)                             &&
     xdr_float(xdrs,&gp->polarfixLocationZ)                             &&
     xdr_float(xdrs,&gp->polarfixReferenceX)                            &&
     xdr_float(xdrs,&gp->polarfixReferenceY)                            &&
     xdr_float(xdrs,&gp->polarfixReferenceZ)                            &&
     xdr_float(xdrs,&gp->polarfixReferenceDistance)                     &&
     xdr_float(xdrs,&gp->polarfixReferenceAngle)                        &&
     xdr_SurfTime(xdrs,(char*)&gp->timeOfLastPolarfixEdit)              &&
     xdr_float(xdrs,&gp->polarfixEditLocationX)                         &&
     xdr_float(xdrs,&gp->polarfixEditLocationY)                         &&
     xdr_float(xdrs,&gp->polarfixEditLocationZ)                         &&
     xdr_float(xdrs,&gp->polarfixEditReferenceX)                        &&
     xdr_float(xdrs,&gp->polarfixEditReferenceY)                        &&
     xdr_float(xdrs,&gp->polarfixEditReferenceZ)                        &&
     xdr_float(xdrs,&gp->polarfixEditReferenceDistance)                 &&
     xdr_float(xdrs,&gp->polarfixEditReferenceAngle)                    &&
     xdr_float(xdrs,&gp->polarfixAntennaPositionAhead)                  &&
     xdr_float(xdrs,&gp->polarfixAntennaPositionStar)                   &&
     xdr_float(xdrs,&gp->polarfixAntennaPositionHeight)                 );
}



XdrSurf xdr_SurfPositionAnySensor(XDR *xdrs,SurfPositionAnySensor* gp)
{
  return(
     xdr_float(xdrs,&gp->none1)                         &&
     xdr_float(xdrs,&gp->none2)                         &&
     xdr_float(xdrs,&gp->none3)                         &&
     xdr_float(xdrs,&gp->none4)                         &&
     xdr_float(xdrs,&gp->none5)                         &&
     xdr_float(xdrs,&gp->none6)                         &&
     xdr_float(xdrs,&gp->none7)                         &&
     xdr_float(xdrs,&gp->none8)                         &&
     xdr_SurfTime(xdrs,(char*)&gp->time9)               &&
     xdr_float(xdrs,&gp->none10)                        &&
     xdr_float(xdrs,&gp->none11)                        &&
     xdr_float(xdrs,&gp->none12)                        &&
     xdr_float(xdrs,&gp->none13)                        &&
     xdr_float(xdrs,&gp->none14)                        &&
     xdr_float(xdrs,&gp->none15)                        &&
     xdr_float(xdrs,&gp->none16)                        &&
     xdr_float(xdrs,&gp->none17)                        &&
     xdr_float(xdrs,&gp->sensorAntennaPositionAhead)    &&
     xdr_float(xdrs,&gp->sensorAntennaPositionStar)     &&
     xdr_float(xdrs,&gp->sensorAntennaPositionHeight)   );
} 




XdrSurf xdr_SurfUnknownPositionSensor(XDR *xdrs,char *gp)
{
  u_int sizeU;

  sizeU=UNION_SIZE;
  return(xdr_bytes(xdrs,&gp,&sizeU,sizeU));
}


 
 
XdrSurf xdr_PositionSensorArray(XDR *xdrs,SurfPositionSensorArray *gp,short oldVers)
{
  SurfPositionAnySensor* toAny;
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL,sizeS;
  char* toLabel;
  short vers30;

  vers30 = getSurfVersion(SURF_VERS3_0);

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  sizeS=STRING_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_POSITION_SENSOR_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  

  /* Try to get the specific Sensor */
  
  if (xdr_SurfString(xdrs,(char*)&gp->positionSensorName) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }

  /* Test if Polarfix */
  
  if(strncpy(buffer,POLARFIX,sizeS) == NULL)
  {
   return(SURF_FAILURE);
  }
  if(strncmp(gp->positionSensorName,(const char*)buffer,sizeS) == 0)
  {
   return(xdr_SurfPositionPolarfix(xdrs,
                            (SurfPositionPolarfix*)gp->sensorUnion));
  }
  else                          
  {
   if(oldVers < vers30)
   {
    ret=xdr_SurfUnknownPositionSensor(xdrs,(char*)&gp->sensorUnion);
    toAny = (SurfPositionAnySensor*) gp->sensorUnion;
    toAny->sensorAntennaPositionAhead = (float)0.0;
    toAny->sensorAntennaPositionStar  = (float)0.0;
    toAny->sensorAntennaPositionHeight= (float)0.0;
    return(ret);
   }
   else
   {
    return(xdr_SurfPositionAnySensor(xdrs,
                            (SurfPositionAnySensor*)gp->sensorUnion));
   }                         
  }  
}

            



 
XdrSurf xdr_SurfMultiBeamAngleTable(XDR *xdrs,
                                    SurfMultiBeamAngleTable *gp,
                                    u_short maxBeamNr)
{
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL;
  u_short n;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_MULTIBEAM_ANGLE_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxBeamNr == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }
  
  ret = xdr_u_short(xdrs,&gp->actualNumberOfBeams);
  for(n = 0;n < maxBeamNr;n++)
  {
   if(xdr_float(xdrs,&gp->beamAngle[n]) != SURF_SUCCESS)
   {
    n = maxBeamNr;
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}

            



 
XdrSurf xdr_SurfTransducerParameterTable(XDR *xdrs,
                                         SurfTransducerParameterTable *gp)
{
  char buffer[LABEL_SIZE];
  u_int sizeL;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_TRANSDUCER_TABLE_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  
    
  return(
            xdr_float(xdrs,&gp->transducerDepth)                &&
            xdr_float(xdrs,&gp->transducerPositionAhead)        &&
            xdr_float(xdrs,&gp->transducerPositionStar)         &&
            xdr_float(xdrs,&gp->transducerTwoThetaHFreq)        &&
            xdr_float(xdrs,&gp->transducerTwoThetaMFreq)        &&
            xdr_float(xdrs,&gp->transducerTwoThetaLFreq)        );
}
 

            



 
XdrSurf xdr_SurfCProfileTable(XDR *xdrs,
                              SurfCProfileTable *gp,
                              u_short maxNrOfElementsPerTable)
{
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL;
  u_short n;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  ret = SURF_SUCCESS;
  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_C_PROFILE_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxNrOfElementsPerTable == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }

  if((xdr_float(xdrs,&gp->relTime)                != SURF_SUCCESS) ||
     (xdr_u_short(xdrs,&gp->numberOfActualValues) != SURF_SUCCESS))
    return(SURF_FAILURE);
  for(n = 0;n < maxNrOfElementsPerTable;n++)
  {
   if((xdr_float(xdrs,&gp->values[n].depth)  != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->values[n].cValue) != SURF_SUCCESS)  )
   {
    n = maxNrOfElementsPerTable;
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}
 

            


XdrSurf xdr_SurfCProfileTableTpes(XDR *xdrs,
                              SurfCProfileTpeTable *gp,
                              u_short maxNrOfElementsPerTable)
{
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL;
  u_short n;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  ret = SURF_SUCCESS;
  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_C_PROFILE_TPE_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxNrOfElementsPerTable == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }

  for(n = 0;n < maxNrOfElementsPerTable;n++)
  {
   if(xdr_float(xdrs,&gp->cpTpes[n])  != SURF_SUCCESS)
   {
    n = maxNrOfElementsPerTable;
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}
 

            



 
XdrSurf xdr_SurfPolygons(XDR *xdrs,
                         SurfPolygons *gp,
                         u_short maxNrOfElementsPerTable)
{
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL;
  u_short n;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_POLYGONS_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxNrOfElementsPerTable == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }
  
  ret = SURF_SUCCESS;
  for(n = 0;n < maxNrOfElementsPerTable;n++)
  {
   if((xdr_double(xdrs,&gp->values[n].polygonX)  != SURF_SUCCESS)  ||
      (xdr_double(xdrs,&gp->values[n].polygonY)  != SURF_SUCCESS)  )
   {
    n = maxNrOfElementsPerTable;
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}
 

            



 
XdrSurf xdr_SurfEventText(XDR *xdrs,char *gp)
{
  u_int sizeE;

  sizeE=EVENT_SIZE; 
  return(xdr_bytes(xdrs,&gp,&sizeE,sizeE));
}


XdrSurf xdr_SurfEvents  (XDR *xdrs,
                         SurfEvents   *gp,
                         u_short maxNrOfElementsPerTable)
{
  XdrSurf ret;
  char buffer[LABEL_SIZE];
  u_int sizeL;
  u_short n;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_EVENT_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxNrOfElementsPerTable == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }
  
  ret = SURF_SUCCESS;
  for(n = 0;n < maxNrOfElementsPerTable;n++)
  {
   if((xdr_double(xdrs,&gp->values[n].positionX)      != SURF_SUCCESS)  ||
      (xdr_double(xdrs,&gp->values[n].positionY)      != SURF_SUCCESS)  ||
      (xdr_float (xdrs,&gp->values[n].relTime)        != SURF_SUCCESS)  ||
      (xdr_SurfEventText(xdrs,(char*)&gp->values[n].text) != SURF_SUCCESS))
   {
    n = maxNrOfElementsPerTable;
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}
 

            



XdrSurf xdr_SurfTpeStatics(XDR *xdrs,SurfTpeStatics *gp)
{
  char buffer[LABEL_SIZE];
  u_int sizeL;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_TPE_STATICS_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  

  return(
    xdr_u_int(xdrs,&gp->tpeFlag       )                       &&
    xdr_SurfTime(xdrs,(char*)&gp->timeDateOfLastTpeCalculation)&&
    xdr_double(xdrs,&gp->ltncyHprMb)                           &&
    xdr_double(xdrs,&gp->ltncyNavHss)                          &&
    xdr_double(xdrs,&gp->initRoll)                             &&
    xdr_double(xdrs,&gp->initPtch)                             &&
    xdr_double(xdrs,&gp->initHve)                              &&
    xdr_double(xdrs,&gp->initYaw)                              &&
    xdr_double(xdrs,&gp->rollRateC)                            &&
    xdr_double(xdrs,&gp->ptchRateC)                            &&
    xdr_double(xdrs,&gp->hveRateC)                             &&
    xdr_double(xdrs,&gp->yawRateC)                             &&
    xdr_double(xdrs,&gp->lvrml)                                &&
    xdr_double(xdrs,&gp->lvrmw)                                &&
    xdr_double(xdrs,&gp->lvrmh)                                &&
    xdr_double(xdrs,&gp->shpFctr)                              &&
    xdr_double(xdrs,&gp->bwx)                                  &&
    xdr_double(xdrs,&gp->bwy)                                  &&
    xdr_double(xdrs,&gp->tmtDurn)                              &&
    xdr_double(xdrs,&gp->dTide)                                &&
    xdr_double(xdrs,&gp->Ss)                                   &&
    xdr_double(xdrs,&gp->detect)                               &&
    xdr_double(xdrs,&gp->Ts)                                   &&
    xdr_double(xdrs,&gp->svTrns)                               &&
    xdr_double(xdrs,&gp->reserve1)                             &&
    xdr_double(xdrs,&gp->reserve2)                             &&
    xdr_double(xdrs,&gp->reserve3)                             &&
    xdr_double(xdrs,&gp->reserve4)                             );
}   





 
XdrSurf xdr_SurfFreeText (XDR *xdrs,
                         SurfFreeText   *gp,
                         u_short maxNrOfElementsPerTable)
{
  XdrSurf ret;
  char* bp;
  char buffer[LABEL_SIZE];
  u_int sizeL,sizeFT;
  char* toLabel;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE; 
  toLabel = (char *)gp->label;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_FREE_TEXT_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }
    
  if(maxNrOfElementsPerTable == 0)
  {
   return(SURF_NR_OF_TABLE_ELEMENTS_ZERO);
  }
  
  ret = SURF_SUCCESS;
  bp = (char*)gp->blocks[0].text;
  sizeFT = maxNrOfElementsPerTable * FREE_TEXT_BLOCK_SIZE;
  {
   if(xdr_bytes(xdrs,&bp,&sizeFT,sizeFT)
                != SURF_SUCCESS)
   {
    ret = SURF_FAILURE; 
   }
  }
  return(ret);
}




static XdrSurf xdr_SurfReductionParameters(XDR *xdrs,SurfReductionParameters *gp)
{
  return(
    xdr_double(xdrs,&gp->variation)                            &&
    xdr_double(xdrs,&gp->pointDistance)                        &&
    xdr_double(xdrs,&gp->maxAstar)                             &&
    xdr_double(xdrs,&gp->dFuture)                              &&
    xdr_u_short(xdrs,&gp->isReduced)                           &&
    xdr_u_short(xdrs,&gp->fromBeam)                            &&
    xdr_u_short(xdrs,&gp->toBeam)                              &&
    xdr_u_short(xdrs,&gp->reduceOuterBeams)                    );
}

    
static XdrSurf xdr_SurfLastFilterParameters(XDR *xdrs,SurfLastFilterParameters *gp)
{
  return(
    xdr_double(xdrs,&gp->depthMinDepth)                        &&
    xdr_double(xdrs,&gp->depthMaxDepth)                        &&
    xdr_double(xdrs,&gp->depthSlopeOver2)                      &&
    xdr_double(xdrs,&gp->depthSlopeOver3)                      &&
    xdr_u_short(xdrs,&gp->depthHasParams)                      &&
    xdr_u_short(xdrs,&gp->depthFilterAhead)                    &&
    xdr_u_short(xdrs,&gp->depthFilterAcross)                   &&
    xdr_u_short(xdrs,&gp->posHasParams)                        &&
    xdr_double(xdrs,&gp->posFilterRadius)                      &&
    xdr_double(xdrs,&gp->posMaxCourseChange)                   &&
    xdr_double(xdrs,&gp->dFuture1)                             &&
    xdr_double(xdrs,&gp->dFuture2)                             );
}

    

XdrSurf xdr_SurfAddStatistics(XDR *xdrs,SurfAddStatistics *gp)
{
  char buffer[LABEL_SIZE];
  u_int sizeL,sizeT;
  char *toLabel,*toText;

  /* Get the Label and check the consistency of the stream */

  sizeL=LABEL_SIZE;
  toLabel = (char *)gp->label;
  sizeT=TEXT_SIZE ; 
  toText  = (char *)gp->serverReduction;
   
  if (xdr_bytes(xdrs,&toLabel,&sizeL,sizeL) != SURF_SUCCESS)
  { 
   return(SURF_FAILURE);
  }
  if(strncpy(buffer,SURF_ADD_STATISTICS_LABEL,sizeL) == NULL)
  {
   return(SURF_FAILURE);
  }
  if((strncmp(gp->label,(const char*)buffer,sizeL)) != 0)
  {
   return(SURF_CORRUPTED_DATASET);
  }  
    
  return(
    xdr_u_int(xdrs,&gp->flag)                                 &&
    xdr_u_int(xdrs,&gp->nrNotDeletedDepth)                    &&
    xdr_u_int(xdrs,&gp->nrNotReducedDepth)                    &&
    xdr_u_int(xdrs,&gp->nrNotDeletedSoundings)                &&
    xdr_SurfReductionParameters(xdrs,&gp->redParm)             &&
    xdr_SurfLastFilterParameters(xdrs,&gp->filterParm)         &&
    xdr_bytes(xdrs,&toText,&sizeT,sizeT)                       &&
    xdr_double(xdrs,&(gp->dFuture[0]))                         &&
    xdr_double(xdrs,&(gp->dFuture[1]))                         &&
    xdr_double(xdrs,&(gp->dFuture[2]))                         &&
    xdr_double(xdrs,&(gp->dFuture[3]))                         &&
    xdr_double(xdrs,&(gp->dFuture[4]))                         &&
    xdr_double(xdrs,&(gp->dFuture[5]))                         &&
    xdr_double(xdrs,&(gp->dFuture[6]))                         &&
    xdr_double(xdrs,&(gp->dFuture[7]))                         &&
    xdr_double(xdrs,&(gp->dFuture[8]))                         &&
    xdr_double(xdrs,&(gp->dFuture[9]))                         &&
    xdr_u_short(xdrs,&(gp->iFuture[0]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[1]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[2]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[3]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[4]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[5]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[6]))                        &&
    xdr_u_short(xdrs,&(gp->iFuture[7]))                        );
}    

    

XdrSurf xdr_SurfVendorText(XDR *xdrs,SurfVendorText *gp)
{
  u_int sizeT = TEXT_SIZE;
  char *toText = (char *)gp->text;

  return(xdr_bytes(xdrs,&toText,&sizeT,sizeT));
}    


XdrSurf xdr_SurfFreeSixDataDescr(XDR *xdrs,SurfFreeSixDataDescr *gp)
{
  u_int sizeT = STRING_SIZE;
  char *toText = (char *)gp->descr;

  return(xdr_bytes(xdrs,&toText,&sizeT,sizeT));
}    


XdrSurf xdr_SurfFreeSndgDataDescr(XDR *xdrs,SurfFreeSndgDataDescr *gp)
{
  u_int sizeT = STRING_SIZE;
  char *toText = (char *)gp->descr;

  return(xdr_bytes(xdrs,&toText,&sizeT,sizeT));
}    


XdrSurf xdr_SurfFreeBeamDataDescr(XDR *xdrs,SurfFreeBeamDataDescr *gp)
{
  u_int sizeT = STRING_SIZE;
  char *toText = (char *)gp->descr;

  return(xdr_bytes(xdrs,&toText,&sizeT,sizeT));
}    
 

XdrSurf xdr_SurfFreeSixAttachedData(XDR *xdrs,SurfFreeSixAttachedData *gp)
{
  return(xdr_double(xdrs,gp));
}    
 



/***************************************/
/*                                     */
/* SURF-conversions for     SDA-files  */
/*                                     */
/***************************************/





 
XdrSurf xdr_SurfSoundingData(XDR *xdrs,SurfSoundingData *gp,short versLess2)
{
 if(versLess2 == 1) /* Version < 2.0 */
 {
  gp->dynChartZero = (float)0.0;
  return(
              xdr_u_short(xdrs,&gp->soundingFlag)                  &&
              xdr_u_short(xdrs,&gp->indexToAngle)                  &&
              xdr_u_short(xdrs,&gp->indexToTransducer)             &&
              xdr_u_short(xdrs,&gp->indexToCProfile)               &&
              xdr_float(xdrs,&gp->relTime)                         &&
              xdr_float(xdrs,&gp->relWay)                          &&
              xdr_float(xdrs,&gp->tide)                            &&
              xdr_float(xdrs,&gp->headingWhileTransmitting)        &&
              xdr_float(xdrs,&gp->heaveWhileTransmitting)          &&
              xdr_float(xdrs,&gp->rollWhileTransmitting)           &&
              xdr_float(xdrs,&gp->pitchWhileTransmitting)          &&
              xdr_float(xdrs,&gp->cKeel)                           &&
              xdr_float(xdrs,&gp->cMean)                           );
 }
 else
 {
  return(
              xdr_u_short(xdrs,&gp->soundingFlag)                  &&
              xdr_u_short(xdrs,&gp->indexToAngle)                  &&
              xdr_u_short(xdrs,&gp->indexToTransducer)             &&
              xdr_u_short(xdrs,&gp->indexToCProfile)               &&
              xdr_float(xdrs,&gp->relTime)                         &&
              xdr_float(xdrs,&gp->relWay)                          &&
              xdr_float(xdrs,&gp->tide)                            &&
              xdr_float(xdrs,&gp->headingWhileTransmitting)        &&
              xdr_float(xdrs,&gp->heaveWhileTransmitting)          &&
              xdr_float(xdrs,&gp->rollWhileTransmitting)           &&
              xdr_float(xdrs,&gp->pitchWhileTransmitting)          &&
              xdr_float(xdrs,&gp->cKeel)                           &&
              xdr_float(xdrs,&gp->cMean)                           &&
              xdr_float(xdrs,&gp->dynChartZero)                    );
 }
}



 

XdrSurf xdr_SurfFreeSoundingAttachedData(XDR *xdrs,SurfFreeSoundingAttachedData *gp)
{
  return(xdr_float(xdrs,gp));
}    



 
XdrSurf xdr_SurfCenterPosition(XDR *xdrs,SurfCenterPosition *gp)
{

  return(
              xdr_u_short(xdrs,&gp->positionFlag)                  &&
              xdr_float(xdrs,&gp->centerPositionX)                 &&
              xdr_float(xdrs,&gp->centerPositionY)                 &&
              xdr_float(xdrs,&gp->speed)                           );
}




 
XdrSurf xdr_SurfSingleBeamDepth(XDR *xdrs,SurfSingleBeamDepth *gp)
{

  return(
              xdr_u_short(xdrs,&gp->depthFlag)                     &&
              xdr_float(xdrs,&gp->travelTimeOfRay)                 &&
              xdr_float(xdrs,&gp->depthHFreq)                      &&
              xdr_float(xdrs,&gp->depthMFreq)                      &&
              xdr_float(xdrs,&gp->depthLFreq)                      );
}




 
XdrSurf xdr_SurfMultiBeamDepth(XDR *xdrs,SurfMultiBeamDepth *gp)
{

  return(
              xdr_u_short(xdrs,&gp->depthFlag)                     &&
              xdr_float(xdrs,&gp->depth)                           &&
              xdr_float(xdrs,&gp->beamPositionAhead)               &&
              xdr_float(xdrs,&gp->beamPositionStar)                );
}




 
XdrSurf xdr_SurfMultiBeamTT(XDR *xdrs,SurfMultiBeamTT *gp)
{

  return(
              xdr_float(xdrs,&gp->travelTimeOfRay)                 );
}




 
XdrSurf xdr_SurfMultiBeamReceive(XDR *xdrs,SurfMultiBeamReceive *gp)
{

  return(
              xdr_float(xdrs,&gp->headingWhileReceiving)           &&
              xdr_float(xdrs,&gp->heaveWhileReceiving)             );
}



 
XdrSurf xdr_SurfAmplitudes(XDR *xdrs,SurfAmplitudes *gp)
{
  return(     xdr_u_short(xdrs,&gp->beamAmplitude)                 );
}



 
XdrSurf xdr_SurfExtendedAmplitudes(XDR *xdrs,SurfExtendedAmplitudes *gp)
{
  return(     xdr_float(xdrs,&gp->mtau)                            &&
              xdr_u_short(xdrs,&gp->nis)                           &&
              xdr_u_short(xdrs,&gp->beamAmplitude)                 );
}




XdrSurf xdr_SurfFreeBeamAttachedData(XDR *xdrs,SurfFreeBeamAttachedData *gp)
{
  return(xdr_float(xdrs,gp));
}    



 
XdrSurf xdr_SurfSignalParameter(XDR *xdrs,SurfSignalParameter *gp,short nrSets)
{
 XdrSurf ret;
 short ii;

  ret = (
          xdr_u_short(xdrs,&gp->bscatClass)                        &&
          xdr_u_short(xdrs,&gp->nrActualGainSets)                  &&
          xdr_float(xdrs,&gp->rxGup)                               &&   
          xdr_float(xdrs,&gp->rxGain)                              &&      
          xdr_float(xdrs,&gp->ar)                                  );
  for(ii=0;ii<nrSets;ii++)
  {
   ret = ret && (
          xdr_float(xdrs,&gp->rxSets[ii].time)  &&
          xdr_float(xdrs,&gp->rxSets[ii].gain)    ); 
  }        
  return(ret);            
}




XdrSurf xdr_SurfTxParameter(XDR *xdrs,SurfTxParameter *gp,short nrSets)
{
 XdrSurf ret;
 short ii;

  ret = SURF_SUCCESS;
  for(ii=0;ii<nrSets;ii++)
  {
   ret = ret && (
          xdr_u_int(xdrs,&gp->txSets[ii].txBeamIndex)   &&
          xdr_float(xdrs,&gp->txSets[ii].txLevel)        &&  
          xdr_float(xdrs,&gp->txSets[ii].txBeamAngle)    && 
          xdr_float(xdrs,&gp->txSets[ii].pulseLength)    );   
  }        
  return(ret);            
}



 
XdrSurf xdr_SurfSignalAmplitudes(XDR *xdrs,SurfSignalAmplitudes *gp,
                                                   u_short nrAmplitudes)
{
 XdrSurf ret;
 char* toAmplitudes;
 u_int sizeA;
 
  ret = SURF_SUCCESS;
  if(nrAmplitudes > (u_short)0)
  {
   toAmplitudes = (char *)gp->amplitudes;
   sizeA=nrAmplitudes; 

   if((xdr_u_short(xdrs,&gp->amplitudesFlag)        != SURF_SUCCESS)  ||
      (xdr_u_short(xdrs,&gp->actualNrOfAmplitudes)  != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->maxAmplPosAstar)         != SURF_SUCCESS)  ||
      (xdr_bytes(xdrs,&toAmplitudes,&sizeA,sizeA)   != SURF_SUCCESS))
   {
     ret = SURF_FAILURE; 
   }
  }
  return(ret);
}



 
XdrSurf xdr_SurfSidescanData(XDR *xdrs,SurfSidescanData *gp,
                                                   u_short nrSsData)
{
 XdrSurf ret;
 char* toSsData;
 u_int sizeA;
 
  ret = SURF_SUCCESS;
  if(nrSsData > (u_short)0)
  {
   toSsData = (char *)gp->ssData;
   sizeA=nrSsData;
   
   if((xdr_u_int(xdrs,&gp->sidescanFlag)           != SURF_SUCCESS)  ||
      (xdr_u_short(xdrs,&gp->actualNrOfSsDataPort)  != SURF_SUCCESS)  ||
      (xdr_u_short(xdrs,&gp->actualNrOfSsDataStb)   != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->minSsPosPort)            != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->minSsPosStb)             != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->maxSsPosPort)            != SURF_SUCCESS)  ||
      (xdr_float(xdrs,&gp->maxSsPosStb)             != SURF_SUCCESS)  ||
      (xdr_bytes(xdrs,&toSsData,&sizeA,sizeA)       != SURF_SUCCESS))
   {
     ret = SURF_FAILURE;
   }
  }
  return(ret);
}




XdrSurf xdr_SurfTpeValues(XDR *xdrs,SurfTpeValues *gp)
{
  return(     xdr_float(xdrs,&gp->depthTpe)                            &&
              xdr_float(xdrs,&gp->posTpe)                              &&
              xdr_float(xdrs,&gp->minDetectionVolumeTpe)               );
}



 
XdrSurf xdr_SurfPositionCepData(XDR *xdrs,SurfPositionCepData *gp)
{
  return(     xdr_float(xdrs,gp)                                       );
}



 

