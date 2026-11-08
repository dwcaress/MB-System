/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_callbacks.c    4/27/2007
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
/*
 *
 * MBeditviz is an interactive swath bathymetry editor and patch
 * test tool for  MB-System.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:  D. W. Caress
 * Date:  April 27, 2007
 */

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"
#include "mbsys_singlebeam.h"
#include "mb_xmutil.h"

#include <Xm/Xm.h>

#include "mbview.h"
#include "mbeditviz.h"

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include "mbeditviz_creation.h"

/* GUI debugging define */
//#define MBEDITVIZ_GUI_DEBUG 1

/* fileSelectionBox modes */
#define MBEDITVIZ_OPENSWATH 1

/* Projection defines */
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

/* widgets */
static XtAppContext app;
static Widget parent;
static int mbview_id[MBV_MAX_WINDOWS];
static Widget fileSelectionList;
static Widget fileSelectionText;
int mformat;
static Cardinal ac;
static Arg args[256];
static char value_text[MB_PATH_MAXLINE];

Widget BxFindTopShell(Widget);
WidgetList BxWidgetIdsFromNames(Widget, char *, char *);

static char *mbev_grid_algorithm_label[] = {"Simple Mean", "Footprint", "Shoal Bias"};

/*---------------------------------------------------------------------------------------*/
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

  // This function returns a NULL terminated WidgetList.  The memory for
  // the list needs to be freed when it is no longer needed.
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxManageCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    XtManageChild(widgets[i]);
    i++;
  }
  XtFree((char *)widgets);
}

/*---------------------------------------------------------------------------------------*/
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

  // This function returns a NULL terminated WidgetList.  The memory for
  // the list needs to be freed when it is no longer needed.
  WidgetList widgets = BxWidgetIdsFromNames(w, "BxUnmanageCB", (String)client);

  int i = 0;
  while (widgets && widgets[i] != NULL) {
    XtUnmanageChild(widgets[i]);
    i++;
  }
  XtFree((char *)widgets);
}

/*---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------*/

int do_mbeditviz_init(Widget parentwidget, XtAppContext appcon) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       mbev_verbose:   %d\n", mbev_verbose);
    fprintf(stderr, "dbg2       parentwidget:   %p\n", parentwidget);
    fprintf(stderr, "dbg2       appcon:         %p\n", appcon);
  }

  parent = parentwidget;
  app = appcon;
  mbev_message_on = false;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_init\n");
#endif

  // set about version label
  snprintf(value_text, sizeof(value_text), "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION, MB_VERSION_DATE);
  set_mbview_label_multiline_string(label_about_version, value_text);

  // set file selection widgets
  fileSelectionList = (Widget)XtNameToWidget(fileSelectionBox, "*ItemsList");
  fileSelectionText = (Widget)XtNameToWidget(fileSelectionBox, "Text");
  XtUnmanageChild((Widget)XtNameToWidget(fileSelectionBox, "Help"));
	XtAddCallback(fileSelectionList, XmNbrowseSelectionCallback, do_mbeditviz_fileselection_list, NULL);

  mformat = -1;
  snprintf(value_text, sizeof(value_text), "%d", mformat);
  XmTextSetString(text_format, value_text);

  /* set the output mode */
  if (mbev_mode_output == MBEV_OUTPUT_MODE_EDIT) {
    XmToggleButtonSetState(toggleButton_mode_edit, TRUE, FALSE);
    XmToggleButtonSetState(toggleButton_mode_browse, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_openmodeedit, TRUE, FALSE);
    XmToggleButtonSetState(toggleButton_openmodebrowse, FALSE, FALSE);
  }
  else {
    XmToggleButtonSetState(toggleButton_mode_edit, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_mode_browse, TRUE, FALSE);
    XmToggleButtonSetState(toggleButton_openmodeedit, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_openmodebrowse, TRUE, FALSE);
  }

  /* initialize mbview_id list */
  for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
    mbview_id[i] = false;
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_mbeditviz_update_gui();

  /* set timer for function to keep updating the filelist */
  timer_function_set = false;
  do_mbeditviz_settimer();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_init status:%d\n", mbev_status);
#endif

  return (0);
}
/*---------------------------------------------------------------------------------------*/

