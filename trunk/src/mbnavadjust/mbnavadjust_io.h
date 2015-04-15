/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_io.h	4/18/2014
 *    $Id$
 
 *    Copyright (c) 2014-2015 by
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
 * Mbnavadjustmerge merges two existing mbnavadjust projects. The result
 * can be to add one project to another or to create a new, third project
 * combining the two source projects.
 *
 * Author:	D. W. Caress
 * Date:	April 18, 2014
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

/* mbnavadjust global defines */
#define STRING_MAX 			MB_PATH_MAXLINE
#define BUFFER_MAX 			1024
#define ALLOC_NUM			10
#define MBNA_SNAV_NUM			11
#define	MBNA_STATUS_GUI			0
#define	MBNA_STATUS_AUTOPICK		1
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
#define	MBNA_TIE_NONE			0
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
#define MBNA_VIEW_LIST_MEDIOCRECROSSINGS	4
#define MBNA_VIEW_LIST_GOODCROSSINGS	5
#define MBNA_VIEW_LIST_BETTERCROSSINGS	6
#define MBNA_VIEW_LIST_TRUECROSSINGS	7
#define MBNA_VIEW_LIST_TIES		8
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

#define	MBNA_MODELPLOT_TIMESERIES	0
#define	MBNA_MODELPLOT_PERTURBATION	1
#define	MBNA_MODELPLOT_TIEOFFSETS	2
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

/* minimum initial sigma_crossing (meters) */
#define	SIGMA_MINIMUM	0.1

/* ping type defines */
#define	SIDE_PORT	0
#define	SIDE_STBD	1
#define	SIDE_FULLSWATH	2

/* route version define */
#define ROUTE_VERSION "1.00"

/* route color defines (colors different in MBgrdviz than in MBnavadjust) */
#define	ROUTE_COLOR_BLACK			0
#define	ROUTE_COLOR_WHITE			1
#define	ROUTE_COLOR_RED			2
#define	ROUTE_COLOR_YELLOW		3
#define	ROUTE_COLOR_GREEN			4
#define	ROUTE_COLOR_BLUEGREEN		5
#define	ROUTE_COLOR_BLUE			6
#define	ROUTE_COLOR_PURPLE		7

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
	int	show_in_modelplot;
	int	modelplot_start_count;
	int	contoursuptodate;
	int	global_tie_status;
	int	global_tie_snav;
	double	global_tie_offset_x;
	double	global_tie_offset_y;
	double	global_tie_offset_x_m;
	double	global_tie_offset_y_m;
	double	global_tie_offset_z_m;
	double	global_tie_xsigma;
	double	global_tie_ysigma;
	double	global_tie_zsigma;
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
	int	show_in_modelplot;
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
	char	logfile[STRING_MAX];
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
    
int mbnavadjust_new_project(int verbose, char *projectpath,
                            double section_length,
                            int	section_soundings,
                            double cont_int,
                            double col_int,
                            double tick_int,
                            double label_int,
                            int	decimation,
                            double smoothing,
                            double zoffsetwidth,
                            struct mbna_project *project, int *error);
int mbnavadjust_read_project(int verbose, char *projectpath,
                                struct mbna_project *project, int *error);
int mbnavadjust_write_project(int verbose, struct mbna_project *project,
                                int *error);
int mbnavadjust_close_project(int verbose, struct mbna_project *project,
                                int *error);





/*--------------------------------------------------------------------*/
