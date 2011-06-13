/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust.h	6/24/95
 *    $Id$
 *
 *    Copyright (c) 2000-2011 by
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
 * $Log: mbnavadjust.h,v $
 * Revision 5.12  2008/12/22 08:32:52  caress
 * Added additional model view - survey vs survey rather than sequential.
 *
 * Revision 5.11  2008/09/11 20:12:43  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.10  2008/05/16 22:42:32  caress
 * Release 5.1.1beta18 - working towards use of 3D uncertainty.
 *
 * Revision 5.9  2007/10/08 16:02:46  caress
 * MBnavadjust now performs an initial inversion for the average offsets for each independent block of data and then removes that average signal before performing the full inversion.
 *
 * Revision 5.8  2007/05/14 06:34:11  caress
 * Many changes to mbnavadjust, including adding z offsets and 3D search grids.
 *
 * Revision 5.7  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.6  2005/06/04 04:34:07  caress
 * Added notion of "truecrossings", so it's possible to process the data while only looking at crossing tracks and ignoring overlap points.
 *
 * Revision 5.5  2004/12/02 06:34:27  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.4  2004/05/21 23:31:28  caress
 * Moved to new version of BX GUI builder
 *
 * Revision 5.3  2002/03/26 07:43:57  caress
 * Release 5.0.beta15
 *
 * Revision 5.2  2001/10/19 00:55:42  caress
 * Now tries to use relative paths.
 *
 * Revision 5.1  2001/07/20  00:33:43  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2000/12/01  22:55:48  caress
 * First cut at Version 5.0.
 *
 * Revision 4.0  2000/09/30  07:00:06  caress
 * Snapshot for Dale.
 *
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
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* mbnavadjust global defines */
#define STRING_MAX 			MB_PATH_MAXLINE
#define BUFFER_MAX 			1024
#define ALLOC_NUM			10
#define MBNA_SNAV_NUM			11
#define	MBNA_STATUS_GUI			0
#define	MBNA_STATUS_MAKECONTOUR		1
#define	MBNA_STATUS_NAVERR		2
#define	MBNA_STATUS_NAVSOLVE		3
#define	MBNA_INVERSION_NONE		0
#define	MBNA_INVERSION_OLD		1
#define	MBNA_INVERSION_CURRENT		2
#define	MBNA_FILE_POORNAV		1
#define	MBNA_FILE_GOODNAV		2
#define	MBNA_FILE_FIXEDNAV		3
#define	MBNA_FILE_FIXEDXYNAV		4
#define	MBNA_FILE_FIXEDZNAV		5
#define	MBNA_TIE_XYZ			1
#define	MBNA_TIE_XY			2
#define	MBNA_TIE_Z			3
#define	MBNA_CROSSING_STATUS_NONE	0
#define	MBNA_CROSSING_STATUS_SET	1
#define	MBNA_CROSSING_STATUS_SKIP	2
#define MBNA_TIME_GAP_MAX		120.0
#define MBNA_TIME_DIFF_THRESHOLD	2.0
#define MBNA_VIEW_LIST_SURVEYS		0
#define MBNA_VIEW_LIST_FILES		1
#define MBNA_VIEW_LIST_FILESECTIONS	2
#define MBNA_VIEW_LIST_CROSSINGS	3
#define MBNA_VIEW_LIST_GOODCROSSINGS	4
#define MBNA_VIEW_LIST_BETTERCROSSINGS	5
#define MBNA_VIEW_LIST_TRUECROSSINGS	6
#define MBNA_VIEW_LIST_TIES		7
#define MBNA_VIEW_MODE_ALL		0
#define MBNA_VIEW_MODE_SURVEY		1
#define MBNA_VIEW_MODE_WITHSURVEY	2
#define MBNA_VIEW_MODE_FILE		3
#define MBNA_VIEW_MODE_WITHFILE		4
#define MBNA_VIEW_MODE_WITHSECTION	5
#define MBNA_SELECT_NONE		-1
#define MBNA_VECTOR_ALLOC_INC		1000
#define MBNA_PEN_UP			3
#define MBNA_PEN_DOWN			2
#define MBNA_PEN_ORIGIN			-3
#define MBNA_PEN_COLOR			0
#define MBNA_PLOT_MODE_FIRST 		0
#define MBNA_PLOT_MODE_MOVE 		1
#define MBNA_PLOT_MODE_ZOOMFIRST 	2
#define MBNA_PLOT_MODE_ZOOM 		3
#define MBNA_MASK_DIM			25
#define MBNA_MISFIT_ZEROCENTER		0
#define MBNA_MISFIT_AUTOCENTER		1
#define MBNA_MISFIT_DIMXY		61
#define MBNA_MISFIT_NTHRESHOLD		(MBNA_MISFIT_DIMXY * MBNA_MISFIT_DIMXY / 36)
#define MBNA_MISFIT_DIMZ		51
#define MBNA_BIAS_SAME			0
#define MBNA_BIAS_DIFFERENT		1
#define MBNA_OVERLAP_THRESHOLD		25

