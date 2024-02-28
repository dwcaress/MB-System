/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DNavList.h	10/28/2003
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

#ifndef MBVIEW_MB3DNavList_H_
#define MBVIEW_MB3DNavList_H_

typedef struct _MB3DNavListData {
	Widget MB3DNavList;
	Widget mbview_navlist_label;
	Widget mbview_pushButton_navlist_delete;
	Widget mbview_pushButton_navlist_dismiss;
	Widget mbview_scrolledWindow_navlist;
	Widget mbview_list_navlist;
} MB3DNavListData;

typedef struct _MB3DNavListData *MB3DNavListDataPtr;

MB3DNavListDataPtr MB3DNavListCreate(MB3DNavListDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DNavList_H_
