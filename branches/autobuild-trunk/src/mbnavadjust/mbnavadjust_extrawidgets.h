/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *    $Id: mbnavadjust_extrawidgets.h 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 2000-2012 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global widget parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	March 22, 2000
 *
 * $Log: mbnavadjust_extrawidgets.h,v $
 * Revision 5.1  2008/05/16 22:42:32  caress
 * Release 5.1.1beta18 - working towards use of 3D uncertainty.
 *
 * Revision 5.0  2000/12/01 22:55:48  caress
 * First cut at Version 5.0.
 *
 * Revision 4.0  2000/09/30  07:00:06  caress
 * Snapshot for Dale.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVADJUST_DECLARE_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* Global Widget Declarations Added By Hand */

EXTERNAL Widget       fileSelectionBox_list;
EXTERNAL Widget       fileSelectionBox_text;

/*--------------------------------------------------------------------*/
