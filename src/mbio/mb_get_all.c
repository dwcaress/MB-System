/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_all.c	3.00	1/26/93
 *    $Id: mb_get_all.c,v 3.1 1993-05-14 22:35:26 sohara Exp $
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
 * mb_get_all.c reads multibeam data from a file
 * which has been initialized by mb_read_init(). Crosstrack distances
 * are not mapped into lon and lat.  The data is not averaged, and
 * values are also read into a storage data structure including
 * all possible values output by the particular multibeam system 
 * associated with the specified format.
 *
 * Author:	D. W. Caress
 * Date:	January 26, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.0  1993/04/23  15:57:18  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"

/*--------------------------------------------------------------------*/
int mb_get_all(verbose,mbio_ptr,store_ptr,kind,time_i,time_d,
		navlon,navlat,speed,heading,distance,
		nbath,bath,bathdist,nback,back,backdist,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*kind;
int	time_i[6];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*distance;
int	*nbath;
int	*bath;
int	*bathdist;
int	*nback;
int	*back;
int	*backdist;
char	*comment;
int	*error;
{
  static char rcs_id[]="$Id: mb_get_all.c,v 3.1 1993-05-14 22:35:26 sohara Exp $";
	char	*function_name = "mb_get_all";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;
	int	done;
	double	mtodeglon, mtodeglat;
	double	dx, dy;
	double	delta_time;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		}

	/* get mbio and data structure descriptors */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	*store_ptr = mb_io_ptr->store_data;

	/* initialize return values */
	*kind = MB_DATA_NONE;
	for (i=0;i<6;i++)
		time_i[i] = 0;
	*time_d = 0.0;
	*navlon = 0.0;
	*navlat = 0.0;
	*speed = 0.0;
	*heading = 0.0;
	*nbath = 0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		bath[i] = 0;
		bathdist[i] = 0;
		}
	*nback = 0;
	for (i=0;i<mb_io_ptr->beams_back;i++)
		{
		back[i] = 0;
		backdist[i] = 0;
		}
	strcpy(comment,"\0");

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg2  About to read ping in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       ping_count:    %d\n",
			mb_io_ptr->ping_count);
		fprintf(stderr,"dbg2       status:        %d\n",status);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		}

	/* get next ping */
	status = mb_read_ping(verbose,mbio_ptr,*store_ptr,error);

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg2  New ping read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       status:        %d\n",
			status);
		fprintf(stderr,"dbg2       error:         %d\n",
			*error);
		fprintf(stderr,"dbg2       kind:          %d\n",
			mb_io_ptr->new_kind);
		}

	/* set data kind */
	*kind = mb_io_ptr->new_kind;

	/* increment counters */
	if (status == MB_SUCCESS)
		{
		if (*kind == MB_DATA_COMMENT)
			mb_io_ptr->comment_count++;
		else
			mb_io_ptr->ping_count++;
		}

	/* if first ping read set "old" navigation values */
	if (status == MB_SUCCESS 
		&& *kind != MB_DATA_COMMENT
		&& mb_io_ptr->ping_count == 1)
		{
		mb_io_ptr->old_time_d = mb_io_ptr->new_time_d;
		mb_io_ptr->old_lon = mb_io_ptr->new_lon;
		mb_io_ptr->old_lat = mb_io_ptr->new_lat;
		}

	/* get return values */
	if (status == MB_SUCCESS && *kind == MB_DATA_COMMENT)
		{
		strcpy(comment,mb_io_ptr->new_comment);
		}
	else if (status == MB_SUCCESS)
		{
		for (i=0;i<6;i++)
			time_i[i] = mb_io_ptr->new_time_i[i];
		*time_d = mb_io_ptr->new_time_d;
		*navlon = mb_io_ptr->new_lon;
		*navlat = mb_io_ptr->new_lat;
		*speed = mb_io_ptr->new_speed;
		*heading = mb_io_ptr->new_heading;
		}
	if (status == MB_SUCCESS && *kind == MB_DATA_DATA)
		{
		*nbath = mb_io_ptr->beams_bath;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			bath[i] = mb_io_ptr->new_bath[i];
			bathdist[i] = mb_io_ptr->new_bathdist[i];
			}
		*nback = mb_io_ptr->beams_back;
		for (i=0;i<mb_io_ptr->beams_back;i++)
			{
			back[i] = mb_io_ptr->new_back[i];
			backdist[i] = mb_io_ptr->new_backdist[i];
			}
		}

	/* calculate speed and distance */
	if (status == MB_SUCCESS && *kind != MB_DATA_COMMENT)
		{
		/* get coordinate scaling */
		mb_coor_scale(verbose,*navlat,&mtodeglon,&mtodeglat);

		/* get distance value */
		if (mb_io_ptr->old_time_d > 0.0)
			{
			dx = (*navlon - mb_io_ptr->old_lon)/mtodeglon;
			dy = (*navlat - mb_io_ptr->old_lat)/mtodeglat;
			*distance = 0.001*sqrt(dx*dx + dy*dy); /* km */
			}
		else
			*distance = 0.0;

		/* get speed value */
		if (*speed <= 0.0 && mb_io_ptr->old_time_d > 0.0)
			{
			delta_time = 0.0166667*
				(*time_d - mb_io_ptr->old_time_d); /* hours */
			if (delta_time > 0.0)
				*speed = *distance/delta_time; /* km/hr */
			else
				*speed = 0.0;
			}
		else if (*speed < 0.0)
			*speed = 0.0;

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Distance and Speed Calculated in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Speed and Distance Related Values:\n");
			fprintf(stderr,"dbg4       ping_count:   %d\n",
					mb_io_ptr->ping_count);
			fprintf(stderr,"dbg4       time:         %f\n",
					*time_d);
			fprintf(stderr,"dbg4       lon:          %f\n",
					*navlon);
			fprintf(stderr,"dbg4       lat:          %f\n",
					*navlat);
			fprintf(stderr,"dbg4       old time:     %f\n",
					mb_io_ptr->old_time_d);
			fprintf(stderr,"dbg4       old lon:      %f\n",
					mb_io_ptr->old_lon);
			fprintf(stderr,"dbg4       old lat:      %f\n",
					mb_io_ptr->old_lat);
			fprintf(stderr,"dbg4       distance:     %f\n",
					*distance);
			fprintf(stderr,"dbg4       delta_time:   %f\n",
					delta_time);
			fprintf(stderr,"dbg4       raw speed:    %f\n",
					mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       speed:        %f\n",
					*speed);
			fprintf(stderr,"dbg4       error:        %d\n",*error);
			fprintf(stderr,"dbg4       status:       %d\n",status);
			}
		}

	/* check for out of location or time bounds */
	if (status == MB_SUCCESS && *kind != MB_DATA_COMMENT)
		{
		if (*navlon < mb_io_ptr->bounds[0] 
			|| *navlon > mb_io_ptr->bounds[1] 
			|| *navlat < mb_io_ptr->bounds[2] 
			|| *navlat > mb_io_ptr->bounds[3])
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_BOUNDS;
			}
		else if (*time_d > mb_io_ptr->etime_d 
			|| *time_d < mb_io_ptr->btime_d)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
			}
		}

	/* check for time gap */
	if (status == MB_SUCCESS 
		&& *kind != MB_DATA_COMMENT 
		&& mb_io_ptr->ping_count > 1)
		{
		if ((*time_d - mb_io_ptr->last_time_d) 
			> mb_io_ptr->timegap)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_TIME_GAP;
			}
		}

	/* check for less than minimum speed */
	if ((*error == MB_ERROR_NO_ERROR 
		|| *error == MB_ERROR_TIME_GAP)
		&& mb_io_ptr->ping_count > 1)
		{
		if (*speed < mb_io_ptr->speedmin)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_SPEED_TOO_SMALL;
			}
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  New ping checked by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       kind:          %d\n",
			*kind);
		fprintf(stderr,"dbg4       ping_count:    %d\n",
			mb_io_ptr->ping_count);
		fprintf(stderr,"dbg4       comment_count: %d\n",
			mb_io_ptr->comment_count);
		fprintf(stderr,"dbg4       error:         %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg4       status:        %d\n",
			status);
		}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT)
		{
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:         %d\n",*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       bath[%d]: %d  bathdist[%d]: %d\n",
			i,bath[i],i,bathdist[i]);
		fprintf(stderr,"dbg2       nback:         %d\n",*nback);
		for (i=0;i<*nback;i++)
		  fprintf(stderr,"dbg2       back[%d]: %d  backdist[%d]: %d\n",
			i,back[i],i,backdist[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
