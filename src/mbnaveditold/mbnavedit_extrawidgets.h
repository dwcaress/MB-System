/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *    $Id: mbnavedit_extrawidgets.h,v 4.0 1995-08-07 18:33:22 caress Exp $
 *
 *    Copyright (c) 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
