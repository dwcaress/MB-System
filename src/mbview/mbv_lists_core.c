/*--------------------------------------------------------------------
 *    The MB-system:  mbv_lists_core.c  11/7/2026
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
 * Core functions of the site, route and navigation lists that use no X11 or
 * Motif: the text of the list rows, and the actions on the selected rows
 * (select, delete). The Motif list windows (mbv_ui_motif.c,
 * mbview_callbacks.c) and other toolkits show the rows and pass the selected
 * list positions. The row texts and the selection handling were moved from
 * mbv_ui_motif.c and mbview_callbacks.c.
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

#include "mbv_gl.h"

#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_ui.h"

/*------------------------------------------------------------------------------*/
/* add the text of a row to a list */
int mbview_listrows_add(struct mbview_listrows *rows, const char *text) {
	if (rows->nrows >= rows->nalloc) {
		const int nalloc = rows->nalloc > 0 ? 2 * rows->nalloc : 64;
		char **row = (char **)realloc(rows->row, nalloc * sizeof(char *));
		if (row == NULL)
			return (MB_FAILURE);
		rows->row = row;
		rows->nalloc = nalloc;
	}
	const size_t length = strlen(text) + 1;
	rows->row[rows->nrows] = (char *)malloc(length);
	if (rows->row[rows->nrows] == NULL)
		return (MB_FAILURE);
	memcpy(rows->row[rows->nrows], text, length);
	rows->nrows++;
	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
void mbview_listrows_free(struct mbview_listrows *rows) {
	for (int i = 0; i < rows->nrows; i++)
		free(rows->row[i]);
	free(rows->row);
	rows->row = NULL;
	rows->nrows = 0;
	rows->nalloc = 0;
	rows->selected = 0;
}

/*------------------------------------------------------------------------------*/
/* the rows of the site list (from mbview_updatesitelist): one per active site */
int mbview_sitelist_rows(struct mbview_listrows *rows) {
	memset(rows, 0, sizeof(*rows));
	char value_string[2 * MB_PATH_MAXLINE];

	for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
		if (shared.shareddata.sites[isite].active) {
			/* add list item for each site */
			char londstr0[24], lonmstr0[24];
			char latdstr0[24], latmstr0[24];
			mbview_setlonlatstrings(shared.shareddata.sites[isite].point.xlon, shared.shareddata.sites[isite].point.ylat,
			                        londstr0, latdstr0, lonmstr0, latmstr0);
			if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
				snprintf(value_string, sizeof(value_string), "%3d | %s | %s | %.3f | %s | %d | %s", isite, londstr0, latdstr0,
				         shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
				         shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
			else
				snprintf(value_string, sizeof(value_string), "%3d | %s | %s | %.3f | %s | %d | %s", isite, lonmstr0, latmstr0,
				         shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
				         shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
			if (mbview_listrows_add(rows, value_string) != MB_SUCCESS)
				return (MB_FAILURE);
		}
	}

	/* select list item for selected site - translate the site index to
	    a widget position by counting only active sites up to and
	    including it, keeping this in sync with the active-only rows */
	if (rows->nrows > 0 && shared.shareddata.site_selected != MBV_SELECT_NONE &&
	    shared.shareddata.sites[shared.shareddata.site_selected].active) {
		int iitem = 0;
		for (int isite = 0; isite <= shared.shareddata.site_selected; isite++) {
			if (shared.shareddata.sites[isite].active)
				iitem++;
		}
		rows->selected = iitem;
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the rows of the route list (from mbview_updateroutelist): each active route
    followed by its waypoints */
int mbview_routelist_rows(struct mbview_listrows *rows) {
	memset(rows, 0, sizeof(*rows));
	char value_string[2 * MB_PATH_MAXLINE];
	char londstr0[24], lonmstr0[24];
	char latdstr0[24], latmstr0[24];
	char waypointstr[10];

	for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
		if (shared.shareddata.routes[iroute].active) {
			if (shared.shareddata.routes[iroute].editmode)
				snprintf(value_string, sizeof(value_string), "Editable Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s",
				         iroute, shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
				         shared.shareddata.routes[iroute].distancetopo, mbview_colorname[shared.shareddata.routes[iroute].color],
				         shared.shareddata.routes[iroute].name);
			else
				snprintf(value_string, sizeof(value_string), "Static Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s",
				         iroute, shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
				         shared.shareddata.routes[iroute].distancetopo, mbview_colorname[shared.shareddata.routes[iroute].color],
				         shared.shareddata.routes[iroute].name);
			if (mbview_listrows_add(rows, value_string) != MB_SUCCESS)
				return (MB_FAILURE);

			/* add list item for each waypoint */
			for (int jpoint = 0; jpoint < shared.shareddata.routes[iroute].npoints; jpoint++) {
				mbview_setlonlatstrings(shared.shareddata.routes[iroute].points[jpoint].xlon,
				                        shared.shareddata.routes[iroute].points[jpoint].ylat, londstr0, latdstr0, lonmstr0, latmstr0);

				if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_SIMPLE)
					strcpy(waypointstr, "---------");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_TRANSIT)
					strcpy(waypointstr, "-TRANSIT-");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE)
					strcpy(waypointstr, "--START--");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE)
					strcpy(waypointstr, "---END---");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE2)
					strcpy(waypointstr, "--START2-");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE2)
					strcpy(waypointstr, "---END2--");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE3)
					strcpy(waypointstr, "--START3-");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE3)
					strcpy(waypointstr, "---END3--");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE4)
					strcpy(waypointstr, "--START4-");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE4)
					strcpy(waypointstr, "---END4--");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE5)
					strcpy(waypointstr, "--START5-");
				else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE5)
					strcpy(waypointstr, "---END5--");
				else
					strcpy(waypointstr, "-------");
				if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
					snprintf(value_string, sizeof(value_string), "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint,
					         londstr0, latdstr0, shared.shareddata.routes[iroute].points[jpoint].zdata,
					         shared.shareddata.routes[iroute].distlateral[jpoint], shared.shareddata.routes[iroute].disttopo[jpoint],
					         waypointstr);
				else
					snprintf(value_string, sizeof(value_string), "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint,
					         lonmstr0, latmstr0, shared.shareddata.routes[iroute].points[jpoint].zdata,
					         shared.shareddata.routes[iroute].distlateral[jpoint], shared.shareddata.routes[iroute].disttopo[jpoint],
					         waypointstr);
				if (mbview_listrows_add(rows, value_string) != MB_SUCCESS)
					return (MB_FAILURE);
			}
		}
	}

	/* select list item for selected route - inactive routes contribute no
	    rows, so they are skipped here too */
	if (rows->nrows > 0 && shared.shareddata.route_selected != MBV_SELECT_NONE) {
		int iitem = 0;
		for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
			if (!shared.shareddata.routes[iroute].active)
				continue;
			iitem++;
			if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == MBV_SELECT_ALL)
				rows->selected = iitem;
			for (int jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints; jwaypoint++) {
				iitem++;
				if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == jwaypoint)
					rows->selected = iitem;
			}
		}
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the rows of the navigation list (from mbview_updatenavlist): one per active
    navigation */
