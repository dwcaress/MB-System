/*--------------------------------------------------------------------
 *    The MB-system:  mbv_soundings_core.c  11/7/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Core functions of the mbview 3D soundings view that use no X11 or Motif:
 * notify functions, reset, drawing (mb3dsoundings_plot), the mouse and key
 * handling (mb3dsoundings_input), picking, and the editing and scaling
 * functions. They were moved unchanged from
 * mb3dsoundings_callbacks.c, which keeps the Motif user interface. The
 * soundings global variables (mb3dsoundingsprivate.h) are defined here.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#ifndef MBVIEW_NO_MOTIF
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#else
#include "mbv_xtypes.h"
#endif
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "Mb3dsdg.h"

#include "mbv_gl.h"

#define MB3DSOUNDINGSGLOBAL
#include "mb3dsoundingsprivate.h"
#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_input.h"
#include "mbv_ui.h"
#include "mbv_camera.h"

/*------------------------------------------------------------------------------*/
/* What changed in the soundings, for users outside the drawing code: a
   renderer that keeps its own copy of their positions and colours. The
   counters of src/mbview/mbv_marks.h only rise, and reading them changes
   nothing. The soundings use three of them and no blocks:
     position - glx/gly/glz of every sounding. Only mb3dsoundings_scale(),
                mb3dsoundings_scalez() and mb3dsoundings_setzscale() write
                them, and each raises this once after its loop; a new writer
                must do the same.
     color    - the colour of a sounding or whether it is drawn: its flag,
                beam colour, r/g/b, z or a, the view switches and the colour
                mode (mb3dsoundings_colors()).
     mask     - the sounding array itself was handed over or released.
   A mark is raised where values become invalid, before any replot, never
   where they are computed again. A mark raised by mb3dsoundings_reset() or
   mb3dsoundings_dismissed() means stop reading: the calling program
   releases the array right afterwards. These are file statics, not fields
   of struct mb3dsoundings_world_struct, so that the layout of that
   structure, which the Motif programs share, does not change. */
static struct mbv_marks mb3dsoundings_marks;

/* the camera and the data the screen positions winx/winy were last computed
   for, by the drawing or by mb3dsoundings_screen_positions() */
static struct {
  bool valid;
  uint64_t position;
  uint64_t mask;
  float elevation;
  float azimuth;
  float gl_offset_x;
  float gl_offset_y;
  float gl_size;
  int gl_width;
  int gl_height;
} mb3dsoundings_screen;

/* record that winx/winy now belong to the current camera and soundings */
static void mb3dsoundings_screen_stamp(void) {
  mb3dsoundings_screen.valid = true;
  mb3dsoundings_screen.position = mbv_marks_getposition(&mb3dsoundings_marks);
  mb3dsoundings_screen.mask = mbv_marks_getmask(&mb3dsoundings_marks);
  mb3dsoundings_screen.elevation = mb3dsoundings.elevation;
  mb3dsoundings_screen.azimuth = mb3dsoundings.azimuth;
  mb3dsoundings_screen.gl_offset_x = mb3dsoundings.gl_offset_x;
  mb3dsoundings_screen.gl_offset_y = mb3dsoundings.gl_offset_y;
  mb3dsoundings_screen.gl_size = mb3dsoundings.gl_size;
  mb3dsoundings_screen.gl_width = mb3dsoundings.gl_width;
  mb3dsoundings_screen.gl_height = mb3dsoundings.gl_height;
}

/* whether winx/winy belong to the current camera and soundings */
static bool mb3dsoundings_screen_current(void) {
  return mb3dsoundings_screen.valid
         && mb3dsoundings_screen.position == mbv_marks_getposition(&mb3dsoundings_marks)
         && mb3dsoundings_screen.mask == mbv_marks_getmask(&mb3dsoundings_marks)
         && mb3dsoundings_screen.elevation == mb3dsoundings.elevation
         && mb3dsoundings_screen.azimuth == mb3dsoundings.azimuth
         && mb3dsoundings_screen.gl_offset_x == mb3dsoundings.gl_offset_x
         && mb3dsoundings_screen.gl_offset_y == mb3dsoundings.gl_offset_y
         && mb3dsoundings_screen.gl_size == mb3dsoundings.gl_size
         && mb3dsoundings_screen.gl_width == mb3dsoundings.gl_width
         && mb3dsoundings_screen.gl_height == mb3dsoundings.gl_height;
}

