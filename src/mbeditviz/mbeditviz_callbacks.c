/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_callbacks.c    4/27/2007
 *
 *    Copyright (c) 2007-2023 by
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
  sprintf(value_text, "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION, MB_VERSION_DATE);
  set_mbview_label_multiline_string(label_about_version, value_text);

  // set file selection widgets
  fileSelectionList = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_LIST);
  fileSelectionText = (Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_TEXT);
  XtAddCallback(fileSelectionList, XmNbrowseSelectionCallback, do_mbeditviz_fileselection_list, NULL);
  XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionBox, XmDIALOG_HELP_BUTTON));
  mformat = -1;
  sprintf(value_text, "%d", mformat);
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
  get_text_string(text_format, value_text);
  int format;
  sscanf(value_text, "%d", &format);

  XmFileSelectionBoxCallbackStruct *acs = (XmFileSelectionBoxCallbackStruct *)call_data;

  /* read the input file name */
  char *file_ptr;
  XmStringGetLtoR(acs->value, XmSTRING_DEFAULT_CHARSET, &file_ptr);
  if (strlen(file_ptr) <= 0 && file_ptr != NULL) {
    XtFree(file_ptr);
    file_ptr = NULL;
  }

  /* open data */
  mbev_status = do_mbeditviz_opendata(file_ptr, format);

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

  static char selection_text[128];

  /* get selected text */
  get_text_string(fileSelectionText, (String)selection_text);

  /* get output file */
  if ((int)strlen(selection_text) > 0) {
    /* look for MB suffix convention */
    int form = mformat;
    if ((mbev_status = mbeditviz_get_format(selection_text, &form)) == MB_SUCCESS) {
      mformat = form;
      char value_text[10];
      sprintf(value_text, "%d", mformat);
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
void do_mbeditviz_quit(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_quit\n");
#endif
  do_mbeditviz_message_on("Shutting down...");

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy the grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();

  /* loop over all files to be sure all files are unloaded */
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status) {
      // Unload file, asserting unlock
      mbeditviz_unload_file(ifile, true);
    }
  }

  /* reset the gui */
  do_mbeditviz_update_gui();
  do_mbeditviz_message_off();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_quit status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_viewall(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall destroyed previous windows\n");
#endif
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE) {
    mbeditviz_destroy_grid();
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall destroyed old grid\n");
#endif
  }


/* loop over all files to be sure all files are loaded */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall loading files...\n");
#endif
  do_mbeditviz_message_on("Loading files...");
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall loading file %d\n", ifile);
#endif
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (!file->load_status) {
      sprintf(value_text, "Loading file %d of %d...", ifile + 1, mbev_num_files);
      do_mbeditviz_message_on(value_text);
#ifdef MBEDITVIZ_GUI_DEBUG
      fprintf(stderr, "do_mbeditviz_viewall loading file %d of %d...\n", ifile + 1, mbev_num_files);
#endif
      // Load file, asserting lock
      mbeditviz_load_file(ifile, true);
    }
    loadcount++;
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  }
  do_mbeditviz_message_off();

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall calling do_mbeditviz_gridparameters\n");
#endif
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    XBell(XtDisplay(list_filelist), 100);
  }

/* reset the gui */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall calling do_mbeditviz_update_gui\n");
#endif
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewall status:%d\n", mbev_status);
#endif
}

/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_viewselected(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewselected\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();

  /* get positions of selected list items */
  ac = 0;
  int position_count = 0;
  XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
  ac++;
  int *position_list = NULL;
  XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
  ac++;
  XtGetValues(list_filelist, args, ac);
#ifdef MBEDITVIZ_GUI_DEBUG
  {
    fprintf(stderr, "position_count:%d\n", position_count);
    for (int i = 0; i < position_count; i++)
      fprintf(stderr, "  %d %d\n", i, position_list[i]);
  }
#endif


  /* loop over all files to be sure selected files are loaded */
  do_mbeditviz_message_on("Loading files...");
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    /* find out if file is in selected list */
    bool selected = false;
    for (int i = 0; i < position_count; i++) {
      if (ifile == position_list[i] - 1)
        selected = true;
    }

    /* load unloaded selected files, unload loaded unselected files */
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (selected && !file->load_status) {
      loadcount++;
      sprintf(value_text, "Loading file %d of %d...", loadcount, position_count);
      do_mbeditviz_message_on(value_text);
      // Load file, asserting lock
      mbeditviz_load_file(ifile, true);
    }
    else if (selected && file->load_status) {
      loadcount++;
    }
    else if (!selected && file->load_status) {
      // Unload file, asserting unlock
      mbeditviz_unload_file(ifile, true);
    }
  }
  do_mbeditviz_message_off();

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, " mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    XBell(XtDisplay(list_filelist), 100);
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewselected status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/
void do_mbeditviz_regrid(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_regrid\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();

  /* loop over all files to be count loaded files */
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status)
      loadcount++;
  }

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, " mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    XBell(XtDisplay(list_filelist), 100);
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_regrid status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/

