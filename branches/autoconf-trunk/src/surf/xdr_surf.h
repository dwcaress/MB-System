/*-----------------------------------------------------------------------
/ H E A D E R K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : xdr_surf.h    Version 3.0
/  ERSTELLUNGSDATUM : 28.07.93
/ ----------------------------------------------------------------------*/
/*!
/ ------------------------------------------------------------------------
/ COPYRIGHT (C) 1993: ATLAS ELEKTRONIK GMBH, 28305 BREMEN
/ ------------------------------------------------------------------------
/
/
/  See README file for copying and redistribution conditions.
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
/ BESCHREIBUNG:   Definitions describing xdr_surf-functions V2.0
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   K O P F E S
/ ------------------------------------------------------------------------
*/
/************************************************************************/

#ifndef _xdr_surf_h
#define _xdr_surf_h

#include "surf.h"

/* Return-values of XDR-SURF-routines */

typedef short XdrSurf;
 
#define SURF_FAILURE                    0
#define SURF_SUCCESS                    1
#define SURF_WRONG_VERSION              2
#define SURF_CORRUPTED_DATASET          3
#define SURF_NR_OF_TABLE_ELEMENTS_ZERO  4
#define SURF_CANT_GET_MEMORY            5
#define SURF_CANT_OPEN_FILE             6
#define SURF_EOF                        7
#define SURF_BAD_POSITION               8

      
#ifdef _XDR_SURF 

short getSurfVersion(char* version);

FILE* xdrSurfOpenWrite(XDR* xdrs,const char* filename);
FILE* xdrSurfOpenRead(XDR* xdrs,const char* filename);

XdrSurf xdr_SurfDescriptor(XDR *xdrs,SurfDescriptor *gp,
                                  short* newVersion,short* oldVersion);
XdrSurf xdr_SurfGlobalData(XDR* xdrs,SurfGlobalData *gp);
XdrSurf xdr_SurfStatistics(XDR* xdrs,SurfStatistics *gp);
XdrSurf xdr_PositionSensorArray(XDR* xdrs,SurfPositionSensorArray *gp,
                                                              short oldVers);
XdrSurf xdr_SurfMultiBeamAngleTable(XDR* xdrs,
                                           SurfMultiBeamAngleTable *gp,
                                           u_short maxBeamNr);
XdrSurf xdr_SurfTransducerParameterTable(XDR* xdrs,
                                           SurfTransducerParameterTable *gp);
XdrSurf xdr_SurfCProfileTableTpes(XDR *xdrs,
                                           SurfCProfileTpeTable *gp,
                                           u_short maxNrOfElementsPerTable);
XdrSurf xdr_SurfCProfileTable(XDR* xdrs,
                                           SurfCProfileTable *gp,
                                           u_short maxNrOfElementsPerTable);
XdrSurf xdr_SurfPolygons(XDR* xdrs,
                                           SurfPolygons *gp,
                                           u_short maxNrOfElementsPerTable);
XdrSurf xdr_SurfEvents  (XDR* xdrs,
                                           SurfEvents   *gp,
                                           u_short maxNrOfElementsPerTable);
XdrSurf xdr_SurfFreeText(XDR* xdrs,
                                           SurfFreeText *gp,
                                           u_short maxNrOfElementsPerTable);
XdrSurf xdr_SurfTpeStatics(XDR *xdrs,SurfTpeStatics *gp);
XdrSurf xdr_SurfAddStatistics(XDR *xdrs,SurfAddStatistics *gp);
XdrSurf xdr_SurfVendorText(XDR *xdrs,SurfVendorText *gp);
XdrSurf xdr_SurfFreeSixDataDescr(XDR *xdrs,SurfFreeSixDataDescr *gp);
XdrSurf xdr_SurfFreeSndgDataDescr(XDR *xdrs,SurfFreeSndgDataDescr *gp);
XdrSurf xdr_SurfFreeBeamDataDescr(XDR *xdrs,SurfFreeBeamDataDescr *gp);
XdrSurf xdr_SurfFreeSixAttachedData(XDR *xdrs,SurfFreeSixAttachedData *gp);


XdrSurf xdr_SurfSoundingData(XDR* xdrs,SurfSoundingData *gp,short versLess2);
XdrSurf xdr_SurfFreeSoundingAttachedData(XDR *xdrs,SurfFreeSoundingAttachedData *gp);
XdrSurf xdr_SurfCenterPosition(XDR* xdrs,SurfCenterPosition *gp);
XdrSurf xdr_SurfPositionCepData(XDR *xdrs,SurfPositionCepData *gp);
XdrSurf xdr_SurfSingleBeamDepth(XDR* xdrs,SurfSingleBeamDepth *gp);
XdrSurf xdr_SurfMultiBeamDepth(XDR* xdrs,SurfMultiBeamDepth *gp);
XdrSurf xdr_SurfMultiBeamTT(XDR* xdrs,SurfMultiBeamTT *gp);
XdrSurf xdr_SurfMultiBeamReceive(XDR* xdrs,SurfMultiBeamReceive *gp);
XdrSurf xdr_SurfFreeBeamAttachedData(XDR *xdrs,SurfFreeBeamAttachedData *gp);
XdrSurf xdr_SurfTpeValues(XDR *xdrs,SurfTpeValues *gp);
XdrSurf xdr_SurfSignalParameter(XDR *xdrs,SurfSignalParameter *gp,short nrSets);
XdrSurf xdr_SurfTxParameter(XDR *xdrs,SurfTxParameter *gp,short nrSets);
XdrSurf xdr_SurfSignalAmplitudes(XDR* xdrs,SurfSignalAmplitudes *gp,
                                           u_short nrAmplitudes);
                                          
