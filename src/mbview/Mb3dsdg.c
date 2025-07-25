/*------------------------------------------------------------------------------
 *    The MB-system:	Mb3dsdg.c	10/28/2003
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
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/DrawingA.h>
#include "Mb3dsdg.h"

/**
 * Common constant and pixmap declarations.
 */
#include "mb3dsoundings_creation.h"

void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/**
 * Declarations for callbacks and handlers.
 */
void do_mb3dsdg_resetview(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_panzoom(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_rotate(Widget, XtPointer, XtPointer);
void do_mb3dsdg_rollbias(Widget, XtPointer, XtPointer);
void do_mb3dsdg_pitchbias(Widget, XtPointer, XtPointer);
void do_mb3dsdg_headingbias(Widget, XtPointer, XtPointer);
void do_mb3dsdg_timelag(Widget, XtPointer, XtPointer);
void do_mb3dsdg_snell(Widget, XtPointer, XtPointer);
void do_mb3dsdg_input(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_boundingbox(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_flagged(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_secondary(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_noprofile(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_goodprofile(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_allprofile(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_scalewithflagged(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_colorbyflag(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_colorbytopo(Widget, XtPointer, XtPointer);
void do_mb3dsdg_view_colorbyamp(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_applybias(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_A(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_B(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_C(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_D(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_E(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_flagsparsevoxels_F(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsblack(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsred(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsyellow(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsgreen(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsbluegreen(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingsblue(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_colorsoundingspurple(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_r(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_p(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_h(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_rp(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_rph(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_t(Widget, XtPointer, XtPointer);
void do_mb3dsdg_action_optimizebiasvalues_s(Widget, XtPointer, XtPointer);
void do_mb3dsdg_dismiss(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_toggle(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_pick(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_erase(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_restore(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_grab(Widget, XtPointer, XtPointer);
void do_mb3dsdg_mouse_info(Widget, XtPointer, XtPointer);

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

static Boolean doInitAppDefaults = True;
static UIAppDefault appDefaults[] = {{NULL, NULL, NULL, NULL, NULL}};

// The functions to call in the apputils.c
void InitAppDefaults(Widget, UIAppDefault *);
void SetAppDefaults(Widget, UIAppDefault *, char *, Boolean);

Mb3dsdgDataPtr Mb3dsdgCreate(Mb3dsdgDataPtr class_in, Widget parent, String name, ArgList args_in, Cardinal ac_in) {
	(void)args_in;  // Unused parameter
	(void)ac_in;  // Unused parameter

	// Register the converters for the widgets.
	RegisterBxConverters(XtWidgetToApplicationContext(parent));
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmPushButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScaleWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmToggleButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmLabelWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmRowColumnWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmCascadeButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmSeparatorWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmDrawingAreaWidgetClass);

	// Setup app-defaults fallback table if not already done.
	if (doInitAppDefaults) {
		InitAppDefaults(parent, appDefaults);
		doInitAppDefaults = False;
	}

	SetAppDefaults(parent, appDefaults, name, False);

	Cardinal ac = 0;
	Arg args[256];
	Boolean argok = False;
	{
		XmString tmp0 = (XmString)BX_CONVERT(parent, (char *)"3D Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNdialogTitle, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
		ac++;
		XtSetArg(args[ac], XmNx, 671);
		ac++;
		XtSetArg(args[ac], XmNy, 275);
		ac++;
		XtSetArg(args[ac], XmNwidth, 987);
		ac++;
		XtSetArg(args[ac], XmNheight, 584);
		ac++;
		XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING);
		ac++;
		class_in->Mb3dsdg = XmCreateForm(parent, (char *)name, args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Reset View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 910);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 160);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_reset = XmCreatePushButton(class_in->Mb3dsdg, (char *)"pushButton_reset", args, ac);
		XtManageChild(class_in->pushButton_reset);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_reset, XmNactivateCallback, do_mb3dsdg_resetview, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Pan and Zoom", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 650);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_panzoom1 =
		    XmCreateToggleButton(class_in->Mb3dsdg, (char *)"toggleButton_mouse_panzoom1", args, ac);
		XtManageChild(class_in->toggleButton_mouse_panzoom1);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_panzoom1, XmNvalueChangedCallback, do_mb3dsdg_mouse_panzoom, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Rotate Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 650);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_rotate1 =
		    XmCreateToggleButton(class_in->Mb3dsdg, (char *)"toggleButton_mouse_rotate1", args, ac);
		XtManageChild(class_in->toggleButton_mouse_rotate1);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_rotate1, XmNvalueChangedCallback, do_mb3dsdg_mouse_rotate, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(
		    class_in->Mb3dsdg, (char *)":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Rotate Soundings\"\"R: Exageration\"",
		    XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNborderWidth, 1);
		ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNbackground, BX_CONVERT(class_in->Mb3dsdg, (char *)"white", XmRPixel, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNrecomputeSize, False);
		ac++;
		XtSetArg(args[ac], XmNx, 1050);
		ac++;
		XtSetArg(args[ac], XmNy, 50);
		ac++;
		XtSetArg(args[ac], XmNwidth, 160);
		ac++;
		XtSetArg(args[ac], XmNheight, 80);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->label_mousemode = XmCreateLabel(class_in->Mb3dsdg, (char *)"label_mousemode", args, ac);
		XtManageChild(class_in->label_mousemode);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Heading Bias (degrees)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -100);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
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
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->scale_headingbias = XmCreateScale(class_in->Mb3dsdg, (char *)"scale_headingbias", args, ac);
		XtManageChild(class_in->scale_headingbias);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->scale_headingbias, XmNvalueChangedCallback, do_mb3dsdg_headingbias, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Roll Bias (degrees)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -100);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 250);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->scale_rollbias = XmCreateScale(class_in->Mb3dsdg, (char *)"scale_rollbias", args, ac);
		XtManageChild(class_in->scale_rollbias);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->scale_rollbias, XmNvalueChangedCallback, do_mb3dsdg_rollbias, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Pitch Bias (degrees)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -100);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 400);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->scale_pitchbias = XmCreateScale(class_in->Mb3dsdg, (char *)"scale_pitchbias", args, ac);
		XtManageChild(class_in->scale_pitchbias);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->scale_pitchbias, XmNvalueChangedCallback, do_mb3dsdg_pitchbias, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Time Lag (seconds)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, -100);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 2);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 550);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->scale_timelag = XmCreateScale(class_in->Mb3dsdg, (char *)"scale_timelag", args, ac);
		XtManageChild(class_in->scale_timelag);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->scale_timelag, XmNvalueChangedCallback, do_mb3dsdg_timelag, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Snell Correction", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNtitleString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNminimum, 9900);
		ac++;
		XtSetArg(args[ac], XmNmaximum, 10100);
		ac++;
		XtSetArg(args[ac], XmNdecimalPoints, 4);
		ac++;
		XtSetArg(args[ac], XmNshowArrows, TRUE);
		ac++;
		XtSetArg(args[ac], XmNscaleMultiple, 1);
		ac++;
		XtSetArg(args[ac], XmNshowValue, TRUE);
		ac++;
		XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
		ac++;
		XtSetArg(args[ac], XmNx, 700);
		ac++;
		XtSetArg(args[ac], XmNy, 70);
		ac++;
		XtSetArg(args[ac], XmNwidth, 150);
		ac++;
		XtSetArg(args[ac], XmNheight, 63);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->scale_snell = XmCreateScale(class_in->Mb3dsdg, (char *)"scale_snell", args, ac);
		XtManageChild(class_in->scale_snell);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->scale_snell, XmNvalueChangedCallback, do_mb3dsdg_snell, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->Mb3dsdg, (char *)"Azimuth: 0.00 | Elevation: 0.00 | Vert. Exager.: 1.00",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNborderWidth, 1);
		ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNbackground, BX_CONVERT(class_in->Mb3dsdg, (char *)"white", XmRPixel, 0, &argok));
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 100);
		ac++;
		XtSetArg(args[ac], XmNy, 40);
		ac++;
		XtSetArg(args[ac], XmNwidth, 540);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->Mb3dsdg, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->label_status = XmCreateLabel(class_in->Mb3dsdg, (char *)"label_status", args, ac);
		XtManageChild(class_in->label_status);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmVERTICAL);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 81);
	ac++;
	XtSetArg(args[ac], XmNheight, 106);
	ac++;
	class_in->menuBar = XmCreateMenuBar(class_in->Mb3dsdg, (char *)"menuBar", args, ac);
	XtManageChild(class_in->menuBar);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->menuBar, (char *)"View", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 5);
		ac++;
		XtSetArg(args[ac], XmNwidth, 71);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->menuBar, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->cascadeButton_view = XmCreateCascadeButton(class_in->menuBar, (char *)"cascadeButton_view", args, ac);
		XtManageChild(class_in->cascadeButton_view);

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
	XtSetArg(args[ac], XmNwidth, 236);
	ac++;
	XtSetArg(args[ac], XmNheight, 154);
	ac++;
	class_in->pulldownMenu_view =
	    XmCreatePulldownMenu(XtParent(class_in->cascadeButton_view), (char *)"pulldownMenu_view", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Show Bounding Box", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_boundingbox =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_boundingbox", args, ac);
		XtManageChild(class_in->toggleButton_view_boundingbox);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_boundingbox, XmNvalueChangedCallback, do_mb3dsdg_view_boundingbox, (XtPointer)0);

	ac = 0;
	class_in->separator1 = XmCreateSeparator(class_in->pulldownMenu_view, (char *)"separator1", args, ac);
	XtManageChild(class_in->separator1);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Show Flagged Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_flagged =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_flagged", args, ac);
		XtManageChild(class_in->toggleButton_view_flagged);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_flagged, XmNvalueChangedCallback, do_mb3dsdg_view_flagged, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"View Secondary Pick Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_secondary =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_secondary", args, ac);
		XtManageChild(class_in->toggleButton_view_secondary);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_secondary, XmNvalueChangedCallback, do_mb3dsdg_view_secondary, (XtPointer)0);

	ac = 0;
	class_in->separator = XmCreateSeparator(class_in->pulldownMenu_view, (char *)"separator", args, ac);
	XtManageChild(class_in->separator);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Show No Profiles", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_noconnect =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_noconnect", args, ac);
		XtManageChild(class_in->toggleButton_view_noconnect);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_noconnect, XmNvalueChangedCallback, do_mb3dsdg_view_noprofile, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Show Good Profiles", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_connectgood =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_connectgood", args, ac);
		XtManageChild(class_in->toggleButton_view_connectgood);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_connectgood, XmNvalueChangedCallback, do_mb3dsdg_view_goodprofile, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Show All Profiles", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_connectall =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_connectall", args, ac);
		XtManageChild(class_in->toggleButton_view_connectall);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_connectall, XmNvalueChangedCallback, do_mb3dsdg_view_allprofile, (XtPointer)0);

	ac = 0;
	class_in->separator2 = XmCreateSeparator(class_in->pulldownMenu_view, (char *)"separator2", args, ac);
	XtManageChild(class_in->separator2);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Scale with Flagged Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_scalewithflagged =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_scalewithflagged", args, ac);
		XtManageChild(class_in->toggleButton_view_scalewithflagged);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_scalewithflagged, XmNvalueChangedCallback, do_mb3dsdg_view_scalewithflagged,
	              (XtPointer)0);

	ac = 0;
	class_in->separator3 = XmCreateSeparator(class_in->pulldownMenu_view, (char *)"separator3", args, ac);
	XtManageChild(class_in->separator3);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Color by Flag State", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_colorbyflag =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_colorbyflag", args, ac);
		XtManageChild(class_in->toggleButton_view_colorbyflag);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_colorbyflag, XmNvalueChangedCallback, do_mb3dsdg_view_colorbyflag,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Color by Topography", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_colorbytopo =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_colorbytopo", args, ac);
		XtManageChild(class_in->toggleButton_view_colorbytopo);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_colorbytopo, XmNvalueChangedCallback, do_mb3dsdg_view_colorbytopo,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_view, (char *)"Color by Amplitude", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_view, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_view_colorbyamp =
		    XmCreateToggleButton(class_in->pulldownMenu_view, (char *)"toggleButton_view_colorbyamp", args, ac);
		XtManageChild(class_in->toggleButton_view_colorbyamp);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_view_colorbyamp, XmNvalueChangedCallback, do_mb3dsdg_view_colorbyamp,
	              (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->pulldownMenu_view);
	ac++;
	XtSetValues(class_in->cascadeButton_view, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->menuBar, (char *)"Mouse", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 29);
		ac++;
		XtSetArg(args[ac], XmNwidth, 71);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->menuBar, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->cascadeButton_mouse = XmCreateCascadeButton(class_in->menuBar, (char *)"cascadeButton_mouse", args, ac);
		XtManageChild(class_in->cascadeButton_mouse);

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
	XtSetArg(args[ac], XmNwidth, 152);
	ac++;
	XtSetArg(args[ac], XmNheight, 52);
	ac++;
	class_in->pulldownMenu_mouse =
	    XmCreatePulldownMenu(XtParent(class_in->cascadeButton_mouse), (char *)"pulldownMenu_mouse", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_mouse, (char *)"Rotate Soundings", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_mouse, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_rotate =
		    XmCreateToggleButton(class_in->pulldownMenu_mouse, (char *)"toggleButton_mouse_rotate", args, ac);
		XtManageChild(class_in->toggleButton_mouse_rotate);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_rotate, XmNvalueChangedCallback, do_mb3dsdg_mouse_rotate, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_mouse, (char *)"Pan and Zoom", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_mouse, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList,
		                    0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_panzoom =
		    XmCreateToggleButton(class_in->pulldownMenu_mouse, (char *)"toggleButton_mouse_panzoom", args, ac);
		XtManageChild(class_in->toggleButton_mouse_panzoom);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_panzoom, XmNvalueChangedCallback, do_mb3dsdg_mouse_panzoom, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->pulldownMenu_mouse);
	ac++;
	XtSetValues(class_in->cascadeButton_mouse, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->menuBar, (char *)"Action", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 53);
		ac++;
		XtSetArg(args[ac], XmNwidth, 71);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->menuBar, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->cascadeButton_action = XmCreateCascadeButton(class_in->menuBar, (char *)"cascadeButton_action", args, ac);
		XtManageChild(class_in->cascadeButton_action);

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
	XtSetArg(args[ac], XmNwidth, 189);
	ac++;
	XtSetArg(args[ac], XmNheight, 28);
	ac++;
	class_in->pulldownMenu_action =
	    XmCreatePulldownMenu(XtParent(class_in->cascadeButton_action), (char *)"pulldownMenu_action", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Apply Bias Values to Grid", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_applybias =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_applybias", args, ac);
		XtManageChild(class_in->pushButton_action_applybias);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_applybias, XmNactivateCallback, do_mb3dsdg_action_applybias, (XtPointer)0);

	ac = 0;
	class_in->separator4 = XmCreateSeparator(class_in->pulldownMenu_action, (char *)"separator4", args, ac);
	XtManageChild(class_in->separator4);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (1 X cell, n<10)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_A =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_A);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_A, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_A,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (1 X cell, n<2)", XmRXmString,
		                            0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_B =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_B);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_B, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_B,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (4 X cell, n<10)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_C =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_C);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_C, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_C,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (4 X cell, n<2)", XmRXmString,
		                            0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_D =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_D);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_D, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_D,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (8 X cell, n<10)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_E =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_E);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_E, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_E,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Filter by sparse voxels (8 X cell, n<2)", XmRXmString,
		                            0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_flagsparsevoxels_F =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_flagsparsevoxels", args, ac);
		XtManageChild(class_in->pushButton_action_flagsparsevoxels_F);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_flagsparsevoxels_F, XmNactivateCallback, do_mb3dsdg_action_flagsparsevoxels_F,
	              (XtPointer)0);

	ac = 0;
	class_in->separator5 = XmCreateSeparator(class_in->pulldownMenu_action, (char *)"separator5", args, ac);
	XtManageChild(class_in->separator5);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Black", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsblack =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsblack", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsblack);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsblack, XmNactivateCallback, do_mb3dsdg_action_colorsoundingsblack,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Red", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsred =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsred", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsred);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsred, XmNactivateCallback, do_mb3dsdg_action_colorsoundingsred,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Yellow", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsyellow =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsyellow", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsyellow);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsyellow, XmNactivateCallback, do_mb3dsdg_action_colorsoundingsyellow,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Green", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsgreen =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsgreen", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsgreen);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsgreen, XmNactivateCallback, do_mb3dsdg_action_colorsoundingsgreen,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Bluegreen", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsbluegreen =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsbluegreen", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsbluegreen);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsbluegreen, XmNactivateCallback,
	              do_mb3dsdg_action_colorsoundingsbluegreen, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Blue", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingsblue =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingsblue", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingsblue);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingsblue, XmNactivateCallback, do_mb3dsdg_action_colorsoundingsblue,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Color Unflagged Soundings Purple", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_colorsoundingspurple =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_colorsoundingspurple", args, ac);
		XtManageChild(class_in->pushButton_action_colorsoundingspurple);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_colorsoundingspurple, XmNactivateCallback, do_mb3dsdg_action_colorsoundingspurple,
	              (XtPointer)0);

	ac = 0;
	class_in->separator6 = XmCreateSeparator(class_in->pulldownMenu_action, (char *)"separator6", args, ac);
	XtManageChild(class_in->separator6);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Bias Values (roll)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_r =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_r", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_r);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_r, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_r,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Bias Values (pitch)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_p =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_p", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_p);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_p, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_p,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 =
		    (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Bias Values (heading)", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_h =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_h", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_h);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_h, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_h,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Bias Values (roll-pitch)", XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_rp =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_rp", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_rp);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_rp, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_rp,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Bias Values (roll-pitch-heading)",
		                            XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_rph =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_rph", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_rph);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_rph, XmNactivateCallback,
	              do_mb3dsdg_action_optimizebiasvalues_rph, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Time Lag Values", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_t =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_t", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_t);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_t, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_t,
	              (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_action, (char *)"Optimize Snell Correction Values", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_action, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_action_optimizebiasvalues_s =
		    XmCreatePushButton(class_in->pulldownMenu_action, (char *)"pushButton_action_optimizebiasvalues_s", args, ac);
		XtManageChild(class_in->pushButton_action_optimizebiasvalues_s);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_action_optimizebiasvalues_s, XmNactivateCallback, do_mb3dsdg_action_optimizebiasvalues_s,
	              (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->pulldownMenu_action);
	ac++;
	XtSetValues(class_in->cascadeButton_action, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->menuBar, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 5);
		ac++;
		XtSetArg(args[ac], XmNy, 77);
		ac++;
		XtSetArg(args[ac], XmNwidth, 71);
		ac++;
		XtSetArg(args[ac], XmNheight, 24);
		ac++;
		XtSetArg(
		    args[ac], XmNfontList,
		    BX_CONVERT(class_in->menuBar, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->cascadeButton_dismiss = XmCreateCascadeButton(class_in->menuBar, (char *)"cascadeButton_dismiss", args, ac);
		XtManageChild(class_in->cascadeButton_dismiss);

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
	XtSetArg(args[ac], XmNwidth, 67);
	ac++;
	XtSetArg(args[ac], XmNheight, 28);
	ac++;
	class_in->pulldownMenu_dismiss =
	    XmCreatePulldownMenu(XtParent(class_in->cascadeButton_dismiss), (char *)"pulldownMenu_dismiss", args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->pulldownMenu_dismiss, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->pulldownMenu_dismiss, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->pushButton_dismiss = XmCreatePushButton(class_in->pulldownMenu_dismiss, (char *)"pushButton_dismiss", args, ac);
		XtManageChild(class_in->pushButton_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->pushButton_dismiss, XmNactivateCallback, do_mb3dsdg_dismiss, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNsubMenuId, class_in->pulldownMenu_dismiss);
	ac++;
	XtSetValues(class_in->cascadeButton_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNborderWidth, 1);
	ac++;
	XtSetArg(args[ac], XmNbackground, BX_CONVERT(class_in->Mb3dsdg, (char *)"white", XmRPixel, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNborderColor, BX_CONVERT(class_in->Mb3dsdg, (char *)"black", XmRPixel, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 140);
	ac++;
	XtSetArg(args[ac], XmNwidth, 970);
	ac++;
	XtSetArg(args[ac], XmNheight, 435);
	ac++;
	class_in->drawingArea = XmCreateDrawingArea(class_in->Mb3dsdg, (char *)"drawingArea", args, ac);
	XtManageChild(class_in->drawingArea);
	XtAddCallback(class_in->drawingArea, XmNinputCallback, do_mb3dsdg_input, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNx, 100);
	ac++;
	XtSetArg(args[ac], XmNy, 10);
	ac++;
	XtSetArg(args[ac], XmNwidth, 519);
	ac++;
	XtSetArg(args[ac], XmNheight, 34);
	ac++;
	XtSetArg(args[ac], XmNisHomogeneous, False);
	ac++;
	class_in->radioBox_soundingsmode = XmCreateRadioBox(class_in->Mb3dsdg, (char *)"radioBox_soundingsmode", args, ac);
	XtManageChild(class_in->radioBox_soundingsmode);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Toggle", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_toggle =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_toggle", args, ac);
		XtManageChild(class_in->toggleButton_mouse_toggle);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_toggle, XmNvalueChangedCallback, do_mb3dsdg_mouse_toggle, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Pick", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_pick =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_pick", args, ac);
		XtManageChild(class_in->toggleButton_mouse_pick);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_pick, XmNvalueChangedCallback, do_mb3dsdg_mouse_pick, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Erase", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_erase =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_erase", args, ac);
		XtManageChild(class_in->toggleButton_mouse_erase);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_erase, XmNvalueChangedCallback, do_mb3dsdg_mouse_erase, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Restore", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_restore =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_restore", args, ac);
		XtManageChild(class_in->toggleButton_mouse_restore);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_restore, XmNvalueChangedCallback, do_mb3dsdg_mouse_restore, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Grab", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_grab =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_grab", args, ac);
		XtManageChild(class_in->toggleButton_mouse_grab);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_grab, XmNvalueChangedCallback, do_mb3dsdg_mouse_grab, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"Info", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNwidth, 83);
		ac++;
		XtSetArg(args[ac], XmNheight, 28);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->radioBox_soundingsmode, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
		                    XmRFontList, 0, &argok));
		if (argok)
			ac++;
		class_in->toggleButton_mouse_info =
		    XmCreateToggleButton(class_in->radioBox_soundingsmode, (char *)"toggleButton_mouse_info", args, ac);
		XtManageChild(class_in->toggleButton_mouse_info);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->toggleButton_mouse_info, XmNvalueChangedCallback, do_mb3dsdg_mouse_info, (XtPointer)0);
	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 860);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->pushButton_reset, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 630);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 40);
	ac++;
	XtSetValues(class_in->toggleButton_mouse_panzoom1, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 630);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->toggleButton_mouse_rotate1, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 860);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 0);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 50);
	ac++;
	XtSetValues(class_in->label_mousemode, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 100);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 70);
	ac++;
	XtSetValues(class_in->scale_headingbias, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 250);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 70);
	ac++;
	XtSetValues(class_in->scale_rollbias, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 400);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 70);
	ac++;
	XtSetValues(class_in->scale_pitchbias, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 550);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 70);
	ac++;
	XtSetValues(class_in->scale_timelag, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 700);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 70);
	ac++;
	XtSetValues(class_in->scale_snell, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->menuBar, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 100);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->label_status, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 100);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 40);
	ac++;
	XtSetValues(class_in->radioBox_soundingsmode, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 7);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 5);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 140);
	ac++;
	XtSetValues(class_in->drawingArea, args, ac);

	/*
	 * Assign functions to class record
	 */

	/* Begin user code block <end_Mb3dsdgCreate> */
	/* End user code block <end_Mb3dsdgCreate> */

	return (class_in);
}
