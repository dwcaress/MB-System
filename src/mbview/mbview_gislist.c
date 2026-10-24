/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_gislist.c	10/23/2026
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
/*
 * List dialogs for the GIS polylines, polygons, and points, and the route
 * list button that converts routes to polylines. Each list shows the layers
 * of one type, each followed by its features. Clicking an item twice cycles
 * its color (all features of a layer for a layer item); the buttons show or
 * hide, delete, or (polylines only) convert the selected items to routes.
 *
 * Author:	D. W. Caress
 * Date:	October 23, 2026
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/List.h>
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "MB3DGisList.h"

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"

/* rows of each list: a feature index (>= 0) or a layer index ilayer
    stored as -(ilayer + 1) */
static int *gislist_row[MBV_GISLAYER_GISPOINT + 1];
static int gislist_nrow[MBV_GISLAYER_GISPOINT + 1];
static int gislist_nrow_alloc[MBV_GISLAYER_GISPOINT + 1];

/* last single selected row, so that a second click cycles the color */
static int gislist_lastrow[MBV_GISLAYER_GISPOINT + 1] = {-1, -1, -1, -1};

static const char *gislist_title[MBV_GISLAYER_GISPOINT + 1] = {"", "Polyline List", "Polygon List", "Point List"};
static const char *gislist_header[MBV_GISLAYER_GISPOINT + 1] = {
    "", "Layer or Polyline | Display | Color | Points | Name", "Layer or Polygon | Display | Color | Points | Name",
    "Layer or Point | Display | Color | Lon | Lat | Z | Name"};
static const char *gislist_noun[MBV_GISLAYER_GISPOINT + 1] = {"", "polylines", "polygons", "points"};

/*------------------------------------------------------------------------------*/
/* accessors for the features of each type */
static int gislist_count(int type) {
	if (type == MBV_GISLAYER_POLYLINE)
		return (shared.shareddata.npolyline);
	else if (type == MBV_GISLAYER_POLYGON)
		return (shared.shareddata.npolygon);
	else if (type == MBV_GISLAYER_GISPOINT)
		return (shared.shareddata.ngispoint);
	return (0);
}
static bool *gislist_active(int type, int i) {
	if (type == MBV_GISLAYER_POLYLINE)
		return (&(shared.shareddata.polylines[i].active));
	else if (type == MBV_GISLAYER_POLYGON)
		return (&(shared.shareddata.polygons[i].active));
	return (&(shared.shareddata.gispoints[i].active));
}
static int *gislist_color(int type, int i) {
	if (type == MBV_GISLAYER_POLYLINE)
		return (&(shared.shareddata.polylines[i].color));
	else if (type == MBV_GISLAYER_POLYGON)
		return (&(shared.shareddata.polygons[i].color));
	return (&(shared.shareddata.gispoints[i].color));
}
static int gislist_layer(int type, int i) {
	if (type == MBV_GISLAYER_POLYLINE)
		return (shared.shareddata.polylines[i].layer);
	else if (type == MBV_GISLAYER_POLYGON)
		return (shared.shareddata.polygons[i].layer);
	return (shared.shareddata.gispoints[i].layer);
}
static int *gislist_selected(int type) {
	if (type == MBV_GISLAYER_POLYLINE)
		return (&(shared.shareddata.polyline_selected));
	else if (type == MBV_GISLAYER_POLYGON)
		return (&(shared.shareddata.polygon_selected));
	return (&(shared.shareddata.gispoint_selected));
}
static int gislist_delete(int type, size_t instance, int i) {
	int error = MB_ERROR_NO_ERROR;
	if (type == MBV_GISLAYER_POLYLINE)
		return (mbview_deletepolyline(mbv_verbose, instance, i, &error));
	else if (type == MBV_GISLAYER_POLYGON)
		return (mbview_deletepolygon(mbv_verbose, instance, i, &error));
	return (mbview_deletegispoint(mbv_verbose, instance, i, &error));
}

/*------------------------------------------------------------------------------*/
static int gislist_nextcolor(int color) {
	color++;
	if (color == MBV_COLOR_RED)
		color++;
	if (color > MBV_COLOR_PURPLE)
		color = MBV_COLOR_BLACK;
	return (color);
}

/*------------------------------------------------------------------------------*/
/* first active instance, or MBV_NO_WINDOW */
static size_t gislist_instance(void) {
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active)
			return (i);
	}
	return (MBV_NO_WINDOW);
}

