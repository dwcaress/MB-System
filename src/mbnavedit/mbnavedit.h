/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.h	6/24/95
 *
 *    Copyright (c) 1995-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBNAVEDIT is an interactive navigation editor for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	June 24,  1995
 * Date:	August 28, 2000 (New version - no buffered i/o)
 */

#ifndef MBNAVEDIT_MBNAVEDIT_H_
#define MBNAVEDIT_MBNAVEDIT_H_

#include "mb_status.h"

/* mbnavedit global control parameters */
int output_mode;
int run_mbprocess;  // TODO(schwehr): bool
int gui_mode;  // TODO(schwehr): bool
int data_show_max;
int data_show_size;
int data_step_max;
int data_step_size;
int mode_pick;
int mode_set_interval;
int plot_tint;
int plot_tint_org;
int plot_lon;
int plot_lon_org;
int plot_lon_dr;
int plot_lat;
int plot_lat_org;
int plot_lat_dr;
int plot_speed;
int plot_speed_org;
int plot_smg;
int plot_heading;
int plot_heading_org;
int plot_cmg;
int plot_draft;
int plot_draft_org;
int plot_draft_dr;
int plot_roll;
int plot_pitch;
int plot_heave;
int mean_time_window;
int drift_lon;
int drift_lat;
int timestamp_problem;  // TODO(schwehr): bool
int use_ping_data;  // TODO(schwehr): bool
int strip_comments;  // TODO(schwehr): bool
int format;
char ifile[MB_PATH_MAXLINE];
char nfile[MB_PATH_MAXLINE];
int nfile_defined;
int model_mode;
double weight_speed;
double weight_acceleration;
int scrollcount;
double offset_lon;
double offset_lat;
double offset_lon_applied;
double offset_lat_applied;

/* mbnavedit plot size parameters */
int plot_width;
int plot_height;
int number_plots;
int window_width;
int window_height;

/* Mode value defines */
#define PICK_MODE_PICK 0
#define PICK_MODE_SELECT 1
#define PICK_MODE_DESELECT 2
#define PICK_MODE_SELECTALL 3
#define PICK_MODE_DESELECTALL 4
#define OUTPUT_MODE_OUTPUT 0
#define OUTPUT_MODE_BROWSE 1
#define PLOT_TINT 0
#define PLOT_LONGITUDE 1
#define PLOT_LATITUDE 2
#define PLOT_SPEED 3
#define PLOT_HEADING 4
#define PLOT_DRAFT 5
#define PLOT_ROLL 6
#define PLOT_PITCH 7
#define PLOT_HEAVE 8
#define MODEL_MODE_OFF 0
#define MODEL_MODE_MEAN 1
#define MODEL_MODE_DR 2
#define MODEL_MODE_INVERT 3
#define NUM_FILES_MAX 1000

/*--------------------------------------------------------------------*/

