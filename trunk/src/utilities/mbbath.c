/*--------------------------------------------------------------------
 *    The MB-system:	mbbath.c	3/31/93
 *    $Id: mbbath.c,v 4.29 2000-09-30 07:06:28 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * MBBATH is a tool for processing swath sonar data.  This program
 * calculates bathymetry from the travel time data by raytracing
 * through a layered water velocity model.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	March 31, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.28  2000/09/19  22:21:09  caress
 * Added -T option for travel time scaling.
 *
 * Revision 4.27  2000/03/08  00:03:45  caress
 * Release 4.6.10
 *
 * Revision 4.26  1999/01/01  23:34:40  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.25  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.24  1997/09/15  19:11:06  caress
 * Real Version 4.5
 *
 * Revision 4.23  1997/07/25  14:28:10  caress
 * Version 4.5beta2
 *
 * Revision 4.22  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.22  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.21  1996/08/26  17:35:08  caress
 * Release 4.4 revision.
 *
 * Revision 4.20  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.19  1996/01/26  21:25:58  caress
 * Version 4.3 distribution
 *
 * Revision 4.18  1995/10/23  19:32:03  caress
 * Now user specified draught is added to depth offset rather
 * than replacing it.
 *
 * Revision 4.17  1995/10/23  19:26:45  caress
 * Now uses depth offset as correction after bathymetry is
 * calculated rather than before raytracing.
 *
 * Revision 4.16  1995/09/22  18:40:21  caress
 * Added SB2100 specific fix for bad range scale.
 *
 * Revision 4.15  1995/08/21  17:25:25  caress
 * Changed handling of draught, since that value is
 * now provided automatically with the travel times.
 *
 * Revision 4.14  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.13  1995/03/22  19:52:56  caress
 * Fixed some ANSI C compliance details.
 *
 * Revision 4.12  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.11  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.10  1995/02/27  14:43:18  caress
 * Fixed bug regarding closing a text input file.
 *
 * Revision 4.9  1995/02/14  21:18:41  caress
 * Version 4.2
 *
 * Revision 4.8  1994/11/29  00:42:55  caress
 * Fixed errors in data comments and verbose messages.
 *
 * Revision 4.7  1994/11/24  01:56:52  caress
 * First cut at gradient raytracing version of mbbath.
 *
 * Revision 4.6  1994/11/09  22:01:21  caress
 * Fixed problems with initializing raytracing tables.
 *
 * Revision 4.5  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.4  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.3  1994/04/12  18:58:00  caress
 * Fixed fprintf statements on lines 689 and 691.
 *
 * Revision 4.2  1994/04/12  17:01:29  caress
 * First cut at mbbath.  Will need to deal with alongtrack angles
 * as well as acrosstrack angles.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.6  1993/11/05  18:13:20  caress
 * Fixed some minor details, e.g. updated the comments written
 * into the output data stream, and updated the manual page.
 *
 * Revision 3.5  1993/11/03  21:17:12  caress
 * Added a capability to read in and apply static corrections
 * as a function of beam number.
 * Also fixed a few typos in comment lines.
 *
 * Revision 3.4  1993/08/16  22:22:25  caress
 * Don't know what the changes are exactly.
 *
 * Revision 3.3  1993/06/21  01:25:06  caress
 * Added ability to read in and use roll corrections.
 *
 * Revision 3.2  1993/06/09  08:38:09  caress
 * Changed way takeoff angles of beams are calculated. Now the
 * beam angle spacing is taken from mbsys_hsds.h file. The
 * Hydrosweep documentation gives the beam spacing as 1.525 degrees
 * but a close examination of some data (6/9/93) shows that Hydrosweep
 * must use a value more like 1.510 degrees internally.
 * Also removed velocity bias option as the creation of HSVELOCITYTOOL
 * obviates the need for that option.
 *
 * Revision 3.1  1993/05/25  04:55:58  caress
 * Added velocity bias option.
 *
 * Revision 3.0  1993/05/04  22:26:11  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_sb2100.h"

/* mbbath defines */
#define SSV_CORRECT	1
#define SSV_INCORRECT	2

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbbath.c,v 4.29 2000-09-30 07:06:28 caress Exp $";
	static char program_name[] = "MBBATH";
	static char help_message[] =  "MBBATH calculates bathymetry from \
the travel time data by raytracing \nthrough a layered water velocity \
model. The depths may be saved as \ncalculated by raytracing (corrected \
meters) or adjusted as if the \nvertical water velocity is 1500 m/s \
(uncorrected meters). The default \ninput and output streams are stdin \
and stdout.";
	static char usage_message[] = "mbbath [-Brollbias -C \
-Ddraft -Fformat  \n\t-Iinfile -Kssv -Ooutfile -Ppitch_bias -Rrollfile \
\n\t-Sstaticfile -Tttscale -U -Wvelfile -Z -V -H]";

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
	char	ifile[128];
	char	*imbio_ptr = NULL;
	char	ofile[128];
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
	int	ocomment = 0;
	char	comment[256];

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* velocity profile handling variables */
	int	use_svp = MB_NO;
	char	vfile[128];
	double	roll_bias;
	double	pitch_bias;
	double	roll_angle_correction;
	double	pitch_angle_correction;
	double	range, alpha, beta;
	int	draught_use = MB_NO;
	double	draught;
	int	uncorrected;
	FILE	*vfp;
	int	nvel;
	double	*depth = NULL;
	double	*velocity = NULL;
	double	*velocity_sum = NULL;
	char	*rt_svp;

	/* ssv handling variables */
	int	ssv_mode = SSV_CORRECT;
	int	ssv_prelimpass = MB_YES;
	double	ssv_default;
	double	ssv_start;
	
	/* travel time scaling variables */
	int	ttscale_mode = MB_NO;
	double	ttscale;

	/* roll error correction handling variables */
	char	rfile[128];
	int	nroll;
	double	*roll_time = NULL;
	double	*roll_corr = NULL;
	double	roll_correction;

	/* static bathymetry correction handling variables */
	char	sfile[128];
	int	nbath_corr;
	double	*bath_corr = NULL;

	/* survey ping raytracing arrays */
	double	*ttimes = NULL;
	double	*angles = NULL;
	double	*angles_forward = NULL;
	double	*angles_null = NULL;
	double	*heave = NULL;
	double	*alongtrack_offset = NULL;
	double	draft;
	double	ssv;
	double	draft_use;
	double	depth_offset_use;
	double	ttime;
	int	ray_stat;
	int	fix_2100_tt = MB_NO;

	/* sb2100 store ptr */
	struct mbsys_sb2100_struct *store;

	char	buffer[128], *result;
	int	size;
	double	static_shift;
	int	nbeams;
	double	zz, xx, vavg, vsum;
	double	value_max;
	int	i, j, k, l, m;
	
	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults */
	format = MBF_HSATLRAW;
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
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (vfile, "\0");
	strcpy (rfile, "\0");
	strcpy (sfile, "\0");
	nroll = 0;
	nbath_corr = 0;

	/* set default control parameters */
	roll_bias = 0.0;
	pitch_bias = 0.0;
	roll_correction = 0.0;
	draught = 0.0;
	ssv_default = 1500.0;
	uncorrected = MB_NO;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:CcD:d:F:f:I:i:K:k:O:o:P:p:R:r:S:s:T:t:UuW:w:XxZz")) != -1)
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
		case 'B':
		case 'b':
			sscanf (optarg,"%lf", &roll_bias);
			flag++;
			break;
		case 'C':
		case 'c':
			ssv_mode = SSV_INCORRECT;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%lf", &draught);
			draught_use = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf (optarg,"%lf", &ssv_default);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%lf", &pitch_bias);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%s", rfile);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%s", sfile);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &ttscale);
			ttscale_mode = MB_YES;
			flag++;
			break;
		case 'U':
		case 'u':
			uncorrected = MB_YES;
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", vfile);
			use_svp = MB_YES;
			flag++;
			break;
		case 'X':
		case 'x':
			fix_2100_tt = MB_YES;
			flag++;
			break;
		case 'Z':
		case 'z':
			ssv_prelimpass = MB_NO;
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

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
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
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       output file:     %s\n",ofile);
		fprintf(stderr,"dbg2       velocity file:   %s\n",vfile);
		fprintf(stderr,"dbg2       use_svp:         %d\n",use_svp);
		fprintf(stderr,"dbg2       roll bias:       %f\n",roll_bias);
		fprintf(stderr,"dbg2       pitch bias:      %f\n",pitch_bias);
		fprintf(stderr,"dbg2       draught_use:     %d\n",draught_use);
		fprintf(stderr,"dbg2       draught:         %f\n",draught);
		fprintf(stderr,"dbg2       ssv_default:     %f\n",ssv_default);
		fprintf(stderr,"dbg2       roll file:       %s\n",rfile);
		fprintf(stderr,"dbg2       statics file:    %s\n",sfile);
		if (ttscale_mode == MB_YES)
		    fprintf(stderr,"dbg2       tt scale:        %f\n",ttscale);
		else
		    fprintf(stderr,"dbg2       tt scale:        OFF\n");
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"MB-System Version %s\n",MB_VERSION);
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* check for format with travel time data */
	if (use_svp == MB_YES)
	    {
	    status = mb_format(verbose,&format,&format_num,&error);
	    if (mb_traveltime_table[format_num] != MB_YES)
		{
		fprintf(stderr,"\nProgram <%s> requires travel time data to recalculate\n",program_name);
		fprintf(stderr,"bathymetry from travel times and angles.\n");
		fprintf(stderr,"Format %d is unacceptable because it does not inlude travel time data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}
	    }

	/* if raytracing to be done get svp */
	if (use_svp == MB_YES)
		{
		/* if a velocity profile file has been specified then
			count the velocity points */
		if (((int) strlen(vfile)) > 0)
			{
			nvel = 0;
			if ((vfp = fopen(vfile, "r")) == NULL) 
				{
				error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",vfile);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			while ((result = fgets(buffer,128,vfp)) == buffer)
				if (buffer[0] != '#')
					nvel++;
			fclose(vfp);
			}		
		/* else assume a 1500 m/s half space */
		else
			nvel = 2;
	
		/* allocate space for the velocity profile and raytracing tables */
		size = (nvel+1)*sizeof(double);
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
	
		/* if a velocity profile file has been specified then
			read in velocity profile data */
		if (((int)strlen(vfile)) > 0)
			{
			nvel = 0;
			if ((vfp = fopen(vfile, "r")) == NULL) 
				{
				error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr,"\nUnable to Open Velocity Profile File <%s> for reading\n",vfile);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			strncpy(buffer,"\0",sizeof(buffer));
			while ((result = fgets(buffer,128,vfp)) == buffer)
			  {
			  if (buffer[0] != '#')
				{
				m = sscanf(buffer,"%lf %lf",&depth[nvel],&velocity[nvel]);
	
				/* output some debug values */
				if (verbose >= 5 && m == 2)
					{
					fprintf(stderr,"\ndbg5  New velocity value read in program <%s>\n",program_name);
					fprintf(stderr,"dbg5       depth[%d]: %f  velocity[%d]: %f\n",
						nvel,depth[nvel],nvel,velocity[nvel]);
					}
				if (m == 2)
					nvel++;
				}
			  strncpy(buffer,"\0",sizeof(buffer));
			  }
			fclose(vfp);
			}
		/* else assume a 1500 m/s half space */
		else
			{
			nvel = 2;
			velocity[0] = 1500.0;
			velocity[1] = 1500.0;
			depth[0] = 0.0;
			depth[1] = 12000.0;
			}
	
		/* if velocity profile doesn't extend to 12000 m depth
			extend it to that depth */
		if (depth[nvel-1] < 12000.0)
			{
			depth[nvel] = 12000.0;
			velocity[nvel] = velocity[nvel-1];
			nvel++;
			}
	
		/* get velocity sums */
		velocity_sum[0] = 0.5*(velocity[1] + velocity[0])
			*(depth[1] - depth[0]);
		for (i=1;i<nvel-1;i++)
			{
			velocity_sum[i] = velocity_sum[i-1] 
			    + 0.5*(velocity[i+1] + velocity[i])
			    *(depth[i+1] - depth[i]);
			}
		}

	/* if a roll correction file has been specified then
		count the roll correction points */
	if (((int) strlen(rfile)) > 0)
		{
		nroll = 0;
		if ((vfp = fopen(rfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Roll Correction File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,128,vfp)) == buffer)
			{
			if (buffer[0] != '#')
				nroll++;
			}
		fclose(vfp);
		}

	/* allocate space for the roll correction arrays */
	status = mb_malloc(verbose,nroll*sizeof(double),&roll_time,&error);
	status = mb_malloc(verbose,nroll*sizeof(double),&roll_corr,&error);

	/* if a roll correction file has been specified then
		read in roll correction data */
	if (((int) strlen(rfile)) > 0)
		{
		nroll = 0;
		if ((vfp = fopen(rfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Roll Correction File <%s> for reading\n",vfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		strncpy(buffer,"\0",sizeof(buffer));
		while ((result = fgets(buffer,128,vfp)) == buffer)
		  {
		  if (buffer[0] != '#')
			{
			sscanf(buffer,"%lf %lf",&roll_time[nroll],&roll_corr[nroll]);

			/* output some debug values */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  New roll correction value read in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       time[%d]: %f  roll[%d]: %f\n",
					nroll,roll_time[nroll],nroll,roll_corr[nroll]);
				}
			nroll++;
			}
		  strncpy(buffer,"\0",sizeof(buffer));
		  }
		fclose(vfp);
		}
	if (verbose > 0 && nroll > 0)
		fprintf(stderr,"\n%d roll correction records read\n",nroll);

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
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
	status = mb_malloc(verbose,beams_bath*sizeof(double),&heave,&error);
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
				heave,alongtrack_offset,
				&draft,&ssv,&error);
				
			/* check surface sound velocity */
			if (ssv > 0.0)
				ssv_start = ssv;
			}
		}
	
	    /* close and reopen the input file */
	    status = mb_close(verbose,&imbio_ptr,&error);
	    if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
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

	/* if specified get static beam depth corrections */
	if (((int) strlen(sfile)) > 0)
		{
		if ((vfp = fopen(sfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Static Beam Correction File <%s> for reading\n",sfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		status = mb_malloc(verbose,beams_bath*sizeof(double),&bath_corr,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		nbath_corr = 0;
		while (nbath_corr < beams_bath &&
			(result = fgets(buffer,128,vfp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				sscanf(buffer,"%d %lf",&i,
					&bath_corr[nbath_corr]);
				}
			nbath_corr++;
			}
		if (nbath_corr != beams_bath)
			{
			fprintf(stderr,"\nRequire %d static beam depth corrections but only read %d from <%s>\n",
				beams_bath,nbath_corr,sfile);
			fprintf(stderr,"No static beam depth corrections applied to data\n");
			nbath_corr = 0;
			}
		fclose(vfp);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"Bathymetry data generated by program %s",program_name);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
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
	gethostname(host,128);
	strncpy(comment,"\0",256);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	if (use_svp == MB_YES)
	    {
	    strncpy(comment,"\0",256);
	    sprintf(comment,"Depths and crosstrack distances recalculated from travel times");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    sprintf(comment,"  by raytracing through a water velocity profile specified");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    sprintf(comment,"  by the user.  The depths have been saved in units of");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    if (uncorrected == MB_YES)
		    sprintf(comment,"  uncorrected meters (the depth values are adjusted to be");
	    else
		    sprintf(comment,"  corrected meters (the depth values obtained by");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    if (uncorrected == MB_YES)
		    sprintf(comment,"  consistent with a vertical water velocity of 1500 m/s).");
	    else
		    sprintf(comment,"  raytracing are not adjusted further).");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	    
	else
	    {
	    strncpy(comment,"\0",256);
	    sprintf(comment,"Depths and crosstrack distances adjusted for roll bias, ");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    sprintf(comment,"  pitch bias, and static offsets.");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    }
	    
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	if (use_svp == MB_YES)
	    {
	    if (ssv_mode == SSV_CORRECT)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  SSV mode:           original SSV correct");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    else
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  SSV mode:           original SSV incorrect");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    strncpy(comment,"\0",256);
	    sprintf(comment,"  Default SSV:        %f",ssv_default);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    if (ssv_prelimpass == MB_YES)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  SSV initial pass:   on");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    else
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  SSV initial pass:   off");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}

	    strncpy(comment,"\0",256);
	    sprintf(comment,"  Velocity file:      %s",vfile);
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    sprintf(comment,"  Input water sound velocity profile:");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    strncpy(comment,"\0",256);
	    sprintf(comment,"    depth (m)   velocity (m/s)");
	    status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	    if (error == MB_ERROR_NO_ERROR) ocomment++;
	    for (i=0;i<nvel;i++)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"     %10.2f     %10.2f",
			depth[i],velocity[i]);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	    }

	strncpy(comment,"\0",256);
	sprintf(comment,"  Roll bias:    %f degrees (starboard: -, port: +)",
		roll_bias);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Pitch bias:   %f degrees (aft: -, forward: +)",
		pitch_bias);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	if (draught_use == MB_YES)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  Draft:        %f meters",
			draught);
		}
	else
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  Draft:        as specified in data");
		}
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	if (ttscale_mode == MB_YES)
	    {
	    fprintf(stderr,"dbg2       tt scale:        %f\n",ttscale);
	    }
	else
	    {
	    fprintf(stderr,"dbg2       tt scale:        OFF\n");
	    }

	strncpy(comment,"\0",256);
	sprintf(comment,"  Roll correction file:      %s",rfile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Static depth correction file:      %s",sfile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	if (nbath_corr == beams_bath)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  Static beam depth corrections:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		for (i=0;i<beams_bath;i++)
			{
			strncpy(comment,"\0",256);
			sprintf(comment,"    %2d  %f",i,bath_corr[i]);
			status = mb_put_comment(verbose,ombio_ptr,comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		}
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* set up the raytracing */
	status = mb_rt_init(verbose, nvel, depth, velocity, &rt_svp, &error);

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

		/* time gaps do not matter to mbbath */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* non-survey data do not matter to mbbath */
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

		/* if survey or calibrate data encountered, 
			get the bathymetry */
		if (error == MB_ERROR_NO_ERROR
			&& (kind == MB_DATA_DATA 
			|| kind == MB_DATA_CALIBRATE))
			{
			/* extract travel times */
			status = mb_ttimes(verbose,imbio_ptr,
				store_ptr,&kind,&nbeams,
				ttimes,angles,
				angles_forward,angles_null,
				heave,alongtrack_offset,
				&draft,&ssv,&error);
				
			/* rescale travel times if necessary */
			if (ttscale_mode == MB_YES)
				{
				for (i=0;i<beams_bath;i++)
					ttimes[i] = ttscale * ttimes[i];
				}

			/* set surface sound speed to default if needed */
			if (ssv <= 0.0)
				ssv = ssv_start;
			else
				ssv_start = ssv;

			/* fix possible problem with SB2100 data */
			if (error == MB_ERROR_NO_ERROR
				&& kind == MB_DATA_DATA
				&& format == 41
				&& fix_2100_tt == MB_YES)
				{
				/* check ping pulsewidth and double ttime if < 3 
					this is temporary problem with the
					Seis Surveyor SB2136 data */

				/* get data structure pointer */
				store = (struct mbsys_sb2100_struct *) store_ptr;
				if (store->frequency == 'H' 
					&& store->ping_pulse_width < 3)
					{
					for (i=0;i<beams_bath;i++)
						ttimes[i] = 2.0 * ttimes[i];
					}
				}

			/* if needed get roll correction */
			if (nroll > 0 && kind == MB_DATA_DATA)
				{
				status = get_roll_correction(verbose,
					nroll,roll_time,roll_corr,
					time_d,&roll_correction,&error);
				}

			/* get angle correction */
			if (kind == MB_DATA_DATA)
				{
				roll_angle_correction = 
					roll_bias + roll_correction;
				pitch_angle_correction = 
					pitch_bias;
				}
			else if (kind == MB_DATA_CALIBRATE)
				{
				roll_angle_correction = pitch_bias;
				pitch_angle_correction = 
					roll_bias + roll_correction;
				}

			/* if svp specified recalculate bathymetry
			    by raytracing  */
			if (use_svp == MB_YES)
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
				if (roll_angle_correction != 0.0 
					|| pitch_angle_correction != 0.0)
					{
					mb_takeoff_to_rollpitch(
						verbose,
						angles[i], angles_forward[i], 
						&alpha, &beta, 
						&error);
					alpha += pitch_angle_correction;
					beta += roll_angle_correction;
					mb_rollpitch_to_takeoff(
						verbose, 
						alpha, beta, 
						&angles[i], &angles_forward[i], 
						&error); 
					}
    
				/* add user specified draught */
				if (draught_use == MB_YES)
					draft_use = draught;
				else
					draft_use = draft;
				depth_offset_use = heave[i] + draft_use;
				static_shift = 0.0;
	
				/* check depth_offset - use static shift if depth_offset negative */
				if (depth_offset_use < 0.0)
				    {
				    fprintf(stderr, "\nWarning: Depth offset negative - transducers above water?!\n");
				    fprintf(stderr, "Raytracing performed from zero depth followed by static shift.\n");
				    fprintf(stderr, "Depth offset is sum of heave + transducer depth.\n");
				    fprintf(stderr, "Draft from data:       %f\n", draft);
				    fprintf(stderr, "Heave from data:       %f\n", heave[i]);
				    fprintf(stderr, "User specified draft:  %f\n", draught);
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
				if (uncorrected == MB_YES)
				    {
				    k = -1;
				    for (j=0;j<nvel-1;j++)
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
    
				/* apply static correction */
				if (nbath_corr == beams_bath)
				    bath[i] -= bath_corr[i];
				
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

			/* if svp not specified recalculate bathymetry
			    by rigid rotations  */
			else if (use_svp == MB_NO)
			    {
			    /* loop over the beams */
			    for (i=0;i<beams_bath;i++)
			      {
			      if (mb_beam_ok(beamflag[i]))
				{
				/* strip off heave + draft */
				bath[i] -= (heave[i] + draft_use);
				
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
				alpha += DTR * pitch_angle_correction;
				beta +=  DTR * roll_angle_correction;
				
				/* recalculate bathymetry */
				bath[i] 
				    = range * cos(alpha) * sin(beta);
				bathalongtrack[i] 
				    = range * sin(alpha);
				bathacrosstrack[i] 
				    = range * cos(alpha) * cos(beta);	
					
				/* add heave and draft back in */	    
				bath[i] += (heave[i] + draft_use);
    
				/* apply static correction */
				if (nbath_corr == beams_bath)
				    bath[i] -= bath_corr[i];
    
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

			/* fix possible problem with SB2100 data */
			if (error == MB_ERROR_NO_ERROR
				&& kind == MB_DATA_DATA
				&& format == 41)
				{
				/* get max sizes of depth and acrosstrack values */
				value_max = 0.0;
				for (i=0;i<beams_bath;i++)
					{
					if (fabs(bath[i]) > 0.0)
						{
						value_max = MAX(fabs(bath[i]),value_max);
						value_max = MAX(fabs(bathacrosstrack[i]),value_max);
						}
					}

				/* get data structure pointer */
				store = (struct mbsys_sb2100_struct *) store_ptr;

				/* set range scale to accomodate the largest values */
				if (value_max < 1000.0)
					{
					store->range_scale = 'S';
					}
				else if (value_max < 10000.0)
					{
					store->range_scale = 'I';
					}
				else
					{
					store->range_scale = 'D';
					}
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
				else if (kind == MB_DATA_COMMENT)
					ocomment++;
				}
			else
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
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
	mb_free(verbose,&roll_time,&error);
	mb_free(verbose,&roll_corr,&error);
	mb_free(verbose,&bath_corr,&error);
	mb_free(verbose,&ttimes,&error);
	mb_free(verbose,&angles,&error);
	mb_free(verbose,&angles_forward,&error);
	mb_free(verbose,&angles_null,&error);
	mb_free(verbose,&heave,&error);
	mb_free(verbose,&alongtrack_offset,&error);
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output comment records\n",ocomment);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int get_roll_correction(verbose,nroll,roll_time,roll_corr,time_d,
		roll_correction,error)
int	verbose;
int	nroll;
double	*roll_time;
double	*roll_corr;
double	time_d;
double	*roll_correction;
int	*error;
{
	char	*function_name = "get_roll_correction";
	int	status = MB_SUCCESS;
	int	iroll, found;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       nroll:      %d\n",nroll);
		fprintf(stderr,"dbg2       roll_time:  %d\n",roll_time);
		fprintf(stderr,"dbg2       roll_corr:  %d\n",roll_corr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* search for the current time */
	found = MB_NO;
	for (i=0;i<nroll-1;i++)
		if (time_d >= roll_time[i] && time_d <= roll_time[i+1])
			{
			found = MB_YES;
			iroll = i;
			}

	/* set the correction */
	if (found == MB_YES)
		{
		*roll_correction = roll_corr[iroll] 
			+ (roll_corr[iroll+1] - roll_corr[iroll])
			*(time_d - roll_time[iroll])
			/(roll_time[iroll+1] - roll_time[iroll]);
		}
	else
		*roll_correction = 0.0;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       roll_correction: %f\n",
						*roll_correction);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
