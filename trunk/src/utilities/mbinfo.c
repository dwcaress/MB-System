/*--------------------------------------------------------------------
 *    The MB-system:	mbinfo.c	2/1/93
 *    $Id: mbinfo.c,v 4.8 1995-03-02 13:49:21 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBINFO reads a multibeam data file and outputs
 * some basic statistics.  If pings are averaged (pings > 2)
 * MBINFO estimates the variance for each of the multibeam 
 * bathymetry beams by reading a set number of pings (>2) and then finding 
 * the variance of the detrended values for each beam. The variances
 * for the multibeam amplitude beams and sidescan values are 
 * calculated without detrending.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * $Log: not supported by cvs2svn $
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
#include <strings.h>

/* MBIO include files */
#include "../../include/mb_status.h"

/* min max defines */
#define	min(A, B)	((A) < (B) ? (A) : (B))
#define	max(A, B)	((A) > (B) ? (A) : (B))

#define MBINFO_MAXPINGS 50
struct ping
	{
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	};

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbinfo.c,v 4.8 1995-03-02 13:49:21 caress Exp $";
	static char program_name[] = "MBINFO";
	static char help_message[] =  "MBINFO reads a multibeam data file and outputs \nsome basic statistics.  If pings are averaged (pings > 2) \nMBINFO estimates the variance for each of the multibeam \nbeams by reading a set number of pings (>2) and then finding \nthe variance of the detrended values for each beam. \nThe results are dumped to stdout.";
	static char usage_message[] = "mbinfo [-Byr/mo/da/hr/mn/sc -C -Eyr/mo/da/hr/mn/sc -Fformat -Ifile -Llonflip -Ppings -Rw/e/s/n -Sspeed -V -H]";
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

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[128];
	FILE	*fp;
	int	format;
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
	char	file[128];
	int	pings_get = 1;
	int	pings_read = 1;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	char	*mbio_ptr = NULL;
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
	double	*bath = NULL;
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	char	comment[256];
	int	icomment = 0;
	int	comments = MB_NO;

	/* limit variables */
	double	lonmin = 0.0;
	double	lonmax = 0.0;
	double	latmin = 0.0;
	double	latmax = 0.0;
	double	bathmin = 0.0;
	double	bathmax = 0.0;
	double	ampmin = 0.0;
	double	ampmax = 0.0;
	double	ssmin = 0.0;
	double	ssmax = 0.0;
	double	bathbeg = 0.0;
	double	lonbeg = 0.0;
	double	latbeg = 0.0;
	double	bathend = 0.0;
	double	lonend = 0.0;
	double	latend = 0.0;
	double	spdbeg = 0.0;
	double	hdgbeg = 0.0;
	double	spdend = 0.0;
	double	hdgend = 0.0;
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
	int	ngdbeams = 0;
	int	nzdbeams = 0;
	int	nfdbeams = 0;
	int	ngabeams = 0;
	int	nzabeams = 0;
	int	nfabeams = 0;
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
	int	beginbath = 0;
	int	beginamp = 0;
	int	beginss = 0;
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

	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*output;

	int	read_data;
	char	line[128];
	int i, j, k, l, m;

	char	*getenv();

	/* initialize some time variables */
	for (i=0;i<7;i++)
		{
		timbeg_i[i] = 0;
		timend_i[i] = 0;
		}
	/* get current default values */
	status = mb_defaults(verbose,&format,&pings_get,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:P:p:L:l:R:r:B:b:E:e:S:s:T:t:I:i:Cc")) != -1)
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
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
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
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		output = stdout;
	else
		output = stderr;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(output,"usage: %s\n", usage_message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(output,"\ndbg2  Program <%s>\n",program_name);
		fprintf(output,"dbg2  Version %s\n",rcs_id);
		fprintf(output,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(output,"dbg2  Control Parameters:\n");
		fprintf(output,"dbg2       verbose:    %d\n",verbose);
		fprintf(output,"dbg2       help:       %d\n",help);
		fprintf(output,"dbg2       format:     %d\n",format);
		fprintf(output,"dbg2       pings:      %d\n",pings_read);
		fprintf(output,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(output,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(output,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(output,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(output,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(output,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(output,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(output,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(output,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(output,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(output,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(output,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(output,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(output,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(output,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(output,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(output,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(output,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(output,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(output,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(output,"dbg2       timegap:    %f\n",timegap);
		fprintf(output,"dbg2       comments:   %d\n",comments);
		fprintf(output,"dbg2       file:       %s\n",read_file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* if reading from datalist then variance calculations
		are disabled */
	if (read_datalist == MB_YES)
		pings_read = 1;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((fp = fopen(read_file,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (fgets(line,128,fp) != NULL
		&& sscanf(line,"%s %d",file,&format) == 2)
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

	/* obtain format array location - format id will 
		be aliased to current id if old format id given */
	status = mb_format(verbose,&format,&format_num,&error);

	/* initialize reading the multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings_get,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(output,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(output,"\nProgram <%s> Terminated\n",
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
			datacur->bath = NULL;
			datacur->amp = NULL;
			datacur->bathlon = NULL;
			datacur->bathlat = NULL;
			datacur->ss = NULL;
			datacur->sslon = NULL;
			datacur->sslat = NULL;
			}
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bath,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp*sizeof(double),
					&datacur->amp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bathlon,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(double),
					&datacur->bathlat,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->ss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->sslon,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&datacur->sslat,&error);
		}
	if (pings_read > 1)
		{
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(double),
				&bathmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(double),
				&bathvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_bath*sizeof(int),
				&nbathvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp*sizeof(double),
				&ampmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp*sizeof(double),
				&ampvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,beams_amp*sizeof(int),
				&nampvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&ssmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&ssvar,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,pixels_ss*sizeof(int),
				&nssvar,&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize data arrays */
	if (pings_read > 1)
		{
		for (i=0;i<beams_bath;i++)
			{
			bathmean[i] = 0.0;
			bathvar[i] = 0.0;
			nbathvar[i] = 0;
			}
		for (i=0;i<beams_amp;i++)
			{
			ampmean[i] = 0.0;
			ampvar[i] = 0.0;
			nampvar[i] = 0;
			}
		for (i=0;i<pixels_ss;i++)
			{
			ssmean[i] = 0.0;
			ssvar[i] = 0.0;
			nssvar[i] = 0;
			}
		}

	/* printf out file and format */
	mb_format_inf(verbose,format_num,&message);
	fprintf(output,"\nMultibeam Data File:  %s\n",file);
	fprintf(output,"MBIO Data Format ID:  %d\n",format);
	fprintf(output,"%s",message);

	/* read and process data */
	while (error <= MB_ERROR_NO_ERROR)
		{
		nread = 0;
		error = MB_ERROR_NO_ERROR;
		while (nread < pings_read && error == MB_ERROR_NO_ERROR)
			{

			/* read a ping of data */
			datacur = data[nread];
			bath = datacur->bath;
			amp = datacur->amp;
			bathlon = datacur->bathlon;
			bathlat = datacur->bathlat;
			ss = datacur->ss;
			sslon = datacur->sslon;
			sslat = datacur->sslat;
			status = mb_read(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathlon,bathlat,
				ss,sslon,sslat,
				comment,&error);

			/* increment counters */
			if (error == MB_ERROR_NO_ERROR 
				|| error == MB_ERROR_TIME_GAP)
				{
				irec++;
				nread++;
				}

			/* print comment records */
			if (error == MB_ERROR_COMMENT && comments == MB_YES)
				{
				if (icomment == 0)
					{
					fprintf(output,"\nComments in file %s:\n",file);
					icomment++;
					}
				fprintf(output,"  %s\n",comment);
				}

			/* output error messages */
			if (error == MB_ERROR_COMMENT)
				{
				/* do nothing */
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
				&& error >= MB_ERROR_OTHER)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nNonfatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Number of good records so far: %d\n",irec);
				}
			else if (verbose >= 1 && error > MB_ERROR_NO_ERROR 
				&& error != MB_ERROR_EOF)
				{
				mb_error(verbose,error,&message);
				fprintf(output,"\nFatal MBIO Error:\n%s\n",
					message);
				fprintf(output,"Last Good Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				}

			/* take note of min and maxes */
			if (error == MB_ERROR_NO_ERROR 
				|| error == MB_ERROR_TIME_GAP)
				{
				if (irec == 1)
					{
					lonmin = navlon;
					lonmax = navlon;
					latmin = navlat;
					latmax = navlat;
					if (beams_bath > 0)
						bathbeg = bath[beams_bath/2];
					lonbeg = navlon;
					latbeg = navlat;
					timbeg = time_d;
					for (i=0;i<7;i++)
						timbeg_i[i] = time_i[i];
					spdbeg = speed;
					hdgbeg = heading;
					}
				if (irec == 2 && spdbeg <= 0.0)
					spdbeg = speed;
				if (beginbath == 0 && beams_bath > 0)
					for (i=0;i<beams_bath;i++)
						if (bath[i] > 0.0)
							{
							bathmin = bath[i];
							bathmax = bath[i];
							beginbath = 1;
							}
				if (beginamp == 0 && beams_amp > 0)
					for (i=0;i<beams_amp;i++)
						if (amp[i] > 0.0)
							{
							ampmin = amp[i];
							ampmax = amp[i];
							beginamp = 1;
							}
				if (beginss == 0 && pixels_ss > 0)
					for (i=0;i<pixels_ss;i++)
						if (ss[i] > 0.0)
							{
							ssmin = ss[i];
							ssmax = ss[i];
							beginss = 1;
							}
				lonmin = min(lonmin, navlon);
				lonmax = max(lonmax, navlon);
				latmin = min(latmin, navlat);
				latmax = max(latmax, navlat);
				for (i=0;i<beams_bath;i++)
					{
					if (bath[i] > 0.0)
						{
						bathmin = min(bathmin, bath[i]);
						bathmax = max(bathmax, bath[i]);
						lonmin = min(lonmin, bathlon[i]);
						lonmax = max(lonmax, bathlon[i]);
						latmin = min(latmin, bathlat[i]);
						latmax = max(latmax, bathlat[i]);
						ngdbeams++;
						}
					else if (bath[i] == 0.0)
						nzdbeams++;
					else
						nfdbeams++;
					}
				for (i=0;i<beams_amp;i++)
					{
					if (amp[i] > 0)
						{
						ampmin = min(ampmin, amp[i]);
						ampmax = max(ampmax, amp[i]);
						ngabeams++;
						}
					else if (amp[i] == 0.0)
						nzabeams++;
					else
						nfabeams++;
					}
				for (i=0;i<pixels_ss;i++)
					{
					if (ss[i] > 0.0)
						{
						ssmin = min(ssmin, ss[i]);
						ssmax = max(ssmax, ss[i]);
						lonmin = min(lonmin, sslon[i]);
						lonmax = max(lonmax, sslon[i]);
						latmin = min(latmin, sslat[i]);
						latmax = max(latmax, sslat[i]);
						ngsbeams++;
						}
					else if (ss[i] == 0.0)
						nzsbeams++;
					else
						nfsbeams++;
					}
				distot = distot + distance;
				if (beams_bath > 0)
					bathend = bath[beams_bath/2];
				lonend = navlon;
				latend = navlat;
				spdend = speed;
				hdgend = heading;
				timend = time_d;
				for (i=0;i<7;i++)
					timend_i[i] = time_i[i];
				}
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(output,"\ndbg2  Reading loop finished in program <%s>\n",
				program_name);
			fprintf(output,"dbg2       status:     %d\n",status);
			fprintf(output,"dbg2       error:      %d\n",error);
			fprintf(output,"dbg2       nread:      %d\n",nread);
			fprintf(output,"dbg2       pings_read: %d\n",pings_read);
			}

		/* process the pings */
		if (pings_read > 2 
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
					if (bath[i] > 0.0)
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
					  if (bath[i] > 0.0)
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

				/* get mean sidescan */
				namp  = 0;
				mean  = 0.0;
				variance = 0.0;
				for (j=0;j<nread;j++)
					{
					datacur = data[j];
					amp = datacur->amp;
					if (amp[i] > 0.0)
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
					  if (amp[i] > 0.0)
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
			fprintf(output,"\ndbg2  Processing loop finished in program <%s>\n",
				program_name);
			fprintf(output,"dbg2       status:     %d\n",status);
			fprintf(output,"dbg2       error:      %d\n",error);
			fprintf(output,"dbg2       nread:      %d\n",nread);
			fprintf(output,"dbg2       pings_read: %d\n",pings_read);
			}
		}

	/* close the multibeam file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	for (i=0;i<pings_read;i++)
		{
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
                if (fgets(line,128,fp) != NULL
                        && sscanf(line,"%s %d",file,&format) == 2)
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
		fclose (fp);

	/* calculate final variances */
	if (pings_read > 2)
		{
		for (i=0;i<beams_bath;i++)
			if (nbathvar[i] > 0)
				{
				bathmean[i] = bathmean[i]/nbathvar[i];
				bathvar[i] = bathvar[i]/nbathvar[i];
				}
		for (i=0;i<beams_amp;i++)
			if (nampvar[i] > 0)
				{
				ampmean[i] = ampmean[i]/nampvar[i];
				ampvar[i] = ampvar[i]/nampvar[i];
				}
		for (i=0;i<pixels_ss;i++)
			if (nssvar[i] > 0)
				{
				ssmean[i] = ssmean[i]/nssvar[i];
				ssvar[i] = ssvar[i]/nssvar[i];
				}
		}

	/* calculate percentages of data */
	if (beams_bath*irec > 0)
		{
		ngd_percent = 100.0*ngdbeams/(beams_bath*irec);
		nzd_percent = 100.0*nzdbeams/(beams_bath*irec);
		nfd_percent = 100.0*nfdbeams/(beams_bath*irec);
		}
	else
		{
		ngd_percent = 0.0;
		nzd_percent = 0.0;
		nfd_percent = 0.0;
		}
	if (beams_amp*irec > 0)
		{
		nga_percent = 100.0*ngabeams/(beams_amp*irec);
		nza_percent = 100.0*nzabeams/(beams_amp*irec);
		nfa_percent = 100.0*nfabeams/(beams_amp*irec);
		}
	else
		{
		nga_percent = 0.0;
		nza_percent = 0.0;
		nfa_percent = 0.0;
		}
	if (pixels_ss*irec > 0)
		{
		ngs_percent = 100.0*ngsbeams/(pixels_ss*irec);
		nzs_percent = 100.0*nzsbeams/(pixels_ss*irec);
		nfs_percent = 100.0*nfsbeams/(pixels_ss*irec);
		}
	else
		{
		ngs_percent = 0.0;
		nzs_percent = 0.0;
		nfs_percent = 0.0;
		}

	/* now print out the results */
	timtot = (timend - timbeg)/3600.0;
	if (distot > 0.0)
		spdavg = distot/timtot;
	mb_get_jtime(verbose,timbeg_i,timbeg_j);
	mb_get_jtime(verbose,timend_i,timend_j);
	fprintf(output,"\nData Totals:\n");
	fprintf(output,"Number of Records:         %8d\n",irec);
	fprintf(output,"Bathymetry Data (%d beams):\n",beams_bath);
	fprintf(output,"  Number of Beams:         %8d\n",
	    (irec*beams_bath));
	fprintf(output,"  Number of Good Beams:    %8d     %5.2f%%\n",
		ngdbeams, ngd_percent);
	fprintf(output,"  Number of Zero Beams:    %8d     %5.2f%%\n",
		nzdbeams, nzd_percent);
	fprintf(output,"  Number of Flagged Beams: %8d     %5.2f%%\n",
		nfdbeams, nfd_percent);
	fprintf(output,"Amplitude Data (%d beams):\n",beams_amp);
	fprintf(output,"  Number of Beams:         %8d\n",
		(irec*beams_amp));
	fprintf(output,"  Number of Good Beams:    %8d     %5.2f%%\n",
		ngabeams, nga_percent);
	fprintf(output,"  Number of Zero Beams:    %8d     %5.2f%%\n",
		nzabeams, nza_percent);
	fprintf(output,"  Number of Flagged Beams: %8d     %5.2f%%\n",
		nfabeams, nfa_percent);
	fprintf(output,"Sidescan Data (%d pixels):\n",pixels_ss);
	fprintf(output,"  Number of Pixels:        %8d\n",
		(irec*pixels_ss));
	fprintf(output,"  Number of Good Pixels:   %8d     %5.2f%%\n",
		ngsbeams, ngs_percent);
	fprintf(output,"  Number of Zero Pixels:   %8d     %5.2f%%\n",
		nzsbeams, nzs_percent);
	fprintf(output,"  Number of Flagged Pixels:%8d     %5.2f%%\n",
		nfsbeams, nfs_percent);
	fprintf(output,"\nNavigation Totals:\n");
	fprintf(output,"Total Time:         %10.4f hours\n",timtot);
	fprintf(output,"Total Track Length: %10.4f km\n",distot);
	fprintf(output,"Average Speed:      %10.4f km/hr (%7.4f knots)\n",spdavg,spdavg/1.85);
	fprintf(output,"\nStart of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timbeg_i[1],timbeg_i[2],timbeg_i[0],timbeg_i[3],
		timbeg_i[4],timbeg_i[5],timbeg_i[6],timbeg_j[1]);
	fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f meters\n",
		lonbeg,latbeg,bathbeg);
	fprintf(output,"Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n",
		spdbeg,spdbeg/1.85,hdgbeg);
	fprintf(output,"\nEnd of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timend_i[1],timend_i[2],timend_i[0],timend_i[3],
		timend_i[4],timend_i[5],timend_i[6],timend_j[1]);
	fprintf(output,"Lon: %9.4f     Lat: %9.4f     Depth: %10.4f meters\n",
		lonend,latend,bathend);
	fprintf(output,"Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n",
		spdend,spdend/1.85,hdgend);
	fprintf(output,"\nLimits:\n");
	fprintf(output,"Minimum Longitude: %10.4f   Maximum Longitude: %10.4f\n",lonmin,lonmax);
	fprintf(output,"Minimum Latitude:  %10.4f   Maximum Latitude:  %10.4f\n",latmin,latmax);
	if (ngdbeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Depth:     %10.4f   Maximum Depth:     %10.4f\n",
			bathmin,bathmax);
	if (ngabeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Amplitude: %10.4f   Maximum Amplitude: %10.4f\n",
			ampmin,ampmax);
	if (ngsbeams > 0 || verbose >= 1)
		fprintf(output,"Minimum Sidescan:  %10.4f   Maximum Sidescan:  %10.4f\n",
			ssmin,ssmax);
	if (pings_read > 2 && beams_bath > 0 
		&& (ngdbeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nBeam Bathymetry Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<beams_bath;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nbathvar[i],bathmean[i],
				bathvar[i],sqrt(bathvar[i]));
		fprintf(output,"\n");
		}
	if (pings_read > 2 && beams_amp > 0 
		&& (ngabeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nBeam Amplitude Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<beams_amp;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nampvar[i],ampmean[i],
				ampvar[i],sqrt(ampvar[i]));
		fprintf(output,"\n");
		}
	if (pings_read > 2 && pixels_ss > 0 
		&& (ngsbeams > 0 || verbose >= 1))
		{
		fprintf(output,"\nPixel Sidescan Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<pixels_ss;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nssvar[i],ssmean[i],
				ssvar[i],sqrt(ssvar[i]));
		fprintf(output,"\n");
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

	/* set program status */
	status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(output,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(output,"dbg2  Ending status:\n");
		fprintf(output,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	fprintf(output,"\n");
	exit(status);
}
