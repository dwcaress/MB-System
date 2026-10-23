/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DGisList.c	10/23/2026
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
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include "MB3DGisList.h"

// Convenience functions from utilities file.
void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

void do_mbview_gislist_showhide(Widget, XtPointer, XtPointer);
void do_mbview_gislist_delete(Widget, XtPointer, XtPointer);
void do_mbview_gislist_toroutes(Widget, XtPointer, XtPointer);
void do_mbview_gislist_popdown(Widget, XtPointer, XtPointer);
void do_mbview_gislistselect(Widget, XtPointer, XtPointer);

/* polyline type, as MBV_GISLAYER_POLYLINE in mbview.h - only polylines can be converted to routes */
#define MB3DGISLIST_POLYLINE 1

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
/*
 * The functions to call in the apputils.c
 */
void InitAppDefaults(Widget, UIAppDefault *);
void SetAppDefaults(Widget, UIAppDefault *, char *, Boolean);

MB3DGisListDataPtr MB3DGisListCreate(MB3DGisListDataPtr class_in, Widget parent, String name, ArgList args_in, Cardinal ac_in,
                                     int type) {
	(void)args_in;  // Unused parameter
	(void)ac_in;  // Unused parameter

	/**
	 * Register the converters for the widgets.
	 */
	RegisterBxConverters(XtWidgetToApplicationContext(parent));
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmLabelWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmPushButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScrolledWindowWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmListWidgetClass);
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
	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 72);
	ac++;
	XtSetArg(args[ac], XmNy, 524);
	ac++;
	XtSetArg(args[ac], XmNwidth, 720);
	ac++;
	XtSetArg(args[ac], XmNheight, 300);
	ac++;
	class_in->MB3DGisList = XmCreateForm(parent, (char *)name, args, ac);

	Boolean argok = False;

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DGisList, (char *)"GIS List:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DGisList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_gislist_label = XmCreateLabel(class_in->MB3DGisList, (char *)"mbview_gislist_label", args, ac);
		XtManageChild(class_in->mbview_gislist_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DGisList, (char *)"Show/Hide Selected", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DGisList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_gislist_showhide = XmCreatePushButton(class_in->MB3DGisList, (char *)"mbview_pushButton_gislist_showhide", args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_gislist_showhide, XmNactivateCallback, do_mbview_gislist_showhide, (XtPointer)(size_t)type);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DGisList, (char *)"Delete Selected", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DGisList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_gislist_delete = XmCreatePushButton(class_in->MB3DGisList, (char *)"mbview_pushButton_gislist_delete", args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_gislist_delete, XmNactivateCallback, do_mbview_gislist_delete, (XtPointer)(size_t)type);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DGisList, (char *)"Convert Selected to Routes", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DGisList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_gislist_toroutes = XmCreatePushButton(class_in->MB3DGisList, (char *)"mbview_pushButton_gislist_toroutes", args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_gislist_toroutes, XmNactivateCallback, do_mbview_gislist_toroutes, (XtPointer)(size_t)type);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DGisList, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DGisList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_gislist_dismiss = XmCreatePushButton(class_in->MB3DGisList, (char *)"mbview_pushButton_gislist_dismiss", args, ac);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_gislist_dismiss, XmNactivateCallback, do_mbview_gislist_popdown, (XtPointer)(size_t)type);

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED);
	ac++;
	class_in->mbview_scrolledWindow_gislist =
	    XmCreateScrolledWindow(class_in->MB3DGisList, (char *)"mbview_scrolledWindow_gislist", args, ac);
	XtManageChild(class_in->mbview_scrolledWindow_gislist);

	ac = 0;
	XtSetArg(args[ac], XmNselectionPolicy, XmEXTENDED_SELECT);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbview_scrolledWindow_gislist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	class_in->mbview_list_gislist = XmCreateList(class_in->mbview_scrolledWindow_gislist, (char *)"mbview_list_gislist", args, ac);
	XtManageChild(class_in->mbview_list_gislist);
	XtAddCallback(class_in->mbview_list_gislist, XmNsingleSelectionCallback, do_mbview_gislistselect, (XtPointer)(size_t)type);
	XtAddCallback(class_in->mbview_list_gislist, XmNextendedSelectionCallback, do_mbview_gislistselect, (XtPointer)(size_t)type);
	XtAddCallback(class_in->mbview_list_gislist, XmNbrowseSelectionCallback, do_mbview_gislistselect, (XtPointer)(size_t)type);

	/* label across the top */
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 15);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_gislist_label, args, ac);

	/* buttons along the bottom, Dismiss at the right */
	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_pushButton_gislist_showhide, args, ac);
	XtManageChild(class_in->mbview_pushButton_gislist_showhide);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, class_in->mbview_pushButton_gislist_showhide);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_pushButton_gislist_delete, args, ac);
	XtManageChild(class_in->mbview_pushButton_gislist_delete);

	ac = 0;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, class_in->mbview_pushButton_gislist_delete);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_pushButton_gislist_toroutes, args, ac);
	if (type == MB3DGISLIST_POLYLINE)
		XtManageChild(class_in->mbview_pushButton_gislist_toroutes);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 15);
	ac++;
	XtSetValues(class_in->mbview_pushButton_gislist_dismiss, args, ac);
	XtManageChild(class_in->mbview_pushButton_gislist_dismiss);

	/* list fills the rest */
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, class_in->mbview_gislist_label);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 49);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 15);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 0);
	ac++;
	XtSetValues(class_in->mbview_scrolledWindow_gislist, args, ac);

	return (class_in);
}
