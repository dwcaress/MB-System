/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_callbacks.c	2/22/2000
 *
 *    Copyright (c) 2000-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the callback functions from the MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	March 22, 2000
 *
 *
 */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Xm.h>
#include <Xm/List.h>

#define MBNAVADJUST_DECLARE_GLOBALS
#include "mbnavadjust_extrawidgets.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mb_aux.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
#include "mb_xgraphics.h"
#include "mbview.h"

#include "mbnavadjust_creation.h"

/*--------------------------------------------------------------------*/

/*
 * Standard includes for builtins.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * Macros to make code look nicer between ANSI and K&R.
 */
#ifndef ARGLIST
#if (NeedFunctionPrototypes == 0)
#define PROTOTYPE(p) ()
#define ARGLIST(p) p
#define ARG(a, b) a b;
#define GRA(a, b) a b;
#define UARG(a, b) a b;
#define GRAU(a, b) a b;
#else
#define PROTOTYPE(p) p
#define ARGLIST(p)	(
#define ARG(a, b) a b,
#define GRA(a, b)	a b)
#ifdef __cplusplus
#define UARG(a, b) a,
#define GRAU(a, b)      a)
#else
#define UARG(a, b) a b,
#define GRAU(a, b)      a b)
#endif
#endif
#endif

#ifndef FIXED
#define FIXED "fixed"
#endif

Widget BxFindTopShell PROTOTYPE((Widget));
WidgetList BxWidgetIdsFromNames PROTOTYPE((Widget, char *, char *));

/*--------------------------------------------------------------------*/

/* id variables */
static const char program_name[] = "MBnavadjust";

/* XG variable declarations */
#define xgfont "-*-" FIXED "-bold-r-normal-*-13-*-75-75-c-70-iso8859-1"
#define EV_MASK (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask)
XtAppContext app_context;
Display *display;
Window cont_xid, corr_xid, zoff_xid;
Colormap colormap;
GC cont_gc, corr_gc, modp_gc;
XGCValues xgcv;
XFontStruct *fontStruct;
#define XG_SOLIDLINE 0
#define XG_DASHLINE 1
void *cont_xgid = NULL; /* XG graphics id */
void *corr_xgid = NULL; /* XG graphics id */
void *zoff_xgid = NULL; /* XG graphics id */
void *modp_xgid = NULL; /* XG graphics id */
Cursor myCursor;

/* Set the colors used for this program here. */
#define NCOLORS 256
XColor colors[NCOLORS];
unsigned int mpixel_values[NCOLORS];
XColor db_color;

/* Set these to the dimensions of your canvas drawing */
/* areas, minus 1, located in the uil file             */
static int cont_borders[4] = {0, 600, 0, 600};
static int corr_borders[4] = {0, 301, 0, 301};
static int zoff_borders[4] = {0, 300, 0, 60};
static int modp_borders[4];

/* file opening parameters */
#define FILE_MODE_NONE 0
#define FILE_MODE_NEW 1
#define FILE_MODE_OPEN 2
#define FILE_MODE_IMPORT 3
size_t file_mode = FILE_MODE_NONE;
int format = 0;
int expose_plot_ok = True;
int selected = 0; /* indicates an input file is selected */

/* button parameters */
static int button1down = MB_NO;
static int button2down = MB_NO;
static int button3down = MB_NO;
static int loc_x, loc_y;

int status;
char string[STRING_MAX];

Cardinal ac = 0;
Arg args[256];
Boolean argok;
XmString tmp0;

/*--------------------------------------------------------------------*/
/*      Function Name: 	BxManageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxManageCB attempts to convert the name to a Widget
 *			ID and manage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and manage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */

/* ARGSUSED */
void BxManageCB ARGLIST((w, client, call)) ARG(Widget, w) ARG(XtPointer, client) GRAU(XtPointer, call) {
	WidgetList widgets;
	int i;

	/*
	 * This function returns a NULL terminated WidgetList.  The memory for
	 * the list needs to be freed when it is no longer needed.
	 */
	widgets = BxWidgetIdsFromNames(w, "BxManageCB", (String)client);

	i = 0;
	while (widgets && widgets[i] != NULL) {
		XtManageChild(widgets[i]);
		i++;
	}
	XtFree((char *)widgets);
}

/*--------------------------------------------------------------------*/
/*      Function Name:	BxSetValuesCB
 *
 *      Description:   	This function accepts a string of the form:
 *			"widgetName.resourceName = value\n..."
 *			It then attempts to convert the widget name to a widget
 *			ID and the value to a valid resource value.  It then
 *			sets the value on the given widget.
 *
 *      Arguments:      Widget		w:	the activating widget.
 *			XtPointer	client:	the set values string.
 *			XtPointer	call:	the call data (unused).
 *
 *      Notes:        * This function expects that there is an application
 *                      shell from which all other widgets are descended.
 */
#include <X11/StringDefs.h>

/* ARGSUSED */
void BxSetValuesCB ARGLIST((w, client, call)) ARG(Widget, w) ARG(XtPointer, client) GRAU(XtPointer, call) {
#define CHUNK 512

	Boolean first = True;
	String rscs = XtNewString((String)client);
	String *valueList = (String *)XtCalloc(CHUNK, sizeof(String));
	char *start;
	char *ptr, *cptr;
	String name;
	String rsc;
	int i, count = 0;
	Widget *current;

	for (start = rscs; rscs && *rscs; rscs = strtok(NULL, "\n")) {
		if (first) {
			rscs = strtok(rscs, "\n");
			first = False;
		}
		valueList[count] = XtNewString(rscs);
		count++;
		if (count == CHUNK) {
			valueList = (String *)XtRealloc((char *)valueList, (count + CHUNK) * sizeof(String));
		}
	}
	XtFree((char *)start);

	for (i = 0; i < count; i++) {
		/*
		 * First, extract the widget name and generate a string to
		 * pass to BxWidgetIdsFromNames().
		 */
		cptr = strrchr(valueList[i], '.');
		if (cptr != NULL) {
			*cptr = '\000';
		}
		else {
			printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
			XtFree((char *)(valueList[i]));
			continue;
		}
		name = valueList[i];
		while ((name && *name) && isspace(*name)) {
			name++;
		}
		ptr = name + strlen(name) - 1;
		while (ptr && *ptr) {
			if (isspace(*ptr)) {
				ptr--;
			}
			else {
				ptr++;
				break;
			}
		}
		if (ptr && *ptr) {
			*ptr = '\0';
		}
		if (ptr == NULL) {
			printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
			XtFree((char *)(valueList[i]));
			return;
		}

		/*
		 * Next, get the resource name to set.
		 */
		rsc = ++cptr;
		cptr = strchr(rsc, '=');
		if (cptr != NULL) {
			*cptr = '\000';
		}
		else {
			printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
			XtFree((char *)(valueList[i]));
			continue;
		}
		while ((rsc && *rsc) && isspace(*rsc)) {
			rsc++;
		}

		ptr = rsc + strlen(rsc) - 1;
		while (ptr && *ptr) {
			if (isspace(*ptr)) {
				ptr--;
			}
			else {
				ptr++;
				break;
			}
		}
		if (ptr && *ptr) {
			*ptr = '\0';
		}

		/*
		 * Lastly, get the string value to which to set the resource.
		 */
		start = ++cptr;
		while ((start && *start) && isspace(*start)) {
			start++;
		}

		if (start == NULL) {
			printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
			XtFree((char *)(valueList[i]));
			return;
		}

		ptr = start + strlen(start) - 1;
		while (ptr && *ptr) {
			if (isspace(*ptr)) {
				ptr--;
			}
			else {
				ptr++;
				break;
			}
		}
		if (ptr && *ptr) {
			*ptr = '\0';
		}

		/*
		 * Now convert the widget name to a Widget ID
		 */
		current = BxWidgetIdsFromNames(w, "BxSetValuesCB", name);
		if (current[0] == NULL) {
			XtFree((char *)(valueList[i]));
			continue;
		}

		/*
		 * If the widget name conversion succeeded, we now need to get the
		 * resource list for the widget so that we can do a resource conversion
		 * of the value.
		 */
		XtVaSetValues(*current, XtVaTypedArg, rsc, XtRString, start, strlen(start) + 1, NULL);
		XtFree((char *)(valueList[i]));
	}
	XtFree((char *)valueList);

#undef CHUNK
}

/*--------------------------------------------------------------------*/
/*      Function Name: 	BxUnmanageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxUnmanageCB attempts to convert the name to a Widget
 *			ID and unmanage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and unmanage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */

/* ARGSUSED */
void BxUnmanageCB ARGLIST((w, client, call)) ARG(Widget, w) ARG(XtPointer, client) GRAU(XtPointer, call) {
	WidgetList widgets;
	int i;

	/*
	 * This function returns a NULL terminated WidgetList.  The memory for
	 * the list needs to be freed when it is no longer needed.
	 */
	widgets = BxWidgetIdsFromNames(w, "BxUnmanageCB", (String)client);

	i = 0;
	while (widgets && widgets[i] != NULL) {
		XtUnmanageChild(widgets[i]);
		i++;
	}
	XtFree((char *)widgets);
}

/*--------------------------------------------------------------------*/
/*      Function Name:	BxExitCB
 *
 *      Description:   	This functions expects an integer to be passed in
 *		       	client data.  It calls the exit() system call with
 *			the integer value as the argument to the function.
 *
 *      Arguments:      Widget		w: 	the activating widget.
 *			XtPointer	client:	the integer exit value.
 *			XtPointer	call:	the call data (unused).
 */

#ifdef VMS
#include <stdlib.h>
#endif

/* ARGSUSED */
void BxExitCB ARGLIST((w, client, call)) UARG(Widget, w) ARG(XtPointer, client) GRAU(XtPointer, call) {
	long exitValue = EXIT_FAILURE;
	exit(exitValue);
}

/*--------------------------------------------------------------------*/

void do_mbnavadjust_init(int argc, char **argv) {
	int i, j;
	String translations = "<Btn1Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn1Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn1Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <Btn2Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn2Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn2Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <Btn3Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn3Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn3Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <KeyDown>:		DrawingAreaInput() \n\
	     <KeyUp>:		DrawingAreaInput() ManagerGadgetKeyInput()";

	/* get additional widgets */
	fileSelectionBox_list = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_LIST);
	fileSelectionBox_text = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_TEXT);
	XtAddCallback(fileSelectionBox_list, XmNbrowseSelectionCallback, do_fileselection_list, NULL);

	XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_HELP_BUTTON));
	ac = 0;
	tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.nvh", XmRXmString, 0, &argok);
	XtSetArg(args[ac], XmNpattern, tmp0);
	ac++;
	XtSetValues(fileSelectionBox, args, ac);
	XmStringFree((XmString)tmp0);

	/* reset translation table for drawingArea widgets */
	XtVaSetValues(drawingArea_naverr_cont, XmNtranslations, XtParseTranslationTable(translations), NULL);
	XtVaSetValues(drawingArea_naverr_corr, XmNtranslations, XtParseTranslationTable(translations), NULL);
	XtVaSetValues(drawingArea_naverr_zcorr, XmNtranslations, XtParseTranslationTable(translations), NULL);
	XtVaSetValues(drawingArea_modelplot, XmNtranslations, XtParseTranslationTable(translations), NULL);

	/* add resize event handler to modelplot */
	XtAddEventHandler(bulletinBoard_modelplot, StructureNotifyMask, False, do_modelplot_resize, (XtPointer)0);

	/* Setup the entire screen. */
	display = XtDisplay(form_mbnavadjust);
	colormap = DefaultColormap(display, XDefaultScreen(display));

	/* Load the colors that will be used in this program. */
	status = XLookupColor(display, colormap, "white", &db_color, &colors[0]);
	if ((status = XAllocColor(display, colormap, &colors[0])) == 0)
		fprintf(stderr, "Failure to allocate color: white\n");
	status = XLookupColor(display, colormap, "black", &db_color, &colors[1]);
	if ((status = XAllocColor(display, colormap, &colors[1])) == 0)
		fprintf(stderr, "Failure to allocate color: black\n");
	status = XLookupColor(display, colormap, "red", &db_color, &colors[2]);
	if ((status = XAllocColor(display, colormap, &colors[2])) == 0)
		fprintf(stderr, "Failure to allocate color: red\n");
	status = XLookupColor(display, colormap, "green", &db_color, &colors[3]);
	if ((status = XAllocColor(display, colormap, &colors[3])) == 0)
		fprintf(stderr, "Failure to allocate color: green\n");
	status = XLookupColor(display, colormap, "blue", &db_color, &colors[4]);
	if ((status = XAllocColor(display, colormap, &colors[4])) == 0)
		fprintf(stderr, "Failure to allocate color: blue\n");
	status = XLookupColor(display, colormap, "coral", &db_color, &colors[5]);
	if ((status = XAllocColor(display, colormap, &colors[5])) == 0)
		fprintf(stderr, "Failure to allocate color: coral\n");
	status = XLookupColor(display, colormap, "yellow", &db_color, &colors[6]);
	if ((status = XAllocColor(display, colormap, &colors[6])) == 0)
		fprintf(stderr, "Failure to allocate color: yellow\n");
	j = 7;
	for (i = 0; i < 16; i++) {
		colors[j + i].red = 65535;
		/* colors[j+i].green = i * 4096; */
		colors[j + i].green = i * 2048;
		colors[j + i].blue = 0;
		status = XAllocColor(display, colormap, &colors[j + i]);
		if (status == 0) {
			fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j + i, colors[j + i].red, colors[j + i].green,
			        colors[j + i].blue);
		}
	}
	j += 16;
	for (i = 0; i < 16; i++) {
		colors[j + i].red = 65535 - i * 4096;
		/* colors[j+i].green = 65535; */
		colors[j + i].green = 32767 + i * 2048;
		colors[j + i].blue = 0;
		status = XAllocColor(display, colormap, &colors[j + i]);
		if (status == 0) {
			fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j + i, colors[j + i].red, colors[j + i].green,
			        colors[j + i].blue);
		}
	}
	j += 16;
	for (i = 0; i < 16; i++) {
		colors[j + i].red = 0;
		colors[j + i].green = 65535;
		colors[j + i].blue = i * 4096;
		status = XAllocColor(display, colormap, &colors[j + i]);
		if (status == 0) {
			fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j + i, colors[j + i].red, colors[j + i].green,
			        colors[j + i].blue);
		}
	}
	j += 16;
	for (i = 0; i < 16; i++) {
		colors[j + i].red = 0;
		colors[j + i].green = 65535 - i * 4096;
		colors[j + i].blue = 65535;
		status = XAllocColor(display, colormap, &colors[j + i]);
		if (status == 0) {
			fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j + i, colors[j + i].red, colors[j + i].green,
			        colors[j + i].blue);
		}
	}
	j += 16;
	for (i = 0; i < 16; i++) {
		colors[j + i].red = i * 4096;
		colors[j + i].green = 0;
		colors[j + i].blue = 65535;
		status = XAllocColor(display, colormap, &colors[j + i]);
		if (status == 0) {
			fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j + i, colors[j + i].red, colors[j + i].green,
			        colors[j + i].blue);
		}
	}
	j += 16;
	colors[j].red = 65535;
	colors[j].green = 0;
	colors[j].blue = 65535;
	status = XAllocColor(display, colormap, &colors[j]);
	if (status == 0) {
		fprintf(stderr, "Failure to allocate color[%d]: %d %d %d\n", j, colors[j].red, colors[j].green, colors[j].blue);
	}
	for (i = 0; i < NCOLORS; i++) {
		mpixel_values[i] = colors[i].pixel;
	}
	status = mbnavadjust_set_colors(NCOLORS, (int *)mpixel_values);
	status = mbnavadjust_set_borders(cont_borders, corr_borders, zoff_borders);

	/* set verbose */
	mbna_verbose = 0;

	/* put up info text */
	sprintf(string, "Program MBnavadjust initialized.\nMB-System Release %s %s\n", MB_VERSION, MB_BUILD_DATE);
	do_info_add(string, MB_YES);

	/* initialize mbnavadjust proper */
	status = mbnavadjust_init_globals();
	status = mbnavadjust_init(argc, argv);
	do_set_controls();
	do_update_status();
}

