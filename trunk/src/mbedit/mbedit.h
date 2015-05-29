/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.h	10/14/2009
 *    $Id$
 *
 *    Copyright (c) 2009-2015 by
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
 * MBEDIT is an interactive beam editor for multibeam bathymetry data.
 * It can work with any data format supported by the MBIO library.
 * This version uses the MOTIF toolkit and has been developed using
 * the Builder Xsessory package by ICS.  This file contains
 * contains function prototypes and was added in 2009.
 *
 * Author:	D. W. Caress
 * Date:	October 14, 2009
 *
 * $Log: $
 *
 *
 */

/*--------------------------------------------------------------------*/

/* function prototypes */
int mbedit_init(int argc, char ** argv, int *startup_file);
int mbedit_set_graphics(void *xgid, int ncol, unsigned int *pixels);
int mbedit_set_scaling(int *brdr, int sh_time);
int mbedit_set_filters(int f_m, int f_m_t,
			int f_m_x, int f_m_l,
			int f_w, int f_w_t,
			int f_b, int f_b_b, int f_b_e,
			int f_d, double f_d_b, double f_d_e,
			int f_a, double f_a_b, double f_a_e);
int mbedit_get_filters( int *b_m, double *d_m,
			int *f_m, int *f_m_t,
			int *f_m_x, int *f_m_l,
			int *f_w, int *f_w_t,
			int *f_b, int *f_b_b, int *f_b_e,
			int *f_d, double *f_d_b, double *f_d_e,
			int *f_a, double *f_a_b, double *f_a_e);
int mbedit_get_defaults(
		int	*plt_size_max,
		int	*plt_size,
		int	*sh_dtcts,
		int	*sh_flggd,
		int	*sh_time,
		int	*buffer_size_max,
		int	*buffer_size,
		int	*hold_size,
		int	*form,
		int	*plwd,
		int	*exgr,
		int	*xntrvl,
		int	*yntrvl,
		int	*ttime_i,
		int	*outmode);
