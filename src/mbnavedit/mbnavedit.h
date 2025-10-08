/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.h	6/24/95
 *
 *    Copyright (c) 1995-2025 by
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

#include <X11/Intrinsic.h>

#include "mb_status.h"

#include "mbnavedit_prog.h"

#ifdef MBNAVEDIT_DECLARE_GLOBALS
#define MBNAVEDIT_EXTERNAL
#else
#define MBNAVEDIT_EXTERNAL extern
#endif

/* ***
// mbnavedit global control parameters 
MBNAVEDIT_EXTERNAL int output_mode;
MBNAVEDIT_EXTERNAL bool run_mbprocess;
MBNAVEDIT_EXTERNAL bool gui_mode;
MBNAVEDIT_EXTERNAL int data_show_max;
MBNAVEDIT_EXTERNAL int data_show_size;
MBNAVEDIT_EXTERNAL int data_step_max;
MBNAVEDIT_EXTERNAL int data_step_size;
MBNAVEDIT_EXTERNAL int mode_pick;
MBNAVEDIT_EXTERNAL int mode_set_interval;
MBNAVEDIT_EXTERNAL int plot_tint;
MBNAVEDIT_EXTERNAL int plot_tint_org;
MBNAVEDIT_EXTERNAL int plot_lon;
MBNAVEDIT_EXTERNAL int plot_lon_org;
MBNAVEDIT_EXTERNAL int plot_lon_dr;
MBNAVEDIT_EXTERNAL int plot_lat;
MBNAVEDIT_EXTERNAL int plot_lat_org;
MBNAVEDIT_EXTERNAL int plot_lat_dr;
MBNAVEDIT_EXTERNAL int plot_speed;
MBNAVEDIT_EXTERNAL int plot_speed_org;
MBNAVEDIT_EXTERNAL int plot_smg;
MBNAVEDIT_EXTERNAL int plot_heading;
MBNAVEDIT_EXTERNAL int plot_heading_org;
MBNAVEDIT_EXTERNAL int plot_cmg;
MBNAVEDIT_EXTERNAL int plot_draft;
MBNAVEDIT_EXTERNAL int plot_draft_org;
MBNAVEDIT_EXTERNAL int plot_draft_dr;
MBNAVEDIT_EXTERNAL int plot_roll;
MBNAVEDIT_EXTERNAL int plot_pitch;
MBNAVEDIT_EXTERNAL int plot_heave;
MBNAVEDIT_EXTERNAL int mean_time_window;
MBNAVEDIT_EXTERNAL int drift_lon;
MBNAVEDIT_EXTERNAL int drift_lat;
MBNAVEDIT_EXTERNAL bool timestamp_problem;
MBNAVEDIT_EXTERNAL bool use_ping_data;
MBNAVEDIT_EXTERNAL bool strip_comments;
MBNAVEDIT_EXTERNAL int format;
MBNAVEDIT_EXTERNAL char ifile[MB_PATH_MAXLINE];
MBNAVEDIT_EXTERNAL char nfile[MB_PATHPLUS_MAXLINE];
MBNAVEDIT_EXTERNAL int nfile_defined;
MBNAVEDIT_EXTERNAL int model_mode;
MBNAVEDIT_EXTERNAL double weight_speed;
MBNAVEDIT_EXTERNAL double weight_acceleration;
MBNAVEDIT_EXTERNAL int scrollcount;
MBNAVEDIT_EXTERNAL double offset_lon;
MBNAVEDIT_EXTERNAL double offset_lat;
MBNAVEDIT_EXTERNAL double offset_lon_applied;
MBNAVEDIT_EXTERNAL double offset_lat_applied;

// mbnavedit plot size parameters 
MBNAVEDIT_EXTERNAL int plot_width;
MBNAVEDIT_EXTERNAL int plot_height;
MBNAVEDIT_EXTERNAL int number_plots;
MBNAVEDIT_EXTERNAL int window_width;
MBNAVEDIT_EXTERNAL int window_height;
>>>>>>> master
*** */

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
void do_toggle_sensordepth(Widget w, XtPointer client_data, XtPointer call_data);
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
void do_toggle_org_sensordepth(Widget w, XtPointer client_data, XtPointer call_data);
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

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
void BxPopdownCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

#endif  // MBNAVEDIT_MBNAVEDIT_H_
