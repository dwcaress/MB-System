/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.h	9/11/00
 *    $Id: mb_process.h,v 5.3 2001-06-01 00:14:06 caress Exp $
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
 *   - applying bathymetry edits from edit save files.
 * This file defines processing parameters and structures 
 * saved in mbprocess parameter files. The reading, writing, and
 * updating of the mbprocess control parameter values are done
 * using functions in mb_process.c
 *
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter files consist of single line
 * commands and comment lines beginning with '#'. The commands
 * are given by a keyword and a corresponding value (separated by
 * spaces or tabs). A list of the keywords and values follows, 
 * with default values in square brackets:
 *
 * GENERAL PARAMETERS:
 *   EXPLICIT			    # causes mbprocess to set modes implicitely
 *                                  # - e.g. the SVPFILE command will also set 
 *                                  #   raytracing on even if the RAYTRACE command 
 *                                  #   is not given [explicit mode commands required] 
 *   FORMAT constant                # sets format id [no default]
 *   INFILE filename                # sets input file path [no default]
 *   OUTFILE filename               # sets output file path [no default]
 *
 * NAVIGATION MERGING:
 *   NAVMODE boolean                # sets navigation merging [0]
 *                                  #   0: navigation merge off
 *                                  #   1: navigation merge on
 *   NAVFILE filename               # sets navigation file path [no default]
 *   NAVFORMAT constant             # sets navigation file format [9]
 *   NAVHEADING boolean             # sets heading to be merged from navigation file
 *                                  # - note: heading merged from navigation before 
 *                                  #   heading correction applied
 *                                  #   0: heading not changed
 *                                  #   1: heading merged from navigation file
 *   NAVSPEED boolean               # sets speed to be merged from navigation file
 *                                  #   0: speed not changed
 *                                  #   1: speed merged from navigation file
 *   NAVDRAFT boolean               # sets draft to be merged from navigation file
 *                                  # - note: draft merged from navigation before 
 *                                  #   draft correction applied
 *                                  #   0: draft not changed
 *                                  #   1: draft merged from navigation file
 *   NAVINTERP boolean              # sets navigation interpolation algorithm [0]
 *                                  #   0: linear interpolation (recommended)
 *                                  #   1: spline interpolation
 *   NAVTIMESHIFT constant          # sets navigation time shift (seconds) [0.0]
 *                                  # - note: time shift added to timestamps of 
 *                                  #   navigation fixes read in from NAVFILE 
 *                                  #   prior to merging
 *   NAVSHIFT boolean               # sets navigation offset [0]
 *                                  # - note: offsets applied to navigation read
 *                                  #   in from NAVFILE prior to merging
 *   NAVOFFSETX constant            # sets navigation athwartship offset (meters) [0.0]
 *                                  # - note: the effective navigation shift is
 *                                  #   (NAVOFFSETX - SONAROFFSETX), and the 
 *                                  #   navigation is corrected by subtracting 
 *                                  #   this effective shift.
 *                                  # - note: athwartship shift is positive to 
 *                                  #   starboard.
 *   NAVOFFSETY constant            # sets navigation fore-aft offset (meters) [0.0]
 *                                  # - note: the effective navigation shift is
 *                                  #   (NAVOFFSETY - SONAROFFSETY), and the 
 *                                  #   navigation is corrected by subtracting 
 *                                  #   this effective shift.
 *                                  # - note: fore-aft shift is positive forward.
 *   NAVOFFSETZ constant            # sets navigation vertical offset (meters) [0.0]
 *                                  # - note: this value is not yet used for
 *                                  #   anything.
 *                                  # - note: vertical shift is positive down.
 *
 * ADJUSTED NAVIGATION MERGING:
 *   NAVADJMODE boolean             # sets navigation merging from mbnavadjust [0]
 *                                  # - longitude and latitude only
 *                                  #   0: adjusted navigation merge off
 *                                  #   1: adjusted navigation merge on
 *   NAVADJFILE filename            # sets adjusted navigation file path
 *                                  # - this file supercedes navigation file for
 *                                  #   lon and lat only
 *                                  # - uses mbnavadjust output
 *   NAVADJINTERP boolean           # sets adjusted navigation interpolation algorithm [0]
 *                                  #   0: linear interpolation (recommended)
 *                                  #   1: spline interpolation
 *
 * BATHYMETRY EDITING:
 *   EDITSAVEMODE boolean           # turns on reading edit save file (from mbedit) [0]
 *   EDITSAVEFILE filename          # sets edit save file path (from mbedit) [none]
 *
 * BATHYMETRY RECALCULATION:
 *   RAYTRACE boolean               # sets bathymetry recalculation by raytracing [0]
 *                                  #  0: raytracing off
 *                                  #  1: raytracing on
 *   SVPFILE filename               # sets svp file path [no default]
 *   SSVMODE mode                   # sets ssv mode [0]
 *                                  #  0: use ssv from file
 *                                  #  1: offset ssv from file (set by SSV)
 *                                  #  2: use constant ssv (set by SSV)
 *   SSV constant/offset            # sets ssv value or offset (m/s) [1500.0]
 *   TTMODE mode                    # sets handling of travel times [0]
 *                                  #  0: do not change travel times.
 *                                  #  1: travel time correction by multiply
 *   TTMULTIPLY multiplier          # sets value multiplied by travel times [1.0]
 *   ANGLEMODE mode                 # sets handling of beam angles [0]
 *                                  #  0: do not change beam angles
 *                                  #  1: adjust beams angles by Snell's law
 *                                  #     ignoring sonar array geometry
 *                                  #  2: adjust beams angles by Snell's law
 *                                  #     using array geometry
 *   CORRECTED boolean              # sets raytraced bathymetry to "corrected" values [1]
 *
 * DRAFT CORRECTION:
 *   DRAFTMODE mode                 # sets draft correction [0]
 *                                  # - note: draft merged from navigation before 
 *                                  #   draft correction applied
 *                                  #   0: no draft correction
 *                                  #   1: draft correction by offset 
 *                                  #   2: draft correction by multiply
 *                                  #   3: draft correction by offset and multiply
 *                                  #   4: draft set to constant
 *   DRAFT constant                 # sets draft value (m) [0.0]
 *   DRAFTOFFSET offset             # sets value added to draft (m) [0.0]
 *   DRAFTMULTIPLY multiplier       # sets value multiplied by draft [1.0]
 *
 * HEAVE CORRECTION:
 *   HEAVEMODE mode                 # sets heave correction [0]
 *                                  #   0: no heave correction
 *                                  #   1: heave correction by offset
 *                                  #   2: heave correction by multiply
 *                                  #   3: heave correction by offset and multiply
 *   HEAVEOFFSET offset             # sets value added to heave (m) 
 *   HEAVEMULTIPLY multiplier       # sets value multiplied by heave
 *
 * LEVER CORRECTION:
 *   LEVERMODE mode                 # sets heave correction by lever correction [0]
 *                                  #   0: no heave correction by lever calculation
 *                                  #   1: heave correction by lever calculation
 *				    		# - note: lever calculation finds heave at the
 *                                  #   sonar head location by adding motion
 *                                  #   inferred from roll and pitch projected
 *                                  #   from the vru location.
 *   VRUOFFSETX constant            # sets vru athwartship offset (meters) [0.0]
 *                                  # - note: the effective athwartship distance
 *                                  #   between the vru and the sonar head is  
 *                                  #   (VRUOFFSETX - SONAROFFSETX), and the 
 *                                  #   lever calculation is made using this 
 *                                  #   effective distance.
 *                                  # - note: athwartship distance is positive to 
 *                                  #   starboard.
 *   VRUOFFSETY constant            # sets vru fore-aft offset (meters) [0.0]
 *                                  # - note: the effective fore-aft distance
 *                                  #   between the vru and the sonar head is  
 *                                  #   (VRUOFFSETY - SONAROFFSETY), and the 
 *                                  #   lever calculation is made using this 
 *                                  #   effective distance.
 *                                  # - note: fore-aft distance is positive forward.
 *   VRUOFFSETZ constant            # sets vru vertical offset (meters) [0.0]
 *                                  # - note: the effective vertical distance
 *                                  #   between the vru and the sonar head is  
 *                                  #   (VRUOFFSETZ - SONAROFFSETZ), and the 
 *                                  #   lever calculation is made using this 
 *                                  #   effective distance.
 *                                  # - note: vertical distance is positive down.
 *   SONAROFFSETX constant          # sets sonar athwartship offset (meters) [0.0]
 *                                  # - note: this value is used for both 
 *                                  #   navigation shifts and lever calculations.
 *                                  # - note: athwartship distance is positive to 
 *                                  #   starboard.
 *   SONAROFFSETY constant          # sets vru fore-aft offset (meters) [0.0]
 *                                  # - note: this value is used for both 
 *                                  #   navigation shifts and lever calculations.
 *                                  # - note: fore-aft distance is positive forward.
 *   SONAROFFSETZ constant          # sets vru vertical offset (meters) [0.0]
 *                                  # - note: this value is used for lever 
 *                                  #   calculations.
 *                                  # - note: vertical distance is positive down.
 *
 * ROLL CORRECTION:
 *   ROLLBIASMODE mode              # sets roll correction [0]
 *                                  #   0: no roll correction
 *                                  #   1: roll correction by single roll bias
 *                                  #   2: roll correction by separate port and starboard roll bias
 *   ROLLBIAS offset                # sets roll bias (degrees)
 *   ROLLBIASPORT offset            # sets port roll bias (degrees)
 *   ROLLBIASSTBD offset            # sets starboard roll bias (degrees)
 *
 * PITCH CORRECTION:
 *   PITCHBIASMODE mode             # sets pitch correction [0]
 *                                  #   0: no pitch correction
 *                                  #   1: pitch correction by pitch bias
 *   PITCHBIAS offset               # sets pitch bias (degrees)
 *
 * HEADING CORRECTION:
 *   HEADINGMODE mode               # sets heading correction [no heading correction]
 *                                  # - note: heading merged from navigation before 
 *                                  #   heading correction applied
 *                                  #   0: no heading correction
 *                                  #   1: heading correction using course made good
 *                                  #   2: heading correction by offset
 *                                  #   3: heading correction using course made good and offset
 *   HEADINGOFFSET offset           # sets value added to heading (degrees)
 *
 * TIDE CORRECTION:
 *   TIDEMODE mode                  # sets tide correction [0]
 *                                  # - note: tide added to bathymetry after
 *                                  #   all other calculations and corrections
 *                                  #   0: tide correction off
 *                                  #   1: tide correction on
 *   TIDEFILE filename              # sets tide file path
 *   TIDEFORMAT constant            # sets tide file format [1]
 *
 * SIDESCAN RECALCULATION:
 *   SSRECALCMODE  boolean          # sets recalculation of sidescan for Simrad multibeam data
 *                                  #   0: sidescan recalculation off
 *                                  #   1: sidescan recalculation on
 *   SSPIXELSIZE constant           # sets recalculated sidescan pixel size (m) [0.0]
 *                                  # - a zero value causes the pixel size to be recalculated
 *                                  #   for every data record
 *   SSSWATHWIDTH  constant         # sets sidescan swath width (degrees) [0.0]
 *                                  # - a zero value causes the swath width to be recalculated
 *                                  #   for every data record
 *   SSINTERPOLATE  constant        # sets sidescan interpolation distance (number of pixels)
 *
 * METADATA INSERTION:
 *   METAOPERATOR  string           # sets mbinfo metadata string for survey operator institution or company
 *   METAPLATFORM  string           # sets mbinfo metadata string for platform (ship or vehicle)
 *   METASONAR     string           # sets mbinfo metadata string for sonar model name
 *   METASURVEY    string           # sets mbinfo metadata string for survey name
 *   METAPI        string           # sets mbinfo metadata string for principal investigator
 *   METACLIENT    string           # sets mbinfo metadata string for survey client institution or company
 *
 * PROCESSING KLUGES:
 *   KLUGE001                       # processing kluge 001 (not yet defined)
 *                                  # - occasionaly odd processing problems will occur that
 *                                  #   are specific to a particular survey or sonar version
 *                                  # - mbprocess will allow one-time fixes to be defined
 *                                  #   as "kluges" that can be turned on throught the parameter
 *                                  #   files.
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
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.0  2000/09/30  06:28:42  caress
 * Snapshot for Dale.
 *
 *
 */