int mbedit_get_viewmode(int *vw_mode);
int mbedit_set_viewmode(int vw_mode);
int mbedit_action_open(
		char	*file,
		int	form,
		int	fileid,
		int	numfiles,
		int	savemode,
		int	outmode,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*buffer_size,
		int	*buffer_size_max,
		int	*hold_size,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_next_buffer(
		int	hold_size,
		int	buffer_size,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt,
		int	*quit);
int mbedit_action_close(
		int	buffer_size,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent);
int mbedit_action_done(
		int	buffer_size,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*quit);
int mbedit_action_quit(
		int	buffer_size,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent);
int mbedit_action_step(
		int	step,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_plot(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_mouse_toggle(
		int	x_loc,
		int	y_loc,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_mouse_pick(
	int	x_loc,
	int	y_loc,
	int	plwd,
	int	exgr,
	int	xntrvl,
	int	yntrvl,
	int	plt_size,
	int	sh_dtcts,
	int	sh_flggd,
	int	sh_time,
	int	*nbuffer,
	int	*ngood,
	int	*icurrent,
	int	*nplt);
int mbedit_action_mouse_erase(
		int	x_loc,
		int	y_loc,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_mouse_restore(
		int	x_loc,
		int	y_loc,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_mouse_grab(
		int	grabmode,
		int	x_loc,
		int	y_loc,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_mouse_info(
		int	x_loc,
		int	y_loc,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_zap_outbounds(
		int	iping,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_bad_ping(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_good_ping(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_left_ping(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_right_ping(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_zero_ping(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_flag_view(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_unflag_view(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_unflag_all(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_action_filter_all(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_filter_ping(int iping);
int mbedit_get_format(char *file, int *form);
int mbedit_open_file(char *file, int form, int savemode);
int mbedit_close_file(void);
int mbedit_dump_data(int hold_size, int *ndumped, int *nbuffer);
int mbedit_load_data(int buffer_size,
		int *nloaded, int *nbuffer, int *ngood, int *icurrent);
int mbedit_clear_screen(void);
int mbedit_plot_all(
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*nplt,
		int	autoscale);
int mbedit_plot_beam(int iping, int jbeam);
int mbedit_plot_ping(int iping);
int mbedit_plot_ping_label(int iping, int save);
int mbedit_plot_info(void);
int mbedit_unplot_beam(int iping, int jbeam);
int mbedit_unplot_ping(int iping);
int mbedit_unplot_info(void);
int mbedit_action_goto(
		int	ttime_i[7],
		int	hold_size,
		int	buffer_size,
		int	plwd,
		int	exgr,
		int	xntrvl,
		int	yntrvl,
		int	plt_size,
		int	sh_dtcts,
		int	sh_flggd,
		int	sh_time,
		int	*ndumped,
		int	*nloaded,
		int	*nbuffer,
		int	*ngood,
		int	*icurrent,
		int	*nplt);
int mbedit_tslabel(int data_id, char *label);
int mbedit_tsvalue(int iping, int data_id, double *value);
int mbedit_tsminmax(int iping, int nping, int data_id, double *tsmin, double *tsmax);
int mbedit_xtrackslope(int iping, double *slope);

void do_mbedit_init(int argc, char **argv);
void do_parse_datalist( char *file, int form);
void do_editlistselection( Widget w, XtPointer client_data, XtPointer call_data);
void do_filelist_remove( Widget w, XtPointer client_data, XtPointer call_data);
void do_load_specific_file(int i_file);
int do_setup_data(void);
void do_build_filelist(void);
void do_get_filters(void);
void do_file_selection_cancel( Widget w, XtPointer client_data, XtPointer call_data);
void do_expose( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_toggle( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_pick( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_erase( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_restore( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_grab( Widget w, XtPointer client_data, XtPointer call_data);
void do_mode_info( Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_y( Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_list( Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_x( Widget w, XtPointer client_data, XtPointer call_data);
int do_reset_scale_x(int pwidth, int maxx, int xntrvl, int yntrvl);
void do_output_edit( Widget w, XtPointer client_data, XtPointer call_data);
void do_output_browse( Widget w, XtPointer client_data, XtPointer call_data);
void do_output_edit_filelist( Widget w, XtPointer client_data, XtPointer call_data);
void do_output_browse_filelist( Widget w, XtPointer client_data, XtPointer call_data);
void do_x_interval( Widget w, XtPointer client_data, XtPointer call_data);
void do_y_interval( Widget w, XtPointer client_data, XtPointer call_data);
void do_load(int save_mode);
void do_load_ok( Widget w, XtPointer client_data, XtPointer call_data);
void do_load_ok_with_save( Widget w, XtPointer client_data, XtPointer call_data);
void do_load_check( Widget w, XtPointer client_data, XtPointer call_data);
void do_checkuseprevious(void);
void do_filebutton_on(void);
void do_filebutton_off(void);
void do_nextbutton_on(void);
void do_nextbutton_off(void);
void do_end( Widget w, XtPointer client_data, XtPointer call_data);
void do_forward( Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse( Widget w, XtPointer client_data, XtPointer call_data);
void do_start( Widget w, XtPointer client_data, XtPointer call_data);
void do_quit( Widget w, XtPointer client_data, XtPointer call_data);
void do_event( Widget w, XtPointer client_data, XtPointer call_data);
void do_flag_view( Widget w, XtPointer client_data, XtPointer call_data);
void do_unflag_view( Widget w, XtPointer client_data, XtPointer call_data);
void do_unflag_all( Widget w, XtPointer client_data, XtPointer call_data);
void do_next_buffer( Widget w, XtPointer client_data, XtPointer call_data);
void do_number_step( Widget w, XtPointer client_data, XtPointer call_data);
void do_show_flagged( Widget w, XtPointer client_data, XtPointer call_data);
void do_view_mode( Widget w, XtPointer client_data, XtPointer call_data);
void do_show_time( Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse_mouse( Widget w, XtPointer client_data, XtPointer call_data);
void do_reverse_keys( Widget w, XtPointer client_data, XtPointer call_data);
void do_show_flags( Widget w, XtPointer client_data, XtPointer call_data);
void do_show_detects( Widget w, XtPointer client_data, XtPointer call_data);
void do_show_pulsetypes( Widget w, XtPointer client_data, XtPointer call_data);
void do_buffer_hold( Widget w, XtPointer client_data, XtPointer call_data);
void do_buffer_size( Widget w, XtPointer client_data, XtPointer call_data);
void do_done( Widget w, XtPointer client_data, XtPointer call_data);
void do_number_pings( Widget w, XtPointer client_data, XtPointer call_data);
void do_goto_apply( Widget w, XtPointer client_data, XtPointer call_data);
void do_set_filters( Widget w, XtPointer client_data, XtPointer call_data);
void do_reset_filters( Widget w, XtPointer client_data, XtPointer call_data);
void do_check_median_xtrack( Widget w, XtPointer client_data, XtPointer call_data);
void do_check_median_ltrack( Widget w, XtPointer client_data, XtPointer call_data);
int do_wait_until_viewed(XtAppContext app);
int do_mbedit_settimer(void);
int do_mbedit_workfunction(XtPointer client_data);
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

/*--------------------------------------------------------------------*/