/*--------------------------------------------------------------------*/

void do_set_controls() {
	char value_text[128];

	/* set about version label */
	sprintf(value_text, ":::t\"MB-System Release %s\":t\"%s\"", MB_VERSION, MB_BUILD_DATE);
	set_label_multiline_string(label_about_version, value_text);

	/* set value of format text item */
	sprintf(string, "%2.2d", format);
	XmTextFieldSetString(textField_format, string);

	/* set model view style togglebuttons */
	if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
		XmToggleButtonSetState(toggleButton_modelplot_timeseries, TRUE, TRUE);
	}
	else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
		XmToggleButtonSetState(toggleButton_modelplot_perturbation, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(toggleButton_modelplot_tieoffsets, TRUE, TRUE);
	}
}
/*--------------------------------------------------------------------*/

void do_update_status() {
	XmString *xstr;
	struct mbna_file *file;
	struct mbna_file *file2;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	char status_char;
	char truecrossing;
	int iselect, ivalue, imax;
	int num_surveys, num_blocks, num_files, num_crossings, num_ties, num_sections;
	double btime_d, etime_d;
	int btime_i[7], etime_i[7];
	double dr1, dr2, dr3, rs;
	double dx, dy, dz, s;
	char *tiestatus;
	char *tiestatus_xyz = "XYZ";
	char *tiestatus_xy = "XY ";
	char *tiestatus_z = "  Z";
	char *filestatus;
	char *filestatus_poor = " poor  ";
	char *filestatus_good = " good  ";
	char *filestatus_fixed = " fixed ";
	char *filestatus_fixedxy = "fixedxy";
	char *filestatus_fixedz = "fixedz ";
	char *filestatus_unknown = "unknown";
    int *tie_list = NULL;
	int n_tcrossing = 0;
	int n_50crossing = 0;
	int n_25crossing = 0;
	int n_allcrossing = 0;
	int n_tie = 0;
	int i, ii, j, k, kk;

	/* set status label */
	sprintf(string,
	        ":::t\"Project: %s\":t\"Number of Files:                             %4d     Selected Survey:%4d\":t\"Number of "
	        "Crossings Found:           %4d     Selected File:    %4d\":t\"Number of Crossings Analyzed:       %4d     Selected "
	        "Section:%4d\":t\"Number of True Crossings:              %4d     Selected Crossing:%4d\":t\"Number of True Crossings "
	        "Analyzed:%4d     Selected Tie:   %4d\":t\"Number of Ties Set:                         %d\"",
	        project.name, project.num_files, mbna_survey_select, project.num_crossings, mbna_file_select,
	        project.num_crossings_analyzed, mbna_section_select, project.num_truecrossings, mbna_crossing_select,
	        project.num_truecrossings_analyzed, mbna_tie_select, project.num_ties);
	if (project.inversion_status == MBNA_INVERSION_CURRENT)
		strcat(string, ":t\"Inversion Performed:                       Current\"");
	else if (project.inversion_status == MBNA_INVERSION_OLD)
		strcat(string, ":t\"Inversion Performed:                       Out of Date\"");
	else
		strcat(string, ":t\"Inversion Performed:                       No\"");
	if (project.grid_status == MBNA_GRID_CURRENT)
		strcat(string, ":t\"Topography Grid Status:                    Current\"");
	else if (project.grid_status == MBNA_GRID_OLD)
		strcat(string, ":t\"Topography Grid Status:                    Out of Date\"");
	else
		strcat(string, ":t\"Topography Grid Status:                    Not made yet\"");
	set_label_multiline_string(label_status, string);
	if (mbna_verbose > 0) {
		sprintf(string,
		        "Project:                                       %s\nNumber of Files:                           %d\nNumber of "
		        "Crossings Found:         %d\nNumber of Crossings Analyzed:     %d\nNumber of True Crossings:        %d\nNumber "
		        "of True Crossings Analyzed:%d\nNumber of Ties Set:                     %d\n",
		        project.name, project.num_files, project.num_crossings, project.num_crossings_analyzed, project.num_truecrossings,
		        project.num_truecrossings_analyzed, project.num_ties);
		if (project.inversion_status == MBNA_INVERSION_CURRENT)
			strcat(string, "Inversion Performed:                    Current\n");
		else if (project.inversion_status == MBNA_INVERSION_OLD)
			strcat(string, "Inversion Performed:                    Out of Date\n");
		else
			strcat(string, "Inversion Performed:                    No\n");
		if (project.grid_status == MBNA_GRID_CURRENT)
			strcat(string, "Topography Grid Status:                 Current\n");
		else if (project.grid_status == MBNA_GRID_OLD)
			strcat(string, "Topography Grid Status:                 Out of Date\n");
		else
			strcat(string, "Topography Grid Status:                 Not made yet\n");
		fprintf(stderr, "%s", string);
	}

	/* set list_data */
	iselect = MBNA_SELECT_NONE;
	XmListDeleteAllItems(list_data);
	if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
		sprintf(string, "Surveys:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count the number of surveys */
			num_surveys = 0;
			num_files = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if (file->block == num_surveys) {
					num_surveys++;
					num_files = 1;
				}
				else
					num_files++;
			}
			xstr = (XmString *)malloc(num_surveys * sizeof(XmString));

			/* generate list */
			num_surveys = 0;
			num_files = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);

				if (i == 0) {
					btime_d = file->sections[0].btime_d;
				}
				if (file->block == num_surveys) {
					/* find end time for this block */
					num_files = 0;
					btime_d = file->sections[0].etime_d;
					for (ii = i; ii < project.num_files; ii++) {
						file2 = &(project.files[ii]);
						if (file2->block == file->block) {
							etime_d = file2->sections[file2->num_sections - 1].etime_d;
							num_files++;
						}
					}

					/* make survey list item */
					if (file->status == MBNA_FILE_POORNAV)
						filestatus = filestatus_poor;
					else if (file->status == MBNA_FILE_GOODNAV)
						filestatus = filestatus_good;
					else if (file->status == MBNA_FILE_FIXEDNAV)
						filestatus = filestatus_fixed;
					else if (file->status == MBNA_FILE_FIXEDXYNAV)
						filestatus = filestatus_fixedxy;
					else if (file->status == MBNA_FILE_FIXEDZNAV)
						filestatus = filestatus_fixedz;
					else
						filestatus = filestatus_unknown;
					mb_get_date(mbna_verbose, btime_d, btime_i);
					mb_get_date(mbna_verbose, etime_d, etime_i);
					sprintf(string,
					        "%2.2d %2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %s",
					        num_surveys, num_files, btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5],
					        btime_i[6], etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6],
					        filestatus);
					xstr[num_surveys] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);

					/* increment counter */
					num_surveys++;
				}
			}
			XmListAddItems(list_data, xstr, num_surveys, 0);
			for (i = 0; i < num_surveys; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (mbna_survey_select != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, mbna_survey_select + 1, 0);
			XmListSetPos(list_data, MAX(mbna_survey_select + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
		sprintf(string, "Survey-vs-Survey Blocks:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count the number of blocks */
			num_surveys = 0;
			num_blocks = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if (file->block == num_surveys) {
					num_surveys++;
					num_blocks += num_surveys;
				}
			}
			xstr = (XmString *)malloc(num_blocks * sizeof(XmString));

			/* generate list */
			num_blocks = 0;
			for (i = 0; i < num_surveys; i++) {
				for (j = 0; j <= i; j++) {
					n_tcrossing = 0;
					n_50crossing = 0;
					n_25crossing = 0;
					n_allcrossing = 0;
					n_tie = 0;
					for (k = 0; k < project.num_crossings; k++) {
						crossing = &project.crossings[k];
						if ((project.files[crossing->file_id_1].block == i && project.files[crossing->file_id_2].block == j) ||
						    (project.files[crossing->file_id_2].block == i && project.files[crossing->file_id_1].block == j)) {
							if (crossing->truecrossing == MB_YES)
								n_tcrossing++;
							if (crossing->overlap >= 50)
								n_50crossing++;
							if (crossing->overlap >= 25)
								n_25crossing++;
							n_allcrossing++;
							n_tie += crossing->num_ties;
						}
					}
					sprintf(string, "block %4.4d: Survey %2.2d vs Survey %2.2d : Crossings: %4d %4d %4d %4d : Ties: %4d",
					        num_blocks, j, i, n_tcrossing, n_50crossing, n_25crossing, n_allcrossing, n_tie);
					xstr[num_blocks] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					num_blocks++;
				}
			}
			XmListAddItems(list_data, xstr, num_blocks, 0);
			for (i = 0; i < num_surveys; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (mbna_block_select != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, mbna_block_select + 1, 0);
			XmListSetPos(list_data, MAX(mbna_block_select + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "Data Files:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "Data Files of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "Data Files of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "Data File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "Data Files of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "Data File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "Data File of Selected Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
		else
			sprintf(string, "Data Files:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count files */
			num_files = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_FILE) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION))
					num_files++;
			}
			xstr = (XmString *)malloc(num_files * sizeof(XmString));

			/* generate list */
			num_files = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_FILE) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)) {
					if (file->status == MBNA_FILE_POORNAV)
						filestatus = filestatus_poor;
					else if (file->status == MBNA_FILE_GOODNAV)
						filestatus = filestatus_good;
					else if (file->status == MBNA_FILE_FIXEDNAV)
						filestatus = filestatus_fixed;
					else if (file->status == MBNA_FILE_FIXEDXYNAV)
						filestatus = filestatus_fixedxy;
					else if (file->status == MBNA_FILE_FIXEDZNAV)
						filestatus = filestatus_fixedz;
					else
						filestatus = filestatus_unknown;
					sprintf(string, "%4.4d:%2.2d %s %4d %4.1f %4.1f %s", file->id, file->block, filestatus, file->num_sections,
					        file->heading_bias, file->roll_bias, file->file);
					xstr[num_files] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_file_select)
						iselect = num_files;
					num_files++;
				}
			}
			XmListAddItems(list_data, xstr, num_files, 0);
			for (i = 0; i < num_files; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "Data File Sections:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "Data File Sections of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "Data Files Sections of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "Data File Sections of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
		else
			sprintf(string, "Data Files Sections:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count sections */
			num_sections = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				for (j = 0; j < file->num_sections; j++) {
					section = &(file->sections[j]);
					if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i)) {
						num_sections++;
					}
				}
			}
			xstr = (XmString *)malloc(num_sections * sizeof(XmString));

			/* generate list */
			num_sections = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				for (j = 0; j < file->num_sections; j++) {
					section = &(file->sections[j]);
					if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i)) {
						mb_get_date(mbna_verbose, section->btime_d, btime_i);
						mb_get_date(mbna_verbose, section->etime_d, etime_i);
						sprintf(string,
						        "%2.2d:%4.4d:%2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d "
						        "%2.2d:%2.2d:%2.2d.%6.6d",
						        file->block, file->id, j, btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5],
						        btime_i[6], etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6]);
						xstr[num_sections] = XmStringCreateLocalized(string);
						if (mbna_verbose > 0)
							fprintf(stderr, "%s\n", string);
						if (i == mbna_file_select && j == mbna_section_select)
							iselect = num_sections;
						num_sections++;
					}
				}
			}
			XmListAddItems(list_data, xstr, num_sections, 0);
			for (i = 0; i < num_sections; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "Crossings:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "Crossings of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "Crossings with Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
		else
			sprintf(string, "Crossings:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossings */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES)
					num_crossings++;
			}
			xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
					        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
					        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, crossing->overlap,
					        crossing->num_ties);
					xstr[num_crossings] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_crossing_select)
						iselect = num_crossings;
					num_crossings++;
				}
			}
			XmListAddItems(list_data, xstr, num_crossings, 0);
			for (i = 0; i < num_crossings; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, ">10%% Overlap Crossings:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, ">10%% Overlap Crossings of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, ">10%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, ">10%% Overlap Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, ">10%% Overlap Crossings with Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, ">10%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, ">10%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
			        mbna_section_select);
		else
			sprintf(string, ">10%% Overlap Crossings:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossings */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES)
					num_crossings++;
			}
			xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
					        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
					        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, crossing->overlap,
					        crossing->num_ties);
					xstr[num_crossings] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_crossing_select)
						iselect = num_crossings;
					num_crossings++;
				}
			}
			XmListAddItems(list_data, xstr, num_crossings, 0);
			for (i = 0; i < num_crossings; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, ">25%% Overlap Crossings:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, ">25%% Overlap Crossings of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, ">25%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, ">25%% Overlap Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, ">25%% Overlap Crossings with Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, ">25%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, ">25%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
			        mbna_section_select);
		else
			sprintf(string, ">25%% Overlap Crossings:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossings */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES)
					num_crossings++;
			}
			xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
					        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
					        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, crossing->overlap,
					        crossing->num_ties);
					xstr[num_crossings] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_crossing_select)
						iselect = num_crossings;
					num_crossings++;
				}
			}
			XmListAddItems(list_data, xstr, num_crossings, 0);
			for (i = 0; i < num_crossings; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, ">50%% Crossings:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, ">50%% Crossings of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, ">50%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, ">50%% Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, ">50%% Overlap Crossings with Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, ">50%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, ">50%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
			        mbna_section_select);
		else
			sprintf(string, ">50%% Crossings:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossings */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES)
					num_crossings++;
			}
			xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
					        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
					        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, crossing->overlap,
					        crossing->num_ties);
					xstr[num_crossings] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_crossing_select)
						iselect = num_crossings;
					num_crossings++;
				}
			}
			XmListAddItems(list_data, xstr, num_crossings, 0);
			for (i = 0; i < num_crossings; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "True Crossings:");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "True Crossings of Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "True Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "True Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "True Crossings with Survey %d:", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "True Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "True Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
		else
			sprintf(string, "True Crossings:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossings */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES)
					num_crossings++;
			}
			xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			iselect = MBNA_SELECT_NONE;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
					        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
					        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, crossing->overlap,
					        crossing->num_ties);
					xstr[num_crossings] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr, "%s\n", string);
					if (i == mbna_crossing_select)
						iselect = num_crossings;
					num_crossings++;
				}
			}
			XmListAddItems(list_data, xstr, num_crossings, 0);
			for (i = 0; i < num_crossings; i++) {
				XmStringFree(xstr[i]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
		else
			sprintf(string, "Ties:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossing ties and global ties */
			num_ties = 0;

			/* count crossing ties */
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					num_ties += crossing->num_ties;
				}
			}

			/* count global ties */
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				for (j = 0; j < file->num_sections; j++) {
					section = &(file->sections[j]);
					if (section->global_tie_status != MBNA_TIE_NONE)
						num_ties++;
				}
			}

			/* allocate strings for list */
			xstr = (XmString *)malloc(num_ties * sizeof(XmString));

			/* generate list */
			num_crossings = 0;
			num_ties = 0;
			iselect = MBNA_SELECT_NONE;

			/* start with crossing ties */
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					for (j = 0; j < crossing->num_ties; j++) {
						tie = (struct mbna_tie *)&crossing->ties[j];
						if (tie->status == MBNA_TIE_XYZ)
							tiestatus = tiestatus_xyz;
						else if (tie->status == MBNA_TIE_XY)
							tiestatus = tiestatus_xy;
						else if (tie->status == MBNA_TIE_Z)
							tiestatus = tiestatus_z;
						if (tie->inversion_status == MBNA_INVERSION_CURRENT) {
							sprintf(string,
							        "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                    "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
							        i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
							        crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
							        crossing->file_id_2, crossing->section_2, tie->snav_2,
                                    tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                    tie->sigmar1, tie->sigmar2, tie->sigmar3,
                                    tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
                        }
						else if (tie->inversion_status == MBNA_INVERSION_OLD) {
							sprintf(string,
							        "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                    "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
							        i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
							        crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
							        crossing->file_id_2, crossing->section_2, tie->snav_2,
                                    tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                    tie->sigmar1, tie->sigmar2, tie->sigmar3,
                                    tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
                        }
						else {
							sprintf(string,
							        "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                    "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
							        i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
							        crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
							        crossing->file_id_2, crossing->section_2, tie->snav_2,
                                    tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                    tie->sigmar1, tie->sigmar2, tie->sigmar3);
                        }
						xstr[num_ties] = XmStringCreateLocalized(string);
						if (mbna_verbose > 0)
							fprintf(stderr, "%s\n", string);
						if (i == mbna_crossing_select && j == mbna_tie_select)
							iselect = num_ties;
						num_ties++;
					}
				}
			}

			/* now list global ties */
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				for (j = 0; j < file->num_sections; j++) {
					section = &(file->sections[j]);
					if (section->global_tie_status != MBNA_TIE_NONE &&
					    ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
					     (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
					     (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
					     (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
					     (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
					     (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i && mbna_section_select == j))) {
						if (section->global_tie_status == MBNA_TIE_XYZ)
							tiestatus = tiestatus_xyz;
						else if (section->global_tie_status == MBNA_TIE_XY)
							tiestatus = tiestatus_xy;
						else if (section->global_tie_status == MBNA_TIE_Z)
							tiestatus = tiestatus_z;
						if (section->global_tie_inversion_status == MBNA_INVERSION_CURRENT)
                            sprintf(string,
                                    "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
                                    project.files[i].block, i, j, section->global_tie_snav, tiestatus,
                                    section->offset_x_m, section->offset_y_m, section->offset_z_m,
                                    section->xsigma, section->ysigma, section->zsigma,
                                    section->dx_m, section->dy_m, section->dz_m, section->sigma_m, section->rsigma_m);
						else if (section->global_tie_inversion_status == MBNA_INVERSION_OLD)
                            sprintf(string,
                                    "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
                                    project.files[i].block, i, j, section->global_tie_snav, tiestatus,
                                    section->offset_x_m, section->offset_y_m, section->offset_z_m,
                                    section->xsigma, section->ysigma, section->zsigma,
                                    section->dx_m, section->dy_m, section->dz_m, section->sigma_m, section->rsigma_m);
						else
                            sprintf(string,
                                    "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
                                    project.files[i].block, i, j, section->global_tie_snav, tiestatus,
                                    section->offset_x_m, section->offset_y_m, section->offset_z_m,
                                    section->xsigma, section->ysigma, section->zsigma);
						xstr[num_ties] = XmStringCreateLocalized(string);
						if (mbna_verbose > 0)
							fprintf(stderr, "%s\n", string);
						num_ties++;
					}
				}
			}

			XmListAddItems(list_data, xstr, num_ties, 0);
			for (k = 0; k < num_ties; k++) {
				XmStringFree(xstr[k]);
			}
			free(xstr);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
		if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			sprintf(string, "Sorted Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
		else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			sprintf(string, "Sorted Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
			sprintf(string, "Sorted Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			sprintf(string, "Sorted Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			sprintf(string, "Sorted Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			sprintf(string, "Sorted Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			sprintf(string, "Sorted Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
		else
			sprintf(string, "Sorted Ties:");
		set_label_string(label_listdata, string);
		if (mbna_verbose > 0)
			fprintf(stderr, "%s\n", string);
		if (project.num_files > 0) {
			/* count crossing ties */
			num_ties = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					num_ties += crossing->num_ties;
				}
			}

			/* allocate strings for list */
			xstr = (XmString *)malloc(num_ties * sizeof(XmString));

            /* allocate array of tie pointers for list to be sorted */
            tie_list = (int *) malloc(num_ties * sizeof(int));

            /* get list of tie pointers */
            num_ties = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					crossing = &(project.crossings[i]);
					for (j = 0; j < crossing->num_ties; j++) {
						tie_list[num_ties] = 100 * i + j;
                        num_ties++;
                    }
                }
            }

            /* sort the ties from smallest to largest model misfit */
            qsort((void *)tie_list, (size_t) num_ties, sizeof(int), mbnavadjust_tie_compare);

			/* generate list */
			iselect = MBNA_SELECT_NONE;
            kk = 0;
            for (k  = num_ties - 1; k >= 0; k--) {
                i = tie_list[k] / 100;
				if (do_check_crossing_listok(i) == MB_YES) {
                    crossing = &(project.crossings[i]);
                    j = tie_list[k] % 100;
                    tie = (struct mbna_tie *)&crossing->ties[j];
                    if (tie->status == MBNA_TIE_XYZ)
                        tiestatus = tiestatus_xyz;
                    else if (tie->status == MBNA_TIE_XY)
                        tiestatus = tiestatus_xy;
                    else if (tie->status == MBNA_TIE_Z)
                        tiestatus = tiestatus_z;
                    if (tie->inversion_status == MBNA_INVERSION_CURRENT)
                        sprintf(string,
                                "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
                                i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
                                crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
                                crossing->file_id_2, crossing->section_2, tie->snav_2,
                                tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                tie->sigmar1, tie->sigmar2, tie->sigmar3,
                                tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
                    else if (tie->inversion_status == MBNA_INVERSION_OLD)
                        sprintf(string,
                                "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
                                i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
                                crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
                                crossing->file_id_2, crossing->section_2, tie->snav_2,
                                tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                tie->sigmar1, tie->sigmar2, tie->sigmar3,
                                tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
                    else
                        sprintf(string,
                                "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
                                "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
                                i, j, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
                                crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
                                crossing->file_id_2, crossing->section_2, tie->snav_2,
                                tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                                tie->sigmar1, tie->sigmar2, tie->sigmar3);
                    xstr[kk] = XmStringCreateLocalized(string);
                    if (mbna_verbose > 0)
                        fprintf(stderr, "%s\n", string);
                    if (i == mbna_crossing_select && j == mbna_tie_select)
                        iselect = kk;
                    kk++;
                }
            }

			XmListAddItems(list_data, xstr, num_ties, 0);
			for (kk = 0; kk < num_ties; kk++) {
				XmStringFree(xstr[kk]);
			}
			free(xstr);

            free(tie_list);
		}
		if (iselect != MBNA_SELECT_NONE) {
			XmListSelectPos(list_data, iselect + 1, 0);
			XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
		}
	}

	XtVaSetValues(toggleButton_showallsurveys, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showselectedsurvey, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showselectedblock, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showselectedfile, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showwithselectedsurvey, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showwithselectedfile, XmNsensitive, True, NULL);
	XtVaSetValues(toggleButton_showselectedsection, XmNsensitive, True, NULL);
	XmToggleButtonSetState(toggleButton_showallsurveys, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showselectedsurvey, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showselectedblock, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showselectedfile, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showwithselectedsurvey, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showwithselectedfile, FALSE, FALSE);
	XmToggleButtonSetState(toggleButton_showselectedsection, FALSE, FALSE);
	if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
		XmToggleButtonSetState(toggleButton_showallsurveys, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
		XmToggleButtonSetState(toggleButton_showselectedsurvey, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
		XmToggleButtonSetState(toggleButton_showselectedblock, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
		XmToggleButtonSetState(toggleButton_showselectedfile, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
		XmToggleButtonSetState(toggleButton_showwithselectedsurvey, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
		XmToggleButtonSetState(toggleButton_showwithselectedfile, TRUE, FALSE);
	else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
		XmToggleButtonSetState(toggleButton_showselectedsection, TRUE, FALSE);

	if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && project.num_files > 0 && mbna_survey_select != MBNA_SELECT_NONE) {
		XtVaSetValues(pushButton_poornav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_goodnav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixednav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixedxynav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixedznav, XmNsensitive, True, NULL);
		if (project.files[mbna_file_select].status == MBNA_FILE_POORNAV) {
			XtVaSetValues(pushButton_poornav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_GOODNAV) {
			XtVaSetValues(pushButton_goodnav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDNAV) {
			XtVaSetValues(pushButton_fixednav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDXYNAV) {
			XtVaSetValues(pushButton_fixedxynav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDZNAV) {
			XtVaSetValues(pushButton_fixedznav, XmNsensitive, False, NULL);
		}
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_FILES && project.num_files > 0 && mbna_file_select != MBNA_SELECT_NONE) {
		XtVaSetValues(pushButton_poornav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_goodnav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixednav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixedxynav, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_fixedznav, XmNsensitive, True, NULL);
		if (project.files[mbna_file_select].status == MBNA_FILE_POORNAV) {
			XtVaSetValues(pushButton_poornav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_GOODNAV) {
			XtVaSetValues(pushButton_goodnav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDNAV) {
			XtVaSetValues(pushButton_fixednav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDXYNAV) {
			XtVaSetValues(pushButton_fixedxynav, XmNsensitive, False, NULL);
		}
		else if (project.files[mbna_file_select].status == MBNA_FILE_FIXEDZNAV) {
			XtVaSetValues(pushButton_fixedznav, XmNsensitive, False, NULL);
		}
	}
	else {
		XtVaSetValues(pushButton_poornav, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_goodnav, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_fixednav, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_fixedxynav, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_fixedznav, XmNsensitive, False, NULL);
	}

	if ((mbna_view_list == MBNA_VIEW_LIST_TIES || mbna_view_list == MBNA_VIEW_LIST_TIESSORTED)
        && project.num_files > 0 && mbna_tie_select != MBNA_SELECT_NONE
        && project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XY) {
		XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_tie_xy, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
	}
	else if ((mbna_view_list == MBNA_VIEW_LIST_TIES || mbna_view_list == MBNA_VIEW_LIST_TIESSORTED)
             && project.num_files > 0 && mbna_tie_select != MBNA_SELECT_NONE
             && project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_Z) {
		XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_tie_z, XmNsensitive, False, NULL);
	}
	else if ((mbna_view_list == MBNA_VIEW_LIST_TIES || mbna_view_list == MBNA_VIEW_LIST_TIESSORTED)
             && project.num_files > 0 && mbna_tie_select != MBNA_SELECT_NONE
             && project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XYZ) {
		XtVaSetValues(pushButton_tie_xyz, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
	}
	else {
		XtVaSetValues(pushButton_tie_xyz, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_tie_xy, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_tie_z, XmNsensitive, False, NULL);
	}

	if (mbna_status != MBNA_STATUS_GUI) {
		XtVaSetValues(pushButton_new, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_open, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_close, XmNsensitive, False, NULL);
	}
	else if (project.open == MB_YES) {
		XtVaSetValues(pushButton_new, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_open, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_close, XmNsensitive, True, NULL);
	}
	else {
		XtVaSetValues(pushButton_new, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_open, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_close, XmNsensitive, False, NULL);
	}
	if (mbna_status == MBNA_STATUS_GUI && project.open == MB_YES && project.num_files >= 0) {
		XtVaSetValues(pushButton_importdata, XmNsensitive, True, NULL);
	}
	else {
		XtVaSetValues(pushButton_importdata, XmNsensitive, False, NULL);
	}
	if (project.open == MB_YES && project.num_files > 0) {
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_truecrossings == project.num_truecrossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
			XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showties, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_showtiessorted, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
	}
	else {
		XtVaSetValues(pushButton_showsurveys, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showblocks, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showdata, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showsections, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showcrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showties, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_showtiessorted, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showallsurveys, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showselectedsurvey, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showselectedblock, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showselectedfile, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showwithselectedsurvey, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showwithselectedfile, XmNsensitive, False, NULL);
		XtVaSetValues(toggleButton_showselectedsection, XmNsensitive, False, NULL);
	}

	if (mbna_status == MBNA_STATUS_GUI && project.open == MB_YES && project.num_files > 0) {
		XtVaSetValues(pushButton_autopick, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_autopickhorizontal, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_autosetsvsvertical, XmNsensitive, True, NULL);
		if (project.inversion_status == MBNA_INVERSION_CURRENT)
			XtVaSetValues(pushButton_newcrossings, XmNsensitive, True, NULL);
		else
			XtVaSetValues(pushButton_newcrossings, XmNsensitive, False, NULL);
		do_visualize_sensitivity();
		XtVaSetValues(pushButton_analyzecrossings, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_zerozoffsets, XmNsensitive, True, NULL);
		if (project.num_truecrossings == project.num_truecrossings_analyzed || project.num_crossings_analyzed >= 10)
			XtVaSetValues(pushButton_invertnav, XmNsensitive, True, NULL);
		else
			XtVaSetValues(pushButton_invertnav, XmNsensitive, False, NULL);
		if (project.grid_status == MBNA_GRID_CURRENT)
			XtVaSetValues(pushButton_updategrids, XmNsensitive, False, NULL);
		else
			XtVaSetValues(pushButton_updategrids, XmNsensitive, True, NULL);
		if (project.inversion_status != MBNA_INVERSION_NONE)
			XtVaSetValues(pushButton_showmodelplot, XmNsensitive, True, NULL);
		else
			XtVaSetValues(pushButton_showmodelplot, XmNsensitive, False, NULL);
		if (project.inversion_status == MBNA_INVERSION_CURRENT)
			XtVaSetValues(pushButton_applynav, XmNsensitive, True, NULL);
		else
			XtVaSetValues(pushButton_applynav, XmNsensitive, False, NULL);
	}
	else {
		XtVaSetValues(pushButton_autopick, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_autopickhorizontal, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_autosetsvsvertical, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_newcrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_visualize, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_analyzecrossings, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_zerozoffsets, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_invertnav, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_updategrids, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_applynav, XmNsensitive, False, NULL);
		if (project.inversion_status != MBNA_INVERSION_NONE)
			XtVaSetValues(pushButton_showmodelplot, XmNsensitive, True, NULL);
		else
			XtVaSetValues(pushButton_showmodelplot, XmNsensitive, False, NULL);
	}

	/* set values of decimation slider */
	XtVaSetValues(scale_controls_decimation, XmNvalue, project.decimation, NULL);

	/* set values of section length slider */
	ivalue = (int)(100 * project.section_length);
	imax = (int)(100 * 50.0);
	XtVaSetValues(scale_controls_sectionlength, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

	/* set values of section soundings slider */
	ivalue = project.section_soundings;
	XtVaSetValues(scale_controls_sectionsoundings, XmNvalue, ivalue, NULL);

	/* set values of contour interval slider */
	ivalue = (int)(100 * project.cont_int);
	if (project.cont_int >= 10.0)
		imax = (int)(100 * 400.0);
	else
		imax = (int)(100 * 50.0);
	XtVaSetValues(scale_controls_contourinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

	/* set values of color interval slider */
	ivalue = (int)(100 * project.col_int);
	if (project.col_int >= 10.0)
		imax = (int)(100 * 400.0);
	else
		imax = (int)(100 * 50.0);
	XtVaSetValues(scale_controls_colorinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

	/* set values of tick interval slider */
	ivalue = (int)(100 * project.tick_int);
	if (project.tick_int >= 10.0)
		imax = (int)(100 * 400.0);
	else
		imax = (int)(100 * 50.0);
	XtVaSetValues(scale_controls_tickinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

	/* set values of inversion smoothing weight slider */
	ivalue = (int)(100 * project.smoothing);
	imax = (int)(100 * 10.0);
	XtVaSetValues(scale_controls_smoothing, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

	/* set values of z offset width slider */
	ivalue = (int)(project.zoffsetwidth);
	XtVaSetValues(scale_controls_zoffset, XmNvalue, ivalue, NULL);

	/* set misfit offset center toggles */
	if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER) {
		XmToggleButtonSetState(toggleButton_misfitcenter_zero, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(toggleButton_misfitcenter_auto, TRUE, TRUE);
	}
}
/*--------------------------------------------------------------------*/

void do_update_modelplot_status() {
	struct mbna_crossing *crossing;

	/* deal with modelplot */
	if (project.modelplot == MB_YES) {
		/* set model view clear block button sensitivity */
		if (mbna_crossing_select == MBNA_SELECT_NONE) {
			XtVaSetValues(pushButton_modelplot_clearblock, XmNsensitive, False, NULL);
		}
		else {
			XtVaSetValues(pushButton_modelplot_clearblock, XmNsensitive, True, NULL);
		}

		/* set model view status label */
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
			if (mbna_crossing_select == MBNA_SELECT_NONE) {
				sprintf(string, ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
				                "extent\":t\"No Selection\"");
			}
			else {
				crossing = &(project.crossings[mbna_crossing_select]);
				sprintf(string,
				        ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
				        "extent\":t\"Selected Crossing: %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d\"",
				        mbna_crossing_select, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
				        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2);
			}
		}
		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
			if (mbna_crossing_select == MBNA_SELECT_NONE) {
				sprintf(string, ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
				                "extent\":t\"No Selection\"");
			}
			else {
				crossing = &(project.crossings[mbna_crossing_select]);
				sprintf(string,
				        ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
				        "extent\":t\"Selected Crossing: %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d\"",
				        mbna_crossing_select, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
				        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2);
			}
		}
		else {
			if (mbna_crossing_select == MBNA_SELECT_NONE) {
				sprintf(string, ":::t\"Mouse: <left> select  tie; <middle> select block to view; <right> drag zoom "
				                "extent\":t\"No Selection\"");
			}
			else {
				crossing = &(project.crossings[mbna_crossing_select]);
				sprintf(string,
				        ":::t\"Mouse: <left> select  tie; <middle> select block to view; <right> drag zoom extent\":t\"Selected "
				        "Crossing: %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d\"",
				        mbna_crossing_select, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
				        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2);
			}
		}

		set_label_multiline_string(label_modelplot_status, string);
	}
}
/*--------------------------------------------------------------------*/

void do_update_visualization_status() {
	int error = MB_ERROR_NO_ERROR;
	struct mbview_shareddata_struct *shareddata;
	struct mbna_file *file_1, *file_2;
	struct mbna_crossing *crossing;
	mb_path name = "";

	// fprintf(stderr,"do_update_visualization_status: mbna_crossing_select:%d mbna_tie_select:%d\n",
	// mbna_crossing_select,mbna_tie_select);
	/* deal with modelplot */
	if (project.visualization_status == MB_YES) {
		if (mbna_crossing_select != MBNA_SELECT_NONE) {
			/* clear any navadjust related interactive nav picks */
			mbview_getsharedptr(mbna_verbose, &shareddata, &error);
			shareddata->nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
			shareddata->nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;

			/* select the data sections associated with the loaded crossing */
			crossing = &(project.crossings[mbna_crossing_select]);
			mbview_clearnavpicks(0);

			sprintf(name, "%4.4d:%4.4d", crossing->file_id_1, crossing->section_1);
			mbview_picknavbyname(mbna_verbose, 0, name, &error);

			sprintf(name, "%4.4d:%4.4d", crossing->file_id_2, crossing->section_2);
			mbview_picknavbyname(mbna_verbose, 0, name, &error);

			/* select the route associated with the selected tie */
			if (mbna_tie_select != MBNA_SELECT_NONE) {
				file_1 = (struct mbna_file *)&project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *)&project.files[crossing->file_id_2];
				sprintf(name, "%4.4d:%1d %2.2d:%4.4d:%2.2d %2.2d:%4.4d:%2.2d", mbna_crossing_select, mbna_tie_select,
				        file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
				        crossing->section_2);
			}
			else {
				sprintf(name, "MBNA_SELECT_NONE");
			}
			mbview_pick_routebyname(mbna_verbose, 0, name, &error);
		}
		// fprintf(stderr,"Calling mbview_update from do_update_visualization_status\n");

		mbview_update(mbna_verbose, 0, &error);
	}
}
/*--------------------------------------------------------------------*/

void do_naverr_init() {
	/* manage naverr */
	XtManageChild(bulletinBoard_naverr);

	/* Setup just the "canvas" part of the screen. */
	cont_xid = XtWindow(drawingArea_naverr_cont);
	corr_xid = XtWindow(drawingArea_naverr_corr);
	zoff_xid = XtWindow(drawingArea_naverr_zcorr);

	/* Setup the "graphics Context" for just the "canvas" */
	xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.line_width = 2;
	cont_gc = XCreateGC(display, cont_xid, GCBackground | GCForeground | GCLineWidth, &xgcv);
	corr_gc = XCreateGC(display, corr_xid, GCBackground | GCForeground | GCLineWidth, &xgcv);

	/* Setup the font for just the "canvas" screen. */
	fontStruct = XLoadQueryFont(display, xgfont);
	if (fontStruct == NULL) {
		fprintf(stderr, "\nFailure to load font using XLoadQueryFont: %s\n", xgfont);
		fprintf(stderr, "\tSource file: %s\n\tSource line: %d", __FILE__, __LINE__);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(-1);
	}
	XSetFont(display, cont_gc, fontStruct->fid);
	XSetFont(display, corr_gc, fontStruct->fid);

	XSelectInput(display, cont_xid, EV_MASK);
	XSelectInput(display, corr_xid, EV_MASK);

	/* Setup cursors. */
	myCursor = XCreateFontCursor(display, XC_target);
	XRecolorCursor(display, myCursor, &colors[2], &colors[5]);
	XDefineCursor(display, cont_xid, myCursor);
	XDefineCursor(display, corr_xid, myCursor);

	/* initialize graphics */
	xg_init(display, cont_xid, cont_borders, xgfont, &cont_xgid);
	xg_init(display, corr_xid, corr_borders, xgfont, &corr_xgid);
	xg_init(display, zoff_xid, zoff_borders, xgfont, &zoff_xgid);
	status = mbnavadjust_set_graphics(cont_xgid, corr_xgid, zoff_xgid);

	/* set status flag */
	mbna_status = MBNA_STATUS_NAVERR;

	/* get current crossing */
	if (mbna_crossing_select == MBNA_SELECT_NONE)
		mbnavadjust_naverr_nextunset();
	else
		mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);

	/* update naverr labels */
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
}

/*--------------------------------------------------------------------*/

void do_update_naverr() {
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	double plot_width, misfit_width;
	double zoom_factor;
	double timediff;

	if (mbna_current_crossing >= 0) {
		/* get zoom factor */
		if ((mbna_plot_lon_max - mbna_plot_lon_min) > 0.0)
			zoom_factor = 100.0 * MAX((mbna_lon_max - mbna_lon_min) / (mbna_plot_lon_max - mbna_plot_lon_min),
			                          (mbna_lat_max - mbna_lat_min) / (mbna_plot_lat_max - mbna_plot_lat_min));
		else
			zoom_factor = 0.0;
		plot_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;
		misfit_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;

		/* get time difference */
		timediff = (project.files[mbna_file_id_2].sections[mbna_section_2].btime_d -
		            project.files[mbna_file_id_1].sections[mbna_section_1].btime_d) /
		           86400.0;

		/* set main naverr status label */
		crossing = &project.crossings[mbna_current_crossing];
		tie = &crossing->ties[mbna_current_tie];
		if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
			sprintf(string, ":::t\"Crossing: %d of %d\"\
:t\"Sections: %2.2d:%3.3d:%3.3d and %2.2d:%3.3d:%3.3d\"\
:t\"Time Difference: %f days \"\
:t\"Status: Unset \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   None None None\"", mbna_current_crossing, project.num_crossings,
			        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
			        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, timediff, plot_width,
			        misfit_width, project.zoffsetwidth, zoom_factor);
		}
		else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
			sprintf(string, ":::t\"Crossing: %d of %d\"\
:t\"Sections: %2.2d:%3.3d:%3.3d and %2.2d:%3.3d:%3.3d\"\
:t\"Time Difference: %f days \"\
:t\"Current Tie Point: %2d of %2d  Nav Points: %4d %4d\"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets (m):   %.3f %.3f %.3f\"\
:t\"Sigma (m):   %.3f %.3f %.3f\"", mbna_current_crossing, project.num_crossings,
			        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
			        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, timediff,
			        mbna_current_tie, crossing->num_ties, tie->snav_1, tie->snav_2, plot_width, misfit_width,
			        project.zoffsetwidth, zoom_factor, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1,
			        tie->sigmar2, tie->sigmar3);
		}
		else {
			sprintf(string, ":::t\"Crossing: %d of %d\"\
:t\"Sections: %2.2d:%3.3d:%3.3d and %2.2d:%3.3d:%3.3d\"\
:t\"Time Difference: %f days \"\
:t\"Status: Skipped \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   Skipped Skipped Skipped\"", mbna_current_crossing, project.num_crossings,
			        project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
			        project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, timediff, plot_width,
			        misfit_width, project.zoffsetwidth, zoom_factor);
		}
		set_label_multiline_string(label_naverr_status, string);

		/* set some button sensitivities */
		XtVaSetValues(pushButton_naverr_deletetie, XmNsensitive, (mbna_current_tie >= 0), NULL);
		XtVaSetValues(pushButton_naverr_selecttie, XmNsensitive, (crossing->num_ties > 0), NULL);
		XtVaSetValues(pushButton_naverr_fullsize, XmNsensitive,
		              (mbna_plot_lon_min != mbna_lon_min || mbna_plot_lon_max != mbna_lon_max ||
		               mbna_plot_lat_min != mbna_lat_min || mbna_plot_lat_max != mbna_lat_max),
		              NULL);
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			if (project.num_truecrossings == project.num_truecrossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
			XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
		}

		do_naverr_offsetlabel();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_offsetlabel() {
	/* look at current crossing */
	if (mbna_current_crossing >= 0) {
		/* set main naverr status label */
		sprintf(string, ":::t\"Working Offsets (m): %.3f %.3f %.3f %d:%d\":t\"Sigma (m): %.3f %.3f %.3f\"",
		        mbna_offset_x / mbna_mtodeglon, mbna_offset_y / mbna_mtodeglat, mbna_offset_z, mbna_snav_1, mbna_snav_2,
		        mbna_minmisfit_sr1, mbna_minmisfit_sr2, mbna_minmisfit_sr3);
	}

	else {
		/* set main naverr status label */
		sprintf(string, ":::t\"Working Offsets (m): %.3f %.3f %.3f\":t\"Working Tie Points: %d:%d\"", 0.0, 0.0, 0.0, 0, 0);
	}
	set_label_multiline_string(label_naverr_offsets, string);

	/* set button sensitivity for setting or resetting offsets */
	XtVaSetValues(pushButton_naverr_settie, XmNsensitive, mbna_allow_set_tie, NULL);
	XtVaSetValues(pushButton_naverr_resettie, XmNsensitive, mbna_allow_set_tie, NULL);
}

/*--------------------------------------------------------------------*/

void do_naverr_test_graphics() {
	int i, j, k;
	int ox, oy, dx, dy;
	double rx, ry, rr, r;

	/* now test graphics */
	ox = 0;
	oy = 0;
	dx = (cont_borders[1] - cont_borders[0]) / 16;
	dy = (cont_borders[3] - cont_borders[2]) / 16;
	rx = cont_borders[1] - ox;
	ry = cont_borders[3] - oy;
	rr = sqrt(rx * rx + ry * ry);
	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++) {
			k = 16 * j + i;
			ox = i * dx;
			oy = j * dy;
			xg_fillrectangle(cont_xgid, ox, oy, dx, dy, mpixel_values[k], 0);
			xg_fillrectangle(cont_xgid, ox + dx / 4, oy + dy / 4, dx / 2, dy / 2, k, 0);
		}
	ox = (corr_borders[1] - corr_borders[0]) / 2;
	oy = (corr_borders[3] - corr_borders[2]) / 2;
	rx = corr_borders[1] - ox;
	ry = corr_borders[3] - oy;
	rr = sqrt(rx * rx + ry * ry);
	for (i = corr_borders[0]; i < corr_borders[1]; i++)
		for (j = corr_borders[2]; j < corr_borders[3]; j++) {
			rx = i - ox;
			ry = j - oy;
			r = sqrt(rx * rx + ry * ry);
			k = 6 + (int)(80 * r / rr);
			xg_fillrectangle(corr_xgid, i, j, 1, 1, mpixel_values[k], 0);
		}
}
/*--------------------------------------------------------------------*/

void do_list_data_select(Widget w, XtPointer client_data, XtPointer call_data) {
	XmListCallbackStruct *acs;
	acs = (XmListCallbackStruct *)call_data;
    char selected_item[STRING_MAX];
    char *tmp = NULL;
	struct mbna_file *file;
	int *position_list = NULL;
	int position_count = 0;
	int num_files, num_sections, num_crossings, num_ties, num_surveys, num_blocks;
	int found;
    int nscan;
	int i, j;

	if (XmListGetSelectedPos(list_data, &position_list, &position_count)) {
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
			mbna_section_select = 0;
			mbna_file_select = MBNA_SELECT_NONE;
			mbna_survey_select = position_list[0] - 1;

			/* get selected file from list */
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if (mbna_file_select == MBNA_SELECT_NONE && mbna_survey_select == file->block) {
					mbna_file_select = i;
					mbna_section_select = 0;
				}
			}
			// fprintf(stderr,"mbna_survey_select:%d:%d:%d\n",mbna_survey_select,mbna_file_select,mbna_section_select);
            project.modelplot_uptodate = MB_NO;
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
			mbna_block_select = position_list[0] - 1;
			num_surveys = 0;
			num_blocks = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if (file->block == num_surveys) {
					for (j = 0; j <= num_surveys; j++) {
						if (mbna_block_select == num_blocks) {
							mbna_block_select1 = j;
							mbna_block_select2 = num_surveys;
						}
						num_blocks++;
					}
					num_surveys++;
				}
			}
			// fprintf(stderr,"mbna_block_select:%d:%d:%d\n",mbna_block_select,mbna_block_select1,mbna_block_select2);
            project.modelplot_uptodate = MB_NO;
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
			num_files = 0;

			/* get selected file from list */
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_FILE) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
				    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)) {
					if (num_files == position_list[0] - 1) {
						mbna_section_select = 0;
						mbna_file_select = i;
						mbna_survey_select = file->block;
					}
					num_files++;
				}
			}
            project.modelplot_uptodate = MB_NO;
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
			/* get selected section from list */
			num_sections = 0;
			for (i = 0; i < project.num_files; i++) {
				file = &(project.files[i]);
				for (j = 0; j < file->num_sections; j++) {
					if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
					    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i)) {
						if (num_sections == position_list[0] - 1) {
							mbna_section_select = j;
							mbna_file_select = i;
							mbna_survey_select = file->block;
						}
						num_sections++;
					}
				}
			}
            project.modelplot_uptodate = MB_NO;
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS || mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS ||
		         mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS || mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS ||
		         mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
			/* get selected crossing from list */
			num_crossings = 0;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					if (num_crossings == position_list[0] - 1) {
						mbna_crossing_select = i;
						mbna_tie_select = 0;
					}
					num_crossings++;
				}
			}
            project.modelplot_uptodate = MB_NO;

			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}

			/* else if naverr window is up, load selected crossing */
			else {
				mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				do_update_naverr();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
			/* get crossing and tie from list */
			num_crossings = 0;
			num_ties = 0;
			found = MB_NO;
			for (i = 0; i < project.num_crossings; i++) {
				if (do_check_crossing_listok(i) == MB_YES) {
					for (j = 0; j < project.crossings[i].num_ties; j++) {
						if (num_ties == position_list[0] - 1) {
							mbna_crossing_select = i;
							mbna_tie_select = j;
							found = MB_YES;
						}
						num_ties++;
					}
					num_crossings++;
				}
			}

			/* load selected crossing tie into naverr window, global ties ignored */
			if (found == MB_YES) {
				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO) {
					do_naverr_init();
				}

				/* else if naverr window is up, load selected crossing */
				else {
					mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
					mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
					do_update_naverr();
				}
                project.modelplot_uptodate = MB_NO;
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
			/* get crossing and tie from selected item in the list */
            if ((acs->item != NULL && XmStringGetLtoR(acs->item, XmFONTLIST_DEFAULT_TAG, &tmp))
                || (acs->selected_items != NULL && XmStringGetLtoR(acs->selected_items[0], XmFONTLIST_DEFAULT_TAG, &tmp))){
                strncpy(selected_item, tmp, STRING_MAX);
                XtFree(tmp);
                tmp = NULL;
                nscan = sscanf(selected_item, "%d %d ", &i, &j);
                if (nscan == 2 && i >= 0 && i < project.num_crossings
                    && do_check_crossing_listok(i) == MB_YES
                    && j >= 0 && j < project.crossings[i].num_ties) {
 					mbna_crossing_select = i;
					mbna_tie_select = j;
					found = MB_YES;
                }
            }

			/* load selected crossing tie into naverr window, global ties ignored */
			if (found == MB_YES) {
				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO) {
					do_naverr_init();
				}

				/* else if naverr window is up, load selected crossing */
				else {
					mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
					mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
					do_update_naverr();
				}
                project.modelplot_uptodate = MB_NO;
			}
		}

		free(position_list);
	}

	/* else user selected same list item, deselecting it  - don't change anything
	    - bring up naverr if needed - and let do_update_status redraw list with
	previous item selected */
	else {
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO) {
				do_naverr_init();
			}
		}
	}

	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

int do_check_crossing_listok(int icrossing) {
	int use_status = MB_NO;
	struct mbna_crossing *crossing;

	/* get crossing */
	crossing = &(project.crossings[icrossing]);

	/* check for list type */
	if (icrossing == mbna_crossing_select) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS && crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS && crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS && crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS && crossing->truecrossing == MB_YES) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIES && crossing->num_ties > 0) {
		use_status = MB_YES;
	}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTED && crossing->num_ties > 0) {
		use_status = MB_YES;
	}
	else {
		use_status = MB_NO;
	}

	/* check view mode modifiers */
	if (use_status == MB_YES) {
		// fprintf(stderr,"icrossing:%d mbna_view_mode:%d mbna_survey_select:%d mbna_block_select:%d:%d:%d mbna_file_select:%d
		// mbna_section_select:%d",
		// icrossing,mbna_view_mode,mbna_survey_select,mbna_block_select,mbna_block_select1,mbna_block_select2,
		// mbna_file_select,mbna_section_select);
		if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == project.files[crossing->file_id_1].block &&
		     mbna_survey_select == project.files[crossing->file_id_2].block) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_BLOCK && mbna_block_select1 == project.files[crossing->file_id_1].block &&
		     mbna_block_select2 == project.files[crossing->file_id_2].block) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
		     mbna_file_select == crossing->file_id_2) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && (mbna_survey_select == project.files[crossing->file_id_1].block ||
		                                                     mbna_survey_select == project.files[crossing->file_id_2].block)) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
		     (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
		     mbna_section_select == crossing->section_1) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
		     mbna_section_select == crossing->section_2)) {
			use_status = MB_YES;
		}
		else {
			use_status = MB_NO;
		}
	}

	return (use_status);
}