/*------------------------------------------------------------------------------*/
/* The rule that decides which soundings are drawn and in which colour, shared
   by the drawing (mb3dsoundings_plot) and by mb3dsoundings_colors(), so that a
   renderer without OpenGL colours them exactly as OpenGL does. */

/* the colour mode MBS_VIEW_COLOR_FLAG: unflagged soundings in their beam
   colour and, if flagged soundings are shown, flagged ones in the colour of
   their flag reason; false if the sounding is not drawn */
static bool mb3dsoundings_flag_color(const struct mb3dsoundings_sounding_struct *sounding, float *r, float *g,
                                     float *b) {
  /* unflagged sounding */
  if (mb_beam_ok(sounding->beamflag)) {
    *r = colortable_object_red[sounding->beamcolor];
    *g = colortable_object_green[sounding->beamcolor];
    *b = colortable_object_blue[sounding->beamcolor];
    return (true);
  }

  /* flagged sounding if requested */
  if (mb3dsoundings.view_flagged) {
    if (mb_beam_check_flag_manual(sounding->beamflag)) {
      *r = 1.0;
      *g = 0.0;
      *b = 0.0;
      return (true);
    } else if (mb_beam_check_flag_filter(sounding->beamflag)
                || mb_beam_check_flag_filter2(sounding->beamflag)) {
      *r = 0.0;
      *g = 0.0;
      *b = 1.0;
      return (true);
    } else if (mb_beam_check_flag_sonar(sounding->beamflag)) {
      *r = 0.0;
      *g = 1.0;
      *b = 0.0;
      return (true);
    } else if (mb3dsoundings.view_secondary && mb_beam_check_flag_multipick(sounding->beamflag)) {
      *r = 0.0;
      *g = 1.0;
      *b = 1.0;
      return (true);
    }
  }

  return (false);
}

/* the other colour modes: unflagged soundings and, if flagged soundings are
   shown, flagged ones that are not null and not hidden secondary picks */
static bool mb3dsoundings_shown(const struct mb3dsoundings_sounding_struct *sounding) {
  return (mb_beam_ok(sounding->beamflag)
          || (mb3dsoundings.view_flagged
              && !mb_beam_check_flag_null(sounding->beamflag)
              && (!mb_beam_check_flag_multipick(sounding->beamflag)
                  || mb3dsoundings.view_secondary)));
}

/* the depth range of MBS_VIEW_COLOR_SOUNDING: the topography min max over the
   soundings currently viewed, regardless of beamflag state if flagged
   soundings are being shown, else over the unflagged soundings only */
static void mb3dsoundings_depth_range(const struct mb3dsoundings_struct *soundingdata, double *zmin, double *zmax) {
  double min = 0.0;
  double max = 0.0;
  bool first = true;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    const struct mb3dsoundings_sounding_struct *sounding = &(soundingdata->soundings[i]);
    if (mb3dsoundings.view_flagged || mb_beam_ok(sounding->beamflag)) {
      if (first) {
        first = false;
        min = sounding->z;
        max = sounding->z;
      } else {
        min = MIN(min, sounding->z);
        max = MAX(max, sounding->z);
      }
    }
  }
  *zmin = min;
  *zmax = max;
}

/* the colour of MBS_VIEW_COLOR_SOUNDING: the topography colour scale
   stretched over the range of the soundings viewed */
static void mb3dsoundings_depth_color(double z, double zmin, double zmax, float *r, float *g, float *b) {
  mbview_getcolor(z, zmin, zmax, MBV_COLORTABLE_NORMAL,
                  colortable_haxby_red[0], colortable_haxby_green[0], colortable_haxby_blue[0],
                  colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
                  colortable_haxby_blue[MBV_NUM_COLORS - 1],
                  colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue,
                  r, g, b);
}

/* the amplitude range of MBS_VIEW_COLOR_AMP: over the soundings drawn */
static void mb3dsoundings_amplitude_range(const struct mb3dsoundings_struct *soundingdata, double *ampmin,
                                          double *ampmax) {
  double min = 0.0;
  double max = 0.0;
  bool first = true;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    const struct mb3dsoundings_sounding_struct *sounding = &(soundingdata->soundings[i]);
    if (mb3dsoundings_shown(sounding)) {
      if (first) {
        first = false;
        min = sounding->a;
        max = sounding->a;
      } else {
        min = MIN(min, sounding->a);
        max = MAX(max, sounding->a);
      }
    }
  }
  *ampmin = min;
  *ampmax = max;
}

