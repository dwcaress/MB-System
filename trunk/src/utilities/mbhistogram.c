/*--------------------------------------------------------------------
 *    The MB-system:	mbhistogram.c	12/28/94
 *    $Id: mbhistogram.c,v 5.8 2008-09-13 06:08:09 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2003 by
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
 * MBHISTOGRAM reads a swath sonar data file and generates a histogram
 * of the bathymetry,  amplitude,  or sidescan values. Alternatively, 
 * mbhistogram can output a list of values which break up the
 * distribution into equal sized regions.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	December 28, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.7  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.6  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.5  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2005/03/25 04:42:59  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.3  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.12  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.9  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.8  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1995/08/11  18:51:37  caress
 * Added Gaussian distribution option.
 *
 * Revision 4.5  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.4  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.2  1995/02/27  14:43:18  caress
 * Fixed bug regarding closing a text input file.
 *
 * Revision 4.1  1995/01/06  00:06:41  caress
 * Can now read from either single data files or from multiple
 * data files specified in a datalist.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* mode defines */
#define	MBHISTOGRAM_BATH	0
#define	MBHISTOGRAM_AMP		1
#define	MBHISTOGRAM_SS		2

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbhistogram.c,v 5.8 2008-09-13 06:08:09 caress Exp $";
	static char program_name[] = "MBHISTOGRAM";
	static char help_message[] =  "MBHISTOGRAM reads a swath sonar data file and generates a histogram\n\tof the bathymetry,  amplitude,  or sidescan values. Alternatively, \n\tmbhistogram can output a list of values which break up the\n\tdistribution into equal sized regions.\n\tThe results are dumped to stdout.";
	static char usage_message[] = "mbhistogram [-Akind -Byr/mo/da/hr/mn/sc -Dmin/max -Eyr/mo/da/hr/mn/sc -Fformat -G -Ifile -Llonflip -Mnintervals -Nnbins -Ppings -Rw/e/s/n -Sspeed -V -H]";
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
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
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
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;

	/* histogram variables */
	int	mode = MBHISTOGRAM_SS;
	int	gaussian = MB_NO;
	int	nbins = 0;
	int	nintervals = 0;
	double	value_min = 0.0;
	double	value_max = 128.0;
	double	dvalue_bin;
	double	value_bin_min;
	double	value_bin_max;
	double	data_min;
	double	data_max;
	int	data_first = MB_YES;
	double	target_min;
	double	target_max;
	double	*histogram = NULL;
	double	*intervals = NULL;
	double	total;
	double	sum;
	double	p;
	double	target;
	double	dinterval;
	double	bin_fraction;
	int	ibin;

	/* output stream for basic stuff (stdout if verbose <= 1,
		stderr if verbose > 1) */
	FILE	*output;

	int	read_data;
	char	line[128];
	int	nrec, nvalue;
	int	nrectot = 0;
	int	nvaluetot = 0;
	int	i, j, k, l, m;
	double	qsnorm();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:D:d:E:e:F:f:GgHhI:i:L:l:M:m:N:n:P:p:R:r:S:s:T:t:Vv")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &mode);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%lf/%lf", &value_min,&value_max);
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
			gaussian = MB_YES;
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
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &nintervals);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &nbins);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
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
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* figure out histogram dimensions */
	if (nintervals > 0 && nbins <= 0)
		nbins = 50*nintervals;
	if (nbins <= 0)
		nbins = 16;

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
		fprintf(output,"dbg2       pings:      %d\n",pings);
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
		fprintf(output,"dbg2       file:       %s\n",read_file);
		fprintf(output,"dbg2       mode:       %d\n",mode);
		fprintf(output,"dbg2       gaussian:   %d\n",gaussian);
		fprintf(output,"dbg2       nbins:      %d\n",nbins);
		fprintf(output,"dbg2       nintervals: %d\n",nintervals);
		fprintf(output,"dbg2       value_min:  %f\n",value_min);
		fprintf(output,"dbg2       value_max:  %f\n",value_max);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* allocate memory for histogram arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose,__FILE__,__LINE__,nbins*sizeof(double),
				(void **)&histogram,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose,__FILE__,__LINE__,nintervals*sizeof(double),
				(void **)&intervals,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating histogram arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output some information */
	if (verbose > 0)
		{
		fprintf(stderr, "\nNumber of data bins: %d\n", nbins);
		fprintf(stderr, "Minimum value:         %f\n", value_min);
		fprintf(stderr, "Maximum value:         %f\n", value_max);
		if (mode == MBHISTOGRAM_BATH)
			fprintf(stderr, "Working on bathymetry data...\n");
		else if (mode == MBHISTOGRAM_AMP)
			fprintf(stderr, "Working on beam amplitude data...\n");
		else
			fprintf(stderr, "Working on sidescan data...\n");
		}

	/* get size of bins */
	dvalue_bin = (value_max - value_min)/(nbins-1);
	value_bin_min = value_min - 0.5*dvalue_bin;
	value_bin_max = value_max + 0.5*dvalue_bin;

	/* initialize histogram */
	for (i=0;i<nbins;i++)
		histogram[i] = 0;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

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

	/* obtain format array location - format id will 
		be aliased to current id if old format id given */
	status = mb_format(verbose,&format,&error);

	/* initialize reading the swath sonar data file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
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
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssalongtrack, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "\nprocessing file: %s %d\n", file, format);
	    }

	/* initialize counting variables */
	nrec = 0;
	nvalue = 0;

	/* read and process data */
	while (error <= MB_ERROR_NO_ERROR)
		{

		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* process the pings */
		if (error == MB_ERROR_NO_ERROR 
			|| error == MB_ERROR_TIME_GAP)
			{
			/* increment record counter */
			nrec++;

			/* do the bathymetry */
			if (mode == MBHISTOGRAM_BATH)
			for (i=0;i<beams_bath;i++)
				{
				if (mb_beam_ok(beamflag[i]))
					{
					nvalue++;
					j = (bath[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					if (data_first == MB_YES)
						{
						data_min = bath[i];
						data_max = bath[i];
						data_first = MB_NO;
						}
					else
						{
						data_min = MIN(bath[i], data_min);
						data_max = MAX(bath[i], data_max);
						}
					}
				}

			/* do the amplitude */
			if (mode == MBHISTOGRAM_AMP)
			for (i=0;i<beams_amp;i++)
				{
				if (mb_beam_ok(beamflag[i]))
					{
					nvalue++;
					j = (amp[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					if (data_first == MB_YES)
						{
						data_min = amp[i];
						data_max = amp[i];
						data_first = MB_NO;
						}
					else
						{
						data_min = MIN(amp[i], data_min);
						data_max = MAX(amp[i], data_max);
						}
					}
				}

			/* do the sidescan */
			if (mode == MBHISTOGRAM_SS)
			for (i=0;i<pixels_ss;i++)
				{
				if (ss[i] > MB_SIDESCAN_NULL)
					{
					nvalue++;
					j = (ss[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					if (data_first == MB_YES)
						{
						data_min = ss[i];
						data_max = ss[i];
						data_first = MB_NO;
						}
					else
						{
						data_min = MIN(ss[i], data_min);
						data_max = MAX(ss[i], data_max);
						}
					}
				}

			}
		}

	/* close the swath sonar data file */
	status = mb_close(verbose,&mbio_ptr,&error);
	nrectot += nrec;
	nvaluetot += nvalue;

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "%d records processed\n%d data processed\n", 
		    nrec, nvalue);
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

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "\n%d total records processed\n", nrectot);
	    fprintf(stderr, "%d total data processed\n\n", nvaluetot);
	    }

	/* recast histogram as gaussian */
	if (gaussian == MB_YES)
		{
		/* get total number of good values */
		total = 0.0;
		for (i=0;i<nbins;i++)
			total = total + histogram[i];

		/* recast histogram */
		sum = 0.0;
		for (i=0;i<nbins;i++)
			{
			p = (histogram[i]/2 + sum)/(total + 1);
			sum = sum + histogram[i];
			histogram[i] = qsnorm(p);
			}
		}

	/* calculate gaussian intervals if required */
	if (nintervals > 0 && gaussian == MB_YES)
		{
		/* get interval spacing */
		target_min = -2.0;
		target_max = 2.0;
		dinterval = (target_max - target_min)/(nintervals-1);

		/* get intervals */
		intervals[0] = MAX(data_min, value_min);
		intervals[nintervals-1] = MIN(data_max, value_max);
		ibin = 0;
		for (j=1;j<nintervals-1;j++)
			{
			target = target_min + j*dinterval;
			while (histogram[ibin] < target && ibin < nbins-1)
				ibin++;
			if (ibin > 0)
				bin_fraction = 1.0 - (histogram[ibin] - target)
					/(histogram[ibin] - histogram[ibin-1]);
			else
				bin_fraction = 0.0;
			intervals[j] = value_bin_min 
					+ dvalue_bin*ibin
					+ bin_fraction*dvalue_bin;
			}
		}

	/* calculate linear intervals if required */
	else if (nintervals > 0)
		{
		/* get total number of good values */
		total = 0.0;
		for (i=0;i<nbins;i++)
			total = total + histogram[i];

		/* get interval spacing */
		dinterval = total/(nintervals-1);

		/* get intervals */
		intervals[0] = value_bin_min;
		total = 0.0;
		ibin = -1;
		for (j=1;j<nintervals;j++)
			{
			target = j*dinterval;
			while (total < target && ibin < nbins-1)
				{
				ibin++;
				total = total + histogram[ibin];
				if (total <= 0.0)
					intervals[0] = value_bin_min
						+ dvalue_bin*ibin;
				}
			bin_fraction = 1.0 - (total - target)/histogram[ibin];
			intervals[j] = value_bin_min 
					+ dvalue_bin*ibin
					+ bin_fraction*dvalue_bin;
			}
		}

	/* print out the results */
	if (nintervals <= 0 && gaussian == MB_YES)
		{
		for (i=0;i<nbins;i++)
			{
			fprintf(output,"%f %f\n",
				value_min+i*dvalue_bin,histogram[i]);
			}
		}
	else if (nintervals <= 0)
		{
		for (i=0;i<nbins;i++)
			{
			fprintf(output,"%f %d\n",
				value_min+i*dvalue_bin,(int)histogram[i]);
			}
		}
	else
		{
		for (i=0;i<nintervals;i++)
			fprintf(output,"%f\n",intervals[i]);
		}

	/* deallocate memory used for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&histogram,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&intervals,&error);

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
	exit(error);
}
/*--------------------------------------------------------------------*/

/* double qsnorm(p)
 * double	p;
 *
 * Function to invert the cumulative normal probability
 * function.  If z is a standardized normal random deviate,
 * and Q(z) = p is the cumulative Gaussian probability 
 * function, then z = qsnorm(p).
 *
 * Note that 0.0 < p < 1.0.  Data values outside this range
 * will return +/- a large number (1.0e6).
 * To compute p from a sample of data to test for Normalcy,
 * sort the N samples into non-decreasing order, label them
 * i=[1, N], and then compute p = i/(N+1).
 *
 * Author:	Walter H. F. Smith
 * Date:	19 February, 1991-1995.
 *
 * Based on a Fortran subroutine by R. L. Parker.  I had been
 * using IMSL library routine DNORIN(DX) to do what qsnorm(p)
 * does, when I was at the Lamont-Doherty Geological Observatory
 * which had a site license for IMSL.  I now need to invert the
 * gaussian CDF without calling IMSL; hence, this routine.
 *
 */

double	qsnorm(double p)
{
	double	t, z;
	
	if (p <= 0.0) {
		return(-1.0e6);
	}
	else if (p >= 1.0) {
		return(1.0e6);
	}
	else if (p == 0.5) {
		return(0.0);
	}
	else if (p > 0.5) {
		t = sqrt(-2.0 * log(1.0 - p) );
		z = t - (2.515517 +t*(0.802853 +t*0.010328))/
			(1.0 + t*(1.432788 + t*(0.189269+ t*0.001308)));
		return(z);
	}
	else {
		t = sqrt(-2.0 * log(p) );
		z = t - (2.515517 +t*(0.802853 +t*0.010328))/
			(1.0 + t*(1.432788 + t*(0.189269+ t*0.001308)));
		return(-z);
	}
}
/*--------------------------------------------------------------------*/
