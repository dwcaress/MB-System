/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_callbacks.c  2/22/2000
 *
 *    Copyright (c) 2000-2023 by
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
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the callback functions from the MOTIF interface.
 *
 * Author:  D. W. Caress
 * Date:  March 22, 2000
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#  ifndef WIN32
#    define WIN32
#  endif
#  include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Xm.h>

#define MBNAVADJUST_DECLARE_GLOBALS
#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbnavadjust.h"
#include "mbnavadjust_creation.h"
#include "mbnavadjust_extrawidgets.h"
#include "mbnavadjust_io.h"
#include "mbview.h"

/*--------------------------------------------------------------------*/

#ifndef FIXED
#define FIXED "fixed"
#endif

Widget BxFindTopShell(Widget start);
WidgetList BxWidgetIdsFromNames(Widget ref, char *cbName, char *stringList);

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
/* areas, minus 1, located in the uil file       */
static int cont_borders[4] = {0, 600, 0, 600};
static int corr_borders[4] = {0, 301, 0, 301};
static int zoff_borders[4] = {0, 300, 0, 60};
static int modp_borders[4];

/* file opening parameters */
#define FILE_MODE_NONE 0
#define FILE_MODE_NEW 1
#define FILE_MODE_OPEN 2
#define FILE_MODE_IMPORT 3
#define FILE_MODE_REFERENCE 4
size_t file_mode = FILE_MODE_NONE;
int format = 0;
int selected = 0; /* indicates an input file is selected */

/* button parameters */
static bool button1down = false;
static bool button2down = false;
static bool button3down = false;
static int loc_x;
static int loc_y;

int status;

Cardinal ac = 0;
Arg args[256];
Boolean argok;
XmString tmp0;

/*--------------------------------------------------------------------*/
/*      Function Name:   BxManageCB
 *
 *      Description:     Given a string of the form:
 *       "(WL)[widgetName, widgetName, ...]"
 *      BxManageCB attempts to convert the name to a Widget
 *      ID and manage the widget.
 *
 *      Arguments:       Widget      w:      the widget activating the callback.
 *       XtPointer   client: the list of widget names to attempt
 *        to find and manage.
 *       XtPointer   call:   the call data (unused).
 *
 *      Notes:  *  This function expects that there is an application
 *       shell from which all other widgets are descended.
 */

/* ARGSUSED */
void BxManageCB(Widget w, XtPointer client, XtPointer call) {
  (void)call; // Unused parameter

  /*
   * This function returns a NULL terminated WidgetList.  The memory for
   * the list needs to be freed when it is no longer needed.
   */
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxManageCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    XtManageChild(widgets[i]);
    i++;
  }
  XtFree((char *)widgets);
}

/*--------------------------------------------------------------------*/
/*      Function Name:  BxSetValuesCB
 *
 *      Description:     This function accepts a string of the form:
 *      "widgetName.resourceName = value\n..."
 *      It then attempts to convert the widget name to a widget
 *      ID and the value to a valid resource value.  It then
 *      sets the value on the given widget.
 *
 *      Arguments:      Widget    w:  the activating widget.
 *      XtPointer  client:  the set values string.
 *      XtPointer  call:  the call data (unused).
 *
 *      Notes:  * This function expects that there is an application
 *          shell from which all other widgets are descended.
 */

