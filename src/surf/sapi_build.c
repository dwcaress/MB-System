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
extern SurfSoundingData*  sapiToSdaBlock;
extern Boolean            loadIntoMemory;


static SurfDescriptor defaultDescriptor =
{
 SURF_DESCRIPTOR_LABEL,
 SIX_M,
 {DESCRIPTOR,                       1},
 {GLOBALDATA,                       1},
 {STATISTICS,                       1},
 {POSITIONSENSORS,                  1},
 {TRANSDUCERPARAM,                  1},
 {BEAMANGLE,                        0},
 {CPROFILE,                         0},
 {AREAPOLYGON,                      0},
 {EVENTS,                           0},
 {FREETEXT,                         1},
#ifdef SURF30
 {ADDSTATISTICS,                    0},
 {TPESTATICS,                       0},
 {CPROFTPES,                        0},
 {FREESIXDESCR,                     0},
 {FREESNDGDESCR,                    0},
 {FREEBEAMDESCR,                    0},
 {SIXATTDATA,                       0},
 {VENDORTEXT,                       0},
#endif
 SDA_M,
 {SOUNDING,                         1},
 {CENTERPOSITION,                   1},
 {SINGLEBEAMDEPTH,                  1},
 {MULTIBEAMDEPTH,                   0},
 {MULTIBEAMTT,                      0},
 {MULTIBEAMRECV,                    0},
 {SIGNALPARMS,                      0},
 {SIGNALAMPLITUDE,                  0},
 {BEAMAMPLITUDES,                   0},
 {EXTBEAMAMPLI,                     0},
 {SIDESCANDATA,                     0},
 {TXPARMS,                          0},
#ifdef SURF30
 {POSITIONCEP,                      0},
 {MULTITPES,                        0},
 {SINGLETPES,                       0},
 {SNDGATTDATA,                      0},
 {BEAMATTDATA,                      0},
#endif
 NROF_M,
 {MAX_NROF_BEAMS_PER_TABLE,         0},
 {MAX_NROF_CPROFILES_PER_TABLE,     0},
 {MAX_NROF_POLYGONS_PER_TABLE,      0},
 {MAX_NROF_EVENTS,                  0},
 {MAX_NROF_FREE_TEXT_BLOCKS,       20},
 {MAX_NROF_SIDESCAN_DATA,           0},
 {NROF_RX_TVG_SETS,                 0},
 {NROF_TX_TVG_SETS,                 0},
 EOD_M
};


static SurfGlobalData defaultGlobalData =
{
 SURF_GLOBAL_DATA_LABEL,
 "UNKNOWN",
 "","","",
 0.0,0.0,
 0l,0l,
 "","","","",
 0,
 0.0,0.0,0.0,0.0,0.0,
 "UNKNOWN",'V',
 0.0,
 0.0,
 0.0,
 "WGS84",6378137.0,0.00335281070480037,"Mercator",
 'E',0.0,0.0,0.0,
 0.0,0.0,
 'l',
 0.0,0.0,0.0,0.0,
 0.0,0.0,0.0,0.0,
 0.0,0.0,
 "",
 0.0,0.0,0.0,0.0,
 0.0
};


static SurfStatistics defaultStatistics =
{
 SURF_STATISTICS_LABEL
};

static SurfPositionSensorArray defaultPositionSensorArray =
{
 SURF_POSITION_SENSOR_LABEL,
 UNKNOWNPOSSENS
};

static SurfTransducerParameterTable defaultTransducerTable =
{
 SURF_TRANSDUCER_TABLE_LABEL
};

static SurfMultiBeamAngleTable defaultAngleTable =
{
 SURF_MULTIBEAM_ANGLE_LABEL
};




static long createSDAs(void)
{
 SdaInfo*          sapiToSdaInfo;
 SurfSoundingData* toSdaBlock;
 size_t         sizeOfSdaBlock;
 u_long  nrSoundings,nrBeams;
 u_long  ii,jj;

 nrSoundings = sapiToSurfData->nrOfSoundings;
 nrBeams     = sapiToSurfData->nrOfMultiBeamDepth;

 sapiToSdaInfo = (SdaInfo*)calloc(1,sizeof(SdaInfo));
 sapiToSurfData->toSdaInfo = sapiToSdaInfo;
 if (sapiToSdaInfo == NULL)
 {
  free_SdaMemory(sapiToSurfData);
  return(-1);
 }
 sapiToSurfData->toSdaThread =
           (SurfSdaThread*)calloc((u_int)nrSoundings,sizeof(SurfSdaThread));
 if (sapiToSurfData->toSdaThread == NULL)
 {
  free_SdaMemory(sapiToSurfData);
  return(-1);
 }

 sizeOfSdaBlock = initializeSdaInfo(sapiToSurfData,sapiToSdaInfo);
 for(ii=0;ii < nrSoundings;ii++)
 {
  toSdaBlock = (SurfSoundingData*)calloc(1,sizeOfSdaBlock);
  if(toSdaBlock == NULL)
  {
   for(jj=0;jj<ii;jj++)
   {
    if(sapiToSurfData->toSdaThread->thread[jj].sounding != NULL)
            free(sapiToSurfData->toSdaThread->thread[jj].sounding);
    sapiToSurfData->toSdaThread->thread[jj].sounding = NULL;
   }
   free_SdaMemory(sapiToSurfData);
   return(-1);
  }
  else
  {
   sapiToSurfData->toSdaThread->thread[ii].sounding = toSdaBlock;
  }
 }
 return(0);
}