#define	MBNA_MODELPLOT_SEQUENTIAL	0
#define	MBNA_MODELPLOT_SURVEY		1
#define	MBNA_MODELPLOT_LEFT_WIDTH	25
#define	MBNA_MODELPLOT_LEFT_HEIGHT	65
#define	MBNA_MODELPLOT_X_SPACE		10
#define	MBNA_MODELPLOT_Y_SPACE		30

#define	MBNA_INTERP_NONE		0
#define	MBNA_INTERP_CONSTANT		1
#define	MBNA_INTERP_INTERP		2

#define MBNA_SMOOTHING_DEFAULT		2

#define MBNA_INTERATION_MAX		10000
#define MBNA_CONVERGENCE		0.000001
#define MBNA_SMALL			0.0001

/* mbnavadjust project and file structures */
struct mbna_section {
	int	num_pings;
	int	num_beams;
	int	global_start_ping;
	int	global_start_snav;
	int	continuity;
	double	distance;
	double	btime_d;
	double 	etime_d;
	double	lonmin;
	double	lonmax;
 	double	latmin;
	double	latmax;
	double	depthmin;
	double	depthmax;
	int	coverage[MBNA_MASK_DIM * MBNA_MASK_DIM];
	int	num_snav;
	int	snav_id[MBNA_SNAV_NUM];
	int	snav_num_ties[MBNA_SNAV_NUM];
	int	snav_invert_id[MBNA_SNAV_NUM];
	int	snav_invert_constraint[MBNA_SNAV_NUM];
	double	snav_distance[MBNA_SNAV_NUM];
	double	snav_time_d[MBNA_SNAV_NUM];
	double	snav_lon[MBNA_SNAV_NUM];
	double	snav_lat[MBNA_SNAV_NUM];
	double	snav_lon_offset[MBNA_SNAV_NUM];
	double	snav_lat_offset[MBNA_SNAV_NUM];
	double	snav_z_offset[MBNA_SNAV_NUM];
	double	snav_lon_offset_int[MBNA_SNAV_NUM];
	double	snav_lat_offset_int[MBNA_SNAV_NUM];
	double	snav_z_offset_int[MBNA_SNAV_NUM];
	int	contoursuptodate;
};
struct mbna_file {
	int	status;
	int	id;
	int	output_id;
	char 	file[STRING_MAX];
	char 	path[STRING_MAX];
	int	format;
	double	heading_bias_import;
	double	roll_bias_import;
	double	heading_bias;
	double	roll_bias;
	int	block;
	double	block_offset_x;
	double	block_offset_y;
	double	block_offset_z;
	int	num_snavs;
	int	num_pings;
	int	num_beams;
	int	num_sections;
	int	num_sections_alloc;
	struct mbna_section *sections;
};
struct mbna_tie {
	int	status;
	int	snav_1;
	double	snav_1_time_d;
	int	snav_2;
	double	snav_2_time_d;
	double	offset_x;
	double	offset_y;
	double	offset_x_m;
	double	offset_y_m;
	double	offset_z_m;
	double	sigmar1;
	double	sigmax1[3];
	double	sigmar2;
	double	sigmax2[3];
	double	sigmar3;
	double	sigmax3[3];
	int	inversion_status;
	double	inversion_offset_x;
	double	inversion_offset_y;
	double	inversion_offset_x_m;
	double	inversion_offset_y_m;
	double	inversion_offset_z_m;
	int	block_1;
	int	block_2;
	int	isurveyplotindex;
};
struct mbna_crossing {
	int	status;
	int	truecrossing;
	int	overlap;
	int	file_id_1;
	int	section_1;
	int	file_id_2;
	int	section_2;
	int	num_ties;
	struct mbna_tie ties[MBNA_SNAV_NUM];
};
struct mbna_project {
	int	open;
	char	name[STRING_MAX];
	char	path[STRING_MAX];
	char	home[STRING_MAX];
	char	datadir[STRING_MAX];
	int	num_files;
	int	num_files_alloc;
	struct mbna_file *files;
	int	num_blocks;
	int	num_snavs;
	int	num_pings;
	int	num_beams;
	int	num_crossings;
	int	num_crossings_alloc;
	int	num_crossings_analyzed;
	int	num_goodcrossings;
	int	num_truecrossings;
	int	num_truecrossings_analyzed;
	struct mbna_crossing *crossings;
	int	num_ties;
	double	section_length;
	int	section_soundings;
	double	cont_int;
	double	col_int;
	double	tick_int;
	double	label_int;
	int	decimation;
	double	precision;
	double	smoothing;
	double	zoffsetwidth;
	int	inversion;
	int	modelplot;
	int	modelplot_style;
	FILE	*logfp;
};
struct mbna_plot_vector 
    {
    int	    command;
    int	    color;
    double  x;
    double  y;
    };