/*------------------------------------------------------------------------------*/
static void gislist_redraw(void) {
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active)
			mbview_plotlowhigh(i);
	}
}

/*------------------------------------------------------------------------------*/
/* Mark the features of the selected rows; a selected layer row marks all
    of its features. Returns the number of features marked. */
static int gislist_markselected(int type, bool *marked) {
	const int count = gislist_count(type);
	for (int i = 0; i < count; i++)
		marked[i] = false;
	int position_count = 0;
	int *position_list = NULL;
	if (!XmListGetSelectedPos(shared.mb3d_gislist[type].mbview_list_gislist, &position_list, &position_count))
		return (0);
	for (int k = 0; k < position_count; k++) {
		const int row = position_list[k] - 1;
		if (row < 0 || row >= gislist_nrow[type])
			continue;
		const int code = gislist_row[type][row];
		if (code >= 0 && code < count) {
			marked[code] = true;
		}
		else if (code < 0) {
			for (int i = 0; i < count; i++) {
				if (gislist_layer(type, i) == -code - 1)
					marked[i] = true;
			}
		}
	}
	XtFree((char *)position_list);
	int nmarked = 0;
	for (int i = 0; i < count; i++) {
		if (marked[i])
			nmarked++;
	}
	return (nmarked);
}

/*------------------------------------------------------------------------------*/
int mbview_gislist_create(void) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* create the list windows like the site, route, and navigation lists */
	for (int type = MBV_GISLAYER_POLYLINE; type <= MBV_GISLAYER_GISPOINT; type++) {
		shared.init_gislist[type] = MBV_WINDOW_NULL;
		Arg args[256];
		Cardinal ac = 0;
		XtSetArg(args[ac], XmNtitle, gislist_title[type]);
		ac++;
		shared.topLevelShell_gislist[type] =
		    XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
		shared.mainWindow_gislist[type] = XmCreateMainWindow(shared.topLevelShell_gislist[type], "mainWindow_gislist", args, ac);
		XtManageChild(shared.mainWindow_gislist[type]);
		MB3DGisListCreate(&(shared.mb3d_gislist[type]), shared.mainWindow_gislist[type], "mbview_gislist", args, ac, type);
		ac = 0;
		Boolean argok = False;
		XmString tmp0 =
		    (XmString)BX_CONVERT(shared.mb3d_gislist[type].MB3DGisList, (char *)gislist_header[type], XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetValues(shared.mb3d_gislist[type].mbview_gislist_label, args, ac);
		XmStringFree(tmp0);
		XtManageChild(shared.mb3d_gislist[type].MB3DGisList);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
void mbview_gislist_reset(bool mode) {
	if (mode) {
		for (int type = MBV_GISLAYER_POLYLINE; type <= MBV_GISLAYER_GISPOINT; type++) {
			shared.init_gislist[type] = MBV_WINDOW_NULL;
			shared.topLevelShell_gislist[type] = NULL;
			shared.mainWindow_gislist[type] = NULL;
		}
	}
	for (int type = MBV_GISLAYER_POLYLINE; type <= MBV_GISLAYER_GISPOINT; type++) {
		gislist_nrow[type] = 0;
		gislist_lastrow[type] = -1;
	}
}

/*------------------------------------------------------------------------------*/
void mbview_gislist_popdownall(void) {
	for (int type = MBV_GISLAYER_POLYLINE; type <= MBV_GISLAYER_GISPOINT; type++) {
		if (shared.mainWindow_gislist[type] != NULL) {
			shared.init_gislist[type] = MBV_WINDOW_NULL;
			XmListDeleteAllItems(shared.mb3d_gislist[type].mbview_list_gislist);
			XtPopdown(XtParent(shared.mainWindow_gislist[type]));
		}
	}
}

/*------------------------------------------------------------------------------*/
int mbview_updategislist(int type) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       type:             %d\n", type);
	}

	int status = MB_SUCCESS;
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT || shared.init_gislist[type] != MBV_WINDOW_VISIBLE)
		return (status);

	Widget list = shared.mb3d_gislist[type].mbview_list_gislist;
	XmListDeleteAllItems(list);
	gislist_nrow[type] = 0;

	/* allocate the row map: at most every layer and every feature */
	const int count = gislist_count(type);
	const int nrow_max = count + shared.shareddata.ngislayer;
	if (nrow_max > gislist_nrow_alloc[type]) {
		int error = MB_ERROR_NO_ERROR;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, nrow_max * sizeof(int), (void **)&(gislist_row[type]), &error);
		if (status == MB_SUCCESS) {
			gislist_nrow_alloc[type] = nrow_max;
		}
		else {
			gislist_nrow_alloc[type] = 0;
			return (status);
		}
	}
	if (nrow_max == 0)
		return (status);
	XmString *xstr = (XmString *)malloc(nrow_max * sizeof(XmString));
	if (xstr == NULL)
		return (MB_FAILURE);

	/* each layer of this type with features, followed by its features */
	char text[2 * MB_PATH_MAXLINE];
	int nrow = 0;
	int selectedrow = -1;
	const int selected = *gislist_selected(type);
	for (int ilayer = 0; ilayer < shared.shareddata.ngislayer; ilayer++) {
		const struct mbview_gislayer_struct *layer = &(shared.shareddata.gislayers[ilayer]);
		int nfeature = 0;
		for (int i = 0; i < count; i++) {
			if (gislist_layer(type, i) == ilayer)
				nfeature++;
		}
		if (layer->type != type || nfeature == 0)
			continue;
		snprintf(text, sizeof(text), "Layer %d | %s | %d %s | %s", ilayer, layer->active ? "Shown" : "Hidden", nfeature,
		         gislist_noun[type], layer->name);
		gislist_row[type][nrow] = -(ilayer + 1);
		xstr[nrow++] = XmStringCreateLocalized(text);

		for (int i = 0; i < count; i++) {
			if (gislist_layer(type, i) != ilayer)
				continue;
			const char *shown = *gislist_active(type, i) ? "Shown" : "Hidden";
			const char *colorname = mbview_colorname[*gislist_color(type, i)];
			if (type == MBV_GISLAYER_POLYLINE) {
				const struct mbview_polyline_struct *polyline = &(shared.shareddata.polylines[i]);
				snprintf(text, sizeof(text), "    %4d | %s | %s | %d points in %d %s | %s", i, shown, colorname,
				         polyline->npoints, polyline->npart, polyline->npart == 1 ? "part" : "parts", polyline->name);
			}
			else if (type == MBV_GISLAYER_POLYGON) {
				const struct mbview_polygon_struct *polygon = &(shared.shareddata.polygons[i]);
				snprintf(text, sizeof(text), "    %4d | %s | %s | %d points in %d %s | %s", i, shown, colorname,
				         polygon->npoints, polygon->npart, polygon->npart == 1 ? "ring" : "rings", polygon->name);
			}
			else {
				const struct mbview_gispoint_struct *gispoint = &(shared.shareddata.gispoints[i]);
				char londstr[24], lonmstr[24], latdstr[24], latmstr[24];
				mbview_setlonlatstrings(gispoint->point.xlon, gispoint->point.ylat, londstr, latdstr, lonmstr, latmstr);
				const bool decimal = shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL;
				snprintf(text, sizeof(text), "    %4d | %s | %s | %s | %s | %.2f | %s", i, shown, colorname,
				         decimal ? londstr : lonmstr, decimal ? latdstr : latmstr, gispoint->zsource, gispoint->name);
			}
			if (i == selected)
				selectedrow = nrow;
			gislist_row[type][nrow] = i;
			xstr[nrow++] = XmStringCreateLocalized(text);
		}
	}
	gislist_nrow[type] = nrow;

	/* add list items, select the item for the selected feature */
	XmListAddItems(list, xstr, nrow, 0);
	if (selectedrow >= 0) {
		XmListSelectPos(list, selectedrow + 1, False);
		XmListSetPos(list, MAX(selectedrow + 1 - 5, 1));
	}
	for (int i = 0; i < nrow; i++)
		XmStringFree(xstr[i]);
	free(xstr);

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* show the route list button converting routes to polylines only when
    polylines are displayed, widening the route list if needed */
