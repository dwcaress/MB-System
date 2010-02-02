/*-----------------------------------------------------------------------
/ P R O G R A M M K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : util_surf.c
/  ERSTELLUNGSDATUM : 13.08.93
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
/    Utility-LIBRARY-Functions for SURF-presentation V2.0
/
/ ------------------------------------------------------------------------
/ NAME, STRUKTUR UND KURZBESCHREIBUNG DER EINGABEPARAMETER:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h & util_surf.h
/
/ ------------------------------------------------------------------------
/ NAME, STRUKTUR UND KURZBESCHREIBUNG DER AUSGABEPARAMETER:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h & util_surf.h
/
/ ------------------------------------------------------------------------
/ VERHALTEN IM FEHLERFALL:
/ ------------------------------------------------------------------------
/
/    see mem_surf.h & util_surf.h
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


#define _UTIL_SURF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"


/* ***********************************************************************
*                                                                        *
*  ENDE DES DEKLARATIONSTEILS                                            *
*                                                                        *
*********************************************************************** */

static char sccsid[50] = {"@(#)libsurf.a  Version 3.1 15.12.1998"};


/* There are C++ - Compilers, which omit unreferenced statics */
char* forCCsurf(void)
{
 return(sccsid);
}





/************************************************************
*************************************************************
*                                                           *
*  Funktionen zur Manipulationen im SDA-Thread              *
*                                                           *
*                                                           *
*************************************************************
************************************************************/


/************************************************************
*                                                           *
*  Setzen des Threadindex an eine 'mode' entsprechende      *
*  Stelle im SDA-Thread und update des Pointerarrays        *
*  auf diese Thread-Position                                *
*                                                           *
************************************************************/


MoveInSdaThread surf_moveInSdaThread(SurfDataInfo* toSurfDataInfo,
                                ModeMoveInSdaThread mode,
                                u_long        nrOfSteps )
                           
{                          
 SurfSdaThread* toThread;
 u_long index,ii;

 index = toSurfDataInfo->activeThreadIndex;   
 ii = toSurfDataInfo->nrOfSoundings - 1;
 switch(mode)
 {
   case BACK_ONE_STEP :
           if(index == 0) 
           {
             return(END_OF_THREAD);
           }
           index --;
           break;
           
   case FORE_ONE_STEP :
           if(index >= ii)
           {
             return(END_OF_THREAD);
           }
           index ++;
           break; 
           
   case BACK_X_STEPS:
           if(index == 0) 
           {
             return(END_OF_THREAD);
           }
           if(index < nrOfSteps)
           {
             index = 0;
           }
           else
           {
             index = index - nrOfSteps;
           }
           break;
           
   case FORE_X_STEPS:
           if(index >= ii)
           {
             return(END_OF_THREAD);
           }
           if(((long)(ii-index)) < (long)nrOfSteps)
           {
             index = ii;
           }
           else
           {
             index = index + nrOfSteps;
           }
           break;

   case ABS_POSITION     :
           if(nrOfSteps > ii)
           {
             index = ii;
           }
           else
           {
             index = nrOfSteps;
           }
           break;
           
   case HALF_WAY_ABS     :
           index = ii/2;
           break;
           
   case BACK_HALF_WAY_REL:
           index = index /2;
           break;
           
   case FORE_HALF_WAY_REL:
           index = index + ((ii-index)/2);
           break;
           
   case TO_START:
           index = 0;
           break;
           
   case TO_END:   
           index = ii; 
           break;
           
   default:
           return(END_OF_THREAD);
 }
 
 toSurfDataInfo->activeThreadIndex = index;   
 toThread = toSurfDataInfo->toSdaThread;
 setPointersInSdaInfo(toThread->thread[index].sounding,
                      toSurfDataInfo->toSdaInfo);
 return(STEP_DONE);
}





/************************************************************
*                                                           *
*  Retten eines SDA-Blocks bevor Daten manipuliert werden   *
*                                                           *
*                                                           *
************************************************************/