int mbview_navlist_rows(struct mbview_listrows *rows) {
	memset(rows, 0, sizeof(*rows));
	char value_string[2 * MB_PATH_MAXLINE];

	for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
		if (shared.shareddata.navs[inav].active) {
			/* add list item for each nav */
			snprintf(value_string, sizeof(value_string), "%3d | %3d | %s | %d | %s", inav, shared.shareddata.navs[inav].npoints,
			         mbview_colorname[shared.shareddata.navs[inav].color], shared.shareddata.navs[inav].size,
			         shared.shareddata.navs[inav].name);
			if (mbview_listrows_add(rows, value_string) != MB_SUCCESS)
				return (MB_FAILURE);
		}
	}

	/* check for completely selected navs */
	/* (the list holds only active navs, so count list positions) */
	if (rows->nrows > 0) {
		int inavselect = MBV_SELECT_NONE;
		int iitem = 0;
		for (int inav = 0; inav < shared.shareddata.nnav && inavselect == MBV_SELECT_NONE; inav++) {
			if (shared.shareddata.navs[inav].active) {
				iitem++;
				if (shared.shareddata.navs[inav].npoints > 1 &&
				    shared.shareddata.navs[inav].nselected == shared.shareddata.navs[inav].npoints) {
					inavselect = inav;
				}
			}
		}
		/* select first item with fully selected nav */
		if (inavselect != MBV_SELECT_NONE)
			rows->selected = iitem;
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
static int mbview_navlist_position_to_inav(int position) {
	int iitem = 0;
	for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
		if (shared.shareddata.navs[inav].active) {
			iitem++;
			if (iitem == position)
				return inav;
		}
	}
	return MBV_SELECT_NONE;
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_sitelistselect() */
int mbview_sitelist_select(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* save last site selection if any */
	int site_selected_old = shared.shareddata.site_selected;

	/* find selected site point if any */
	shared.shareddata.site_selected = MBV_SELECT_NONE;
	if (position_count == 1) {
		shared.shareddata.site_selected = position_list[0] - 1;
	}

	/* change site color if clicked more than once - only if a valid site
	    is selected; with no single selection both values are MBV_SELECT_NONE */
	if (site_selected_old == shared.shareddata.site_selected && shared.shareddata.site_selected >= 0 &&
	    shared.shareddata.site_selected < shared.shareddata.nsite) {
		const int isite = shared.shareddata.site_selected;

		/* increment color */
		shared.shareddata.sites[isite].color++;
		if (shared.shareddata.sites[isite].color == MBV_COLOR_RED)
			shared.shareddata.sites[isite].color++;
		if (shared.shareddata.sites[isite].color > MBV_COLOR_PURPLE)
			shared.shareddata.sites[isite].color = MBV_COLOR_BLACK;

		/* update site list */
		mbview_updatesitelist();
	}

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_SITE;
			mbview_pick_text(i);
			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from %s instance:%zu\n", __func__, instance);
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);
		}
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_sitelist_delete() */
int mbview_sitelist_delete(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* deselect any selected site */
	shared.shareddata.site_selected = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* delete selected site points in reverse order if any */
	for (int i = position_count - 1; i >= 0; i--) {
		const int isite = position_list[i] - 1;
		mbview_site_delete(instance, isite);
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_SITE)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
				mbview_plotlowhigh(i);
				mbview_plotlowhighall(i);
			}
		}

		/* update site list */
		mbview_updatesitelist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_routelistselect() */
