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

  return (mbs_status);
}

/*------------------------------------------------------------------------------*/
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
    /* replot the data */
    mb3dsoundings_plot(mbs_verbose, &mbs_error);
    mb3dsoundings_updatestatus();

    /* flush the edit events in the calling application */
    (mb3dsoundings.mb3dsoundings_edit_notify)(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  return (mbs_status);
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
              && (!mb_beam_check_flag_multipick(sounding->beamflag)
                  || mb3dsoundings.view_secondary))) {
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

    /* get the topography min max over the soundings currently viewed,
       regardless of beamflag state if flagged soundings are being shown,
       else over the unflagged soundings only */
    double zmin = 0.0;
    double zmax = 0.0;
    bool first = true;
    for (int i = 0; i < soundingdata->num_soundings; i++) {
      struct mb3dsoundings_sounding_struct *sounding =
        (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[i]);
      if (mb3dsoundings.view_flagged || mb_beam_ok(sounding->beamflag)) {
        if (first) {
          first = false;
          zmin = sounding->z;
          zmax = sounding->z;
        } else {
          zmin = MIN(zmin, sounding->z);
          zmax = MAX(zmax, sounding->z);
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
              && (!mb_beam_check_flag_multipick(sounding->beamflag)
                  || mb3dsoundings.view_secondary))) {
        float r, g, b;
        mbview_getcolor(sounding->z, zmin, zmax, MBV_COLORTABLE_NORMAL,
                        colortable_haxby_red[0], colortable_haxby_green[0], colortable_haxby_blue[0],
                        colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
                        colortable_haxby_blue[MBV_NUM_COLORS - 1],
                        colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue,
                        &r, &g, &b);
        glColor3f(r, g, b);
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
              && (!mb_beam_check_flag_multipick(sounding->beamflag)
                  || mb3dsoundings.view_secondary))) {
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
              && (!mb_beam_check_flag_multipick(sounding->beamflag)
                  || mb3dsoundings.view_secondary))) {
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

