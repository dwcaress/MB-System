/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_ui.c  11/8/2026
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
 * The user interface functions of mbeditviz for Qt: the five that
 * mbeditviz_core.c and mbeditviz_prog.c call (do_mbeditviz_update_gui,
 * do_mbeditviz_message_on, do_mbeditviz_message_off,
 * do_mbeditviz_gridparameters, mbeditviz_ui_bell), the file list and error
 * functions passed to mbeditviz_init(), and the actions of the Qt window.
 * The error function has a name of its own, as other Qt tools that may share
 * a program with mbeditviz-qt define do_error_dialog too.
 * They follow the Motif versions in mbeditviz_callbacks.c; the toolkit-free
 * parts are shared in mbeditviz_core.c.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
#include "mbeditviz_qt_bridge.h"

/*------------------------------------------------------------------------------*/
void do_mbeditviz_update_gui(void) {
	/* set status text */
	char string[MB_PATH_MAXLINE];
	mbeditviz_status_text(string, sizeof(string));
	mbeditviz_qt_status(string);

	/* build available file list */
	do_mbeditviz_update_filelist();

	/* set sensitivity */
	const bool nogrid = mbev_grid.status == MBEV_GRID_NONE;
	mbeditviz_qt_sensitivity(nogrid, mbev_num_files > 0 && nogrid);
}

void do_mbeditviz_update_filelist(void) {
	/* check to see if anything has changed */
	bool update_filelist = mbeditviz_qt_list_count() != mbev_num_files;
	if (mbeditviz_filelist_check())
		update_filelist = true;

	/* only update the filelist if necessary */
	if (update_filelist) {
		mbeditviz_qt_list_begin();
		char string[MB_PATH_MAXLINE + 40];
		for (int i = 0; i < mbev_num_files; i++) {
			mbeditviz_filelist_item(i, string, sizeof(string));
			mbeditviz_qt_list_add(string);
		}
		mbeditviz_qt_list_end();
	}
}

int do_mbeditviz_message_on(char *message) {
	mbev_message_on = true;
	mbeditviz_qt_message(message, true);
	return (1);
}

int do_mbeditviz_message_off(void) {
	mbev_message_on = false;
	mbeditviz_qt_message(NULL, false);
	return (1);
}

int mbeditviz_qt_error_dialog(char *s1, char *s2, char *s3) {
	mbeditviz_qt_error(s1, s2, s3);
	return (1);
}

void do_mbeditviz_gridparameters(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;
	(void)client_data;
	(void)call_data;

	/* get calculated grid parameters */
	mbeditviz_get_grid_bounds();
	mbeditviz_qt_gridparameters();
}

void mbeditviz_ui_bell(void) { mbeditviz_qt_bell(); }

/*------------------------------------------------------------------------------*/
/* the window's actions */
void mbeditviz_qt_open(const char *path, int format) {
	mb_path file;
	snprintf(file, sizeof(file), "%s", path);
	mbev_status = do_mbeditviz_opendata(file, format);
}

int mbeditviz_qt_format_of(const char *path) {
	mb_path file;
	snprintf(file, sizeof(file), "%s", path);
	int form = -1;
	if (mbeditviz_get_format(file, &form) != MB_SUCCESS)
		return (-1);
	return (form);
}

void mbeditviz_qt_view_all(void) { do_mbeditviz_viewall(NULL, NULL, NULL); }

void mbeditviz_qt_view_selected(void) {
	/* destroy the views and the old grid, then read the selection, as in Motif */
	do_mbeditviz_viewselected_prepare();
	int *positions = (int *)malloc(sizeof(int) * (size_t)(mbev_num_files > 0 ? mbev_num_files : 1));
	if (positions == NULL) {
		mbeditviz_ui_bell();
		return;
	}
	const int count = mbeditviz_qt_list_selection(positions, mbev_num_files);
	do_mbeditviz_viewselected_files(NULL, NULL, NULL, count, positions);
	free(positions);
}

void mbeditviz_qt_delete_selected(void) {
	/* as the Motif do_mbeditviz_deleteselected */
	int *positions = (int *)malloc(sizeof(int) * (size_t)(mbev_num_files > 0 ? mbev_num_files : 1));
	if (positions == NULL) {
		mbeditviz_ui_bell();
		return;
	}
	const int count = mbeditviz_qt_list_selection(positions, mbev_num_files);

	/* delete the selected files */
	for (int i = count - 1; i >= 0; i--)
		mbeditviz_delete_file(positions[i] - 1);
	free(positions);

	/* reset the gui */
	do_mbeditviz_update_gui();
}

void mbeditviz_qt_regrid(void) { do_mbeditviz_regrid(NULL, NULL, NULL); }

void mbeditviz_qt_quit(void) { do_mbeditviz_quit(NULL, NULL, NULL); }

void mbeditviz_qt_set_edit_mode(bool edit) {
	mbev_mode_output = edit ? MBEV_OUTPUT_MODE_EDIT : MBEV_OUTPUT_MODE_BROWSE;
}

bool mbeditviz_qt_edit_mode(void) { return mbev_mode_output == MBEV_OUTPUT_MODE_EDIT; }

void mbeditviz_qt_tick(void) {
	/* reset filelist, as the Motif work function */
	if (mbev_num_files > 0)
		do_mbeditviz_update_filelist();
}

void mbeditviz_qt_grid_get(struct mbeditviz_qt_grid *grid) {
	grid->cellsize = (int)(1000 * mbev_grid_cellsize);
	grid->algorithm = (int)mbev_grid_algorithm;
	grid->interpolation = mbev_grid_interpolation;
}

void mbeditviz_qt_grid_set(const struct mbeditviz_qt_grid *grid) {
	/* a zero cell size would make the grid dimensions infinite */
	mbev_grid_cellsize = 0.001 * MAX(grid->cellsize, 1);
	if (grid->algorithm == MBEV_GRID_ALGORITHM_SIMPLEMEAN)
		mbev_grid_algorithm = MBEV_GRID_ALGORITHM_SIMPLEMEAN;
	else if (grid->algorithm == MBEV_GRID_ALGORITHM_FOOTPRINT)
		mbev_grid_algorithm = MBEV_GRID_ALGORITHM_FOOTPRINT;
	else
		mbev_grid_algorithm = MBEV_GRID_ALGORITHM_SHOALBIAS;
	mbev_grid_interpolation = grid->interpolation;

	/* get updated grid dimensions */
	mbeditviz_set_grid_dimensions();
}

void mbeditviz_qt_grid_texts(char *current, char *implied, size_t size) {
	mbeditviz_gridparameters_text(current, size);
	mbeditviz_gridparameters_implied(implied, size);
}

void mbeditviz_qt_grid_apply(void) { do_mbeditviz_updategrid(NULL, NULL, NULL); }
