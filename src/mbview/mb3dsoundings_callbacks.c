/*--------------------------------------------------------------------
 *    The MB-system:  mb3dsoundings_callbacks.c    5/25/2007
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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_status.h"
#include "mb_define.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#  ifndef WIN32
#    define WIN32
#  endif
#  include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include "Mb3dsdg.h"
#include "MB3DRouteList.h"
#include "MB3DSiteList.h"
#include "MB3DNavList.h"
#include "MB3DView.h"

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

/* Set flag to define mb3dsoundings global variables in this code block */
#define MB3DSOUNDINGSGLOBAL

/* mb3dsoundings include */
#include "mb3dsoundingsprivate.h"
#include "mbview.h"
#include "mbviewprivate.h"

//#define MBV_DEBUG_GLX 1
//#define MBV_GET_GLX_ERRORS 1

/*------------------------------------------------------------------------------*/

/* local variables */
static Cardinal ac;
static Arg args[256];
static mb_path value_text;

/*------------------------------------------------------------------------------*/
/* code below used for mb3dsoundings library                                           */
/*------------------------------------------------------------------------------*/
int mb3dsoundings_startup(int verbose, Widget parent, XtAppContext app, int *error) {
  /* set local verbosity */
  mbs_verbose = verbose;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       parent:                  %p\n", parent);
    fprintf(stderr, "dbg2       app:                     %p\n", app);
  }

  /* set parent widget and app context */
  mbs_parent_widget = parent;
  mbs_app_context = app;
  mbs_work_function_set = false;
  mbs_timer_count = 0;

  /* initialize window */
  mb3dsoundings_reset();

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_updatecursor() {
  /* deal with pick according to edit_mode */
  if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.TargetRedCursor);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.TargetRedCursor);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.ExchangeRedCursor);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.ExchangeGreenCursor);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.TargetRedCursor);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
    XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                  mb3dsoundings.TargetBlueCursor);
  }

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_updategui() {
  // struct mb3dsoundings_struct *soundingdata =
  //  (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;

  int ibiasmin = 100 * (mb3dsoundings.irollbias / 100) - 100;
  int ibiasmax = 100 * (mb3dsoundings.irollbias / 100) + 100;
  ac = 0;
  XtSetArg(args[ac], XmNminimum, ibiasmin);
  ac++;
  XtSetArg(args[ac], XmNmaximum, ibiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, mb3dsoundings.irollbias);
  ac++;
  XtSetValues(mb3dsoundings.mb3dsdg.scale_rollbias, args, ac);

  ibiasmin = 100 * (mb3dsoundings.ipitchbias / 100) - 100;
  ibiasmax = 100 * (mb3dsoundings.ipitchbias / 100) + 100;
  ac = 0;
  XtSetArg(args[ac], XmNminimum, ibiasmin);
  ac++;
  XtSetArg(args[ac], XmNmaximum, ibiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, mb3dsoundings.ipitchbias);
  ac++;
  XtSetValues(mb3dsoundings.mb3dsdg.scale_pitchbias, args, ac);

  ibiasmin = 100 * (mb3dsoundings.iheadingbias / 100) - 100;
  ibiasmax = 100 * (mb3dsoundings.iheadingbias / 100) + 100;
  ac = 0;
  XtSetArg(args[ac], XmNminimum, ibiasmin);
  ac++;
  XtSetArg(args[ac], XmNmaximum, ibiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, mb3dsoundings.iheadingbias);
  ac++;
  XtSetValues(mb3dsoundings.mb3dsdg.scale_headingbias, args, ac);

  ibiasmin = 100 * (mb3dsoundings.itimelag / 100) - 100;
  ibiasmax = 100 * (mb3dsoundings.itimelag / 100) + 100;
  ac = 0;
  XtSetArg(args[ac], XmNminimum, ibiasmin);
  ac++;
  XtSetArg(args[ac], XmNmaximum, ibiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, mb3dsoundings.itimelag);
  ac++;
  XtSetValues(mb3dsoundings.mb3dsdg.scale_timelag, args, ac);

  ibiasmin = 100 * (mb3dsoundings.isnell / 100) - 100;
  ibiasmax = 100 * (mb3dsoundings.isnell / 100) + 100;
  ac = 0;
  XtSetArg(args[ac], XmNminimum, ibiasmin);
  ac++;
  XtSetArg(args[ac], XmNmaximum, ibiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, mb3dsoundings.isnell);
  ac++;
  XtSetValues(mb3dsoundings.mb3dsdg.scale_snell, args, ac);

  if (mb3dsoundings.view_boundingbox) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_boundingbox, True, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_boundingbox, False, False);
  }

  if (mb3dsoundings.view_flagged) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_flagged, True, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_flagged, False, False);
  }

  if (mb3dsoundings.view_secondary) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_secondary, True, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_secondary, False, False);
  }

  if (mb3dsoundings.view_scalewithflagged) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_scalewithflagged, True, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_scalewithflagged, False, False);
  }

  if (mb3dsoundings.view_color == MBS_VIEW_COLOR_FLAG) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_TOPO) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }
  else /* if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP) */ {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }

  if (mb3dsoundings.view_profiles == MBS_VIEW_PROFILES_NONE) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, False, False);
  }
  else if (mb3dsoundings.view_profiles == MBS_VIEW_PROFILES_UNFLAGGED) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, False, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, True, False);
  }

  if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate1, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom1, False, False);
  }
  else {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate1, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom1, True, False);
  }

  /* set the mode toggles */
  mb3dsoundings_updatemodetoggles();

  /* set status label */
  mb3dsoundings_updatestatus();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_updatemodetoggles() {

  /* set the mode toggles */
  if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, TRUE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, FALSE, FALSE);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, TRUE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, FALSE, FALSE);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, TRUE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, FALSE, FALSE);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, TRUE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, FALSE, FALSE);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, TRUE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, FALSE, FALSE);
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, FALSE, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, TRUE, FALSE);
  }
  if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Pan\"\"R: Zoom\"");
  }
  set_mbview_label_multiline_string(mb3dsoundings.mb3dsdg.label_mousemode, value_text);

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_updatestatus() {
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;

  /* if in info mode and sounding picked print info as status */
  if (mb3dsoundings.edit_mode == MBS_EDIT_INFO && mb3dsoundings.last_sounding_defined) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    (mb3dsoundings.mb3dsoundings_info_notify)(sounding->ifile, sounding->iping, sounding->ibeam, value_text);
    fprintf(stderr, "\n%s\n", value_text);
    //fprintf(stderr, "xyz bounds:%f %f %f %f %f %f  bearing:%f scale:%f zscale:%f zorigin:%f\n",
    //        soundingdata->xmin, soundingdata->xmax, soundingdata->ymin,
    //        soundingdata->ymax, soundingdata->zmin, soundingdata->zmax, soundingdata->bearing, soundingdata->scale,
    //        soundingdata->zscale, soundingdata->zorigin);
    //fprintf(stderr, "SOUNDING: xyz: %f %f %f   glxyz: %f %f %f  winxy: %d %d\n", sounding->x, sounding->y, sounding->z,
    //        sounding->glx, sounding->gly, sounding->glz, sounding->winx, sounding->winy);
    XtUnmanageChild(mb3dsoundings.mb3dsdg.scale_rollbias);
    XtUnmanageChild(mb3dsoundings.mb3dsdg.scale_pitchbias);
    XtUnmanageChild(mb3dsoundings.mb3dsdg.scale_headingbias);
    XtUnmanageChild(mb3dsoundings.mb3dsdg.scale_timelag);
    XtUnmanageChild(mb3dsoundings.mb3dsdg.scale_snell);
  }

  /* else set standard status label */
  else {
    sprintf(value_text, "Azi:%.2f | Elev: %.2f | Exager:%.2f | Tot:%d Good:%d Flagged:%d", mb3dsoundings.azimuth,
            mb3dsoundings.elevation, mb3dsoundings.exageration, soundingdata->num_soundings,
            soundingdata->num_soundings_unflagged, soundingdata->num_soundings_flagged);
    XtManageChild(mb3dsoundings.mb3dsdg.scale_rollbias);
    XtManageChild(mb3dsoundings.mb3dsdg.scale_pitchbias);
    XtManageChild(mb3dsoundings.mb3dsdg.scale_headingbias);
    XtManageChild(mb3dsoundings.mb3dsdg.scale_timelag);
    XtManageChild(mb3dsoundings.mb3dsdg.scale_snell);
  }

  /* put up the new status string */
  set_mbview_label_string(mb3dsoundings.mb3dsdg.label_status, value_text);

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_updatelabelmousemode() {
  /* set mouse mode label */
  if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
    if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Pan\"\"R: Zoom\"");
    }
  }
  else /* if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) */ {
    if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
      sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Rotate Soundings\"\"R: Exageration\"");
    }
  }
  set_mbview_label_multiline_string(mb3dsoundings.mb3dsdg.label_mousemode, value_text);

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_end(int verbose, int *error) {
  /* set local verbosity */
  mbs_verbose = verbose;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* handle destruction if not already handled */
  if (mb3dsoundings.init != MBS_WINDOW_NULL) {
    /* delete old glx_context if it exists */
    if (mb3dsoundings.glx_init) {
      glXDestroyContext(mb3dsoundings.dpy, mb3dsoundings.glx_context);
      XtDestroyWidget(mb3dsoundings.glwmda);
      mb3dsoundings.glx_init = false;
    }

    /* destroy the topLevelShell and all its children */
    XtDestroyWidget(mb3dsoundings.topLevelShell);

    /* reset init flag */
    mb3dsoundings.init = MBS_WINDOW_NULL;
  }

  /* reinitialize parameters */
  mb3dsoundings_reset();

  /* set flags */
  mbs_status = MB_SUCCESS;
  mbs_error = MB_ERROR_NO_ERROR;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/

int mb3dsoundings_set_dismiss_notify(int verbose, void(dismiss_notify)(), int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_dismiss_notify = dismiss_notify;

  *error = mbs_error;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_edit_notify(int verbose, void(edit_notify)(int, int, int, char, int), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_edit_notify = edit_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_info_notify(int verbose, void(info_notify)(int, int, int, char *), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_info_notify = info_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_bias_notify(int verbose, void(bias_notify)(double, double, double, double, double), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_bias_notify = bias_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_biasapply_notify(int verbose, void(biasapply_notify)(double, double, double, double, double), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_biasapply_notify = biasapply_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_flagsparsevoxels_notify(int verbose, void(flagsparsevoxels_notify)(int, int), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify = flagsparsevoxels_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_colorsoundings_notify(int verbose, void(colorsoundings_notify)(int), int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_colorsoundings_notify = colorsoundings_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_set_optimizebiasvalues_notify(int verbose,
                                                void(optimizebiasvalues_notify)(int, double *, double *, double *, double *, double *),
                                                int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set the function pointer */
  mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify = optimizebiasvalues_notify;

  *error = mbs_error;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_reset() {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  /* initialize mb3dsoungings data and parameters */
  mb3dsoundings.init = MBS_WINDOW_NULL;

  mb3dsoundings.mb3dsoundings_dismiss_notify = NULL;
  mb3dsoundings.mb3dsoundings_edit_notify = NULL;
  mb3dsoundings.mb3dsoundings_info_notify = NULL;
  mb3dsoundings.mb3dsoundings_bias_notify = NULL;
  mb3dsoundings.mb3dsoundings_biasapply_notify = NULL;
  mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify = NULL;
  mb3dsoundings.mb3dsoundings_colorsoundings_notify = NULL;

  mb3dsoundings.topLevelShell = NULL;
  mb3dsoundings.mainWindow = NULL;
  mb3dsoundings.glwmda = NULL;
  mb3dsoundings.dpy = NULL;
  mb3dsoundings.vi = NULL;
  mb3dsoundings.glx_init = false;
  mb3dsoundings.glx_context = NULL;
  mb3dsoundings.message_on = false;
  mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
  mb3dsoundings.mouse_mode = MBS_MOUSE_ROTATE;
  mb3dsoundings.keyreverse_mode = false;
  mb3dsoundings.mousereverse_mode = false;

  /* drawing variables */
  mb3dsoundings.elevation = 0.0;
  mb3dsoundings.azimuth = 0.0;
  mb3dsoundings.exageration = 1.0;
  mb3dsoundings.gl_width = 0;
  mb3dsoundings.gl_height = 0;
  ;
  mb3dsoundings.right = -1.0;
  mb3dsoundings.left = 1.0;
  mb3dsoundings.top = 1.0;
  mb3dsoundings.bottom = -1.0;
  mb3dsoundings.aspect_ratio = 1.0;
  mb3dsoundings.gl_offset_x = 0.0;
  mb3dsoundings.gl_offset_y = 0.0;
  mb3dsoundings.gl_offset_x_save = 0.0;
  mb3dsoundings.gl_offset_y_save = 0.0;
  mb3dsoundings.gl_size = 1.0;
  mb3dsoundings.gl_size_save = 1.0;

  /* button parameters */
  mb3dsoundings.button1down = false;
  mb3dsoundings.button2down = false;
  mb3dsoundings.button3down = false;
  mb3dsoundings.button_down_x = 0;
  mb3dsoundings.button_down_y = 0;
  mb3dsoundings.button_move_x = 0;
  mb3dsoundings.button_move_y = 0;
  mb3dsoundings.button_up_x = 0;
  mb3dsoundings.button_up_y = 0;

  /* edit grab parameters */
  mb3dsoundings.grab_start_defined = false;
  mb3dsoundings.grab_end_defined = false;
  mb3dsoundings.grab_start_x = 0;
  mb3dsoundings.grab_start_y = 0;
  mb3dsoundings.grab_end_x = 0;
  mb3dsoundings.grab_end_y = 0;

  /* patch test parameters */
  mb3dsoundings.irollbias = 0;
  mb3dsoundings.ipitchbias = 0;
  mb3dsoundings.iheadingbias = 0;
  mb3dsoundings.itimelag = 0;
  mb3dsoundings.isnell = 10000;

  /* view parameters */
  mb3dsoundings.view_boundingbox = true;
  mb3dsoundings.view_flagged = true;
  mb3dsoundings.view_secondary = false;
  mb3dsoundings.view_profiles = MBS_VIEW_PROFILES_NONE;
  mb3dsoundings.view_scalewithflagged = true;
  mb3dsoundings.view_color = MBS_VIEW_COLOR_FLAG;

  /* last sounding edited */
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.last_sounding_edited = 0;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*---------------------------------------------------------------------------------------*/
int mb3dsoundings_open(int verbose, struct mb3dsoundings_struct *soundingdata, int *error) {
  /* fprintf(stderr,"Called mb3dsoundings_open\n"); */

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       soundingdata:  %p\n", soundingdata);
  }

  /* print out some statistics of the selected soundings */
  struct mb3dsoundings_sounding_struct *soundings = soundingdata->soundings;
  int num_soundings = 0;
  int num_soundings_null = 0;
  int num_soundings_unflagged = 0;
  int num_soundings_flagged = 0;
  int num_soundings_flagged_manual = 0;
  int num_soundings_flagged_sonar = 0;
  int num_soundings_flagged_filter = 0;
  int num_soundings_flagged_filter2 = 0;
  int num_soundings_flagged_secondary = 0;
  int num_soundings_flagged_interpolated = 0;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    num_soundings++;
    num_soundings_null += mb_beam_check_flag_null(soundings[i].beamflag);
    num_soundings_unflagged += mb_beam_ok(soundings[i].beamflag);
    num_soundings_flagged += mb_beam_check_flag_flagged(soundings[i].beamflag);
    num_soundings_flagged_manual += mb_beam_check_flag_manual(soundings[i].beamflag);
    num_soundings_flagged_sonar += mb_beam_check_flag_sonar(soundings[i].beamflag);
    num_soundings_flagged_filter += mb_beam_check_flag_filter(soundings[i].beamflag);
    num_soundings_flagged_filter2 += mb_beam_check_flag_filter2(soundings[i].beamflag);
    num_soundings_flagged_secondary += mb_beam_check_flag_multipick(soundings[i].beamflag);
    num_soundings_flagged_interpolated += mb_beam_check_flag_interpolate(soundings[i].beamflag);
  }
  //if (verbose) {
    fprintf(stdout, "\nMBeditviz 3D Sounding View:\n");
    fprintf(stdout, "  Soundings:                        %d\n", num_soundings);
    fprintf(stdout, "  Null Soundings:                   %d\n", num_soundings_null);
    fprintf(stdout, "  Unflagged Soundings:              %d\n", num_soundings_unflagged);
    fprintf(stdout, "  Flagged Soundings:                %d\n", num_soundings_flagged);
    fprintf(stdout, "  Manual Flagged Soundings:         %d\n", num_soundings_flagged_manual);
    fprintf(stdout, "  Sonar Flagged Soundings:          %d\n", num_soundings_flagged_sonar);
    fprintf(stdout, "  Filter Flagged Soundings:         %d\n", num_soundings_flagged_filter);
    fprintf(stdout, "  Filter2 Flagged Soundings:        %d\n", num_soundings_flagged_filter2);
    fprintf(stdout, "  Secondary Flagged Soundings:      %d\n", num_soundings_flagged_secondary);
    fprintf(stdout, "  Interpolated Flagged Soundings:   %d\n", num_soundings_flagged_interpolated);
  //}

  /* set the data pointer */
  mb3dsoundings.soundingdata = (struct mb3dsoundings_struct *)soundingdata;
  mb3dsoundings_scale(verbose, error);

  /* reset info flag */
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.last_sounding_edited = 0;

  /* if not yet created then create the MB3DView class in
      a topLevelShell as a child of Widget parent */
  if (mb3dsoundings.init == MBS_WINDOW_NULL) {
    ac = 0;
    XtSetArg(args[ac], XmNtitle, "3D Soundings");
    ac++;
    XtSetArg(args[ac], XmNwidth, 1040);
    ac++;
    XtSetArg(args[ac], XmNheight, 600);
    ac++;
    mb3dsoundings.topLevelShell = XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, mbs_parent_widget, args, ac);
    mb3dsoundings.mainWindow = XmCreateMainWindow(mb3dsoundings.topLevelShell, "mainWindow_mb3dsoundings", args, ac);
    XtManageChild(mb3dsoundings.mainWindow);
    Mb3dsdgCreate(&(mb3dsoundings.mb3dsdg), mb3dsoundings.mainWindow, "mb3dsdg", args, ac);

    ac = 0;
    XtSetArg(args[ac], XmNx, mb3dsoundings.gl_xo);
    ac++;
    XtSetArg(args[ac], XmNy, mb3dsoundings.gl_yo);
    ac++;
    XtSetArg(args[ac], XmNwidth, mb3dsoundings.gl_width + MBS_LEFT_WIDTH);
    ac++;
    XtSetArg(args[ac], XmNheight, mb3dsoundings.gl_height + MBS_LEFT_HEIGHT);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.Mb3dsdg, args, ac);

    XtManageChild(mb3dsoundings.mb3dsdg.Mb3dsdg);

    /* get resize events - add event handlers */
    XtAddEventHandler(mb3dsoundings.mb3dsdg.drawingArea, StructureNotifyMask, False, (XtEventHandler)do_mb3dsdg_resize,
                      (XtPointer)0);

    /* set the mode toggles */
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, 0, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, 0, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, 0, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, 0, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, 0, FALSE);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, 0, FALSE);
    if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_toggle, 1, FALSE);
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_pick, 1, FALSE);
    else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_erase, 1, FALSE);
    else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_restore, 1, FALSE);
    else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_grab, 1, FALSE);
    else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO)
      XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_info, 1, FALSE);

    /* get display and xid */
    mb3dsoundings.dpy = (Display *)XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg);
    mb3dsoundings.xid = XtWindow(mb3dsoundings.mb3dsdg.drawingArea);

    XColor XColorBlack;
    XColor XColorWhite;
    XColor XColorRed;
    XColor XColorGreen;
    XColor XColorBlue;
    XColor XColorCoral;
    XColor exact;

    /* generate cursors for later use */
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "red",
                     &XColorRed, &exact);
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "green",
                     &XColorGreen, &exact);
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "blue",
                     &XColorBlue, &exact);
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "black",
                     &XColorBlack, &exact);
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "white",
                     &XColorWhite, &exact);
    XAllocNamedColor(mb3dsoundings.dpy, DefaultColormap(mb3dsoundings.dpy, XDefaultScreen(mb3dsoundings.dpy)), "coral",
                     &XColorCoral, &exact);
    mb3dsoundings.TargetBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_target);
    mb3dsoundings.TargetGreenCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_target);
    mb3dsoundings.TargetRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_target);
    mb3dsoundings.TargetBlueCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_target);
    mb3dsoundings.ExchangeBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_exchange);
    mb3dsoundings.ExchangeGreenCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_exchange);
    mb3dsoundings.ExchangeRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_exchange);
    mb3dsoundings.FleurBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_fleur);
    mb3dsoundings.FleurRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_fleur);
    mb3dsoundings.SizingBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_sizing);
    mb3dsoundings.SizingRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_sizing);
    mb3dsoundings.BoatBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_boat);
    mb3dsoundings.BoatRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_boat);
    mb3dsoundings.WatchBlackCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_watch);
    mb3dsoundings.WatchRedCursor = XCreateFontCursor(mb3dsoundings.dpy, XC_watch);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.TargetRedCursor, &XColorRed, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.TargetGreenCursor, &XColorGreen, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.TargetBlueCursor, &XColorBlue, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.ExchangeRedCursor, &XColorRed, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.ExchangeGreenCursor, &XColorGreen, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.FleurRedCursor, &XColorRed, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.SizingRedCursor, &XColorRed, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.BoatRedCursor, &XColorRed, &XColorCoral);
    XRecolorCursor(mb3dsoundings.dpy, mb3dsoundings.WatchRedCursor, &XColorRed, &XColorCoral);

    mb3dsoundings.init = MBS_WINDOW_HIDDEN;
  }

  if (mb3dsoundings.init == MBS_WINDOW_HIDDEN) {
    XtPopup(XtParent(mb3dsoundings.mainWindow), XtGrabNone);
    mb3dsoundings.init = MBS_WINDOW_VISIBLE;
  }

  /* update gui widgets */
  mb3dsoundings_updategui();

  /* reset OpenGL */
  mb3dsoundings_reset_glx();

  /* recalculate vertical scaling */
  mb3dsoundings_setzscale(verbose, error);

  /* replot the data */
  mb3dsoundings_plot(verbose, error);

  *error = mbs_error;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