/*--------------------------------------------------------------------*/

int do_check_globaltie_listok(int ifile, int isection) {
	int use_status = MB_NO;
	struct mbna_file *file;
	struct mbna_section *section;

	/* get file and section */
	file = &(project.files[ifile]);
	section = &(file->sections[isection]);

	/* if there is a global time check for view mode */
	use_status = MB_NO;
	if (section->global_tie_status != MBNA_TIE_NONE) {
		if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
		     (mbna_block_select1 == file->block || mbna_block_select2 == file->block)) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection) ||
		    (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection)) {
			use_status = MB_YES;
		}
	}

	return (use_status);
}

/*--------------------------------------------------------------------*/

void do_naverr_cont_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void do_naverr_corr_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void do_naverr_cont_input(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	XEvent *event = acs->event;
	double x1, x2, y1, y2;

	/* If there is input in the drawing area */
	if (acs->reason == XmCR_INPUT) {
		/* Check for mouse pressed. */
		if (event->xany.type == ButtonPress) {
			/* If left mouse button is pushed then save position. */
			if (event->xbutton.button == 1) {
				button1down = MB_YES;
				loc_x = event->xbutton.x;
				loc_y = event->xbutton.y;
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
				mbna_offset_z_old = mbna_offset_z;

				/* reset offset label */
				mbnavadjust_naverr_checkoksettie();
				do_naverr_offsetlabel();

			} /* end of left button events */

			/* If middle mouse button is pushed */
			if (event->xbutton.button == 2) {
				button2down = MB_YES;
				mbna_zoom_x1 = event->xbutton.x;
				mbna_zoom_y1 = event->xbutton.y;
				mbna_zoom_x2 = event->xbutton.x;
				mbna_zoom_y2 = event->xbutton.y;

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOMFIRST);
			} /* end of middle button events */

			/* If right mouse button is pushed */
			if (event->xbutton.button == 3) {
				button3down = MB_YES;

				/* get new snav points */
				mbnavadjust_naverr_snavpoints(event->xbutton.x, event->xbutton.y);

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				do_update_naverr();
			} /* end of right button events */
		}     /* end of button press events */

		/* Check for mouse released. */
		if (event->xany.type == ButtonRelease) {
			if (event->xbutton.button == 1) {
				button1down = MB_NO;
			}
			if (event->xbutton.button == 2) {
				button2down = MB_NO;
				mbna_zoom_x2 = event->xbutton.x;
				mbna_zoom_y2 = event->xbutton.y;

				x1 = mbna_zoom_x1 / mbna_plotx_scale + mbna_plot_lon_min;
				y1 = (cont_borders[3] - mbna_zoom_y1) / mbna_ploty_scale + mbna_plot_lat_min;
				x2 = mbna_zoom_x2 / mbna_plotx_scale + mbna_plot_lon_min;
				y2 = (cont_borders[3] - mbna_zoom_y2) / mbna_ploty_scale + mbna_plot_lat_min;

				/* get new plot bounds */
				mbna_plot_lon_min = MIN(x1, x2);
				mbna_plot_lon_max = MAX(x1, x2);
				mbna_plot_lat_min = MIN(y1, y2);
				mbna_plot_lat_max = MAX(y1, y2);

				/* replot contours and misfit */
				mbnavadjust_get_misfit();
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				do_update_naverr();
			}
			if (event->xbutton.button == 3) {
				button3down = MB_NO;
			}

		} /* end of button release events */

		/* Check for mouse motion while pressed. */
		if (event->xany.type == MotionNotify) {
			if (button1down == MB_YES) {
				/* move offset */
				mbna_offset_x = mbna_offset_x_old + (event->xmotion.x - loc_x) / mbna_plotx_scale;
				mbna_offset_y = mbna_offset_y_old - (event->xmotion.y - loc_y) / mbna_ploty_scale;

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
				do_naverr_offsetlabel();

				/* reset old position */
				loc_x = event->xmotion.x;
				loc_y = event->xmotion.y;
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
			}
			else if (button2down == MB_YES) {
				mbna_zoom_x2 = event->xmotion.x;
				mbna_zoom_y2 = event->xmotion.y;

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOM);
			}
		}
	} /* end of inputs from window */
}

