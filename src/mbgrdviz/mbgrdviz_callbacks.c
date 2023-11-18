/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_callbacks.c    10/9/2002
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

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef USE_UUID
#include <uuid.h>
#endif

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"
#include "mbsys_singlebeam.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include "mbgrdviz_creation.h"

#include "mbview.h"

#ifndef SANS
#define SANS "helvetica"
#endif
#ifndef SERIF
#define SERIF "times"
#endif
#ifndef MONO
#define MONO "courier"
#endif

/* fileSelectionBox modes */
#define MBGRDVIZ_OPENGRID 0
#define MBGRDVIZ_OPENOVERLAY 1
#define MBGRDVIZ_OPENSITE 2
#define MBGRDVIZ_OPENROUTE 3
#define MBGRDVIZ_OPENVECTOR 4
#define MBGRDVIZ_OPENNAV 5
#define MBGRDVIZ_OPENSWATH 6
#define MBGRDVIZ_SAVEROUTE 7
#define MBGRDVIZ_SAVEROUTEREVERSED 8
#define MBGRDVIZ_SAVERISISCRIPTHEADING 9
#define MBGRDVIZ_SAVERISISCRIPTNOHEADING 10
#define MBGRDVIZ_SAVEWINFROGPTS 11
#define MBGRDVIZ_SAVEWINFROGWPT 12
#define MBGRDVIZ_SAVEDEGDECMIN 13
#define MBGRDVIZ_SAVELNW 14
#define MBGRDVIZ_SAVEGREENSEAYML 15
#define MBGRDVIZ_SAVETECDISLST 16
#define MBGRDVIZ_SAVESITE 17
#define MBGRDVIZ_SAVESITEWPT 18
#define MBGRDVIZ_SAVEPROFILE 19
#define MBGRDVIZ_REALTIME 20

/* Projection defines */
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

/* Site and route file versions */
#define MBGRDVIZ_SITE_VERSION "1.00"
#define MBGRDVIZ_ROUTE_VERSION "1.00"
#define MBGRDVIZ_PROFILE_VERSION "1.00"
#define MBGRDVIZ_RISISCRIPT_VERSION "1.00"

/* Survey planning parameters */
#define MBGRDVIZ_SURVEY_MODE_UNIFORM 0
#define MBGRDVIZ_SURVEY_MODE_VARIABLE 1
#define MBGRDVIZ_SURVEY_PLATFORM_SURFACE 0
#define MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_ALTITUDE 1
#define MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_DEPTH 2
#define MBGRDVIZ_SURVEY_DIRECTION_SW 0
#define MBGRDVIZ_SURVEY_DIRECTION_SE 1
#define MBGRDVIZ_SURVEY_DIRECTION_NW 2
#define MBGRDVIZ_SURVEY_DIRECTION_NE 3
#define MBGRDVIZ_REALTIME_ICON_SHIP 0
#define MBGRDVIZ_REALTIME_ICON_ROV 1
#define MBGRDVIZ_REALTIME_ICON_AUV 2
#define MBGRDVIZ_REALTIME_OFF 0
#define MBGRDVIZ_REALTIME_ON 1
#define MBGRDVIZ_REALTIME_PAUSE 2
static int working_route = -1;
static int survey_instance = 0;
static int survey_mode = MBGRDVIZ_SURVEY_MODE_UNIFORM;
static int survey_platform = MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_ALTITUDE;
static int survey_interleaving = 1;
static int survey_direction = MBGRDVIZ_SURVEY_DIRECTION_SW;
static bool survey_crosslines_last = false;
static int survey_crosslines = 0;
static int survey_linespacing = 200;
static int survey_swathwidth = 120;
static int survey_depth = 0;
static int survey_altitude = 150;
static int survey_color = MBV_COLOR_BLACK;
static char survey_name[MB_PATH_MAXLINE];

static const char program_name[] = "MBgrdviz";

/* status variables */
static int verbose;
static int error;
static int pargc;
static char **pargv;

/* widgets */
static bool mbview_id[MBV_MAX_WINDOWS];
extern Widget mainWindow;
static Widget fileSelectionList;
static Widget fileSelectionText;

