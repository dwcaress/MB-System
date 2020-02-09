/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_extrawidgets.h	8/7/95
 *
 *    Copyright (c) 1995-2019 by
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
 */

#ifndef MBNAVEDIT_MBNAVEDIT_EXTRAWIDGETS_H_
#define MBNAVEDIT_MBNAVEDIT_EXTRAWIDGETS_H_

#include "mb_status.h"

Widget fileSelectionBox_list;
Widget fileSelectionBox_text;
Widget scrolledWindow_hscrollbar;
Widget scrolledWindow_vscrollbar;

#endif  // MBNAVEDIT_MBNAVEDIT_EXTRAWIDGETS_H_
