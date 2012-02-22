/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit.h	6/24/95
 *    $Id$
 *
 *    Copyright (c) 1995, 2000, 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *    David W. Caress (caress@mbari.org)
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
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
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01 22:56:26  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/09/30  07:04:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  2000/09/30  07:04:05  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  2000/08/28  22:45:11  caress
 * About to kick off new version.
 *
 * Revision 4.5  1999/04/09 22:34:08  caress
 * Added time interval plot.
 *
 * Revision 4.4  1997/04/21  17:07:38  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1996/08/26  17:33:29  caress
 * Release 4.4 revision.
 *
 * Revision 4.3  1996/08/26  17:33:29  caress
 * Release 4.4 revision.
 *
 * Revision 4.2  1996/04/05  20:07:02  caress
 * Added GUI mode so done means quit for real. Also changed done and
 * quit handling in browse mode so that the program doesn't read the
 * entire data file before closing it.
 *
 * Revision 4.1  1995/08/17  14:59:39  caress
 * Revision for release 4.3.
 *
 * Revision 4.0  1995/08/07  18:33:22  caress
 * First cut.
 *
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
EXTERNAL int	gui_mode;
EXTERNAL int	data_show_max;
EXTERNAL int	data_show_size;
EXTERNAL int	data_step_max;
EXTERNAL int	data_step_size;
EXTERNAL int	mode_pick;
EXTERNAL int	mode_set_interval;
EXTERNAL int	plot_tint;
EXTERNAL int	plot_tint_org;
EXTERNAL int	plot_lon;
EXTERNAL int	plot_lon_org;
EXTERNAL int	plot_lon_dr;
EXTERNAL int	plot_lat;
EXTERNAL int	plot_lat_org;
EXTERNAL int	plot_lat_dr;
EXTERNAL int	plot_speed;
EXTERNAL int	plot_speed_org;
EXTERNAL int	plot_smg;
EXTERNAL int	plot_heading;
EXTERNAL int	plot_heading_org;
EXTERNAL int	plot_cmg;
EXTERNAL int	plot_roll;
EXTERNAL int	plot_pitch;
EXTERNAL int	plot_heave;
EXTERNAL int	drift_lon;
EXTERNAL int	drift_lat;
EXTERNAL int	time_fix;
EXTERNAL int	use_ping_data;
EXTERNAL int	format;
EXTERNAL char	ifile[128];
EXTERNAL char	ofile[128];
EXTERNAL char	nfile[128];
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
#define	OUTPUT_MODE_NAV		1
#define	OUTPUT_MODE_BROWSE	2
#define	PLOT_TINT	0
#define	PLOT_LONGITUDE	1
#define	PLOT_LATITUDE	2
#define	PLOT_SPEED	3
#define	PLOT_HEADING	4
#define	PLOT_ROLL	5
#define	PLOT_PITCH	6
#define	PLOT_HEAVE	7
/*--------------------------------------------------------------------*/
