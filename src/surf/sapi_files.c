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
