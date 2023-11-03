/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DSiteList.h	10/28/2003
 *
 *    Copyright (c) 2003-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#ifndef MBVIEW_MB3DSiteList_H_
#define MBVIEW_MB3DSiteList_H_

typedef struct _MB3DSiteListData {
	Widget MB3DSiteList;
	Widget mbview_pushButton_sitelist_dismiss;
	Widget mbview_pushButton_sitelist_delete;
	Widget mbview_sitelist_label;
	Widget mbview_scrolledWindow_sitelist;
	Widget mbview_list_sitelist;
} MB3DSiteListData;

typedef struct _MB3DSiteListData *MB3DSiteListDataPtr;

MB3DSiteListDataPtr MB3DSiteListCreate(
    MB3DSiteListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DSiteList_H_
