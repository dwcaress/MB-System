/*--------------------------------------------------------------------
 *    The MB-system:	mbinfo.c	3.00	2/1/93
 *    $Id: mbinfo.c,v 3.1 1993-06-12 04:29:33 caress Exp $
 *
 *    Copyright (c) 1993 by 
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
 * for the multibeam backscatter beams are calculated without detrending.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * $Log: not supported by cvs2svn $
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

/*--------------------------------------------------------------------*/

#define MBINFO_MAXPINGS 50
struct ping
	{
	int	*bath;
	int	*bathdist;
	int	*back;
	int	*backdist;
	};

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbinfo.c,v 3.1 1993-06-12 04:29:33 caress Exp $";
	static char program_name[] = "MBINFO";
	static char help_message[] =  "MBINFO reads a multibeam bathymetry data file and outputs \nsome basic statistics.  If pings are averaged (pings > 2) \nMBINFO estimates the variance for each of the multibeam \nbeams by reading a set number of pings (>2) and then finding \nthe variance of the detrended values for each beam. \nThe results are dumped to stdout.";
	static char usage_message[] = "mbinfo [-Fformat -Rw/e/s/n -Ppings -Sspeed -Llonflip\n	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -N -V -H -Ifile]";
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
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	pings_get = 1;
	int	pings_read = 1;
	int	beams_bath;
	int	beams_back;

	/* MBIO read values */
	char	*mbio_ptr;
	int	kind;
	struct ping *data[MBINFO_MAXPINGS];
	struct ping *datacur;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	*bath;
	int	*bathdist;
	int	*back;
	int	*backdist;
	char	comment[256];
	int	icomment = 0;
	int	comments = MB_YES;

	/* limit variables */
	double	lonmin, lonmax, latmin, latmax;
	double	bathmin, bathmax, backmin, backmax;
	double	bathbeg, lonbeg, latbeg, bathend, lonend, latend;
	double	spdbeg, hdgbeg, spdend, hdgend;
	double	timbeg, timend;
	int	timbeg_i[6], timend_i[6];
	double	distot = 0.0;
	double	timtot, spdavg;
	int	irec = 0;
	int	ngdbeams = 0;
	int	nzdbeams = 0;
	int	nfdbeams = 0;
	int	ngbbeams = 0;
	int	nzbbeams = 0;
	int	nfbbeams = 0;
	int	begin = 0;
	int	nread;

	/* variance finding variables */
	int	nbath;
	int	nback;
	double	sumx, sumxx, sumy, sumxy, delta, a, b, dev, variance;
	double	*bathmean;
	double	*bathvar;
	int	*nbathvar;
	double	*backmean;
	double	*backvar;
	int	*nbackvar;

	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*output;

	int i, j, k, l, m;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings_get,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:P:p:L:l:R:r:B:b:E:e:S:s:T:t:I:i:Nn")) != -1)
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
			sscanf (optarg,"%d", &format);
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
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'N':
		case 'n':
			comments = MB_NO;
			flag++;
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
		fprintf(output,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(output,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(output,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(output,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(output,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(output,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(output,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(output,"dbg2       timegap:    %f\n",timegap);
		fprintf(output,"dbg2       comments:   %d\n",comments);
		fprintf(output,"dbg2       file:       %s\n",file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* initialize reading the multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings_get,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
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
		if ((data[i] = (struct ping *) 
			calloc(pings_read,sizeof(struct ping))) == NULL) 
			error = MB_ERROR_MEMORY_FAIL;
		if (error == MB_ERROR_NO_ERROR)
			{
			datacur = data[i];
			if ((datacur->bath = (int *) 
				calloc(beams_bath,sizeof(int))) == NULL) 
				error = MB_ERROR_MEMORY_FAIL;
			if ((datacur->bathdist = (int *) 
				calloc(beams_bath,sizeof(int))) == NULL)
				error = MB_ERROR_MEMORY_FAIL;
			if ((datacur->back = (int *) 
				calloc(beams_back,sizeof(int))) == NULL) 
				error = MB_ERROR_MEMORY_FAIL;
			if ((datacur->backdist = (int *) 
				calloc(beams_back,sizeof(int))) == NULL)
				error = MB_ERROR_MEMORY_FAIL;
			}
		}
	if ((bathmean = (double *) calloc(beams_bath,sizeof(double))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;
	if ((bathvar = (double *) calloc(beams_bath,sizeof(double))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;
	if ((nbathvar = (int *) calloc(beams_bath,sizeof(int))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;
	if ((backmean = (double *) calloc(beams_back,sizeof(double))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;
	if ((backvar = (double *) calloc(beams_back,sizeof(double))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;
	if ((nbackvar = (int *) calloc(beams_back,sizeof(int))) == NULL) 
		error = MB_ERROR_MEMORY_FAIL;

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* printf out file and format */
	mb_format_inf(verbose,format,&message);
	fprintf(output,"\nMultibeam Data File:  %s\n",file);
	fprintf(output,"MBIO Data Format ID:  %d\n",format);
	fprintf(output,"%s",message);

	/* initialize data arrays */
	for (i=0;i<beams_bath;i++)
		{
		bathmean[i] = 0.0;
		bathvar[i] = 0.0;
		nbathvar[i] = 0;
		}
	for (i=0;i<beams_back;i++)
		{
		backmean[i] = 0.0;
		backvar[i] = 0.0;
		nbackvar[i] = 0;
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
			bath = datacur->bath;
			bathdist = datacur->bathdist;
			back = datacur->back;
			backdist = datacur->backdist;
			status = mb_get(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,&speed,&heading,&distance,
				&beams_bath,bath,bathdist,
				&beams_back,back,backdist,
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
				fprintf(output,"Time: %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
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
				fprintf(output,"Last Good Time: %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
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
					bathbeg = (double) bath[beams_bath/2];
					lonbeg = navlon;
					latbeg = navlat;
					timbeg = time_d;
					for (i=0;i<6;i++)
						timbeg_i[i] = time_i[i];
					spdbeg = speed;
					hdgbeg = heading;
					}
				if (irec == 2 && spdbeg <= 0.0)
					spdbeg = speed;
				if (begin == 0)
					for (i=0;i<beams_bath;i++)
						if (bath[i] > 0)
							{
							bathmin = (double) bath[i];
							bathmax = (double) bath[i];
							begin = 1;
							}
				if (navlon < lonmin) lonmin = navlon;
				if (navlon > lonmax) lonmax = navlon;
				if (navlat < latmin) latmin = navlat;
				if (navlat > latmax) latmax = navlat;
				for (i=0;i<beams_bath;i++)
					{
					if (bath[i] > 0)
						{
						if (bath[i] < (int) bathmin) bathmin = (double) bath[i];
						if (bath[i] > (int) bathmax) bathmax = (double) bath[i];
						ngdbeams++;
						}
					else if (bath[i] == 0)
						nzdbeams++;
					else
						nfdbeams++;
					}
				distot = distot + distance;
				bathend = (double) bath[beams_bath/2];
				lonend = navlon;
				latend = navlat;
				spdend = speed;
				hdgend = heading;
				timend = time_d;
				for (i=0;i<6;i++)
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
					if (bath[i] > 0)
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
					  if (bath[i] > 0)
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

			/* do the backscatter */
			for (i=0;i<beams_back;i++)
				{

				/* get mean backscatter */
				nback  = 0;
				backmean[i]  = 0.0;
				variance = 0.0;
				for (j=0;j<nread;j++)
					{
					datacur = data[j];
					back = datacur->back;
					if (back[i] > 0)
					  {
					  nback++;
					  backmean[i]  = backmean[i] + back[i];
					  }
					}
				if (nback == pings_read)
					{
					backmean[i] = backmean[i]/nback;
					for (j=0;j<nread;j++)
					  {
					  datacur = data[j];
					  back = datacur->back;
					  if (back[i] > 0)
					    {
					    dev = back[i] - backmean[i];
					    variance = variance + dev*dev;
					    }
					  }
					backvar[i] = backvar[i] + variance;
					nbackvar[i] = nbackvar[i] + nback;
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
	status = mb_close(verbose,mbio_ptr,&error);

	/* calculate final variances */
	if (pings_read > 2)
		{
		for (i=0;i<beams_bath;i++)
			if (nbathvar[i] > 0)
				{
				bathmean[i] = bathmean[i]/nbathvar[i];
				bathvar[i] = bathvar[i]/nbathvar[i];
				}
		for (i=0;i<beams_back;i++)
			if (nbackvar[i] > 0)
				{
				backmean[i] = backmean[i]/nbackvar[i];
				backvar[i] = backvar[i]/nbackvar[i];
				}
		}

	/* now print out the results */
	timtot = (timend - timbeg)/60.0;
	distot = distot;
	spdavg = distot/timtot;
	fprintf(output,"\nData Totals:\n");
	fprintf(output,"Number of Records:        %8d\n",irec);
	fprintf(output,"Bathymetry Data:\n");
	fprintf(output,"  Number of Beams:        %8d\n",(irec*beams_bath));
	fprintf(output,"  Number of Good Beams:   %8d\n",ngdbeams);
	fprintf(output,"  Number of Zero Beams:   %8d\n",nzdbeams);
	fprintf(output,"  Number of flagged beams:%8d\n",nfdbeams);
	fprintf(output,"Backscatter Data:\n");
	fprintf(output,"  Number of Beams:        %8d\n",(irec*beams_back));
	fprintf(output,"  Number of Good Beams:   %8d\n",ngbbeams);
	fprintf(output,"  Number of Zero Beams:   %8d\n",nzbbeams);
	fprintf(output,"  Number of flagged beams:%8d\n",nfbbeams);
	fprintf(output,"\nNavigation Totals:\n");
	fprintf(output,"Total Time:         %10.4f hours\n",timtot);
	fprintf(output,"Total Track Length: %10.4f km\n",distot);
	fprintf(output,"Average Speed:      %10.4f km/hr\n",spdavg);
	fprintf(output,"\nStart of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d\n",timbeg_i[1],
		timbeg_i[2],timbeg_i[0],timbeg_i[3],timbeg_i[4],timbeg_i[5]);
	fprintf(output,"Lon: %9.4f     Lat: %9.4f   Depth:%8.2f\n",lonbeg,latbeg,bathbeg);
	fprintf(output,"Speed:%8.4f  Heading:%9.4f\n",spdbeg,hdgbeg);
	fprintf(output,"\nEnd of Data:\n");
	fprintf(output,"Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d\n",timend_i[1],
		timend_i[2],timend_i[0],timend_i[3],timend_i[4],timend_i[5]);
	fprintf(output,"Lon: %9.4f     Lat: %9.4f   Depth:%8.2f\n",lonend,latend,bathend);
	fprintf(output,"Speed:%8.4f  Heading:%9.4f\n",spdend,hdgend);
	fprintf(output,"\nLimits:\n");
	fprintf(output,"  Minimum Lon: %10.4f     Maximum Lon: %10.4f\n",lonmin,lonmax);
	fprintf(output,"  Minimum Lat: %10.4f     Maximum Lat: %10.4f\n",latmin,latmax);
        fprintf(output,"Minimum Depth: %10.4f   Maximum Depth: %10.4f\n",bathmin,bathmax);
	if (pings_read > 2 && beams_bath > 0)
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
	if (pings_read > 2 && beams_back > 0)
		{
		fprintf(output,"\nBeam Backscatter Variances:\n");
		fprintf(output,"Pings Averaged: %d\n",pings_read);
		fprintf(output," Beam     N      Mean     Variance    Sigma\n");
		fprintf(output," ----     -      ----     --------    -----\n");
		for (i=0;i<beams_back;i++)
			fprintf(output,"%4d  %5d   %8.2f   %8.2f  %8.2f\n",
				i,nbackvar[i],backmean[i],
				backvar[i],sqrt(backvar[i]));
		fprintf(output,"\n");
		}

	/* deallocate memory used for data arrays */
	for (i=0;i<pings_read;i++)
		{
		free(data[i]->bath);
		free(data[i]->bathdist);
		free(data[i]->back);
		free(data[i]->backdist);
		free(data[i]);
		}
	free(bathmean);
	free(bathvar);
	free(backmean);
	free(backvar);

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