void do_mbeditviz_updategrid(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_updategrid\n");
#endif
  do_mbeditviz_mbview_dismiss_notify(0);

  /* loop over all files to be sure all files are loaded */
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status)
      loadcount++;
  }

  /* make the grid and display grid */
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    /* make the grid */
    do_mbeditviz_message_on("Making grid...");
    mbev_status = mbeditviz_setup_grid();
    mbeditviz_project_soundings();
    mbev_status = mbeditviz_make_grid();

    /* display grid */
    do_mbeditviz_viewgrid();

    do_mbeditviz_message_off();
  }
  else {
    do_mbeditviz_message_off();
    XBell(XtDisplay(list_filelist), 100);
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_updategrid status:%d\n", mbev_status);
#endif
}
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
  sprintf(string, ":::t\"Selected Grid Parameters:\""
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
  mbev_grid_cellsize = 0.001 * icellsize;

  /* reset the scale maximum */
  if (icellsize <= 1) {
    iscalemax /= 2;
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
  XtSetArg(args[ac], XmNmaximum, 5 * icellsize);
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

  sprintf(value_text, "%d", mbev_grid_interpolation);
  XmTextSetString(text_interpolation, value_text);

  double xx = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]);
  double yy = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]);

  char string[MB_PATH_MAXLINE];
  sprintf(string, ":::t\"Grid Bounds:\""
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
  get_text_string(text_interpolation, value_text);
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
  get_text_string(text_interpolation, value_text);
  sscanf(value_text, "%d", &mbev_grid_interpolation);

  do_mbeditviz_set_label_implied();

}