struct mbna_contour_vector
    {
    int	    nvector;
    int	    nvector_alloc;
    struct mbna_plot_vector *vector;
    };

/* mbnavadjust global control parameters */
EXTERNAL int	mbna_verbose;
EXTERNAL int	mbna_status;
EXTERNAL int	mbna_view_list;
EXTERNAL int	mbna_view_mode;
EXTERNAL int	mbna_survey_select;
EXTERNAL int	mbna_file_select;
EXTERNAL int	mbna_section_select;
EXTERNAL int	mbna_crossing_select;
EXTERNAL int	mbna_tie_select;
EXTERNAL int	mbna_current_crossing;
EXTERNAL int	mbna_current_tie;
EXTERNAL int	mbna_naverr_load;
EXTERNAL int	mbna_file_id_1;
EXTERNAL int	mbna_section_1;
EXTERNAL int	mbna_file_id_2;
EXTERNAL int	mbna_section_2;
EXTERNAL int	mbna_snav_1;
EXTERNAL double	mbna_snav_1_time_d;
EXTERNAL double	mbna_snav_1_lon;
EXTERNAL double	mbna_snav_1_lat;
EXTERNAL int	mbna_snav_2;
EXTERNAL double	mbna_snav_2_time_d;
EXTERNAL double	mbna_snav_2_lon;
EXTERNAL double	mbna_snav_2_lat;
EXTERNAL double	mbna_offset_x;
EXTERNAL double	mbna_offset_y;
EXTERNAL double	mbna_offset_z;
EXTERNAL double	mbna_invert_offset_x;
EXTERNAL double	mbna_invert_offset_y;
EXTERNAL double	mbna_invert_offset_z;
EXTERNAL double	mbna_offset_x_old;
EXTERNAL double	mbna_offset_y_old;
EXTERNAL double	mbna_offset_z_old;
EXTERNAL double	mbna_lon_min;
EXTERNAL double	mbna_lon_max;
EXTERNAL double	mbna_lat_min;
EXTERNAL double	mbna_lat_max;
EXTERNAL double	mbna_mtodeglon;
EXTERNAL double	mbna_mtodeglat;
EXTERNAL int	mbna_contour_algorithm;
EXTERNAL int	mbna_ncolor;
EXTERNAL double	mbna_ox;
EXTERNAL double	mbna_oy;
EXTERNAL double	mbna_plot_lon_min;
EXTERNAL double	mbna_plot_lon_max;
EXTERNAL double	mbna_plot_lat_min;
EXTERNAL double	mbna_plot_lat_max;
EXTERNAL double	mbna_overlap_lon_min;
EXTERNAL double	mbna_overlap_lon_max;
EXTERNAL double	mbna_overlap_lat_min;
EXTERNAL double	mbna_overlap_lat_max;
EXTERNAL double	mbna_plotx_scale;
EXTERNAL double	mbna_ploty_scale;
EXTERNAL int	mbna_misfit_center;
EXTERNAL double	mbna_misfit_xscale;
EXTERNAL double	mbna_misfit_yscale;
EXTERNAL double mbna_misfit_offset_x;
EXTERNAL double mbna_misfit_offset_y;
EXTERNAL double mbna_misfit_offset_z;
EXTERNAL int	mbna_minmisfit_nthreshold;
EXTERNAL double	mbna_minmisfit;
EXTERNAL int	mbna_minmisfit_n;
EXTERNAL double mbna_minmisfit_x;
EXTERNAL double mbna_minmisfit_y;
EXTERNAL double mbna_minmisfit_z;
EXTERNAL double mbna_minmisfit_xh;
EXTERNAL double mbna_minmisfit_yh;
EXTERNAL double mbna_minmisfit_zh;
EXTERNAL double mbna_minmisfit_sr1;
EXTERNAL double mbna_minmisfit_sx1[4];
EXTERNAL double mbna_minmisfit_sr2;
EXTERNAL double mbna_minmisfit_sx2[4];
EXTERNAL double mbna_minmisfit_sr3;
EXTERNAL double mbna_minmisfit_sx3[4];
EXTERNAL double	mbna_zoff_scale_x;
EXTERNAL double	mbna_zoff_scale_y;