XdrSurf surf_backupSdaBlock(SurfDataInfo* toSurfDataInfo) 
{                          
 SurfSdaThread* toThread;
 SdaInfo* toSdaInfo;
 SurfSoundingData* toSdaBlock;
 SurfSoundingData* toSaveSdaBlock;
 u_long index;

 index = toSurfDataInfo->activeThreadIndex;   
 toThread  = toSurfDataInfo->toSdaThread;
 toSdaInfo = toSurfDataInfo->toSdaInfo;
 toSdaBlock = toThread->thread[index].sounding;
 toSaveSdaBlock = toThread->thread[index].saveSounding;

 if(toSaveSdaBlock == NULL)
 {
  toSaveSdaBlock = (SurfSoundingData*)calloc(1,toSdaInfo->allS);
  if(toSaveSdaBlock == NULL)
  {
   return(SURF_CANT_GET_MEMORY);
  }
  memcpy(toSaveSdaBlock,toSdaBlock,toSdaInfo->allS);
  toThread->thread[index].saveSounding = toSaveSdaBlock;
 }
 
 return(SURF_SUCCESS);
}





/************************************************************
*                                                           *
*  Verwerfen der Manipulation eines SDA-Blocks              *
*                                                           *
*                                                           *
************************************************************/


void surf_restoreSdaBlock(SurfDataInfo* toSurfDataInfo) 
{                          
 SurfSdaThread* toThread;
 SdaInfo*  toSdaInfo;
 SurfSoundingData* toSdaBlock;
 SurfSoundingData* toSaveSdaBlock;
 u_long index;

 index = toSurfDataInfo->activeThreadIndex;   
 toThread  = toSurfDataInfo->toSdaThread;
 toSdaInfo = toSurfDataInfo->toSdaInfo;
 toSdaBlock = toThread->thread[index].sounding;
 toSaveSdaBlock = toThread->thread[index].saveSounding;

 if(toSaveSdaBlock != NULL)
 {
  toThread->thread[index].sounding = toSaveSdaBlock;
  toThread->thread[index].saveSounding = NULL;
  free(toSdaBlock);
 }
}





/************************************************************
*                                                           *
*  Einfuegen eines neuen SDA-Blocks (je nach 'where' vor    *
*    oder hinter die aktuelle Position) und fuellen des     *
*    neuen Datenblocks mit den Daten an der akt. Position.  *
*  Die akt. Position steht anscliessend auf dem neuen Block *
*                                                           *
************************************************************/


XdrSurf surf_insertNewSdaBlockAtActualPosition(SurfDataInfo* toSurfDataInfo,
                                               SDAinsertMode where) 
{                          
 SurfSdaThread* toThread;
 SurfSdaThread* toNewThread;
 SdaInfo* toSdaInfo;
 SurfSoundingData* toSdaBlock;
 SurfSoundingData* toNewSdaBlock;
 u_long index,newBlockIndex,oldLengthOfThread,newLengthOfThread,ii;

 index = toSurfDataInfo->activeThreadIndex;   
 toThread  = toSurfDataInfo->toSdaThread;
 toSdaInfo = toSurfDataInfo->toSdaInfo;
 toSdaBlock = toThread->thread[index].sounding;
 oldLengthOfThread = toSurfDataInfo->nrOfSoundings;
 newLengthOfThread = oldLengthOfThread + 1;

 /* allocate the necessary memory */
 
 toNewSdaBlock = (SurfSoundingData*)calloc(1,toSdaInfo->allS);
 if(toNewSdaBlock == NULL)
 {
  return(SURF_CANT_GET_MEMORY);
 }
 toNewThread = (SurfSdaThread*)
                   calloc((u_int)newLengthOfThread,sizeof(SurfSdaThreadElement));
 if(toNewThread == NULL)
 {
  free(toNewSdaBlock);
  return(SURF_CANT_GET_MEMORY);
 }

 /* fill the new Block with Data */
 
 memcpy(toNewSdaBlock,toSdaBlock,toSdaInfo->allS);

 /* fill the thread - structure */
 
 for(ii=0;ii<=index;ii++)
 {
  memcpy(&(toNewThread->thread[ii].sounding),
         &(toThread->thread[ii].sounding),sizeof(SurfSdaThreadElement));
 }
 for(ii=index;ii<oldLengthOfThread;ii++)
 {
  memcpy(&(toNewThread->thread[ii+1].sounding),
         &(toThread->thread[ii].sounding),sizeof(SurfSdaThreadElement));
 }

 switch (where)
 {
  case INSERT_BEFOR_ACT_POS:
     newBlockIndex = index;
     index++;
     break;

  case INSERT_AFTER_ACT_POS:
     newBlockIndex = index+1;
     break;

  default:
     break;
 }
 
 toNewThread->thread[newBlockIndex].sounding = toNewSdaBlock;
 toNewThread->thread[newBlockIndex].saveSounding = NULL;
 toNewThread->thread[newBlockIndex].flag         = INSERTED_BLOCK;
 
 free(toThread);

 toSurfDataInfo->activeThreadIndex = newBlockIndex;   
 toSurfDataInfo->toSdaThread = toNewThread;
 toSurfDataInfo->nrOfSoundings = newLengthOfThread;
 
 return(SURF_SUCCESS);
}








