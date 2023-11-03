/*--------------------------------------------------------------------
 *    The MB-system:	mbpingedit_callbacks.c		11/13/2007
 *
 *    Copyright (c) 2007-2023 by
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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_status.h"
#include "mb_define.h"

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
#include "Mb3dsdg.h"

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

/* Set flag to define mbpingedit global variables in this code block */
#define MBPINGEDITGLOBAL

#include "mbview.h"
#include "mbpingeditprivate.h"

// Widget BxFindTopShell(Widget);
WidgetList BxWidgetIdsFromNames(Widget, char *, char *);

int mbpingedit_startup(int verbose, Widget parent, XtAppContext app, int *error) {
	mbp_verbose = verbose;

	if (mbp_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr, "dbg2       parent:                  %p\n", parent);
		fprintf(stderr, "dbg2       app:                     %p\n", app);
	}

	/* set parent widget and app context */
	mbp_parent_widget = parent;
	mbp_app_context = app;
	mbp_work_function_set = false;
	mbp_timer_count = 0;

	/* initialize window */
	/*mbpingedit_reset();*/

	/* set error */
	*error = mbp_error;

	if (mbp_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", mbp_status);
	}

	return (mbp_status);
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_toggle(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_show_detects(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_next_buffer(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_scale_x(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_scale_y(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_check_median_ltrack(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_check_median_xtrack(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_grab(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_erase(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_show_flagged(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_reverse(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_number_step(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_set_filters(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_view_mode(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_reset_filters(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_x_interval(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_y_interval(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_info(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_unflag_all(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_flag_view(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*      Function Name: 	BxManageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxManageCB attempts to convert the name to a Widget
 *			ID and manage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and manage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */
void BxManageCB(Widget w, XtPointer client, XtPointer call) {
	(void)call;  // Unused parameter
	/*
	 * This function returns a NULL terminated WidgetList.  The memory for
	 * the list needs to be freed when it is no longer needed.
	 */
	WidgetList widgets = BxWidgetIdsFromNames(w, "BxManageCB", (String)client);

	int i = 0;
	while (widgets && widgets[i] != NULL) {
		XtManageChild(widgets[i]);
		i++;
	}
	XtFree((char *)widgets);
}

/*      Function Name: 	BxUnmanageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxUnmanageCB attempts to convert the name to a Widget
 *			ID and unmanage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and unmanage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */
void BxUnmanageCB(Widget w, XtPointer client, XtPointer call) {
	(void)call;  // Unused parameter
	/*
	 * This function returns a NULL terminated WidgetList.  The memory for
	 * the list needs to be freed when it is no longer needed.
	 */
	WidgetList widgets = BxWidgetIdsFromNames(w, "BxUnmanageCB", (String)client);

	int i = 0;
	while (widgets && widgets[i] != NULL) {
		XtUnmanageChild(widgets[i]);
		i++;
	}
	XtFree((char *)widgets);
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_reverse_keys(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_event(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_number_pings(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_forward(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_show_time(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_restore(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_reverse_mouse(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_mode_pick(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/

void do_mbpingedit_unflag_view(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
}

/*------------------------------------------------------------------------------*/
