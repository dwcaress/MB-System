/*-----------------------------------------------------------------------
/ H E A D E R K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : util_surf.h
/  ERSTELLUNGSDATUM : 09.08.93
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
/ BESCHREIBUNG:
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   K O P F E S
/ ------------------------------------------------------------------------
*/
/************************************************************************/

#ifndef _util_surf_h
#define _util_surf_h

#include <time.h>        


/* Return-Vals of moveInSdaThread */

enum _MoveInSdaThread {
                        STEP_DONE     ,
                        END_OF_THREAD  
                      }; 
typedef enum _MoveInSdaThread MoveInSdaThread;




/* operating-mode of stepInSdaThread */

enum _ModeMoveInSdaThread {
                            FORE_ONE_STEP       ,
                            BACK_ONE_STEP       ,
                            FORE_X_STEPS        ,
                            BACK_X_STEPS        ,
                            ABS_POSITION        ,
                            HALF_WAY_ABS        ,
                            BACK_HALF_WAY_REL   ,
                            FORE_HALF_WAY_REL   ,
                            TO_START            ,
                            TO_END               
                          }; 
typedef enum _ModeMoveInSdaThread ModeMoveInSdaThread;



/* operating-mode of surf_insertNewSdaBlockAtActualPosition */

enum _SDAinsertMode {
                      INSERT_AFTER_ACT_POS,
                      INSERT_BEFOR_ACT_POS 
                    }; 
typedef enum _SDAinsertMode SDAinsertMode;




/************************************************************
*                                                           *
*  relative Zeiten werden im Format 'SurfTime' in           *
*          Sekunden dargestellt                             *
*                                                           *
************************************************************/


typedef double SurfTime;

typedef struct {
                char date[10];
                char time[10];
               }SurfTimeDate;
               
typedef struct {
                struct tm tmTime;         /* see time.h */
                int    fractionalSeconds; /* 1/100 sec. */
               }SurfTm;



#ifdef _UTIL_SURF


MoveInSdaThread surf_moveInSdaThread(SurfDataInfo* toSurfDataInfo,
                                ModeMoveInSdaThread mode,
                                u_long        nrOfSteps );
                                        
XdrSurf surf_backupSdaBlock(SurfDataInfo* toSurfDataInfo); 

void surf_restoreSdaBlock(SurfDataInfo* toSurfDataInfo); 


XdrSurf surf_insertNewSdaBlockAtActualPosition(SurfDataInfo* toSurfDataInfo,
                                               SDAinsertMode where); 


/* Zeit-Funktionen */

void surf_timeSizetoTimeDate(char* timeSize,SurfTimeDate* timeDate);

void surf_timeSizetoSurfTm(char* timeSize,SurfTm* surfTm);

void surf_surfTmToTimeSize(char* timeSize,SurfTm* surfTm);

SurfTime surf_timeOfTheDayFromTimeSize (char* timeSize);

SurfTime surf_timeOfTheDayFromSurfTm (SurfTm* surfTm);

SurfTime surf_timeAbsoluteFromSurfTm (SurfTm* surfTm);

SurfTime surf_difftime (SurfTm* later,SurfTm* earlier);

void surf_putJulianDayIntoTm(SurfTm* surfTm);

long surf_timeSizeToInt(char* timeSize);



void surf_setVendorText(SurfDataInfo* toSurfData);


#else



extern MoveInSdaThread surf_moveInSdaThread(SurfDataInfo* toSurfDataInfo,
                                ModeMoveInSdaThread mode,
                                u_long        nrOfSteps );
                                        
extern XdrSurf surf_backupSdaBlock(SurfDataInfo* toSurfDataInfo); 

extern void surf_restoreSdaBlock(SurfDataInfo* toSurfDataInfo); 


extern XdrSurf surf_insertNewSdaBlockAtActualPosition(
                                 SurfDataInfo* toSurfDataInfo,
                                 SDAinsertMode where); 




/* Zeit-Funktionen */

extern void surf_timeSizetoTimeDate(char* timeSize,SurfTimeDate* timeDate);

extern void surf_timeSizetoSurfTm(char* timeSize,SurfTm* surfTm);

extern void surf_surfTmToTimeSize(char* timeSize,SurfTm* surfTm);

extern SurfTime surf_timeOfTheDayFromTimeSize (char* timeSize);

extern SurfTime surf_timeOfTheDayFromSurfTm (SurfTm* surfTm);

extern SurfTime surf_timeAbsoluteFromSurfTm (SurfTm* surfTm);

extern SurfTime surf_difftime (SurfTm* later,SurfTm* earlier);

extern void surf_putJulianDayIntoTm(SurfTm* surfTm);

extern long surf_timeSizeToInt(char* timeSize);



extern void surf_setVendorText(SurfDataInfo* toSurfData);

#endif


#endif


/*********************************************************************
*         H I S T O R I E
**********************************************************************
*  Edition  History
*   date    comments                                            by
* --------  ------------------------------------------------ ---------
* 09-08-93  created                                            Block
* 21-10-93  new: surf_timeSizeToInt                            Block
*********************************************************************/
