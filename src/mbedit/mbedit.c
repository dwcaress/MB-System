/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.c	1993
 *
 *    Copyright (c) 1993-2023 by
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

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#include <stdbool.h>

#include <X11/StringDefs.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/RepType.h>
#include <Xm/Xm.h>

// TODO(schwehr): Make medit headers follow UWYU to not be after X11/Xm.
#include "mbedit.h"
#include "mbedit_creation.h"

// TODO(schwehr): These should be in headers.
extern void RegisterBxConverters(XtAppContext);
extern Pixmap XPM_PIXMAP(Widget, char **);
Widget Createwindow_mbedit(Widget parent);

#define BX_APP_CLASS "mbedit"

int main(int argc, char **argv) {
	/* make sure that the argc that goes to XtVaAppInitialize
	   is 1 so that no options are removed by its option parsing */
	int argc_save;
	argc_save = argc;
	argc = 1;

	/*
	 * The applicationShell is created as an unrealized
	 * parent for multiple topLevelShells.  The topLevelShells
	 * are created as popup children of the applicationShell.
	 * This is a recommendation of Paul Asente & Ralph Swick in
	 * _X_Window_System_Toolkit_ p. 677.
	 */
	XtAppContext app;
	Widget parent = XtVaOpenApplication(&app, BX_APP_CLASS, NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL);

	RegisterBxConverters(app);
	XmRepTypeInstallTearOffModelConverter();

	Arg args[256];
	Cardinal ac = 0;
	XtSetArg(args[ac], XmNtitle, "MBedit");
	ac++;
	XtSetArg(args[ac], XmNiconName, "MBedit");
	ac++;
	XtSetArg(args[ac], XmNallowShellResize, False);
	ac++;
	XtSetArg(args[ac], XmNdeleteResponse, XmDESTROY);
	ac++;
	XtSetArg(args[ac], XmNx, 114);
	ac++;
	XtSetArg(args[ac], XmNy, 631);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1014);
	ac++;
	XtSetArg(args[ac], XmNheight, 663);
	ac++;
	Widget topLevelShell = XtCreatePopupShell((char *)"topLevelShell", topLevelShellWidgetClass, parent, args, ac);
	XtAddCallback(topLevelShell, XmNdestroyCallback, do_quit, (XtPointer)0);
	window_mbedit = (Widget)Createwindow_mbedit(topLevelShell);
	XtManageChild(window_mbedit);
	XtPopup(XtParent(window_mbedit), XtGrabNone);

	/* initialize app value and wait until view realized */
	do_wait_until_viewed(app);

	/* initialize everything */
	do_mbedit_init(argc_save, argv);

	XtAppMainLoop(app);

	return (0);
}
