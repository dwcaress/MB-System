/*--------------------------------------------------------------------
 *    The MB-system:	mb_check_info.c	1/25/93
 *    $Id: mb_check_info.c,v 4.0 1996-09-05 13:59:47 caress Exp $
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
 * mb_check_info.c opens and initializes a multibeam data file 
 * for reading with mb_read or mb_get.
 *
 * Author:	D. W. Caress
 * Date:	September 3, 1996
 * 
 * $Log: not supported by cvs2svn $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/
int mb_check_info(verbose,file,lonflip,bounds,file_in_bounds,error)
int	verbose;
char	*file;
int	lonflip;
double	bounds[4];
int	*file_in_bounds;
int	*error;
{
	static char rcs_id[]="$Id: mb_check_info.c,v 4.0 1996-09-05 13:59:47 caress Exp $";
	char	*function_name = "mb_check_info";
	int	status;
	char	file_inf[128];
	char	line[128];
	int	nrecords;
	double	lon_min, lon_max;
	double	lat_min, lat_max;
	FILE	*fp;
	char	*stdin_string = "stdin";

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
		*file_in_bounds = MB_YES;
		
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
		    nrecords = 0;
		    lon_min = 0.0;
		    lon_max = 0.0;
		    lat_min = 0.0;
		    lat_max = 0.0;
		    
		    /* read the inf file */
		    while (fgets(line, 128, fp) != NULL)
			{
			if (strncmp(line, "Number of Records:", 18) == 0)
			    sscanf(line, "Number of Records: %d", 
				    &nrecords);
			else if (strncmp(line, "Minimum Longitude:", 18) == 0)
			    sscanf(line, "Minimum Longitude: %lf Maximum Longitude: %lf", 
				    &lon_min, &lon_max);
			else if (strncmp(line, "Minimum Latitude:", 17) == 0)
			    sscanf(line, "Minimum Latitude: %lf Maximum Latitude: %lf", 
				    &lat_min, &lat_max);
			}
			
		    /* check bounds if there is data */
		    if (nrecords > 0)
			{
			/* set lon lat min max according to lonflip */
			if (lonflip == -1 
			    && lon_min > 0.0)
			    lon_min -= 360.0;
			else if (lonflip == 0
			    && lon_min < -180.0)
			    lon_min += 360.0;
			else if (lonflip == 0
			    && lon_min > 180.0)
			    lon_min -= 360.0;
			else if (lonflip == 1
			    && lon_min < 0.0)
			    lon_min += 360.0;
			if (lonflip == -1 
			    && lon_max > 0.0)
			    lon_max -= 360.0;
			else if (lonflip == 0
			    && lon_max < -180.0)
			    lon_max += 360.0;
			else if (lonflip == 0
			    && lon_max > 180.0)
			    lon_max -= 360.0;
			else if (lonflip == 1
			    && lon_max < 0.0)
			    lon_max += 360.0;
			if (lonflip == -1 
			    && lat_min > 0.0)
			    lat_min -= 360.0;
			else if (lonflip == 0
			    && lat_min < -180.0)
			    lat_min += 360.0;
			else if (lonflip == 0
			    && lat_min > 180.0)
			    lat_min -= 360.0;
			else if (lonflip == 1
			    && lat_min < 0.0)
			    lat_min += 360.0;
			if (lonflip == -1 
			    && lat_max > 0.0)
			    lat_max -= 360.0;
			else if (lonflip == 0
			    && lat_max < -180.0)
			    lat_max += 360.0;
			else if (lonflip == 0
			    && lat_max > 180.0)
			    lat_max -= 360.0;
			else if (lonflip == 1
			    && lat_max < 0.0)
			    lat_max += 360.0;
			    
			/* check for lonflip conflict with bounds */
			if (lon_min >= lon_max || lat_min >= lat_max)
			    *file_in_bounds = MB_YES;
			    
			/* else check against desired input bounds */
			else
			    {
			    if (lon_min < bounds[1] && lon_max > bounds[0]
				&& lat_min < bounds[3] && lat_max > bounds[2])
				*file_in_bounds = MB_YES;
			    else
				*file_in_bounds = MB_NO;
			    }
			}
			
		    /* else if no data assume inf file is botched so
			assume file has data in bounds */
		    else
			*file_in_bounds = MB_YES;
		    
		    /* close the file */
		    fclose(fp);
		    }
			
		/* if no inf file assume file has data in bounds */
		else
		    {
		    *file_in_bounds = MB_YES;
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
