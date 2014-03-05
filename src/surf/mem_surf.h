/*-----------------------------------------------------------------------
/ H E A D E R K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : mem_surf.h    Version 3.0
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



/ SPRACHE          : UNIX-C
/ COMPILER         : Silicon Graphix
/ BETRIEBSSYSTEM   : IRIX
/ HARDWARE-UMGEBUNG: SGI Crimson
/ URSPRUNGSHINWEIS :
/
/ ------------------------------------------------------------------------
/ BESCHREIBUNG:  Definitions describing SURF-functions for memory-admin.
/                V2.0
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   K O P F E S
/ ------------------------------------------------------------------------

*************************************************************************/

#ifndef _mem_surf_h_
#define _mem_surf_h_


enum _SurfThreadFlag {
                        OLD_BLOCK       ,
                        INSERTED_BLOCK
                     };
typedef enum _SurfThreadFlag SurfThreadFlag;




typedef struct
            {
              SurfSoundingData*              sounding       ;
              SurfSoundingData*              saveSounding   ;
              SurfThreadFlag                 flag           ;
            } SurfSdaThreadElement;



typedef struct
            {
              SurfSdaThreadElement           thread[1]      ;
            } SurfSdaThread;




typedef struct
            {
              SurfSoundingData*              toSoundings           ;
              SurfFreeSoundingAttachedData*  toFreeSoundingAttachedData;
              SurfCenterPosition*            toCenterPositions     ;
              SurfCenterPosition*            toActCenterPosition   ;
              SurfPositionCepData*           toPositionCepData     ;
              SurfSingleBeamDepth*           toSingleBeamDepth     ;
              SurfTpeValues*                 toSingleBeamTpeValues ;
              SurfMultiBeamDepth*            toMultiBeamDepth      ;
              SurfMultiBeamDepth*            toActMultiBeamDepth   ;
              SurfMultiBeamTT*               toMultiBeamTT         ;
              SurfMultiBeamTT*               toActMultiBeamTT      ;
              SurfMultiBeamReceive*          toMultiBeamRec        ;
              SurfMultiBeamReceive*          toActMultiBeamRec     ;
              SurfTpeValues*                 toMultiBeamTpeValues  ;
              SurfFreeBeamAttachedData*      toFreeBeamAttachedData;
              SurfSignalParameter*           toSignalParams        ;
              SurfTxParameter*               toTxParams            ;
              SurfSignalAmplitudes*          toSignalAmplitudes    ;
              SurfSignalAmplitudes*          toActSignalAmplitudes ;
              SurfAmplitudes*                toAmplitudes          ;
              SurfExtendedAmplitudes*        toExtendedAmpl        ;
              SurfSidescanData*              toSsData              ;

              SURF_U_LONG                    indexCenterPosition   ;
              SURF_U_LONG                    indexMultiBeam        ;
              SURF_U_LONG                    indexAmplitudes       ;
              SURF_U_LONG                    nrCenterPosition      ;
              SURF_U_LONG                    nrBeam                ;
              SURF_U_LONG                    nrAmplitudes          ;
              SURF_U_LONG                    nrSsData              ;
              SURF_U_LONG                    nrRxParams            ;
              SURF_U_LONG                    nrTxParams            ;
              SURF_U_LONG                    nrOfSndgAttachedData  ;
              SURF_U_LONG                    nrOfBeamAttachedData  ;
              size_t                         soundingS             ;
              size_t                         sndgAttDataS          ;
              size_t                         centerPS              ;
              size_t                         positionCepDataS      ;
              size_t                         singleBDS             ;
              size_t                         singleTPEsS           ;
              size_t                         multiBDS              ;
              size_t                         multiBTTS             ;
              size_t                         multiBRS              ;
              size_t                         multiTPEsS            ;
              size_t                         beamAttDataS          ;
              size_t                         signalPS              ;
              size_t                         signalTxPS            ;
              size_t                         signalAS              ;
              size_t                         amplS                 ;
              size_t                         extAmplS              ;
              size_t                         ssDataS               ;
              size_t                         allS                  ;
            } SdaInfo;




