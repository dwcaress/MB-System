/*--------------------------------------------------------------------
 *    The MB-system:	mbinfo.c	2/1/93
 *    $Id: mbinfo.c,v 5.18 2004-12-02 06:38:09 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2004 by
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
 * MBINFO reads a swath sonar data file and outputs
 * some basic statistics.  If pings are averaged (pings > 2)
 * MBINFO estimates the variance for each of the swath 
 * bathymetry beams by reading a set number of pings (>2) and then finding 
 * the variance of the detrended values for each beam. The variances
 * for the amplitude beams and sidescan values are 
 * calculated without detrending.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.17  2004/09/16 00:59:15  caress
 * Fixed good-nav-only mode.
 *
 * Revision 5.16  2004/07/15 19:33:57  caress
 * Improvements to support for Reson 7k data.
 *
 * Revision 5.15  2004/04/27 02:59:48  caress
 * Added support for subbottom data.
 *
 * Revision 5.14  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.13  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.12  2002/09/19 00:28:12  caress
 * Release 5.0.beta23
 *
 * Revision 5.11  2002/07/20 20:56:55  caress
 * Release 5.0.beta20
 *
 * Revision 5.10  2002/05/29 23:43:09  caress
 * Release 5.0.beta18
 *
 * Revision 5.9  2002/02/22 09:07:08  caress
 * Release 5.0.beta13
 *
 * Revision 5.8  2001/11/20  20:41:55  caress
 * Reset output of comments to not use ##.
 *
 * Revision 5.7  2001/11/20  02:00:19  caress
 * Now prints ## before comments when -C option used.
 *
 * Revision 5.6  2001/10/25  16:02:55  caress
 * Fixed bug in parsing DRAFT metadata tag.
 *
 * Revision 5.5  2001/09/17  23:21:14  caress
 * Fixed metadata support.
 *
 * Revision 5.4  2001/08/10  22:42:50  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.3  2001-07-19 17:34:38-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/06/03 07:07:34  caress
 * Release 5.0.beta01.
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.21  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.20  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.19  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.18  1999/03/31 18:33:06  caress
 * MB-System 4.6beta7
 *
 * Revision 4.17  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.16  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.16  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.15  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.14  1995/11/28  21:03:36  caress
 * Fixed scaling for meters to feet.
 *
 * Revision 4.13  1995/11/22  22:21:36  caress
 * Now handles bathymetry in feet with -W option.
 *
 * Revision 4.12  1995/07/18  17:14:55  caress
 * Added -G option to try to exclude bad nav from min max results.
 *
 * Revision 4.11  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.10  1995/05/08  21:32:34  caress
 * Fixed ability to read from stdin.
 *
 * Revision 4.9  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.8  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.7  1995/02/27  14:43:18  caress
 * Fixed bug regarding closing a text input file.
 *
 * Revision 4.6  1995/01/06  00:06:41  caress
 * Can now read from either single data files or from multiple
 * data files specified in a datalist.
 *
 * Revision 4.5  1994/11/03  18:33:41  caress
 * Embellished the output a bit, with speed in knots for
 * the "units impaired".
 *
 * Revision 4.4  1994/11/03  13:28:44  caress
 * Added percentages to data quality statistics.
 *
 * Revision 4.3  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.2  1994/04/28  01:32:57  caress
 * Changed mb_get to mb_read so that min/max of longitude can
 * be calculated using both navigation and beam data.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/02  22:45:03  caress
 * Fixed calculations of mean and variance values for
 * amplitude and sidescan data.
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.3  1993/06/29  23:57:14  caress
 * Made NOT printing out comments the default, with -C
 * instead of -N now the comment printing toggle.
 * Added julian day to the begin and end time strings.
 *
 * Revision 3.2  1993/06/17  16:14:13  caress
 * Initialized several variables so that the programs does
 * not print out trash if no data is found.
 *
 * Revision 3.1  1993/06/12  04:29:33  caress
 * Added -N option which prevents mbinfo from listing out
 * comments encountered in the input data file.
 *
 * Revision 3.0  1993/05/04  22:38:24  dale
 * Inital version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"

#define MBINFO_MAXPINGS 50
struct ping
	{
	char	*beamflag;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	};

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbinfo.c,v 5.18 2004-12-02 06:38:09 caress Exp $";
	static char program_name[] = "MBINFO";
	static char help_message[] =  "MBINFO reads a swath sonar data file and outputs \nsome basic statistics.  If pings are averaged (pings > 2) \nMBINFO estimates the variance for each of the swath \nbeams by reading a set number of pings (>2) and then finding \nthe variance of the detrended values for each beam. \nThe results are dumped to stdout.";
	static char usage_message[] = "mbinfo [-Byr/mo/da/hr/mn/sc -C -Eyr/mo/da/hr/mn/sc -Fformat -Ifile -Llonflip -Mnx/ny -N -Ppings -Rw/e/s/n -Sspeed -V -H]";
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
	char	*message;
	char	format_description[MB_DESCRIPTION_LENGTH];

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	pings_get = 1;
	int	pings_read = 1;
	int	beams_bath_alloc;
	int	beams_amp_alloc;
	int	pixels_ss_alloc;
	int	beams_bath_max = 0;
	int	beams_amp_max = 0;
	int	pixels_ss_max = 0;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	struct mb_io_struct *mb_io_ptr;
	int	kind;
	struct ping *data[MBINFO_MAXPINGS];
	struct ping *datacur;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	char	comment[256];
	int	icomment = 0;
	
	/* metadata controls */
	int	imetadata = 0;
	int	meta_vessel = 0;
	int	meta_institution = 0;
	int	meta_platform = 0;
	int	meta_sonar = 0;
	int	meta_sonarversion = 0;
	int	meta_cruiseid = 0;
	int	meta_cruisename = 0;
	int	meta_pi = 0;
	int	meta_piinstitution = 0;
	int	meta_client = 0;
	int	meta_svcorrected = 0;
	int	meta_tidecorrected = 0;
	int	meta_batheditmanual = 0;
	int	meta_batheditauto = 0;
	int	meta_rollbias = 0;
	int	meta_pitchbias = 0;
	int	meta_headingbias = 0;
	int	meta_draft = 0;

	/* mbinfo control parameters */
	int	comments = MB_NO;
	int	good_nav_only = MB_NO;
	int	good_nav;
	double	speed_threshold = 50.0;
	int	bathy_in_feet = MB_NO;
	double	bathy_scale;
	int	lonflip_use = 0;
	int	lonflip_set = MB_NO;

	/* limit variables */
	double	lonmin = 0.0;
	double	lonmax = 0.0;
	double	latmin = 0.0;
	double	latmax = 0.0;
	double	sdpmin = 0.0;
	double	sdpmax = 0.0;
	double	altmin = 0.0;
	double	altmax = 0.0;
	double	bathmin = 0.0;
	double	bathmax = 0.0;
	double	ampmin = 0.0;
	double	ampmax = 0.0;
	double	ssmin = 0.0;
	double	ssmax = 0.0;
	double	bathbeg = 0.0;
	double	bathend = 0.0;
	double	lonbeg = 0.0;
	double	latbeg = 0.0;
	double	lonend = 0.0;
	double	latend = 0.0;
	double	spdbeg = 0.0;
	double	hdgbeg = 0.0;
	double	sdpbeg = 0.0;
	double	altbeg = 0.0;
	double	spdend = 0.0;
	double	hdgend = 0.0;
	double	sdpend = 0.0;
	double	altend = 0.0;
	double	timbeg = 0.0;
	double	timend = 0.0;
	int	timbeg_i[7];
	int	timend_i[7];
	int	timbeg_j[5];
	int	timend_j[5];
	double	distot = 0.0;
	double	timtot = 0.0;
	double	spdavg = 0.0;
	int	irec = 0;
	int	isbtmrec = 0;
	double	timbegfile = 0.0;
	double	timendfile = 0.0;
	double	distotfile = 0.0;
	double	timtotfile = 0.0;
	double	spdavgfile = 0.0;
	int	irecfile = 0;
	int	isbtmrecfile = 0;
	int	ntdbeams = 0;
	int	ngdbeams = 0;
	int	nzdbeams = 0;
	int	nfdbeams = 0;
	int	ntabeams = 0;
	int	ngabeams = 0;
	int	nzabeams = 0;
	int	nfabeams = 0;
	int	ntsbeams = 0;
	int	ngsbeams = 0;
	int	nzsbeams = 0;
	int	nfsbeams = 0;
	double	ngd_percent;
	double	nzd_percent;
	double	nfd_percent;
	double	nga_percent;
	double	nza_percent;
	double	nfa_percent;
	double	ngs_percent;
	double	nzs_percent;
	double	nfs_percent;
	int	beginnav = MB_NO;
	int	beginsdp = MB_NO;
	int	beginalt = MB_NO;
	int	beginbath = MB_NO;
	int	beginamp = MB_NO;
	int	beginss = MB_NO;
	int	nread = 0;

	/* variance finding variables */
	int	nbath;
	int	namp;
	int	nss;
	double	sumx, sumxx, sumy, sumxy, delta;
	double	a, b, dev, mean, variance;
	double	*bathmean = NULL;
	double	*bathvar = NULL;
	int	*nbathvar = NULL;
	double	*ampmean = NULL;
	double	*ampvar = NULL;
	int	*nampvar = NULL;
	double	*ssmean = NULL;
	double	*ssvar = NULL;
	int	*nssvar = NULL;
	
	/* coverage mask variables */
	int	coverage_mask = MB_NO;
	int	pass;
	int	done;
	int	mask_nx = 0;
	int	mask_ny = 0;
	double	mask_dx = 0.0;
	double	mask_dy = 0.0;
	int	*mask = NULL;
	
	/* notice variables */
	int	print_notices = MB_NO;
	int	notice_list[MB_NOTICE_MAX];
	int	notice_list_tot[MB_NOTICE_MAX];
	char	*notice_msg;

	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*stream = NULL;
	FILE	*output = NULL;
	int	output_usefile = MB_NO;
	char	output_file[MB_PATH_MAXLINE];
	char	*fileprint;

	int	read_data;
	double	speed_apparent, time_d_last;
	int	val_int;
	double	val_double;
	int	ix, iy;
	int	i, j, k;

	char	*getenv();

	/* initialize some variables */
	for (i=0;i<7;i++)
		{
		timbeg_i[i] = 0;
		timend_i[i] = 0;
		}
	for (i=0;i<MB_NOTICE_MAX;i++)
		{
		notice_list[i] = 0;
		notice_list_tot[i] = 0;
		}		
		
	/* get current default values */
	status = mb_defaults(verbose,&format,&pings_get,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:CcE:e:F:f:GgI:i:L:l:M:m:NnOoP:p:R:r:S:s:T:t:Ww")) != -1)
	  switch (c) 
		{
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'C':
		case 'c':
			comments = MB_YES;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			good_nav_only = MB_YES;
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			lonflip_set = MB_YES;
			lonflip_use = lonflip;
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d/%d", &mask_nx, &mask_ny);
			coverage_mask = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			print_notices = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			output_usefile = MB_YES;
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings_read);
			if (pings_read < 1)
				pings_read = 1;
			if (pings_read > MBINFO_MAXPINGS) 
				pings_read = MBINFO_MAXPINGS;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'W':
		case 'w':
			bathy_in_feet = MB_YES;
			break;
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		stream = stdout;
	else
		stream = stderr;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stream,"usage: %s\n", usage_message);
		fprintf(stream,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stream,"\nProgram %s\n",program_name);
		fprintf(stream,"Version %s\n",rcs_id);
		fprintf(stream,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stream,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stream,"dbg2  Version %s\n",rcs_id);
		fprintf(stream,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stream,"dbg2  Control Parameters:\n");
		fprintf(stream,"dbg2       verbose:    %d\n",verbose);
		fprintf(stream,"dbg2       help:       %d\n",help);
		fprintf(stream,"dbg2       format:     %d\n",format);
		fprintf(stream,"dbg2       pings:      %d\n",pings_read);
		fprintf(stream,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(stream,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stream,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stream,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stream,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stream,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(stream,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(stream,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(stream,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(stream,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(stream,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(stream,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(stream,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stream,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stream,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stream,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stream,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stream,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stream,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(stream,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stream,"dbg2       timegap:    %f\n",timegap);
		fprintf(stream,"dbg2       good_nav:   %d\n",good_nav_only);
		fprintf(stream,"dbg2       comments:   %d\n",comments);
		fprintf(stream,"dbg2       file:       %s\n",read_file);
		fprintf(stream,"dbg2       bathy feet: %d\n",bathy_in_feet);
		fprintf(stream,"dbg2       lonflip_set:%d\n",lonflip_set);
		fprintf(stream,"dbg2       coverage:   %d\n",coverage_mask);
		if (coverage_mask == MB_YES)
			{
			fprintf(stream,"dbg2       mask_nx:    %d\n",mask_nx);
			fprintf(stream,"dbg2       mask_ny:    %d\n",mask_ny);
			}
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stream,"\n%s\n",help_message);
		fprintf(stream,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* set bathymetry scaling */
	if (bathy_in_feet == MB_YES)
		bathy_scale = 1.0 / 0.3048;
	else
		bathy_scale = 1.0;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* if reading from datalist then variance calculations
		are disabled */
	if (read_datalist == MB_YES)
		pings_read = 1;
		
	/* Open output file if requested */
	if (output_usefile == MB_YES)
	    {
	    strcpy(output_file, read_file);
	    strcat(output_file, ".inf");
	    if ((output = fopen(output_file, "w")) == NULL)
		output = stream;
	    }
	else
	    {
	    output = stream;
	    }
		
	/* read only once unless coverage mask requested */
	pass = 0;
	done = MB_NO;
	while (done == MB_NO)
	{

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
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings_get,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath_alloc,
		&beams_amp_alloc,
		&pixels_ss_alloc,
		&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stream,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stream,"\nSwath File <%s> not initialized for reading\n",file);
		fprintf(stream,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	for (i=0;i<pings_read;i++)
		{
		data[i] = NULL;
		status = mb_malloc(verbose,pings_read*sizeof(struct ping),
				&data[i],&error);
		if (error == MB_ERROR_NO_ERROR)
			{
			datacur = data[i];
			datacur->beamflag = NULL;
			datacur->bath = NULL;
			datacur->amp = NULL;
			datacur->bathlon = NULL;
			datacur->bathlat = NULL;
			datacur->ss = NULL;
			datacur->sslon = NULL;
			datacur->sslat = NULL;
			}
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(char),
					&datacur->beamflag,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(double),
					&datacur->bath,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp_alloc*sizeof(double),
					&datacur->amp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(double),
					&datacur->bathlon,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(double),
					&datacur->bathlat,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(double),
					&datacur->ss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(double),
					&datacur->sslon,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(double),
					&datacur->sslat,&error);
		}
	if (pings_read > 1 && pass == 0)
		{
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(double),
				&bathmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(double),
				&bathvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath_alloc*sizeof(int),
				&nbathvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp_alloc*sizeof(double),
				&ampmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp_alloc*sizeof(double),
				&ampvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp_alloc*sizeof(int),
				&nampvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(double),
				&ssmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(double),
				&ssvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss_alloc*sizeof(int),
				&nssvar,&error);
		}
		
	/* if coverage mask requested get cell sizes */
	if (pass == 1 && coverage_mask == MB_YES)
	    {
	    if (mask_nx > 1 && mask_ny <= 0)
		{
		if ((lonmax - lonmin) > (latmax - latmin))
		    {
		    mask_ny = mask_nx * (latmax - latmin) / (lonmax - lonmin);
		    }
		else
		    {
		    mask_ny = mask_nx;
		    mask_nx = mask_ny * (lonmax - lonmin) / (latmax - latmin);
		    if (mask_ny < 2)
			mask_ny = 2;
		    }
		}
	    if (mask_nx < 2)
		mask_nx = 2;
	    if (mask_ny < 2)
		mask_ny = 2;
	    mask_dx = (lonmax - lonmin) / mask_nx;
	    mask_dy = (latmax - latmin) / mask_ny;

	    /* allocate mask */
	    status = mb_malloc(verbose,mask_nx*mask_ny*sizeof(int),
				&mask,&error);
	    }

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stream,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stream,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize data arrays */
	irecfile = 0;
	isbtmrecfile = 0;
	distotfile = 0.0;
	timtotfile = 0.0;
	spdavgfile = 0.0;
	if (pass == 0 && pings_read > 1)
		{
		for (i=0;i<beams_bath_alloc;i++)
			{
			bathmean[i] = 0.0;
			bathvar[i] = 0.0;
			nbathvar[i] = 0;
			}
		for (i=0;i<beams_amp_alloc;i++)
			{
			ampmean[i] = 0.0;
			ampvar[i] = 0.0;
			nampvar[i] = 0;
			}
		for (i=0;i<pixels_ss_alloc;i++)
			{
			ssmean[i] = 0.0;
			ssvar[i] = 0.0;
			nssvar[i] = 0;
			}
		}
	if (pass == 1 && coverage_mask == MB_YES)
		{
		for (i=0;i<mask_nx*mask_ny;i++)
		    mask[i] = MB_NO;
		}
		
	/* initialize metadata counters */
	meta_vessel = 0;
	meta_institution = 0;
	meta_platform = 0;
	meta_sonar = 0;
	meta_sonarversion = 0;
	meta_cruiseid = 0;
	meta_cruisename = 0;
	meta_pi = 0;
	meta_piinstitution = 0;
	meta_client = 0;
	meta_svcorrected = 0;
	meta_tidecorrected = 0;
	meta_batheditmanual = 0;
	meta_batheditauto = 0;
	meta_rollbias = 0;
	meta_pitchbias = 0;
	meta_headingbias = 0;
	meta_draft = 0;

	/* printf out file and format */
	if (pass == 0)
		{
		if (strrchr(file, '/') == NULL)
		    fileprint = file;
		else
		    fileprint = strrchr(file, '/') + 1;
		mb_format_description(verbose,&format,format_description,&error);
		fprintf(output,"\nSwath Data File:      %s\n",fileprint);
		fprintf(output,"MBIO Data Format ID:  %d\n",format);
		fprintf(output,"%s",format_description);
		}

	/* read and process data */
	while (error <= MB_ERROR_NO_ERROR)
		{
		nread = 0;
		error = MB_ERROR_NO_ERROR;
		while (nread < pings_read && error == MB_ERROR_NO_ERROR)
			{

			/* read a ping of data */
			datacur = data[nread];
			beamflag = datacur->beamflag;
			bath = datacur->bath;
			amp = datacur->amp;
			bathlon = datacur->bathlon;
			bathlat = datacur->bathlat;
			ss = datacur->ss;
			sslon = datacur->sslon;
			sslat = datacur->sslat;
			status = mb_read(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* increment counters */
			if (pass == 0 
				&& (error == MB_ERROR_NO_ERROR 
				    || error == MB_ERROR_TIME_GAP))
				{
				irec++;
				irecfile++;
				nread++;
				}

			/* increment counters */
			if (pass == 0 
				&& (error == MB_ERROR_SUBBOTTOM))
				{
				isbtmrec++;
				isbtmrecfile++;
				}

			/* print comment records */
			if (pass == 0 
				&& error == MB_ERROR_COMMENT 
				&& comments == MB_YES)
				{
				if (strncmp(comment,"META",4) != 0)
					{
					if (icomment == 0)
						{
						fprintf(output,"\nComments in file %s:\n",file);
						icomment++;
						}
					fprintf(output,"  %s\n",comment);
					}
				}
			
			/* print metadata */
			if (pass == 0
				&& error == MB_ERROR_COMMENT
				&& strncmp(comment,"META",4) == 0)
				{
				if (imetadata == 0)
					{
					fprintf(output,"\nMetadata:\n");
					imetadata++;
					}
				if (strncmp(comment, "METAVESSEL:", 11) == 0)
					{
					if (meta_vessel == 0)
					    fprintf(output,"Vessel:                 %s\n", &comment[11]);
					meta_vessel++;
					}
				else if (strncmp(comment, "METAINSTITUTION:", 16) == 0)
					{
					if (meta_institution == 0)
					    fprintf(output,"Institution:            %s\n", &comment[16]);
					meta_institution++;
					}
				else if (strncmp(comment, "METAPLATFORM:", 13) == 0)
					{
					if (meta_platform == 0)
					    fprintf(output,"Platform:               %s\n", &comment[13]);
					meta_platform++;
					}
				else if (strncmp(comment, "METASONARVERSION:", 17) == 0)
					{
					if (meta_sonarversion == 0)
					    fprintf(output,"Sonar Version:          %s\n", &comment[17]);
					meta_sonarversion++;
					}
				else if (strncmp(comment, "METASONAR:", 10) == 0)
					{
					if (meta_sonar == 0)
					    fprintf(output,"Sonar:                  %s\n", &comment[10]);
					meta_sonar++;
					}
				else if (strncmp(comment, "METACRUISEID:", 13) == 0)
					{
					if (meta_cruiseid == 0)
					    fprintf(output,"Cruise ID:              %s\n", &comment[13]);
					meta_cruiseid++;
					}
				else if (strncmp(comment, "METACRUISENAME:", 15) == 0)
					{
					if (meta_cruisename == 0)
					    fprintf(output,"Cruise Name:            %s\n", &comment[15]);
					meta_cruisename++;
					}
				else if (strncmp(comment, "METAPI:", 7) == 0)
					{
					if (meta_pi == 0)
					    fprintf(output,"PI:                     %s\n", &comment[7]);
					meta_pi++;
					}
				else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0)
					{
					if (meta_piinstitution == 0)
					    fprintf(output,"PI Institution:         %s\n", &comment[18]);
					meta_piinstitution++;
					}
				else if (strncmp(comment, "METACLIENT:", 11) == 0)
					{
					if (meta_client == 0)
					    fprintf(output,"Client:                 %s\n", &comment[11]);
					meta_client++;
					}
				else if (strncmp(comment, "METASVCORRECTED:", 16) == 0)
					{
					if (meta_svcorrected == 0)
					    {
					    sscanf(comment, "METASVCORRECTED:%d", &val_int);
					    if (val_int == MB_YES)
						fprintf(output,"Corrected Depths:       YES\n");
					    else
						fprintf(output,"Corrected Depths:       NO\n");
					    }
					meta_svcorrected++;
					}
				else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0)
					{
					if (meta_tidecorrected == 0)
					    {
					    sscanf(comment, "METATIDECORRECTED:%d", &val_int);
					    if (val_int == MB_YES)
						fprintf(output,"Tide Corrected:         YES\n");
					    else
						fprintf(output,"Tide Corrected:         NO\n");
					    }
					meta_tidecorrected++;
					}
				else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0)
					{
					if (meta_batheditmanual == 0)
					    {
					    sscanf(comment, "METABATHEDITMANUAL:%d", &val_int);
					    if (val_int == MB_YES)
						fprintf(output,"Depths Manually Edited: YES\n");
					    else
						fprintf(output,"Depths Manually Edited: NO\n");
					    }
					meta_batheditmanual++;
					}
				else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0)
					{
					if (meta_batheditauto == 0)
					    {
					    sscanf(comment, "METABATHEDITAUTO:%d", &val_int);
					    if (val_int == MB_YES)
						fprintf(output,"Depths Auto-Edited:     YES\n");
					    else
						fprintf(output,"Depths Auto-Edited:     NO\n");
					    }
					meta_batheditauto++;
					}
				else if (strncmp(comment, "METAROLLBIAS:", 13) == 0)
					{
					if (meta_rollbias == 0)
					    {
					    sscanf(comment, "METAROLLBIAS:%lf", &val_double);
					    fprintf(output,"Roll Bias:              %f degrees\n", val_double);
					    }
					meta_rollbias++;
					}
				else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0)
					{
					if (meta_pitchbias == 0)
					    {
					    sscanf(comment, "METAPITCHBIAS:%lf", &val_double);
					    fprintf(output,"Pitch Bias:             %f degrees\n", val_double);
					    }
					meta_pitchbias++;
					}
				else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0)
					{
					if (meta_headingbias == 0)
					    {
					    sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);
					    fprintf(output,"Heading Bias:           %f degrees\n", val_double);
					    }
					meta_headingbias++;
					}
				else if (strncmp(comment, "METADRAFT:", 10) == 0)
					{
					if (meta_draft == 0)
					    {
					    sscanf(comment, "METADRAFT:%lf", &val_double);
					    fprintf(output,"Draft:                  %f m\n", val_double);
					    }
					meta_draft++;
					}
				}

			/* output error messages */
			if (pass != 0 || error == MB_ERROR_COMMENT)
				{
				/* do nothing */
				}
			else if (error == MB_ERROR_SUBBOTTOM)
				{
				/* do nothing */
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
				&& error >= MB_ERROR_OTHER)
				{
				mb_error(verbose,error,&message);
				fprintf(stream,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(stream,"Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
				{
				mb_error(verbose,error,&message);
				fprintf(stream,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(stream,"Number of good records so far: %d\n",irecfile);
				}
			else if (verbose >= 1 && error > MB_ERROR_NO_ERROR 
				&& error != MB_ERROR_EOF)
				{
				mb_error(verbose,error,&message);
				fprintf(stream,"\nFatal MBIO Error:\n%s\n",
					message);
				fprintf(stream,"Last Good Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}

			/* take note of min and maxes */
			if (pass == 0
				&& (error == MB_ERROR_NO_ERROR 
				    || error == MB_ERROR_TIME_GAP))
				{
				/* update data counts */
				beams_bath_max 
				    = MAX(beams_bath_max, beams_bath);
				beams_amp_max 
				    = MAX(beams_amp_max, beams_amp);
				pixels_ss_max 
				    = MAX(pixels_ss_max, pixels_ss);
				ntdbeams += beams_bath;
				ntabeams += beams_amp;
				ntsbeams += pixels_ss;
				
				/* set lonflip if needed */
				if (lonflip_set == MB_NO
				    && (navlon != 0.0 || navlat != 0.0))
				    {
				    lonflip_set = MB_YES;
				    if (navlon < -270.0)
					lonflip_use = 0;
				    else if (navlon >= -270.0 && navlon < -90.0)
					lonflip_use = -1;
				    else if (navlon >= -90.0 && navlon < 90.0)
					lonflip_use = 0;
				    else if (navlon >= 90.0 && navlon < 270.0)
					lonflip_use = 1;
				    else if (navlon >= 270.0)
					lonflip_use = 0;
				    
				    /* change and apply lonflip if needed */
				    if (lonflip_use != lonflip)
					{
					/* change lonflip used in reading */
					mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
					mb_io_ptr->lonflip = lonflip_use;
					lonflip = lonflip_use;
					
					/* apply lonflip to data already read */
					if (lonflip_use == -1)
					    {
					    if (navlon > 0.0)
						navlon -= 360.0;
					    for (i=0;i<beams_bath;i++)
						{
						if (bathlon[i] > 0.0)
						    bathlon[i] -= 360.0;
						}
					    for (i=0;i<pixels_ss;i++)
						{
						if (sslon[i] > 0.0)
						    sslon[i] -= 360.0;
						}
					    }
					else if (lonflip_use == 1)
					    {
					    if (navlon < 0.0)
						navlon += 360.0;
					    for (i=0;i<beams_bath;i++)
						{
						if (bathlon[i] < 0.0)
						    bathlon[i] += 360.0;
						}
					    for (i=0;i<pixels_ss;i++)
						{
						if (sslon[i] < 0.0)
						    sslon[i] += 360.0;
						}
					    }
					else if (lonflip_use == 0)
					    {
					    if (navlon < -180.0)
						navlon += 360.0;
					    if (navlon > 180.0)
						navlon -= 360.0;
					    for (i=0;i<beams_bath;i++)
						{
						if (bathlon[i] < -180.0)
						    bathlon[i] += 360.0;
						if (bathlon[i] > 180.0)
						    bathlon[i] -= 360.0;
						}
					    for (i=0;i<pixels_ss;i++)
						{
						if (sslon[i] < -180.0)
						    sslon[i] += 360.0;
						if (sslon[i] > 180.0)
						    sslon[i] -= 360.0;
						}
					    }
					}
				    }
				
				/* get beginning values */
				if (irec == 1)
					{
					if (beams_bath > 0)
						bathbeg = bath[beams_bath/2];
					lonbeg = navlon;
					latbeg = navlat;
					timbeg = time_d;
					timbegfile = time_d;
					for (i=0;i<7;i++)
						timbeg_i[i] = time_i[i];
					spdbeg = speed;
					hdgbeg = heading;
					sdpbeg = sonardepth;
					altbeg = altitude;
					}
				else if (good_nav_only == MB_YES)
					{
					if (lonbeg == 0.0 && latbeg == 0.0
						&& navlon != 0.0 && navlat != 0.0)
						{
						lonbeg = navlon;
						latbeg = navlat;
					if (spdbeg == 0.0 && speed != 0.0)
						spdbeg = speed;
					if (hdgbeg == 0.0 && heading != 0.0)
						hdgbeg = heading;
					if (sdpbeg == 0.0 && sonardepth != 0.0)
						sdpbeg = sonardepth;
					if (altbeg == 0.0 && altitude != 0.0)
						altbeg = altitude;
						}
					}

				/* reset ending values each time */
				if (beams_bath > 0)
					bathend = bath[beams_bath/2];
				lonend = navlon;
				latend = navlat;
				spdend = speed;
				hdgend = heading;
				sdpend = sonardepth;
				altend = altitude;
				timend = time_d;
				timendfile = time_d;
				for (i=0;i<7;i++)
					timend_i[i] = time_i[i];

				/* check for good nav */
				speed_apparent = 3600.0*distance
					/(time_d - time_d_last);
				if (good_nav_only == MB_YES)
					{
					if (navlon == 0.0 || navlat == 0.0)
					    good_nav = MB_NO;
					else if (beginnav == MB_YES
					    && speed_apparent >= speed_threshold)
					    good_nav = MB_NO;
					else
					    good_nav = MB_YES;
					}
				else
					good_nav = MB_YES;

				/* get total distance */
				if (good_nav_only == MB_NO ||
					(good_nav == MB_YES 
					&& speed_apparent < speed_threshold))
					{
					distot+= distance;
					distotfile += distance;
					}

				/* get starting mins and maxs */
				if (beginnav == MB_NO && good_nav == MB_YES) 
					{
					lonmin = navlon;
					lonmax = navlon;
					latmin = navlat;
					latmax = navlat;
					beginnav = MB_YES;
					}
				if (beginsdp == MB_NO && sonardepth > 0.0) 
					{
					sdpmin = sonardepth;
					sdpmax = sonardepth;
					beginsdp = MB_YES;
					}
				if (beginalt == MB_NO && altitude > 0.0) 
					{
					altmin = altitude;
					altmax = altitude;
					beginalt = MB_YES;
					}
				if (beginbath == MB_NO && beams_bath > 0)
					for (i=0;i<beams_bath;i++)
						if (mb_beam_ok(beamflag[i]))
							{
							bathmin = bath[i];
							bathmax = bath[i];
							beginbath = MB_YES;
							}
				if (beginamp == MB_NO && beams_amp > 0)
					for (i=0;i<beams_amp;i++)
						if (mb_beam_ok(beamflag[i]))
							{
							ampmin = amp[i];
							ampmax = amp[i];
							beginamp = MB_YES;
							}
				if (beginss == MB_NO && pixels_ss > 0)
					for (i=0;i<pixels_ss;i++)
						if (ss[i] > 0.0)
							{
							ssmin = ss[i];
							ssmax = ss[i];
							beginss = MB_YES;
							}

				/* get mins and maxs */
				if (good_nav == MB_YES && beginnav == MB_YES)
					{
					lonmin = MIN(lonmin, navlon);
					lonmax = MAX(lonmax, navlon);
					latmin = MIN(latmin, navlat);
					latmax = MAX(latmax, navlat);
					}
				if (beginsdp == MB_YES)
					{
					sdpmin = MIN(sdpmin, sonardepth);
					sdpmax = MAX(sdpmax, sonardepth);
					}
				if (beginalt == MB_YES)
					{
					altmin = MIN(altmin, altitude);
					altmax = MAX(altmax, altitude);
					}
				for (i=0;i<beams_bath;i++)
					{
					if (mb_beam_ok(beamflag[i]))
						{
						if (good_nav == MB_YES && beginnav == MB_YES)
							{
							lonmin = MIN(lonmin, bathlon[i]);
							lonmax = MAX(lonmax, bathlon[i]);
							latmin = MIN(latmin, bathlat[i]);
							latmax = MAX(latmax, bathlat[i]);
							}
						bathmin = MIN(bathmin, bath[i]);
						bathmax = MAX(bathmax, bath[i]);
						ngdbeams++;
						}
					else if (beamflag[i] == MB_FLAG_NULL)
						nzdbeams++;
					else
						nfdbeams++;
					}
				for (i=0;i<beams_amp;i++)
					{
					if (mb_beam_ok(beamflag[i]))
						{
						ampmin = MIN(ampmin, amp[i]);
						ampmax = MAX(ampmax, amp[i]);
						ngabeams++;
						}
					else if (beamflag[i] == MB_FLAG_NULL)
						nzabeams++;
					else
						nfabeams++;
					}
				for (i=0;i<pixels_ss;i++)
					{
					if (ss[i] > 0.0)
						{
						if (good_nav == MB_YES && beginnav == MB_YES)
							{
							lonmin = MIN(lonmin, sslon[i]);
							lonmax = MAX(lonmax, sslon[i]);
							latmin = MIN(latmin, sslat[i]);
							latmax = MAX(latmax, sslat[i]);
							}
						ssmin = MIN(ssmin, ss[i]);
						ssmax = MAX(ssmax, ss[i]);
						ngsbeams++;
						}
					else if (ss[i] == 0.0)
						nzsbeams++;
					else
						nfsbeams++;
					}

				/* reset time of last ping */
				time_d_last = time_d;
				}
				
			/* update coverage mask */
			if (pass == 1 && coverage_mask == MB_YES)
			    {
			    ix = (int)((navlon - lonmin) / mask_dx);
			    iy = (int)((navlat - latmin) / mask_dy);
			    if (ix >= 0 && ix < mask_nx
				&& iy >= 0 && iy < mask_ny)
				{
				mask[ix+iy*mask_nx] = MB_YES;
				}
			    for (i=0;i<beams_bath;i++)
				{
				if (mb_beam_ok(beamflag[i]))
				    {
				    ix = (int)((bathlon[i] - lonmin) / mask_dx);
				    iy = (int)((bathlat[i] - latmin) / mask_dy);
				    if (ix >= 0 && ix < mask_nx
					&& iy >= 0 && iy < mask_ny)
					{
					mask[ix+iy*mask_nx] = MB_YES;
					}
				    }
				}
			    for (i=0;i<pixels_ss;i++)
				{
				if (ss[i] > 0.0)
				    {
				    ix = (int)((sslon[i] - lonmin) / mask_dx);
				    iy = (int)((sslat[i] - latmin) / mask_dy);
				    if (ix >= 0 && ix < mask_nx
					&& iy >= 0 && iy < mask_ny)
					{
					mask[ix+iy*mask_nx] = MB_YES;
					}
				    }
				}
			    }

			/* look for problems */
			if (pass == 0
				&& (error == MB_ERROR_NO_ERROR 
				    || error == MB_ERROR_TIME_GAP))
			    {
			    if (navlon == 0.0 || navlat == 0.0)
				mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_ZERO_NAV);
			    else if (beginnav == MB_YES
				&& speed_apparent >= speed_threshold)
				mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_TOO_FAST);
			    for (i=0;i<beams_bath;i++)
				{
				if (mb_beam_ok(beamflag[i]))
				    {
				    if (bath[i] > 11000.0)
					mb_notice_log_problem(verbose, mbio_ptr, 
						MB_PROBLEM_TOO_DEEP);
				    }
				}
			    }
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stream,"\ndbg2  Reading loop finished in program <%s>\n",
				program_name);
			fprintf(stream,"dbg2       status:     %d\n",status);
			fprintf(stream,"dbg2       error:      %d\n",error);
			fprintf(stream,"dbg2       nread:      %d\n",nread);
			fprintf(stream,"dbg2       pings_read: %d\n",pings_read);
			}

		/* process the pings */
		if (pass == 0
			&& pings_read > 2 
			&& nread == pings_read
			&& (error == MB_ERROR_NO_ERROR 
			|| error == MB_ERROR_TIME_GAP))
			{

			/* do the bathymetry */
			for (i=0;i<beams_bath;i++)
				{

				/* fit line to depths */
				nbath  = 0;
				sumx  = 0.0;
				sumxx = 0.0;
				sumy  = 0.0;
				sumxy = 0.0;
				variance = 0.0;
				for (j=0;j<nread;j++)
					{
					datacur = data[j];
					bath = datacur->bath;
					beamflag = datacur->beamflag;
					if (mb_beam_ok(beamflag[i]))
					  {
					  nbath++;
					  sumx  = sumx + j;
					  sumxx = sumxx + j*j;
					  sumy  = sumy + bath[i];
					  sumxy = sumxy + j*bath[i];
					  }
					}
				if (nbath == pings_read)
					{
					delta = nbath*sumxx - sumx*sumx;
					a = (sumxx*sumy - sumx*sumxy)/delta;
					b = (nbath*sumxy - sumx*sumy)/delta;
					for (j=0;j<nread;j++)
					  {
					  datacur = data[j];
					  bath = datacur->bath;
					  beamflag = datacur->beamflag;
					  if (mb_beam_ok(beamflag[i]))
					    {
					    dev = bath[i] - a - b*j;
					    variance = variance + dev*dev;
					    }
					  }
					bathmean[i] = bathmean[i] + sumy;
					bathvar[i] = bathvar[i] + variance;
					nbathvar[i] = nbathvar[i] + nbath;
					}
				}

			/* do the amplitude */
			for (i=0;i<beams_amp;i++)
				{

				/* get mean amplitude */
				namp  = 0;
				mean  = 0.0;
				variance = 0.0;
				for (j=0;j<nread;j++)
					{
					datacur = data[j];
					amp = datacur->amp;
					beamflag = datacur->beamflag;
					if (mb_beam_ok(beamflag[i]))
					  {
					  namp++;
					  mean  = mean + amp[i];
					  }
					}
				if (namp == pings_read)
					{
					mean = mean/namp;
					for (j=0;j<nread;j++)
					  {
					  datacur = data[j];
					  amp = datacur->amp;
					  if (mb_beam_ok(beamflag[i]))
					    {
					    dev = amp[i] - mean;
					    variance = variance + dev*dev;
					    }
					  }
					ampmean[i] = ampmean[i] + namp*mean;
					ampvar[i] = ampvar[i] + variance;
					nampvar[i] = nampvar[i] + namp;
					}
				}

			/* do the sidescan */
			for (i=0;i<pixels_ss;i++)
				{

				/* get mean sidescan */
				nss  = 0;
				mean  = 0.0;
				variance = 0.0;
				for (j=0;j<nread;j++)
					{
					datacur = data[j];
					ss = datacur->ss;
					if (ss[i] > 0.0)
					  {
					  nss++;
					  mean  = mean + ss[i];
					  }
					}
				if (nss == pings_read)
					{
					mean = mean/nss;
					for (j=0;j<nread;j++)
					  {
					  datacur = data[j];
					  ss = datacur->ss;
					  if (ss[i] > 0.0)
					    {
					    dev = ss[i] - mean;
					    variance = variance + dev*dev;
					    }
					  }
					ssmean[i] = ssmean[i] + nss*mean;
					ssvar[i] = ssvar[i] + variance;
					nssvar[i] = nssvar[i] + nss;
					}
				}

			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stream,"\ndbg2  Processing loop finished in program <%s>\n",
				program_name);
			fprintf(stream,"dbg2       status:     %d\n",status);
			fprintf(stream,"dbg2       error:      %d\n",error);
			fprintf(stream,"dbg2       nread:      %d\n",nread);
			fprintf(stream,"dbg2       pings_read: %d\n",pings_read);
			}
		}

	/* look for problems */
	timtotfile = (timendfile - timbegfile)/3600.0;
	if (timtotfile > 0.0)
		spdavgfile = distotfile/timtotfile;
	if (irecfile <= 0)
	    mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_NO_DATA);
	else if (timtotfile > 0.0 && spdavgfile >= speed_threshold)
		mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_AVG_TOO_FAST);

	/* get notices if desired */
	if (print_notices == MB_YES && pass == 0)
		{
		status = mb_notice_get_list(verbose, mbio_ptr, 
					    notice_list);
		for (i=0;i<MB_NOTICE_MAX;i++)
			notice_list_tot[i] += notice_list[i];
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	for (i=0;i<pings_read;i++)
		{
		mb_free(verbose,&data[i]->beamflag,&error);
		mb_free(verbose,&data[i]->bath,&error);
		mb_free(verbose,&data[i]->amp,&error);
		mb_free(verbose,&data[i]->bathlon,&error);
		mb_free(verbose,&data[i]->bathlat,&error);
		mb_free(verbose,&data[i]->ss,&error);
		mb_free(verbose,&data[i]->sslon,&error);
		mb_free(verbose,&data[i]->sslat,&error);
		mb_free(verbose,&data[i],&error);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);
		
	/* figure out if done */
	if (pass > 0 || coverage_mask == MB_NO)
	    done = MB_YES;
	pass++;
		
	/* end loop over reading passes */
	}

	/* calculate final variances */
	if (pings_read > 2)
		{
		for (i=0;i<beams_bath_alloc;i++)
			if (nbathvar[i] > 0)
				{
				bathmean[i] = bathmean[i]/nbathvar[i];
				bathvar[i] = bathvar[i]/nbathvar[i];
				}
		for (i=0;i<beams_amp_alloc;i++)
			if (nampvar[i] > 0)
				{
				ampmean[i] = ampmean[i]/nampvar[i];
				ampvar[i] = ampvar[i]/nampvar[i];
				}
		for (i=0;i<pixels_ss_alloc;i++)
			if (nssvar[i] > 0)
				{
				ssmean[i] = ssmean[i]/nssvar[i];
				ssvar[i] = ssvar[i]/nssvar[i];
				}
		}

	/* calculate percentages of data */
	if (ntdbeams > 0)
		{
		ngd_percent = 100.0*ngdbeams/ntdbeams;
		nzd_percent = 100.0*nzdbeams/ntdbeams;
		nfd_percent = 100.0*nfdbeams/ntdbeams;
		}
	else
		{
		ngd_percent = 0.0;
		nzd_percent = 0.0;
		nfd_percent = 0.0;
		}
	if (ntabeams > 0)
		{
		nga_percent = 100.0*ngabeams/ntabeams;
		nza_percent = 100.0*nzabeams/ntabeams;
		nfa_percent = 100.0*nfabeams/ntabeams;
		}
	else
		{
		nga_percent = 0.0;
		nza_percent = 0.0;
		nfa_percent = 0.0;
		}
	if (ntsbeams > 0)
		{
		ngs_percent = 100.0*ngsbeams/ntsbeams;
		nzs_percent = 100.0*nzsbeams/ntsbeams;
		nfs_percent = 100.0*nfsbeams/ntsbeams;
		}
	else
		{
		ngs_percent = 0.0;
		nzs_percent = 0.0;
		nfs_percent = 0.0;
		}

	/* now print out the results */
	timtot = (timend - timbeg)/3600.0;
	if (timtot > 0.0)
		spdavg = distot/timtot;
	mb_get_jtime(verbose,timbeg_i,timbeg_j);
	mb_get_jtime(verbose,timend_i,timend_j);
	fprintf(output,"\nData Totals:\n");
	fprintf(output,"Number of Records:           %8d\n",irec);
	if (isbtmrec > 0)
		fprintf(output,"Number of Subbottom Records: %8d\n",isbtmrec);
	fprintf(output,"Bathymetry Data (%d beams):\n",beams_bath_max);
	fprintf(output,"  Number of Beams:         %8d\n",
		ntdbeams);
	fprintf(output,"  Number of Good Beams:    %8d     %5.2f%%\n",
		ngdbeams, ngd_percent);
	fprintf(output,"  Number of Zero Beams:    %8d     %5.2f%%\n",
		nzdbeams, nzd_percent);
	fprintf(output,"  Number of Flagged Beams: %8d     %5.2f%%\n",
		nfdbeams, nfd_percent);
	fprintf(output,"Amplitude Data (%d beams):\n",beams_amp_max);
	fprintf(output,"  Number of Beams:         %8d\n",
		ntabeams);
	fprintf(output,"  Number of Good Beams:    %8d     %5.2f%%\n",
		ngabeams, nga_percent);
	fprintf(output,"  Number of Zero Beams:    %8d     %5.2f%%\n",
		nzabeams, nza_percent);
	fprintf(output,"  Number of Flagged Beams: %8d     %5.2f%%\n",
		nfabeams, nfa_percent);
	fprintf(output,"Sidescan Data (%d pixels):\n",pixels_ss_max);
	fprintf(output,"  Number of Pixels:        %8d\n",
		ntsbeams);
	fprintf(output,"  Number of Good Pixels:   %8d     %5.2f%%\n",
		ngsbeams, ngs_percent);
	fprintf(output,"  Number of Zero Pixels:   %8d     %5.2f%%\n",
		nzsbeams, nzs_percent);
	fprintf(output,"  Number of Flagged Pixels:%8d     %5.2f%%\n",
		nfsbeams, nfs_percent);
	fprintf(output,"\nNavigation Totals:\n");
	fprintf(output,"Total Time:         %10.4f hours\n",timtot);
	fprintf(output,"Total Track Length: %10.4f km\n",distot);
	fprintf(output,"Average Speed:      %10.4f km/hr (%7.4f knots)\n",
		spdavg,spdavg/1.85);
	fprintf(output,"\nStart of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timbeg_i[1],timbeg_i[2],timbeg_i[0],timbeg_i[3],
		timbeg_i[4],timbeg_i[5],timbeg_i[6],timbeg_j[1]);
	if (bathy_in_feet == MB_NO)
		fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f meters\n",
			lonbeg,latbeg,bathbeg);
	else
		fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f feet\n",
			lonbeg,latbeg,bathy_scale*bathbeg);
	fprintf(output,"Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n",
		spdbeg,spdbeg/1.85,hdgbeg);
	fprintf(output,"Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n",
		sdpbeg,altbeg);
	fprintf(output,"\nEnd of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timend_i[1],timend_i[2],timend_i[0],timend_i[3],
		timend_i[4],timend_i[5],timend_i[6],timend_j[1]);
	if (bathy_in_feet == MB_NO)
		fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f meters\n",
			lonend,latend,bathend);
	else
		fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f feet\n",
			lonend,latend,bathy_scale*bathend);
	fprintf(output,"Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n",
		spdend,spdend/1.85,hdgend);
	fprintf(output,"Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n",
		sdpend,altend);
	fprintf(output,"\nLimits:\n");
	fprintf(output,"Minimum Longitude:   %10.4f   Maximum Longitude:   %10.4f\n",lonmin,lonmax);
	fprintf(output,"Minimum Latitude:    %10.4f   Maximum Latitude:    %10.4f\n",latmin,latmax);
	fprintf(output,"Minimum Sonar Depth: %10.4f   Maximum Sonar Depth: %10.4f\n",sdpmin,sdpmax);
	fprintf(output,"Minimum Altitude:    %10.4f   Maximum Altitude:    %10.4f\n",altmin,altmax);
	if (ngdbeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Depth:       %10.4f   Maximum Depth:       %10.4f\n",
			bathy_scale*bathmin,bathy_scale*bathmax);
	if (ngabeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Amplitude:   %10.4f   Maximum Amplitude:   %10.4f\n",
			ampmin,ampmax);
	if (ngsbeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Sidescan:    %10.4f   Maximum Sidescan:    %10.4f\n",
			ssmin,ssmax);
	if (pings_read > 2 && beams_bath_max > 0 
		&& (ngdbeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nBeam Bathymetry Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<beams_bath_max;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nbathvar[i],bathy_scale*bathmean[i],
				bathy_scale*bathy_scale*bathvar[i],
				bathy_scale*sqrt(bathvar[i]));
		fprintf(output,"\n");
		}
	if (pings_read > 2 && beams_amp_max > 0 
		&& (ngabeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nBeam Amplitude Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<beams_amp_max;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nampvar[i],ampmean[i],
				ampvar[i],sqrt(ampvar[i]));
		fprintf(output,"\n");
		}
	if (pings_read > 2 && pixels_ss_max > 0 
		&& (ngsbeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nPixel Sidescan Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<pixels_ss_max;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nssvar[i],ssmean[i],
				ssvar[i],sqrt(ssvar[i]));
		fprintf(output,"\n");
		}
	if (print_notices == MB_YES)
		{
		fprintf(output,"\nData Record Type Notices:\n");
		for (i=0;i<=MB_DATA_KINDS;i++)
			{
			if (notice_list_tot[i] > 0)
				{
				mb_notice_message(verbose, i, &notice_msg);
				fprintf(output, "DN: %d %s\n", 
					notice_list_tot[i], notice_msg);
				}
			}
		fprintf(output,"\nNonfatal Error Notices:\n");
		for (i=MB_DATA_KINDS+1;i<=MB_DATA_KINDS-(MB_ERROR_MIN);i++)
			{
			if (notice_list_tot[i] > 0)
				{
				mb_notice_message(verbose, i, &notice_msg);
				fprintf(output, "EN: %d %s\n", 
					notice_list_tot[i], notice_msg);
				}
			}
		fprintf(output,"\nProblem Notices:\n");
		for (i=MB_DATA_KINDS-(MB_ERROR_MIN)+1;i<MB_NOTICE_MAX;i++)
			{
			if (notice_list_tot[i] > 0)
				{
				mb_notice_message(verbose, i, &notice_msg);
				fprintf(output, "PN: %d %s\n", 
					notice_list_tot[i], notice_msg);
				}
			}
		}
	if (coverage_mask == MB_YES)
		{
		fprintf(output,"\nCoverage Mask:\nCM dimensions: %d %d\n", mask_nx, mask_ny);
		for (j=mask_ny-1;j>=0;j--)
		    {
		    fprintf(output, "CM:  ");
		    for (i=0;i<mask_nx;i++)
			{
			k = i + j * mask_nx;
			fprintf(output, " %1d", mask[k]);
			}
		    fprintf(output, "\n");
		    }
		}

	/* close output file */
	if (output_usefile == MB_YES
	    && output != NULL)
	    {
	    fclose(output);
	    }

	/* deallocate memory used for data arrays */
	mb_free(verbose,&bathmean,&error);
	mb_free(verbose,&bathvar,&error);
	mb_free(verbose,&nbathvar,&error);
	mb_free(verbose,&ampmean,&error);
	mb_free(verbose,&ampvar,&error);
	mb_free(verbose,&nampvar,&error);
	mb_free(verbose,&ssmean,&error);
	mb_free(verbose,&ssvar,&error);
	mb_free(verbose,&nssvar,&error);
	mb_free(verbose,&mask,&error);

	/* set program status */
	status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stream,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stream,"dbg2  Ending status:\n");
		fprintf(stream,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	fprintf(output,"\n");
	exit(error);
}
/*--------------------------------------------------------------------*/