void do_mbeditviz_mode_change(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_mode_change\n");
#endif

  XmToggleButtonCallbackStruct *acs = (XmToggleButtonCallbackStruct *)call_data;

  /* set values if needed */
  if (acs->reason == XmCR_VALUE_CHANGED) {
    if (acs->set)
      mbev_mode_output = MBEV_OUTPUT_MODE_EDIT;
    else
      mbev_mode_output = MBEV_OUTPUT_MODE_BROWSE;
    if (mbev_mode_output == MBEV_OUTPUT_MODE_EDIT) {
      XmToggleButtonSetState(toggleButton_mode_edit, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_mode_browse, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodeedit, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodebrowse, FALSE, FALSE);
    }
    else {
      XmToggleButtonSetState(toggleButton_mode_edit, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_mode_browse, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodeedit, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodebrowse, TRUE, FALSE);
    }
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "acs->set:%d mode:%d\n", acs->set, mbev_mode_output);
#endif
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_mode_change status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_openfile(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_openfile\n");
#endif
  /* read the mbio format number */
  mb_get_text_string(text_format, value_text, sizeof(value_text));
  int format = 0;
  if (sscanf(value_text, "%d", &format) != 1)
    format = 0;

  XmFileSelectionBoxCallbackStruct *acs = (XmFileSelectionBoxCallbackStruct *)call_data;

  /* read the input file name */
  // char *file_ptr;
  // XmStringGetLtoR(acs->value, XmSTRING_DEFAULT_CHARSET, &file_ptr);
	char *file_ptr = (char *)XmStringUnparse(acs->value, NULL, XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);
  if (file_ptr != NULL && strlen(file_ptr) == 0) {
    XtFree(file_ptr);
    file_ptr = NULL;
  }

  /* open data */
  if (file_ptr != NULL) {
    mbev_status = do_mbeditviz_opendata(file_ptr, format);
    XtFree(file_ptr);
    file_ptr = NULL;
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_openfile status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_fileselection_list\n");
#endif

  static char selection_text[MB_PATH_MAXLINE];

  /* get selected text */
  mb_get_text_string(fileSelectionText, selection_text, sizeof(selection_text));

  /* get output file */
  if ((int)strlen(selection_text) > 0) {
    /* look for MB suffix convention */
    int form = mformat;
    if ((mbev_status = mbeditviz_get_format(selection_text, &form)) == MB_SUCCESS) {
      mformat = form;
      char value_text[10];
      snprintf(value_text, sizeof(value_text), "%d", mformat);
      XmTextSetString(text_format, value_text);
    }
  }
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_fileselection_list status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_fileSelectionBox_openswath\n");
#endif

  /* set title to open swath data */
  ac = 0;
  XtSetArg(args[ac], XmNtitle, "Open Swath Data");
  ac++;
  XtSetValues(dialogShell_open, args, ac);
  BxManageCB(w, (XtPointer) "fileSelectionBox", call_data);

  /* set filter */
  ac = 0;
  Boolean argok;
  XmString tmp0 = (XmString)BX_CONVERT(dialogShell_open, "*.mb*", XmRXmString, 0, &argok);
  XtSetArg(args[ac], XmNpattern, tmp0);
  ac++;
  XtSetValues(fileSelectionBox, args, ac);
  XmStringFree((XmString)tmp0);
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_fileSelectionBox_openswath status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_viewselected(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

  /* destroy the views and the old grid */
  do_mbeditviz_viewselected_prepare();

  /* get positions of selected list items */
  ac = 0;
  int position_count = 0;
  XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
  ac++;
  int *position_list = NULL;
  XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
  ac++;
  XtGetValues(list_filelist, args, ac);

  /* load and view the selected files */
  do_mbeditviz_viewselected_files(w, client_data, call_data, position_count, position_list);
}
/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

void do_mbeditviz_set_label_implied() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_set_label_implied\n");
#endif

  char string[MB_PATH_MAXLINE];
  // char string2[MB_PATH_MAXLINE];
  snprintf(string, sizeof(string), ":::t\"Selected Grid Parameters:\""
                  ":t\"    Cell Size: %.2f m\""
                  ":t\"    Dimensions: %d %d\""
                  ":t\"    Algorithm: %s\""
                  ":t\"    Interpolation: %d cell gaps\"",
                  mbev_grid_cellsize,
                  mbev_grid_n_columns, mbev_grid_n_rows,
                  mbev_grid_algorithm_label[mbev_grid_algorithm],
                  mbev_grid_interpolation);
  set_label_multiline_string(label_implied, (String)string);
}

/*--------------------------------------------------------------------*/
void do_mbeditviz_changecellsize(Widget w, XtPointer client_data, XtPointer call_data) {
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  // char string[MB_PATH_MAXLINE];

  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_changecellsize\n");
#endif

  /* get cell size value */
  ac = 0;
  int icellsize;
  int iscalemax;
  XtSetArg(args[ac], XmNvalue, &icellsize);
  ac++;
  XtSetArg(args[ac], XmNmaximum, &iscalemax);
  ac++;
  XtGetValues(scale_cellsize, args, ac);
  /* a zero cell size would make the grid dimensions infinite */
  mbev_grid_cellsize = 0.001 * MAX(icellsize, 1);

  /* reset the scale maximum, keeping it above the scale minimum (0) */
  if (icellsize <= 1 && iscalemax > 2) {
    iscalemax = MAX(iscalemax / 2, 2);
    ac = 0;
    XtSetArg(args[ac], XmNmaximum, iscalemax);
    ac++;
    XtSetValues(scale_cellsize, args, ac);
  }
  else if (icellsize == iscalemax) {
    iscalemax *= 2;
    ac = 0;
    XtSetArg(args[ac], XmNmaximum, iscalemax);
    ac++;
    XtSetValues(scale_cellsize, args, ac);
  }


  /* get updated grid dimensions */
  mbev_grid_n_columns = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]) / mbev_grid_cellsize + 1;
  mbev_grid_n_rows = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]) / mbev_grid_cellsize + 1;
  if (mbev_verbose > 0) {
    fprintf(stderr, "Grid bounds: %f %f %f %f    %f %f %f %f\n",
            mbev_grid_bounds[0], mbev_grid_bounds[1],
            mbev_grid_bounds[2], mbev_grid_bounds[3],
            mbev_grid_boundsutm[0], mbev_grid_boundsutm[1],
            mbev_grid_boundsutm[2],  mbev_grid_boundsutm[3]);
    fprintf(stderr, "cell size:%f dimensions: %d %d\n", mbev_grid_cellsize, mbev_grid_n_columns, mbev_grid_n_rows);
  }

  do_mbeditviz_set_label_implied();

}