int mbview_routelist_select(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* save last route selection if any */
	const int route_selected_old = shared.shareddata.route_selected;
	const int route_point_selected_old = shared.shareddata.route_point_selected;

	/* find selected route point if any */
	shared.shareddata.route_selected = MBV_SELECT_NONE;
	shared.shareddata.route_point_selected = MBV_SELECT_NONE;
	if (position_count == 1) {
		const int iposition = position_list[0] - 1;
		int iroutepos = 0;
		for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
			/* mbview_updateroutelist() skips inactive routes when building the
			    widget's rows, so they must be skipped here too to keep iroutepos
			    in sync with the actual widget position */
			if (!shared.shareddata.routes[iroute].active)
				continue;
			if (iroutepos == iposition) {
				shared.shareddata.route_selected = iroute;
				shared.shareddata.route_point_selected = MBV_SELECT_ALL;
			}
			else if (iroutepos < iposition && iroutepos + shared.shareddata.routes[iroute].npoints >= iposition) {
				shared.shareddata.route_selected = iroute;
				shared.shareddata.route_point_selected = iposition - iroutepos - 1;
			}
			iroutepos += shared.shareddata.routes[iroute].npoints + 1;
		}

		/* change route color if clicked more than once */
		if (route_selected_old == shared.shareddata.route_selected && route_point_selected_old == MBV_SELECT_ALL &&
		    shared.shareddata.route_point_selected == MBV_SELECT_ALL) {
			const int iroute = shared.shareddata.route_selected;

			/* increment color */
			shared.shareddata.routes[iroute].color++;
			if (shared.shareddata.routes[iroute].color == MBV_COLOR_RED)
				shared.shareddata.routes[iroute].color++;
			if (shared.shareddata.routes[iroute].color > MBV_COLOR_PURPLE)
				shared.shareddata.routes[iroute].color = MBV_COLOR_BLACK;

			/* update route list */
			mbview_updateroutelist();
		}

		/* change waypoint type if a waypoint (not a whole route) is clicked more than once */
		if (shared.shareddata.route_selected >= 0 && shared.shareddata.route_point_selected >= 0 &&
		    route_selected_old == shared.shareddata.route_selected &&
		    route_point_selected_old == shared.shareddata.route_point_selected) {
			const int iroute = shared.shareddata.route_selected;
			const int iwaypoint = shared.shareddata.route_point_selected;

			/* increment waypoint type */
			shared.shareddata.routes[iroute].waypoint[iwaypoint]++;
			if (shared.shareddata.routes[iroute].waypoint[iwaypoint] < MBV_ROUTE_WAYPOINT_SIMPLE ||
			    shared.shareddata.routes[iroute].waypoint[iwaypoint] > MBV_ROUTE_WAYPOINT_ENDLINE5)
				shared.shareddata.routes[iroute].waypoint[iwaypoint] = MBV_ROUTE_WAYPOINT_SIMPLE;

			/* update route list */
			mbview_updateroutelist();
		}
	}

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_ROUTE;
			mbview_pick_text(i);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);
		}
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_routelist_delete() */
int mbview_routelist_delete(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* deselect any selected route */
	shared.shareddata.route_selected = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* figure out which routes and waypoints are selected,
	    and flag them for deletion by settting waypoint
	    values to MBV_ROUTE_WAYPOINT_DELETEFLAG */
	for (int i = 0; i < position_count; i++) {
		int iposition = 0;
		bool done = false;
		for (int iroute = 0; iroute < shared.shareddata.nroute && !done; iroute++) {
			/* mbview_updateroutelist() skips inactive routes when building the
			    widget's rows, so they must be skipped here too to keep iposition
			    in sync with the actual widget position */
			if (!shared.shareddata.routes[iroute].active)
				continue;
			iposition++;

			/* delete entire route */
			if (iposition == position_list[i]) {
				for (int jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints; jwaypoint++) {
					shared.shareddata.routes[iroute].waypoint[jwaypoint] = MBV_ROUTE_WAYPOINT_DELETEFLAG;
				}
				done = true;
			} else {
				/* else check waypoints */
				for (int jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints && !done; jwaypoint++) {
					iposition++;
					if (iposition == position_list[i]) {
						shared.shareddata.routes[iroute].waypoint[jwaypoint] = MBV_ROUTE_WAYPOINT_DELETEFLAG;
						done = true;
					}
				}
			}
		}
	}

	/* now loop over all route waypoints backwards, deleting any that have been flagged */
	for (int iroute = shared.shareddata.nroute - 1; iroute >= 0; iroute--) {
		for (int jwaypoint = shared.shareddata.routes[iroute].npoints - 1; jwaypoint >= 0; jwaypoint--) {
			if (shared.shareddata.routes[iroute].waypoint[jwaypoint] == MBV_ROUTE_WAYPOINT_DELETEFLAG)
				mbview_route_delete(instance, iroute, jwaypoint);
		}
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_ROUTE)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
				mbview_plotlowhigh(i);
				mbview_plotlowhighall(i);
			}
		}

		/* update route list */
		mbview_updateroutelist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_navlistselect() */
