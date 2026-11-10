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
#include "mbv_input.h"

/* defined in mbview_callbacks.c */
void do_mbview_reset_view(Widget w, XtPointer client_data, XtPointer call_data);

/* scratch buffers, as in the files the code came from */
static Cardinal ac = 0;
static Arg args[256];

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
	}

	int status = MB_SUCCESS;

	/* update nav list */
	if (shared.init_navlist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);

		/* the rows of the core */
		struct mbview_listrows rows;
		status = mbview_navlist_rows(&rows);
		if (status == MB_SUCCESS)
			mbv_ui_motif_fill_list(shared.mb3d_navlist.mbview_list_navlist, &rows);
		mbview_listrows_free(&rows);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updateroutelist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	int status = MB_SUCCESS;

	/* update route list */
	if (shared.init_routelist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);

		/* the rows of the core */
		struct mbview_listrows rows;
		status = mbview_routelist_rows(&rows);
		if (status == MB_SUCCESS)
			mbv_ui_motif_fill_list(shared.mb3d_routelist.mbview_list_routelist, &rows);
		mbview_listrows_free(&rows);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updatesitelist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	int status = MB_SUCCESS;

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

		/* the rows of the core */
		struct mbview_listrows rows;
		status = mbview_sitelist_rows(&rows);
		if (status == MB_SUCCESS)
			mbv_ui_motif_fill_list(shared.mb3d_sitelist.mbview_list_sitelist, &rows);
		mbview_listrows_free(&rows);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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

/*------------------------------------------------------------------------------*/
void mbv_ui_cursor(size_t instance, int cursor) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	switch (cursor) {
	case MBV_CURSOR_TARGET_BLACK: XDefineCursor(view->dpy, view->xid, view->TargetBlackCursor); break;
	case MBV_CURSOR_TARGET_GREEN: XDefineCursor(view->dpy, view->xid, view->TargetGreenCursor); break;
	case MBV_CURSOR_TARGET_RED: XDefineCursor(view->dpy, view->xid, view->TargetRedCursor); break;
	case MBV_CURSOR_FLEUR_BLACK: XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor); break;
	case MBV_CURSOR_FLEUR_RED: XDefineCursor(view->dpy, view->xid, view->FleurRedCursor); break;
	case MBV_CURSOR_SIZING_BLACK: XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor); break;
	case MBV_CURSOR_SIZING_RED: XDefineCursor(view->dpy, view->xid, view->SizingRedCursor); break;
	default: break;
	}
}

/*------------------------------------------------------------------------------*/
void mbv_ui_view_values_changed(size_t instance, int values) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	switch (values) {
	case MBV_VALUES_VIEW_2DOFFSET:
		if (XtIsManaged(view->mb3dview.mbview_textField_view_2doffsetx)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", view->offset2d_x);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsetx, value_text);
			snprintf(value_text, sizeof(value_text), "%g", view->offset2d_y);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsety, value_text);
		}
		break;
	case MBV_VALUES_VIEW_3DOFFSET:
		if (XtIsManaged(view->mb3dview.mbview_textField_view_3doffsetx)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", view->offset3d_x);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsetx, value_text);
			snprintf(value_text, sizeof(value_text), "%g", view->offset3d_y);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsety, value_text);
		}
		break;
	case MBV_VALUES_MODEL_ORIENTATION:
		if (XtIsManaged(view->mb3dview.mbview_textField_model_azimuth)) {
            mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->modelazimuth3d);
			XmTextFieldSetString(view->mb3dview.mbview_textField_model_azimuth, value_text);
			snprintf(value_text, sizeof(value_text), "%g", data->modelelevation3d);
			XmTextFieldSetString(view->mb3dview.mbview_textField_model_elevation, value_text);
		}
		break;
	case MBV_VALUES_ILLUMINATION:
		if (XtIsManaged(view->mb3dview.mbview_textField_illum_azi)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->illuminate_azimuth);
			XmTextFieldSetString(view->mb3dview.mbview_textField_illum_azi, value_text);
			snprintf(value_text, sizeof(value_text), "%g", data->illuminate_elevation);
			XmTextFieldSetString(view->mb3dview.mbview_textField_illum_elev, value_text);
		}
		break;
	case MBV_VALUES_VIEW_ORIENTATION:
		if (XtIsManaged(view->mb3dview.mbview_textField_view_azimuth)) {
            mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->viewazimuth3d);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_azimuth, value_text);
			snprintf(value_text, sizeof(value_text), "%g", data->viewelevation3d);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_elevation, value_text);
		}
		break;
	case MBV_VALUES_VIEW_2DZOOM:
		if (XtIsManaged(view->mb3dview.mbview_textField_view_2dzoom)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", view->size2d);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_2dzoom, value_text);
		}
		break;
	case MBV_VALUES_MODEL_3DZOOM:
		if (XtIsManaged(view->mb3dview.mbview_textField_model_3dzoom)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", view->offset3d_z);
			XmTextFieldSetString(view->mb3dview.mbview_textField_model_3dzoom, value_text);
		}
		break;
	case MBV_VALUES_EXAGGERATION:
		if (XtIsManaged(view->mb3dview.mbview_textField_exaggeration)) {
            mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->exaggeration);
			XmTextFieldSetString(view->mb3dview.mbview_textField_exaggeration, value_text);
		}
		break;
	case MBV_VALUES_ILLUMINATION_AMP:
		if (XtIsManaged(view->mb3dview.mbview_textField_illum_amp)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->illuminate_magnitude);
			XmTextFieldSetString(view->mb3dview.mbview_textField_illum_amp, value_text);
		}
		break;
	case MBV_VALUES_SLOPE_AMP:
		if (XtIsManaged(view->mb3dview.mbview_textField_slope_amp)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->slope_magnitude);
			XmTextFieldSetString(view->mb3dview.mbview_textField_slope_amp, value_text);
		}
		break;
	case MBV_VALUES_OVERLAY_AMP:
		if (XtIsManaged(view->mb3dview.mbview_textField_overlay_amp)) {
              mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", data->overlay_shade_magnitude);
			XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, value_text);
		}
		break;
	case MBV_VALUES_VIEW_3DZOOM:
		if (XtIsManaged(view->mb3dview.mbview_textField_view_3dzoom)) {
			mb_path value_text;
			snprintf(value_text, sizeof(value_text), "%g", view->viewoffset3d_z);
			XmTextFieldSetString(view->mb3dview.mbview_textField_view_3dzoom, value_text);
		}
		break;
	case MBV_VALUES_RESOLUTION:
		/* set values of resolution sliders */
		XtVaSetValues(view->mb3dview.mbview_scale_lowresolution, XmNvalue, data->lorez_dimension, NULL);
		XtVaSetValues(view->mb3dview.mbview_scale_mediumresolution, XmNvalue, data->hirez_dimension, NULL);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------------*/
