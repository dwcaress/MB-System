/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_bridge.h  11/7/2026
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
 * Between the C user interface functions of mbgrdviz-qt (mbgrdviz_qt_ui.c)
 * and its Qt window (mbgrdviz_qt_window.cc). Plain C types only.
 */

#ifndef MBGRDVIZ_QT_BRIDGE_H
#define MBGRDVIZ_QT_BRIDGE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void mbgrdviz_qt_bell(void);
void mbgrdviz_qt_realtime_path(const char *path);
/* the main window menu items: open primary grid, the other open items, save
   site, save route */
void mbgrdviz_qt_sensitivity(bool open_primary, bool open_other, bool save_site, bool save_route);
/* a file dialog for mode (MBGRDVIZ_OPENGRID and the other MBGRDVIZ_ modes) */
void mbgrdviz_qt_file_dialog(int mode, size_t instance, const char *title, const char *pattern);
/* start the tool argv[0] with the arguments argv (NULL-terminated) in the
   background; the Qt version of the tool where one exists */
void mbgrdviz_qt_launch(const char *const argv[]);

/* the survey generation dialog: the values of its fields (the survey state of
   mbgrdviz_core.c; mode, platform, direction and color are list positions) */
struct mbgrdviz_qt_survey {
  int mode;
  int platform;
  int interleaving;
  int direction;
  bool crosslines_last;
  int crosslines;
  int linespacing;
  int swathwidth;
  int depth;
  int altitude;
  int color;
  char name[1024];
};

/* implemented in mbgrdviz_qt_ui.c */
void mbgrdviz_qt_survey_get(struct mbgrdviz_qt_survey *survey);
void mbgrdviz_qt_survey_set(const struct mbgrdviz_qt_survey *survey);
void mbgrdviz_qt_survey_generate(void);
void mbgrdviz_qt_survey_dismiss(void);

/* implemented in mbgrdviz_qt_dialogs.cc */
void mbgrdviz_qt_survey_dialog(void);
void mbgrdviz_qt_survey_sensitivity(bool linespacing, bool platform, bool swathwidth, bool altitude, bool depth);
void mbgrdviz_qt_survey_info(const char *info_text);
void mbgrdviz_qt_survey_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* MBGRDVIZ_QT_BRIDGE_H */