/*--------------------------------------------------------------------*/
void do_mbeditviz_gridparameters(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_gridparameters\n");
#endif

  /* get calculated grid parameters */
  mbeditviz_get_grid_bounds();

  /* set the widgets */
  ac = 0;
  int icellsize = (int)(1000 * mbev_grid_cellsize);
  XtSetArg(args[ac], XmNvalue, icellsize);
  ac++;
  XtSetArg(args[ac], XmNmaximum, MAX(5 * icellsize, 2));
  ac++;
  XtSetValues(scale_cellsize, args, ac);

  if (mbev_grid_algorithm == MBEV_GRID_ALGORITHM_SIMPLEMEAN) {
    XmToggleButtonSetState(toggleButton_gridalgorithm_simplemean, TRUE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_footprint, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_shoalbias, FALSE, FALSE);
  } else if (mbev_grid_algorithm == MBEV_GRID_ALGORITHM_FOOTPRINT) {
    XmToggleButtonSetState(toggleButton_gridalgorithm_simplemean, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_footprint, TRUE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_shoalbias, FALSE, FALSE);
  } else if (mbev_grid_algorithm == MBEV_GRID_ALGORITHM_SHOALBIAS) {
    XmToggleButtonSetState(toggleButton_gridalgorithm_simplemean, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_footprint, FALSE, FALSE);
    XmToggleButtonSetState(toggleButton_gridalgorithm_shoalbias, TRUE, FALSE);
  }

  snprintf(value_text, sizeof(value_text), "%d", mbev_grid_interpolation);
  XmTextSetString(text_interpolation, value_text);

  double xx = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]);
  double yy = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]);

  char string[MB_PATH_MAXLINE];
  snprintf(string, sizeof(string), ":::t\"Grid Bounds:\""
          ":t\"    Longitude: %10.5f %10.5f  | %6.3f km  | %9.3f m\""
          ":t\"    Latitude: %9.5f %9.5f | %6.3f km  | %9.3f m\""
          ":t\"Suggested Grid Parameters:\""
          ":t\"    Cell Size: %.2f m\""
          ":t\"    Dimensions: %d %d\"",
          mbev_grid_bounds[0], mbev_grid_bounds[1], 0.001 * xx, xx,
          mbev_grid_bounds[2], mbev_grid_bounds[3], 0.001 * yy, yy,
          mbev_grid_cellsize, mbev_grid_n_columns, mbev_grid_n_rows);
  set_label_multiline_string(label_current, (String)string);

  do_mbeditviz_set_label_implied();

}