/* ARGSUSED */
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call) {
  (void)call; // Unused parameter

#define CHUNK 512

  bool first = true;
  String rscs = XtNewString((String)client);
  String *valueList = (String *)XtCalloc(CHUNK, sizeof(String));
  int count = 0;

  char *start = rscs;
  char *saveptr;
  for (; rscs && *rscs; rscs = strtok_r(NULL, "\n", &saveptr)) {
    if (first) {
      rscs = strtok_r(rscs, "\n", &saveptr);
      first = false;
    }
    valueList[count] = XtNewString(rscs);
    count++;
    if (count == CHUNK) {
      valueList = (String *)XtRealloc((char *)valueList, (count + CHUNK) * sizeof(String));
    }
  }
  XtFree((char *)start);

  for (int i = 0; i < count; i++) {
    /*
     * First, extract the widget name and generate a string to
     * pass to BxWidgetIdsFromNames().
     */
    char *cptr = strrchr(valueList[i], '.');
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
    String name = valueList[i];
    while ((name && *name) && isspace(*name)) {
      name++;
    }
    char *ptr = name + strlen(name) - 1;
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
    String rsc = ++cptr;
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
    Widget *current = BxWidgetIdsFromNames(w, "BxSetValuesCB", name);
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
/*      Function Name:   BxUnmanageCB
 *
 *      Description:     Given a string of the form:
 *       "(WL)[widgetName, widgetName, ...]"
 *      BxUnmanageCB attempts to convert the name to a Widget
 *      ID and unmanage the widget.
 *
 *      Arguments:       Widget      w:      the widget activating the callback.
 *       XtPointer   client: the list of widget names to attempt
 *        to find and unmanage.
 *       XtPointer   call:   the call data (unused).
 *
 *      Notes:  *  This function expects that there is an application
 *       shell from which all other widgets are descended.
 */

/* ARGSUSED */
void BxUnmanageCB(Widget w, XtPointer client, XtPointer call) {
  (void)call; // Unused parameter

  /*
   * This function returns a NULL terminated WidgetList.  The memory for
   * the list needs to be freed when it is no longer needed.
   */
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxUnmanageCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    XtUnmanageChild(widgets[i]);
    i++;
  }
  XtFree((char *)widgets);
}

/*--------------------------------------------------------------------*/
/*      Function Name:  BxExitCB
 *
 *      Description:     This functions expects an integer to be passed in
 *       client data.  It calls the exit() system call with
 *      the integer value as the argument to the function.
 *
 *      Arguments:      Widget    w:   the activating widget.
 *      XtPointer  client:  the integer exit value.
 *      XtPointer  call:  the call data (unused).
 */

/* ARGSUSED */
void BxExitCB(Widget w, XtPointer client, XtPointer call) {
  (void)w; // Unused parameter
  (void)client; // Unused parameter
  (void)call; // Unused parameter

  exit(EXIT_FAILURE);
}

/*--------------------------------------------------------------------*/

void do_mbnavadjust_init(int argc, char **argv) {
  const String translations = "<Btn1Down>:  DrawingAreaInput() ManagerGadgetArm() \n\
       <Btn1Up>:    DrawingAreaInput() ManagerGadgetActivate() \n\
       <Btn1Motion>:  DrawingAreaInput() ManagerGadgetButtonMotion() \n\
       <Btn2Down>:  DrawingAreaInput() ManagerGadgetArm() \n\
       <Btn2Up>:    DrawingAreaInput() ManagerGadgetActivate() \n\
       <Btn2Motion>:  DrawingAreaInput() ManagerGadgetButtonMotion() \n\
       <Btn3Down>:  DrawingAreaInput() ManagerGadgetArm() \n\
       <Btn3Up>:    DrawingAreaInput() ManagerGadgetActivate() \n\
       <Btn3Motion>:  DrawingAreaInput() ManagerGadgetButtonMotion() \n\
       <KeyDown>:    DrawingAreaInput() \n\
       <KeyUp>:    DrawingAreaInput() ManagerGadgetKeyInput()";

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
  int j = 7;
  for (int i = 0; i < 16; i++) {
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
  for (int i = 0; i < 16; i++) {
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
  for (int i = 0; i < 16; i++) {
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
  for (int i = 0; i < 16; i++) {
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
  for (int i = 0; i < 16; i++) {
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
  for (int i = 0; i < NCOLORS; i++) {
    mpixel_values[i] = colors[i].pixel;
  }
  status = mbnavadjust_set_colors(NCOLORS, (int *)mpixel_values);
  status = mbnavadjust_set_borders(cont_borders, corr_borders, zoff_borders);

  /* set verbose */
  mbna_verbose = 0;

  /* put up info text */
  mb_path string = "";
  snprintf(string, sizeof(string), "Program MBnavadjust initialized.\nMB-System Release %s %s\n", MB_VERSION, MB_VERSION_DATE);
  do_info_add(string, true);

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
  sprintf(value_text, ":::t\"MB-System Release %s\":t\"%s\"", MB_VERSION, MB_VERSION_DATE);
  set_label_multiline_string(label_about_version, value_text);

  /* set value of format text item */
  mb_path string = "";
  snprintf(string, sizeof(string), "%2.2d", format);
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

  /* set status label */
  mb_path refgrid_name;
  char *use_mode_primary = "Primary";
  char *use_mode_secondary = "Secondary";
  char *use_mode_tertiary = "Tertiary";
  char *use_mode = NULL;
  if (project.use_mode == MBNA_USE_MODE_PRIMARY)
    use_mode = use_mode_primary;
  else if (project.use_mode == MBNA_USE_MODE_SECONDARY)
    use_mode = use_mode_secondary; 
  else if (project.use_mode == MBNA_USE_MODE_TERTIARY)
    use_mode = use_mode_tertiary; 
  else {
    project.use_mode = MBNA_USE_MODE_PRIMARY;
    use_mode = use_mode_primary;
  }
  if (project.refgrid_select >= 0 && project.refgrid_select < project.num_refgrids)
    strcpy(refgrid_name, project.refgrid_names[project.refgrid_select]);
  else
    strcpy(refgrid_name, "NONE");
  mb_command string = "";
  snprintf(string, sizeof(string), 
    ":::t\"Project: %s\""
    ":t\"Navigation Adjustment Use Mode:       %s\""
    ":t\"Number of Files:                               %4d      Selected Survey:  %4d\""
    ":t\"Number of Crossings Found:             %4d     Selected File:    %4d\""
    ":t\"Number of Crossings Analyzed:        %4d     Selected Section: %4d\""
    ":t\"Number of True Crossings:               %4d     Selected Crossing:%4d\""
    ":t\"Number of True Crossings Analyzed: %4d     Selected Tie:     %4d\""
    ":t\"Number of Ties Set:                        %4d\""
    ":t\"Number of Global Ties Set:              %4d\""
    ":t\"Reference Grid: %s\"",
    project.name, use_mode, project.num_files, mbna_survey_select, project.num_crossings, mbna_file_select,
    project.num_crossings_analyzed, mbna_section_select, project.num_truecrossings, mbna_crossing_select,
    project.num_truecrossings_analyzed, mbna_tie_select, project.num_ties, project.num_globalties, refgrid_name);

  if (project.inversion_status == MBNA_INVERSION_CURRENT)
    strcat(string, ":t\"Inversion Performed:                     Current\"");
  else if (project.inversion_status == MBNA_INVERSION_OLD)
    strcat(string, ":t\"Inversion Performed:                     Out of Date\"");
  else
    strcat(string, ":t\"Inversion Performed:                         No\"");
  if (project.grid_status == MBNA_GRID_CURRENT)
    strcat(string, ":t\"Topography Grid Status:                      Current\"");
  else if (project.grid_status == MBNA_GRID_OLD)
    strcat(string, ":t\"Topography Grid Status:                    Out of Date\"");
  else
    strcat(string, ":t\"Topography Grid Status:                    Not made yet\"");
  set_label_multiline_string(label_status, string);
  if (mbna_verbose > 0) {
    snprintf(string, sizeof(string),
      "Project:                           %s\n"
      "Number of Files:                   %d\n"
      "Number of Crossings Found:         %d\n"
      "Number of Crossings Analyzed:      %d\n"
      "Number of True Crossings:          %d\n"
      "Number of True Crossings Analyzed: %d\n"
      "Number of Ties Set:                %d\n"
      "Number of Global Ties Set:         %d\n"
      "Reference Grid:                    %s\n",
      project.name, project.num_files, project.num_crossings, project.num_crossings_analyzed, project.num_truecrossings,
      project.num_truecrossings_analyzed, project.num_ties, project.num_globalties, refgrid_name);
    if (project.inversion_status == MBNA_INVERSION_CURRENT)
      strcat(string, "Inversion Performed:               Current\n");
    else if (project.inversion_status == MBNA_INVERSION_OLD)
      strcat(string, "Inversion Performed:               Out of Date\n");
    else
      strcat(string, "Inversion Performed:               No\n");
    if (project.grid_status == MBNA_GRID_CURRENT)
      strcat(string, "Topography Grid Status:            Current\n");
    else if (project.grid_status == MBNA_GRID_OLD)
      strcat(string, "Topography Grid Status:            Out of Date\n");
    else
      strcat(string, "Topography Grid Status:            Not made yet\n");
    fprintf(stderr, "%s", string);
  }

  char *tiestatus_xyz =   "XYZU";
  char *tiestatus_xy =    "XY_U";
  char *tiestatus_z =     "__ZU";
  char *tiestatus_xyz_f = "XYZF";
  char *tiestatus_xy_f =  "XY_F";
  char *tiestatus_z_f =   "__ZF";
  char *filestatus;
  char *filestatus_poor = " poor  ";
  char *filestatus_good = " good  ";
  char *filestatus_fixed = " fixed ";
  char *filestatus_fixedxy = "fixedxy";
  char *filestatus_fixedz = "fixedz ";
  char *filestatus_unknown = "unknown";

  /* set list_data */
  int iselect = MBNA_SELECT_NONE;
  XmListDeleteAllItems(list_data);
  if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
    snprintf(string, sizeof(string), "Reference Grids:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_refgrids > 0) {
      XmString *xstr = (XmString *)malloc(project.num_refgrids * sizeof(XmString));
      for (int i = 0; i < project.num_refgrids; i++) {
        xstr[i] = XmStringCreateLocalized(project.refgrid_names[i]);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", project.refgrid_names[i]);
      }
      XmListAddItems(list_data, xstr, project.num_refgrids, 0);
      for (int i = 0; i < project.num_refgrids; i++) {
        XmStringFree(xstr[i]);
      }
      free(xstr);
    }
    XmListSelectPos(list_data, project.refgrid_select + 1, 0);
    XmListSetPos(list_data, MAX(project.refgrid_select + 1 - 5, 1));
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
    snprintf(string, sizeof(string), "Surveys:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count the number of surveys */
      int num_surveys = 0;
      int num_files = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        if (file->block == num_surveys) {
          num_surveys++;
          num_files = 1;
        }
        else
          num_files++;
      }
      XmString *xstr = (XmString *)malloc(num_surveys * sizeof(XmString));

      /* generate list */
      num_surveys = 0;
      num_files = 0;
      int num_global_ties = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);

        double btime_d = 0.0;
        double etime_d = 0.0;
        if (i == 0) {
          btime_d = file->sections[0].btime_d;
        }
        if (file->block == num_surveys) {
          /* find end time for this block */
          num_files = 0;
          num_global_ties = 0;
          btime_d = file->sections[0].etime_d;
          for (int ii = i; ii < project.num_files; ii++) {
            struct mbna_file *file2 = &(project.files[ii]);
            if (file2->block == file->block) {
              etime_d = file2->sections[file2->num_sections - 1].etime_d;
              num_files++;
              for (int isection=0; isection < file2->num_sections; isection++) {
                struct mbna_section *section = &file2->sections[isection];
                if (section->globaltie.status != MBNA_TIE_NONE) {
                  num_global_ties++;
                }
              }
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
          int btime_i[7], etime_i[7];
          mb_get_date(mbna_verbose, btime_d, btime_i);
          mb_get_date(mbna_verbose, etime_d, etime_i);
          mb_path string = "";
          snprintf(string, sizeof(string), 
            "%3d %3d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %s g-ties:%d",
            num_surveys, num_files, btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5],
            btime_i[6], etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6],
            filestatus, num_global_ties);
          xstr[num_surveys] = XmStringCreateLocalized(string);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", string);

          /* increment counter */
          num_surveys++;
        }
      }
      XmListAddItems(list_data, xstr, num_surveys, 0);
      for (int i = 0; i < num_surveys; i++) {
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
    snprintf(string, sizeof(string), "Survey-vs-Survey Blocks:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* calculate the number of blocks */
      int num_blocks = project.num_surveys + (project.num_surveys * (project.num_surveys - 1) / 2);
      XmString *xstr = (XmString *)malloc(num_blocks * sizeof(XmString));
      int *survey1 = (int *)malloc(num_blocks * sizeof(int));
      int *survey2 = (int *)malloc(num_blocks * sizeof(int));
      int *n_tcrossing = (int *)malloc(num_blocks * sizeof(int));
      int *n_50crossing = (int *)malloc(num_blocks * sizeof(int));
      int *n_25crossing = (int *)malloc(num_blocks * sizeof(int));
      int *n_allcrossing = (int *)malloc(num_blocks * sizeof(int));
      int *n_tie = (int *)malloc(num_blocks * sizeof(int));
      memset(survey1, 0, (num_blocks * sizeof(int)));
      memset(survey2, 0, (num_blocks * sizeof(int)));
      memset(n_tcrossing, 0, (num_blocks * sizeof(int)));
      memset(n_50crossing, 0, (num_blocks * sizeof(int)));
      memset(n_25crossing, 0, (num_blocks * sizeof(int)));
      memset(n_allcrossing, 0, (num_blocks * sizeof(int)));
      memset(n_tie, 0, (num_blocks * sizeof(int)));
      int iblock = 0;
      for (int isurvey2=0; isurvey2 < project.num_surveys; isurvey2++) {
        for (int isurvey1 = 0; isurvey1 <= isurvey2; isurvey1++) {
          survey1[iblock] = isurvey1;
          survey2[iblock] = isurvey2;
          iblock++;
        }
      }
      /* generate list */
      for (int k = 0; k < project.num_crossings; k++) {
        struct mbna_crossing *crossing = &project.crossings[k];
        int iblock = project.files[crossing->file_id_1].block
               + (project.files[crossing->file_id_2].block * (project.files[crossing->file_id_2].block + 1) / 2);
        if (crossing->truecrossing)
          n_tcrossing[iblock]++;
        if (crossing->overlap >= 50)
          n_50crossing[iblock]++;
        if (crossing->overlap >= 25)
          n_25crossing[iblock]++;
        n_allcrossing[iblock]++;
        n_tie[iblock] += crossing->num_ties;
      }

      int nblocklist = 0;
      int iblocklist_select = MBNA_SELECT_NONE;
      for (int iblock=0; iblock < num_blocks; iblock++) {
        if (mbna_view_mode == MBNA_VIEW_MODE_ALL
            || (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
          && survey1[iblock] == mbna_survey_select
          && survey2[iblock] == mbna_survey_select)
            || (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
            || (mbna_view_mode == MBNA_VIEW_MODE_FILE
          && survey1[iblock] == project.files[mbna_file_select].block
          && survey2[iblock] == project.files[mbna_file_select].block)
            || (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
          && (survey1[iblock] == mbna_survey_select
              || survey2[iblock] == mbna_survey_select))
            || (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
          && (survey1[iblock] == project.files[mbna_file_select].block
              || survey2[iblock] == project.files[mbna_file_select].block))
            || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
          && (survey1[iblock] == project.files[mbna_file_select].block
              || survey2[iblock] == project.files[mbna_file_select].block))) {
          snprintf(string, sizeof(string), "block %4.4d: Survey %2.2d vs Survey %2.2d : Crossings: %4d %4d %4d %4d : Ties: %4d",
            iblock, survey1[iblock], survey2[iblock],
            n_tcrossing[iblock], n_50crossing[iblock], n_25crossing[iblock],
            n_allcrossing[iblock], n_tie[iblock]);
          xstr[nblocklist] = XmStringCreateLocalized(string);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", string);
          if (iblock == mbna_block_select)
            iblocklist_select = nblocklist;
          nblocklist++;
        }
      }

      XmListAddItems(list_data, xstr, nblocklist, 0);
      for (int iblocklist = 0; iblocklist < nblocklist; iblocklist++) {
        XmStringFree(xstr[iblocklist]);
      }
      free(xstr);
      free(survey1);
      free(survey2);
      free(n_tcrossing);
      free(n_50crossing);
      free(n_25crossing);
      free(n_allcrossing);
      free(n_tie);
      if (mbna_block_select != MBNA_SELECT_NONE && iblocklist_select != MBNA_SELECT_NONE) {
        XmListSelectPos(list_data, iblocklist_select + 1, 0);
        XmListSetPos(list_data, MAX(iblocklist_select + 1 - 5, 1));
      }
    }
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
    if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
      snprintf(string, sizeof(string), "Data Files:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Data Files of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Data Files of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Data File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Data Files of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Data File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Data File of Selected Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Data Files:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count files */
      int num_files = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
            (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
            (mbna_view_mode == MBNA_VIEW_MODE_FILE) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
            (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) || (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION))
          num_files++;
      }
      XmString *xstr = (XmString *)malloc(num_files * sizeof(XmString));

      /* generate list */
      num_files = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
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
          int num_global_ties = 0;
          for (int isection=0; isection < file->num_sections; isection++) {
            struct mbna_section *section = &file->sections[isection];
            if (section->globaltie.status != MBNA_TIE_NONE) {
              num_global_ties++;
            }
          }
          snprintf(string, sizeof(string), "%4.4d:%2.2d %s %4d %4.1f %4.1f g-ties:%d  %s", file->id, file->block, filestatus, file->num_sections,
            file->heading_bias, file->roll_bias, num_global_ties, file->file);
          xstr[num_files] = XmStringCreateLocalized(string);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", string);
          if (i == mbna_file_select)
            iselect = num_files;
          num_files++;
        }
      }
      XmListAddItems(list_data, xstr, num_files, 0);
      for (int i = 0; i < num_files; i++) {
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
      snprintf(string, sizeof(string), "Data File Sections:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Data File Sections of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Data Files Sections of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Data File Sections of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Data File Sections of File %d:%d:", mbna_survey_select, mbna_file_select);
    else
      snprintf(string, sizeof(string), "Data Files Sections:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count sections */
      int num_sections = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
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
      XmString *xstr = (XmString *)malloc(num_sections * sizeof(XmString));

      /* generate list */
      num_sections = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
              (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
              (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i)) {
            int btime_i[7], etime_i[7];
            mb_get_date(mbna_verbose, section->btime_d, btime_i);
            mb_get_date(mbna_verbose, section->etime_d, etime_i);
            char status_char;
            if (section->status == MBNA_CROSSING_STATUS_NONE)
              status_char = 'U';
            else if (section->status == MBNA_CROSSING_STATUS_SET)
              status_char = '*';
            else
              status_char = '-';
            if (section->status != MBNA_CROSSING_STATUS_SET)
              snprintf(string, sizeof(string), 
                "%c %2.2d:%4.4d:%2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d "
                "%2.2d:%2.2d:%2.2d.%6.6d",
                status_char, file->block, file->id, j, btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5],
                btime_i[6], etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6]);
            else {
              char *tiestatus = NULL;
              if (section->globaltie.status == MBNA_TIE_XYZ)
                tiestatus = tiestatus_xyz;
              else if (section->globaltie.status == MBNA_TIE_XY)
                tiestatus = tiestatus_xy;
              else if (section->globaltie.status == MBNA_TIE_Z)
                tiestatus = tiestatus_z;
              else if (section->globaltie.status == MBNA_TIE_XYZ_FIXED)
                tiestatus = tiestatus_xyz_f;
              else if (section->globaltie.status == MBNA_TIE_XY_FIXED)
                tiestatus = tiestatus_xy_f;
              else if (section->globaltie.status == MBNA_TIE_Z_FIXED)
                tiestatus = tiestatus_z_f;
              if (section->globaltie.inversion_status == MBNA_INVERSION_CURRENT)
                snprintf(string, sizeof(string), 
                  "%c %2.2d:%4.4d:%2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d"
                  " | %2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
                  status_char, file->block, file->id, j,
                  btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5], btime_i[6],
                  etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6],
                  section->globaltie.snav, tiestatus,
                  section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                  section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
                  section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
                  section->globaltie.sigma_m, section->globaltie.rsigma_m);
              else if (section->globaltie.inversion_status == MBNA_INVERSION_OLD)
                snprintf(string, sizeof(string), 
                  "%c %2.2d:%4.4d:%2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d"
                  " | %2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
                  status_char, file->block, file->id, j,
                  btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5], btime_i[6],
                  etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6],
                  section->globaltie.snav, tiestatus,
                  section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                  section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
                  section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
                  section->globaltie.sigma_m, section->globaltie.rsigma_m);
              else
                snprintf(string, sizeof(string), 
                  "%c %2.2d:%4.4d:%2.2d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d"
                  " | %2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
                  status_char, file->block, file->id, j,
                  btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4], btime_i[5], btime_i[6],
                  etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4], etime_i[5], etime_i[6],
                  section->globaltie.snav, tiestatus,
                  section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                  section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3);
            }
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
      for (int i = 0; i < num_sections; i++) {
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
      snprintf(string, sizeof(string), "Crossings:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Crossings of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Crossings with Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Crossings:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossings */
      int num_crossings = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i))
          num_crossings++;
      }
      XmString *xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

      /* generate list */
      num_crossings = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          char status_char = '-';
          char truecrossing = ' ';
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (!crossing->truecrossing)
            truecrossing = ' ';
          else
            truecrossing = 'X';
          snprintf(string, sizeof(string), "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
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
      for (int i = 0; i < num_crossings; i++) {
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
      snprintf(string, sizeof(string), ">10%% Overlap Crossings:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings with Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), ">10%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
        mbna_section_select);
    else
      snprintf(string, sizeof(string), ">10%% Overlap Crossings:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossings */
      int num_crossings = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i))
          num_crossings++;
      }
      XmString *xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

      /* generate list */
      num_crossings = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          char status_char = '-';
          char truecrossing = ' ';
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (!crossing->truecrossing)
            truecrossing = ' ';
          else
            truecrossing = 'X';
          snprintf(string, sizeof(string), "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
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
      for (int i = 0; i < num_crossings; i++) {
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
      snprintf(string, sizeof(string), ">25%% Overlap Crossings:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings with Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), ">25%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
        mbna_section_select);
    else
      snprintf(string, sizeof(string), ">25%% Overlap Crossings:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossings */
      int num_crossings = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i))
          num_crossings++;
      }
      XmString *xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

      /* generate list */
      num_crossings = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          char status_char = '-';
          char truecrossing = ' ';
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (!crossing->truecrossing)
            truecrossing = ' ';
          else
            truecrossing = 'X';
          snprintf(string, sizeof(string), "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
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
      for (int i = 0; i < num_crossings; i++) {
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
      snprintf(string, sizeof(string), ">50%% Crossings:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), ">50%% Crossings of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), ">50%% Overlap Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), ">50%% Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), ">50%% Overlap Crossings with Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), ">50%% Overlap Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), ">50%% Overlap Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select,
        mbna_section_select);
    else
      snprintf(string, sizeof(string), ">50%% Crossings:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossings */
      int num_crossings = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i))
          num_crossings++;
      }
      XmString *xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

      /* generate list */
      num_crossings = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          char status_char = '-';
          char truecrossing = ' ';
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (!crossing->truecrossing)
            truecrossing = ' ';
          else
            truecrossing = 'X';
          snprintf(string, sizeof(string), "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
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
      for (int i = 0; i < num_crossings; i++) {
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
      snprintf(string, sizeof(string), "True Crossings:");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "True Crossings of Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "True Crossings of Survey-vs-Survey Block %d:", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "True Crossings of File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "True Crossings with Survey %d:", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "True Crossings with File %d:%d:", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "True Crossings with Section %d:%d:%d:", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "True Crossings:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossings */
      int num_crossings = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i))
          num_crossings++;
      }
      XmString *xstr = (XmString *)malloc(num_crossings * sizeof(XmString));

      /* generate list */
      num_crossings = 0;
      iselect = MBNA_SELECT_NONE;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          char status_char = '-';
          char truecrossing = ' ';
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (crossing->truecrossing)
            truecrossing = ' ';
          else
            truecrossing = 'X';
          snprintf(string, sizeof(string), "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing, i,
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
      for (int i = 0; i < num_crossings; i++) {
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
      snprintf(string, sizeof(string), "Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Ties:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossing ties and global ties */
      int num_ties = 0;

      /* count crossing ties */
      project.tiessortedthreshold = 0.0;
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          num_ties += crossing->num_ties;
        }
      }

      /* allocate strings for list */
      XmString *xstr = (XmString *)malloc(num_ties * sizeof(XmString));

      /* generate list */
      num_ties = 0;
      iselect = MBNA_SELECT_NONE;

      /* list crossing ties */
      for (int i = 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          struct mbna_crossing *crossing = &(project.crossings[i]);
          for (int j = 0; j < crossing->num_ties; j++) {
            struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[j];
            char *tiestatus = NULL;
            if (tie->status == MBNA_TIE_XYZ)
              tiestatus = tiestatus_xyz;
            else if (tie->status == MBNA_TIE_XY)
              tiestatus = tiestatus_xy;
            else if (tie->status == MBNA_TIE_Z)
              tiestatus = tiestatus_z;
            else if (tie->status == MBNA_TIE_XYZ_FIXED)
              tiestatus = tiestatus_xyz_f;
            else if (tie->status == MBNA_TIE_XY_FIXED)
              tiestatus = tiestatus_xy_f;
            else if (tie->status == MBNA_TIE_Z_FIXED)
              tiestatus = tiestatus_z_f;
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

      XmListAddItems(list_data, xstr, num_ties, 0);
      for (int k = 0; k < num_ties; k++) {
        XmStringFree(xstr[k]);
      }
      free(xstr);
    }
    if (iselect != MBNA_SELECT_NONE) {
      XmListSelectPos(list_data, iselect + 1, 0);
      XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
    }
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
            || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
            || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {

    if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
      snprintf(string, sizeof(string), "Sorted Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Sorted Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Sorted Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Sorted Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Sorted Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Sorted Ties with File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Sorted Ties with Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Sorted Ties:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count crossing ties */
      int num_ties = 0;
      project.tiessortedthreshold = 0.0;
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        if (do_check_crossing_listok(icrossing)) {
          struct mbna_crossing *crossing = &(project.crossings[icrossing]);
          num_ties += crossing->num_ties;
        }
      }

      /* allocate strings for list */
      XmString *xstr = (XmString *)malloc(num_ties * sizeof(XmString));

      /* allocate array of tie pointers for list to be sorted */
      struct mbna_tie **tie_ptr_list = NULL;
      tie_ptr_list = (struct mbna_tie **) malloc(num_ties * sizeof(struct mbna_tie *));

      /* get list of ties */
      num_ties = 0;
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        if (do_check_crossing_listok(icrossing)) {
          struct mbna_crossing *crossing = &(project.crossings[icrossing]);
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            struct mbna_tie *tie = &crossing->ties[itie];
            tie->icrossing = icrossing;
            tie->itie = itie;
            tie_ptr_list[num_ties] = tie;
            num_ties++;
          }
        }
      }

      /* sort the ties from smallest to largest model misfit and get threshold for displaying */
      if (num_ties > 0) {
        qsort((void *)tie_ptr_list, (size_t) num_ties, sizeof(struct mbna_tie *), mbnavadjust_tie_compare);

        /* get threshold for displaying */
        project.tiessortedthreshold = 0.0;
        if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL) {
          project.tiessortedthreshold = 0.0;
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST) {
          project.tiessortedthreshold = tie_ptr_list[99 * num_ties / 100]->sigma_m;
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
          project.tiessortedthreshold = tie_ptr_list[19 * num_ties / 20]->sigma_m;
        }
      }

      /* generate list */
      iselect = MBNA_SELECT_NONE;
      int num_ties_list = 0;
      for (int ktie  = num_ties - 1; ktie >= 0; ktie--) {
        struct mbna_tie *tie = tie_ptr_list[ktie];
        if (do_check_crossing_listok(tie->icrossing) && tie->sigma_m >= project.tiessortedthreshold) {
          struct mbna_crossing *crossing = &(project.crossings[tie->icrossing]);
          tie = (struct mbna_tie *)&crossing->ties[tie->itie];
          char *tiestatus = NULL;
          if (tie->status == MBNA_TIE_XYZ)
            tiestatus = tiestatus_xyz;
          else if (tie->status == MBNA_TIE_XY)
            tiestatus = tiestatus_xy;
          else if (tie->status == MBNA_TIE_Z)
            tiestatus = tiestatus_z;
          else if (tie->status == MBNA_TIE_XYZ_FIXED)
            tiestatus = tiestatus_xyz_f;
          else if (tie->status == MBNA_TIE_XY_FIXED)
            tiestatus = tiestatus_xy_f;
          else if (tie->status == MBNA_TIE_Z_FIXED)
            tiestatus = tiestatus_z_f;
          if (tie->inversion_status == MBNA_INVERSION_CURRENT)
            sprintf(string,
              "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
              "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
              tie->icrossing, tie->itie, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
              crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
              crossing->file_id_2, crossing->section_2, tie->snav_2,
              tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
              tie->sigmar1, tie->sigmar2, tie->sigmar3,
              tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
          else if (tie->inversion_status == MBNA_INVERSION_OLD)
            sprintf(string,
              "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
              "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
              tie->icrossing, tie->itie, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
              crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
              crossing->file_id_2, crossing->section_2, tie->snav_2,
              tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
              tie->sigmar1, tie->sigmar2, tie->sigmar3,
              tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m, tie->rsigma_m);
          else
            sprintf(string,
              "%6d %2d %s %2.2d:%4.4d:%3.3d:%2.2d %2.2d:%4.4d:%3.3d:%2.2d "
              "%8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
              tie->icrossing, tie->itie, tiestatus, project.files[crossing->file_id_1].block, crossing->file_id_1,
              crossing->section_1, tie->snav_1, project.files[crossing->file_id_2].block,
              crossing->file_id_2, crossing->section_2, tie->snav_2,
              tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
              tie->sigmar1, tie->sigmar2, tie->sigmar3);
          xstr[num_ties_list] = XmStringCreateLocalized(string);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", string);
          if (tie->icrossing == mbna_crossing_select && tie->itie == mbna_tie_select)
            iselect = num_ties_list;
          num_ties_list++;
        }
      }

      XmListAddItems(list_data, xstr, num_ties_list, 0);
      for (int kk = 0; kk < num_ties_list; kk++) {
        XmStringFree(xstr[kk]);
      }
      free(xstr);

      free(tie_ptr_list);
    }
    if (iselect != MBNA_SELECT_NONE) {
      XmListSelectPos(list_data, iselect + 1, 0);
      XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
    }
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
    if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
      snprintf(string, sizeof(string), "Global Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Global Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Global Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Global Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Global Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Global Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Global Ties of Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Global Ties:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {
      /* count global ties */
      int num_globalties = 0;

      /* count global ties */
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if (section->status == MBNA_CROSSING_STATUS_SET &&
              ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
               (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i && mbna_section_select == j))) {
            num_globalties++;
          }
        }
      }

      /* allocate strings for list */
      XmString *xstr = (XmString *)malloc(num_globalties * sizeof(XmString));

      /* generate list */
      num_globalties = 0;
      iselect = MBNA_SELECT_NONE;

      /* list global ties */
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if (section->status == MBNA_CROSSING_STATUS_SET &&
              ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
               (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i && mbna_section_select == j))) {
            char *tiestatus = NULL;
            if (section->globaltie.status == MBNA_TIE_XYZ)
              tiestatus = tiestatus_xyz;
            else if (section->globaltie.status == MBNA_TIE_XY)
              tiestatus = tiestatus_xy;
            else if (section->globaltie.status == MBNA_TIE_Z)
              tiestatus = tiestatus_z;
            else if (section->globaltie.status == MBNA_TIE_XYZ_FIXED)
              tiestatus = tiestatus_xyz_f;
            else if (section->globaltie.status == MBNA_TIE_XY_FIXED)
              tiestatus = tiestatus_xy_f;
            else if (section->globaltie.status == MBNA_TIE_Z_FIXED)
              tiestatus = tiestatus_z_f;
            if (section->globaltie.inversion_status == MBNA_INVERSION_CURRENT)
              sprintf(string,
                "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
                project.files[i].block, i, j, section->globaltie.snav, tiestatus,
                section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
                section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
                section->globaltie.sigma_m, section->globaltie.rsigma_m);
            else if (section->globaltie.inversion_status == MBNA_INVERSION_OLD)
              sprintf(string,
                "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
                project.files[i].block, i, j, section->globaltie.snav, tiestatus,
                section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
                section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
                section->globaltie.sigma_m, section->globaltie.rsigma_m);
            else
              sprintf(string,
                "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
                project.files[i].block, i, j, section->globaltie.snav, tiestatus,
                section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
                section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3);
            xstr[num_globalties] = XmStringCreateLocalized(string);
            if (mbna_verbose > 0)
              fprintf(stderr, "%s\n", string);
            if (section->section_id == mbna_section_select && section->file_id == mbna_file_select)
              iselect = num_globalties;
            num_globalties++;
          }
        }
      }

      XmListAddItems(list_data, xstr, num_globalties, 0);
      for (int k = 0; k < num_globalties; k++) {
        XmStringFree(xstr[k]);
      }
      free(xstr);
    }
    if (iselect != MBNA_SELECT_NONE) {
      XmListSelectPos(list_data, iselect + 1, 0);
      XmListSetPos(list_data, MAX(iselect + 1 - 5, 1));
    }
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
      snprintf(string, sizeof(string), "Global Ties:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr");
    else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
      snprintf(string, sizeof(string), "Global Ties of Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK)
      snprintf(string, sizeof(string), "Global Ties of Survey-vs-Survey Block %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_block_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
      snprintf(string, sizeof(string), "Global Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
      snprintf(string, sizeof(string), "Global Ties with Survey %d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
      snprintf(string, sizeof(string), "Global Ties of File %d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select);
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
      snprintf(string, sizeof(string), "Global Ties of Section %d:%d:%d:  Xing Tie Stat Sur1:Fil1:Sec1:Nv1 Sur2:Fil2:Sec2:Nv2 Offx Offy Offz | S1 S2 S3 | Ex Ey Ez | Se Sr", mbna_survey_select, mbna_file_select, mbna_section_select);
    else
      snprintf(string, sizeof(string), "Global Ties:");
    set_label_string(label_listdata, string);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", string);
    if (project.num_files > 0) {

      /* count global ties */
      int num_globalties = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if (section->status == MBNA_CROSSING_STATUS_SET &&
              ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
               (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i && mbna_section_select == j))) {
            num_globalties++;
          }
        }
      }

      /* allocate strings for list */
      XmString *xstr = (XmString *)malloc(num_globalties * sizeof(XmString));

      /* allocate array of section pointers for list to be sorted */
      struct mbna_section **section_ptr_list = NULL;
      section_ptr_list = (struct mbna_section **) malloc(num_globalties * sizeof(struct mbna_section *));

      /* get list of global ties */
      num_globalties = 0;
      for (int i = 0; i < project.num_files; i++) {
        struct mbna_file *file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
          struct mbna_section *section = &(file->sections[j]);
          if (section->status == MBNA_CROSSING_STATUS_SET &&
              ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
               (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == i) ||
               (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == i && mbna_section_select == j))) {
            section_ptr_list[num_globalties] = section;
            num_globalties++;
          }
        }
      }

      /* sort the ties from smallest to largest model misfit */
      qsort((void *)section_ptr_list, (size_t) num_globalties, sizeof(struct mbna_section *), mbnavadjust_globaltie_compare);

      /* generate list */
      iselect = MBNA_SELECT_NONE;
      int kk = 0;
      for (int kglobaltie  = num_globalties - 1; kglobaltie >= 0; kglobaltie--) {
        struct mbna_section *section = section_ptr_list[kglobaltie];
        char *tiestatus = NULL;
        if (section->globaltie.status == MBNA_TIE_XYZ)
          tiestatus = tiestatus_xyz;
        else if (section->globaltie.status == MBNA_TIE_XY)
          tiestatus = tiestatus_xy;
        else if (section->globaltie.status == MBNA_TIE_Z)
          tiestatus = tiestatus_z;
        else if (section->globaltie.status == MBNA_TIE_XYZ_FIXED)
          tiestatus = tiestatus_xyz_f;
        else if (section->globaltie.status == MBNA_TIE_XY_FIXED)
          tiestatus = tiestatus_xy_f;
        else if (section->globaltie.status == MBNA_TIE_Z_FIXED)
          tiestatus = tiestatus_z_f;
        if (section->globaltie.inversion_status == MBNA_INVERSION_CURRENT)
          sprintf(string,
            "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f",
            project.files[section->file_id].block,
            section->file_id, section->section_id,
            section->globaltie.snav, tiestatus,
            section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
            section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
            section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
            section->globaltie.sigma_m, section->globaltie.rsigma_m);
        else if (section->globaltie.inversion_status == MBNA_INVERSION_OLD)
          sprintf(string,
            "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %8.2f %6.3f ***",
            project.files[section->file_id].block,
            section->file_id, section->section_id,
            section->globaltie.snav, tiestatus,
            section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
            section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3,
            section->globaltie.dx_m, section->globaltie.dy_m, section->globaltie.dz_m,
            section->globaltie.sigma_m, section->globaltie.rsigma_m);
        else
          sprintf(string,
            "%2.2d:%4.4d:%3.3d:%2.2d %s %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
            project.files[section->file_id].block,
            section->file_id, section->section_id,
            section->globaltie.snav, tiestatus,
            section->globaltie.offset_x_m, section->globaltie.offset_y_m, section->globaltie.offset_z_m,
            section->globaltie.sigmar1, section->globaltie.sigmar2, section->globaltie.sigmar3);
        xstr[kk] = XmStringCreateLocalized(string);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", string);
        if (section->section_id == mbna_section_select && section->file_id == mbna_file_select)
          iselect = kk;
        kk++;
      }

      XmListAddItems(list_data, xstr, num_globalties, 0);
      for (kk = 0; kk < num_globalties; kk++) {
        XmStringFree(xstr[kk]);
      }
      free(xstr);

      free(section_ptr_list);
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

  if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
    XtVaSetValues(pushButton_poornav, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_goodnav, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_fixednav, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_fixedxynav, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_fixedznav, XmNsensitive, False, NULL);
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && project.num_files > 0 && mbna_survey_select != MBNA_SELECT_NONE) {
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

  if ((mbna_view_list == MBNA_VIEW_LIST_TIES
        || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
        || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
        || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD)
      && project.num_files > 0 && mbna_tie_select != MBNA_SELECT_NONE) {
    if (project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XY
        || project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XY_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
    }
    else if (project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_Z
        || project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_Z_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, False, NULL);
    }
    else if (project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XYZ
        || project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XYZ_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
    }
    if (project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XY
        || project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_Z
        || project.crossings[mbna_crossing_select].ties[mbna_tie_select].status == MBNA_TIE_XYZ) {
      XtVaSetValues(pushButton_tie_unfixed, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_fixed, XmNsensitive, True, NULL);
    }
    else {
      XtVaSetValues(pushButton_tie_unfixed, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_fixed, XmNsensitive, False, NULL);
    }
  }
  else if ((mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
            || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED)
      && project.num_files > 0 && mbna_file_select != MBNA_SELECT_NONE
      && mbna_section_select != MBNA_SELECT_NONE
      && project.files[mbna_file_select].sections[mbna_section_select].status == MBNA_CROSSING_STATUS_SET) {
    if (project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XY
        || project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XY_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
    }
    else if (project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_Z
        || project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_Z_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, False, NULL);
    }
    else if (project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XYZ
        || project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XYZ_FIXED) {
      XtVaSetValues(pushButton_tie_xyz, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_xy, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_z, XmNsensitive, True, NULL);
    }
    if (project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XY
        || project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_Z
        || project.files[mbna_file_select].sections[mbna_section_select].globaltie.status == MBNA_TIE_XYZ) {
      XtVaSetValues(pushButton_tie_unfixed, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_tie_fixed, XmNsensitive, True, NULL);
    }
    else {
      XtVaSetValues(pushButton_tie_unfixed, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_tie_fixed, XmNsensitive, False, NULL);
    }
  }
  else {
    XtVaSetValues(pushButton_tie_xyz, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_tie_xy, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_tie_z, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_tie_unfixed, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_tie_fixed, XmNsensitive, False, NULL);
  }

  if (mbna_status != MBNA_STATUS_GUI) {
    XtVaSetValues(pushButton_new, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_open, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_close, XmNsensitive, False, NULL);
  }
  else if (project.open) {
    XtVaSetValues(pushButton_new, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_open, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_close, XmNsensitive, True, NULL);
  }
  else {
    XtVaSetValues(pushButton_new, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_open, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_close, XmNsensitive, False, NULL);
  }
  if (mbna_status == MBNA_STATUS_GUI && project.open && project.num_files >= 0) {
    XtVaSetValues(pushButton_importdata, XmNsensitive, True, NULL);
  }
  else {
    XtVaSetValues(pushButton_importdata, XmNsensitive, False, NULL);
  }

  if (project.open && project.num_files > 0) {
    XtVaSetValues(pushButton_showreferencegrids, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showsurveys, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showblocks, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showdata, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showsections, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showcrossings, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showcrossingties, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedall, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedworst, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedbad, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showglobalties, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_showglobaltiessorted, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showallsurveys, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showselectedsurvey, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showselectedblock, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showselectedfile, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showwithselectedsurvey, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showwithselectedfile, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_showselectedsection, XmNsensitive, True, NULL);
    if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
      XtVaSetValues(pushButton_showreferencegrids, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
      XtVaSetValues(pushButton_showsurveys, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
      XtVaSetValues(pushButton_showblocks, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
      XtVaSetValues(pushButton_showdata, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
      XtVaSetValues(pushButton_showsections, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
      XtVaSetValues(pushButton_showcrossings, XmNsensitive, False, NULL);
      if (project.num_crossings == project.num_crossings_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
      XtVaSetValues(pushButton_showmediocrecrossings, XmNsensitive, False, NULL);
      if (project.num_crossings == project.num_crossings_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
      XtVaSetValues(pushButton_showgoodcrossings, XmNsensitive, False, NULL);
      if (project.num_crossings == project.num_crossings_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
      XtVaSetValues(pushButton_showbettercrossings, XmNsensitive, False, NULL);
      if (project.num_crossings == project.num_crossings_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
      XtVaSetValues(pushButton_showtruecrossings, XmNsensitive, False, NULL);
      if (project.num_truecrossings == project.num_truecrossings_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
      XtVaSetValues(pushButton_showcrossingties, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL) {
      XtVaSetValues(pushButton_showcrossingtiessortedall, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST) {
      XtVaSetValues(pushButton_showcrossingtiessortedworst, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
      XtVaSetValues(pushButton_showcrossingtiessortedbad, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
      XtVaSetValues(pushButton_showglobalties, XmNsensitive, False, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      XtVaSetValues(pushButton_showglobaltiessorted, XmNsensitive, False, NULL);
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
    XtVaSetValues(pushButton_showcrossingties, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedall, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedworst, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_showcrossingtiessortedbad, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_showglobalties, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_showglobaltiessorted, XmNsensitive, False, NULL);
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

  if (mbna_status == MBNA_STATUS_GUI && project.open && project.num_files > 0) {
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
    XtVaSetValues(pushButton_unsetskipped, XmNsensitive, True, NULL);
    if (project.num_truecrossings == project.num_truecrossings_analyzed
        || project.num_crossings_analyzed >= 10)
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
    XtVaSetValues(pushButton_unsetskipped, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_invertnav, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_updategrids, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_applynav, XmNsensitive, False, NULL);
    if (project.inversion_status != MBNA_INVERSION_NONE)
      XtVaSetValues(pushButton_showmodelplot, XmNsensitive, True, NULL);
    else
      XtVaSetValues(pushButton_showmodelplot, XmNsensitive, False, NULL);
  }
}
/*--------------------------------------------------------------------*/

void do_update_modelplot_status() {

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  /* deal with modelplot */
  if (project.modelplot) {
    mb_path string = "";

    /* set model view clear block button sensitivity */
    if (mbna_crossing_select == MBNA_SELECT_NONE) {
      XtVaSetValues(pushButton_modelplot_clearblock, XmNsensitive, False, NULL);
    }
    else {
      XtVaSetValues(pushButton_modelplot_clearblock, XmNsensitive, True, NULL);
    }

    struct mbna_crossing *crossing;

    /* set model view status label */
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      if (mbna_crossing_select == MBNA_SELECT_NONE) {
        snprintf(string, sizeof(string), ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
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
        snprintf(string, sizeof(string), ":::t\"Mouse: <left> select  tie; <middle> select untied crossing; <right> drag zoom "
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
        snprintf(string, sizeof(string), ":::t\"Mouse: <left> select  tie; <middle> select block to view; <right> drag zoom "
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

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}
/*--------------------------------------------------------------------*/

void do_update_visualization_status() {

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();
  // fprintf(stderr,"do_update_visualization_status: mbna_crossing_select:%d mbna_tie_select:%d\n",
  // mbna_crossing_select,mbna_tie_select);
  /* deal with modelplot */
  int error = MB_ERROR_NO_ERROR;
  if (project.visualization_status) {

    /* clear any navadjust related interactive nav picks */
    struct mbview_shareddata_struct *shareddata;
    mbview_getsharedptr(mbna_verbose, &shareddata, &error);

    if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING
        && mbna_crossing_select != MBNA_SELECT_NONE) {
      /* select the data sections associated with the loaded crossing */
      struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
      mbview_clearnavpicks(0);
      shareddata->nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
      shareddata->nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
      mb_path name;
      sprintf(name, "%4.4d:%4.4d", crossing->file_id_1, crossing->section_1);
      mbview_picknavbyname(mbna_verbose, 0, name, &error);
      sprintf(name, "%4.4d:%4.4d", crossing->file_id_2, crossing->section_2);
      mbview_picknavbyname(mbna_verbose, 0, name, &error);

      /* select the route associated with the selected tie */
      if (mbna_tie_select != MBNA_SELECT_NONE) {
        struct mbna_file *file_1 = (struct mbna_file *)&project.files[crossing->file_id_1];
        struct mbna_file *file_2 = (struct mbna_file *)&project.files[crossing->file_id_2];
        sprintf(name, "%4.4d:%1d %2.2d:%4.4d:%2.2d %2.2d:%4.4d:%2.2d", mbna_crossing_select, mbna_tie_select,
          file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
          crossing->section_2);
      }
      else {
        sprintf(name, "MBNA_SELECT_NONE");
      }
      mbview_pick_routebyname(mbna_verbose, 0, name, &error);
    }

    else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION
            && mbna_file_select != MBNA_SELECT_NONE
            && mbna_section_select != MBNA_SELECT_NONE) {
      /* select the data section associated with the loaded section */
      shareddata->nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
      shareddata->nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
      mbview_clearnavpicks(0);
      mb_path name;
      sprintf(name, "%4.4d:%4.4d", mbna_file_select, mbna_section_select);
      mbview_picknavbyname(mbna_verbose, 0, name, &error);
    }
  }
  // fprintf(stderr,"Calling mbview_update from do_update_visualization_status\n");

  mbview_update(mbna_verbose, 0, &error);

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}
/*--------------------------------------------------------------------*/

void do_naverr_init(int mode) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called   mode:%d\n", __func__, mode);
  }

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

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

  /* set naverr mode and open crossing or section appropriately */
  if (mode == MBNA_NAVERR_MODE_CROSSING) {
    /* get current crossing - mbna_naverr_mode will be set to MBNA_NAVERR_MODE_CROSSING */
    if (mbna_crossing_select == MBNA_SELECT_NONE)
      mbnavadjust_naverr_nextunset_crossing();
    else
      mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
  } else if (mode == MBNA_NAVERR_MODE_SECTION) {
    /* get current section - mbna_naverr_mode will be set to MBNA_NAVERR_MODE_SECTION */
    if (mbna_section_select == MBNA_SELECT_NONE)
      mbnavadjust_naverr_nextunset_section();
    else
      mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
  }

  /* update naverr labels */
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}

/*--------------------------------------------------------------------*/

void do_naverr_update() {

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  mb_path string = "";

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0) {

    const double zoom_factor =
      (mbna_plot_lon_max - mbna_plot_lon_min) > 0.0
      ? 100.0 * MAX((mbna_lon_max - mbna_lon_min) / (mbna_plot_lon_max - mbna_plot_lon_min),
        (mbna_lat_max - mbna_lat_min) / (mbna_plot_lat_max - mbna_plot_lat_min))
      : 0.0;

    const double plot_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;
    const double misfit_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;

    /* get time difference */
    const double timediff =
      (project.files[mbna_file_id_2].sections[mbna_section_2].btime_d -
       project.files[mbna_file_id_1].sections[mbna_section_1].btime_d) /
      86400.0;

    /* set main naverr status label */
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    struct mbna_tie *tie = &crossing->ties[mbna_current_tie];
    if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
      snprintf(string, sizeof(string), ":::t\"Crossing: %d of %d\"\
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
      snprintf(string, sizeof(string), ":::t\"Crossing: %d of %d\"\
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
      snprintf(string, sizeof(string), ":::t\"Crossing: %d of %d\"\
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
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }

    do_naverr_offsetlabel();
  }

  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0) {

    const double zoom_factor =
      (mbna_plot_lon_max - mbna_plot_lon_min) > 0.0
      ? 100.0 * MAX((mbna_lon_max - mbna_lon_min) / (mbna_plot_lon_max - mbna_plot_lon_min),
        (mbna_lat_max - mbna_lat_min) / (mbna_plot_lat_max - mbna_plot_lat_min))
      : 0.0;

    const double plot_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;
    const double misfit_width = (mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon;

    /* set main naverr status label */
    struct mbna_file *file = &project.files[mbna_current_file];
    struct mbna_section *section = &file->sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section->globaltie;

    if (section->status == MBNA_CROSSING_STATUS_NONE) {
      snprintf(string, sizeof(string), ":::t\"Section: %2.2d:%3.3d:%3.3d\"\
:t\"Global Tie Status: Unset \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   None None None\"",
        project.files[mbna_current_file].block, mbna_current_file, mbna_current_section, plot_width,
        misfit_width, project.zoffsetwidth, zoom_factor);
    }
    else if (section->status == MBNA_CROSSING_STATUS_SKIP) {
      snprintf(string, sizeof(string), ":::t\"Section: %2.2d:%3.3d:%3.3d\"\
:t\"Global Tie Status: Unset (skipped) \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   None None None\"",
        project.files[mbna_current_file].block, mbna_current_file, mbna_current_section, plot_width,
        misfit_width, project.zoffsetwidth, zoom_factor);
    }
    else {
      snprintf(string, sizeof(string), ":::t\"Section: %2.2d:%3.3d:%3.3d\"\
:t\"Global Tie Status: Set \"\
:t\"Nav Point: %4d\"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets (m):   %.3f %.3f %.3f\"\
:t\"Sigma (m):   %.3f %.3f %.3f\"",
        project.files[mbna_current_file].block, mbna_current_file, mbna_current_section,
        globaltie->snav, plot_width, misfit_width, project.zoffsetwidth, zoom_factor,
        globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m,
        globaltie->sigmar1, globaltie->sigmar2, globaltie->sigmar3);
    }
    set_label_multiline_string(label_naverr_status, string);

    /* set some button sensitivities */
    XtVaSetValues(pushButton_naverr_deletetie, XmNsensitive, (section->status == MBNA_CROSSING_STATUS_SET), NULL);
    XtVaSetValues(pushButton_naverr_selecttie, XmNsensitive, false, NULL);
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
    else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      if (project.num_globalties == project.num_globalties_analyzed)
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
      else
        XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, True, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      if (project.num_globalties == project.num_globalties_analyzed)
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
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      XtVaSetValues(pushButton_naverr_previous, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_next, XmNsensitive, True, NULL);
      XtVaSetValues(pushButton_naverr_nextunset, XmNsensitive, False, NULL);
    }

    do_naverr_offsetlabel();
  }

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}

/*--------------------------------------------------------------------*/

void do_naverr_offsetlabel() {
  mb_path string = "";
  
  /* look at current crossing */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && mbna_current_crossing >= 0) {
    snprintf(string, sizeof(string), ":::t\"Working Offsets (m): %.3f %.3f %.3f %d:%d\":t\"Sigma (m): %.3f %.3f %.3f\"",
      mbna_offset_x / mbna_mtodeglon, mbna_offset_y / mbna_mtodeglat, mbna_offset_z, mbna_snav_1, mbna_snav_2,
      mbna_minmisfit_sr1, mbna_minmisfit_sr2, mbna_minmisfit_sr3);
  }

  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0) {
    snprintf(string, sizeof(string), ":::t\"Working Offsets (m): %.3f %.3f %.3f %d:%d\":t\"Sigma (m): %.3f %.3f %.3f\"",
      mbna_offset_x / mbna_mtodeglon, mbna_offset_y / mbna_mtodeglat, mbna_offset_z, mbna_snav_1, mbna_snav_2,
      mbna_minmisfit_sr1, mbna_minmisfit_sr2, mbna_minmisfit_sr3);
  }

  else {
    snprintf(string, sizeof(string), ":::t\"Working Offsets (m): %.3f %.3f %.3f\":t\"Working Tie Points: %d:%d\"", 0.0, 0.0, 0.0, 0, 0);
  }
  set_label_multiline_string(label_naverr_offsets, string);

  /* set button sensitivity for setting or resetting offsets */
  XtVaSetValues(pushButton_naverr_settie, XmNsensitive, mbna_allow_set_tie, NULL);
  XtVaSetValues(pushButton_naverr_resettie, XmNsensitive, mbna_allow_set_tie, NULL);
  XtVaSetValues(pushButton_naverr_addtie, XmNsensitive, mbna_allow_add_tie, NULL);
}

/*--------------------------------------------------------------------*/

void do_naverr_test_graphics() {
  /* now test graphics */
  int ox = 0;
  int oy = 0;
  int dx = (cont_borders[1] - cont_borders[0]) / 16;
  int dy = (cont_borders[3] - cont_borders[2]) / 16;
  double rx = cont_borders[1] - ox;
  double ry = cont_borders[3] - oy;
  double rr = sqrt(rx * rx + ry * ry);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      const int k = 16 * j + i;
      ox = i * dx;
      oy = j * dy;
      xg_fillrectangle(cont_xgid, ox, oy, dx, dy, mpixel_values[k], 0);
      xg_fillrectangle(cont_xgid, ox + dx / 4, oy + dy / 4, dx / 2, dy / 2, k, 0);
    }
  }
  ox = (corr_borders[1] - corr_borders[0]) / 2;
  oy = (corr_borders[3] - corr_borders[2]) / 2;
  rx = corr_borders[1] - ox;
  ry = corr_borders[3] - oy;
  rr = sqrt(rx * rx + ry * ry);
  for (int i = corr_borders[0]; i < corr_borders[1]; i++) {
    for (int j = corr_borders[2]; j < corr_borders[3]; j++) {
      rx = i - ox;
      ry = j - oy;
      const double r = sqrt(rx * rx + ry * ry);
      const int k = 6 + (int)(80 * r / rr);
      xg_fillrectangle(corr_xgid, i, j, 1, 1, mpixel_values[k], 0);
    }
  }
}
/*--------------------------------------------------------------------*/

void do_list_data_select(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  int position_count = 0;
  int *position_list = NULL;

  if (XmListGetSelectedPos(list_data, &position_list, &position_count)) {
    XmListCallbackStruct *acs = (XmListCallbackStruct *)call_data;
    mb_path selected_item;
    char *tmp = NULL;
    struct mbna_file *file;
    int num_files, num_sections, num_crossings, num_ties;
    bool found = true;

    if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
      project.refgrid_select = position_list[0] - 1;
fprintf(stderr,"mbna_referencegrid_select:%d of %d\n", project.refgrid_select, project.num_refgrids);
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
      mbna_section_select = 0;
      mbna_file_select = MBNA_SELECT_NONE;
      mbna_survey_select = position_list[0] - 1;

      /* get selected file from list */
      for (int i= 0; i < project.num_files; i++) {
        file = &(project.files[i]);
        if (mbna_file_select == MBNA_SELECT_NONE && mbna_survey_select == file->block) {
          mbna_file_select = i;
          mbna_section_select = 0;
        }
      }
      // fprintf(stderr,"mbna_survey_select:%d:%d:%d\n",mbna_survey_select,mbna_file_select,mbna_section_select);
      project.modelplot_uptodate = false;
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_BLOCKS) {
      if ((acs->item != NULL && XmStringGetLtoR(acs->item, XmFONTLIST_DEFAULT_TAG, &tmp))
          || (acs->selected_items != NULL && XmStringGetLtoR(acs->selected_items[0], XmFONTLIST_DEFAULT_TAG, &tmp))) {
        strncpy(selected_item, tmp, sizeof(selected_item));
        XtFree(tmp);
        tmp = NULL;
      }
      int iblock_select, isurvey1, isurvey2, d1, d2, d3, d4, d5;
      const int nscan = sscanf(selected_item, "block %d: Survey %d vs Survey %d : Crossings: %d %d %d %d : Ties: %d",
             &iblock_select, &isurvey1, &isurvey2, &d1, &d2, &d3, &d4, &d5);
      if (nscan == 8) {
        mbna_block_select = iblock_select;
        mbna_block_select1 = isurvey1;
        mbna_block_select2 = isurvey2;
      }
      // fprintf(stderr,"mbna_block_select:%d:%d:%d\n",mbna_block_select,mbna_block_select1,mbna_block_select2);
      project.modelplot_uptodate = false;
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
      num_files = 0;

      /* get selected file from list */
      for (int i= 0; i < project.num_files; i++) {
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
      project.modelplot_uptodate = false;
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
      /* get selected section from list */
      num_sections = 0;
      for (int i= 0; i < project.num_files; i++) {
        file = &(project.files[i]);
        for (int j = 0; j < file->num_sections; j++) {
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
      project.modelplot_uptodate = false;

      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }

      /* else if naverr window is up, load selected section */
      else {
        mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS || mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS ||
       mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS || mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS ||
       mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
      /* get selected crossing from list */
      num_crossings = 0;
      for (int i= 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          if (num_crossings == position_list[0] - 1) {
            mbna_crossing_select = i;
            mbna_tie_select = 0;
          }
          num_crossings++;
        }
      }
      project.modelplot_uptodate = false;

      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }

      /* else if naverr window is up, load selected crossing */
      else {
        mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
      /* get crossing and tie from list */
      num_crossings = 0;
      num_ties = 0;
      int found = false;
      for (int i= 0; i < project.num_crossings; i++) {
        if (do_check_crossing_listok(i)) {
          for (int j = 0; j < project.crossings[i].num_ties; j++) {
            if (num_ties == position_list[0] - 1) {
              mbna_crossing_select = i;
              mbna_tie_select = j;
              found = true;
            }
            num_ties++;
          }
          num_crossings++;
        }
      }

      /* load selected crossing tie into naverr window, global ties ignored */
      if (found) {
        /* bring up naverr window if required */
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
        }
        project.modelplot_uptodate = false;
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
      /* get crossing and tie from selected item in the list */
      if ((acs->item != NULL && XmStringGetLtoR(acs->item, XmFONTLIST_DEFAULT_TAG, &tmp))
          || (acs->selected_items != NULL && XmStringGetLtoR(acs->selected_items[0], XmFONTLIST_DEFAULT_TAG, &tmp))) {
        strncpy(selected_item, tmp, sizeof(selected_item));
        XtFree(tmp);
        tmp = NULL;
        int i;
        int j;
        const int nscan = sscanf(selected_item, "%d %d ", &i, &j);
        if (nscan == 2 && i >= 0 && i < project.num_crossings
            && do_check_crossing_listok(i)
            && j >= 0 && j < project.crossings[i].num_ties) {
          mbna_crossing_select = i;
          mbna_tie_select = j;
          found = true;
        }
      }

      /* load selected crossing tie into naverr window, global ties ignored */
      if (found) {
        /* bring up naverr window if required */
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
        }
        project.modelplot_uptodate = false;
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      /* get global tie from selected item in the list */
      if ((acs->item != NULL && XmStringGetLtoR(acs->item, XmFONTLIST_DEFAULT_TAG, &tmp))
          || (acs->selected_items != NULL && XmStringGetLtoR(acs->selected_items[0], XmFONTLIST_DEFAULT_TAG, &tmp))) {
        strncpy(selected_item, tmp, sizeof(selected_item));
        XtFree(tmp);
        tmp = NULL;
        int isurvey, ifile, jsection, ksnav;
        const int nscan = sscanf(selected_item, "%d:%d:%d:%d ", &isurvey, &ifile, &jsection, &ksnav);
        if (nscan == 4 && ifile >= 0 && ifile < project.num_files
            && jsection >= 0 && jsection < project.files[ifile].num_sections) {
          mbna_file_select = ifile;
          mbna_section_select = jsection;
          found = true;
        }
      }

      /* load selected section into naverr window */
      if (found) {
        /* bring up naverr window if required */
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_SECTION);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
        }
        project.modelplot_uptodate = false;
      }
    }

    free(position_list);
  }

  /* else user selected same list item, deselecting it  - don't change anything
      - bring up naverr if needed - and let do_update_status redraw list with
     previous item selected */
  else {
    if (mbna_view_list == MBNA_VIEW_LIST_REFERENCEGRIDS) {
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS) {
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILES) {
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIES) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
              || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }
    }
    else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }
    }
  }

  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}

/*--------------------------------------------------------------------*/

int do_check_crossing_listok(int icrossing) {
  bool use_status = false;

  /* get crossing */
  struct mbna_crossing *crossing = &(project.crossings[icrossing]);

  /* check for list type */
  if (icrossing == mbna_crossing_select) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS && crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS && crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS && crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS && crossing->truecrossing) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_TIES && crossing->num_ties > 0) {
    use_status = true;
  }
  else if ((mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
            || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
            || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD)
          && crossing->num_ties > 0) {
    for (int itie=0; itie < crossing->num_ties; itie++) {
      struct mbna_tie *tie = &crossing->ties[itie];
      if (tie->sigma_m >= project.tiessortedthreshold) {
        use_status = true;
      }
    }
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
    use_status = false;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    use_status = false;
  }
  else {
    use_status = false;
  }

  /* check view mode modifiers */
  if (use_status) {
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
      use_status = true;
    }
    else {
      use_status = false;
    }
  }

  return (use_status);
}

/*--------------------------------------------------------------------*/

int do_check_section_listok(int ifile, int isection) {
  bool use_status = false;

  /* check for list type */
  if (ifile == mbna_file_select && isection == mbna_section_select) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
    use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES) {
    if (project.files[ifile].sections[isection].status == MBNA_CROSSING_STATUS_SET)
      use_status = true;
  }
  else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    if (project.files[ifile].sections[isection].status == MBNA_CROSSING_STATUS_SET)
      use_status = true;
  }
  else {
    use_status = false;
  }

  /* check view mode modifiers */
  if (use_status) {
    if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
        (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == project.files[ifile].block) ||
        (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_file_select == project.files[ifile].block) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection) ) {
      use_status = true;
    }
    else {
      use_status = false;
    }
  }

  return (use_status);
}

