/*--------------------------------------------------------------------
 *    The MB-system:	mbbackangle.c	1/6/95
 *    $Id: mbbackangleold.c,v 4.0 1995-02-14 21:17:15 caress Exp $
 *
 *    Copyright (c) 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBBACKANGLE reads a multibeam data file and generates a table
 * of the average amplitude or sidescan values as a function of
 * the grazing angle with the seafloor. If bathymetry is
 * not available,  the seafloor is assumed to be flat. The takeoff
 * angle for each beam or pixel arrival is projected to the seafloor;
 * no raytracing is done.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 6, 1995
 *
 * $Log: not supported by cvs2svn $
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* MBIO include files */
#include "../../include/mb_status.h"

/* mode defines */
#define	MBBACKANGLE_AMP	1
#define	MBBACKANGLE_SS	2

/* RTD define */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#define RTD     (180./M_PI)

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbbackangleold.c,v 4.0 1995-02-14 21:17:15 caress Exp $";
	static char program_name[] = "mbbackangle";
	static char help_message[] =  
"mbbackangle reads a multibeam data file and generates a table\n\t\
of the average amplitude or sidescan values as a function of\n\t\
the angle of interaction with the seafloor. If bathymetry is\n\t\
not available,  the seafloor is assumed to be flat.\n\t\
The results are dumped to stdout.";
	static char usage_message[] = "mbbackangle [-Akind \
-Byr/mo/da/hr/mn/sc -C -Dmax_angle -Eyr/mo/da/hr/mn/sc -Fformat \
-Ifile -Llonflip -Nnangles -Ppings -Rw/e/s/n -Sspeed -Zdepth -V -H]";
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

	/* slope calculation variables */
	int	ndepths;
	double	*depths;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;

	/* angle function variables */
	int	ampkind = MBBACKANGLE_SS;
	int	symmetry = MB_YES;
	int	nangles = 161;
	double	angle_min = -80.0;
	double	angle_max = 80.0;
	double	angle_start;
	double	dangle;
	int	*nmean = NULL;
	double	*mean = NULL;
	double	*angles = NULL;
	double	*sigma = NULL;
	double	depth_default = 0.0;

	int	read_data;
	char	line[128];
	double	bathy;
	double	slope;
	double	angle;
	double	slopeangle;
	int	nrec, nvalue;
	int	nrectot = 0;
	int	nvaluetot = 0;
	int	i, j, k, l, m;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset pings and timegap */
	pings = 1;
	timegap = 10000000.0;

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:CcE:e:F:f:HhI:i:N:n:R:r:S:s:VvZ:z:")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &ampkind);
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
		case 'C':
		case 'c':
			symmetry = MB_NO;
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
		case 'N':
		case 'n':
			sscanf (optarg,"%d/%lf", &nangles, &angle_max);
			angle_min = -angle_max;
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
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &depth_default);
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       help:       %d\n",help);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		fprintf(stderr,"dbg2       pings:      %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",timegap);
		fprintf(stderr,"dbg2       file:       %s\n",read_file);
		fprintf(stderr,"dbg2       ampkind:    %d\n",ampkind);
		fprintf(stderr,"dbg2       nangles:    %d\n",nangles);
		fprintf(stderr,"dbg2       angle_min:  %f\n",angle_min);
		fprintf(stderr,"dbg2       angle_max:  %f\n",angle_max);
		fprintf(stderr,"dbg2       depth_def:  %f\n",depth_default);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* allocate memory for angle arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nangles*sizeof(int),
				&nmean,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nangles*sizeof(double),
				&mean,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nangles*sizeof(double),
				&angles,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nangles*sizeof(double),
				&sigma,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating angle arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output some information */
	if (verbose > 0)
		{
		fprintf(stderr, "\nNumber of angle bins: %d\n", nangles);
		fprintf(stderr, "Minimum angle:         %f\n", angle_min);
		fprintf(stderr, "Maximum angle:         %f\n", angle_max);
		fprintf(stderr, "Default depth:         %f\n", depth_default);
		if (ampkind == MBBACKANGLE_AMP)
			fprintf(stderr, "Working on beam amplitude data...\n");
		else
			fprintf(stderr, "Working on sidescan data...\n");
		}

	/* get size of bins */
	dangle = (angle_max - angle_min)/(nangles-1);
	angle_start = angle_min - 0.5*dangle;

	/* initialize histogram */
	for (i=0;i<nangles;i++)
		{
		nmean[i] = 0;
		mean[i] = 0.0;
		angles[i] = angle_min + i*dangle;
		sigma[i] = 0.0;
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
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
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
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depths,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depthacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopes,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopeacrosstrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
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
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* process the pings */
		if (error == MB_ERROR_NO_ERROR 
		    || error == MB_ERROR_TIME_GAP)
		    {
		    /* increment record counter */
		    nrec++;

		    /* get the seafloor slopes */
		    if (beams_bath > 0)
			set_bathyslope(verbose, 
				beams_bath,bath,bathacrosstrack,
				&ndepths,depths,depthacrosstrack,
				&nslopes,slopes,slopeacrosstrack,
				&error);

		    /* do the amplitude */
		    if (ampkind == MBBACKANGLE_AMP)
		    for (i=0;i<beams_amp;i++)
			{
			if (amp[i] > 0.0)
			    {
			    nvalue++;
			    if (beams_bath != beams_amp)
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    else
				{
				status = get_bathyslope(verbose,
				    ndepths,depths,depthacrosstrack,
				    nslopes,slopes,slopeacrosstrack,
				    bathacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    if (bathy > 0.0)
				{
				angle = RTD*(atan(bathacrosstrack[i]/bathy)
				    + atan(slope));
				j = (angle - angle_start)/dangle;
				if (j >= 0 && j < nangles)
				    {
				    mean[j] += amp[i];
				    sigma[j] += amp[i]*amp[i];
				    nmean[j]++;
				    }
				}
			    }
			}

		    /* do the sidescan */
		    if (ampkind == MBBACKANGLE_SS)
		    for (i=0;i<pixels_ss;i++)
			{
			if (ss[i] > 0.0)
			    {
			    nvalue++;
			    if (beams_bath > 0)
				{
				status = get_bathyslope(verbose,
				    ndepths,depths,depthacrosstrack,
				    nslopes,slopes,slopeacrosstrack,
				    ssacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    if (bathy > 0.0)
				{
				angle = RTD*(atan(ssacrosstrack[i]/bathy) 
				    + atan(slope));
				j = (angle - angle_start)/dangle;
				if (j >= 0 && j < nangles)
				    {
				    mean[j] += ss[i];
				    sigma[j] += ss[i]*ss[i];
				    nmean[j]++;
				    }
				}
			    }
			}

		    }
		}

	/* close the multibeam file */
	status = mb_close(verbose,&mbio_ptr,&error);
	nrectot += nrec;
	nvaluetot += nvalue;

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "%d records processed\n%d data processed\n", 
		    nrec, nvalue);
	    }

	/* deallocate memory used for data arrays */
	mb_free(verbose,&bath,&error);
	mb_free(verbose,&amp,&error);
	mb_free(verbose,&bathacrosstrack,&error);
	mb_free(verbose,&bathalongtrack,&error);
	mb_free(verbose,&ss,&error);
	mb_free(verbose,&ssacrosstrack,&error);
	mb_free(verbose,&ssalongtrack,&error);
	mb_free(verbose,&depths,&error);
	mb_free(verbose,&depthacrosstrack,&error);
	mb_free(verbose,&slopes,&error);
	mb_free(verbose,&slopeacrosstrack,&error);

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
	fclose (fp);

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "\n%d total records processed\n", nrectot);
	    fprintf(stderr, "%d total data processed\n\n", nvaluetot);
	    }

	/* process sums */
	if (symmetry == MB_NO)
	{
	for (i=0;i<nangles;i++)
		{
		if (nmean[i] > 0)
			{
			mean[i] = mean[i]/nmean[i];
			sigma[i] = sqrt(sigma[i]/nmean[i] 
				- mean[i]*mean[i]);
			}
		}
	}
	else
	{
	k = nangles/2;
	if (fmod((double)nangles, 2) > 0.0)
		k++;
	for (i=0;i<k;i++)
		{
		j = nangles - i - 1;
		if (nmean[i] + nmean[j] > 0)
			{
			mean[i] = mean[i] + mean[j];
			nmean[i] = nmean[i] + nmean[j];
			sigma[i] = sigma[i] + sigma[j];
			mean[i] = mean[i]/nmean[i];
			sigma[i] = sqrt(sigma[i]/nmean[i] 
				- mean[i]*mean[i]);
			mean[j] = mean[i];
			nmean[j] = nmean[i];
			sigma[j] = sigma[i];
			}
		}
	}

	/* print out the results */
	for (i=0;i<nangles;i++)
		{
		if (nmean[i] > 0)
			{
			fprintf(stdout,"%f %f\n",
				angles[i],mean[i]);
			}
		}

	/* deallocate memory used for data arrays */
	mb_free(verbose,&nmean,&error);
	mb_free(verbose,&mean,&error);
	mb_free(verbose,&angles,&error);
	mb_free(verbose,&sigma,&error);

	/* set program status */
	status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	if (verbose > 0)
		fprintf(stderr,"\n");
	exit(status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(verbose,
	nbath,bath,bathacrosstrack,
	ndepths,depths,depthacrosstrack, 
	nslopes,slopes,slopeacrosstrack, 
	error)
int	verbose;
int	nbath;
double	*bath;
double	*bathacrosstrack;
int	*ndepths;
double	*depths;
double	*depthacrosstrack;
int	*nslopes;
double	*slopes;
double	*slopeacrosstrack;
int	*error;
{
	char	*function_name = "set_bathyslope";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, bath[i], bathacrosstrack[i]);
		}

	/* first find all depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		if (bath[i] > 0.0)
			{
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
			}
		}

	/* now calculate slopes */
	*nslopes = *ndepths + 1;
	for (i=0;i<*ndepths-1;i++)
		{
		slopes[i+1] = (depths[i+1] - depths[i])
			/(depthacrosstrack[i+1] - depthacrosstrack[i]);
		slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
			+ depthacrosstrack[i]);
		}
	if (*ndepths > 1)
		{
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0;
		slopeacrosstrack[*ndepths] = 
			depthacrosstrack[*ndepths-1];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<*ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(verbose,
	ndepths,depths,depthacrosstrack,
	nslopes,slopes,slopeacrosstrack, 
	acrosstrack,depth,slope,error)
int	verbose;
int	ndepths;
double	*depths;
double	*depthacrosstrack;
int	nslopes;
double	*slopes;
double	*slopeacrosstrack;
double	acrosstrack;
double	*depth;
double	*slope;
int	*error;
{
	char	*function_name = "get_bathyslope";
	int	status = MB_SUCCESS;
	int	found_depth, found_slope;
	int	idepth, islope;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       acrosstrack:     %f\n",acrosstrack);
		}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
	if (acrosstrack >= depthacrosstrack[0]
		&& acrosstrack <= depthacrosstrack[ndepths-1])
	    {

	    /* look for depth */
	    idepth = -1;
	    while (found_depth == MB_NO && idepth < ndepths - 2)
		{
		idepth++;
		if (acrosstrack >= depthacrosstrack[idepth]
		    && acrosstrack <= depthacrosstrack[idepth+1])
		    {
		    *depth = depths[idepth] 
			    + (acrosstrack - depthacrosstrack[idepth])
			    /(depthacrosstrack[idepth+1] 
			    - depthacrosstrack[idepth])
			    *(depths[idepth+1] - depths[idepth]);
		    found_depth = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}

	    /* look for slope */
	    islope = -1;
	    while (found_slope == MB_NO && islope < nslopes - 2)
		{
		islope++;
		if (acrosstrack >= slopeacrosstrack[islope]
		    && acrosstrack <= slopeacrosstrack[islope+1])
		    {
		    *slope = slopes[islope] 
			    + (acrosstrack - slopeacrosstrack[islope])
			    /(slopeacrosstrack[islope+1] 
			    - slopeacrosstrack[islope])
			    *(slopes[islope+1] - slopes[islope]);
		    found_slope = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}
	    }

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:           %f\n",*depth);
		fprintf(stderr,"dbg2       slope:           %f\n",*slope);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
