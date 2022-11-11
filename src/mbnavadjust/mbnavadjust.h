/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust.h	6/24/95
 *
 *    Copyright (c) 2000-2020 by
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
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	March 22, 2000
 *
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVADJUST_DECLARE_GLOBALS
#define MBNAVADJUST_EXTERNAL
#else
#define MBNAVADJUST_EXTERNAL extern
#endif

/* mbnavadjust global control parameters */
MBNAVADJUST_EXTERNAL int mbna_verbose;
MBNAVADJUST_EXTERNAL int mbna_status;
MBNAVADJUST_EXTERNAL int mbna_view_list;
MBNAVADJUST_EXTERNAL int mbna_view_mode;
MBNAVADJUST_EXTERNAL int mbna_invert_mode;
MBNAVADJUST_EXTERNAL int mbna_save_frequency;
MBNAVADJUST_EXTERNAL int mbna_color_foreground;
MBNAVADJUST_EXTERNAL int mbna_color_background;
MBNAVADJUST_EXTERNAL int mbna_survey_select;
MBNAVADJUST_EXTERNAL int mbna_block_select;
MBNAVADJUST_EXTERNAL int mbna_block_select1;
MBNAVADJUST_EXTERNAL int mbna_block_select2;
MBNAVADJUST_EXTERNAL int mbna_file_select;
MBNAVADJUST_EXTERNAL int mbna_section_select;
MBNAVADJUST_EXTERNAL int mbna_crossing_select;
MBNAVADJUST_EXTERNAL int mbna_tie_select;
MBNAVADJUST_EXTERNAL int mbna_current_crossing;
MBNAVADJUST_EXTERNAL int mbna_current_tie;
MBNAVADJUST_EXTERNAL int mbna_current_file;
MBNAVADJUST_EXTERNAL int mbna_current_section;
MBNAVADJUST_EXTERNAL int mbna_naverr_mode;
MBNAVADJUST_EXTERNAL int mbna_file_id_1;
MBNAVADJUST_EXTERNAL int mbna_section_1;
MBNAVADJUST_EXTERNAL int mbna_file_id_2;
MBNAVADJUST_EXTERNAL int mbna_section_2;
MBNAVADJUST_EXTERNAL int mbna_snav_1;
MBNAVADJUST_EXTERNAL double mbna_snav_1_time_d;
MBNAVADJUST_EXTERNAL double mbna_snav_1_lon;
MBNAVADJUST_EXTERNAL double mbna_snav_1_lat;
MBNAVADJUST_EXTERNAL int mbna_snav_2;
MBNAVADJUST_EXTERNAL double mbna_snav_2_time_d;
MBNAVADJUST_EXTERNAL double mbna_snav_2_lon;
MBNAVADJUST_EXTERNAL double mbna_snav_2_lat;
MBNAVADJUST_EXTERNAL double mbna_offset_x;
MBNAVADJUST_EXTERNAL double mbna_offset_y;
MBNAVADJUST_EXTERNAL double mbna_offset_z;
MBNAVADJUST_EXTERNAL double mbna_invert_offset_x;
MBNAVADJUST_EXTERNAL double mbna_invert_offset_y;
MBNAVADJUST_EXTERNAL double mbna_invert_offset_z;
MBNAVADJUST_EXTERNAL double mbna_offset_x_old;
MBNAVADJUST_EXTERNAL double mbna_offset_y_old;
MBNAVADJUST_EXTERNAL double mbna_offset_z_old;
MBNAVADJUST_EXTERNAL double mbna_lon_min;
MBNAVADJUST_EXTERNAL double mbna_lon_max;
MBNAVADJUST_EXTERNAL double mbna_lat_min;
MBNAVADJUST_EXTERNAL double mbna_lat_max;
MBNAVADJUST_EXTERNAL double mbna_mtodeglon;
MBNAVADJUST_EXTERNAL double mbna_mtodeglat;
MBNAVADJUST_EXTERNAL double mbna_ox;
MBNAVADJUST_EXTERNAL double mbna_oy;
MBNAVADJUST_EXTERNAL int mbna_bin_beams_bath;
MBNAVADJUST_EXTERNAL double mbna_bin_swathwidth;
MBNAVADJUST_EXTERNAL double mbna_bin_pseudobeamwidth;
MBNAVADJUST_EXTERNAL double mbna_plot_lon_min;
MBNAVADJUST_EXTERNAL double mbna_plot_lon_max;
MBNAVADJUST_EXTERNAL double mbna_plot_lat_min;
MBNAVADJUST_EXTERNAL double mbna_plot_lat_max;
MBNAVADJUST_EXTERNAL double mbna_overlap_lon_min;
MBNAVADJUST_EXTERNAL double mbna_overlap_lon_max;
MBNAVADJUST_EXTERNAL double mbna_overlap_lat_min;
MBNAVADJUST_EXTERNAL double mbna_overlap_lat_max;
MBNAVADJUST_EXTERNAL double mbna_plotx_scale;
MBNAVADJUST_EXTERNAL double mbna_ploty_scale;
MBNAVADJUST_EXTERNAL int mbna_misfit_center;
MBNAVADJUST_EXTERNAL double mbna_misfit_xscale;
MBNAVADJUST_EXTERNAL double mbna_misfit_yscale;
MBNAVADJUST_EXTERNAL double mbna_misfit_offset_x;
MBNAVADJUST_EXTERNAL double mbna_misfit_offset_y;
MBNAVADJUST_EXTERNAL double mbna_misfit_offset_z;
MBNAVADJUST_EXTERNAL int mbna_minmisfit_nthreshold;
MBNAVADJUST_EXTERNAL double mbna_minmisfit;
MBNAVADJUST_EXTERNAL int mbna_minmisfit_n;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_x;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_y;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_z;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_xh;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_yh;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_zh;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sr1;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sx1[4];
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sr2;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sx2[4];
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sr3;
MBNAVADJUST_EXTERNAL double mbna_minmisfit_sx3[4];
MBNAVADJUST_EXTERNAL double mbna_zoff_scale_x;
MBNAVADJUST_EXTERNAL double mbna_zoff_scale_y;

