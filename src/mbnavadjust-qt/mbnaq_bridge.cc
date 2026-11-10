/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_bridge.cc  9/11/2026
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
 * The functions that mbnavadjust_prog.c expects from its user interface. In
 * the Motif program they are in mbnavadjust_callbacks.c and mb_xgraphics.c.
 * Here they forward to the mbnaq::Session and to a DrawSurface. The four list
 * filters decide which crossings, sections and ties the current list shows and
 * are used by the next/previous logic, so they follow the Motif code exactly.
 */

#include <cstddef>
#include <cstdio>

extern "C" {
#include <X11/Intrinsic.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
}

#include "mbnaq_core.h"

using mbnaq::DrawSurface;
using mbnaq::Session;

extern "C" {

int do_visualize_dismiss_notify(size_t instance);
void do_visualize_sensitivity(void);
void do_update_visualization_status(void);
void do_pickroute_notify(size_t instance);
void do_picknav_notify(size_t instance);
void do_mbnavadjust_addcrossing(Widget w, XtPointer client_data, XtPointer call_data);
int do_info_add(char *info, int timetag);

void do_update_status(void) {
  if (Session::current() != nullptr)
    Session::current()->updateStatus();
}

void do_update_modelplot_status(void) {
  if (Session::current() != nullptr)
    Session::current()->updateModelPlotStatus();
}

/* The map view arrives with mbview-qt. */
void do_update_visualization_status(void) {}
int do_visualize_dismiss_notify(size_t) { return MB_SUCCESS; }
void do_visualize_sensitivity(void) {}
void do_pickroute_notify(size_t) {}
void do_picknav_notify(size_t) {}
void do_mbnavadjust_addcrossing(Widget, XtPointer, XtPointer) {}

void do_naverr_init(int mode) {
  if (Session::current() != nullptr)
    Session::current()->initCrossingWindow(mode);
}

void do_naverr_update(void) {
  if (Session::current() != nullptr)
    Session::current()->updateCrossing();
}

int do_message_on(char *message) {
  if (Session::current() != nullptr)
    Session::current()->notifyMessageOn(message);
  return MB_SUCCESS;
}

int do_message_update(char *message) {
  if (Session::current() != nullptr)
    Session::current()->notifyMessageUpdate(message);
  return MB_SUCCESS;
}

int do_message_off(void) {
  if (Session::current() != nullptr)
    Session::current()->notifyMessageOff();
  return MB_SUCCESS;
}

int do_info_add(char *info, int timetag) {
  if (Session::current() != nullptr)
    Session::current()->addInfo(info, timetag != 0);
  return MB_SUCCESS;
}

int do_error_dialog(char *s1, char *s2, char *s3) {
  if (Session::current() != nullptr)
    Session::current()->notifyError(s1, s2, s3);
  return MB_SUCCESS;
}

int do_check_crossing_listok(int icrossing) {
  bool use_status = false;
  struct mbna_crossing *crossing = &(project.crossings[icrossing]);

  if (icrossing == mbna_crossing_select) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS && crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS && crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS && crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS && crossing->truecrossing) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_TIES && crossing->num_ties > 0) {
    use_status = true;
  } else if ((mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST ||
              mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) &&
             crossing->num_ties > 0) {
    for (int itie = 0; itie < crossing->num_ties; itie++) {
      struct mbna_tie *tie = &crossing->ties[itie];
      if (tie->sigma_m >= project.tiessortedthreshold)
        use_status = true;
    }
  }

  if (use_status) {
    const int survey1 = project.files[crossing->file_id_1].survey;
    const int survey2 = project.files[crossing->file_id_2].survey;
    use_status =
        (mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
        (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == survey1 && mbna_survey_select == survey2) ||
        (mbna_view_mode == MBNA_VIEW_MODE_BLOCK && mbna_survey_select1 == survey1 && mbna_survey_select2 == survey2) ||
        (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
         mbna_file_select == crossing->file_id_2) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && (mbna_survey_select == survey1 || mbna_survey_select == survey2)) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
         (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
         mbna_section_select == crossing->section_1) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
         mbna_section_select == crossing->section_2);
  }
  return use_status;
}

int do_check_section_listok(int ifile, int isection) {
  bool use_status = false;

  if (ifile == mbna_file_select && isection == mbna_section_select) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS) {
    use_status = true;
  } else if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
    if (project.files[ifile].sections[isection].status == MBNA_CROSSING_STATUS_SET)
      use_status = true;
  }

  if (use_status) {
    /* The Motif code compares the selected file with the survey in the
       with-survey mode, and this keeps that behaviour for now. */
    use_status =
        (mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
        (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == project.files[ifile].survey) ||
        (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_file_select == project.files[ifile].survey) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
        (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection);
  }
  return use_status;
}

static bool nav_in_view(struct mbna_file *file, int ifile, int isection) {
  return (mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
         (mbna_view_mode == MBNA_VIEW_MODE_SURVEY && mbna_survey_select == file->survey) ||
         (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
          (mbna_survey_select1 == file->survey || mbna_survey_select2 == file->survey)) ||
         (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == ifile) ||
         (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY && mbna_survey_select == file->survey) ||
         (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE && mbna_file_select == ifile) ||
         (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == ifile && mbna_section_select == isection);
}

int do_check_globaltie_listok(int ifile, int isection) {
  struct mbna_file *file = &(project.files[ifile]);
  return file->sections[isection].status == MBNA_CROSSING_STATUS_SET && nav_in_view(file, ifile, isection);
}

int do_check_nav_active(int ifile, int isection) {
  return nav_in_view(&(project.files[ifile]), ifile, isection);
}

void xg_drawline(void *xgid, int x1, int y1, int x2, int y2, unsigned int pixel, int style) {
  if (xgid != nullptr) {
    static_cast<DrawSurface *>(xgid)->lines++;
    static_cast<DrawSurface *>(xgid)->line(x1, y1, x2, y2, pixel, style);
  }
}

void xg_drawrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style) {
  if (xgid != nullptr) {
    static_cast<DrawSurface *>(xgid)->rectangles++;
    static_cast<DrawSurface *>(xgid)->rectangle(x, y, width, height, pixel, style);
  }
}

void xg_fillrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style) {
  if (xgid != nullptr) {
    static_cast<DrawSurface *>(xgid)->fills++;
    static_cast<DrawSurface *>(xgid)->fillRectangle(x, y, width, height, pixel, style);
  }
}

void xg_drawstring(void *xgid, int x, int y, char *string, unsigned int pixel, int style) {
  if (xgid != nullptr) {
    static_cast<DrawSurface *>(xgid)->texts++;
    static_cast<DrawSurface *>(xgid)->text(x, y, string, pixel, style);
  }
}

void xg_justify(void *xgid, char *string, int *width, int *ascent, int *descent) {
  *width = 0;
  *ascent = 0;
  *descent = 0;
  if (xgid != nullptr)
    static_cast<DrawSurface *>(xgid)->measure(string, width, ascent, descent);
}

void xg_setclip(void *xgid, int x, int y, int width, int height) {
  if (xgid != nullptr)
    static_cast<DrawSurface *>(xgid)->clip(x, y, width, height);
}

}  // extern "C"