/*--------------------------------------------------------------------*/

void do_naverr_corr_input(Widget w, XtPointer client_data, XtPointer call_data) {
	XEvent *event;
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	event = acs->event;

	/* If there is input in the drawing area */
	if (acs->reason == XmCR_INPUT) {
		/* Check for mouse pressed. */
		if (event->xany.type == ButtonPress) {
			/* If left mouse button is pushed then save position. */
			if (event->xbutton.button == 1) {
				button1down = MB_YES;
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
				mbna_offset_z_old = mbna_offset_z;
				mbna_offset_x =
				    mbna_misfit_offset_x + (event->xbutton.x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
				mbna_offset_y =
				    mbna_misfit_offset_y - (event->xbutton.y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
				do_update_naverr();
				do_naverr_offsetlabel();

			} /* end of left button events */
		}     /* end of button press events */

		/* Check for mouse released. */
		if (event->xany.type == ButtonRelease) {
			if (event->xbutton.button == 1) {
				button1down = MB_NO;
			}
		} /* end of button release events */

		/* Check for mouse motion while pressed. */
		if (event->xany.type == MotionNotify) {
			if (button1down == MB_YES) {
				/* move offset */
				mbna_offset_x =
				    mbna_misfit_offset_x + (event->xmotion.x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
				mbna_offset_y =
				    mbna_misfit_offset_y - (event->xmotion.y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
				do_update_naverr();
				do_naverr_offsetlabel();

				/* reset old position */
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
				mbna_offset_z_old = mbna_offset_z;
			}
		}
	} /* end of inputs from window */
}
/*--------------------------------------------------------------------*/

void do_naverr_zcorr_input(Widget w, XtPointer client_data, XtPointer call_data) {
	XEvent *event;
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	event = acs->event;

	/* If there is input in the drawing area */
	if (acs->reason == XmCR_INPUT) {
		/* Check for mouse pressed. */
		if (event->xany.type == ButtonPress) {
			/* If left mouse button is pushed then save position. */
			if (event->xbutton.button == 1) {
				button1down = MB_YES;
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
				mbna_offset_z_old = mbna_offset_z;
				mbna_offset_z = ((event->xbutton.x - zoff_borders[0]) / mbna_zoff_scale_x) + mbna_misfit_offset_z -
				                0.5 * project.zoffsetwidth;
				/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

				/* recalculate minimum misfit at current z offset */
				mbnavadjust_get_misfitxy();

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
				do_update_naverr();
				do_naverr_offsetlabel();
			} /* end of left button events */
		}     /* end of button press events */

		/* Check for mouse released. */
		if (event->xany.type == ButtonRelease) {
			if (event->xbutton.button == 1) {
				button1down = MB_NO;
				mbnavadjust_crossing_replot();
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				do_update_naverr();
				do_naverr_offsetlabel();
			}
		} /* end of button release events */

		/* Check for mouse motion while pressed. */
		if (event->xany.type == MotionNotify) {
			if (button1down == MB_YES) {
				/* move offset */
				mbna_offset_z = ((event->xbutton.x - zoff_borders[0]) / mbna_zoff_scale_x) + mbna_misfit_offset_z -
				                0.5 * project.zoffsetwidth;
				/* fprintf(stderr,"buttonx:%d %f  mbna_misfit_offset_z:%f project.zoffsetwidth:%f  mbna_offset_z:%f\n",
				event->xbutton.x,((event->xbutton.x - zoff_borders[0])/mbna_zoff_scale_x), mbna_misfit_offset_z,
				project.zoffsetwidth, mbna_offset_z);
				fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

				/* recalculate minimum misfit at current z offset */
				mbnavadjust_get_misfitxy();

				/* replot contours */
				mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
				do_update_naverr();
				do_naverr_offsetlabel();

				/* reset old position */
				mbna_offset_x_old = mbna_offset_x;
				mbna_offset_y_old = mbna_offset_y;
				mbna_offset_z_old = mbna_offset_z;
			}
		}
	} /* end of inputs from window */
}

/*--------------------------------------------------------------------*/

void do_naverr_previous(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_previous();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_next(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_next();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_nextunset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_nextunset();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_addtie(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_addtie();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_deletetie(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_deletetie();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_selecttie(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_selecttie();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_unset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_unset();
	mbnavadjust_naverr_next();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_setnone(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_skip();
	mbnavadjust_naverr_nextunset();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_setoffset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_save();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
        project.modelplot_uptodate = MB_NO;
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_naverr_resettie(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_naverr_resettie();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_dismiss_naverr(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* unload loaded crossing */
	if (mbna_naverr_load == MB_YES) {
		status = mbnavadjust_crossing_unload();
	}

	/* deallocate graphics */
	mbna_status = MBNA_STATUS_GUI;
	XFreeGC(display, cont_gc);
	XFreeGC(display, corr_gc);
	xg_free(cont_xgid);
	xg_free(corr_xgid);
	mbna_current_crossing = MBV_SELECT_NONE;
	mbna_current_tie = MBV_SELECT_NONE;
	mbna_crossing_select = MBV_SELECT_NONE;
	mbna_tie_select = MBV_SELECT_NONE;
	mbnavadjust_naverr_checkoksettie();
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/
void do_naverr_fullsize(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* reset the plot bounds */
	mbna_plot_lon_min = mbna_lon_min;
	mbna_plot_lon_max = mbna_lon_max;
	mbna_plot_lat_min = mbna_lat_min;
	mbna_plot_lat_max = mbna_lat_max;
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
}

/*--------------------------------------------------------------------*/

void do_naverr_zerooffset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* move offset */
	mbna_offset_x = 0.0;
	mbna_offset_y = 0.0;
	mbna_offset_z = 0.0;
	/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

	/* recalculate minimum misfit at current z offset */
	mbnavadjust_get_misfitxy();

	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void do_naverr_zerozoffset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* move offset */
	mbna_offset_z = 0.0;
	/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

	/* recalculate minimum misfit at current z offset */
	mbnavadjust_get_misfitxy();

	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void do_naverr_applyzoffset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* recalculate minimum misfit at current z offset */
	mbnavadjust_get_misfitxy();

	/* replot contours */
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_minmisfit(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* move offset */
	mbna_offset_x = mbna_minmisfit_x;
	mbna_offset_y = mbna_minmisfit_y;
	mbna_offset_z = mbna_minmisfit_z;
	/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

	/* recalculate minimum misfit at current z offset */
	mbnavadjust_get_misfitxy();

	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_minxymisfit(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* move offset */
	mbna_offset_x = mbna_minmisfit_xh;
	mbna_offset_y = mbna_minmisfit_yh;
	mbna_offset_z = mbna_minmisfit_zh;
	/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_naverr_misfitcenter(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	if (XmToggleButtonGetState(toggleButton_misfitcenter_zero))
		mbna_misfit_center = MBNA_MISFIT_ZEROCENTER;
	else
		mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;

	/* replot contours and misfit */
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_biases_apply(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	struct mbna_file *file1;
	struct mbna_file *file2;
	int ivalue;
	int isection;

	/* get file structures */
	file1 = &(project.files[mbna_file_id_1]);
	file2 = &(project.files[mbna_file_id_2]);

	XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
	file1->heading_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
	file1->roll_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_heading2, XmNvalue, &ivalue, NULL);
	file2->heading_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_roll2, XmNvalue, &ivalue, NULL);
	file2->roll_bias = 0.1 * ivalue;

	/* set contours out of date */
	for (isection = 0; isection < file1->num_sections; isection++) {
		file1->sections[isection].contoursuptodate = MB_NO;
	}
	for (isection = 0; isection < file2->num_sections; isection++) {
		file2->sections[isection].contoursuptodate = MB_NO;
	}

	mbnavadjust_crossing_replot();
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_biases_applyall(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	double heading_bias;
	double roll_bias;
	struct mbna_file *file;
	int ivalue;
	int ifile, isection;

	/* get bias values */
	XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
	heading_bias = 0.1 * ivalue;
	XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
	roll_bias = 0.1 * ivalue;

	/* loop over files */
	for (ifile = 0; ifile < project.num_files; ifile++) {
		file = &(project.files[ifile]);
		file->heading_bias = heading_bias;
		file->roll_bias = roll_bias;

		/* set contours out of date */
		for (isection = 0; isection < file->num_sections; isection++) {
			file->sections[isection].contoursuptodate = MB_NO;
		}
	}

	mbnavadjust_crossing_replot();
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_biases_init(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	struct mbna_file *file1;
	struct mbna_file *file2;
	char value_text[128];

	/* get file structures */
	file1 = &(project.files[mbna_file_id_1]);
	file2 = &(project.files[mbna_file_id_2]);

	/* set biases label */
	sprintf(value_text, ":::t\"Section ID\'s (file:section):\":t\"  Section 1: %4.4d:%4.4d\"\"  Section 2: %4.4d:%4.4d\"",
	        mbna_file_id_1, mbna_section_1, mbna_file_id_2, mbna_section_2);
	set_label_multiline_string(label_biases_files, value_text);

	/* set biases radiobox */
	if (file1->heading_bias == file2->heading_bias && file1->roll_bias == file2->roll_bias) {
		mbna_bias_mode = MBNA_BIAS_SAME;
		XmToggleButtonSetState(toggleButton_biases_together, TRUE, TRUE);
	}
	else {
		mbna_bias_mode = MBNA_BIAS_DIFFERENT;
		XmToggleButtonSetState(toggleButton_biases_separate, TRUE, TRUE);
	}

	/* set values of bias sliders */
	XtVaSetValues(scale_biases_heading1, XmNvalue, (int)(10 * file1->heading_bias), NULL);
	XtVaSetValues(scale_biases_roll1, XmNvalue, (int)(10 * file1->roll_bias), NULL);
	if (mbna_bias_mode == MBNA_BIAS_DIFFERENT) {
		XtVaSetValues(scale_biases_heading2, XmNvalue, (int)(10 * file2->heading_bias), XmNsensitive, True, NULL);
		XtVaSetValues(scale_biases_roll2, XmNvalue, (int)(10 * file2->roll_bias), XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_biases_applyall, XmNsensitive, False, NULL);
	}
	else {
		XtVaSetValues(scale_biases_heading2, XmNvalue, (int)(10 * file2->heading_bias), XmNsensitive, False, NULL);
		XtVaSetValues(scale_biases_roll2, XmNvalue, (int)(10 * file2->roll_bias), XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_biases_applyall, XmNsensitive, True, NULL);
	}
}

/*--------------------------------------------------------------------*/

void do_biases_toggle(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;

	if (XmToggleButtonGetState(toggleButton_biases_together)) {
		if (mbna_bias_mode == MBNA_BIAS_DIFFERENT) {
			mbna_bias_mode = MBNA_BIAS_SAME;
			XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
			XtVaSetValues(scale_biases_heading2, XmNvalue, ivalue, XmNsensitive, False, NULL);
			XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
			XtVaSetValues(scale_biases_roll2, XmNvalue, ivalue, XmNsensitive, False, NULL);
			XtVaSetValues(pushButton_biases_applyall, XmNsensitive, True, NULL);
		}
	}
	else {
		if (mbna_bias_mode == MBNA_BIAS_SAME) {
			mbna_bias_mode = MBNA_BIAS_DIFFERENT;
			XtVaSetValues(scale_biases_heading2, XmNsensitive, True, NULL);
			XtVaSetValues(scale_biases_roll2, XmNsensitive, True, NULL);
			XtVaSetValues(pushButton_biases_applyall, XmNsensitive, False, NULL);
		}
	}
}

/*--------------------------------------------------------------------*/

void do_biases_heading(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;

	if (mbna_bias_mode == MBNA_BIAS_SAME) {
		XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
		XtVaSetValues(scale_biases_heading2, XmNvalue, ivalue, XmNsensitive, False, NULL);
	}
}

/*--------------------------------------------------------------------*/

void do_biases_roll(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;

	if (mbna_bias_mode == MBNA_BIAS_SAME) {
		XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
		XtVaSetValues(scale_biases_roll2, XmNvalue, ivalue, XmNsensitive, False, NULL);
	}
}

/*--------------------------------------------------------------------*/

void do_controls_apply(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;
	int error = MB_ERROR_NO_ERROR;

	/* get value of decimation slider */
	XtVaGetValues(scale_controls_decimation, XmNvalue, &project.decimation, NULL);

	/* get values of section length slider */
	XtVaGetValues(scale_controls_sectionlength, XmNvalue, &ivalue, NULL);
	project.section_length = ((double)ivalue) / 100.0;

	/* get values of section soundings slider */
	XtVaGetValues(scale_controls_sectionsoundings, XmNvalue, &ivalue, NULL);
	project.section_soundings = ivalue;

	/* get values of contour interval slider */
	XtVaGetValues(scale_controls_contourinterval, XmNvalue, &ivalue, NULL);
	project.cont_int = ((double)ivalue) / 100.0;

	/* get values of color interval slider */
	XtVaGetValues(scale_controls_colorinterval, XmNvalue, &ivalue, NULL);
	project.col_int = ((double)ivalue) / 100.0;

	/* get values of tick interval slider */
	XtVaGetValues(scale_controls_tickinterval, XmNvalue, &ivalue, NULL);
	project.tick_int = ((double)ivalue) / 100.0;

	/* get values of inversion smoothing slider */
	XtVaGetValues(scale_controls_smoothing, XmNvalue, &ivalue, NULL);
	project.smoothing = ((double)ivalue) / 100.0;

	/* get values of z offset width slider */
	XtVaGetValues(scale_controls_zoffset, XmNvalue, &ivalue, NULL);
	project.zoffsetwidth = ((double)ivalue);

	if (mbna_file_id_1 >= 0 && mbna_section_1 >= 0)
		project.files[mbna_file_id_1].sections[mbna_section_1].contoursuptodate = MB_NO;
	if (mbna_file_id_2 >= 0 && mbna_section_2 >= 0)
		project.files[mbna_file_id_2].sections[mbna_section_2].contoursuptodate = MB_NO;

	mbnavadjust_crossing_replot();
	mbnavadjust_write_project(mbna_verbose, &project, &error);
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/
void do_scale_controls_sectionlength(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_scale_controls_sectionsoundings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;
	int imin, imax;

	/* get values of section soundings slider */
	XtVaGetValues(scale_controls_sectionsoundings, XmNvalue, &ivalue, XmNminimum, &imin, XmNmaximum, &imax, NULL);

	/* recalculate max value if needed */
	if (ivalue == imin) {
		imax = MAX(imax / 2, 2 * imin);
	}
	if (ivalue == imax) {
		imax = 2 * imax;
	}

	/* reset values of section soundings slider */
	XtVaSetValues(scale_controls_sectionsoundings, XmNmaximum, imax, XmNvalue, ivalue, NULL);
}

/*--------------------------------------------------------------------*/
void do_scale_controls_decimation(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_scale_contourinterval(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;
	int imax;

	/* get values of contour interval slider */
	XtVaGetValues(scale_controls_contourinterval, XmNvalue, &ivalue, XmNmaximum, &imax, NULL);

	/* reset values of contour interval slider to round numbers */
	if (ivalue > 2500)
		ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
		ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
		ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
		ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
		ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
		ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
		imax = 500;
	if (ivalue == imax && imax <= 500)
		imax = 40000;
	XtVaSetValues(scale_controls_contourinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);
}

/*--------------------------------------------------------------------*/
void do_scale_controls_tickinterval(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;
	int imax;

	/* get values of tick interval slider */
	XtVaGetValues(scale_controls_tickinterval, XmNvalue, &ivalue, XmNmaximum, &imax, NULL);

	/* reset values of tick interval slider to round numbers */
	if (ivalue > 2500)
		ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
		ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
		ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
		ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
		ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
		ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
		imax = 500;
	if (ivalue == imax && imax <= 500)
		imax = 40000;
	XtVaSetValues(scale_controls_tickinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);
}

/*--------------------------------------------------------------------*/
void do_controls_scale_colorinterval(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int ivalue;
	int imax;

	/* get values of color interval slider */
	XtVaGetValues(scale_controls_colorinterval, XmNvalue, &ivalue, XmNmaximum, &imax, NULL);

	/* reset values of color interval slider to round numbers */
	if (ivalue > 2500)
		ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
		ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
		ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
		ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
		ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
		ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
		imax = 500;
	if (ivalue == imax && imax <= 500)
		imax = 40000;
	XtVaSetValues(scale_controls_colorinterval, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);
}
/*--------------------------------------------------------------------*/

void do_scale_controls_smoothing(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
}
/*--------------------------------------------------------------------*/

void do_scale_controls_zoffset(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_file_new(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	fprintf(stderr, "do_file_new\n");
}

/*--------------------------------------------------------------------*/
void do_file_open(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	fprintf(stderr, "do_file_open\n");
}

/*--------------------------------------------------------------------*/
void do_file_importdata(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	fprintf(stderr, "do_file_importdata\n");
}

/*--------------------------------------------------------------------*/
void do_file_close(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	int error = MB_ERROR_NO_ERROR;

	mbnavadjust_close_project(mbna_verbose, &project, &error);
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/
void do_quit(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* unload loaded crossing */
	if (mbna_naverr_load == MB_YES) {
		status = mbnavadjust_crossing_unload();

		/* deallocate graphics */
		mbna_status = MBNA_STATUS_GUI;
		XFreeGC(display, cont_gc);
		XFreeGC(display, corr_gc);
		xg_free(cont_xgid);
		xg_free(corr_xgid);
		mbnavadjust_naverr_checkoksettie();
		do_update_naverr();
		do_update_status();
	}
}

/*--------------------------------------------------------------------*/
void do_fileselection_mode(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	file_mode = (size_t)client_data;

	/* desl with selection */
	if (file_mode == FILE_MODE_NEW) {
		tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.nvh", XmRXmString, 0, &argok);
	}
	else if (file_mode == FILE_MODE_OPEN) {
		tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.nvh", XmRXmString, 0, &argok);
	}
	else if (file_mode == FILE_MODE_IMPORT) {
		tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.mb-1", XmRXmString, 0, &argok);
	}
	else {
		tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.nvh", XmRXmString, 0, &argok);
	}

	ac = 0;
	XtSetArg(args[ac], XmNpattern, tmp0);
	ac++;
	XtSetValues(fileSelectionBox, args, ac);
	XmStringFree((XmString)tmp0);
}

/*--------------------------------------------------------------------*/
void do_fileselection_ok(Widget w, XtPointer client_data, XtPointer call_data) {
	char ifile[STRING_MAX];
	char format_text[40];
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* get input filename */
	get_text_string(fileSelectionBox_text, ifile);

	/* desl with selection */
	if (file_mode == FILE_MODE_NEW) {
		status = mbnavadjust_file_new(ifile);
		do_update_status();
	}
	else if (file_mode == FILE_MODE_OPEN) {
		status = mbnavadjust_file_open(ifile);
		do_update_status();
	}
	else if (file_mode == FILE_MODE_IMPORT) {
		get_text_string(textField_format, format_text);
		sscanf(format_text, "%d", &format);
		status = mbnavadjust_import_data(ifile, format);

		/* update datalist files and topography grids */
		mbna_status = MBNA_STATUS_NAVSOLVE;
		mbnavadjust_updategrid();
		mbna_status = MBNA_STATUS_GUI;
		do_update_status();
		if (project.modelplot == MB_YES) {
			do_update_modelplot_status();
			mbnavadjust_modelplot_plot(__FILE__, __LINE__);
		}
		if (project.visualization_status == MB_YES)
			do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_fileselection_cancel(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	file_mode = FILE_MODE_NONE;
}

/*--------------------------------------------------------------------*/

void do_view_showallsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_ALL) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_ALL;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showselectedsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_SURVEY) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_SURVEY;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showselectedblock(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_BLOCK) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_BLOCK;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showselectedfile(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_FILE) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_FILE;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showwithselectedsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_WITHSURVEY) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_WITHSURVEY;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showwithselectedfile(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_WITHFILE) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_WITHFILE;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showselectedsection(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_mode != MBNA_VIEW_MODE_WITHSECTION) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_mode = MBNA_VIEW_MODE_WITHSECTION;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_SURVEYS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_SURVEYS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_view_showblocks(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_BLOCKS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_BLOCKS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_view_showdata(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_FILES) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_FILES;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_view_showsections(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_FILESECTIONS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_FILESECTIONS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_view_showcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_CROSSINGS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_CROSSINGS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_view_showmediocrecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_MEDIOCRECROSSINGS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_view_showgoodcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_GOODCROSSINGS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_GOODCROSSINGS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_view_showbettercrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_BETTERCROSSINGS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_BETTERCROSSINGS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_view_showtruecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_TRUECROSSINGS) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_TRUECROSSINGS;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_view_showties(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_TIES) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_TIES;
	do_update_status();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
void do_view_showtiessorted(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

    if (mbna_view_list != MBNA_VIEW_LIST_TIESSORTED) {
        project.modelplot_uptodate = MB_NO;
    }
	mbna_view_list = MBNA_VIEW_LIST_TIESSORTED;
	do_update_status();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_action_poornav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_poornav_file();
	do_update_status();
}

/*--------------------------------------------------------------------*/

void do_action_goodnav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_goodnav_file();
	do_update_status();
}

/*--------------------------------------------------------------------*/

void do_action_fixednav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_fixednav_file();
	do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_fixedxynav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_fixedxynav_file();
	do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_fixedznav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_fixedznav_file();
	do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_xy(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_set_tie_xy();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_z(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_set_tie_z();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_xyz(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_set_tie_xyz();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_autopick(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbna_status = MBNA_STATUS_AUTOPICK;
	mbnavadjust_autopick(MB_YES);
	mbna_status = MBNA_STATUS_GUI;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_action_autopickhorizontal(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbna_status = MBNA_STATUS_AUTOPICK;
	mbnavadjust_autopick(MB_NO);
	mbna_status = MBNA_STATUS_GUI;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}
/*--------------------------------------------------------------------*/

void do_action_autosetsvsvertical(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbna_status = MBNA_STATUS_AUTOPICK;
	mbnavadjust_autosetsvsvertical();
	mbna_status = MBNA_STATUS_GUI;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_action_analyzecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_action_checknewcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_findcrossings();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_zerozoffsets(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_zerozoffsets();
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_action_invertnav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbna_status = MBNA_STATUS_NAVSOLVE;
	mbnavadjust_invertnav();
	mbna_status = MBNA_STATUS_GUI;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_action_updategrids(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbna_status = MBNA_STATUS_NAVSOLVE;
	mbnavadjust_updategrid();
	mbna_status = MBNA_STATUS_GUI;
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES) {
		mbnavadjust_reset_visualization_navties();
		do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/

void do_apply_nav(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	mbnavadjust_applynav();
	do_update_status();
}

/*--------------------------------------------------------------------*/

void do_modelplot_show(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	Window modp_xid;
	Dimension width, height;

	/* get drawingArea size */
	XtVaGetValues(drawingArea_modelplot, XmNwidth, &width, XmNheight, &height, NULL);
	mbna_modelplot_width = width;
	mbna_modelplot_height = height;
	modp_borders[0] = 0;
	modp_borders[1] = mbna_modelplot_width - 1;
	modp_borders[2] = 0;
	modp_borders[3] = mbna_modelplot_height - 1;

	/* Setup just the "canvas" part of the screen. */
	modp_xid = XtWindow(drawingArea_modelplot);

	/* Setup the "graphics Context" for just the "canvas" */
	xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.line_width = 2;
	modp_gc = XCreateGC(display, modp_xid, GCBackground | GCForeground | GCLineWidth, &xgcv);

	/* Setup the font for just the "canvas" screen. */
	fontStruct = XLoadQueryFont(display, xgfont);
	if (fontStruct == NULL) {
		fprintf(stderr, "\nFailure to load font using XLoadQueryFont: %s\n", xgfont);
		fprintf(stderr, "\tSource file: %s\n\tSource line: %d", __FILE__, __LINE__);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(-1);
	}
	XSetFont(display, modp_gc, fontStruct->fid);
	XSelectInput(display, modp_xid, EV_MASK);

	/* Setup cursors. */
	myCursor = XCreateFontCursor(display, XC_target);
	XRecolorCursor(display, myCursor, &colors[2], &colors[5]);
	XDefineCursor(display, modp_xid, myCursor);

	/* initialize graphics */
	xg_init(display, modp_xid, modp_borders, xgfont, &modp_xgid);
	status = mbnavadjust_set_modelplot_graphics(modp_xgid, modp_borders);

	/* set status flag */
	project.modelplot = MB_YES;
    project.modelplot_uptodate = MB_NO;
	mbna_modelplot_zoom = MB_NO;
	mbna_modelplot_zoom_x1 = 0;
	mbna_modelplot_zoom_x2 = 0;
	mbna_modelplot_start = 0;
	mbna_modelplot_end = 0;

	/* update status */
	do_update_status();
	if (project.modelplot == MB_YES) {
		do_update_modelplot_status();
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
	if (project.visualization_status == MB_YES)
		do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_modelplot_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* deallocate graphics */
	project.modelplot = MB_NO;
	XFreeGC(display, modp_gc);
	xg_free(modp_xgid);
}

/*--------------------------------------------------------------------*/

void do_modelplot_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused) {
	Window modp_xid;
	XConfigureEvent *cevent = (XConfigureEvent *)event;
	Dimension width, height;

	/* do this only if a resize event happens */
	if (cevent->type == ConfigureNotify) {
		/* get new shell size */
		XtVaGetValues(bulletinBoard_modelplot, XmNwidth, &width, XmNheight, &height, NULL);

		/* do this only if the shell was REALLY resized and not just moved */
		if (mbna_modelplot_width != width - MBNA_MODELPLOT_LEFT_WIDTH ||
		    mbna_modelplot_height != height - MBNA_MODELPLOT_LEFT_HEIGHT) {
			/* set drawing area size */
			mbna_modelplot_width = width - MBNA_MODELPLOT_LEFT_WIDTH;
			mbna_modelplot_height = height - MBNA_MODELPLOT_LEFT_HEIGHT;
			ac = 0;
			XtSetArg(args[ac], XmNwidth, (Dimension)mbna_modelplot_width);
			ac++;
			XtSetArg(args[ac], XmNheight, (Dimension)mbna_modelplot_height);
			ac++;
			XtSetValues(drawingArea_modelplot, args, ac);

			/* deallocate graphics */
			XFreeGC(display, modp_gc);
			xg_free(modp_xgid);

			/* get drawingArea size */
			modp_borders[0] = 0;
			modp_borders[1] = mbna_modelplot_width - 1;
			modp_borders[2] = 0;
			modp_borders[3] = mbna_modelplot_height - 1;

			/* Setup just the "canvas" part of the screen. */
			modp_xid = XtWindow(drawingArea_modelplot);

			/* Setup the "graphics Context" for just the "canvas" */
			xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
			xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
			xgcv.line_width = 2;
			modp_gc = XCreateGC(display, modp_xid, GCBackground | GCForeground | GCLineWidth, &xgcv);

			/* Setup the font for just the "canvas" screen. */
			fontStruct = XLoadQueryFont(display, xgfont);
			if (fontStruct == NULL) {
				fprintf(stderr, "\nFailure to load font using XLoadQueryFont: %s\n", xgfont);
				fprintf(stderr, "\tSource file: %s\n\tSource line: %d", __FILE__, __LINE__);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(-1);
			}
			XSetFont(display, modp_gc, fontStruct->fid);
			XSelectInput(display, modp_xid, EV_MASK);

			/* Setup cursors. */
			myCursor = XCreateFontCursor(display, XC_target);
			XRecolorCursor(display, myCursor, &colors[2], &colors[5]);
			XDefineCursor(display, modp_xid, myCursor);

			/* initialize graphics */
			xg_init(display, modp_xid, modp_borders, xgfont, &modp_xgid);
			status = mbnavadjust_set_modelplot_graphics(modp_xgid, modp_borders);

			/* set to replot the model after the final resize when the expose events start */
            project.modelplot_uptodate = MB_NO;
		}
	}
}

/*--------------------------------------------------------------------*/

void do_modelplot_fullsize(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* replot model */
	mbna_modelplot_zoom_x1 = 0;
	mbna_modelplot_zoom_x2 = 0;
	if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
		mbna_modelplot_zoom = MB_NO;
		mbna_modelplot_start = 0;
		mbna_modelplot_end = 0;
	}
	else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
		mbna_modelplot_zoom = MB_NO;
		mbna_modelplot_start = 0;
		mbna_modelplot_end = 0;
	}
	else {
		mbna_modelplot_tiezoom = MB_NO;
		mbna_modelplot_tiestartzoom = 0;
		mbna_modelplot_tieendzoom = 0;
		mbna_block_select = MBNA_SELECT_NONE;
		mbna_block_select1 = MBNA_SELECT_NONE;
		mbna_block_select2 = MBNA_SELECT_NONE;
	}

    project.modelplot_uptodate = MB_NO;
	mbnavadjust_modelplot_setzoom();
	mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	do_update_modelplot_status();
}

/*--------------------------------------------------------------------*/

void do_modelplot_input(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	XEvent *event = acs->event;

	/* If there is input in the drawing area */
	if (acs->reason == XmCR_INPUT) {
		/* Check for mouse pressed. */
		if (event->xany.type == ButtonPress) {
			/* If left mouse button is pushed */
			if (event->xbutton.button == 1) {
				button1down = MB_YES;

			} /* end of left button events */

			/* If middle mouse button is pushed */
			if (event->xbutton.button == 2) {
				button2down = MB_YES;
			} /* end of middle button events */

			/* If right mouse button is pushed */
			if (event->xbutton.button == 3) {
				button3down = MB_YES;
				mbna_modelplot_zoom_x1 = event->xbutton.x;
				mbna_modelplot_zoom_x2 = event->xbutton.x;

				/* replot model if zoom set */
                project.modelplot_uptodate = MB_NO;
				mbnavadjust_modelplot_plot(__FILE__, __LINE__);
			} /* end of right button events */
		}     /* end of button press events */

		/* Check for mouse released. */
		if (event->xany.type == ButtonRelease) {
			/* If left mouse button is released */
			if (event->xbutton.button == 1) {
				button1down = MB_NO;

				/* pick nearest tie point */
				mbnavadjust_modelplot_pick(event->xbutton.x, event->xbutton.y);

                /* if a pick happened update everything */
                if (project.modelplot_uptodate == MB_NO) {
                    /* update model status */
                    do_update_modelplot_status();

                    /* update status */
                    do_update_status();

                    /* update model status */
                    do_update_modelplot_status();

                    /* replot model */
                    mbnavadjust_modelplot_plot(__FILE__, __LINE__);

                    /* update visualization */
                    if (project.visualization_status == MB_YES)
                        do_update_visualization_status();
                }
			}

			/* If middle mouse button is released */
			if (event->xbutton.button == 2) {
				button2down = MB_NO;

				/* pick nearest tie point */
				mbnavadjust_modelplot_middlepick(event->xbutton.x, event->xbutton.y);

                /* if a pick happened update everything */
                if (project.modelplot_uptodate == MB_NO) {
                    /* update model status */
                    do_update_modelplot_status();

                    /* update status */
                    do_update_status();

                    /* update model status */
                    do_update_modelplot_status();

                    /* replot model */
                    mbnavadjust_modelplot_plot(__FILE__, __LINE__);

                    /* update visualization */
                    if (project.visualization_status == MB_YES)
                        do_update_visualization_status();
                }
            }

			/* If right mouse button is released */
			if (event->xbutton.button == 3) {
				button3down = MB_NO;
				mbna_modelplot_zoom_x2 = event->xbutton.x;

				/* update model status */
				do_update_modelplot_status();

				/* replot model if zoom set */
				mbnavadjust_modelplot_setzoom();
                project.modelplot_uptodate = MB_NO;
				mbnavadjust_modelplot_plot(__FILE__, __LINE__);
				mbna_modelplot_zoom_x1 = 0;
				mbna_modelplot_zoom_x2 = 0;
			}

		} /* end of button release events */

		/* Check for mouse motion while pressed. */
		if (event->xany.type == MotionNotify) {
			/* If right mouse button is held during motion */
			if (button3down == MB_YES) {
				mbna_modelplot_zoom_x2 = event->xbutton.x;

				/* replot model */
                project.modelplot_uptodate = MB_NO;
				mbnavadjust_modelplot_plot(__FILE__, __LINE__);
			}
		}
	} /* end of inputs from window */
}

/*--------------------------------------------------------------------*/

void do_modelplot_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	/* update model status */
	do_update_modelplot_status();

	/* replot the model */
    //project.modelplot_uptodate = MB_NO;
	mbnavadjust_modelplot_plot(__FILE__, __LINE__);
}

/*--------------------------------------------------------------------*/

void do_modelplot_tieoffsets(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	// fprintf(stderr,"Called do_modelplot_tieoffsets %d\n",XmToggleButtonGetState(toggleButton_modelplot_tieoffsets));

	if (XmToggleButtonGetState(toggleButton_modelplot_tieoffsets)) {
		project.modelplot_style = MBNA_MODELPLOT_TIEOFFSETS;
        project.modelplot_uptodate = MB_NO;

		/* update model status */
		do_update_modelplot_status();

		/* replot the model */
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
}

/*--------------------------------------------------------------------*/

void do_modelplot_perturbation(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	// fprintf(stderr,"Called do_modelplot_perturbation %d\n",XmToggleButtonGetState(toggleButton_modelplot_perturbation));

	if (XmToggleButtonGetState(toggleButton_modelplot_perturbation)) {
		project.modelplot_style = MBNA_MODELPLOT_PERTURBATION;
        project.modelplot_uptodate = MB_NO;

		/* update model status */
		do_update_modelplot_status();

		/* replot the model */
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
}

/*--------------------------------------------------------------------*/

void do_modelplot_timeseries(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	// fprintf(stderr,"Called do_modelplot_timeseries %d\n",XmToggleButtonGetState(toggleButton_modelplot_timeseries));

	if (XmToggleButtonGetState(toggleButton_modelplot_timeseries)) {
		project.modelplot_style = MBNA_MODELPLOT_TIMESERIES;
        project.modelplot_uptodate = MB_NO;

		/* update model status */
		do_update_modelplot_status();

		/* replot the model */
		mbnavadjust_modelplot_plot(__FILE__, __LINE__);
	}
}

/*--------------------------------------------------------------------*/

void do_modelplot_clearblock(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;
	/* fprintf(stderr,"Called do_modelplot_clearblock\n"); */

	mbnavadjust_modelplot_clearblock();

	if (mbna_naverr_load == MB_YES) {
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		do_update_naverr();
	}

	/* update status */
	do_update_status();

	/* update model status */
	do_update_modelplot_status();

	/* replot the model */
	mbnavadjust_modelplot_plot(__FILE__, __LINE__);
}
/*--------------------------------------------------------------------*/

void do_visualize(Widget w, XtPointer client_data, XtPointer call_data) {
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call_data;

	mbnavadjust_open_visualization(MBNA_GRID_ADJUSTED);
}

/*--------------------------------------------------------------------*/

int do_visualize_dismiss_notify(size_t instance) {
	int status;

	status = mbnavadjust_dismiss_visualization();

	do_visualize_sensitivity();

	return (status);
}

/*--------------------------------------------------------------------*/

void do_visualize_sensitivity(void) {
	if (project.grid_status != MBNA_GRID_NONE && project.visualization_status == MB_NO)
		XtVaSetValues(pushButton_visualize, XmNsensitive, True, NULL);
	else
		XtVaSetValues(pushButton_visualize, XmNsensitive, False, NULL);
}

/*--------------------------------------------------------------------*/

void do_pickroute_notify(size_t instance) {
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	struct mbview_shareddata_struct *shareddata;
	struct mbview_route_struct *route;
	int icrossing, itie, ifile1, isection1, isnav1, ifile2, isection2, isnav2;

	/* print input debug statements */
	if (mbna_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:    %zu\n", instance);
	}

	if (mbna_verbose > 0)
		fprintf(stderr, "do_pickroute_notify:%zu\n", instance);

	/* get selected route and translate to selected crossing */
	status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);
	if (shareddata->route_selected != MBV_SELECT_NONE) {
		route = &shareddata->routes[shareddata->route_selected];
		sscanf(route->name, "%d:%d %d:%d:%d %d:%d:%d", &icrossing, &itie, &ifile1, &isection1, &isnav1, &ifile2, &isection2,
		       &isnav2);

		// fprintf(stderr,"in do_pickroute_notify A: icrossing:%d itie:%d  route selected:%d %d\n",
		// icrossing,itie,shareddata->route_selected,shareddata->route_point_selected);
		mbnavadjust_visualization_selectcrossingfromroute(icrossing, itie);

		// fprintf(stderr,"in do_pickroute_notify B: icrossing:%d itie:%d  route selected:%d %d\n",
		// icrossing,itie,shareddata->route_selected,shareddata->route_point_selected);

		/* bring up naverr window if required */
		if (mbna_naverr_load == MB_NO) {
			do_naverr_init();
		}
		else {
			mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
			do_update_naverr();
			do_update_status();
		}

		// fprintf(stderr,"in do_pickroute_notify C: icrossing:%d itie:%d  route selected:%d %d\n",
		// icrossing,itie,shareddata->route_selected,shareddata->route_point_selected);
		if (project.modelplot == MB_YES) {
			do_update_modelplot_status();
			mbnavadjust_modelplot_plot(__FILE__, __LINE__);
		}

		// fprintf(stderr,"in do_pickroute_notify D: icrossing:%d itie:%d  route selected:%d %d\n",
		// icrossing,itie,shareddata->route_selected,shareddata->route_point_selected);
		if (project.visualization_status == MB_YES)
			do_update_visualization_status();

		// fprintf(stderr,"in do_pickroute_notify E: icrossing:%d itie:%d  route selected:%d %d\n",
		// icrossing,itie,shareddata->route_selected,shareddata->route_point_selected);
	}

	if (mbna_verbose > 0)
		fprintf(stderr, "return do_pickroute_notify status:%d\n", status);
}
/*--------------------------------------------------------------------*/

void do_picknav_notify(size_t instance) {
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	struct mbview_shareddata_struct *shareddata;
	struct mbview_nav_struct *nav1, *nav2;
	int ifile1, isection1, ifile2, isection2;

	/* print input debug statements */
	if (mbna_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:    %zu\n", instance);
	}

	/* get shared data */
	status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);

	// fprintf(stderr,"\n*************\ndo_picknav_notify:%zu  selected: %d %d  navadjust: %d %d\n",
	// instance,shareddata->nav_selected[0],shareddata->nav_selected[1],
	// shareddata->nav_selected_mbnavadjust[0],shareddata->nav_selected_mbnavadjust[1]);

	/* if any navigation selected then unselect and unload any selected crossing */
	if (shareddata->nav_selected_mbnavadjust[0] != MBV_SELECT_NONE) {
		/* unload currently loaded crossing */
		// fprintf(stderr,"Need to unload current crossing: mbna_naverr_load:%d mbna_current_crossing:%d mbna_current_tie:%d\n",
		// mbna_naverr_load, mbna_current_crossing, mbna_current_tie);

		if (mbna_naverr_load == MB_YES) {
			/* unload crossing, remove naverr window */
			do_dismiss_naverr(NULL, NULL, NULL);
			BxUnmanageCB(pushButton_naverr_dismiss, (XtPointer) "bulletinBoard_biases", NULL);
			BxUnmanageCB(pushButton_naverr_dismiss, (XtPointer) "bulletinBoard_naverr", NULL);

			/* remove second nav pick that was associated with previous crossing */
			shareddata->nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
			shareddata->nav_selected[1] = MBV_SELECT_NONE;
			mbna_current_crossing = MBV_SELECT_NONE;
			mbna_current_tie = MBV_SELECT_NONE;
		}
		// fprintf(stderr,"Crossing should be unloaded: mbna_naverr_load:%d mbna_current_crossing:%d mbna_current_tie:%d\n",
		// mbna_naverr_load, mbna_current_crossing, mbna_current_tie);
	}
	// fprintf(stderr,"A do_picknav_notify: selected: %d %d  navadjust: %d %d\n",
	// shareddata->nav_selected[0],shareddata->nav_selected[1],
	// shareddata->nav_selected_mbnavadjust[0],shareddata->nav_selected_mbnavadjust[1]);

	/* get selected navigation and translate to selected crossing */
	if (shareddata->nav_selected_mbnavadjust[0] != MBV_SELECT_NONE &&
	    shareddata->nav_selected_mbnavadjust[1] != MBV_SELECT_NONE) {
		nav1 = (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[0]];
		nav2 = (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[1]];
		sscanf(nav1->name, "%d:%d", &ifile1, &isection1);
		sscanf(nav2->name, "%d:%d", &ifile2, &isection2);
		// fprintf(stderr,"do_picknav_notify: nav1->name:%s   nav2->name:%s\n",nav1->name,nav2->name);

		status = mbnavadjust_visualization_selectcrossingfromnav(ifile1, isection1, ifile2, isection2);

		/* bring up naverr window if required */
		if (mbna_current_crossing != MBV_SELECT_NONE && mbna_naverr_load == MB_NO) {
			do_naverr_init();
		}

		/* or replot the existing window */
		else if (mbna_current_crossing != MBV_SELECT_NONE) {
			mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
			do_update_naverr();
			do_update_status();
		}
		if (project.modelplot == MB_YES) {
			do_update_modelplot_status();
			mbnavadjust_modelplot_plot(__FILE__, __LINE__);
		}
		if (project.visualization_status == MB_YES)
			do_update_visualization_status();
	}

	if (mbna_verbose > 0)
		fprintf(stderr, "return do_picknav_notify status:%d\n", status);
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void do_mbnavadjust_addcrossing(Widget w, XtPointer client, XtPointer call) {
	int error;
	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call;
	struct mbview_shareddata_struct *shareddata;
	struct mbview_nav_struct *nav1, *nav2;
	int ifile1, isection1, ifile2, isection2;

	/* get shared data */
	status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);

	// fprintf(stderr,"\n*************\ndo_mbnavadjust_addcrossing: selected: %d %d  navadjust: %d %d\n",
	// shareddata->nav_selected[0],shareddata->nav_selected[1],
	// shareddata->nav_selected_mbnavadjust[0],shareddata->nav_selected_mbnavadjust[1]);
	// fprintf(stderr,"mbna_current_crossing:%d mbna_current_tie:%d mbna_crossing_select:%d mbna_tie_select:%d\n",
	// mbna_current_crossing, mbna_current_tie, mbna_crossing_select, mbna_tie_select);

	if (mbna_current_crossing == MBNA_SELECT_NONE && shareddata->nav_selected_mbnavadjust[0] != MBNA_SELECT_NONE &&
	    shareddata->nav_selected_mbnavadjust[1] != MBNA_SELECT_NONE &&
	    shareddata->nav_selected_mbnavadjust[0] != shareddata->nav_selected_mbnavadjust[1]) {
		/* add and select a new crossing */
		nav1 = (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[0]];
		nav2 = (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[1]];
		sscanf(nav1->name, "%d:%d", &ifile1, &isection1);
		sscanf(nav2->name, "%d:%d", &ifile2, &isection2);
		status = mbnavadjust_addcrossing(ifile1, isection1, ifile2, isection2);

		/* bring up naverr window */
		if (status == MB_SUCCESS) {
			do_naverr_init();
			mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
			do_update_naverr();
			do_update_status();
		}
		if (project.modelplot == MB_YES) {
            project.modelplot_uptodate = MB_NO;
			do_update_modelplot_status();
			mbnavadjust_modelplot_plot(__FILE__, __LINE__);
		}
		if (project.visualization_status == MB_YES)
			do_update_visualization_status();
	}
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void do_fileselection_list(Widget w, XtPointer client, XtPointer call) {
	int error;
	char fileroot[MB_PATH_MAXLINE];
	int form;
	char value_text[128];

	XmAnyCallbackStruct *acs;
	acs = (XmAnyCallbackStruct *)call;

	/* get selected text */
	get_text_string(fileSelectionBox_text, string);

	/* get output file */
	if ((int)strlen(string) > 0) {
		status = mb_get_format(mbna_verbose, string, fileroot, &form, &error);
		if (status == MB_SUCCESS) {
			format = form;
			sprintf(value_text, "%d", format);
			XmTextFieldSetString(textField_format, value_text);
		}
	}
}

/*--------------------------------------------------------------------*/

int do_wait_until_viewed(XtAppContext app) {
	Widget topshell;
	Window topwindow;
	XWindowAttributes xwa;
	XEvent event;

	/* set app_context */
	app_context = app;

	/* find the top level shell */
	for (topshell = scrolledWindow_datalist; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
		;

	/* keep processing events until it is viewed */
	if (XtIsRealized(topshell)) {
		topwindow = XtWindow(topshell);

		/* wait for the window to be mapped */
		while (XGetWindowAttributes(XtDisplay(form_mbnavadjust), topwindow, &xwa) && xwa.map_state != IsViewable) {
			XtAppNextEvent(app_context, &event);
			XtDispatchEvent(&event);
		}
	}

	XmUpdateDisplay(topshell);

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int do_message_on(char *message) {
	Widget diashell, topshell;
	Window diawindow, topwindow;
	XWindowAttributes xwa;
	XEvent event;

	if (mbna_verbose >= 1)
		fprintf(stderr, "%s\n", message);

	set_label_string(label_message, message);
	XtManageChild(bulletinBoard_message);

	/* force the label to be visible */
	for (diashell = label_message; !XtIsShell(diashell); diashell = XtParent(diashell))
		;
	for (topshell = diashell; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
		;
	if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
		diawindow = XtWindow(diashell);
		topwindow = XtWindow(topshell);

		/* wait for the dialog to be mapped */
		while (XGetWindowAttributes(XtDisplay(bulletinBoard_message), diawindow, &xwa) && xwa.map_state != IsViewable) {
			if (XGetWindowAttributes(XtDisplay(bulletinBoard_message), topwindow, &xwa) && xwa.map_state != IsViewable)
				break;

			XtAppNextEvent(app_context, &event);
			XtDispatchEvent(&event);
		}
	}

	XmUpdateDisplay(topshell);

	return (MB_SUCCESS);
}
/*--------------------------------------------------------------------*/

int do_message_update(char *message) {
	if (mbna_verbose >= 1)
		fprintf(stderr, "%s\n", message);

	set_label_string(label_message, message);
	XSync(XtDisplay(bulletinBoard_message), 0);
	XmUpdateDisplay(bulletinBoard_message);

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int do_message_off() {
	XtUnmanageChild(bulletinBoard_message);
	XSync(XtDisplay(bulletinBoard_message), 0);
	XmUpdateDisplay(bulletinBoard_message);

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int do_info_add(char *info, int timetag) {
	int pos;
	char tag[STRING_MAX];
	time_t right_now;
	char date[32], user[128], *user_ptr, host[128];

	/* reposition to end of text */
	pos = XmTextGetLastPosition(text_messages);
	XmTextSetInsertionPosition(text_messages, pos);

	/* add text */
	if (timetag == MB_YES)
		XmTextInsert(text_messages, pos, info);
	if (project.logfp != NULL)
		fputs(info, project.logfp);
	if (mbna_verbose > 0)
		fputs(info, stderr);

	/* put time tag in if requested */
	if (timetag == MB_YES) {
		right_now = time((time_t *)0);
		strcpy(date, ctime(&right_now));
		date[strlen(date) - 1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user, user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host, 128);
		sprintf(tag, " > User <%s> on cpu <%s> at <%s>\n", user, host, date);
		pos = XmTextGetLastPosition(text_messages);
		XmTextSetInsertionPosition(text_messages, pos);
		XmTextInsert(text_messages, pos, tag);
		if (project.logfp != NULL)
			fputs(tag, project.logfp);
		if (mbna_verbose > 0)
			fputs(tag, stderr);
	}

	/* reposition to end of text */
	if (timetag == MB_YES) {
		pos = XmTextGetLastPosition(text_messages);
		XmTextShowPosition(text_messages, pos);
		XmTextSetInsertionPosition(text_messages, pos);
	}

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int do_error_dialog(char *s1, char *s2, char *s3) {
	set_label_string(label_error_one, s1);
	set_label_string(label_error_two, s2);
	set_label_string(label_error_three, s3);
	XtManageChild(bulletinBoard_error);
	XBell(XtDisplay(form_mbnavadjust), 100);

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
void do_bell(int length) { XBell(display, length); }

/*--------------------------------------------------------------------*/
/* Change label string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void set_label_string(Widget w, String str) {
	XmString xstr;

	xstr = XmStringCreateLocalized(str);
	if (xstr != NULL)
		XtVaSetValues(w, XmNlabelString, xstr, NULL);
	else
		XtWarning("Failed to update labelString");

	XmStringFree(xstr);
}
/*--------------------------------------------------------------------*/
/* Change multiline label string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void set_label_multiline_string(Widget w, String str) {
	XmString xstr;
	Boolean argok;

	xstr = (XtPointer)BX_CONVERT(w, str, XmRXmString, 0, &argok);
	if (xstr != NULL && argok)
		XtVaSetValues(w, XmNlabelString, xstr, NULL);
	else
		XtWarning("Failed to update labelString");

	XmStringFree(xstr);
}
/*--------------------------------------------------------------------*/
/* Get text item string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void get_text_string(Widget w, String str) {
	char *str_tmp;

	str_tmp = (char *)XmTextGetString(w);
	strcpy(str, str_tmp);
	XtFree(str_tmp);
}

/*--------------------------------------------------------------------*/