typedef struct
            {
              FILE*                          fp                  ;
              XDR*                           xdrs                ;

              SurfDescriptor*                toDescriptor        ;
              SurfGlobalData*                toGlobalData        ;
              SurfStatistics*                toStatistics        ;
              SURF_U_LONG                         nrStatistics        ;
              SurfPositionSensorArray*       toPosiSensors       ;
              SURF_U_LONG                         nrPosiSensors       ;
              SurfMultiBeamAngleTable*       toAngleTables       ;
              SURF_U_LONG                         nrAngleTables       ;
              SURF_U_LONG                         nrBeams             ;
              SurfTransducerParameterTable*  toTransducers       ;
              SURF_U_LONG                         nrTransducers       ;
              SurfCProfileTable*             toCProfiles         ;
              SURF_U_LONG                         nrCProfiles         ;
              SURF_U_LONG                         nrCPElements        ;
              SurfCProfileTpeTable*          toCProfileTpes      ;
              SURF_U_LONG                         nrCprofTpes         ;
              SurfPolygons*                  toPolygons          ;
              SURF_U_LONG                         nrPolyElements      ;
              SurfEvents*                    toEvents            ;
              SURF_U_LONG                         nrEvents            ;
              SurfFreeText*                  toFreeText          ;
              SURF_U_LONG                         nrFreeTextUnits     ;
              SurfAddStatistics*             toAddStatistics     ;
              SURF_U_LONG                         nrAddStatistics     ;
              SurfTpeStatics*                toTpeStatics        ;
              SURF_U_LONG                         nrTpeStatics        ;
              SurfFreeSixDataDescr*          toFreeSixDataDescr  ;
              SURF_U_LONG                         nrOfSixAttachedData ;
              SurfFreeSndgDataDescr*         toFreeSndgDataDescr ;
              SURF_U_LONG                         nrOfSndgAttachedData;
              SurfFreeBeamDataDescr*         toFreeBeamDataDescr ;
              SURF_U_LONG                         nrOfBeamAttachedData;
              SurfFreeSixAttachedData*       toFreeSixData       ;
              SurfVendorText*                toVendorText        ;
              SURF_U_LONG                         nrOfVendorText      ;

              SurfSdaThread*                 toSdaThread         ;
              SURF_U_LONG                         activeThreadIndex   ;
              SdaInfo*                       toSdaInfo           ;
              /* Data of SDA-blocks */
              SURF_U_LONG                         nrOfSoundings       ;
              SURF_U_LONG                         nrOfCenterPositions ;
              SURF_U_LONG                         nrOfCeps            ;
              SURF_U_LONG                         nrOfSingleBeamDepth ;
              SURF_U_LONG                         nrOfMultiBeamDepth  ;
              SURF_U_LONG                         nrOfMultiBeamTT     ;
              SURF_U_LONG                         nrOfMultiBeamRec    ;
              SURF_U_LONG                         nrOfSignalParams    ;
              SURF_U_LONG                         nrOfTxParams        ;
              SURF_U_LONG                         nrOfRxSets          ;
              SURF_U_LONG                         nrOfTxSets          ;
              SURF_U_LONG                         nrOfSignalAmplitudes;
              SURF_U_LONG                         nrOfSsData          ;
              SURF_U_LONG                         maxNrSsData         ;
              SURF_U_LONG                         nrOfAmplitudes      ;
              SURF_U_LONG                         nrOfExtAmplitudes   ;
              SURF_U_LONG                         nrOfMultiTPEs       ;
              SURF_U_LONG                         nrOfSingleTPEs      ;
              short                          sourceVersionLess2  ;
            } SurfDataInfo;






#ifdef _MEM_SURF



XdrSurf mem_ReadSixStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);
XdrSurf mem_WriteSixStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);
XdrSurf mem_ReadSdaStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);
XdrSurf mem_WriteSdaStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);
XdrSurf mem_destroyAWholeSurfStructure(SurfDataInfo* toSurfDataInfo);

XdrSurf mem_buildSurfSdaStructure(SurfDataInfo* toSurfDataInfo);


XdrSurf freeSixBlocks(SurfDataInfo* toSurfDataInfo,XdrSurf returnvalue);
XdrSurf checkAndLoadSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                      SurfDataInfo* toSurfDataInfo);
XdrSurf checkAndUpdateSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                      SurfDataInfo* toSurfDataInfo);
size_t initializeSdaInfo(SurfDataInfo* toSurfDataInfo,SdaInfo* toSdaInfo);
void setPointersInSdaInfo(void* toSdaBlock,SdaInfo* toSdaInfo);
void free_SdaMemory(SurfDataInfo* toSurfDataInfo);

SurfMultiBeamAngleTable* getSurfAngleTable(SurfMultiBeamAngleTable*
                                toAngleTable,short nrBeams,long index);
SurfCProfileTable* getSurfCProfileTable(SurfCProfileTable*
                                toCProf,short nrCPElements,long index);
SurfCProfileTpeTable* getSurfCProfileTpeTable(SurfCProfileTpeTable*
                                toCProfTpe,short nrCPElements,long index);
#else

/********************************************/
/*                                          */
/*        general LIBRARY-functions         */
/*                                          */
/********************************************/



extern XdrSurf mem_ReadSixStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);

/* This function doesn't destroy the internal data-structures */

extern XdrSurf mem_WriteSixStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);

extern XdrSurf mem_ReadSdaStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);

/* This function doesn't destroy the internal data-structures */

extern XdrSurf mem_WriteSdaStructure(char* filename,
                                    SurfDataInfo* toSurfDataInfo);

extern XdrSurf mem_destroyAWholeSurfStructure(SurfDataInfo* toSurfDataInfo);

extern XdrSurf mem_buildSurfSdaStructure(SurfDataInfo* toSurfDataInfo);


/*****************************************************/
/*                                                   */
/* some internal functios,that might be useful       */
/* useful (to use them , it is necessary to under-   */
/*         stand the internal architecture)          */
/*                                                   */
/*****************************************************/



extern XdrSurf freeSixBlocks(SurfDataInfo* toSurfDataInfo,XdrSurf returnvalue);

extern XdrSurf checkAndLoadSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                          SurfDataInfo* toSurfDataInfo);
extern XdrSurf checkAndUpdateSurfDescriptor(SurfDescriptor* toSurfDescriptor,
                                            SurfDataInfo* toSurfDataInfo);
extern size_t initializeSdaInfo(SurfDataInfo* toSurfDataInfo,
                                SdaInfo* toSdaInfo);
extern void setPointersInSdaInfo(void* toSdaBlock,
                                 SdaInfo* toSdaInfo);
extern void free_SdaMemory(SurfDataInfo* toSurfDataInfo);

extern SurfMultiBeamAngleTable* getSurfAngleTable(SurfMultiBeamAngleTable*
                                toAngleTable,short nrBeams,long index);
extern SurfCProfileTable* getSurfCProfileTable(SurfCProfileTable*
                                toCProf,short nrCPElements,long index);
extern SurfCProfileTpeTable* getSurfCProfileTpeTable(SurfCProfileTpeTable*
                                toCProfTpe,short nrCPElements,long index);

#endif


#endif
