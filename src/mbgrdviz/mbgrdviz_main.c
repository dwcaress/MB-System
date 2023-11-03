/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdviz_main.c		10/9/2002
 *
 *    Copyright (c) 2002-2023 by
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
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>

#include "mbgrdviz_creation.h"

#include "mbview.h"

extern void RegisterBxConverters(XtAppContext);
extern XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);
extern XtPointer BX_DOUBLE(double);
extern XtPointer BX_SINGLE(float);
extern void BX_MENU_POST(Widget, XtPointer, XEvent *, Boolean *);
extern Pixmap XPM_PIXMAP(Widget, char **);
extern void BX_SET_BACKGROUND_COLOR(Widget, ArgList, Cardinal *, Pixel);

extern void do_mbgrdviz_quit(Widget, XtPointer, XtPointer);
extern void BxExitCB(Widget, XtPointer, XtPointer);

/* Avoid conflict due to BOOL redefinitions (Xm vs Win headers) */
#ifdef WIN32
#undef BOOL
#endif

Widget mainWindow;

Widget CreatemainWindow_mbgrdviz(Widget parent);
int do_mbgrdviz_init(int argc, char **argv, int verbosity);
void do_mbgrdviz_sensitivity();
int do_mbgrdviz_dismiss_notify(size_t instance);
void do_mbgrdviz_fileSelectionBox(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openoverlay(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opensite(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opennav(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savewinfrogpts(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savewinfrogwpt(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesite(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbgrdviz_openprimary(char *input_file_ptr);
int do_mbgrdviz_openoverlay(size_t instance, char *input_file_ptr);
int do_mbgrdviz_opensite(size_t instance, char *input_file_ptr);
int do_mbgrdviz_savesite(size_t instance, char *output_file_ptr);
int do_mbgrdviz_openroute(size_t instance, char *input_file_ptr);
int do_mbgrdviz_saveroute(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savewinfrogpts(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savewinfrogwpt(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveprofile(size_t instance, char *output_file_ptr);
int do_mbgrdviz_opennav(size_t instance, int swathbounds, char *input_file_ptr);
int do_mbgrdviz_readnav(size_t instance, char *swathfile, int pathstatus, char *pathraw, char *pathprocessed, int format,
                        int formatorg, double weight, int *error);
int do_mbgrdviz_readgrd(size_t instance, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue,
                        int *nxy, int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin,
                        double *ymax, double *dx, double *dy, float **data);
int do_mbgrdviz_opentest(size_t instance, double factor1, double factor2, double factor3, int *grid_projection_mode,
                         char *grid_projection_id, float *nodatavalue, int *nxy, int *n_columns, int *n_rows, double *min, double *max,
                         double *xmin, double *xmax, double *ymin, double *ymax, double *dx, double *dy, float **data);
void do_mbgrdviz_open_region(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_make_survey(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_generate_survey(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_parameterchange(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_recalc(size_t instance);
void do_mbgrdviz_arearoute_info(size_t instance);
void do_mbgrdviz_arearoute_linespacing_increment(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_altitude_increment(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_depth_increment(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtime_start(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_path_reset(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtime_pause(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtime_stop(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtime_resume(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_path_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_icon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_path_browse(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_updaterate(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_path_test(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_realtimesetup_pathmode(Widget w, XtPointer client_data, XtPointer call_data);

static const char program_name[] = "MBgrdviz";
static const char help_message[] = "MBgrdviz provides simple interactive 2D/3Dvizualization of GMT grids.";
static const char usage_message[] = "mbgrdviz [-Igrdfile -T -V -H]";
char ifile[MB_PATH_MAXLINE];
char jfile[MB_PATH_MAXLINE];

/* parsing variables */
extern char *optarg;
extern int optkind;
int errflg = 0;
int c;
int help = 0;
int flag = 0;

// Change this line via the Output Application Names Dialog.
#define BX_APP_CLASS "MB-System"

int main(int argc, char **argv) {
	Widget parent;
	XtAppContext app;
	Arg args[256];
	Cardinal ac;
	Widget topLevelShell;
	Widget mainWindow_mbgrdviz;

	/* Begin user code block <declarations> */
	int error = MB_ERROR_NO_ERROR;
	int verbose = 0;
	int ifileflag = 0;
	int jfileflag = 0;
	int testflag = 0;
	// sessionShellWidgetClass widget_class;
	// widget_class.core_class.class_inited = NULL;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhI:i:J:j:Tt")) != -1) {
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", ifile);
			flag++;
			ifileflag++;
			break;
		case 'J':
		case 'j':
			sscanf(optarg, "%s", jfile);
			flag++;
			jfileflag++;
			break;
		case 'T':
		case 't':
			flag++;
			testflag++;
			break;
		case '?':
			errflg++;
		}
	}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* print starting message */
	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/* print starting message */
	if (help) {
		fprintf(stderr, "\n%s\n\nUsage: %s\n", help_message, usage_message);
		error = MB_ERROR_NO_ERROR;
		exit(error);
	}

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
	XtSetArg(args[ac], XmNtitle, "MBgrdviz");
	ac++;
	XtSetArg(args[ac], XmNx, 108);
	ac++;
	XtSetArg(args[ac], XmNy, 241);
	ac++;
	XtSetArg(args[ac], XmNwidth, 260);
	ac++;
	XtSetArg(args[ac], XmNheight, 215);
	ac++;
	topLevelShell = XtCreatePopupShell((char *)"topLevelShell", topLevelShellWidgetClass, parent, args, ac);
	XtAddCallback(topLevelShell, XmNdestroyCallback, do_mbgrdviz_quit, (XtPointer)0);
	XtAddCallback(topLevelShell, XmNdestroyCallback, BxExitCB, (XtPointer)0);
	mainWindow_mbgrdviz = (Widget)CreatemainWindow_mbgrdviz(topLevelShell);
	XtManageChild(mainWindow_mbgrdviz);
	XtPopup(XtParent(mainWindow_mbgrdviz), XtGrabNone);

	/* Begin user code block <app_procedures> */

	/* set top level widget */
	mainWindow = mainWindow_mbgrdviz;

	/* End user code block <app_procedures> */

	/* Begin user code block <main_loop> */

	/* initialize the vizualization widgets code */
	mbview_startup(verbose, parent, app, &error);

	/* open the file specified on the command line */
	do_mbgrdviz_init(argc, argv, verbose);
	if (ifileflag > 0) {
		do_mbgrdviz_openprimary(ifile);
		if (jfileflag > 0) {
			do_mbgrdviz_openoverlay(0, jfile);
		}
	}
	else if (testflag > 0) {
		do_mbgrdviz_openprimary(NULL);
	}
	/* End user code block <main_loop> */

	XtAppMainLoop(app);

	/*
	 * A return value regardless of whether or not the main loop ends.
	 */
	return (0);
}