MBNAVADJUST_EXTERNAL int mbna_zoom_x1;
MBNAVADJUST_EXTERNAL int mbna_zoom_y1;
MBNAVADJUST_EXTERNAL int mbna_zoom_x2;
MBNAVADJUST_EXTERNAL int mbna_zoom_y2;
MBNAVADJUST_EXTERNAL double mbna_smoothweight;
MBNAVADJUST_EXTERNAL double mbna_offsetweight;
MBNAVADJUST_EXTERNAL double mbna_zweightfactor;
MBNAVADJUST_EXTERNAL double mbna_global_tie_influence;
MBNAVADJUST_EXTERNAL int mbna_bias_mode;
MBNAVADJUST_EXTERNAL int mbna_allow_set_tie;
MBNAVADJUST_EXTERNAL int mbna_allow_add_tie;

/* plot vector data */
MBNAVADJUST_EXTERNAL struct mbna_contour_vector *mbna_contour;
MBNAVADJUST_EXTERNAL struct mbna_contour_vector mbna_contour1;
MBNAVADJUST_EXTERNAL struct mbna_contour_vector mbna_contour2;

/* model plot parameters */
MBNAVADJUST_EXTERNAL int mbna_modelplot_width;
MBNAVADJUST_EXTERNAL int mbna_modelplot_height;
MBNAVADJUST_EXTERNAL int mbna_modelplot;
MBNAVADJUST_EXTERNAL int mbna_modelplot_count;
MBNAVADJUST_EXTERNAL int mbna_modelplot_start;
MBNAVADJUST_EXTERNAL int mbna_modelplot_end;
MBNAVADJUST_EXTERNAL int mbna_modelplot_xo;
MBNAVADJUST_EXTERNAL int mbna_modelplot_yo_lon;
MBNAVADJUST_EXTERNAL int mbna_modelplot_yo_lat;
MBNAVADJUST_EXTERNAL int mbna_modelplot_yo_z;
MBNAVADJUST_EXTERNAL double mbna_modelplot_yxmid;
MBNAVADJUST_EXTERNAL double mbna_modelplot_yymid;
MBNAVADJUST_EXTERNAL double mbna_modelplot_yzmid;
MBNAVADJUST_EXTERNAL double mbna_modelplot_xscale;
MBNAVADJUST_EXTERNAL double mbna_modelplot_yscale;
MBNAVADJUST_EXTERNAL double mbna_modelplot_yzscale;
MBNAVADJUST_EXTERNAL int mbna_modelplot_zoom_x1;
MBNAVADJUST_EXTERNAL int mbna_modelplot_zoom_x2;
MBNAVADJUST_EXTERNAL int mbna_modelplot_zoom;
MBNAVADJUST_EXTERNAL int mbna_modelplot_startzoom;
MBNAVADJUST_EXTERNAL int mbna_modelplot_endzoom;
MBNAVADJUST_EXTERNAL int mbna_num_ties_plot;
MBNAVADJUST_EXTERNAL int mbna_modelplot_tiestart;
MBNAVADJUST_EXTERNAL int mbna_modelplot_tieend;
MBNAVADJUST_EXTERNAL int mbna_modelplot_tiezoom;
MBNAVADJUST_EXTERNAL int mbna_modelplot_tiestartzoom;
MBNAVADJUST_EXTERNAL int mbna_modelplot_tieendzoom;
MBNAVADJUST_EXTERNAL int mbna_modelplot_pickfile;
MBNAVADJUST_EXTERNAL int mbna_modelplot_picksection;
MBNAVADJUST_EXTERNAL int mbna_modelplot_picksnav;

