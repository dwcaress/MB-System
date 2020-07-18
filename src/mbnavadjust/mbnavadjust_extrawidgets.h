/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *
 *    Copyright (c) 2000-2020 by
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
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVADJUST_DECLARE_GLOBALS
#define MBNAVADJUST_EXTERNAL
#else
#define MBNAVADJUST_EXTERNAL extern
#endif

/* Global Widget Declarations Added By Hand */

MBNAVADJUST_EXTERNAL Widget fileSelectionBox_list;
MBNAVADJUST_EXTERNAL Widget fileSelectionBox_text;

/*--------------------------------------------------------------------*/
