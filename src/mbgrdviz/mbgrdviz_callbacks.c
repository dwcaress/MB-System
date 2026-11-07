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

  /* open or save the file */
  mbgrdviz_openfile(mode, instance, file_ptr);

  /* free the string */
  if (file_ptr != NULL) {
    XtFree(file_ptr);
    file_ptr = NULL;
  }
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

void mbgrdviz_ui_arearoute_info(char *info_text) {
  set_mbview_label_multiline_string(label_arearoute_info, info_text);
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
/* show the path of the realtime data source in the realtime setup (mbgrdviz_core.c) */
void mbgrdviz_ui_realtime_path(char *path) { XmTextSetString(textField_realtimesetup_path, path); }
/*------------------------------------------------------------------------------*/
