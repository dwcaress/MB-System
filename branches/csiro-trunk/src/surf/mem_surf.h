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
              
              u_long                         indexCenterPosition   ;
              u_long                         indexMultiBeam        ;
              u_long                         indexAmplitudes       ;
              u_long                         nrCenterPosition      ;
              u_long                         nrBeam                ;
              u_long                         nrAmplitudes          ;
              u_long                         nrSsData              ;
              u_long                         nrRxParams            ;
              u_long                         nrTxParams            ;
              u_long                         nrOfSndgAttachedData  ;
              u_long                         nrOfBeamAttachedData  ;
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
              u_long                         nrStatistics        ;
              SurfPositionSensorArray*       toPosiSensors       ;
              u_long                         nrPosiSensors       ;
              SurfMultiBeamAngleTable*       toAngleTables       ;
              u_long                         nrAngleTables       ;
              u_long                         nrBeams             ;
              SurfTransducerParameterTable*  toTransducers       ;
              u_long                         nrTransducers       ;
              SurfCProfileTable*             toCProfiles         ;
              u_long                         nrCProfiles         ;
              u_long                         nrCPElements        ;
              SurfCProfileTpeTable*          toCProfileTpes      ;
              u_long                         nrCprofTpes         ;
              SurfPolygons*                  toPolygons          ;
              u_long                         nrPolyElements      ;
              SurfEvents*                    toEvents            ;
              u_long                         nrEvents            ;
              SurfFreeText*                  toFreeText          ;
              u_long                         nrFreeTextUnits     ;
              SurfAddStatistics*             toAddStatistics     ;
              u_long                         nrAddStatistics     ;
              SurfTpeStatics*                toTpeStatics        ;
              u_long                         nrTpeStatics        ;
              SurfFreeSixDataDescr*          toFreeSixDataDescr  ;
              u_long                         nrOfSixAttachedData ;
              SurfFreeSndgDataDescr*         toFreeSndgDataDescr ;
              u_long                         nrOfSndgAttachedData;
              SurfFreeBeamDataDescr*         toFreeBeamDataDescr ;
              u_long                         nrOfBeamAttachedData;
              SurfFreeSixAttachedData*       toFreeSixData       ;
              SurfVendorText*                toVendorText        ;
              u_long                         nrOfVendorText      ;

              SurfSdaThread*                 toSdaThread         ;
              u_long                         activeThreadIndex   ;
              SdaInfo*                       toSdaInfo           ;
              /* Data of SDA-blocks */
              u_long                         nrOfSoundings       ;
              u_long                         nrOfCenterPositions ;
              u_long                         nrOfCeps            ;
              u_long                         nrOfSingleBeamDepth ;
              u_long                         nrOfMultiBeamDepth  ;
              u_long                         nrOfMultiBeamTT     ;
              u_long                         nrOfMultiBeamRec    ;
              u_long                         nrOfSignalParams    ;
              u_long                         nrOfTxParams        ;
              u_long                         nrOfRxSets          ;
              u_long                         nrOfTxSets          ;
              u_long                         nrOfSignalAmplitudes;
              u_long                         nrOfSsData          ;
              u_long                         maxNrSsData         ;
              u_long                         nrOfAmplitudes      ;
              u_long                         nrOfExtAmplitudes   ;
              u_long                         nrOfMultiTPEs       ;
              u_long                         nrOfSingleTPEs      ;
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
