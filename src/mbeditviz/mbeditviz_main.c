/*--------------------------------------------------------------------
 *    The MB-system:	mbeditviz_main.c		4/27/2007
 *
 *    Copyright (c) 2007-2025 by
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

#include <stdio.h>
#include <stdlib.h>

#include "mb_status.h"
#include "mb_define.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>

#include "mbview.h"
#include "mbeditviz.h"
#include "mbeditviz_creation.h"

/* Avoid conflict due to BOOL redefinitions (Xm vs Win headers) */
#ifdef WIN32
#undef BOOL
#endif

/* function prototypes */
Widget CreatemainWindow_mbeditviz(Widget parent);

/* use these parameters only when debugging X events */
#ifdef MBEDITVIZ_DEBUG
char eventname[64];
XEvent event;
XAnyEvent *xany;
XKeyEvent *xkey;
XButtonEvent *xbutton;
XMotionEvent *xmotion;
XCrossingEvent *xcrossing;
XFocusChangeEvent *xfocus;
XExposeEvent *xexpose;
XGraphicsExposeEvent *xgraphicsexpose;
XNoExposeEvent *xnoexpose;
XVisibilityEvent *xvisibility;
XCreateWindowEvent *xcreatewindow;
XDestroyWindowEvent *xdestroywindow;
XUnmapEvent *xunmap;
XMapEvent *xmap;
XMapRequestEvent *xmaprequest;
XReparentEvent *xreparent;
XConfigureEvent *xconfigure;
XGravityEvent *xgravity;
XResizeRequestEvent *xresizerequest;
XConfigureRequestEvent *xconfigurerequest;
XCirculateEvent *xcirculate;
XCirculateRequestEvent *xcirculaterequest;
XPropertyEvent *xproperty;
XSelectionClearEvent *xselectionclear;
XSelectionRequestEvent *xselectionrequest;
XSelectionEvent *xselection;
XColormapEvent *xcolormap;
XClientMessageEvent *xclient;
XMappingEvent *xmapping;
XErrorEvent *xerror;
XKeymapEvent *xkeymap;
#endif

// Change this line via the Output Application Names Dialog.
#define BX_APP_CLASS "MB-System"

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
	const int argcsave = argc;
	argc = 1;

	// The applicationShell is created as an unrealized
	// parent for multiple topLevelShells.  The topLevelShells
	// are created as popup children of the applicationShell.
	// This is a recommendation of Paul Asente & Ralph Swick in
	// _X_Window_System_Toolkit_ p. 677.
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
	Widget parent = XtVaOpenApplication(&app, BX_APP_CLASS, NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL);
	XtSetWarningHandler((XtErrorHandler)NULL);

	RegisterBxConverters(app);
	XmRepTypeInstallTearOffModelConverter();

	// Create classes and widgets used in this program.
	Arg args[256];
	Cardinal ac = 0;
	XtSetArg(args[ac], XmNtitle, "MBeditviz");
	ac++;
	XtSetArg(args[ac], XmNx, 180);
	ac++;
	XtSetArg(args[ac], XmNy, 583);
	ac++;
	XtSetArg(args[ac], XmNwidth, 453);
	ac++;
	XtSetArg(args[ac], XmNheight, 557);
	ac++;
	Widget topLevelShell = XtCreatePopupShell((char *)"topLevelShell", topLevelShellWidgetClass, parent, args, ac);
	XtAddCallback(topLevelShell, XmNdestroyCallback, do_mbeditviz_quit, (XtPointer)0);
	XtAddCallback(topLevelShell, XmNdestroyCallback, BxExitCB, (XtPointer)0);
	Widget mainWindow_mbeditviz = (Widget)CreatemainWindow_mbeditviz(topLevelShell);
	XtManageChild(mainWindow_mbeditviz);
	XtPopup(XtParent(mainWindow_mbeditviz), XtGrabNone);

	/* initialize the gui code */
	do_mbeditviz_init(parent, app);

	/* initialize the vizualization widgets code */
	mbview_startup(mbev_verbose, parent, app, &mbev_error);
	mb3dsoundings_startup(mbev_verbose, parent, app, &mbev_error);
	mb3dsoundings_set_dismiss_notify(mbev_verbose, mbeditviz_mb3dsoundings_dismiss, &mbev_error);
	mb3dsoundings_set_edit_notify(mbev_verbose, mbeditviz_mb3dsoundings_edit, &mbev_error);

	/* initialize app value and wait until view realized */
	do_wait_until_viewed();

	mbeditviz_init(argcsave, argv,
                       "MBeditviz",
                       "MBeditviz is a bathymetry editor and ptch test tool",
                       "mbeditviz\n"
                       "\t--delete-input-file {-R}\n"
                       "\t--format=format_id {-Fformat_id}\n"
                       "\t--help {-H}\n"
                       "\t--input=file {-Ifile}\n"
                       "\t--simple-mean-grid {-G}\n"
                       "\t--verbose {-V}\n",
                       &do_mbeditviz_message_on,
                       &do_mbeditviz_message_off,
                       &do_mbeditviz_update_gui,
                       &do_error_dialog);

	XtAppMainLoop(app);

	// A return value regardless of whether or not the main loop ends.
	return (0);
}