/* include mb_io.h if needed */
#ifndef MB_IO_DEF
#include "mb_io.h"
#endif

/* mbprocess value defines */
#define MBP_FILENAMESIZE	MB_PATH_MAXLINE
#define MBP_NAV_OFF		0
#define MBP_NAV_ON		1
#define MBP_NAV_LINEAR		0
#define MBP_NAV_SPLINE		1
#define MBP_EDIT_OFF		0
#define MBP_EDIT_ON		1
#define MBP_EDIT_FLAG		1
#define MBP_EDIT_UNFLAG		2
#define MBP_EDIT_ZERO		3
#define MBP_EDIT_FILTER		4
#define MBP_BATHRECALC_OFF	0
#define MBP_BATHRECALC_RAYTRACE	1
#define MBP_BATHRECALC_ROTATE	2
#define MBP_BATHRECALC_OFFSET	3
#define MBP_SVP_OFF		0
#define MBP_SVP_ON		1
#define MBP_SSV_OFF		0
#define MBP_SSV_OFFSET		1
#define MBP_SSV_SET		2
#define MBP_TT_OFF		0
#define MBP_TT_MULTIPLY		1
#define	MBP_ANGLES_OK		0
#define	MBP_ANGLES_SNELL	1
#define	MBP_ANGLES_SNELLNULL	2
#define MBP_DRAFT_OFF		0
#define MBP_DRAFT_OFFSET	1
#define MBP_DRAFT_MULTIPLY	2
#define MBP_DRAFT_MULTIPLYOFFSET	3
#define MBP_DRAFT_SET		4
#define MBP_HEAVE_OFF		0
#define MBP_HEAVE_OFFSET	1
#define MBP_HEAVE_MULTIPLY	2
#define MBP_HEAVE_MULTIPLYOFFSET	3
#define MBP_LEVER_OFF		0
#define MBP_LEVER_ON		1
#define MBP_ROLLBIAS_OFF	0
#define MBP_ROLLBIAS_SINGLE	1
#define MBP_ROLLBIAS_DOUBLE	2
#define MBP_PITCHBIAS_OFF	0
#define MBP_PITCHBIAS_ON	1
#define MBP_HEADING_OFF         0
#define MBP_HEADING_CALC        1
#define MBP_HEADING_OFFSET      2
#define MBP_HEADING_CALCOFFSET  3
#define MBP_TIDE_OFF		0
#define MBP_TIDE_ON		1
#define MBP_SSRECALC_OFF	0
#define MBP_SSRECALC_ON		1


