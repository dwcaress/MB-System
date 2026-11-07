/*--------------------------------------------------------------------
 *    The MB-system:  mb3dsoundings_callbacks.c    5/25/2007
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

/* mb3dsoundings include */
#include "mb3dsoundingsprivate.h"
#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_input.h"
#include "mbv_ui.h"

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

  /* draw with GLX */
  mbv_render_use_glx();

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
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_TOPO) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_SOUNDING) {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, True, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);
  }
  else /* if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP) */ {
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
    XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, True, False);
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
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Pan\"\"R: Zoom\"");
  }
  else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
    snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Pan\"\"R: Zoom\"");
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
    snprintf(value_text, sizeof(value_text), "Azi:%.2f | Elev: %.2f | exagger:%.2f | Tot:%d Good:%d Flagged:%d", mb3dsoundings.azimuth,
            mb3dsoundings.elevation, mb3dsoundings.exaggeration, soundingdata->num_soundings,
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
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Pan\"\"R: Zoom\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Pan\"\"R: Zoom\"");
    }
  }
  else /* if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) */ {
    if (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Toggle)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_PICK) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Pick)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Erase)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Restore)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Grab)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
    }
    else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO) {
      snprintf(value_text, sizeof(value_text), ":::t\"Mouse Mode:\":t\"L: Edit (Info)\":t\"M: Rotate Soundings\"\"R: exaggeration\"");
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
      mbv_render_context_destroy(MBV_RENDER_SOUNDINGS, 0);
      XtDestroyWidget(mb3dsoundings.glwmda);
      mb3dsoundings.glx_init = false;
    }

    /* destroy the topLevelShell and all its children */
    XtDestroyWidget(mb3dsoundings.topLevelShell);

    /* reset init flag */
    mb3dsoundings.init = MBS_WINDOW_NULL;
  }

  /* reinitialize parameters, but keep the patch test bias values: the
   * calling application keeps applying them (e.g. mbeditviz regrids
   * with the values returned by mb3dsoundings_get_bias_values()), so the
   * sliders must still show them when the window is reopened */
  const int irollbias = mb3dsoundings.irollbias;
  const int ipitchbias = mb3dsoundings.ipitchbias;
  const int iheadingbias = mb3dsoundings.iheadingbias;
  const int itimelag = mb3dsoundings.itimelag;
  const int isnell = mb3dsoundings.isnell;
  mb3dsoundings_reset();
  mb3dsoundings.irollbias = irollbias;
  mb3dsoundings.ipitchbias = ipitchbias;
  mb3dsoundings.iheadingbias = iheadingbias;
  mb3dsoundings.itimelag = itimelag;
  mb3dsoundings.isnell = isnell;

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
/* bell of the soundings view (see mbv_ui.h) */
void mbv_ui_soundings_bell(void) {
  XBell(mb3dsoundings.dpy, 100);
}

/*------------------------------------------------------------------------------*/
/* mouse cursor of the soundings view (see mbv_ui.h) */
void mbv_ui_soundings_cursor(int cursor) {
  Cursor xcursor;
  switch (cursor) {
  case MBV_CURSOR_TARGET_BLACK: xcursor = mb3dsoundings.TargetBlackCursor; break;
  case MBV_CURSOR_TARGET_GREEN: xcursor = mb3dsoundings.TargetGreenCursor; break;
  case MBV_CURSOR_TARGET_RED: xcursor = mb3dsoundings.TargetRedCursor; break;
  case MBV_CURSOR_TARGET_BLUE: xcursor = mb3dsoundings.TargetBlueCursor; break;
  case MBV_CURSOR_EXCHANGE_BLACK: xcursor = mb3dsoundings.ExchangeBlackCursor; break;
  case MBV_CURSOR_EXCHANGE_GREEN: xcursor = mb3dsoundings.ExchangeGreenCursor; break;
  case MBV_CURSOR_EXCHANGE_RED: xcursor = mb3dsoundings.ExchangeRedCursor; break;
  case MBV_CURSOR_FLEUR_BLACK: xcursor = mb3dsoundings.FleurBlackCursor; break;
  case MBV_CURSOR_FLEUR_RED: xcursor = mb3dsoundings.FleurRedCursor; break;
  default: return;
  }
  XDefineCursor(XtDisplay(mb3dsoundings.mb3dsdg.Mb3dsdg), XtWindow(mb3dsoundings.mb3dsdg.drawingArea), xcursor);
}

/*------------------------------------------------------------------------------*/
/* GLX calls of the soundings view, used by the GLX renderer (mbv_render_glx.c) */
void mb3dsoundings_glx_context_create(void) {
  ac = 0;
  XtSetArg(args[ac], mbGLwNvisualInfo, &(mb3dsoundings.vi));
  ac++;
  XtGetValues(mb3dsoundings.glwmda, args, ac);
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXCreateContext(%p,%p)\n", __FILE__, __LINE__, __func__, mb3dsoundings.dpy, mb3dsoundings.vi);
#endif
  mb3dsoundings.glx_context = glXCreateContext(mb3dsoundings.dpy, mb3dsoundings.vi, NULL, GL_TRUE);
}
void mb3dsoundings_glx_context_destroy(void) {
  glXDestroyContext(mb3dsoundings.dpy, mb3dsoundings.glx_context);
}
void mb3dsoundings_glx_make_current(void) {
  glXMakeCurrent(XtDisplay(mb3dsoundings.glwmda), XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);
}
void mb3dsoundings_glx_present(void) {
  glXSwapBuffers(XtDisplay(mb3dsoundings.glwmda), XtWindow(mb3dsoundings.glwmda));
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
    mbv_render_context_destroy(MBV_RENDER_SOUNDINGS, 0);
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
  mbv_render_context_create(MBV_RENDER_SOUNDINGS, 0);
#ifdef MBV_DEBUG_GLX
  fprintf(stderr, "%s:%d:%s glXMakeCurrent(%p,%p,%p)\n", __FILE__, __LINE__, __func__, XtDisplay(mb3dsoundings.glwmda),
          XtWindow(mb3dsoundings.glwmda), mb3dsoundings.glx_context);
#endif
  mbv_render_make_current(MBV_RENDER_SOUNDINGS, 0);
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
  /* If there is input in the drawing area pass it on as a toolkit-neutral event */
  if (acs->reason == XmCR_INPUT) {
    struct mbv_input_event input;
    mbv_input_from_xevent(event, &input);
    mb3dsoundings_input(&input);
  }
}

/*------------------------------------------------------------------------------*/
/* Handle mouse and key input in the 3D soundings view (see mbv_input.h) */

/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_glwda_resize(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;
  /* fprintf(stderr,"Called do_mb3dsdg_glwda_resize\n"); */
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
  mb3dsoundings.exaggeration = 1.0;

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
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
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
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyamp, False, False);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*---------------------------------------------------------------------------------------*/

void do_mb3dsdg_view_colorbysounding(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  // fprintf(stderr,"Called do_mb3dsdg_view_colorbysounding\n");

  mb3dsoundings.view_color = MBS_VIEW_COLOR_SOUNDING;

  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbyflag, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbytopo, False, False);
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, True, False);
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
  XmToggleButtonSetState(mb3dsoundings.mb3dsdg.toggleButton_view_colorbysounding, False, False);
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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
    mb3dsoundings.irollbias = (int)round(100 * rollbias);
    mb3dsoundings.ipitchbias = (int)round(100 * pitchbias);
    mb3dsoundings.iheadingbias = (int)round(100 * headingbias);
    mb3dsoundings.itimelag = (int)round(100 * timelag);
    mb3dsoundings.isnell = (int)round(10000 * snell);

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