/*--------------------------------------------------------------------*/
void do_mbeditviz_viewgrid() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewgrid\n");
#endif

  struct mbev_file_struct *file;
  struct mbev_ping_struct *ping;
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
  double mbv_secondary_colortable_min;
  double mbv_secondary_colortable_max;
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
  int ifile, iping;

  /* display grid */
  if (mbev_status == MB_SUCCESS && mbev_grid.status == MBEV_GRID_NOTVIEWED) {
    /* set parameters */
    sprintf(mbv_title, "MBeditviz Survey Viewer");
    mbv_xo = 200;
    mbv_yo = 200;
    mbv_width = 560;
    mbv_height = 500;
    mbv_lorez_dimension = 100;
    mbv_hirez_dimension = 500;
    mbv_lorez_navdecimate = 5;
    mbv_hirez_navdecimate = 1;
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
    mbv_primary_colortable_min = mbev_grid.min;
    mbv_primary_colortable_max = mbev_grid.max;
    mbv_slope_colortable = MBV_COLORTABLE_HAXBY;
    mbv_slope_colortable_mode = MBV_COLORTABLE_REVERSED;
    mbv_slope_colortable_min = 0.0;
    mbv_slope_colortable_max = 0.5;
    mbv_secondary_colortable = MBV_COLORTABLE_HAXBY;
    mbv_secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
    mbv_secondary_colortable_min = mbev_grid.smin;
    mbv_secondary_colortable_max = mbev_grid.smax;
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
    mbv_contour_interval = pow(10.0, floor(log10(mbev_grid.max - mbev_grid.min)) - 1.0);

    /* initialize mbview window */
    mbev_status = mbview_init(mbev_verbose, &mbev_instance, &mbev_error);

    /* set basic mbview window parameters */
    mbev_status = mbview_setwindowparms(mbev_verbose, mbev_instance, &do_mbeditviz_mbview_dismiss_notify, mbv_title, mbv_xo,
                                        mbv_yo, mbv_width, mbv_height, mbv_lorez_dimension, mbv_hirez_dimension,
                                        mbv_lorez_navdecimate, mbv_hirez_navdecimate, &mbev_error);

    /* set basic mbview view controls */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setviewcontrols(
          mbev_verbose, mbev_instance, mbv_display_mode, mbv_mouse_mode, mbv_grid_mode, mbv_primary_histogram,
          mbv_primaryslope_histogram, mbv_secondary_histogram, mbv_primary_shade_mode, mbv_slope_shade_mode,
          mbv_secondary_shade_mode, mbv_grid_contour_mode, mbv_site_view_mode, mbv_route_view_mode, mbv_nav_view_mode,
          mbv_navdrape_view_mode, mbv_vector_view_mode, mbv_exageration, mbv_modelelevation3d, mbv_modelazimuth3d,
          mbv_viewelevation3d, mbv_viewazimuth3d, mbv_illuminate_magnitude, mbv_illuminate_elevation,
          mbv_illuminate_azimuth, mbv_slope_magnitude, mbv_overlay_shade_magnitude, mbv_overlay_shade_center,
          mbv_overlay_shade_mode, mbv_contour_interval, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id, &mbev_error);

    /* set primary grid data */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setprimarygrid(mbev_verbose, mbev_instance, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id,
                                          mbev_grid.nodatavalue, mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.min, mbev_grid.max,
                                          mbev_grid.boundsutm[0], mbev_grid.boundsutm[1], mbev_grid.boundsutm[2],
                                          mbev_grid.boundsutm[3], mbev_grid.dx, mbev_grid.dy, mbev_grid.val, &mbev_error);

    /* set more mbview control values */
    if (mbev_status == MB_SUCCESS)
      mbev_status =
          mbview_setprimarycolortable(mbev_verbose, mbev_instance, mbv_primary_colortable, mbv_primary_colortable_mode,
                                      mbv_primary_colortable_min, mbv_primary_colortable_max, &mbev_error);
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setslopecolortable(mbev_verbose, mbev_instance, mbv_slope_colortable, mbv_slope_colortable_mode,
                                              mbv_slope_colortable_min, mbv_slope_colortable_max, &mbev_error);

    /* open up mbview window */
    mbev_status = mbview_open(mbev_verbose, mbev_instance, &mbev_error);

    /* set grid status */
    if (mbev_status == MB_SUCCESS)
      mbev_grid.status = MBEV_GRID_VIEWED;

    /* set more mbview control values */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setsecondarygrid(
          mbev_verbose, mbev_instance, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id, mbev_grid.nodatavalue,
          mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.smin, mbev_grid.smax, mbev_grid.boundsutm[0], mbev_grid.boundsutm[1],
          mbev_grid.boundsutm[2], mbev_grid.boundsutm[3], mbev_grid.dx, mbev_grid.dy, mbev_grid.sgm, &mbev_error);
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setsecondarycolortable(mbev_verbose, mbev_instance, mbv_secondary_colortable,
                                                  mbv_secondary_colortable_mode, mbv_secondary_colortable_min,
                                                  mbv_secondary_colortable_max, mbv_overlay_shade_magnitude,
                                                  mbv_overlay_shade_center, mbv_overlay_shade_mode, &mbev_error);
    mbev_status = mbview_setsecondaryname(mbev_verbose, mbev_instance, "Standard Deviation", &mbev_error);

    /* update widgets */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_update(mbev_verbose, mbev_instance, &mbev_error);

    /* add navigation to view */
    for (ifile = 0; ifile < mbev_num_files; ifile++) {
      file = &(mbev_files[ifile]);
      if (file->load_status && file->num_pings > 0) {
        /* set message */
        sprintf(value_text, "Loading nav %d of %d...", ifile + 1, mbev_num_files);
        do_mbeditviz_message_on(value_text);

        /* allocate arrays */
        if ((navtime_d = (double *)malloc(file->num_pings * sizeof(double))) == NULL)
          mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navz = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navheading = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navspeed = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navportlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navportlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navstbdlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navstbdlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navline = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navshot = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navcdp = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;

        /* copy nav data */
        if (mbev_status == MB_SUCCESS)
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = (struct mbev_ping_struct *)&(file->pings[iping]);
            navtime_d[iping] = ping->time_d;
            navlon[iping] = ping->navlon;
            navlat[iping] = ping->navlat;
            navz[iping] = -ping->sensordepth;
            navheading[iping] = ping->heading;
            navspeed[iping] = ping->speed;
            navportlon[iping] = ping->portlon;
            navportlat[iping] = ping->portlat;
            navstbdlon[iping] = ping->stbdlon;
            navstbdlat[iping] = ping->stbdlat;
            navline[iping] = 0;
            navshot[iping] = iping;
            navcdp[iping] = 0;
          }

        /* add nav data to mbview */
        if (mbev_status == MB_SUCCESS)
          mbev_status = mbview_addnav(mbev_verbose, mbev_instance, file->num_pings, navtime_d, navlon, navlat, navz,
                                      navheading, navspeed, navportlon, navportlat, navstbdlon, navstbdlat, navline,
                                      navshot, navcdp, MBV_COLOR_BLACK, 2, file->name, MB_PROCESSED_NONE, file->path,
                                      file->path, file->format, true, false, false, false, 1, &mbev_error);

        /* deallocate memory used for data arrays */
        free(navtime_d);
        navtime_d = NULL;
        free(navlon);
        navlon = NULL;
        free(navlat);
        navlat = NULL;
        free(navz);
        navz = NULL;
        free(navheading);
        navheading = NULL;
        free(navspeed);
        navspeed = NULL;
        free(navportlon);
        navportlon = NULL;
        free(navportlat);
        navportlat = NULL;
        free(navstbdlon);
        navstbdlon = NULL;
        free(navstbdlat);
        navstbdlat = NULL;
        free(navline);
        navline = NULL;
        free(navshot);
        navshot = NULL;
        free(navcdp);
        navcdp = NULL;
      }
    }
    mbview_enableviewnavs(mbev_verbose, mbev_instance, &mbev_error);
    mbev_status = mbview_update(mbev_verbose, mbev_instance, &mbev_error);
    do_mbeditviz_message_off();

    /* add pick notify functions */
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_ONEPOINT, do_mbeditviz_pickonepoint_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_TWOPOINT, do_mbeditviz_picktwopoint_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_AREA, do_mbeditviz_pickarea_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_REGION, do_mbeditviz_pickregion_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_SITE, do_mbeditviz_picksite_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_ROUTE, do_mbeditviz_pickroute_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_NAV, do_mbeditviz_picknav_notify, &mbev_error);

    /* add action button */
    mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_regrid_notify, "Update Bathymetry Grid", MBV_PICKMASK_NONE,
                     &mbev_error);

    /* if some soundings flagged as secondary have been loaded, add actions to enable/disable using them */
    if (mbev_num_soundings_secondary > 0) {

      /* add action button */
      mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_enablesecondarypicks_notify, "Enable Secondary Picks", MBV_STATEMASK_20,
                       &mbev_error);
      mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 1, &mbev_error);

      /* add action button */
      mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_disablesecondarypicks_notify, "Disable Secondary Picks", MBV_STATEMASK_21,
                       &mbev_error);
      mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 0, &mbev_error);
    }

    /* add colorchange notify function */
    mbview_setcolorchangenotify(mbev_verbose, mbev_instance, do_mbeditviz_colorchange_notify, &mbev_error);
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewgrid status:%d\n", mbev_status);
#endif
}
/*---------------------------------------------------------------------------------------*/

