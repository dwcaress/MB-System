/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_bridge.h  11/8/2026
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
 * Between the C half of mbeditviz-qt (mbeditviz_qt_ui.c: the user interface
 * functions that mbeditviz_core.c and mbeditviz_prog.c call) and its Qt
 * controller (mbeditviz_qt_controller.cc). Plain C types only.
 */

#ifndef MBEDITVIZ_QT_BRIDGE_H
#define MBEDITVIZ_QT_BRIDGE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- implemented by the C half (mbeditviz_qt_ui.c) ---- */

/* the window's buttons and menus */
void mbeditviz_qt_open(const char *path, int format);
int mbeditviz_qt_format_of(const char *path); /* the MBIO format of a file name, -1 if unknown */
void mbeditviz_qt_view_all(void);
void mbeditviz_qt_view_selected(void);
void mbeditviz_qt_delete_selected(void);
void mbeditviz_qt_regrid(void);
void mbeditviz_qt_quit(void);
void mbeditviz_qt_set_edit_mode(bool edit);
bool mbeditviz_qt_edit_mode(void);
/* the file list timer (the Motif work function) */
void mbeditviz_qt_tick(void);
/* the error function passed to mbeditviz_init(), as the Motif do_error_dialog */
int mbeditviz_qt_error_dialog(char *s1, char *s2, char *s3);

/* the grid parameters dialog */
struct mbeditviz_qt_grid {
	int cellsize;      /* thousandths of a metre */
	int algorithm;     /* MBEV_GRID_ALGORITHM_*: 0 simple mean, 1 footprint, 2 shoal bias */
	int interpolation; /* grid cells */
};
void mbeditviz_qt_grid_get(struct mbeditviz_qt_grid *grid);
/* set the cell size, algorithm and interpolation and recompute the dimensions */
void mbeditviz_qt_grid_set(const struct mbeditviz_qt_grid *grid);
void mbeditviz_qt_grid_texts(char *current, char *implied, size_t size);
void mbeditviz_qt_grid_apply(void);

/* ---- implemented by the Qt controller (mbeditviz_qt_controller.cc) ---- */

void mbeditviz_qt_status(const char *bxstring);
void mbeditviz_qt_sensitivity(bool open, bool files);
/* the file list: number of rows; rebuild (keeping the selection when the
   number of rows does not change); the selected rows, 1-based as Motif list
   positions, at most max */
int mbeditviz_qt_list_count(void);
void mbeditviz_qt_list_begin(void);
void mbeditviz_qt_list_add(const char *label);
void mbeditviz_qt_list_end(void);
int mbeditviz_qt_list_selection(int *positions, int max);
void mbeditviz_qt_message(const char *message, bool on);
void mbeditviz_qt_error(const char *s1, const char *s2, const char *s3);
void mbeditviz_qt_gridparameters(void);
void mbeditviz_qt_bell(void);

#ifdef __cplusplus
}
#endif

#endif /* MBEDITVIZ_QT_BRIDGE_H */
