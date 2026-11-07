/*--------------------------------------------------------------------
 *    The MB-system:  mbv_ui_motif.c  11/7/2026
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
 * Motif user interface functions called by the mbview core (see mbv_ui.h).
 * The code was moved unchanged from the core files: the list updates from
 * mbview_nav.c, mbview_route.c and mbview_site.c, mbview_reset_prglx()
 * from mbview_profile.c, the secondary grid widget updates from
 * mbview_secondary.c, and the XBell() calls of the pick functions.
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
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

#include "mbv_gl.h"
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_ui.h"

/* scratch buffers, as in the files the code came from */
static Cardinal ac = 0;
static Arg args[256];
static char value_string[2*MB_PATH_MAXLINE];

/*------------------------------------------------------------------------------*/
void mbv_ui_bell(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	XBell(view->dpy, 100);
}

/*------------------------------------------------------------------------------*/
void mbv_ui_secondary_colortable_changed(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	char value_text[MB_PATH_MAXLINE];

	/* set secondary color control widgets if managed */
	if (XtIsManaged(view->mb3dview.mbview_textField_overlaymin)) {
		snprintf(value_text, sizeof(value_text), "%g", data->secondary_colortable_min);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymin, value_text);
		snprintf(value_text, sizeof(value_text), "%g", data->secondary_colortable_max);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymax, value_text);
		if (data->secondary_colortable_mode == MBV_COLORTABLE_NORMAL) {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_ctoh, TRUE, TRUE);
		}
		else {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_htoc, TRUE, TRUE);
		}
	}
	if (XtIsManaged(view->mb3dview.mbview_textField_overlay_amp)) {
		snprintf(value_text, sizeof(value_text), "%g", data->overlay_shade_magnitude);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, value_text);
		snprintf(value_text, sizeof(value_text), "%g", data->overlay_shade_center);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_center, value_text);
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL) {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, TRUE, TRUE);
		}
		else {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, TRUE, TRUE);
		}
	}
}

/*------------------------------------------------------------------------------*/
void mbv_ui_secondary_name_changed(size_t instance, char *name) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	char value_text[MB_PATH_MAXLINE];

	/* set secondary grid labels */
	if (XtIsManaged(view->mb3dview.mbview_toggleButton_data_secondary)) {
		Boolean argok = False;
		Cardinal ac = 0;
		XmString tmp0 = (XmString)BX_CONVERT(view->mb3dview.mbview_toggleButton_data_secondary, (char *)name, XmRXmString, 0, &argok);
		Arg args[256];
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_data_secondary, args, ac);
		XmStringFree((XmString)tmp0);

		ac = 0;
		snprintf(value_text, sizeof(value_text), "Shading by %s", name);
		tmp0 = (XmString)BX_CONVERT(view->mb3dview.mbview_toggleButton_overlay_secondary, (char *)value_text, XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_secondary, args, ac);
		XmStringFree((XmString)tmp0);
	}
}

