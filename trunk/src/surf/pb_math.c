/*-----------------------------------------------------------------------
/ P R O G R A M M K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : pb_math.c
/  ERSTELLUNGSDATUM : 20.09.93
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


#define _PB_MATH 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include "types_win32.h"
#include <string.h>
#endif

#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"
#include "pb_math.h"

static char sccsid[50] = {"@(#)libpbmath.a Version 1.2 15.12.1998"};

/* There are C++ - Compilers, which omit unreferenced statics */
char* forCCmath(void)
{
 return(sccsid);
}


double pbAtan2(double y,double x)
{
 if(fabs(x) > 0.0)
 {
  return(atan2(y,x));
 } 
 else
 {
  if(y< 0.0)
   return((double)-HALF_PI);
  else
   return((double)HALF_PI); 
 }
}



double setToPlusMinusPI(double angle)
{
 if(angle > PI)
 {
  angle = setToPlusMinusPI(angle - (2.0*PI)); 
 } 
 if(angle < -PI)
 {
  angle = setToPlusMinusPI(angle + (2.0*PI)); 
 }
 return(angle); 
}




void rotateCoordinates(double rotAngle,XY_Coords* origCoords,
                                       XY_Coords* targetCoords)
{
 double angle;

 angle = setToPlusMinusPI(rotAngle); 
 targetCoords->x =   (origCoords->x * cos(angle)) 
                   + (origCoords->y * sin(angle));
 targetCoords->y = - (origCoords->x * sin(angle)) 
                   + (origCoords->y * cos(angle));
}


void xyToRhoPhi(double x0,double y0,double pointX,double pointY,
                                             double* rho,double* phi)
{
 double x,y,angle;

 x = pointX - x0;
 y = pointY - y0;
 *rho = sqrt((x*x) + (y*y));
 angle = pbAtan2(x,y);
 if(angle < 0.0)
  *phi = angle + (2.0 * PI);
 else
  *phi = angle;
}

 
void lambdaPhiToRhoPhi(double x0,double y0,double pointX,double pointY,
                                             double* rho,double* phi)
{
 double x,y,angle;

 x = pointX - x0;
 y = pointY - y0;
 x = RAD_TO_METER_X(x,y0);
 y = RAD_TO_METER_Y(y);
 *rho = sqrt((x*x) + (y*y));
 angle = pbAtan2(x,y);
 if(angle < 0.0)
  *phi = angle + (2.0 * PI);
 else
  *phi = angle;
}

 
Boolean signf(double value) 
{
 if(value < 0.0) return(False);
 return(True);
}


 
Boolean signsh(short value) 
{
 if(value < 0) return(False);
 return(True);
}




/* Berechnung von Tiefe und Ablagen aus Traveltime */

Boolean depthFromTT(FanParam* fanParam,Boolean isPitchcompensated)
{
 double alpha,arg,travelWay,depth,tanAl,tan2Al,tanP,tan2P;

 /* aktuellen Winkel Aufgrund cmean rechnen */
 
  if((fanParam->cmean == 0.0) || (fanParam->ckeel == 0.0)
     || (fanParam->travelTime == 0.0 )) return (False);
  arg = (fanParam->cmean / fanParam->ckeel) * sin(fanParam->angle);
  if((arg < 1.0) && (arg > -1.0)) 
     alpha = asin(arg);
  else
     return(False);  

 /* Winkelparameter */

  tanAl  = tan(alpha);
  tan2Al = tanAl * tanAl;    
  if(isPitchcompensated == False)
  {
   tanP  = tan(fanParam->pitchTx);
   tan2P = tanP * tanP;
  }
  else
  {
   tanP  = 0.0;
   tan2P = 0.0;
  }

 /* mittleren rueckgestreuten Weg auf Bezugsniveau HubRx rechnen */

  travelWay = fanParam->travelTime * fanParam->cmean;

  travelWay = travelWay 
            - (((fanParam->heaveTx - fanParam->heaveRx)/2.0) * cos(alpha)); 

 /* Pitchkompensierte Tiefe ohne Huboffset (3D-Pythagoras) */

  depth = travelWay / sqrt(tan2Al + tan2P + 1);


 /* Positionen */

  fanParam->posAhead = (depth * tanP)  + fanParam->transducerOffsetAhead;

  fanParam->posStar  = (depth * tanAl) + fanParam->transducerOffsetStar;

 
 /* Tiefe rechnen */
           
  fanParam->depth = depth                                  /* Pitch-kompensierte Messtiefe */
                  - fanParam->heaveRx                      /* absolute Hubkompensation     */
                  + fanParam->draught;                     /* Tiefgang                     */

  return(True);                   
}