/*--------------------------------------------------------------------*/

int do_check_globaltie_listok(int ifile, int isection) {
  /* get file and section */
  struct mbna_file *file = &(project.files[ifile]);
  struct mbna_section *section = &(file->sections[isection]);

  /* if there is a global tie check for view mode */
  bool use_status = false;
  if (section->status == MBNA_CROSSING_STATUS_SET) {
    if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
        (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
        (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
         (mbna_block_select1 == file->block || mbna_block_select2 == file->block)) ||
        (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection)) {
      use_status = true;
    }
  }

  return (use_status);
}
/*--------------------------------------------------------------------*/

int do_check_nav_active(int ifile, int isection) {
  /* get file and section */
  struct mbna_file *file = &(project.files[ifile]);

  /* check section nav for view mode */
  bool active = false;
  if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
      (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->block) ||
      (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
       (mbna_block_select1 == file->block || mbna_block_select2 == file->block)) ||
      (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
      (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->block) ||
      (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
      (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection) ||
      (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection)) {
    active = true;
  }

  return (active);
}

/*--------------------------------------------------------------------*/

void do_naverr_cont_expose(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void do_naverr_corr_expose(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void do_naverr_cont_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* If there is input in the drawing area */
  if (acs->reason == XmCR_INPUT) {
    XEvent *event = acs->event;

    /* Check for mouse pressed. */
    if (event->xany.type == ButtonPress) {
      /* If left mouse button is pushed then save position. */
      if (event->xbutton.button == 1) {
        button1down = true;
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
        button2down = true;
        mbna_zoom_x1 = event->xbutton.x;
        mbna_zoom_y1 = event->xbutton.y;
        mbna_zoom_x2 = event->xbutton.x;
        mbna_zoom_y2 = event->xbutton.y;

        /* replot contours */
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOMFIRST);
      } /* end of middle button events */

      /* If right mouse button is pushed */
      if (event->xbutton.button == 3) {
        button3down = true;

        /* get new snav points */
        mbnavadjust_naverr_snavpoints(event->xbutton.x, event->xbutton.y);

        /* replot contours */
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
      } /* end of right button events */
    } /* end of button press events */

    /* Check for mouse released. */
    if (event->xany.type == ButtonRelease) {
      if (event->xbutton.button == 1) {
        button1down = false;
      }
      if (event->xbutton.button == 2) {
        button2down = false;
        mbna_zoom_x2 = event->xbutton.x;
        mbna_zoom_y2 = event->xbutton.y;

        const double x1 = mbna_zoom_x1 / mbna_plotx_scale + mbna_plot_lon_min;
        const double y1 = (cont_borders[3] - mbna_zoom_y1) / mbna_ploty_scale + mbna_plot_lat_min;
        const double x2 = mbna_zoom_x2 / mbna_plotx_scale + mbna_plot_lon_min;
        const double y2 = (cont_borders[3] - mbna_zoom_y2) / mbna_ploty_scale + mbna_plot_lat_min;

        /* get new plot bounds */
        mbna_plot_lon_min = MIN(x1, x2);
        mbna_plot_lon_max = MAX(x1, x2);
        mbna_plot_lat_min = MIN(y1, y2);
        mbna_plot_lat_max = MAX(y1, y2);

        /* replot contours and misfit */
        mbnavadjust_get_misfit();
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
      }
      if (event->xbutton.button == 3) {
        button3down = false;
      }

    } /* end of button release events */

    /* Check for mouse motion while pressed. */
    if (event->xany.type == MotionNotify) {
      if (button1down) {
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
      else if (button2down) {
        mbna_zoom_x2 = event->xmotion.x;
        mbna_zoom_y2 = event->xmotion.y;

        /* replot contours */
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOM);
      }
    }
  } /* end of inputs from window */

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}

/*--------------------------------------------------------------------*/

void do_naverr_corr_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* If there is input in the drawing area */
  if (acs->reason == XmCR_INPUT) {
    XEvent *event = acs->event;

    /* Check for mouse pressed. */
    if (event->xany.type == ButtonPress) {
      /* If left mouse button is pushed then save position. */
      if (event->xbutton.button == 1) {
        button1down = true;
        mbna_offset_x_old = mbna_offset_x;
        mbna_offset_y_old = mbna_offset_y;
        mbna_offset_z_old = mbna_offset_z;
        mbna_offset_x =
          mbna_misfit_offset_x + (event->xbutton.x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
        mbna_offset_y =
          mbna_misfit_offset_y - (event->xbutton.y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;

        /* replot contours */
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
        do_naverr_update();
        do_naverr_offsetlabel();

      } /* end of left button events */
    } /* end of button press events */

    /* Check for mouse released. */
    if (event->xany.type == ButtonRelease) {
      if (event->xbutton.button == 1) {
        button1down = false;
      }
    } /* end of button release events */

    /* Check for mouse motion while pressed. */
    if (event->xany.type == MotionNotify) {
      if (button1down) {
        /* move offset */
        mbna_offset_x =
          mbna_misfit_offset_x + (event->xmotion.x - (corr_borders[0] + corr_borders[1]) / 2) / mbna_misfit_xscale;
        mbna_offset_y =
          mbna_misfit_offset_y - (event->xmotion.y - (corr_borders[3] + corr_borders[2]) / 2) / mbna_misfit_yscale;

        /* replot contours */
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
        do_naverr_update();
        do_naverr_offsetlabel();

        /* reset old position */
        mbna_offset_x_old = mbna_offset_x;
        mbna_offset_y_old = mbna_offset_y;
        mbna_offset_z_old = mbna_offset_z;
      }
    }
  } /* end of inputs from window */

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}
/*--------------------------------------------------------------------*/

void do_naverr_zcorr_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter

  XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* If there is input in the drawing area */
  if (acs->reason == XmCR_INPUT) {
    XEvent *event = acs->event;

    /* Check for mouse pressed. */
    if (event->xany.type == ButtonPress) {
      /* If left mouse button is pushed then save position. */
      if (event->xbutton.button == 1) {
        button1down = true;
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
        do_naverr_update();
        do_naverr_offsetlabel();
      } /* end of left button events */
    } /* end of button press events */

    /* Check for mouse released. */
    if (event->xany.type == ButtonRelease) {
      if (event->xbutton.button == 1) {
        button1down = false;
        mbnavadjust_naverr_replot();
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
        do_naverr_offsetlabel();
      }
    } /* end of button release events */

    /* Check for mouse motion while pressed. */
    if (event->xany.type == MotionNotify) {
      if (button1down) {
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
        do_naverr_update();
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    mbnavadjust_naverr_previous_crossing();
  } else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    mbnavadjust_naverr_previous_section();
  }
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_next(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    mbnavadjust_naverr_next_crossing();
  } else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    mbnavadjust_naverr_next_section();
  }
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);
}