/* mbnavadjust global project parameters */
MBNAVADJUST_EXTERNAL struct mbna_project project;

/* flag to reset all crossings to unanalyzed when a project is opened */
MBNAVADJUST_EXTERNAL int mbna_reset_crossings;

/* function prototype definitions */
void do_mbnavadjust_init(int argc, char **argv);
void do_set_controls(void);
void do_update_status(void);
void do_update_modelplot_status(void);
void do_naverr_init(int mode);
void do_naverr_update(void);
void do_naverr_offsetlabel(void);
void do_naverr_test_graphics(void);
void do_naverr_plot(int plotmode);
int do_message_on(char *message);
int do_message_update(char *message);
int do_message_off(void);
int do_error_dialog(char *s1, char *s2, char *s3);
void do_bell(int length);
int mbnavadjust_init_globals(void);
int mbnavadjust_init(int argc, char **argv);
int mbnavadjust_set_colors(int ncol, int *pixels);
int mbnavadjust_set_borders(int *cn_brdr, int *cr_brdr, int *zc_brdr);
int mbnavadjust_set_graphics(void *cn_xgid, void *cr_xgid, void *zc_xgid);
int mbnavadjust_file_new(char *projectname);
int mbnavadjust_file_open(char *projectname);
int mbnavadjust_poornav_file(void);
int mbnavadjust_goodnav_file(void);
int mbnavadjust_fixednav_file(void);
int mbnavadjust_fixedxynav_file(void);
int mbnavadjust_fixedznav_file(void);
int mbnavadjust_set_tie_xy(void);
int mbnavadjust_set_tie_z(void);
int mbnavadjust_set_tie_xyz(void);
int mbnavadjust_naverr_save(void);
int mbnavadjust_naverr_specific_crossing(int new_crossing, int new_tie);
int mbnavadjust_naverr_specific_section(int new_file, int new_section);
int mbnavadjust_naverr_next_crossing(void);
int mbnavadjust_naverr_next_section(void);
int mbnavadjust_naverr_previous_crossing(void);
int mbnavadjust_naverr_previous_section(void);
int mbnavadjust_naverr_nextunset_crossing(void);
int mbnavadjust_naverr_nextunset_section(void);
int mbnavadjust_naverr_selecttie(void);
int mbnavadjust_naverr_addtie(void);
int mbnavadjust_naverr_deletetie(void);
int mbnavadjust_deletetie(int icrossing, int jtie, int deletestatus);
int mbnavadjust_naverr_settie(void);
int mbnavadjust_naverr_resettie(void);
int mbnavadjust_naverr_checkoksettie(void);
int mbnavadjust_naverr_skip(void);
int mbnavadjust_naverr_unset(void);
int mbnavadjust_crossing_load(void);
int mbnavadjust_crossing_unload(void);
int mbnavadjust_naverr_replot(void);
int mbnavadjust_crossing_replot(void);
int mbnavadjust_referencesection_replot(void);
int mbnavadjust_naverr_snavpoints(int ix, int iy);
int mbnavadjust_get_misfit(void);
int mbnavadjust_get_misfitxy(void);
void plot(double x, double y, int ipen);
void newpen(int ipen);
void justify_string(double height, char *string, double *s);
void plot_string(double x, double y, double hgt, double angle, char *label);
void mbnavadjust_naverr_scale(void);
void mbnavadjust_naverr_plot(int plotmode);
int mbnavadjust_autopick(bool do_vertical);
int mbnavadjust_autosetsvsvertical(void);
int mbnavadjust_zerozoffsets(void);
int mbnavadjust_unsetskipped(void);
int mbnavadjust_referenceplussection_load(void);
int mbnavadjust_referenceplussection_unload(void);
int mbnavadjust_referencegrid_unload(void);
int mbnavadjust_invertnav(void);
int mbnavadjust_applynav(void);
int mbnavadjust_updategrid(void);
int mbnavadjust_modelplot_plot(const char *sourcefile, int sourceline);
int mbnavadjust_set_modelplot_graphics(void *modp_xgid, int *modp_borders);
int mbnavadjust_modelplot_plot_timeseries(void);
int mbnavadjust_modelplot_plot_perturbation(void);
int mbnavadjust_modelplot_plot_tieoffsets(void);
int mbnavadjust_modelplot_pick(int x, int y);
int mbnavadjust_modelplot_pick_timeseries(int x, int y);
int mbnavadjust_modelplot_pick_perturbation(int x, int y);
int mbnavadjust_modelplot_pick_tieoffsets(int x, int y);
int mbnavadjust_modelplot_pick_globaltieoffsets(int x, int y);
int mbnavadjust_modelplot_middlepick(int x, int y);
int mbnavadjust_modelplot_setzoom(void);
int mbnavadjust_modelplot_clearblock(void);