/*--------------------------------------------------------------------*/

extern void do_mbeditviz_gridalgorithm_change(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_gridalgorithm_change\n");
#endif

  /* get grid definition values from the dismissed dialog */
  if (XmToggleButtonGetState(toggleButton_gridalgorithm_simplemean)) {
    mbev_grid_algorithm = MBEV_GRID_ALGORITHM_SIMPLEMEAN;
  } else if (XmToggleButtonGetState(toggleButton_gridalgorithm_footprint)) {
    mbev_grid_algorithm = MBEV_GRID_ALGORITHM_FOOTPRINT;
  } else /* if (XmToggleButtonGetState(toggleButton_gridalgorithm_shoalbias)) */ {
    mbev_grid_algorithm = MBEV_GRID_ALGORITHM_SHOALBIAS;
  }

  /* get interpolation scale */
  mb_get_text_string(text_interpolation, value_text, sizeof(value_text));
  sscanf(value_text, "%d", &mbev_grid_interpolation);

  do_mbeditviz_set_label_implied();

}

/*--------------------------------------------------------------------*/
extern void do_mbeditviz_gridinterpolation_change(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_gridinterpolation_change\n");
#endif

  /* get interpolation scale */
  mb_get_text_string(text_interpolation, value_text, sizeof(value_text));
  sscanf(value_text, "%d", &mbev_grid_interpolation);

  do_mbeditviz_set_label_implied();

}

/*--------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_deleteselected(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_deleteselected\n");
#endif

  /* get positions of selected list items */
  ac = 0;
  int position_count = 0;
  XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
  ac++;
  int *position_list = NULL;
  XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
  ac++;
  XtGetValues(list_filelist, args, ac);

  /* delete the selected files */
  for (int i = position_count - 1; i >= 0; i--)
    mbeditviz_delete_file(position_list[i] - 1);

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_deleteselected status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_changeoutputmode(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)client_data;
  XmToggleButtonCallbackStruct *acs = (XmToggleButtonCallbackStruct *)call_data;

  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    // fprintf(stderr, "dbg2       w:           %p\n", w);
    // fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    // fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_changeoutputmode\n");