/*--------------------------------------------------------------------*/

void do_naverr_nextunset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    mbnavadjust_naverr_nextunset_crossing();
  } else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    mbnavadjust_naverr_nextunset_section();
  }
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_addtie(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_addtie();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_deletetie(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_deletetie();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_selecttie(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_selecttie();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_naverr_unset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_unset();
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    mbnavadjust_naverr_next_crossing();
  } else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    mbnavadjust_naverr_next_section();
  }
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_setnone(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_skip();
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    mbnavadjust_naverr_nextunset_crossing();
  } else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    mbnavadjust_naverr_nextunset_section();
  }
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_setoffset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_naverr_save();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    project.modelplot_uptodate = false;
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_resettie(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  mbnavadjust_naverr_resettie();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_naverr_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* unload loaded crossing */
  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    status = mbnavadjust_crossing_unload();
    status = mbnavadjust_referencegrid_unload();
  }

  /* deallocate graphics */
  mbna_status = MBNA_STATUS_GUI;
  XFreeGC(display, cont_gc);
  XFreeGC(display, corr_gc);
  xg_free(cont_xgid);
  xg_free(corr_xgid);
  mbna_current_crossing = MBV_SELECT_NONE;
  mbna_current_tie = MBV_SELECT_NONE;
  mbna_current_file = MBV_SELECT_NONE;
  mbna_current_section = MBV_SELECT_NONE;
  //mbna_crossing_select = MBV_SELECT_NONE;
  //mbna_tie_select = MBV_SELECT_NONE;
  mbnavadjust_naverr_checkoksettie();
  do_naverr_update();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/