/* function prototypes */
void do_mbnavedit_init(int argc, char **argv);
void do_parse_datalist(char *file, int form);
void do_editlistselection(Widget w, XtPointer client_data, XtPointer call_data);
void do_filelist_remove(Widget w, XtPointer client_data, XtPointer call_data);
void do_load_specific_file(int i_file);
void do_build_filelist(void);
void do_set_controls(void);
void do_nextbuffer(Widget w, XtPointer client_data, XtPointer call_data);
void do_done(Widget w, XtPointer client_data, XtPointer call_data);
void do_start(Widget w, XtPointer client_data, XtPointer call_data);
void do_end(Widget w, XtPointer client_data, XtPointer call_data);
void do_forward(Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse(Widget w, XtPointer client_data, XtPointer call_data);
void do_timespan(Widget w, XtPointer client_data, XtPointer call_data);
void do_timestep(Widget w, XtPointer client_data, XtPointer call_data);
void do_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_event(Widget w, XtPointer client_data, XtPointer call_data);
void do_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
void do_toggle_time(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_lon(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_lat(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_heading(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_speed(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_sonardepth(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_time(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_lon(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_lat(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_speed(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_dr_lat(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_dr_lon(Widget w, XtPointer client_data, XtPointer call_data);
void do_flag(Widget w, XtPointer client_data, XtPointer call_data);
void do_unflag(Widget w, XtPointer client_data, XtPointer call_data);
void do_modeling_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_model_mode(Widget w, XtPointer client_data, XtPointer call_data);
void do_timeinterpolation_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_deletebadtimetag_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_meantimewindow(Widget w, XtPointer client_data, XtPointer call_data);
void do_driftlon(Widget w, XtPointer client_data, XtPointer call_data);
void do_driftlat(Widget w, XtPointer client_data, XtPointer call_data);
void do_offset_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_show_smg(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_heading(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_org_sonardepth(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_show_cmg(Widget w, XtPointer client_data, XtPointer call_data);
void do_button_use_dr(Widget w, XtPointer client_data, XtPointer call_data);
void do_button_use_smg(Widget w, XtPointer client_data, XtPointer call_data);
void do_button_use_cmg(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_output_on(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_output_off(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_output_on_filelist(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_output_off_filelist(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_cancel(Widget w, XtPointer client_data, XtPointer call_data);
void do_filebutton_on(void);
void do_filebutton_off(void);
void do_fileselection_ok(Widget w, XtPointer client_data, XtPointer call_data);
void do_checkuseprevious(void);
void do_useprevious_yes(Widget w, XtPointer client_data, XtPointer call_data);
void do_useprevious_no(Widget w, XtPointer client_data, XtPointer call_data);
void do_load(int useprevious);
void do_fileselection_filter(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_nomatch(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_pick(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_select(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_deselect(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_selectall(Widget w, XtPointer client_data, XtPointer call_data);
void do_toggle_deselectall(Widget w, XtPointer client_data, XtPointer call_data);
void do_quit(Widget w, XtPointer client_data, XtPointer call_data);
void do_interpolation(Widget w, XtPointer client_data, XtPointer call_data);
void do_interpolationrepeats(Widget w, XtPointer client_data, XtPointer call_data);
void do_scroll(Widget w, XtPointer client_data, XtPointer call_data);
void do_revert(Widget w, XtPointer client_data, XtPointer call_data);
void do_showall(Widget w, XtPointer client_data, XtPointer call_data);
void do_set_interval(Widget w, XtPointer client_data, XtPointer call_data);
int do_unset_interval(void);
void do_toggle_vru(Widget w, XtPointer client_data, XtPointer call_data);
void mbnavedit_bell(int length);
void mbnavedit_get_position(int *win_x, int *win_y, unsigned int *mask_return);
void mbnavedit_pickcursor(void);
void mbnavedit_selectcursor(void);
void mbnavedit_deselectcursor(void);
void mbnavedit_selectallcursor(void);
void mbnavedit_deselectallcursor(void);
void mbnavedit_setintervalcursor(void);
int do_wait_until_viewed(XtAppContext app);
int do_mbnavedit_settimer(void);
int do_mbnavedit_workfunction(XtPointer client_data);
int do_message_on(char *message);
int do_message_off(void);
int do_error_dialog(char *s1, char *s2, char *s3);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);

int mbnavedit_init_globals(void);
int mbnavedit_init(int argc, char **argv, int *startup_file);
int mbnavedit_set_graphics(void *xgid, int ncol, unsigned int *pixels);
int mbnavedit_action_open(int useprevious);
int mbnavedit_open_file(int useprevious);
int mbnavedit_close_file(void);
int mbnavedit_dump_data(int hold);
int mbnavedit_load_data(void);
int mbnavedit_clear_screen(void);
int mbnavedit_action_next_buffer(int *quit);
int mbnavedit_action_offset(void);
int mbnavedit_action_close(void);
int mbnavedit_action_done(int *quit);
int mbnavedit_action_quit(void);
int mbnavedit_action_step(int step);
int mbnavedit_action_start(void);
int mbnavedit_action_end(void);
int mbnavedit_action_mouse_pick(int xx, int yy);
int mbnavedit_action_mouse_select(int xx, int yy);
int mbnavedit_action_mouse_deselect(int xx, int yy);
int mbnavedit_action_mouse_selectall(int xx, int yy);
int mbnavedit_action_mouse_deselectall(int xx, int yy);
int mbnavedit_action_deselect_all(int type);
int mbnavedit_action_set_interval(int xx, int yy, int which);
int mbnavedit_action_use_dr(void);
int mbnavedit_action_use_smg(void);
int mbnavedit_action_use_cmg(void);
int mbnavedit_action_interpolate(void);
int mbnavedit_action_interpolaterepeats(void);
int mbnavedit_action_revert(void);
int mbnavedit_action_flag(void);
int mbnavedit_action_unflag(void);
int mbnavedit_action_fixtime(void);
int mbnavedit_action_deletebadtime(void);
int mbnavedit_action_showall(void);
int mbnavedit_get_smgcmg(int i);
int mbnavedit_get_model(void);
int mbnavedit_get_gaussianmean(void);
int mbnavedit_get_dr(void);
int mbnavedit_get_inversion(void);
int mbnavedit_plot_all(void);
int mbnavedit_plot_tint(int iplot);
int mbnavedit_plot_lon(int iplot);
int mbnavedit_plot_lat(int iplot);
int mbnavedit_plot_speed(int iplot);
int mbnavedit_plot_heading(int iplot);
int mbnavedit_plot_draft(int iplot);
int mbnavedit_plot_roll(int iplot);
int mbnavedit_plot_pitch(int iplot);
int mbnavedit_plot_heave(int iplot);
int mbnavedit_plot_tint_value(int iplot, int iping);
int mbnavedit_plot_lon_value(int iplot, int iping);
int mbnavedit_plot_lat_value(int iplot, int iping);
int mbnavedit_plot_speed_value(int iplot, int iping);
int mbnavedit_plot_heading_value(int iplot, int iping);
int mbnavedit_plot_draft_value(int iplot, int iping);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
void BxPopdownCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

#endif  // MBNAVEDIT_MBNAVEDIT_H_
