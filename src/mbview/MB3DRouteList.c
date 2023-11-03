/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DRouteList.c	10/28/2003
 *
 *    Copyright (c) 2003-2023 by
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
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include "MB3DRouteList.h"

// Common constant and pixmap declarations.
#include "creation-c.h"

// Convenience functions from utilities file.
void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

void do_mbview_routelist_delete(Widget, XtPointer, XtPointer);
void do_mbview_routelist_popdown(Widget, XtPointer, XtPointer);
void do_mbview_routelistselect(Widget, XtPointer, XtPointer);

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

MB3DRouteListDataPtr MB3DRouteListCreate(MB3DRouteListDataPtr class_in, Widget parent, String name, ArgList args_in,
                                         Cardinal ac_in) {
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
	XtSetArg(args[ac], XmNwidth, 415);
	ac++;
	XtSetArg(args[ac], XmNheight, 239);
	ac++;
	class_in->MB3DRouteList = XmCreateForm(parent, (char *)name, args, ac);

	Boolean argok = False;

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DRouteList, (char *)"Route List:", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNalignment, XmALIGNMENT_BEGINNING);
		ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 10);
		ac++;
		XtSetArg(args[ac], XmNwidth, 390);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DRouteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_routelist_label = XmCreateLabel(class_in->MB3DRouteList, (char *)"mbview_routelist_label", args, ac);
		XtManageChild(class_in->mbview_routelist_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DRouteList, (char *)"Delete Selected Routes", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 200);
		ac++;
		XtSetArg(args[ac], XmNwidth, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DRouteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_routelist_delete =
		    XmCreatePushButton(class_in->MB3DRouteList, (char *)"mbview_pushButton_routelist_delete", args, ac);
		XtManageChild(class_in->mbview_pushButton_routelist_delete);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_routelist_delete, XmNactivateCallback, do_mbview_routelist_delete, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DRouteList, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 290);
		ac++;
		XtSetArg(args[ac], XmNy, 200);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DRouteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_routelist_dismiss =
		    XmCreatePushButton(class_in->MB3DRouteList, (char *)"mbview_pushButton_routelist_dismiss", args, ac);
		XtManageChild(class_in->mbview_pushButton_routelist_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_routelist_dismiss, XmNactivateCallback, do_mbview_routelist_popdown, (XtPointer)0);

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 40);
	ac++;
	XtSetArg(args[ac], XmNwidth, 390);
	ac++;
	XtSetArg(args[ac], XmNheight, 150);
	ac++;
	class_in->mbview_scrolledWindow_routelist =
	    XmCreateScrolledWindow(class_in->MB3DRouteList, (char *)"mbview_scrolledWindow_routelist", args, ac);
	XtManageChild(class_in->mbview_scrolledWindow_routelist);

	ac = 0;
	XtSetArg(args[ac], XmNselectionPolicy, XmEXTENDED_SELECT);
	ac++;
	XtSetArg(args[ac], XmNwidth, 390);
	ac++;
	XtSetArg(args[ac], XmNheight, 150);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbview_scrolledWindow_routelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	class_in->mbview_list_routelist =
	    XmCreateList(class_in->mbview_scrolledWindow_routelist, (char *)"mbview_list_routelist", args, ac);
	XtManageChild(class_in->mbview_list_routelist);
	XtAddCallback(class_in->mbview_list_routelist, XmNsingleSelectionCallback, do_mbview_routelistselect, (XtPointer)0);
	XtAddCallback(class_in->mbview_list_routelist, XmNextendedSelectionCallback, do_mbview_routelistselect, (XtPointer)0);
	XtAddCallback(class_in->mbview_list_routelist, XmNbrowseSelectionCallback, do_mbview_routelistselect, (XtPointer)0);
	ac = 0;
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
	XtSetValues(class_in->mbview_routelist_label, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_pushButton_routelist_delete, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 9);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 15);
	ac++;
	XtSetValues(class_in->mbview_pushButton_routelist_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
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
	XtSetArg(args[ac], XmNtopWidget, class_in->mbview_routelist_label);
	ac++;
	XtSetValues(class_in->mbview_scrolledWindow_routelist, args, ac);

	return (class_in);
}
