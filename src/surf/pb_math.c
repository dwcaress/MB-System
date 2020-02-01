// DATEINAME        : pb_math.c
// ERSTELLUNGSDATUM : 20.09.93
// COPYRIGHT (C) 1993: ATLAS ELEKTRONIK GMBH, 28305 BREMEN
//
// See README file for copying and redistribution conditions.

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xdr_surf.h"
#include "mem_surf.h"
#include "util_surf.h"
#include "pb_math.h"

double setToPlusMinusPI(double angle) {
  if (angle > PI) {
    angle = setToPlusMinusPI(angle - (2.0*PI));
  }
  if(angle < -PI) {
    angle = setToPlusMinusPI(angle + (2.0*PI));
  }
 return angle;
}

// Berechnung von Tiefe und Ablagen aus Traveltime
bool depthFromTT(FanParam* fanParam, bool isPitchcompensated) {
  // aktuellen Winkel Aufgrund cmean rechnen
  if(fanParam->cmean == 0.0 || fanParam->ckeel == 0.0 ||
     fanParam->travelTime == 0.0)
    return false;

  const double arg = (fanParam->cmean / fanParam->ckeel) * sin(fanParam->angle);

  if (arg >= 1.0 || arg <= -1.0)
     return false;

  const double alpha = asin(arg);

  // Winkelparameter

  double tanAl  = tan(alpha);
  double tan2Al = tanAl * tanAl;

  // TODO(schwehr): Simplify
  double tanP;
  double tan2P;
  if (!isPitchcompensated) {
    tanP  = tan(fanParam->pitchTx);
    tan2P = tanP * tanP;
  } else {
    tanP  = 0.0;
    tan2P = 0.0;
  }

  // mittleren rueckgestreuten Weg auf Bezugsniveau HubRx rechnen
  double travelWay = fanParam->travelTime * fanParam->cmean;
  travelWay = travelWay
            - (((fanParam->heaveTx - fanParam->heaveRx)/2.0) * cos(alpha));

  // Pitchkompensierte Tiefe ohne Huboffset (3D-Pythagoras)

  const double depth = travelWay / sqrt(tan2Al + tan2P + 1);


  // Positionen
  fanParam->posAhead = (depth * tanP)  + fanParam->transducerOffsetAhead;
  fanParam->posStar  = (depth * tanAl) + fanParam->transducerOffsetStar;


  // Tiefe rechnen
  fanParam->depth = depth              // Pitch-kompensierte Messtiefe
                  - fanParam->heaveRx  // absolute Hubkompensation
                  + fanParam->draught; // Tiefgang

  return true;
}
