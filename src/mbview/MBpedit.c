/*------------------------------------------------------------------------------
 *    The MB-system:	MBpedit.c	10/28/2003
 *
 *    Copyright (c) 2003-2025 by
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

#include <stdbool.h>

#ifndef SANS
#define SANS "helvetica"
#endif
#ifndef SERIF
#define SERIF "times"
#endif
#ifndef MONO
#define MONO "courier"
#endif

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/BulletinB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/DrawingA.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/BulletinB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/DrawingA.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/ScrolledW.h>
#include "MBpedit.h"

/**
 * Common constant and pixmap declarations.
 */
#include "mbpingedit_creation.h"

/**
 * Convenience functions from utilities file.
 */
void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/**
 * Declarations for callbacks and handlers.
 */
void do_mbpingedit_event(Widget, XtPointer, XtPointer);
void do_mbpingedit_expose(Widget, XtPointer, XtPointer);
void do_mbpingedit_flag_view(Widget, XtPointer, XtPointer);
void do_mbpingedit_unflag_all(Widget, XtPointer, XtPointer);
void do_mbpingedit_unflag_view(Widget, XtPointer, XtPointer);
void do_mbpingedit_next_buffer(Widget, XtPointer, XtPointer);
void do_mbpingedit_dismiss(Widget, XtPointer, XtPointer);
void do_mbpingedit_forward(Widget, XtPointer, XtPointer);
void do_mbpingedit_reverse(Widget, XtPointer, XtPointer);
void do_mbpingedit_scale_x(Widget, XtPointer, XtPointer);
void do_mbpingedit_scale_y(Widget, XtPointer, XtPointer);
void do_mbpingedit_number_pings(Widget, XtPointer, XtPointer);
void do_mbpingedit_number_step(Widget, XtPointer, XtPointer);
void do_mbpingedit_view_mode(Widget, XtPointer, XtPointer);
void do_mbpingedit_show_flagged(Widget, XtPointer, XtPointer);
void do_mbpingedit_show_detects(Widget, XtPointer, XtPointer);
void do_mbpingedit_show_time(Widget, XtPointer, XtPointer);
void BxManageCB(Widget, XtPointer, XtPointer);
void do_mbpingedit_reverse_keys(Widget, XtPointer, XtPointer);
void do_mbpingedit_reverse_mouse(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_toggle(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_pick(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_erase(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_restore(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_grab(Widget, XtPointer, XtPointer);
void do_mbpingedit_mode_info(Widget, XtPointer, XtPointer);
void do_mbpingedit_reset_filters(Widget, XtPointer, XtPointer);
void do_mbpingedit_set_filters(Widget, XtPointer, XtPointer);
void BxUnmanageCB(Widget, XtPointer, XtPointer);
void do_mbpingedit_check_median_ltrack(Widget, XtPointer, XtPointer);
void do_mbpingedit_check_median_xtrack(Widget, XtPointer, XtPointer);
void do_mbpingedit_y_interval(Widget, XtPointer, XtPointer);
void do_mbpingedit_x_interval(Widget, XtPointer, XtPointer);

/*
 * This table is used to define class resources that are placed
 * in app-defaults. This table is necessary so each instance
 * of this class has the proper default resource values set.
 * This eliminates the need for each instance to have
 * its own app-defaults values. This table must be NULL terminated.
 */
typedef struct _UIAppDefault {
	char *cName;     /* Class name */
	char *wName;     /* Widget name */
	char *cInstName; /* Name of class instance (nested class) */
	char *wRsc;      /* Widget resource */
	char *value;     /* value read from app-defaults */
} UIAppDefault;

static bool doInitAppDefaults = true;
static UIAppDefault appDefaults[] = {{NULL, NULL, NULL, NULL, NULL}};
/*
 * The functions to call in the apputils.c
 */
void InitAppDefaults(Widget, UIAppDefault *);
void SetAppDefaults(Widget, UIAppDefault *, char *, Boolean);

MBpeditDataPtr MBpeditCreate(MBpeditDataPtr class_in, Widget parent, String name, ArgList args_in, Cardinal ac_in) {
	(void)args_in;  // Unused parameter
	(void)ac_in;  // Unused parameter

	// Register the converters for the widgets.
	RegisterBxConverters(XtWidgetToApplicationContext(parent));
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmBulletinBoardWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmPushButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmCascadeButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmToggleButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmSeparatorWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmLabelWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScaleWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDrawingAreaWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDialogShellWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScrolledWindowWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	/**
	 * Setup app-defaults fallback table if not already done.
	 */
	if (doInitAppDefaults) {
		InitAppDefaults(parent, appDefaults);
		doInitAppDefaults = False;
	}
	/**
	 * Now set the app-defaults for this instance.
	 */
	SetAppDefaults(parent, appDefaults, name, False);

	Cardinal ac = 0;
	Arg args[256];
	Boolean argok = False;
	{
		XmString tmp0 = (XmString)BX_CONVERT(parent, (char *)"MBeditviz Swath View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 452);
		ac++;
		XtSetArg(args[ac], XmNy, 354);
		ac++;
		XtSetArg(args[ac], XmNwidth, 1004);
		ac++;
		XtSetArg(args[ac], XmNheight, 694);
		ac++;
		class_in->MBpedit = XmCreateForm(parent, (char *)name, args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNmarginHeight, 0);
	ac++;
	XtSetArg(args[ac], XmNmarginWidth, 0);
	ac++;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1005);
	ac++;
	XtSetArg(args[ac], XmNheight, 154);
	ac++;
	class_in->mbpingedit_controls = XmCreateBulletinBoard(class_in->MBpedit, (char *)"mbpingedit_controls", args, ac);
	XtManageChild(class_in->mbpingedit_controls);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Flag View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 480);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_flag_view =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_flag_view", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_flag_view);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_flag_view, XmNactivateCallback, do_mbpingedit_flag_view, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 58);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	class_in->mbpingedit_menuBar_view =
	    XmCreateMenuBar(class_in->mbpingedit_controls, (char *)"mbpingedit_menuBar_view", args, ac);
	XtManageChild(class_in->mbpingedit_menuBar_view);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_menuBar_view, (char *)"View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 48);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_menuBar_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_cascadeButton_view =
		    XmCreateCascadeButton(class_in->mbpingedit_menuBar_view, (char *)"mbpingedit_cascadeButton_view", args, ac);
		XtManageChild(class_in->mbpingedit_cascadeButton_view);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 247);
	ac++;
	XtSetArg(args[ac], XmNheight, 440);
	ac++;
	class_in->mbpingedit_pulldownMenu_view =
	    XmCreatePulldownMenu(XtParent(class_in->mbpingedit_cascadeButton_view), (char *)"mbpingedit_pulldownMenu_view", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Waterfall View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_view_waterfall = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_view_waterfall", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_view_waterfall);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_view_waterfall, XmNvalueChangedCallback, do_mbpingedit_view_mode,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Alongtrack View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_view_alongtrack = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_view_alongtrack", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_view_alongtrack);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_view_alongtrack, XmNvalueChangedCallback, do_mbpingedit_view_mode,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Acrosstrack View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_view_acrosstrack = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_view_acrosstrack", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_view_acrosstrack);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_view_acrosstrack, XmNvalueChangedCallback, do_mbpingedit_view_mode,
	              (XtPointer)0);

	ac = 0;
	class_in->mbpingedit_separator2 =
	    XmCreateSeparator(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_separator2", args, ac);
	XtManageChild(class_in->mbpingedit_separator2);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Show Flagged Profile", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_flagged_on = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_flagged_on", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_flagged_on);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_flagged_on, XmNvalueChangedCallback, do_mbpingedit_show_flagged,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Show Bottom Detect Algorithms", XmRXmString,
		                            0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_detects = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_detects", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_detects);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_detects, XmNvalueChangedCallback, do_mbpingedit_show_detects,
	              (XtPointer)0);

	ac = 0;
	class_in->mbpingedit_separator9 =
	    XmCreateSeparator(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_separator9", args, ac);
	XtManageChild(class_in->mbpingedit_separator9);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Wide Bathymetry Profiles", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_wideplot = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_wideplot", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_wideplot);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_wideplot, XmNvalueChangedCallback, do_mbpingedit_show_time,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Print Time Stamps", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_time =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_time", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_time);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_time, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Ping Interval", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_interval = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_interval", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_interval);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_interval, XmNvalueChangedCallback, do_mbpingedit_show_time,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Longitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_lon =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_lon", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_lon);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_lon, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Latitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_latitude = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_latitude", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_latitude);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_latitude, XmNvalueChangedCallback, do_mbpingedit_show_time,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Heading", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_heading = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_heading", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_heading);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_heading, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Speed", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_speed =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_speed", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_speed);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_speed, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Center Beam Depth", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_depth =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_depth", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_depth);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_depth, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Sonar Altitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_altitude = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_altitude", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_altitude);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_altitude, XmNvalueChangedCallback, do_mbpingedit_show_time,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Sonar Depth", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_sensordepth = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_sensordepth", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_sensordepth);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_sensordepth, XmNvalueChangedCallback, do_mbpingedit_show_time,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Roll", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_roll =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_roll", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_roll);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_roll, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Pitch", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_pitch =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_pitch", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_pitch);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_pitch, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"Plot Heave", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_show_heave =
		    XmCreateToggleButton(class_in->mbpingedit_pulldownMenu_view, (char *)"mbpingedit_toggleButton_show_heave", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_show_heave);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_show_heave, XmNvalueChangedCallback, do_mbpingedit_show_time, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->mbpingedit_pulldownMenu_view);
	ac++;
	XtSetValues(class_in->mbpingedit_cascadeButton_view, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Unflag Forward", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 730);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_unflag_all =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_unflag_all", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_unflag_all);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_unflag_all, XmNactivateCallback, do_mbpingedit_unflag_all, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Unflag View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 610);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_unflag_view =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_unflag_view", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_unflag_view);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_unflag_view, XmNactivateCallback, do_mbpingedit_unflag_view, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 80);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 87);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	class_in->mbpingedit_menuBar_controls =
	    XmCreateMenuBar(class_in->mbpingedit_controls, (char *)"mbpingedit_menuBar_controls", args, ac);
	XtManageChild(class_in->mbpingedit_menuBar_controls);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_menuBar_controls, (char *)"Controls", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 77);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_menuBar_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_cascadeButton_controls =
		    XmCreateCascadeButton(class_in->mbpingedit_menuBar_controls, (char *)"mbpingedit_cascadeButton_controls", args, ac);
		XtManageChild(class_in->mbpingedit_cascadeButton_controls);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 240);
	ac++;
	XtSetArg(args[ac], XmNheight, 150);
	ac++;
	class_in->mbpingedit_pulldownMenu_controls = XmCreatePulldownMenu(XtParent(class_in->mbpingedit_cascadeButton_controls),
	                                                                  (char *)"mbpingedit_pulldownMenu_controls", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Go To Specified Time...", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_goto =
		    XmCreatePushButton(class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_pushButton_goto", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_goto);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_goto, XmNactivateCallback, BxManageCB,
	              (XtPointer) "mbpingedit_bulletinBoard_goto");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Buffer Controls...", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_buffer =
		    XmCreatePushButton(class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_pushButton_buffer", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_buffer);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_buffer, XmNactivateCallback, BxManageCB,
	              (XtPointer) "mbpingedit_bulletinBoard_buffer");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Annotation...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_annotation =
		    XmCreatePushButton(class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_pushButton_annotation", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_annotation);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_annotation, XmNactivateCallback, BxManageCB,
	              (XtPointer) "mbpingedit_bulletinBoard_annotation");

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Filters...", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_filters =
		    XmCreatePushButton(class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_pushButton_filters", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_filters);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_filters, XmNactivateCallback, BxManageCB,
	              (XtPointer) "mbpingedit_bulletinBoard_filters");

	ac = 0;
	class_in->mbpingedit_separator7 =
	    XmCreateSeparator(class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_separator7", args, ac);
	XtManageChild(class_in->mbpingedit_separator7);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Reverse Right/Left Key Macros",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_reverse_keys = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_toggleButton_reverse_keys", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_reverse_keys);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_reverse_keys, XmNvalueChangedCallback, do_mbpingedit_reverse_keys,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls, (char *)"Reverse Mouse Buttons", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_pulldownMenu_controls,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_reverse_mouse = XmCreateToggleButton(
		    class_in->mbpingedit_pulldownMenu_controls, (char *)"mbpingedit_toggleButton_reverse_mouse", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_reverse_mouse);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_toggleButton_reverse_mouse, XmNvalueChangedCallback, do_mbpingedit_reverse_mouse,
	              (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->mbpingedit_pulldownMenu_controls);
	ac++;
	XtSetValues(class_in->mbpingedit_cascadeButton_controls, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Next File", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 360);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_next =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_next", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_next);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_next, XmNactivateCallback, do_mbpingedit_next_buffer, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 910);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_dismiss =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_dismiss", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_dismiss, XmNactivateCallback, do_mbpingedit_dismiss, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Forward", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 270);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_forward =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_forward", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_forward);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_forward, XmNactivateCallback, do_mbpingedit_forward, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Reverse", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 180);
		ac++;
		XtSetArg(args[ac], XmNy, 0);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_reverse =
		    XmCreatePushButton(class_in->mbpingedit_controls, (char *)"mbpingedit_pushButton_reverse", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_reverse);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_reverse, XmNactivateCallback, do_mbpingedit_reverse, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Acrosstrack Width (m):  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 30);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_mbpingedit_scale_x_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_x_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_x_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 1000);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 210);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
	                    &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_mbpingedit_scale_x =
	    XmCreateScale(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_x", args, ac);
	XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_x);
	XtAddCallback(class_in->mbpingedit_slider_mbpingedit_scale_x, XmNvalueChangedCallback, do_mbpingedit_scale_x, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"20000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 470);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 60);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_mbpingedit_scale_x_max_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_x_max_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_x_max_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Vertical Exaggeration: 0.01", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 20);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_mbpingedit_scale_y_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_y_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_y_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNdecimalPoints, 2);
	ac++;
	XtSetArg(args[ac], XmNvalue, 100);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 2000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 210);
	ac++;
	XtSetArg(args[ac], XmNy, 80);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
	                    &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_mbpingedit_scale_y =
	    XmCreateScale(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_y", args, ac);
	XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_y);
	XtAddCallback(class_in->mbpingedit_slider_mbpingedit_scale_y, XmNvalueChangedCallback, do_mbpingedit_scale_y, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"20.00", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 470);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_mbpingedit_scale_y_max_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_mbpingedit_scale_y_max_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_mbpingedit_scale_y_max_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Pings shown:   1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_number_pings_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_number_pings_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_number_pings_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 10);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 670);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 290);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
	                    &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_number_pings =
	    XmCreateScale(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_number_pings", args, ac);
	XtManageChild(class_in->mbpingedit_slider_number_pings);
	XtAddCallback(class_in->mbpingedit_slider_number_pings, XmNvalueChangedCallback, do_mbpingedit_number_pings, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"20", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 960);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_num_pings_max_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_num_pings_max_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_num_pings_max_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Pings to step:  1", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_number_step_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_number_step_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_number_step_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 5);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 20);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 670);
	ac++;
	XtSetArg(args[ac], XmNy, 80);
	ac++;
	XtSetArg(args[ac], XmNwidth, 290);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
	                    &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_number_step =
	    XmCreateScale(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_number_step", args, ac);
	XtManageChild(class_in->mbpingedit_slider_number_step);
	XtAddCallback(class_in->mbpingedit_slider_number_step, XmNvalueChangedCallback, do_mbpingedit_number_step, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"20", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 960);
		ac++;
		XtSetArg(args[ac], XmNy, 90);
		ac++;
		XtSetArg(args[ac], XmNwidth, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 20);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_number_max_step_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_slider_number_max_step_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_number_max_step_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_controls, (char *)"Mode:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNmarginWidth, 0);
		ac++;
		XtSetArg(args[ac], XmNx, 70);
		ac++;
		XtSetArg(args[ac], XmNy, 120);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_controls, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_setting_mode_label =
		    XmCreateLabel(class_in->mbpingedit_controls, (char *)"mbpingedit_setting_mode_label", args, ac);
		XtManageChild(class_in->mbpingedit_setting_mode_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNnumColumns, 1);
	ac++;
	XtSetArg(args[ac], XmNpacking, XmPACK_TIGHT);
	ac++;
	XtSetArg(args[ac], XmNradioBehavior, True);
	ac++;
	XtSetArg(args[ac], XmNspacing, 0);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 120);
	ac++;
	XtSetArg(args[ac], XmNy, 120);
	ac++;
	XtSetArg(args[ac], XmNwidth, 405);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	class_in->mbpingedit_setting_mode =
	    XmCreateRowColumn(class_in->mbpingedit_controls, (char *)"mbpingedit_setting_mode", args, ac);
	XtManageChild(class_in->mbpingedit_setting_mode);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Toggle", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 75);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_toggle =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_toggle", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_toggle);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_toggle, XmNvalueChangedCallback, do_mbpingedit_mode_toggle, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Pick", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_pick =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_pick", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_pick);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_pick, XmNvalueChangedCallback, do_mbpingedit_mode_pick, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Erase", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_erase =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_erase", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_erase);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_erase, XmNvalueChangedCallback, do_mbpingedit_mode_erase, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Restore", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_restore =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_restore", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_restore);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_restore, XmNvalueChangedCallback, do_mbpingedit_mode_restore, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Grab", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 62);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_grab =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_grab", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_grab);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_grab, XmNvalueChangedCallback, do_mbpingedit_mode_grab, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"Info", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 54);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_setting_mode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_togglebutton_info =
		    XmCreateToggleButton(class_in->mbpingedit_setting_mode, (char *)"mbpingedit_togglebutton_info", args, ac);
		XtManageChild(class_in->mbpingedit_togglebutton_info);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_togglebutton_info, XmNvalueChangedCallback, do_mbpingedit_mode_info, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg(args[ac], XmNbackground, BX_CONVERT(class_in->MBpedit, (char *)"white", XmRPixel, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 150);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1000);
	ac++;
	XtSetArg(args[ac], XmNheight, 540);
	ac++;
	class_in->mbpingedit_canvas = XmCreateDrawingArea(class_in->MBpedit, (char *)"mbpingedit_canvas", args, ac);
	XtManageChild(class_in->mbpingedit_canvas);
	XtAddCallback(class_in->mbpingedit_canvas, XmNinputCallback, do_mbpingedit_event, (XtPointer)0);
	XtAddCallback(class_in->mbpingedit_canvas, XmNexposeCallback, do_mbpingedit_expose, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "MBeditviz Swath View Annotation");
	ac++;
	XtSetArg(args[ac], XmNx, 630);
	ac++;
	XtSetArg(args[ac], XmNy, 480);
	ac++;
	XtSetArg(args[ac], XmNwidth, 524);
	ac++;
	XtSetArg(args[ac], XmNheight, 136);
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING);
	ac++;
	class_in->mbpingedit_dialogShell_annotation =
	    XmCreateDialogShell(class_in->MBpedit, (char *)"mbpingedit_dialogShell_annotation", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 630);
	ac++;
	XtSetArg(args[ac], XmNy, 480);
	ac++;
	XtSetArg(args[ac], XmNwidth, 524);
	ac++;
	XtSetArg(args[ac], XmNheight, 136);
	ac++;
	class_in->mbpingedit_form_annotation = XtCreateWidget((char *)"mbpingedit_form_annotation", xmFormWidgetClass,
	                                                      class_in->mbpingedit_dialogShell_annotation, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 230);
		ac++;
		XtSetArg(args[ac], XmNy, 93);
		ac++;
		XtSetArg(args[ac], XmNwidth, 77);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_annotation_dismiss = XmCreatePushButton(
		    class_in->mbpingedit_form_annotation, (char *)"mbpingedit_pushButton_annotation_dismiss", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_annotation_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"1000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 450);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_y_max_interval_label =
		    XmCreateLabel(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_y_max_interval_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_y_max_interval_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 250);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 1000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 190);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 270);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_y_interval =
	    XmCreateScale(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_y_interval", args, ac);
	XtManageChild(class_in->mbpingedit_slider_y_interval);
	XtAddCallback(class_in->mbpingedit_slider_y_interval, XmNvalueChangedCallback, do_mbpingedit_y_interval, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"Y Axis Tick Interval (m): 1", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 0);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_y_interval_label =
		    XmCreateLabel(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_y_interval_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_y_interval_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"5000", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 450);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 65);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_x_max_interval_label =
		    XmCreateLabel(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_x_max_interval_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_x_max_interval_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNminimum, 1);
	ac++;
	XtSetArg(args[ac], XmNvalue, 1000);
	ac++;
	XtSetArg(args[ac], XmNmaximum, 5000);
	ac++;
	XtSetArg(args[ac], XmNscaleHeight, 15);
	ac++;
	XtSetArg(args[ac], XmNshowArrows, TRUE);
	ac++;
	XtSetArg(args[ac], XmNscaleMultiple, 1);
	ac++;
	XtSetArg(args[ac], XmNshowValue, TRUE);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 190);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 270);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	class_in->mbpingedit_slider_x_interval =
	    XmCreateScale(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_x_interval", args, ac);
	XtManageChild(class_in->mbpingedit_slider_x_interval);
	XtAddCallback(class_in->mbpingedit_slider_x_interval, XmNvalueChangedCallback, do_mbpingedit_x_interval, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"X Axis Tick Interval (m): 1", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_END);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_annotation, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_slider_x_interval_label =
		    XmCreateLabel(class_in->mbpingedit_form_annotation, (char *)"mbpingedit_slider_x_interval_label", args, ac);
		XtManageChild(class_in->mbpingedit_slider_x_interval_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNtitle, "MBeditviz Swath View Filters");
	ac++;
	XtSetArg(args[ac], XmNx, 950);
	ac++;
	XtSetArg(args[ac], XmNy, 657);
	ac++;
	XtSetArg(args[ac], XmNwidth, 408);
	ac++;
	XtSetArg(args[ac], XmNheight, 361);
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING);
	ac++;
	class_in->mbpingedit_dialogShell_filters =
	    XmCreateDialogShell(class_in->MBpedit, (char *)"mbpingedit_dialogShell_filters", args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 950);
	ac++;
	XtSetArg(args[ac], XmNy, 657);
	ac++;
	XtSetArg(args[ac], XmNwidth, 408);
	ac++;
	XtSetArg(args[ac], XmNheight, 361);
	ac++;
	class_in->mbpingedit_form_filters =
	    XtCreateWidget((char *)"mbpingedit_form_filters", xmFormWidgetClass, class_in->mbpingedit_dialogShell_filters, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAUTOMATIC);
	ac++;
	XtSetArg(args[ac], XmNx, 0);
	ac++;
	XtSetArg(args[ac], XmNy, 0);
	ac++;
	XtSetArg(args[ac], XmNwidth, 404);
	ac++;
	XtSetArg(args[ac], XmNheight, 301);
	ac++;
	class_in->scrolledWindow_filters =
	    XmCreateScrolledWindow(class_in->mbpingedit_form_filters, (char *)"scrolledWindow_filters", args, ac);
	XtManageChild(class_in->scrolledWindow_filters);

	ac = 0;
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNwidth, 375);
	ac++;
	XtSetArg(args[ac], XmNheight, 810);
	ac++;
	class_in->mbpingedit_bulletinBoard_scrollfilters =
	    XmCreateBulletinBoard(class_in->scrolledWindow_filters, (char *)"mbpingedit_bulletinBoard_scrollfilters", args, ac);
	XtManageChild(class_in->mbpingedit_bulletinBoard_scrollfilters);

	ac = 0;
	XtSetArg(args[ac], XmNx, 90);
	ac++;
	XtSetArg(args[ac], XmNy, 70);
	ac++;
	XtSetArg(args[ac], XmNwidth, 16);
	ac++;
	XtSetArg(args[ac], XmNheight, 16);
	ac++;
	XtSetArg(args[ac], XmNisHomogeneous, False);
	ac++;
	class_in->mbpingedit_radioBox_mediancalc =
	    XmCreateRadioBox(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_radioBox_mediancalc", args, ac);
	XtManageChild(class_in->mbpingedit_radioBox_mediancalc);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Median Alongtrack Dimension",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 20);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 140);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_median_local_ltrack = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                               (char *)"mbpingedit_scale_median_local_ltrack", args, ac);
		XtManageChild(class_in->mbpingedit_scale_median_local_ltrack);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_scale_median_local_ltrack, XmNvalueChangedCallback, do_mbpingedit_check_median_ltrack,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Median Acrosstrack Dimension",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNvalue, 5);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 80);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_median_local_xtrack = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                               (char *)"mbpingedit_scale_median_local_xtrack", args, ac);
		XtManageChild(class_in->mbpingedit_scale_median_local_xtrack);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_scale_median_local_xtrack, XmNvalueChangedCallback, do_mbpingedit_check_median_xtrack,
	              (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 650);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	class_in->mbpingedit_separator6 =
	    XmCreateSeparator(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_separator6", args, ac);
	XtManageChild(class_in->mbpingedit_separator6);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"End Flagging Angle (deg)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 580);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutangleend = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                               (char *)"mbpingedit_scale_filters_cutangleend", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutangleend);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Start Flagging Angle (deg)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 520);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutanglestart = XmCreateScale(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_scale_filters_cutanglestart", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutanglestart);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                            (char *)":::t\"Flag by\":t\"Beam\"\"Angle\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 530);
		ac++;
		XtSetArg(args[ac], XmNwidth, 80);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_filters_cutangle = XmCreateToggleButton(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_toggleButton_filters_cutangle", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_filters_cutangle);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 500);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	class_in->mbpingedit_separator5 =
	    XmCreateSeparator(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_separator5", args, ac);
	XtManageChild(class_in->mbpingedit_separator5);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"End Flagging Distance (m)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 440);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutdistanceend = XmCreateScale(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_scale_filters_cutdistanceend", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutdistanceend);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Start Flagging Distance (m)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNvalue, 1000);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10000);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 380);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutdistancestart = XmCreateScale(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_scale_filters_cutdistancestart", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutdistancestart);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)":::t\"Flag by\"\"Distance\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 390);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_filters_cutdistance = XmCreateToggleButton(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_toggleButton_filters_cutdistance", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_filters_cutdistance);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 210);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	class_in->mbpingedit_separator4 =
	    XmCreateSeparator(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_separator4", args, ac);
	XtManageChild(class_in->mbpingedit_separator4);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"End Flagging Beam Number",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 290);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutbeamend = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                              (char *)"mbpingedit_scale_filters_cutbeamend", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutbeamend);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Start Flagging Beam Number",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 230);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_cutbeamstart = XmCreateScale(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_scale_filters_cutbeamstart", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_cutbeamstart);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                            (char *)":::t\"Flag by\":t\"Beam\"\"Number\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 240);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_filters_cutbeam = XmCreateToggleButton(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_toggleButton_filters_cutbeam", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_filters_cutbeam);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 360);
	ac++;
	XtSetArg(args[ac], XmNwidth, 350);
	ac++;
	XtSetArg(args[ac], XmNheight, 20);
	ac++;
	class_in->mbpingedit_separator3 =
	    XmCreateSeparator(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_separator3", args, ac);
	XtManageChild(class_in->mbpingedit_separator3);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"Beams from Center Threshold",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 100);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 670);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_wrongside = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                             (char *)"mbpingedit_scale_filters_wrongside", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_wrongside);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)":::t\"Wrong\":t\"Side\"\"Filter\"",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 680);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_filters_wrongside = XmCreateToggleButton(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_toggleButton_filters_wrongside", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_filters_wrongside);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"% Median Depth Threshold ",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 1);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 0);
		ac++;
		XtSetArg(args[ac], XmNvalue, 10);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 260);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_scale_filters_medianspike = XmCreateScale(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                                                               (char *)"mbpingedit_scale_filters_medianspike", args, ac);
		XtManageChild(class_in->mbpingedit_scale_filters_medianspike);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                            (char *)":::t\"Median\":t\"Spike\"\"Filter\"", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 20);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 60);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_bulletinBoard_scrollfilters,
		                    (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_toggleButton_filters_medianspike = XmCreateToggleButton(
		    class_in->mbpingedit_bulletinBoard_scrollfilters, (char *)"mbpingedit_toggleButton_filters_medianspike", args, ac);
		XtManageChild(class_in->mbpingedit_toggleButton_filters_medianspike);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"Reset", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 150);
		ac++;
		XtSetArg(args[ac], XmNy, 311);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_filters_reset =
		    XmCreatePushButton(class_in->mbpingedit_form_filters, (char *)"mbpingedit_pushButton_filters_reset", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_filters_reset);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_filters_reset, XmNactivateCallback, do_mbpingedit_reset_filters, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"Apply", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 40);
		ac++;
		XtSetArg(args[ac], XmNy, 311);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_filters_apply =
		    XmCreatePushButton(class_in->mbpingedit_form_filters, (char *)"mbpingedit_pushButton_filters_apply", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_filters_apply);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_filters_apply, XmNactivateCallback, do_mbpingedit_set_filters, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 260);
		ac++;
		XtSetArg(args[ac], XmNy, 311);
		ac++;
		XtSetArg(args[ac], XmNwidth, 90);
		ac++;
		XtSetArg(args[ac], XmNheight, 40);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->mbpingedit_form_filters, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->mbpingedit_pushButton_filters_dismiss =
		    XmCreatePushButton(class_in->mbpingedit_form_filters, (char *)"mbpingedit_pushButton_filters_dismiss", args, ac);
		XtManageChild(class_in->mbpingedit_pushButton_filters_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbpingedit_pushButton_filters_dismiss, XmNactivateCallback, BxUnmanageCB,
	              (XtPointer) "mbpingedit_bulletinBoard_filters");
	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, -1);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 0);
	ac++;
	XtSetValues(class_in->mbpingedit_controls, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 2);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 2);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 150);
	ac++;
	XtSetValues(class_in->mbpingedit_canvas, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 60);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 4);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 0);
	ac++;
	XtSetValues(class_in->scrolledWindow_filters, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetValues(class_in->mbpingedit_pushButton_filters_reset, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetValues(class_in->mbpingedit_pushButton_filters_apply, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 10);
	ac++;
	XtSetValues(class_in->mbpingedit_pushButton_filters_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 15);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 230);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 217);
	ac++;
	XtSetValues(class_in->mbpingedit_pushButton_annotation_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 450);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 50);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_y_max_interval_label, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 190);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 64);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 40);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_y_interval, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 50);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_y_interval_label, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 450);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_x_max_interval_label, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 190);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 64);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 0);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_x_interval, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->mbpingedit_slider_x_interval_label, args, ac);

	return (class_in);
}
