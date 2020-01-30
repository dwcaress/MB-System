// See README file for copying and redistribution conditions.

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

SurfSoundingData *SAPI_getSoundingData(void) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  return sapiToSdaInfo->toSoundings;
}

SurfTransducerParameterTable *SAPI_getActualTransducerTable(void) {
  if (sapiToSurfData == NULL || sapiToSurfData->toTransducers == NULL)
    return NULL;

  SdaInfo *sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;

  const short index = (short)sapiToSdaInfo->toSoundings->indexToTransducer;
  SurfTransducerParameterTable *toTransdTable = &(sapiToSurfData->toTransducers[index]);
  return toTransdTable;
}

SurfMultiBeamAngleTable *SAPI_getActualAngleTable(void) {
  if (sapiToSurfData == NULL || sapiToSurfData->toAngleTables == NULL)
    return NULL;

  SdaInfo *sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;

  const short index = (short)sapiToSdaInfo->toSoundings->indexToAngle;
  const short nrBeams = (short)SAPI_getNrBeams();

  SurfMultiBeamAngleTable *toAngleTable =
      getSurfAngleTable(sapiToSurfData->toAngleTables, nrBeams, index);
  return toAngleTable;
}

SurfCProfileTable *SAPI_getActualCProfileTable(void) {
  if (sapiToSurfData == NULL || sapiToSurfData->toCProfiles == NULL)
    return NULL;
  SdaInfo *sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;

  const short index = (short)sapiToSdaInfo->toSoundings->indexToCProfile;
  const short nrCEles = (short)sapiToSurfData->nrCPElements;

  SurfCProfileTable *toCProf =
      getSurfCProfileTable(sapiToSurfData->toCProfiles, nrCEles, index);

  return toCProf;
}

SurfCenterPosition *SAPI_getCenterPosition(long nrPositionSensor) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  if (nrPositionSensor < SAPI_getNrPositionsensors() && nrPositionSensor >= 0) {
    SurfCenterPosition *toPosition =
        (SurfCenterPosition *)
        &sapiToSdaInfo->toActCenterPosition[nrPositionSensor].positionFlag;
    return toPosition;
  }
  return NULL;
}

SurfSingleBeamDepth *SAPI_getSingleBeamDepth(void) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  return sapiToSdaInfo->toSingleBeamDepth;
}

SurfMultiBeamDepth *SAPI_getMultiBeamDepth(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL || sapiToSdaInfo->toMultiBeamDepth == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toMultiBeamDepth[beam]);
  }
  return NULL;
}

SurfMultiBeamTT *SAPI_getMultiBeamTraveltime(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL || sapiToSdaInfo->toMultiBeamTT == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toMultiBeamTT[beam]);
  }
  return NULL;
}

SurfMultiBeamReceive *SAPI_getMultiBeamReceiveParams(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL || sapiToSdaInfo->toMultiBeamRec == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toMultiBeamRec[beam]);
  }
  return NULL;
}

// New Data in SURF 2.0

SurfAmplitudes *SAPI_getMultibeamBeamAmplitudes(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL || sapiToSdaInfo->toAmplitudes == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toAmplitudes[beam]);
  }
  return NULL;
}

SurfExtendedAmplitudes *SAPI_getMultibeamExtendedBeamAmplitudes(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL || sapiToSdaInfo->toExtendedAmpl == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toExtendedAmpl[beam]);
  }
  return NULL;
}

SurfSignalParameter *SAPI_getMultibeamSignalParameters(void) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  return sapiToSdaInfo->toSignalParams;
}

SurfTxParameter *SAPI_getMultibeamTransmitterParameters(int *nTxParams) {
  SdaInfo *sapiToSdaInfo = NULL;

  if (nTxParams != NULL)
    *nTxParams = 0;
  if (sapiToSurfData != NULL) sapiToSdaInfo = sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  if (nTxParams != NULL)
    *nTxParams = sapiToSdaInfo->nrTxParams;
  return sapiToSdaInfo->toTxParams;
}

SurfSidescanData *SAPI_getSidescanData(void) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  return sapiToSdaInfo->toSsData;
}

SurfAddStatistics *SAPI_getAddStatistics(void) {
  if (sapiToSurfData == NULL) return NULL;
  return sapiToSurfData->toAddStatistics;
}

SurfTpeStatics *SAPI_getTpeStatics(void) {
  if (sapiToSurfData == NULL) return NULL;
  return sapiToSurfData->toTpeStatics;
}

SurfTpeValues *SAPI_getMultiBeamTPEValues(long beam) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  if (sapiToSdaInfo->toMultiBeamTpeValues == NULL) return NULL;
  if (beam >= 0 && beam < SAPI_getNrBeams()) {
    return &(sapiToSdaInfo->toMultiBeamTpeValues[beam]);
  }
  return NULL;
}

SurfTpeValues *SAPI_getSingleBeamTPEValues(void) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  return sapiToSdaInfo->toSingleBeamTpeValues;
}

SurfPositionCepData *SAPI_getPositionCep(long nrPositionSensor) {
  SdaInfo *sapiToSdaInfo =
      sapiToSurfData == NULL ? NULL : sapiToSurfData->toSdaInfo;
  if (sapiToSdaInfo == NULL) return NULL;
  if (sapiToSdaInfo->toPositionCepData == NULL) return NULL;
  if (nrPositionSensor < SAPI_getNrPositionsensors() && nrPositionSensor >= 0) {
    return &(sapiToSdaInfo->toPositionCepData[nrPositionSensor]);
  }
  return NULL;
}