void mbv_ui_reset_view(size_t instance) {
	/* the input callback passed its own widget, the map drawing area, whose
	   user data is the instance; the other arguments are unused */
	do_mbview_reset_view(mbviews[instance].glwmda, NULL, NULL);
}

/*------------------------------------------------------------------------------*/
void mbv_input_from_xevent(XEvent *xevent, struct mbv_input_event *event) {
	memset(event, 0, sizeof(struct mbv_input_event));
	switch (xevent->xany.type) {
	case ButtonPress:
	case ButtonRelease:
		event->type = xevent->xany.type == ButtonPress ? MBV_INPUT_PRESS : MBV_INPUT_RELEASE;
		event->button = xevent->xbutton.button;
		event->x = xevent->xbutton.x;
		event->y = xevent->xbutton.y;
		break;
	case MotionNotify:
		event->type = MBV_INPUT_MOTION;
		event->x = xevent->xmotion.x;
		event->y = xevent->xmotion.y;
		break;
	case KeyPress:
	case KeyRelease: {
		event->type = xevent->xany.type == KeyPress ? MBV_INPUT_KEYPRESS : MBV_INPUT_KEYRELEASE;
		char buffer[2] = {0};
		KeySym keysym;
		const int actual = XLookupString((XKeyEvent *)xevent, buffer, 1, &keysym, NULL);
		event->key = actual > 0 ? buffer[0] : '\0';
	} break;
	case Expose:
	case GraphicsExpose:
		event->type = MBV_INPUT_EXPOSE;
		break;
	default:
		event->type = MBV_INPUT_OTHER;
		break;
	}
}

/*------------------------------------------------------------------------------*/
/* destroy the window of an instance (mbview_destroy) */
void mbv_ui_destroy_window(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	XtDestroyWidget(view->topLevelShell);
}

/*------------------------------------------------------------------------------*/
/* clear and close the site, route, navigation and GIS list windows when the last
    instance is destroyed (mbview_destroy) */
void mbv_ui_close_lists(void) {
	XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);
	XtPopdown(XtParent(shared.mainWindow_sitelist));

	XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);
	XtPopdown(XtParent(shared.mainWindow_routelist));

	XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);
	XtPopdown(XtParent(shared.mainWindow_navlist));

	mbview_gislist_popdownall();
}

/*------------------------------------------------------------------------------*/
/* show the rows of the core in a Motif list (the list is empty) */
void mbv_ui_motif_fill_list(Widget list, const struct mbview_listrows *rows) {
	if (rows->nrows <= 0)
		return;

	/* allocate array of label XmStrings */
	XmString *xstr = (XmString *)malloc(rows->nrows * sizeof(XmString));
	if (xstr == NULL)
		return;
	for (int iitem = 0; iitem < rows->nrows; iitem++)
		xstr[iitem] = XmStringCreateLocalized(rows->row[iitem]);

	/* add list items, select the item for the selected entry */
	XmListAddItems(list, xstr, rows->nrows, 0);
	if (rows->selected > 0) {
		XmListSelectPos(list, rows->selected, 0);
		XmListSetPos(list, MAX(rows->selected - 5, 1));
	}

	/* deallocate memory no longer needed */
	for (int iitem = 0; iitem < rows->nrows; iitem++)
		XmStringFree(xstr[iitem]);
	free(xstr);
}