int do_mbgrdviz_init(int argc, char **argv, int verbosity);
void do_mbgrdviz_sensitivity(void);
int do_mbgrdviz_dismiss_notify(size_t instance);
void do_mbgrdviz_fileSelectionBox(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openoverlay(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openvector(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opensite(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opennav(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesite(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesitewpt(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisiscriptheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savewinfrogpts(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savewinfrogwpt(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savedegdecmin(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savelnw(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savetecdislst(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveprofile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_realtime(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_openfile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_close(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_quit(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbgrdviz_openprimary(char *input_file_ptr);
int do_mbgrdviz_openoverlay(size_t instance, char *input_file_ptr);
int do_mbgrdviz_opensite(size_t instance, char *input_file_ptr);
int do_mbgrdviz_opennav(size_t instance, bool swathbounds, char *input_file_ptr);
int do_mbgrdviz_openroute(size_t instance, char *input_file_ptr);
int do_mbgrdviz_openvector(size_t instance, char *input_file_ptr);
int do_mbgrdviz_savesite(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesitewpt(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveroute(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveroutereversed(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptnoheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savewinfrogpts(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savewinfrogwpt(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savedegdecmin(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savelnw(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savegreenseayml(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savetecdislst(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveprofile(size_t instance, char *output_file_ptr);
int do_mbgrdviz_readnav(size_t instance, char *swathfile, int pathstatus, char *pathraw, char *pathprocessed, int format,
                        int formatorg, double weight, int *error);
int do_mbgrdviz_readgrd(size_t instance, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue,
                        int *nxy, int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin,
                        double *ymax, double *dx, double *dy, float **data);
int do_mbgrdviz_opentest(size_t instance, double factor1, double factor2, double factor3, int *grid_projection_mode,
                         char *grid_projection_id, float *nodatavalue, int *nxy, int *n_columns, int *n_rows, double *min, double *max,
                         double *xmin, double *xmax, double *ymin, double *ymax, double *dx, double *dy, float **data);
void do_mbgrdviz_open_mbedit(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbnavedit(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbvelocitytool(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_region(Widget w, XtPointer client_data, XtPointer call_data);
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

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
void BxPopdownCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

Widget BxFindTopShell(Widget);
WidgetList BxWidgetIdsFromNames(Widget, char *, char *);

/*      Function Name:  BxExitCB
 *
 *      Description:     This functions expects an integer to be passed in
 *             client data.  It calls the exit() system call with
 *      the integer value as the argument to the function.
 *
 *      Arguments:      Widget    w:   the activating widget.
 *      XtPointer  client:  the integer exit value.
 *      XtPointer  call:  the call data (unused).
 */
void BxExitCB(Widget w, XtPointer client, XtPointer call) {
  (void)w;  // Unused parameter
  (void)client;  // Unused parameter
  (void)call;  // Unused parameter
  exit(EXIT_FAILURE);
}

/*      Function Name:   BxManageCB
 *
 *      Description:     Given a string of the form:
 *             "(WL)[widgetName, widgetName, ...]"
 *      BxManageCB attempts to convert the name to a Widget
 *      ID and manage the widget.
 *
 *      Arguments:       Widget      w:      the widget activating the callback.
 *             XtPointer   client: the list of widget names to attempt
 *              to find and manage.
 *             XtPointer   call:   the call data (unused).
 *
 *      Notes:        *  This function expects that there is an application
 *             shell from which all other widgets are descended.
 */
void BxManageCB(Widget w, XtPointer client, XtPointer call) {
  (void)call;  // Unused parameter

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

/*      Function Name:   BxUnmanageCB
 *
 *      Description:     Given a string of the form:
 *             "(WL)[widgetName, widgetName, ...]"
 *      BxUnmanageCB attempts to convert the name to a Widget
 *      ID and unmanage the widget.
 *
 *      Arguments:       Widget      w:      the widget activating the callback.
 *             XtPointer   client: the list of widget names to attempt
 *              to find and unmanage.
 *             XtPointer   call:   the call data (unused).
 *
 *      Notes:        *  This function expects that there is an application
 *             shell from which all other widgets are descended.
 */
void BxUnmanageCB(Widget w, XtPointer client, XtPointer call) {
  (void)call;  // Unused parameter

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
/*      Function Name:  BxPopdownCB
 *
 *      Description:     This function accepts a string of the form:
 *      "(WL)[widgetName, widgetName, ...]"
 *      It attempts to convert the widget names to Widget IDs
 *      and then popdown the widgets WITHOUT any grab.
 *
 *      Arguments:      Widget    w:  the activating widget.
 *      XtPointer  client:  the string of widget names to
 *            popup.
 *      XtPointer  call:  the call data (unused).
 *
 *      Notes:        * This function expects that there is an application
 *                      shell from which all other widgets are descended.
 *          * BxPopdownCB can only work on Shell widgets.  It will
 *      not work on other object types.  This is because
 *      popping down can only be done to a shell.  A check
 *      is made using XtIsShell() and an appropriate error
 *      is output if the passed object is not a Shell.
 */

/* ARGSUSED */
void BxPopdownCB(Widget w, XtPointer client, XtPointer call) {
  (void)call;  // Unused parameter

  /*
   * This function returns a NULL terminated WidgetList.  The memory for
   * the list needs to be freed when it is no longer needed.
   */
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxPopdownCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    if (XtIsShell(widgets[i])) {
      XtPopdown(widgets[i]);
    }
    else {
      printf("Callback Error (BxPopdownCB):\n\t\
Object %s is not a Shell\n",
             XtName(widgets[i]));
    }
    i++;
  }
  XtFree((char *)widgets);
}

/*      Function Name:  BxPopupCB
 *
 *      Description:     This function accepts a string of the form:
 *      "(WL)[widgetName, widgetName, ...]"
 *      It attempts to convert the widget names to Widget IDs
 *      and then popup the widgets WITHOUT any grab.
 *
 *      Arguments:      Widget    w:  the activating widget.
 *      XtPointer  client:  the string of widget names to
 *            popup.
 *      XtPointer  call:  the call data (unused).
 *
 *      Notes:        * This function expects that there is an application
 *                      shell from which all other widgets are descended.
 *          * BxPopupCB can only work on Shell widgets.  It will not
 *      work on other object types.  This is because popping up
 *      can only be done to a shell.  A check is made using
 *      XtIsShell() and an appropriate error is output if the
 *      passed object is not a Shell.
 */
void BxPopupCB(Widget w, XtPointer client, XtPointer call) {
  (void)call;  // Unused parameter

  /*
   * This function returns a NULL terminated WidgetList.  The memory for
   * the list needs to be freed when it is no longer needed.
   */
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxPopupCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    if (XtIsShell(widgets[i])) {
      XtPopup(widgets[i], XtGrabNone);
    }
    else {
      printf("Callback Error (BxPopupCB):\n\tObject %s is not a Shell\n",
             XtName(widgets[i]));
    }
    i++;
  }
  XtFree((char *)widgets);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_init(int argc, char **argv, int verbosity) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       argc:           %d\n", argc);
    fprintf(stderr, "dbg2       argv:\n");
    for (int i = 0; i < argc; i++)
      fprintf(stderr, "dbg2       argv[%d]:    %s\n", i, argv[i]);
    fprintf(stderr, "dbg2       verbosity:   %d\n", verbosity);
  }

  pargc = 1;
  pargv = argv;
  verbose = verbosity;
  error = MB_ERROR_NO_ERROR;

  /* set about version label */
  char value_text[MB_PATH_MAXLINE];
  sprintf(value_text, "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION, MB_VERSION_DATE);
  set_mbview_label_multiline_string(label_about_version, value_text);

  /* get additional widgets */
  fileSelectionList = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_LIST);
  fileSelectionText = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_TEXT);
  XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_HELP_BUTTON));

  /* set up survey planning widgets */

  /* set up line control */
  XmStringTable str_list = (XmStringTable)XtMalloc(2 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("Uniform");
  str_list[1] = XmStringCreateLocalized("Variable by Swath Width");
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNnumValues, 2);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_arearoute_linecontrol, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XtFree((XtPointer)str_list);

  /* set up platform type */
  str_list = (XmStringTable)XtMalloc(3 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("Surface Vessel");
  str_list[1] = XmStringCreateLocalized("Submerged - constant altitude");
  str_list[2] = XmStringCreateLocalized("Submerged - constant depth");
  ac = 0;
  XtSetArg(args[ac], XmNnumValues, 3);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_arearoute_platform, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XmStringFree(str_list[2]);
  XtFree((XtPointer)str_list);

  /* set up crosslinesfirstlast type */
  str_list = (XmStringTable)XtMalloc(2 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("Crosslines first");
  str_list[1] = XmStringCreateLocalized("Crosslines last");
  ac = 0;
  XtSetArg(args[ac], XmNnumValues, 2);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_arearoute_crosslinesfirstlast, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XtFree((XtPointer)str_list);

  /* set up survey direction */
  str_list = (XmStringTable)XtMalloc(4 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("SW");
  str_list[1] = XmStringCreateLocalized("SE");
  str_list[2] = XmStringCreateLocalized("NW");
  str_list[3] = XmStringCreateLocalized("NE");
  ac = 0;
  XtSetArg(args[ac], XmNnumValues, 4);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_arearoute_direction, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XmStringFree(str_list[2]);
  XmStringFree(str_list[3]);
  XtFree((XtPointer)str_list);

  /* set up survey color */
  str_list = (XmStringTable)XtMalloc(6 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("Black");
  str_list[1] = XmStringCreateLocalized("Yellow");
  str_list[2] = XmStringCreateLocalized("Green");
  str_list[3] = XmStringCreateLocalized("Bluegreen");
  str_list[4] = XmStringCreateLocalized("Blue");
  str_list[5] = XmStringCreateLocalized("Purple");
  ac = 0;
  XtSetArg(args[ac], XmNnumValues, 6);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_arearoute_color, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XmStringFree(str_list[2]);
  XmStringFree(str_list[3]);
  XmStringFree(str_list[4]);
  XmStringFree(str_list[5]);
  XtFree((XtPointer)str_list);

  /* set up realtime control widgets */

  /* set up realtime display icon */
  str_list = (XmStringTable)XtMalloc(3 * sizeof(XmString *));
  str_list[0] = XmStringCreateLocalized("Ship");
  str_list[1] = XmStringCreateLocalized("ROV");
  str_list[2] = XmStringCreateLocalized("AUV");
  ac = 0;
  XtSetArg(args[ac], XmNnumValues, 3);
  ac++;
  XtSetArg(args[ac], XmNvalues, str_list);
  ac++;
  XtSetValues(spinText_realtimesetup_icon, args, ac);
  XmStringFree(str_list[0]);
  XmStringFree(str_list[1]);
  XtFree((XtPointer)str_list);

  /* initialize mbview_id list */
  for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
    mbview_id[i] = false;
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  return (0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_sensitivity() {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* fprintf(stderr,"do_mbgrdviz_sensitivity called\n");*/
  /* set file opening menu items only if an mbview instance is active */
  bool mbview_active = false;
  bool mbview_allactive = true;
  size_t instance = MBV_NO_WINDOW;
  for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (mbview_id[i]) {
      mbview_active = true;
      if (instance == MBV_NO_WINDOW)
        instance = i;
    }
    else
      mbview_allactive = false;
  }

  Cardinal ac = 0;
  Arg args[256];

  /* set file opening menu item only if not all mbview instances are active */
  if (mbview_allactive != true) {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
  }
  else {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, False);
    ac++;
  }
  XtSetValues(pushButton_file_openprimary, args, ac);

  /* set other file opening menu items only if an mbview instance is active */
  if (mbview_active) {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
  }
  else {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, False);
    ac++;
  }
  XtSetValues(pushButton_opensite, args, ac);
  XtSetValues(pushButton_openroute, args, ac);
  XtSetValues(pushButton_opennav, args, ac);
  XtSetValues(pushButton_openswath, args, ac);
  XtSetValues(pushButton_openvector, args, ac);

  int nsite;
  mbview_getsitecount(verbose, instance, &nsite, &error);
  if (mbview_active && nsite > 0) {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
  }
  else {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, False);
    ac++;
  }
  XtSetValues(pushButton_savesite, args, ac);

  int nroute;
  mbview_getroutecount(verbose, instance, &nroute, &error);
  if (mbview_active && nroute > 0) {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
  }
  else {
    ac = 0;
    XtSetArg(args[ac], XmNsensitive, False);
    ac++;
  }
  XtSetValues(pushButton_saveroute, args, ac);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* figure out what kind of file is to be opened */
  const size_t actionid = (size_t)client_data;
  // const size_t mode = actionid / MBV_MAX_WINDOWS;
  // const size_t instance = actionid - mode * MBV_MAX_WINDOWS;

  /* set title to open primary grid */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open GMT Grid File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* open primary grid */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.grd", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;

  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_openoverlay(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* get instance */
  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open Overlay GMT Grid File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.grd", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENOVERLAY * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_opensite(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open Site File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.ste", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENSITE * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_openroute(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open Route File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.rte", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENROUTE * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_opennav(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  ac = 0;
  XtSetArg(args[ac], XmNtitle, "Open Navigation Datalist File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.mb-1", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENNAV * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open Swath Datalist File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.mb-1", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENSWATH * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_openvector(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Open Vector File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_OPENVECTOR * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savesite(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Site File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVESITE * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savesitewpt(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Sites as Winfrog WPT File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVESITEWPT * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saveroute(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEROUTE * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saveroutereversed(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEROUTEREVERSED * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saverisiscriptheading(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Risi Script File (heading varies)");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVERISISCRIPTHEADING * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Risi Script File (heading static)");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVERISISCRIPTNOHEADING * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savewinfrogpts(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as Winfrog PTS File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEWINFROGPTS * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savewinfrogwpt(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as Winfrog WPT File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEWINFROGWPT * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savedegdecmin(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as Degrees + Decimal Minutes File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEDEGDECMIN * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savelnw(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as Hypack LNW File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVELNW * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savegreenseayml(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as Greensea YML File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEGREENSEAYML * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savetecdislst(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Route as TECDIS LST File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVETECDISLST * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saveprofile(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = (size_t)client_data;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Save Profile File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEPROFILE * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_realtime(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  const size_t instance = 0;

  /* set title to open file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, "Set Realtime Navigation Source");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_REALTIME * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_close(Widget w, XtPointer client_data, XtPointer call_data) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_quit(Widget w, XtPointer client_data, XtPointer call_data) {

  /* close any active mbview instances */
  // const int status =
  mbview_quit(verbose, &error);

  XtUnmanageChild(XtParent(mainWindow));

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_dismiss_notify(size_t instance) {
  // int verbose = 0;
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:  %zu\n", instance);
  }

  /* set mbview window <id> to inactive */
  if (instance != MBV_NO_WINDOW && instance < MBV_MAX_WINDOWS && mbview_id[instance]) {
    mbview_id[instance] = false;
    /* fprintf(stderr, "Freeing mbview window %d in local list...\n",
            instance); */
  }
  else {
    /* fprintf(stderr, "Unable to free mbview - mbview window %d not found in local list...\n",
            instance); */
  }

  /* update widgets of remaining mbview windows */
  int status = MB_SUCCESS;
  for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  return (status);
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_openfile(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  char *file_ptr = NULL;
  XmFileSelectionBoxCallbackStruct *acs = (XmFileSelectionBoxCallbackStruct *)call_data;

  /* read the input file name */
  XmStringGetLtoR(acs->value, XmSTRING_DEFAULT_CHARSET, &file_ptr);
  if (strlen(file_ptr) <= 0 && file_ptr != NULL) {
    XtFree(file_ptr);
    file_ptr = NULL;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg4  Extracted input file name from call_data:\n");
    fprintf(stderr, "dbg4       call_data:           %p\n", call_data);
    fprintf(stderr, "dbg4       acs:                 %p\n", acs);
    fprintf(stderr, "dbg4       acs->value:          %p\n", acs->value);
    fprintf(stderr, "dbg4       file_ptr:            %p\n", file_ptr);
    fprintf(stderr, "dbg4       file_ptr:            %s\n", file_ptr);
  }

  /* figure out what kind of file is to be opened */
  Cardinal ac = 0;
  Arg args[256];
  size_t actionid;
  XtSetArg(args[ac], XmNuserData, (XtPointer)&actionid);
  ac++;
  XtGetValues(fileSelectionBox, args, ac);

  size_t mode = actionid / MBV_MAX_WINDOWS;
  size_t instance;
  if (mode > 0)
    instance = actionid - mode * MBV_MAX_WINDOWS;
  else
    instance = 0;

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg4  Extracted user data from widget fileSelectionBox:\n");
    fprintf(stderr, "dbg4       fileSelectionBox:    %p\n", fileSelectionBox);
    fprintf(stderr, "dbg4       actionid:            %zu\n", actionid);
    fprintf(stderr, "dbg4       mode:                %zu\n", mode);
  }

  /* open primary grid */
  // int status = MB_SUCCESS;
  if (mode <= MBGRDVIZ_OPENGRID) {
    /* read the grid and open mbview window */
    /* status = */ do_mbgrdviz_openprimary(file_ptr);
  }

  /* else open overlay grid */
  else if (mode == MBGRDVIZ_OPENOVERLAY) {
    /* read the grid and update mbview window */
    do_mbview_message_on("Reading overlay grid...", instance);
    /* status = */ do_mbgrdviz_openoverlay(instance, file_ptr);
  }

  /* else open site data */
  else if (mode == MBGRDVIZ_OPENSITE) {
    /* read site file and update mbview window */
    do_mbview_message_on("Reading site data...", instance);
    /* status = */ do_mbgrdviz_opensite(instance, file_ptr);
  }

  /* else open route data */
  else if (mode == MBGRDVIZ_OPENROUTE) {
    /* read route file and update mbview window */
    do_mbview_message_on("Reading route data...", instance);
    /* status = */ do_mbgrdviz_openroute(instance, file_ptr);
  }

  /* else open nav data */
  else if (mode == MBGRDVIZ_OPENNAV) {
    /* read nav file and update mbview window */
    do_mbview_message_on("Reading navigation data...", instance);
    /* status = */ do_mbgrdviz_opennav(instance, false, file_ptr);
  }

  /* else open swath data */
  else if (mode == MBGRDVIZ_OPENSWATH) {
    /* read nav file and update mbview window */
    do_mbview_message_on("Reading swath data...", instance);
    /* status = */ do_mbgrdviz_opennav(instance, true, file_ptr);
  }

  /* else open vector data */
  else if (mode == MBGRDVIZ_OPENVECTOR) {
    /* read vector file and update mbview window */
    do_mbview_message_on("Reading vector data...", instance);
    /* status = */ do_mbgrdviz_openvector(instance, file_ptr);
  }

  /* else write site data */
  else if (mode == MBGRDVIZ_SAVESITE) {
    /* write site file */
    do_mbview_message_on("Saving site data...", instance);
    /* status = */ do_mbgrdviz_savesite(instance, file_ptr);
  }

  /* else write site data */
  else if (mode == MBGRDVIZ_SAVESITEWPT) {
    /* write site file */
    do_mbview_message_on("Saving site data...", instance);
    /* status = */ do_mbgrdviz_savesitewpt(instance, file_ptr);
  }

  /* else write route data */
  else if (mode == MBGRDVIZ_SAVEROUTE) {
    /* write route file */
    do_mbview_message_on("Saving route data...", instance);
    /* status = */ do_mbgrdviz_saveroute(instance, file_ptr);
  }

  /* else write route data reversed */
  else if (mode == MBGRDVIZ_SAVEROUTEREVERSED) {
    /* write route file */
    do_mbview_message_on("Saving reversed route data...", instance);
    /* status = */ do_mbgrdviz_saveroutereversed(instance, file_ptr);
  }

  /* else write route data as Risi script with variable heading */
  else if (mode == MBGRDVIZ_SAVERISISCRIPTHEADING) {
    /* write route file */
    do_mbview_message_on("Saving route as Risi script with variable heading...", instance);
    /* status = */ do_mbgrdviz_saverisiscriptheading(instance, file_ptr);
  }

  /* else write route data as Risi script with static heading */
  else if (mode == MBGRDVIZ_SAVERISISCRIPTNOHEADING) {
    /* write route file */
    do_mbview_message_on("Saving route as Risi script with static heading...", instance);
    /* status = */ do_mbgrdviz_saverisiscriptnoheading(instance, file_ptr);
  }

  /* else write route data as Winfrog pts file */
  else if (mode == MBGRDVIZ_SAVEWINFROGPTS) {
    /* write route file */
    do_mbview_message_on("Saving route as Winfrog PTS file...", instance);
    /* status = */ do_mbgrdviz_savewinfrogpts(instance, file_ptr);
  }

  /* else write route data as Winfrog wpt file */
  else if (mode == MBGRDVIZ_SAVEWINFROGWPT) {
    /* write route file */
    do_mbview_message_on("Saving route as Winfrog WPT file...", instance);
    /* status = */ do_mbgrdviz_savewinfrogwpt(instance, file_ptr);
  }

  /* else write route data as degrees decimal minutes file */
  else if (mode == MBGRDVIZ_SAVEDEGDECMIN) {
    /* write route file */
    do_mbview_message_on("Saving route as degrees + decimal minutes file...", instance);
    /* status = */ do_mbgrdviz_savedegdecmin(instance, file_ptr);
  }

  /* else write route data as Hypack lnw file */
  else if (mode == MBGRDVIZ_SAVELNW) {
    /* write route file */
    do_mbview_message_on("Saving route as Hypack LNW file...", instance);
    /* status = */ do_mbgrdviz_savelnw(instance, file_ptr);
  }

  /* else write route data as Hypack lnw file */
  else if (mode == MBGRDVIZ_SAVEGREENSEAYML) {
    /* write route file */
    do_mbview_message_on("Saving route as Greensea YML file...", instance);
    /* status = */ do_mbgrdviz_savegreenseayml(instance, file_ptr);
  }

  /* else write route data as TECDIS lst file */
  else if (mode == MBGRDVIZ_SAVETECDISLST) {
    /* write route file */
    do_mbview_message_on("Saving route as TECDIS LST file...", instance);
    /* status = */ do_mbgrdviz_savetecdislst(instance, file_ptr);
  }

  /* else write route data */
  else if (mode == MBGRDVIZ_SAVEPROFILE) {
    /* write route file */
    do_mbview_message_on("Saving profile data...", instance);
    /* status = */ do_mbgrdviz_saveprofile(instance, file_ptr);
  }

  /* else set realtime data source */
  else if (mode == MBGRDVIZ_REALTIME) {
    /* Set realtime source path */
    XmTextSetString(textField_realtimesetup_path, file_ptr);
  }

  /* free the string */
  if (file_ptr != NULL) {
    XtFree(file_ptr);
    file_ptr = NULL;
  }

  /* close the message */
  if (mode > MBGRDVIZ_OPENGRID && mode != MBGRDVIZ_REALTIME)
    do_mbview_message_off(instance);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_openprimary(char *input_file_ptr) {
  int status = MB_SUCCESS;
  char *button_name_ptr;
  size_t instance;
  char *testname = "Internal Test Grid";
  int projectionid, utmzone;
  double reference_lon;

  /* mbview parameters */
  char mbv_title[MB_PATH_MAXLINE];
  int mbv_xo;
  int mbv_yo;
  int mbv_width;
  int mbv_height;
  int mbv_lorez_dimension;
  int mbv_hirez_dimension;
  int mbv_lorez_navdecimate;
  int mbv_hirez_navdecimate;
  int mbv_display_mode;
  int mbv_mouse_mode;
  int mbv_grid_mode;
  int mbv_primary_histogram;
  int mbv_primaryslope_histogram;
  int mbv_secondary_histogram;
  int mbv_primary_shade_mode;
  int mbv_slope_shade_mode;
  int mbv_secondary_shade_mode;
  int mbv_grid_contour_mode;
  int mbv_site_view_mode;
  int mbv_route_view_mode;
  int mbv_nav_view_mode;
  int mbv_navdrape_view_mode;
  int mbv_vector_view_mode;
  int mbv_primary_colortable;
  int mbv_primary_colortable_mode;
  double mbv_primary_colortable_min;
  double mbv_primary_colortable_max;
  int mbv_slope_colortable;
  int mbv_slope_colortable_mode;
  double mbv_slope_colortable_min;
  double mbv_slope_colortable_max;
  int mbv_secondary_colortable;
  int mbv_secondary_colortable_mode;
  double mbv_exageration;
  double mbv_modelelevation3d;
  double mbv_modelazimuth3d;
  double mbv_viewelevation3d;
  double mbv_viewazimuth3d;
  double mbv_illuminate_magnitude;
  double mbv_illuminate_elevation;
  double mbv_illuminate_azimuth;
  double mbv_slope_magnitude;
  double mbv_overlay_shade_magnitude;
  double mbv_overlay_shade_center;
  int mbv_overlay_shade_mode;
  double mbv_contour_interval;
  int mbv_primary_grid_projection_mode = MBV_PROJECTION_PROJECTED;
  char mbv_primary_grid_projection_id[MB_PATH_MAXLINE];
  int mbv_display_projection_mode;
  char mbv_display_projection_id[MB_PATH_MAXLINE];
  float mbv_primary_nodatavalue;
  int mbv_primary_nxy;
  int mbv_primary_n_columns;
  int mbv_primary_n_rows;
  double mbv_primary_min;
  double mbv_primary_max;
  double mbv_primary_xmin;
  double mbv_primary_xmax;
  double mbv_primary_ymin;
  double mbv_primary_ymax;
  double mbv_primary_dx;
  double mbv_primary_dy;
  float *mbv_primary_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* get next instance number */
  status = mbview_init(verbose, &instance, &error);
  if (instance == MBV_NO_WINDOW) {
    fprintf(stderr, "Unable to create mbview - %d mbview windows already created\n", MBV_MAX_WINDOWS);
  }

  else {
    /* get button name */
    if (input_file_ptr != NULL) {
#ifdef WIN32
      button_name_ptr = strrchr(input_file_ptr, '/'); /* This one should work for gcc as well */
#else
      button_name_ptr = (char *)rindex(input_file_ptr, '/');
#endif
      if (button_name_ptr == NULL)
        button_name_ptr = input_file_ptr;
      else
        button_name_ptr++;
    }
    else {
      button_name_ptr = testname;
    }

    /* set parameters */
    sprintf(mbv_title, "MBgrdviz: %s\n", button_name_ptr);
    mbv_xo = 200;
    mbv_yo = 200;
    mbv_width = 560;
    mbv_height = 500;
    mbv_lorez_dimension = 100;
    mbv_hirez_dimension = 500;
    mbv_lorez_navdecimate = 5;
    mbv_hirez_navdecimate = 1;

    /* set basic mbview window parameters */
    status = mbview_setwindowparms(verbose, instance, &do_mbgrdviz_dismiss_notify, mbv_title, mbv_xo, mbv_yo, mbv_width,
                                   mbv_height, mbv_lorez_dimension, mbv_hirez_dimension, mbv_lorez_navdecimate,
                                   mbv_hirez_navdecimate, &error);

    /* read in the grd file */
    if (status == MB_SUCCESS && input_file_ptr != NULL)
      status = mb_read_gmt_grd(verbose, input_file_ptr, &mbv_primary_grid_projection_mode, mbv_primary_grid_projection_id,
                               &mbv_primary_nodatavalue, &mbv_primary_nxy, &mbv_primary_n_columns, &mbv_primary_n_rows,
                               &mbv_primary_min, &mbv_primary_max, &mbv_primary_xmin, &mbv_primary_xmax, &mbv_primary_ymin,
                               &mbv_primary_ymax, &mbv_primary_dx, &mbv_primary_dy, &mbv_primary_data, NULL, NULL, &error);

    else if (status == MB_SUCCESS)
      status =
          do_mbgrdviz_opentest(instance, 1000.0, 3.0, 2.0, &mbv_primary_grid_projection_mode,
                               mbv_primary_grid_projection_id, &mbv_primary_nodatavalue, &mbv_primary_nxy, &mbv_primary_n_columns,
                               &mbv_primary_n_rows, &mbv_primary_min, &mbv_primary_max, &mbv_primary_xmin, &mbv_primary_xmax,
                               &mbv_primary_ymin, &mbv_primary_ymax, &mbv_primary_dx, &mbv_primary_dy, &mbv_primary_data);

    /* set parameters */
    if (status == MB_SUCCESS) {
      mbv_display_mode = MBV_DISPLAY_2D;
      mbv_mouse_mode = MBV_MOUSE_MOVE;
      mbv_grid_mode = MBV_GRID_VIEW_PRIMARY;
      mbv_primary_histogram = false;
      mbv_primaryslope_histogram = false;
      mbv_secondary_histogram = false;
      mbv_primary_shade_mode = MBV_SHADE_VIEW_SLOPE;
      mbv_slope_shade_mode = MBV_SHADE_VIEW_NONE;
      mbv_secondary_shade_mode = MBV_SHADE_VIEW_NONE;
      mbv_grid_contour_mode = MBV_VIEW_OFF;
      mbv_site_view_mode = MBV_VIEW_OFF;
      mbv_route_view_mode = MBV_VIEW_OFF;
      mbv_nav_view_mode = MBV_VIEW_OFF;
      mbv_navdrape_view_mode = MBV_VIEW_OFF;
      mbv_vector_view_mode = MBV_VIEW_OFF;
      mbv_primary_colortable = MBV_COLORTABLE_HAXBY;
      mbv_primary_colortable_mode = MBV_COLORTABLE_NORMAL;
      mbv_primary_colortable_min = mbv_primary_min;
      mbv_primary_colortable_max = mbv_primary_max;
      mbv_slope_colortable = MBV_COLORTABLE_HAXBY;
      mbv_slope_colortable_mode = MBV_COLORTABLE_REVERSED;
      mbv_slope_colortable_min = 0.0;
      mbv_slope_colortable_max = 0.5;
      mbv_secondary_colortable = MBV_COLORTABLE_HAXBY;
      mbv_secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
      // double mbv_secondary_colortable_min = 0.0;
      // double mbv_secondary_colortable_max = 0.0;
      mbv_exageration = 1.0;
      mbv_modelelevation3d = 90.0;
      mbv_modelazimuth3d = 0.0;
      mbv_viewelevation3d = 90.0;
      mbv_viewazimuth3d = 0.0;
      mbv_illuminate_magnitude = 1.0;
      mbv_illuminate_elevation = 5.0;
      mbv_illuminate_azimuth = 90.0;
      mbv_slope_magnitude = 1.0;
      mbv_overlay_shade_magnitude = 1.0;
      mbv_overlay_shade_center = 0.0;
      mbv_overlay_shade_mode = MBV_COLORTABLE_NORMAL;
      mbv_contour_interval = pow(10.0, floor(log10(mbv_primary_max - mbv_primary_min)) - 1.0);

      /* set mbview default values */
      status = mb_mbview_defaults(verbose, &mbv_primary_colortable, &mbv_primary_colortable_mode, &mbv_primary_shade_mode,
                                  &mbv_slope_colortable, &mbv_slope_colortable_mode, &mbv_secondary_colortable,
                                  &mbv_secondary_colortable_mode, &mbv_illuminate_magnitude, &mbv_illuminate_elevation,
                                  &mbv_illuminate_azimuth, &mbv_slope_magnitude);
    }

    /* set the display projection */
    if (status == MB_SUCCESS) {
      /* if grid projected then use the same projected coordinate system by default */
      if (mbv_primary_grid_projection_mode == MBV_PROJECTION_PROJECTED) {
        mbv_display_projection_mode = mbv_primary_grid_projection_mode;
        strcpy(mbv_display_projection_id, mbv_primary_grid_projection_id);
      }

      /* else if grid geographic and covers much of the world use spheroid */
      else if (mbv_primary_xmax - mbv_primary_xmin > 20.0 || mbv_primary_ymax - mbv_primary_ymin > 20.0) {
        mbv_display_projection_mode = MBV_PROJECTION_SPHEROID;
        sprintf(mbv_display_projection_id, "SPHEROID");
      }

      /* else if grid geographic then use appropriate UTM zone for non-polar grids */
      else if (mbv_primary_ymax > -80.0 && mbv_primary_ymin < 84.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        reference_lon = 0.5 * (mbv_primary_xmin + mbv_primary_xmax);
        if (reference_lon > 180.0)
          reference_lon -= 360.0;
        utmzone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
        if (0.5 * (mbv_primary_ymin + mbv_primary_ymax) >= 0.0)
          projectionid = 32600 + utmzone;
        else
          projectionid = 32700 + utmzone;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more northerly than 84 deg N then use
              North Universal Polar Stereographic Projection */
      else if (mbv_primary_ymin > 84.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32661;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more southerly than 80 deg S then use
              South Universal Polar Stereographic Projection */
      else if (mbv_primary_ymax < 80.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32761;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else just use geographic */
      else {
        mbv_display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
        sprintf(mbv_display_projection_id, "EPSG:%d", GCS_WGS_84);
      }
    }

    if (mbv_primary_grid_projection_mode != MBV_PROJECTION_PROJECTED) {
      double mtodeglon, mtodeglat;
      mb_coor_scale(verbose, 0.5 * (mbv_primary_ymin + mbv_primary_ymax), &mtodeglon, &mtodeglat);
      fprintf(stderr, "Geographic grid bounds: %f %f %f %f   Longitude scaling: %.8f m/deg  Latitude scaling: %.8f m/deg\n",
          mbv_primary_xmin, mbv_primary_xmax, mbv_primary_ymin, mbv_primary_ymax, mtodeglon, mtodeglat);
    }

    /* set basic mbview view controls */
    if (status == MB_SUCCESS)
      status = mbview_setviewcontrols(
          verbose, instance, mbv_display_mode, mbv_mouse_mode, mbv_grid_mode, mbv_primary_histogram,
          mbv_primaryslope_histogram, mbv_secondary_histogram, mbv_primary_shade_mode, mbv_slope_shade_mode,
          mbv_secondary_shade_mode, mbv_grid_contour_mode, mbv_site_view_mode, mbv_route_view_mode, mbv_nav_view_mode,
          mbv_navdrape_view_mode, mbv_vector_view_mode, mbv_exageration, mbv_modelelevation3d, mbv_modelazimuth3d,
          mbv_viewelevation3d, mbv_viewazimuth3d, mbv_illuminate_magnitude, mbv_illuminate_elevation,
          mbv_illuminate_azimuth, mbv_slope_magnitude, mbv_overlay_shade_magnitude, mbv_overlay_shade_center,
          mbv_overlay_shade_mode, mbv_contour_interval, mbv_display_projection_mode, mbv_display_projection_id, &error);

    /* set primary grid data */
    if (status == MB_SUCCESS)
      status = mbview_setprimarygrid(verbose, instance, mbv_primary_grid_projection_mode, mbv_primary_grid_projection_id,
                                     mbv_primary_nodatavalue, mbv_primary_n_columns, mbv_primary_n_rows, mbv_primary_min,
                                     mbv_primary_max, mbv_primary_xmin, mbv_primary_xmax, mbv_primary_ymin,
                                     mbv_primary_ymax, mbv_primary_dx, mbv_primary_dy, mbv_primary_data, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&mbv_primary_data, &error);

    /* set more mbview control values */
    if (status == MB_SUCCESS)
      status = mbview_setprimarycolortable(verbose, instance, mbv_primary_colortable, mbv_primary_colortable_mode,
                                           mbv_primary_colortable_min, mbv_primary_colortable_max, &error);
    if (status == MB_SUCCESS)
      status = mbview_setslopecolortable(verbose, instance, mbv_slope_colortable, mbv_slope_colortable_mode,
                                         mbv_slope_colortable_min, mbv_slope_colortable_max, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableeditsites(verbose, instance, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableeditroutes(verbose, instance, &error);

    /* open up mbview window */
    if (status == MB_SUCCESS) {
      status = mbview_open(verbose, instance, &error);
      if (status == MB_SUCCESS)
        mbview_id[instance] = true;
      else
        mbview_id[instance] = false;

      /* set sensitivity callback routine */
      if (status == MB_SUCCESS) {
        mbview_setsensitivitynotify(verbose, instance, do_mbgrdviz_sensitivity, &error);
      }

      /* add action button */
      if (status == MB_SUCCESS) {
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openoverlay, "Open Overlay Grid",
                         MBV_PICKMASK_NONE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opensite, "Open Site File", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openroute, "Open Route File", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opennav, "Open Navigation", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openswath, "Open Swath Data", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openvector, "Open Vector File",
                         MBV_PICKMASK_NONE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savesite, "Save Site File", MBV_EXISTMASK_SITE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savesitewpt, "Save Sites as Winfrog WPT File", MBV_EXISTMASK_SITE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveroute, "Save Route File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveroutereversed, "Save Route File Reversed",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptheading, "Save Risi Script File (variable heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading, "Save Risi Script File (static heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savewinfrogpts, "Save Route as Winfrog PTS File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savewinfrogwpt, "Save Route as Winfrog WPT File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savedegdecmin,
                         "Save Route as Degrees + Decimal Minutes File", MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savelnw, "Save Route as Hypack LNW File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savegreenseayml, "Save Route as Greensea YML File",
                          MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savetecdislst, "Save Route as TECDIS LST File",
                          MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveprofile, "Save Profile File",
                         MBV_PICKMASK_TWOPOINT + MBV_PICKMASK_ROUTE + MBV_PICKMASK_NAVTWOPOINT, &error);

        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbedit, "Open Selected Nav in MBedit", MBV_PICKMASK_NAVANY,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbeditviz, "Open Selected Nav in MBeditviz",
                         MBV_PICKMASK_NAVANY, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbnavedit, "Open Selected Nav in MBnavedit",
                         MBV_PICKMASK_NAVANY, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbvelocitytool, "Open Selected Nav in MBvelocitytool",
                         MBV_PICKMASK_NAVANY, &error);

        mbview_addaction(verbose, instance, do_mbgrdviz_open_region, "Open Region as New View",
                         MBV_PICKMASK_REGION + MBV_PICKMASK_NEWINSTANCE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_make_survey, "Generate Survey Route from Area", MBV_PICKMASK_AREA,
                         &error);
      }
    }
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_openoverlay(size_t instance, char *input_file_ptr) {
  int status = MB_SUCCESS;

  /* mbview parameters */
  int mbv_secondary_colortable;
  int mbv_secondary_colortable_mode;
  double mbv_secondary_colortable_min;
  double mbv_secondary_colortable_max;
  int mbv_secondary_grid_projection_mode;
  char mbv_secondary_grid_projection_id[MB_PATH_MAXLINE];
  float mbv_secondary_nodatavalue;
  int mbv_secondary_nxy;
  int mbv_secondary_n_columns;
  int mbv_secondary_n_rows;
  double mbv_secondary_min;
  double mbv_secondary_max;
  double mbv_secondary_xmin;
  double mbv_secondary_xmax;
  double mbv_secondary_ymin;
  double mbv_secondary_ymax;
  double mbv_secondary_dx;
  double mbv_secondary_dy;
  float *mbv_secondary_data;
  double mbv_overlay_shade_magnitude;
  double mbv_overlay_shade_center;
  int mbv_overlay_shade_mode;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* read in the grd file */
    if (status == MB_SUCCESS && input_file_ptr != NULL)
      status = mb_read_gmt_grd(verbose, input_file_ptr, &mbv_secondary_grid_projection_mode,
                               mbv_secondary_grid_projection_id, &mbv_secondary_nodatavalue, &mbv_secondary_nxy,
                               &mbv_secondary_n_columns, &mbv_secondary_n_rows, &mbv_secondary_min, &mbv_secondary_max,
                               &mbv_secondary_xmin, &mbv_secondary_xmax, &mbv_secondary_ymin, &mbv_secondary_ymax,
                               &mbv_secondary_dx, &mbv_secondary_dy, &mbv_secondary_data, NULL, NULL, &error);

    else if (status == MB_SUCCESS)
      status = do_mbgrdviz_opentest(instance, 1000.0, 6.0, 1.5, &mbv_secondary_grid_projection_mode,
                                    mbv_secondary_grid_projection_id, &mbv_secondary_nodatavalue, &mbv_secondary_nxy,
                                    &mbv_secondary_n_columns, &mbv_secondary_n_rows, &mbv_secondary_min, &mbv_secondary_max,
                                    &mbv_secondary_xmin, &mbv_secondary_xmax, &mbv_secondary_ymin, &mbv_secondary_ymax,
                                    &mbv_secondary_dx, &mbv_secondary_dy, &mbv_secondary_data);

    /* set parameters */
    if (status == MB_SUCCESS) {
      mbv_secondary_colortable = MBV_COLORTABLE_HAXBY;
      mbv_secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
      mbv_secondary_colortable_min = mbv_secondary_min;
      mbv_secondary_colortable_max = mbv_secondary_max;
      mbv_overlay_shade_magnitude = 1.0;
      mbv_overlay_shade_center = 0.5 * (mbv_secondary_max + mbv_secondary_min);
      mbv_overlay_shade_mode = MBV_COLORTABLE_NORMAL;
    }

    /* set more mbview control values */
    if (status == MB_SUCCESS) {
      status =
          mbview_setsecondarygrid(verbose, instance, mbv_secondary_grid_projection_mode, mbv_secondary_grid_projection_id,
                                  mbv_secondary_nodatavalue, mbv_secondary_n_columns, mbv_secondary_n_rows, mbv_secondary_min,
                                  mbv_secondary_max, mbv_secondary_xmin, mbv_secondary_xmax, mbv_secondary_ymin,
                                  mbv_secondary_ymax, mbv_secondary_dx, mbv_secondary_dy, mbv_secondary_data, &error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&mbv_secondary_data, &error);
    }
    if (status == MB_SUCCESS)
      status = mbview_setsecondarycolortable(verbose, instance, mbv_secondary_colortable, mbv_secondary_colortable_mode,
                                             mbv_secondary_colortable_min, mbv_secondary_colortable_max,
                                             mbv_overlay_shade_magnitude, mbv_overlay_shade_center, mbv_overlay_shade_mode,
                                             &error);

    /* update widgets */
    if (status == MB_SUCCESS)
      status = mbview_update(verbose, instance, &error);
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_opensite(size_t instance, char *input_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  mb_path buffer;
  mb_path lonstring, latstring;
  int nsite;
  double *sitelon;
  double *sitelat;
  double *sitetopo;
  int *sitecolor;
  int *sitesize;
  mb_path *sitename;
  char *result;
  char *name;
  int nget;
  bool site_ok;
  double londeg, lonmin, latdeg, latmin;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* count the sites in the input file */
    nsite = 0;
    if ((sfp = fopen(input_file_ptr, "r")) == NULL) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to Open Site File <%s> for reading\n", input_file_ptr);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
      return (status);
    }
    while ((result = fgets(buffer, MB_PATH_MAXLINE, sfp)) == buffer) {
      if (buffer[0] != '#')
        nsite++;
    }
    fclose(sfp);

    /* allocate arrays for sites */
    if (nsite > 0) {
      /* allocate the arrays */
      sitelon = NULL;
      sitelat = NULL;
      sitetopo = NULL;
      sitecolor = NULL;
      sitesize = NULL;
      sitename = NULL;
      status =
          mbview_allocsitearrays(verbose, nsite, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);

      /* if error initializing memory then cancel dealing with sites */
      if (status == MB_FAILURE) {
        nsite = 0;
        fprintf(stderr, "\nUnable to allocate arrays for %d sites\n", nsite);
        XBell((Display *)XtDisplay(mainWindow), 100);
        return (status);
      }
    }

    /* read the sites from the input file */
    if (nsite > 0) {
      nsite = 0;
      if ((sfp = fopen(input_file_ptr, "r")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to open site file <%s> for reading\n", input_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
        return (status);
      }
      while ((result = fgets(buffer, MB_PATH_MAXLINE, sfp)) == buffer) {
        site_ok = false;

        /* deal with site in form: lon lat topo color size name */
        if (buffer[0] != '#') {
          nget = sscanf(buffer, "%s %s %lf %d %d %[^\n]", lonstring, latstring, &sitetopo[nsite],
                        &sitecolor[nsite], &sitesize[nsite], sitename[nsite]);
          if (nget >= 2) {
            if (strchr(lonstring, ':') != NULL) {
              if (sscanf(lonstring, "%lf:%lf", &londeg,&lonmin) == 2) {
                sitelon[nsite] = copysign((fabs(londeg) + fabs(lonmin) / 60.0), londeg);
                site_ok = true;
              }
            } else if (sscanf(lonstring, "%lf", &sitelon[nsite]) == 1) {
                site_ok = true;
            }
            if (site_ok) {
              if (strchr(latstring, ':') != NULL) {
                if (sscanf(latstring, "%lf:%lf", &latdeg,&latmin) == 2) {
                  sitelat[nsite] = copysign((fabs(latdeg) + fabs(latmin) / 60.0), latdeg);
                }
              } else if (sscanf(latstring, "%lf", &sitelat[nsite]) == 1) {
              } else {
                site_ok = false;
              }
            }
          }
        }
        if (site_ok) {
          if (nget < 6) {
            name = (char *)sitename[nsite];
            name[0] = '\0';
          }
          if (nget < 5)
            sitesize[nsite] = 0;
          if (nget < 4)
            sitecolor[nsite] = 0;
          if (nget < 3)
            sitetopo[nsite] = MBV_DEFAULT_NODATA;
        }

        /* output some debug values */
        if (verbose > 0 && site_ok) {
          fprintf(stderr, "\ndbg5  Site point read in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       site[%d]: %f %f %f  %d %d  %s\n", nsite, sitelon[nsite], sitelat[nsite],
                  sitetopo[nsite], sitecolor[nsite], sitesize[nsite], sitename[nsite]);
        }
        else if (verbose > 0 && !site_ok) {
          fprintf(stderr, "\ndbg5  Unintelligible line read from site file in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       buffer:  %s\n", buffer);
        }

        strncpy(buffer, "", sizeof(buffer));
        if (site_ok)
          nsite++;
      }
      fclose(sfp);
    }

    /* add the sites */
    if (nsite > 0) {
      status = mbview_addsites(verbose, instance, nsite, sitelon, sitelat, sitetopo, sitecolor, sitesize, sitename, &error);

      /* update widgets */
      if (status == MB_SUCCESS)
        status = mbview_update(verbose, instance, &error);
    }

    /* deallocate memory */
    if (nsite > 0) {
      status = mbview_freesitearrays(verbose, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);
    }
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savesite(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nsite;
  double *sitelon;
  double *sitelat;
  double *sitetopo;
  int *sitecolor;
  int *sitesize;
  mb_path *sitename;
  int i;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of sites to be written to the output file */
    status = mbview_getsitecount(verbose, instance, &nsite, &error);
    if (status == MB_SUCCESS && nsite <= 0) {
      fprintf(stderr, "Unable to write site file...\nCurrently %d sites defined for instance %zu!\n", nsite, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* allocate arrays for sites */
    if (status == MB_SUCCESS && nsite > 0) {
      /* allocate the arrays */
      sitelon = NULL;
      sitelat = NULL;
      sitetopo = NULL;
      sitecolor = NULL;
      sitesize = NULL;
      sitename = NULL;
      status =
          mbview_allocsitearrays(verbose, nsite, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);

      /* if error initializing memory then cancel dealing with sites */
      if (status == MB_FAILURE) {
        nsite = 0;
        fprintf(stderr, "Unable to write site file...\nArray allocation for %d sites failed for instance %zu!\n", nsite,
                instance);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* get the sites */
    if (status == MB_SUCCESS) {
      status =
          mbview_getsites(verbose, instance, &nsite, sitelon, sitelat, sitetopo, sitecolor, sitesize, sitename, &error);
    }

    /* write the sites to the output file */
    if (status == MB_SUCCESS) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the site file header */
        fprintf(sfp, "## Site File Version %s\n", MBGRDVIZ_SITE_VERSION);
        fprintf(sfp, "## Output by Program %s\n", program_name);
        fprintf(sfp, "## MB-System Version %s\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(sfp, "## Number of sites: %d\n", nsite);
        fprintf(sfp, "## Site colors:\n");
        fprintf(sfp, "##   COLOR_BLACK     0\n");
        fprintf(sfp, "##   COLOR_WHITE     1\n");
        fprintf(sfp, "##   COLOR_RED       2\n");
        fprintf(sfp, "##   COLOR_YELLOW    3\n");
        fprintf(sfp, "##   COLOR_GREEN     4\n");
        fprintf(sfp, "##   COLOR_BLUEGREEN 5\n");
        fprintf(sfp, "##   COLOR_BLUE      6\n");
        fprintf(sfp, "##   COLOR_PURPLE    7\n");
        fprintf(sfp, "## Site point format:\n");
        fprintf(sfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <color> <size> <name>\n");

        /* loop over the sites */
        for (i = 0; i < nsite; i++) {
          fprintf(sfp, "%12.7f %12.7f %10.3f %2d %2d %s\n", sitelon[i], sitelat[i], sitetopo[i], sitecolor[i],
                  sitesize[i], sitename[i]);
        }

        /* close the output file */
        fclose(sfp);
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr, "\nUnable to Open Site File <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
        status = MB_FAILURE;
      }
    }

    /* deallocate arrays for sites */
    if (nsite > 0) {
      status = mbview_freesitearrays(verbose, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savesitewpt(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nsite;
  double *sitelon;
  double *sitelat;
  double *sitetopo;
  int *sitecolor;
  int *sitesize;
  mb_path *sitename;
  int i;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of sites to be written to the output file */
    status = mbview_getsitecount(verbose, instance, &nsite, &error);
    if (status == MB_SUCCESS && nsite <= 0) {
      fprintf(stderr, "Unable to write site file...\nCurrently %d sites defined for instance %zu!\n", nsite, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* allocate arrays for sites */
    if (status == MB_SUCCESS && nsite > 0) {
      /* allocate the arrays */
      sitelon = NULL;
      sitelat = NULL;
      sitetopo = NULL;
      sitecolor = NULL;
      sitesize = NULL;
      sitename = NULL;
      status =
          mbview_allocsitearrays(verbose, nsite, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);

      /* if error initializing memory then cancel dealing with sites */
      if (status == MB_FAILURE) {
        nsite = 0;
        fprintf(stderr, "Unable to write site file...\nArray allocation for %d sites failed for instance %zu!\n", nsite,
                instance);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* get the sites */
    if (status == MB_SUCCESS) {
      status =
          mbview_getsites(verbose, instance, &nsite, sitelon, sitelat, sitetopo, sitecolor, sitesize, sitename, &error);
    }

    /* write the sites to the output file */
    if (status == MB_SUCCESS) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the site file header */
        fprintf(sfp, "## Site File Version %s\n", MBGRDVIZ_SITE_VERSION);
        fprintf(sfp, "## Output by Program %s\n", program_name);
        fprintf(sfp, "## MB-System Version %s\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(sfp, "## Number of sites: %d\n", nsite);

        /* loop over the sites */
        for (i = 0; i < nsite; i++) {
          fprintf(sfp, "%s %d,%.10f,%.10f,17,100.0,0.00,0.00,255,0.00\r\n", sitename[i], i, sitelat[i], sitelon[i]);
        }

        /* close the output file */
        fclose(sfp);
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr, "\nUnable to Open Site File <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
        status = MB_FAILURE;
      }
    }

    /* deallocate arrays for sites */
    if (nsite > 0) {
      status = mbview_freesitearrays(verbose, &sitelon, &sitelat, &sitetopo, &sitecolor, &sitesize, &sitename, &error);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_openroute(size_t instance, char *input_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  char buffer[MB_PATH_MAXLINE];
  int npoint = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  double lon, lat, topo;
  int *routewaypoint = NULL;
  int routecolor;
  int routesize;
  int routeeditmode;
  mb_path routename;
  int iroute;
  int waypoint;
  bool rawroutefile = true;
  char *result;
  int nget;
  bool point_ok;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {
    /* initialize route values */
    routecolor = MBV_COLOR_BLUE;
    routesize = 1;
    routeeditmode = true;
    routename[0] = '\0';
    rawroutefile = true;
    npoint = 0;
    npointalloc = 0;

    /* open the input file */
    if ((sfp = fopen(input_file_ptr, "r")) == NULL) {
      error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
      fprintf(stderr, "\nUnable to open route file <%s> for reading\n", input_file_ptr);
      XBell((Display *)XtDisplay(mainWindow), 100);
    }

    /* loop over reading */
    if (status == MB_SUCCESS) {
      while ((result = fgets(buffer, MB_PATH_MAXLINE, sfp)) == buffer) {
        /* deal with comments */
        if (buffer[0] == '#') {
          if (rawroutefile && strncmp(buffer, "## Route File Version", 21) == 0) {
            rawroutefile = false;
          }
          else if (strncmp(buffer, "## ROUTENAME", 12) == 0) {
            strcpy(routename, &buffer[13]);
            if (routename[strlen(routename) - 1] == '\n')
              routename[strlen(routename) - 1] = '\0';
            if (routename[strlen(routename) - 1] == '\r')
              routename[strlen(routename) - 1] = '\0';
          }
          else if (strncmp(buffer, "## ROUTECOLOR", 13) == 0) {
            sscanf(buffer, "## ROUTECOLOR %d", &routecolor);
          }
          else if (strncmp(buffer, "## ROUTESIZE", 12) == 0) {
            sscanf(buffer, "## ROUTESIZE %d", &routesize);
          }
          else if (strncmp(buffer, "## ROUTEEDITMODE", 16) == 0) {
            sscanf(buffer, "## ROUTEEDITMODE %d", &routeeditmode);
          }
        }

        /* deal with route segment marker */
        else if (buffer[0] == '>') {
          /* if data accumulated call mbview_addroute() */
          if (npoint > 0) {
            status = mbview_addroute(verbose, instance, npoint, routelon, routelat, routewaypoint, routecolor,
                                     routesize, routeeditmode, routename, &iroute, &error);
            npoint = 0;
          }
        }

        /* deal with data */
        else {
          /* read the data from the buffer */
          nget = sscanf(buffer, "%lf %lf %lf %d", &lon, &lat, &topo, &waypoint);
          if ((rawroutefile && nget >= 2) ||
              (!rawroutefile && nget >= 3 && waypoint > MBV_ROUTE_WAYPOINT_NONE))
            point_ok = true;
          else
            point_ok = false;

          /* if good data check for need to allocate more space */
          if (point_ok && npoint + 1 > npointalloc) {
            npointalloc += MBV_ALLOC_NUM;
            status = mbview_allocroutearrays(verbose, npointalloc, &routelon, &routelat, &routewaypoint, NULL, NULL,
                                             NULL, NULL, NULL, &error);
            if (status != MB_SUCCESS) {
              npointalloc = 0;
            }
          }

          /* add good point to route */
          if (point_ok && npointalloc > npoint) {
            routelon[npoint] = lon;
            routelat[npoint] = lat;
            routewaypoint[npoint] = waypoint;
            npoint++;
          }
        }
      }

      /* add last route if not already handled */
      if (npoint > 0) {
        status = mbview_addroute(verbose, instance, npoint, routelon, routelat, routewaypoint, routecolor, routesize,
                                 routeeditmode, routename, &iroute, &error);
        npoint = 0;
      }

      /* free the memory */
      if (npointalloc > 0)
        status =
            mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, NULL, NULL, NULL, NULL, NULL, &error);

      /* close the input file */
      fclose(sfp);
    }

    /* update widgets */
    mbview_updateroutelist();
    status = mbview_update(verbose, instance, &error);
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_saveroute(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int nroutewrite = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  bool selected;
  int iroute, j;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    for (iroute = 0; iroute < nroute; iroute++) {
      mbview_getrouteselected(verbose, instance, iroute, &selected, &error);
      if (selected)
        nroutewrite++;
    }
    if (nroutewrite == 0)
      nroutewrite = nroute;
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the route file header */
        fprintf(sfp, "## Route File Version %s\n", MBGRDVIZ_ROUTE_VERSION);
        fprintf(sfp, "## Output by Program %s\n", program_name);
        fprintf(sfp, "## MB-System Version %s\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(sfp, "## Number of routes: %d\n", nroutewrite);
        fprintf(sfp, "## Route waypoint type definitions:\n");
        fprintf(sfp, "##   WAYPOINT_NONE         0  Defines topography between waypoints\n");
        fprintf(sfp, "##   WAYPOINT_SIMPLE       1  Waypoint along survey line\n");
        fprintf(sfp, "##   WAYPOINT_TRANSIT      2  Waypoint along survey line\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE    3  Start survey line type 1\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE      4  End survey line type 1\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE2   5  Start survey line type 2\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE2     6  End survey line type 2\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE3   7  Start survey line type 3\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE3     8  End survey line type 3\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE4   9  Start survey line type 4\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE4    10  End survey line type 4\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE5  11  Start survey line type 5\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE5    12  End survey line type 5\n");
        fprintf(sfp, "## Route point format:\n");
        fprintf(sfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint type> <bearing (deg)> "
                     "<lateral distance (m)> <distance along topography (m)> <slope (m/m)>\n");
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* check if this route is selected for writing */
        if (nroutewrite == nroute)
          selected = true;
        else
          mbview_getrouteselected(verbose, instance, iroute, &selected, &error);

        /* output if selected */
        if (selected) {
          /* get point count for current route */
          status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

          /* allocate route arrays */
          npointtotal = npoint + nintpoint;
          if (status == MB_SUCCESS && npointalloc < npointtotal) {
            status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                             &routebearing, &distlateral, &distovertopo, &slope, &error);
            if (status == MB_SUCCESS) {
              npointalloc = npointtotal;
            }

            /* if error initializing memory then cancel dealing with this route */
            else {
              fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                      npointtotal, instance);
              XBell((Display *)XtDisplay(mainWindow), 100);
              npoint = 0;
              nintpoint = 0;
              npointtotal = 0;
            }
          }

          /* extract data for route */
          status = mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint,
                                   routetopo, routebearing, distlateral, distovertopo, slope, &routecolor, &routesize,
                                   routename, &error);

          /* write the route header */
          fprintf(sfp, "## ROUTENAME %s\n", routename);
          fprintf(sfp, "## ROUTESIZE %d\n", routesize);
          fprintf(sfp, "## ROUTECOLOR %d\n", routecolor);
          fprintf(sfp, "## ROUTEPOINTS %d\n", npointtotal);
          fprintf(sfp, "> ## STARTROUTE\n");

          /* write the route points */
          for (j = 0; j < npointtotal; j++) {
            fprintf(sfp, "%f %f %f %d %f %f %f %f", routelon[j], routelat[j], routetopo[j], routewaypoint[j],
                    routebearing[j], distlateral[j], distovertopo[j], slope[j]);
            if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_SIMPLE)
              fprintf(sfp, " ## WAYPOINT\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_TRANSIT)
              fprintf(sfp, " ## WAYPOINT TRANSIT\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE)
              fprintf(sfp, " ## WAYPOINT STARTLINE\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE)
              fprintf(sfp, " ## WAYPOINT ENDLINE\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE2)
              fprintf(sfp, " ## WAYPOINT STARTLINE2\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE2)
              fprintf(sfp, " ## WAYPOINT ENDLINE2\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE3)
              fprintf(sfp, " ## WAYPOINT STARTLINE3\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE3)
              fprintf(sfp, " ## WAYPOINT ENDLINE3\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE4)
              fprintf(sfp, " ## WAYPOINT STARTLINE4\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE4)
              fprintf(sfp, " ## WAYPOINT ENDLINE4\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE5)
              fprintf(sfp, " ## WAYPOINT STARTLINE5\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE5)
              fprintf(sfp, " ## WAYPOINT ENDLINE5\n");
            else
              fprintf(sfp, "\n");
          }

          /* write the route end */
          fprintf(sfp, "> ## ENDROUTE\n");
        }

        /* deallocate arrays */
        if (npointalloc > 0) {
          status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                          &distlateral, &distovertopo, &slope, &error);
        }
      }

      /* close the output file */
      fclose(sfp);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_saveroutereversed(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int nroutewrite = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  bool selected;
  int iroute, j;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    for (iroute = 0; iroute < nroute; iroute++) {
      mbview_getrouteselected(verbose, instance, iroute, &selected, &error);
      if (selected)
        nroutewrite++;
    }
    if (nroutewrite == 0)
      nroutewrite = nroute;
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the route file header */
        fprintf(sfp, "## Route File Version %s\n", MBGRDVIZ_ROUTE_VERSION);
        fprintf(sfp, "## Output by Program %s\n", program_name);
        fprintf(sfp, "## MB-System Version %s\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(sfp, "## Number of routes: %d\n", nroutewrite);
        fprintf(sfp, "## Route waypoint type definitions:\n");
        fprintf(sfp, "##   WAYPOINT_NONE         0  Defines topography between waypoints\n");
        fprintf(sfp, "##   WAYPOINT_SIMPLE       1  Waypoint along survey line\n");
        fprintf(sfp, "##   WAYPOINT_TRANSIT      2  Waypoint along survey line\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE    3  Start survey line type 1\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE      4  End survey line type 1\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE2   5  Start survey line type 2\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE2     6  End survey line type 2\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE3   7  Start survey line type 3\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE3     8  End survey line type 3\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE4   9  Start survey line type 4\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE4    10  End survey line type 4\n");
        fprintf(sfp, "##   WAYPOINT_STARTLINE5  11  Start survey line type 5\n");
        fprintf(sfp, "##   WAYPOINT_ENDLINE5    12  End survey line type 5\n");
        fprintf(sfp, "## Route point format:\n");
        fprintf(sfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint type> <bearing (deg)> "
                     "<lateral distance (m)> <distance along topography (m)> <slope (m/m)>\n");
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* check if this route is selected for writing */
        if (nroutewrite == nroute)
          selected = true;
        else
          mbview_getrouteselected(verbose, instance, iroute, &selected, &error);

        /* output if selected */
        if (selected) {
          /* get point count for current route */
          status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

          /* allocate route arrays */
          npointtotal = npoint + nintpoint;
          if (status == MB_SUCCESS && npointalloc < npointtotal) {
            status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                             &routebearing, &distlateral, &distovertopo, &slope, &error);
            if (status == MB_SUCCESS) {
              npointalloc = npointtotal;
            }

            /* if error initializing memory then cancel dealing with this route */
            else {
              fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                      npointtotal, instance);
              XBell((Display *)XtDisplay(mainWindow), 100);
              npoint = 0;
              nintpoint = 0;
              npointtotal = 0;
            }
          }

          /* extract data for route */
          status = mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint,
                                   routetopo, routebearing, distlateral, distovertopo, slope, &routecolor, &routesize,
                                   routename, &error);

          /* write the route header */
          fprintf(sfp, "## ROUTENAME %s\n", routename);
          fprintf(sfp, "## ROUTESIZE %d\n", routesize);
          fprintf(sfp, "## ROUTECOLOR %d\n", routecolor);
          fprintf(sfp, "## ROUTEPOINTS %d\n", npointtotal);
          fprintf(sfp, "> ## STARTROUTE\n");

          /* write the route points */
          for (j = npointtotal - 1; j >= 0; j-- ) {
            double bearing = *routebearing - 180.0;
            if (bearing < 0.0)
              bearing += 360.0;
            fprintf(sfp, "%f %f %f %d %f %f %f %f", routelon[j], routelat[j], routetopo[j], routewaypoint[j],
                    routebearing[j], distlateral[j], distovertopo[j], slope[j]);
            if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_SIMPLE)
              fprintf(sfp, " ## WAYPOINT\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_TRANSIT)
              fprintf(sfp, " ## WAYPOINT TRANSIT\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE)
              fprintf(sfp, " ## WAYPOINT STARTLINE\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE)
              fprintf(sfp, " ## WAYPOINT ENDLINE\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE2)
              fprintf(sfp, " ## WAYPOINT STARTLINE2\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE2)
              fprintf(sfp, " ## WAYPOINT ENDLINE2\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE3)
              fprintf(sfp, " ## WAYPOINT STARTLINE3\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE3)
              fprintf(sfp, " ## WAYPOINT ENDLINE3\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE4)
              fprintf(sfp, " ## WAYPOINT STARTLINE4\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE4)
              fprintf(sfp, " ## WAYPOINT ENDLINE4\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_STARTLINE5)
              fprintf(sfp, " ## WAYPOINT STARTLINE5\n");
            else if (routewaypoint[j] == MBV_ROUTE_WAYPOINT_ENDLINE5)
              fprintf(sfp, " ## WAYPOINT ENDLINE5\n");
            else
              fprintf(sfp, "\n");
          }

          /* write the route end */
          fprintf(sfp, "> ## ENDROUTE\n");
        }

        /* deallocate arrays */
        if (npointalloc > 0) {
          status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                          &distlateral, &distovertopo, &slope, &error);
        }
      }

      /* close the output file */
      fclose(sfp);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_saverisiscriptheading(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int nroutewrite = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  bool selected;
  int iroute, j;
  void *pjptr = NULL;
  double origin_x, origin_y;
  double vvspeed = 0.2;
  double settlingtime = 3.0;
  double altitude = 3.0;
  int turndirection = 1;

  if (verbose >= 0) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    for (iroute = 0; iroute < nroute; iroute++) {
      mbview_getrouteselected(verbose, instance, iroute, &selected, &error);
      if (selected)
        nroutewrite++;
    }
    if (nroutewrite == 0)
      nroutewrite = nroute;
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the route file header */
        fprintf(sfp, "## Risi Script Version %s\r\n", MBGRDVIZ_RISISCRIPT_VERSION);
        fprintf(sfp, "## Output by Program %s\r\n", program_name);
        fprintf(sfp, "## MB-System Version %s\r\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\r\n", user, host, date);
        fprintf(sfp, "## Number of routes: %d\r\n", nroutewrite);
        fprintf(sfp, "## Risi script format:\r\n");
        fprintf(sfp, "##   ALT, <altitude (m)>, <speed (m/s)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##   HDG, <heading (deg)>, <turn direction +/-1>, <rate (deg/sec)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##   POS, <north (m)>, <east (m)>, <down (m) (ignored)>, <speed (m/sec)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##\r\n");
        fprintf(sfp, "## This script assumes the survey platform starts at the origin with heading 0.0\r\n");
        fprintf(sfp, "##\r\n");
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\r\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* check if this route is selected for writing */
        if (nroutewrite == nroute)
          selected = true;
        else
          mbview_getrouteselected(verbose, instance, iroute, &selected, &error);

        /* output if selected */
        if (selected) {
          /* get point count for current route */
          status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

          /* allocate route arrays */
          npointtotal = npoint + nintpoint;
          if (status == MB_SUCCESS && npointalloc < npointtotal) {
            status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                             &routebearing, &distlateral, &distovertopo, &slope, &error);
            if (status == MB_SUCCESS) {
              npointalloc = npointtotal;
            }

            /* if error initializing memory then cancel dealing with this route */
            else {
              fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                      npointtotal, instance);
              XBell((Display *)XtDisplay(mainWindow), 100);
              npoint = 0;
              nintpoint = 0;
              npointtotal = 0;
            }
          }

          /* extract data for route */
          status = mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint,
                                   routetopo, routebearing, distlateral, distovertopo, slope, &routecolor, &routesize,
                                   routename, &error);

          /* if this the first route define the projection */
          if (pjptr == NULL && npointtotal > 0) {
            double reference_lon = routelon[0];
            double reference_lat = routelat[0];

            /* calculate eastings and northings using the appropriate UTM projection */
            int projectionid;
            mb_path projection_id;
            if (reference_lat > -80.0 && reference_lat < 84.0) {
              if (reference_lon > 180.0)
                reference_lon -= 360.0;
              int utmzone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
              if (reference_lat >= 0.0)
                projectionid = 32600 + utmzone;
              else
                projectionid = 32700 + utmzone;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }

            /* else if more northerly than 84 deg N then use
                    North Universal Polar Stereographic Projection */
            else if (reference_lat > 84.0) {
              projectionid = 32661;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }

            /* else if more southerly than 80 deg S then use
                    South Universal Polar Stereographic Projection */
            else if (reference_lat < 80.0) {
              projectionid = 32761;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }
            fprintf(stderr, "Reference longitude: %.9f latitude:%.9f Projection ID: %s\n",
                    reference_lon, reference_lat, projection_id);

            /* initialize projection */
            if (mb_proj_init(2, projection_id, &(pjptr), &error) != MB_SUCCESS) {
              char *error_message = NULL;
              mb_error(verbose, error, &error_message);
              fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", error_message);
              fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
              mb_memory_clear(verbose, &error);
              exit(error);
            }
            mb_proj_forward(verbose, pjptr, reference_lon, reference_lat, &origin_x, &origin_y, &error);
          }

          /* output route as Risi script */
          if (pjptr != NULL && npointtotal > 0) {
            /* write the route header */
            fprintf(sfp, "## ROUTENAME %s\r\n", routename);
            fprintf(sfp, "## ROUTEPOINTS %d\r\n", npointtotal);
            fprintf(sfp, "## STARTROUTE\r\n");

            /* write the route points */
            vvspeed = 0.20;
            settlingtime = 3.0;
            altitude = 3.0;
            turndirection = 1;
            double turns = 0.0;
            double heading = routebearing[0];
            double headinglast = 0.0;
            double dheading = heading - headinglast;
            if (dheading > 180.0)
              dheading -= 360.0;
            else if (dheading < -180.0)
              dheading += 360.0;
            if (dheading >= -180.0 && dheading < 0.0) {
              turndirection = -1;
            } else if (dheading >= 0.0 && dheading < 180.0) {
              turndirection = 1;
            }

            fprintf(sfp, "ALT, %.3f, 0.1, 3\r\n", altitude);
            fprintf(sfp, "##\n");
            fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", heading, turndirection, settlingtime);
            headinglast = heading;
            for (j = 0; j < npointtotal; j++) {
              if (j >= 0 && routewaypoint[j] > MBV_ROUTE_WAYPOINT_NONE) {
                double xxxx, yyyy, zz;
                mb_proj_forward(verbose, pjptr, routelon[j], routelat[j], &xxxx, &yyyy, &error);
                xxxx = xxxx - origin_x;
                yyyy = yyyy - origin_y;
                zz = -altitude;
                heading = routebearing[j];
                dheading = heading - headinglast;
                if (dheading > 180.0)
                  dheading -= 360.0;
                else if (dheading < -180.0)
                  dheading += 360.0;
                if (dheading >= -180.0 && dheading < 0.0) {
                  turndirection = -1;
                } else if (dheading >= 0.0 && dheading < 180.0) {
                  turndirection = 1;
                }
                fprintf(sfp, "POS, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", yyyy, xxxx, zz, vvspeed, settlingtime);
                fprintf(sfp, "##\n");
                fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", heading, turndirection, settlingtime);
                turns += dheading / 360.0;
                headinglast = heading;
                fprintf(stderr, "j:%d turns: %f\n", j, turns);
              }
            }
            if (turns < 0.0)
              turndirection = -1;
            else
              turndirection = 1;
            fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", 0.0, turndirection, settlingtime);

            /* write the route end */
            fprintf(sfp, "## End\r\n");
          }

          /* deallocate arrays */
          if (npointalloc > 0) {
            status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                            &distlateral, &distovertopo, &slope, &error);
          }
        }
      }

      /* close the output file */
      fclose(sfp);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_saverisiscriptnoheading(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int nroutewrite = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  bool selected;
  int iroute, j;
  void *pjptr = NULL;
  double origin_x, origin_y;
  double vvspeed = 0.2;
  double settlingtime = 3.0;
  double altitude = 3.0;
  int turndirection = 1;

  if (verbose >= 0) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    for (iroute = 0; iroute < nroute; iroute++) {
      mbview_getrouteselected(verbose, instance, iroute, &selected, &error);
      if (selected)
        nroutewrite++;
    }
    if (nroutewrite == 0)
      nroutewrite = nroute;
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the route file header */
        fprintf(sfp, "## Risi Script Version %s\r\n", MBGRDVIZ_RISISCRIPT_VERSION);
        fprintf(sfp, "## Output by Program %s\r\n", program_name);
        fprintf(sfp, "## MB-System Version %s\r\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\r\n", user, host, date);
        fprintf(sfp, "## Number of routes: %d\r\n", nroutewrite);
        fprintf(sfp, "## Risi script format:\r\n");
        fprintf(sfp, "##   ALT, <altitude (m)>, <speed (m/s)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##   HDG, <heading (deg)>, <turn direction +/-1>, <rate (deg/sec)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##   POS, <north (m)>, <east (m)>, <down (m) (ignored)>, <speed (m/sec)>, <settling time (sec)>\r\n");
        fprintf(sfp, "##\r\n");
        fprintf(sfp, "## This script assumes the survey platform starts at the origin with heading 0.0\r\n");
        fprintf(sfp, "##\r\n");
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\r\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS && nroutewrite > 0) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* check if this route is selected for writing */
        if (nroutewrite == nroute)
          selected = true;
        else
          mbview_getrouteselected(verbose, instance, iroute, &selected, &error);

        /* output if selected */
        if (selected) {
          /* get point count for current route */
          status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

          /* allocate route arrays */
          npointtotal = npoint + nintpoint;
          if (status == MB_SUCCESS && npointalloc < npointtotal) {
            status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                             &routebearing, &distlateral, &distovertopo, &slope, &error);
            if (status == MB_SUCCESS) {
              npointalloc = npointtotal;
            }

            /* if error initializing memory then cancel dealing with this route */
            else {
              fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                      npointtotal, instance);
              XBell((Display *)XtDisplay(mainWindow), 100);
              npoint = 0;
              nintpoint = 0;
              npointtotal = 0;
            }
          }

          /* extract data for route */
          status = mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint,
                                   routetopo, routebearing, distlateral, distovertopo, slope, &routecolor, &routesize,
                                   routename, &error);

          /* if this the first route define the projection using the starting point */
          if (pjptr == NULL && npointtotal > 0) {
            double reference_lon = routelon[0];
            double reference_lat = routelat[0];

            /* calculate eastings and northings using the appropriate UTM projection */
            int projectionid;
            mb_path projection_id;
            if (reference_lat > -80.0 && reference_lat < 84.0) {
              if (reference_lon > 180.0)
                reference_lon -= 360.0;
              int utmzone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
              if (reference_lat >= 0.0)
                projectionid = 32600 + utmzone;
              else
                projectionid = 32700 + utmzone;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }

            /* else if more northerly than 84 deg N then use
                    North Universal Polar Stereographic Projection */
            else if (reference_lat > 84.0) {
              projectionid = 32661;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }

            /* else if more southerly than 80 deg S then use
                    South Universal Polar Stereographic Projection */
            else if (reference_lat < 80.0) {
              projectionid = 32761;
              sprintf(projection_id, "EPSG:%d", projectionid);
            }
            fprintf(stderr, "Reference longitude: %.9f latitude:%.9f Projection ID: %s\n",
                    reference_lon, reference_lat, projection_id);

            /* initialize projection */
            if (mb_proj_init(2, projection_id, &(pjptr), &error) != MB_SUCCESS) {
              char *error_message = NULL;
              mb_error(verbose, error, &error_message);
              fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", error_message);
              fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
              mb_memory_clear(verbose, &error);
              exit(error);
            }
            mb_proj_forward(verbose, pjptr, reference_lon, reference_lat, &origin_x, &origin_y, &error);
          }

          /* output route as Risi script */
          if (pjptr != NULL && npointtotal > 0) {
            /* write the route header */
            fprintf(sfp, "## ROUTENAME %s\r\n", routename);
            fprintf(sfp, "## ROUTEPOINTS %d\r\n", npointtotal);
            fprintf(sfp, "## STARTROUTE\r\n");

            /* write the route points */
            vvspeed = 0.20;
            settlingtime = 3.0;
            altitude = 3.0;
            turndirection = 1;
            double turns = 0.0;
            double heading = routebearing[0];
            double headinglast = 0.0;
            double dheading = heading - headinglast;
            if (dheading > 180.0)
              dheading -= 360.0;
            else if (dheading < -180.0)
              dheading += 360.0;
            if (dheading >= -180.0 && dheading < 0.0) {
              turndirection = -1;
            } else if (dheading >= 0.0 && dheading < 180.0) {
              turndirection = 1;
            }

            fprintf(sfp, "ALT, %.3f, 0.1, 3\r\n", altitude);
            fprintf(sfp, "##\n");
            fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", heading, turndirection, settlingtime);
            headinglast = heading;
            for (j = 0; j < npointtotal; j++) {
              if (j >= 0 && routewaypoint[j] > MBV_ROUTE_WAYPOINT_NONE) {
                double xxxx, yyyy, zz;
                mb_proj_forward(verbose, pjptr, routelon[j], routelat[j], &xxxx, &yyyy, &error);
                xxxx = xxxx - origin_x;
                yyyy = yyyy - origin_y;
                zz = -altitude;
                //heading = routebearing[j];
                dheading = heading - headinglast;
                if (dheading > 180.0)
                  dheading -= 360.0;
                else if (dheading < -180.0)
                  dheading += 360.0;
                if (dheading >= -180.0 && dheading < 0.0) {
                  turndirection = -1;
                } else if (dheading >= 0.0 && dheading < 180.0) {
                  turndirection = 1;
                }
                fprintf(sfp, "POS, %.3f, %.3f, %.3f, %.3f, %.3f\r\n", yyyy, xxxx, zz, vvspeed, settlingtime);
                //fprintf(sfp, "##\n");
                //fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", heading, turndirection, settlingtime);
                turns += dheading / 360.0;
                headinglast = heading;
                fprintf(stderr, "j:%d turns: %f\n", j, turns);
              }
            }
            if (turns < 0.0)
              turndirection = -1;
            else
              turndirection = 1;
            fprintf(sfp, "HDG, %.3f, %d, 6, %.3f\r\n", 0.0, turndirection, settlingtime);

            /* write the route end */
            fprintf(sfp, "## End\r\n");
          }

          /* deallocate arrays */
          if (npointalloc > 0) {
            status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                            &distlateral, &distovertopo, &slope, &error);
          }
        }
      }

      /* close the output file */
      fclose(sfp);
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savewinfrogpts(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  int iroute, j;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroute > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* get point count for current route */
        status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

        /* allocate route arrays */
        npointtotal = npoint + nintpoint;
        if (status == MB_SUCCESS && npointalloc < npointtotal) {
          status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                           &routebearing, &distlateral, &distovertopo, &slope, &error);
          if (status == MB_SUCCESS) {
            npointalloc = npointtotal;
          }

          /* if error initializing memory then cancel dealing with this route */
          else {
            fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                    npointtotal, instance);
            XBell((Display *)XtDisplay(mainWindow), 100);
            npoint = 0;
            nintpoint = 0;
            npointtotal = 0;
          }
        }

        /* extract data for route */
        status =
            mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint, routetopo,
                            routebearing, distlateral, distovertopo, slope, &routecolor, &routesize, routename, &error);

        /* write the route header */
        fprintf(sfp, "0,%s,0,0.000,0.000,1,2,65280,0,0.200,0,0,1.000\r\n", routename);

        /* write the route points */
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE)
            fprintf(sfp, "1,%.10f,%.10f,0.00m,0.00m,0.00,0.00,%.3f\r\n", routelat[j], routelon[j], distlateral[j]);
        }
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savewinfrogwpt(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  int iroute, j, n;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroute > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* get point count for current route */
        status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

        /* allocate route arrays */
        npointtotal = npoint + nintpoint;
        if (status == MB_SUCCESS && npointalloc < npointtotal) {
          status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                           &routebearing, &distlateral, &distovertopo, &slope, &error);
          if (status == MB_SUCCESS) {
            npointalloc = npointtotal;
          }

          /* if error initializing memory then cancel dealing with this route */
          else {
            fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                    npointtotal, instance);
            XBell((Display *)XtDisplay(mainWindow), 100);
            npoint = 0;
            nintpoint = 0;
            npointtotal = 0;
          }
        }

        /* extract data for route */
        status =
            mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint, routetopo,
                            routebearing, distlateral, distovertopo, slope, &routecolor, &routesize, routename, &error);

        /* write the route points */
        n = 0;
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            n++;
            fprintf(sfp, "%s %d,%.10f,%.10f,17,100.0,0.00,0.00,255,0.00\r\n", routename, n, routelat[j], routelon[j]);
          }
        }
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savedegdecmin(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  char latNS, lonEW;
  int latDeg, lonDeg;
  double latMin, lonMin;
  int iroute, j, n;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroute > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* get point count for current route */
        status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

        /* allocate route arrays */
        npointtotal = npoint + nintpoint;
        if (status == MB_SUCCESS && npointalloc < npointtotal) {
          status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                           &routebearing, &distlateral, &distovertopo, &slope, &error);
          if (status == MB_SUCCESS) {
            npointalloc = npointtotal;
          }

          /* if error initializing memory then cancel dealing with this route */
          else {
            fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                    npointtotal, instance);
            XBell((Display *)XtDisplay(mainWindow), 100);
            npoint = 0;
            nintpoint = 0;
            npointtotal = 0;
          }
        }

        /* extract data for route */
        status =
            mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint, routetopo,
                            routebearing, distlateral, distovertopo, slope, &routecolor, &routesize, routename, &error);

        /* write the route points */
        n = 0;
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            n++;
          }
        }
        if (iroute > 0)
          fprintf(sfp, "#\r\n");
        fprintf(sfp, "# Route: %s\r\n", routename);
        fprintf(sfp, "# Number of waypoints: %d\r\n", n);
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            if (routelat[j] >= 0.0)
              latNS = 'N';
            else
              latNS = 'S';
            latDeg = (int)(floor(fabs(routelat[j])));
            latMin = (fabs(routelat[j]) - (double)latDeg) * 60.0;
            if (routelon[j] >= 0.0)
              lonEW = 'E';
            else
              lonEW = 'W';
            lonDeg = (int)(floor(fabs(routelon[j])));
            lonMin = (fabs(routelon[j]) - (double)lonDeg) * 60.0;

            fprintf(sfp, "%c %3d %9.6f   %c %3d %9.6f \r\n", latNS, latDeg, latMin, lonEW, lonDeg, lonMin);
          }
        }
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savelnw(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  char *error_message;
  char projection_id[MB_PATH_MAXLINE];
  void *pjptr = NULL;
  int proj_status;
  int utm_zone;
  double reference_lon, reference_lat;
  double easting, northing;
  int iroute, j, n;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroute > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* output number of routes */
      fprintf(sfp, "LNS %d\r\n", nroute);

      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* get point count for current route */
        status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

        /* allocate route arrays */
        npointtotal = npoint + nintpoint;
        if (status == MB_SUCCESS && npointalloc < npointtotal) {
          status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                           &routebearing, &distlateral, &distovertopo, &slope, &error);
          if (status == MB_SUCCESS) {
            npointalloc = npointtotal;
          }

          /* if error initializing memory then cancel dealing with this route */
          else {
            fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                    npointtotal, instance);
            XBell((Display *)XtDisplay(mainWindow), 100);
            npoint = 0;
            nintpoint = 0;
            npointtotal = 0;
          }
        }

        /* extract data for route */
        status =
            mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint, routetopo,
                            routebearing, distlateral, distovertopo, slope, &routecolor, &routesize, routename, &error);

        /* if this the first route define the projection */
        if (pjptr == NULL && npointtotal > 0) {
          reference_lon = 0.0;
          reference_lat = 0.0;
          for (j = 0; j < npointtotal; j++) {
            reference_lon += routelon[j];
            reference_lat += routelat[j];
          }
          reference_lon = reference_lon / npointtotal;
          reference_lat = reference_lat / npointtotal;
          if (reference_lon < 180.0)
            reference_lon += 360.0;
          if (reference_lon >= 180.0)
            reference_lon -= 360.0;
          utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
          if (reference_lat >= 0.0)
            sprintf(projection_id, "UTM%2.2dN", utm_zone);
          else
            sprintf(projection_id, "UTM%2.2dS", utm_zone);
          fprintf(stderr, "Reference longitude: %.9f latitude:%.9f\nOutput lnw file in projection:%s\n", reference_lon,
                  reference_lat, projection_id);

          /* initialize appropriate UTM projection */
          proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

          /* quit if projection fails */
          if (proj_status != MB_SUCCESS) {
            mb_error(verbose, error, &error_message);
            fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", error_message);
            fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
            mb_memory_clear(verbose, &error);
            exit(error);
          }
        }

        /* write the route points */
        n = 0;
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            n++;
          }
        }
        fprintf(sfp, "LIN %d\r\n", n);
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            proj_status = mb_proj_forward(verbose, pjptr, routelon[j], routelat[j], &easting, &northing, &error);
            fprintf(sfp, "PTS %.2f %.2f\r\n", easting, northing);
          }
        }
        fprintf(sfp, "LNN %d\r\nEOL\r\n", iroute + 1);
      }

      /* free the projection */
      mb_proj_free(verbose, &pjptr, &error);

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savegreenseayml(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  bool selected;
  // char *error_message;
  // char projection_id[MB_PATH_MAXLINE];
  // void *pjptr = NULL;
  // int proj_status;
  int nroutewrite = 0;
  int iroutewrite = 0;
  int iroute, j, n;
  #ifdef USE_UUID
  uuid_t mission_uuid, *waypoints_uuid;
  #endif
  char uuid_str[37];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    for (iroute = 0; iroute < nroute; iroute++) {
      mbview_getrouteselected(verbose, instance, iroute, &selected, &error);
      if (selected) {
        nroutewrite++;
        iroutewrite = iroute;
      }
    }
    if (nroutewrite == 0 && nroute == 1) {
      nroutewrite = 1;
      iroutewrite = 0;
    }
    if (nroutewrite != 1) {
      fprintf(stderr, "Unable to write Greensea YML survey script...\n");
      fprintf(stderr, "Exactly one route must be selected, but %d routes are selected for instance %zu!\n", nroutewrite, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroutewrite == 1) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open Greensea survey script file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* get point count for current route */
      status = mbview_getroutepointcount(verbose, instance, iroutewrite, &npoint, &nintpoint, &error);

      /* allocate route arrays */
      npointtotal = npoint + nintpoint;
      if (status == MB_SUCCESS && npointalloc < npointtotal) {
        status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                         &routebearing, &distlateral, &distovertopo, &slope, &error);
#ifdef USE_UUID
        if (status == MB_SUCCESS) {
          status = mb_mallocd(verbose, __FILE__, __LINE__,
                              npointtotal * sizeof(uuid_t),
                              (void **)&waypoints_uuid, &error);
        }
#endif
        if (status == MB_SUCCESS) {
          npointalloc = npointtotal;
        }

        /* if error initializing memory then cancel dealing with this route */
        else {
          fprintf(stderr, "Unable to write route...\nArray allocation for %d points failed for instance %zu!\n",
                  npointtotal, instance);
          XBell((Display *)XtDisplay(mainWindow), 100);
          npoint = 0;
          nintpoint = 0;
          npointtotal = 0;
        }
      }

      /* extract data for route */
      status = mbview_getroute(verbose, instance, iroutewrite, &npointtotal,
                                routelon, routelat, routewaypoint, routetopo,
                                routebearing, distlateral, distovertopo, slope,
                                &routecolor, &routesize, routename, &error);

      /* output header of mission */
      fprintf(sfp, "mission_data:\n");
#ifdef USE_UUID
      uuid_generate(mission_uuid);
      uuid_unparse(mission_uuid, uuid_str);
#else
      sprintf(uuid_str, "MBsystem-1962-1991-2018-%12.12d", npointtotal);
#endif
      fprintf(sfp, "  - id: %s\n", uuid_str);
      fprintf(sfp, "    name: Low_Altitude_Survey\n");
      fprintf(sfp, "    locked: true\n");
      fprintf(sfp, "    waypoints:\n");

      /* write the route points */
      for (j = 0; j < npointtotal; j++) {
        if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
#ifdef USE_UUID
          uuid_generate(waypoints_uuid[j]);
          uuid_unparse(waypoints_uuid[j], uuid_str);
#else
          sprintf(uuid_str, "Waypoint-abcd-efgh-ijkl-%12.12d", j);
#endif
          fprintf(sfp, "    - id: %s\n", uuid_str);
        }
      }
      fprintf(sfp, "waypoint_data:\n");
      n = 0;
      for (j = 0; j < npointtotal; j++) {
        if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
#ifdef USE_UUID
          uuid_unparse(waypoints_uuid[j], uuid_str);
#else
          sprintf(uuid_str, "Waypoint-abcd-efgh-ijkl-%12.12d", j);
#endif
          fprintf(sfp, "  - id: %s\n", uuid_str);
          fprintf(sfp, "    name: SPS%4.4d\n", n);
          fprintf(sfp, "    x: %.9f\n", routelon[j]);
          fprintf(sfp, "    y: %.9f\n", routelat[j]);
          fprintf(sfp, "    z: %.3f\n", 3.0);
          fprintf(sfp, "    tolerance: %.3f\n", 0.500);
          fprintf(sfp, "    z_alt: true\n");
          fprintf(sfp, "    z_matters: true\n");
          fprintf(sfp, "    speed: %.3f\n", 0.150);
          fprintf(sfp, "    use_speed: true\n");
          fprintf(sfp, "    effort: 70.000\n");
          if (j==0)
            fprintf(sfp, "    heading: %.3f\n", routebearing[j]);
          else
              fprintf(sfp, "    heading: %.3f\n", routebearing[j-1]);
          fprintf(sfp, "    heading_mode: FIXED\n");
          n++;
        }
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
#ifdef USE_UUID
        status = mb_freed(verbose, __FILE__, __LINE__, (void **)&waypoints_uuid, &error);
#endif
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_savetecdislst(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int nroute = 0;
  int npoint = 0;
  int nintpoint = 0;
  int npointtotal = 0;
  int npointalloc = 0;
  double *routelon = NULL;
  double *routelat = NULL;
  int *routewaypoint = NULL;
  double *routetopo = NULL;
  double *routebearing = NULL;
  double *distlateral = NULL;
  double *distovertopo = NULL;
  double *slope = NULL;
  int routecolor;
  int routesize;
  mb_path routename;
  char latNS, lonEW;
  int latDeg, lonDeg;
  double latMin, lonMin;
  int iroute, j, n;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of routes to be written to the output file */
    status = mbview_getroutecount(verbose, instance, &nroute, &error);
    if (nroute <= 0) {
      fprintf(stderr, "Unable to write TECDIS LST route file...\nCurrently %d routes defined for instance %zu!\n", nroute, instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && nroute > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) == NULL) {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to open route file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output routes */
    if (status == MB_SUCCESS) {
      /* loop over routes */
      for (iroute = 0; iroute < nroute; iroute++) {
        /* get point count for current route */
        status = mbview_getroutepointcount(verbose, instance, iroute, &npoint, &nintpoint, &error);

        /* allocate route arrays */
        npointtotal = npoint + nintpoint;
        if (status == MB_SUCCESS && npointalloc < npointtotal) {
          status = mbview_allocroutearrays(verbose, npointtotal, &routelon, &routelat, &routewaypoint, &routetopo,
                                           &routebearing, &distlateral, &distovertopo, &slope, &error);
          if (status == MB_SUCCESS) {
            npointalloc = npointtotal;
          }

          /* if error initializing memory then cancel dealing with this route */
          else {
            fprintf(stderr, "Unable to write TECDIS LST route file...\nArray allocation for %d points failed for instance %zu!\n",
                    npointtotal, instance);
            XBell((Display *)XtDisplay(mainWindow), 100);
            npoint = 0;
            nintpoint = 0;
            npointtotal = 0;
          }
        }

        /* extract data for route */
        status =
            mbview_getroute(verbose, instance, iroute, &npointtotal, routelon, routelat, routewaypoint, routetopo,
                            routebearing, distlateral, distovertopo, slope, &routecolor, &routesize, routename, &error);

        /* write the route points */
        n = 0;
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            n++;
          }
        }
        if (iroute > 0)
          fprintf(sfp, "#\r\n");
        fprintf(sfp, "# Route: %s\r\n", routename);
        fprintf(sfp, "# Number of waypoints: %d\r\n", n);
        for (j = 0; j < npointtotal; j++) {
          if (routewaypoint[j] != MBV_ROUTE_WAYPOINT_NONE) {
            if (routelat[j] >= 0.0)
              latNS = 'N';
            else
              latNS = 'S';
            latDeg = (int)(floor(fabs(routelat[j])));
            latMin = (fabs(routelat[j]) - (double)latDeg) * 60.0;
            if (routelon[j] >= 0.0)
              lonEW = 'E';
            else
              lonEW = 'W';
            lonDeg = (int)(floor(fabs(routelon[j])));
            lonMin = (fabs(routelon[j]) - (double)lonDeg) * 60.0;

			if (j == 0) {
              fprintf(sfp, "$PTLKR,0,0,%s\r\n", output_file_ptr);
              fprintf(sfp, "$PTLKP,8,%2.2d%9.6f,%c,%3.3d%9.6f,%c\r\n", latDeg, latMin, latNS, lonDeg, lonMin, lonEW);
			}
            fprintf(sfp, "$PTLKP,9,%2.2d%9.6f,%c,%3.3d%9.6f,%c\r\n", latDeg, latMin, latNS, lonDeg, lonMin, lonEW);
          }
        }
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeroutearrays(verbose, &routelon, &routelat, &routewaypoint, &routetopo, &routebearing,
                                        &distlateral, &distovertopo, &slope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_openvector(size_t instance, char *input_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  char buffer[MB_PATH_MAXLINE];
  int npoint = 0;
  int npointalloc = 0;
  double *vectorlon = NULL;
  double *vectorlat = NULL;
  double *vectorz = NULL;
  double *vectordata = NULL;
  double lon, lat, z, data;
  int vectorcolor;
  int vectorsize;
  double vectordatamin;
  double vectordatamax;
  bool minmax_set = false;
  mb_path vectorname;
  bool rawvectorfile = true;
  char *result;
  int nget;
  int point_ok;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {
    /* initialize vector values */
    vectorcolor = MBV_COLOR_BLUE;
    vectorsize = 4;
    vectorname[0] = '\0';
    rawvectorfile = true;
    npoint = 0;
    npointalloc = 0;
    vectordatamin = 0.0;
    vectordatamax = 0.0;
    minmax_set = false;

    /* open the input file */
    if ((sfp = fopen(input_file_ptr, "r")) == NULL) {
      error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
      fprintf(stderr, "\nUnable to open vector file <%s> for reading\n", input_file_ptr);
      XBell((Display *)XtDisplay(mainWindow), 100);
    }

    /* loop over reading */
    if (status == MB_SUCCESS) {
      fprintf(stderr, "Reading from vector file:%s\n", input_file_ptr);
      while ((result = fgets(buffer, MB_PATH_MAXLINE, sfp)) == buffer) {
        /* deal with comments */
        if (buffer[0] == '#') {
          if (rawvectorfile && strncmp(buffer, "## Vector File Version", 21) == 0) {
            rawvectorfile = false;
          }
          else if (strncmp(buffer, "## VECTORNAME", 12) == 0) {
            sscanf(buffer, "## VECTORNAME %s", vectorname);
          }
          else if (strncmp(buffer, "## VECTORCOLOR", 13) == 0) {
            sscanf(buffer, "## ROUTECOLOR %d", &vectorcolor);
          }
          else if (strncmp(buffer, "## ROUTESIZE", 12) == 0) {
            sscanf(buffer, "## ROUTESIZE %d", &vectorsize);
          }
          else if (strncmp(buffer, "## MIN", 6) == 0) {
            sscanf(buffer, "## MIN %lf", &vectordatamin);
            minmax_set = true;
          }
          else if (strncmp(buffer, "## MAX", 6) == 0) {
            sscanf(buffer, "## MAX %lf", &vectordatamax);
            minmax_set = true;
          }
        }

        /* deal with vector segment marker */
        else if (buffer[0] == '>') {
          /* if data accumulated call mbview_addvector() */
          if (npoint > 0) {
            status = mbview_addvector(verbose, instance, npoint, vectorlon, vectorlat, vectorz, vectordata,
                                      vectorcolor, vectorsize, vectorname, vectordatamin, vectordatamax, &error);
            npoint = 0;
          }
        }

        /* deal with data */
        else {
          /* read the data from the buffer */
          nget = sscanf(buffer, "%lf %lf %lf %lf", &lon, &lat, &z, &data);
          if (nget == 4)
            point_ok = true;
          else
            point_ok = false;

          /* if good data check for need to allocate more space */
          if (point_ok && npoint + 1 > npointalloc) {
            npointalloc += MBV_ALLOC_NUM;
            status =
                mbview_allocvectorarrays(verbose, npointalloc, &vectorlon, &vectorlat, &vectorz, &vectordata, &error);
            if (status != MB_SUCCESS) {
              npointalloc = 0;
            }
          }

          /* add good point to vector */
          if (point_ok && npointalloc > npoint) {
            vectorlon[npoint] = lon;
            vectorlat[npoint] = lat;
            vectorz[npoint] = z;
            vectordata[npoint] = data;

            /* get min max bounds if not set in file header */
            if (!minmax_set) {
              if (npoint == 0) {
                vectordatamin = data;
                vectordatamax = data;
              }
              else {
                vectordatamin = MIN(vectordatamin, data);
                vectordatamax = MAX(vectordatamax, data);
              }
            }

            /* increment the counter */
            npoint++;
          }
        }
      }

      /* add last vector if not already handled */
      if (npoint > 0) {
        fprintf(stderr, "Adding vector npoints:%d value min max: %f %f\n", npoint, vectordatamin, vectordatamax);
        status = mbview_addvector(verbose, instance, npoint, vectorlon, vectorlat, vectorz, vectordata, vectorcolor,
                                  vectorsize, vectorname, vectordatamin, vectordatamax, &error);
        npoint = 0;
      }

      /* free the memory */
      if (npointalloc > 0)
        status = mbview_freevectorarrays(verbose, &vectorlon, &vectorlat, &vectorz, &vectordata, &error);

      /* close the input file */
      fclose(sfp);
    }

    /* update widgets */
    mbview_enableviewvectors(verbose, instance, &error);
    status = mbview_update(verbose, instance, &error);
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_saveprofile(size_t instance, char *output_file_ptr) {
  int status = MB_SUCCESS;
  FILE *sfp;
  int npoints = 0;
  int npointalloc = 0;
  double *prdistance = NULL;
  double *prtopo = NULL;
  int *prboundary = NULL;
  double *prlon = NULL;
  double *prlat = NULL;
  double *prdistovertopo = NULL;
  double *prbearing = NULL;
  double *prslope = NULL;
  mb_path prsourcename;
  double prlength;
  double przmin;
  double przmax;
  int j;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       output_file_ptr: %s\n", output_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {

    /* get the number of profiles to be written to the output file */
    status = mbview_getprofilecount(verbose, instance, &npoints, &error);
    if (npoints <= 0) {
      fprintf(stderr, "Unable to write profile file...\nCurrently %d profile points defined for instance %zu!\n", npoints,
              instance);
      XBell((Display *)XtDisplay(mainWindow), 100);
      status = MB_FAILURE;
    }

    /* initialize the output file */
    if (status == MB_SUCCESS && npoints > 0) {
      /* open the output file */
      if ((sfp = fopen(output_file_ptr, "w")) != NULL) {
        /* write the profile file header */
        fprintf(sfp, "## Profile File Version %s\n", MBGRDVIZ_PROFILE_VERSION);
        fprintf(sfp, "## Output by Program %s\n", program_name);
        fprintf(sfp, "## MB-System Version %s\n", MB_VERSION);
        char user[256], host[256], date[32];
        status = mb_user_host_date(verbose, user, host, date, &error);
        fprintf(sfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(sfp, "## Number of profile points: %d\n", npoints);
        fprintf(sfp, "## Profile point format:\n");
        fprintf(sfp, "##   <lateral distance (m)> <topography (m)> <boundary (boolean)> <longitude (deg)> <latitude "
                     "(deg)> <distance over topo (m)> <bearing (deg)> <slope (m/m)>\n");
      }

      /* output error message */
      else {
        error = MB_ERROR_OPEN_FAIL;
        status = MB_FAILURE;
        fprintf(stderr, "\nUnable to Open profile file <%s> for writing\n", output_file_ptr);
        XBell((Display *)XtDisplay(mainWindow), 100);
      }
    }

    /* if all ok proceed to extract and output profiles */
    if (status == MB_SUCCESS) {
      /* allocate profile arrays */
      if (status == MB_SUCCESS && npointalloc < npoints) {
        status = mbview_allocprofilearrays(verbose, npoints, &prdistance, &prtopo, &prboundary, &prlon, &prlat,
                                           &prdistovertopo, &prbearing, &prslope, &error);
        if (status == MB_SUCCESS) {
          npointalloc = npoints;
        }

        /* if error initializing memory then cancel dealing with this profile */
        else {
          fprintf(stderr, "Unable to write profile...\nArray allocation for %d points failed for instance %zu!\n",
                  npoints, instance);
          XBell((Display *)XtDisplay(mainWindow), 100);
          npoints = 0;
        }
      }

      /* extract data for profile */
      status = mbview_getprofile(verbose, instance, prsourcename, &prlength, &przmin, &przmax, &npoints, prdistance, prtopo,
                                 prboundary, prlon, prlat, prdistovertopo, prbearing, prslope, &error);

      /* write the profile header */
      fprintf(sfp, "## PROFILESOURCE %s\n", prsourcename);
      fprintf(sfp, "## PROFILELENGTH %f\n", prlength);
      fprintf(sfp, "## PROFILEZMIN %f\n", przmin);
      fprintf(sfp, "## PROFILEZMAX %f\n", przmax);
      fprintf(sfp, "## PROFILEPOINTS %d\n", npoints);

      /* write the profile points */
      for (j = 0; j < npoints; j++) {
        fprintf(sfp, "%f %f %d %f %f %f %f %f\n", prdistance[j], prtopo[j], prboundary[j], prlon[j], prlat[j],
                prdistovertopo[j], prbearing[j], prslope[j]);
      }

      /* close the output file */
      fclose(sfp);

      /* deallocate arrays */
      if (npointalloc > 0) {
        status = mbview_freeprofilearrays(verbose, &prdistance, &prtopo, &prboundary, &prlon, &prlat, &prdistovertopo,
                                          &prbearing, &prslope, &error);
      }
    }
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/
int do_mbgrdviz_opennav(size_t instance, bool swathbounds, char *input_file_ptr) {
  int status = MB_SUCCESS;
  void *datalist;
  mb_path swathfile;
  int swathfilestatus;
  mb_path swathfileraw;
  mb_path swathfileprocessed;
  mb_path dfile;
  int astatus = MB_ALTNAV_USE;
  mb_path apath;
  int format;
  int formatorg;
  double weight;
  mb_path messagestr;
  char *lastslash;
  int nfiledatalist = 0;
  int nfileread = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       swathbounds:     %d\n", swathbounds);
    fprintf(stderr, "dbg2       input_file_ptr:  %s\n", input_file_ptr);
  }

  /* read data for valid instance */
  if (instance != MBV_NO_WINDOW) {
    bool done = false;
    while (!done) {
      if ((status = mb_datalist_open(verbose, &datalist, input_file_ptr, MB_DATALIST_LOOK_UNSET, &error)) == MB_SUCCESS) {
        while (!done) {
          if ((status = mb_datalist_read3(verbose, datalist, &swathfilestatus, swathfileraw, swathfileprocessed, 
                                          &astatus, apath, dfile,
                                          &format, &weight, &error)) == MB_SUCCESS) {
            nfiledatalist++;
            if (format != MBF_ASCIIXYZ && format != MBF_ASCIIYXZ && format != MBF_ASCIIXYT &&
                format != MBF_ASCIIYXT) {
              /* check for available nav file if that is
                 all that is needed */
              if (swathfilestatus == MB_PROCESSED_USE)
                strcpy(swathfile, swathfileprocessed);
              else
                strcpy(swathfile, swathfileraw);
              formatorg = format;
              if (!swathbounds)
                mb_get_fnv(verbose, swathfile, &format, &error);

              /* else check for available fbt file  */
              else
                mb_get_fbt(verbose, swathfile, &format, &error);

              /* read the swath or nav data using mbio calls */

              /* update message */
              if (!swathbounds)
                strcpy(messagestr, "Reading navigation: ");
              else
                strcpy(messagestr, "Reading swath data: ");
              lastslash = strrchr(swathfile, '/');
              if ((lastslash = strrchr(swathfile, '/')) != NULL)
                strcat(messagestr, &(lastslash[1]));
              else
                strcat(messagestr, swathfile);
              do_mbview_message_on(messagestr, instance);
              fprintf(stderr, "%s\n", messagestr);

              /* read the data */
              nfileread++;
              do_mbgrdviz_readnav(instance, swathfile, swathfilestatus, swathfileraw, swathfileprocessed, format,
                                  formatorg, weight, &error);
            }
            else
              fprintf(stderr, "Skipped xyz data: %s\n", swathfile);
          }
          else {
            mb_datalist_close(verbose, &datalist, &error);
            done = true;
          }
        }
      }
    }
    fprintf(stderr, "Attempted to load %d files, actually read %d files\n", nfiledatalist, nfileread);

    /* update widgets */
    mbview_enableviewnavs(verbose, instance, &error);
    status = mbview_update(verbose, instance, &error);
  }

  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_readnav(size_t instance, char *swathfile, int pathstatus, char *pathraw, char *pathprocessed, int format,
                        int formatorg, double weight, int *error) {
  int status = MB_SUCCESS;
  char *error_message;

  /* MBIO control parameters */
  int pings = 1;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double btime_d;
  double etime_d;
  double speedmin;
  double timegap;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  void *mbio_ptr = NULL;

  /* mbio read and write values */
  void *store_ptr = NULL;
  int kind;
  int time_i[7];
  double time_d;
  double lon;
  double lat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  char *beamflag = NULL;
  double *bath = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *amp = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  char comment[MB_COMMENT_MAXLINE];

  int npoint;
  int npointread;
  int npointalloc;
  double *navtime_d = NULL;
  double *navlon = NULL;
  double *navlat = NULL;
  double *navz = NULL;
  double *navheading = NULL;
  double *navspeed = NULL;
  double *navportlon = NULL;
  double *navportlat = NULL;
  double *navstbdlon = NULL;
  double *navstbdlat = NULL;
  unsigned int *navline = NULL;
  unsigned int *navshot = NULL;
  unsigned int *navcdp = NULL;
  int color;
  int size;
  mb_path name;
  bool swathbounds;
  int line;
  int shot;
  int cdp;
  int decimation;

  struct mbview_struct *data;

  double mtodeglon, mtodeglat;
  double headingx, headingy;
  double xd, yd, zd;

  double cellsize;
  double distancealongtrack;

  int form;
  int icenter, iport, istbd;
  double centerdistance, portdistance, stbddistance;
  char *lastslash;
  int i;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       swathfile:       %s\n", swathfile);
    fprintf(stderr, "dbg2       pathstatus:      %d\n", pathstatus);
    fprintf(stderr, "dbg2       pathraw:         %s\n", pathraw);
    fprintf(stderr, "dbg2       pathprocessed:   %s\n", pathprocessed);
    fprintf(stderr, "dbg2       format:          %d\n", format);
    fprintf(stderr, "dbg2       formatorg:       %d\n", formatorg);
    fprintf(stderr, "dbg2       weight:          %f\n", weight);
  }

  *error = MB_ERROR_NO_ERROR;

  /* initialize nav values */
  color = MBV_COLOR_BLACK;
  size = 2;
  name[0] = '\0';
  lastslash = strrchr(swathfile, '/');
  if ((lastslash = strrchr(swathfile, '/')) != NULL)
    strcpy(name, &(lastslash[1]));
  else
    strcpy(name, swathfile);

  swathbounds = false;
  line = false;
  shot = true;
  cdp = false;
  npoint = 0;
  npointread = 0;
  npointalloc = 0;
  distancealongtrack = 0.0;

  /* set mbio default values */
  status = mb_defaults(verbose, &form, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  /* get data structure of current instance */
  status = mbview_getdataptr(verbose, instance, &data, error);
  if (status == MB_SUCCESS) {
    bounds[0] = data->primary_xmin;
    bounds[1] = data->primary_xmax;
    bounds[2] = data->primary_ymin;
    bounds[3] = data->primary_ymax;
    status = mbview_projectforward(instance, true, data->primary_xmin, data->primary_ymin,
                                   0.5 * (data->primary_min + data->primary_max), &bounds[0], &bounds[2], &xd, &yd, &zd);
    status = mbview_projectforward(instance, true, data->primary_xmax, data->primary_ymax,
                                   0.5 * (data->primary_min + data->primary_max), &bounds[1], &bounds[3], &xd, &yd, &zd);
    mb_coor_scale(verbose, 0.5 * (bounds[2] + bounds[3]), &mtodeglon, &mtodeglat);
    cellsize = 0.0005 * (((bounds[3] - bounds[2]) / ((double)data->primary_n_rows) / mtodeglat) +
                         ((bounds[1] - bounds[0]) / ((double)data->primary_n_columns) / mtodeglon));
  }

  /* rationalize bounds and lonflip */
  if (bounds[1] > 180.0) {
    lonflip = 1;
  }
  else if (bounds[0] < -180.0) {
    lonflip = -1;
  }
  else {
    lonflip = 0;
  }

  /* initialize reading the swath file */
  if ((status = mb_read_init(verbose, swathfile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
                             &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, error)) != MB_SUCCESS) {
    mb_error(verbose, *error, &error_message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", error_message);
    fprintf(stderr, "\nSwath sonar File <%s> not initialized for reading\n", swathfile);
  }
  /* allocate memory for data arrays */
  if (status == MB_SUCCESS) {
    status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&beamflag, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathacrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathalongtrack, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ssacrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ssalongtrack, error);

    /* if error initializing memory then don't read the file */
    if (*error != MB_ERROR_NO_ERROR) {
      mb_error(verbose, *error, &error_message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
    }
  }

  /* read data */
  if (status == MB_SUCCESS) {
    /* set swathbounds true if nore than one beam is expected */
    if (beams_bath > 1)
      swathbounds = true;

    /* enable line and cdp values if segy data */
    if (format == MBF_SEGYSEGY) {
      line = true;
      cdp = true;
    }

    /* loop over successful reads and nonfatal errors
       until a fatal error is encountered */
    while (*error <= MB_ERROR_NO_ERROR) {
      /* read a ping of data */
      status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &lon, &lat, &speed, &heading, &distance,
                          &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack,
                          bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);

      /* ignore minor errors */
      if (kind == MB_DATA_DATA &&
          (*error == MB_ERROR_TIME_GAP || *error == MB_ERROR_OUT_TIME || *error == MB_ERROR_SPEED_TOO_SMALL)) {
        status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
      }

      if (kind == MB_DATA_DATA && *error == MB_ERROR_NO_ERROR) {
        /*fprintf(stderr,"Ping %d: %4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%.6.6d %f %f\n",
        npoint,time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],lon,lat);*/
        /* overwrite previous nav point if distance change does not
            exceed cell size */
        if (npoint == 0) {
          distancealongtrack = 0.0;
        }
        else if (distancealongtrack < cellsize) {
          npoint--;
          distancealongtrack += distance;
        }
        else {
          distancealongtrack = 0.0;
        }

        /* allocate memory if required */
        if (npoint >= npointalloc) {
          npointalloc += MBV_ALLOC_NUM;
          status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navtime_d, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navlon, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navlat, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navz, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navheading, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navspeed, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navportlon, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navportlat, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navstbdlon, error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(double), (void **)&navstbdlat, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(int), (void **)&navline, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(int), (void **)&navshot, error);
          if (status == MB_SUCCESS)
            status = mb_reallocd(verbose, __FILE__, __LINE__, npointalloc * sizeof(int), (void **)&navcdp, error);

          /* if error initializing memory then don't read the file */
          if (*error != MB_ERROR_NO_ERROR) {
            npointalloc = 0;
            mb_error(verbose, *error, &error_message);
            fprintf(stderr, "\nMBIO Error allocating navigation data arrays:\n%s\n", error_message);
          }
        }

        /* get swathbounds */
        if (format == MBF_MBPRONAV) {
          status = mbsys_singlebeam_swathbounds(verbose, mbio_ptr, store_ptr, &kind, &navportlon[npoint],
                                                &navportlat[npoint], &navstbdlon[npoint], &navstbdlat[npoint], error);
          if (navportlon[npoint] != navstbdlon[npoint] || navportlat[npoint] != navstbdlat[npoint])
            swathbounds = true;
        }

        else {
          /* find centermost beam */
          icenter = -1;
          iport = -1;
          istbd = -1;
          centerdistance = 0.0;
          portdistance = 0.0;
          stbddistance = 0.0;
          for (i = 0; i < beams_bath; i++) {
            if (mb_beam_ok(beamflag[i])) {
              if (icenter == -1 || fabs(bathacrosstrack[i]) < centerdistance) {
                icenter = i;
                centerdistance = bathacrosstrack[i];
              }
              if (iport == -1 || bathacrosstrack[i] < portdistance) {
                iport = i;
                portdistance = bathacrosstrack[i];
              }
              if (istbd == -1 || bathacrosstrack[i] > stbddistance) {
                istbd = i;
                stbddistance = bathacrosstrack[i];
              }
            }
          }

          mb_coor_scale(verbose, lat, &mtodeglon, &mtodeglat);
          headingx = sin(heading * DTR);
          headingy = cos(heading * DTR);
          if (icenter >= 0) {
            navportlon[npoint] =
                lon + headingy * mtodeglon * bathacrosstrack[iport] + headingx * mtodeglon * bathalongtrack[iport];
            navportlat[npoint] =
                lat - headingx * mtodeglat * bathacrosstrack[iport] + headingy * mtodeglat * bathalongtrack[iport];
            navstbdlon[npoint] =
                lon + headingy * mtodeglon * bathacrosstrack[istbd] + headingx * mtodeglon * bathalongtrack[istbd];
            navstbdlat[npoint] =
                lat - headingx * mtodeglat * bathacrosstrack[istbd] + headingy * mtodeglat * bathalongtrack[istbd];
          }
          else {
            navportlon[npoint] = lon;
            navportlat[npoint] = lat;
            navstbdlon[npoint] = lon;
            navstbdlat[npoint] = lat;
          }
        }

        /* store the navigation values */
        navtime_d[npoint] = time_d;
        navlon[npoint] = lon;
        navlat[npoint] = lat;
        navz[npoint] = -sensordepth;
        navheading[npoint] = heading;
        navspeed[npoint] = speed;

        mb_segynumber(verbose, mbio_ptr, &(navline[npoint]), &(navshot[npoint]), &(navcdp[npoint]), error);

        /* increment npoint */
        npoint++;
        npointread++;
      }
    }

    /* close the swath file */
    status = mb_close(verbose, &mbio_ptr, error);

    /* insert nav data to mbview */
    if (npoint > 0) {
      decimation = npointread / npoint;
      status = mbview_addnav(verbose, instance, npoint, navtime_d, navlon, navlat, navz, navheading, navspeed, navportlon,
                             navportlat, navstbdlon, navstbdlat, navline, navshot, navcdp, color, size, name, pathstatus,
                             pathraw, pathprocessed, formatorg, swathbounds, line, shot, cdp, decimation, error);
    }
    else
      fprintf(stderr, "    Skipping %s because of 0 nav points read\n", name);

    /* deallocate memory used for data arrays */
    mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bathacrosstrack, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bathalongtrack, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ssacrosstrack, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ssalongtrack, error);

    mb_freed(verbose, __FILE__, __LINE__, (void **)&navtime_d, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navlon, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navlat, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navz, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navheading, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navspeed, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navportlon, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navportlat, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navstbdlon, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navstbdlat, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navline, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navshot, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&navcdp, error);
  }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

int do_mbgrdviz_opentest(size_t instance, double factor1, double factor2, double factor3, int *grid_projection_mode,
                         char *grid_projection_id, float *nodatavalue, int *nxy, int *n_columns, int *n_rows, double *min, double *max,
                         double *xmin, double *xmax, double *ymin, double *ymax, double *dx, double *dy, float **data) {
  int status = MB_SUCCESS;
  double xx, yy;
  float *usedata;
  int i, j, k;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:        %zu\n", instance);
    fprintf(stderr, "dbg2       factor1:         %f\n", factor1);
    fprintf(stderr, "dbg2       factor2:         %f\n", factor2);
    fprintf(stderr, "dbg2       factor3:         %f\n", factor3);
  }

  *grid_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
  sprintf(grid_projection_id, "EPSG:%d", GCS_WGS_84);
  *nodatavalue = MBV_DEFAULT_NODATA;
  *n_columns = 501;
  *n_rows = 501;
  *nxy = *n_columns * *n_rows;
  *xmin = -1.0;
  *xmax = 1.0;
  *ymin = -1.0;
  *ymax = 1.0;
  *dx = (*xmax - *xmin) / (*n_columns - 1);
  *dy = (*ymax - *ymin) / (*n_rows - 1);
  *min = 0.0;
  *max = 1000.0;
  *min = 0.0;
  *max = 0.0;

  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)data, &error);
  usedata = *data;
  if (status != MB_SUCCESS) {
    fprintf(stderr, "\nUnable to allocate memory to store test data...\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }

  for (i = 0; i < *n_columns; i++)
    for (j = 0; j < *n_rows; j++) {
      k = i * *n_rows + j;
      xx = *xmin + i * *dx;
      yy = *ymin + j * *dy;
      usedata[k] = factor1 * sin(factor2 * M_PI * xx) * sin(factor2 * M_PI * yy) * exp(-factor3 * xx * yy);
      *min = MIN(*min, usedata[k]);
      *max = MAX(*max, usedata[k]);
    }

  /* all done */
  return (status);
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_open_region(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;
  int ixmin, ixmax, jymin, jymax;
  int i, j, k, ksource;

  /* existing mbview instance */
  size_t instance_source;
  struct mbview_struct *data_source;
  char button_name_source[MB_PATH_MAXLINE];

  /* new mbview instance */
  size_t instance;
  char button_name[MB_PATH_MAXLINE+12];

  /* mbview parameters */
  char mbv_title[MB_PATH_MAXLINE+25];
  int mbv_xo;
  int mbv_yo;
  int mbv_width;
  int mbv_height;
  int mbv_lorez_dimension;
  int mbv_hirez_dimension;
  int mbv_lorez_navdecimate;
  int mbv_hirez_navdecimate;
  int mbv_primary_nxy;
  int mbv_primary_n_columns;
  int mbv_primary_n_rows;
  double mbv_primary_min;
  double mbv_primary_max;
  double mbv_primary_xmin;
  double mbv_primary_xmax;
  double mbv_primary_ymin;
  double mbv_primary_ymax;
  double mbv_primary_dx;
  double mbv_primary_dy;
  float *mbv_primary_data;
  int mbv_secondary_nxy;
  int mbv_secondary_n_columns;
  int mbv_secondary_n_rows;
  double mbv_secondary_min;
  double mbv_secondary_max;
  double mbv_secondary_xmin;
  double mbv_secondary_xmax;
  double mbv_secondary_ymin;
  double mbv_secondary_ymax;
  double mbv_secondary_dx;
  double mbv_secondary_dy;
  float *mbv_secondary_data;

  /* get source mbview instance */
  instance_source = (size_t)client_data;
  /*fprintf(stderr,"Called do_mbgrdviz_open_region instance:%d\n", instance_source);*/

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* get new instance number */
  if (instance_source != MBV_NO_WINDOW && instance_source < MBV_MAX_WINDOWS) {
    status = mbview_init(verbose, &instance, &error);
    if (instance == MBV_NO_WINDOW) {
      fprintf(stderr, "Unable to create mbview - %d mbview windows already created\n", MBV_MAX_WINDOWS);
      status = MB_FAILURE;
    }
  }
  else {
    status = MB_FAILURE;
  }

  /* check data source for region to extract */
  if (status == MB_SUCCESS) {
    /* get source data */
    mbview_getdataptr(verbose, instance_source, &data_source, &error);

    /* extract the grid from the source */
    if (data_source->region_type != MBV_REGION_QUAD)
      status = MB_FAILURE;
  }

  /* extract data from source and create new mbview instance */
  if (status == MB_SUCCESS) {
    /* get source data */
    mbview_getdataptr(verbose, instance_source, &data_source, &error);

    /* get button name */
    sscanf(data_source->title, "MBgrdviz: %s", button_name_source);
    sprintf(button_name, "Region from %s", button_name_source);

    /* set parameters */
    sprintf(mbv_title, "MBgrdviz: %s\n", button_name);
    mbv_xo = 200;
    mbv_yo = 200;
    mbv_width = 560;
    mbv_height = 500;
    mbv_lorez_dimension = data_source->lorez_dimension;
    mbv_hirez_dimension = data_source->hirez_dimension;
    mbv_lorez_navdecimate = data_source->lorez_navdecimate;
    mbv_hirez_navdecimate = data_source->hirez_navdecimate;

    /* set basic mbview window parameters */
    status = mbview_setwindowparms(verbose, instance, &do_mbgrdviz_dismiss_notify, mbv_title, mbv_xo, mbv_yo, mbv_width,
                                   mbv_height, mbv_lorez_dimension, mbv_hirez_dimension, mbv_lorez_navdecimate,
                                   mbv_hirez_navdecimate, &error);

    /* extract the primary grid from the source */
    mbv_primary_dx = data_source->primary_dx;
    mbv_primary_dy = data_source->primary_dy;
    mbv_primary_xmin = MIN(data_source->region.cornerpoints[0].xgrid, data_source->region.cornerpoints[3].xgrid);
    mbv_primary_xmax = MAX(data_source->region.cornerpoints[0].xgrid, data_source->region.cornerpoints[3].xgrid);
    mbv_primary_ymin = MIN(data_source->region.cornerpoints[0].ygrid, data_source->region.cornerpoints[3].ygrid);
    mbv_primary_ymax = MAX(data_source->region.cornerpoints[0].ygrid, data_source->region.cornerpoints[3].ygrid);
    ixmin = (mbv_primary_xmin - data_source->primary_xmin) / mbv_primary_dx;
    ixmax = ((mbv_primary_xmax - data_source->primary_xmin) / mbv_primary_dx) + 1;
    jymin = (mbv_primary_ymin - data_source->primary_ymin) / mbv_primary_dy;
    jymax = ((mbv_primary_ymax - data_source->primary_ymin) / mbv_primary_dy) + 1;
    ixmin = MAX(ixmin, 0);
    ixmax = MIN(ixmax, data_source->primary_n_columns - 1);
    jymin = MAX(jymin, 0);
    jymax = MIN(jymax, data_source->primary_n_rows - 1);
    mbv_primary_xmin = data_source->primary_xmin + mbv_primary_dx * ixmin;
    mbv_primary_xmax = data_source->primary_xmin + mbv_primary_dx * ixmax;
    mbv_primary_ymin = data_source->primary_ymin + mbv_primary_dy * jymin;
    mbv_primary_ymax = data_source->primary_ymin + mbv_primary_dy * jymax;
    mbv_primary_n_columns = ixmax - ixmin + 1;
    mbv_primary_n_rows = jymax - jymin + 1;
    mbv_primary_nxy = mbv_primary_n_columns * mbv_primary_n_rows;
    status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * mbv_primary_nxy, (void **)&mbv_primary_data, &error);
    mbv_primary_min = data_source->primary_nodatavalue;
    mbv_primary_max = data_source->primary_nodatavalue;
    for (i = 0; i < mbv_primary_n_columns; i++) {
      for (j = 0; j < mbv_primary_n_rows; j++) {
        k = i * mbv_primary_n_rows + j;
        ksource = (i + ixmin) * data_source->primary_n_rows + (j + jymin);
        mbv_primary_data[k] = data_source->primary_data[ksource];
        if (mbv_primary_data[k] != data_source->primary_nodatavalue) {
          if (mbv_primary_min == data_source->primary_nodatavalue || mbv_primary_data[k] < mbv_primary_min) {
            mbv_primary_min = mbv_primary_data[k];
          }
          if (mbv_primary_max == data_source->primary_nodatavalue || mbv_primary_data[k] > mbv_primary_max) {
            mbv_primary_max = mbv_primary_data[k];
          }
        }
      }
    }

    /* set basic mbview view controls */
    if (status == MB_SUCCESS)
      status = mbview_setviewcontrols(
          verbose, instance, data_source->display_mode, data_source->mouse_mode, data_source->grid_mode,
          data_source->primary_histogram, data_source->primaryslope_histogram, data_source->secondary_histogram,
          data_source->primary_shade_mode, data_source->slope_shade_mode, data_source->secondary_shade_mode,
          data_source->grid_contour_mode, data_source->site_view_mode, data_source->route_view_mode,
          data_source->nav_view_mode, data_source->navdrape_view_mode, data_source->vector_view_mode,
          data_source->exageration, data_source->modelelevation3d, data_source->modelazimuth3d,
          data_source->viewelevation3d, data_source->viewazimuth3d, data_source->illuminate_magnitude,
          data_source->illuminate_elevation, data_source->illuminate_azimuth, data_source->slope_magnitude,
          data_source->overlay_shade_magnitude, data_source->overlay_shade_center, data_source->overlay_shade_mode,
          data_source->contour_interval, data_source->display_projection_mode, data_source->display_projection_id, &error);

    /* set more mbview control values */
    if (status == MB_SUCCESS)
      status = mbview_setprimarygrid(verbose, instance, data_source->primary_grid_projection_mode,
                                     data_source->primary_grid_projection_id, data_source->primary_nodatavalue,
                                     mbv_primary_n_columns, mbv_primary_n_rows, mbv_primary_min, mbv_primary_max, mbv_primary_xmin,
                                     mbv_primary_xmax, mbv_primary_ymin, mbv_primary_ymax, mbv_primary_dx, mbv_primary_dy,
                                     mbv_primary_data, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&mbv_primary_data, &error);
    if (status == MB_SUCCESS)
      status = mbview_setprimarycolortable(verbose, instance, data_source->primary_colortable,
                                           data_source->primary_colortable_mode, data_source->primary_colortable_min,
                                           data_source->primary_colortable_max, &error);
    if (status == MB_SUCCESS)
      status =
          mbview_setslopecolortable(verbose, instance, data_source->slope_colortable, data_source->slope_colortable_mode,
                                    data_source->slope_colortable_min, data_source->slope_colortable_max, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableeditsites(verbose, instance, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableeditroutes(verbose, instance, &error);

    /* open up mbview window */
    if (status == MB_SUCCESS) {
      /*fprintf(stderr,"about to open mbview instance:%zu\n",instance);*/
      status = mbview_open(verbose, instance, &error);
      if (status == MB_SUCCESS)
        mbview_id[instance] = true;
      else
        mbview_id[instance] = false;
      /*fprintf(stderr,"done opening mbview instance:%zu\n",instance);*/

      /* add action button */
      if (status == MB_SUCCESS) {
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openoverlay, "Open Overlay Grid",
                         MBV_PICKMASK_NONE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opensite, "Open Site File", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openroute, "Open Route File", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openvector, "Open Vector File",
                         MBV_PICKMASK_NONE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opennav, "Open Navigation", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openswath, "Open Swath Data", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savesite, "Save Site File", MBV_EXISTMASK_SITE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savesitewpt, "Save Sites as Winfrog WPT File", MBV_EXISTMASK_SITE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveroute, "Save Route File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptheading, "Save Risi Script File (variable heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading, "Save Risi Script File (static heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savewinfrogpts, "Save Route as Winfrog PTS File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savewinfrogwpt, "Save Route as Winfrog WPT File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savedegdecmin,
                         "Save Route as Degrees + Decimal Minutes File", MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savelnw, "Save Route as Hypack LNW File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveprofile, "Save Profile File",
                         MBV_PICKMASK_TWOPOINT + MBV_PICKMASK_ROUTE + MBV_PICKMASK_NAVTWOPOINT, &error);

        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbedit, "Open Selected Nav in MBedit", MBV_PICKMASK_NAVANY,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbeditviz, "Open Selected Nav in MBeditviz",
                         MBV_PICKMASK_NAVANY, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbnavedit, "Open Selected Nav in MBnavedit",
                         MBV_PICKMASK_NAVANY, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_open_mbvelocitytool, "Open Selected Nav in MBvelocitytool",
                         MBV_PICKMASK_NAVANY, &error);

        mbview_addaction(verbose, instance, do_mbgrdviz_open_region, "Open Region as New View",
                         MBV_PICKMASK_REGION + MBV_PICKMASK_NEWINSTANCE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_make_survey, "Generate Survey Route from Area", MBV_PICKMASK_AREA,
                         &error);
      }
    }

    /* extract the secondary grid, if it exists, from the source */
    if (data_source->secondary_nxy > 0 && data_source->secondary_data != NULL) {
      mbv_secondary_dx = data_source->secondary_dx;
      mbv_secondary_dy = data_source->secondary_dy;
      mbv_secondary_xmin = MIN(data_source->region.cornerpoints[0].xgrid, data_source->region.cornerpoints[3].xgrid);
      mbv_secondary_xmax = MAX(data_source->region.cornerpoints[0].xgrid, data_source->region.cornerpoints[3].xgrid);
      mbv_secondary_ymin = MIN(data_source->region.cornerpoints[0].ygrid, data_source->region.cornerpoints[3].ygrid);
      mbv_secondary_ymax = MAX(data_source->region.cornerpoints[0].ygrid, data_source->region.cornerpoints[3].ygrid);
      ixmin = (mbv_secondary_xmin - data_source->secondary_xmin) / mbv_secondary_dx;
      ixmax = ((mbv_secondary_xmax - data_source->secondary_xmin) / mbv_secondary_dx) + 1;
      jymin = (mbv_secondary_ymin - data_source->secondary_ymin) / mbv_secondary_dy;
      jymax = ((mbv_secondary_ymax - data_source->secondary_ymin) / mbv_secondary_dy) + 1;
      ixmin = MAX(ixmin, 0);
      ixmax = MIN(ixmax, data_source->secondary_n_columns - 1);
      jymin = MAX(jymin, 0);
      jymax = MIN(jymax, data_source->secondary_n_rows - 1);
      mbv_secondary_xmin = data_source->secondary_xmin + mbv_secondary_dx * ixmin;
      mbv_secondary_xmax = data_source->secondary_xmin + mbv_secondary_dx * ixmax;
      mbv_secondary_ymin = data_source->secondary_ymin + mbv_secondary_dy * jymin;
      mbv_secondary_ymax = data_source->secondary_ymin + mbv_secondary_dy * jymax;
      mbv_secondary_n_columns = ixmax - ixmin + 1;
      mbv_secondary_n_rows = jymax - jymin + 1;
      mbv_secondary_nxy = mbv_secondary_n_columns * mbv_secondary_n_rows;
      status =
          mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * mbv_secondary_nxy, (void **)&mbv_secondary_data, &error);
      mbv_secondary_min = data_source->secondary_nodatavalue;
      mbv_secondary_max = data_source->secondary_nodatavalue;
      for (i = 0; i < mbv_secondary_n_columns; i++) {
        for (j = 0; j < mbv_secondary_n_rows; j++) {
          k = i * mbv_secondary_n_rows + j;
          ksource = (i + ixmin) * data_source->secondary_n_rows + (j + jymin);
          mbv_secondary_data[k] = data_source->secondary_data[ksource];
          if (mbv_secondary_data[k] != data_source->secondary_nodatavalue) {
            if (mbv_secondary_min == data_source->secondary_nodatavalue ||
                mbv_secondary_data[k] < mbv_secondary_min) {
              mbv_secondary_min = mbv_secondary_data[k];
            }
            if (mbv_secondary_max == data_source->secondary_nodatavalue ||
                mbv_secondary_data[k] > mbv_secondary_max) {
              mbv_secondary_max = mbv_secondary_data[k];
            }
          }
        }
      }

      /* set more mbview control values */
      if (status == MB_SUCCESS)
        status = mbview_setsecondarygrid(verbose, instance, data_source->secondary_grid_projection_mode,
                                         data_source->secondary_grid_projection_id, data_source->secondary_nodatavalue,
                                         mbv_secondary_n_columns, mbv_secondary_n_rows, mbv_secondary_min, mbv_secondary_max,
                                         mbv_secondary_xmin, mbv_secondary_xmax, mbv_secondary_ymin, mbv_secondary_ymax,
                                         mbv_secondary_dx, mbv_secondary_dy, mbv_secondary_data, &error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&mbv_secondary_data, &error);
      if (status == MB_SUCCESS)
        status =
            mbview_setsecondarycolortable(verbose, instance, data_source->secondary_colortable,
                                          data_source->secondary_colortable_mode, data_source->secondary_colortable_min,
                                          data_source->secondary_colortable_max, data_source->overlay_shade_magnitude,
                                          data_source->overlay_shade_center, data_source->overlay_shade_mode, &error);
    }
  }

  /* update widgets */
  status = mbview_update(verbose, instance, &error);

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_open_mbedit(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
  char mbedit_cmd[1030];
  char filearg[1050];
  int nselected;

  /* get source mbview instance */
  instance = (size_t)client_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* getting instance from client_data doesn't seem
      to work so use survey_instance instead */
  instance = survey_instance;
  fprintf(stderr, "Called do_mbgrdviz_open_mbedit instance:%zu\n", instance);

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);
  status = mbview_getsharedptr(verbose, &shareddata, &error);

  /* check if any nav is selected */
  nselected = 0;
  sprintf(mbedit_cmd, "mbedit");
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        sprintf(filearg, " -F%d -I%s", nav->format, nav->pathraw);
        strncat(mbedit_cmd, filearg, MB_PATH_MAXLINE - 3);
        nselected += nav->nselected;
        fprintf(stderr, "nselected: %d %d    Adding filearg:%s\n", nav->nselected, nselected, filearg);
      }
    }
  }

  /* open all data files with selected nav into mbedit */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    strncat(mbedit_cmd, " &", MB_PATH_MAXLINE);
    fprintf(stderr, "Calling mbedit: %s\n", mbedit_cmd);
    /* const int shellstatus = */ system(mbedit_cmd);
  }

  /* update widgets of all mbview windows */
  status = mbview_update(verbose, instance, &error);
  for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (i != instance && mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
  char mbeditviz_cmd[1050];
  mb_path datalist_file;
  FILE *dfp;
  int nselected;

  /* get source mbview instance */
  instance = (size_t)client_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* getting instance from client_data doesn't seem
      to work so use survey_instance instead */
  instance = survey_instance;
  fprintf(stderr, "Called do_mbgrdviz_open_mbeditviz instance:%zu\n", instance);

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);
  status = mbview_getsharedptr(verbose, &shareddata, &error);

  /* check if any nav is selected */
  nselected = 0;
  sprintf(mbeditviz_cmd, "mbeditviz");
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      nselected += nav->nselected;
    }
    if(nselected > 0) {
      sprintf(datalist_file,"tmp_datalist_%d.mb-1", getpid());
      dfp = fopen(datalist_file, "w");
      if (dfp != NULL) {
        for (int i = 0; i < shareddata->nnav; i++) {
          nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
          if (nav->nselected > 0) {
            fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n",
                            i, nav->name, nav->pathraw, nav->format, nav->nselected);
            fprintf(dfp, "%s %d\n", nav->pathraw, nav->format);
          }
        }
        fclose(dfp);
        sprintf(mbeditviz_cmd, "mbeditviz -I%s -R &", datalist_file);
        fprintf(stderr, "Calling mbeditviz: %s\n", mbeditviz_cmd);
        /* const int shellstatus = */ system(mbeditviz_cmd);
      }
    }
  }

  /* update widgets of all mbview windows */
  status = mbview_update(verbose, instance, &error);
  for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (i != instance && mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_open_mbnavedit(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
  char mbnavedit_cmd[1050];
  char filearg[1050];
  int nselected;

  /* get source mbview instance */
  instance = (size_t)client_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* getting instance from client_data doesn't seem
      to work so use survey_instance instead */
  instance = survey_instance;
  fprintf(stderr, "Called do_mbgrdviz_open_mbnavedit instance:%zu\n", instance);

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);
  status = mbview_getsharedptr(verbose, &shareddata, &error);

  /* check if any nav is selected */
  nselected = 0;
  sprintf(mbnavedit_cmd, "mbnavedit");
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        sprintf(filearg, " -F%d -I%s", nav->format, nav->pathraw);
        strncat(mbnavedit_cmd, filearg, MB_PATH_MAXLINE - 3);
        nselected += nav->nselected;
        fprintf(stderr, "nselected: %d %d    Adding filearg:%s\n", nav->nselected, nselected, filearg);
      }
    }
  }

  /* open all data files with selected nav into mbnavedit */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    strncat(mbnavedit_cmd, " &", MB_PATH_MAXLINE);
    fprintf(stderr, "Calling mbnavedit: %s\n", mbnavedit_cmd);
    /* const int shellstatus = */ system(mbnavedit_cmd);
  }

  /* update widgets of all mbview windows */
  status = mbview_update(verbose, instance, &error);
  for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (i != instance && mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_open_mbvelocitytool(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
  char mbvelocitytool_cmd[1050];
  char filearg[1050];
  int nselected;

  /* get source mbview instance */
  instance = (size_t)client_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* getting instance from client_data doesn't seem
      to work so use survey_instance instead */
  instance = survey_instance;
  fprintf(stderr, "Called do_mbgrdviz_open_mbvelocitytool instance:%zu\n", instance);

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);
  status = mbview_getsharedptr(verbose, &shareddata, &error);

  /* check if any nav is selected */
  nselected = 0;
  sprintf(mbvelocitytool_cmd, "mbvelocitytool");
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        sprintf(filearg, " -F%d -I%s", nav->format, nav->pathraw);
        strncat(mbvelocitytool_cmd, filearg, MB_PATH_MAXLINE - 3);
        nselected += nav->nselected;
        fprintf(stderr, "nselected: %d %d    Adding filearg:%s\n", nav->nselected, nselected, filearg);
      }
    }
  }

  /* open all data files with selected nav into mbvelocitytool */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    strncat(mbvelocitytool_cmd, " &", MB_PATH_MAXLINE);
    fprintf(stderr, "Calling mbvelocitytool: %s\n", mbvelocitytool_cmd);
    /* const int shellstatus = */ system(mbvelocitytool_cmd);
  }

  /* update widgets of all mbview windows */
  status = mbview_update(verbose, instance, &error);
  for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (i != instance && mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_make_survey(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  Cardinal ac = 0;
  Arg args[256];
  size_t instance;
  struct mbview_struct *data;

  /* get source mbview instance */
  instance = (size_t)client_data;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);

  /* check if area is currently defined */
  if (status == MB_SUCCESS) {
    if (data->area_type != MBV_AREA_QUAD)
      status = MB_FAILURE;
  }

  /* set parameters and display the survey generation dialog */
  if (status == MB_SUCCESS) {

    /* set title to open primary grid */
    ac = 0;
    XtSetArg(args[ac], XmNtitle, "Generate Survey Lines from Area");
    ac++;
    XtSetValues(bulletinBoard_arearoute, args, ac);

    /* set instance into XmNuserData resources */
    ac = 0;
    XtSetArg(args[ac], XmNuserData, (XtPointer)instance);
    ac++;
    XtSetValues(bulletinBoard_arearoute, args, ac);
    XtSetValues(textField_arearoute_name, args, ac);
    XtSetValues(spinBox_arearoute_color, args, ac);
    XtSetValues(spinText_arearoute_color, args, ac);
    XtSetValues(spinBox_arearoute_crosslines, args, ac);
    XtSetValues(spinText_arearoute_crosslines, args, ac);
    XtSetValues(spinBox_arearoute_altitude, args, ac);
    XtSetValues(spinText_arearoute_altitude, args, ac);
    XtSetValues(spinBox_arearoute_depth, args, ac);
    XtSetValues(spinText_arearoute_depth, args, ac);
    XtSetValues(spinBox_arearoute_direction, args, ac);
    XtSetValues(spinText_arearoute_direction, args, ac);
    XtSetValues(spinBox_arearoute_swathwidth, args, ac);
    XtSetValues(spinText_arearoute_swathwidth, args, ac);
    XtSetValues(spinBox_arearoute_platform, args, ac);
    XtSetValues(spinText_arearoute_platform, args, ac);
    XtSetValues(spinBox_arearoute_linespacing, args, ac);
    XtSetValues(spinText_arearoute_linespacing, args, ac);
    XtSetValues(spinBox_arearoute_crosslinesfirstlast, args, ac);
    XtSetValues(spinText_arearoute_crosslinesfirstlast, args, ac);
    XtSetValues(spinBox_arearoute_interleaving, args, ac);
    XtSetValues(spinText_arearoute_interleaving, args, ac);
    XtSetValues(label_arearoute_info, args, ac);
    XtSetValues(spinBox_arearoute_linecontrol, args, ac);
    XtSetValues(spinText_arearoute_linecontrol, args, ac);
    XtSetValues(pushButton_arearoute_ok, args, ac);

    /* setting instance into XmNuserData resources
        doesn't seem to work, so set survey_instance as well */
    survey_instance = instance;
  }

  /* set widgets */
  if (status == MB_SUCCESS) {
    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_mode);
    ac++;
    XtSetValues(spinText_arearoute_linecontrol, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_direction);
    ac++;
    XtSetValues(spinText_arearoute_direction, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_crosslines);
    ac++;
    XtSetValues(spinText_arearoute_crosslines, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_crosslines_last);
    ac++;
    XtSetValues(spinText_arearoute_crosslinesfirstlast, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_interleaving);
    ac++;
    XtSetValues(spinText_arearoute_interleaving, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_color);
    ac++;
    XtSetValues(spinText_arearoute_color, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_linespacing);
    ac++;
    XtSetValues(spinText_arearoute_linespacing, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_platform);
    ac++;
    XtSetValues(spinText_arearoute_platform, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_swathwidth);
    ac++;
    XtSetValues(spinText_arearoute_swathwidth, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_altitude);
    ac++;
    XtSetValues(spinText_arearoute_altitude, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, survey_depth);
    ac++;
    XtSetValues(spinText_arearoute_depth, args, ac);

    XmTextSetString(textField_arearoute_name, survey_name);

    do_mbgrdviz_arearoute_recalc(instance);

    /* put up the dialog */
    BxManageCB(w, (XtPointer) "bulletinBoard_arearoute", call_data);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_generate_survey(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  struct mbview_struct *data;

  /* survey construction parameters */
  int color;
  double line_spacing;
  double line_spacing_use;
  double crossline_spacing;
  double sonar_depth;
  double sonar_altitude;
  double maxtopo;
  struct mbview_linesegment_struct segment;
  int nlines;
  int nlinegroups, npoints;
  double xgrid, ygrid;
  double xlon, ylat, zdata;
  double xdisplay, ydisplay, zdisplay;
  double dsign;
  int waypoint;
  bool first;
  double dsigna[4] = {1.0, -1.0, 1.0, -1.0};
  int jendpointa[4] = {0, 0, 1, 1};

  char *error_message;
  double *xx = NULL;
  double dx, dy, r, dxuse, dyuse, dxd, dyd, dxextra, dyextra;
  double rrr[4], xxx, yyy;
  int iline, jendpoint;
  bool ok;
  int startcorner, endcorner, jstart, kend;
  int nlines_alloc = 0;

  /* get source mbview instance */
  size_t instance = (size_t)client_data;

  /* getting instance from client_data doesn't seem
      to work so use survey_instance instead */
  instance = survey_instance;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* check data source for area to bounding desired survey */
  status = mbview_getdataptr(verbose, instance, &data, &error);

  /* check if area is currently defined */
  if (status == MB_SUCCESS) {
    if (data->area_type != MBV_AREA_QUAD)
      status = MB_FAILURE;
  }

  /* generate survey lines from area and add as new route */
  if (status == MB_SUCCESS) {
    /* delete current working route if defined */
    if (working_route > -1) {
      mbview_deleteroute(verbose, instance, working_route, &error);
      working_route = -1;
    }

    /* get unit vector for survey area boundaries */
    dx = data->area.cornerpoints[1].xdisplay - data->area.cornerpoints[0].xdisplay;
    dy = data->area.cornerpoints[1].ydisplay - data->area.cornerpoints[0].ydisplay;
    r = sqrt(dx * dx + dy * dy);
    dx = dx / r;
    dy = dy / r;

    /* get parameters */
    int k = 0;
    if (data->area.bearing >= 315.0 || data->area.bearing < 45.0) {
      if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SW)
        k = 0;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SE)
        k = 1;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NW)
        k = 2;
      else /* if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NE) */
        k = 3;
    }
    else if (data->area.bearing >= 45.0 && data->area.bearing < 135.0) {
      if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SW)
        k = 1;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SE)
        k = 3;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NW)
        k = 0;
      else /* if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NE) */
        k = 2;
    }
    else if (data->area.bearing >= 135.0 && data->area.bearing < 225.0) {
      if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SW)
        k = 3;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SE)
        k = 2;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NW)
        k = 1;
      else /* if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NE) */
        k = 0;
    }
    else /* if (data->area.bearing >= 225.0 && data->area.bearing < 315.0) */
    {
      if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SW)
        k = 2;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_SE)
        k = 0;
      else if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NW)
        k = 3;
      else /* if (survey_direction == MBGRDVIZ_SURVEY_DIRECTION_NE) */
        k = 1;
    }
    dsign = dsigna[k];
    jendpoint = jendpointa[k];
    if (survey_color == 0)
      color = MBV_COLOR_BLACK;
    else if (survey_color == 1)
      color = MBV_COLOR_YELLOW;
    else if (survey_color == 2)
      color = MBV_COLOR_GREEN;
    else if (survey_color == 3)
      color = MBV_COLOR_BLUEGREEN;
    else if (survey_color == 4)
      color = MBV_COLOR_BLUE;
    else if (survey_color == 5)
      color = MBV_COLOR_PURPLE;

    /* initialize number of waypoints */
    npoints = 0;
    first = true;

    /* do uniform line spacing */
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_UNIFORM) {
      /* get number of lines */
      line_spacing = (double)survey_linespacing;
      line_spacing_use = line_spacing * r / data->area.width;
      nlines = (data->area.width / line_spacing) + 1;

      /* allocate space for line position array */
      status = mb_mallocd(verbose, __FILE__, __LINE__, nlines * sizeof(double), (void **)&xx, &error);
      if (status != MB_SUCCESS) {
        nlines_alloc = 0;
        mb_error(verbose, error, &error_message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
      }
      else
        nlines_alloc = nlines;

      /* calculate line positions */
      if (status == MB_SUCCESS)
        for (int i = 0; i < nlines; i++) {
          /* get line position in survey area */
          xx[i] = dsign * line_spacing_use * (((double)i) - 0.5 * (nlines - 1.0));
        }
    }

    /* do variable line spacing with constant altitude */
    else if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE && survey_platform == MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_ALTITUDE) {
      /* get number of lines */
      line_spacing = (double)survey_altitude * 2.0 * tan(DTR * 0.5 * (double)survey_swathwidth);
      line_spacing_use = line_spacing * r / data->area.width;
      nlines = (data->area.width / line_spacing) + 1;

      /* allocate space for line position array */
      status = mb_mallocd(verbose, __FILE__, __LINE__, nlines * sizeof(double), (void **)&xx, &error);
      if (status != MB_SUCCESS) {
        nlines_alloc = 0;
        mb_error(verbose, error, &error_message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
      }
      else
        nlines_alloc = nlines;

      /* calculate line positions */
      if (status == MB_SUCCESS)
        for (int i = 0; i < nlines; i++) {
          /* get line position in survey area */
          xx[i] = dsign * line_spacing_use * (((double)i) - 0.5 * (nlines - 1.0));
        }
    }

    /* do variable line spacing with variable altitude */
    else if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE) {
      /* get platform depth */
      if (survey_platform == MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_DEPTH) {
        sonar_depth = (double)survey_depth;
      }
      else {
        sonar_depth = 0.0;
      }

      /* allocate space for line position array */
      nlines_alloc += 100;
      status = mb_mallocd(verbose, __FILE__, __LINE__, nlines_alloc * sizeof(double), (void **)&xx, &error);
      if (status != MB_SUCCESS) {
        nlines_alloc = 0;
        mb_error(verbose, error, &error_message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
      }

      /* start on the port side of the survey */
      /* find range of altitude along each line and calculate the swath width
          from the smallest altitude */
      xx[0] = -dsign * 0.5 * r;
      nlines = 1;
      segment.nls = 0;
      segment.nls_alloc = 0;
      segment.lspoints = NULL;

      while (nlines == 1 || fabs(xx[nlines - 1] + dsign * 0.5 * line_spacing_use) < 0.5 * r) {
        /* allocate more space for xx if needed */
        if (nlines_alloc <= nlines) {
          nlines_alloc += 100;
          status = mb_reallocd(verbose, __FILE__, __LINE__, nlines_alloc * sizeof(double), (void **)&xx, &error);
          if (status != MB_SUCCESS) {
            nlines_alloc = 0;
            mb_error(verbose, error, &error_message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
          }
        }
        /* get offset from last xx */
        dxuse = dx * xx[nlines - 1];
        dyuse = dy * xx[nlines - 1];

        /* get first point */
        segment.endpoints[0].xdisplay = data->area.endpoints[0].xdisplay + dxuse;
        segment.endpoints[0].ydisplay = data->area.endpoints[0].ydisplay + dyuse;
        segment.endpoints[0].zdisplay = data->area.endpoints[0].zdisplay;
        mbview_projectinverse(instance, true, segment.endpoints[0].xdisplay, segment.endpoints[0].ydisplay,
                              segment.endpoints[0].zdisplay, &segment.endpoints[0].xlon, &segment.endpoints[0].ylat,
                              &segment.endpoints[0].xgrid, &segment.endpoints[0].ygrid);
        mbview_getzdata(instance, segment.endpoints[0].xgrid, segment.endpoints[0].ygrid, &ok,
                        &segment.endpoints[0].zdata);

        /* get second point */
        segment.endpoints[1].xdisplay = data->area.endpoints[1].xdisplay + dxuse;
        segment.endpoints[1].ydisplay = data->area.endpoints[1].ydisplay + dyuse;
        segment.endpoints[1].zdisplay = data->area.endpoints[1].zdisplay;
        mbview_projectinverse(instance, true, segment.endpoints[1].xdisplay, segment.endpoints[1].ydisplay,
                              segment.endpoints[1].zdisplay, &segment.endpoints[1].xlon, &segment.endpoints[1].ylat,
                              &segment.endpoints[1].xgrid, &segment.endpoints[1].ygrid);
        mbview_getzdata(instance, segment.endpoints[1].xgrid, segment.endpoints[1].ygrid, &ok,
                        &segment.endpoints[1].zdata);

        /* drape line and get max topo */
        mbview_drapesegment(instance, &(segment));
        maxtopo = -9999999.9;
        if (segment.endpoints[0].zdata < -sonar_depth) {
          maxtopo = segment.endpoints[0].zdata;
        }
        if (segment.endpoints[1].zdata < -sonar_depth && segment.endpoints[1].zdata > maxtopo) {
          maxtopo = segment.endpoints[1].zdata;
        }
        for (int i = 0; i < segment.nls; i++) {
          if (segment.lspoints[i].zdata < -sonar_depth)
            maxtopo = MAX(maxtopo, segment.lspoints[i].zdata);
        }

        /* figure minimum swath width and location of next line */
        sonar_altitude = -maxtopo - sonar_depth;
        line_spacing = sonar_altitude * 2.0 * tan(DTR * 0.5 * (double)survey_swathwidth);
        line_spacing_use = line_spacing * r / data->area.width;
        xx[nlines] = xx[nlines - 1] + dsign * line_spacing_use;
        nlines++;
      }

      /* deallocate segment points */
      if (segment.lspoints != NULL) {
        mb_freed(verbose, __FILE__, __LINE__, (void **)&(segment.lspoints), &error);
        segment.nls_alloc = 0;
      }
    }

    /* do crosslines if requested */
    if (survey_crosslines > 0 && !survey_crosslines_last && status == MB_SUCCESS) {
      /* figure out which corner the main lines start at */
      dxuse = dx * xx[0];
      dyuse = dy * xx[0];
      dxextra = 0.0;
      dyextra = 0.0;
      xdisplay = data->area.endpoints[jendpoint].xdisplay + dxuse + dxextra;
      ydisplay = data->area.endpoints[jendpoint].ydisplay + dyuse + dyextra;
      for (int i = 0; i < 4; i++) {
        xxx = xdisplay - data->area.cornerpoints[i].xdisplay;
        yyy = ydisplay - data->area.cornerpoints[i].ydisplay;
        rrr[i] = sqrt(xxx * xxx + yyy * yyy);
      }
      startcorner = 0;
      for (int i = 1; i < 4; i++) {
        if (rrr[i] < rrr[startcorner])
          startcorner = i;
      }

      /* figure out which corner the cross lines should start at */
      if (survey_crosslines % 2 == 0) {
        if (startcorner == 0)
          startcorner = 3;
        else if (startcorner == 1)
          startcorner = 2;
        else if (startcorner == 2)
          startcorner = 1;
        else if (startcorner == 3)
          startcorner = 0;
      }
      else {
        if (startcorner == 0)
          startcorner = 2;
        else if (startcorner == 1)
          startcorner = 3;
        else if (startcorner == 2)
          startcorner = 0;
        else if (startcorner == 3)
          startcorner = 1;
      }

      /* get crossline vector */
      if (startcorner == 0 || startcorner == 3) {
        dx = data->area.cornerpoints[1].xdisplay - data->area.cornerpoints[0].xdisplay;
        dy = data->area.cornerpoints[1].ydisplay - data->area.cornerpoints[0].ydisplay;
      }
      else {
        dx = data->area.cornerpoints[0].xdisplay - data->area.cornerpoints[1].xdisplay;
        dy = data->area.cornerpoints[0].ydisplay - data->area.cornerpoints[1].ydisplay;
      }
      r = sqrt(dx * dx + dy * dy);
      dxd = dx / r;
      dyd = dy / r;

      /* get crossline spacing */
      crossline_spacing = (data->area.length / (survey_crosslines + 1)) * (r / data->area.width);

      /* generate cross lines */
      jstart = startcorner;
      if (startcorner == 0 || startcorner == 2)
        kend = jstart + 1;
      else
        kend = jstart - 1;
      dx = (data->area.endpoints[1].xdisplay - data->area.endpoints[0].xdisplay) / (survey_crosslines + 1);
      dy = (data->area.endpoints[1].ydisplay - data->area.endpoints[0].ydisplay) / (survey_crosslines + 1);
      if (startcorner >= 2) {
        dx = -dx;
        dy = -dy;
      }
      int j = jstart;
      for (int i = 0; i < survey_crosslines; i++) {
        /* get offset from corners */
        dxuse = (i + 1) * dx;
        dyuse = (i + 1) * dy;
        if (j == jstart) {
          dxextra = -dxd * line_spacing_use;
          dyextra = -dyd * line_spacing_use;
        }
        else {
          dxextra = dxd * line_spacing_use;
          dyextra = dyd * line_spacing_use;
        }

        /* get first point */
        waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
        xdisplay = data->area.cornerpoints[j].xdisplay + dxuse + dxextra;
        ydisplay = data->area.cornerpoints[j].ydisplay + dyuse + dyextra;
        zdisplay = data->area.cornerpoints[j].zdisplay;
        mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
        mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
        if (!ok)
          zdata = data->area.cornerpoints[jendpoint].zdata;
        mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
        if (first) {
          mbview_addroute(verbose, instance, 1, &xlon, &ylat, &waypoint, color, 2, true, survey_name, &working_route,
                          &error);
          first = false;
        }
        else {
          mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata,
                           xdisplay, ydisplay, zdisplay);
        }
        npoints++;

        /* get second point */
        if (j == jstart)
          j = kend;
        else
          j = jstart;
        if (j == jstart) {
          dxextra = -dxd * line_spacing_use;
          dyextra = -dyd * line_spacing_use;
        }
        else {
          dxextra = dxd * line_spacing_use;
          dyextra = dyd * line_spacing_use;
        }

        /* get second point */
        waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
        xdisplay = data->area.cornerpoints[j].xdisplay + dxuse + dxextra;
        ydisplay = data->area.cornerpoints[j].ydisplay + dyuse + dyextra;
        zdisplay = data->area.cornerpoints[j].zdisplay;
        mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
        mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
        if (!ok)
          zdata = data->area.cornerpoints[jendpoint].zdata;
        mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
        mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata, xdisplay,
                         ydisplay, zdisplay);
        npoints++;
      }
    }

    /* generate the lines */
    if (nlines > 0 && status == MB_SUCCESS) {
      /* get unit vector for survey area boundaries */
      dx = data->area.cornerpoints[1].xdisplay - data->area.cornerpoints[0].xdisplay;
      dy = data->area.cornerpoints[1].ydisplay - data->area.cornerpoints[0].ydisplay;
      r = sqrt(dx * dx + dy * dy);
      dx = dx / r;
      dy = dy / r;

      /* generate points */
      /* work in display coordinates */
      nlinegroups = nlines / survey_interleaving + 1;
      for (int j = 0; j < survey_interleaving; j++)
        for (int i = 0; i < nlinegroups; i++) {
          /* get line number */
          iline = i * survey_interleaving + j;

          if (iline < nlines) {
            /* get line position in survey area */
            dxuse = dx * xx[iline];
            dyuse = dy * xx[iline];

            /* add a bit of transit before later interleaved lines */
            if (jendpoint == 1) {
              dxextra = -dy * j * 0.25 * line_spacing_use;
              dyextra = dx * j * 0.25 * line_spacing_use;
            }
            else {
              dxextra = dy * j * 0.25 * line_spacing_use;
              dyextra = -dx * j * 0.25 * line_spacing_use;
            }

            /* get first point */
            waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
            xdisplay = data->area.endpoints[jendpoint].xdisplay + dxuse + dxextra;
            ydisplay = data->area.endpoints[jendpoint].ydisplay + dyuse + dyextra;
            zdisplay = data->area.endpoints[jendpoint].zdisplay;
            mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
            mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
            if (!ok)
              zdata = data->area.endpoints[jendpoint].zdata;
            mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
            fprintf(stderr, "\nSurvey Line:%d Point:%d  Position: %f %f %f  %f %f   %f %f %f\n", iline, jendpoint,
                    xlon, ylat, zdata, xgrid, ygrid, xdisplay, ydisplay, zdisplay);

            /* add new route for first point, just add single point
                after that */
            if (first) {
              mbview_addroute(verbose, instance, 1, &xlon, &ylat, &waypoint, color, 2, true, survey_name,
                              &working_route, &error);
              first = false;
            }
            else {
              mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata,
                               xdisplay, ydisplay, zdisplay);
            }
            npoints++;

            /* switch endpoint */
            jendpoint = (jendpoint + 1) % 2;

            /* add a bit of transit before interleaved lines */
            if (jendpoint == 1) {
              dxextra = -dy * j * 0.25 * line_spacing_use;
              dyextra = dx * j * 0.25 * line_spacing_use;
            }
            else {
              dxextra = dy * j * 0.25 * line_spacing_use;
              dyextra = -dx * j * 0.25 * line_spacing_use;
            }

            /* get second point */
            waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
            xdisplay = data->area.endpoints[jendpoint].xdisplay + dxuse + dxextra;
            ydisplay = data->area.endpoints[jendpoint].ydisplay + dyuse + dyextra;
            zdisplay = data->area.endpoints[jendpoint].zdisplay;
            mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
            mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
            if (!ok)
              zdata = data->area.endpoints[jendpoint].zdata;
            mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
            fprintf(stderr, "Survey Line:%d Point:%d  Position: %f %f %f  %f %f   %f %f %f\n", iline, jendpoint, xlon,
                    ylat, zdata, xgrid, ygrid, xdisplay, ydisplay, zdisplay);

            /* add single point */
            mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata,
                             xdisplay, ydisplay, zdisplay);
            npoints++;
          }
        }

      /* deallocate line position array */
      mb_freed(verbose, __FILE__, __LINE__, (void **)&xx, &error);
    }

    /* do crosslines if requested */
    if (survey_crosslines > 0 && survey_crosslines_last && status == MB_SUCCESS) {
      /* figure out which corner the mail lines ended at */
      for (int i = 0; i < 4; i++) {
        xxx = xdisplay - data->area.cornerpoints[i].xdisplay;
        yyy = ydisplay - data->area.cornerpoints[i].ydisplay;
        rrr[i] = sqrt(xxx * xxx + yyy * yyy);
      }
      endcorner = 0;
      for (int i = 1; i < 4; i++) {
        if (rrr[i] < rrr[endcorner])
          endcorner = i;
      }

      /* get crossline vector */
      if (endcorner == 0 || endcorner == 3) {
        dx = data->area.cornerpoints[1].xdisplay - data->area.cornerpoints[0].xdisplay;
        dy = data->area.cornerpoints[1].ydisplay - data->area.cornerpoints[0].ydisplay;
      }
      else {
        dx = data->area.cornerpoints[0].xdisplay - data->area.cornerpoints[1].xdisplay;
        dy = data->area.cornerpoints[0].ydisplay - data->area.cornerpoints[1].ydisplay;
      }
      r = sqrt(dx * dx + dy * dy);
      dxd = dx / r;
      dyd = dy / r;

      /* get crossline spacing */
      crossline_spacing = (data->area.length / (crossline_spacing + 1)) * (r / data->area.width);

      /* generate cross lines */
      jstart = endcorner;
      if (endcorner == 0 || endcorner == 2)
        kend = jstart + 1;
      else
        kend = jstart - 1;
      dx = (data->area.endpoints[1].xdisplay - data->area.endpoints[0].xdisplay) / (survey_crosslines + 1);
      dy = (data->area.endpoints[1].ydisplay - data->area.endpoints[0].ydisplay) / (survey_crosslines + 1);
      if (endcorner >= 2) {
        dx = -dx;
        dy = -dy;
      }
      int j = jstart;
      for (int i = 0; i < survey_crosslines; i++) {
        /* get offset from corners */
        dxuse = (i + 1) * dx;
        dyuse = (i + 1) * dy;
        if (j == jstart) {
          dxextra = -dxd * line_spacing_use;
          dyextra = -dyd * line_spacing_use;
        }
        else {
          dxextra = dxd * line_spacing_use;
          dyextra = dyd * line_spacing_use;
        }

        /* get first point */
        waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
        xdisplay = data->area.cornerpoints[j].xdisplay + dxuse + dxextra;
        ydisplay = data->area.cornerpoints[j].ydisplay + dyuse + dyextra;
        zdisplay = data->area.cornerpoints[j].zdisplay;
        mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
        mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
        if (!ok)
          zdata = data->area.cornerpoints[jendpoint].zdata;
        mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
        mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata, xdisplay,
                         ydisplay, zdisplay);
        npoints++;

        /* get second point */
        if (j == jstart)
          j = kend;
        else
          j = jstart;
        if (j == jstart) {
          dxextra = -dxd * line_spacing_use;
          dyextra = -dyd * line_spacing_use;
        }
        else {
          dxextra = dxd * line_spacing_use;
          dyextra = dyd * line_spacing_use;
        }

        /* get second point */
        waypoint = MBV_ROUTE_WAYPOINT_STARTLINE;
        xdisplay = data->area.cornerpoints[j].xdisplay + dxuse + dxextra;
        ydisplay = data->area.cornerpoints[j].ydisplay + dyuse + dyextra;
        zdisplay = data->area.cornerpoints[j].zdisplay;
        mbview_projectinverse(instance, true, xdisplay, ydisplay, zdisplay, &xlon, &ylat, &xgrid, &ygrid);
        mbview_getzdata(instance, xgrid, ygrid, &ok, &zdata);
        if (!ok)
          zdata = data->area.cornerpoints[jendpoint].zdata;
        mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
        mbview_route_add(verbose, instance, working_route, npoints, waypoint, xgrid, ygrid, xlon, ylat, zdata, xdisplay,
                         ydisplay, zdisplay);
        npoints++;
      }
    }

    /* free the memory for xx */
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&xx, &error);

    /* update widgets */
    mbview_updateroutelist();
    do_mbgrdviz_arearoute_info(instance);
    mbview_enableviewnavs(verbose, instance, &error);
    status = mbview_update(verbose, instance, &error);
  }

  /* update widgets of remaining mbview windows */
  for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (i != instance && mbview_id[i])
      status = mbview_update(verbose, i, &error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
  /* reset current working route so the last one generated is saved */
  working_route = -1;
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_parameterchange(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* get source mbview instance */
  const size_t instance = (size_t)client_data;

  /* check data source for area to bounding desired survey */
  struct mbview_struct *data;
  int status = mbview_getdataptr(verbose, instance, &data, &error);

  /* check if area is currently defined */
  if (status == MB_SUCCESS) {
    if (data->area_type != MBV_AREA_QUAD)
      status = MB_FAILURE;
  }

  /* get parameters */
  if (status == MB_SUCCESS) {
    Cardinal ac = 0;
    Arg args[256];
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_mode);
    ac++;
    XtGetValues(spinText_arearoute_linecontrol, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_direction);
    ac++;
    XtGetValues(spinText_arearoute_direction, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_crosslines);
    ac++;
    XtGetValues(spinText_arearoute_crosslines, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_crosslines_last);
    ac++;
    XtGetValues(spinText_arearoute_crosslinesfirstlast, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_interleaving);
    ac++;
    XtGetValues(spinText_arearoute_interleaving, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_color);
    ac++;
    XtGetValues(spinText_arearoute_color, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_linespacing);
    ac++;
    XtGetValues(spinText_arearoute_linespacing, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_platform);
    ac++;
    XtGetValues(spinText_arearoute_platform, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_swathwidth);
    ac++;
    XtGetValues(spinText_arearoute_swathwidth, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_altitude);
    ac++;
    XtGetValues(spinText_arearoute_altitude, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNposition, (XtPointer)&survey_depth);
    ac++;
    XtGetValues(spinText_arearoute_depth, args, ac);

    char *tmp = XmTextGetString(textField_arearoute_name);
    if (tmp != NULL && strlen(tmp) > 0)
      strcpy(survey_name, tmp);
    else {
      if (strlen(survey_name) <= 0)
        sprintf(survey_name, "Survey");
      XmTextSetString(textField_arearoute_name, survey_name);
    }
    if (tmp != NULL)
      XtFree(tmp);

    fprintf(stderr, "\nIn do_mbgrdviz_arearoute_parameterchange:\n");
    fprintf(stderr, "  survey_mode:                %d\n", survey_mode);
    fprintf(stderr, "  survey_platform:            %d\n", survey_platform);
    fprintf(stderr, "  survey_interleaving:        %d\n", survey_interleaving);
    fprintf(stderr, "  survey_direction:           %d\n", survey_direction);
    fprintf(stderr, "  survey_crosslines_last:     %d\n", survey_crosslines_last);
    fprintf(stderr, "  survey_crosslines:          %d\n", survey_crosslines);
    fprintf(stderr, "  survey_linespacing:         %d\n", survey_linespacing);
    fprintf(stderr, "  survey_swathwidth:          %d\n", survey_swathwidth);
    fprintf(stderr, "  survey_depth:               %d\n", survey_depth);
    fprintf(stderr, "  survey_altitude:            %d\n", survey_altitude);
    fprintf(stderr, "  survey_color:               %d\n", survey_color);
    fprintf(stderr, "  survey_name:                %s\n", survey_name);

    /* reset widgets accordingly (sensitivity and info) */
    do_mbgrdviz_arearoute_recalc(instance);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_recalc(size_t instance) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:   %zu\n", instance);
  }

  /* check data source for area to bounding desired survey */
  struct mbview_struct *data;
  int status = mbview_getdataptr(verbose, instance, &data, &error);

  /* check if area is currently defined */
  if (status == MB_SUCCESS) {
    if (data->area_type != MBV_AREA_QUAD)
      status = MB_FAILURE;
  }

  /* set widgets */
  if (status == MB_SUCCESS) {
    Cardinal ac = 0;
    Arg args[256];

    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_linecontrol, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_direction, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_crosslines, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_crosslinesfirstlast, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_interleaving, args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNsensitive, True);
    ac++;
    XtSetValues(spinText_arearoute_color, args, ac);

    ac = 0;
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_UNIFORM) {
      XtSetArg(args[ac], XmNsensitive, True);
      ac++;
    }
    else {
      XtSetArg(args[ac], XmNsensitive, False);
      ac++;
    }
    XtSetValues(spinText_arearoute_linespacing, args, ac);

    ac = 0;
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE) {
      XtSetArg(args[ac], XmNsensitive, True);
      ac++;
    }
    else {
      XtSetArg(args[ac], XmNsensitive, False);
      ac++;
    }
    XtSetValues(spinText_arearoute_platform, args, ac);

    ac = 0;
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE) {
      XtSetArg(args[ac], XmNsensitive, True);
      ac++;
    }
    else {
      XtSetArg(args[ac], XmNsensitive, False);
      ac++;
    }
    XtSetValues(spinText_arearoute_swathwidth, args, ac);

    ac = 0;
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE && survey_platform == MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_ALTITUDE) {
      XtSetArg(args[ac], XmNsensitive, True);
      ac++;
    }
    else {
      XtSetArg(args[ac], XmNsensitive, False);
      ac++;
    }
    XtSetValues(spinText_arearoute_altitude, args, ac);

    ac = 0;
    if (survey_mode == MBGRDVIZ_SURVEY_MODE_VARIABLE && survey_platform == MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_DEPTH) {
      XtSetArg(args[ac], XmNsensitive, True);
      ac++;
    }
    else {
      XtSetArg(args[ac], XmNsensitive, False);
      ac++;
    }
    XtSetValues(spinText_arearoute_depth, args, ac);

    do_mbgrdviz_arearoute_info(instance);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_info(size_t instance) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:   %zu\n", instance);
  }

  /* check data source for area to bounding desired survey */
  struct mbview_struct *data;
  int status = mbview_getdataptr(verbose, instance, &data, &error);

  /* check if area is currently defined */
  char info_text[2*MB_PATH_MAXLINE];
  if (status == MB_SUCCESS) {
    if (data->area_type != MBV_AREA_QUAD)
      status = MB_FAILURE;
    sprintf(info_text, ":::t\"No Current Area:\"");
  }

  /* set widgets */
  if (status == MB_SUCCESS) {
    if (working_route >= 0) {
      int nroutewaypoint;
      int nroutpoint;
      char routename[MB_PATH_MAXLINE];
      int routecolor;
      int routesize;
      double routedistancelateral;
      double routedistancetopo;

      /* get info for working route */
      status = mbview_getrouteinfo(verbose, instance, working_route, &nroutewaypoint, &nroutpoint, routename, &routecolor,
                                   &routesize, &routedistancelateral, &routedistancetopo, &error);

      sprintf(info_text,
              ":::t\"Current Area:\":t\" Length: %.1f m  Width: %.1f m  Bearing: %.1f deg\":t\"New Route: %d  Name: "
              "%s\":t\" Waypoints: %d  Total Points:%d\":t\" Distance: %.1f m (lateral) %.1f m (over bottom)\"",
              data->area.length, data->area.width, data->area.bearing, working_route, routename, nroutewaypoint, nroutpoint,
              routedistancelateral, routedistancetopo);
    } else {
      sprintf(info_text, ":::t\"Current Area:\":t\" Length: %.3f m\":t\" Width: %.3f m\":t\" Bearing: %.1f deg\"",
              data->area.length, data->area.width, data->area.bearing);
    }
    set_mbview_label_multiline_string(label_arearoute_info, info_text);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_linespacing_increment(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* reset line spacing spinbox increment as value changes */
  Cardinal ac = 0;
  Arg args[256];
  int linespacing;

  XtSetArg(args[ac], XmNposition, (XtPointer)&linespacing);
  ac++;
  XtGetValues(spinText_arearoute_linespacing, args, ac);
  int increment;
  if (linespacing < 25)
    increment = 1;
  else if (linespacing < 100)
    increment = 5;
  else if (linespacing < 250)
    increment = 10;
  else if (linespacing < 1000)
    increment = 25;
  else if (linespacing < 2000)
    increment = 50;
  else
    increment = 100;
  ac = 0;
  XtSetArg(args[ac], XmNincrementValue, increment);
  ac++;
  XtSetValues(spinText_arearoute_linespacing, args, ac);
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_altitude_increment(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* reset line spacing spinbox increment as value changes */
  Arg args[256];
  int altitude;

  Cardinal ac = 0;
  XtSetArg(args[ac], XmNposition, (XtPointer)&altitude);
  ac++;
  XtGetValues(spinText_arearoute_altitude, args, ac);
  int increment;
  if (altitude < 25)
    increment = 1;
  else if (altitude < 100)
    increment = 5;
  else if (altitude < 250)
    increment = 10;
  else if (altitude < 1000)
    increment = 25;
  else if (altitude < 2000)
    increment = 50;
  else
    increment = 100;
  ac = 0;
  XtSetArg(args[ac], XmNincrementValue, increment);
  ac++;
  XtSetValues(spinText_arearoute_altitude, args, ac);
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_arearoute_depth_increment(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* reset line spacing spinbox increment as value changes */
  Arg args[256];
  int depth;

  Cardinal ac = 0;
  XtSetArg(args[ac], XmNposition, (XtPointer)&depth);
  ac++;
  XtGetValues(spinText_arearoute_depth, args, ac);
  int increment;
  if (depth < 25)
    increment = 1;
  else if (depth < 100)
    increment = 5;
  else if (depth < 250)
    increment = 10;
  else if (depth < 1000)
    increment = 25;
  else if (depth < 2000)
    increment = 50;
  else
    increment = 100;
  ac = 0;
  XtSetArg(args[ac], XmNincrementValue, increment);
  ac++;
  XtSetValues(spinText_arearoute_depth, args, ac);


  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    const int status = MB_SUCCESS;
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mbgrdviz_realtime_start(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_path_reset(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtime_pause(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtime_stop(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtime_resume(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_path_apply(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_icon(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_path_browse(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  do_mbgrdviz_fileSelectionBox_realtime(w, client_data, call_data);
}

void do_mbgrdviz_realtimesetup_updaterate(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_path_test(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}

void do_mbgrdviz_realtimesetup_pathmode(Widget w, XtPointer client_data, XtPointer call_data) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }
}