int mb3dsoundings_reset_glx() {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }
  /* fprintf(stderr,"mb3dsoundings_reset_glx 1\n"); */

  /* delete old glx_context if it exists */
  if (mb3dsoundings.glx_init) {
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXDestroyContext(%p,%p)\n", __FILE__, __LINE__, __func__, mb3dsoundings.dpy, mb3dsoundings.glx_context);
#endif
    glXDestroyContext(mb3dsoundings.dpy, mb3dsoundings.glx_context);
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s XtDestroyWidget(%p)\n", __FILE__, __LINE__, __func__, mb3dsoundings.glwmda);
#endif
    XtDestroyWidget(mb3dsoundings.glwmda);
#ifdef MBV_GET_GLX_ERRORS
  mbview_glerrorcheck(0, __FILE__, __LINE__, __func__);
#endif
    mb3dsoundings.glx_init = false;
  }

  /* get dimensions of the drawingArea */
  XtVaGetValues(mb3dsoundings.mb3dsdg.drawingArea, XmNwidth, &mb3dsoundings.gl_width, XmNheight, &mb3dsoundings.gl_height,
                NULL);
  mb3dsoundings.gl_width -= 20;
  mb3dsoundings.gl_height -= 20;

  /* intitialize OpenGL graphics */
  ac = 0;
  XtSetArg(args[ac], mbGLwNrgba, TRUE);
  ac++;
  XtSetArg(args[ac], mbGLwNdepthSize, 1);
  ac++;
  XtSetArg(args[ac], mbGLwNdoublebuffer, True);
  ac++;
  XtSetArg(args[ac], mbGLwNallocateBackground, TRUE);
  ac++;
  XtSetArg(args[ac], XmNwidth, mb3dsoundings.gl_width);
  ac++;
  XtSetArg(args[ac], XmNheight, mb3dsoundings.gl_height);
  ac++;
  mb3dsoundings.glwmda = mbGLwCreateMDrawingArea(mb3dsoundings.mb3dsdg.drawingArea, "glwidget", args, ac);

  XtManageChild(mb3dsoundings.glwmda);
  XtAddCallback(mb3dsoundings.glwmda, "exposeCallback", &(do_mb3dsdg_glwda_expose), (XtPointer)NULL);
  XtAddCallback(mb3dsoundings.glwmda, "resizeCallback", &(do_mb3dsdg_glwda_resize), (XtPointer)NULL);
  XtAddCallback(mb3dsoundings.glwmda, "inputCallback", &(do_mb3dsdg_glwda_input), (XtPointer)NULL);

  /* set up a new opengl context */
  ac = 0;
  XtSetArg(args[ac], mbGLwNvisualInfo, &(mb3dsoundings.vi));
  ac++;
  XtGetValues(mb3dsoundings.glwmda, args, ac);
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXCreateContext(%p,%p)\n", __FILE__, __LINE__, __func__, mb3dsoundings.dpy, mb3dsoundings.vi);
#endif
  mb3dsoundings.glx_context = glXCreateContext(mb3dsoundings.dpy, mb3dsoundings.vi, NULL, GL_TRUE);
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXMakeCurrent(%p,%p,%p)\n", __FILE__, __LINE__, __func__, XtDisplay(mb3dsoundings.glwmda),
          XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);
