/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *    $Id: mbnavedit_extrawidgets.h,v 5.0 2000-12-01 22:56:26 caress Exp $
 *
 *    Copyright (c) 1995, 2000 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *    David W. Caress (caress@mbari.org)
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
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
 * $Log: not supported by cvs2svn $
 * Revision 4.3  2000/09/30  07:04:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  2000/09/30  07:04:05  caress
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
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* Global Widget Declarations Added By Hand */

EXTERNAL Widget       fileSelectionBox_list;
EXTERNAL Widget       fileSelectionBox_text;
EXTERNAL Widget       scrolledWindow_hscrollbar;
EXTERNAL Widget       scrolledWindow_vscrollbar;

/*--------------------------------------------------------------------*/
