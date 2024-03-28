/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.h	10/14/2009
 *
 *    Copyright (c) 2009-2024 by
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
 * MBEDIT is an interactive beam editor for multibeam bathymetry data.
 * It can work with any data format supported by the MBIO library.
 * This version uses the MOTIF toolkit and has been developed using
 * the Builder Xsessory package by ICS.  This file contains
 * contains function prototypes and was added in 2009.
 *
 * Author:	D. W. Caress
 * Date:	October 14, 2009
 */

#ifndef MBEDIT_H
#define MBEDIT_H

#include "mbedit_prog.h"

void do_mbedit_init(int argc, char **argv);
void do_parse_datalist(char *file, int form);
void do_editlistselection(Widget w, XtPointer client_data, XtPointer call_data);
void do_filelist_remove(Widget w, XtPointer client_data, XtPointer call_data);
void do_load_specific_file(int i_file);
int do_setup_data(void);
void do_build_filelist(void);
void do_get_filters(void);
void do_file_selection_cancel(Widget w, XtPointer client_data, XtPointer call_data);
void do_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_toggle(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_pick(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_erase(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_restore(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_grab(Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_info(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_y(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_x(Widget w, XtPointer client_data, XtPointer call_data);
int do_reset_scale_x(int pwidth, int maxx, int xntrvl, int yntrvl);
void do_output_edit(Widget w, XtPointer client_data, XtPointer call_data);
void do_output_browse(Widget w, XtPointer client_data, XtPointer call_data);
void do_output_edit_filelist(Widget w, XtPointer client_data, XtPointer call_data);
void do_output_browse_filelist(Widget w, XtPointer client_data, XtPointer call_data);
void do_x_interval(Widget w, XtPointer client_data, XtPointer call_data);
void do_y_interval(Widget w, XtPointer client_data, XtPointer call_data);
void do_load(int save_mode);
void do_load_ok(Widget w, XtPointer client_data, XtPointer call_data);
void do_load_ok_with_save(Widget w, XtPointer client_data, XtPointer call_data);
void do_load_check(Widget w, XtPointer client_data, XtPointer call_data);
void do_checkuseprevious(void);
void do_filebutton_on(void);
void do_filebutton_off(void);
void do_nextbutton_on(void);
void do_nextbutton_off(void);
void do_end(Widget w, XtPointer client_data, XtPointer call_data);
void do_forward(Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse(Widget w, XtPointer client_data, XtPointer call_data);
void do_start(Widget w, XtPointer client_data, XtPointer call_data);
void do_quit(Widget w, XtPointer client_data, XtPointer call_data);
void do_event(Widget w, XtPointer client_data, XtPointer call_data);
void do_flag_view(Widget w, XtPointer client_data, XtPointer call_data);
void do_unflag_view(Widget w, XtPointer client_data, XtPointer call_data);
void do_unflag_all(Widget w, XtPointer client_data, XtPointer call_data);
void do_next_buffer(Widget w, XtPointer client_data, XtPointer call_data);
void do_number_step(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_flaggedsoundings(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_flaggedprofiles(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_mode(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_time(Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse_mouse(Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse_keys(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_flags(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_detects(Widget w, XtPointer client_data, XtPointer call_data);
void do_show_pulsetypes(Widget w, XtPointer client_data, XtPointer call_data);
void do_buffer_hold(Widget w, XtPointer client_data, XtPointer call_data);
void do_buffer_size(Widget w, XtPointer client_data, XtPointer call_data);
void do_done(Widget w, XtPointer client_data, XtPointer call_data);
void do_number_pings(Widget w, XtPointer client_data, XtPointer call_data);
void do_goto_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_set_filters(Widget w, XtPointer client_data, XtPointer call_data);
void do_reset_filters(Widget w, XtPointer client_data, XtPointer call_data);
void do_check_median_xtrack(Widget w, XtPointer client_data, XtPointer call_data);
void do_check_median_ltrack(Widget w, XtPointer client_data, XtPointer call_data);
int do_wait_until_viewed(XtAppContext app);
int do_mbedit_settimer(void);
// int do_mbedit_workfunction(XtPointer client_data);
int do_message_on(char *message);
int do_message_off(void);
int do_error_dialog(char *s1, char *s2, char *s3);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);

#endif
