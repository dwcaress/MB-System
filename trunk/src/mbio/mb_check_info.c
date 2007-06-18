/*--------------------------------------------------------------------
 *    The MB-system:	mb_check_info.c	1/25/93
 *    $Id: mb_check_info.c,v 5.18 2007-06-18 01:19:48 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003, 2006 by
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
 * mb_check_info.c checks if a file has data within the specified
 * bounds by reading the mbinfo output of that file. The mbinfo
 * output must be in an ascii file with a name consisting of the
 * data file name followed by a ".inf" suffix. If the ".inf" file
 * does not exist then the file is assumed to have data within the
 * specified bounds. 
 *
 * Author:	D. W. Caress
 * Date:	September 3, 1996
 * 
 * $Log: not supported by cvs2svn $
 * Revision 5.17  2006/11/26 09:37:09  caress
 * Making distribution 5.1.0.
 *
 * Revision 5.16  2006/10/05 18:58:28  caress
 * Changes for 5.1.0beta4
 *
 * Revision 5.15  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.14  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.13  2004/12/02 06:33:30  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.12  2004/09/24 20:43:47  caress
 * Changed character array dimensions.
 *
 * Revision 5.11  2003/09/23 23:41:27  caress
 * Another change in adding formats 68 and 69.
 *
 * Revision 5.10  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.9  2003/01/15 20:54:46  caress
 * Release 5.0.beta28
 *
 * Revision 5.8  2002/09/20 17:45:43  caress
 * Release 5.0.beta23
 *
 * Revision 5.7  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.6  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.5  2002/04/08 20:59:38  caress
 * Release 5.0.beta17
 *
 * Revision 5.4  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.3  2002/03/18 18:28:46  caress
 * Fixed handling of data bounds that cross the current lonflip setting.
 *
 * Revision 5.2  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.1  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1996/12/08  04:14:22  caress
 * Fixed problem where lonflipping was applied to latitude.
 *
 * Revision 4.1  1996/12/08  04:14:22  caress
 * Fixed problem where lonflipping was applied to latitude.
 *
 * Revision 4.0  1996/09/05  13:59:47  caress
 * Initial revision.
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
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_format.h"
#include "../../include/mb_info.h"

/*--------------------------------------------------------------------*/
int mb_check_info(int verbose, char *file, int lonflip, 
		    double bounds[4], int *file_in_bounds,
		    int *error)
{
	static char rcs_id[]="$Id: mb_check_info.c,v 5.18 2007-06-18 01:19:48 caress Exp $";
	char	*function_name = "mb_check_info";
	int	status;
	char	file_inf[MB_PATH_MAXLINE];
	char	line[MB_PATH_MAXLINE];
	int	nrecords, nrecords_read;
	double	lon_min, lon_max;
	double	lat_min, lat_max;
	int	mask_nx, mask_ny;
	double	mask_dx, mask_dy;
	double	lonwest, loneast, latsouth, latnorth;
	int	*mask = NULL;
	char	*startptr, *endptr;
	FILE	*fp;
	char	*stdin_string = "stdin";
	int	nscan;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		}

	/* cannot check bounds if input is stdin */
	if (strncmp(file,stdin_string,5) == 0)
		{
		*file_in_bounds = MB_YES;
		
		/*print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"dbg4  Cannot check bounds if input is stdin...\n");
			}
		}
		
	/* check for inf file */
	else
		{
		/* get info file path */
		strcpy(file_inf, file);
		strcat(file_inf, ".inf");
		
		/* open if possible */
		if ((fp = fopen(file_inf,"r")) != NULL)
		    {
		    /* initialize the parameters */
		    nrecords = -1;
		    lon_min = 0.0;
		    lon_max = 0.0;
		    lat_min = 0.0;
		    lat_max = 0.0;
		    mask_nx = 0;
		    mask_ny = 0;
		    
		    /* read the inf file */
		    while (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			{
			if (strncmp(line, "Number of Records:", 18) == 0)
			    {
			    nscan = sscanf(line, "Number of Records: %d", 
					    &nrecords_read);
			    if (nscan == 1)
				nrecords = nrecords_read;
			    }
			else if (strncmp(line, "Minimum Longitude:", 18) == 0)
			    sscanf(line, "Minimum Longitude: %lf Maximum Longitude: %lf", 
				    &lon_min, &lon_max);
			else if (strncmp(line, "Minimum Latitude:", 17) == 0)
			    sscanf(line, "Minimum Latitude: %lf Maximum Latitude: %lf", 
				    &lat_min, &lat_max);
			else if (strncmp(line, "CM dimensions:", 14) == 0)
			    {
			    sscanf(line, "CM dimensions: %d %d", &mask_nx, &mask_ny);
			    status = mb_malloc(verbose,mask_nx*mask_ny*sizeof(int),
							&mask,error);
			    for (j=mask_ny-1;j>=0;j--)
				{
				if ((startptr = fgets(line, 128, fp)) != NULL)
				    {
				    startptr = &line[6];
				    for (i=0;i<mask_nx;i++)
					{
					k = i + j * mask_nx;
					mask[k] = strtol(startptr, &endptr, 0);
					startptr = endptr;
					}
				    }
				}
			    }
			}
			
		    /* check bounds if there is data */
		    if (nrecords > 0)
			{
			/* set lon lat min max according to lonflip */
			if (lonflip == -1 
			    && lon_min > 0.0)
			    {
			    lon_min -= 360.0;
			    lon_max -= 360.0;
			    }
			else if (lonflip == 0
			    && lon_max < -180.0)
			    {
			    lon_min += 360.0;
			    lon_max += 360.0;
			    }
			else if (lonflip == 0
			    && lon_min > 180.0)
			    {
			    lon_min -= 360.0;
			    lon_max -= 360.0;
			    }
			else if (lonflip == 1
			    && lon_max < 0.0)
			    {
			    lon_min += 360.0;
			    lon_max += 360.0;
			    }
			    
			/* check for lonflip conflict with bounds */
			if (lon_min > lon_max || lat_min > lat_max)
			    *file_in_bounds = MB_YES;
			    
			/* else check mask against desired input bounds */
			else if (mask_nx > 0 && mask_ny > 0)
			    {
			    *file_in_bounds = MB_NO;
			    mask_dx = (lon_max - lon_min) / mask_nx;
			    mask_dy = (lat_max - lat_min) / mask_ny;
			    for (i=0; i<mask_nx && *file_in_bounds == MB_NO; i++)
				for (j=0; j<mask_ny && *file_in_bounds == MB_NO; j++)
				    {
				    k = i + j * mask_nx;
				    lonwest = lon_min + i * mask_dx;
				    loneast = lonwest + mask_dx;
				    latsouth = lat_min + j * mask_dy;
				    latnorth = latsouth + mask_dy;
				    if (mask[k] == 1
					&& lonwest < bounds[1] && loneast > bounds[0]
					&& latsouth < bounds[3] && latnorth > bounds[2])
					*file_in_bounds = MB_YES;
				    }
			    mb_free(verbose, &mask, error);
			    }
			    
			/* else check whole file against desired input bounds */
			else
			    {
			    if (lon_min < bounds[1] && lon_max > bounds[0]
				&& lat_min < bounds[3] && lat_max > bounds[2])
				*file_in_bounds = MB_YES;
			    else
				*file_in_bounds = MB_NO;
			    }

                        /*print debug statements */
                        if (verbose >= 4)
			    {
                            fprintf(stderr,"dbg4  Bounds from inf file:\n");
                            fprintf(stderr,"dbg4      lon_min: %f\n", lon_min);
                            fprintf(stderr,"dbg4      lon_max: %f\n", lon_max);
                            fprintf(stderr,"dbg4      lat_min: %f\n", lat_min);
                            fprintf(stderr,"dbg4      lat_max: %f\n", lat_max);
			    }
			}
			
		    /* else if no data records in inf file 
			treat file as out of bounds */
		    else if (nrecords == 0)
			{
			*file_in_bounds = MB_NO;
      
             		/*print debug statements */
                    	if (verbose >= 4)
                        	fprintf(stderr,"dbg4  The inf file shows zero records so out of bounds...\n");
			}
			
		    /* else if no data assume inf file is botched so
			assume file has data in bounds */
		    else
			{
			*file_in_bounds = MB_YES;
      
             		/*print debug statements */
                    	if (verbose >= 4)
                        	fprintf(stderr,"dbg4  No data listed in inf file so cannot check bounds...\n");
			}
		    
		    /* close the file */
		    fclose(fp);
		    }
			
		/* if no inf file assume file has data in bounds */
		else
		    {
		    *file_in_bounds = MB_YES;

		    /*print debug statements */
		    if (verbose >= 4)
                        fprintf(stderr,"dbg4  Cannot open inf file so cannot check bounds...\n");
		    }	
		}

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       file_in_bounds: %d\n",*file_in_bounds);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_get_info(int verbose, char *file, struct mb_info_struct *mb_info, int lonflip,
		    int *error)
{
	static char rcs_id[]="$Id: mb_check_info.c,v 5.18 2007-06-18 01:19:48 caress Exp $";
	char	*function_name = "mb_get_info";
	int	status;
	char	file_inf[MB_PATH_MAXLINE];
	char	line[MB_PATH_MAXLINE];
	char	*startptr, *endptr;
	FILE	*fp;
	char	*stdin_string = "stdin";
	int	time_i[7];
	double	speedkts;
	int	nscan, nproblem, problemid;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       info:       %d\n",mb_info);
		fprintf(stderr,"dbg2       lonflip:    %d\n",lonflip);
		}

	/* initialize the parameters */
	mb_info->loaded = MB_NO;
	strcpy(mb_info->file, file);
	mb_info->nrecords = 0;
	mb_info->nrecords_ss1 = 0;
	mb_info->nrecords_ss2 = 0;
	mb_info->nrecords_sbp = 0;
	mb_info->nbeams_bath = 0;
	mb_info->nbeams_bath_total = 0;
	mb_info->nbeams_bath_good = 0;
	mb_info->nbeams_bath_zero = 0;
	mb_info->nbeams_bath_flagged = 0;
	mb_info->nbeams_amp = 0;
	mb_info->nbeams_amp_total = 0;
	mb_info->nbeams_amp_good = 0;
	mb_info->nbeams_amp_zero = 0;
	mb_info->nbeams_amp_flagged = 0;
	mb_info->npixels_ss = 0;
	mb_info->npixels_ss_total = 0;
	mb_info->npixels_ss_good = 0;
	mb_info->npixels_ss_zero = 0;
	mb_info->npixels_ss_flagged = 0;
	mb_info->time_total = 0.0;
	mb_info->dist_total = 0.0;
	mb_info->speed_avg = 0.0;
	mb_info->time_start = 0.0;
	mb_info->lon_start = 0.0;
	mb_info->lat_start = 0.0;
	mb_info->depth_start = 0.0;
	mb_info->heading_start = 0.0;
	mb_info->speed_start = 0.0;
	mb_info->sonardepth_start = 0.0;
	mb_info->sonaraltitude_start = 0.0;
	mb_info->time_end = 0.0;
	mb_info->lon_end = 0.0;
	mb_info->lat_end = 0.0;
	mb_info->depth_end = 0.0;
	mb_info->heading_end = 0.0;
	mb_info->speed_end = 0.0;
	mb_info->sonardepth_end = 0.0;
	mb_info->sonaraltitude_end = 0.0;
	mb_info->lon_min = 0.0;
	mb_info->lon_max = 0.0;
	mb_info->lat_min = 0.0;
	mb_info->lat_max = 0.0;
	mb_info->sonardepth_min = 0.0;
	mb_info->sonardepth_max = 0.0;
	mb_info->altitude_min = 0.0;
	mb_info->altitude_max = 0.0;
	mb_info->depth_min = 0.0;
	mb_info->depth_max = 0.0;
	mb_info->amp_min = 0.0;
	mb_info->amp_max = 0.0;
	mb_info->ss_min = 0.0;
	mb_info->ss_max = 0.0;
	mb_info->problem_nodata = 0;
	mb_info->problem_zeronav = 0;
	mb_info->problem_toofast = 0;
	mb_info->problem_avgtoofast = 0;
	mb_info->problem_toodeep = 0;
	mb_info->problem_baddatagram = 0;
	mb_info->mask_nx = 0;
	mb_info->mask_ny = 0;
	mb_info->mask_dx = 0.0;
	mb_info->mask_dy = 0.0;

	/* get info file path */
	strcpy(file_inf, file);
	strcat(file_inf, ".inf");

	/* open if possible */
	if ((fp = fopen(file_inf,"r")) == NULL)
		{
		/* set error */
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;

		/*print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"dbg2  Cannot open requested inf file: %s\n", file_inf);
			}
			
		/* return in disgrace */
		return(status);
		}
		
	/* load information from inf file */
	else
		{
		/* read the inf file */
		while (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			{
			if (strncmp(line, "Number of Records:", 18) == 0)
			    {
			    nscan = sscanf(line, "Number of Records: %d", 
					    &mb_info->nrecords);
			    }
			else if (strncmp(line, "Number of Subbottom Records:", 28) == 0)
			    {
			    nscan = sscanf(line, "Number of Subbottom Records: %d", 
					    &mb_info->nrecords_sbp);
			    }
			else if (strncmp(line, "Number of Secondary Sidescan Records:", 37) == 0)
			    {
			    nscan = sscanf(line, "Number of Secondary Sidescan Records: %d", 
					    &mb_info->nrecords_ss1);
			    }
			else if (strncmp(line, "Number of Tertiary Sidescan Records:", 36) == 0)
			    {
			    nscan = sscanf(line, "Number of Tertiary Sidescan Records: %d", 
					    &mb_info->nrecords_ss2);
			    }

			else if (strncmp(line, "Bathymetry Data (", 17) == 0)
			    {
			    nscan = sscanf(line, "Bathymetry Data (%d beams):", 
					    &mb_info->nbeams_bath);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Beams: %d", 
					    &mb_info->nbeams_bath_total);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Good Beams: %d", 
					    &mb_info->nbeams_bath_good);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Zero Beams: %d", 
					    &mb_info->nbeams_bath_zero);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Flagged Beams: %d", 
					    &mb_info->nbeams_bath_flagged);
			    }

			else if (strncmp(line, "Amplitude Data (", 17) == 0)
			    {
			    nscan = sscanf(line, "Amplitude Data (%d beams):", 
					    &mb_info->nbeams_amp);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Beams: %d", 
					    &mb_info->nbeams_amp_total);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Good Beams: %d", 
					    &mb_info->nbeams_amp_good);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Zero Beams: %d", 
					    &mb_info->nbeams_amp_zero);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Flagged Beams: %d", 
					    &mb_info->nbeams_amp_flagged);
			    }

			else if (strncmp(line, "Sidescan Data (", 15) == 0)
			    {
			    nscan = sscanf(line, "Sidescan Data (%d pixels):", 
					    &mb_info->npixels_ss);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Pixels: %d", 
					    &mb_info->npixels_ss_total);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Good Pixels: %d", 
					    &mb_info->npixels_ss_good);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Zero Pixels: %d", 
					    &mb_info->npixels_ss_zero);
			    if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			    nscan = sscanf(line, "  Number of Flagged Pixels: %d", 
					    &mb_info->npixels_ss_flagged);
			    }

			else if (strncmp(line, "Total Time:", 11) == 0)
			    {
			    nscan = sscanf(line, "Total Time: %lf hours", 
					    &mb_info->time_total);
			    }
			else if (strncmp(line, "Total Track Length:", 19) == 0)
			    {
			    nscan = sscanf(line, "Total Track Length: %lf km", 
					    &mb_info->dist_total);
			    }
			else if (strncmp(line, "Average Speed:", 14) == 0)
			    {
			    nscan = sscanf(line, "Average Speed: %lf km/hr", 
					    &mb_info->speed_avg);
			    }

			else if (strncmp(line, "Start of Data:", 14) == 0)
			    {
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Time:  %d %d %d %d:%d:%d.%d  JD", 
					    &time_i[1], &time_i[2], &time_i[0], 
					    &time_i[4], &time_i[5], &time_i[6], &time_i[7]);
				if (nscan == 7)
					mb_get_time(verbose, time_i, &(mb_info->time_start));
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Lon: %lf	 Lat: %lf Depth: %lf meters", 
					    &mb_info->lon_start, &mb_info->lat_start, &mb_info->depth_start);
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Speed: %lf km/hr ( %lf knots)  Heading: %lf degrees", 
					    &mb_info->speed_start, &speedkts, &mb_info->heading_start);
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Sonar Depth:  %lf m  Sonar Altitude:   %lf m", 
					    &mb_info->sonardepth_start, &mb_info->sonaraltitude_start);
				}
			    }

			else if (strncmp(line, "End of Data:", 12) == 0)
			    {
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Time:  %d %d %d %d:%d:%d.%d  JD", 
					    &time_i[1], &time_i[2], &time_i[0], 
					    &time_i[4], &time_i[5], &time_i[6], &time_i[7]);
				if (nscan == 7)
					mb_get_time(verbose, time_i, &(mb_info->time_end));
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Lon: %lf	 Lat: %lf Depth: %lf meters", 
					    &mb_info->lon_end, &mb_info->lat_end, &mb_info->depth_end);
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Speed: %lf km/hr ( %lf knots)  Heading: %lf degrees", 
					    &mb_info->speed_end, &speedkts, &mb_info->heading_end);
				}
			    if ((startptr = fgets(line, 128, fp)) != NULL)
				{
			    	nscan = sscanf(line, "Sonar Depth:  %lf m  Sonar Altitude:   %lf m", 
					    &mb_info->sonardepth_end, &mb_info->sonaraltitude_end);
				}
			    }

			else if (strncmp(line, "Minimum Longitude:", 18) == 0)
			    sscanf(line, "Minimum Longitude: %lf Maximum Longitude: %lf", 
				    &mb_info->lon_min, &mb_info->lon_max);
			else if (strncmp(line, "Minimum Latitude:", 17) == 0)
			    sscanf(line, "Minimum Latitude: %lf Maximum Latitude: %lf", 
				    &mb_info->lat_min, &mb_info->lat_max);

			else if (strncmp(line, "Minimum Sonar Depth:", 20) == 0)
			    sscanf(line, "Minimum Sonar Depth: %lf Maximum Sonar Depth: %lf", 
				    &mb_info->sonardepth_min, &mb_info->sonardepth_max);

			else if (strncmp(line, "Minimum Altitude:", 17) == 0)
			    sscanf(line, "Minimum Altitude: %lf Maximum Altitude: %lf", 
				    &mb_info->altitude_min, &mb_info->altitude_max);

			else if (strncmp(line, "Minimum Depth:", 14) == 0)
			    sscanf(line, "Minimum Depth: %lf Maximum Depth: %lf", 
				    &mb_info->depth_min, &mb_info->depth_max);

			else if (strncmp(line, "Minimum Amplitude:", 18) == 0)
			    sscanf(line, "Minimum Amplitude: %lf Maximum Amplitude: %lf", 
				    &mb_info->amp_min, &mb_info->amp_max);

			else if (strncmp(line, "Minimum Sidescan:", 17) == 0)
			    sscanf(line, "Minimum Sidescan: %lf Maximum Sidescan: %lf", 
				    &mb_info->ss_min, &mb_info->ss_max);

			else if (strncmp(line, "PN:", 3) == 0)
			    {
			    sscanf(line, "PN: %d DATA PROBLEM (ID=%d):", 
				    &nproblem,&problemid);
			    if (problemid == MB_PROBLEM_NO_DATA)
				mb_info->problem_nodata += nproblem;
			    else if (problemid == MB_PROBLEM_ZERO_NAV)
				mb_info->problem_zeronav += nproblem;
			    else if (problemid == MB_PROBLEM_TOO_FAST)
			 	mb_info->problem_toofast += nproblem;
			    else if (problemid == MB_PROBLEM_AVG_TOO_FAST)
				mb_info->problem_avgtoofast += nproblem;
			    else if (problemid == MB_PROBLEM_TOO_DEEP)
				mb_info->problem_toodeep += nproblem;
			    else if (problemid == MB_PROBLEM_BAD_DATAGRAM)
				mb_info->problem_baddatagram += nproblem;
			    }
			    
			else if (strncmp(line, "CM dimensions:", 14) == 0)
			    {
			    sscanf(line, "CM dimensions: %d %d", &mb_info->mask_nx, &mb_info->mask_ny);
			    status = mb_malloc(verbose,mb_info->mask_nx*mb_info->mask_ny*sizeof(int),
							&mb_info->mask,error);
			    for (j=mb_info->mask_ny-1;j>=0;j--)
				{
				if ((startptr = fgets(line, 128, fp)) != NULL)
				    {
				    startptr = &line[6];
				    for (i=0;i<mb_info->mask_nx;i++)
					{
					k = i + j * mb_info->mask_nx;
					mb_info->mask[k] = strtol(startptr, &endptr, 0);
					startptr = endptr;
					}
				    }
				}
			    }
			}
		    
		/* close the file */
		fclose(fp);
		}

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       loaded:                 %d\n",mb_info->loaded);
		fprintf(stderr,"dbg2       file:                   %s\n",mb_info->file);
		fprintf(stderr,"dbg2       nrecords:                 %d\n",mb_info->nrecords);
		fprintf(stderr,"dbg2       nrecords_sbp:             %d\n",mb_info->nrecords_sbp);
		fprintf(stderr,"dbg2       nrecords_ss1:             %d\n",mb_info->nrecords_ss1);
		fprintf(stderr,"dbg2       nrecords_ss2:             %d\n",mb_info->nrecords_ss2);
		fprintf(stderr,"dbg2       nbeams_bath:              %d\n",mb_info->nbeams_bath);
		fprintf(stderr,"dbg2       nbeams_bath_total:        %d\n",mb_info->nbeams_bath_total);
		fprintf(stderr,"dbg2       nbeams_bath_good:         %d\n",mb_info->nbeams_bath_good);
		fprintf(stderr,"dbg2       nbeams_bath_zero:         %d\n",mb_info->nbeams_bath_zero);
		fprintf(stderr,"dbg2       nbeams_bath_flagged:      %d\n",mb_info->nbeams_bath_flagged);
		fprintf(stderr,"dbg2       nbeams_amp:               %d\n",mb_info->nbeams_amp);
		fprintf(stderr,"dbg2       nbeams_amp_total:         %d\n",mb_info->nbeams_amp_total);
		fprintf(stderr,"dbg2       nbeams_amp_good:          %d\n",mb_info->nbeams_amp_good);
		fprintf(stderr,"dbg2       nbeams_amp_zero:          %d\n",mb_info->nbeams_amp_zero);
		fprintf(stderr,"dbg2       nbeams_amp_flagged:       %d\n",mb_info->nbeams_amp_flagged);
		fprintf(stderr,"dbg2       npixels_ss:               %d\n",mb_info->npixels_ss);
		fprintf(stderr,"dbg2       npixels_ss_total:         %d\n",mb_info->npixels_ss_total);
		fprintf(stderr,"dbg2       npixels_ss_good:          %d\n",mb_info->npixels_ss_good);
		fprintf(stderr,"dbg2       npixels_ss_zero:          %d\n",mb_info->npixels_ss_zero);
		fprintf(stderr,"dbg2       npixels_ss_flagged:       %d\n",mb_info->npixels_ss_flagged);
		fprintf(stderr,"dbg2       time_total:               %f\n",mb_info->time_total);
		fprintf(stderr,"dbg2       dist_total:               %f\n",mb_info->dist_total);
		fprintf(stderr,"dbg2       speed_avg:                %f\n",mb_info->speed_avg);
		fprintf(stderr,"dbg2       time_start:               %f\n",mb_info->time_start);
		fprintf(stderr,"dbg2       lon_start:                %f\n",mb_info->lon_start);
		fprintf(stderr,"dbg2       lat_start:                %f\n",mb_info->lat_start);
		fprintf(stderr,"dbg2       depth_start:              %f\n",mb_info->depth_start);
		fprintf(stderr,"dbg2       heading_start:            %f\n",mb_info->heading_start);
		fprintf(stderr,"dbg2       speed_start:              %f\n",mb_info->speed_start);
		fprintf(stderr,"dbg2       sonardepth_start:         %f\n",mb_info->sonardepth_start);
		fprintf(stderr,"dbg2       sonaraltitude_start:      %f\n",mb_info->sonaraltitude_start);
		fprintf(stderr,"dbg2       time_end:                 %f\n",mb_info->time_end);
		fprintf(stderr,"dbg2       lon_end:                  %f\n",mb_info->lon_end);
		fprintf(stderr,"dbg2       lat_end:                  %f\n",mb_info->lat_end);
		fprintf(stderr,"dbg2       depth_end:                %f\n",mb_info->depth_end);
		fprintf(stderr,"dbg2       heading_end:              %f\n",mb_info->heading_end);
		fprintf(stderr,"dbg2       speed_end:                %f\n",mb_info->speed_end);
		fprintf(stderr,"dbg2       sonardepth_end:           %f\n",mb_info->sonardepth_end);
		fprintf(stderr,"dbg2       sonaraltitude_end:        %f\n",mb_info->sonaraltitude_end);
		fprintf(stderr,"dbg2       lon_min:                  %f\n",mb_info->lon_min);
		fprintf(stderr,"dbg2       lon_max:                  %f\n",mb_info->lon_max);
		fprintf(stderr,"dbg2       lat_min:                  %f\n",mb_info->lat_min);
		fprintf(stderr,"dbg2       lat_max:                  %f\n",mb_info->lat_max);
		fprintf(stderr,"dbg2       sonardepth_min:           %f\n",mb_info->sonardepth_min);
		fprintf(stderr,"dbg2       sonardepth_max:           %f\n",mb_info->sonardepth_max);
		fprintf(stderr,"dbg2       altitude_min:             %f\n",mb_info->altitude_min);
		fprintf(stderr,"dbg2       altitude_max:             %f\n",mb_info->altitude_max);
		fprintf(stderr,"dbg2       depth_min:                %f\n",mb_info->depth_min);
		fprintf(stderr,"dbg2       depth_max:                %f\n",mb_info->depth_max);
		fprintf(stderr,"dbg2       amp_min:                  %f\n",mb_info->amp_min);
		fprintf(stderr,"dbg2       amp_max:                  %f\n",mb_info->amp_max);
		fprintf(stderr,"dbg2       ss_min:                   %f\n",mb_info->ss_min);
		fprintf(stderr,"dbg2       ss_max:                   %f\n",mb_info->ss_max);
		fprintf(stderr,"dbg2       problem_nodata:           %d\n",mb_info->problem_nodata);
		fprintf(stderr,"dbg2       problem_zeronav:          %d\n",mb_info->problem_zeronav);
		fprintf(stderr,"dbg2       problem_toofast:          %d\n",mb_info->problem_toofast);
		fprintf(stderr,"dbg2       problem_avgtoofast:       %d\n",mb_info->problem_avgtoofast);
		fprintf(stderr,"dbg2       problem_toodeep:          %d\n",mb_info->problem_toodeep);
		fprintf(stderr,"dbg2       problem_baddatagram:      %d\n",mb_info->problem_baddatagram);
		fprintf(stderr,"dbg2       mask_nx:                  %d\n",mb_info->mask_nx);
		fprintf(stderr,"dbg2       mask_ny:                  %d\n",mb_info->mask_ny);
		fprintf(stderr,"dbg2       mask_dx:                  %f\n",mb_info->mask_dx);
		fprintf(stderr,"dbg2       mask_dy:                  %f\n",mb_info->mask_dy);
		fprintf(stderr,"dbg2       mask:\n");
		for (j=mb_info->mask_ny-1;j>=0;j--)
			{
			fprintf(stderr, "dbg2       ");
			for (i=0;i<mb_info->mask_nx;i++)
				{
				k = i + j * mb_info->mask_nx;
				fprintf(stderr, " %1d", mb_info->mask[k]);
				}
			fprintf(stderr, "\n");
			}
		fprintf(stderr,"dbg2       error:                    %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_make_info(int verbose, int force,
		    char *file, int format, int *error)
{
	char	*function_name = "mb_make_info";
	int	status = MB_SUCCESS;
	char	inffile[MB_PATH_MAXLINE];
	char	fbtfile[MB_PATH_MAXLINE];
	char	fnvfile[MB_PATH_MAXLINE];
	char	command[MB_PATH_MAXLINE];
	int	datmodtime = 0;
	int	infmodtime = 0;
	int	fbtmodtime = 0;
	int	fnvmodtime = 0;
	struct stat file_status;
	int	fstat;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       force:      %d\n",force);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		}
		
	/* check for existing ancillary files */
	sprintf(inffile, "%s.inf", file);
	sprintf(fbtfile, "%s.fbt", file);
	sprintf(fnvfile, "%s.fnv", file);
	if ((fstat = stat(file, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		datmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(inffile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		infmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(fbtfile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		fbtmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(fnvfile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		fnvmodtime = file_status.st_mtime;
		}
		
	/* make new inf file if not there or out of date */
	if (force == MB_YES
		|| (datmodtime > 0 && datmodtime > infmodtime))
		{
		if (verbose >= 1)
			fprintf(stderr,"\nGenerating inf file for %s\n",file);
		sprintf(command, "mbinfo -F %d -I %s -G -N -O -M10/10", 
			format, file);
		system(command);
		}
		
	/* make new fbt file if not there or out of date */
	if ((force 
		|| (datmodtime > 0 
	    	&& datmodtime > fbtmodtime))
	    && format != MBF_SBSIOMRG
	    && format != MBF_SBSIOCEN
	    && format != MBF_SBSIOLSI
	    && format != MBF_SBURICEN
	    && format != MBF_SBURIVAX
	    && format != MBF_SBSIOSWB
	    && format != MBF_HSLDEDMB
	    && format != MBF_HSURICEN
	    && format != MBF_HSURIVAX
	    && format != MBF_SB2000SS
	    && format != MBF_SB2000SB
	    && format != MBF_MSTIFFSS
	    && format != MBF_MBLDEOIH
	    && format != MBF_MBNETCDF
	    && format != MBF_ASCIIXYZ
	    && format != MBF_ASCIIYXZ
	    && format != MBF_ASCIIXYT
	    && format != MBF_ASCIIYXT
	    && format != MBF_HYDROB93
	    && format != MBF_SEGYSEGY
	    && format != MBF_MGD77DAT
	    && format != MBF_MBARIROV
	    && format != MBF_MBARROV2
	    && format != MBF_MBPRONAV)
		{
		if (verbose >= 1)
			fprintf(stderr,"Generating fbt file for %s\n",file);
		sprintf(command, "mbcopy -F %d/71 -I %s -D -O %s.fbt", 
			format, file, file);
		system(command);
		}
		
	/* make new fnv file if not there or out of date */
	if ((force
		|| (datmodtime > 0 
	    		&& datmodtime > fnvmodtime))
	    && format != MBF_ASCIIXYZ
	    && format != MBF_ASCIIYXZ
	    && format != MBF_ASCIIXYT
	    && format != MBF_ASCIIYXT
	    && format != MBF_HYDROB93
	    && format != MBF_SEGYSEGY
	    && format != MBF_MGD77DAT
	    && format != MBF_MBARIROV
	    && format != MBF_MBARROV2
	    && format != MBF_NVNETCDF
	    && format != MBF_MBPRONAV)
		{
		if (verbose >= 1)
			fprintf(stderr,"Generating fnv file for %s\n",file);
		sprintf(command, "mblist -F %d -I %s -O tMXYHScRPr=X=Y+X+Y > %s.fnv", 
			format, file, file);
		system(command);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_get_fbt(int verbose, 
		    char *file, int *format, int *error)
{
	char	*function_name = "mb_get_fbt";
	int	status = MB_SUCCESS;
	char	fbtfile[MB_PATH_MAXLINE];
	int	datmodtime = 0;
	int	fbtmodtime = 0;
	struct stat file_status;
	int	fstat;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}
		
	/* check for existing fbt file */
	sprintf(fbtfile, "%s.fbt", file);
	if ((fstat = stat(file, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		datmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(fbtfile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		fbtmodtime = file_status.st_mtime;
		}
		
	/* replace file with fbt file if fbt file exists */
	if (datmodtime > 0 && fbtmodtime > 0)
	    {
	    strcpy(file, fbtfile);
	    *format = 71;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_get_fnv(int verbose, 
		    char *file, int *format, int *error)
{
	char	*function_name = "mb_get_fnv";
	int	status = MB_SUCCESS;
	char	fnvfile[MB_PATH_MAXLINE];
	int	datmodtime = 0;
	int	fnvmodtime = 0;
	struct stat file_status;
	int	fstat;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		}
		
	/* check for existing fnv file */
	sprintf(fnvfile, "%s.fnv", file);
	if ((fstat = stat(file, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		datmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(fnvfile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		fnvmodtime = file_status.st_mtime;
		}
		
	/* replace file with fnv file if fnv file exists */
	if (datmodtime > 0 && fnvmodtime > 0)
	    {
	    strcpy(file, fnvfile);
	    *format = 166;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_swathbounds(int verbose, int checkgood,
			double navlon, double navlat, double heading, 
			int nbath, int nss,
			char *beamflag, double *bath, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			int *ibeamport,
			int *ibeamcntr,
			int *ibeamstbd,
			int *ipixelport,
			int *ipixelcntr,
			int *ipixelstbd,
			int *error)
{
	char	*function_name = "mb_swathbounds";
	int	status = MB_SUCCESS;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	xtrackmin, xtrackmax, distmin;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       checkgood:     %d\n",checkgood);
		fprintf(stderr,"dbg2       longitude:     %f\n",navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",navlat);
		fprintf(stderr,"dbg2       heading:       %f\n",heading);
		fprintf(stderr,"dbg2       nbath:         %d\n",nbath);
		if (verbose >= 3 && nbath > 0)
		  {
		  fprintf(stderr,"dbg3       beam   flag  bath  crosstrack alongtrack\n");
		  for (i=0;i<nbath;i++)
		    fprintf(stderr,"dbg3       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       nss:      %d\n",nss);
		if (verbose >= 3 && nss > 0)
		  {
		  fprintf(stderr,"dbg3       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<nss;i++)
		    fprintf(stderr,"dbg3       %4d   %f    %f     %f\n",
			i,ss[i],
			ssacrosstrack[i],ssalongtrack[i]);
		  }
		}
		
	/* get coordinate scaling */
	mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
	headingx = sin(heading * DTR);
	headingy = cos(heading * DTR);
	
	/* set starting values */
	*ibeamport = 0;
	*ibeamcntr = 0;
	*ibeamstbd = 0;
	*ipixelport = 0;
	*ipixelcntr = 0;
	*ipixelstbd = 0;
	
	/* get min max of non-null beams */
	xtrackmin = 0.0;
	xtrackmax = 0.0;
	distmin = 0.0;
	found = MB_NO;
	for (i=0;i<nbath;i++)
		{
		if ((checkgood && mb_beam_ok(beamflag[i])) || beamflag[i] != MB_FLAG_NULL)
			{
			if (found == MB_NO)
				{
				*ibeamport = i;
				*ibeamcntr = i;
				*ibeamstbd = i;
				xtrackmin = bathacrosstrack[i];
				distmin = fabs(bathacrosstrack[i]);
				xtrackmax = bathacrosstrack[i];
				found = MB_YES;
				}
			else 
				{
				if (fabs(bathacrosstrack[i]) < distmin)
					{
					*ibeamcntr = i;
					distmin = fabs(bathacrosstrack[i]);
					}
				if (bathacrosstrack[i] < xtrackmin)
					{
					*ibeamport = i;
					xtrackmin = bathacrosstrack[i];
					}
				else if (bathacrosstrack[i] > xtrackmax)
					{
					*ibeamstbd = i;
					xtrackmax = bathacrosstrack[i];
					}
				}
			}
		}
	
	/* get min max of non-null pixels */
	xtrackmin = 0.0;
	xtrackmax = 0.0;
	distmin = 0.0;
	found = MB_NO;
	for (i=0;i<nss;i++)
		{
		if (ss[i] > 0.0)
			{
			if (found == MB_NO)
				{
				*ipixelport = i;
				*ipixelcntr = i;
				*ipixelstbd = i;
				xtrackmin = ssacrosstrack[i];
				distmin = fabs(ssacrosstrack[i]);
				xtrackmax = ssacrosstrack[i];
				found = MB_YES;
				}
			else 
				{
				if (fabs(ssacrosstrack[i]) < distmin)
					{
					*ipixelcntr = i;
					distmin = fabs(ssacrosstrack[i]);
					}
				if (ssacrosstrack[i] < xtrackmin)
					{
					*ipixelport = i;
					xtrackmin = ssacrosstrack[i];
					}
				else if (ssacrosstrack[i] > xtrackmax)
					{
					*ipixelstbd = i;
					xtrackmax = ssacrosstrack[i];
					}
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ibeamport:     %d\n",*ibeamport);
		fprintf(stderr,"dbg2       ibeamport:     %d\n",*ibeamcntr);
		fprintf(stderr,"dbg2       ibeamstbd:     %d\n",*ibeamstbd);
		fprintf(stderr,"dbg2       ipixelport:    %d\n",*ipixelport);
		fprintf(stderr,"dbg2       ipixelport:    %d\n",*ipixelcntr);
		fprintf(stderr,"dbg2       ipixelstbd:    %d\n",*ipixelstbd);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
