/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust.h	6/24/95
 *    $Id: mbnavadjust.h,v 4.0 2000-09-30 07:00:06 caress Exp $
 *
 *    Copyright (c) 2000 by
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
 * $Log: not supported by cvs2svn $
 *
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVADJUST_DECLARE_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* mbnavadjust global defines */
#define STRING_MAX 			512
#define BUFFER_MAX 			1024
#define ALLOC_NUM			10
#define MBNA_SNAV_NUM			11
#define	MBNA_STATUS_GUI			0
#define	MBNA_STATUS_NAVERR		1
#define	MBNA_STATUS_NAVSOLVE		2
#define	MBNA_INVERSION_NONE		0
#define	MBNA_INVERSION_OLD		1
#define	MBNA_INVERSION_CURRENT		2
#define	MBNA_FILE_OK			1
#define	MBNA_FILE_FIXED			2
#define	MBNA_CROSSING_STATUS_NONE	0
#define	MBNA_CROSSING_STATUS_SET	1
#define	MBNA_CROSSING_STATUS_SKIP	2
#define MBNA_TIME_GAP_MAX		120.0
#define MBNA_VIEW_LIST_FILES		0
#define MBNA_VIEW_LIST_CROSSINGS	1
#define MBNA_VIEW_LIST_TIES		2
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
	double	snav_distance[MBNA_SNAV_NUM];
	double	snav_time_d[MBNA_SNAV_NUM];
	double	snav_lon[MBNA_SNAV_NUM];
	double	snav_lat[MBNA_SNAV_NUM];
	double	snav_lon_offset[MBNA_SNAV_NUM];
	double	snav_lat_offset[MBNA_SNAV_NUM];
};
struct mbna_file {
	int	status;
	int	id;
	int	output_id;
	char 	file[STRING_MAX];
	int	format;
	int	num_sections;
	int	num_sections_alloc;
	struct mbna_section *sections;
};
struct mbna_tie {
	int	snav_1;
	double	snav_1_time_d;
	int	snav_2;
	double	snav_2_time_d;
	double	offset_x;
	double	offset_y;
	double	offset_x_m;
	double	offset_y_m;
	int	inversion_status;
	double	inversion_offset_x;
	double	inversion_offset_y;
	double	inversion_offset_x_m;
	double	inversion_offset_y_m;
};
struct mbna_crossing {
	int	status;
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
	int	num_files_good;
	int	num_files_alloc;
	struct mbna_file *files;
	int	num_crossings;
	int	num_crossings_good;
	int	num_crossings_alloc;
	int	num_crossings_analyzed;
	struct mbna_crossing *crossings;
	int	num_ties;
	double	section_length;
	double	cont_int;
	double	col_int;
	double	tick_int;
	double	label_int;
	int	decimation;
	int	inversion;
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
EXTERNAL int	mbna_file_select;
EXTERNAL int	mbna_crossing_select;
EXTERNAL int	mbna_tie_select;
EXTERNAL int	mbna_current_crossing;
EXTERNAL int	mbna_current_tie;
EXTERNAL int	mbna_total_num_pings;
EXTERNAL int	mbna_total_num_snavs;
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
EXTERNAL double	mbna_invert_offset_x;
EXTERNAL double	mbna_invert_offset_y;
EXTERNAL double	mbna_offset_x_old;
EXTERNAL double	mbna_offset_y_old;
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
EXTERNAL double	mbna_plotx_scale;
EXTERNAL double	mbna_ploty_scale;
EXTERNAL int	mbna_misfit_center;
EXTERNAL double	mbna_misfit_lon_min;
EXTERNAL double	mbna_misfit_lon_max;
EXTERNAL double	mbna_misfit_lat_min;
EXTERNAL double	mbna_misfit_lat_max;
EXTERNAL double	mbna_misfit_scale;
EXTERNAL double mbna_misfit_offset_x;
EXTERNAL double mbna_misfit_offset_y;
EXTERNAL double mbna_minmisfit_offset_x;
EXTERNAL double mbna_minmisfit_offset_y;
EXTERNAL int	mbna_zoom_x1;
EXTERNAL int	mbna_zoom_y1;
EXTERNAL int	mbna_zoom_x2;
EXTERNAL int	mbna_zoom_y2;
EXTERNAL double mbna_smoothweight;
EXTERNAL double mbna_offsetweight;

/* plot vector data */
EXTERNAL struct mbna_contour_vector *mbna_contour;
EXTERNAL struct mbna_contour_vector mbna_contour1;
EXTERNAL struct mbna_contour_vector mbna_contour2;

/* mbnavadjust global project parameters */
EXTERNAL struct	mbna_project project;

/* function prototype definitions */
void	do_mbnavadjust_init(int argc, char **argv);
void	do_set_controls();
int	do_update_status();
void	do_naverr_init();
void	do_update_naverr();
void	do_naverr_offsetlabel();
void	do_naverr_test_graphics();
void	do_naverr_plot();
int	do_message_on(char *message);
int	do_message_off();
int	do_error_dialog(char *s1, char *s2, char *s3);
void    do_bell();
int	mbnavadjust_init_globals();
int	mbnavadjust_init(int argc,char **argv,int *startup_file);
int	mbnavadjust_set_graphics(int cn_xgid, int cr_xgid, 
				int *cn_brdr, int *cr_brdr,  
				int ncol, int *pixels);
int	mbnavadjust_file_new(char *projectname);
int	mbnavadjust_file_open(char *projectname);
int	mbnavadjust_close_project();
int	mbnavadjust_write_project();
int	mbnavadjust_read_project();
int	mbnavadjust_import_data(char *path, int format);
int	mbnavadjust_import_file(char *path, int format);
int	mbnavadjust_naverr_specific(int new_crossing, int new_tie);
int	mbnavadjust_naverr_next();
int	mbnavadjust_naverr_previous();
int	mbnavadjust_naverr_nextunset();
int	mbnavadjust_naverr_addtie();
int	mbnavadjust_naverr_deletetie();
int	mbnavadjust_naverr_settie();
int	mbnavadjust_naverr_resettie();
int	mbnavadjust_naverr_skip();
int	mbnavadjust_crossing_load();
int	mbnavadjust_crossing_unload();
int	mbnavadjust_section_load(char *fpath, void **swath_ptr, int num_pings);
void 	plot(double x,double y,int ipen);
void 	newpen(int ipen);
void 	justify_string(double height,char *string, double *s);
void 	plot_string(double x, double y, double hgt, double angle, char *label);
void	mbnavadjust_naverr_plot(int plotmode);

/*--------------------------------------------------------------------*/