void mbview_gis_routelistbutton(void) {
	if (shared.mainWindow_routelist == NULL)
		return;
	Widget button = shared.mb3d_routelist.mbview_pushButton_routelist_topolylines;
	if (shared.shareddata.polyline_mode != MBV_POLYLINE_OFF) {
		Dimension width = 0;
		XtVaGetValues(shared.mb3d_routelist.MB3DRouteList, XmNwidth, &width, NULL);
		if (width < 600)
			XtVaSetValues(shared.mb3d_routelist.MB3DRouteList, XmNwidth, (Dimension)600, NULL);
		XtManageChild(button);
	}
	else {
		XtUnmanageChild(button);
	}
}

/*------------------------------------------------------------------------------*/
/* turn the display of a GIS type on or off in all active instances */
void mbview_gis_setviewmode(int type, int mode) {
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		struct mbview_struct *data = &(mbviews[i].data);
		if (!data->active)
			continue;
		if (type == MBV_GISLAYER_POLYLINE) {
			data->polyline_view_mode = mode;
			set_mbview_polyline_view_mode(i, mode);
		}
		else if (type == MBV_GISLAYER_POLYGON) {
			data->polygon_view_mode = mode;
			set_mbview_polygon_view_mode(i, mode);
		}
		else if (type == MBV_GISLAYER_GISPOINT) {
			data->gispoint_view_mode = mode;
			set_mbview_gispoint_view_mode(i, mode);
		}
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislistpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislistpopup: type:%d\n", type);

	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT || shared.mainWindow_gislist[type] == NULL)
		return;
	XtPopup(XtParent(shared.mainWindow_gislist[type]), XtGrabNone);
	shared.init_gislist[type] = MBV_WINDOW_VISIBLE;
	gislist_lastrow[type] = -1;
	mbview_updategislist(type);
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislistselect(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislistselect: type:%d\n", type);

	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT)
		return;
	Widget list = shared.mb3d_gislist[type].mbview_list_gislist;
	int position_count = 0;
	int *position_list = NULL;
	if (!XmListGetSelectedPos(list, &position_list, &position_count))
		position_count = 0;
	const int row = position_count == 1 ? position_list[0] - 1 : -1;
	if (position_list != NULL)
		XtFree((char *)position_list);

	/* a single feature is selected and highlighted; a second click on the
	    same item cycles the color of the feature, or of all features of a
	    layer item */
	int *selected = gislist_selected(type);
	*selected = MBV_SELECT_NONE;
	bool recolored = false;
	if (row >= 0 && row < gislist_nrow[type]) {
		const int code = gislist_row[type][row];
		const int count = gislist_count(type);
		if (code >= 0 && code < count) {
			*selected = code;
			if (gislist_lastrow[type] == row) {
				*gislist_color(type, code) = gislist_nextcolor(*gislist_color(type, code));
				recolored = true;
			}
		}
		else if (code < 0 && gislist_lastrow[type] == row) {
			int color = -1;
			for (int i = 0; i < count; i++) {
				if (gislist_layer(type, i) == -code - 1) {
					if (color < 0)
						color = gislist_nextcolor(*gislist_color(type, i));
					*gislist_color(type, i) = color;
				}
			}
			recolored = color >= 0;
		}
	}
	gislist_lastrow[type] = row;

	/* show the new colors, keeping the selected item */
	if (recolored) {
		mbview_updategislist(type);
		XmListSelectPos(list, row + 1, False);
	}

	gislist_redraw();
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_showhide(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_showhide: type:%d\n", type);

	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT)
		return;
	Widget list = shared.mb3d_gislist[type].mbview_list_gislist;
	int position_count = 0;
	int *position_list = NULL;
	if (!XmListGetSelectedPos(list, &position_list, &position_count) || position_count == 0)
		return;

	/* toggle the selected layers and features */
	const int count = gislist_count(type);
	for (int k = 0; k < position_count; k++) {
		const int row = position_list[k] - 1;
		if (row < 0 || row >= gislist_nrow[type])
			continue;
		const int code = gislist_row[type][row];
		if (code >= 0 && code < count)
			*gislist_active(type, code) = !*gislist_active(type, code);
		else if (code < 0)
			shared.shareddata.gislayers[-code - 1].active = !shared.shareddata.gislayers[-code - 1].active;
	}

	/* update the list keeping the selection, and the displays */
	mbview_updategislist(type);
	XmListDeselectAllItems(list);
	for (int k = 0; k < position_count; k++)
		XmListSelectPos(list, position_list[k], False);
	XtFree((char *)position_list);
	gislist_redraw();
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_delete(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_delete: type:%d\n", type);

	const size_t instance = gislist_instance();
	const int count = type >= MBV_GISLAYER_POLYLINE && type <= MBV_GISLAYER_GISPOINT ? gislist_count(type) : 0;
	if (instance == MBV_NO_WINDOW || count == 0)
		return;
	bool *marked = (bool *)malloc(count * sizeof(bool));
	if (marked == NULL)
		return;

	/* delete the marked features from the last down */
	if (gislist_markselected(type, marked) > 0) {
		for (int i = count - 1; i >= 0; i--) {
			if (marked[i])
				gislist_delete(type, instance, i);
		}
		gislist_lastrow[type] = -1;
		mbview_updategislist(type);
		gislist_redraw();
		mbview_action_sensitivityall();
	}
	free(marked);
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_toroutes(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_toroutes: type:%d\n", type);

	const size_t instance = gislist_instance();
	const int count = type == MBV_GISLAYER_POLYLINE ? gislist_count(type) : 0;
	if (instance == MBV_NO_WINDOW || count == 0)
		return;
	bool *marked = (bool *)malloc(count * sizeof(bool));
	if (marked == NULL)
		return;

	/* convert the marked polylines, each part to a route */
	const int nmarked = gislist_markselected(type, marked);
	int nroute_total = 0;
	int ndropped_total = 0;
	for (int i = 0; i < count; i++) {
		if (!marked[i])
			continue;
		int nroute = 0;
		int ndropped = 0;
		int error = MB_ERROR_NO_ERROR;
		mbview_polylinetoroutes(mbv_verbose, instance, i, &nroute, &ndropped, &error);
		nroute_total += nroute;
		ndropped_total += ndropped;
	}
	free(marked);

	if (nmarked > 0) {
		fprintf(stderr, "Converted %d polylines to %d routes", nmarked, nroute_total);
		if (ndropped_total > 0)
			fprintf(stderr, ", %d vertices outside the view were not used", ndropped_total);
		fprintf(stderr, "\n");
		if (nroute_total > 0) {
			mbview_updateroutelist();
			gislist_redraw();
			mbview_action_sensitivityall();
		}
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_popdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_popdown: type:%d\n", type);

	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT || shared.mainWindow_gislist[type] == NULL)
		return;
	shared.init_gislist[type] = MBV_WINDOW_NULL;
	XmListDeleteAllItems(shared.mb3d_gislist[type].mbview_list_gislist);
	XtPopdown(XtParent(shared.mainWindow_gislist[type]));
}

/*------------------------------------------------------------------------------*/
/* Convert the routes selected in the route list (a route item or any of its
    waypoint items selects the route) to polylines in the layer for routes
    converted to polylines. */
void do_mbview_routelist_topolylines(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_routelist_topolylines:\n");

	const size_t instance = gislist_instance();
	if (instance == MBV_NO_WINDOW || shared.shareddata.polyline_mode == MBV_POLYLINE_OFF || shared.shareddata.nroute == 0)
		return;
	int position_count = 0;
	int *position_list = NULL;
	if (!XmListGetSelectedPos(shared.mb3d_routelist.mbview_list_routelist, &position_list, &position_count) ||
	    position_count == 0)
		return;
	bool *marked = (bool *)calloc(shared.shareddata.nroute, sizeof(bool));
	if (marked == NULL) {
		XtFree((char *)position_list);
		return;
	}

	/* map list positions to routes the same way as mbview_updateroutelist(),
	    which lists each active route followed by its waypoints */
	for (int k = 0; k < position_count; k++) {
		const int iposition = position_list[k] - 1;
		int iroutepos = 0;
		for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
			if (!shared.shareddata.routes[iroute].active)
				continue;
			if (iposition >= iroutepos && iposition <= iroutepos + shared.shareddata.routes[iroute].npoints)
				marked[iroute] = true;
			iroutepos += shared.shareddata.routes[iroute].npoints + 1;
		}
	}
	XtFree((char *)position_list);

	int nconverted = 0;
	for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
		int ipolyline;
		int error = MB_ERROR_NO_ERROR;
		if (marked[iroute] && mbview_routetopolyline(mbv_verbose, instance, iroute, -1, &ipolyline, &error) == MB_SUCCESS)
			nconverted++;
	}
	free(marked);

	if (nconverted > 0) {
		fprintf(stderr, "Converted %d routes to polylines in layer \"%s\"\n", nconverted, MBV_GISLAYER_FROMROUTES);
		mbview_gis_setviewmode(MBV_GISLAYER_POLYLINE, MBV_VIEW_ON);
		mbview_updategislist(MBV_GISLAYER_POLYLINE);
		gislist_redraw();
		mbview_action_sensitivityall();
	}
}
/*------------------------------------------------------------------------------*/