#endif

  /* set values if needed */
  if (acs->reason == XmCR_VALUE_CHANGED) {
    if (acs->set)
      mbev_mode_output = MBEV_OUTPUT_MODE_EDIT;
    else
      mbev_mode_output = MBEV_OUTPUT_MODE_BROWSE;
    if (mbev_mode_output == MBEV_OUTPUT_MODE_EDIT) {
      XmToggleButtonSetState(toggleButton_mode_edit, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_mode_browse, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodeedit, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodebrowse, FALSE, FALSE);
    }
    else {
      XmToggleButtonSetState(toggleButton_mode_edit, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_mode_browse, TRUE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodeedit, FALSE, FALSE);
      XmToggleButtonSetState(toggleButton_openmodebrowse, TRUE, FALSE);
    }
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "acs->set:%d mbev_mode_output:%d\n", acs->set, mbev_mode_output);
#endif
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_changeoutputmode status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_update_gui() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_update_gui status:%d\n", mbev_status);
#endif

  /* set status text */
  mbev_num_files_loaded = 0;
  mbev_num_pings_loaded = 0;
  mbev_num_soundings_loaded = 0;
  mbev_num_soundings_secondary = 0;
  for (int i = 0; i < mbev_num_files; i++) {
    struct mbev_file_struct *file = &(mbev_files[i]);
    if (file->load_status) {
      mbev_num_files_loaded++;
      mbev_num_pings_loaded += file->num_pings;
      for (int j = 0; j < file->num_pings; j++) {
        struct mbev_ping_struct *ping = &(file->pings[j]);
        for (int k = 0; k < ping->beams_bath; k++) {
          if (!mb_beam_check_flag_unusable(ping->beamflag[k]))
            mbev_num_soundings_loaded++;
          if (mb_beam_check_flag_multipick(ping->beamflag[k]))
            mbev_num_soundings_secondary++;
        }
      }
    }
  }

  char string[MB_PATH_MAXLINE];
  if (mbev_grid.status == MBEV_GRID_NONE)
    snprintf(string, sizeof(string), ":::t\"Available Files: %d\":t\"Loaded Files: %d\":t\"Grid Not Generated\"", mbev_num_files,
            mbev_num_files_loaded);
  else
    snprintf(string, sizeof(string),
            ":::t\"Available Files: %d\":t\"Loaded Files: %d\":t\"Grid:\":t\"  Lon: %f %f\":t\"  Lat: %f %f\":t\"  Cell "
            "Size: %f m\":t\"  Algorithm: %d\":t\"  Interpolation: %d\":t\"  Dimensions: %d %d\"",
            mbev_num_files, mbev_num_files_loaded, mbev_grid.bounds[0], mbev_grid.bounds[1], mbev_grid.bounds[2],
            mbev_grid.bounds[3], mbev_grid.dx, mbev_grid_algorithm, mbev_grid_interpolation,
            mbev_grid.n_columns, mbev_grid.n_rows);
  set_label_multiline_string(label_mbeditviz_status, (String)string);

  /* build available file list */
  do_mbeditviz_update_filelist();

  /*set sensitivity */
  if (mbev_grid.status == MBEV_GRID_NONE) {
    XtVaSetValues(pushButton_openswath, XmNsensitive, True, NULL);
  }
  else {
    XtVaSetValues(pushButton_openswath, XmNsensitive, False, NULL);
  }
  if (mbev_num_files > 0 && mbev_grid.status == MBEV_GRID_NONE) {
    XtVaSetValues(pushButton_deleteselected, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_viewselected, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_viewall, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_mode_edit, XmNsensitive, True, NULL);
    XtVaSetValues(toggleButton_mode_browse, XmNsensitive, True, NULL);
    XtVaSetValues(pushButton_updategrid, XmNsensitive, False, NULL);
  }
  else {
    XtVaSetValues(pushButton_deleteselected, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_viewselected, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_viewall, XmNsensitive, False, NULL);
    XtVaSetValues(toggleButton_mode_edit, XmNsensitive, False, NULL);
    XtVaSetValues(toggleButton_mode_browse, XmNsensitive, False, NULL);
    XtVaSetValues(pushButton_updategrid, XmNsensitive, True, NULL);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_update_gui status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_update_filelist() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_update_filelist status:%d\n", mbev_status);
#endif

  struct mbev_file_struct *file;

  char string[MB_PATH_MAXLINE+40];
  int *position_list = NULL;
  int position_list_save[MB_PATH_MAXLINE];
  int position_count = 0;
  char athchar;
  char atschar;
  char atachar;
  XmString *xstr;
  char *lockstrptr;
  const char *lockedstr = "<Locked>";
  const char *unlockedstr = "        ";
  const char *loadedstr = "<loaded>";
  const char *esfyesstr = "<esf>";
  const char *esfnostr = "     ";

  /* esf file checking variables */
  bool esf_exists = false;
  struct stat file_status;
  char save_file[MB_PATH_MAXLINE+40];

  /* check to see if anything has changed */
  bool update_filelist = false;

  /* check for change in number of files */
  Cardinal ac = 0;
  int item_count;
  Arg args[256];
  XtSetArg(args[ac], XmNitemCount, (XtPointer)&item_count);
  ac++;
  XtGetValues(list_filelist, args, ac);
  if (item_count != mbev_num_files)
    update_filelist = true;

  /* check for change in load status, lock status, or esf status */
  for (int i = 0; i < mbev_num_files; i++) {
    file = &(mbev_files[i]);

    /* check load status */
    if (file->load_status != file->load_status_shown) {
      file->load_status_shown = file->load_status;
      update_filelist = true;
    }

		/* swath file locking variables */
		int lock_error = MB_ERROR_NO_ERROR;
		bool locked = false;
		int lock_purpose;
		mb_path lock_program;
		mb_path lock_cpu;
		mb_path lock_user;
		char lock_date[25];

    /* check for locks */
    // int lock_status =
    mb_pr_lockinfo(mbev_verbose, mbev_files[i].path, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                   lock_date, &lock_error);
    if (locked != file->locked) {
      file->locked = locked;
      update_filelist = true;
    }

    /* check for edit save file */
    snprintf(save_file, sizeof(save_file), "%s.esf", mbev_files[i].path);
    const int fstat = stat(save_file, &file_status);
    if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
      esf_exists = true;
    else
      esf_exists = false;
    if (esf_exists != file->esf_exists) {
      file->esf_exists = esf_exists;
      update_filelist = true;
    }
  }

  /* only update the filelist if necessary */
  if (update_filelist) {
    /* get the current selection, if any, from the list */
    ac = 0;
    XtSetArg(args[ac], XmNitemCount, (XtPointer)&item_count);
    ac++;
    XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
    ac++;
    XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
    ac++;
    XtGetValues(list_filelist, args, ac);
    if (position_count > MB_PATH_MAXLINE)
      position_count = MB_PATH_MAXLINE;
    for (int i = 0; i < position_count; i++) {
      position_list_save[i] = position_list[i];
    }

    /* delete existing file list */
    XmListDeleteAllItems(list_filelist);

    /* build available file list */
    if (mbev_num_files > 0) {
      xstr = (XmString *)malloc(mbev_num_files * sizeof(XmString));
      if (xstr == NULL) {
        /* out of memory - report it and leave the list empty rather than
            writing through a NULL pointer below */
        do_error_dialog("Unable to update the file list", "due to a memory", "allocation error!");
      }
      else {
        char *esfstrptr;
        for (int i = 0; i < mbev_num_files; i++) {
          file = &(mbev_files[i]);

          /* set label strings */
          if (file->load_status)
            lockstrptr = (char *) loadedstr;
          else if (file->locked)
            lockstrptr = (char *) lockedstr;
          else
            lockstrptr = (char *) unlockedstr;
          if (file->esf_exists)
            esfstrptr = (char *) esfyesstr;
          else
            esfstrptr = (char *) esfnostr;
          if (file->n_async_heading > 0)
            athchar = 'H';
          else
            athchar = ' ';
          if (file->n_async_sensordepth > 0)
            atschar = 'S';
          else
            atschar = ' ';
          if (file->n_async_attitude > 0)
            atachar = 'A';
          else
            atachar = ' ';
          snprintf(string, sizeof(string), "%s %s %c%c%c %s %d", lockstrptr, esfstrptr, athchar, atschar, atachar, mbev_files[i].name,
                  mbev_files[i].format);
          xstr[i] = XmStringCreateLocalized(string);
        }
        XmListAddItems(list_filelist, xstr, mbev_num_files, 0);
        for (int i = 0; i < mbev_num_files; i++) {
          XmStringFree(xstr[i]);
        }
        free(xstr);
      }

      /* reinstate selection if the number of items is the same as before */
      if (item_count == mbev_num_files && position_count > 0) {
        ac = 0;
        XtSetArg(args[ac], XmNselectedPositionCount, position_count);
        ac++;
        XtSetArg(args[ac], XmNselectedPositions, (XtPointer)position_list_save);
        ac++;
        XtSetValues(list_filelist, args, ac);
      }
    }
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_update_filelist status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* Mbeditviz message functions */
/*------------------------------------------------------------------------------*/

int do_mbeditviz_message_on(char *message) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       message:   %s\n", message);
  }

  mbev_message_on = true;

  set_mbview_label_string(label_mbeditviz_message, message);
  XtManageChild(bulletinBoard_mbeditviz_message);
  XSync(XtDisplay(bulletinBoard_mbeditviz_message), 0);
  XmUpdateDisplay(bulletinBoard_mbeditviz_message);

  return (1);
}

