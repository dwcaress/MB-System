/*--------------------------------------------------------------------
 *    The MB-system:	mbprocess.c	3/31/93
 *    $Id: mbprocess.c,v 5.8 2001-07-31 00:42:12 caress Exp $
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
 *   - applying bathymetry edits from edit save files.
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter file syntax is documented by
 * comments in the source file mbsystem/src/mbio/mb_process.h
 * and the manual pages for mbprocess and mbset. The program
 * mbset is used to create and modify parameter files.
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
 * Revision 5.7  2001/07/27  19:09:41  caress
 * Started adding data cutting, but not done yet.
 *
 * Revision 5.6  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/08 21:45:46  caress
 * Version 5.0.beta01
 *
 * Revision 5.4  2001/06/03  07:07:34  caress
 * Release 5.0.beta01.
 *
 * Revision 5.3  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.2  2001/01/22  07:54:22  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:30:44  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
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
	static char rcs_id[] = "$Id: mbprocess.c,v 5.8 2001-07-31 00:42:12 caress Exp $";
	static char program_name[] = "mbprocess";
	static char help_message[] =  "mbprocess is a tool for processing swath sonar bathymetry data.\n\
This program performs a number of functions, including:\n\
  - merging navigation\n\
  - recalculating bathymetry from travel time and angle data\n\
    by raytracing through a layered water sound velocity model.\n\
  - applying changes to ship draft, roll bias and pitch bias\n\
  - applying bathymetry edits from edit save files.\n\
The parameters controlling mbprocess are included in an ascii\n\
parameter file. The parameter file syntax is documented by\n\
the manual pages for mbprocess and mbset. The program\n\
mbset is used to create and modify parameter files.\n\
The input file \"infile\"  must be specified with the -I option. The\n\
data format can also be specified, thought the program can\n\
infer the format if the standard MB-System suffix convention\n\
is used (*.mbXXX where XXX is the MB-System format id number).\n\
The program will look for and use a parameter file with the \n\
name \"infile.par\". If no parameter file exists, the program \n\
will infer a reasonable processing path by looking for navigation\n\
and mbedit edit save files.\n";
	static char usage_message[] = "mbprocess -Iinfile [-C -Fformat -N -Ooutfile -V -H]";

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
	void	*imbio_ptr = NULL;
	void	*ombio_ptr = NULL;

	/* mbio read and write values */
	void	*store_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
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
	int	inav = 0;
	int	icomment = 0;
	int	iother = 0;
	int	odata = 0;
	int	onav = 0;
	int	ocomment = 0;
	int	oother = 0;
	char	comment[MBP_FILENAMESIZE];
	
	/* sidescan recalculation */
	int	pixel_size_set;
	int	swath_width_set;
	int	pixel_int;
	double	pixel_size;
	double	swath_width;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];
	
	/* parameter controls */
	struct mb_process_struct process;
	
	/* processing variables */
	int	checkuptodate = MB_YES;
	int	read_datalist = MB_NO;
	int	read_data = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_NO;
	double	file_weight;
	int	proceedprocess= MB_NO;
	int	ifilemodtime = 0;
	int	ofilemodtime = 0;
	int	pfilemodtime = 0;
	int	navfilemodtime = 0;
	int	navadjfilemodtime = 0;
	int	esfmodtime = 0;
	int	svpmodtime = 0;
 	int	format = 0;
	int	variable_beams;
	int	traveltime;
	int	beam_flagging; 
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	char	mbp_pfile[MBP_FILENAMESIZE];
	int	mbp_ofile_specified;
	char	mbp_ofile[MBP_FILENAMESIZE];
	int	mbp_format_specified;
	int	mbp_format;
	int	strip_comments;
	FILE	*tfp;
	struct stat file_status;
	int	fstat;
	int	nnav = 0;
	int	nanav = 0;
	int	ntide = 0;
	int	size, nchar, len, nget, nav_ok, tide_ok;
	int	time_j[5], stime_i[7], ftime_i[7];
	int	ihr, isec;
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
	double	*tidetime, *tide;
	int	itime, iatime;
	double	headingx, headingy;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy, dz, r, dist;
	double	lever_heave = 0.0;
	int	intstat;
	double	time_d_old = 0.0;
	double	navlon_old = 0.0;
	double	navlat_old = 0.0;
	double	speed_old = 0.0;
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
	int	angle_mode = MBP_ANGLES_OK;
	int	ssv_prelimpass = MB_NO;
	double	ssv_default;
	double	ssv_start;
	
	int	stat_status;
	struct stat statbuf;
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result;
	int	nbeams;
	int	istart, iend, icut;
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
	strip_comments = MB_NO;
	
	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:NnO:o:Pp")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			mbp_format_specified = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			mbp_ifile_specified = MB_YES;
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'N':
		case 'n':
			strip_comments = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			mbp_ofile_specified = MB_YES;
			sscanf (optarg,"%s", mbp_ofile);
			flag++;
			break;
		case 'P':
		case 'p':
			checkuptodate = MB_NO;
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

	/* if help desired then print it and exit */
	if (help)
	    {
	    fprintf(stderr,"\n%s\n",help_message);
	    fprintf(stderr,"\nusage: %s\n", usage_message);
	    exit(error);
	    }

	/* quit if no input file specified */
	if (mbp_ifile_specified == MB_NO)
	    {
	    fprintf(stderr,"\nProgram <%s> requires an input data file.\n",program_name);
	    fprintf(stderr,"The input file must be specified with the -I option.\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    error = MB_ERROR_OPEN_FAIL;
	    exit(error);
	    }

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);
  
	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;
	else if (mbp_format_specified == MB_YES)
		mbp_format = format;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (status = mb_datalist_read(verbose,datalist,
			    mbp_ifile,&mbp_format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(mbp_ifile, read_file);
	    read_data = MB_YES;
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
	    fprintf(stderr,"dbg2       read_file:       %s\n",read_file);
	    fprintf(stderr,"dbg2       format:          %d\n",mbp_format);
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
	    fprintf(stderr,"dbg2       strip_comments:  %d\n",strip_comments);
	    fprintf(stderr,"dbg2       checkuptodate:   %d\n",checkuptodate);
	    fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
	    }

	/* print starting info statements */
	else
	    {
	    fprintf(stderr,"\nProgram <%s>\n",program_name);
	    fprintf(stderr,"Version %s\n",rcs_id);
	    fprintf(stderr,"MB-system Version %s\n",MB_VERSION);		
	    fprintf(stderr,"\nProgram Operation:\n");
	    fprintf(stderr,"  Input file:      %s\n",read_file);
	    fprintf(stderr,"  Format:          %d\n",mbp_format);
	    if (checkuptodate == MB_YES)
		fprintf(stderr,"  Files processed only if out of date.\n");
	    else
		fprintf(stderr,"  All files processed.\n");
	    if (strip_comments == MB_NO)
		fprintf(stderr,"  Comments embedded in ouput.\n\n");
	    else
		fprintf(stderr,"  Comments stripped from ouput.\n\n");
	    }
	    
	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* load parameters */
	status = mb_pr_readpar(verbose, mbp_ifile, MB_YES, 
			&process, &error);
			
	/* reset output file and format if not reading from datalist */
	if (read_datalist == MB_NO)
	    {
	    if (mbp_ofile_specified == MB_YES)
		{
		strcpy(process.mbp_ofile, mbp_ofile);
		}
	    if (mbp_format_specified == MB_YES)
		{
		process.mbp_format = mbp_format;
		}
	    }
			
	/* make output file path global if needed */
	if (status == MB_SUCCESS
	    && mbp_ofile_specified == MB_NO
	    && process.mbp_ofile[0] != '/'
	    && (len = strrchr(process.mbp_ifile,'/') 
			- process.mbp_ifile + 1) > 1)
	    {
	    strcpy(mbp_ofile,process.mbp_ofile);
	    strncpy(process.mbp_ofile,process.mbp_ifile,len);
	    process.mbp_ofile[len] = '\0';
	    strcat(process.mbp_ofile,mbp_ofile);
	    }
	    
	/* skip if processing cannot be inferred */
	if (status == MB_FAILURE)
	    {
	    proceedprocess = MB_NO;
	    fprintf(stderr,"Data skipped - processing unknown: %s\n",
		mbp_ifile);
	    }

	/* check for up to date if required */
	else if (checkuptodate == MB_YES)
		{
		/* check for existing parameter file */
		sprintf(mbp_pfile, "%s.par", mbp_ifile);
	        if ((fstat = stat(mbp_pfile, &file_status)) == 0
			&& (file_status.st_mode & S_IFMT) != S_IFDIR)
			{
			proceedprocess = MB_YES;
			pfilemodtime = file_status.st_mtime;
			}
		else
			{
			proceedprocess = MB_NO;
	    		fprintf(stderr,"Data skipped - no parameter file: %s\n",
				mbp_ifile);
			}

		/* get mod time for the input file */
		if ((fstat = stat(process.mbp_ifile, &file_status)) == 0
			&& (file_status.st_mode & S_IFMT) != S_IFDIR)
			{
 			ifilemodtime = file_status.st_mtime;
			}
		else
			{
			proceedprocess = MB_NO;
	    		fprintf(stderr,"Data skipped - no input file: %s\n",
				mbp_ifile);
			}

		/* if input and parameter files found check output and dependencies */
		if (proceedprocess == MB_YES)
		    {
		    /* get mod time for the output file */
		    if ((fstat = stat(process.mbp_ofile, &file_status)) == 0
			    && (file_status.st_mode & S_IFMT) != S_IFDIR)
			    ofilemodtime = file_status.st_mtime;
		    else
			    ofilemodtime = 0;
    
		    /* get mod time for the navigation file if needed */
		    if (process.mbp_nav_mode != MBP_NAV_OFF
			    && (fstat = stat(process.mbp_navfile, &file_status)) == 0
			    && (file_status.st_mode & S_IFMT) != S_IFDIR)
			    navfilemodtime = file_status.st_mtime;
		    else
			    navfilemodtime = 0;
    
		    /* get mod time for the navigation adjustment file if needed */
		    if (process.mbp_navadj_mode != MBP_NAV_OFF
			    && (fstat = stat(process.mbp_navadjfile, &file_status)) == 0
			    && (file_status.st_mode & S_IFMT) != S_IFDIR)
			    navadjfilemodtime = file_status.st_mtime;
		    else
			    navadjfilemodtime = 0;
    
		    /* get mod time for the edit save file if needed */
		    if (process.mbp_edit_mode != MBP_EDIT_OFF
			    && (fstat = stat(process.mbp_editfile, &file_status)) == 0
			    && (file_status.st_mode & S_IFMT) != S_IFDIR)
			    esfmodtime = file_status.st_mtime;
		    else
			    esfmodtime = 0;
    
		    /* get mod time for the svp file if needed */
		    if (process.mbp_svp_mode != MBP_SVP_OFF
			    && (fstat = stat(process.mbp_svpfile, &file_status)) == 0
			    && (file_status.st_mode & S_IFMT) != S_IFDIR)
			    svpmodtime = file_status.st_mtime;
		    else
			    svpmodtime = 0;
			    
		    /* now check if processed file is out of date */
		    if (ofilemodtime > 0
			    && ofilemodtime >= ifilemodtime
			    && ofilemodtime >= pfilemodtime
			    && ofilemodtime >= navfilemodtime
			    && ofilemodtime >= navadjfilemodtime
			    && ofilemodtime >= esfmodtime
			    && ofilemodtime >= svpmodtime)
			{
			proceedprocess = MB_NO;
	    		fprintf(stderr,"Data skipped - up to date: %s\n",
				mbp_ifile);
			}
		    else
			{
			fprintf(stderr,"Data processed - out of date: \n\tInput:  %s\n\tOutput: %s\n",
				mbp_ifile, process.mbp_ofile);
			}
		    }
		}

	/* else just do it */
	else
		{
		proceedprocess = MB_YES;
		fprintf(stderr,"Data processed: \n\tInput:  %s\n\tOutput: %s\n",
			mbp_ifile, process.mbp_ofile);
		}
		
	/* now process the input file */
	if (proceedprocess == MB_YES)
	{

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
	    status = mb_format_flags(verbose,&process.mbp_format,
			&variable_beams, &traveltime, &beam_flagging, 
			&error);
	    if (traveltime != MB_YES)
		{
		process.mbp_bathrecalc_mode = MBP_BATHRECALC_OFF;
		fprintf(stderr,"Bathymetry recalculation disabled because format %d does not include travel time data.\n",
			process.mbp_format);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}
	    }

	/* check for right format if recalculating sidescan is on */
	if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON
	    && process.mbp_format != MBF_EM300MBA)
	    {
	    fprintf(stderr,"\nProgram <%s> only recalculates sidescan for format %d\n",program_name,MBF_EM300MBA);
	    fprintf(stderr,"Format %d is specified. Sidescan recalculation disabled\n",process.mbp_format);
	    process.mbp_ssrecalc_mode = MBP_SSRECALC_OFF;
	    }

	/* print starting info statements */
	if (verbose == 1)
	    {
	    fprintf(stderr,"\nInput and Output Files:\n");
	    if (process.mbp_format_specified == MB_YES)
		    fprintf(stderr,"  Format:                        %d\n",process.mbp_format);
	    if (process.mbp_ifile_specified == MB_YES)
		    fprintf(stderr,"  Input file:                    %s\n",process.mbp_ifile);
	    if (process.mbp_ifile_specified == MB_YES)
		    fprintf(stderr,"  Output file:                   %s\n",process.mbp_ofile);
	    if (strip_comments == MB_YES)
		fprintf(stderr,"  Comments in output:            OFF\n");
	    else
		fprintf(stderr,"  Comments in output:            ON\n");

	    fprintf(stderr,"\nNavigation Merging:\n");
	    if (process.mbp_nav_mode == MBP_NAV_ON)
		{
		fprintf(stderr,"  Navigation merged from navigation file.\n");
		if (process.mbp_nav_heading == MBP_NAV_ON)
		    fprintf(stderr,"  Heading merged from navigation file.\n");
		else
		    fprintf(stderr,"  Heading not merged from navigation file.\n");
		if (process.mbp_nav_speed == MBP_NAV_ON)
		    fprintf(stderr,"  Speed merged from navigation file.\n");
		else
		    fprintf(stderr,"  Speed not merged from navigation file.\n");
		if (process.mbp_nav_draft == MBP_NAV_ON)
		    fprintf(stderr,"  Draft merged from navigation file.\n");
		else
		    fprintf(stderr,"  Draft not merged from navigation file.\n");
	    	fprintf(stderr,"  Navigation file:               %s\n", process.mbp_navfile);
	    	if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
			fprintf(stderr,"  Navigation algorithm:          linear interpolation\n");
	    	else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
			fprintf(stderr,"  Navigation algorithm:          spline interpolation\n");
	    	fprintf(stderr,"  Navigation time shift:         %f\n", process.mbp_nav_timeshift);
	    	if (process.mbp_nav_shift == MBP_NAV_ON)
			{
			fprintf(stderr,"  Navigation positions shifted.\n");
			fprintf(stderr,"  Navigation offset x:       %f\n", process.mbp_nav_offsetx);
			fprintf(stderr,"  Navigation offset y:       %f\n", process.mbp_nav_offsety);
			fprintf(stderr,"  Navigation offset z:       %f\n", process.mbp_nav_offsetz);
			}
	    	else 
			fprintf(stderr,"  Navigation positions not shifted.\n");
		}
	    else
		fprintf(stderr,"  Navigation not merged from navigation file.\n");

	    fprintf(stderr,"\nAdjusted Navigation Merging:\n");
	    if (process.mbp_navadj_mode == MBP_NAV_ON)
		fprintf(stderr,"  Navigation merged from adjusted navigation file.\n");
	    else
		fprintf(stderr,"  Navigation not merged from adjusted navigation file.\n");
	    fprintf(stderr,"  Adjusted navigation file:      %s\n", process.mbp_navadjfile);
	    if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
		fprintf(stderr,"  Adjusted navigation algorithm: linear interpolation\n");
	    else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
		fprintf(stderr,"  Adjusted navigation algorithm: spline interpolation\n");

	    fprintf(stderr,"\nData Cutting:\n");
	    if (process.mbp_cut_num > 0)
		fprintf(stderr,"  Data cutting enabled (%d commands).\n", process.mbp_cut_num);
	    else
		fprintf(stderr,"  Data cutting disabled.\n");
	    for (i=0;i<process.mbp_cut_num;i++)
		{
		if (process.mbp_cut_kind[i] == MBP_CUT_DATA_BATH)
		    fprintf(stderr, "  Cut[%d]: bathymetry", i);
		else if (process.mbp_cut_kind[i] == MBP_CUT_DATA_AMP)
		    fprintf(stderr, "  Cut[%d]: amplitude ", i);
		else if (process.mbp_cut_kind[i] == MBP_CUT_DATA_SS)
		    fprintf(stderr, "  Cut[%d]: sidescan  ", i);
		if (process.mbp_cut_mode[i] == MBP_CUT_MODE_NUMBER)
		    fprintf(stderr, "  number   ");
		else if (process.mbp_cut_kind[i] == MBP_CUT_MODE_DISTANCE)
		    fprintf(stderr, "  distance ");
		fprintf(stderr, "  %f %f\n", process.mbp_cut_min[i], process.mbp_cut_max[i]);
		}

	    fprintf(stderr,"\nBathymetry Editing:\n");
	    if (process.mbp_edit_mode == MBP_EDIT_ON)
		fprintf(stderr,"  Bathymetry edits applied from file.\n");
	    else
		fprintf(stderr,"  Bathymetry edits not applied from file.\n");
	    fprintf(stderr,"  Bathymetry edit file:          %s\n", process.mbp_editfile);

	    fprintf(stderr,"\nBathymetry Recalculation:\n");
	    if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
		fprintf(stderr,"  Bathymetry not recalculated.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
		fprintf(stderr,"  Bathymetry recalculated by raytracing.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
		fprintf(stderr,"  Bathymetry recalculated by rigid rotation.\n");
	    else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
		fprintf(stderr,"  Bathymetry recalculated by sonar depth shift.\n");
	    fprintf(stderr,"  SVP file:                      %s\n", process.mbp_svpfile);
	    if (process.mbp_ssv_mode == MBP_SSV_OFF)
		fprintf(stderr,"  SSV not modified.\n");
	    else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
		fprintf(stderr,"  SSV offset by constant.\n");
	    else
		fprintf(stderr,"  SSV set to constant.\n");
	    fprintf(stderr,"  SSV offset/constant:           %f m/s\n", process.mbp_ssv);
	    fprintf(stderr,"  Travel time multiplier:        %f m\n", process.mbp_tt_mult);

	    fprintf(stderr,"\nBathymetry Water Sound Speed Reference:\n");
	    if (process.mbp_corrected == MB_YES)
		fprintf(stderr,"  Output bathymetry reference:   CORRECTED\n");
	    else if (process.mbp_corrected == MB_NO)
		fprintf(stderr,"  Bathymetry reference:          UNCORRECTED\n");
	    if (process.mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF)
		{
		if (process.mbp_corrected == MB_YES)
		    fprintf(stderr,"  Depths modified from uncorrected to corrected\n");
		else
		    fprintf(stderr,"  Depths modified from corrected to uncorrected\n");
		}
	    else if (process.mbp_svp_mode == MBP_SVP_ON)
		{
		if (process.mbp_corrected == MB_YES)
		    fprintf(stderr,"  Depths recalculated as corrected\n");
		else
		    fprintf(stderr,"  Depths recalculated as uncorrected\n");
		}
	    else
		{
		fprintf(stderr,"  Depths unmodified with respect to water sound speed reference\n");
		}

	    fprintf(stderr,"\nDraft Correction:\n");
	    if (process.mbp_draft_mode == MBP_DRAFT_OFF)
		fprintf(stderr,"  Draft not modified.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_SET)
		fprintf(stderr,"  Draft set to constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
		fprintf(stderr,"  Draft offset by constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
		fprintf(stderr,"  Draft multiplied by constant.\n");
	    else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
		fprintf(stderr,"  Draft multiplied and offset by constants.\n");
	    fprintf(stderr,"  Draft constant:                %f m\n", process.mbp_draft);
	    fprintf(stderr,"  Draft offset:                  %f m\n", process.mbp_draft_offset);
	    fprintf(stderr,"  Draft multiplier:              %f m\n", process.mbp_draft_mult);

	    fprintf(stderr,"\nHeave Correction:\n");
	    if (process.mbp_heave_mode == MBP_HEAVE_OFF)
		fprintf(stderr,"  Heave not modified.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_OFFSET)
		fprintf(stderr,"  Heave offset by constant.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
		fprintf(stderr,"  Heave multiplied by constant.\n");
	    else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
		fprintf(stderr,"  Heave multiplied and offset by constants.\n");
	    fprintf(stderr,"  Heave offset:                  %f m\n", process.mbp_heave);
	    fprintf(stderr,"  Heave multiplier:              %f m\n", process.mbp_heave_mult);

	    fprintf(stderr,"\nLever Correction:\n");
	    if (process.mbp_lever_mode == MBP_LEVER_OFF)
		fprintf(stderr,"  Lever calculation off.\n");
	    else
		{
		fprintf(stderr,"  Lever calculation used to calculate heave correction.\n");
	    	fprintf(stderr,"  Heave offset:                  %f m\n", process.mbp_heave);
	    	fprintf(stderr,"  VRU offset x:                  %f m\n", process.mbp_vru_offsetx);
	    	fprintf(stderr,"  VRU offset y:                  %f m\n", process.mbp_vru_offsety);
	    	fprintf(stderr,"  VRU offset z:                  %f m\n", process.mbp_vru_offsetz);
	    	fprintf(stderr,"  Sonar offset x:                %f m\n", process.mbp_sonar_offsetx);
	    	fprintf(stderr,"  Sonar offset y:                %f m\n", process.mbp_sonar_offsety);
	    	fprintf(stderr,"  Sonar offset z:                %f m\n", process.mbp_sonar_offsetz);
		}

	    fprintf(stderr,"\nTide Correction:\n");
	    if (process.mbp_tide_mode == MBP_TIDE_OFF)
		fprintf(stderr,"  Tide calculation off.\n");
	    else
		{
		fprintf(stderr,"  Tide correction applied to bathymetry.\n");
	    	fprintf(stderr,"  Tide file:                     %s m\n", process.mbp_tidefile);
	    	fprintf(stderr,"  Tide format:                   %d m\n", process.mbp_tide_format);
 		}

	    fprintf(stderr,"\nRoll Correction:\n");
	    if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		fprintf(stderr,"  Roll not modified.\n");
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		fprintf(stderr,"  Roll offset by bias.\n");
	    else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		fprintf(stderr,"  Roll offset by separate port and starboard biases.\n");
	    fprintf(stderr,"  Roll bias:                     %f deg\n", process.mbp_rollbias);
	    fprintf(stderr,"  Port roll bias:                %f deg\n", process.mbp_rollbias_port);
	    fprintf(stderr,"  Starboard roll bias:           %f deg\n", process.mbp_rollbias_stbd);

	    fprintf(stderr,"\nPitch Correction:\n");
	    if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		fprintf(stderr,"  Pitch not modified.\n");
	    else
		fprintf(stderr,"  Pitch offset by bias.\n");
	    fprintf(stderr,"  Pitch bias:                    %f deg\n", process.mbp_pitchbias);

	    fprintf(stderr,"\nHeading Correction:\n");
	    if (process.mbp_heading_mode == MBP_HEADING_OFF)
		fprintf(stderr,"  Heading not modified.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_CALC)
		fprintf(stderr,"  Heading replaced by course-made-good.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_OFFSET)
		fprintf(stderr,"  Heading offset by bias.\n");
	    else if (process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
		fprintf(stderr,"  Heading replaced by course-made-good and then offset by bias.\n");
	    fprintf(stderr,"  Heading offset:                %f deg\n", process.mbp_headingbias);

	    fprintf(stderr,"\nSidescan Recalculation:\n");
	    if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON)
		fprintf(stderr,"  Sidescan recalculated.\n");
	    else
		fprintf(stderr,"  Sidescan not recalculated.\n");
	    fprintf(stderr,"  Sidescan pixel size:           %f\n",process.mbp_ssrecalc_pixelsize);
	    fprintf(stderr,"  Sidescan swath width:          %f\n",process.mbp_ssrecalc_swathwidth);
	    fprintf(stderr,"  Sidescan interpolation:        %d\n",process.mbp_ssrecalc_interpolate);

	    fprintf(stderr,"\nMetadata Insertion:\n");
	    fprintf(stderr,"  Metadata operator:             %s\n",process.mbp_meta_operator);
	    fprintf(stderr,"  Metadata platform:             %s\n",process.mbp_meta_platform);
	    fprintf(stderr,"  Metadata sonar:                %s\n",process.mbp_meta_sonar);
	    fprintf(stderr,"  Metadata survey:               %s\n",process.mbp_meta_survey);
	    fprintf(stderr,"  Metadata pi:                   %s\n",process.mbp_meta_pi);
	    fprintf(stderr,"  Metadata client:               %s\n",process.mbp_meta_client);
	    }

	/*--------------------------------------------
	  get svp
	  --------------------------------------------*/

	/* if raytracing or correction/uncorrection to be done get svp */
	if (process.mbp_svp_mode != MBP_SVP_OFF)
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

	    /* set ssv_default */
	    ssv_default = velocity[0];

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
				&time_j[0],&time_j[1],&ihr,
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
		    
	    /* apply time shift if needed */
	    if (process.mbp_nav_timeshift != 0.0)
		for (i=0;i<nnav;i++)
		    ntime[i] += process.mbp_nav_timeshift;
		    
	    /* apply position shift if needed */
    	    if (process.mbp_nav_shift == MBP_NAV_ON)
		{
		for (i=0;i<nnav;i++)
		    {
		    mb_coor_scale(verbose,nlat[i],&mtodeglon,&mtodeglat);
		    headingx = sin(nheading[i] * DTR);
		    headingy = cos(nheading[i] * DTR);
		    nlon[i] -= (headingy * mtodeglon
				* process.mbp_nav_offsetx
			    + headingx * mtodeglon
				* process.mbp_nav_offsety);
		    nlat[i] -= (-headingx * mtodeglat
				* process.mbp_nav_offsetx
			    + headingy * mtodeglat
				* process.mbp_nav_offsety);
		    }
		}
    
	    /* set up spline interpolation of nav points */
	    splineflag = 1.0e30;
	    mb_spline_init(verbose, ntime-1, nlon-1, nnav,
			splineflag, splineflag, nlonspl-1, &error);
	    mb_spline_init(verbose, ntime-1, nlat-1, nnav,
			splineflag, splineflag, nlatspl-1, &error);
    
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
	    mb_spline_init(verbose, natime-1, nalon-1, nanav,
			splineflag, splineflag, nalonspl-1, &error);
	    mb_spline_init(verbose, natime-1, nalat-1, nanav,
			splineflag, splineflag, nalatspl-1, &error);
    
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
	  get tide
	  --------------------------------------------*/

	/* if tide correction to be done get tide */
	if (process.mbp_tide_mode == MBP_TIDE_ON)
	    {
	    /* set max number of characters to be read at a time */
	    nchar = 128;

	    /* count the data points in the tide file */
	    ntide = 0;
	    if ((tfp = fopen(process.mbp_tidefile, "r")) == NULL) 
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    fprintf(stderr,"\nUnable to Open Tide File <%s> for reading\n",process.mbp_tidefile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		    ntide++;
	    fclose(tfp);
	    
	    /* allocate arrays for nav */
	    if (ntide > 1)
		{
		size = (ntide+1)*sizeof(double);
		status = mb_malloc(verbose,ntide*sizeof(double),&tidetime,&error);
		status = mb_malloc(verbose,ntide*sizeof(double),&tide,&error);
 	
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
	
	    /* if no tide data then quit */
	    else
		{
		error = MB_ERROR_BAD_DATA;
		fprintf(stderr,"\nUnable to read data from tide file <%s>\n",process.mbp_tidefile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}		    
		
	    /* read the data points in the tide file */
	    nnav = 0;
	    if ((tfp = fopen(process.mbp_tidefile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Tide File <%s> for reading\n",process.mbp_tidefile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    while ((result = fgets(buffer,nchar,tfp)) == buffer)
		{
		tide_ok = MB_NO;

		/* deal with tide in form: time_d tide */
		if (process.mbp_tide_format == 1)
			{
			nget = sscanf(buffer,"%lf %lf",
				&tidetime[ntide],&tide[ntide]);
			if (nget == 2)
				tide_ok = MB_YES;
			}

		/* deal with tide in form: yr mon day hour min sec tide */
		else if (process.mbp_tide_format == 2)
			{
			nget = sscanf(buffer,"%d %d %d %d %d %lf %lf",
				&time_i[0],&time_i[1],&time_i[2],
				&time_i[3],&time_i[4],&sec,
				&tide[ntide]);
			time_i[5] = (int) sec;
			time_i[6] = 1000000*(sec - time_i[5]);
			mb_get_time(verbose,time_i,&time_d);
			tidetime[ntide] = time_d;
			if (nget == 7)
				tide_ok = MB_YES;
			}

		/* deal with tide in form: yr jday hour min sec tide */
		else if (process.mbp_tide_format == 3)
			{
			nget = sscanf(buffer,"%d %d %d %d %lf %lf",
				&time_j[0],&time_j[1],&ihr,
				&time_j[2],&sec,
				&tide[ntide]);
			time_j[2] = time_j[2] + 60*hr;
			time_j[3] = (int) sec;
			time_j[4] = 1000000*(sec - time_j[3]);
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			tidetime[ntide] = time_d;
			if (nget == 7)
				tide_ok = MB_YES;
			}

		/* deal with tide in form: yr jday daymin sec tide */
		else if (process.mbp_tide_format == 4)
			{
			nget = sscanf(buffer,"%d %d %d %lf %lf",
				&time_j[0],&time_j[1],&time_j[2],
				&sec,
				&tide[ntide]);
			time_j[3] = (int) sec;
			time_j[4] = 1000000*(sec - time_j[3]);
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			tidetime[ntide] = time_d;
			if (nget == 5)
				tide_ok = MB_YES;
			}

		/* output some debug values */
		if (verbose >= 5 && tide_ok == MB_YES)
			{
			fprintf(stderr,"\ndbg5  New tide point read in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
				ntide,tidetime[ntide],tide[ntide]);
			}
		else if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Error parsing line in tide file in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       line: %s\n",buffer);
			}

		/* check for reverses or repeats in time */
		if (tide_ok == MB_YES)
			{
			if (ntide == 0)
				ntide++;
			else if (tidetime[ntide] > tidetime[ntide-1])
				ntide++;
			else if (ntide > 0 && tidetime[ntide] <= tidetime[ntide-1] 
				&& verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Tide time error in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
					ntide-1,tidetime[ntide-1],tide[ntide-1]);
				fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
					ntide,tidetime[ntide],tide[ntide]);
				}
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	    fclose(tfp);

		
	    /* check for tide */
	    if (ntide < 2)
		    {
		    fprintf(stderr,"\nNo tide read from file <%s>\n",process.mbp_tidefile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
     
	    /* get start and finish times of tide */
	    mb_get_date(verbose,tidetime[0],stime_i);
	    mb_get_date(verbose,tidetime[ntide-1],ftime_i);
    
	    /* give the statistics */
	    if (verbose >= 1)
		    {
		    fprintf(stderr,"\n%d tide records read\n",ntide);
		    fprintf(stderr,"Tide start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			    stime_i[0],stime_i[1],stime_i[2],stime_i[3],
			    stime_i[4],stime_i[5],stime_i[6]);
		    fprintf(stderr,"Tide end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
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
	if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE
		&& process.mbp_ssv_mode != MBP_SSV_SET)
	    {
	    ssv_start = 0.0;
	    ssv_prelimpass = MB_YES;
	    error = MB_ERROR_NO_ERROR;
	    while (error <= MB_ERROR_NO_ERROR
		&& ssv_start <= 0.0)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
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
	if (strip_comments == MB_NO)
		{
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
		    if (process.mbp_corrected == MB_NO)
			    sprintf(comment,"  uncorrected meters (the depth values are adjusted to be");
		    else
			    sprintf(comment,"  corrected meters (the depth values obtained by");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    if (process.mbp_corrected == MB_NO)
			    sprintf(comment,"  consistent with a vertical water velocity of 1500 m/s).");
		    else
			    sprintf(comment,"  raytracing are not adjusted further).");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }		    
		else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"Depths and crosstrack distances adjusted for roll bias");
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
		    sprintf(comment,"Depths and crosstrack distances adjusted for ");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  change in transducer depth and/or heave.");
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
		    if (angle_mode == MBP_ANGLES_OK)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Angle mode:         angles not altered");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		    else if (angle_mode == MBP_ANGLES_SNELL)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Angle mode:         angles corrected using Snell's Law");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		    else if (angle_mode == MBP_ANGLES_SNELLNULL)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Angle mode:         angles corrected using Snell's Law and array geometry");
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
		if (process.mbp_svp_mode != MBP_SVP_OFF)
		    {
		    if (process.mbp_corrected == MB_YES)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"Output bathymetry reference:   CORRECTED");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			}
		    else if (process.mbp_corrected == MB_NO)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"Output bathymetry reference:   UNCORRECTED");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			}
		    }		    
		if (process.mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF)
		    {
		    if (process.mbp_corrected == MB_YES)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"Depths modified from uncorrected to corrected.");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			}		    
		    else
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"Depths modified from corrected to uncorrected.");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			}
		    }
	
		if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  roll bias:       OFF");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  roll bias:       %f degrees (starboard: -, port: +)", 
			    process.mbp_rollbias);
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  port roll bias:  %f degrees (starboard: -, port: +)", 
			    process.mbp_rollbias_port);
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  port roll stbd:  %f degrees (starboard: -, port: +)", 
			    process.mbp_rollbias_stbd);
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  pitch bias:      OFF");
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
		else if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
		    {
		    strncpy(comment,"\0",MBP_FILENAMESIZE);
		    sprintf(comment,"  pitch bias:      %f degrees (aft: -, forward: +)", 
			    process.mbp_pitchbias);
		    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		    if (error == MB_ERROR_NO_ERROR) ocomment++;
		    }
	
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
				process.mbp_draft_offset);
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
				process.mbp_draft_offset);
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
		if (process.mbp_heave_mode == MBP_HEAVE_OFFSET)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heave offset: %f meters",
				process.mbp_heave);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heave multiplier: %f",
				process.mbp_heave_mult);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heave offset: %f meters",
				process.mbp_heave);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heave multiplier: %f",
				process.mbp_heave_mult);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_heave_mode == MBP_HEAVE_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heave:        not modified");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (process.mbp_tt_mode == MBP_TT_MULTIPLY)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Travel time multiplier: %f",
				process.mbp_tt_mult);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_tt_mode == MBP_TT_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Travel time:     not modified");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
	    	if (process.mbp_lever_mode == MBP_LEVER_OFF)
			{
			sprintf(comment,"  Lever calculation off.\n");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
	    	else
			{
			sprintf(comment,"  Lever calculation used to calculate heave correction.\n");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  VRU offset x:                  %f m\n", process.mbp_vru_offsetx);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  VRU offset y:                  %f m\n", process.mbp_vru_offsety);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  VRU offset z:                  %f m\n", process.mbp_vru_offsetz);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  Sonar offset x:                %f m\n", process.mbp_sonar_offsetx);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  Sonar offset y:                %f m\n", process.mbp_sonar_offsety);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  Sonar offset z:                %f m\n", process.mbp_sonar_offsetz);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
	    	if (process.mbp_tide_mode == MBP_TIDE_OFF)
			{
			sprintf(comment,"  Tide calculation off.\n");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
	    	else
			{
			sprintf(comment,"  Tide correction applied to bathymetry.\n");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  Tide file:                     %s m\n", process.mbp_tidefile);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		sprintf(comment,"  Tide format:                   %d m\n", process.mbp_tide_format);
 			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (process.mbp_navadj_mode == MBP_NAV_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Merge adjusted navigation: OFF");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_navadj_mode == MBP_NAV_ON)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Adjusted navigation file: %s", process.mbp_navadjfile);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
			    sprintf(comment,"  Adjusted navigation algorithm: linear interpolation");
			else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
			    sprintf(comment,"  Adjusted navigation algorithm: spline interpolation");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (process.mbp_nav_mode == MBP_NAV_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Merge edited navigation:     OFF");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_nav_mode == MBP_NAV_ON)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Edited navigation file:      %s", process.mbp_navfile);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Edited navigation format:    %d", process.mbp_nav_format);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	
			if (process.mbp_nav_heading == MBP_NAV_ON)
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Heading merge:    ON");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			else
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Heading merge:    OFF");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			if (process.mbp_nav_speed == MBP_NAV_ON)
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Speed merge:      ON");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			else
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Speed merge:      OFF");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			if (process.mbp_nav_draft == MBP_NAV_ON)
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Draft merge:      ON");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			else
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Draft merge:      OFF");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Navigation algorithm: linear interpolation");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
			else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
			    {
			    strncpy(comment,"\0",MBP_FILENAMESIZE);
			    sprintf(comment,"  Navigation algorithm: spline interpolation");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
			    }
	    		sprintf(comment,"  Navigation time shift:         %f\n", process.mbp_nav_timeshift);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
	    		if (process.mbp_nav_shift == MBP_NAV_ON)
				{
				sprintf(comment,"  Navigation positions shifted.\n");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
				sprintf(comment,"  Navigation offset x:       %f\n", process.mbp_nav_offsetx);
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
				sprintf(comment,"  Navigation offset y:       %f\n", process.mbp_nav_offsety);
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
				sprintf(comment,"  Navigation offset z:       %f\n", process.mbp_nav_offsetz);
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
				}
	    		else 
				{
				sprintf(comment,"  Navigation positions not shifted.\n");
			    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			    if (error == MB_ERROR_NO_ERROR) ocomment++;
				}
			}
		if (process.mbp_heading_mode == MBP_HEADING_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heading modify:       OFF");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (process.mbp_heading_mode == MBP_HEADING_CALC
				|| process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heading modify:       COURSE MADE GOOD");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (process.mbp_heading_mode == MBP_HEADING_OFFSET
				|| process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Heading offset:       %f deg", process.mbp_headingbias);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}

		strncpy(comment,"\0",MBP_FILENAMESIZE);
		if (process.mbp_cut_num > 0)
			sprintf(comment,"  Data cutting enabled (%d commands).\n", process.mbp_cut_num);
		else
			sprintf(comment,"  Data cutting disabled.\n");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		for (i=0;i<process.mbp_cut_num;i++)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment, "  Cut[%d]: %d %d %f %f", 
				i, process.mbp_cut_kind[i], process.mbp_cut_mode[i], 
				process.mbp_cut_min[i], process.mbp_cut_max[i]);
			sprintf(comment, "  %f %f\n", process.mbp_cut_min[i], process.mbp_cut_max[i]);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}

		if (process.mbp_edit_mode == MBP_EDIT_OFF)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Herge bath edit:      OFF");
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		else if (process.mbp_edit_mode == MBP_EDIT_ON)
			{
			strncpy(comment,"\0",MBP_FILENAMESIZE);
			sprintf(comment,"  Bathy edit file:      %s", process.mbp_editfile);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
	
		strncpy(comment,"\0",MBP_FILENAMESIZE);
		sprintf(comment," ");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;

		/* insert metadata */
		if (strlen(process.mbp_meta_operator) > 0)
			{
			sprintf(comment,"METAOPERATOR:%s", process.mbp_meta_operator);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (strlen(process.mbp_meta_platform) > 0)
			{
			sprintf(comment,"METAPLATFORM:%s", process.mbp_meta_platform);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (strlen(process.mbp_meta_sonar) > 0)
			{
			sprintf(comment,"METASONAR:%s", process.mbp_meta_sonar);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (strlen(process.mbp_meta_survey) > 0)
			{
			sprintf(comment,"METASURVEY:%s", process.mbp_meta_survey);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (strlen(process.mbp_meta_pi) > 0)
			{
			sprintf(comment,"METAPI:%s", process.mbp_meta_pi);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		if (strlen(process.mbp_meta_client) > 0)
			{
			sprintf(comment,"METACLIENT:%s", process.mbp_meta_client);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		}

	/* set up the raytracing */
	status = mb_rt_init(verbose, nsvp, depth, velocity, &rt_svp, &error);
	
	/* set up the sidescan recalculation */
	if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON)
		{
		if (process.mbp_ssrecalc_pixelsize != 0.0)
			{
			pixel_size_set = MB_YES;
			pixel_size = process.mbp_ssrecalc_pixelsize;
			}
		else
			{
			pixel_size_set = MB_NO;
			pixel_size = 0.0;
			}
		if (process.mbp_ssrecalc_swathwidth != 0.0)
			{
			swath_width_set = MB_YES;
			swath_width = process.mbp_ssrecalc_swathwidth;
			}
		else
			{
			swath_width_set = MB_NO;
			swath_width = 0.0;
			}
		pixel_int = process.mbp_ssrecalc_interpolate;
		}

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&nbath,&namp,&nss,
				beamflag,bath,amp,
				bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata++;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_NAV)
			inav++;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;
		else if (error <= MB_ERROR_NO_ERROR)
			iother++;

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
			
		/* do lever calculation to find heave implied by roll and pitch
		   for a sonar displaced from the vru - this will be added to the
		   bathymetry
		   	x = r * COS(alpha) * COS(beta) 
		   	y = r * SIN(alpha)
		   	z = r * COS(alpha) * SIN(beta) */
	    	if (process.mbp_lever_mode == MBP_LEVER_ON)
			{
			dx = process.mbp_sonar_offsetx - process.mbp_vru_offsetx;
			dy = process.mbp_sonar_offsety - process.mbp_vru_offsety;
			dz = process.mbp_sonar_offsetz - process.mbp_vru_offsetz;
			r = sqrt(dx * dx + dy * dy + dz * dz);
			if (r > 0.0)
			    {
			    alpha = RTD * asin(dy / r);
			    if (cos(DTR * alpha) != 0.0)
				beta = RTD * acos(dx / (r * cos(DTR * alpha)));
			    else
				beta = 0.0;
			    alpha += pitch + process.mbp_pitchbias;
			    beta += roll + process.mbp_rollbias;
			    lever_heave =  r * cos(DTR * alpha) * sin(DTR * beta) - dz;
			    }
/*fprintf(stderr, "dx:%f dy:%f dz:%f alpha:%f beta:%f lever:%f\n", 
dx, dy, dz, alpha, beta, lever_heave);*/
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
			    intstat = mb_spline_interp(verbose, 
					ntime-1, nlon-1, nlonspl-1,
					nnav, time_d, &navlon, &itime, 
					&error);
			    intstat = mb_spline_interp(verbose, 
					ntime-1, nlat-1, nlatspl-1,
					nnav, time_d, &navlat, &itime, 
					&error);
			    }
			else
			    {
			    intstat = mb_linear_interp(verbose, 
					ntime-1, nlon-1,
					nnav, time_d, &navlon, &itime, 
					&error);
			    intstat = mb_linear_interp(verbose, 
					ntime-1, nlat-1,
					nnav, time_d, &navlat, &itime, 
					&error);
			    }
			    
			/* interpolate heading */
			if (process.mbp_nav_heading == MBP_NAV_ON)
			    {
			    intstat = mb_linear_interp(verbose, 
					ntime-1, nheading-1,
					nnav, time_d, &heading, &itime, 
					&error);
			    }
			    
			/* interpolate speed */
			if (process.mbp_nav_speed == MBP_NAV_ON)
			    {
			    intstat = mb_linear_interp(verbose, 
					ntime-1, nspeed-1,
					nnav, time_d, &speed, &itime, 
					&error);
			    }
			    
			/* interpolate draft */
			if (process.mbp_nav_draft == MBP_NAV_ON)
			    {
			    intstat = mb_linear_interp(verbose, 
					ntime-1, ndraft-1,
					nnav, time_d, &draft, &itime, 
					&error);
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
			    intstat = mb_spline_interp(verbose, 
					natime-1, nalon-1, nalonspl-1,
					nanav, time_d, &navlon, &iatime, 
					&error);
			    intstat = mb_spline_interp(verbose, 
					ntime-1, nalat-1, nalatspl-1,
					nanav, time_d, &navlat, &iatime, 
					&error);
			    }
			else
			    {
			    intstat = mb_linear_interp(verbose, 
					natime-1, nalon-1,
					nanav, time_d, &navlon, &iatime, 
					&error);
			    intstat = mb_linear_interp(verbose, 
					natime-1, nalat-1,
					nanav, time_d, &navlat, &iatime, 
					&error);
			    }
			}
    
		/* add user specified draft correction if desired */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV))
			{		
			if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
				draft = draft + process.mbp_draft_offset;
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
				draft = draft * process.mbp_draft_mult;
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
				draft = draft * process.mbp_draft_mult + process.mbp_draft_offset;
			else if (process.mbp_draft_mode == MBP_DRAFT_SET)
				draft = process.mbp_draft;
			}

		/* make up heading and speed if required */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV)
			&& (process.mbp_heading_mode == MBP_HEADING_CALC
			    || process.mbp_heading_mode == MBP_HEADING_CALCOFFSET))
			{
			if (process.mbp_nav_mode == MBP_NAV_ON)
			    {
			    mb_coor_scale(verbose,nlat[itime-1],&mtodeglon,&mtodeglat);
			    del_time = ntime[itime] - ntime[itime-1];
			    dx = (nlon[itime] - nlon[itime-1])/mtodeglon;
			    dy = (nlat[itime] - nlat[itime-1])/mtodeglat;
			    }
			else if (process.mbp_navadj_mode == MBP_NAV_ON)
			    {
			    mb_coor_scale(verbose,nalat[iatime-1],&mtodeglon,&mtodeglat);
			    del_time = natime[iatime] - natime[iatime-1];
			    dx = (nalon[iatime] - nalon[iatime-1])/mtodeglon;
			    dy = (nalat[iatime] - nalat[iatime-1])/mtodeglat;
			    }
			else if (kind == MB_DATA_DATA && idata > 1
					|| kind == MB_DATA_NAV && inav > 1)
			    {
			    mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
			    del_time = time_d - time_d_old;
			    dx = (navlon - navlon_old)/mtodeglon;
			    dy = (navlat - navlat_old)/mtodeglat;
			    }
			if (process.mbp_nav_mode == MBP_NAV_ON
				|| process.mbp_navadj_mode == MBP_NAV_ON
				|| (kind == MB_DATA_DATA && idata > 1
					|| kind == MB_DATA_NAV && inav > 1))
			    {
			    dist = sqrt(dx*dx + dy*dy);
			    if (del_time > 0.0)
				{
				speed = 3.6*dist/del_time;
				}
			    else
				speed = speed_old;
			    if (dist > 0.0)
				{
				heading = RTD*atan2(dx/dist,dy/dist);
				}
			    else
				heading = heading_old;
			    }
			time_d_old = time_d;
			navlon_old = navlon;
			navlat_old = navlat;
			heading_old = heading;
			speed_old = speed;
			}

		/* adjust heading if required */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_NAV)
			&& (process.mbp_heading_mode == MBP_HEADING_OFFSET
			    || process.mbp_heading_mode == MBP_HEADING_CALCOFFSET))
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
				
			/* if heave adjustment specified do it */
			if (process.mbp_heave_mode != MBP_HEAVE_OFF)
			    {
			    if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY
				|| process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
				{
				for (i=0;i<beams_bath;i++)
				    bheave[i] *= process.mbp_heave_mult;
				}
			    if (process.mbp_heave_mode == MBP_HEAVE_OFFSET
				|| process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
				{
				for (i=0;i<beams_bath;i++)
				    bheave[i] += process.mbp_heave;
				}
			    }
				
			/* if tt adjustment specified do it */
			if (process.mbp_tt_mode == MBP_TT_MULTIPLY)
			    {
			    for (i=0;i<beams_bath;i++)
				ttimes[i] *= process.mbp_tt_mult;
			    }
				
			/* if ssv adjustment specified do it */
			if (process.mbp_ssv_mode == MBP_SSV_SET)
			    {
			    ssv = process.mbp_ssv;
			    }
			else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
			    {
			    ssv += process.mbp_ssv;
			    }

			/* if svp specified recalculate bathymetry
			    by raytracing  */
			if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
			    {
			    /* loop over the beams */
			    for (i=0;i<nbeams;i++)
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
				depth_offset_use = bheave[i] + draft + lever_heave;
				static_shift = 0.0;
	
				/* check depth_offset - use static shift if depth_offset negative */
				if (depth_offset_use < 0.0)
				    {
				    fprintf(stderr, "\nWarning: Depth offset negative - transducers above water?!\n");
				    fprintf(stderr, "Raytracing performed from zero depth followed by static shift.\n");
				    fprintf(stderr, "Depth offset is sum of heave + transducer depth.\n");
				    fprintf(stderr, "Draft from data:       %f\n", draft);
				    fprintf(stderr, "Heave from data:       %f\n", bheave[i]);
				    fprintf(stderr, "Heave from lever calc: %f\n", lever_heave);
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
					angle_mode, ssv, angles_null[i], 
					0, NULL, NULL, NULL, 
					&xx, &zz, 
					&ttime, &ray_stat, &error);
					
				/* apply static shift if needed */
				if (static_shift < 0.0)
				    zz += static_shift;
/*fprintf(stderr, "%d %d : heave:%f draft:%f depth_offset:%f static:%f zz:%f\n", 
idata, i, bheave[i], draft, depth_offset_use, static_shift, zz);*/
 
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
				depth_offset_use = bheave[i] + draft + lever_heave;
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
			    depth_offset_change = draft - draft_org + lever_heave;
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
			    
			/* change bathymetry water sound reference if required */
			if (process.mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF
			    || (process.mbp_svp_mode == MBP_SVP_ON
				&& process.mbp_corrected == MB_NO))
			    {
			    for (i=0;i<beams_bath;i++)
				{
				if (beamflag[i] != MB_FLAG_NULL)
				    {
				    /* calculate average water sound speed 
					for current depth value */
				    depth_offset_use = bheave[i] + draft + lever_heave;
				    zz = bath[i] - depth_offset_use;
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
					}
				    if (vavg <= 0.0) vavg = 1500.0;
					
				    /* if uncorrected value desired */
				    if (process.mbp_corrected == MB_NO)
					bath[i] = zz * 1500.0 / vavg + depth_offset_use;
				    else
					bath[i] = zz * vavg / 1500.0 + depth_offset_use;

				    }
				}
			    }

			/* output some debug messages */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Depth values calculated in program <%s>:\n",program_name);
			    fprintf(stderr,"dbg5       kind:  %d\n",kind);
			    fprintf(stderr,"dbg5      beam    time      depth        dist\n");	
			    for (i=0;i<beams_bath;i++)
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

		/* apply data cutting if specified */
		if (process.mbp_cut_num > 0
		    && error == MB_ERROR_NO_ERROR
		    && kind == MB_DATA_DATA)
		    {
		    for (icut=0;icut<process.mbp_cut_num;icut++)
			{
			/* flag data according to beam number range */
			if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_BATH
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER)
			    {
			    istart = MAX((int)process.mbp_cut_min[icut], 0);
			    iend = MIN((int)process.mbp_cut_max[icut], beams_bath - 1);
			    for (i=istart;i<=iend;i++)
				{
				if (mb_beam_ok(beamflag[i]))
					beamflag[i]= MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
			    }

			/* flag data according to beam 
				acrosstrack distance */
			else if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_BATH
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=beams_bath;i++)
				{
				if (mb_beam_ok(beamflag[i]) 
				    && bathacrosstrack[i] >= process.mbp_cut_min[icut]
				    && bathacrosstrack[i] <= process.mbp_cut_max[icut])
					beamflag[i]= MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
			    }
			/* flag data according to beam number range */
			else if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_AMP
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER)
			    {
			    istart = MAX((int)process.mbp_cut_min[icut], 0);
			    iend = MIN((int)process.mbp_cut_max[icut], beams_amp - 1);
			    for (i=istart;i<=iend;i++)
				{
				if (mb_beam_ok(beamflag[i]))
					beamflag[i]= MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
			    }

			/* flag data according to beam 
				acrosstrack distance */
			else if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_AMP
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=beams_amp;i++)
				{
				if (mb_beam_ok(beamflag[i]) 
				    && bathacrosstrack[i] >= process.mbp_cut_min[icut]
				    && bathacrosstrack[i] <= process.mbp_cut_max[icut])
					beamflag[i]= MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
			    }
			/* flag data according to pixel number range */
			else if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_SS
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER)
			    {
			    istart = MAX((int)process.mbp_cut_min[icut], 0);
			    iend = MIN((int)process.mbp_cut_max[icut], pixels_ss - 1);
			    for (i=istart;i<=iend;i++)
				{
				ss[i] = 0.0;
				}
			    }

			/* flag data according to pixel 
				acrosstrack distance */
			else if (process.mbp_cut_kind[icut] == MBP_CUT_DATA_SS
			    && process.mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=pixels_ss;i++)
				{
				if (ssacrosstrack[i] >= process.mbp_cut_min[icut]
				    && ssacrosstrack[i] <= process.mbp_cut_max[icut])
					ss[i]= 0.0;
				}
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

		/* insert the altered data if available */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA
			    || kind == MB_DATA_COMMENT))
			{
			status = mb_insert(verbose,imbio_ptr,
					store_ptr,kind, 
					time_i,time_d,
					navlon,navlat,speed,heading,
					nbath,namp,nss,
					beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			}

		/* recalculate the sidescan */
		if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON
		    && error == MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			{
			status = mbsys_simrad2_makess(verbose,
					imbio_ptr,store_ptr,
					pixel_size_set,&pixel_size, 
					swath_width_set,&swath_width, 
					pixel_int, 
					&error);
			}

		/* write some data */
		if (error == MB_ERROR_NO_ERROR
			|| (kind == MB_DATA_COMMENT 
				&& strip_comments == MB_NO))
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,MB_NO,kind,
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
		fprintf(stderr,"%d input nav records\n",inav);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d input other records\n",iother);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output nav records\n",onav);
		fprintf(stderr,"%d output comment records\n",ocomment);
		fprintf(stderr,"%d output other records\n",oother);
		}
		
	/* generate inf file */
	if (status == MB_SUCCESS)
		{
		if (verbose >= 1)
			fprintf(stderr,"\nGenerating inf file for %s\n",process.mbp_ofile);
		sprintf(user, "mbinfo -F %d -I %s -G > %s.inf", 
			process.mbp_format, process.mbp_ofile, process.mbp_ofile);
		system(user);
		}
		
	} /* end processing file */

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    mbp_ifile,&format,&file_weight,&error)
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }


	} /* end loop over datalist */

	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);


	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/

