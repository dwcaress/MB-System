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



SurfSoundingData* SAPI_getSoundingData(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toSoundings);
}


SurfTransducerParameterTable* SAPI_getActualTransducerTable(void)
{
 SurfTransducerParameterTable* toTransdTable;
 short index;
 
 if((sapiToSurfData == NULL) || (sapiToSurfData->toTransducers == NULL)) return(NULL);
 if(sapiToSdaInfo == NULL) return(NULL);
 index=(short)sapiToSdaInfo->toSoundings->indexToTransducer;
 toTransdTable = &(sapiToSurfData->toTransducers[index]);
 return(toTransdTable); 
}


SurfMultiBeamAngleTable* SAPI_getActualAngleTable(void)
{
 SurfMultiBeamAngleTable* toAngleTable;
 short index,nrBeams;
 
 if((sapiToSurfData == NULL) || (sapiToSurfData->toAngleTables == NULL)) return(NULL);
 if(sapiToSdaInfo == NULL) return(NULL);
 index=(short)sapiToSdaInfo->toSoundings->indexToAngle;
 nrBeams=(short)SAPI_getNrBeams();
 toAngleTable = getSurfAngleTable(sapiToSurfData->toAngleTables,nrBeams,index);
 return(toAngleTable); 
}


SurfCProfileTable* SAPI_getActualCProfileTable(void)
{
 SurfCProfileTable* toCProf;
 short index,nrCEles;
 
 if((sapiToSurfData == NULL) || (sapiToSurfData->toCProfiles == NULL)) return(NULL);
 if(sapiToSdaInfo == NULL) return(NULL);
 index=(short)sapiToSdaInfo->toSoundings->indexToCProfile;
 nrCEles=(short)sapiToSurfData->nrCPElements;
 toCProf=getSurfCProfileTable(sapiToSurfData->toCProfiles,nrCEles,index);
 return(toCProf); 
}


SurfCenterPosition* SAPI_getCenterPosition(long nrPositionSensor)
{
 SurfCenterPosition* toPosition;
 
 if(sapiToSdaInfo == NULL) return(NULL);
 if((nrPositionSensor<SAPI_getNrPositionsensors()) && (nrPositionSensor>=0))
 {
  toPosition = (SurfCenterPosition*)
                &sapiToSdaInfo->toActCenterPosition[nrPositionSensor].positionFlag;
  return(toPosition);              
 }
 return(NULL);
}


SurfSingleBeamDepth* SAPI_getSingleBeamDepth(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toSingleBeamDepth);
}


SurfMultiBeamDepth* SAPI_getMultiBeamDepth(long beam)
{
 if((sapiToSdaInfo == NULL) || (sapiToSdaInfo->toMultiBeamDepth == NULL)) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toMultiBeamDepth[beam]));
 }
 return(NULL); 
}


SurfMultiBeamTT* SAPI_getMultiBeamTraveltime(long beam)
{
 if((sapiToSdaInfo == NULL) || (sapiToSdaInfo->toMultiBeamTT == NULL)) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toMultiBeamTT[beam]));
 }
 return(NULL); 
}


SurfMultiBeamReceive* SAPI_getMultiBeamReceiveParams(long beam)
{
 if((sapiToSdaInfo == NULL) || (sapiToSdaInfo->toMultiBeamRec == NULL)) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toMultiBeamRec[beam]));
 }
 return(NULL); 
}



/* New Data in SURF2.0 */


SurfAmplitudes* SAPI_getMultibeamBeamAmplitudes(long beam)
{
 if((sapiToSdaInfo == NULL) || (sapiToSdaInfo->toAmplitudes == NULL)) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toAmplitudes[beam]));
 }
 return(NULL); 
}



SurfExtendedAmplitudes* SAPI_getMultibeamExtendedBeamAmplitudes(long beam)
{
 if((sapiToSdaInfo == NULL) || (sapiToSdaInfo->toExtendedAmpl == NULL)) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toExtendedAmpl[beam]));
 }
 return(NULL); 
}



SurfSignalParameter* SAPI_getMultibeamSignalParameters(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toSignalParams);
}



SurfTxParameter* SAPI_getMultibeamTransmitterParameters(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toTxParams);
}



SurfSidescanData* SAPI_getSidescanData(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toSsData);
}


#ifdef SURF30

SurfAddStatistics*  SAPI_getAddStatistics(void)
{
 if(sapiToSurfData == NULL) return(NULL);
 return(sapiToSurfData->toAddStatistics);
}

SurfTpeStatics*  SAPI_getTpeStatics(void)
{
 if(sapiToSurfData == NULL) return(NULL);
 return(sapiToSurfData->toTpeStatics);
}

SurfTpeValues* SAPI_getMultiBeamTPEValues(long beam)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 if(sapiToSdaInfo->toMultiBeamTpeValues == NULL) return(NULL);
 if((beam >= 0) && (beam<SAPI_getNrBeams()))
 {
  return(&(sapiToSdaInfo->toMultiBeamTpeValues[beam]));
 }
 return(NULL); 
}

SurfTpeValues* SAPI_getSingleBeamTPEValues(void)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 return(sapiToSdaInfo->toSingleBeamTpeValues);
}

SurfPositionCepData* SAPI_getPositionCep(long nrPositionSensor)
{
 if(sapiToSdaInfo == NULL) return(NULL);
 if(sapiToSdaInfo->toPositionCepData == NULL) return(NULL);
 if((nrPositionSensor<SAPI_getNrPositionsensors()) && (nrPositionSensor>=0))
 {
  return(&(sapiToSdaInfo->toPositionCepData[nrPositionSensor]));              
 }
 return(NULL);
}

#endif
