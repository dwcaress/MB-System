/*--------------------------------------------------------------------
 *    The MB-system:	mbhistogram.c	12/28/94
 *    $Id: mbhistogram.c,v 4.5 1995-05-12 17:12:32 caress Exp $
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
 * MBHISTOGRAM reads a multibeam data file and generates a histogram
 * of the bathymetry,  amplitude,  or sidescan values. Alternatively, 
 * mbhistogram can output a list of values which break up the
 * distribution into equal sized regions.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	December 28, 1994
 *
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"

/* mode defines */
#define	MBHISTOGRAM_BATH	0
#define	MBHISTOGRAM_AMP		1
#define	MBHISTOGRAM_SS		2

/* min max defines */
#define	min(A, B)	((A) < (B) ? (A) : (B))
#define	max(A, B)	((A) > (B) ? (A) : (B))

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbhistogram.c,v 4.5 1995-05-12 17:12:32 caress Exp $";
	static char program_name[] = "MBHISTOGRAM";
	static char help_message[] =  "MBHISTOGRAM reads a multibeam data file and generates a histogram\n\tof the bathymetry,  amplitude,  or sidescan values. Alternatively, \n\tmbhistogram can output a list of values which break up the\n\tdistribution into equal sized regions.\n\tThe results are dumped to stdout.";
	static char usage_message[] = "mbhistogram [-Akind -Byr/mo/da/hr/mn/sc -Dmin/max -Eyr/mo/da/hr/mn/sc -Fformat -Ifile -Llonflip -Mnintervals -Nnbins -Ppings -Rw/e/s/n -Sspeed -V -H]";
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
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	char	*mbio_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[256];
	int	icomment = 0;

	/* histogram variables */
	int	mode = MBHISTOGRAM_SS;
	int	nbins = 0;
	int	nintervals = 0;
	double	value_min;
	double	value_max;
	double	dvalue_bin;
	double	value_bin_min;
	int	*histogram = NULL;
	double	*intervals = NULL;
	double	total;
	double	target;
	double	dinterval;
	double	bin_fraction;
	int	ibin;

	/* output stream for basic stuff (stdout if verbose <= 1,
		stderr if verbose > 1) */
	FILE	*output;

	int	read_data;
	char	line[128];
	int	i, j, k, l, m;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:D:d:E:e:F:f:HhI:i:L:l:M:m:N:n:P:p:R:r:S:s:T:t:Vv")) != -1)
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
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
		}

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
		status = mb_malloc(verbose,nbins*sizeof(int),
				&histogram,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nintervals*sizeof(double),
				&intervals,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating histogram arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get size of bins */
	dvalue_bin = (value_max - value_min)/(nbins-1);
	value_bin_min = value_min - 0.5*dvalue_bin;

	/* initialize histogram */
	for (i=0;i<nbins;i++)
		{
		histogram[i] = 0;
		}

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

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
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bath,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_amp*sizeof(double),
					&amp,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bathacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bathalongtrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ss,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ssacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ssalongtrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read and process data */
	while (error <= MB_ERROR_NO_ERROR)
		{

		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* process the pings */
		if (error == MB_ERROR_NO_ERROR 
			|| error == MB_ERROR_TIME_GAP)
			{

			/* do the bathymetry */
			if (mode == MBHISTOGRAM_BATH)
			for (i=0;i<beams_bath;i++)
				{
				if (bath[i] > 0.0)
					{
					j = (bath[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					}
				}

			/* do the amplitude */
			if (mode == MBHISTOGRAM_AMP)
			for (i=0;i<beams_amp;i++)
				{
				if (amp[i] > 0.0)
					{
					j = (amp[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					}
				}

			/* do the sidescan */
			if (mode == MBHISTOGRAM_SS)
			for (i=0;i<pixels_ss;i++)
				{
				if (ss[i] > 0.0)
					{
					j = (ss[i] - value_bin_min)
						/dvalue_bin;
					if (j >= 0 && j < nbins)
						histogram[j]++;
					}
				}

			}
		}

	/* close the multibeam file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,&bath,&error);
	mb_free(verbose,&amp,&error);
	mb_free(verbose,&bathacrosstrack,&error);
	mb_free(verbose,&bathalongtrack,&error);
	mb_free(verbose,&ss,&error);
	mb_free(verbose,&ssacrosstrack,&error);
	mb_free(verbose,&ssalongtrack,&error);

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

	/* calculate intervals if required */
	if (nintervals > 0)
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
			if (total < target)
				{
				while (total < target && ibin < nbins-1)
					{
					ibin++;
					total = total + histogram[ibin];
					}
				}
			bin_fraction = 1.0 - (total - target)/histogram[ibin];
			intervals[j] = value_bin_min 
					+ dvalue_bin*ibin
					+ bin_fraction*dvalue_bin;
			}
		}

	/* print out the results */
	if (nintervals <= 0)
		{
		for (i=0;i<nbins;i++)
			{
			fprintf(output,"%f %d\n",
				value_min+i*dvalue_bin,histogram[i]);
			}
		}
	else
		{
		for (i=0;i<nintervals;i++)
			fprintf(output,"%f\n",intervals[i]);
		}

	/* deallocate memory used for data arrays */
	mb_free(verbose,&histogram,&error);
	mb_free(verbose,&intervals,&error);

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