/*------------------------------------------------------------------------------*/

int do_mbeditviz_message_off() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  mbev_message_on = false;

  XtUnmanageChild(bulletinBoard_mbeditviz_message);
  XSync(XtDisplay(bulletinBoard_mbeditviz_message), 0);
  XmUpdateDisplay(bulletinBoard_mbeditviz_message);

  return (1);
}
/*--------------------------------------------------------------------*/

int do_error_dialog(char *s1, char *s2, char *s3) {
  set_label_string(label_error_one, s1);
  set_label_string(label_error_two, s2);
  set_label_string(label_error_three, s3);
  XtManageChild(bulletinBoard_error);
  XBell(XtDisplay(bulletinBoard_error), 100);
  /* fprintf(stderr,"do_error_dialog:\n\t%s\n\t%s\n\t%s\n",s1,s2,s3); */

  return (1);
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

int do_wait_until_viewed() {
  Widget topshell;

  /* find the top level shell */
  for (topshell = parent; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
    ;

  /* keep processing events until it is viewed */
  if (XtIsRealized(topshell)) {
    Window topwindow = XtWindow(topshell);
    XWindowAttributes xwa;
    XEvent event;

    /* wait for the window to be mapped */
    while (XGetWindowAttributes(XtDisplay(parent), topwindow, &xwa) && xwa.map_state != IsViewable) {
      XtAppNextEvent(app, &event);
      XtDispatchEvent(&event);
    }
  }

  XmUpdateDisplay(topshell);

  return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/

int do_mbeditviz_settimer() {
  int status = MB_SUCCESS;

  /* set timer function if none set for this instance */
  if (!timer_function_set) {
    const int timer_timeout_time = 1000;
    int id = XtAppAddTimeOut(app, (unsigned long)timer_timeout_time, (XtTimerCallbackProc)do_mbeditviz_workfunction,
                         (XtPointer)-1);
    if (id > 0)
      timer_function_set = true;
    else
      status = MB_FAILURE;
  }

  /* else
  fprintf(stderr,"do_mbeditviz_settimer: FUNCTION ALREADY SET!!\n"); */

  return (status);
}

/*------------------------------------------------------------------------------*/

int do_mbeditviz_workfunction(XtPointer client_data) {
  (void)client_data;  // Unused parameter
  timer_function_set = false;

  /* reset filelist */
  if (mbev_num_files > 0) {
    do_mbeditviz_update_filelist();
  }

  /* reset the timer function */
  do_mbeditviz_settimer();

  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
void mbeditviz_ui_bell(void) {
  XBell(XtDisplay(list_filelist), 100);
}