void do_naverr_fullsize(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* reset the plot bounds */
  mbna_plot_lon_min = mbna_lon_min;
  mbna_plot_lon_max = mbna_lon_max;
  mbna_plot_lat_min = mbna_lat_min;
  mbna_plot_lat_max = mbna_lat_max;
  mbnavadjust_get_misfit();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
}

/*--------------------------------------------------------------------*/

void do_naverr_zerooffset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* move offset */
  mbna_offset_x = 0.0;
  mbna_offset_y = 0.0;
  mbna_offset_z = 0.0;
  /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

  /* recalculate minimum misfit at current z offset */
  mbnavadjust_get_misfitxy();

  /* replot contours */
  mbnavadjust_naverr_replot();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void do_naverr_zerozoffset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* move offset */
  mbna_offset_z = 0.0;
  /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

  /* recalculate minimum misfit at current z offset */
  mbnavadjust_get_misfitxy();

  /* replot contours */
  mbnavadjust_naverr_replot();

  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void do_naverr_applyzoffset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* recalculate minimum misfit at current z offset */
  mbnavadjust_get_misfitxy();

  /* replot contours */
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_offsetlabel();
//  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
//  if (project.visualization_status) {
//    mbnavadjust_reset_visualization_navties();
//    do_update_visualization_status();
//  }
}

