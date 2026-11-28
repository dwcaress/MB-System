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
		XtSetArg(args[ac], XmNtitle, mbview_gislist_title(type));
		ac++;
		shared.topLevelShell_gislist[type] =
		    XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
		shared.mainWindow_gislist[type] = XmCreateMainWindow(shared.topLevelShell_gislist[type], "mainWindow_gislist", args, ac);
		XtManageChild(shared.mainWindow_gislist[type]);
		MB3DGisListCreate(&(shared.mb3d_gislist[type]), shared.mainWindow_gislist[type], "mbview_gislist", args, ac, type);
		ac = 0;
		Boolean argok = False;
		XmString tmp0 =
		    (XmString)BX_CONVERT(shared.mb3d_gislist[type].MB3DGisList, (char *)mbview_gislist_header(type), XmRXmString, 0, &argok);
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

	/* the rows of the core, with the item for the selected feature */
	struct mbview_listrows rows;
	status = mbview_gislist_rows(type, &rows);
	if (status == MB_SUCCESS)
		mbv_ui_motif_fill_list(list, &rows);
	mbview_listrows_free(&rows);

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
	const int reselect = mbview_gislist_select(type, position_count, position_list);
	if (position_list != NULL)
		XtFree((char *)position_list);
	if (reselect > 0)
		XmListSelectPos(list, reselect, False);
	mbview_gislist_redraw();
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

	/* toggle the selected layers and features, update the list */
	mbview_gislist_showhide(type, position_count, position_list);

	/* keep the selection, and update the displays */
	XmListDeselectAllItems(list);
	for (int k = 0; k < position_count; k++)
		XmListSelectPos(list, position_list[k], False);
	XtFree((char *)position_list);
	mbview_gislist_redraw();
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_delete(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_delete: type:%d\n", type);

	/* the selected rows */
	int position_count = 0;
	int *position_list = NULL;
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT ||
	    !XmListGetSelectedPos(shared.mb3d_gislist[type].mbview_list_gislist, &position_list, &position_count))
		position_count = 0;
	mbview_gislist_delete(type, position_count, position_list);
	if (position_list != NULL)
		XtFree((char *)position_list);
}

/*------------------------------------------------------------------------------*/
void do_mbview_gislist_toroutes(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)call_data;  // Unused parameter
	const int type = (int)(size_t)client_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_gislist_toroutes: type:%d\n", type);

	/* the selected rows */
	int position_count = 0;
	int *position_list = NULL;
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT ||
	    !XmListGetSelectedPos(shared.mb3d_gislist[type].mbview_list_gislist, &position_list, &position_count))
		position_count = 0;
	mbview_gislist_toroutes(type, position_count, position_list);
	if (position_list != NULL)
		XtFree((char *)position_list);
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

	int position_count = 0;
	int *position_list = NULL;
	if (!XmListGetSelectedPos(shared.mb3d_routelist.mbview_list_routelist, &position_list, &position_count) ||
	    position_count == 0)
		return;
	mbview_routelist_topolylines(position_count, position_list);
	XtFree((char *)position_list);
}
/*------------------------------------------------------------------------------*/
