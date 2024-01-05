/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DRouteList.h	10/28/2003
 *
 *    Copyright (c) 2003-2024 by
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

#ifndef MBVIEW_MB3DRouteList_H_
#define MBVIEW_MB3DRouteList_H_

typedef struct _MB3DRouteListData {
	Widget MB3DRouteList;
	Widget mbview_routelist_label;
	Widget mbview_pushButton_routelist_delete;
	Widget mbview_pushButton_routelist_dismiss;
	Widget mbview_scrolledWindow_routelist;
	Widget mbview_list_routelist;
} MB3DRouteListData;

typedef struct _MB3DRouteListData *MB3DRouteListDataPtr;

MB3DRouteListDataPtr MB3DRouteListCreate(
    MB3DRouteListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DRouteList_H_