EXTERNAL int	mbna_zoom_x1;
EXTERNAL int	mbna_zoom_y1;
EXTERNAL int	mbna_zoom_x2;
EXTERNAL int	mbna_zoom_y2;
EXTERNAL double mbna_smoothweight;
EXTERNAL double mbna_offsetweight;
EXTERNAL int	mbna_bias_mode;
EXTERNAL int	mbna_allow_set_tie;

/* plot vector data */
EXTERNAL struct mbna_contour_vector *mbna_contour;
EXTERNAL struct mbna_contour_vector mbna_contour1;
EXTERNAL struct mbna_contour_vector mbna_contour2;

/* model plot parameters */
EXTERNAL int	mbna_modelplot_width;
EXTERNAL int	mbna_modelplot_height;
EXTERNAL int	mbna_modelplot;
EXTERNAL int	mbna_modelplot_xo;
EXTERNAL int	mbna_modelplot_yo_lon;
EXTERNAL int	mbna_modelplot_yo_lat;
EXTERNAL int	mbna_modelplot_yo_z;
EXTERNAL double	mbna_modelplot_xscale;
EXTERNAL double	mbna_modelplot_yscale;
EXTERNAL double	mbna_modelplot_yzscale;
EXTERNAL int	mbna_modelplot_zoom_x1;
EXTERNAL int	mbna_modelplot_zoom_x2;
EXTERNAL int	mbna_modelplot_pingstart;
EXTERNAL int	mbna_modelplot_pingend;
EXTERNAL int	mbna_modelplot_pickfile;
EXTERNAL int	mbna_modelplot_picksection;
EXTERNAL int	mbna_modelplot_picksnav;

/* mbnavadjust global project parameters */
EXTERNAL struct	mbna_project project;

/* flag to reset all crossings to unanalyzed when a project is opened */
EXTERNAL int	mbna_reset_crossings;

/* function prototype definitions */
void	do_mbnavadjust_init(int argc, char **argv);
void	do_set_controls();
void	do_update_status();
void	do_naverr_init();
void	do_update_naverr();
void	do_naverr_offsetlabel();
void	do_naverr_test_graphics();
void	do_naverr_plot(int plotmode);
int	do_message_on(char *message);
int	do_message_update(char *message);
int	do_message_off();
int	do_error_dialog(char *s1, char *s2, char *s3);
void    do_bell();
int	mbnavadjust_init_globals();
int	mbnavadjust_init(int argc,char **argv);
int	mbnavadjust_set_colors(int ncol, int *pixels);
int	mbnavadjust_set_borders(int *cn_brdr, int *cr_brdr, int *zc_brdr);
int	mbnavadjust_set_graphics(void *cn_xgid, void *cr_xgid, void *zc_xgid);
int	mbnavadjust_file_new(char *projectname);
int	mbnavadjust_file_open(char *projectname);
int	mbnavadjust_close_project();
int	mbnavadjust_write_project();
int	mbnavadjust_read_project();
int	mbnavadjust_import_data(char *path, int format);
int	mbnavadjust_import_file(char *path, int format);
int	mbnavadjust_findcrossings();
int	mbnavadjust_findcrossingsfile(int ifile);
int	mbnavadjust_poornav_file();
int	mbnavadjust_goodnav_file();
int	mbnavadjust_fixednav_file();
int	mbnavadjust_fixedxynav_file();
int	mbnavadjust_fixedznav_file();
int	mbnavadjust_set_tie_xy();
int	mbnavadjust_set_tie_z();
int	mbnavadjust_set_tie_xyz();
int	mbnavadjust_naverr_save();
int	mbnavadjust_naverr_specific(int new_crossing, int new_tie);
int	mbnavadjust_naverr_next();
int	mbnavadjust_naverr_previous();
int	mbnavadjust_naverr_nextunset();
int	mbnavadjust_naverr_selecttie();
int	mbnavadjust_naverr_addtie();
int	mbnavadjust_naverr_deletetie();
int	mbnavadjust_deletetie(int icrossing, int jtie, int deletestatus);
int	mbnavadjust_naverr_settie();
int	mbnavadjust_naverr_resettie();
int	mbnavadjust_naverr_checkoksettie();
int	mbnavadjust_naverr_skip();
int	mbnavadjust_naverr_unset();
int	mbnavadjust_crossing_load();
int	mbnavadjust_crossing_unload();
int	mbnavadjust_crossing_replot();
int	mbnavadjust_section_load(int file_id, int section_id, void **swathraw_ptr, void **swath_ptr, int num_pings);
int	mbnavadjust_section_translate(int file_id, void *swathraw_ptr, void *swath_ptr, double zoffset);
int	mbnavadjust_section_contour(int fileid, int sectionid, 
				struct swath *swath,
				struct mbna_contour_vector *contour);
