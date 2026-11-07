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
/* an action of a later step of the port */
void mbgrdviz_qt_not_yet(size_t instance, const char *what);

#ifdef __cplusplus
}
#endif

#endif /* MBGRDVIZ_QT_BRIDGE_H */
