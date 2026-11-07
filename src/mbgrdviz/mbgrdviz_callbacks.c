/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_callbacks.c    10/9/2002
 *
 *    Copyright (c) 2002-2025 by
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
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifdef USE_UUID
#include <uuid.h>
#endif

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"
#include "mb_xmutil.h"
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
#include "mbgrdviz_core.h"
#include "mbgrdviz_state.h"

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


/* status variables */
static int pargc;
static char **pargv;

/* widgets */
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
void do_mbgrdviz_fileSelectionBox_savedegdecmin(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savelnw(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savetecdislst(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savekongsbergdp(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesisasciiplan1(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesisasciiplan2(Widget w, XtPointer client_data, XtPointer call_data);
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
void do_mbgrdviz_fileSelectionBox_openpolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openpolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opengispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_importshapefile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savepolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savepolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savegispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportpolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportpolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportgispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_addgisactions(size_t instance);
int do_mbgrdviz_opengis(size_t instance, int type, char *input_file_ptr);
int do_mbgrdviz_importshapefile(size_t instance, char *input_file_ptr);
int do_mbgrdviz_savegis(size_t instance, int type, bool shapefile, char *output_file_ptr);
int do_mbgrdviz_savesite(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesitewpt(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveroute(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveroutereversed(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptnoheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisi2scriptheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisi2scriptnoheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savedegdecmin(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savelnw(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savegreenseayml(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savetecdislst(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savekongsbergdp(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesisasciiplan1(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesisasciiplan2(size_t instance, char *output_file_ptr);
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
  snprintf(value_text, sizeof(value_text), "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION, MB_VERSION_DATE);
  set_mbview_label_multiline_string(label_about_version, value_text);

  /* get additional widgets */
  fileSelectionList = (Widget)XtNameToWidget(fileSelectionBox, "*ItemsList");
  fileSelectionText = (Widget)XtNameToWidget(fileSelectionBox, "Text");
  XtUnmanageChild((Widget)XtNameToWidget(fileSelectionBox, "Help"));

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
  XmStringFree(str_list[2]);
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
  XtSetValues(pushButton_openpolyline, args, ac);
  XtSetValues(pushButton_openpolygon, args, ac);
  XtSetValues(pushButton_opengispoint, args, ac);

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
void do_mbgrdviz_fileSelectionBox_saverisi2scriptheading(Widget w, XtPointer client_data, XtPointer call_data) {

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
  const size_t actionid = MBGRDVIZ_SAVERISI2SCRIPTHEADING * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_saverisi2scriptnoheading(Widget w, XtPointer client_data, XtPointer call_data) {

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
  const size_t actionid = MBGRDVIZ_SAVERISI2SCRIPTNOHEADING * MBV_MAX_WINDOWS + instance;
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
void do_mbgrdviz_fileSelectionBox_savekongsbergdp(Widget w, XtPointer client_data, XtPointer call_data) {

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
  XtSetArg(args[ac], XmNtitle, "Save Route as Kongsberg DP Waypoint File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVEKONGSBERGDP * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savesisasciiplan1(Widget w, XtPointer client_data, XtPointer call_data) {

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
  XtSetArg(args[ac], XmNtitle, "Save Route as SIS ASCIIPlan Waypoint File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVESISASCIIPLAN1 * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_savesisasciiplan2(Widget w, XtPointer client_data, XtPointer call_data) {

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
  XtSetArg(args[ac], XmNtitle, "Save Route as SIS ASCIIPlan Waypoint File");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = MBGRDVIZ_SAVESISASCIIPLAN2 * MBV_MAX_WINDOWS + instance;
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
  file_ptr = (char *)XmStringUnparse(acs->value, NULL, XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);
  if (file_ptr != NULL && strlen(file_ptr) <= 0) {
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

  /* the main window file menu items pass instance 0; if that mbview window
     is not open use the first open one (as do_mbgrdviz_sensitivity() does) */
  if (mode > MBGRDVIZ_OPENGRID && mode != MBGRDVIZ_REALTIME
      && (instance >= MBV_MAX_WINDOWS || !mbview_id[instance])) {
    instance = MBV_NO_WINDOW;
    for (size_t i = 0; i < MBV_MAX_WINDOWS && instance == MBV_NO_WINDOW; i++) {
      if (mbview_id[i])
        instance = i;
    }
  }

  /* every file operation other than opening a primary grid needs a file name,
     and all but setting the realtime source need an open mbview window */
  if (mode > MBGRDVIZ_OPENGRID && (file_ptr == NULL || (mode != MBGRDVIZ_REALTIME && instance == MBV_NO_WINDOW))) {
    XBell((Display *)XtDisplay(mainWindow), 100);
    if (file_ptr != NULL)
      XtFree(file_ptr);
    return;
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

  /* else write route data as Risi 2 script with variable heading */
  else if (mode == MBGRDVIZ_SAVERISI2SCRIPTHEADING) {
    /* write route file */
    do_mbview_message_on("Saving route as Risi 2 script with variable heading...", instance);
    /* status = */ do_mbgrdviz_saverisi2scriptheading(instance, file_ptr);
  }

  /* else write route data as Risi 2 script with static heading */
  else if (mode == MBGRDVIZ_SAVERISI2SCRIPTNOHEADING) {
    /* write route file */
    do_mbview_message_on("Saving route as Risi 2 script with static heading...", instance);
    /* status = */ do_mbgrdviz_saverisi2scriptnoheading(instance, file_ptr);
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

  /* else write route data as Kongsberg DP waypoint file */
  else if (mode == MBGRDVIZ_SAVEKONGSBERGDP) {
    /* write route file */
    do_mbview_message_on("Saving route as Kongsberg DP waypoint file...", instance);
    /* status = */ do_mbgrdviz_savekongsbergdp(instance, file_ptr);
  }

  /* else write route data as SIS ASCIIPlan waypoint file */
  else if (mode == MBGRDVIZ_SAVESISASCIIPLAN1) {
    /* write route file */
    do_mbview_message_on("Saving route as SIS ASCIIPlan lines file...", instance);
    /* status = */ do_mbgrdviz_savesisasciiplan1(instance, file_ptr);
  }

  /* else write route data as SIS ASCIIPlan waypoint file */
  else if (mode == MBGRDVIZ_SAVESISASCIIPLAN2) {
    /* write route file */
    do_mbview_message_on("Saving route as SIS ASCIIPlan route file...", instance);
    /* status = */ do_mbgrdviz_savesisasciiplan2(instance, file_ptr);
  }

  /* else write route data */
  else if (mode == MBGRDVIZ_SAVEPROFILE) {
    /* write route file */
    do_mbview_message_on("Saving profile data...", instance);
    /* status = */ do_mbgrdviz_saveprofile(instance, file_ptr);
  }

  /* else open GIS polyline, polygon, or point data */
  else if (mode == MBGRDVIZ_OPENPOLYLINE || mode == MBGRDVIZ_OPENPOLYGON || mode == MBGRDVIZ_OPENGISPOINT) {
    do_mbview_message_on("Reading GIS data...", instance);
    const int type = mode == MBGRDVIZ_OPENPOLYLINE ? MBV_GISLAYER_POLYLINE
                         : (mode == MBGRDVIZ_OPENPOLYGON ? MBV_GISLAYER_POLYGON : MBV_GISLAYER_GISPOINT);
    /* status = */ do_mbgrdviz_opengis(instance, type, file_ptr);
  }

  /* else import a shapefile */
  else if (mode == MBGRDVIZ_IMPORTSHAPEFILE) {
    do_mbview_message_on("Importing shapefile...", instance);
    /* status = */ do_mbgrdviz_importshapefile(instance, file_ptr);
  }

  /* else save GIS polyline, polygon, or point data as an MB-System file or a shapefile */
  else if (mode >= MBGRDVIZ_SAVEPOLYLINE && mode <= MBGRDVIZ_EXPORTGISPOINT) {
    const bool shapefile = mode >= MBGRDVIZ_EXPORTPOLYLINE;
    do_mbview_message_on(shapefile ? "Exporting shapefile..." : "Saving GIS data...", instance);
    const size_t offset = shapefile ? mode - MBGRDVIZ_EXPORTPOLYLINE : mode - MBGRDVIZ_SAVEPOLYLINE;
    const int type = offset == 0 ? MBV_GISLAYER_POLYLINE : (offset == 1 ? MBV_GISLAYER_POLYGON : MBV_GISLAYER_GISPOINT);
    /* status = */ do_mbgrdviz_savegis(instance, type, shapefile, file_ptr);
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

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

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
  size_t instance = MBV_NO_WINDOW;
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
    for (i = 0; status == MB_SUCCESS && i < mbv_primary_n_columns; i++) {
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
          data_source->nav_view_mode, data_source->navswathbounds_view_mode, data_source->navdrape_view_mode, data_source->vector_view_mode,
          data_source->exaggeration, data_source->modelelevation3d, data_source->modelazimuth3d,
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

    /* show GIS polylines, polygons, and points as in the source window */
    if (status == MB_SUCCESS) {
      struct mbview_struct *data_new = NULL;
      mbview_getdataptr(verbose, instance, &data_new, &error);
      data_new->polyline_view_mode = data_source->polyline_view_mode;
      data_new->polygon_view_mode = data_source->polygon_view_mode;
      data_new->gispoint_view_mode = data_source->gispoint_view_mode;
    }

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
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opennav, "Open Navigation", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openswath, "Open Swath Data", MBV_PICKMASK_NONE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openvector, "Open Vector File",
                         MBV_PICKMASK_NONE, &error);
        do_mbgrdviz_addgisactions(instance);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savesite, "Save Site File", MBV_EXISTMASK_SITE,
                         &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveroute, "Save Route File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptheading, "Save Risi Script File (variable heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading, "Save Risi Script File (static heading)",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savedegdecmin,
                         "Save Route as Degrees + Decimal Minutes File", MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savelnw, "Save Route as Hypack LNW File",
                         MBV_EXISTMASK_ROUTE, &error);
        mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_saveprofile, "Save Profile File",
                         MBV_PICKMASK_TWOPOINT + MBV_PICKMASK_ROUTE + MBV_PICKMASK_NAVTWOPOINT, &error);
        mbview_addactionseparator(verbose, instance, &error);

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
        mbview_addactionseparator(verbose, instance, &error);
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
      for (i = 0; status == MB_SUCCESS && i < mbv_secondary_n_columns; i++) {
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
  if (instance != MBV_NO_WINDOW && instance < MBV_MAX_WINDOWS && mbview_id[instance])
    status = mbview_update(verbose, instance, &error);

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbgrdviz_sensitivity();
}
/*---------------------------------------------------------------------------------------*/

/* Launch argv[0] with the given (NULL-terminated) argument list in the
    background, without ever passing user- or file-derived strings through
    a shell. A double fork detaches the new program so that it is not left
    as a zombie or child of this process, mimicking the effect of the
    "program args &" shell commands this replaces. */
static void mbgrdviz_launch_background(char *const argv[]) {
  const pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Unable to fork to launch %s: %s\n", argv[0], strerror(errno));
    return;
  }
  if (pid == 0) {
    /* first child: fork again and exit immediately so the grandchild is
        reparented to init and never becomes a zombie */
    const pid_t pid2 = fork();
    if (pid2 == 0) {
      setsid();
      execvp(argv[0], argv);
      fprintf(stderr, "Unable to exec %s: %s\n", argv[0], strerror(errno));
      _exit(127);
    }
    else if (pid2 < 0) {
      fprintf(stderr, "Unable to fork to launch %s: %s\n", argv[0], strerror(errno));
    }
    _exit(0);
  }

  /* parent: reap the short-lived first child right away */
  int wstatus;
  waitpid(pid, &wstatus, 0);
}

void do_mbgrdviz_open_mbedit(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
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
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        nselected += nav->nselected;
      }
    }
  }

  /* open all data files with selected nav into mbedit */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    /* build an argv array directly - no shell, no fixed-size string
        concatenation - so neither an overlong path nor shell metacharacters
        in a path can overflow a buffer or be interpreted as commands */
    char **argv = (char **)malloc((size_t)(1 + 2 * shareddata->nnav + 1) * sizeof(char *));
    if (argv == NULL) {
      fprintf(stderr, "\nUnable to allocate memory for mbedit argument list\n");
      XBell((Display *)XtDisplay(mainWindow), 100);
    }
    else {
      int argc = 0;
      bool alloc_ok = true;
      argv[argc] = strdup("mbedit");
      if (argv[argc] == NULL)
        alloc_ok = false;
      else
        argc++;
      for (int i = 0; alloc_ok && i < shareddata->nnav; i++) {
        nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
        if (nav->nselected > 0) {
          char formatarg[32];
          char patharg[MB_PATH_MAXLINE + 4];
          snprintf(formatarg, sizeof(formatarg), "-F%d", nav->format);
          snprintf(patharg, sizeof(patharg), "-I%s", nav->pathraw);
          argv[argc] = strdup(formatarg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
          argv[argc] = strdup(patharg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
        }
      }

      if (alloc_ok) {
        argv[argc] = NULL;
        fprintf(stderr, "Calling mbedit with %d arguments\n", argc);
        mbgrdviz_launch_background(argv);
      }
      else {
        fprintf(stderr, "\nUnable to allocate memory for mbedit arguments\n");
        XBell((Display *)XtDisplay(mainWindow), 100);
      }

      for (int i = 0; i < argc; i++)
        free(argv[i]);
      free(argv);
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

void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
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
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      nselected += nav->nselected;
    }
    if(nselected > 0) {
      /* create the temporary datalist file securely: a random,
          non-predictable name created atomically (O_EXCL) by mkstemp() in
          the system temp directory, rather than a guessable
          "tmp_datalist_<pid>" name fopen()'ed for write in the current
          directory - the old approach let anyone who could predict or
          watch for the pid pre-create (e.g. as a symlink) the target path
          and have this process write through it */
      const char *tmpdir = getenv("TMPDIR");
      if (tmpdir == NULL || tmpdir[0] == '\0')
        tmpdir = "/tmp";
      snprintf(datalist_file, sizeof(mb_path), "%s/mbgrdviz_datalist_XXXXXX", tmpdir);
      const int dfd = mkstemp(datalist_file);
      dfp = dfd == -1 ? NULL : fdopen(dfd, "w");
      if (dfp == NULL && dfd != -1)
        close(dfd);
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

        char patharg[MB_PATH_MAXLINE + 4];
        snprintf(patharg, sizeof(patharg), "-I%s", datalist_file);
        char *argv[5];
        int argc = 0;
        bool alloc_ok = true;
        argv[argc] = strdup("mbeditviz");
        if (argv[argc] == NULL)
          alloc_ok = false;
        else
          argc++;
        if (alloc_ok) {
          argv[argc] = strdup(patharg);
          if (argv[argc] == NULL)
            alloc_ok = false;
          else
            argc++;
        }
        /* explicit datalist format (-1) since the mkstemp() generated
            datalist_file name no longer carries a recognizable ".mb-1"
            suffix for mb_get_format() to auto-detect */
        if (alloc_ok) {
          argv[argc] = strdup("-F-1");
          if (argv[argc] == NULL)
            alloc_ok = false;
          else
            argc++;
        }
        if (alloc_ok) {
          argv[argc] = strdup("-R");
          if (argv[argc] == NULL)
            alloc_ok = false;
          else
            argc++;
        }

        if (alloc_ok) {
          argv[argc] = NULL;
          fprintf(stderr, "Calling mbeditviz -I%s -F-1 -R\n", datalist_file);
          mbgrdviz_launch_background(argv);
        }
        else {
          fprintf(stderr, "\nUnable to allocate memory for mbeditviz arguments\n");
          XBell((Display *)XtDisplay(mainWindow), 100);
        }

        for (int i = 0; i < argc; i++)
          free(argv[i]);
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
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        nselected += nav->nselected;
      }
    }
  }

  /* open all data files with selected nav into mbnavedit */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    char **argv = (char **)malloc((size_t)(1 + 2 * shareddata->nnav + 1) * sizeof(char *));
    if (argv == NULL) {
      fprintf(stderr, "\nUnable to allocate memory for mbnavedit argument list\n");
      XBell((Display *)XtDisplay(mainWindow), 100);
    }
    else {
      int argc = 0;
      bool alloc_ok = true;
      argv[argc] = strdup("mbnavedit");
      if (argv[argc] == NULL)
        alloc_ok = false;
      else
        argc++;
      for (int i = 0; alloc_ok && i < shareddata->nnav; i++) {
        nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
        if (nav->nselected > 0) {
          char formatarg[32];
          char patharg[MB_PATH_MAXLINE + 4];
          snprintf(formatarg, sizeof(formatarg), "-F%d", nav->format);
          snprintf(patharg, sizeof(patharg), "-I%s", nav->pathraw);
          argv[argc] = strdup(formatarg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
          argv[argc] = strdup(patharg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
        }
      }

      if (alloc_ok) {
        argv[argc] = NULL;
        fprintf(stderr, "Calling mbnavedit with %d arguments\n", argc);
        mbgrdviz_launch_background(argv);
      }
      else {
        fprintf(stderr, "\nUnable to allocate memory for mbnavedit arguments\n");
        XBell((Display *)XtDisplay(mainWindow), 100);
      }

      for (int i = 0; i < argc; i++)
        free(argv[i]);
      free(argv);
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

void do_mbgrdviz_open_mbvelocitytool(Widget w, XtPointer client_data, XtPointer call_data) {
  int status = MB_SUCCESS;

  /* mbview instance */
  size_t instance;
  struct mbview_struct *data;
  struct mbview_shareddata_struct *shareddata;
  struct mbview_nav_struct *nav;
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
  if (status == MB_SUCCESS && shareddata->nnav > 0) {
    for (int i = 0; i < shareddata->nnav; i++) {
      nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
      fprintf(stderr, "Nav %d name:%s path:%s format:%d nselected:%d\n", i, nav->name, nav->pathraw, nav->format,
              nav->nselected);
      if (nav->nselected > 0) {
        nselected += nav->nselected;
      }
    }
  }

  /* open all data files with selected nav into mbvelocitytool */
  if (status == MB_SUCCESS && shareddata->nnav > 0 && nselected > 0) {
    char **argv = (char **)malloc((size_t)(1 + 2 * shareddata->nnav + 1) * sizeof(char *));
    if (argv == NULL) {
      fprintf(stderr, "\nUnable to allocate memory for mbvelocitytool argument list\n");
      XBell((Display *)XtDisplay(mainWindow), 100);
    }
    else {
      int argc = 0;
      bool alloc_ok = true;
      argv[argc] = strdup("mbvelocitytool");
      if (argv[argc] == NULL)
        alloc_ok = false;
      else
        argc++;
      for (int i = 0; alloc_ok && i < shareddata->nnav; i++) {
        nav = (struct mbview_nav_struct *)&(shareddata->navs[i]);
        if (nav->nselected > 0) {
          char formatarg[32];
          char patharg[MB_PATH_MAXLINE + 4];
          snprintf(formatarg, sizeof(formatarg), "-F%d", nav->format);
          snprintf(patharg, sizeof(patharg), "-I%s", nav->pathraw);
          argv[argc] = strdup(formatarg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
          argv[argc] = strdup(patharg);
          if (argv[argc] == NULL) {
            alloc_ok = false;
            break;
          }
          argc++;
        }
      }

      if (alloc_ok) {
        argv[argc] = NULL;
        fprintf(stderr, "Calling mbvelocitytool with %d arguments\n", argc);
        mbgrdviz_launch_background(argv);
      }
      else {
        fprintf(stderr, "\nUnable to allocate memory for mbvelocitytool arguments\n");
        XBell((Display *)XtDisplay(mainWindow), 100);
      }

      for (int i = 0; i < argc; i++)
        free(argv[i]);
      free(argv);
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
    /* delete current working route if defined - but first confirm that
       working_route still refers to the survey route we created. Its
       index can go stale if the user deleted some other route via
       3D-view picking while this dialog stayed open, which shifts route
       array indices and would otherwise make working_route refer to an
       unrelated route */
    if (working_route > -1) {
      int wr_nroutewaypoint, wr_nroutpoint, wr_routecolor, wr_routesize;
      double wr_routedistancelateral, wr_routedistancetopo;
      char wr_routename[MB_PATH_MAXLINE];
      int wr_status = mbview_getrouteinfo(verbose, instance, working_route, &wr_nroutewaypoint, &wr_nroutpoint,
                                          wr_routename, &wr_routecolor, &wr_routesize, &wr_routedistancelateral,
                                          &wr_routedistancetopo, &error);
      if (wr_status == MB_SUCCESS && strcmp(wr_routename, survey_name) == 0) {
        mbview_deleteroute(verbose, instance, working_route, &error);
      }
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
      nlines = 1;
      segment.nls = 0;
      segment.nls_alloc = 0;
      segment.lspoints = NULL;

      if (status == MB_SUCCESS) {
        bool line_spacing_limited = false;
        xx[0] = -dsign * 0.5 * r;

        while (nlines == 1 || fabs(xx[nlines - 1] + dsign * 0.5 * line_spacing_use) < 0.5 * r) {
          /* allocate more space for xx if needed */
          if (nlines_alloc <= nlines) {
            nlines_alloc += 100;
            status = mb_reallocd(verbose, __FILE__, __LINE__, nlines_alloc * sizeof(double), (void **)&xx, &error);
            if (status != MB_SUCCESS) {
              nlines_alloc = 0;
              mb_error(verbose, error, &error_message);
              fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
              break;
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

          /* keep the number of lines finite when the altitude (and thus the
             swath width) is near zero */
          if (!(line_spacing_use >= r / MBGRDVIZ_SURVEY_MAXLINES)) {
            line_spacing_use = r / MBGRDVIZ_SURVEY_MAXLINES;
            line_spacing = line_spacing_use * data->area.width / r;
            if (!line_spacing_limited) {
              fprintf(stderr, "Survey line spacing limited to %.3f m (at most %d lines) because the altitude is near zero\n",
                      line_spacing, MBGRDVIZ_SURVEY_MAXLINES);
              line_spacing_limited = true;
            }
          }

          xx[nlines] = xx[nlines - 1] + dsign * line_spacing_use;
          nlines++;
        }
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

  /* get source mbview instance - the callbacks are registered with
      client_data 0, so use survey_instance as the other survey callbacks do */
  const size_t instance = survey_instance;

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

    mb_get_text_string(textField_arearoute_name, survey_name, sizeof(survey_name));
    if (strlen(survey_name) <= 0) {
      sprintf(survey_name, "Survey");
      XmTextSetString(textField_arearoute_name, survey_name);
    }

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
      int nroutewaypoint = 0;
      int nroutpoint = 0;
      char routename[MB_PATH_MAXLINE] = "";
      int routecolor;
      int routesize;
      double routedistancelateral = 0.0;
      double routedistancetopo = 0.0;

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
/*---------------------------------------------------------------------------------------*/
/* GIS polylines, polygons, and points: MB-System polyline (.pln), polygon (.plg),
    and point (.pts) files and ESRI shapefiles, read and written with mb_gisfile.c
    in the mbaux library and displayed (not edited) by mbview */
/*---------------------------------------------------------------------------------------*/
static void do_mbgrdviz_gisdialog(Widget w, size_t instance, char *title, char *pattern, size_t mode,
                                  XtPointer call_data) {
  /* set title to file dialog  */
  Cardinal ac = 0;
  Arg args[256];
  XtSetArg(args[ac], XmNtitle, title);
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set fileSelectionBox parameters */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, pattern, XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  const size_t actionid = mode * MBV_MAX_WINDOWS + instance;
  XtSetArg(args[ac], XmNuserData, (XtPointer)actionid);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
}
/*---------------------------------------------------------------------------------------*/
void do_mbgrdviz_fileSelectionBox_openpolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Open Polyline File", "*.pln", MBGRDVIZ_OPENPOLYLINE, call_data);
}
void do_mbgrdviz_fileSelectionBox_openpolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Open Polygon File", "*.plg", MBGRDVIZ_OPENPOLYGON, call_data);
}
void do_mbgrdviz_fileSelectionBox_opengispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Open Point File", "*.pts", MBGRDVIZ_OPENGISPOINT, call_data);
}
void do_mbgrdviz_fileSelectionBox_importshapefile(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Import Shapefile", "*.shp", MBGRDVIZ_IMPORTSHAPEFILE, call_data);
}
void do_mbgrdviz_fileSelectionBox_savepolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Save Polyline File", "*.pln", MBGRDVIZ_SAVEPOLYLINE, call_data);
}
void do_mbgrdviz_fileSelectionBox_savepolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Save Polygon File", "*.plg", MBGRDVIZ_SAVEPOLYGON, call_data);
}
void do_mbgrdviz_fileSelectionBox_savegispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Save Point File", "*.pts", MBGRDVIZ_SAVEGISPOINT, call_data);
}
void do_mbgrdviz_fileSelectionBox_exportpolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Export Polylines as Shapefile", "*.shp", MBGRDVIZ_EXPORTPOLYLINE, call_data);
}
void do_mbgrdviz_fileSelectionBox_exportpolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Export Polygons as Shapefile", "*.shp", MBGRDVIZ_EXPORTPOLYGON, call_data);
}
void do_mbgrdviz_fileSelectionBox_exportgispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  do_mbgrdviz_gisdialog(w, (size_t)client_data, "Export Points as Shapefile", "*.shp", MBGRDVIZ_EXPORTGISPOINT, call_data);
}
/*---------------------------------------------------------------------------------------*/
/* add the GIS actions to the action menu of an mbview window */
void do_mbgrdviz_addgisactions(size_t instance) {
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openpolyline, "Open Polyline File", MBV_PICKMASK_NONE,
                   &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_openpolygon, "Open Polygon File", MBV_PICKMASK_NONE,
                   &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_opengispoint, "Open Point File", MBV_PICKMASK_NONE,
                   &error);
  mbview_addactionseparator(verbose, instance, &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_importshapefile, "Import Shapefile", MBV_PICKMASK_NONE,
                   &error);
  mbview_addactionseparator(verbose, instance, &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savepolyline, "Save Polyline File",
                   MBV_EXISTMASK_POLYLINE, &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savepolygon, "Save Polygon File", MBV_EXISTMASK_POLYGON,
                   &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_savegispoint, "Save Point File", MBV_EXISTMASK_GISPOINT,
                   &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_exportpolyline, "Export Polylines as Shapefile",
                   MBV_EXISTMASK_POLYLINE, &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_exportpolygon, "Export Polygons as Shapefile",
                   MBV_EXISTMASK_POLYGON, &error);
  mbview_addaction(verbose, instance, do_mbgrdviz_fileSelectionBox_exportgispoint, "Export Points as Shapefile",
                   MBV_EXISTMASK_GISPOINT, &error);
}
/*---------------------------------------------------------------------------------------*/
/* add GIS data read from a file to mbview: each layer of the file becomes an
    mbview layer, then the display of the type is turned on */
/*---------------------------------------------------------------------------------------*/
/* name of a GIS type for messages */
/*---------------------------------------------------------------------------------------*/
/* read an MB-System polyline, polygon, or point file */
/*---------------------------------------------------------------------------------------*/
/* import the polylines, polygons, or points of a shapefile that overlap the grid
    displayed in this mbview window */
/*---------------------------------------------------------------------------------------*/
/* write all polylines, polygons, or points (shown or hidden) to an MB-System
    file or a shapefile; z values are those read or imported, and are written
    if any layer of the type had them */
/*---------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
/* signal a refused or failed action (mbgrdviz_core.c) */
void mbgrdviz_ui_bell(void) { XBell((Display *)XtDisplay(mainWindow), 100); }
/*------------------------------------------------------------------------------*/