int do_mbeditviz_mbview_dismiss_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:   %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

/* destroy any mb3dsoundings window */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "1 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif
  mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_verbose);
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "2 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif
  mbeditviz_mb3dsoundings_dismiss();
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "3 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

/* destroy the grid */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "mbev_grid.status:%d\n", mbev_grid.status);
#endif
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "4 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

  return (mbev_status);
}
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

int do_mbeditviz_opendata(char *input_file_ptr, int format) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       input_file_ptr:    %s\n", input_file_ptr);
    fprintf(stderr, "dbg2       format:            %d\n", format);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_opendata:%s %d\n", input_file_ptr, format);
#endif
  do_mbeditviz_message_on("Reading datalismbeditviz_unload_filet...");

  mbeditviz_open_data(input_file_ptr, format);

  do_mbeditviz_message_off();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_opendata status:%d\n", mbev_status);
#endif

  return (mbev_status);
}
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
    sprintf(string, ":::t\"Available Files: %d\":t\"Loaded Files: %d\":t\"Grid Not Generated\"", mbev_num_files,
            mbev_num_files_loaded);
  else
    sprintf(string,
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

  char string[MB_PATH_MAXLINE+20];
  int *position_list = NULL;
  int position_list_save[MB_PATH_MAXLINE];
  int position_count = 0;
  char athchar;
  char atschar;
  char atachar;
  XmString *xstr;
  char *lockstrptr;
  char *lockedstr = "<Locked>";
  char *unlockedstr = "        ";
  char *loadedstr = "<loaded>";
  char *esfyesstr = "<esf>";
  char *esfnostr = "     ";

  /* swath file locking variables */
  int lock_error = MB_ERROR_NO_ERROR;
  bool locked = false;
  int lock_purpose;
  mb_path lock_program;
  mb_path lock_cpu;
  mb_path lock_user;
  char lock_date[25];

  /* esf file checking variables */
  bool esf_exists = false;
  struct stat file_status;
  char save_file[MB_PATH_MAXLINE+20];

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

    /* check for locks */
    // int lock_status =
    mb_pr_lockinfo(mbev_verbose, mbev_files[i].path, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                   lock_date, &lock_error);
    if (locked != file->locked) {
      file->locked = locked;
      update_filelist = true;
    }

    /* check for edit save file */
    sprintf(save_file, "%s.esf", mbev_files[i].path);
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
      char *esfstrptr;
      for (int i = 0; i < mbev_num_files; i++) {
        file = &(mbev_files[i]);

        /* set label strings */
        if (file->load_status)
          lockstrptr = loadedstr;
        else if (file->locked)
          lockstrptr = lockedstr;
        else
          lockstrptr = unlockedstr;
        if (file->esf_exists)
          esfstrptr = esfyesstr;
        else
          esfstrptr = esfnostr;
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
        sprintf(string, "%s %s %c%c%c %s %d", lockstrptr, esfstrptr, athchar, atschar, atachar, mbev_files[i].name,
                mbev_files[i].format);
        xstr[i] = XmStringCreateLocalized(string);
      }
      XmListAddItems(list_filelist, xstr, mbev_num_files, 0);
      for (int i = 0; i < mbev_num_files; i++) {
        XmStringFree(xstr[i]);
      }
      free(xstr);

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
void do_mbeditviz_pickonepoint_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickonepoint_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_pickonepoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_picktwopoint_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picktwopoint_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_picktwopoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_pickarea_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickarea_notify:%zu\n", instance);
#endif

  mbeditviz_selectarea(instance);
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_picktwopoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_pickregion_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickregion_notify:%zu\n", instance);
#endif

  mbeditviz_selectregion(instance);
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_pickregion_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_picksite_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picksite_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_picksite_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_pickroute_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickroute_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_pickroute_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/

void do_mbeditviz_picknav_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picknav_notify:%zu\n", instance);
#endif

  mbeditviz_selectnav(instance);

  /* open ping editor */
  /* BxManageCB(parent, (XtPointer)"form_pingedit", NULL); */

  /* open mb3dsoundings 3d editor */
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_picknav_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_regrid_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_regrid_notify\n");
#endif

  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;

  /* get current bias parameters */
  mb3dsoundings_get_bias_values(mbev_verbose, &rollbias, &pitchbias, &headingbias, &timelag, &snell, &mbev_error);

  /* regrid the bathymetry */
  mbeditviz_mb3dsoundings_biasapply(rollbias, pitchbias, headingbias, timelag, snell);

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_regrid_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_enablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_enablesecondarypicks_notify\n");
#endif

  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 0, &mbev_error);
  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 1, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_enablesecondarypicks_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_disablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_disablesecondarypicks_notify\n");
