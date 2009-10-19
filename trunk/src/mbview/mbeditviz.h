/*--------------------------------------------------------------------
 *    The MB-system:	mbeditviz.h		4/27/2007
 *    $Id: mbeditviz.h,v 5.2 2008/11/16 21:51:18 caress Exp $
 *
 *    Copyright (c) 2007 by
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
 *
 * MBeditviz is an interactive swath bathymetry editor and patch
 * test tool for  MB-System.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	April 27, 2007
 *
 * $Log: mbeditviz.h,v $
 * Revision 5.2  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.1  2007/11/16 17:26:56  caress
 * Progress on MBeditviz
 *
 * Revision 5.0  2007/06/17 23:24:12  caress
 * Added NBeditviz.
 *
 *
 */

/*--------------------------------------------------------------------*/

/* start this include */
#ifndef MB_EDITVIZ_DEF

/* header file flag */
#define	MB_EDITVIZ_DEF		1

#ifndef MB_STATUS_DEF
#include "../../include/mb_status.h"
#endif

#ifndef MB_DEFINE_DEF
#include "../../include/mb_define.h"
#endif

#ifndef MB_PROCESS_DEF
#include "mb_process.h"
#endif

#ifndef MB_INFO_DEF
#include "mb_info.h"
#endif

#ifdef MBEDITVIZ_DECLARE_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* MBeditviz defines */
#define MBEV_GRID_NONE		0
#define MBEV_GRID_NOTVIEWED	1
#define MBEV_GRID_VIEWED	2
#define MBEV_GRID_ALGORITH_SIMPLE	0
#define MBEV_GRID_ALGORITH_FOOTPRINT	1
#define MBEV_GRID_WEIGHT_TINY	0.0000001
#define MBEV_OUTPUT_MODE_EDIT	0
#define MBEV_OUTPUT_MODE_BROWSE	1
#define MBEV_ALLOC_NUM		24
#define MBEV_ALLOCK_NUM		1024
#define MBEV_NODATA		-10000000.0

/* usage of footprint based weight */
#define MBEV_USE_NO		0
#define MBEV_USE_YES		1
#define MBEV_USE_CONDITIONAL	2

/* mbeditviz structures */
struct	mbev_ping_struct
	{
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	navlonx;
	double	navlaty;
	double	portlon;
	double	portlat;
	double	stbdlon;
	double	stbdlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	ssv;
	int	beams_bath;
	char	*beamflag;
	char	*beamflagorg;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*bathcorr;
	double	*bathlon;
	double	*bathlat;
	double	*bathx;
	double	*bathy;
	double	*angles;
	double	*angles_forward;
	double	*angles_null;
	double	*ttimes;
	double	*bheave;
	double	*alongtrack_offset;
	};
struct mbev_file_struct
	{
	int	load_status;
	int	proc_status;
	char 	name[MB_PATH_MAXLINE];
	char 	path[MB_PATH_MAXLINE];
	int	format;
	int	raw_info_loaded;
	int	processed_info_loaded;
	struct mb_info_struct raw_info;
	struct mb_info_struct processed_info;
	struct mb_process_struct process;
	int	esf_open;
	char 	esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	int	num_pings;
	int	num_pings_alloc;
	struct mbev_ping_struct *pings;
	double	beamwidth_xtrack;
	double	beamwidth_ltrack;

	int	n_async_heading;
	int	n_async_heading_alloc;
	double	*async_heading_time_d;
	double	*async_heading_heading;
	int	n_async_sonardepth;
	int	n_async_sonardepth_alloc;
	double	*async_sonardepth_time_d;
	double	*async_sonardepth_sonardepth;
	int	n_async_attitude;
	int	n_async_attitude_alloc;
	double	*async_attitude_time_d;
	double	*async_attitude_roll;
	double	*async_attitude_pitch;
	int	n_sync_attitude;
	int	n_sync_attitude_alloc;
	double	*sync_attitude_time_d;
	double	*sync_attitude_roll;
	double	*sync_attitude_pitch;
	};
struct mbev_grid_struct
	{
	int	status;
	char	projection_id[MB_PATH_MAXLINE];
	void	*pjptr;
	double	bounds[4];
	double	boundsutm[4];
	double	dx;
	double	dy;
	int	nx;
	int	ny;
	double	min;
	double	max;
	double	smin;
	double	smax;
	float	nodatavalue;
	float	*sum;
	float	*wgt;
	float	*val;
	float	*sgm;
	};		

/*--------------------------------------------------------------------*/

/* mbeditviz global control parameters */

/* status parameters */
EXTERNAL int	mbev_status;
EXTERNAL int	mbev_error;
EXTERNAL int	mbev_verbose;

/* gui parameters */
EXTERNAL int	mbev_message_on;

/* mode parameters */
EXTERNAL int	mbev_mode_output;
EXTERNAL int	mbev_grid_algorithm;

/* data parameters */
EXTERNAL int	mbev_num_files;
EXTERNAL int	mbev_num_files_alloc;
EXTERNAL int	mbev_num_files_loaded;
EXTERNAL int	mbev_num_pings_loaded;
EXTERNAL int	mbev_num_soundings_loaded;
EXTERNAL double mbev_bounds[4];
EXTERNAL struct mbev_file_struct *mbev_files;
EXTERNAL struct mbev_grid_struct mbev_grid;
EXTERNAL int	mbev_instance;

/* gridding parameters */
EXTERNAL double	mbev_grid_bounds[4];
EXTERNAL double	mbev_grid_boundsutm[4];
EXTERNAL double mbev_grid_cellsize;
EXTERNAL int mbev_grid_nx;
EXTERNAL int mbev_grid_ny;

/* global patch test parameters */
EXTERNAL double	mbev_rollbias;
EXTERNAL double	mbev_pitchbias;
EXTERNAL double	mbev_headingbias;
EXTERNAL double	mbev_timelag;
EXTERNAL double	mbev_rollbias_3dsdg;
EXTERNAL double	mbev_pitchbias_3dsdg;
EXTERNAL double	mbev_headingbias_3dsdg;
EXTERNAL double	mbev_timelag_3dsdg;

/* selected sounding parameters */
EXTERNAL struct mb3dsoundings_struct mbev_selected;

void mbeditviz_mb3dsoundings_dismiss();
void mbeditviz_mb3dsoundings_edit(int ifile, int iping, int ibeam, char beamflag, int flush);
void mbeditviz_mb3dsoundings_info(int ifile, int iping, int ibeam, char *infostring);
void mbeditviz_mb3dsoundings_bias(double rollbias, double pitchbias, double headingbias, double timelag);
void mbeditviz_mb3dsoundings_biasapply(double rollbias, double pitchbias, double headingbias, double timelag);

/* end this include */
#endif

/*--------------------------------------------------------------------*/