/*--------------------------------------------------------------------*/

void do_naverr_minmisfit(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* move offset */
  mbna_offset_x = mbna_minmisfit_x;
  mbna_offset_y = mbna_minmisfit_y;
  mbna_offset_z = mbna_minmisfit_z;
  /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

  /* recalculate minimum misfit at current z offset */
  mbnavadjust_get_misfitxy();

  /* replot contours */
  mbnavadjust_naverr_replot();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
//  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
//  if (project.visualization_status) {
//    mbnavadjust_reset_visualization_navties();
//    do_update_visualization_status();
//  }
}

/*--------------------------------------------------------------------*/

void do_naverr_minxymisfit(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* move offset */
  mbna_offset_x = mbna_minmisfit_xh;
  mbna_offset_y = mbna_minmisfit_yh;
  mbna_offset_z = mbna_minmisfit_zh;
fprintf(stderr,"do_naverr_minxymisfit mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
mbna_minmisfit_xh, mbna_minmisfit_yh, mbna_minmisfit_zh);
fprintf(stderr,"%s %d: mbna_offset_z:%f mbna_offset_z:%f mbna_offset_z:%f\n",
__FILE__, __LINE__, mbna_offset_x, mbna_offset_y, mbna_offset_z);

  /* replot contours */
  mbnavadjust_naverr_replot();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();

}
/*--------------------------------------------------------------------*/

void do_naverr_misfitcenter(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (XmToggleButtonGetState(toggleButton_misfitcenter_zero))
    mbna_misfit_center = MBNA_MISFIT_ZEROCENTER;
  else
    mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;

  /* replot contours and misfit */
  mbnavadjust_get_misfit();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_naverr_update();
//  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
//  if (project.visualization_status) {
//    mbnavadjust_reset_visualization_navties();
//    do_update_visualization_status();
//  }
}

/*--------------------------------------------------------------------*/

void do_biases_apply(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get file structures */
  struct mbna_file *file1 = &(project.files[mbna_file_id_1]);
  struct mbna_file *file2 = &(project.files[mbna_file_id_2]);

  int ivalue;
  XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
  file1->heading_bias = 0.1 * ivalue;

  XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
  file1->roll_bias = 0.1 * ivalue;

  XtVaGetValues(scale_biases_heading2, XmNvalue, &ivalue, NULL);
  file2->heading_bias = 0.1 * ivalue;

  XtVaGetValues(scale_biases_roll2, XmNvalue, &ivalue, NULL);
  file2->roll_bias = 0.1 * ivalue;

  /* set contours out of date */
  for (int isection = 0; isection < file1->num_sections; isection++) {
    file1->sections[isection].contoursuptodate = false;
  }
  for (int isection = 0; isection < file2->num_sections; isection++) {
    file2->sections[isection].contoursuptodate = false;
  }

  mbnavadjust_naverr_replot();
  mbnavadjust_get_misfit();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
//  if (project.visualization_status) {
//    mbnavadjust_reset_visualization_navties();
//    do_update_visualization_status();
//  }
}

/*--------------------------------------------------------------------*/

void do_biases_applyall(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  int ivalue;

  /* get bias values */
  XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
  const double heading_bias = 0.1 * ivalue;
  XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
  const double roll_bias = 0.1 * ivalue;

  /* loop over files */
  for (int ifile = 0; ifile < project.num_files; ifile++) {
    struct mbna_file *file = &(project.files[ifile]);
    file->heading_bias = heading_bias;
    file->roll_bias = roll_bias;

    /* set contours out of date */
    for (int isection = 0; isection < file->num_sections; isection++) {
      file->sections[isection].contoursuptodate = false;
    }
  }

  mbnavadjust_naverr_replot();
  mbnavadjust_get_misfit();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_biases_init(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get file structures */
  struct mbna_file *file1 = &(project.files[mbna_file_id_1]);
  struct mbna_file *file2 = &(project.files[mbna_file_id_2]);

  /* set biases label */
  char value_text[128];
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (XmToggleButtonGetState(toggleButton_biases_together)) {
    if (mbna_bias_mode == MBNA_BIAS_DIFFERENT) {
      mbna_bias_mode = MBNA_BIAS_SAME;
      int ivalue;
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_bias_mode == MBNA_BIAS_SAME) {
    int ivalue;
    XtVaGetValues(scale_biases_heading1, XmNvalue, &ivalue, NULL);
    XtVaSetValues(scale_biases_heading2, XmNvalue, ivalue, XmNsensitive, False, NULL);
  }
}

/*--------------------------------------------------------------------*/

void do_biases_roll(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_bias_mode == MBNA_BIAS_SAME) {
    int ivalue;
    XtVaGetValues(scale_biases_roll1, XmNvalue, &ivalue, NULL);
    XtVaSetValues(scale_biases_roll2, XmNvalue, ivalue, XmNsensitive, False, NULL);
  }
}

/*--------------------------------------------------------------------*/

void do_controls_show(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* set values of decimation slider */
  XtVaSetValues(scale_controls_decimation, XmNvalue, project.decimation, NULL);

  /* set values of section length slider */
  int ivalue = (int)(100 * project.section_length);
  int imax = (int)(100 * 50.0);
  XtVaSetValues(scale_controls_sectionlength, XmNminimum, 1, XmNmaximum, imax, XmNdecimalPoints, 2, XmNvalue, ivalue, NULL);

  /* set values of section soundings slider */
  XtVaGetValues(scale_controls_sectionsoundings, XmNvalue, &ivalue, XmNmaximum, &imax, NULL);
  ivalue = project.section_soundings;
  if (ivalue >= imax) {
    if (ivalue >= 2 * imax)
      imax = 2 * ivalue;
    else
      imax = 2 * imax;
  } else if (ivalue < imax/2) {
    imax = MIN(2 * ivalue, 100000);
  }
  XtVaSetValues(scale_controls_sectionsoundings, XmNminimum, 1, XmNmaximum, imax, XmNvalue, ivalue, NULL);

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
  ivalue = (int)(10 * project.zoffsetwidth);
  XtVaSetValues(scale_controls_zoffset, XmNvalue, ivalue, NULL);

  /* set value of use mode radioBox toggles */
  if (project.use_mode <= MBNA_USE_MODE_PRIMARY) {
    XmToggleButtonSetState(toggleButton_controls_use_primary, TRUE, TRUE);
    project.use_mode = MBNA_USE_MODE_PRIMARY;
  }
  else if (project.use_mode == MBNA_USE_MODE_SECONDARY) {
    XmToggleButtonSetState(toggleButton_controls_use_secondary, TRUE, TRUE);
  }
  else {
    XmToggleButtonSetState(toggleButton_controls_use_tertiary, TRUE, TRUE);
    project.use_mode = MBNA_USE_MODE_TERTIARY;
  }

  /* set misfit offset center toggles */
  if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER) {
    XmToggleButtonSetState(toggleButton_misfitcenter_zero, TRUE, TRUE);
  }
  else {
    XmToggleButtonSetState(toggleButton_misfitcenter_auto, TRUE, TRUE);
  }

}

/*--------------------------------------------------------------------*/

void do_controls_apply(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get value of decimation slider */
  XtVaGetValues(scale_controls_decimation, XmNvalue, &project.decimation, NULL);

  /* get values of section length slider */
  int ivalue;
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
  project.zoffsetwidth = ((double)ivalue) / 10.0;

  /* get value of use mode toggles */
  if (XmToggleButtonGetState(toggleButton_controls_use_primary))
    project.use_mode = MBNA_USE_MODE_PRIMARY;
  else if (XmToggleButtonGetState(toggleButton_controls_use_secondary))
    project.use_mode = MBNA_USE_MODE_SECONDARY;
  else if (XmToggleButtonGetState(toggleButton_controls_use_tertiary))
    project.use_mode = MBNA_USE_MODE_TERTIARY;
  else
    project.use_mode = MBNA_USE_MODE_PRIMARY;

  if (mbna_file_id_1 >= 0 && mbna_section_1 >= 0)
    project.files[mbna_file_id_1].sections[mbna_section_1].contoursuptodate = false;
  if (mbna_file_id_2 >= 0 && mbna_section_2 >= 0)
    project.files[mbna_file_id_2].sections[mbna_section_2].contoursuptodate = false;

  mbnavadjust_naverr_replot();
  int error = MB_ERROR_NO_ERROR;
  mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
  mbnavadjust_get_misfit();
  mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
  do_update_status();
  do_naverr_update();
//  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
//  if (project.visualization_status) {
//    mbnavadjust_reset_visualization_navties();
//    do_update_visualization_status();
//  }
}

