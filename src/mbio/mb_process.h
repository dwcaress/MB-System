/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.h	9/11/00
 *    $Id: mb_process.h,v 4.0 2000-09-30 06:28:42 caress Exp $
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
 * MBprocess is a tool for processing swath sonar bathymetry data.  
 * This program performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit mask files or edit save
 *     files.
 * This file defines processing parameters and structures 
 * saved in mbprocess parameter files. The reading, writing, and
 * updating of the mbprocess control parameter values are done
 * using functions in mb_process.c
 *
 * The parameters controlling mbprocess are included in an ascii
 * parameter file with the following possible entries:
 *   FORMAT format                  # sets format id\n\
 *   INFILE file                    # sets input file path
 *   OUTFILE file                   # sets output file path
 *   DRAFT draft                    # sets draft value (m)
 *   DRAFTOFFSET offset             # sets value added to draft (m)
 *   DRAFTMULTIPLY multiplier       # sets value multiplied by draft
 *   ROLLBIAS                       # sets roll bias (degrees)
 *   ROLLBIASPORT                   # sets port roll bias (degrees)
 *   ROLLBIASSTBD                   # sets starboard roll bias (degrees)
 *   PITCHBIAS                      # sets pitch bias
 *   NAVADJFILE file                # sets adjusted navigation file path
 *                                  # - this file supercedes nav file for
 *                                  #   lon and lat only
 *                                  # - uses mbnavadjust output
 *   NAVADJSPLINE                   # sets spline adjusted navigation interpolation
 *   NAVFILE file                   # sets navigation file path
 *   NAVFORMAT format               # sets navigation file format
 *   NAVHEADING                     # sets heading to be merged from nav file
 *   NAVSPEED                       # sets speed to be merged from nav file
 *   NAVDRAFT                       # sets draft to be merged from nav file
 *   NAVSPLINE                      # sets spline navigation interpolation
 *   HEADING                        # sets heading to course made good
 *   HEADINGOFFSET offset           # sets value added to heading (degree)
 *   SVPFILE file                   # sets svp file path
 *   SSV                            # sets ssv value (m/s)
 *   SSVOFFSET                      # sets value added to ssv (m/s)
 *   UNCORRECTED                    # sets raytraced bathymetry to "uncorrected" values
 *   EDITSAVEFILE                   # sets edit save file path (from mbedit)
 *   EDITMASKFILE                   # sets edit mask file path (from mbmask)
 *
 * MBprocess and its associated functions and programs use
 * the following file naming convention. The unprocessed swath
 * data file should have a name like "fileroot.mbxxx", where
 * fileroot can be anything and xxx corresponds to the MB-System
 * format id number. Given this kind of filename, we then have:
 *	fileroot.mbxxx.par	    : parameter file
 *	fileroot.mbxxx.nve	    : edited navigation from 
 *					mbnavedit
 *	fileroot.mbxxx.nvX	    : adjusted navigation from
 *					mbnavadjust, with 
 *					X={0,1,2,3,4,5,6,7,8,9}
 *	fileroot.mbxxx.esf	    : bathymetry edit save file 
 *					from mbedit
 *
 * 
 *
 * Author:	D. W. Caress
 * Date:	September 11, 2000
 *
 * $Log: not supported by cvs2svn $
 *
 */

/* include mb_io.h if needed */
#ifndef MB_IO_DEF
#include "mb_io.h"
#endif

/* mbprocess value defines */
#define MBP_FILENAMESIZE	MB_PATH_MAXLINE
#define MBP_BATHRECALC_OFF	0
#define MBP_BATHRECALC_RAYTRACE	1
#define MBP_BATHRECALC_ROTATE	2
#define MBP_BATHRECALC_OFFSET	3
#define MBP_ROLLBIAS_OFF	0
#define MBP_ROLLBIAS_SINGLE	1
#define MBP_ROLLBIAS_DOUBLE	2
#define MBP_PITCHBIAS_OFF	0
#define MBP_PITCHBIAS_ON	1
#define MBP_DRAFT_OFF		0
#define MBP_DRAFT_OFFSET	1
#define MBP_DRAFT_MULTIPLY	2
#define MBP_DRAFT_MULTIPLYOFFSET	3
#define MBP_DRAFT_SET		4
#define MBP_NAV_OFF		0
#define MBP_NAV_ON		1
#define MBP_NAV_LINEAR		0
#define MBP_NAV_SPLINE		1
#define MBP_HEADING_OFF         0
#define MBP_HEADING_CALC        1
#define MBP_HEADING_OFFSET      2
#define MBP_SSV_OFF		0
#define MBP_SSV_OFFSET		1
#define MBP_SSV_SET		2
#define MBP_SSV_CORRECT		1
#define MBP_SSV_INCORRECT	2
#define MBP_SVP_OFF		0
#define MBP_SVP_ON		1
#define MBP_EDIT_OFF		0
#define MBP_EDIT_ON		1
#define	MBP_EDIT_FLAG		1
#define	MBP_EDIT_UNFLAG		2
#define	MBP_EDIT_ZERO		3
#define	MBP_EDIT_FILTER		4
#define MBP_MASK_OFF		0
#define MBP_MASK_ON		1

struct mb_process_struct 
	{
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	int	mbp_ofile_specified;
	char	mbp_ofile[MBP_FILENAMESIZE];
	int	mbp_format_specified;
	int	mbp_format;
	int	mbp_bathrecalc_mode;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	int	mbp_pitchbias_mode;
	double	mbp_pitchbias;
	int	mbp_draft_mode;
	double	mbp_draft;
	double	mbp_draft_mult;
	char	mbp_dfile[MBP_FILENAMESIZE];
	int	mbp_ssv_mode;
	double	mbp_ssv;
	int	mbp_svp_mode;
	char	mbp_svpfile[MBP_FILENAMESIZE];
	int	mbp_uncorrected;
	int	mbp_navadj_mode;
	char	mbp_navadjfile[MBP_FILENAMESIZE];
	int	mbp_navadj_algorithm;
	int	mbp_nav_mode;
	char	mbp_navfile[MBP_FILENAMESIZE];
	int	mbp_nav_format;
	int	mbp_nav_heading;
	int	mbp_nav_speed;
	int	mbp_nav_draft;
	int	mbp_nav_algorithm;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_edit_mode;
	char	mbp_editfile[MBP_FILENAMESIZE];
	int	mbp_mask_mode;
	char	mbp_maskfile[MBP_FILENAMESIZE];
	};
