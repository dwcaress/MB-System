/*--------------------------------------------------------------------
 *    The MB-system:	mbprocess.c	3/31/93
 *    $Id: mbprocess.c,v 4.3 2000-10-11 01:06:15 caress Exp $
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
 * mbprocess is a tool for processing swath sonar bathymetry data.  
 * This program performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit mask files or edit save
 *     files.
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
 * The data format and the input and output data files can be
 * specified using command line options. If no parameter file is 
 * specified (using the -P option) but an input file is specified
 * (with the -I option), then mbprocess will look for a parameter
 * file with the path inputfile.par, where inputfile is the input
 * file path.\n";
 *
 * Author:	D. W. Caress
 * Date:	January 4, 2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.2  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  2000/09/11  20:10:02  caress
 * Added suppport for merging edited sonar depth values.
 *
 * Revision 4.0  2000/03/08  00:04:28  caress
 * Release 4.6.10
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_process.h"
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: mbprocess.c,v 4.3 2000-10-11 01:06:15 caress Exp $";
	static char program_name[] = "mbprocess";
	static char help_message[] =  "mbprocess is a tool for processing swath sonar bathymetry data.\n\
This program performs a number of functions, including:\n\
  - merging navigation\n\
  - recalculating bathymetry from travel time and angle data\n\
    by raytracing through a layered water sound velocity model.\n\
  - applying changes to ship draft, roll bias and pitch bias\n\
  - applying bathymetry edits from edit mask files or edit save\n\
    files.\n\
The parameters controlling mbprocess are included in an ascii\n\
parameter file with the following possible entries:\n\
  FORMAT format                  # sets format id\n\
  INFILE file                    # sets input file path\n\
  OUTFILE file                   # sets output file path\n\
  DRAFT draft                    # sets draft value (m)\n\
  DRAFTOFFSET offset             # sets value added to draft (m)\n\
  DRAFTMULTIPLY multiplier       # sets value multiplied by draft\n\
  ROLLBIAS                       # sets roll bias (degrees)\n\
  ROLLBIASPORT                   # sets port roll bias (degrees)\n\
  ROLLBIASSTBD                   # sets starboard roll bias (degrees)\n\
  PITCHBIAS                      # sets pitch bias\n\
  NAVADJFILE file                # sets adjusted navigation file path\n\
                                 # - this file supercedes nav file for\n\
                                 #   lon and lat only\n\
                                 # - uses mbnavadjust output\n\
  NAVADJSPLINE                   # sets spline adjusted navigation interpolation\n\
  NAVFILE file                   # sets navigation file path\n\
  NAVFORMAT format               # sets navigation file format\n\
  NAVHEADING                     # sets heading to be merged from nav file\n\
  NAVSPEED                       # sets speed to be merged from nav file\n\
  NAVDRAFT                       # sets draft to be merged from nav file\n\
  NAVSPLINE                      # sets spline navigation interpolation\n\
  HEADING                        # sets heading to course made good\n\
  HEADINGOFFSET offset           # sets value added to heading (degree)\n\
  SVPFILE file                   # sets svp file path\n\
  SSV                            # sets ssv value (m/s)\n\
  SSVOFFSET                      # sets value added to ssv (m/s)\n\
  UNCORRECTED                    # sets raytraced bathymetry to uncorrected values\n\
  EDITSAVEFILE                   # sets edit save file path (from mbedit)\n\
  EDITMASKFILE                   # sets edit mask file path (from mbmask)\n\
The input file \"infile\"  must be specified with the -I option. The\n\
data format can also be specified, thought the program can\n\
infer the format if the standard MB-System suffix convention\n\
is used (*.mbXXX where XXX is the MB-System format id number).\n\
The program will look for and use a parameter file with the \n\
name \"infile.par\". If no parameter file exists, the program \n\
will infer a reasonable processing path by looking for navigation\n\
and mbedit edit save files.\n";
	static char usage_message[] = "mbprocess [-Fformat  \n\t-Iinfile -Ooutfile -V -H]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read and write control parameters */
	int	format = 0;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*imbio_ptr = NULL;
	char	*ombio_ptr = NULL;

	/* mbio read and write values */
	char	*store_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	int	nbath;
	int	namp;
	int	nss;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	onav = 0;
	int	ocomment = 0;
	int	oother = 0;
	char	comment[MBP_FILENAMESIZE];

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];
	
	/* parameter controls */
	struct mb_process_struct process;
	
	/* processing variables */
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	int	mbp_ofile_specified;
	char	mbp_ofile[MBP_FILENAMESIZE];
	int	mbp_format_specified;
	int	mbp_format;
	FILE	*tfp;
	struct stat file_status;
	int	fstat;
	int	nnav = 0;
	int	nanav = 0;
	int	size, nchar, len, nget, nav_ok;
	int	time_j[5], stime_i[7], ftime_i[7];
	double	sec, hr;
	char	*bufftmp;
	char	NorS[2], EorW[2];
	double	mlon, llon, mlat, llat;
	int	degree, minute, time_set;
	double	dminute;
	double	second;
	double	splineflag;
	double	*ntime, *nlon, *nlat, *nheading, *nspeed, *ndraft;
	double	*natime, *nalon, *nalat;
	double	*nlonspl, *nlatspl;
	double	*nalonspl, *nalatspl;
	int	itime, iatime;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy, dist;
	int	intstat;
	double	heading_old = 0.0;
	int	nsvp = 0;
	double	*depth = NULL;
	double	*velocity = NULL;
	double	*velocity_sum = NULL;
	char	*rt_svp;
	double	ssv;
	int	nedit = 0;
	double	*edit_time_d;
	int	*edit_beam;
	int	*edit_action;
	double	stime_d;
	int	sbeam;
	int	saction;
	int	insert, firstedit, lastedit;
	double	draft_org, depth_offset_use, depth_offset_change, depth_offset_org, static_shift;
	double	ttime, range;
	double	xx, zz, vsum, vavg;
	double	alpha, beta, theta, phi;
	int	ray_stat;
	double	*ttimes = NULL;
	double	*angles = NULL;
	double	*angles_forward = NULL;
	double	*angles_null = NULL;
	double	*bheave = NULL;
	double	*alongtrack_offset = NULL;

	/* ssv handling variables */
	int	ssv_mode = MBP_SSV_CORRECT;
	int	ssv_prelimpass = MB_YES;
	double	ssv_default;
	double	ssv_start;
	
	int	stat_status;
	struct stat statbuf;
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result;
	int	nbeams;
	int	i, j, k, l, m, n, mm;
	
	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&mbp_format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults */
	pings = 1;
	lonflip = 0;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	mbp_ifile_specified = MB_NO;
	strcpy (mbp_ifile, "\0");
	mbp_ofile_specified = MB_NO;
	strcpy (mbp_ofile, "\0");
	mbp_format_specified = MB_NO;
	
	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:O:o:")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &mbp_format);
			mbp_format_specified = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			mbp_ifile_specified = MB_YES;
			sscanf (optarg,"%s", mbp_ifile);
			flag++;
			break;
		case 'O':
		case 'o':
			mbp_ofile_specified = MB_YES;
			sscanf (optarg,"%s", mbp_ofile);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
		}

	/* quit if no input file specified */
	if (mbp_ifile_specified == MB_NO)
	    {
	    fprintf(stderr,"\nProgram <%s> requires an input data file.\n",program_name);
	    fprintf(stderr,"The input file may be specified with the -I option\n");
	    fprintf(stderr,"or it may be set in a parameter file specified with the -P option.\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    error = MB_ERROR_OPEN_FAIL;
	    exit(error);
	    }
		
	/* load parameters */
	status = mb_pr_readpar(verbose, mbp_ifile, MB_YES, 
			&process, &error);
			
	/* reset output file and format */
	if (mbp_ofile_specified == MB_YES)
	    {
	    strcpy(process.mbp_ofile, mbp_ofile);
	    }
	if (mbp_format_specified == MB_YES)
	    {
	    process.mbp_format = mbp_format;
	    }

	/* quit if no knowledge of what to do */
	if (status == MB_FAILURE)
	    {
	    fprintf(stderr,"\nProgram <%s> requires a parameter file.\n",program_name);
	    fprintf(stderr,"The parameter file must exist as 'infile.par', where the\n");
	    fprintf(stderr,"input file 'infile' is specified with the -I option.\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    exit(error);
	    }

	/* check for nav format with heading, speed, and draft merge */
	if (process.mbp_nav_mode == MBP_NAV_ON
	    && (process.mbp_nav_heading == MBP_NAV_ON
		|| process.mbp_nav_speed == MBP_NAV_ON
		|| process.mbp_nav_draft == MBP_NAV_ON)
	    && process.mbp_nav_format != 9)
	    {
	    fprintf(stderr,"\nNavigation format <%d> does not include \n",process.mbp_nav_format);
	    fprintf(stderr,"heading, speed, and draft values.\n");
	    if (process.mbp_nav_heading == MBP_NAV_ON)
		{
		fprintf(stderr,"Merging of heading data disabled.\n");
		process.mbp_nav_heading = MBP_NAV_OFF;
		}
	    if (process.mbp_nav_speed == MBP_NAV_ON)
		{
		fprintf(stderr,"Merging of speed data disabled.\n");
		process.mbp_nav_speed = MBP_NAV_OFF;
		}
	    if (process.mbp_nav_draft == MBP_NAV_ON)
		{
		fprintf(stderr,"Merging of draft data disabled.\n");
		process.mbp_nav_draft = MBP_NAV_OFF;
		}
	    }

	/* check for format with travel time data */
	if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
	    {
	    status = mb_format(verbose,&process.mbp_format,&format_num,&error);
	    if (mb_traveltime_table[format_num] != MB_YES)
		{
		fprintf(stderr,"\nProgram <%s> requires travel time data to recalculate\n",program_name);
		fprintf(stderr,"bathymetry from travel times and angles.\n");
		fprintf(stderr,"Format %d is unacceptable because it does not inlude travel time data.\n",process.mbp_format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}
	    }

	/* print starting debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
	    fprintf(stderr,"dbg2  Version %s\n",rcs_id);
	    fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
	    fprintf(stderr,"\ndbg2  MB-System Control Parameters:\n");
	    fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
	    fprintf(stderr,"dbg2       help:            %d\n",help);
	    fprintf(stderr,"dbg2       format:          %d\n",format);
	    fprintf(stderr,"dbg2       pings:           %d\n",pings);
	    fprintf(stderr,"dbg2       lonflip:         %d\n",lonflip);
	    fprintf(stderr,"dbg2       bounds[0]:       %f\n",bounds[0]);
	    fprintf(stderr,"dbg2       bounds[1]:       %f\n",bounds[1]);
	    fprintf(stderr,"dbg2       bounds[2]:       %f\n",bounds[2]);
	    fprintf(stderr,"dbg2       bounds[3]:       %f\n",bounds[3]);
	    fprintf(stderr,"dbg2       btime_i[0]:      %d\n",btime_i[0]);
	    fprintf(stderr,"dbg2       btime_i[1]:      %d\n",btime_i[1]);
	    fprintf(stderr,"dbg2       btime_i[2]:      %d\n",btime_i[2]);
	    fprintf(stderr,"dbg2       btime_i[3]:      %d\n",btime_i[3]);
	    fprintf(stderr,"dbg2       btime_i[4]:      %d\n",btime_i[4]);
	    fprintf(stderr,"dbg2       btime_i[5]:      %d\n",btime_i[5]);
	    fprintf(stderr,"dbg2       btime_i[6]:      %d\n",btime_i[6]);
	    fprintf(stderr,"dbg2       etime_i[0]:      %d\n",etime_i[0]);
	    fprintf(stderr,"dbg2       etime_i[1]:      %d\n",etime_i[1]);
	    fprintf(stderr,"dbg2       etime_i[2]:      %d\n",etime_i[2]);
	    fprintf(stderr,"dbg2       etime_i[3]:      %d\n",etime_i[3]);
	    fprintf(stderr,"dbg2       etime_i[4]:      %d\n",etime_i[4]);
	    fprintf(stderr,"dbg2       etime_i[5]:      %d\n",etime_i[5]);
	    fprintf(stderr,"dbg2       etime_i[6]:      %d\n",etime_i[6]);
	    fprintf(stderr,"dbg2       speedmin:        %f\n",speedmin);
	    fprintf(stderr,"dbg2       timegap:         %f\n",timegap);
	    fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
	    fprintf(stderr,"\ndbg2  Processing Parameters:\n");
	    if (process.mbp_format_specified == MB_YES)
		fprintf(stderr,"dbg2       format:          %d\n",process.mbp_format);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"dbg2       input file:      %s\n",process.mbp_ifile);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"dbg2       output file:     %s\n",process.mbp_ofile);
	    if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
		fprintf(stderr,"dbg2       Bathymetry not recalculated.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
		{
		fprintf(stderr,"dbg2       Bathymetry recalculated by raytracing.\n");
		if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		    fprintf(stderr,"dbg2       roll bias:       OFF\n");
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		    fprintf(stderr,"dbg2       roll bias:       %f deg\n", process.mbp_rollbias);
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		    {
		    fprintf(stderr,"dbg2       port roll bias:  %f deg\n", process.mbp_rollbias_port);
		    fprintf(stderr,"dbg2       port roll stbd:  %f deg\n", process.mbp_rollbias_stbd);
		    }
		if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		    fprintf(stderr,"dbg2       pitch bias:      OFF\n");
		else if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
		    fprintf(stderr,"dbg2       pitch bias:      %f deg\n", process.mbp_pitchbias);
		else if (process.mbp_draft_mode == MBP_DRAFT_SET)
		    fprintf(stderr,"dbg2       draft set:       %f m\n", process.mbp_draft);
		if (process.mbp_ssv_mode == MBP_SSV_OFF)
		    fprintf(stderr,"dbg2       ssv:             OFF\n");
		else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
		    fprintf(stderr,"dbg2       offset ssv:      %f m/s\n", process.mbp_ssv);
		else if (process.mbp_ssv_mode == MBP_SSV_SET)
		    fprintf(stderr,"dbg2       set ssv:         %f m/s\n", process.mbp_ssv);
		if (process.mbp_svp_mode == MBP_SVP_OFF)
		    fprintf(stderr,"dbg2       svp:             OFF\n");
		else if (process.mbp_svp_mode == MBP_SVP_ON)
		    fprintf(stderr,"dbg2       svp file:        %s\n", process.mbp_svpfile);
		if (process.mbp_uncorrected == MB_NO)
		    fprintf(stderr,"dbg2       bathymetry mode: CORRECTED\n");
		else if (process.mbp_uncorrected == MB_YES)
		    fprintf(stderr,"dbg2       bathymetry mode: UNCORRECTED\n");
		}
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
		{
		fprintf(stderr,"dbg2       Bathymetry recalculated by rigid rotation.\n");
		if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		    fprintf(stderr,"dbg2       roll bias:       OFF\n");
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		    fprintf(stderr,"dbg2       roll bias:       %f deg\n", process.mbp_rollbias);
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		    {
		    fprintf(stderr,"dbg2       port roll bias:  %f deg\n", process.mbp_rollbias_port);
		    fprintf(stderr,"dbg2       port roll stbd:  %f deg\n", process.mbp_rollbias_stbd);
		    }
		if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		    fprintf(stderr,"dbg2       pitch bias:      OFF\n");
		else if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
		    fprintf(stderr,"dbg2       pitch bias:      %f deg\n", process.mbp_pitchbias);
		}
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
		{
		fprintf(stderr,"dbg2       Bathymetry recalculated by transducer depth shift.\n");
		}
	    if (process.mbp_navadj_mode == MBP_NAV_OFF)
		fprintf(stderr,"dbg2       merge adjusted navigation:OFF\n");
	    else if (process.mbp_navadj_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"dbg2       adjusted navigation file: %s\n", process.mbp_navadjfile);
		if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
		    fprintf(stderr,"dbg2       adjusted navigation algorithm: linear interpolation\n");
		else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
		    fprintf(stderr,"dbg2       adjusted navigation algorithm: spline interpolation\n");
		}
	    if (process.mbp_nav_mode == MBP_NAV_OFF)
		fprintf(stderr,"dbg2       merge navigation:OFF\n");
	    else if (process.mbp_nav_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"dbg2       navigation file:      %s\n", process.mbp_navfile);
		fprintf(stderr,"dbg2       navigation format:    %d\n", process.mbp_nav_format);
		if (process.mbp_nav_heading == MBP_NAV_ON)
		    fprintf(stderr,"dbg2     heading merge:    ON\n");
		else
		    fprintf(stderr,"dbg2     heading merge:    OFF\n");
		if (process.mbp_nav_speed == MBP_NAV_ON)
		    fprintf(stderr,"dbg2     speed merge:      ON\n");
		else
		    fprintf(stderr,"dbg2     speed merge:      OFF\n");
		if (process.mbp_nav_draft == MBP_NAV_ON)
		    fprintf(stderr,"dbg2     draft merge:      ON\n");
		else
		    fprintf(stderr,"dbg2     draft merge:      OFF\n");

		if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
		    fprintf(stderr,"dbg2       navigation algorithm: linear interpolation\n");
		else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
		    fprintf(stderr,"dbg2       navigation algorithm: spline interpolation\n");
		}
	    if (process.mbp_draft_mode == MBP_DRAFT_OFF)
		fprintf(stderr,"dbg2       draft modify:    OFF\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
		fprintf(stderr,"dbg2       draft offset:    %f m\n", process.mbp_draft);
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
		fprintf(stderr,"dbg2       draft multiplier:%f m\n", process.mbp_draft_mult);
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		{
		fprintf(stderr,"dbg2       draft offset:    %f m\n", process.mbp_draft);
		fprintf(stderr,"dbg2       draft multiplier:%f m\n", process.mbp_draft_mult);
		}
	    else if (process.mbp_draft_mode == MBP_DRAFT_SET)
		fprintf(stderr,"dbg2       draft set:       %f m\n", process.mbp_draft);
	    if (process.mbp_edit_mode == MBP_EDIT_OFF)
		fprintf(stderr,"dbg2       merge bath edit: OFF\n");
	    else if (process.mbp_edit_mode == MBP_EDIT_ON)
		fprintf(stderr,"dbg2       bathy edit file: %s\n", process.mbp_editfile);
	    if (process.mbp_mask_mode == MBP_MASK_OFF)
		fprintf(stderr,"dbg2       merge bath mask: OFF\n");
	    else if (process.mbp_mask_mode == MBP_MASK_ON)
		fprintf(stderr,"dbg2       bathy mask file: %s\n", process.mbp_maskfile);
	    }

	/* print starting debug statements */
	if (verbose == 1)
	    {
	    fprintf(stderr,"\nProgram <%s>\n",program_name);
	    fprintf(stderr,"Version %s\n",rcs_id);
	    fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
	    fprintf(stderr,"\nProcessing Parameters:\n");
	    if (process.mbp_format_specified == MB_YES)
		fprintf(stderr,"     format:          %d\n",process.mbp_format);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"     input file:      %s\n",process.mbp_ifile);
	    if (process.mbp_ifile_specified == MB_YES)
		fprintf(stderr,"     output file:     %s\n",process.mbp_ofile);
	    if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
		fprintf(stderr,"     Bathymetry not recalculated.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
		{
		fprintf(stderr,"     Bathymetry recalculated by raytracing.\n");
		if (process.mbp_ssv_mode == MBP_SSV_OFF)
		    fprintf(stderr,"     ssv:             OFF\n");
		else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
		    fprintf(stderr,"     offset ssv:      %f m/s\n", process.mbp_ssv);
		else if (process.mbp_ssv_mode == MBP_SSV_SET)
		    fprintf(stderr,"     set ssv:         %f m/s\n", process.mbp_ssv);
		if (process.mbp_svp_mode == MBP_SVP_OFF)
		    fprintf(stderr,"     svp:             OFF\n");
		else if (process.mbp_svp_mode == MBP_SVP_ON)
		    fprintf(stderr,"     svp file:        %s\n", process.mbp_svpfile);
		if (process.mbp_uncorrected == MB_NO)
		    fprintf(stderr,"     bathymetry mode: CORRECTED\n");
		else if (process.mbp_uncorrected == MB_YES)
		    fprintf(stderr,"     bathymetry mode: UNCORRECTED\n");
		}
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
		{
		fprintf(stderr,"     Bathymetry recalculated by rigid rotation.\n");
		}
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
		{
		fprintf(stderr,"     Bathymetry recalculated by transducer depth shift.\n");
		}
	    if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		fprintf(stderr,"     roll bias:       OFF\n");
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		fprintf(stderr,"     roll bias:       %f deg\n", process.mbp_rollbias);
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		{
		fprintf(stderr,"     port roll bias:  %f deg\n", process.mbp_rollbias_port);
		fprintf(stderr,"     port roll stbd:  %f deg\n", process.mbp_rollbias_stbd);
		}
	    if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		fprintf(stderr,"     pitch bias:      OFF\n");
	    else if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
		fprintf(stderr,"     pitch bias:      %f deg\n", process.mbp_pitchbias);
	    if (process.mbp_draft_mode == MBP_DRAFT_OFF)
		fprintf(stderr,"     draft modify:    OFF\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
		fprintf(stderr,"     draft offset:         %f m\n", process.mbp_draft);
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
		fprintf(stderr,"     draft multiplier:%f m\n", process.mbp_draft_mult);
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		{
		fprintf(stderr,"     draft multiplier:     %f m\n", process.mbp_draft_mult);
		fprintf(stderr,"     draft offset:         %f m\n", process.mbp_draft);
		}
	    else if (process.mbp_draft_mode == MBP_DRAFT_SET)
		fprintf(stderr,"     set draft:            %f m\n", process.mbp_draft);
	    if (process.mbp_navadj_mode == MBP_NAV_OFF)
		fprintf(stderr,"     merge adjusted navigation: OFF\n");
	    else if (process.mbp_navadj_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"     adjusted navigation file: %s\n", process.mbp_navadjfile);
		if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
		    fprintf(stderr,"     adjusted navigation algorithm: linear interpolation\n");
		else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
		    fprintf(stderr,"     adjusted navigation algorithm: spline interpolation\n");
		}
	    if (process.mbp_nav_mode == MBP_NAV_OFF)
		fprintf(stderr,"     merge navigation:     OFF\n");
	    else if (process.mbp_nav_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"     navigation file:      %s\n", process.mbp_navfile);
		fprintf(stderr,"     navigation format:    %d\n", process.mbp_nav_format);
		if (process.mbp_nav_heading == MBP_NAV_ON)
		    fprintf(stderr,"     heading merge:    ON\n");
		else
		    fprintf(stderr,"     heading merge:    OFF\n");
		if (process.mbp_nav_speed == MBP_NAV_ON)
		    fprintf(stderr,"     speed merge:      ON\n");
		else
		    fprintf(stderr,"     speed merge:      OFF\n");
		if (process.mbp_nav_draft == MBP_NAV_ON)
		    fprintf(stderr,"     draft merge:      ON\n");
		else
		    fprintf(stderr,"     draft merge:      OFF\n");
		if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
		    fprintf(stderr,"     navigation algorithm: linear interpolation\n");
		else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
		    fprintf(stderr,"     navigation algorithm: spline interpolation\n");
		}
	    if (process.mbp_heading_mode == MBP_HEADING_OFF)
		fprintf(stderr,"     heading modify:  OFF\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_OFFSET)
		fprintf(stderr,"     heading offset:       %f m\n", process.mbp_headingbias);
	    else if (process.mbp_heading_mode == MBP_HEADING_CALC)
		fprintf(stderr,"     heading modify:  COURSE MADE GOOD\n");
	    if (process.mbp_edit_mode == MBP_EDIT_OFF)
		fprintf(stderr,"     merge bath edit:      OFF\n");
	    else if (process.mbp_edit_mode == MBP_EDIT_ON)
		fprintf(stderr,"     bathy edit file:      %s\n", process.mbp_editfile);
	    if (process.mbp_mask_mode == MBP_MASK_OFF)
		fprintf(stderr,"     merge bath mask:      OFF\n");
	    else if (process.mbp_mask_mode == MBP_MASK_ON)
		fprintf(stderr,"     bathy mask file:      %s\n", process.mbp_maskfile);
	    }

	/* if help desired then print it and exit */
	if (help)
	    {
	    fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
	    fprintf(stderr,"\n%s\n",help_message);
	    fprintf(stderr,"\nusage: %s\n", usage_message);
	    exit(error);
	    }

	/*--------------------------------------------
	  get svp
	  --------------------------------------------*/

	/* if raytracing to be done get svp */
	if (process.mbp_svp_mode == MBP_SVP_ON)
	    {
	    /* count the data points in the svp file */
	    nsvp = 0;
	    if ((tfp = fopen(process.mbp_svpfile, "r")) == NULL) 
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",process.mbp_svpfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
	    while ((result = fgets(buffer,MBP_FILENAMESIZE,tfp)) == buffer)
		    if (buffer[0] != '#')
			nsvp++;
	    fclose(tfp);
	    
	    /* allocate arrays for svp */
	    if (nsvp > 1)
		{
		size = (nsvp+1)*sizeof(double);
		status = mb_malloc(verbose,size,&depth,&error);
		status = mb_malloc(verbose,size,&velocity,&error);
		status = mb_malloc(verbose,size,&velocity_sum,&error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    
		}
	
	    /* if no svp data then quit */
	    else
		{
		error = MB_ERROR_BAD_DATA;
		fprintf(stderr,"\nUnable to read data from SVP file <%s>\n",process.mbp_svpfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}		    
		
	    /* read the data points in the svp file */
	    nsvp = 0;
	    if ((tfp = fopen(process.mbp_svpfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",process.mbp_svpfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    while ((result = fgets(buffer,MBP_FILENAMESIZE,tfp)) == buffer)
		{
		if (buffer[0] != '#')
		    {
		    mm = sscanf(buffer,"%lf %lf",&depth[nsvp],&velocity[nsvp]);
		
		    /* output some debug values */
		    if (verbose >= 5 && mm == 2)
			    {
			    fprintf(stderr,"\ndbg5  New velocity value read in program <%s>\n",program_name);
			    fprintf(stderr,"dbg5       depth[%d]: %f  velocity[%d]: %f\n",
				    nsvp,depth[nsvp],nsvp,velocity[nsvp]);
			    }
		    if (mm == 2)
			nsvp++;
		    }
		}
	    fclose(tfp);

	    /* if velocity profile doesn't extend to 12000 m depth
		    extend it to that depth */
	    if (depth[nsvp-1] < 12000.0)
		    {
		    depth[nsvp] = 12000.0;
		    velocity[nsvp] = velocity[nsvp-1];
		    nsvp++;
		    }
    
	    /* get velocity sums */
	    velocity_sum[0] = 0.5*(velocity[1] + velocity[0])
		    *(depth[1] - depth[0]);
	    for (i=1;i<nsvp-1;i++)
		    {
		    velocity_sum[i] = velocity_sum[i-1] 
			+ 0.5*(velocity[i+1] + velocity[i])
			*(depth[i+1] - depth[i]);
		    }
	    }

	/*--------------------------------------------
	  get nav
	  --------------------------------------------*/

	/* if nav merging to be done get nav */
	if (process.mbp_nav_mode == MBP_NAV_ON)
	    {
	    /* set max number of characters to be read at a time */
	    if (process.mbp_nav_format == 8)
		    nchar = 96;
	    else
		    nchar = 128;

	    /* count the data points in the nav file */
	    nnav = 0;
	    if ((tfp = fopen(process.mbp_navfile, "r")) == NULL) 
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",process.mbp_navfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		    nnav++;
	    fclose(tfp);
	    
	    /* allocate arrays for nav */
	    if (nnav > 1)
		{
		size = (nnav+1)*sizeof(double);
		status = mb_malloc(verbose,nnav*sizeof(double),&ntime,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nlon,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nlat,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nheading,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nspeed,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&ndraft,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nlonspl,&error);
		status = mb_malloc(verbose,nnav*sizeof(double),&nlatspl,&error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    
		}
	
	    /* if no nav data then quit */
	    else
		{
		error = MB_ERROR_BAD_DATA;
		fprintf(stderr,"\nUnable to read data from navigation file <%s>\n",process.mbp_navfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}		    
		
	    /* read the data points in the nav file */
	    nnav = 0;
	    if ((tfp = fopen(process.mbp_navfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open navigation File <%s> for reading\n",process.mbp_navfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		{
		nav_ok = MB_NO;

		/* deal with nav in form: time_d lon lat */
		if (process.mbp_nav_format == 1)
			{
			nget = sscanf(buffer,"%lf %lf %lf",
				&ntime[nnav],&nlon[nnav],&nlat[nnav]);
			if (nget == 3)
				nav_ok = MB_YES;
			}

		/* deal with nav in form: yr mon day hour min sec lon lat */
		else if (process.mbp_nav_format == 2)
			{
			nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf",
				&time_i[0],&time_i[1],&time_i[2],
				&time_i[3],&time_i[4],&sec,
				&nlon[nnav],&nlat[nnav]);
			time_i[5] = (int) sec;
			time_i[6] = 1000000*(sec - time_i[5]);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;
			if (nget == 8)
				nav_ok = MB_YES;
			}

		/* deal with nav in form: yr jday hour min sec lon lat */
		else if (process.mbp_nav_format == 3)
			{
			nget = sscanf(buffer,"%d %d %d %d %lf %lf %lf",
				&time_j[0],&time_j[1],&hr,
				&time_j[2],&sec,
				&nlon[nnav],&nlat[nnav]);
			time_j[2] = time_j[2] + 60*hr;
			time_j[3] = (int) sec;
			time_j[4] = 1000000*(sec - time_j[3]);
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;
			if (nget == 7)
				nav_ok = MB_YES;
			}

		/* deal with nav in form: yr jday daymin sec lon lat */
		else if (process.mbp_nav_format == 4)
			{
			nget = sscanf(buffer,"%d %d %d %lf %lf %lf",
				&time_j[0],&time_j[1],&time_j[2],
				&sec,
				&nlon[nnav],&nlat[nnav]);
			time_j[3] = (int) sec;
			time_j[4] = 1000000*(sec - time_j[3]);
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;
			if (nget == 6)
				nav_ok = MB_YES;
			}

		/* deal with nav in L-DEO processed nav format */
		else if (process.mbp_nav_format == 5)
			{
			strncpy(dummy,"\0",128);
			time_j[0] = atoi(strncpy(dummy,buffer,2));
			mb_fix_y2k(verbose, time_j[0], &time_j[0]);
			strncpy(dummy,"\0",128);
			time_j[1] = atoi(strncpy(dummy,buffer+3,3));
			strncpy(dummy,"\0",128);
			hr = atoi(strncpy(dummy,buffer+7,2));
			strncpy(dummy,"\0",128);
			time_j[2] = atoi(strncpy(dummy,buffer+10,2))
				+ 60*hr;
			strncpy(dummy,"\0",128);
			time_j[3] = atof(strncpy(dummy,buffer+13,3));
			time_j[4] = 0;
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;

			strncpy(NorS,"\0",sizeof(NorS));
			strncpy(NorS,buffer+20,1);
			strncpy(dummy,"\0",128);
			mlat = atof(strncpy(dummy,buffer+21,3));
			strncpy(dummy,"\0",128);
			llat = atof(strncpy(dummy,buffer+24,8));
			strncpy(EorW,"\0",sizeof(EorW));
			strncpy(EorW,buffer+33,1);
			strncpy(dummy,"\0",128);
			mlon = atof(strncpy(dummy,buffer+34,4));
			strncpy(dummy,"\0",128);
			llon = atof(strncpy(dummy,buffer+38,8));
			nlon[nnav] = mlon + llon/60.;
			if (strncmp(EorW,"W",1) == 0) 
				nlon[nnav] = -nlon[nnav];
			nlat[nnav] = mlat + llat/60.;
			if (strncmp(NorS,"S",1) == 0) 
				nlat[nnav] = -nlat[nnav];
			nav_ok = MB_YES;
			}

		/* deal with nav in real and pseudo NMEA 0183 format */
		else if (process.mbp_nav_format == 6 || process.mbp_nav_format == 7)
			{
			/* check if real sentence */
			len = strlen(buffer);
			if (strncmp(buffer,"$",1) == 0)
			    {
			    if (strncmp(&buffer[3],"DAT",3) == 0
				&& len > 15)
				{
				time_set = MB_NO;
				strncpy(dummy,"\0",128);
				time_i[0] = atoi(strncpy(dummy,buffer+7,4));
				time_i[1] = atoi(strncpy(dummy,buffer+11,2));
				time_i[2] = atoi(strncpy(dummy,buffer+13,2));
				}
			    else if ((strncmp(&buffer[3],"ZDA",3) == 0
				    || strncmp(&buffer[3],"UNX",3) == 0)
				    && len > 14)
				{
				time_set = MB_NO;
				/* find start of ",hhmmss.ss" */
				if ((bufftmp = strchr(buffer, ',')) != NULL)
				    {
				    strncpy(dummy,"\0",128);
				    time_i[3] = atoi(strncpy(dummy,bufftmp+1,2));
				    strncpy(dummy,"\0",128);
				    time_i[4] = atoi(strncpy(dummy,bufftmp+3,2));
				    strncpy(dummy,"\0",128);
				    time_i[5] = atoi(strncpy(dummy,bufftmp+5,2));
				    if (bufftmp[7] == '.')
					{
					strncpy(dummy,"\0",128);
					time_i[6] = 10000*
					    atoi(strncpy(dummy,bufftmp+8,2));
					}
				    else
					time_i[6] = 0;
				    /* find start of ",dd,mm,yyyy" */
				    if ((bufftmp = strchr(&bufftmp[1], ',')) != NULL)
					{
					strncpy(dummy,"\0",128);
					time_i[2] = atoi(strncpy(dummy,bufftmp+1,2));
					strncpy(dummy,"\0",128);
					time_i[1] = atoi(strncpy(dummy,bufftmp+4,2));
					strncpy(dummy,"\0",128);
					time_i[0] = atoi(strncpy(dummy,bufftmp+7,4));
					time_set = MB_YES;
					}
				    }
				}
			    else if (((process.mbp_nav_format == 6 && strncmp(&buffer[3],"GLL",3) == 0)
				|| (process.mbp_nav_format == 7 && strncmp(&buffer[3],"GGA",3) == 0))
				&& time_set == MB_YES && len > 26)
				{
				time_set = MB_NO;
				/* find start of ",ddmm.mm,N,ddmm.mm,E" */
				if ((bufftmp = strchr(buffer, ',')) != NULL)
				    {
				    if (process.mbp_nav_format == 7)
					bufftmp = strchr(&bufftmp[1], ',');
				    strncpy(dummy,"\0",128);
				    degree = atoi(strncpy(dummy,bufftmp+1,2));
				    strncpy(dummy,"\0",128);
				    dminute = atof(strncpy(dummy,bufftmp+3,5));
				    strncpy(NorS,"\0",sizeof(NorS));
				    strncpy(NorS,bufftmp+9,1);
				    nlat[nnav] = degree + dminute/60.;
				    if (strncmp(NorS,"S",1) == 0) 
					nlat[nnav] = -nlat[nnav];
				    strncpy(dummy,"\0",128);
				    degree = atoi(strncpy(dummy,bufftmp+11,3));
				    strncpy(dummy,"\0",128);
				    dminute = atof(strncpy(dummy,bufftmp+14,5));
				    strncpy(EorW,"\0",sizeof(EorW));
				    strncpy(EorW,bufftmp+20,1);
				    nlon[nnav] = degree + dminute/60.;
				    if (strncmp(EorW,"W",1) == 0) 
					nlon[nnav] = -nlon[nnav];
				    mb_get_time(verbose,time_i,&time_d);
				    ntime[nnav] = time_d;
				    nav_ok = MB_YES;
				    }
				}
			    }
			}

		/* deal with nav in Simrad 90 format */
		else if (process.mbp_nav_format == 8)
			{
			mb_get_int(&(time_i[2]), buffer+2,  2);
			mb_get_int(&(time_i[1]), buffer+4,  2);
			mb_get_int(&(time_i[0]), buffer+6,  2);
			mb_fix_y2k(verbose, time_i[0], &time_i[0]);
			mb_get_int(&(time_i[3]), buffer+9,  2);
			mb_get_int(&(time_i[4]), buffer+11, 2);
			mb_get_int(&(time_i[5]), buffer+13, 2);
			mb_get_int(&(time_i[6]), buffer+15, 2);
			time_i[6] = 10000 * time_i[6];
			mb_get_time(verbose,time_i,&time_d);
			ntime[nnav] = time_d;

			mb_get_double(&mlat,    buffer+18,   2);
			mb_get_double(&llat, buffer+20,   7);
			NorS[0] = buffer[27];
			nlat[nnav] = mlat + llat/60.0;
			if (NorS[0] == 'S' || NorS[0] == 's')
				nlat[nnav] = -nlat[nnav];
			mb_get_double(&mlon,    buffer+29,   3);
			mb_get_double(&llon, buffer+32,   7);
			EorW[0] = buffer[39];
			nlon[nnav] = mlon + llon/60.0;
			if (EorW[0] == 'W' || EorW[0] == 'w')
				nlon[nnav] = -nlon[nnav];
			nav_ok = MB_YES;
			}

		/* deal with nav in form: yr mon day hour min sec time_d lon lat heading speed draft*/
		else if (process.mbp_nav_format == 9)
			{
			nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf",
				&time_i[0],&time_i[1],&time_i[2],
				&time_i[3],&time_i[4],&sec,
				&ntime[nnav],
				&nlon[nnav],&nlat[nnav],
				&nheading[nnav],&nspeed[nnav],
				&ndraft[nnav]);
			if (nget >= 9)
				nav_ok = MB_YES;
			if (nav_ok == MB_YES)
			    {
			    if (process.mbp_nav_heading == MBP_NAV_ON && nget < 10)
				{
				fprintf(stderr,"\nHeading data missing from nav file.\nMerging of heading data disabled.\n");
				process.mbp_nav_heading = MBP_NAV_OFF;
				}
			    if (process.mbp_nav_speed == MBP_NAV_ON && nget < 11)
				{
				fprintf(stderr,"Speed data missing from nav file.\nMerging of speed data disabled.\n");
				process.mbp_nav_speed = MBP_NAV_OFF;
				}
			    if (process.mbp_nav_draft == MBP_NAV_ON && nget < 12)
				{
				fprintf(stderr,"Draft data missing from nav file.\nMerging of draft data disabled.\n");
				process.mbp_nav_draft = MBP_NAV_OFF;
				}
			    if (process.mbp_nav_heading == MBP_NAV_OFF)
				{
				nheading[nnav] = 0.0;
				}
			    if (process.mbp_nav_speed == MBP_NAV_OFF)
				{
				nheading[nnav] = 0.0;
				}
			    if (process.mbp_nav_draft == MBP_NAV_OFF)
				{
				nheading[nnav] = 0.0;
				}
			    }
			}


		/* make sure longitude is defined according to lonflip */
		if (nav_ok == MB_YES)
			{
			if (lonflip == -1 && nlon[nnav] > 0.0)
				nlon[nnav] = nlon[nnav] - 360.0;
			else if (lonflip == 0 && nlon[nnav] < -180.0)
				nlon[nnav] = nlon[nnav] + 360.0;
			else if (lonflip == 0 && nlon[nnav] > 180.0)
				nlon[nnav] = nlon[nnav] - 360.0;
			else if (lonflip == 1 && nlon[nnav] < 0.0)
				nlon[nnav] = nlon[nnav] + 360.0;
			}

		/* output some debug values */
		if (verbose >= 5 && nav_ok == MB_YES)
			{
			fprintf(stderr,"\ndbg5  New navigation point read in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
				nnav,ntime[nnav],nlon[nnav],nlat[nnav]);
			}
		else if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       line: %s\n",buffer);
			}

		/* check for reverses or repeats in time */
		if (nav_ok == MB_YES)
			{
			if (nnav == 0)
				nnav++;
			else if (ntime[nnav] > ntime[nnav-1])
				nnav++;
			else if (nnav > 0 && ntime[nnav] <= ntime[nnav-1] 
				&& verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
					nnav-1,ntime[nnav-1],nlon[nnav-1],
					nlat[nnav-1]);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
					nnav,ntime[nnav],nlon[nnav],
					nlat[nnav]);
				}
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	    fclose(tfp);

		
	    /* check for nav */
	    if (nnav < 2)
		    {
		    fprintf(stderr,"\nNo navigation read from file <%s>\n",process.mbp_navfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
    
	    /* set up spline interpolation of nav points */
	    splineflag = 1.0e30;
	    spline(ntime-1,nlon-1,nnav,splineflag,splineflag,nlonspl-1);
	    spline(ntime-1,nlat-1,nnav,splineflag,splineflag,nlatspl-1);
    
	    /* get start and finish times of nav */
	    mb_get_date(verbose,ntime[0],stime_i);
	    mb_get_date(verbose,ntime[nnav-1],ftime_i);
    
	    /* give the statistics */
	    if (verbose >= 1)
		    {
		    fprintf(stderr,"\n%d navigation records read\n",nnav);
		    fprintf(stderr,"Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			    stime_i[0],stime_i[1],stime_i[2],stime_i[3],
			    stime_i[4],stime_i[5],stime_i[6]);
		    fprintf(stderr,"Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			    ftime_i[0],ftime_i[1],ftime_i[2],ftime_i[3],
			    ftime_i[4],ftime_i[5],ftime_i[6]);
		    }
	    }

	/*--------------------------------------------
	  get adjusted nav
	  --------------------------------------------*/

	/* if adjusted nav merging to be done get adjusted nav */
	if (process.mbp_navadj_mode == MBP_NAV_ON)
	    {
	    /* set max number of characters to be read at a time */
	    nchar = 128;

	    /* count the data points in the adjusted nav file */
	    nanav = 0;
	    if ((tfp = fopen(process.mbp_navadjfile, "r")) == NULL) 
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr,"\nUnable to Open Adjusted Navigation File <%s> for reading\n",process.mbp_navadjfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		    nanav++;
	    fclose(tfp);
	    
	    /* allocate arrays for adjusted nav */
	    if (nanav > 1)
		{
		size = (nanav+1)*sizeof(double);
		status = mb_malloc(verbose,nanav*sizeof(double),&natime,&error);
		status = mb_malloc(verbose,nanav*sizeof(double),&nalon,&error);
		status = mb_malloc(verbose,nanav*sizeof(double),&nalat,&error);
		status = mb_malloc(verbose,nanav*sizeof(double),&nalonspl,&error);
		status = mb_malloc(verbose,nanav*sizeof(double),&nalatspl,&error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    
		}
	
	    /* if no adjusted nav data then quit */
	    else
		{
		error = MB_ERROR_BAD_DATA;
		fprintf(stderr,"\nUnable to read data from adjusted navigation file <%s>\n",process.mbp_navadjfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}		    
		
	    /* read the data points in the nav file */
	    nanav = 0;
	    if ((tfp = fopen(process.mbp_navadjfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open navigation File <%s> for reading\n",process.mbp_navadjfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		{
		nav_ok = MB_NO;

		/* deal with nav in form: yr mon day hour min sec time_d lon lat */
		nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf",
			&time_i[0],&time_i[1],&time_i[2],
			&time_i[3],&time_i[4],&sec,
			&natime[nanav],
			&nalon[nanav],&nalat[nanav]);
		if (nget >= 9)
			nav_ok = MB_YES;

		/* make sure longitude is defined according to lonflip */
		if (nav_ok == MB_YES)
			{
			if (lonflip == -1 && nalon[nanav] > 0.0)
				nalon[nanav] = nalon[nanav] - 360.0;
			else if (lonflip == 0 && nalon[nanav] < -180.0)
				nalon[nanav] = nalon[nanav] + 360.0;
			else if (lonflip == 0 && nalon[nanav] > 180.0)
				nalon[nanav] = nalon[nanav] - 360.0;
			else if (lonflip == 1 && nalon[nanav] < 0.0)
				nalon[nanav] = nalon[nanav] + 360.0;
			}

		/* output some debug values */
		if (verbose >= 5 && nav_ok == MB_YES)
			{
			fprintf(stderr,"\ndbg5  New adjusted navigation point read in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
				nanav,natime[nanav],nalon[nanav],nalat[nanav]);
			}
		else if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       line: %s\n",buffer);
			}

		/* check for reverses or repeats in time */
		if (nav_ok == MB_YES)
			{
			if (nanav == 0)
				nanav++;
			else if (natime[nanav] > natime[nanav-1])
				nanav++;
			else if (nanav > 0 && natime[nanav] <= natime[nanav-1] 
				&& verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       adjusted nav[%d]: %f %f %f\n",
					nanav-1,natime[nanav-1],nalon[nanav-1],
					nalat[nanav-1]);
				fprintf(stderr,"dbg5       adjusted nav[%d]: %f %f %f\n",
					nanav,natime[nanav],nalon[nanav],
					nalat[nanav]);
				}
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	    fclose(tfp);

		
	    /* check for adjusted nav */
	    if (nanav < 2)
		    {
		    fprintf(stderr,"\nNo adjusted navigation read from file <%s>\n",process.mbp_navadjfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
    
	    /* set up spline interpolation of adjusted nav points */
	    splineflag = 1.0e30;
	    spline(natime-1,nalon-1,nanav,splineflag,splineflag,nalonspl-1);
	    spline(natime-1,nalat-1,nanav,splineflag,splineflag,nalatspl-1);
    
	    /* get start and finish times of nav */
	    mb_get_date(verbose,natime[0],stime_i);
	    mb_get_date(verbose,natime[nanav-1],ftime_i);
    
	    /* give the statistics */
	    if (verbose >= 1)
		    {
		    fprintf(stderr,"\n%d adjusted navigation records read\n",nanav);
		    fprintf(stderr,"Adjusted nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			    stime_i[0],stime_i[1],stime_i[2],stime_i[3],
			    stime_i[4],stime_i[5],stime_i[6]);
		    fprintf(stderr,"Adjusted nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			    ftime_i[0],ftime_i[1],ftime_i[2],ftime_i[3],
			    ftime_i[4],ftime_i[5],ftime_i[6]);
		    }
	    }

	/*--------------------------------------------
	  get edits
	  --------------------------------------------*/

	/* get edits */
	if (process.mbp_edit_mode == MBP_EDIT_ON)
	    {
	    /* count the data points in the edit file */
	    nedit = 0;
	    firstedit = 0;
	    fstat = stat(process.mbp_editfile, &file_status);
	    if (fstat == 0 
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		nedit = file_status.st_size 
			/ (sizeof(double) + 2 * sizeof(int));
		}
	    
	    /* allocate arrays for edit */
	    if (nedit > 0)
		{
		size = nedit *sizeof(double);
		status = mb_malloc(verbose,size,&edit_time_d,&error);
		status = mb_malloc(verbose,size,&edit_beam,&error);
		status = mb_malloc(verbose,size,&edit_action,&error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    
		}
	    }
		
	if (process.mbp_edit_mode == MBP_EDIT_ON
	    && nedit > 0)
	    {
	    /* read the data points in the edit file */
	    if ((tfp = fopen(process.mbp_editfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Edit Save File <%s> for reading\n",process.mbp_editfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	    error = MB_ERROR_NO_ERROR;
	    insert = 0;
	    for (i=0;i<nedit && error == MB_ERROR_NO_ERROR;i++)
		{
		/* reset message */
		if (verbose == 1 && (i+1) == 25000)
		    fprintf(stderr, "\nSorted %d of %d old edits...\n", i+1, nedit);
		else if (verbose == 1 && (i+1)%25000 == 0)
		    fprintf(stderr, "Sorted %d of %d old edits...\n", i+1, nedit);

		if (fread(&stime_d, sizeof(double), 1, tfp) != 1
		    || fread(&sbeam, sizeof(int), 1, tfp) != 1
		    || fread(&saction, sizeof(int), 1, tfp) != 1)
		    {
		    status = MB_FAILURE;
		    error = MB_ERROR_EOF;
		    }
#ifdef BYTESWAPPED
		else
		    {
		    mb_swap_double(&stime_d);
		    sbeam = mb_swap_int(sbeam);
		    saction = mb_swap_int(saction);
		    }
#endif
    
		/* insert into sorted array */
		if (i > 0)
		    {
		    if (stime_d < edit_time_d[insert - 1])
			{
			for (j = insert - 1; j >= 0 && stime_d < edit_time_d[j]; j--)
			    insert--;
			}
		    else if (stime_d >= edit_time_d[insert - 1])
			{
			for (j = insert; j < i && stime_d >= edit_time_d[j]; j++)
			    insert++;
			}
		    if (insert < i)
			{
			memmove(&edit_time_d[insert+1], 
				&edit_time_d[insert], 
				sizeof(double) * (i - insert));
			memmove(&edit_beam[insert+1], 
				&edit_beam[insert], 
				sizeof(int) * (i - insert));
			memmove(&edit_action[insert+1], 
				&edit_action[insert], 
				sizeof(int) * (i - insert));
			}
		    }
		edit_time_d[insert] = stime_d;
		edit_beam[insert] = sbeam;
		edit_action[insert] = saction;
		}
	    fclose(tfp);
    
	    /* give the statistics */
	    if (verbose >= 1)
		    {
		    fprintf(stderr,"\n%d bathymetry edits read\n",nedit);
		    }
	    }

	/*--------------------------------------------
	  now read the file
	  --------------------------------------------*/

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,process.mbp_ifile,process.mbp_format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",process.mbp_ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,process.mbp_ofile,process.mbp_format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",process.mbp_ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
				&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
				&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssacrosstrack,
				&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssalongtrack,
				&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&ttimes,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&angles,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&angles_forward,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&angles_null,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bheave,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&alongtrack_offset,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	
	/* read input file until a surface sound velocity value
		is obtained, then close and reopen the file 
		this provides the starting surface sound velocity
		for recalculating the bathymetry */
	ssv_start = 0.0;
	if (ssv_prelimpass == MB_YES)
	    {
	    error = MB_ERROR_NO_ERROR;
	    while (error <= MB_ERROR_NO_ERROR
		&& ssv_start <= 0.0)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&nbath,&namp,&nss,
				beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);
				
		if (kind == MB_DATA_DATA 
			&& error <= MB_ERROR_NO_ERROR)
			{
			/* extract travel times */
			status = mb_ttimes(verbose,imbio_ptr,
				store_ptr,&kind,&nbeams,
				ttimes,angles,
				angles_forward,angles_null,
				bheave,alongtrack_offset,
				&draft,&ssv,&error);
				
			/* check surface sound velocity */
			if (ssv > 0.0)
				ssv_start = ssv;
			}
		}
	
	    /* close and reopen the input file */
	    status = mb_close(verbose,&imbio_ptr,&error);
	    if ((status = mb_read_init(
		verbose,process.mbp_ifile,process.mbp_format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",process.mbp_ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    }
	if (ssv_start <= 0.0)
		ssv_start = ssv_default;
	
	/* reset error */
	error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"Swath data modified by program %s",program_name);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(date,"\0",25);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,MBP_FILENAMESIZE);
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"Depths and crosstrack distances recalculated from travel times");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  by raytracing through a water velocity profile specified");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  by the user.  The depths have been saved in units of");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    if (process.mbp_uncorrected == MB_YES)
		    sprintf(comment,"  uncorrected meters (the depth values are adjusted to be");
	    else
		    sprintf(comment,"  corrected meters (the depth values obtained by");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    if (process.mbp_uncorrected == MB_YES)
		    sprintf(comment,"  consistent with a vertical water velocity of 1500 m/s).");
	    else
		    sprintf(comment,"  raytracing are not adjusted further).");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	    
	else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"Depths and crosstrack distances adjusted for roll bias, ");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  and pitch bias.");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"Depths and crosstrack distances adjusted for, ");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  change in transducer depth.");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	    
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"  MBIO data format:   %d",process.mbp_format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"  Input file:         %s",process.mbp_ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"  Output file:        %s",process.mbp_ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
	    {
	    if (ssv_mode == MBP_SSV_CORRECT)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  SSV mode:           original SSV correct");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    else
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  SSV mode:           original SSV incorrect");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  Default SSV:        %f",ssv_default);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    if (ssv_prelimpass == MB_YES)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  SSV initial pass:   on");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    else
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  SSV initial pass:   off");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}

	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  SVP file:               %s",process.mbp_svpfile);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"  Input water sound velocity profile:");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    sprintf(comment,"    depth (m)   velocity (m/s)");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    for (i=0;i<nsvp;i++)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"     %10.2f     %10.2f",
			depth[i],velocity[i]);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    }

	if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  roll bias:       OFF\n");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  roll bias:       %f deg\n", process.mbp_rollbias);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  port roll bias:  %f deg\n", process.mbp_rollbias_port);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  port roll stbd:  %f deg\n", process.mbp_rollbias_stbd);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  pitch bias:      OFF\n");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	else if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
	    {
	    strncpy(comment,"\0",MBP_FILENAMESIZE);
	    fprintf(stderr,"  pitch bias:      %f deg\n", process.mbp_pitchbias);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }

	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"  Roll bias:    %f degrees (starboard: -, port: +)",
		process.mbp_rollbias);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment,"  Pitch bias:   %f degrees (aft: -, forward: +)",
		process.mbp_pitchbias);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	if (process.mbp_draft_mode == MBP_DRAFT_SET)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft set:    %f meters",
			process.mbp_draft);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft offset: %f meters",
			process.mbp_draft);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft multiplier: %f",
			process.mbp_draft_mult);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft offset: %f meters",
			process.mbp_draft);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft multiplier: %f",
			process.mbp_draft_mult);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	else if (process.mbp_draft_mode == MBP_DRAFT_OFF)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Draft:        not modified");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	if (process.mbp_nav_mode == MBP_NAV_OFF)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Merge navigation:     OFF");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	else if (process.mbp_nav_mode == MBP_NAV_ON)
		{
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Navigation file:      %s\n", process.mbp_navfile);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;

		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment,"  Navigation format:    %d\n", process.mbp_nav_format);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;

		if (process.mbp_nav_heading == MBP_NAV_ON)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Heading merge:    ON\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Heading merge:    OFF\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		if (process.mbp_nav_speed == MBP_NAV_ON)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Speed merge:      ON\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Speed merge:      OFF\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		if (process.mbp_nav_draft == MBP_NAV_ON)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Draft merge:      ON\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Draft merge:      OFF\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Navigation algorithm: linear interpolation\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  Navigation algorithm: spline interpolation\n");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		}


	strncpy(comment,"\0",MBP_FILENAMESIZE);
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* set up the raytracing */
	status = mb_rt_init(verbose, nsvp, depth, velocity, &rt_svp, &error);

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&nbath,&namp,&nss,
				beamflag,bath,amp,
				bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;

		/* time gaps do not matter to mbprocess */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* out of bounds do not matter to mbprocess */
		if (error == MB_ERROR_OUT_BOUNDS)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* non-survey data do not matter to mbprocess */
		if (error == MB_ERROR_OTHER)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error == MB_ERROR_COMMENT)
			{
			if (icomment == 1)
				fprintf(stderr,"\nComments in Input:\n");
			fprintf(stderr,"%s\n",comment);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error > MB_ERROR_OTHER)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			fprintf(stderr,"Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nFatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
			}

		/* extract the navigation if available */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{
			status = mb_extract_nav(verbose,imbio_ptr,store_ptr,&kind,
					time_i,&time_d,&navlon,&navlat,
					&speed,&heading,&draft_org,&roll,&pitch,&heave,&error);
			draft = draft_org;
			}

		/* interpolate the navigation if desired */
		if (error == MB_ERROR_NO_ERROR
			&& process.mbp_nav_mode == MBP_NAV_ON
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{			
			/* interpolate navigation */
			if (process.mbp_nav_algorithm == MBP_NAV_SPLINE
			    && time_d >= ntime[0] 
			    && time_d <= ntime[nnav-1])
			    {
			    intstat = splint(ntime-1,nlon-1,nlonspl-1,
				    nnav,time_d,&navlon,&itime);
			    intstat = splint(ntime-1,nlat-1,nlatspl-1,
				    nnav,time_d,&navlat,&itime);
			    }
			else
			    {
			    intstat = linint(ntime-1,nlon-1,
				    nnav,time_d,&navlon,&itime);
			    intstat = linint(ntime-1,nlat-1,
				    nnav,time_d,&navlat,&itime);
			    }
			    
			/* interpolate heading */
			if (process.mbp_nav_heading == MBP_NAV_ON)
			    {
			    intstat = linint(ntime-1,nheading-1,
				    nnav,time_d,&heading,&itime);				
			    }
			    
			/* interpolate speed */
			if (process.mbp_nav_speed == MBP_NAV_ON)
			    {
			    intstat = linint(ntime-1,nspeed-1,
				    nnav,time_d,&speed,&itime);				
			    }
			    
			/* interpolate draft */
			if (process.mbp_nav_draft == MBP_NAV_ON)
			    {
			    intstat = linint(ntime-1,ndraft-1,
				    nnav,time_d,&draft,&itime);				
			    }
			}

		/* interpolate the adjusted navigation if desired */
		if (error == MB_ERROR_NO_ERROR
			&& process.mbp_navadj_mode == MBP_NAV_ON
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{			
			/* interpolate adjusted navigation */
			if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE
			    && time_d >= natime[0] 
			    && time_d <= natime[nanav-1])
			    {
			    intstat = splint(natime-1,nalon-1,nalonspl-1,
				    nanav,time_d,&navlon,&iatime);
			    intstat = splint(natime-1,nalat-1,nalatspl-1,
				    nanav,time_d,&navlat,&iatime);
			    }
			else
			    {
			    intstat = linint(natime-1,nalon-1,
				    nanav,time_d,&navlon,&iatime);
			    intstat = linint(natime-1,nalat-1,
				    nanav,time_d,&navlat,&iatime);
			    }
			}
    
		/* add user specified draft correction if desired */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{		
			if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
				draft = draft + process.mbp_draft;
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
				draft = draft * process.mbp_draft_mult;
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
				draft = draft * process.mbp_draft_mult + process.mbp_draft;
			else if (process.mbp_draft_mode == MBP_DRAFT_SET)
				draft = process.mbp_draft;
			}

		/* make up heading and speed if required */
		if (error == MB_ERROR_NO_ERROR
			&& (process.mbp_nav_mode == MBP_NAV_ON
			    || process.mbp_navadj_mode == MBP_NAV_ON)
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV)
			&& process.mbp_heading_mode == MBP_HEADING_CALC)
			{
			if (process.mbp_nav_mode == MBP_NAV_ON)
			    {
			    mb_coor_scale(verbose,nlat[itime-1],&mtodeglon,&mtodeglat);
			    del_time = ntime[itime] - ntime[itime-1];
			    dx = (nlon[itime] - nlon[itime-1])/mtodeglon;
			    dy = (nlat[itime] - nlat[itime-1])/mtodeglat;
			    }
			else
			    {
			    mb_coor_scale(verbose,nalat[iatime-1],&mtodeglon,&mtodeglat);
			    del_time = natime[iatime] - natime[iatime-1];
			    dx = (nalon[iatime] - nalon[iatime-1])/mtodeglon;
			    dy = (nalat[iatime] - nalat[iatime-1])/mtodeglat;
			    }
			dist = sqrt(dx*dx + dy*dy);
			if (del_time > 0.0)
				speed = 3.6*dist/del_time;
			else
				speed = 0.0;
			if (dist > 0.0)
				{
				heading = RTD*atan2(dx/dist,dy/dist);
				heading_old = heading;
				}
			else
				heading = heading_old;
			}

		/* else adjust heading if required */
		else if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV)
			&& process.mbp_heading_mode == MBP_HEADING_OFFSET)
			{
			heading += process.mbp_headingbias;
			if (heading > 360.0)
			    heading -= 360.0;
			else if (heading < 0.0)
			    heading += 360.0;
			}

		/* if survey data encountered, 
			get the bathymetry */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA))
			{
			/* extract travel times */
			status = mb_ttimes(verbose,imbio_ptr,
				store_ptr,&kind,&nbeams,
				ttimes,angles,
				angles_forward,angles_null,
				bheave,alongtrack_offset,
				&draft_org,&ssv,&error);

			/* set surface sound speed to default if needed */
			if (ssv <= 0.0)
				ssv = ssv_start;
			else
				ssv_start = ssv;

			/* if svp specified recalculate bathymetry
			    by raytracing  */
			if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
			    {
			    /* loop over the beams */
			    for (i=0;i<beams_bath;i++)
			      {
			      if (ttimes[i] > 0.0)
				{
				/* if needed, translate angles from takeoff
					angle coordinates to roll-pitch 
					coordinates, apply roll and pitch
					corrections, and translate back */
				if (process.mbp_rollbias != 0.0 
					|| process.mbp_pitchbias != 0.0)
					{
					mb_takeoff_to_rollpitch(
						verbose,
						angles[i], angles_forward[i], 
						&alpha, &beta, 
						&error);
					alpha += process.mbp_pitchbias;
					beta += process.mbp_rollbias;
					mb_rollpitch_to_takeoff(
						verbose, 
						alpha, beta, 
						&angles[i], &angles_forward[i], 
						&error); 
					}
    
				/* add heave and draft */
				depth_offset_use = bheave[i] + draft;
				static_shift = 0.0;
	
				/* check depth_offset - use static shift if depth_offset negative */
				if (depth_offset_use < 0.0)
				    {
				    fprintf(stderr, "\nWarning: Depth offset negative - transducers above water?!\n");
				    fprintf(stderr, "Raytracing performed from zero depth followed by static shift.\n");
				    fprintf(stderr, "Depth offset is sum of heave + transducer depth.\n");
				    fprintf(stderr, "Draft from data:       %f\n", draft);
				    fprintf(stderr, "Heave from data:       %f\n", bheave[i]);
				    fprintf(stderr, "User specified draft:  %f\n", process.mbp_draft);
				    fprintf(stderr, "Depth offset used:     %f\n", depth_offset_use);
				    fprintf(stderr, "Data Record: %d\n",odata);
				    fprintf(stderr, "Ping time:  %4d %2d %2d %2d:%2d:%2d.%6d\n", 
					    time_i[0], time_i[1], time_i[2], 
					    time_i[3], time_i[4], time_i[5], time_i[6]);
	    
				    static_shift = depth_offset_use;
				    depth_offset_use = 0.0;
				    }

				/* raytrace */
				status = mb_rt(verbose, rt_svp, depth_offset_use, 
					angles[i], 0.5*ttimes[i],
					ssv_mode, ssv, angles_null[i], 
					0, NULL, NULL, NULL, 
					&xx, &zz, 
					&ttime, &ray_stat, &error);
					
				/* apply static shift if needed */
				if (static_shift < 0.0)
				    zz += static_shift;
    
				/* uncorrect depth if desired */
				if (process.mbp_uncorrected == MB_YES)
				    {
				    k = -1;
				    for (j=0;j<nsvp-1;j++)
					{
					if (depth[j] < zz & depth[j+1] >= zz)
					    k = j;
					}
				    if (k > 0)
					vsum = velocity_sum[k-1];
				    else
					vsum = 0.0;
				    if (k >= 0)
					{
					vsum += 0.5*(2*velocity[k] 
					    + (zz - depth[k])*(velocity[k+1] - velocity[k])
					    /(depth[k+1] - depth[k]))*(zz - depth[k]);
					vavg = vsum / zz;
					zz = zz*1500./vavg;
					}
				    }
    
				/* get alongtrack and acrosstrack distances
					and depth */
				bathacrosstrack[i] = xx*cos(DTR*angles_forward[i]);
				bathalongtrack[i] = xx*sin(DTR*angles_forward[i]);
				bath[i] = zz;
				
				/* output some debug values */
				if (verbose >= 5)
				    fprintf(stderr,"dbg5       %3d %3d %6.3f %6.3f %6.3f %8.2f %8.2f %8.2f\n",
					idata, i, 0.5*ttimes[i], angles[i], angles_forward[i],  
					bathacrosstrack[i], bathalongtrack[i], bath[i]);
    
				/* output some debug messages */
				if (verbose >= 5)
				    {
				    fprintf(stderr,"\ndbg5  Depth value calculated in program <%s>:\n",program_name);
				    fprintf(stderr,"dbg5       kind:  %d\n",kind);
				    fprintf(stderr,"dbg5       beam:  %d\n",i);
				    fprintf(stderr,"dbg5       tt:     %f\n",ttimes[i]);
				    fprintf(stderr,"dbg5       xx:     %f\n",xx);
				    fprintf(stderr,"dbg5       zz:     %f\n",zz);
				    fprintf(stderr,"dbg5       xtrack: %f\n",bathacrosstrack[i]);
				    fprintf(stderr,"dbg5       ltrack: %f\n",bathalongtrack[i]);
				    fprintf(stderr,"dbg5       depth:  %f\n",bath[i]);
				    }
				}
				
			      /* else if no travel time no data */
			      else
				beamflag[i] = MB_FLAG_NULL;
			      }
			    }

			/* recalculate bathymetry by rigid rotations  */
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
			    {
			    /* loop over the beams */
			    for (i=0;i<beams_bath;i++)
			      {
			      if (beamflag[i] != MB_FLAG_NULL)
				{
				/* add heave and draft */
				depth_offset_use = bheave[i] + draft;
				depth_offset_org = bheave[i] + draft_org;

				/* strip off heave + draft */
				bath[i] -= depth_offset_org;
				
				/* get range and angles in 
				    roll-pitch frame */
				range = sqrt(bath[i] * bath[i] 
					    + bathacrosstrack[i] 
						* bathacrosstrack[i]
					    + bathalongtrack[i] 
						* bathalongtrack[i]);
				alpha = asin(bathalongtrack[i] 
					/ range);
				beta = acos(bathacrosstrack[i] 
					/ range / cos(alpha));

				/* apply roll pitch corrections */
				alpha += DTR * process.mbp_pitchbias;
				beta +=  DTR * process.mbp_rollbias;
				
				/* recalculate bathymetry */
				bath[i] 
				    = range * cos(alpha) * sin(beta);
				bathalongtrack[i] 
				    = range * sin(alpha);
				bathacrosstrack[i] 
				    = range * cos(alpha) * cos(beta);	
					
				/* add heave and draft back in */	    
				bath[i] += depth_offset_use;
    
				/* output some debug values */
				if (verbose >= 5)
				    fprintf(stderr,"dbg5       %3d %3d %8.2f %8.2f %8.2f\n",
					idata, i, 
					bathacrosstrack[i], 
					bathalongtrack[i], 
					bath[i]);
    
				/* output some debug messages */
				if (verbose >= 5)
				    {
				    fprintf(stderr,"\ndbg5  Depth value calculated in program <%s>:\n",program_name);
				    fprintf(stderr,"dbg5       kind:  %d\n",kind);
				    fprintf(stderr,"dbg5       beam:  %d\n",i);
				    fprintf(stderr,"dbg5       xtrack: %f\n",bathacrosstrack[i]);
				    fprintf(stderr,"dbg5       ltrack: %f\n",bathalongtrack[i]);
				    fprintf(stderr,"dbg5       depth:  %f\n",bath[i]);
				    }
				}
			      }
			    }

			/* recalculate bathymetry by changes to transducer depth  */
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
			    {
			    /* get draft change */
			    depth_offset_change = draft - draft_org;
/*fprintf(stderr, "depth offset: %f %f %f %f\n", time_d, draft, draft_org, depth_offset_change);*/

			    /* loop over the beams */
			    for (i=0;i<beams_bath;i++)
			      {
			      if (beamflag[i] != MB_FLAG_NULL)
				{
				/* apply transducer depth change to depths */	    
				bath[i] += depth_offset_change;
    
				/* output some debug values */
				if (verbose >= 5)
				    fprintf(stderr,"dbg5       %3d %3d %8.2f %8.2f %8.2f\n",
					idata, i, 
					bathacrosstrack[i], 
					bathalongtrack[i], 
					bath[i]);
    
				/* output some debug messages */
				if (verbose >= 5)
				    {
				    fprintf(stderr,"\ndbg5  Depth value calculated in program <%s>:\n",program_name);
				    fprintf(stderr,"dbg5       kind:  %d\n",kind);
				    fprintf(stderr,"dbg5       beam:  %d\n",i);
				    fprintf(stderr,"dbg5       xtrack: %f\n",bathacrosstrack[i]);
				    fprintf(stderr,"dbg5       ltrack: %f\n",bathalongtrack[i]);
				    fprintf(stderr,"dbg5       depth:  %f\n",bath[i]);
				    }
				}
			      }
/*fprintf(stderr, "time:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d draft:%f depth_offset_change:%f\n", 
time_i[0], time_i[1], time_i[2], time_i[3], 
time_i[4], time_i[5], time_i[6], draft, depth_offset_change);*/
			    }

			/* output some debug messages */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Depth values calculated in program <%s>:\n",program_name);
			    fprintf(stderr,"dbg5       kind:  %d\n",kind);
			    fprintf(stderr,"dbg5      beam    time      depth        dist\n");	
			    for (i=0;i<nbath;i++)
				fprintf(stderr,"dbg5       %2d   %f   %f   %f   %f\n",
				    i,ttimes[i],
				    bath[i],bathacrosstrack[i],
				    bathalongtrack[i]);
			    }
			}
			
		/* apply the saved edits */
		if (process.mbp_edit_mode == MBP_EDIT_ON
		    && nedit > 0
		    && error == MB_ERROR_NO_ERROR
		    && kind == MB_DATA_DATA)
		    {			    
		    /* find first and last edits for this ping */
		    lastedit = firstedit - 1;
		    for (j = firstedit; j < nedit && time_d >= edit_time_d[j]; j++)
			{
			if (edit_time_d[j] == time_d)
			    {
			    if (lastedit < firstedit)
				firstedit = j;
			    lastedit = j;
			    }
			}
			
		    /* apply edits */
		    for (j=firstedit;j<=lastedit;j++)
			{
			if (edit_beam[j] >= 0 
			    && edit_beam[j] < nbath)
			    {
			    /* apply edit */
			    if (edit_action[j] == MBP_EDIT_FLAG
				&& mb_beam_ok(beamflag[edit_beam[j]]))
				beamflag[edit_beam[j]] 
				    = MB_FLAG_FLAG + MB_FLAG_MANUAL;
			    else if (edit_action[j] == MBP_EDIT_FILTER
				&& mb_beam_ok(beamflag[edit_beam[j]]))
				beamflag[edit_beam[j]] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
			    else if (edit_action[j] == MBP_EDIT_UNFLAG
				&& !mb_beam_ok(beamflag[edit_beam[j]]))
				beamflag[edit_beam[j]] = MB_FLAG_NONE;
			    else if (edit_action[j] == MBP_EDIT_ZERO)
				beamflag[edit_beam[j]] = MB_FLAG_NULL;
			    }			
			}
		    }

		/* insert the altered navigation if available */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{
			status = mb_insert_nav(verbose,imbio_ptr,store_ptr,
					time_i,time_d,navlon,navlat,
					speed,heading,draft,roll,pitch,heave,&error);
			}

		/* write some data */
		if (error == MB_ERROR_NO_ERROR
			|| kind == MB_DATA_COMMENT)
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,MB_YES,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					nbath,namp,nss,
					beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			if (status == MB_SUCCESS)
				{
				if (kind == MB_DATA_DATA)
					odata++;
				else if (kind == MB_DATA_NAV)
					onav++;
				else if (kind == MB_DATA_COMMENT)
					ocomment++;
				else
					oother++;
				}
			else
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",process.mbp_ofile);
				fprintf(stderr,"Output Record: %d\n",odata+1);
				fprintf(stderr,"Time: %4d %2d %2d %2d:%2d:%2d.%6d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* close the files */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,&depth,&error);
	mb_free(verbose,&velocity,&error);
	mb_free(verbose,&velocity_sum,&error);
	mb_free(verbose,&ttimes,&error);
	mb_free(verbose,&angles,&error);
	mb_free(verbose,&angles_forward,&error);
	mb_free(verbose,&angles_null,&error);
	mb_free(verbose,&bheave,&error);
	mb_free(verbose,&alongtrack_offset,&error);
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 
	if (nnav > 0)
		{
		mb_free(verbose,&ntime,&error);
		mb_free(verbose,&nlon,&error);
		mb_free(verbose,&nlat,&error);
		mb_free(verbose,&nheading,&error);
		mb_free(verbose,&nspeed,&error);
		mb_free(verbose,&ndraft,&error);
		mb_free(verbose,&nlonspl,&error);
		mb_free(verbose,&nlatspl,&error);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output nav records\n",onav);
		fprintf(stderr,"%d output comment records\n",ocomment);
		fprintf(stderr,"%d output other records\n",oother);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
/* From Numerical Recipies */
int spline(double *x, double *y, int n, double yp1, double ypn, double *y2)
{
	int i,k;
	double p,qn,sig,un,*u,*vector();
	void free_vector();

	u=vector(1,n-1);
	if (yp1 > 0.99e30)
		y2[1]=u[1]=0.0;
	else {
		y2[1] = -0.5;
		u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
	}
	for (i=2;i<=n-1;i++) {
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30)
		qn=un=0.0;
	else {
		qn=0.5;
		un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
	}
	y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
	for (k=n-1;k>=1;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
	free_vector(u,1,n-1);
	return(0);
}
/*--------------------------------------------------------------------*/
/* From Numerical Recipies */
int splint(double *xa, double *ya, double *y2a,
		int n, double x, double *y, int *i)
{
	int klo,khi,k;
	double h,b,a;

	klo=1;
	khi=n;
	while (khi-klo > 1) {
		k=(khi+klo) >> 1;
		if (xa[k] > x) khi=k;
		else klo=k;
	}
	if (khi == 1) khi = 2;
	if (klo == n) klo = n - 1;
	h=xa[khi]-xa[klo];
/*	if (h == 0.0) 
		{
		fprintf(stderr,"ERROR: interpolation time out of nav bounds\n");
		return(-1);
		}
*/
	a=(xa[khi]-x)/h;
	b=(x-xa[klo])/h;
	*y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]
		+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
	*i=klo;
	return(0);
}
/*--------------------------------------------------------------------*/
int linint(double *xa, double *ya,
		int n, double x, double *y, int *i)
{
	int klo,khi,k;
	double h,b;

	klo=1;
	khi=n;
	while (khi-klo > 1) {
		k=(khi+klo) >> 1;
		if (xa[k] > x) khi=k;
		else klo=k;
	}
	if (khi == 1) khi = 2;
	if (klo == n) klo = n - 1;
	h=xa[khi]-xa[klo];
/*	if (h == 0.0) 
		{
		fprintf(stderr,"ERROR: interpolation time out of nav bounds\n");
		return(-1);
		}
*/
	b = (ya[khi] - ya[klo]) / h;
	*y = ya[klo] + b * (x - xa[klo]);
	*i=klo;
	return(0);
}
/*--------------------------------------------------------------------*/
double *vector(int nl, int nh)
{
	double *v;
	v = (double *) malloc ((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) fprintf(stderr,"allocation failure in vector()");
	return v-nl;
}
/*--------------------------------------------------------------------*/
void free_vector(double *v, int nl, int nh)
{
	free((char*) (v+nl));
}
/*--------------------------------------------------------------------*/