/* Berechnung von Traveltime und Ablagen aus Depth */

Boolean TTfromDepth(FanParam* fanParam,Boolean isPitchcompensated)
{
 double alpha,arg,travelWay,depth,tanAl,tan2Al,tanP,tan2P;

 /* aktuellen Winkel Aufgrund cmean rechnen */
 
  if((fanParam->cmean == 0.0) || (fanParam->ckeel == 0.0)
     || (fanParam->travelTime == 0.0 )) return (False);
  arg = (fanParam->cmean / fanParam->ckeel) * sin(fanParam->angle);
  if((arg < 1.0) && (arg > -1.0)) 
     alpha = asin(arg);
  else
     return(False);  

 /* Winkelparameter */

  tanAl  = tan(alpha);
  tan2Al = tanAl * tanAl;    
  if(isPitchcompensated == False)
  {
   tanP  = tan(fanParam->pitchTx);
   tan2P = tanP * tanP;
  }
  else
  {
   tanP  = 0.0;
   tan2P = 0.0;
  }

 /* Tiefe auf HubRx-Niveau rechnen */
           
  depth =           fanParam->depth                        /* Pitch-kompensierte Messtiefe */
                  + fanParam->heaveRx                      /* absolute Hubkompensation     */
                  - fanParam->draught;                     /* Tiefgang                     */

 /* Travelway auf HubRx-Niveau (3D-Pythagoras) */

  travelWay = depth * sqrt(tan2Al + tan2P + 1);

 /* mittleren rueckgestreuten Weg auf Bezugsniveau HubRx rechnen */

  travelWay = travelWay 
            + (((fanParam->heaveTx - fanParam->heaveRx)/2.0) * cos(alpha/2.0)); 
  fanParam->travelTime = travelWay / fanParam->cmean;

 /* Positionen */

  fanParam->posAhead = (depth * tanP)  + fanParam->transducerOffsetAhead;

  fanParam->posStar  = (depth * tanAl) + fanParam->transducerOffsetStar;


 return(True);                   
}





/* Berechnung von Draught aus Ablage und Depth */

Boolean draughtFromDepth(FanParam* fanParam)
{
 double alpha,arg,fanDepth;

 /* aktuellen Winkel aufgrund cmean rechnen */
 
 if((fanParam->cmean == 0.0) || (fanParam->ckeel == 0.0)) return (False);
 arg = (fanParam->cmean / fanParam->ckeel) * sin(fanParam->angle);
 if((arg < 1.0) && (arg > -1.0)) 
    alpha = asin(arg);
 else
    return(False);  

 /* Faechertiefe */
 
 fanDepth = 
   fabs(fanParam->posStar - fanParam->transducerOffsetStar)/tan(fabs(alpha));

 fanParam->draught = fanParam->depth - fanDepth;
 return(True);                   
}


/* Berechnung von Heave aus Ablage,Tiefgang und Depth */

Boolean heaveFromDepth(FanParam* fanParam)
{
 double alpha,arg,fanDepth;

 /* aktuellen Winkel aufgrund cmean rechnen */
 
 if((fanParam->cmean == 0.0) || (fanParam->ckeel == 0.0)) return (False);
 arg = (fanParam->cmean / fanParam->ckeel) * sin(fanParam->angle);
 if((arg < 1.0) && (arg > -1.0)) 
    alpha = asin(arg);
 else
    return(False);  

 /* Faechertiefe */
 
 fanDepth = (fanParam->posStar - fanParam->transducerOffsetStar)/tan(alpha);

 fanParam->heaveTx = - (fanParam->depth - fanDepth - fanParam->draught);
 fanParam->heaveRx = fanParam->heaveTx;

 return(True);                   
}