#endif
  glXMakeCurrent(XtDisplay(mb3dsoundings.glwmda), XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);
#ifdef MBV_GET_GLX_ERRORS
  mbview_glerrorcheck(0, __FILE__, __LINE__, __func__);
#endif
  glViewport(0, 0, mb3dsoundings.gl_width, mb3dsoundings.gl_height);
  mb3dsoundings.aspect_ratio = ((float)mb3dsoundings.gl_width) / ((float)mb3dsoundings.gl_height);
  mb3dsoundings.glx_init = true;

  mb3dsoundings.dpy = (Display *)XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg);
  mb3dsoundings.xid = XtWindow(mb3dsoundings.mb3dsdg.drawingArea);
  XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                mb3dsoundings.TargetBlackCursor);
#ifdef MBV_GET_GLX_ERRORS
  mbview_glerrorcheck(0, __FILE__, __LINE__, __func__);
#endif

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/

void do_mb3dsdg_resize(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_resize\n"); */

  /* reset OpenGL */
  mb3dsoundings_reset_glx();

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_dismiss\n"); */

  XtPopdown(XtParent(mb3dsoundings.mainWindow));
  mb3dsoundings.init = MBS_WINDOW_HIDDEN;
  (mb3dsoundings.mb3dsoundings_dismiss_notify)();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_toggle(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_toggle\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_pick(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_pick\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_PICK;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_erase(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_erase\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_ERASE;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_restore(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_restore\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_RESTORE;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_grab(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_grab\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_GRAB;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_info(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_mouse_info\n"); */

  mb3dsoundings.edit_mode = MBS_EDIT_INFO;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();

  /* set edit cursor */
  mb3dsoundings_updatecursor();

  /* set status label */
  mb3dsoundings_updatestatus();
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* get event */
  // XEvent *event = acs->event;

  /* fprintf(stderr,"--------\nCalled do_mb3dsdg_input: reason:%d type:%d\n-------\n", acs->reason, event->xany.type); */
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_glwda_expose(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_glwda_expose\n"); */
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_glwda_input(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  mbGLwDrawingAreaCallbackStruct *acs = (mbGLwDrawingAreaCallbackStruct *)call_data;

  /* get event */
  XEvent *event = acs->event;

  /* fprintf(stderr,"Called do_mb3dsdg_glwda_input: reason:%d type:%d\n", acs->reason, event->xany.type); */
  /* If there is input in the drawing area */
  if (acs->reason == XmCR_INPUT) {

    /* Check for mouse pressed. */
    if (event->xany.type == ButtonPress) {
      /* fprintf(stderr, "event->xany.type == ButtonPress  button:%d position: %d %d  mouse mode:%d\n",
      event->xbutton.button,event->xbutton.x,event->xbutton.y, mb3dsoundings.mouse_mode); */
      /* save location */
      mb3dsoundings.button_down_x = event->xbutton.x;
      mb3dsoundings.button_down_y = mb3dsoundings.gl_height - 1 - event->xbutton.y;

      /* If left mouse button is pushed */
      if (event->xbutton.button == 1) {
        /* set button1down flag */
        mb3dsoundings.button1down = true;

        /* deal with pick according to edit_mode */
        if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
          mb3dsoundings_pick(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
          mb3dsoundings_pick(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
          mb3dsoundings_eraserestore(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
          mb3dsoundings_eraserestore(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
          mb3dsoundings_grab(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y, MBS_EDIT_GRAB_START);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
          mb3dsoundings_info(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y);
        }

      } /* end of left button events */

      /* If middle mouse button is pushed */
      else if (event->xbutton.button == 2) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set button2down flag */
          mb3dsoundings.button2down = true;

          /* set cursor for rotate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurBlackCursor);

          mb3dsoundings.azimuth_save = mb3dsoundings.azimuth;
          mb3dsoundings.elevation_save = mb3dsoundings.elevation;
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set button2down flag */
          mb3dsoundings.button2down = true;

          /* set cursor for rotate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurBlackCursor);

          mb3dsoundings.gl_offset_x_save = mb3dsoundings.gl_offset_x;
          mb3dsoundings.gl_offset_y_save = mb3dsoundings.gl_offset_y;
        }
      } /* end of middle button events */

      /* If right mouse button is pushed */
      else if (event->xbutton.button == 3) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set button3down flag */
          mb3dsoundings.button3down = true;

          /* set cursor for exaggerate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurBlackCursor);

          mb3dsoundings.exageration_save = mb3dsoundings.exageration;
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set button3down flag */
          mb3dsoundings.button3down = true;

          /* set cursor for exaggerate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurBlackCursor);

          mb3dsoundings.gl_size_save = mb3dsoundings.gl_size;
        }

      } /* end of right button events */

    } /* end of button press events */

    /* Check for mouse motion while pressed. */
    if (event->xany.type == MotionNotify) {
      /* fprintf(stderr, "event->xany.type == MotionNotify  %d %d  mouse mode:%d\n",
      event->xbutton.x,event->xmotion.y, mb3dsoundings.mouse_mode); */

      /* save location */
      mb3dsoundings.button_move_x = event->xmotion.x;
      mb3dsoundings.button_move_y = mb3dsoundings.gl_height - 1 - event->xmotion.y;

      /* If left mouse button is dragged */
      if (mb3dsoundings.button1down) {

        /* deal with pick according to edit_mode */
        if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
          mb3dsoundings_eraserestore(mb3dsoundings.button_move_x, mb3dsoundings.button_move_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
          mb3dsoundings_eraserestore(mb3dsoundings.button_move_x, mb3dsoundings.button_move_y);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
          mb3dsoundings_grab(mb3dsoundings.button_move_x, mb3dsoundings.button_move_y, MBS_EDIT_GRAB_MOVE);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
        }

      } /* end of left button events */

      /* If middle mouse button is dragged */
      else if (mb3dsoundings.button2down) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set cursor for rotate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurRedCursor);

          /* rotate viewpoint of 3D map */
          mb3dsoundings.azimuth = mb3dsoundings.azimuth_save +
                                  180.0 * ((double)(mb3dsoundings.button_move_x - mb3dsoundings.button_down_x)) /
                                      ((double)mb3dsoundings.gl_width);
          mb3dsoundings.elevation = mb3dsoundings.elevation_save +
                                    180.0 * ((double)(mb3dsoundings.button_down_y - mb3dsoundings.button_move_y)) /
                                        ((double)mb3dsoundings.gl_height);

          /* keep elevation and azimuth values in appropriate bounds */
          if (mb3dsoundings.elevation > 180.0)
            mb3dsoundings.elevation -= 360.0;
          if (mb3dsoundings.elevation < -180.0)
            mb3dsoundings.elevation += 360.0;
          if (mb3dsoundings.azimuth < 0.0)
            mb3dsoundings.azimuth += 360.0;
          if (mb3dsoundings.azimuth > 360.0)
            mb3dsoundings.azimuth -= 360.0;

          mb3dsoundings_updatestatus();
          mb3dsoundings_plot(mbs_verbose, &mbs_error);
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set cursor for pan zoom */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurRedCursor);

          /* pan */
          mb3dsoundings.gl_offset_x = mb3dsoundings.gl_offset_x_save +
                                      ((double)(mb3dsoundings.button_move_x - mb3dsoundings.button_down_x)) *
                                          (mb3dsoundings.right - mb3dsoundings.left) / ((double)mb3dsoundings.gl_width);
          mb3dsoundings.gl_offset_y = mb3dsoundings.gl_offset_y_save +
                                      ((double)(mb3dsoundings.button_move_y - mb3dsoundings.button_down_y)) *
                                          (mb3dsoundings.top - mb3dsoundings.bottom) / ((double)mb3dsoundings.gl_width);

          mb3dsoundings_updatestatus();
          mb3dsoundings_plot(mbs_verbose, &mbs_error);
        }
      } /* end of middle button events */

      /* If right mouse button is dragged */
      else if (mb3dsoundings.button3down) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set cursor for exaggerate */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurRedCursor);

          /* change vertical exageration of 3D map */
          mb3dsoundings.exageration = mb3dsoundings.exageration_save *
                                      exp(((double)(mb3dsoundings.button_move_y - mb3dsoundings.button_down_y)) /
                                          ((double)mb3dsoundings.gl_height));

          mb3dsoundings_scalez(mbs_verbose, &mbs_error);
          mb3dsoundings_updatestatus();
          mb3dsoundings_plot(mbs_verbose, &mbs_error);
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set cursor for zoom */
          XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea),
                        mb3dsoundings.FleurRedCursor);

          /* change zoom */
          mb3dsoundings.gl_size =
              mb3dsoundings.gl_size_save * exp(((double)(mb3dsoundings.button_move_y - mb3dsoundings.button_down_y)) /
                                               ((double)mb3dsoundings.gl_height));

          mb3dsoundings_updatestatus();
          mb3dsoundings_plot(mbs_verbose, &mbs_error);
        }
      } /* end of right button events */

    } /* end of motion notify events */

    /* Check for mouse released. */
    if (event->xany.type == ButtonRelease) {
      /* fprintf(stderr, "event->xany.type == ButtonRelease  %d %d  mouse mode:%d\n",
      event->xbutton.x,event->xbutton.y, mb3dsoundings.mouse_mode); */

      /* save location */
      mb3dsoundings.button_up_x = event->xbutton.x;
      mb3dsoundings.button_up_y = mb3dsoundings.gl_height - 1 - event->xbutton.y;

      /* If left mouse button is released */
      if (mb3dsoundings.button1down) {

        /* deal with pick according to edit_mode */
        if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
          mb3dsoundings_grab(mb3dsoundings.button_down_x, mb3dsoundings.button_down_y, MBS_EDIT_GRAB_END);
        }
        else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
        }
      }

      /* If middle mouse button is released */
      else if (mb3dsoundings.button2down) {
      } /* end of middle button events */

      /* If right mouse button is released */
      else if (mb3dsoundings.button3down) {
      } /* end of right button events */

      /* unset all buttondown flags */
      mb3dsoundings.button1down = false;
      mb3dsoundings.button2down = false;
      mb3dsoundings.button3down = false;

      /* set edit cursor */
      mb3dsoundings_updatecursor();

    } /* end of button release events */

    /* deal with expose events by replotting */
    if (event->xany.type == Expose || event->xany.type == GraphicsExpose) {
      mb3dsoundings_updatestatus();
      mb3dsoundings_plot(mbs_verbose, &mbs_error);
    }

    /* Deal with KeyPress events */
    if (event->xany.type == KeyPress) {
      fprintf(stderr,"KeyPress event\n");
      /* Get key pressed - buffer[0] */
      // int actual =
      char buffer[1];
      KeySym keysym;
      XLookupString((XKeyEvent *)event, buffer, 1, &keysym, NULL);

      /* process events */
      switch (buffer[0]) {
      case 'G':
      case 'g':
        key_g_down = 1;
        break;
      case 'M':
      case 'm':
      case 'Z':
      case 'z':
        mb3dsoundings_bad_ping();
        key_z_down = 1;
        key_s_down = 0;
        key_a_down = 0;
        key_d_down = 0;
        break;
      case 'K':
      case 'k':
      case 'S':
      case 's':
        mb3dsoundings_good_ping();
        key_z_down = 0;
        key_s_down = 1;
        key_a_down = 0;
        key_d_down = 0;
        break;
      case 'J':
      case 'j':
      case 'A':
      case 'a':
        if (!mb3dsoundings.keyreverse_mode)
          mb3dsoundings_left_ping();
        else
          mb3dsoundings_right_ping();
        key_z_down = 0;
        key_s_down = 0;
        key_a_down = 1;
        key_d_down = 0;
        break;
      case 'L':
      case 'l':
      case 'D':
      case 'd':
        if (!mb3dsoundings.keyreverse_mode)
          mb3dsoundings_right_ping();
        else
          mb3dsoundings_left_ping();
        key_z_down = 0;
        key_s_down = 0;
        key_a_down = 0;
        key_d_down = 1;
        break;
      case '<':
      case ',':
      case 'X':
      case 'x':
        mb3dsoundings_flag_view();
        break;
      case '>':
      case '.':
      case 'C':
      case 'c':
        mb3dsoundings_unflag_view();
        break;
      case '!':
        mb3dsoundings_zero_ping();
        break;
      case 'U':
      case 'u':
      case 'Q':
      case 'q': {
        mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      case 'I':
      case 'i':
      case 'W':
      case 'w': {
        mb3dsoundings.edit_mode = MBS_EDIT_PICK;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      case 'O':
      case 'o':
      case 'E':
      case 'e': {
        mb3dsoundings.edit_mode = MBS_EDIT_ERASE;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      case 'P':
      case 'p':
      case 'R':
      case 'r': {
        mb3dsoundings.edit_mode = MBS_EDIT_RESTORE;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      case '{':
      case '[':
      case 'T':
      case 't': {
        mb3dsoundings.edit_mode = MBS_EDIT_GRAB;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      case '}':
      case ']':
      case 'Y':
      case 'y': {
        mb3dsoundings.edit_mode = MBS_EDIT_INFO;
        mb3dsoundings_updatemodetoggles();
        mb3dsoundings_updatecursor();
      } break;
      default:
        break;
      } /* end of key switch */
    }     /* end of key press events */

    /* Deal with KeyRelease events */
    if (event->xany.type == KeyRelease) {
      /* Get key pressed - buffer[0] */
      // int actual =
      char buffer[1];
      KeySym keysym;
      XLookupString((XKeyEvent *)event, buffer, 1, &keysym, NULL);

      /* process events */
      switch (buffer[0]) {
      case 'G':
      case 'g':
        key_g_down = 0;
        break;
      case 'M':
      case 'm':
      case 'Z':
      case 'z':
        key_z_down = 0;
        break;
      case 'K':
      case 'k':
      case 'S':
      case 's':
        key_s_down = 0;
        break;
      case 'J':
      case 'j':
      case 'A':
      case 'a':
        key_a_down = 0;
        break;
      case 'L':
      case 'l':
      case 'D':
      case 'd':
        key_d_down = 0;
        break;
      default:
        break;
      } /* end of key switch */
    }     /* end of key release events */

  } /* end of inputs from window */
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_glwda_resize(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_glwda_resize\n"); */
}

/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_scale(int verbose, int *error) {
  (void)verbose;  // Unused parameter
  (void)error;  // Unused parameter

  /* fprintf(stderr,"Called mb3dsoundings_scale\n"); */

  /* loop over all soundings */
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    sounding->glx = soundingdata->scale * sounding->x;
    sounding->gly = soundingdata->scale * sounding->y;
    sounding->glz = mb3dsoundings.exageration * soundingdata->zscale * (sounding->z - soundingdata->zorigin);
  }

  return (mbs_status);
}

/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_scalez(int verbose, int *error) {
  (void)verbose;  // Unused parameter
  (void)error;  // Unused parameter

  /* fprintf(stderr,"Called mb3dsoundings_scalez\n"); */

  /* loop over all soundings */
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    sounding->glz = mb3dsoundings.exageration * soundingdata->zscale * (sounding->z - soundingdata->zorigin);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_pick(int x, int y) {
  /* fprintf(stderr,"Called mb3dsoundings_pick\n"); */

  /* loop over all soundings */
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  double rmin = 10000.0;
  int irmin = 0;
  bool editevent = false;
  struct mb3dsoundings_sounding_struct *sounding;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    sounding = (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    const double dx = (double)(x - sounding->winx);
    const double dy = (double)(y - sounding->winy);
    const double r = sqrt(dx * dx + dy * dy);
    if (r < rmin && (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE || mb_beam_ok(sounding->beamflag))) {
      irmin = i;
      rmin = r;
    }
  }
  if (rmin < MBS_PICK_THRESHOLD) {
    sounding = (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[irmin]);
    if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
      if (mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        editevent = true;

        /* last sounding edited */
        mb3dsoundings.last_sounding_defined = true;
        mb3dsoundings.last_sounding_edited = irmin;
      }
      else if (mb3dsoundings.view_secondary || !mb_beam_check_flag_multipick(sounding->beamflag)) {
        sounding->beamflag = MB_FLAG_NONE;
        soundingdata->num_soundings_unflagged++;
        soundingdata->num_soundings_flagged--;
        editevent = true;

        /* last sounding edited */
        mb3dsoundings.last_sounding_defined = true;
        mb3dsoundings.last_sounding_edited = irmin;
      }
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
      if (mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        editevent = true;

        /* last sounding edited */
        mb3dsoundings.last_sounding_defined = true;
        mb3dsoundings.last_sounding_edited = irmin;
      }
    }
  }
  else {
    XBell(mb3dsoundings.dpy, 100);
  }

  /* replot the data */
  if (editevent) {
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();
  }

  /* communicate edit event back to calling application */
  if (editevent && mb3dsoundings.mb3dsoundings_edit_notify != NULL) {
    (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam, sounding->beamflag,
                                              MB3DSDG_EDIT_FLUSH);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_eraserestore(int x, int y) {
  /* fprintf(stderr,"\nCalled mb3dsoundings_eraserestore\n"); */

  /* loop over all soundings */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    const double dx = x - sounding->winx;
    const double dy = y - sounding->winy;
    const double r = sqrt(dx * dx + dy * dy);
    if (r < MBS_ERASE_THRESHOLD) {
      bool editevent = false;
      if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE && mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        editevent = true;

        /* last sounding edited */
        mb3dsoundings.last_sounding_defined = true;
        mb3dsoundings.last_sounding_edited = i;
      }
      else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE
                && !mb_beam_ok(sounding->beamflag)
                && (mb3dsoundings.view_secondary
                    || !mb_beam_check_flag_multipick(sounding->beamflag))) {

        sounding->beamflag = MB_FLAG_NONE;
        soundingdata->num_soundings_unflagged++;
        soundingdata->num_soundings_flagged--;
        editevent = true;

        /* last sounding edited */
        mb3dsoundings.last_sounding_defined = true;
        mb3dsoundings.last_sounding_edited = i;
      }

      /* handle valid edit event */
      if (editevent) {
        neditevent++;

        /* communicate edit event back to calling application */
        /* fprintf(stderr,"calling mb3dsoundings_edit_notify: sounding:%d file:%d ping:%d beam:%d beamflag:%d flush:%d\n",
        i,sounding->ifile, sounding->iping,sounding->ibeam, sounding->beamflag,MB3DSDG_EDIT_NOFLUSH); */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_grab(int x, int y, int grabmode) {
  /* fprintf(stderr,"Called mb3dsoundings_grab mode:%d x:%d y:%d\n", grabmode, x, y); */

  /* save grab start point */
  if (grabmode == MBS_EDIT_GRAB_START) {
    /* set grab parameters */
    mb3dsoundings.grab_start_defined = true;
    mb3dsoundings.grab_end_defined = false;
    mb3dsoundings.grab_start_x = x;
    mb3dsoundings.grab_start_y = y;
    mb3dsoundings.grab_end_x = x;
    mb3dsoundings.grab_end_y = y;

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }

  /* save grab end point */
  else if (grabmode == MBS_EDIT_GRAB_MOVE) {
    /* set grab parameters */
    mb3dsoundings.grab_end_defined = true;
    mb3dsoundings.grab_end_x = x;
    mb3dsoundings.grab_end_y = y;

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }

  /* apply grab */
  else if (grabmode == MBS_EDIT_GRAB_END) {
    /* loop over all soundings */
    int neditevent = 0;
    const int xmin = MIN(mb3dsoundings.grab_start_x, mb3dsoundings.grab_end_x);
    const int xmax = MAX(mb3dsoundings.grab_start_x, mb3dsoundings.grab_end_x);
    const int ymin = MIN(mb3dsoundings.grab_start_y, mb3dsoundings.grab_end_y);
    const int ymax = MAX(mb3dsoundings.grab_start_y, mb3dsoundings.grab_end_y);
    /* fprintf(stderr, "Grab bounds: %d %d %d %d\n", xmin, xmax, ymin, ymax); */
    struct mb3dsoundings_struct *soundingdata =
      (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      // bool editevent = false;
      if (sounding->winx >= xmin && sounding->winx <= xmax && sounding->winy >= ymin && sounding->winy <= ymax) {
        if (mb_beam_ok(sounding->beamflag)) {
          if (sounding->beamflag != sounding->beamflagorg)
            sounding->beamflag = sounding->beamflagorg;
          else
            sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
          soundingdata->num_soundings_unflagged--;
          soundingdata->num_soundings_flagged++;
          // editevent = true;
          neditevent++;

          /* last sounding edited */
          mb3dsoundings.last_sounding_defined = true;
          mb3dsoundings.last_sounding_edited = i;

          /* communicate edit event back to calling application */
          if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
            (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                      sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
        }
      }
    }

    /* grab done so unset grab flags */
    mb3dsoundings.grab_start_defined = false;
    mb3dsoundings.grab_end_defined = false;

    /* replot and flush the edit events in the calling application */
    if (neditevent > 0) {
      /* replot the data */
      mb3dsoundings_plot(mbs_verbose, &mbs_error);
      mb3dsoundings_updatestatus();

      /* flush the edit events in the calling application */
      (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
    }
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_unflag_view() {
  /* fprintf(stderr,"Called mb3dsoundings_unflag_view\n"); */

  /* loop over all soundings */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    if (!mb_beam_ok(sounding->beamflag)) {
      sounding->beamflag = MB_FLAG_NONE;
      soundingdata->num_soundings_unflagged++;
      soundingdata->num_soundings_flagged--;
      neditevent++;

      /* communicate edit event back to calling application */
      if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
        (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam, sounding->beamflag,
                                                  MB3DSDG_EDIT_NOFLUSH);
    }
  }

  /* last sounding edited */
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.last_sounding_edited = 0;

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_flag_view() {
  /* fprintf(stderr,"Called mb3dsoundings_flag_view\n"); */

  /* loop over all soundings */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    if (mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
      soundingdata->num_soundings_unflagged--;
      soundingdata->num_soundings_flagged++;
      neditevent++;

      /* communicate edit event back to calling application */
      if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
        (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam, sounding->beamflag,
                                                  MB3DSDG_EDIT_NOFLUSH);
    }
  }

  /* last sounding edited */
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.last_sounding_edited = 0;

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_info(int x, int y) {
  /* fprintf(stderr,"Called mb3dsoundings_info\n"); */

  /* loop over all soundings */
  struct mb3dsoundings_struct *soundingdata;
  soundingdata = (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  double rmin = 10000.0;
  int irmin = 0;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    const double dx = (double)(x - sounding->winx);
    const double dy = (double)(y - sounding->winy);
    const double r = sqrt(dx * dx + dy * dy);
    if (r < rmin && (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE || mb_beam_ok(sounding->beamflag))) {
      irmin = i;
      rmin = r;
    }
  }
  if (rmin < MBS_PICK_THRESHOLD) {
    /* select closest sounding */
    mb3dsoundings.last_sounding_defined = true;
    mb3dsoundings.last_sounding_edited = irmin;

    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();
  }
  else {
    XBell(mb3dsoundings.dpy, 100);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_bad_ping() {
  /* fprintf(stderr,"Called mb3dsoundings_bad_ping last sounding: %d %d\n",
  mb3dsoundings.last_sounding_defined, mb3dsoundings.last_sounding_edited); */

  /* only check if last sounding defined */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  if (mb3dsoundings.last_sounding_defined && mb3dsoundings.last_sounding_edited < soundingdata->num_soundings) {
    /* loop over all soundings */
    struct mb3dsoundings_sounding_struct *lastsounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping &&
          mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        neditevent++;

        /* communicate edit event back to calling application */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  } else {
    XBell(mb3dsoundings.dpy, 100);
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_zero_ping() {
  /* fprintf(stderr,"Called mb3dsoundings_bad_ping last sounding: %d %d\n",
  mb3dsoundings.last_sounding_defined, mb3dsoundings.last_sounding_edited); */

  /* only check if last sounding defined */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  if (mb3dsoundings.last_sounding_defined && mb3dsoundings.last_sounding_edited < soundingdata->num_soundings) {
    /* loop over all soundings */
    struct mb3dsoundings_sounding_struct *lastsounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping) {
        if (mb_beam_ok(sounding->beamflag))
          soundingdata->num_soundings_unflagged--;
        if (!mb_beam_ok(sounding->beamflag))
          soundingdata->num_soundings_flagged--;
        sounding->beamflag = MB_FLAG_NULL;
        neditevent++;

        /* communicate edit event back to calling application */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  }
  else {
    XBell(mb3dsoundings.dpy, 100);
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_left_ping() {

  /* fprintf(stderr,"Called mb3dsoundings_left_ping last sounding: %d %d\n",
  mb3dsoundings.last_sounding_defined, mb3dsoundings.last_sounding_edited); */

  /* only check if last sounding defined */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  if (mb3dsoundings.last_sounding_defined && mb3dsoundings.last_sounding_edited < soundingdata->num_soundings) {
    /* loop over all soundings */
    struct mb3dsoundings_sounding_struct *lastsounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping &&
          sounding->ibeam <= lastsounding->ibeam && mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        neditevent++;

        /* communicate edit event back to calling application */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  } else {
    XBell(mb3dsoundings.dpy, 100);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_right_ping() {
  /* fprintf(stderr,"Called mb3dsoundings_right_ping last sounding: %d %d\n",
  mb3dsoundings.last_sounding_defined, mb3dsoundings.last_sounding_edited); */

  /* only check if last sounding defined */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  if (mb3dsoundings.last_sounding_defined && mb3dsoundings.last_sounding_edited < soundingdata->num_soundings) {
    /* loop over all soundings */
    struct mb3dsoundings_sounding_struct *lastsounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping &&
          sounding->ibeam >= lastsounding->ibeam && mb_beam_ok(sounding->beamflag)) {
        if (sounding->beamflag != sounding->beamflagorg)
          sounding->beamflag = sounding->beamflagorg;
        else
          sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        soundingdata->num_soundings_unflagged--;
        soundingdata->num_soundings_flagged++;
        neditevent++;

        /* communicate edit event back to calling application */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  } else {
    XBell(mb3dsoundings.dpy, 100);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_good_ping() {
  /* fprintf(stderr,"Called mb3dsoundings_good_ping last sounding: %d %d\n",
  mb3dsoundings.last_sounding_defined, mb3dsoundings.last_sounding_edited); */

  /* only check if last sounding defined */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  if (mb3dsoundings.last_sounding_defined && mb3dsoundings.last_sounding_edited < soundingdata->num_soundings) {
    /* loop over all soundings */
    struct mb3dsoundings_sounding_struct *lastsounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping &&
          !mb_beam_ok(sounding->beamflag)) {
        sounding->beamflag = MB_FLAG_NONE;
        soundingdata->num_soundings_unflagged++;
        soundingdata->num_soundings_flagged--;
        neditevent++;

        /* communicate edit event back to calling application */
        if (mb3dsoundings.mb3dsoundings_edit_notify != NULL)
          (mb3dsoundings.mb3dsoundings_edit_notify)(sounding->ifile, sounding->iping, sounding->ibeam,
                                                    sounding->beamflag, MB3DSDG_EDIT_NOFLUSH);
      }
    }
  }
  else {
    XBell(mb3dsoundings.dpy, 100);
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}
/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_setzscale(int verbose, int *error) {
  (void)verbose;  // Unused parameter
  (void)error;  // Unused parameter

  /* get sounding data structure */
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  /* fprintf(stderr,"Called mb3dsoundings_setzscale: %d soundings\n",soundingdata->num_soundings); */

  /* initialize zmin and zmax */
  double zmin = 0.0;
  double zmax = 0.0;

  /* get vertical min maxes for scaling of all soundings */
  if (mb3dsoundings.view_scalewithflagged && soundingdata->num_soundings > 0) {
    zmin = soundingdata->soundings[0].z;
    zmax = soundingdata->soundings[0].z;
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      zmin = MIN(soundingdata->soundings[i].z, zmin);
      zmax = MAX(soundingdata->soundings[i].z, zmax);
    }
  }

  /* else get vertical min maxes for scaling of only unflagged soundings */
  else if (soundingdata->num_soundings > 0) {
    int nunflagged = 0;
    for (int i = 1; i < soundingdata->num_soundings; i++) {
      if (mb_beam_ok(soundingdata->soundings[i].beamflag)) {
        if (nunflagged == 0) {
          zmin = soundingdata->soundings[i].z;
          zmax = soundingdata->soundings[i].z;
        }
        else {
          zmin = MIN(soundingdata->soundings[i].z, zmin);
          zmax = MAX(soundingdata->soundings[i].z, zmax);
        }
        nunflagged++;
      }
    }
  }

  soundingdata->zorigin = 0.5 * (zmin + zmax);
  soundingdata->zmin = -0.5 * (zmax - zmin);
  soundingdata->zmax = 0.5 * (zmax - zmin);
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    soundingdata->soundings[i].glz = mb3dsoundings.exageration * soundingdata->zscale
                                      * (soundingdata->soundings[i].z - soundingdata->zorigin);
  }

  return (mbs_status);
}

/*---------------------------------------------------------------------------------------*/

int mb3dsoundings_plot(int verbose, int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* get sounding data structure */
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
/* fprintf(stderr,"Called mb3dsoundings_plot: %d soundings\n",
soundingdata->num_soundings); */

/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXMakeCurrent(%p,%p,%p)\n", __FILE__, __LINE__, __func__, XtDisplay(mb3dsoundings.glwmda),
          XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);
#endif
  glXMakeCurrent(XtDisplay(mb3dsoundings.glwmda), XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);

#ifdef MBV_GET_GLX_ERRORS
  mbview_glerrorcheck(0, __FILE__, __LINE__, __func__);
#endif


  /* set background color */
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glClearDepth((GLclampd)(2000 * MBS_OPENGL_WIDTH));
  glEnable (GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* set projection */
  mb3dsoundings.left = -1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.right = 1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.bottom = -1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.top = 1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.aspect_ratio = 1.0 / mb3dsoundings.gl_size;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(mb3dsoundings.left, mb3dsoundings.right, mb3dsoundings.bottom, mb3dsoundings.top, MBS_OPENGL_ZMIN2D,
          MBS_OPENGL_ZMAX2D);

  /* set up translations */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(mb3dsoundings.gl_offset_x, mb3dsoundings.gl_offset_y, MBS_OPENGL_ZMIN2D);
  glRotated((float)(mb3dsoundings.elevation - 90.0), 1.0, 0.0, 0.0);
  glRotated((float)(mb3dsoundings.azimuth), 0.0, 0.0, 1.0);
  /* fprintf(stderr,"elevation:%f azimuth:%f\n",mb3dsoundings.elevation,mb3dsoundings.azimuth); */

  /* get modelview and projection matrices and viewport */
  GLdouble model_matrix[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
  GLdouble projection_matrix[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  float glxmin, glymin, glxmax, glymax;

  /* Plot the bounding box if desired */
  if (mb3dsoundings.view_boundingbox) {
    glxmin = soundingdata->scale * soundingdata->xmin;
    glxmax = soundingdata->scale * soundingdata->xmax;
    glymin = soundingdata->scale * soundingdata->ymin;
    glymax = soundingdata->scale * soundingdata->ymax;
    float glzmin = mb3dsoundings.exageration * soundingdata->zscale * soundingdata->zmin;
    float glzmax = mb3dsoundings.exageration * soundingdata->zscale * soundingdata->zmax;
    glLineWidth(1.0);
    glColor3f(0.0, 0.0, 0.0);
    glEnable(GL_LINE_STIPPLE);

    if (mb3dsoundings.elevation <= 0.0)
      glLineStipple(1, 0xFFFF);
    else
      glLineStipple(1, 0x1111);
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmin, glymin, glzmin);
    glVertex3f(glxmax, glymin, glzmin);
    glVertex3f(glxmax, glymax, glzmin);
    glVertex3f(glxmin, glymax, glzmin);
    glEnd();

    if (mb3dsoundings.elevation >= 0.0)
      glLineStipple(1, 0xFFFF);
    else
      glLineStipple(1, 0x1111);
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmin, glymin, glzmax);
    glVertex3f(glxmax, glymin, glzmax);
    glVertex3f(glxmax, glymax, glzmax);
    glVertex3f(glxmin, glymax, glzmax);
    glEnd();

    if ((mb3dsoundings.azimuth >= 0.0 && mb3dsoundings.azimuth <= 90.0) ||
        (mb3dsoundings.azimuth >= 270.0 && mb3dsoundings.azimuth <= 360.0)) {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0xFFFF);
      else
        glLineStipple(1, 0x1111);
    }
    else {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0x1111);
      else
        glLineStipple(1, 0xFFFF);
    }
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmin, glymin, glzmin);
    glVertex3f(glxmax, glymin, glzmin);
    glVertex3f(glxmax, glymin, glzmax);
    glVertex3f(glxmin, glymin, glzmax);
    glEnd();

    if (mb3dsoundings.azimuth >= 180.0 && mb3dsoundings.azimuth <= 360.0) {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0xFFFF);
      else
        glLineStipple(1, 0x1111);
    }
    else {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0x1111);
      else
        glLineStipple(1, 0xFFFF);
    }
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmax, glymin, glzmin);
    glVertex3f(glxmax, glymax, glzmin);
    glVertex3f(glxmax, glymax, glzmax);
    glVertex3f(glxmax, glymin, glzmax);
    glEnd();

    if (mb3dsoundings.azimuth >= 90.0 && mb3dsoundings.azimuth <= 270.0) {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0xFFFF);
      else
        glLineStipple(1, 0x1111);
    }
    else {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0x1111);
      else
        glLineStipple(1, 0xFFFF);
    }
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmax, glymax, glzmin);
    glVertex3f(glxmin, glymax, glzmin);
    glVertex3f(glxmin, glymax, glzmax);
    glVertex3f(glxmax, glymax, glzmax);
    glEnd();

    if ((mb3dsoundings.azimuth >= 0.0 && mb3dsoundings.azimuth <= 180.0) ||
        (mb3dsoundings.azimuth >= 0.0 && mb3dsoundings.azimuth <= 90.0)) {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0xFFFF);
      else
        glLineStipple(1, 0x1111);
    }
    else {
      if (mb3dsoundings.elevation >= -90.0 && mb3dsoundings.elevation <= 90.0)
        glLineStipple(1, 0x1111);
      else
        glLineStipple(1, 0xFFFF);
    }
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmin, glymax, glzmin);
    glVertex3f(glxmin, glymin, glzmin);
    glVertex3f(glxmin, glymin, glzmax);
    glVertex3f(glxmin, glymax, glzmax);
    glEnd();

    glDisable(GL_LINE_STIPPLE);
  }

  /* Plot the profiles if desired */
  if (mb3dsoundings.view_profiles != MBS_VIEW_PROFILES_NONE) {
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 0; i < soundingdata->num_soundings - 1; i++) {
      struct mb3dsoundings_sounding_struct *sounding = (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      struct mb3dsoundings_sounding_struct *sounding2 = (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i + 1]);

      /* plot segment only if soundings are from the same ping */
      if (sounding2->ifile == sounding->ifile && sounding->iping == sounding2->iping) {
        /* plot in black if both soundings are good */
        if (mb_beam_ok(sounding->beamflag) && mb_beam_ok(sounding2->beamflag)) {
          glColor3f(0.0, 0.0, 0.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
          glVertex3f(sounding2->glx, sounding2->gly, sounding2->glz);
        }

        /* else plot in red if flagged profiles are desired */
        else if (mb3dsoundings.view_profiles == MBS_VIEW_PROFILES_ALL) {
          glColor3f(1.0, 0.0, 0.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
          glVertex3f(sounding2->glx, sounding2->gly, sounding2->glz);
        }
      }
    }
    glEnd();
  }

  /* Plot the soundings */

  /* Color by flag state */
  if (mb3dsoundings.view_color == MBS_VIEW_COLOR_FLAG) {
    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);

      /* plot unflagged sounding */
      if (mb_beam_ok(sounding->beamflag)) {
        glColor3f(colortable_object_red[sounding->beamcolor], colortable_object_green[sounding->beamcolor],
                  colortable_object_blue[sounding->beamcolor]);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      }

      /* plot flagged sounding if requested */
      else if (mb3dsoundings.view_flagged) {
        if (mb_beam_check_flag_manual(sounding->beamflag)) {
          glColor3f(1.0, 0.0, 0.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
        } else if (mb_beam_check_flag_filter(sounding->beamflag)
                    || mb_beam_check_flag_filter2(sounding->beamflag)) {
          glColor3f(0.0, 0.0, 1.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
        } else if (mb_beam_check_flag_sonar(sounding->beamflag)) {
          glColor3f(0.0, 1.0, 0.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
        } else if (mb3dsoundings.view_secondary && mb_beam_check_flag_multipick(sounding->beamflag)) {
          glColor3f(0.0, 1.0, 1.0);
          glVertex3f(sounding->glx, sounding->gly, sounding->glz);
        }
      }
    }
    glEnd();
  }

  /* Color by topography */
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_TOPO) {
    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);

      /* plot sounding */
      if (mb_beam_ok(sounding->beamflag)
          || (mb3dsoundings.view_flagged
              && !mb_beam_check_flag_null(sounding->beamflag)
              && (!mb_beam_check_flag_multipick(sounding->beamflag
                  || mb3dsoundings.view_secondary)))) {
        glColor3f(sounding->r, sounding->g, sounding->b);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      }
    }
    glEnd();
  }

  /* Color by amplitude */
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP) {

    double ampmin = 0.0;
    double ampmax = 0.0;
    bool first = true;
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (mb_beam_ok(sounding->beamflag)
          || (mb3dsoundings.view_flagged
              && !mb_beam_check_flag_null(sounding->beamflag)
              && (!mb_beam_check_flag_multipick(sounding->beamflag
                  || mb3dsoundings.view_secondary)))) {
        if (first) {
          first = false;
          ampmin = sounding->a;
          ampmax = sounding->a;
        } else {
          ampmin = MIN(ampmin, sounding->a);
          ampmax = MAX(ampmax, sounding->a);
        }
      }
    }

    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);

      /* plot sounding */
      if (mb_beam_ok(sounding->beamflag)
          || (mb3dsoundings.view_flagged
              && !mb_beam_check_flag_null(sounding->beamflag)
              && (!mb_beam_check_flag_multipick(sounding->beamflag
                  || mb3dsoundings.view_secondary)))) {
        float r, g, b;
        mbview_getcolor(sounding->a, ampmin, ampmax, MBV_COLORTABLE_NORMAL,
                        (float)0.0, (float)0.0, (float)1.0, (float)1.0, (float)0.0, (float)0.0,
                        colortable_redtoblue_red, colortable_redtoblue_green, colortable_redtoblue_blue,
                        &r, &g, &b);
        glColor3f(r, g, b);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      }
    }
    glEnd();
  }

  /* If in info mode and sounding picked plot it green if view color by flag, black otherwise */
  if (mb3dsoundings.edit_mode == MBS_EDIT_INFO && mb3dsoundings.last_sounding_defined) {
    if (mb3dsoundings.view_color == MBS_VIEW_COLOR_FLAG) {
      glColor3f(0.0, 1.0, 1.0);
    }
    else {
      glColor3f(0.0, 0.0, 0.0);
    }
    glBegin(GL_POINTS);
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
    glVertex3f(sounding->glx, sounding->gly, sounding->glz);
    glEnd();
  }

  /* save the screen positions of the soundings to facilitate picking */
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    if (mb_beam_ok(sounding->beamflag)) {
      glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      /* fprintf(stderr," PLOTTED");*/
    }
    double xx;
    double yy;
    double zz;
    gluProject((double)(sounding->glx), (double)(sounding->gly), (double)(sounding->glz), model_matrix, projection_matrix,
               viewport, &xx, &yy, &zz);
    sounding->winx = (int)xx;
    sounding->winy = (int)yy;
  }

  /* plot grab rectangle before rotations */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if (mb3dsoundings.button1down && mb3dsoundings.grab_start_defined &&
      mb3dsoundings.grab_end_defined) {
    const int grabxmin = MIN(mb3dsoundings.grab_start_x, mb3dsoundings.grab_end_x);
    const int grabxmax = MAX(mb3dsoundings.grab_start_x, mb3dsoundings.grab_end_x);
    const int grabymin = MIN(mb3dsoundings.grab_start_y, mb3dsoundings.grab_end_y);
    const int grabymax = MAX(mb3dsoundings.grab_start_y, mb3dsoundings.grab_end_y);
    glxmin = (mb3dsoundings.right - mb3dsoundings.left) * ((float)grabxmin) / ((float)mb3dsoundings.gl_width) -
             0.5 * (mb3dsoundings.right - mb3dsoundings.left);
    glxmax = (mb3dsoundings.right - mb3dsoundings.left) * ((float)grabxmax) / ((float)mb3dsoundings.gl_width) -
             0.5 * (mb3dsoundings.right - mb3dsoundings.left);
    glymin = (mb3dsoundings.top - mb3dsoundings.bottom) * ((float)grabymin) / ((float)mb3dsoundings.gl_height) -
             0.5 * (mb3dsoundings.top - mb3dsoundings.bottom);
    glymax = (mb3dsoundings.top - mb3dsoundings.bottom) * ((float)grabymax) / ((float)mb3dsoundings.gl_height) -
             0.5 * (mb3dsoundings.top - mb3dsoundings.bottom);
    /* fprintf(stderr,"glxmin:%f glxmax:%f glymin:%f glymax:%f\n", glxmin, glxmax, glymin, glymax); */
    glColor3f(1.0, 1.0, 0.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(glxmin, glymin, -MBS_OPENGL_ZMIN2D - 0.5 * (MBS_OPENGL_ZMAX2D - MBS_OPENGL_ZMIN2D));
    glVertex3f(glxmax, glymin, -MBS_OPENGL_ZMIN2D - 0.5 * (MBS_OPENGL_ZMAX2D - MBS_OPENGL_ZMIN2D));
    glVertex3f(glxmax, glymax, -MBS_OPENGL_ZMIN2D - 0.5 * (MBS_OPENGL_ZMAX2D - MBS_OPENGL_ZMIN2D));
    glVertex3f(glxmin, glymax, -MBS_OPENGL_ZMIN2D - 0.5 * (MBS_OPENGL_ZMAX2D - MBS_OPENGL_ZMIN2D));
    glEnd();
    glLineWidth(1.0);
  }
  glDisable (GL_DEPTH_TEST);

  /* flush opengl buffers */
  glFlush();

  /* swap opengl buffers */
  glXSwapBuffers(XtDisplay(mb3dsoundings.glwmda), XtWindow(mb3dsoundings.glwmda));

#ifdef MBV_GET_GLX_ERRORS
  mbview_glerrorcheck(0, __FILE__, __LINE__, __func__);
#endif

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*---------------------------------------------------------------------------------------*/
int mb3dsoundings_get_bias_values(int verbose, double *rollbias, double *pitchbias,
          double *headingbias, double *timelag, double *snell,
                                  int *error) {
  (void)verbose;  // Unused parameter
  (void)error;  // Unused parameter
  /* fprintf(stderr,"Called mb3dsoundings_get_bias_values\n"); */

  /* get bias parameters */
  *rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
  *pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
  *headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
  *timelag = 0.01 * ((double)mb3dsoundings.itimelag);
  *snell = 0.0001 * ((double)mb3dsoundings.isnell);

  return (mbs_status);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_rollbias(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_rollbias: %d\n", acs->value); */

  mb3dsoundings.irollbias = acs->value;

  ac = 0;
  int irollbiasmin;
  XtSetArg(args[ac], XmNminimum, &irollbiasmin);
  ac++;
  int irollbiasmax;
  XtSetArg(args[ac], XmNmaximum, &irollbiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, &(mb3dsoundings.irollbias));
  ac++;
  XtGetValues(mb3dsoundings.mb3dsdg.scale_rollbias, args, ac);

  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);

  /* reset scale min max */
  if (mb3dsoundings.irollbias == irollbiasmin || mb3dsoundings.irollbias == irollbiasmax) {
    irollbiasmin = mb3dsoundings.irollbias - 100;
    irollbiasmax = mb3dsoundings.irollbias + 100;
    ac = 0;
    XtSetArg(args[ac], XmNminimum, irollbiasmin);
    ac++;
    XtSetArg(args[ac], XmNmaximum, irollbiasmax);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.scale_rollbias, args, ac);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_pitchbias(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_pitchbias: %d\n", acs->value); */

  mb3dsoundings.ipitchbias = acs->value;

  ac = 0;
  int ipitchbiasmin;
  XtSetArg(args[ac], XmNminimum, &ipitchbiasmin);
  ac++;
  int ipitchbiasmax;
  XtSetArg(args[ac], XmNmaximum, &ipitchbiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, &(mb3dsoundings.ipitchbias));
  ac++;
  XtGetValues(mb3dsoundings.mb3dsdg.scale_pitchbias, args, ac);

  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);

  /* reset scale min max */
  if (mb3dsoundings.ipitchbias == ipitchbiasmin || mb3dsoundings.ipitchbias == ipitchbiasmax) {
    ipitchbiasmin = mb3dsoundings.ipitchbias - 100;
    ipitchbiasmax = mb3dsoundings.ipitchbias + 100;
    ac = 0;
    XtSetArg(args[ac], XmNminimum, ipitchbiasmin);
    ac++;
    XtSetArg(args[ac], XmNmaximum, ipitchbiasmax);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.scale_pitchbias, args, ac);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_headingbias(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_headingbias: %d\n", acs->value); */

  mb3dsoundings.iheadingbias = acs->value;

  ac = 0;
  int iheadingbiasmin;
  XtSetArg(args[ac], XmNminimum, &iheadingbiasmin);
  ac++;
  int iheadingbiasmax;
  XtSetArg(args[ac], XmNmaximum, &iheadingbiasmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, &(mb3dsoundings.iheadingbias));
  ac++;
  XtGetValues(mb3dsoundings.mb3dsdg.scale_headingbias, args, ac);

  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);

  /* reset scale min max */
  if (mb3dsoundings.iheadingbias == iheadingbiasmin || mb3dsoundings.iheadingbias == iheadingbiasmax) {
    iheadingbiasmin = mb3dsoundings.iheadingbias - 100;
    iheadingbiasmax = mb3dsoundings.iheadingbias + 100;
    ac = 0;
    XtSetArg(args[ac], XmNminimum, iheadingbiasmin);
    ac++;
    XtSetArg(args[ac], XmNmaximum, iheadingbiasmax);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.scale_headingbias, args, ac);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_timelag(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_timelag: %d\n", acs->value); */

  mb3dsoundings.itimelag = acs->value;

  ac = 0;
  int itimelagmin;
  XtSetArg(args[ac], XmNminimum, &itimelagmin);
  ac++;
  int itimelagmax;
  XtSetArg(args[ac], XmNmaximum, &itimelagmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, &(mb3dsoundings.itimelag));
  ac++;
  XtGetValues(mb3dsoundings.mb3dsdg.scale_timelag, args, ac);

  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);

  /* reset scale min max */
  if (mb3dsoundings.itimelag == itimelagmin || mb3dsoundings.itimelag == itimelagmax) {
    itimelagmin = mb3dsoundings.itimelag - 100;
    itimelagmax = mb3dsoundings.itimelag + 100;
    ac = 0;
    XtSetArg(args[ac], XmNminimum, itimelagmin);
    ac++;
    XtSetArg(args[ac], XmNmaximum, itimelagmax);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.scale_timelag, args, ac);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_snell(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

  //fprintf(stderr,"Called do_mb3dsdg_snell: %d\n", acs->value);

  mb3dsoundings.isnell = acs->value;

  ac = 0;
  int isnellmin;
  XtSetArg(args[ac], XmNminimum, &isnellmin);
  ac++;
  int isnellmax;
  XtSetArg(args[ac], XmNmaximum, &isnellmax);
  ac++;
  XtSetArg(args[ac], XmNvalue, &(mb3dsoundings.isnell));
  ac++;
  XtGetValues(mb3dsoundings.mb3dsdg.scale_snell, args, ac);

  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);

  /* reset scale min max */
  if (mb3dsoundings.isnell == isnellmin || mb3dsoundings.isnell == isnellmax) {
    isnellmin = mb3dsoundings.isnell - 100;
    isnellmax = mb3dsoundings.isnell + 100;
    ac = 0;
    XtSetArg(args[ac], XmNminimum, isnellmin);
    ac++;
    XtSetArg(args[ac], XmNmaximum, isnellmax);
    ac++;
    XtSetValues(mb3dsoundings.mb3dsdg.scale_snell, args, ac);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_flagged(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_flagged\n"); */

  mb3dsoundings.view_flagged = XmToggleButtonGetState(mb3dsoundings.mb3dsdg.toggleButton_view_flagged);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_secondary(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_secondary\n"); */

  mb3dsoundings.view_secondary = XmToggleButtonGetState(mb3dsoundings.mb3dsdg.toggleButton_view_secondary);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_noprofile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_noprofile\n"); */

  mb3dsoundings.view_profiles = MBS_VIEW_PROFILES_NONE;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, False, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_goodprofile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_goodprofile\n"); */

  mb3dsoundings.view_profiles = MBS_VIEW_PROFILES_UNFLAGGED;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, False, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_allprofile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_allprofile\n"); */

  mb3dsoundings.view_profiles = MBS_VIEW_PROFILES_ALL;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_noconnect, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectgood, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_connectall, True, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_resetview(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_reset\n"); */

  /* reset view orientation */
  mb3dsoundings.elevation = 0.0;
  mb3dsoundings.azimuth = 0.0;
  mb3dsoundings.exageration = 1.0;

  /* rescale */
  mb3dsoundings_scalez(mbs_verbose, &mbs_error);
  mb3dsoundings_updatestatus();

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_boundingbox(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_boundingbox\n"); */

  mb3dsoundings.view_boundingbox = XmToggleButtonGetState(mb3dsoundings.mb3dsdg.toggleButton_view_boundingbox);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_scalewithflagged(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_view_scalewithflagged\n"); */

  mb3dsoundings.view_scalewithflagged = XmToggleButtonGetState(mb3dsoundings.mb3dsdg.toggleButton_view_scalewithflagged);

  /* replot the data */
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_colorbyflag(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  // fprintf(stderr,"Called do_mb3dsdg_view_colorbyflag\n");

  mb3dsoundings.view_color = MBS_VIEW_COLOR_FLAG;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_colorbytopo(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  // fprintf(stderr,"Called do_mb3dsdg_view_colorbytopo\n");

  mb3dsoundings.view_color = MBS_VIEW_COLOR_TOPO;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_colorbyamp(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  // fprintf(stderr,"Called do_mb3dsdg_view_colorbyamp\n");

  mb3dsoundings.view_color = MBS_VIEW_COLOR_AMP;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, True, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_applybias(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_applybias\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_biasapply_notify != NULL)
    (mb3dsoundings.mb3dsoundings_biasapply_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_A(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_A\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(1, 10);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_B(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_B\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(1, 2);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_C(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_C\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(4, 10);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_D(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_D\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(4, 2);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_E(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_E\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(8, 10);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_flagsparsevoxels_F(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_F\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(8, 2);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsblack(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsblack\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLACK);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsred(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsred\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_RED);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsyellow(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsyellow\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_YELLOW);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsgreen(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsgreen\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_GREEN);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsbluegreen(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsbluegreen\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLUEGREEN);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingsblue(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsblue\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLUE);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_colorsoundingspurple(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingspurple\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_PURPLE);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_r(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_r\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    double rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    double pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    double headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    double timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    double snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_R, &rollbias, &pitchbias, &headingbias,
                                                            &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_p(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_p\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_P, &rollbias, &pitchbias, &headingbias,
                                                            &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_h(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_h\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    double rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    double pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    double headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    double timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    double snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_H, &rollbias, &pitchbias, &headingbias,
                                                            &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_rp(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_rp\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_RP, &rollbias, &pitchbias,
                                                            &headingbias, &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_rph(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_rph\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_RPH, &rollbias, &pitchbias,
                                                            &headingbias, &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_t(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_rph\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    double rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    double pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    double headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    double timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    double snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_T, &rollbias, &pitchbias, &headingbias,
                                                            &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_action_optimizebiasvalues_s(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_optimizebiasvalues_rph\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify != NULL) {
    /* get bias parameters */
    double rollbias = 0.01 * ((double)mb3dsoundings.irollbias);
    double pitchbias = 0.01 * ((double)mb3dsoundings.ipitchbias);
    double headingbias = 0.01 * ((double)mb3dsoundings.iheadingbias);
    double timelag = 0.01 * ((double)mb3dsoundings.itimelag);
    double snell = 0.0001 * ((double)mb3dsoundings.isnell);

    (mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify)(MB3DSDG_OPTIMIZEBIASVALUES_S, &rollbias, &pitchbias, &headingbias,
                                                            &timelag, &snell);

    /* set the bias parameters stored for the gui */
    mb3dsoundings.irollbias = (int)(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)(100 * headingbias);
    mb3dsoundings.itimelag = (int)(100 * timelag);
    mb3dsoundings.isnell = (int)(10000 * snell);

    /* update the gui */
    mb3dsoundings_updategui();

    /* rescale data to the gl coordinates */
    mb3dsoundings_scale(mbs_verbose, &mbs_error);
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
  }
}

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_mouse_panzoom(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_mouse_panzoom\n"); */

  mb3dsoundings.mouse_mode = MBS_MOUSE_PANZOOM;
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate1, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom1, True, False);

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();
}

/*---------------------------------------------------------------------------------------*/
void do_mb3dsdg_mouse_rotate(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_mouse_rotate\n"); */

  mb3dsoundings.mouse_mode = MBS_MOUSE_ROTATE;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_rotate1, True, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_mouse_panzoom1, False, False);

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();
}

/*---------------------------------------------------------------------------------------*/
