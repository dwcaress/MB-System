/*--------------------------------------------------------------------
 *    The MB-system:  mbv_gislist_core.c  11/7/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Core functions of the polyline, polygon and point (GIS) lists that use no
 * X11 or Motif: the row map and text of the list rows, and the actions on the
 * selected rows (select, show or hide, delete, convert). The Motif list
 * windows (mbview_gislist.c) and other toolkits show the rows and pass the
 * selected list positions. Moved from mbview_gislist.c.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#ifndef MBVIEW_NO_MOTIF
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#else
#include "mbv_xtypes.h"
#endif
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "MB3DGisList.h"

#include "mbv_gl.h"

#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_ui.h"

/* rows of each list: a feature index (>= 0) or a layer index ilayer
    stored as -(ilayer + 1) */
static int *gislist_row[MBV_GISLAYER_GISPOINT + 1];
static int gislist_nrow[MBV_GISLAYER_GISPOINT + 1];
static int gislist_nrow_alloc[MBV_GISLAYER_GISPOINT + 1];

/* last single selected row, so that a second click cycles the color */
static int gislist_lastrow[MBV_GISLAYER_GISPOINT + 1] = {-1, -1, -1, -1};

static const char *gislist_title_text[MBV_GISLAYER_GISPOINT + 1] = {"", "Polyline List", "Polygon List", "Point List"};
static const char *gislist_header_text[MBV_GISLAYER_GISPOINT + 1] = {
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
const char *mbview_gislist_title(int type) {
	return (type >= MBV_GISLAYER_POLYLINE && type <= MBV_GISLAYER_GISPOINT ? gislist_title_text[type] : "");
}
const char *mbview_gislist_header(int type) {
	return (type >= MBV_GISLAYER_POLYLINE && type <= MBV_GISLAYER_GISPOINT ? gislist_header_text[type] : "");
}

/*------------------------------------------------------------------------------*/
/* redraw all active instances */
void mbview_gislist_redraw(void) { gislist_redraw(); }

/*------------------------------------------------------------------------------*/
/* Mark the features of the selected rows (list positions, 1 for the first
    row); a selected layer row marks all of its features. Returns the number of features marked. */
static int gislist_markselected(int type, bool *marked, int position_count, const int *position_list) {
	const int count = gislist_count(type);
	for (int i = 0; i < count; i++)
		marked[i] = false;
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
	int nmarked = 0;
	for (int i = 0; i < count; i++) {
		if (marked[i])
			nmarked++;
	}
	return (nmarked);
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
/* The rows of a GIS list (from mbview_updategislist): each layer of the type
    with features, followed by its features. Also sets the row map that the
    actions below use. */
int mbview_gislist_rows(int type, struct mbview_listrows *rows) {
	memset(rows, 0, sizeof(*rows));
	int status = MB_SUCCESS;
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT)
		return (status);
	gislist_nrow[type] = 0;
	gislist_lastrow[type] = -1;

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
		if (mbview_listrows_add(rows, text) != MB_SUCCESS)
			return (MB_FAILURE);
		nrow++;

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
			if (mbview_listrows_add(rows, text) != MB_SUCCESS)
			return (MB_FAILURE);
		nrow++;
		}
	}
	gislist_nrow[type] = nrow;
	rows->selected = selectedrow + 1;


	return (status);
}

/*------------------------------------------------------------------------------*/
/* A row of a GIS list was selected (positions as in the list, 1 for the first
    row). Returns the position the list has to select again after a color
    change, 0 otherwise. The caller redraws (mbview_gislist_redraw). */
int mbview_gislist_select(int type, int position_count, const int *position_list) {
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT)
		return (0);
	const int row = position_count == 1 ? position_list[0] - 1 : -1;

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

	/* show the new colors; the list selects the row again */
	int reselect = 0;
	if (recolored) {
		mbview_updategislist(type);
		reselect = row + 1;
		gislist_lastrow[type] = row;
	}

	return (reselect);
}

/*------------------------------------------------------------------------------*/
/* Show or hide the selected layers and features of a GIS list and update the
    list. The caller selects the rows again and redraws. */
int mbview_gislist_showhide(int type, int position_count, const int *position_list) {
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT)
		return (MB_FAILURE);

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

	mbview_updategislist(type);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* Delete the features of the selected rows of a GIS list (positions as in the list) */
int mbview_gislist_delete(int type, int position_count, const int *position_list) {
	const size_t instance = gislist_instance();
	const int count = type >= MBV_GISLAYER_POLYLINE && type <= MBV_GISLAYER_GISPOINT ? gislist_count(type) : 0;
	if (instance == MBV_NO_WINDOW || count == 0)
		return (MB_SUCCESS);
	bool *marked = (bool *)malloc(count * sizeof(bool));
	if (marked == NULL)
		return (MB_SUCCESS);

	/* delete the marked features from the last down */
	if (gislist_markselected(type, marked, position_count, position_list) > 0) {
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

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* Convert the polylines of the selected rows of a GIS list (positions as in the list) */
int mbview_gislist_toroutes(int type, int position_count, const int *position_list) {
	const size_t instance = gislist_instance();
	const int count = type == MBV_GISLAYER_POLYLINE ? gislist_count(type) : 0;
	if (instance == MBV_NO_WINDOW || count == 0)
		return (MB_SUCCESS);
	bool *marked = (bool *)malloc(count * sizeof(bool));
	if (marked == NULL)
		return (MB_SUCCESS);

	/* convert the marked polylines, each part to a route */
	const int nmarked = gislist_markselected(type, marked, position_count, position_list);
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

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* Convert the routes selected in the route list (positions as in the list; a
    route item or any of its waypoint items selects the route) to polylines in
    the layer for routes converted to polylines. */
int mbview_routelist_topolylines(int position_count, const int *position_list) {
	const size_t instance = gislist_instance();
	if (instance == MBV_NO_WINDOW || shared.shareddata.polyline_mode == MBV_POLYLINE_OFF || shared.shareddata.nroute == 0 ||
	    position_count == 0)
		return (MB_SUCCESS);
	bool *marked = (bool *)calloc(shared.shareddata.nroute, sizeof(bool));
	if (marked == NULL)
		return (MB_FAILURE);

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

	return (MB_SUCCESS);
}
