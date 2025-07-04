/*--------------------------------------------------------------------
 *    The MB-system:	mbvelocity_callbacks.c	4/7/97
 *
 *    Copyright (c) 1993-2025 by
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
/*
 * MBVELOCITYTOOL is an interactive water velocity profile editor
 * used to examine multiple water velocity profiles and to create
 * new water velocity profiles which can be used for the processing
 * of swath sonar data.  In general, this tool is used to examine
 * water velocity profiles obtained from XBTs, CTDs, or databases,
 * and to construct new profiles consistent with these various
 * sources of information.
 *
 * Author:	D. W. Caress
 * Date:	April 8, 1993
 * Date:	April 7, 1997  GUI recast
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#include <Xm/Xm.h>
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

/* mbedit widget includes */
#include "mbvelocity_creation.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbvelocity.h"

#ifndef FIXED
#define FIXED "fixed"
#endif

Widget BxFindTopShell(Widget);
WidgetList BxWidgetIdsFromNames(Widget, char *, char *);

/*--------------------------------------------------------------------*/

/* id variables */
static char program_name[] = "MBvelocitytool";

/* additional widgets */
Widget fileSelectionList;
Widget fileSelectionText;

/* global defines and variables */
#define EV_MASK (ButtonPressMask | KeyPressMask | KeyReleaseMask | ExposureMask)
#define xgfont "-*-" FIXED "-bold-r-normal-*-13-*-75-75-c-70-iso8859-1"

XtAppContext app_context;
Display *display;
Window can_xid, root_return, child_return;
Colormap colormap;
GC gc;
XGCValues xgcv;
XFontStruct *fontStruct;

int status;

static char message_str[2048];
static mb_path input_file;
int selected = 0; /* indicates an input file is selected */

void *can_xgid; /* XG graphics id */
Cursor myCursor;
XColor closest[2];
XColor exact[2];

/* Set the colors used for this program here. */
#define NCOLORS 7
XColor colors[NCOLORS];
unsigned int mpixel_values[NCOLORS];
XColor db_color;

/* Global mbvelocitytool definitions */
bool expose_plot_ok = false;
int edit_gui;
int ndisplay_gui;
double maxdepth_gui;
double velrange_gui;
double velcenter_gui;
double resrange_gui;
int format_gui;
int anglemode_gui;
int nload;

/* file opening parameters */
#define MBVT_IO_NONE 0
#define MBVT_IO_OPEN_DISPLAY_SVP 1
#define MBVT_IO_OPEN_EDIT_SVP 2
#define MBVT_IO_SAVE_EDIT_SVP 3
#define MBVT_IO_OPEN_MB 4
int open_type = MBVT_IO_NONE;

/* Set these to the dimensions of your canvas drawing */
/* area, minus 1, located in mbvelocity.uil.              */
static int borders[4] = {0, 1019, 0, 550};

