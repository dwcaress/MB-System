/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*-----------------------------------------------------------------------
/ P R O G R A M M K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : mem_surf.c     Version 3.0
/  ERSTELLUNGSDATUM : 29.07.93
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

/*
/ SPRACHE          : UNIX-C
/ COMPILER         : Silicon Graphix
/ BETRIEBSSYSTEM   : IRIX
/ HARDWARE-UMGEBUNG: SGI Crimson
/ URSPRUNGSHINWEIS :
/
/ ------------------------------------------------------------------------
/ PROGRAMMBESCHREIBUNG:
/ ------------------------------------------------------------------------
/
/    LIBRARY-Functions for SURF-memory-administration V2.0
/
/ ------------------------------------------------------------------------
/ NAME, STRUKTUR UND KURZBESCHREIBUNG DER EINGABEPARAMETER:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h
/
/ ------------------------------------------------------------------------
/ NAME, STRUKTUR UND KURZBESCHREIBUNG DER AUSGABEPARAMETER:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h
/
/ ------------------------------------------------------------------------
/ VERHALTEN IM FEHLERFALL:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   P R O G R A M M K O P F E S
/ ------------------------------------------------------------------------
*/
/* ***********************************************************************
*                                                                        *
*  BEGINN DES DEKLARATIONSTEILS                                          *
*                                                                        *
*********************************************************************** */


#define _MEM_SURF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xdr_surf.h"
#include "mem_surf.h"


/* ***********************************************************************
*                                                                        *
*  ENDE DES DEKLARATIONSTEILS                                            *
*                                                                        *
*********************************************************************** */






/***************************************/
/*                                     */
/* free the memory for SIX-Block       */
/* according to the Info-Structure     */
/*                                     */
/* a NULL-pointer in Info tells that   */
/* this block is not existing in that  */
/* specific configuration              */
/*                                     */
/***************************************/


/* free a given six-structure */


XdrSurf freeSixBlocks(SurfDataInfo* toSurfDataInfo,XdrSurf returnvalue )
{
  if(toSurfDataInfo->fp            != NULL)
  {
    fclose(toSurfDataInfo->fp);
    toSurfDataInfo->fp = NULL;
  }  
  if(toSurfDataInfo->xdrs          != NULL)
  {
    free(toSurfDataInfo->xdrs);
    toSurfDataInfo->xdrs = NULL;
  }  
  if(toSurfDataInfo->toDescriptor  != NULL)
  {
    free(toSurfDataInfo->toDescriptor);
    toSurfDataInfo->toDescriptor = NULL;
  }  
  if(toSurfDataInfo->toGlobalData  != NULL)
  {
    free(toSurfDataInfo->toGlobalData);
    toSurfDataInfo->toGlobalData = NULL;
  }  
  if(toSurfDataInfo->toStatistics  != NULL)
  {
    free(toSurfDataInfo->toStatistics);
    toSurfDataInfo->toStatistics = NULL;
  }  
  if(toSurfDataInfo->toPosiSensors != NULL)
  {
    free(toSurfDataInfo->toPosiSensors);
    toSurfDataInfo->toPosiSensors = NULL;
  }  
  if(toSurfDataInfo->toAngleTables != NULL)
  {
    free(toSurfDataInfo->toAngleTables);
    toSurfDataInfo->toAngleTables = NULL;
  }  
  if(toSurfDataInfo->toTransducers != NULL)
  {
    free(toSurfDataInfo->toTransducers);
    toSurfDataInfo->toTransducers = NULL;
  }  
  if(toSurfDataInfo->toCProfiles   != NULL)
  {
    free(toSurfDataInfo->toCProfiles);
    toSurfDataInfo->toCProfiles = NULL;
  }  
  if(toSurfDataInfo->toPolygons    != NULL)
  {
    free(toSurfDataInfo->toPolygons);
    toSurfDataInfo->toPolygons = NULL;
  }  
  if(toSurfDataInfo->toEvents      != NULL)
  {
    free(toSurfDataInfo->toEvents  );
    toSurfDataInfo->toEvents = NULL;
  } 
  if(toSurfDataInfo->toFreeText    != NULL)
  {
    free(toSurfDataInfo->toFreeText);
    toSurfDataInfo->toFreeText = NULL;
  } 
  if(toSurfDataInfo->toAddStatistics != NULL)
  {
    free(toSurfDataInfo->toAddStatistics);
    toSurfDataInfo->toAddStatistics = NULL;
  } 
  if(toSurfDataInfo->toTpeStatics != NULL)
  {
    free(toSurfDataInfo->toTpeStatics);
    toSurfDataInfo->toTpeStatics = NULL;
  } 
  if(toSurfDataInfo->toCProfileTpes  != NULL)
  {
    free(toSurfDataInfo->toCProfileTpes);
    toSurfDataInfo->toCProfileTpes = NULL;
  }  
  if(toSurfDataInfo->toFreeSixDataDescr != NULL)
  {
    free(toSurfDataInfo->toFreeSixDataDescr);
    toSurfDataInfo->toFreeSixDataDescr = NULL;
  } 
  if(toSurfDataInfo->toFreeSndgDataDescr != NULL)
  {
    free(toSurfDataInfo->toFreeSndgDataDescr);
    toSurfDataInfo->toFreeSndgDataDescr = NULL;
  } 
  if(toSurfDataInfo->toFreeBeamDataDescr != NULL)
  {
    free(toSurfDataInfo->toFreeBeamDataDescr);
    toSurfDataInfo->toFreeBeamDataDescr = NULL;
  } 
  if(toSurfDataInfo->toFreeSixData != NULL)
  {
    free(toSurfDataInfo->toFreeSixData);
    toSurfDataInfo->toFreeSixData = NULL;
  } 
  if(toSurfDataInfo->toVendorText != NULL)
  {
    free(toSurfDataInfo->toVendorText);
    toSurfDataInfo->toVendorText = NULL;
  } 
  return(returnvalue);
}




/***************************************/
/*                                     */
/* Allocate memory for SIX-Block and   */
/* read data from file                 */
/*                                     */
/***************************************/

/* fill the Descriptor- & Info-Structure with data from the file */           

XdrSurf checkAndLoadSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                      SurfDataInfo* toSurfDataInfo)
{
  if((toSurfDescriptor->six                != SIX_M               ) ||
     (toSurfDescriptor->descriptor.typ     != DESCRIPTOR          ) ||
     (toSurfDescriptor->globalData.typ     != GLOBALDATA          ) ||
     (toSurfDescriptor->statistics.typ     != STATISTICS          ) ||
     (toSurfDescriptor->positionSensor.typ != POSITIONSENSORS     ) ||
     (toSurfDescriptor->transducer.typ     != TRANSDUCERPARAM     ) ||
     (toSurfDescriptor->angleTab.typ       != BEAMANGLE           ) ||
     (toSurfDescriptor->cProfile.typ       != CPROFILE            ) ||
     (toSurfDescriptor->polygon.typ        != AREAPOLYGON         ) ||
     (toSurfDescriptor->events.typ         != EVENTS              ) ||
     (toSurfDescriptor->freeText.typ       != FREETEXT            ) ||
     (toSurfDescriptor->addStatistics.typ  != ADDSTATISTICS       ) ||
     (toSurfDescriptor->tpeStatics.typ     != TPESTATICS          ) ||
     (toSurfDescriptor->cprofTpes.typ      != CPROFTPES           ) ||
     (toSurfDescriptor->freeSixDescr.typ   != FREESIXDESCR        ) ||
     (toSurfDescriptor->freeSndgDescr.typ  != FREESNDGDESCR       ) ||
     (toSurfDescriptor->freeBeamDescr.typ  != FREEBEAMDESCR       ) ||
     (toSurfDescriptor->freeSixAttData.typ != SIXATTDATA          ) ||
     (toSurfDescriptor->vendorText.typ     != VENDORTEXT          ) ||
     (toSurfDescriptor->sda                != SDA_M               ) ||
     (toSurfDescriptor->nrof               != NROF_M              ) ||
     (toSurfDescriptor->eod                != EOD_M               ))   
  {
   return(SURF_CORRUPTED_DATASET);
  }

  toSurfDataInfo->nrStatistics  = toSurfDescriptor->statistics.nr          ;
  toSurfDataInfo->nrPosiSensors = toSurfDescriptor->positionSensor.nr      ;
  toSurfDataInfo->nrAngleTables = toSurfDescriptor->angleTab.nr            ;
  toSurfDataInfo->nrBeams       = toSurfDescriptor->maxNrOfBeams.nr        ;
  toSurfDataInfo->nrTransducers = toSurfDescriptor->transducer.nr          ;
  toSurfDataInfo->nrCProfiles   = toSurfDescriptor->cProfile.nr            ;
  toSurfDataInfo->nrCPElements  = toSurfDescriptor->maxNrOfCProfileElements.nr;
  toSurfDataInfo->nrPolyElements= toSurfDescriptor->maxNrOfPolygonElements.nr ;
  toSurfDataInfo->nrEvents      = toSurfDescriptor->maxNrOfEvents.nr       ;
  toSurfDataInfo->nrFreeTextUnits=toSurfDescriptor->maxNrOfFreeTextBlocks.nr  ;
  toSurfDataInfo->nrAddStatistics     = toSurfDescriptor->addStatistics.nr;
  toSurfDataInfo->nrTpeStatics        = toSurfDescriptor->tpeStatics.nr;
  toSurfDataInfo->nrCprofTpes         = toSurfDescriptor->cprofTpes.nr;
  toSurfDataInfo->nrOfSixAttachedData = toSurfDescriptor->freeSixDescr.nr;
  toSurfDataInfo->nrOfSndgAttachedData= toSurfDescriptor->freeSndgDescr.nr;
  toSurfDataInfo->nrOfBeamAttachedData= toSurfDescriptor->freeBeamDescr.nr;
  toSurfDataInfo->nrOfVendorText      = toSurfDescriptor->vendorText.nr;
  toSurfDataInfo->nrOfSoundings = toSurfDescriptor->soundings.nr           ;
  toSurfDataInfo->maxNrSsData   = toSurfDescriptor->maxNrOfSidescanData.nr ;
  toSurfDataInfo->nrOfRxSets    = toSurfDescriptor->nrOfRxTvgSets.nr ;
  toSurfDataInfo->nrOfTxSets    = toSurfDescriptor->nrOfTxTvgSets.nr ;
  toSurfDataInfo->nrOfCenterPositions 
                                = toSurfDescriptor->centerPositions.nr     ;
  toSurfDataInfo->nrOfCeps      = toSurfDescriptor->positionCpes.nr        ;
  toSurfDataInfo->nrOfSingleBeamDepth 
                                = toSurfDescriptor->singleBeamDepth.nr     ;
  toSurfDataInfo->nrOfMultiBeamDepth 
                                = toSurfDescriptor->multiBeamDepth.nr      ;
  toSurfDataInfo->nrOfMultiBeamTT    
                                = toSurfDescriptor->multiBeamTT.nr         ;
  toSurfDataInfo->nrOfMultiBeamRec   
                                = toSurfDescriptor->multiBeamRecv.nr       ;
  toSurfDataInfo->nrOfSignalParams   
                                = toSurfDescriptor->signalParams.nr        ;
  toSurfDataInfo->nrOfTxParams   
                                = toSurfDescriptor->txParams.nr            ;
  toSurfDataInfo->nrOfSignalAmplitudes
                                = toSurfDescriptor->signalAmplitudes.nr    ;
  toSurfDataInfo->nrOfAmplitudes
                                = toSurfDescriptor->beamAmplitudes.nr      ;
  toSurfDataInfo->nrOfExtAmplitudes
                                = toSurfDescriptor->extendBeamAmplitudes.nr;
  toSurfDataInfo->nrOfSsData
                                = toSurfDescriptor->sidescanData.nr        ;
  toSurfDataInfo->nrOfSingleTPEs = toSurfDescriptor->singleTpeParams.nr    ;
  toSurfDataInfo->nrOfMultiTPEs = toSurfDescriptor->multiTpeParams.nr      ;
  return(SURF_SUCCESS);          
}                                      



 
XdrSurf mem_ReadSixStructure(char* filename,
                             SurfDataInfo* toSurfDataInfo)
                             