XdrSurf xdr_SurfAmplitudes(XDR *xdrs,SurfAmplitudes *gp);
XdrSurf xdr_SurfExtendedAmplitudes(XDR *xdrs,SurfExtendedAmplitudes *gp);
XdrSurf xdr_SurfSidescanData(XDR *xdrs,SurfSidescanData *gp,
                                                   u_short nrSsData);


#else

extern short getSurfVersion(char* version);

extern FILE* xdrSurfOpenWrite(XDR* xdrs,const char* filename);
extern FILE* xdrSurfOpenRead(XDR* xdrs,const char* filename);

extern XdrSurf xdr_SurfDescriptor(XDR *xdrs,SurfDescriptor *gp,
                                         short* newVersion,short* oldVersion);
extern XdrSurf xdr_SurfGlobalData(XDR* xdrs,SurfGlobalData *gp);
extern XdrSurf xdr_SurfStatistics(XDR* xdrs,SurfStatistics *gp);
extern XdrSurf xdr_PositionSensorArray(XDR* xdrs,SurfPositionSensorArray *gp,
                                                              short oldVers);
extern XdrSurf xdr_SurfMultiBeamAngleTable(XDR* xdrs,
                                           SurfMultiBeamAngleTable *gp,
                                           u_short maxBeamNr);
extern XdrSurf xdr_SurfTransducerParameterTable(XDR* xdrs,
                                           SurfTransducerParameterTable *gp);
extern XdrSurf xdr_SurfCProfileTableTpes(XDR *xdrs,
                                           SurfCProfileTpeTable *gp,
                                           u_short maxNrOfElementsPerTable);
extern XdrSurf xdr_SurfCProfileTable(XDR* xdrs,
                                           SurfCProfileTable *gp,
                                           u_short maxNrOfElementsPerTable);
extern XdrSurf xdr_SurfPolygons(XDR* xdrs,
                                           SurfPolygons *gp,
                                           u_short maxNrOfElementsPerTable);
extern XdrSurf xdr_SurfEvents  (XDR* xdrs,
                                           SurfEvents   *gp,
                                           u_short maxNrOfElementsPerTable);
extern XdrSurf xdr_SurfFreeText(XDR* xdrs,
                                           SurfFreeText *gp,
                                           u_short maxNrOfElementsPerTable);
extern XdrSurf xdr_SurfTpeStatics(XDR *xdrs,SurfTpeStatics *gp);
extern XdrSurf xdr_SurfAddStatistics(XDR *xdrs,SurfAddStatistics *gp);
extern XdrSurf xdr_SurfVendorText(XDR *xdrs,SurfVendorText *gp);
extern XdrSurf xdr_SurfFreeSixDataDescr(XDR *xdrs,SurfFreeSixDataDescr *gp);
extern XdrSurf xdr_SurfFreeSndgDataDescr(XDR *xdrs,SurfFreeSndgDataDescr *gp);
extern XdrSurf xdr_SurfFreeBeamDataDescr(XDR *xdrs,SurfFreeBeamDataDescr *gp);
extern XdrSurf xdr_SurfFreeSixAttachedData(XDR *xdrs,SurfFreeSixAttachedData *gp);


extern XdrSurf xdr_SurfSoundingData(XDR* xdrs,SurfSoundingData *gp,short versLess2);
extern XdrSurf xdr_SurfFreeSoundingAttachedData(XDR *xdrs,SurfFreeSoundingAttachedData *gp);
extern XdrSurf xdr_SurfCenterPosition(XDR* xdrs,SurfCenterPosition *gp);
extern XdrSurf xdr_SurfPositionCepData(XDR *xdrs,SurfPositionCepData *gp);
extern XdrSurf xdr_SurfSingleBeamDepth(XDR* xdrs,SurfSingleBeamDepth *gp);
extern XdrSurf xdr_SurfMultiBeamDepth(XDR* xdrs,SurfMultiBeamDepth *gp);
extern XdrSurf xdr_SurfMultiBeamTT(XDR* xdrs,SurfMultiBeamTT *gp);
extern XdrSurf xdr_SurfMultiBeamReceive(XDR* xdrs,SurfMultiBeamReceive *gp);
extern XdrSurf xdr_SurfFreeBeamAttachedData(XDR *xdrs,SurfFreeBeamAttachedData *gp);
extern XdrSurf xdr_SurfTpeValues(XDR *xdrs,SurfTpeValues *gp);
extern XdrSurf xdr_SurfSignalParameter(XDR *xdrs,SurfSignalParameter *gp,
                                                              short nrSets);
extern XdrSurf xdr_SurfTxParameter(XDR *xdrs,SurfTxParameter *gp,
                                                              short nrSets);
extern XdrSurf xdr_SurfSignalAmplitudes(XDR* xdrs,SurfSignalAmplitudes *gp,
                                                      u_short nrAmplitudes);
                                          
extern XdrSurf xdr_SurfAmplitudes(XDR *xdrs,SurfAmplitudes *gp);
extern XdrSurf xdr_SurfExtendedAmplitudes(XDR *xdrs,SurfExtendedAmplitudes *gp);
extern XdrSurf xdr_SurfSidescanData(XDR *xdrs,SurfSidescanData *gp,
                                                              u_short nrSsData);
                                          

#endif /* ifdef _XDR_SURF */


#endif
