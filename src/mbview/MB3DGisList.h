/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DGisList.h	10/23/2026
 *
 *    Copyright (c) 2026-2026 by
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

#ifndef MBVIEW_MB3DGisList_H_
#define MBVIEW_MB3DGisList_H_

/* list dialog used for GIS polylines, polygons, and points; the type
    (MBV_GISLAYER_POLYLINE, MBV_GISLAYER_POLYGON, or MBV_GISLAYER_GISPOINT)
    is passed to the callbacks as client data */
typedef struct _MB3DGisListData {
	Widget MB3DGisList;
	Widget mbview_pushButton_gislist_dismiss;
	Widget mbview_pushButton_gislist_showhide;
	Widget mbview_pushButton_gislist_delete;
	Widget mbview_pushButton_gislist_toroutes;
	Widget mbview_gislist_label;
	Widget mbview_scrolledWindow_gislist;
	Widget mbview_list_gislist;
} MB3DGisListData;

typedef struct _MB3DGisListData *MB3DGisListDataPtr;

MB3DGisListDataPtr MB3DGisListCreate(MB3DGisListDataPtr, Widget, String, ArgList, Cardinal, int);

#endif  // MBVIEW_MB3DGisList_H_