/* Allocates the necessary memory for a SIX-structure and reads 
   the data from file;
   actual description of the structure will be returned in
   SurfDataInfo                                               */  
                              
{
  XdrSurf ret;
  XDR* xdrs;
  FILE* fp;
  SurfDescriptor* surfDescriptor;
  SurfGlobalData* surfGlobalData;
  SurfStatistics* surfStatistics;
  SurfPositionSensorArray* surfPositionSensorArray;
  SurfMultiBeamAngleTable* surfMultiBeamAngleTable; 
  SurfTransducerParameterTable* surfTransducerParameterTable;
  SurfCProfileTable* surfCProfileTable;
  SurfCProfileTpeTable* surfCProfileTpeTable;
  SurfPolygons*   surfPolygons;
  SurfEvents*     surfEvents;   
  SurfFreeText*   surfFreeText; 
  SurfAddStatistics*       surfAddStatistics;
  SurfTpeStatics*          surfTpeStatics;
  SurfFreeSixDataDescr*    surfFreeSixDataDescr;
  SurfFreeSndgDataDescr*   surfFreeSndgDataDescr;
  SurfFreeBeamDataDescr*   surfFreeBeamDataDescr; 
  SurfFreeSixAttachedData* surfFreeSixAttachedData;
  SurfVendorText*          surfVendorText;
  u_long  ii,jj,nn;
  short newVersion,oldVersion,vers20;

  /* initialize SurfDataInfo */

  toSurfDataInfo->fp            = NULL;
  toSurfDataInfo->xdrs          = NULL;
  toSurfDataInfo->toDescriptor  = NULL;
  toSurfDataInfo->toGlobalData  = NULL;
  toSurfDataInfo->toStatistics  = NULL;
  toSurfDataInfo->toPosiSensors = NULL;
  toSurfDataInfo->toAngleTables = NULL;
  toSurfDataInfo->toTransducers = NULL;
  toSurfDataInfo->toCProfiles   = NULL;
  toSurfDataInfo->toPolygons    = NULL;
  toSurfDataInfo->toEvents      = NULL;
  toSurfDataInfo->toFreeText    = NULL;
  toSurfDataInfo->toAddStatistics    = NULL; 
  toSurfDataInfo->toTpeStatics       = NULL;
  toSurfDataInfo->toCProfileTpes     = NULL; 
  toSurfDataInfo->toFreeSixDataDescr = NULL; 
  toSurfDataInfo->toFreeSndgDataDescr= NULL; 
  toSurfDataInfo->toFreeBeamDataDescr= NULL; 
  toSurfDataInfo->toFreeSixData      = NULL; 
  toSurfDataInfo->toVendorText       = NULL; 

  /* allocate structure for xdr-conversion and SurfDescriptor */
  
  xdrs = (XDR*)calloc(1,sizeof(XDR));
  toSurfDataInfo->xdrs = xdrs; 
  surfDescriptor = (SurfDescriptor*)calloc(1,sizeof(SurfDescriptor));
  toSurfDataInfo->toDescriptor = surfDescriptor;
  if ((xdrs == NULL) || (surfDescriptor == NULL))
  {
   return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
  }


  /* open conversion and file Read  */
  
  fp =  xdrSurfOpenRead(xdrs,(const char*)filename);
  toSurfDataInfo->fp = fp; 
  if(fp == NULL)
  {
   return(freeSixBlocks(toSurfDataInfo,SURF_CANT_OPEN_FILE));
  }


  /* read SurfDescriptor */
  
  ret = xdr_SurfDescriptor(xdrs,surfDescriptor,&newVersion,&oldVersion);
  if(ret != SURF_SUCCESS)
  {
   return(freeSixBlocks(toSurfDataInfo,ret));
  }
  toSurfDataInfo->sourceVersionLess2 = 0;
  vers20 = getSurfVersion(SURF_VERS2_0);
  if(oldVersion < vers20)
  {
   toSurfDataInfo->sourceVersionLess2 = 1;
  } 
  
  ret = checkAndLoadSurfDescriptor(surfDescriptor,toSurfDataInfo);
  if(ret != SURF_SUCCESS)
  {
   return(freeSixBlocks(toSurfDataInfo,ret));
  }


  /* read Global Data */

  surfGlobalData = (SurfGlobalData*)calloc(1,sizeof(SurfGlobalData));
  toSurfDataInfo->toGlobalData = surfGlobalData;
  if (surfGlobalData == NULL)
  {
   return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
  }
  ret = xdr_SurfGlobalData(xdrs,surfGlobalData);
  if(ret != SURF_SUCCESS)
  {
   return(freeSixBlocks(toSurfDataInfo,ret));
  }


  /* read Statistics  */

  if(toSurfDataInfo->nrStatistics != 0)
  {
   surfStatistics = (SurfStatistics*)calloc(1,sizeof(SurfStatistics));
   toSurfDataInfo->toStatistics = surfStatistics;
   if (surfStatistics == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfStatistics(xdrs,surfStatistics);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  /* read Position Sensors  */

  ii = toSurfDataInfo->nrPosiSensors;
  if(ii == 0)
  {
   return(freeSixBlocks(toSurfDataInfo,SURF_CORRUPTED_DATASET));
  }
  else
  {
   surfPositionSensorArray = (SurfPositionSensorArray*)
                            calloc((u_int)ii,sizeof(SurfPositionSensorArray));
   toSurfDataInfo->toPosiSensors = surfPositionSensorArray;
   if (surfPositionSensorArray == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(nn=0;nn<ii;nn++)
   {
    ret = xdr_PositionSensorArray(xdrs,surfPositionSensorArray,oldVersion);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
    surfPositionSensorArray++;
   }
  }


  /* read transducer data of singlebeamsounders  */

  ii = toSurfDataInfo->nrTransducers;
  if(ii != 0)
  {
   surfTransducerParameterTable = (SurfTransducerParameterTable*)calloc((u_int)ii,
                                  sizeof(SurfTransducerParameterTable));
   toSurfDataInfo->toTransducers = surfTransducerParameterTable;
   if (surfTransducerParameterTable == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(nn=0;nn<ii;nn++)
   {
    ret = xdr_SurfTransducerParameterTable(xdrs,surfTransducerParameterTable);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
    surfTransducerParameterTable++;
   }
  }


  /* read beam-angle tables of multibeamsounders  */

  ii = toSurfDataInfo->nrAngleTables;  
  jj = toSurfDataInfo->nrBeams;        
  if((ii != 0) && (jj != 0))
  {
   surfMultiBeamAngleTable = (SurfMultiBeamAngleTable*)
                 calloc((u_int)ii,(size_t)SIZE_OF_SURF_MULTIBEAM_ANGLE_TAB((u_int)jj));
   toSurfDataInfo->toAngleTables = surfMultiBeamAngleTable;
   if (surfMultiBeamAngleTable == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(nn=0;nn<ii;nn++)
   {
    ret = xdr_SurfMultiBeamAngleTable(xdrs,surfMultiBeamAngleTable,(u_short)jj);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
    surfMultiBeamAngleTable = (SurfMultiBeamAngleTable*)
                             ((char*)(surfMultiBeamAngleTable)
                             +(size_t)(SIZE_OF_SURF_MULTIBEAM_ANGLE_TAB(jj)));
   }
  }


  /* read C-profile tables */

  ii = toSurfDataInfo->nrCProfiles;    
  jj = toSurfDataInfo->nrCPElements;   
  if((ii != 0) && (jj != 0))
  {
   surfCProfileTable = (SurfCProfileTable*)
                         calloc((u_int)ii,(size_t)SIZE_OF_SURF_C_PROFILE_TAB((u_int)jj));
   toSurfDataInfo->toCProfiles = surfCProfileTable;
   if (surfCProfileTable == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(nn=0;nn<ii;nn++)
   {
    surfCProfileTable =  getSurfCProfileTable(toSurfDataInfo->toCProfiles,
                                                       (short)jj,(long)nn);
    ret = xdr_SurfCProfileTable(xdrs,surfCProfileTable,(u_short)jj);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
   }
  }


  /* read C-profile TPE-values */

  if(toSurfDataInfo->nrCprofTpes > 0)
  {
   ii = toSurfDataInfo->nrCProfiles;    
   jj = toSurfDataInfo->nrCPElements;   
   if((ii != 0) && (jj != 0))
   {
    surfCProfileTpeTable = (SurfCProfileTpeTable*)
                  calloc((u_int)ii,(size_t)SIZE_OF_SURF_C_PROFILE_TPE_TAB((u_int)jj));
    toSurfDataInfo->toCProfileTpes = surfCProfileTpeTable;
    if (surfCProfileTpeTable == NULL)
    {
     return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
    }
    for(nn=0;nn<ii;nn++)
    {
     surfCProfileTpeTable =  getSurfCProfileTpeTable(toSurfDataInfo->toCProfileTpes,
                                                                (short)jj,(long)nn);
     ret = xdr_SurfCProfileTableTpes(xdrs,surfCProfileTpeTable,(u_short)jj);
     if(ret != SURF_SUCCESS)
     {
      return(freeSixBlocks(toSurfDataInfo,ret));
     }
    }
   }
  }



  /* read areapolygon */

  jj = toSurfDataInfo->nrPolyElements;   
  if(jj != 0)
  {
   surfPolygons = (SurfPolygons*)calloc(1,(size_t)SIZE_OF_SURF_POLYGON_ARRAY((u_int)jj));
   toSurfDataInfo->toPolygons = surfPolygons;
   if (surfPolygons == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfPolygons(xdrs,surfPolygons,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  /* read event blocks*/

  jj = toSurfDataInfo->nrEvents;
  if(jj != 0)
  {
   surfEvents = (SurfEvents*)calloc(1,(size_t)SIZE_OF_SURF_EVENT_ARRAY((u_int)jj));
   toSurfDataInfo->toEvents = surfEvents;
   if (surfEvents == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfEvents(xdrs,surfEvents,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  /* read free text block*/

  jj = toSurfDataInfo->nrFreeTextUnits;
  if(jj != 0)
  {
   surfFreeText = (SurfFreeText*)calloc(1,(size_t)SIZE_OF_FREE_TEXT_ARRAY((u_int)jj));
   toSurfDataInfo->toFreeText = surfFreeText;
   if (surfFreeText == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfFreeText(xdrs,surfFreeText,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  /* read additional Statistics  */

  if(toSurfDataInfo->nrAddStatistics != 0)
  {
   surfAddStatistics = (SurfAddStatistics*)calloc(1,sizeof(SurfAddStatistics));
   toSurfDataInfo->toAddStatistics = surfAddStatistics;
   if (surfStatistics == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfAddStatistics(xdrs,surfAddStatistics);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  
  /* read TPE Static Data  */

  if(toSurfDataInfo->nrTpeStatics != 0)
  {
   surfTpeStatics = (SurfTpeStatics*)calloc(1,sizeof(SurfTpeStatics));
   toSurfDataInfo->toTpeStatics = surfTpeStatics;
   if (surfTpeStatics == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfTpeStatics(xdrs,surfTpeStatics);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }



  
  /* read Free Six Data Descriptor */

  jj = toSurfDataInfo->nrOfSixAttachedData;
  if(jj > 0)
  {
   surfFreeSixDataDescr = (SurfFreeSixDataDescr*)calloc((u_int)jj,sizeof(SurfFreeSixDataDescr));
   toSurfDataInfo->toFreeSixDataDescr = surfFreeSixDataDescr;
   if (surfFreeSixDataDescr == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSixDataDescr = &(toSurfDataInfo->toFreeSixDataDescr[ii]);
    ret = xdr_SurfFreeSixDataDescr(xdrs,surfFreeSixDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
   }
  }



  /* read Free Sounding Data Descriptor */

  jj = toSurfDataInfo->nrOfSndgAttachedData;
  if(jj > 0)
  {
   surfFreeSndgDataDescr = (SurfFreeSndgDataDescr*)calloc((u_int)jj,sizeof(SurfFreeSndgDataDescr));
   toSurfDataInfo->toFreeSndgDataDescr = surfFreeSndgDataDescr;
   if (surfFreeSndgDataDescr == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSndgDataDescr = &(toSurfDataInfo->toFreeSndgDataDescr[ii]);
    ret = xdr_SurfFreeSndgDataDescr(xdrs,surfFreeSndgDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
   }
  }



  /* read Free Beam Data Descriptor */

  jj = toSurfDataInfo->nrOfBeamAttachedData;
  if(jj > 0)
  {
   surfFreeBeamDataDescr = (SurfFreeBeamDataDescr*)calloc((u_int)jj,sizeof(SurfFreeBeamDataDescr));
   toSurfDataInfo->toFreeBeamDataDescr = surfFreeBeamDataDescr;
   if (surfFreeBeamDataDescr == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeBeamDataDescr = &(toSurfDataInfo->toFreeBeamDataDescr[ii]);
    ret = xdr_SurfFreeBeamDataDescr(xdrs,surfFreeBeamDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
   }
  }



  /* read Free Six Data */

  jj = toSurfDataInfo->nrOfSixAttachedData;
  if(jj > 0)
  {
   surfFreeSixAttachedData = (SurfFreeSixAttachedData*)calloc((u_int)jj,sizeof(SurfFreeSixAttachedData));
   toSurfDataInfo->toFreeSixData = surfFreeSixAttachedData;
   if (surfFreeSixAttachedData == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSixAttachedData = &(toSurfDataInfo->toFreeSixData[ii]);
    ret = xdr_SurfFreeSixAttachedData(xdrs,surfFreeSixAttachedData);
    if(ret != SURF_SUCCESS)
    {
     return(freeSixBlocks(toSurfDataInfo,ret));
    }
   }
  }



  /* read Vendor Text  */

  if(toSurfDataInfo->nrOfVendorText != 0)
  {
   surfVendorText = (SurfVendorText*)calloc(1,sizeof(SurfVendorText));
   toSurfDataInfo->toVendorText = surfVendorText;
   if (surfVendorText == NULL)
   {
    return(freeSixBlocks(toSurfDataInfo,SURF_CANT_GET_MEMORY));
   }
   ret = xdr_SurfVendorText(xdrs,surfVendorText);
   if(ret != SURF_SUCCESS)
   {
    return(freeSixBlocks(toSurfDataInfo,ret));
   }
  }


  /* Everything is done successfully (hope so ?)*/

  if(toSurfDataInfo->fp != NULL)fclose(fp);
  toSurfDataInfo->fp = NULL;
  return(SURF_SUCCESS);

}




/***************************************/
/*                                     */
/* Allocate memory for SDA-Block and   */
/* read data from file                 */
/*                                     */
/***************************************/


static size_t align64(size_t size)
{
 return( (size_t) ( ((int) ((size+7)/8))*8 ) );
}

/* calculate the sizes within a SDA-Block */


size_t initializeSdaInfo(SurfDataInfo* toSurfDataInfo,SdaInfo* toSdaInfo)
{
 size_t sum;
 short nrAmpli;
 
 toSdaInfo->indexCenterPosition = 0;
 toSdaInfo->indexMultiBeam      = 0;
 toSdaInfo->indexAmplitudes     = 0;

 toSdaInfo->nrCenterPosition = toSurfDataInfo->nrOfCenterPositions;
 toSdaInfo->nrBeam           = toSurfDataInfo->nrOfMultiBeamDepth;
 toSdaInfo->nrAmplitudes     = toSurfDataInfo->nrOfSignalAmplitudes;
 toSdaInfo->nrSsData         = toSurfDataInfo->maxNrSsData;
 toSdaInfo->nrRxParams       = toSurfDataInfo->nrOfRxSets;
 toSdaInfo->nrTxParams       = toSurfDataInfo->nrOfTxSets;
 toSdaInfo->nrOfSndgAttachedData = toSurfDataInfo->nrOfSndgAttachedData;
 toSdaInfo->nrOfBeamAttachedData = toSurfDataInfo->nrOfBeamAttachedData;

 toSdaInfo->soundingS        = sizeof(SurfSoundingData);
 toSdaInfo->sndgAttDataS = 0;
 if(toSurfDataInfo->nrOfSndgAttachedData > 0)
 {
  toSdaInfo->sndgAttDataS = sizeof(SurfFreeSoundingAttachedData);
 }
 toSdaInfo->centerPS         = sizeof(SurfCenterPosition);
 toSdaInfo->positionCepDataS = 0;
 if(toSurfDataInfo->nrOfCeps > 0)
 {
  toSdaInfo->positionCepDataS= sizeof(SurfPositionCepData);
 }
 toSdaInfo->singleBDS       = 0;
 if(toSurfDataInfo->nrOfSingleBeamDepth > 0)
 {
  toSdaInfo->singleBDS        = sizeof(SurfSingleBeamDepth);
 }
 toSdaInfo->singleTPEsS       = 0;
 if(toSurfDataInfo->nrOfSingleTPEs > 0)
 {
  toSdaInfo->singleTPEsS      = sizeof(SurfTpeValues);
 }
 toSdaInfo->multiBDS       = 0;
 if(toSurfDataInfo->nrOfMultiBeamDepth > 0)
 {
  toSdaInfo->multiBDS        = sizeof(SurfMultiBeamDepth);
 } 
 toSdaInfo->multiBTTS      = 0;
 if(toSurfDataInfo->nrOfMultiBeamTT > 0)
 {
  toSdaInfo->multiBTTS       = sizeof(SurfMultiBeamTT);
 } 
 toSdaInfo->multiBRS       = 0;
 if(toSurfDataInfo->nrOfMultiBeamRec > 0)
 {
  toSdaInfo->multiBRS        = sizeof(SurfMultiBeamReceive);
 } 
 toSdaInfo->multiTPEsS       = 0;
 if(toSurfDataInfo->nrOfMultiTPEs > 0)
 {
  toSdaInfo->multiTPEsS      = sizeof(SurfTpeValues);
 }
 toSdaInfo->beamAttDataS = 0;
 if(toSurfDataInfo->nrOfBeamAttachedData > 0)
 {
  toSdaInfo->beamAttDataS = sizeof(SurfFreeBeamAttachedData);
 }
 toSdaInfo->amplS           = 0;
 if(toSurfDataInfo->nrOfAmplitudes > 0)
 {
  toSdaInfo->amplS           = sizeof(SurfAmplitudes);
 } 
 toSdaInfo->extAmplS        = 0;
 if(toSurfDataInfo->nrOfExtAmplitudes > 0)
 {
  toSdaInfo->extAmplS        = sizeof(SurfExtendedAmplitudes);
 } 
 toSdaInfo->signalPS       = 0;
 if(toSurfDataInfo->nrOfSignalParams > 0)
 {
  toSdaInfo->signalPS        = 
                        SIZE_OF_SURF_SIGNAL_PARAMETER((u_int)toSdaInfo->nrRxParams);
 } 
 toSdaInfo->signalTxPS       = 0;
 if(toSurfDataInfo->nrOfTxParams > 0)
 {
  toSdaInfo->signalTxPS       = 
                        SIZE_OF_SURF_TX_PARAMETER((u_int)toSdaInfo->nrTxParams);
 } 
 toSdaInfo->signalAS       = 0;
 if(toSurfDataInfo->nrOfSignalAmplitudes > 0)
 {
  toSdaInfo->signalAS        = 
    SIZE_OF_SURF_SIGNAL_AMPLITUDES_ARRAY((u_int)toSurfDataInfo->nrOfSignalAmplitudes);
 }
 toSdaInfo->ssDataS       = 0;
 if(toSurfDataInfo->nrOfSsData > 0)
 {
  toSdaInfo->ssDataS        = 
    SIZE_OF_SURF_SIDESCAN_DATA_ARRAY((u_int)toSurfDataInfo->maxNrSsData);
 }

 nrAmpli=(short)toSdaInfo->nrBeam;
 if((nrAmpli%2) != 0)nrAmpli++;

 sum = align64( toSdaInfo->soundingS                                                )
     + align64( toSdaInfo->sndgAttDataS  * (size_t)(toSdaInfo->nrOfSndgAttachedData))
     + align64( toSdaInfo->centerPS          * (size_t)(toSdaInfo->nrCenterPosition))
     + align64( toSdaInfo->positionCepDataS  * (size_t)(toSdaInfo->nrCenterPosition))
     + align64( toSdaInfo->singleBDS                                                )
     + align64( toSdaInfo->singleTPEsS                                              )
     + align64( toSdaInfo->multiBDS   * (size_t)(toSdaInfo->nrBeam                 ))
     + align64( toSdaInfo->multiBTTS  * (size_t)(toSdaInfo->nrBeam                 ))
     + align64( toSdaInfo->multiBRS   * (size_t)(toSdaInfo->nrBeam                 ))
     + align64( toSdaInfo->multiTPEsS * (size_t)(toSdaInfo->nrBeam                 ))
     + align64( toSdaInfo->beamAttDataS  * (size_t)(toSdaInfo->nrOfBeamAttachedData)
                                  * (size_t)(toSdaInfo->nrBeam )             )
     + align64( toSdaInfo->amplS      * (size_t)(nrAmpli                           ))
     + align64( toSdaInfo->extAmplS   * (size_t)(toSdaInfo->nrBeam                 ))
     + align64( toSdaInfo->signalPS                                                 )
     + align64( toSdaInfo->signalTxPS                                               )
     + align64( toSdaInfo->signalAS                                                 )
     + align64( toSdaInfo->ssDataS                                                  );
     
 toSdaInfo->allS = sum;
 return(sum);
}




/* sets the sda pointers in SdaInfo         */

void setPointersInSdaInfo(void* toSdaBlock,SdaInfo* toSdaInfo)
{
 char* bp;
 short nrAmpli;

 bp = (char*)toSdaBlock;
 
 toSdaInfo->toSoundings = (SurfSoundingData*) bp;
 bp = bp + align64(toSdaInfo->soundingS);

 if( toSdaInfo->sndgAttDataS == 0)
 {
  toSdaInfo->toFreeSoundingAttachedData = NULL;
 }
 else
 { 
  toSdaInfo->toFreeSoundingAttachedData = (SurfFreeSoundingAttachedData*) bp;
  bp = bp + align64(toSdaInfo->sndgAttDataS * (size_t)(toSdaInfo->nrOfSndgAttachedData));
 }
 
 toSdaInfo->toCenterPositions    = (SurfCenterPosition*) bp;
 toSdaInfo->toActCenterPosition  = (SurfCenterPosition*)(bp 
              + (toSdaInfo->centerPS * toSdaInfo->indexCenterPosition));
 bp = bp + align64((size_t)(toSdaInfo->centerPS * toSdaInfo->nrCenterPosition));

 if( toSdaInfo->positionCepDataS == 0)
 {
  toSdaInfo->toPositionCepData    = NULL;
 }
 else
 {
  toSdaInfo->toPositionCepData    = (SurfPositionCepData*) bp;
  bp = bp + align64((size_t)(toSdaInfo->positionCepDataS * toSdaInfo->nrCenterPosition));
 }

 if( toSdaInfo->singleBDS == 0)
 {
  toSdaInfo->toSingleBeamDepth = NULL;
 }
 else
 {
  toSdaInfo->toSingleBeamDepth = (SurfSingleBeamDepth*) bp;
  bp = bp + align64(toSdaInfo->singleBDS); 
 }

 if( toSdaInfo->singleTPEsS == 0)
 {
  toSdaInfo->toSingleBeamTpeValues = NULL;
 }
 else
 {
  toSdaInfo->toSingleBeamTpeValues = (SurfTpeValues*) bp;
  bp = bp + align64(toSdaInfo->singleTPEsS); 
 }

 if( toSdaInfo->multiBDS == 0)
 {
  toSdaInfo->toMultiBeamDepth = NULL;
  toSdaInfo->toActMultiBeamDepth = NULL;
 }
 else
 {
  toSdaInfo->toMultiBeamDepth = (SurfMultiBeamDepth*) bp;
  toSdaInfo->toActMultiBeamDepth  = (SurfMultiBeamDepth*)(bp 
              + (toSdaInfo->multiBDS * toSdaInfo->indexMultiBeam));
  bp = bp + align64((size_t)(toSdaInfo->multiBDS * toSdaInfo->nrBeam)); 
 }

 if( toSdaInfo->multiBTTS == 0)
 {
  toSdaInfo->toMultiBeamTT = NULL;
  toSdaInfo->toActMultiBeamTT = NULL;
 }
 else
 {
  toSdaInfo->toMultiBeamTT = (SurfMultiBeamTT*) bp;
  toSdaInfo->toActMultiBeamTT  = (SurfMultiBeamTT*)(bp 
              + (toSdaInfo->multiBTTS * toSdaInfo->indexMultiBeam));
  bp = bp + align64((size_t)(toSdaInfo->multiBTTS * toSdaInfo->nrBeam)); 
 }

 if( toSdaInfo->multiBRS == 0)
 {
  toSdaInfo->toMultiBeamRec = NULL;
  toSdaInfo->toActMultiBeamRec = NULL;
 }
 else
 {
  toSdaInfo->toMultiBeamRec = (SurfMultiBeamReceive*) bp;
  toSdaInfo->toActMultiBeamRec  = (SurfMultiBeamReceive*)(bp 
              + (toSdaInfo->multiBRS * toSdaInfo->indexMultiBeam));
  bp = bp + align64((size_t)(toSdaInfo->multiBRS * toSdaInfo->nrBeam)); 
 }

 if( toSdaInfo->multiTPEsS == 0)
 {
  toSdaInfo->toMultiBeamTpeValues = NULL;
 }
 else
 {
  toSdaInfo->toMultiBeamTpeValues = (SurfTpeValues*) bp;
  bp = bp + align64(toSdaInfo->multiTPEsS * (size_t)(toSdaInfo->nrBeam)); 
 }

 if( toSdaInfo->beamAttDataS == 0)
 {
  toSdaInfo->toFreeBeamAttachedData = NULL;
 }
 else
 {
  toSdaInfo->toFreeBeamAttachedData = (SurfFreeBeamAttachedData*) bp;
  bp = bp + align64( toSdaInfo->beamAttDataS  * (size_t)(toSdaInfo->nrOfBeamAttachedData)
                                       * (size_t)(toSdaInfo->nrBeam ));
 }

 if( toSdaInfo->amplS    == 0)
 {
  toSdaInfo->toAmplitudes = NULL;
 }
 else
 {
  toSdaInfo->toAmplitudes = (SurfAmplitudes*) bp;
  nrAmpli=(short)toSdaInfo->nrBeam;
  if((nrAmpli%2) != 0)nrAmpli++;
  bp = bp + align64(toSdaInfo->amplS * nrAmpli); 
 }

 if( toSdaInfo->extAmplS    == 0)
 {
  toSdaInfo->toExtendedAmpl = NULL;
 }
 else
 {
  toSdaInfo->toExtendedAmpl = (SurfExtendedAmplitudes*) bp;
  bp = bp + align64((size_t)(toSdaInfo->extAmplS * toSdaInfo->nrBeam)); 
 }

 if( toSdaInfo->signalPS == 0)
 {
  toSdaInfo->toSignalParams = NULL;
 }
 else
 {
  toSdaInfo->toSignalParams = (SurfSignalParameter*) bp;
  bp = bp + align64(toSdaInfo->signalPS);
 }

 if( toSdaInfo->signalTxPS == 0)
 {
  toSdaInfo->toTxParams = NULL;
 }
 else
 {
  toSdaInfo->toTxParams = (SurfTxParameter*) bp;
  bp = bp + align64(toSdaInfo->signalTxPS);
 }

 if( toSdaInfo->signalAS == 0)
 {
  toSdaInfo->toSignalAmplitudes = NULL;
  toSdaInfo->toActSignalAmplitudes = NULL;
 }
 else
 {
  toSdaInfo->toSignalAmplitudes = (SurfSignalAmplitudes*) bp;
  toSdaInfo->toActSignalAmplitudes = (SurfSignalAmplitudes*)bp; 
  bp = bp + align64(toSdaInfo->signalAS);
 }

 if( toSdaInfo->ssDataS == 0)
 {
  toSdaInfo->toSsData = NULL;
 }
 else
 {
  toSdaInfo->toSsData = (SurfSidescanData*) bp;
  bp = bp + align64(toSdaInfo->ssDataS);
 }
}




/* converts  one SDA-Block memory->file or file->memory */
 
XdrSurf mem_convertOneSdaBlock2(XDR* xdrs,SdaInfo* sdaInfo,short versLess2)
{
 XdrSurf ret;
 char* bp;
 u_long ii,jj;
 
 
 ret = xdr_SurfSoundingData(xdrs,sdaInfo->toSoundings,versLess2);
 if(ret != SURF_SUCCESS) return(ret);

 if(sdaInfo->toFreeSoundingAttachedData != NULL)
 { 
  for(ii=0;ii < sdaInfo->nrOfSndgAttachedData;ii++)
  {
   bp = (char*)(sdaInfo->toFreeSoundingAttachedData)+(ii * sdaInfo->sndgAttDataS);
   ret = xdr_SurfFreeSoundingAttachedData(xdrs,(SurfFreeSoundingAttachedData*) bp);
   if(ret != SURF_SUCCESS)
   {
    return(ret);
   } 
  }
 }

 for(ii=0;ii < sdaInfo->nrCenterPosition;ii++)
 {
  bp = (char*)(sdaInfo->toCenterPositions)+(ii * sdaInfo->centerPS);
  ret = xdr_SurfCenterPosition(xdrs,(SurfCenterPosition*) bp);
  if(ret != SURF_SUCCESS)
  {
   return(ret);
  } 
 }

 if(sdaInfo->toPositionCepData != NULL)
 { 
  for(ii=0;ii < sdaInfo->nrCenterPosition;ii++)
  {
   bp = (char*)(sdaInfo->toPositionCepData)+(ii * sdaInfo->positionCepDataS);
   ret = xdr_SurfPositionCepData(xdrs,(SurfPositionCepData*) bp);
   if(ret != SURF_SUCCESS)
   {
    return(ret);
   } 
  }
 }

 if(sdaInfo->toSingleBeamDepth != NULL)
 {
  ret = xdr_SurfSingleBeamDepth(xdrs,sdaInfo->toSingleBeamDepth);
  if(ret != SURF_SUCCESS)
  {
   return(ret);
  } 
 }

 if(sdaInfo->toSingleBeamTpeValues != NULL)
 {
  ret = xdr_SurfTpeValues(xdrs,(SurfTpeValues*) sdaInfo->toSingleBeamTpeValues);
  if(ret != SURF_SUCCESS)
  {
   return(ret);
  } 
 }

 if(sdaInfo->toMultiBeamDepth != NULL)
 {
   for(ii=0;ii < sdaInfo->nrBeam;ii++)
   {
    bp = (char*)(sdaInfo->toMultiBeamDepth)+(ii * sdaInfo->multiBDS);
    ret = xdr_SurfMultiBeamDepth(xdrs,(SurfMultiBeamDepth*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    } 
   }
 }

 if(sdaInfo->toMultiBeamTT != NULL)
 {
   for(ii=0;ii < sdaInfo->nrBeam;ii++)
   {
    bp = (char*)(sdaInfo->toMultiBeamTT)+(ii * sdaInfo->multiBTTS);
    ret = xdr_SurfMultiBeamTT(xdrs,(SurfMultiBeamTT*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    } 
   }
 }

 if(sdaInfo->toMultiBeamRec != NULL)
 {
   for(ii=0;ii < sdaInfo->nrBeam;ii++)
   {
    bp = (char*)(sdaInfo->toMultiBeamRec)+(ii * sdaInfo->multiBRS);
    ret = xdr_SurfMultiBeamReceive(xdrs,(SurfMultiBeamReceive*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    } 
   }
 }

 if(sdaInfo->toMultiBeamTpeValues != NULL)
 {
  for(ii=0;ii < sdaInfo->nrBeam;ii++)
  {
   bp = (char*)(sdaInfo->toMultiBeamTpeValues)+(ii * sizeof(SurfTpeValues));
   ret = xdr_SurfTpeValues(xdrs,(SurfTpeValues*) bp);
   if(ret != SURF_SUCCESS)
   {
    return(ret);
   } 
  }
 }

 if(sdaInfo->toFreeBeamAttachedData != NULL)
 {
  for(ii=0;ii < sdaInfo->nrBeam;ii++)
  {
   for(jj=0;jj < sdaInfo->nrOfBeamAttachedData;jj++)
   {
    bp = (char*)(sdaInfo->toFreeBeamAttachedData)
         +((size_t)((ii * sdaInfo->nrOfBeamAttachedData) + jj) 
                                        * sizeof(SurfFreeBeamAttachedData));
    ret = xdr_SurfFreeBeamAttachedData(xdrs,(SurfFreeBeamAttachedData*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    }
   } 
  }
 }

 if(sdaInfo->toAmplitudes != NULL)
 {
   for(ii=0;ii < sdaInfo->nrBeam;ii++)
   {
    bp = (char*)(sdaInfo->toAmplitudes)+(ii * sdaInfo->amplS);
    ret = xdr_SurfAmplitudes(xdrs,(SurfAmplitudes*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    } 
   }
 }

 if(sdaInfo->toExtendedAmpl != NULL)
 {
   for(ii=0;ii < sdaInfo->nrBeam;ii++)
   {
    bp = (char*)(sdaInfo->toExtendedAmpl)+(ii * sdaInfo->extAmplS);
    ret = xdr_SurfExtendedAmplitudes(xdrs,(SurfExtendedAmplitudes*) bp);
    if(ret != SURF_SUCCESS)
    {
     return(ret);
    } 
   }
 }

 if(sdaInfo->toSignalParams != NULL)
 {
  ret = xdr_SurfSignalParameter(xdrs,sdaInfo->toSignalParams,
                                                 (short)sdaInfo->nrRxParams);
  if(ret != SURF_SUCCESS)
  {
   return(ret);
  } 
 }

 if(sdaInfo->toTxParams != NULL)
 {
  ret = xdr_SurfTxParameter(xdrs,sdaInfo->toTxParams,(short)sdaInfo->nrTxParams);
  if(ret != SURF_SUCCESS)
  {
   return(ret);
  } 
 }

 if(sdaInfo->toSignalAmplitudes != NULL)
 {
   bp = (char*)(sdaInfo->toSignalAmplitudes);
   ret = xdr_SurfSignalAmplitudes(xdrs,(SurfSignalAmplitudes*) bp,
         (u_short)sdaInfo->nrAmplitudes);
   return(ret);
 }


 if(sdaInfo->toSsData != NULL)
 {
   bp = (char*)(sdaInfo->toSsData);
   ret = xdr_SurfSidescanData(xdrs,(SurfSidescanData*) bp,
         (u_short)sdaInfo->nrSsData);
   return(ret);
 }
 return(SURF_SUCCESS);
}


XdrSurf mem_convertOneSdaBlock(XDR* xdrs,SdaInfo* sdaInfo)
{
 return(mem_convertOneSdaBlock2(xdrs,sdaInfo,0));
}


/* frees Sda-Memory */
 
void free_SdaMemory(SurfDataInfo* toSurfDataInfo)
{
  SurfSoundingData* toSounding;
  u_long ii;

  if(toSurfDataInfo->xdrs != NULL)
  {
   free(toSurfDataInfo->xdrs);
   toSurfDataInfo->xdrs = NULL;
  }
  if(toSurfDataInfo->toSdaInfo != NULL)
  {
   free(toSurfDataInfo->toSdaInfo);
   toSurfDataInfo->toSdaInfo = NULL;
  }
  if(toSurfDataInfo->toSdaThread != NULL)
  {
   for(ii=0;ii < toSurfDataInfo->nrOfSoundings;ii++)
   {
    toSounding = toSurfDataInfo->toSdaThread->thread[ii].sounding;
    if(toSounding != NULL) free(toSounding);
    toSurfDataInfo->toSdaThread->thread[ii].sounding = NULL;
    toSounding = toSurfDataInfo->toSdaThread->thread[ii].saveSounding;
    if(toSounding != NULL) free(toSounding);
    toSurfDataInfo->toSdaThread->thread[ii].saveSounding = NULL;
   } 
   free(toSurfDataInfo->toSdaThread);
   toSurfDataInfo->toSdaThread = NULL;
  }
  if(toSurfDataInfo->fp != NULL)
  {
   fclose(toSurfDataInfo->fp);
   toSurfDataInfo->fp = NULL;
  }
}



/* reads a whole SDA-File into memory */
 
XdrSurf mem_ReadSdaStructure(char* filename,
                             SurfDataInfo* toSurfDataInfo)
                             
/* Allocates the necessary memory for a SDA-structure and reads 
   the data from file;
                                               */
{
  XdrSurf ret;
  XDR* xdrs;
  FILE* fp;
  SurfSdaThread* toSdaThread    ;
  SdaInfo*       toSdaInfo      ;
  SurfSoundingData* toSdaBlock;
  size_t         sizeOfSdaBlock;
  u_long  nrSoundings,nrBeams,nrPositions,nrAmplitudes;
  u_long  ii,jj;
  short versLess2;


  /* initialize local variables */

  nrSoundings = toSurfDataInfo->nrOfSoundings;
  nrPositions = toSurfDataInfo->nrOfCenterPositions;
  nrBeams     = toSurfDataInfo->nrOfMultiBeamDepth;
  nrAmplitudes = toSurfDataInfo->nrOfSignalAmplitudes;

  versLess2 = toSurfDataInfo->sourceVersionLess2;

  /* allocate structure for xdr-conversion and SdaInfo and Sda-Thread */
  
  xdrs = (XDR*)calloc(1,sizeof(XDR));
  toSurfDataInfo->xdrs = xdrs; 
  if (xdrs == NULL)
  {
   return(SURF_CANT_GET_MEMORY);
  }
  toSdaInfo = (SdaInfo*)calloc(1,sizeof(SdaInfo));
  toSurfDataInfo->toSdaInfo = toSdaInfo;
  if (toSdaInfo == NULL)
  {
   free_SdaMemory(toSurfDataInfo);
   return(SURF_CANT_GET_MEMORY);
  }
  toSdaThread = (SurfSdaThread*)calloc((u_int)nrSoundings,sizeof(SurfSdaThread));
  toSurfDataInfo->toSdaThread = toSdaThread;
  if (toSdaThread == NULL)
  {
   free_SdaMemory(toSurfDataInfo);
   return(SURF_CANT_GET_MEMORY);
  }

           

  /* open conversion and file Read  */
  
  fp =  xdrSurfOpenRead(xdrs,(const char*)filename);
  toSurfDataInfo->fp = fp; 
  if(fp == NULL)
  {
   free_SdaMemory(toSurfDataInfo);
   return(SURF_CANT_OPEN_FILE);
  }

  /* fill SdaInfo and allocate memory for Sda-Blocks */

  sizeOfSdaBlock = initializeSdaInfo(toSurfDataInfo,toSdaInfo);
  for(ii=0;ii < nrSoundings;ii++)
  {
   toSdaBlock = (SurfSoundingData*)malloc(sizeOfSdaBlock);
   if(toSdaBlock == NULL)
   {
    ret = (SURF_CANT_GET_MEMORY);
   }
   else
   {
    toSdaThread->thread[ii].sounding = toSdaBlock;
    setPointersInSdaInfo(toSdaBlock,toSdaInfo);
    ret = mem_convertOneSdaBlock2(xdrs,toSdaInfo,versLess2);
   }
   if(ret != SURF_SUCCESS)
   {
    for(jj=0;jj<ii;jj++)
    {
     if(toSdaThread->thread[jj].sounding != NULL)
       free(toSdaThread->thread[jj].sounding);
     toSdaThread->thread[jj].sounding = NULL;
    }
    free_SdaMemory(toSurfDataInfo);
    return(ret);
   }
  }

  free(xdrs);
  if(toSurfDataInfo->fp != NULL)fclose(fp);
  toSurfDataInfo->fp = NULL;
  toSurfDataInfo->xdrs = NULL;
  return(SURF_SUCCESS);
}                                               







/* for external use: converters,etc. */

XdrSurf mem_buildSurfSdaStructure(SurfDataInfo* toSurfDataInfo)
{
  XdrSurf ret;
  SurfSdaThread* toSdaThread    ;
  SdaInfo*       toSdaInfo      ;
  SurfSoundingData* toSdaBlock;
  size_t         sizeOfSdaBlock;
  u_long  nrSoundings,nrBeams,nrPositions,nrAmplitudes,nrSsData;
  u_long  ii,jj;

    /* initialize local variables */

  nrSoundings = toSurfDataInfo->nrOfSoundings;
  nrPositions = toSurfDataInfo->nrOfCenterPositions;
  nrBeams     = toSurfDataInfo->nrOfMultiBeamDepth;
  nrAmplitudes = toSurfDataInfo->nrOfSignalAmplitudes;
  nrSsData    = toSurfDataInfo->maxNrSsData;

  toSdaInfo = (SdaInfo*)calloc(1,sizeof(SdaInfo));
  toSurfDataInfo->toSdaInfo = toSdaInfo;
  if (toSdaInfo == NULL)
  {
   free_SdaMemory(toSurfDataInfo);
   return(SURF_CANT_GET_MEMORY);
  }
  toSdaThread = (SurfSdaThread*)calloc((u_int)nrSoundings,sizeof(SurfSdaThread));
  toSurfDataInfo->toSdaThread = toSdaThread;
  if (toSdaThread == NULL)
  {
   free_SdaMemory(toSurfDataInfo);
   return(SURF_CANT_GET_MEMORY);
  }

  ret = SURF_SUCCESS;
  sizeOfSdaBlock = initializeSdaInfo(toSurfDataInfo,toSdaInfo);
  for(ii=0;ii < nrSoundings;ii++)
  {
   toSdaBlock = (SurfSoundingData*)calloc(1,sizeOfSdaBlock);
   if(toSdaBlock == NULL)
   {
    ret = (SURF_CANT_GET_MEMORY);
   }
   else
   {
    toSdaThread->thread[ii].sounding = toSdaBlock;
   }
   if(ret != SURF_SUCCESS)
   {
    for(jj=0;jj<ii;jj++)
    {
     if(toSdaThread->thread[jj].sounding != NULL)
       free(toSdaThread->thread[jj].sounding);
     toSdaThread->thread[jj].sounding = NULL;
    }
    free_SdaMemory(toSurfDataInfo);
    return(ret);
   }
  }
  return(SURF_SUCCESS);
}                                               
             


/***************************************/
/*                                     */
/* write a SIX-File in memory back to  */
/* the file                            */
/*                                     */
/* the specific file-structure is given*/
/* in the descriptor- and pointers     */
/* to memory in the info-structure     */
/*                                     */
/* a NULL-pointer in Info tells that   */
/* this block is not existing in that  */
/* specific configuration              */
/*                                     */
/* if the information in Info is       */
/* different from the Descriptor,      */
/* the Descriptor will be updated      */
/*                                     */
/***************************************/


/* update the Descriptor-structure with data from the Info-Structure  */

XdrSurf checkAndUpdateSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                      SurfDataInfo* toSurfDataInfo)
{
  if((toSurfDescriptor->six                != SIX_M               ) ||
     (toSurfDescriptor->descriptor.typ     != DESCRIPTOR          ) ||
     (toSurfDescriptor->globalData.typ     != GLOBALDATA          ) ||
     (toSurfDescriptor->statistics.typ     != STATISTICS          ) ||
     (toSurfDescriptor->positionSensor.typ != POSITIONSENSORS     ) ||
     (toSurfDescriptor->transducer.typ     != TRANSDUCERPARAM     ) ||
     (toSurfDescriptor->angleTab.typ       != BEAMANGLE           ) ||
     (toSurfDescriptor->cProfile.typ       != CPROFILE            ) ||
     (toSurfDescriptor->polygon.typ        != AREAPOLYGON         ) ||
     (toSurfDescriptor->events.typ         != EVENTS              ) ||
     (toSurfDescriptor->freeText.typ       != FREETEXT            ) ||
     (toSurfDescriptor->addStatistics.typ  != ADDSTATISTICS       ) ||
     (toSurfDescriptor->tpeStatics.typ     != TPESTATICS          ) ||
     (toSurfDescriptor->cprofTpes.typ      != CPROFTPES           ) ||
     (toSurfDescriptor->freeSixDescr.typ   != FREESIXDESCR        ) ||
     (toSurfDescriptor->freeSndgDescr.typ  != FREESNDGDESCR       ) ||
     (toSurfDescriptor->freeBeamDescr.typ  != FREEBEAMDESCR       ) ||
     (toSurfDescriptor->freeSixAttData.typ != SIXATTDATA          ) ||
     (toSurfDescriptor->vendorText.typ     != VENDORTEXT          ) ||
     (toSurfDescriptor->sda                != SDA_M               ) ||
     (toSurfDescriptor->nrof               != NROF_M              ) ||
     (toSurfDescriptor->eod                != EOD_M               ))   
  {
   return(SURF_CORRUPTED_DATASET);
  }

  toSurfDescriptor->descriptor.nr  = 1;    
  toSurfDescriptor->globalData.nr  = 1;
  toSurfDescriptor->statistics.nr  = 0;
  if(toSurfDataInfo->toStatistics != NULL)
  {
    toSurfDescriptor->statistics.nr  = 1;
  }
  if((toSurfDataInfo->toPosiSensors == NULL) ||
     (toSurfDataInfo->nrPosiSensors == 0   ))
  {
    return(SURF_CORRUPTED_DATASET);
  }
  else
  {
    toSurfDescriptor->positionSensor.nr = toSurfDataInfo->nrPosiSensors;
  }
  toSurfDescriptor->transducer.nr     = toSurfDataInfo->nrTransducers;
  toSurfDescriptor->angleTab.nr       = toSurfDataInfo->nrAngleTables;
  toSurfDescriptor->maxNrOfBeams.nr   = toSurfDataInfo->nrBeams;
  toSurfDescriptor->cProfile.nr       = toSurfDataInfo->nrCProfiles;
  toSurfDescriptor->maxNrOfCProfileElements.nr
                                      = toSurfDataInfo->nrCPElements;
  toSurfDescriptor->maxNrOfSidescanData.nr
                                      = toSurfDataInfo->maxNrSsData;
  toSurfDescriptor->nrOfRxTvgSets.nr  = toSurfDataInfo->nrOfRxSets;   
  toSurfDescriptor->nrOfTxTvgSets.nr  = toSurfDataInfo->nrOfTxSets;   

  toSurfDescriptor->polygon.nr        = 0;
  toSurfDescriptor->maxNrOfPolygonElements.nr = 0;
  if(toSurfDataInfo->toPolygons  != NULL)
  {                         
   toSurfDescriptor->polygon.nr        = 1;
   toSurfDescriptor->maxNrOfPolygonElements.nr
                                      = toSurfDataInfo->nrPolyElements;
  }

  toSurfDescriptor->events.nr        = 0;
  toSurfDescriptor->maxNrOfEvents.nr = 0;
  if(toSurfDataInfo->toEvents != NULL)
  {                         
   toSurfDescriptor->events.nr        = 1;
   toSurfDescriptor->maxNrOfEvents.nr = toSurfDataInfo->nrEvents;
  }

  toSurfDescriptor->freeText.nr        = 0;
  toSurfDescriptor->maxNrOfFreeTextBlocks.nr = 0;
  if(toSurfDataInfo->toFreeText != NULL)
  {                         
   toSurfDescriptor->freeText.nr        = 1;
   toSurfDescriptor->maxNrOfFreeTextBlocks.nr 
                                        = toSurfDataInfo->nrFreeTextUnits;
  }
  
  toSurfDescriptor->addStatistics.nr    = 0;
  if(toSurfDataInfo->toAddStatistics != NULL)
  {                         
   toSurfDescriptor->addStatistics.nr   = 1;
  }
  
  toSurfDescriptor->tpeStatics.nr       = 0;
  if(toSurfDataInfo->toTpeStatics != NULL)
  {                         
   toSurfDescriptor->tpeStatics.nr      = 1;
  }

  toSurfDescriptor->cprofTpes.nr        = 0;
  if(toSurfDataInfo->toCProfileTpes != NULL)
  {
   toSurfDescriptor->cprofTpes.nr       = toSurfDataInfo->nrCProfiles;
  } 

  toSurfDescriptor->freeSixDescr.nr   = toSurfDataInfo->nrOfSixAttachedData;
  toSurfDescriptor->freeSndgDescr.nr  = toSurfDataInfo->nrOfSndgAttachedData;
  toSurfDescriptor->freeBeamDescr.nr  = toSurfDataInfo->nrOfBeamAttachedData;
  toSurfDescriptor->freeSixAttData.nr = toSurfDataInfo->nrOfSixAttachedData;

  toSurfDescriptor->vendorText.nr       = 0;
  if(toSurfDataInfo->toVendorText != NULL)
  {                         
   toSurfDescriptor->vendorText.nr      = 1;
  }

  
  /* sda -data */

  toSurfDescriptor->soundings.nr       = toSurfDataInfo->nrOfSoundings;
  toSurfDescriptor->centerPositions.nr = toSurfDataInfo->nrPosiSensors;
  toSurfDescriptor->positionCpes.nr    = toSurfDataInfo->nrOfCeps;
  toSurfDescriptor->singleBeamDepth.nr = toSurfDataInfo->nrOfSingleBeamDepth ;
  toSurfDescriptor->multiBeamDepth.nr  = toSurfDataInfo->nrOfMultiBeamDepth;
  toSurfDescriptor->multiBeamTT.nr     = toSurfDataInfo->nrOfMultiBeamTT;
  toSurfDescriptor->multiBeamRecv.nr   = toSurfDataInfo->nrOfMultiBeamRec;
  toSurfDescriptor->signalParams.nr    = toSurfDataInfo->nrOfSignalParams;   
  toSurfDescriptor->txParams.nr        = toSurfDataInfo->nrOfTxParams;   
  toSurfDescriptor->beamAmplitudes.nr  = toSurfDataInfo->nrOfAmplitudes;   
  toSurfDescriptor->extendBeamAmplitudes.nr
                                       = toSurfDataInfo->nrOfExtAmplitudes;   
  toSurfDescriptor->signalAmplitudes.nr= toSurfDataInfo->nrOfSignalAmplitudes;
  toSurfDescriptor->sidescanData.nr    = toSurfDataInfo->nrOfSsData;
  toSurfDescriptor->singleTpeParams.nr = toSurfDataInfo->nrOfSingleTPEs;
  toSurfDescriptor->multiTpeParams.nr  = toSurfDataInfo->nrOfMultiTPEs;
  toSurfDescriptor->sndgAttData.nr     = toSurfDataInfo->nrOfSndgAttachedData;
  toSurfDescriptor->beamAttData.nr     = toSurfDataInfo->nrOfBeamAttachedData;

  return(SURF_SUCCESS);          
}



                                      
/* cleaning the internal memory-requests */

XdrSurf cleanUpSixWrite(SurfDataInfo* toSurfDataInfo,
                        XDR* xdrs,FILE* fp,XdrSurf returnvalue)
{
  if(fp != NULL)
  {
   if(toSurfDataInfo->fp != NULL)fclose(fp);
   toSurfDataInfo->fp = NULL;
  }
  if(xdrs != NULL)
  {
   free(xdrs);
   toSurfDataInfo->xdrs = NULL;
  }
  return(returnvalue);
}



XdrSurf mem_WriteSixStructure(char* filename,
                             SurfDataInfo* toSurfDataInfo)
                             
/* writes a SIX-structure back to the file 
   according to SurfDataInfo                    */  
                              
{
  XdrSurf ret;
  XDR* xdrs;
  FILE* fp;
  SurfDescriptor* surfDescriptor;
  SurfGlobalData* surfGlobalData;
  SurfStatistics* surfStatistics;
  SurfPositionSensorArray* surfPositionSensorArray;
  SurfMultiBeamAngleTable* surfMultiBeamAngleTable; 
  SurfTransducerParameterTable* surfTransducerParameterTable;
  SurfCProfileTable* surfCProfileTable;
  SurfCProfileTpeTable* surfCProfileTpeTable;
  SurfPolygons* surfPolygons;
  SurfEvents*   surfEvents;   
  SurfFreeText*   surfFreeText;   
  SurfAddStatistics*       surfAddStatistics;
  SurfTpeStatics*          surfTpeStatics;
  SurfFreeSixDataDescr*    surfFreeSixDataDescr;
  SurfFreeSndgDataDescr*   surfFreeSndgDataDescr;
  SurfFreeBeamDataDescr*   surfFreeBeamDataDescr; 
  SurfFreeSixAttachedData* surfFreeSixAttachedData;
  SurfVendorText*          surfVendorText;
  u_long  ii,jj,nn;
  short newVersion,oldVersion;

  xdrs = NULL;
  fp = NULL;
  
  /* allocate structure for xdr-conversion */
  
  xdrs = (XDR*)calloc(1,sizeof(XDR));
  if (xdrs == NULL)
  {
   return(SURF_CANT_GET_MEMORY);
  }
  toSurfDataInfo->xdrs = xdrs; 
  surfDescriptor = toSurfDataInfo->toDescriptor;
  if (surfDescriptor == NULL)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
  }


  /* open conversion and file Write */
  
  fp =  xdrSurfOpenWrite(xdrs,(const char*)filename);
  toSurfDataInfo->fp = fp; 
  if(fp == NULL)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CANT_OPEN_FILE));
  }


  /* write SurfDescriptor */
  
  ret = checkAndUpdateSurfDescriptor(surfDescriptor,toSurfDataInfo);
  if(ret != SURF_SUCCESS)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
  }

  ret = xdr_SurfDescriptor(xdrs,surfDescriptor,&newVersion,&oldVersion);
  if(ret != SURF_SUCCESS)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
  }


  /* write Global Data */

  surfGlobalData = toSurfDataInfo->toGlobalData;
  if (surfGlobalData == NULL)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
  }
  ret = xdr_SurfGlobalData(xdrs,surfGlobalData);
  if(ret != SURF_SUCCESS)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
  }


  /* write Statistics  */

  if(toSurfDataInfo->nrStatistics != 0)
  {
   surfStatistics = toSurfDataInfo->toStatistics;
   if (surfStatistics == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfStatistics(xdrs,surfStatistics);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }


  /* write Position Sensors  */

  ii = toSurfDataInfo->nrPosiSensors;
  if(ii == 0)
  {
   return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
  }
  else
  {
   surfPositionSensorArray = toSurfDataInfo->toPosiSensors;
   if (surfPositionSensorArray == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   for(nn=0;nn<ii;nn++)
   {
    ret = xdr_PositionSensorArray(xdrs,surfPositionSensorArray,newVersion);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
    surfPositionSensorArray++;
   }
  }


  /* write transducer data of singlebeamsounders  */

  ii = toSurfDataInfo->nrTransducers;
  if(ii != 0)
  {
   surfTransducerParameterTable = toSurfDataInfo->toTransducers;
   if (surfTransducerParameterTable == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   for(nn=0;nn<ii;nn++)
   {
    ret = xdr_SurfTransducerParameterTable(xdrs,surfTransducerParameterTable);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
    surfTransducerParameterTable++;
   }
  }


  /* write beam-angle tables of multibeamsounders  */

  ii = toSurfDataInfo->nrAngleTables;  
  jj = toSurfDataInfo->nrBeams;        
  if((ii != 0) && (jj != 0))
  {
   surfMultiBeamAngleTable = toSurfDataInfo->toAngleTables;
   if (surfMultiBeamAngleTable == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   for(nn=0;nn<ii;nn++)
   {
    surfMultiBeamAngleTable = getSurfAngleTable(
                            toSurfDataInfo->toAngleTables,(short)jj,(long)nn);
    ret = xdr_SurfMultiBeamAngleTable(xdrs,surfMultiBeamAngleTable,(u_short)jj);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }


  /* write C-profile tables */

  ii = toSurfDataInfo->nrCProfiles;    
  jj = toSurfDataInfo->nrCPElements;   
  if((ii != 0) && (jj != 0))
  {
   surfCProfileTable = toSurfDataInfo->toCProfiles;
   if (surfCProfileTable == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   for(nn=0;nn<ii;nn++)
   {
    surfCProfileTable =  getSurfCProfileTable(toSurfDataInfo->toCProfiles,
                                                       (short)jj,(long)nn);
    ret = xdr_SurfCProfileTable(xdrs,surfCProfileTable,(u_short)jj);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }


  /* write C-profile TPE-values */

  if(toSurfDataInfo->nrCprofTpes > 0)
  {
   ii = toSurfDataInfo->nrCProfiles;    
   jj = toSurfDataInfo->nrCPElements;   
   if((ii != 0) && (jj != 0))
   {
    surfCProfileTpeTable = toSurfDataInfo->toCProfileTpes;
    if (surfCProfileTpeTable == NULL)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
    }
    for(nn=0;nn<ii;nn++)
    {
     surfCProfileTpeTable =  getSurfCProfileTpeTable(toSurfDataInfo->toCProfileTpes,
                                                                (short)jj,(long)nn);
     ret = xdr_SurfCProfileTableTpes(xdrs,surfCProfileTpeTable,(u_short)jj);
     if(ret != SURF_SUCCESS)
     {
      return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
     }
    }
   }
  }




  /* write areapolygon */

  jj = toSurfDataInfo->nrPolyElements;   
  if(jj != 0)
  {
   surfPolygons = toSurfDataInfo->toPolygons;
   if (surfPolygons == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfPolygons(xdrs,surfPolygons,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }


  /* write event blocks*/

  jj = toSurfDataInfo->nrEvents;
  if(jj != 0)
  {
   surfEvents = toSurfDataInfo->toEvents;
   if (surfEvents == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfEvents(xdrs,surfEvents,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }
  


  /* write free text blocks*/

  jj = toSurfDataInfo->nrFreeTextUnits;
  if(jj != 0)
  {
   surfFreeText = toSurfDataInfo->toFreeText;
   if (surfFreeText == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfFreeText(xdrs,surfFreeText,(u_short)jj);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }
  


  /* write additional Statistics  */

  if(toSurfDataInfo->nrAddStatistics != 0)
  {
   surfAddStatistics = toSurfDataInfo->toAddStatistics;
   if (surfAddStatistics == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfAddStatistics(xdrs,surfAddStatistics);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }


  
  /* write TPE Static Data  */

  if(toSurfDataInfo->nrTpeStatics != 0)
  {
   surfTpeStatics = toSurfDataInfo->toTpeStatics;
   if (surfTpeStatics == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   ret = xdr_SurfTpeStatics(xdrs,surfTpeStatics);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }



  
  /* write Free Six Data Descriptor */

  jj = toSurfDataInfo->nrOfSixAttachedData;
  if(jj > 0)
  {
   surfFreeSixDataDescr = toSurfDataInfo->toFreeSixDataDescr;
   if (surfFreeSixDataDescr == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_CORRUPTED_DATASET));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSixDataDescr = &(toSurfDataInfo->toFreeSixDataDescr[ii]);
    ret = xdr_SurfFreeSixDataDescr(xdrs,surfFreeSixDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }



  /* write Free Sounding Data Descriptor */

  jj = toSurfDataInfo->nrOfSndgAttachedData;
  if(jj > 0)
  {
   surfFreeSndgDataDescr = toSurfDataInfo->toFreeSndgDataDescr;
   if (surfFreeSndgDataDescr == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSndgDataDescr = &(toSurfDataInfo->toFreeSndgDataDescr[ii]);
    ret = xdr_SurfFreeSndgDataDescr(xdrs,surfFreeSndgDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }



  /* write Free Beam Data Descriptor */

  jj = toSurfDataInfo->nrOfBeamAttachedData;
  if(jj > 0)
  {
   surfFreeBeamDataDescr = toSurfDataInfo->toFreeBeamDataDescr;
   if (surfFreeBeamDataDescr == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeBeamDataDescr = &(toSurfDataInfo->toFreeBeamDataDescr[ii]);
    ret = xdr_SurfFreeBeamDataDescr(xdrs,surfFreeBeamDataDescr);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }



  /* write Free Six Data */

  jj = toSurfDataInfo->nrOfSixAttachedData;
  if(jj > 0)
  {
   surfFreeSixAttachedData = toSurfDataInfo->toFreeSixData;
   if (surfFreeSixAttachedData == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
   for(ii=0;ii<jj;ii++)
   {
    surfFreeSixAttachedData = &(toSurfDataInfo->toFreeSixData[ii]);
    ret = xdr_SurfFreeSixAttachedData(xdrs,surfFreeSixAttachedData);
    if(ret != SURF_SUCCESS)
    {
     return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
    }
   }
  }



  /* write Vendor Text  */

  if(toSurfDataInfo->nrOfVendorText != 0)
  {
   surfVendorText = toSurfDataInfo->toVendorText;
   if (surfVendorText == NULL)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
   ret = xdr_SurfVendorText(xdrs,surfVendorText);
   if(ret != SURF_SUCCESS)
   {
    return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,ret));
   }
  }


  /* Everything is done successfully (hope so ?)*/

  return(cleanUpSixWrite(toSurfDataInfo,xdrs,fp,SURF_SUCCESS));

}




/***************************************/
/*                                     */
/* Write SDA-Data into a file          */
/*                                     */
/***************************************/



/* writes a whole SDA-File from memory */
 
XdrSurf mem_WriteSdaStructure(char* filename,
                              SurfDataInfo* toSurfDataInfo)
{
  XdrSurf ret;
  XDR* xdrs;
  FILE* fp;
  SurfSdaThread* toSdaThread    ;
  SdaInfo*       toSdaInfo      ;
  SurfSoundingData* toSdaBlock;
  u_long  nrSoundings,nrBeams,nrPositions,nrAmplitudes;
  u_long  ii;


  /* initialize local variables */

  nrSoundings = toSurfDataInfo->nrOfSoundings;
  nrPositions = toSurfDataInfo->nrOfCenterPositions;
  nrBeams     = toSurfDataInfo->nrOfMultiBeamDepth;
  nrAmplitudes = toSurfDataInfo->nrOfSignalAmplitudes;


  /* allocate structure for xdr-conversion and SdaInfo and Sda-Thread */
  
  xdrs = (XDR*)calloc(1,sizeof(XDR));
  toSurfDataInfo->xdrs = xdrs; 
  if (xdrs == NULL)
  {
   return(SURF_CANT_GET_MEMORY);
  }
  toSdaInfo = toSurfDataInfo->toSdaInfo;
  toSdaThread = toSurfDataInfo->toSdaThread;
           

  /* open conversion and file Read  */
  
  fp =  xdrSurfOpenWrite(xdrs,(const char*)filename);
  toSurfDataInfo->fp = fp; 
  if(fp == NULL)
  {
   free(xdrs);
   toSurfDataInfo->xdrs = NULL; 
   return(SURF_CANT_OPEN_FILE);
  }

  /* fill SdaInfo and allocate memory for Sda-Blocks */

  for(ii=0;ii < nrSoundings;ii++)
  {
   toSdaBlock = toSdaThread->thread[ii].sounding;
   setPointersInSdaInfo(toSdaBlock,toSdaInfo);
   ret = mem_convertOneSdaBlock2(xdrs,toSdaInfo,0);
   if(ret != SURF_SUCCESS)
   {
    free(xdrs);
    if(toSurfDataInfo->fp != NULL) fclose(fp);
    toSurfDataInfo->xdrs = NULL; 
    toSurfDataInfo->fp   = NULL; 
    return(ret);
   }
  }

  free(xdrs);
  if(toSurfDataInfo->fp != NULL) fclose(fp);
  toSurfDataInfo->fp = NULL;
  toSurfDataInfo->xdrs = NULL;
  return(SURF_SUCCESS);
}                                               







/****************************************/
/*                                      */
/* destroys a whole SURF-Structure      */
/* (including SurfDataInfo)             */
/*                                      */
/****************************************/


XdrSurf mem_destroyAWholeSurfStructure(SurfDataInfo* toSurfDataInfo)
{
 XdrSurf ret;

  ret = SURF_SUCCESS;
  if(toSurfDataInfo != NULL)
  { 
   free_SdaMemory(toSurfDataInfo);
   ret = freeSixBlocks(toSurfDataInfo,SURF_SUCCESS);
   free(toSurfDataInfo);
  }
  return(ret);
}




/****************************************/
/*                                      */
/* get Table-pointers                   */
/*     in SIX-structure                 */
/*                                      */
/****************************************/



SurfMultiBeamAngleTable* getSurfAngleTable(SurfMultiBeamAngleTable*
                                toAngleTable,short nrBeams,long index)
{
 SurfMultiBeamAngleTable* ret;
 ret = (SurfMultiBeamAngleTable*)((char*)(toAngleTable) + 
       ((size_t)(index) * 
           (size_t)(SIZE_OF_SURF_MULTIBEAM_ANGLE_TAB(nrBeams))));
 return(ret);        
}        


SurfCProfileTable* getSurfCProfileTable(SurfCProfileTable* 
                                toCProf,short nrCPElements,long index)
{
 SurfCProfileTable* ret;
 ret = (SurfCProfileTable*)((char*)(toCProf) + 
       ((size_t)(index) * 
           (size_t)(SIZE_OF_SURF_C_PROFILE_TAB(nrCPElements))));
 return(ret);        
}


SurfCProfileTpeTable* getSurfCProfileTpeTable(SurfCProfileTpeTable* 
                                toCProfTpe,short nrCPElements,long index)
{
 SurfCProfileTpeTable* ret;
 ret = (SurfCProfileTpeTable*)((char*)(toCProfTpe) + 
       ((size_t)(index) * 
           (size_t)(SIZE_OF_SURF_C_PROFILE_TPE_TAB(nrCPElements))));
 return(ret);        
}