int mbnavadjust_open_visualization(int which_grid);
int mbnavadjust_dismiss_visualization(void);
int mbnavadjust_reset_visualization_navties(void);
int mbnavadjust_visualization_selectcrossingfromroute(int icrossing, int itie);
int mbnavadjust_visualization_selectcrossingfromnav(int ifile1, int isection1, int ifile2, int isection2);

void do_list_data_select(Widget w, XtPointer client_data, XtPointer call_data);
int do_check_crossing_listok(int icrossing);
int do_check_globaltie_listok(int ifile, int isection);
int do_check_section_listok(int ifile, int isection);
int do_check_nav_active(int ifile, int isection);
void do_naverr_cont_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_corr_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_cont_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_corr_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_zcorr_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_previous(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_next(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_nextunset(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_addtie(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_deletetie(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_selecttie(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_settie(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_resettie(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_unset(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_setnone(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_zerooffset(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_zerozoffset(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_minmisfit(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_minxymisfit(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_misfitcenter(Widget w, XtPointer client_data, XtPointer call_data);
void do_file_new(Widget w, XtPointer client_data, XtPointer call_data);
void do_file_open(Widget w, XtPointer client_data, XtPointer call_data);
void do_file_close(Widget w, XtPointer client_data, XtPointer call_data);
void do_file_importdata(Widget w, XtPointer client_data, XtPointer call_data);
void do_quit(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_mode(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_ok(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_cancel(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showsurveys(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showblocks(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showdata(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showsections(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showcrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showmediocrecrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showgoodcrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showbettercrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showtruecrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_list_showcrossingties(Widget w, XtPointer client_data, XtPointer call_data);
void do_list_showcrossingtiessorted(Widget w, XtPointer client_data, XtPointer call_data);
void do_list_showglobalties(Widget w, XtPointer client_data, XtPointer call_data);
void do_list_showglobaltiessorted(Widget w, XtPointer client_data, XtPointer call_data);

void do_view_showallsurveys(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showselectedsurvey(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showselectedblock(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showselectedfile(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showselectedsection(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showwithselectedsurvey(Widget w, XtPointer client_data, XtPointer call_data);
void do_view_showwithselectedfile(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_autopick(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_autopickhorizontal(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_autosetsvsvertical(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_analyzecrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_checknewcrossings(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_invertnav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_updategrids(Widget w, XtPointer client_data, XtPointer call_data);
void do_apply_nav(Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_list(Widget w, XtPointer client, XtPointer call);
int do_wait_until_viewed(XtAppContext app);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);
int do_info_add(char *info, int timetag);

void do_naverr_setoffset(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_fullsize(Widget w, XtPointer client_data, XtPointer call_data);
void do_naverr_applyzoffset(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_applyall(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_init(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_toggle(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_heading(Widget w, XtPointer client_data, XtPointer call_data);
void do_biases_roll(Widget w, XtPointer client_data, XtPointer call_data);
void do_controls_apply(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_sectionlength(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_sectionsoundings(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_decimation(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_contourinterval(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_tickinterval(Widget w, XtPointer client_data, XtPointer call_data);
void do_controls_scale_colorinterval(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_smoothing(Widget w, XtPointer client_data, XtPointer call_data);
void do_scale_controls_zoffset(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_poornav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_goodnav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_fixednav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_fixedxynav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_fixedznav(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_tie_xy(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_z(Widget w, XtPointer client_data, XtPointer call_data);
void do_action_tie_xyz(Widget w, XtPointer client_data, XtPointer call_data);
void do_zerozoffsets(Widget w, XtPointer client_data, XtPointer call_data);
void do_visualize(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_show(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_fullsize(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_tieoffsets(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_perturbation(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_timeseries(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_clearblock(Widget w, XtPointer client_data, XtPointer call_data);
void do_modelplot_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
int do_visualize_dismiss_notify(size_t instance);
void do_visualize_sensitivity(void);
void do_update_visualization_status(void);
void do_pickroute_notify(size_t instance);
void do_picknav_notify(size_t instance);
void do_mbnavadjust_addcrossing(Widget w, XtPointer client, XtPointer call);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

/*--------------------------------------------------------------------*/