/************************************************************
*************************************************************
*                                                           *
*  Funktionen zur Zeit-Darstellung in Surf                  *
*                                                           *
*                                                           *
*************************************************************
************************************************************/

/************************************************************
*                                                           *
*  erzeugt aus ASCII-Text in 'TIME_SIZE' Structure          *
*                fuer SurfTimeDate                          *
*                                                           *
************************************************************/



void surf_timeSizetoTimeDate(char* timeSize,SurfTimeDate* timeDate)
{
 u_short ii,jj;

 jj=0; 
 for(ii=0;ii<(u_short)6;ii++)
 {
  if((ii==2) || (ii==4))
  {
   timeDate->date[jj]='.';
   jj++;
  }
  timeDate->date[jj] = timeSize[ii];
  jj++;
 }
 timeDate->date[jj] = 0; /* cstring !! */

 jj=0; 
 for(ii=6;ii<(u_short)12;ii++)
 {
  if((ii==8) || (ii==10))
  {
   timeDate->time[jj] = ':';
   jj++;
  }
  timeDate->time[jj] = timeSize[ii];
  jj++;
 }
 timeDate->time[jj] = 0; /* cstring !! */
}



/************************************************************
*                                                           *
*  erzeugt aus ASCII-Text in 'TIME_SIZE' Structure          *
*      Darstellung in 'SurfTm'                              *
*                                                           *
************************************************************/


int twoDigitsToInt (char* timeSize,u_short where)
{
 return(((timeSize[where] - '0') * 10) + (timeSize[where+1] - '0'));
}


void surf_timeSizetoSurfTm(char* timeSize,SurfTm* surfTm)
{
 u_int year,month,day,switchCount,daySince1970;

 /* hour,minute,second,secfrac */
 
 surfTm->fractionalSeconds = twoDigitsToInt(timeSize,13);
 surfTm->tmTime.tm_sec     = twoDigitsToInt(timeSize,10);
 surfTm->tmTime.tm_min     = twoDigitsToInt(timeSize,8);
 surfTm->tmTime.tm_hour    = twoDigitsToInt(timeSize,6);

 /* day,month,year */

 day                       = twoDigitsToInt(timeSize,0);
 surfTm->tmTime.tm_mday    = day;
 month                     = twoDigitsToInt(timeSize,2);
 surfTm->tmTime.tm_mon     = month - 1;                      
 year                      = twoDigitsToInt(timeSize,4);
 if (year<70) year = year + 100;
 surfTm->tmTime.tm_year    = year;

 switch(month)
 {
  case 12:
          day=day+30;
  case 11:
          day=day+31;
  case 10:
          day=day+30;
  case  9:
          day=day+31;
  case  8:
          day=day+31;
  case  7:
          day=day+30;
  case  6:
          day=day+31;
  case  5:
          day=day+30;
  case  4:
          day=day+31;
  case  3:
          day=day+28;
  case  2:
          day=day+31;
  default:
          break;
 }

 switchCount = (year >> 2) - 17;     /* alle 4 jahre 1 Schalttag,
                                        17 Schalttage von 1900->1970 */
 daySince1970 = day + switchCount + ((year-70)*365);
 if((year%4) == 0)                   /* aktuelles jahr = Schaltjahr */
 {
  if(month > 2)
    day++;        /* Der aktuelle Schalttag wurde noch nicht bruecksichtigt */
  else
    daySince1970--; /* Ein Schalttag zuviel berechnet */ 
 }                                        

 surfTm->tmTime.tm_yday    = day -1;
 surfTm->tmTime.tm_wday    = (daySince1970 + 4)%7; /* der 1.1.1970 war ein
                                                      Donnerstag => 4 */
 surfTm->tmTime.tm_isdst   = 0;
}




