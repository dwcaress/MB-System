/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.c	6/24/95
 *
 *    Copyright (c) 1995-2025 by
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
#include <stdlib.h>

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

/* Suppress the one-time "locale not supported by Xlib" / "X locale
 * modifiers not supported" messages that Xt's default language
 * procedure prints via XtWarning() during XtVaOpenApplication() on
 * macOS (whose system Xlib has no locale database at all). The
 * underlying XSupportsLocale()/XSetLocaleModifiers() calls still run
 * normally; only the warning text is discarded. */
static void mb_discard_xt_warning(String message) {
	(void)message;
}

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

	XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

	/* macOS's system Xlib only ships locale data for "C". If the
	 * environment requests a UTF-8 locale, Xt/Motif end up with libc
	 * and Xlib in a mismatched locale state, and Motif repeatedly
	 * warns "Locale not supported for XmbTextListToTextProperty" /
	 * "Cannot convert XmString to compound text" on every dialog.
	 * Forcing the environment to "C" before Xt reads it lets Xt's own
	 * (harmless, one-time) locale fallback complete cleanly instead. */
	setenv("LC_ALL", "C", 1);
	setenv("LANG", "C", 1);
	XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

	XtAppContext app;
	XtSetWarningHandler(mb_discard_xt_warning);
	Widget parent =
            XtVaOpenApplication(&app, BX_APP_CLASS, NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL);
	XtSetWarningHandler((XtErrorHandler)NULL);

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