int mbview_navlist_select(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* first deselect all navigation */
	shared.shareddata.navpick_type = MBV_PICK_NONE;
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
		shared.shareddata.navs[inav].nselected = 0;
		for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
			shared.shareddata.navs[inav].navpts[jpt].selected = false;
		}
	}

	/* now select all nav points in selected files */
	for (int j = 0; j < position_count; j++) {
		int inav = mbview_navlist_position_to_inav(position_list[j]);
		if (inav != MBV_SELECT_NONE && shared.shareddata.navs[inav].npoints > 0) {

			/* Select all nav points in inav */
			for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
				shared.shareddata.navs[inav].navpts[jpt].selected = true;
				shared.shareddata.navs[inav].nselected++;
			}

			/* pick first and last navigation points */
			if (j == 0) {
				shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
				shared.shareddata.nav_selected[0] = inav;
				shared.shareddata.nav_point_selected[0] = 0;
				shared.shareddata.navpick.endpoints[0] =
				    shared.shareddata.navs[inav].navpts[shared.shareddata.nav_point_selected[0]].point;
			}
			if (j == position_count - 1) {
				shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
				shared.shareddata.nav_selected[1] = inav;
				shared.shareddata.nav_point_selected[1] = shared.shareddata.navs[inav].npoints - 1;
				shared.shareddata.navpick.endpoints[1] =
				    shared.shareddata.navs[inav].navpts[shared.shareddata.nav_point_selected[1]].point;
			}
		}
	}

	/* only call it a two point pick if the two points are different */
	if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT &&
	    shared.shareddata.nav_selected[0] == shared.shareddata.nav_selected[1] &&
	    shared.shareddata.nav_point_selected[0] == shared.shareddata.nav_point_selected[1])
		shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* generate 3D drape of pick marks */
			mbview_navpicksize(i);

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_NAV;
			mbview_pick_text(i);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from %s: instance:%zu\n", __func__, i);
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);

			/* extract profile if pick is right type */
			if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
				mbview_extract_nav_profile(i);

			/* now replot profile */
			mbview_plotprofile(i);
		}
	}

	/* call picknav notify if defined */
	if (instance != MBV_NO_WINDOW
    && mbviews[instance].data.mbview_picknav_notify != NULL
    && shared.shareddata.navpick_type != MBV_PICK_NONE) {
		(mbviews[instance].data.mbview_picknav_notify)(instance);
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
/* the selected positions of the list (1 for the first row), from do_mbview_navlist_delete() */
int mbview_navlist_delete(int position_count, int *position_list) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       position_count:   %d\n", position_count);
	}

	/* deselect any selected nav */
	shared.shareddata.navpick_type = MBV_PICK_NONE;
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* delete selected nav points in reverse order if any */
	for (int i = position_count - 1; i >= 0; i--) {
		const int inav = mbview_navlist_position_to_inav(position_list[i]);
		if (inav != MBV_SELECT_NONE)
			mbview_nav_delete(instance, inav);
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
				mbview_plotlowhigh(instance);
				mbview_plotlowhighall(instance);

				/* extract profile if pick is right type */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
					mbview_extract_nav_profile(i);

				/* now replot profile */
				mbview_plotprofile(i);
			}
		}

		/* update nav list */
		mbview_updatenavlist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();

	return (MB_SUCCESS);
}