/************************************************************
*                                                           *
*  erzeugt aus ASCII-Text in 'TIME_SIZE' Structure          *
*      Darstellung als "long" fuer HDB                      *
*                                                           *
************************************************************/


long surf_timeSizeToInt(char* timeSize)
{
 char buffer[20];

 strncpy(buffer,timeSize,10);
 buffer[6] = 0;
 return(atol(buffer));
}





/************************************************************
*                                                           *
*  ergaenzt eine Darstellung in 'SurfTm' um den Julian Day  *
*                                                           *
************************************************************/


void surf_putJulianDayIntoTm(SurfTm* surfTm)
{
 int year,month,day;

 /* day,month,year */

 day =  surfTm->tmTime.tm_mday-1;
 month = surfTm->tmTime.tm_mon + 1;                      
 year = surfTm->tmTime.tm_year;

 switch(month)
 {
  case 12:
          day=day+30;
  case 11:
          day=day+31;
  case 10:
          day=day+30;
  case  9:
          day=day+31;
  case  8:
          day=day+31;
  case  7:
          day=day+30;
  case  6:
          day=day+31;
  case  5:
          day=day+30;
  case  4:
          day=day+31;
  case  3:
          day=day+28;
  case  2:
          day=day+31;
  default:
          break;
 }


 if((year%4) == 0)                   /* aktuelles jahr = Schaltjahr */
 {
  if(month > 2)
    day++;        /* Der aktuelle Schalttag wurde noch nicht bruecksichtigt */
 }                                        
 
 surfTm->tmTime.tm_yday    = day;
}





/************************************************************
*                                                           *
*  erzeugt aus Darstellung in 'SurfTm'                      *
*              ASCII-Text in 'TIME_SIZE' Structure          *
*                                                           *
************************************************************/


void intToTwoDigitsInSurfTime (char* timeSize,u_short where,int what)
{
  timeSize[where]    = (char)(what/10) + '0';
  timeSize[where+1]  = (char)(what%10) + '0';
}


void surf_surfTmToTimeSize(char* timeSize,SurfTm* surfTm)
{
 int year;

 /* hour,minute,second,secfrac */
 
 intToTwoDigitsInSurfTime(timeSize,13,surfTm->fractionalSeconds);
 intToTwoDigitsInSurfTime(timeSize,10,surfTm->tmTime.tm_sec );
 intToTwoDigitsInSurfTime(timeSize, 8,surfTm->tmTime.tm_min );
 intToTwoDigitsInSurfTime(timeSize, 6,surfTm->tmTime.tm_hour);

 /* day,month,year */

 intToTwoDigitsInSurfTime(timeSize,0,surfTm->tmTime.tm_mday);
 intToTwoDigitsInSurfTime(timeSize,2,surfTm->tmTime.tm_mon + 1 );
 year = surfTm->tmTime.tm_year;
 if (year >= 100) year = year - 100;
 intToTwoDigitsInSurfTime(timeSize,4,year);
 timeSize[12] = '.';
 timeSize[15] = 0;
}




/************************************************************
*                                                           *
*  erzeugt aus ASCII-Text in 'TIME_SIZE' Structure          *
*      Tageszeit in Sekunden   (SurfTime - Format)          *
*                                                           *
************************************************************/



