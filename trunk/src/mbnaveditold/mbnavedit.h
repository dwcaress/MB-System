/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.h	6/24/95
 *    $Id: mbnavedit.h,v 4.0 1995-08-07 18:33:22 caress Exp $
 *
 *    Copyright (c) 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 *
 * $Log: not supported by cvs2svn $
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_YES
#include "mb_status.h"
#endif

#ifdef MBNAVEDIT_DECLARE_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* mbnavedit global control parameters */
EXTERNAL int	output_mode;
EXTERNAL int	data_show_max;
EXTERNAL int	data_show_size;
EXTERNAL int	data_step_max;
EXTERNAL int	data_step_size;
EXTERNAL int	mode_pick;
EXTERNAL int	mode_set_interval;
EXTERNAL int	plot_lon;
EXTERNAL int	plot_lon_org;
EXTERNAL int	plot_lat;
EXTERNAL int	plot_lat_org;
EXTERNAL int	plot_speed;
EXTERNAL int	plot_speed_org;
EXTERNAL int	plot_smg;
EXTERNAL int	plot_heading;
EXTERNAL int	plot_heading_org;
EXTERNAL int	plot_cmg;
EXTERNAL int	format;
EXTERNAL char	ifile[128];
EXTERNAL char	ofile[128];
EXTERNAL int	ofile_defined;

/* mbnavedit plot size parameters */
EXTERNAL int	plot_width;
EXTERNAL int	plot_height;
EXTERNAL int	number_plots;

/* Mode value defines */
#define	PICK_MODE_PICK		0
#define	PICK_MODE_SELECT	1
#define	PICK_MODE_DESELECT	2
#define	PICK_MODE_SELECTALL	3
#define	PICK_MODE_DESELECTALL	4
#define	OUTPUT_MODE_OUTPUT	0
#define	OUTPUT_MODE_BROWSE	1
#define	PLOT_LONGITUDE	0
#define	PLOT_LATITUDE	1
#define	PLOT_SPEED	2
#define	PLOT_HEADING	3
/*--------------------------------------------------------------------*/