/*--------------------------------------------------------------------*/
void do_scale_controls_sectionlength(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_scale_controls_sectionsoundings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  int ivalue;
  int imin;
  int imax;

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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_scale_contourinterval(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
}
/*--------------------------------------------------------------------*/

void do_scale_controls_zoffset(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
}

/*--------------------------------------------------------------------*/
void do_file_new(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  fprintf(stderr, "do_file_new\n");
}

/*--------------------------------------------------------------------*/
void do_file_open(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  fprintf(stderr, "do_file_open\n");
}

/*--------------------------------------------------------------------*/
void do_file_importdata(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  fprintf(stderr, "do_file_importdata\n");
}

/*--------------------------------------------------------------------*/
void do_file_close(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  int error = MB_ERROR_NO_ERROR;

  mbnavadjust_close_project(mbna_verbose, &project, &error);
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/
void do_quit(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* unload loaded crossing */
  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    status = mbnavadjust_crossing_unload();

    /* deallocate graphics */
    mbna_status = MBNA_STATUS_GUI;
    XFreeGC(display, cont_gc);
    XFreeGC(display, corr_gc);
    xg_free(cont_xgid);
    xg_free(corr_xgid);
    mbnavadjust_naverr_checkoksettie();
    do_naverr_update();
    do_update_status();
  }

  /* write project file if there are outstanding changes */
  int error = MB_ERROR_NO_ERROR;
  if (project.save_count != 0) {
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
    project.save_count = 0;
  }

}

/*--------------------------------------------------------------------*/
void do_fileselection_mode(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

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
  else if (file_mode == FILE_MODE_REFERENCE) {
    tmp0 = (XmString)BX_CONVERT(fileSelectionBox, "*.grd", XmRXmString, 0, &argok);
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get input filename */
  mb_path ifile;
  get_text_string(fileSelectionBox_text, ifile);

  /* desl with selection */
  int error = MB_ERROR_NO_ERROR;
  mb_pathplus message;
  if (file_mode == FILE_MODE_NEW) {
    snprintf(message, sizeof(message), "Creating new MBnavadjust project %s", ifile);
    do_message_on(message);
    status = mbnavadjust_file_new(ifile);
    do_message_off();
    do_update_status();
  }
  else if (file_mode == FILE_MODE_OPEN) {
    snprintf(message, sizeof(message), "Opening MBnavadjust project %s", ifile);
    do_message_on(message);
    status = mbnavadjust_file_open(ifile);
    do_message_off();
    do_update_status();
  }
  else if (file_mode == FILE_MODE_IMPORT) {
    char format_text[40];
    get_text_string(textField_format, format_text);
    sscanf(format_text, "%d", &format);
    snprintf(message, sizeof(message), "Importing data from %s %d", ifile, format);
    do_message_on(message);
    status = mbnavadjust_import_data(mbna_verbose, &project, ifile, format, &error);
    do_message_off();

    /* update datalist files and topography grids */
    mbna_status = MBNA_STATUS_NAVSOLVE;
    mbnavadjust_updategrid();
    mbna_status = MBNA_STATUS_GUI;
    do_update_status();
    if (project.modelplot) {
      do_update_modelplot_status();
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    }
    if (project.visualization_status) {
      mbnavadjust_reset_visualization_navties();
      do_update_visualization_status();
    }
  }
  else if (file_mode == FILE_MODE_REFERENCE) {
    status = mbnavadjust_import_reference(mbna_verbose, &project, ifile, &error);
    if (status == MB_SUCCESS) {
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
    } else {
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }
    do_update_status();
  }
}

/*--------------------------------------------------------------------*/
void do_fileselection_cancel(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  file_mode = FILE_MODE_NONE;
}

/*--------------------------------------------------------------------*/

void do_view_showallsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_ALL) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_ALL;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showselectedsurvey(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_SURVEY) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_SURVEY;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showselectedblock(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_BLOCK) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_BLOCK;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showselectedfile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_FILE) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_FILE;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showwithselectedsurvey(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_WITHSURVEY) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_WITHSURVEY;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showwithselectedfile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_WITHFILE) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_WITHFILE;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_view_showselectedsection(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_mode != MBNA_VIEW_MODE_WITHSECTION) {
    project.modelplot_uptodate = false;
  }
  mbna_view_mode = MBNA_VIEW_MODE_WITHSECTION;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_list_showreferencegrids(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_REFERENCEGRIDS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_REFERENCEGRIDS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_list_showsurveys(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_SURVEYS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_SURVEYS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_list_showblocks(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_BLOCKS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_BLOCKS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showdata(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_FILES) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_FILES;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_list_showsections(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_FILESECTIONS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_FILESECTIONS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_CROSSINGS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_CROSSINGS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_list_showmediocrecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_MEDIOCRECROSSINGS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_list_showgoodcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_GOODCROSSINGS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_GOODCROSSINGS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_list_showbettercrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_BETTERCROSSINGS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_BETTERCROSSINGS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showtruecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_TRUECROSSINGS) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_TRUECROSSINGS;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showcrossingties(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_TIES) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_TIES;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showcrossingtiessortedall(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_TIESSORTEDALL) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_TIESSORTEDALL;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}


/*--------------------------------------------------------------------*/
void do_list_showcrossingtiessortedworst(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_TIESSORTEDWORST) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_TIESSORTEDWORST;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}


/*--------------------------------------------------------------------*/
void do_list_showcrossingtiessortedbad(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_TIESSORTEDBAD) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_TIESSORTEDBAD;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showglobalties(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_GLOBALTIES) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_GLOBALTIES;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
void do_list_showglobaltiessorted(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  if (mbna_view_list != MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    project.modelplot_uptodate = false;
  }
  mbna_view_list = MBNA_VIEW_LIST_GLOBALTIESSORTED;
  do_update_status();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_action_poornav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_poornav_file();
  do_update_status();
}

/*--------------------------------------------------------------------*/

void do_action_goodnav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_goodnav_file();
  do_update_status();
}

/*--------------------------------------------------------------------*/

