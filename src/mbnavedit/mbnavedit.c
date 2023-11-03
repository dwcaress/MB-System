/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.c	6/24/95
 *
 *    Copyright (c) 1995-2023 by
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
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>

#include "mbnavedit_creation.h"

void RegisterBxConverters(XtAppContext);
void BxExitCB(Widget, XtPointer, XtPointer);

Widget CreatemainWindow(Widget parent);
void do_mbnavedit_init(int argc, char **argv);
int do_wait_until_viewed(XtAppContext app);

static const char BX_APP_CLASS[] = "mbnavedit";

int main(int argc, char **argv) {
	/* make sure that the argc that goes to XtVaAppInitialize
	   is 1 so that no options are removed by its option parsing */
	const int argc_save = argc;
	argc = 1;

        // The applicationShell is created as an unrealized
        // parent for multiple topLevelShells.  The topLevelShells
        // are created as popup children of the applicationShell.
        // This is a recommendation of Paul Asente & Ralph Swick in
        // _X_Window_System_Toolkit_ p. 677.

	XtAppContext app;
	Widget parent =
            XtVaOpenApplication(&app, BX_APP_CLASS, NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL);

	RegisterBxConverters(app);
	XmRepTypeInstallTearOffModelConverter();

	// Create classes and widgets used in this program.

	Cardinal ac = 0;
	Arg args[256];
	XtSetArg(args[ac], XmNtitle, "MBnavedit");
	ac++;
	XtSetArg(args[ac], XmNiconName, "MBnavedit");
	ac++;
	XtSetArg(args[ac], XmNallowShellResize, True);
	ac++;
	XtSetArg(args[ac], XmNx, 964);
	ac++;
	XtSetArg(args[ac], XmNy, 300);
	ac++;
	XtSetArg(args[ac], XmNwidth, 1024);
	ac++;
	XtSetArg(args[ac], XmNheight, 683);
	ac++;
	Widget topLevelShell =
            XtCreatePopupShell((char *)"topLevelShell", topLevelShellWidgetClass, parent, args, ac);
	XtAddCallback(topLevelShell, XmNdestroyCallback, BxExitCB, (XtPointer)0);
	Widget mainWindow = (Widget)CreatemainWindow(topLevelShell);
	XtManageChild(mainWindow);
	XtPopup(XtParent(mainWindow), XtGrabNone);

	/* initialize app value and wait until view realized */
	do_wait_until_viewed(app);

	do_mbnavedit_init(argc_save, argv);

	XtAppMainLoop(app);

	return (0);
}