#endif

  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 1, &mbev_error);
  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 0, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_disablesecondarypicks_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_colorchange_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_colorchange_notify\n");
#endif

  /* recolor any selected soundings and then replot */
  if (mbev_selected.displayed && mbev_selected.num_soundings > 0) {
  	for (int isounding = 0; isounding < mbev_selected.num_soundings; isounding++) {
  		struct mb3dsoundings_sounding_struct *sounding = &mbev_selected.soundings[isounding];
  		if (mb_beam_ok(sounding->beamflag)) {
        mbview_colorvalue_instance(instance, sounding->z, &(sounding->r), &(sounding->g), &(sounding->b));
      }
    }
    mbev_status = mb3dsoundings_plot(mbev_verbose, &mbev_error);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_colorchange_notify status:%d\n", mbev_status);
#endif
}

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

  /* force the label to be visible */
  Widget diashell;
  for (diashell = label_mbeditviz_message; !XtIsShell(diashell); diashell = XtParent(diashell))
    ;
  Widget topshell;
  for (topshell = diashell; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
    ;
  if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
    Window diawindow = XtWindow(diashell);
    Window topwindow = XtWindow(topshell);

    XEvent event;
    XWindowAttributes xwa;

    /* wait for the dialog to be mapped */
    while (XGetWindowAttributes(XtDisplay(label_mbeditviz_message), diawindow, &xwa) && xwa.map_state != IsViewable) {
      if (XGetWindowAttributes(XtDisplay(label_mbeditviz_message), topwindow, &xwa) && xwa.map_state != IsViewable)
        break;

      XtAppNextEvent(app, &event);
      XtDispatchEvent(&event);
    }
  }

  XmUpdateDisplay(topshell);

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
/* Get text item string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void get_text_string(Widget w, String str) {
  char *str_tmp = (char *)XmTextGetString(w);
  strcpy(str, str_tmp);
  XtFree(str_tmp);
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