void do_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data);
void set_label_string(Widget, String);
void set_label_multiline_string(Widget, String);
void get_text_string(Widget, String);
void do_set_controls();

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
void BxManageCB(Widget w, XtPointer client, XtPointer call) {
	(void)call;  // Unused parameter
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
void BxUnmanageCB(Widget w, XtPointer client, XtPointer call) {
	(void)call;  // Unused parameter
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
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call) {
	(void)client;  // Unused parameter
	(void)call;  // Unused parameter
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
  char *saveptr;

	for (start = rscs; rscs && *rscs; rscs = strtok_r(NULL, "\n", &saveptr)) {
		if (first) {
			rscs = strtok_r(rscs, "\n", &saveptr);
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

void do_mbvelocity_init(int argc, char **argv) {
	int i;

	/* make sure expose plots are off */
	expose_plot_ok = False;

	/* get additional widgets */
	fileSelectionList = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_LIST);
	fileSelectionText = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_TEXT);
	XtAddCallback(fileSelectionList, XmNbrowseSelectionCallback, do_fileselection_list, NULL);

	XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_HELP_BUTTON));

	/* Setup the entire screen. */
	display = XtDisplay(drawingArea);
	colormap = DefaultColormap(display, XDefaultScreen(display));

	/* Setup just the "canvas" part of the screen. */
	can_xid = XtWindow(drawingArea);

	/* Setup the "graphics Context" for just the "canvas" */
	xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
	xgcv.line_width = 2;
	gc = XCreateGC(display, can_xid, GCBackground | GCForeground | GCLineWidth, &xgcv);

	/* Setup the font for just the "canvas" screen. */
	fontStruct = XLoadQueryFont(display, xgfont);
	if (fontStruct == NULL) {
		fprintf(stderr, "\nFailure to load font using XLoadQueryFont: %s\n", xgfont);
		fprintf(stderr, "\tSource file: %s\n\tSource line: %d", __FILE__, __LINE__);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(-1);
	}
	XSetFont(display, gc, fontStruct->fid);

	XSelectInput(display, can_xid, EV_MASK);

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
	status = XLookupColor(display, colormap, "lightgrey", &db_color, &colors[6]);
	if ((status = XAllocColor(display, colormap, &colors[6])) == 0)
		fprintf(stderr, "Failure to allocate color: lightgrey\n");
	for (i = 0; i < NCOLORS; i++) {
		mpixel_values[i] = colors[i].pixel;
	}

	/* Setup initial cursor. This will be changed when changing "MODE". */
	myCursor = XCreateFontCursor(display, XC_target);
	XAllocNamedColor(display, colormap, "red", &closest[0], &exact[0]);
	XAllocNamedColor(display, colormap, "coral", &closest[1], &exact[1]);
	XRecolorCursor(display, myCursor, &closest[0], &closest[1]);
	XDefineCursor(display, can_xid, myCursor);

	/* initialize graphics */
	xg_init(display, can_xid, borders, xgfont, &can_xgid);

	status = mbvt_set_graphics(can_xgid, borders, NCOLORS, mpixel_values);

	/* initialize some labels */
	strcpy(message_str, "No display SVPs loaded...");
	set_label_string(label_status_display, message_str);
	strcpy(message_str, "No editable SVP loaded...");
	set_label_string(label_status_edit, message_str);
	strcpy(message_str, "No swath sonar data loaded...");
	set_label_string(label_status_mb, message_str);

	/* initialize mbvelocitytool proper */
	status = mbvt_init(argc, argv);

	/* set the controls */
	do_set_controls();

	/* finally allow expose plots */
	expose_plot_ok = True;
}

/*--------------------------------------------------------------------*/

void do_set_controls() {
	/* get some values from mbvelocitytool */
	mbvt_get_values(&edit_gui, &ndisplay_gui, &maxdepth_gui, &velrange_gui, &velcenter_gui, &resrange_gui, &anglemode_gui,
	                &format_gui);

	/* set about version label */
	sprintf(message_str, ":::t\"MB-System Release %s\":t\"%s\"", MB_VERSION, MB_VERSION_DATE);
	set_label_multiline_string(label_about_version, message_str);

	if (ndisplay_gui < 1)
		strcpy(message_str, "No display SVPs loaded...");
	else if (ndisplay_gui == 1)
		sprintf(message_str, "Loaded %d display SVP", ndisplay_gui);
	else
		sprintf(message_str, "Loaded %d display SVPs", ndisplay_gui);
	set_label_string(label_status_display, message_str);

	/* set pushbuttons */
	if (edit_gui == 1) {
		XtVaSetValues(pushButton_save_svp, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_save_svpfile, XmNsensitive, True, NULL);
	}
	else {
		XtVaSetValues(pushButton_save_svp, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_save_svpfile, XmNsensitive, False, NULL);
	}
	if (nload > 0) {
		XtVaSetValues(pushButton_process, XmNsensitive, True, NULL);
		XtVaSetValues(pushButton_save_residuals, XmNsensitive, True, NULL);
	}
	else {
		XtVaSetValues(pushButton_process, XmNsensitive, False, NULL);
		XtVaSetValues(pushButton_save_residuals, XmNsensitive, False, NULL);
	}

	/* set values of maximum depth slider */
	XtVaSetValues(slider_maxdepth, XmNvalue, (int)maxdepth_gui, NULL);

	/* set values of velocity range slider */
	XtVaSetValues(slider_velrange, XmNvalue, (int)velrange_gui, NULL);

	/* set values of velocity center slider */
	XtVaSetValues(slider_velcenter, XmNvalue, (int)velcenter_gui, XmNminimum, (int)1300, XmNmaximum, (int)1700, NULL);

	/* set values of residual range slider */
	XtVaSetValues(slider_residual_range, XmNvalue, ((int)(10 * resrange_gui)), NULL);

	/* set values of angle mode radiobox */
	if (anglemode_gui == 0)
		XmToggleButtonSetState(toggleButton_mode_ok, TRUE, TRUE);
	else if (anglemode_gui == 1)
		XmToggleButtonSetState(toggleButton_mode_snell, TRUE, TRUE);
	else if (anglemode_gui == 2)
		XmToggleButtonSetState(toggleButton_mode_null, TRUE, TRUE);

	/* set value of format text item */
	sprintf(message_str, "%2.2d", format_gui);
	XmTextFieldSetString(textField_mbformat, message_str);
}

