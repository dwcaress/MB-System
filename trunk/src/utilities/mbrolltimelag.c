/*--------------------------------------------------------------------
 *    The MB-system:	mbrolltimelag.c	11/10/2005
 *
 *    $Id$
 *
 *    Copyright (c) 2005-2011 by
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
 * MBrolltimelag extracts the roll time series and the apparent bottom
 * slope (linear fit to unflagged soundings for each ping) time series
 * from swath data, and then calculates the cross correlation between
 * the roll and the slope minus roll for a specified set of time lags.
 * The suite of cross correlation calculations are made for each
 * successive npings pings (default = 100) in each swath file. The
 * results are output to files, and cross correlation plots are 
 * generated.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 2005
 *
 * $Log: mbrolltimelag.c,v $
 * Revision 5.7  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.6  2008/02/12 02:48:39  caress
 * Added ability to set output filename root.
 *
 * Revision 5.5  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.4  2006/04/26 22:05:26  caress
 * Changes to handle MBARI Mapping AUV data better.
 *
 * Revision 5.3  2006/04/19 18:31:23  caress
 * Fixed use of mbm_histplot.
 *
 * Revision 5.2  2006/04/11 19:19:30  caress
 * Various fixes.
 *
 * Revision 5.1  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.0  2006/01/06 18:20:56  caress
 * Working towards 5.0.8
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

#define	MBRTL_ALLOC_CHUNK	1000

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "MBrolltimelag";
	char help_message[] = "MBrolltimelag extracts the roll time series and the apparent \nbottom slope time series from swath data, and then calculates \nthe cross correlation between the roll and the slope minus roll \nfor a specified set of time lags.";
	char usage_message[] = "mbrolltimelag -Iswathdata [-Fformat -Nnping -Ooutputname -Snavchannel -Tnlag/lagmin/lagmax -V -H ]";

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* Files and formats */
	char	swathdata[MB_PATH_MAXLINE];
	char	swathfile[MB_PATH_MAXLINE];
	char	swathroot[MB_PATH_MAXLINE];
	char	outroot[MB_PATH_MAXLINE];
	char	outroot_defined = MB_NO;
	char	xcorfile[MB_PATH_MAXLINE];
	char	xcorfiletot[MB_PATH_MAXLINE];
	char	cmdfile[MB_PATH_MAXLINE];
	char	histfile[MB_PATH_MAXLINE];
	char	fhistfile[MB_PATH_MAXLINE];
	char	modelfile[MB_PATH_MAXLINE];
	int	format = 0;
	int	formatguess = 0;
	FILE	*fp = NULL;
	FILE	*fpx = NULL;
	FILE	*fpf = NULL;
	FILE	*fpt = NULL;
	FILE	*fph = NULL;
	FILE	*fpm = NULL;
	int	read_datalist = MB_NO;
	int	read_data = MB_NO;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	
	/* cross correlation parameters */
	int	navchannel = MB_DATA_DATA;
	int	npings = 100;
	int	nlag = 41;
	double	lagmin = -2.0;
	double	lagmax = 2.0;
	double	lagstep = 0.05;
	double	*rr = NULL;
	
	/* slope data */
	int	nslope = 0;
	int	nslopetot = 0;
	int	nslope_alloc = 0;
	double	*slope_time_d = NULL;
	double	*slope_slope = NULL;
	double	*slope_roll = NULL;
	int	nroll = 0;
	int	nroll_alloc = 0;
	double	*roll_time_d = NULL;
	double	*roll_roll = NULL;
	
	/* timelag histogram array */
	int	*timelaghistogram = NULL;
	
	double	time_d;
	double	roll;
	double	slope;
	double	timelag;
	double	sumsloperoll;
	double	sumslopesq;
	double	sumrollsq;
	double	slopeminusmean;
	double	rollminusmean;
	double	r;
	
	int	nrollmean;
	double	rollmean;
	double	slopemean;
	
	double	maxtimelag;
	double	maxr;
	double	peaktimelag;
	double	peakr;
	int	peakk;
	int	peakkmax;
	int	peakksum;
	double	time_d_avg;
	int	nmodel = 0;
	
	int	nr;
	double	rollint;	
	int	found;
	int	nscan;
	int	j0, j1;
	int	i, j, k, l;
	
	/* set default input */
	strcpy(swathdata, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:O:o:N:n:S:s:T:t:")) != -1)
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", swathdata);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &npings);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", outroot);
			outroot_defined = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d", &navchannel);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%d/%lf/%lf", &nlag, &lagmin, &lagmax);
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
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
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
		fprintf(stderr,"dbg2       swathdata:       %s\n",swathdata);
		fprintf(stderr,"dbg2       npings:          %d\n",npings);
		fprintf(stderr,"dbg2       nlag:            %d\n",nlag);
		fprintf(stderr,"dbg2       lagmin:          %f\n",lagmin);
		fprintf(stderr,"dbg2       lagmax:          %f\n",lagmax);
		fprintf(stderr,"dbg2       navchannel:      %d\n",navchannel);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	mb_get_format(verbose,swathdata,swathroot,&formatguess,&error);
	if (format == 0)
		format = formatguess;
	if (outroot_defined == MB_NO)
		strcpy(outroot, swathroot);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;
		
	/* get time lag step */
	lagstep = (lagmax - lagmin) / (nlag - 1);
	status = mb_reallocd(verbose,__FILE__,__LINE__, nlag * sizeof(double), (void **)&rr, &error);
	status = mb_reallocd(verbose,__FILE__,__LINE__, nlag * sizeof(int), (void **)&timelaghistogram, &error);
		
	/* print out some helpful information */
	if (verbose > 0)
		{
		fprintf(stderr, "Program %s parameters:\n", program_name);
		fprintf(stderr, "  Input:                           %s\n", swathdata);
		fprintf(stderr, "  Format:                          %d\n", format);
		fprintf(stderr, "  Number of pings per estimate:    %d\n", npings);
		fprintf(stderr, "  Number of time lag calculations: %d\n", nlag);
		fprintf(stderr, "  Minimum time lag:                %f\n", lagmin);
		fprintf(stderr, "  Maximum time lag:                %f\n", lagmax);
		fprintf(stderr, "  Time lag step:                   %f\n", lagstep);
		}
		
	/* first get roll data from the entire swathdata (which can be a datalist ) */
	sprintf(cmdfile, "mbnavlist -I%s -F%d -N%d -OMR", swathdata, format, navchannel);
	fprintf(stderr,"\nRunning %s...\n",cmdfile);
	fp = popen(cmdfile, "r");
	while ((nscan = fscanf(fp, "%lf %lf", &time_d, &roll)) == 2)
		{
		if (nroll >= nroll_alloc)
			{
			nroll_alloc += MBRTL_ALLOC_CHUNK;
			status = mb_reallocd(verbose,__FILE__,__LINE__, nroll_alloc * sizeof(double), (void **)&roll_time_d, &error);
			status = mb_reallocd(verbose,__FILE__,__LINE__, nroll_alloc * sizeof(double), (void **)&roll_roll, &error);
			}
		if (nroll == 0 || time_d > roll_time_d[nroll-1])
			{
			roll_time_d[nroll] = time_d;
			roll_roll[nroll] = roll;
			nroll++;
			}
		}
	pclose(fp);
	fprintf(stderr,"%d roll data read from %s\n", nroll, swathdata);
	
	/* open total cross correlation file */
	if (read_datalist == MB_YES)
		{
		sprintf(xcorfiletot, "%s_xcorr.txt", outroot);
		if ((fpt = fopen(xcorfiletot, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open cross correlation output: %s\n",
				xcorfiletot);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		}
	
	/* open time lag histogram file */
	sprintf(histfile, "%s_timelaghist.txt", outroot);
	if ((fph = fopen(histfile, "w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open histogram output: %s\n",
			histfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	
	/* open time lag model file */
	sprintf(modelfile, "%s_timelagmodel.txt", outroot);
	if ((fpm = fopen(modelfile, "w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open time lag model output: %s\n",
			modelfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    swathdata,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			swathdata);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    swathfile,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(swathfile, swathdata);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
		{
		nslope = 0;
		time_d_avg = 0.0;
		sprintf(cmdfile, "mblist -I%s -F%d -OMAR", swathfile, format);
		fprintf(stderr,"\nRunning %s...\n",cmdfile);
		fp = popen(cmdfile, "r");
		while ((nscan = fscanf(fp, "%lf %lf %lf", &time_d, &slope, &roll)) == 3)
			{
			if (nslope >= nslope_alloc)
				{
				nslope_alloc += MBRTL_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__, nslope_alloc * sizeof(double), (void **)&slope_time_d, &error);
				status = mb_reallocd(verbose,__FILE__,__LINE__, nslope_alloc * sizeof(double), (void **)&slope_slope, &error);
				status = mb_reallocd(verbose,__FILE__,__LINE__, nslope_alloc * sizeof(double), (void **)&slope_roll, &error);
				}
			if (nslope == 0 || time_d > slope_time_d[nslope-1])
				{
				slope_time_d[nslope] = time_d;
				time_d_avg += time_d;
				slope_slope[nslope] = roll - slope;
				slope_roll[nslope] = roll;
				nslope++;
				}
			}
		pclose(fp);
		nslopetot += nslope;
		if (nslope > 0)
			time_d_avg /= nslope;
		fprintf(stderr,"%d slope data read from %s\n", nslope, swathfile);
	
		/* open time lag histogram file */
		sprintf(fhistfile, "%s_timelaghist.txt", swathfile);
		if ((fpf = fopen(fhistfile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open histogram output: %s\n",
				fhistfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
	
		/* open cross correlation file */
		sprintf(xcorfile, "%s_xcorr.txt", swathfile);
		if ((fpx = fopen(xcorfile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open cross correlation output: %s\n",
				xcorfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		/* initialize time lag histogram */
		for (k=0;k<nlag;k++)
			{
			timelaghistogram[k] = 0;
			}
		
		/* now do cross correlation calculations */
		for (i=0;i<nslope/npings;i++)
			{
			/* get ping range in this chunk */
			j0 = i * npings;
			j1 = j0 + npings - 1;
	
			/* get mean slope in this chunk */
			slopemean = 0.0;
			for (j = j0; j <= j1; j++)
				{
				slopemean += slope_slope[j];
				}
			slopemean /= npings;
	
			/* get mean roll in this chunk */
			rollmean = 0.0;
			nrollmean = 0;
			for (j = 0; j < nroll; j++)
				{
				if ((roll_time_d[j] >= slope_time_d[j0] - lagmax)
					&& (roll_time_d[j] <= slope_time_d[j1] + lagmax))
					{
					rollmean += roll_roll[j];
					nrollmean++;
					}
				}
			if (nrollmean > 0)
				{
				rollmean /= nrollmean;
				}
	
			/* calculate cross correlation for the specified time lags */
			if (nrollmean > 0)
				{
				fprintf(fpx, ">\n");
				if (fpt != NULL)
					fprintf(fpt, ">\n");
				for (k = 0; k < nlag; k++)
					{
					timelag = -lagmax + k * lagstep;
					sumsloperoll = 0.0;
					sumslopesq = 0.0;
					sumrollsq = 0.0;
					nr = 0;

					for (j = j0; j <= j1; j++)
						{
						/* interpolate lagged roll value */
						found = MB_NO;
						time_d = slope_time_d[j] + timelag;
						for (l = nr; l < nroll - 1 && found == MB_NO; l++)
							{
							if (time_d >= roll_time_d[l] 
								&& time_d <= roll_time_d[l+1])
								{
								nr = l;
								found = MB_YES;
								}
							}
						if (found == MB_NO && time_d < roll_time_d[0])
							{
							rollint = roll_roll[0];
							}
						else if (found == MB_NO && time_d > roll_time_d[nroll - 1])
							{
							rollint = roll_roll[nroll - 1];
							}
						else
							{
							rollint = roll_roll[nr] + (roll_roll[nr+1] - roll_roll[nr])
										* (time_d - roll_time_d[nr])
										/ (roll_time_d[nr+1] - roll_time_d[nr]);
							}

						/* add to sums */
						slopeminusmean = (slope_slope[j] - slopemean);
						rollminusmean = (rollint - rollmean);
						sumslopesq += slopeminusmean * slopeminusmean;
						sumrollsq += rollminusmean * rollminusmean;
						sumsloperoll += slopeminusmean * rollminusmean;
						}

					r = sumsloperoll / sqrt(sumslopesq) / sqrt(sumrollsq);
					rr[k] = r;
					
					/* output results */
					fprintf(fpx, "%5.3f %5.3f \n", timelag, r);
					if (fpt != NULL)
						fprintf(fpt, "%5.3f %5.3f \n", timelag, r);
					}

				/* get max and closest peak cross correlations */
				maxr = 0.0;
				peakr = 0.0;
				peaktimelag = 0.0;
				for (k = 0; k < nlag; k++)
					{
					timelag = -lagmax + k * lagstep;
					if (rr[k] > maxr)
						{
						maxr = rr[k];
						maxtimelag = timelag;
						}
					if (k == 0)
						{
						peakk = k;
						peakr = rr[k];
						peaktimelag = timelag;
						}
					else if (k < nlag - 1
						&& rr[k] > 0.0
						&& rr[k] > rr[k-1]
						&& rr[k] > rr[k+1]
						&& abs(timelag) < abs(peaktimelag))
						{
						peakk = k;
						peakr = rr[k];
						peaktimelag = timelag;
						}
					else if (k == nlag - 1
						&& peaktimelag == -lagmax
						&& rr[k] > peakr)
						{
						peakk = k;
						peakr = rr[k];
						peaktimelag = timelag;
						}
					}
				}

			/* print out best correlated time lag estimates */
			if (peakr > 0.90)
				{
				timelaghistogram[peakk]++;
				}

			/* augment histogram */
			if (peakr > 0.90)
				{
				fprintf(fpf, "%6.3f\n", peaktimelag);
				fprintf(fph, "%6.3f\n", peaktimelag);
				}

			/* print out max and closest peak cross correlations */
			if (verbose > 0)
				{
				fprintf(stderr, "cross correlation pings %5d - %5d: max: %6.3f %5.3f  peak: %6.3f %5.3f\n",
				j0, j1, maxtimelag, maxr, peaktimelag, peakr);
				}
			}
			
		/* close cross correlation and histogram files */
		fclose(fpx);
		fclose(fpf);
		
		/* generate plot shellscript for cross correlation file */
		sprintf(cmdfile, "mbm_xyplot -I%s -N", xcorfile);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		system(cmdfile);

		/* generate plot shellscript for time lag histogram */
		sprintf(cmdfile, "mbm_histplot -I%s -C%g -L\"Frequency Histogram of %s:Time Lag (sec):Frequency:\"", 
				fhistfile, lagstep, swathfile);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		system(cmdfile);
		
		/* output peak time lag */
		peakk = 0;
		peakkmax = 0;
		peakksum = 0;
		for (k=0;k<nlag;k++)
			{
			if (timelaghistogram[k] > peakkmax)
				{
				peakkmax = timelaghistogram[k];
				peakk = k;
				}
			peakksum += timelaghistogram[k];
			}
		if (nslope > 0 && peakksum > 0 && peakkmax > 1
			&& peakkmax > peakksum / 5)
			{
			timelag = -lagmax + peakk * lagstep;
			fprintf(fpm, "%f %f\n", time_d_avg, timelag);
			nmodel++;
			fprintf(stderr,"Time lag model point: %f %f | nslope:%d peakksum:%d peakkmax:%d\n",
				time_d_avg, timelag, nslope, peakksum, peakkmax);
			}
		else
			fprintf(stderr,"Time lag model point: %f %f | nslope:%d peakksum:%d peakkmax:%d | REJECTED\n",
				time_d_avg, timelag, nslope, peakksum, peakkmax);

		/* figure out whether and what to read next */
        	if (read_datalist == MB_YES)
                	{
			if ((status = mb_datalist_read(verbose,datalist,
				    swathfile,&format,&file_weight,&error))
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
			
	/* close cross correlation file */
	if (read_datalist == MB_YES)
		fclose(fpt);
			
	/* close histogram file */
	fclose(fph);
			
	/* close time lag model file */
	fclose(fpm);
		
	/* generate plot shellscript for cross correlation file */
	if (read_datalist == MB_YES)
		{
		sprintf(cmdfile, "mbm_xyplot -I%s -N", xcorfiletot);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		system(cmdfile);
		}
		
	/* generate plot shellscript for time lag histogram */
	sprintf(cmdfile, "mbm_histplot -I%s -C%g -L\"Frequency Histogram of %s:Time Lag (sec):Frequency:\"", 
			histfile, lagstep, swathdata);
	fprintf(stderr, "Running: %s...\n", cmdfile);
	system(cmdfile);
		
	/* generate plot shellscript for time lag model if it exists */
	if (nmodel > 1)
		{
		sprintf(cmdfile, "mbm_xyplot -I%s -ISc0.1:%s -L\"Time lag model of %s:Time (sec):Time Lag (sec):\"", 
				modelfile, modelfile, swathdata);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		system(cmdfile);
		}

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&slope_time_d,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&slope_slope,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&slope_roll,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&roll_time_d,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&roll_roll,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&rr,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&timelaghistogram,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input roll records\n", nroll);
		fprintf(stderr,"%d input slope\n", nslopetot);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