int	mbnavadjust_naverr_snavpoints(int ix, int iy);
int	mbnavadjust_sections_intersect(int crossing_id);
int	mbnavadjust_crossing_overlap(int crossing_id);
int	mbnavadjust_crossing_overlapbounds(int crossing_id, 
				double offset_x, double offset_y,
				double *lonmin, double *lonmax, 
				double *latmin, double *latmax);
int	mbnavadjust_get_misfit();
int	mbnavadjust_get_misfitxy();
void 	plot(double x,double y,int ipen);
void 	newpen(int ipen);
void 	justify_string(double height,char *string, double *s);
void 	plot_string(double x, double y, double hgt, double angle, char *label);
void	mbnavadjust_naverr_scale();
void	mbnavadjust_naverr_plot(int plotmode);
int	mbnavadjust_autopick(int do_vertical);
int	mbnavadjust_zerozoffsets();
int	mbnavadjust_invertnav();
int	mbnavadjust_applynav();
int	mbnavadjust_interpolatesolution();
int	mbnavadjust_modelplot_plot();
int	mbnavadjust_set_modelplot_graphics(void *modp_xgid, int *modp_borders);
int	mbnavadjust_modelplot_plot_sequential();
int	mbnavadjust_modelplot_plot_tielist();
int	mbnavadjust_modelplot_pick(int x, int y);
int	mbnavadjust_modelplot_pick_tielist(int x, int y);
int	mbnavadjust_modelplot_pick_sequential(int x, int y);
int	mbnavadjust_modelplot_middlepick(int x, int y);
int	mbnavadjust_modelplot_setzoom();
int	mbnavadjust_modelplot_clearblock();
int	mbnavadjust_crossing_compare(void *a, void *b);

void	do_list_data_select( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_cont_expose( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_corr_expose( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_cont_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_corr_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zcorr_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_previous( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_next( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_nextunset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_addtie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_deletetie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_selecttie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_settie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_resettie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_unset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_setnone( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zerooffset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zerozoffset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_dismiss_naverr( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_minmisfit( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_minxymisfit( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_misfitcenter( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_new( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_open( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_close( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_importdata( Widget w, XtPointer client_data, XtPointer call_data);
void	do_quit( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_mode( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_ok( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_cancel( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showsurveys( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showdata( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showcrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showgoodcrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showtruecrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showties( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showallsurveys( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showselectedsurveys( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showselectedfile( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showselectedsection( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_autopick( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_autopickhorizontal( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_analyzecrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_checknewcrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_invertnav( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_list(Widget w, XtPointer client, XtPointer call);
int	do_wait_until_viewed(XtAppContext app);
void	set_label_string(Widget w, String str);
void	set_label_multiline_string(Widget w, String str);
void	get_text_string(Widget w, String str);
int	do_info_add(char *info, int timetag);
void	do_modelplot_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused);

XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/*--------------------------------------------------------------------*/