/*--------------------------------------------------------------------*/

void do_velrange(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	velrange_gui = (double)acs->value;

	mbvt_set_values(edit_gui, ndisplay_gui, maxdepth_gui, velrange_gui, velcenter_gui, resrange_gui, anglemode_gui);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_velcenter(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	velcenter_gui = (double)acs->value;

	mbvt_set_values(edit_gui, ndisplay_gui, maxdepth_gui, velrange_gui, velcenter_gui, resrange_gui, anglemode_gui);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_process_mb(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	fprintf(stderr, "\nAbout to process data\n");

	/* turn off expose plots */
	expose_plot_ok = False;

	/* process Swath Sonar data */
	status = mbvt_process_multibeam();
	if (status != 1)
		XBell(display, 100);

	/* turn on expose plots */
	expose_plot_ok = True;

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_maxdepth(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	maxdepth_gui = (double)acs->value;

	mbvt_set_values(edit_gui, ndisplay_gui, maxdepth_gui, velrange_gui, velcenter_gui, resrange_gui, anglemode_gui);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}
/*--------------------------------------------------------------------*/

void do_anglemode(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	if (XmToggleButtonGetState(toggleButton_mode_ok))
		anglemode_gui = 0;
	else if (XmToggleButtonGetState(toggleButton_mode_snell))
		anglemode_gui = 1;
	else if (XmToggleButtonGetState(toggleButton_mode_null))
		anglemode_gui = 2;

	mbvt_set_values(edit_gui, ndisplay_gui, maxdepth_gui, velrange_gui, velcenter_gui, resrange_gui, anglemode_gui);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_quit(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	mbvt_quit();

	fprintf(stderr, "\nExiting mbvelocity!\n");

	exit(0);
}

/*--------------------------------------------------------------------*/

void do_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	static mb_path selection_text;
	int form;

	/* get selected text */
	get_text_string(fileSelectionText, selection_text);

	/* get output file */
	if ((int)strlen(selection_text) > 0) {
		/* look for MB suffix convention */
		form = format_gui;
		if ((status = mbvt_get_format(selection_text, &form)) == MB_SUCCESS) {
			format_gui = form;
			sprintf(message_str, "%d", format_gui);
			XmTextFieldSetString(textField_mbformat, message_str);
		}
	}
}

/*--------------------------------------------------------------------*/

void do_open(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XmFileSelectionBoxCallbackStruct *acs = (XmFileSelectionBoxCallbackStruct *)call_data;

	/* local definitions */
	int selected;
	int status = 0;
	char format_text[10];
	char *input_file_ptr;

	/* read the input file name */
	if (!XmStringGetLtoR(acs->value, XmSTRING_DEFAULT_CHARSET, &input_file_ptr)) {
		selected = 0;
	}
	else {
		selected = 1;
		strncpy(input_file, input_file_ptr, 128);
		XtFree(input_file_ptr);
	}

	if (selected > 0) {
		/* get selected filename id and file format id */
		if (open_type == MBVT_IO_OPEN_DISPLAY_SVP) {
			/* open file */
			status = mbvt_open_display_profile(input_file);

			/* reset status message */
			if (status == 1) {
				sprintf(message_str, "Loaded display SVP from: %s", input_file);
				set_label_string(label_status_display, message_str);
			}
		}
		else if (open_type == MBVT_IO_OPEN_EDIT_SVP) {
			/* open file */
			status = mbvt_open_edit_profile(input_file);

			/* reset status message */
			if (status == 1) {
				edit_gui = 1;
				sprintf(message_str, "Loaded editable SVP from: %s", input_file);
				set_label_string(label_status_edit, message_str);
			}
		}
		else if (open_type == MBVT_IO_SAVE_EDIT_SVP && edit_gui == 1) {
			/* save file */
			status = mbvt_save_edit_profile(input_file);

			/* reset status message */
			if (status == 1) {
				sprintf(message_str, "Saved editable SVP to: %s", input_file);
				set_label_string(label_status_edit, message_str);
			}
		}
		else if (open_type == MBVT_IO_OPEN_MB) {
			/* turn off expose plots */
			expose_plot_ok = False;

			/* get format id value */
			get_text_string(textField_mbformat, format_text);
			sscanf(format_text, "%d", &format_gui);

			/* open file */
			status = mbvt_open_swath_file(input_file, format_gui, &nload);

			/* reset status message */
			if (status == 1) {
				sprintf(message_str, "Read %d pings from swath file: %s", nload, input_file);
				set_label_string(label_status_mb, message_str);
			}
			if (status == 1 && edit_gui != 1) {
				sprintf(message_str, "Loaded default editable SVP");
				set_label_string(label_status_edit, message_str);
			}

			/* turn on expose plots */
			expose_plot_ok = True;
		}

		if (status != 1)
			XBell(display, 100);

		/* replot everything */
		do_set_controls();
		mbvt_plot();
	}
}
/*--------------------------------------------------------------------*/

void do_open_commandline(char *wfile, char *sfile, char *file, int format) {
	/* local definitions */
	int status;

	/* turn off expose plots */
	expose_plot_ok = False;

	/* get selected filename id and file format id */
	if (strlen(file) > 0) {
		/* turn off expose plots */
		expose_plot_ok = False;

		/* get format id value */
		strcpy(input_file, file);
		format_gui = format;

		/* open file */
		status = mbvt_open_swath_file(input_file, format_gui, &nload);

		/* reset status message */
		if (status == 1) {
			sprintf(message_str, "Read %d pings from swath file: %s", nload, input_file);
			set_label_string(label_status_mb, message_str);
		}
		if (status == 1 && edit_gui != 1) {
			sprintf(message_str, "Loaded default editable SVP");
			set_label_string(label_status_edit, message_str);
		}
	}
	if (strlen(wfile) > 0) {
		/* open file */
		edit_gui = 1;
		status = mbvt_open_edit_profile(wfile);

		/* reset status message */
		if (status == 1) {
			sprintf(message_str, "Loaded editable SVP from: %s", wfile);
			set_label_string(label_status_edit, message_str);
		}
	}
	if (strlen(sfile) > 0) {
		/* open file */
		status = mbvt_open_display_profile(sfile);

		/* reset status message */
		if (status == 1) {
			sprintf(message_str, "Loaded display SVP from: %s", wfile);
			set_label_string(label_status_display, message_str);
		}
	}

	/* turn on expose plots */
	expose_plot_ok = True;

	if (status != 1)
		XBell(display, 100);

	/* replot everything */
	do_set_controls();
	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_new_profile(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmListCallbackStruct *acs = (XmListCallbackStruct *)call_data;

	/* get new edit velocity profile */
	mbvt_new_edit_profile();

	sprintf(message_str, "Loaded default editable SVP");
	set_label_string(label_status_edit, message_str);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_residual_range(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	resrange_gui = ((double)acs->value / 10.0);

	mbvt_set_values(edit_gui, ndisplay_gui, maxdepth_gui, velrange_gui, velcenter_gui, resrange_gui, anglemode_gui);

	/* replot everything */
	do_set_controls();

	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_canvas_event(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	XEvent *event;
	XmDrawingAreaCallbackStruct *acs = (XmDrawingAreaCallbackStruct *)call_data;
	event = acs->event;

	static Position x_loc, y_loc;
	int root_x_return, root_y_return, win_x, win_y;
	unsigned int mask_return;
	bool doit;
	bool ring_bell;
	int status;

	/* If there is input in the drawing area */
	if (acs->reason == XmCR_INPUT) {
		/* Check for mouse pressed and not pressed and released. */
		if (event->xany.type == ButtonPress) {
			/* If left mouse button is pushed then move nearest svp node. */
			if (event->xbutton.button == 1) {
				x_loc = event->xbutton.x;
				y_loc = event->xbutton.y;

				status = mbvt_action_select_node(x_loc, y_loc);

				doit = true;
				ring_bell = false;
				while (doit) {
					status = mbvt_action_drag_node(x_loc, y_loc);
					if (status == 0 && !ring_bell) {
						ring_bell = true;
						XBell(display, 100);
					}

					status = XQueryPointer(display, can_xid, &root_return, &child_return, &root_x_return, &root_y_return, &win_x,
					                       &win_y, &mask_return);

					x_loc = win_x;
					y_loc = win_y;

					/* If the button is still pressed then read the location */
					/* of the pointer and run the action mouse function again */
					if (mask_return == 256)
						doit = True;
					else
						doit = False;
				}
				if (ring_bell)
					XBell(display, 100);

				/* replot graph */
				mbvt_plot();

			} /* end of left mouse pressed */

			/* If middle mouse button is pushed then add svp node. */
			else if (event->xbutton.button == 2) {
				x_loc = event->xbutton.x;
				y_loc = event->xbutton.y;

				status = mbvt_action_add_node(x_loc, y_loc);
				if (status != 1)
					XBell(display, 100);

				/* replot graph */
				mbvt_plot();

			} /* end of middle mouse pressed */

			/* If right mouse button is pushed then delete nearest svp node. */
			else if (event->xbutton.button == 3) {
				x_loc = event->xbutton.x;
				y_loc = event->xbutton.y;

				status = mbvt_action_delete_node(x_loc, y_loc);
				if (status != 1)
					XBell(display, 100);

				/* replot graph */
				mbvt_plot();

			}; /* end of right mouse pressed */

		}; /* end of button press event */

		/* button release event */
		if (event->xany.type == ButtonRelease) {
			if (event->xbutton.button == 1) {
				status = mbvt_action_mouse_up(x_loc, y_loc);

				if (status == 0)
					XBell(display, 100);

			}; /* end of button 1 button release */
		};     /* end of button release */
	};         /* end of input to canvas */
}

/*--------------------------------------------------------------------*/

void do_save_swath_svp(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	if (edit_gui == 1) {
		/* save file */
		status = mbvt_save_swath_profile(input_file);

		/* reset status message */
		if (status == 1) {
			strcpy(message_str, "Saved Editable Sound Velocity Profile: ");
			strcat(message_str, input_file);
			set_label_string(label_status_edit, message_str);
		}
	}

	if (status != 1)
		XBell(display, 100);

	/* replot everything */
	do_set_controls();
	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_save_residuals(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	if (edit_gui == 1 && nload > 0) {
		/* save file */
		status = mbvt_save_residuals(input_file);

		/* reset status message */
		if (status == 1) {
			strcpy(message_str, "Saved Residuals as Beam Offsets: ");
			strcat(message_str, input_file);
			set_label_string(label_status_edit, message_str);
		}
	}

	if (status != 1)
		XBell(display, 100);

	/* replot everything */
	do_set_controls();
	mbvt_plot();
}

/*--------------------------------------------------------------------*/

void do_io_mode_mb(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	open_type = MBVT_IO_OPEN_MB;
}

/*--------------------------------------------------------------------*/

void do_io_mode_open_svp_display(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	open_type = MBVT_IO_OPEN_DISPLAY_SVP;
}

/*--------------------------------------------------------------------*/

void do_io_mode_save_svp(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	open_type = MBVT_IO_SAVE_EDIT_SVP;
}

/*--------------------------------------------------------------------*/

void do_io_mode_open_svp_edit(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	open_type = MBVT_IO_OPEN_EDIT_SVP;
}

/*--------------------------------------------------------------------*/

void do_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

	if (expose_plot_ok)
		mbvt_plot();
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
	for (topshell = drawingArea; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
		;

	/* keep processing events until it is viewed */
	if (XtIsRealized(topshell)) {
		topwindow = XtWindow(topshell);

		/* wait for the window to be mapped */
		while (XGetWindowAttributes(XtDisplay(drawingArea), topwindow, &xwa) && xwa.map_state != IsViewable) {
			XtAppNextEvent(app_context, &event);
			XtDispatchEvent(&event);
		}
	}

	XmUpdateDisplay(topshell);

	return (1);
}

/*--------------------------------------------------------------------*/

int do_message_on(char *message) {
	Widget diashell, topshell;
	Window diawindow, topwindow;
	XWindowAttributes xwa;
	XEvent event;

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
		while (XGetWindowAttributes(display, diawindow, &xwa) && xwa.map_state != IsViewable) {
			if (XGetWindowAttributes(display, topwindow, &xwa) && xwa.map_state != IsViewable)
				break;

			XtAppNextEvent(app_context, &event);
			XtDispatchEvent(&event);
		}
	}

	XmUpdateDisplay(topshell);

	return (1);
}

/*--------------------------------------------------------------------*/

int do_message_off() {
	XtUnmanageChild(bulletinBoard_message);
	XSync(XtDisplay(bulletinBoard_message), 0);
	XmUpdateDisplay(bulletinBoard_message);

	return (1);
}

/*--------------------------------------------------------------------*/

int do_error_dialog(char *s1, char *s2, char *s3) {
	set_label_string(label_error_one, s1);
	set_label_string(label_error_two, s2);
	set_label_string(label_error_three, s3);
	XtManageChild(bulletinBoard_error);
	XBell(display, 100);

	return (1);
}

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