/* nach Del Grosso : ....     */


double cMeanToTemperature(double salinity,double cMean)
{
 double k0,k1,k2,arg,t1,t2;

 k0 = -(1448.6 + (1.15*(salinity-35.0)) - cMean);
 k1 = -4.618;
 k2 = 0.0523;

 arg = (k1*k1) - (4.0*k0*k2);
 if(arg < 0) return(0.0); /* nur komplexe Loesungen */

 t1 = (-k1 + sqrt(arg))/(2.0 * k2);
 t2 = (-k1 - sqrt(arg))/(2.0 * k2);
 if((t2 >= 0.0) && (t2 < 60.0))
    return(t2);
 if((t1 >= 0.0) && (t1 < 60.0))
    return(t1);
 else
    return((double)(0.0));   
}

double temperatureToCMean(double salinity,double temperature)
{
 return((double)(1448.6 + (temperature*4.618) 
               - (temperature*temperature*0.0523) +(1.15* (salinity-35.0))));
}


double temperatureToCMeanDelGrosso(double salinity,double temperature)
{
 return((double)(1448.6 + (temperature*4.618) 
               - (temperature*temperature*0.0523) +(1.15* (salinity-35.0))));
}


double temperatureToCMeanMedwin(double salinity,double temperature)
{
 return((double)(1449.2 + (temperature*4.6) - (temperature*temperature*0.055)
               + (temperature*temperature*temperature*0.00029) +((1.34 - 0.01*temperature)* (salinity-35.0))));
}


/* Zeitwandlungen */


SurfTime surfTimeOfDayFromAbsTime (SurfTime absTime)
{
#define HOURS_PER_DAY   3600.0*24.0

 while (absTime > (1000.0 * HOURS_PER_DAY))
   absTime = absTime - (1000.0 * HOURS_PER_DAY); 
 while (absTime > (100.0 * HOURS_PER_DAY))
   absTime = absTime - (100.0 * HOURS_PER_DAY); 
 while (absTime > (10.0 * HOURS_PER_DAY))
   absTime = absTime - (10.0 * HOURS_PER_DAY); 
 while (absTime > (HOURS_PER_DAY))
   absTime = absTime - (HOURS_PER_DAY);
 return(absTime);  
}



void timeFromRelTime (SurfTime relTime,char*buffer)
{
 short day,hour,min,sec;
 day = 0;
 hour = 0;
 min = 0;
 sec = 0;
 
 while (relTime >= (24.0*3600.0))
 {
  relTime = relTime - (24.0*3600.0);
  day++;
 }    
 while (relTime >= 3600.0)
 {
  relTime = relTime - 3600.0;
  hour++;
 }    
 while (relTime >= 60.0)
 {
  relTime = relTime - 60.0;
  min++;
 }    
 while (relTime > 0.0)
 {
  relTime = relTime - 1.0;
  sec++;
 }
 if(sec >= 60)
 {
  min++;
  sec = 0;
 } 
 if(min >= 60)
 {
  hour++;
  min = 0;
 } 
 if(hour >= 24)
 {
  day++;
  hour = 0;
 } 
 
 sprintf(buffer,"%02d:%02d:%02d",hour,min,sec);    
}


Boolean relTimeFromTime (char*buffer,SurfTime* relTime)
{
 int hour,min,sec;
 int nr;

 if(strlen(buffer) >8)buffer[8] = 0; 
 if(strlen(buffer) != 8) return(False);
 if((buffer[2] != ':') || (buffer[5] != ':')) return(False);
 nr = sscanf(buffer,"%2d:%2d:%2d",&hour,&min,&sec);
 if(nr != 3) return(False);

 *relTime = ((double)(hour) * 3600.0)
          + ((double)(min ) *   60.0)
          + ((double)(sec ));
 return(True);
}