struct mb_process_struct 
	{
	/* general parameters */
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	int	mbp_ofile_specified;
	char	mbp_ofile[MBP_FILENAMESIZE];
	int	mbp_format_specified;
	int	mbp_format;
	
	/* navigation merging */
	int	mbp_nav_mode;
	char	mbp_navfile[MBP_FILENAMESIZE];
	int	mbp_nav_format;
	int	mbp_nav_heading;
	int	mbp_nav_speed;
	int	mbp_nav_draft;
	int	mbp_nav_algorithm;
	double	mbp_nav_timeshift;
	int	mbp_nav_shift;
	double	mbp_nav_offsetx;
	double	mbp_nav_offsety;
	double	mbp_nav_offsetz;
	
	/* adjusted navigation merging */
	int	mbp_navadj_mode;
	char	mbp_navadjfile[MBP_FILENAMESIZE];
	int	mbp_navadj_algorithm;
	
	/* bathymetry editing */
	int	mbp_edit_mode;
	char	mbp_editfile[MBP_FILENAMESIZE];
	
	/* bathymetry recalculation */
	int	mbp_bathrecalc_mode;
	int	mbp_svp_mode;
	char	mbp_svpfile[MBP_FILENAMESIZE];
	int	mbp_ssv_mode;
	double	mbp_ssv;
	int	mbp_corrected;
	int	mbp_tt_mode;
	double	mbp_tt_mult;
	int	mbp_angle_mode;
	
	/* draft correction */
	int	mbp_draft_mode;
	double	mbp_draft;
	double	mbp_draft_offset;
	double	mbp_draft_mult;
	
	/* heave correction */
	int	mbp_heave_mode;
	double	mbp_heave;
	double	mbp_heave_mult;
	
	/* lever correction */
	int	mbp_lever_mode;
	double	mbp_vru_offsetx;
	double	mbp_vru_offsety;
	double	mbp_vru_offsetz;
	double	mbp_sonar_offsetx;
	double	mbp_sonar_offsety;
	double	mbp_sonar_offsetz;
	
	/* roll correction */
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	
	/* pitch correction */
	int	mbp_pitchbias_mode;
	double	mbp_pitchbias;
	
	/* heading correction */
	int	mbp_heading_mode;
	double	mbp_headingbias;
	
	/* tide correction */
	int	mbp_tide_mode;
	char	mbp_tidefile[MBP_FILENAMESIZE];
	int	mbp_tide_format;
	
	/* sidescan recalculation */
	int	mbp_ssrecalc_mode;
	double	mbp_ssrecalc_pixelsize;
	double	mbp_ssrecalc_swathwidth;
	int	mbp_ssrecalc_interpolate;

	/* metadata strings */
	char	mbp_meta_operator[MBP_FILENAMESIZE];
	char	mbp_meta_platform[MBP_FILENAMESIZE];
	char	mbp_meta_sonar[MBP_FILENAMESIZE];
	char	mbp_meta_survey[MBP_FILENAMESIZE];
	char	mbp_meta_pi[MBP_FILENAMESIZE];
	char	mbp_meta_client[MBP_FILENAMESIZE];
	};