long SAPI_createSurfBody(long nrSoundings,
                         long nrBeams,
                         long maxNrSidescanSamplesPerSounding,
                         long errorprint)
{

 defaultDescriptor.soundings.nr=nrSoundings;
 defaultGlobalData.numberOfMeasuredSoundings =nrSoundings;
 defaultGlobalData.actualNumberOfSoundingSets=nrSoundings;
 if(nrBeams > 0)
 {
  defaultDescriptor.angleTab.nr = 1;
  defaultDescriptor.maxNrOfBeams.nr = 1;
  defaultDescriptor.singleBeamDepth.nr = 0;
  defaultDescriptor.maxNrOfBeams.nr    = nrBeams;
  defaultDescriptor.multiBeamDepth.nr  = nrBeams;
  defaultDescriptor.multiBeamTT.nr     = nrBeams;
  defaultDescriptor.multiBeamRecv.nr   = nrBeams;
  defaultGlobalData.typeOfSounder = 'F';
  defaultAngleTable.actualNumberOfBeams = (u_short)nrBeams;
 }
 if(maxNrSidescanSamplesPerSounding > 0)
 {
  defaultDescriptor.sidescanData.nr = 1;
  defaultDescriptor.maxNrOfSidescanData.nr = maxNrSidescanSamplesPerSounding;
 }

 sapiToSurfData = (SurfDataInfo*)calloc(1,sizeof(SurfDataInfo));
 if (sapiToSurfData == NULL)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  return((long)-1);
 }

 sapiToSurfData->toDescriptor =
        (SurfDescriptor*)calloc(1,sizeof(SurfDescriptor));
 sapiToSurfData->toGlobalData =
        (SurfGlobalData*)calloc(1,sizeof(SurfGlobalData));
 sapiToSurfData->toStatistics =
        (SurfStatistics*)calloc(1,sizeof(SurfStatistics));
 sapiToSurfData->toPosiSensors =
        (SurfPositionSensorArray*)calloc(1,sizeof(SurfPositionSensorArray));
 sapiToSurfData->toTransducers =
        (SurfTransducerParameterTable*)calloc(1,sizeof(SurfTransducerParameterTable));
 sapiToSurfData->toFreeText =
        (SurfFreeText*)calloc(1,SIZE_OF_FREE_TEXT_ARRAY(20));
 if(  (sapiToSurfData->toDescriptor == NULL)
   || (sapiToSurfData->toGlobalData == NULL)
   || (sapiToSurfData->toStatistics == NULL)
   || (sapiToSurfData->toPosiSensors == NULL)
   || (sapiToSurfData->toTransducers == NULL)
   || (sapiToSurfData->toFreeText    == NULL))
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  freeSixBlocks(sapiToSurfData,0);
  return((long)-1);
 }

 if(nrBeams != 0)
 {
  sapiToSurfData->toAngleTables =
        (SurfMultiBeamAngleTable*)calloc(1,(size_t)SIZE_OF_SURF_MULTIBEAM_ANGLE_TAB((u_int)nrBeams));
  if(sapiToSurfData->toAngleTables == NULL)
  {
   if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
   freeSixBlocks(sapiToSurfData,0);
   return((long)-1);
  }
  *(sapiToSurfData->toAngleTables) = defaultAngleTable;
 }

 *(sapiToSurfData->toDescriptor) = defaultDescriptor;
 *(sapiToSurfData->toGlobalData) = defaultGlobalData;
 *(sapiToSurfData->toStatistics) = defaultStatistics;
 *(sapiToSurfData->toPosiSensors)= defaultPositionSensorArray;
 *(sapiToSurfData->toTransducers)= defaultTransducerTable;

 checkAndLoadSurfDescriptor(sapiToSurfData->toDescriptor,sapiToSurfData);

 if(createSDAs()<0)
 {
  if(errorprint != 0)
    fprintf(stderr,"SAPI-Error: Can't allocate sufficient memory' !\n");
  freeSixBlocks(sapiToSurfData,0);
  return((long)-1);
 }

 surf_moveInSdaThread(sapiToSurfData,TO_START,0);

 loadIntoMemory = True;

 return((long)0);
}

