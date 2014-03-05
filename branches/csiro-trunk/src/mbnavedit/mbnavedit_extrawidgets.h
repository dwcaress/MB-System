/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *    $Id$
 *
 *    Copyright (c) 1995-2013 by
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
 * MBNAVEDIT is an interactive navigation editor for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global widget parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	June 24,  1995
 *
 * $Log: mbnavedit_extrawidgets.h,v $
 * Revision 5.2  2009/03/10 05:11:22  caress
 * Added Gaussian mean smoothing to MBnavedit.
 *
 * Revision 5.1  2003/04/17 21:09:06  caress
 * Release 5.0.beta30
 *
 * Revision 5.0  2000/12/01 22:56:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/09/30  07:03:14  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1997/04/21  17:07:38  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1995/08/07  18:33:22  caress
 * First cut.
 *
 * Revision 4.0  1995/08/07  18:33:22  caress
 * First cut.
 *
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVEDIT_DECLARE_GLOBALS
#define MBNAVEDIT_EXTERNAL
#else
#define MBNAVEDIT_EXTERNAL extern
#endif

/* Global Widget Declarations Added By Hand */

MBNAVEDIT_EXTERNAL Widget       fileSelectionBox_list;
MBNAVEDIT_EXTERNAL Widget       fileSelectionBox_text;
MBNAVEDIT_EXTERNAL Widget       scrolledWindow_hscrollbar;
MBNAVEDIT_EXTERNAL Widget       scrolledWindow_vscrollbar;

/*--------------------------------------------------------------------*/