/* the colour of MBS_VIEW_COLOR_AMP: red to blue over the amplitude range */
static void mb3dsoundings_amplitude_color(double a, double ampmin, double ampmax, float *r, float *g, float *b) {
  mbview_getcolor(a, ampmin, ampmax, MBV_COLORTABLE_NORMAL,
                  (float)0.0, (float)0.0, (float)1.0, (float)1.0, (float)0.0, (float)0.0,
                  colortable_redtoblue_red, colortable_redtoblue_green, colortable_redtoblue_blue,
                  r, g, b);
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
  mb3dsoundings.mb3dsoundings_optimizebiasvalues_notify = NULL;

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
  mb3dsoundings.exaggeration = 1.0;
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

  /* the window state is reset and the calling program releases its soundings
     next: users of the soundings stop reading them, and the screen positions
     belong to no camera (the marks keep rising, see the top of this file) */
  mbv_marks_everything(&mb3dsoundings_marks);
  mb3dsoundings_screen.valid = false;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", mbs_status);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
void mb3dsoundings_input(const struct mbv_input_event *event) {
  /* If there is input in the drawing area */
  if (event != NULL) {

    /* Check for mouse pressed. */
    if (event->type == MBV_INPUT_PRESS) {
      /* fprintf(stderr, "event->type == MBV_INPUT_PRESS  button:%d position: %d %d  mouse mode:%d\n",
      event->button,event->x,event->y, mb3dsoundings.mouse_mode); */
      /* save location */
      mb3dsoundings.button_down_x = event->x;
      mb3dsoundings.button_down_y = mb3dsoundings.gl_height - 1 - event->y;

      /* If left mouse button is pushed */
      if (event->button == 1) {
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
      else if (event->button == 2) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set button2down flag */
          mb3dsoundings.button2down = true;

          /* set cursor for rotate */
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_BLACK);

          mb3dsoundings.azimuth_save = mb3dsoundings.azimuth;
          mb3dsoundings.elevation_save = mb3dsoundings.elevation;
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set button2down flag */
          mb3dsoundings.button2down = true;

          /* set cursor for rotate */
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_BLACK);

          mb3dsoundings.gl_offset_x_save = mb3dsoundings.gl_offset_x;
          mb3dsoundings.gl_offset_y_save = mb3dsoundings.gl_offset_y;
        }
      } /* end of middle button events */

      /* If right mouse button is pushed */
      else if (event->button == 3) {
        /* rotate mode */
        if (mb3dsoundings.mouse_mode == MBS_MOUSE_ROTATE) {
          /* set button3down flag */
          mb3dsoundings.button3down = true;

          /* set cursor for exaggerate */
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_BLACK);

          mb3dsoundings.exaggeration_save = mb3dsoundings.exaggeration;
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set button3down flag */
          mb3dsoundings.button3down = true;

          /* set cursor for exaggerate */
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_BLACK);

          mb3dsoundings.gl_size_save = mb3dsoundings.gl_size;
        }

      } /* end of right button events */

    } /* end of button press events */

    /* Check for mouse motion while pressed. */
    if (event->type == MBV_INPUT_MOTION) {
      /* fprintf(stderr, "event->type == MBV_INPUT_MOTION  %d %d  mouse mode:%d\n",
      event->x,event->y, mb3dsoundings.mouse_mode); */

      /* save location */
      mb3dsoundings.button_move_x = event->x;
      mb3dsoundings.button_move_y = mb3dsoundings.gl_height - 1 - event->y;

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
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_RED);

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
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_RED);

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
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_RED);

          /* change vertical exaggeration of 3D map */
          mb3dsoundings.exaggeration = mb3dsoundings.exaggeration_save *
                                      exp(((double)(mb3dsoundings.button_move_y - mb3dsoundings.button_down_y)) /
                                          ((double)mb3dsoundings.gl_height));

          mb3dsoundings_scalez(mbs_verbose, &mbs_error);
          mb3dsoundings_updatestatus();
          mb3dsoundings_plot(mbs_verbose, &mbs_error);
        }

        /* pan zoom mode */
        else if (mb3dsoundings.mouse_mode == MBS_MOUSE_PANZOOM) {
          /* set cursor for zoom */
          mbv_ui_soundings_cursor(MBV_CURSOR_FLEUR_RED);

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
    if (event->type == MBV_INPUT_RELEASE) {
      /* fprintf(stderr, "event->type == MBV_INPUT_RELEASE  %d %d  mouse mode:%d\n",
      event->x,event->y, mb3dsoundings.mouse_mode); */

      /* save location */
      mb3dsoundings.button_up_x = event->x;
      mb3dsoundings.button_up_y = mb3dsoundings.gl_height - 1 - event->y;

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
    if (event->type == MBV_INPUT_EXPOSE) {
      mb3dsoundings_updatestatus();
      mb3dsoundings_plot(mbs_verbose, &mbs_error);
    }

    /* Deal with KeyPress events */
    if (event->type == MBV_INPUT_KEYPRESS) {
      fprintf(stderr,"KeyPress event\n");
      /* Get key pressed */

      /* process events (keys without a character, e.g. Shift, are ignored) */
      switch (event->key) {
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
    if (event->type == MBV_INPUT_KEYRELEASE) {
      /* Get key pressed */

      /* process events (keys without a character, e.g. Shift, are ignored) */
      switch (event->key) {
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

/*------------------------------------------------------------------------------*/
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
    sounding->glz = mb3dsoundings.exaggeration * soundingdata->zscale * (sounding->z - soundingdata->zorigin);
  }

  /* the display positions changed */
  mbv_marks_position(&mb3dsoundings_marks);

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    sounding->glz = mb3dsoundings.exaggeration * soundingdata->zscale * (sounding->z - soundingdata->zorigin);
  }

  /* the display positions changed */
  mbv_marks_position(&mb3dsoundings_marks);

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_eraserestore(int x, int y) {
  /* fprintf(stderr,"\nCalled mb3dsoundings_eraserestore\n"); */

  /* the screen positions of the current camera */
  mb3dsoundings_screen_positions(mbs_verbose, &mbs_error);

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
                && !mb_beam_check_flag_null(sounding->beamflag)
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
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    /* the screen positions of the current camera */
    mb3dsoundings_screen_positions(mbs_verbose, &mbs_error);

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
      /* flags changed: raise the mark before the replot (top of this file) */
      mbv_marks_color(&mb3dsoundings_marks);

      /* replot the data */
      mb3dsoundings_plot(mbs_verbose, &mbs_error);
      mb3dsoundings_updatestatus();

      /* flush the edit events in the calling application */
      (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
    }
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_unflag_view() {
  /* fprintf(stderr,"Called mb3dsoundings_unflag_view\n"); */

  /* loop over all soundings */
  int neditevent = 0;
  struct mb3dsoundings_struct *soundingdata =
    (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  for (int i = 0; i < soundingdata->num_soundings; i++) {
    struct mb3dsoundings_sounding_struct *sounding =
      (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
    if (!mb_beam_ok(sounding->beamflag) && !mb_beam_check_flag_null(sounding->beamflag)) {
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
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    for (int i = 0; i < soundingdata->num_soundings; i++) {
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
    soundingdata->soundings[i].glz = mb3dsoundings.exaggeration * soundingdata->zscale
                                      * (soundingdata->soundings[i].z - soundingdata->zorigin);
  }

  /* the display positions changed */
  mbv_marks_position(&mb3dsoundings_marks);

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------*/
int mb3dsoundings_pick(int x, int y) {
  /* the screen positions of the current camera */
  mb3dsoundings_screen_positions(mbs_verbose, &mbs_error);

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
    /* only pick displayed soundings: unflagged ones, or in toggle mode flagged
     * ones that are shown (never null/zeroed beams) */
    if (r < rmin
        && (mb_beam_ok(sounding->beamflag)
            || (mb3dsoundings.edit_mode == MBS_EDIT_TOGGLE
                && mb3dsoundings.view_flagged
                && !mb_beam_check_flag_null(sounding->beamflag)
                && (mb3dsoundings.view_secondary || !mb_beam_check_flag_multipick(sounding->beamflag))))) {
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
      else if (!mb_beam_check_flag_null(sounding->beamflag)
               && (mb3dsoundings.view_secondary || !mb_beam_check_flag_multipick(sounding->beamflag))) {
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
    mbv_ui_soundings_bell();
  }

  /* replot the data */
  if (editevent) {
    /* a flag changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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

/*------------------------------------------------------------------------------*/
int mb3dsoundings_info(int x, int y) {
  /* fprintf(stderr,"Called mb3dsoundings_info\n"); */

  /* the screen positions of the current camera */
  mb3dsoundings_screen_positions(mbs_verbose, &mbs_error);

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
    mbv_ui_soundings_bell();
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    mbv_ui_soundings_bell();
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
      if (sounding->ifile == lastsounding->ifile && sounding->iping == lastsounding->iping
          && !mb_beam_check_flag_null(sounding->beamflag)) {
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
    mbv_ui_soundings_bell();
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  } else {
    mbv_ui_soundings_bell();
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  } else {
    mbv_ui_soundings_bell();
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    mbv_ui_soundings_bell();
  }

  /* replot and flush the edit events in the calling application */
  if (neditevent > 0) {
    /* flags changed: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
/* The camera of the 3D soundings view: the orthographic projection matrix and
   the modelview matrix (translation and rotations), in the column-major order of
   OpenGL. Also sets mb3dsoundings.left, right, bottom, top and aspect_ratio. */
int mb3dsoundings_camera(int verbose, double projection[16], double modelview[16], int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* set projection */
  mb3dsoundings.left = -1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.right = 1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.bottom = -1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.top = 1.0 / mb3dsoundings.gl_size;
  mb3dsoundings.aspect_ratio = 1.0 / mb3dsoundings.gl_size;
  mbv_camera_identity(projection);
  mbv_camera_ortho(projection, mb3dsoundings.left, mb3dsoundings.right, mb3dsoundings.bottom, mb3dsoundings.top,
                   MBS_OPENGL_ZMIN2D, MBS_OPENGL_ZMAX2D);

  /* set up translations */
  mbv_camera_identity(modelview);
  mbv_camera_translate(modelview, mb3dsoundings.gl_offset_x, mb3dsoundings.gl_offset_y, MBS_OPENGL_ZMIN2D);
  mbv_camera_rotate(modelview, (float)(mb3dsoundings.elevation - 90.0), 1.0, 0.0, 0.0);
  mbv_camera_rotate(modelview, (float)(mb3dsoundings.azimuth), 0.0, 0.0, 1.0);

  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:                   %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:                  %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* What changed in the soundings since a user last looked (the top of this
   file): three counters that only rise. */
int mb3dsoundings_getmarks(int verbose, uint64_t *position, uint64_t *color, uint64_t *mask, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
  }

  *position = mbv_marks_getposition(&mb3dsoundings_marks);
  *color = mbv_marks_getcolor(&mb3dsoundings_marks);
  *mask = mbv_marks_getmask(&mb3dsoundings_marks);
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       position:         %llu\n", (unsigned long long)*position);
    fprintf(stderr, "dbg2       color:            %llu\n", (unsigned long long)*color);
    fprintf(stderr, "dbg2       mask:             %llu\n", (unsigned long long)*mask);
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* the number of soundings shown, 0 when there are none */
int mb3dsoundings_getcount(int verbose, int *num_soundings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
  }

  const struct mb3dsoundings_struct *soundingdata = (const struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  *num_soundings = 0;
  if (soundingdata != NULL && soundingdata->soundings != NULL && soundingdata->num_soundings > 0)
    *num_soundings = soundingdata->num_soundings;
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       num_soundings:    %d\n", *num_soundings);
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* one colour byte, rounded to nearest as OpenGL rounds a float colour into an
   8-bit framebuffer */
static unsigned char mb3dsoundings_color_byte(float c) {
  if (!(c > 0.0f))
    return (0);
  if (c >= 1.0f)
    return (255);
  return ((unsigned char)lroundf(c * 255.0f));
}

/* The colour of every sounding as the drawing gives it, 4 bytes (RGBA) per
   sounding in the order of the array, alpha 0 (and black) for a sounding that
   is not drawn. nalloc is the number of soundings the caller's buffer holds;
   num_soundings returns the number of soundings. Nothing is kept here: every
   call computes the colours again, as the drawing does every frame. */
int mb3dsoundings_colors(int verbose, int nalloc, unsigned char *rgba, int *num_soundings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       nalloc:           %d\n", nalloc);
    fprintf(stderr, "dbg2       rgba:             %p\n", (void *)rgba);
  }

  const struct mb3dsoundings_struct *soundingdata = (const struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *num_soundings = 0;
  if (soundingdata != NULL && soundingdata->soundings != NULL && soundingdata->num_soundings > 0)
    *num_soundings = soundingdata->num_soundings;

  if (*num_soundings > 0 && (rgba == NULL || nalloc < *num_soundings)) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  } else if (*num_soundings > 0) {
    /* the ranges the colour modes stretch over */
    double zmin = 0.0;
    double zmax = 0.0;
    double ampmin = 0.0;
    double ampmax = 0.0;
    if (mb3dsoundings.view_color == MBS_VIEW_COLOR_SOUNDING)
      mb3dsoundings_depth_range(soundingdata, &zmin, &zmax);
    else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP)
      mb3dsoundings_amplitude_range(soundingdata, &ampmin, &ampmax);

    for (int i = 0; i < *num_soundings; i++) {
      const struct mb3dsoundings_sounding_struct *sounding = &(soundingdata->soundings[i]);
      float r = 0.0f;
      float g = 0.0f;
      float b = 0.0f;
      bool shown = false;
      if (mb3dsoundings.view_color == MBS_VIEW_COLOR_FLAG) {
        shown = mb3dsoundings_flag_color(sounding, &r, &g, &b);
      } else if (mb3dsoundings_shown(sounding)) {
        shown = true;
        if (mb3dsoundings.view_color == MBS_VIEW_COLOR_TOPO) {
          r = sounding->r;
          g = sounding->g;
          b = sounding->b;
        } else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_SOUNDING) {
          mb3dsoundings_depth_color(sounding->z, zmin, zmax, &r, &g, &b);
        } else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP) {
          mb3dsoundings_amplitude_color(sounding->a, ampmin, ampmax, &r, &g, &b);
        } else {
          /* no colour mode the drawing knows: it draws nothing */
          shown = false;
        }
      }
      unsigned char *out = &rgba[4 * (size_t)i];
      if (shown) {
        out[0] = mb3dsoundings_color_byte(r);
        out[1] = mb3dsoundings_color_byte(g);
        out[2] = mb3dsoundings_color_byte(b);
        out[3] = 255;
      } else {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
      }
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       num_soundings:    %d\n", *num_soundings);
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* The display positions glx/gly/glz of every sounding, 3 floats per sounding
   in the order of the array, copied as the drawing uses them (never computed
   again from x/y/z). nalloc and num_soundings as for mb3dsoundings_colors(). */
int mb3dsoundings_positions(int verbose, int nalloc, float *xyz, int *num_soundings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       nalloc:           %d\n", nalloc);
    fprintf(stderr, "dbg2       xyz:              %p\n", (void *)xyz);
  }

  const struct mb3dsoundings_struct *soundingdata = (const struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *num_soundings = 0;
  if (soundingdata != NULL && soundingdata->soundings != NULL && soundingdata->num_soundings > 0)
    *num_soundings = soundingdata->num_soundings;

  if (*num_soundings > 0 && (xyz == NULL || nalloc < *num_soundings)) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  } else {
    for (int i = 0; i < *num_soundings; i++) {
      const struct mb3dsoundings_sounding_struct *sounding = &(soundingdata->soundings[i]);
      xyz[3 * (size_t)i] = sounding->glx;
      xyz[3 * (size_t)i + 1] = sounding->gly;
      xyz[3 * (size_t)i + 2] = sounding->glz;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       num_soundings:    %d\n", *num_soundings);
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* Fill the screen positions winx/winy of every sounding for the current
   camera, as the drawing fills them with gluProject (window y up, truncated
   to whole pixels), unless they already belong to this camera and these
   soundings - as they do after an OpenGL frame. The pick, erase, grab and
   info functions call this first, so they also work when nothing draws the
   soundings with OpenGL. */
int mb3dsoundings_screen_positions(int verbose, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
  }

  struct mb3dsoundings_struct *soundingdata = (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (soundingdata != NULL && soundingdata->soundings != NULL && soundingdata->num_soundings > 0
      && mb3dsoundings.gl_width != 0 && mb3dsoundings.gl_height != 0 && !mb3dsoundings_screen_current()) {
    double projection[16];
    double modelview[16];
    mb3dsoundings_camera(verbose, projection, modelview, error);
    const int viewport[4] = {0, 0, (int)mb3dsoundings.gl_width, (int)mb3dsoundings.gl_height};
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding = &(soundingdata->soundings[i]);
      double xx;
      double yy;
      double zz;
      if (mbv_camera_project(modelview, projection, viewport, (double)(sounding->glx), (double)(sounding->gly),
                             (double)(sounding->glz), &xx, &yy, &zz)) {
        sounding->winx = (int)xx;
        sounding->winy = (int)yy;
      }
    }
    mb3dsoundings_screen_stamp();
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
/* For the calling program, after it changed the colours of the soundings it
   handed over (their r/g/b, from its own colour table) without going through
   this window: raise the colour mark (the top of this file). */
int mb3dsoundings_colors_changed(int verbose, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
  }

  mbv_marks_color(&mb3dsoundings_marks);
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*------------------------------------------------------------------------------*/
int mb3dsoundings_plot(int verbose, int *error) {
  if (mbs_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
  }

  /* the renderer may draw later by itself */
  if (mbv_render_defer_draw(MBV_RENDER_SOUNDINGS, 0, 0))
    return (mbs_status);

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
  mbv_render_make_current(MBV_RENDER_SOUNDINGS, 0);

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
    float glzmin = mb3dsoundings.exaggeration * soundingdata->zscale * soundingdata->zmin;
    float glzmax = mb3dsoundings.exaggeration * soundingdata->zscale * soundingdata->zmax;
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

      /* plot segment only if soundings are from the same ping and profile */
      if (sounding2->ifile == sounding->ifile && sounding->iping == sounding2->iping && sounding->iprofile == sounding2->iprofile) {
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

      /* plot unflagged sounding, and flagged sounding if requested, in the
         colour of its flag (mb3dsoundings_flag_color) */
      float r, g, b;
      if (mb3dsoundings_flag_color(sounding, &r, &g, &b)) {
        glColor3f(r, g, b);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
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
      if (mb3dsoundings_shown(sounding)) {
        glColor3f(sounding->r, sounding->g, sounding->b);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      }
    }
    glEnd();
  }

  /* Color by soundings - stretch the topography color scale over the
     range of soundings currently displayed in the 3D Soundings view
     rather than the associated Survey Viewer's min/max */
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_SOUNDING) {

    /* get the topography min max over the soundings currently viewed
       (mb3dsoundings_depth_range) */
    double zmin;
    double zmax;
    mb3dsoundings_depth_range(soundingdata, &zmin, &zmax);

    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);

      /* plot sounding */
      if (mb3dsoundings_shown(sounding)) {
        float r, g, b;
        mb3dsoundings_depth_color(sounding->z, zmin, zmax, &r, &g, &b);
        glColor3f(r, g, b);
        glVertex3f(sounding->glx, sounding->gly, sounding->glz);
      }
    }
    glEnd();
  }

  /* Color by amplitude */
  else if (mb3dsoundings.view_color == MBS_VIEW_COLOR_AMP) {

    /* get the amplitude min max over the soundings drawn
       (mb3dsoundings_amplitude_range) */
    double ampmin;
    double ampmax;
    mb3dsoundings_amplitude_range(soundingdata, &ampmin, &ampmax);

    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);

      /* plot sounding */
      if (mb3dsoundings_shown(sounding)) {
        float r, g, b;
        mb3dsoundings_amplitude_color(sounding->a, ampmin, ampmax, &r, &g, &b);
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

  /* the screen positions belong to this camera and these soundings, so the
     pick, erase, grab and info functions use them as they are */
  mb3dsoundings_screen_stamp();

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
  mbv_render_present(MBV_RENDER_SOUNDINGS, 0);

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
/*------------------------------------------------------------------------------*/
/* Soundings window logic moved from mb3dsoundings_callbacks.c (Phase 5a): the
   callbacks without Motif calls, unchanged, and the toolkit-free parts of the
   others. */

/* the mouse mode label text (mb3dsoundings_updatelabelmousemode) */
static mb_path value_text;

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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_A(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_A\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(1, 10);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_B(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_B\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(1, 2);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_C(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_C\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(4, 10);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_D(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_D\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(4, 2);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_E(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_E\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(8, 10);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_flagsparsevoxels_F(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_flagsparsevoxels_F\n"); */

  /* send bias parameters to calling program to be applied */
  if (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify != NULL)
    (mb3dsoundings.mb3dsoundings_flagsparsevoxels_notify)(8, 2);

  /* the program flagged soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsblack(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsblack\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLACK);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsred(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsred\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_RED);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsyellow(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsyellow\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_YELLOW);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsgreen(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsgreen\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_GREEN);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsbluegreen(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsbluegreen\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLUEGREEN);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingsblue(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingsblue\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_BLUE);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
void do_mb3dsdg_action_colorsoundingspurple(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;  // Unused parameter
  (void)client_data;  // Unused parameter
  (void)call_data;  // Unused parameter
  // XmAnyCallbackStruct *acs = (XmAnyCallbackStruct *)call_data;

  /* fprintf(stderr,"Called do_mb3dsdg_action_colorsoundingspurple\n"); */

  /* notify calling program to color current selected unflagged soundings */
  if (mb3dsoundings.mb3dsoundings_colorsoundings_notify != NULL)
    (mb3dsoundings.mb3dsoundings_colorsoundings_notify)(MBV_COLOR_PURPLE);

  /* the program coloured soundings: raise the mark before the replot (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
/*------------------------------------------------------------------------------*/
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

    /* the program rewrote every sounding's z, which colours them by depth: raise the mark before the replot (top of this file) */
    mbv_marks_color(&mb3dsoundings_marks);

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
  mbv_ui_soundings_mousemode(value_text);

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
/* the bias values (irollbias ... isnell) changed: send them to the calling
   program, rescale and replot (the bias sliders) */
void mb3dsoundings_bias_changed(void) {
  /* send bias parameters to calling program */
  if (mb3dsoundings.mb3dsoundings_bias_notify != NULL)
    (mb3dsoundings.mb3dsoundings_bias_notify)(
        0.01 * ((double)mb3dsoundings.irollbias), 0.01 * ((double)mb3dsoundings.ipitchbias),
        0.01 * ((double)mb3dsoundings.iheadingbias), 0.01 * ((double)mb3dsoundings.itimelag),
      0.0001 * ((double)mb3dsoundings.isnell));

  /* the program rewrote every sounding's z, which colours them by depth: raise
     the mark (top of this file); the rescaling raises the positions */
  mbv_marks_color(&mb3dsoundings_marks);

  /* rescale data to the gl coordinates */
  mb3dsoundings_scale(mbs_verbose, &mbs_error);
  mb3dsoundings_setzscale(mbs_verbose, &mbs_error);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
/* a view toggle of the soundings window changed (MBS_VIEWTOGGLE_*) */
void mb3dsoundings_setviewtoggle(int toggle, bool state) {
  if (toggle == MBS_VIEWTOGGLE_FLAGGED)
    mb3dsoundings.view_flagged = state;
  else if (toggle == MBS_VIEWTOGGLE_SECONDARY)
    mb3dsoundings.view_secondary = state;
  else if (toggle == MBS_VIEWTOGGLE_BOUNDINGBOX)
    mb3dsoundings.view_boundingbox = state;
  else if (toggle == MBS_VIEWTOGGLE_SCALEWITHFLAGGED)
    mb3dsoundings.view_scalewithflagged = state;

  /* which soundings are drawn changed (top of this file); the scaling raises
     the positions */
  if (toggle == MBS_VIEWTOGGLE_FLAGGED || toggle == MBS_VIEWTOGGLE_SECONDARY)
    mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  if (toggle == MBS_VIEWTOGGLE_SCALEWITHFLAGGED)
    mb3dsoundings_setzscale(mbs_verbose, &mbs_error);
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
/* how profiles are drawn (MBS_VIEW_PROFILES_*) */
void mb3dsoundings_setprofiles(int mode) {
  mb3dsoundings.view_profiles = mode;

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
/* how soundings are coloured (MBS_VIEW_COLOR_*) */
void mb3dsoundings_setcolor(int mode) {
  mb3dsoundings.view_color = mode;

  /* every colour changed (top of this file) */
  mbv_marks_color(&mb3dsoundings_marks);

  /* replot the data */
  mb3dsoundings_plot(mbs_verbose, &mbs_error);
}
/*------------------------------------------------------------------------------*/
/* what the middle and right mouse buttons do (MBS_MOUSE_*) */
void mb3dsoundings_setmousemode(int mode) {
  mb3dsoundings.mouse_mode = mode;

  /* set mouse mode label */
  mb3dsoundings_updatelabelmousemode();
}
/*------------------------------------------------------------------------------*/
/* the soundings to show: print their statistics, set the data pointer and
   scale it (the data part of mb3dsoundings_open) */
int mb3dsoundings_setdata(int verbose, struct mb3dsoundings_struct *soundingdata, int *error) {
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

  /* new soundings, perhaps in the same array as before: raise every mark (top
     of this file); the screen positions belong to no camera yet */
  mbv_marks_everything(&mb3dsoundings_marks);
  mb3dsoundings_screen.valid = false;

  mb3dsoundings_scale(verbose, error);

  /* reset info flag */
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.last_sounding_edited = 0;

  return (mbs_status);
}
/*------------------------------------------------------------------------------*/
/* reset the soundings window state, keeping the bias values (the reset part
   of mb3dsoundings_end) */
void mb3dsoundings_reset_keepbias(void) {
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
}
/*------------------------------------------------------------------------------*/
/* the soundings window was hidden: tell the calling program */
void mb3dsoundings_dismissed(void) {
  mb3dsoundings.init = MBS_WINDOW_HIDDEN;

  /* the calling program releases the soundings in the notify: users of them
     stop reading (top of this file) */
  mbv_marks_everything(&mb3dsoundings_marks);
  mb3dsoundings_screen.valid = false;

  (mb3dsoundings.mb3dsoundings_dismiss_notify)();
}
