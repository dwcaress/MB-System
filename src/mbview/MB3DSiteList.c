/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DSiteList.h	10/28/2003
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
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include "MB3DSiteList.h"

/**
 * Common constant and pixmap declarations.
 */
#include "creation-c.h"

void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/**
 * Declarations for callbacks and handlers.
 */
void do_mbview_sitelist_popdown(Widget, XtPointer, XtPointer);
void do_mbview_sitelist_delete(Widget, XtPointer, XtPointer);
void do_mbview_sitelistselect(Widget, XtPointer, XtPointer);

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

MB3DSiteListDataPtr MB3DSiteListCreate(MB3DSiteListDataPtr class_in, Widget parent, String name, ArgList args_in,
                                       Cardinal ac_in) {
	(void)args_in;  // Unused parameter
	(void)ac_in;  // Unused parameter

	// Register the converters for the widgets.
	RegisterBxConverters(XtWidgetToApplicationContext(parent));
	XtInitializeWidgetClass((WidgetClass)xmFormWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmPushButtonWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmLabelWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmScrolledWindowWidgetClass);
	XtInitializeWidgetClass((WidgetClass)xmListWidgetClass);

	// Setup app-defaults fallback table if not already done.
	if (doInitAppDefaults) {
		InitAppDefaults(parent, appDefaults);
		doInitAppDefaults = False;
	}

	SetAppDefaults(parent, appDefaults, name, False);

	Cardinal ac = 0;
	Arg args[256];
	Boolean argok = False;

	XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_GROW);
	ac++;
	XtSetArg(args[ac], XmNx, 87);
	ac++;
	XtSetArg(args[ac], XmNy, 496);
	ac++;
	XtSetArg(args[ac], XmNwidth, 411);
	ac++;
	XtSetArg(args[ac], XmNheight, 286);
	ac++;
	class_in->MB3DSiteList = XmCreateForm(parent, (char *)name, args, ac);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DSiteList, (char *)"Dismiss", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 290);
		ac++;
		XtSetArg(args[ac], XmNy, 240);
		ac++;
		XtSetArg(args[ac], XmNwidth, 110);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DSiteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_sitelist_dismiss =
		    XmCreatePushButton(class_in->MB3DSiteList, (char *)"mbview_pushButton_sitelist_dismiss", args, ac);
		XtManageChild(class_in->mbview_pushButton_sitelist_dismiss);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_sitelist_dismiss, XmNactivateCallback, do_mbview_sitelist_popdown, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DSiteList, (char *)"Delete Selected Sites", XmRXmString, 0, &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetArg(args[ac], XmNx, 10);
		ac++;
		XtSetArg(args[ac], XmNy, 240);
		ac++;
		XtSetArg(args[ac], XmNwidth, 170);
		ac++;
		XtSetArg(args[ac], XmNheight, 30);
		ac++;
		XtSetArg(args[ac], XmNfontList,
		         BX_CONVERT(class_in->MB3DSiteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_pushButton_sitelist_delete =
		    XmCreatePushButton(class_in->MB3DSiteList, (char *)"mbview_pushButton_sitelist_delete", args, ac);
		XtManageChild(class_in->mbview_pushButton_sitelist_delete);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	XtAddCallback(class_in->mbview_pushButton_sitelist_delete, XmNactivateCallback, do_mbview_sitelist_delete, (XtPointer)0);

	ac = 0;
	{
		XmString tmp0 = (XmString)BX_CONVERT(class_in->MB3DSiteList, (char *)"Site List:", XmRXmString, 0, &argok);
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
		         BX_CONVERT(class_in->MB3DSiteList, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1", XmRFontList, 0,
		                    &argok));
		if (argok)
			ac++;
		class_in->mbview_sitelist_label = XmCreateLabel(class_in->MB3DSiteList, (char *)"mbview_sitelist_label", args, ac);
		XtManageChild(class_in->mbview_sitelist_label);

		/**
		 * Free any memory allocated for resources.
		 */
		XmStringFree((XmString)tmp0);
	}

	ac = 0;
	XtSetArg(args[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED);
	ac++;
	XtSetArg(args[ac], XmNx, 10);
	ac++;
	XtSetArg(args[ac], XmNy, 46);
	ac++;
	XtSetArg(args[ac], XmNwidth, 390);
	ac++;
	XtSetArg(args[ac], XmNheight, 180);
	ac++;
	class_in->mbview_scrolledWindow_sitelist =
	    XmCreateScrolledWindow(class_in->MB3DSiteList, (char *)"mbview_scrolledWindow_sitelist", args, ac);
	XtManageChild(class_in->mbview_scrolledWindow_sitelist);

	ac = 0;
	XtSetArg(args[ac], XmNselectionPolicy, XmEXTENDED_SELECT);
	ac++;
	XtSetArg(args[ac], XmNwidth, 390);
	ac++;
	XtSetArg(args[ac], XmNheight, 180);
	ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(class_in->mbview_scrolledWindow_sitelist, (char *)"-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	class_in->mbview_list_sitelist =
	    XmCreateList(class_in->mbview_scrolledWindow_sitelist, (char *)"mbview_list_sitelist", args, ac);
	XtManageChild(class_in->mbview_list_sitelist);
	XtAddCallback(class_in->mbview_list_sitelist, XmNsingleSelectionCallback, do_mbview_sitelistselect, (XtPointer)0);
	XtAddCallback(class_in->mbview_list_sitelist, XmNextendedSelectionCallback, do_mbview_sitelistselect, (XtPointer)0);
	XtAddCallback(class_in->mbview_list_sitelist, XmNbrowseSelectionCallback, do_mbview_sitelistselect, (XtPointer)0);
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 16);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 11);
	ac++;
	XtSetValues(class_in->mbview_pushButton_sitelist_dismiss, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_NONE);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 16);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_pushButton_sitelist_delete, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 11);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 10);
	ac++;
	XtSetValues(class_in->mbview_sitelist_label, args, ac);

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomOffset, 60);
	ac++;
	XtSetArg(args[ac], XmNleftOffset, 10);
	ac++;
	XtSetArg(args[ac], XmNrightOffset, 11);
	ac++;
	XtSetArg(args[ac], XmNtopOffset, 6);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, class_in->mbview_sitelist_label);
	ac++;
	XtSetValues(class_in->mbview_scrolledWindow_sitelist, args, ac);

	return (class_in);
}