SurfTime surf_timeOfTheDayFromTimeSize (char* timeSize)
{
 SurfTime ret;
 int fracSec,sec,min,hour;
  
 /* hour,minute,second,secfrac */
 
 fracSec = twoDigitsToInt(timeSize,13);
 sec     = twoDigitsToInt(timeSize,10);
 min     = twoDigitsToInt(timeSize,8);
 hour    = twoDigitsToInt(timeSize,6);

 ret = ((SurfTime)(fracSec)  / 100.0)
     + ((SurfTime)(sec    )         )
     + ((SurfTime)(min    ) *   60.0)
     + ((SurfTime)(hour   ) * 3600.0);
  
 return((SurfTime) ret);
}





/************************************************************
*                                                           *
*  erzeugt aus 'SurfTm'                                     *
*      Tageszeit in Sekunden   (SurfTime - Format)          *
*                                                           *
************************************************************/



SurfTime surf_timeOfTheDayFromSurfTm (SurfTm* surfTm)
{
 SurfTime ret;
  
 ret = ((SurfTime)(surfTm->fractionalSeconds)  / 100.0)
     + ((SurfTime)(surfTm->tmTime.tm_sec    )         )
     + ((SurfTime)(surfTm->tmTime.tm_min    ) *   60.0)
     + ((SurfTime)(surfTm->tmTime.tm_hour   ) * 3600.0);
  
 return(ret);
}





/************************************************************
*                                                           *
*  erzeugt aus 'SurfTm'                                     *
*     absolute Zeit in Sekunden   (SurfTime - Format)       *
*                                                           *
************************************************************/



SurfTime surf_timeAbsoluteFromSurfTm (SurfTm* surfTm)
{
 SurfTime ret;
 int year,switchDays;

 surf_putJulianDayIntoTm(surfTm);
  
 ret = surf_timeOfTheDayFromSurfTm (surfTm);
 ret = ret + ((SurfTime)(surfTm->tmTime.tm_yday) * 24.0 * 3600.0 );

 year  = surfTm->tmTime.tm_year;
 switchDays = (year/4) - 17;
 if((year%4) == 0)
            switchDays--; 
 year  = year - 70 ;

 ret = ret + ((SurfTime)(switchDays)   * 24.0 * 3600.0 );
 ret = ret + ((SurfTime)(year) * 365.0 * 24.0 * 3600.0 );
 
 return((SurfTime) ret);
}



/************************************************************
*                                                           *
*  erzeugt die Differenz in Sekunden aus zwei Zeiten in     *
*     einer Darstellung in 'SurfTm'                         *
*     unter Beruecksichtigung der Sekunden-Bruchteile       *
*                                                           *
************************************************************/


SurfTime surf_difftime (SurfTm* later,SurfTm* earlier)
{
 SurfTime ret,laterTime,earlierTime;

 laterTime   = surf_timeAbsoluteFromSurfTm (later);
 earlierTime = surf_timeAbsoluteFromSurfTm (earlier);

 ret = laterTime - earlierTime;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;
 return((SurfTime) ret);
}





void surf_setVendorText(SurfDataInfo* toSurfData)
{
#ifdef _WIN32
 static SurfVendorText vendorText =
 {{"This SURF-Dataset is generated by STN-Atlas utilities !" }};

 toSurfData->toVendorText = &vendorText;
 toSurfData->nrOfVendorText = 1;
#else
 SurfVendorText* toVendorText=NULL;
 static SurfVendorText vendorText =
 {{"This SURF-Dataset is generated by STN-Atlas utilities !" }};

 toVendorText=(SurfVendorText*)calloc(1,sizeof(SurfVendorText));
 if(toVendorText != NULL)
 {
  strcpy(toVendorText->text,vendorText.text);
  toSurfData->toVendorText = toVendorText;
  toSurfData->nrOfVendorText = 1;
 } 
#endif
}





/*********************************************************************
*         H I S T O R I E
**********************************************************************
*  Edition  History
*   date    comments                                            by
* --------  ------------------------------------------------ ---------
* 13-08-93  created                                             pb
* 21-10-93  new: surf_timeSizeToInt                             pb
* 13-06-95  neue Version 2.0 (Sidescan-Backscatter)             pb
*********************************************************************/
