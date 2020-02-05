#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>

#include "mbnavedit_creation.h"

void RegisterBxConverters(XtAppContext);
XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);
XtPointer BX_DOUBLE(double);
XtPointer BX_SINGLE(float);
void BX_MENU_POST(Widget, XtPointer, XEvent *, Boolean *);
Pixmap XPM_PIXMAP(Widget, char **);
void BX_SET_BACKGROUND_COLOR(Widget, ArgList, Cardinal *, Pixel);

void BxExitCB(Widget, XtPointer, XtPointer);

Widget CreatemainWindow(Widget parent);
void do_mbnavedit_init(int argc, char **argv);
int do_wait_until_viewed(XtAppContext app);

#define BX_APP_CLASS "mbnavedit"

int main(int argc, char **argv) {
	Widget parent;
	XtAppContext app;
	Arg args[256];
	Cardinal ac;
	Widget topLevelShell;
	Widget mainWindow;

	/* Begin user code block <declarations> */

	/* make sure that the argc that goes to XtVaAppInitialize
	   is 1 so that no options are removed by its option parsing */
	int argc_save;
	argc_save = argc;
	argc = 1;

	/* End user code block <declarations> */

	/*
	 * Initialize Xt.
	 */

	// XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

	/*
	 * The applicationShell is created as an unrealized
	 * parent for multiple topLevelShells.  The topLevelShells
	 * are created as popup children of the applicationShell.
	 * This is a recommendation of Paul Asente & Ralph Swick in
	 * _X_Window_System_Toolkit_ p. 677.
	 */

	parent = XtVaOpenApplication(&app, BX_APP_CLASS, NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL);

	RegisterBxConverters(app);
	XmRepTypeInstallTearOffModelConverter();

	/* Begin user code block <create_shells> */
	/* End user code block <create_shells> */

	/*
	 * Create classes and widgets used in this program.
	 */

	/* Begin user code block <create_topLevelShell> */
	/* End user code block <create_topLevelShell> */

	ac = 0;
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
	topLevelShell = XtCreatePopupShell((char *)"topLevelShell", topLevelShellWidgetClass, parent, args, ac);
	XtAddCallback(topLevelShell, XmNdestroyCallback, BxExitCB, (XtPointer)0);
	mainWindow = (Widget)CreatemainWindow(topLevelShell);
	XtManageChild(mainWindow);
	XtPopup(XtParent(mainWindow), XtGrabNone);

	/* Begin user code block <app_procedures> */

	/* initialize app value and wait until view realized */
	do_wait_until_viewed(app);

	/* initialize everything */
	do_mbnavedit_init(argc_save, argv);

	/* End user code block <app_procedures> */

	/* Begin user code block <main_loop> */
	/* End user code block <main_loop> */

	XtAppMainLoop(app);

	return (0);
}
