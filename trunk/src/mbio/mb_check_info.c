/*--------------------------------------------------------------------
 *    The MB-system:	mb_check_info.c	1/25/93
 *    $Id: mb_check_info.c,v 5.9 2003-01-15 20:54:46 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002 by
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

/*--------------------------------------------------------------------*/
int mb_check_info(int verbose, char *file, int lonflip, 
		    double bounds[4], int *file_in_bounds,
		    int *error)
{
	static char rcs_id[]="$Id: mb_check_info.c,v 5.9 2003-01-15 20:54:46 caress Exp $";
	char	*function_name = "mb_check_info";
	int	status;
	char	file_inf[128];
	char	line[128];
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
		    while (fgets(line, 128, fp) != NULL)
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
                        	fprintf(stderr,"dbg4  No data listed in inf fileso cannot check bounds...\n");
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
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
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
		sprintf(command, "mbinfo -F %d -I %s -G -O -M10/10", 
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
	    && format != MBF_HYDROB93
	    && format != MBF_MGD77DAT
	    && format != MBF_MBARIROV
	    && format != MBF_MBPRONAV)
		{
		if (verbose >= 1)
			fprintf(stderr,"Generating fbt file for %s\n",file);
		sprintf(command, "mbcopy -F %d/71 -I %s -D -O %s.fbt", 
			format, file, file);
		system(command);
		}
		
	/* make new nv file if not there or out of date */
	if ((force
		|| (datmodtime > 0 
	    		&& datmodtime > fnvmodtime))
	    && format != MBF_ASCIIXYZ
	    && format != MBF_ASCIIYXZ
	    && format != MBF_HYDROB93
	    && format != MBF_MGD77DAT
	    && format != MBF_MBARIROV
	    && format != MBF_NVNETCDF
	    && format != MBF_MBPRONAV)
		{
		if (verbose >= 1)
			fprintf(stderr,"Generating fnv file for %s\n",file);
		sprintf(command, "mblist -F %d -I %s -O tMXYHSc > %s.fnv", 
			format, file, file);
		system(command);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
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
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
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
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
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