void do_action_fixednav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_fixednav_file();
  do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_fixedxynav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_fixedxynav_file();
  do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_fixedznav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_fixedznav_file();
  do_update_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_xy(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_set_tie_xy();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_z(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_set_tie_z();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_xyz(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_set_tie_xyz();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_unfixed(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_set_tie_unfixed();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_tie_fixed(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_set_tie_fixed();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_action_autopick(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbna_status = MBNA_STATUS_AUTOPICK;
  mbnavadjust_autopick(true);
  mbna_status = MBNA_STATUS_GUI;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_action_autopickhorizontal(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbna_status = MBNA_STATUS_AUTOPICK;
  mbnavadjust_autopick(false);
  mbna_status = MBNA_STATUS_GUI;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}
/*--------------------------------------------------------------------*/

void do_action_autosetsvsvertical(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbna_status = MBNA_STATUS_AUTOPICK;
  mbnavadjust_autosetsvsvertical();
  mbna_status = MBNA_STATUS_GUI;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_action_analyzecrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_action_checknewcrossings(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* turn on message */
  mb_path message;
  sprintf(message, "Checking for crossings...");
  do_message_on(message);

  int error = MB_ERROR_NO_ERROR;
  mbnavadjust_findcrossings(mbna_verbose, &project, &error);

  /* turn off message */
  do_message_off();

  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_zerozoffsets(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_zerozoffsets();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_unsetskipped(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbnavadjust_unsetskipped();
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}

/*--------------------------------------------------------------------*/

void do_action_invertnav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbna_status = MBNA_STATUS_NAVSOLVE;
  mbnavadjust_invertnav();
  mbna_status = MBNA_STATUS_GUI;
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_action_updategrids(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  mbna_status = MBNA_STATUS_NAVSOLVE;
  mbnavadjust_updategrid();
  mbna_status = MBNA_STATUS_GUI;
  do_update_status();
//  if (project.modelplot) {
//    do_update_modelplot_status();
//    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
//  }
  if (project.visualization_status) {
    mbnavadjust_reset_visualization_navties();
    do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/

void do_apply_nav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  mbnavadjust_applynav();
  do_update_status();
}

/*--------------------------------------------------------------------*/

void do_modelplot_show(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get drawingArea size */
  Dimension width;
  Dimension height;
  XtVaGetValues(drawingArea_modelplot, XmNwidth, &width, XmNheight, &height, NULL);
  mbna_modelplot_width = width;
  mbna_modelplot_height = height;
  modp_borders[0] = 0;
  modp_borders[1] = mbna_modelplot_width - 1;
  modp_borders[2] = 0;
  modp_borders[3] = mbna_modelplot_height - 1;

  /* Setup just the "canvas" part of the screen. */
  Window modp_xid = XtWindow(drawingArea_modelplot);

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
  project.modelplot = true;
  project.modelplot_uptodate = false;
  mbna_modelplot_zoom = false;
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  mbna_modelplot_start = 0;
  mbna_modelplot_end = 0;

  /* update status */
  do_update_status();
  if (project.modelplot) {
    do_update_modelplot_status();
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
  if (project.visualization_status)
    do_update_visualization_status();
}
/*--------------------------------------------------------------------*/

void do_modelplot_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* deallocate graphics */
  project.modelplot = false;
  XFreeGC(display, modp_gc);
  xg_free(modp_xgid);
}

/*--------------------------------------------------------------------*/

void do_modelplot_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)unused; // Unused parameter

  XConfigureEvent *cevent = (XConfigureEvent *)event;

  /* do this only if a resize event happens */
  if (cevent->type == ConfigureNotify) {
    /* get new shell size */
    Dimension width;
    Dimension height;
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
      Window modp_xid = XtWindow(drawingArea_modelplot);

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
      project.modelplot_uptodate = false;
    }
  }
}

/*--------------------------------------------------------------------*/

void do_modelplot_fullsize(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* replot model */
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
    mbna_modelplot_zoom = false;
    mbna_modelplot_start = 0;
    mbna_modelplot_end = 0;
  }
  else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
    mbna_modelplot_zoom = false;
    mbna_modelplot_start = 0;
    mbna_modelplot_end = 0;
  }
  else {
    mbna_modelplot_tiezoom = false;
    mbna_modelplot_tiestartzoom = 0;
    mbna_modelplot_tieendzoom = 0;
    mbna_block_select = MBNA_SELECT_NONE;
    mbna_block_select1 = MBNA_SELECT_NONE;
    mbna_block_select2 = MBNA_SELECT_NONE;
  }

  project.modelplot_uptodate = false;
  mbnavadjust_modelplot_setzoom();
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  do_update_modelplot_status();
}

/*--------------------------------------------------------------------*/

void do_modelplot_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter

  XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* If there is input in the drawing area */
  if (acs->reason == XmCR_INPUT) {
    XEvent *event = acs->event;

    /* Check for mouse pressed. */
    if (event->xany.type == ButtonPress) {
      /* If left mouse button is pushed */
      if (event->xbutton.button == 1) {
        button1down = true;

      } /* end of left button events */

      /* If middle mouse button is pushed */
      if (event->xbutton.button == 2) {
        button2down = true;
      } /* end of middle button events */

      /* If right mouse button is pushed */
      if (event->xbutton.button == 3) {
        button3down = true;
        mbna_modelplot_zoom_x1 = event->xbutton.x;
        mbna_modelplot_zoom_x2 = event->xbutton.x;

        /* replot model if zoom set */
        project.modelplot_uptodate = false;
        mbnavadjust_modelplot_plot(__FILE__, __LINE__);
      } /* end of right button events */
    } /* end of button press events */

    /* Check for mouse released. */
    if (event->xany.type == ButtonRelease) {
      /* If left mouse button is released */
      if (event->xbutton.button == 1) {
        button1down = false;

        /* pick nearest tie point */
        mbnavadjust_modelplot_pick(event->xbutton.x, event->xbutton.y);

        /* if a pick happened update everything */
        if (!project.modelplot_uptodate) {
          /* update model status */
          do_update_modelplot_status();

          /* update status */
          do_update_status();

          /* update model status */
          do_update_modelplot_status();

          /* replot model */
          mbnavadjust_modelplot_plot(__FILE__, __LINE__);

          /* update visualization */
          if (project.visualization_status)
            do_update_visualization_status();
        }
      }

      /* If middle mouse button is released */
      if (event->xbutton.button == 2) {
        button2down = false;

        /* pick nearest tie point */
        mbnavadjust_modelplot_middlepick(event->xbutton.x, event->xbutton.y);

        /* if a pick happened update everything */
        if (!project.modelplot_uptodate) {
          /* update model status */
          do_update_modelplot_status();

          /* update status */
          do_update_status();

          /* update model status */
          do_update_modelplot_status();

          /* replot model */
          mbnavadjust_modelplot_plot(__FILE__, __LINE__);

          /* update visualization */
          if (project.visualization_status)
            do_update_visualization_status();
        }
      }

      /* If right mouse button is released */
      if (event->xbutton.button == 3) {
        button3down = false;
        mbna_modelplot_zoom_x2 = event->xbutton.x;

        /* update model status */
        do_update_modelplot_status();

        /* replot model if zoom set */
        mbnavadjust_modelplot_setzoom();
        project.modelplot_uptodate = false;
        mbnavadjust_modelplot_plot(__FILE__, __LINE__);
        mbna_modelplot_zoom_x1 = 0;
        mbna_modelplot_zoom_x2 = 0;
      }

    } /* end of button release events */

    /* Check for mouse motion while pressed. */
    if (event->xany.type == MotionNotify) {
      /* If right mouse button is held during motion */
      if (button3down) {
        mbna_modelplot_zoom_x2 = event->xbutton.x;

        /* replot model */
        project.modelplot_uptodate = false;
        mbnavadjust_modelplot_plot(__FILE__, __LINE__);
      }
    }
  } /* end of inputs from window */
}

/*--------------------------------------------------------------------*/

void do_modelplot_expose(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* update model status */
  do_update_modelplot_status();

  /* replot the model */
  // project.modelplot_uptodate = false;
  mbnavadjust_modelplot_plot(__FILE__, __LINE__);
}

/*--------------------------------------------------------------------*/

void do_modelplot_tieoffsets(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  // fprintf(stderr,"Called do_modelplot_tieoffsets %d\n",XmToggleButtonGetState(toggleButton_modelplot_tieoffsets));

  if (XmToggleButtonGetState(toggleButton_modelplot_tieoffsets)) {
    project.modelplot_style = MBNA_MODELPLOT_TIEOFFSETS;
    project.modelplot_uptodate = false;

    /* update model status */
    do_update_modelplot_status();

    /* replot the model */
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
}

/*--------------------------------------------------------------------*/

void do_modelplot_perturbation(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  // fprintf(stderr,"Called do_modelplot_perturbation %d\n",XmToggleButtonGetState(toggleButton_modelplot_perturbation));

  if (XmToggleButtonGetState(toggleButton_modelplot_perturbation)) {
    project.modelplot_style = MBNA_MODELPLOT_PERTURBATION;
    project.modelplot_uptodate = false;

    /* update model status */
    do_update_modelplot_status();

    /* replot the model */
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
}

/*--------------------------------------------------------------------*/

void do_modelplot_timeseries(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  // fprintf(stderr,"Called do_modelplot_timeseries %d\n",XmToggleButtonGetState(toggleButton_modelplot_timeseries));

  if (XmToggleButtonGetState(toggleButton_modelplot_timeseries)) {
    project.modelplot_style = MBNA_MODELPLOT_TIMESERIES;
    project.modelplot_uptodate = false;

    /* update model status */
    do_update_modelplot_status();

    /* replot the model */
    mbnavadjust_modelplot_plot(__FILE__, __LINE__);
  }
}

/*--------------------------------------------------------------------*/

void do_modelplot_clearblock(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_modelplot_clearblock\n"); */

  mbnavadjust_modelplot_clearblock();

  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_naverr_update();
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
  (void)w; // Unused parameter
  (void)client_data; // Unused parameter
  (void)call_data; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  int grid_id = MBNA_GRID_PROJECT;
  if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
    grid_id = mbna_survey_select;
  mbnavadjust_open_visualization(grid_id);
}

/*--------------------------------------------------------------------*/

int do_visualize_dismiss_notify(size_t instance) {
  (void)instance; // Unused parameter

  const int status = mbnavadjust_dismiss_visualization();

  do_visualize_sensitivity();

  return (status);
}

/*--------------------------------------------------------------------*/

void do_visualize_sensitivity(void) {
  if (project.grid_status != MBNA_GRID_NONE && !project.visualization_status)
    XtVaSetValues(pushButton_visualize, XmNsensitive, True, NULL);
  else
    XtVaSetValues(pushButton_visualize, XmNsensitive, False, NULL);
}

/*--------------------------------------------------------------------*/

void do_pickroute_notify(size_t instance) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

  if (mbna_verbose > 0)
    fprintf(stderr, "do_pickroute_notify:%zu\n", instance);

  int error = MB_ERROR_NO_ERROR;
  struct mbview_shareddata_struct *shareddata;
  /* get selected route and translate to selected crossing */
  int status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);

  if (shareddata->route_selected != MBV_SELECT_NONE) {
    struct mbview_route_struct *route = &shareddata->routes[shareddata->route_selected];
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      int isurvey;
      int ifile;
      int isection;
      sscanf(route->name, "%d:%d:%d", &isurvey, &ifile, &isection);
      mbna_current_file = ifile;
      mbna_current_section = isection;
      mbna_file_select = ifile;
      mbna_section_select = isection;
      mbna_survey_select = project.files[mbna_file_select].block;
      mbna_file_id_2 = ifile;
      mbna_section_2 = isection;

      /* bring up naverr window if required */
      if (mbna_current_section != MBV_SELECT_NONE && mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }

      /* or replot the existing window */
      else if (mbna_current_section != MBV_SELECT_NONE) {
        mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
        do_update_status();
      }
    }
    else {
      int icrossing;
      int itie;
      int ifile1;
      int isection1;
      int isnav1;
      int ifile2;
      int isection2;
      int isnav2;
      sscanf(route->name, "%d:%d %d:%d:%d %d:%d:%d", &icrossing, &itie, &ifile1, &isection1, &isnav1, &ifile2, &isection2,
             &isnav2);

      mbnavadjust_visualization_selectcrossingfromroute(icrossing, itie);

      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }
      else {
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
        do_update_status();
      }
    }

    if (project.modelplot) {
      do_update_modelplot_status();
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    }
    if (project.visualization_status)
      do_update_visualization_status();

  }

  if (mbna_verbose > 0)
    fprintf(stderr, "return do_pickroute_notify status:%d\n", status);
}
/*--------------------------------------------------------------------*/

void do_picknav_notify(size_t instance) {
  /* print input debug statements */
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

  // int status = MB_SUCCESS;
  int error = MB_ERROR_NO_ERROR;
  struct mbview_shareddata_struct *shareddata;

  /* get shared data */
  int status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);
//fprintf(stderr, "*********************\n%s:%d:%s: mbna_naverr_mode: %d  Picks Prior: %d %d     New: %d %d\n",
//__FILE__, __LINE__, __FUNCTION__, mbna_naverr_mode,
//shareddata->nav_selected_mbnavadjust[0], shareddata->nav_selected_mbnavadjust[1],
//shareddata->nav_selected[0], shareddata->nav_selected[1]);

  /* if set to look at global ties then translate selected navigation to
     single section and open that in naverr with reference grid */
  if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
      || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
      || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {

    shareddata->nav_selected_mbnavadjust[0] = shareddata->nav_selected[0];
    shareddata->nav_selected_mbnavadjust[1] = shareddata->nav_selected[1];
    if (shareddata->nav_selected_mbnavadjust[0] != MBV_SELECT_NONE) {
      struct mbview_nav_struct *nav1 =
        (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[0]];
      int ifile1;
      int isection1;
      sscanf(nav1->name, "%d:%d", &ifile1, &isection1);
      mbna_current_file = ifile1;
      mbna_current_section = isection1;
      mbna_file_select = ifile1;
      mbna_section_select = isection1;
      mbna_survey_select = project.files[mbna_file_select].block;
      mbna_file_id_2 = ifile1;
      mbna_section_2 = isection1;
//fprintf(stderr, "%s:%d:%s: nav1->name:%s mbna_naverr_mode:%d mbna_current_file:%d mbna_current_section:%d\n",
//__FILE__, __LINE__, __FUNCTION__, nav1->name, mbna_naverr_mode, mbna_current_file, mbna_current_section);

      /* bring up naverr window if required */
      if (mbna_current_section != MBV_SELECT_NONE && mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }

      /* or replot the existing window */
      else if (mbna_current_section != MBV_SELECT_NONE) {
        mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
        do_update_status();
      }
      if (project.modelplot) {
        do_update_modelplot_status();
        mbnavadjust_modelplot_plot(__FILE__, __LINE__);
      }
      if (project.visualization_status)
        do_update_visualization_status();
    }
  }

  /* else if set to look at crossings get selected navigation and translate to selected crossing */
  else {

    shareddata->nav_selected_mbnavadjust[0] = shareddata->nav_selected[0];
    shareddata->nav_selected_mbnavadjust[1] = shareddata->nav_selected[1];
    if (shareddata->nav_selected_mbnavadjust[0] != MBV_SELECT_NONE &&
       shareddata->nav_selected_mbnavadjust[1] != MBV_SELECT_NONE) {
      struct mbview_nav_struct *nav1 =
        (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[0]];
      struct mbview_nav_struct *nav2 =
        (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[1]];
      int ifile1;
      int isection1;
      int ifile2;
      int isection2;
      sscanf(nav1->name, "%d:%d", &ifile1, &isection1);
      sscanf(nav2->name, "%d:%d", &ifile2, &isection2);
      status = mbnavadjust_visualization_selectcrossingfromnav(ifile1, isection1, ifile2, isection2);

      /* bring up naverr window if required */
      if (mbna_current_crossing != MBV_SELECT_NONE && mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }

      /* or replot the existing window */
      else if (mbna_current_crossing != MBV_SELECT_NONE) {
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
        do_update_status();
      }
      if (project.modelplot) {
        do_update_modelplot_status();
        mbnavadjust_modelplot_plot(__FILE__, __LINE__);
      }
      if (project.visualization_status)
        do_update_visualization_status();
    }
  }

  if (mbna_verbose > 0)
    fprintf(stderr, "return do_picknav_notify status:%d\n", status);
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void do_mbnavadjust_addcrossing(Widget w, XtPointer client, XtPointer call) {
  (void)w; // Unused parameter
  (void)client; // Unused parameter
  (void)call; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call;
  int error = MB_ERROR_NO_ERROR;
  struct mbview_shareddata_struct *shareddata;

  /* get shared data */
  status = mbview_getsharedptr(mbna_verbose, &shareddata, &error);


  if (mbna_current_crossing == MBNA_SELECT_NONE && shareddata->nav_selected_mbnavadjust[0] != MBNA_SELECT_NONE &&
      shareddata->nav_selected_mbnavadjust[1] != MBNA_SELECT_NONE &&
      shareddata->nav_selected_mbnavadjust[0] != shareddata->nav_selected_mbnavadjust[1]) {
    /* add and select a new crossing */
    struct mbview_nav_struct *nav1 =
      (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[0]];
    struct mbview_nav_struct *nav2 =
      (struct mbview_nav_struct *)&shareddata->navs[shareddata->nav_selected_mbnavadjust[1]];
    int ifile1;
    int isection1;
    int ifile2;
    int isection2;
    sscanf(nav1->name, "%d:%d", &ifile1, &isection1);
    sscanf(nav2->name, "%d:%d", &ifile2, &isection2);
    status = mbnavadjust_addcrossing(mbna_verbose, &project, ifile1, isection1, ifile2, isection2, &error);
    if (status == MB_SUCCESS)
      mbna_crossing_select = project.num_crossings - 1;
    else
      mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;

    /* bring up naverr window */
    if (status == MB_SUCCESS) {
      do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
      do_naverr_update();
      do_update_status();
    }
    if (project.modelplot) {
      project.modelplot_uptodate = false;
      do_update_modelplot_status();
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    }
    if (project.visualization_status)
      do_update_visualization_status();
  }
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void do_fileselection_list(Widget w, XtPointer client, XtPointer call) {
  (void)w; // Unused parameter
  (void)client; // Unused parameter
  (void)call; // Unused parameter

  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call;

  /* get selected text */
  mb_path string = "";
  get_text_string(fileSelectionBox_text, string);

  /* get output file */
  if ((int)strlen(string) > 0) {
    int error;
    char fileroot[MB_PATH_MAXLINE];
    int form;
    status = mb_get_format(mbna_verbose, string, fileroot, &form, &error);
    if (status == MB_SUCCESS) {
      format = form;
      char value_text[128];
      snprintf(value_text, sizeof(value_text), "%d", format);
      XmTextFieldSetString(textField_format, value_text);
    }
  }
}

/*--------------------------------------------------------------------*/

int do_wait_until_viewed(XtAppContext app) {
  app_context = app;

  /* find the top level shell */
  Widget topshell;
  for (topshell = scrolledWindow_datalist; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
    ;

  /* keep processing events until it is viewed */
  if (XtIsRealized(topshell)) {
    Window topwindow = XtWindow(topshell);

    XWindowAttributes xwa;
    XEvent event;

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
  if (mbna_verbose >= 1)
    fprintf(stderr, "%s\n", message);

  set_label_string(label_message, message);
  XtManageChild(bulletinBoard_message);

  /* force the label to be visible */
  Widget diashell;
  for (diashell = label_message; !XtIsShell(diashell); diashell = XtParent(diashell))
    ;

  Widget topshell;
  for (topshell = diashell; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
    ;

  if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
    Window diawindow = XtWindow(diashell);
    Window topwindow = XtWindow(topshell);

    XWindowAttributes xwa;
    XEvent event;

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
  int status = MB_SUCCESS;

  /* reposition to end of text */
  int pos = XmTextGetLastPosition(text_messages);
  XmTextSetInsertionPosition(text_messages, pos);

  /* add text */
  if (timetag)
    XmTextInsert(text_messages, pos, info);
  if (project.logfp != NULL)
    fputs(info, project.logfp);
  if (mbna_verbose > 0)
    fputs(info, stderr);

  /* put time tag in if requested */
  if (timetag) {
    int error = MB_ERROR_NO_ERROR;
    char user[256], host[256], date[32];
    status = mb_user_host_date(mbna_verbose, user, host, date, &error);
    mb_path tag;
    snprintf(tag, sizeof(tag), " > User <%s> on cpu <%s> at <%s>\n", user, host, date);
    pos = XmTextGetLastPosition(text_messages);
    XmTextSetInsertionPosition(text_messages, pos);
    XmTextInsert(text_messages, pos, tag);
    if (project.logfp != NULL)
      fputs(tag, project.logfp);
    if (mbna_verbose > 0)
      fputs(tag, stderr);
  }

  /* reposition to end of text */
  if (timetag) {
    pos = XmTextGetLastPosition(text_messages);
    XmTextShowPosition(text_messages, pos);
    XmTextSetInsertionPosition(text_messages, pos);
  }

  return (status);
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
void do_bell(int length) {
  XBell(display, length);
}

/*--------------------------------------------------------------------*/
/* Change label string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void set_label_string(Widget w, String str) {
  XmString xstr = XmStringCreateLocalized(str);
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
  Boolean argok;

  XmString xstr = (XtPointer)BX_CONVERT(w, str, XmRXmString, 0, &argok);
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
  char *str_tmp = (char *)XmTextGetString(w);
  strcpy(str, str_tmp);
  XtFree(str_tmp);
}

/*--------------------------------------------------------------------*/