/*------------------------------------------------------------------------------*/
int mbview_updatenavlist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update nav list */
	if (shared.init_navlist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);

		if (shared.shareddata.nnav > 0) {
			/* get number of items */
			int nitems = 0;
			for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
        if (shared.shareddata.navs[inav].active) {
				  nitems += 1;
        }
			}

      if (nitems > 0) {

  			/* allocate array of label XmStrings */
  			XmString *xstr = (XmString *)malloc(nitems * sizeof(XmString));
  			if (xstr == NULL)
  				return (MB_FAILURE);

  			/* loop over the navs */
  			nitems = 0;
  			for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
          if (shared.shareddata.navs[inav].active) {
    				/* add list item for each nav */
    				sprintf(value_string, "%3d | %3d | %s | %d | %s", inav, shared.shareddata.navs[inav].npoints,
    				        mbview_colorname[shared.shareddata.navs[inav].color], shared.shareddata.navs[inav].size,
    				        shared.shareddata.navs[inav].name);
    				xstr[nitems] = XmStringCreateLocalized(value_string);
    				nitems++;
          }
  			}

  			/* add list items */
  			XmListAddItems(shared.mb3d_navlist.mbview_list_navlist, xstr, nitems, 0);

  			/* deallocate memory no longer needed */
  			for (int iitem = 0; iitem < nitems; iitem++) {
  				XmStringFree(xstr[iitem]);
  			}
  			free(xstr);

  			/* check for completely selected navs */
  			/* (the list holds only active navs, so count list positions) */
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
  			if (inavselect != MBV_SELECT_NONE) {
  				XmListSelectPos(shared.mb3d_navlist.mbview_list_navlist, iitem, 0);
  				XmListSetPos(shared.mb3d_navlist.mbview_list_navlist, MAX(iitem - 5, 1));
  			}

      }
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updateroutelist() {
	/* local variables */
	int status = MB_SUCCESS;
	XmString *xstr;
	int jpoint;
	int nitems;
	int iitem;
	char londstr0[24], lonmstr0[24];
	char latdstr0[24], latmstr0[24];
	char waypointstr[10];
	int iroute, jwaypoint;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update route list */
	if (shared.init_routelist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);

		if (shared.shareddata.nroute > 0) {
			/* get number of items */
			nitems = 0;
			for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
        if (shared.shareddata.routes[iroute].active) {
  				nitems += 1 + shared.shareddata.routes[iroute].npoints;
        }
			}

      if (nitems > 0) {
  			/* allocate array of label XmStrings */
  			xstr = (XmString *)malloc(nitems * sizeof(XmString));
  			if (xstr == NULL)
  				return (MB_FAILURE);

  			/* loop over the routes */
  			nitems = 0;
  			for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
          if (shared.shareddata.routes[iroute].active) {
    				if (shared.shareddata.routes[iroute].editmode)
    					sprintf(value_string, "Editable Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s", iroute,
    					        shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
    					        shared.shareddata.routes[iroute].distancetopo,
    					        mbview_colorname[shared.shareddata.routes[iroute].color], shared.shareddata.routes[iroute].name);
    				else
    					sprintf(value_string, "Static Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s", iroute,
    					        shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
    					        shared.shareddata.routes[iroute].distancetopo,
    					        mbview_colorname[shared.shareddata.routes[iroute].color], shared.shareddata.routes[iroute].name);
    				xstr[nitems] = XmStringCreateLocalized(value_string);
    				nitems++;

    				/* add list item for each waypoint */
    				for (jpoint = 0; jpoint < shared.shareddata.routes[iroute].npoints; jpoint++) {
    					/* add list item for each route */
    					mbview_setlonlatstrings(shared.shareddata.routes[iroute].points[jpoint].xlon,
    					                        shared.shareddata.routes[iroute].points[jpoint].ylat, londstr0, latdstr0, lonmstr0,
    					                        latmstr0);

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
    						sprintf(value_string, "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint, londstr0, latdstr0,
    						        shared.shareddata.routes[iroute].points[jpoint].zdata,
    						        shared.shareddata.routes[iroute].distlateral[jpoint],
    						        shared.shareddata.routes[iroute].disttopo[jpoint], waypointstr);
    					else
    						sprintf(value_string, "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint, lonmstr0, latmstr0,
    						        shared.shareddata.routes[iroute].points[jpoint].zdata,
    						        shared.shareddata.routes[iroute].distlateral[jpoint],
    						        shared.shareddata.routes[iroute].disttopo[jpoint], waypointstr);
    					xstr[nitems] = XmStringCreateLocalized(value_string);
    					nitems++;
    				}
          }
  			}

  			/* add list items */
  			XmListAddItems(shared.mb3d_routelist.mbview_list_routelist, xstr, nitems, 0);

  			/* select list item for selected route */
  			if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
    				/* get item number - inactive routes contribute no rows to the
    				    widget (see the nitems loops above), so they must be skipped
    				    here too to keep iitem in sync with the actual widget position */
    				iitem = 0;
    				for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
    					if (!shared.shareddata.routes[iroute].active)
    						continue;
    					iitem++;
    					if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == MBV_SELECT_ALL) {
    						XmListSelectPos(shared.mb3d_routelist.mbview_list_routelist, iitem, 0);
    						XmListSetPos(shared.mb3d_routelist.mbview_list_routelist, MAX(iitem - 5, 1));
    					}
    					for (jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints; jwaypoint++) {
    						iitem++;
    						if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == jwaypoint) {
    							XmListSelectPos(shared.mb3d_routelist.mbview_list_routelist, iitem, 0);
    							XmListSetPos(shared.mb3d_routelist.mbview_list_routelist, MAX(iitem - 5, 1));
    						}
    					}
    				}
        }

  			/* deallocate memory no longer needed */
  			for (iitem = 0; iitem < nitems; iitem++) {
  				XmStringFree(xstr[iitem]);
  			}
  			free(xstr);
      }
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updatesitelist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update site list */
	if (shared.init_sitelist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);

  	/* print site debug statements */
  	if (mbv_verbose >= 2) {
  		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
  		fprintf(stderr, "dbg2  Site values:\n");
  		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
  		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
  		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
  		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
  		for (int i = 0; i < shared.shareddata.nsite; i++) {
  			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
  			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
  			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
  			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
  			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
  			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
  			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
  		}
  	}

		if (shared.shareddata.nsite > 0) {
			/* count active sites - inactive sites (currently only possible in
			    unused tail slots beyond nsite, but checked defensively here
			    since nothing else in this file assumes that invariant) contribute
			    no row to the widget */
			int nitems = 0;
			for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
				if (shared.shareddata.sites[isite].active)
					nitems++;
			}

		if (nitems > 0) {
			/* allocate array of label XmStrings, sized to only the active sites
			    actually being added, so every slot gets initialized below */
			XmString *xstr = (XmString *)malloc(nitems * sizeof(XmString));
			if (xstr == NULL)
				return (MB_FAILURE);

			/* loop over the sites */
			nitems = 0;
			for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
        if (shared.shareddata.sites[isite].active) {
  				/* add list item for each site */
  				char londstr0[24], lonmstr0[24];
  				char latdstr0[24], latmstr0[24];
  				mbview_setlonlatstrings(shared.shareddata.sites[isite].point.xlon, shared.shareddata.sites[isite].point.ylat,
  				                        londstr0, latdstr0, lonmstr0, latmstr0);
  				if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
  					sprintf(value_string, "%3d | %s | %s | %.3f | %s | %d | %s", isite, londstr0, latdstr0,
  					        shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
  					        shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
  				else
  					sprintf(value_string, "%3d | %s | %s | %.3f | %s | %d | %s", isite, lonmstr0, latmstr0,
  					        shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
  					        shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
  				xstr[nitems] = XmStringCreateLocalized(value_string);
  				nitems++;
  			}
      }

			/* add list items */
			XmListAddItems(shared.mb3d_sitelist.mbview_list_sitelist, xstr, nitems, 0);

			/* select list item for selected site - translate the site index to
			    a widget position by counting only active sites up to and
			    including it, keeping this in sync with the active-only rows
			    just added above */
			if (shared.shareddata.site_selected != MBV_SELECT_NONE &&
			    shared.shareddata.sites[shared.shareddata.site_selected].active) {
				int iitem = 0;
				for (int isite = 0; isite <= shared.shareddata.site_selected; isite++) {
					if (shared.shareddata.sites[isite].active)
						iitem++;
				}
				XmListSelectPos(shared.mb3d_sitelist.mbview_list_sitelist, iitem, 0);
				XmListSetPos(shared.mb3d_sitelist.mbview_list_sitelist, MAX(iitem - 5, 1));
			}

			/* deallocate memory no longer needed */
			for (int iitem = 0; iitem < nitems; iitem++) {
				XmStringFree(xstr[iitem]);
			}
			free(xstr);
		}
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_reset_prglx(size_t instance) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	Dimension scrolledWindow_width;
	Dimension scrolledWindow_height;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* If profile view enabled update opengl plotting widget */
	if (data->profile_view_mode == MBV_VIEW_ON) {
		/* delete old glx_context if it exists */
		if (view->prglx_init) {
#ifdef MBV_DEBUG_GLX
			fprintf(stderr, "%s:%d:%s instance:%zu glXDestroyContext(%p,%p)\n", __FILE__, __LINE__, __func__, instance, view->dpy, view->prglx_context);
#endif
			mbv_render_context_destroy(MBV_RENDER_PROFILE, instance);
#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
			view->prglx_init = false;
		}

		/* get and set sizes of the drawingArea */
		XtVaGetValues(view->mb3dview.mbview_scrolledWindow_profile, XmNwidth, &scrolledWindow_width, XmNheight,
		              &scrolledWindow_height, NULL);
		data->prheight = scrolledWindow_height - 35;
		data->prwidth = data->profile_widthfactor * (scrolledWindow_width - 20);

		/* set drawing area size */
		ac = 0;
		XtSetArg(args[ac], XmNwidth, data->prwidth);
		ac++;
		XtSetArg(args[ac], XmNheight, data->prheight);
		ac++;
		XtSetValues(view->mb3dview.mbview_drawingArea_profile, args, ac);
		ac = 0;

		/* set prglwda size */
		XtSetArg(args[ac], XmNwidth, data->prwidth);
		ac++;
		XtSetArg(args[ac], XmNheight, data->prheight);
		ac++;
		XtSetValues(view->prglwmda, args, ac);

		/* set up a new opengl context */
		mbv_render_context_create(MBV_RENDER_PROFILE, instance);
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%p,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->prglwmda), view->prglx_context);
#endif
		mbv_render_make_current(MBV_RENDER_PROFILE, instance);

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
		view->prglx_init = true;
		glViewport(0, 0, data->prwidth, data->prheight);
		view->praspect_ratio = ((float)data->prheight) / ((float)data->prwidth);
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
		fprintf(stderr, "dbg2       view->dpy:             %p\n", view->dpy);
		fprintf(stderr, "dbg2       view->prvi:            %p\n", view->prvi);
		fprintf(stderr, "dbg2       view->prglwmda:        %p\n", view->prglwmda);
		fprintf(stderr, "dbg2       view->prglx_context:   %p\n", view->prglx_context);
		fprintf(stderr, "dbg2       view->prglx_init:      %d\n", view->prglx_init);
	}

	/* return */
	return (status);
}

