/*--------------------------------------------------------------------
 *    The MB-system:	mb_get.c	1/26/93
 *    $Id: mb_get.c,v 4.2 1994-10-21 12:11:53 caress Exp $
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
 * mb_get.c reads and averages multibeam data from a file
 * which has been initialized by mb_read_init(). Crosstrack distances
 * are not mapped into lon and lat.
 *
 * Author:	D. W. Caress
 * Date:	January 26, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.4  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.3  1994/02/23  00:32:27  caress
 * Fixed several debug messages plus a couple real bugs.
 *
 * Revision 4.2  1994/02/20  03:25:30  caress
 * Fixed a bunch of bad variable names.
 *
 * Revision 4.1  1994/02/20  01:32:49  caress
 * Changed some of the debug printing.
 *
 * Revision 4.0  1994/02/18  21:43:12  caress
 * First cut of new version.  Includes new handling of
 * sidescan and amplitude data.
 *
 * Revision 3.3  1993/06/09  08:14:25  caress
 * Some unknown minor change.
 *
 * Revision 3.2  1993/06/05  07:19:31  caress
 * Fixed heading averaging problem.
 *
 * Revision 3.1  1993/05/14  22:35:03  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  15:54:14  dale
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

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)
#define RTD (180./M_PI)

/*--------------------------------------------------------------------*/
int mb_get(verbose,mbio_ptr,kind,pings,time_i,time_d,
		navlon,navlat,speed,heading,distance,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*mbio_ptr;
int	*kind;
int	*pings;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*distance;
int	*nbath;
int	*namp;
int	*nss;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
char	*comment;
int	*error;
{
  static char rcs_id[]="$Id: mb_get.c,v 4.2 1994-10-21 12:11:53 caress Exp $";
	char	*function_name = "mb_get";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;
	int	done;
	int	reset_last;
	double	mtodeglon, mtodeglat;
	double	dx, dy;
	double	delta_time;
	double	headingx, headingy;
	double	denom;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* initialize binning values */
	mb_io_ptr->pings_read = 0;
	mb_io_ptr->pings_binned = 0;
	mb_io_ptr->time_d = 0.0;
	mb_io_ptr->lon = 0.0;
	mb_io_ptr->lat = 0.0;
	mb_io_ptr->speed = 0.0;
	mb_io_ptr->heading = 0.0;
	headingx = 0.0;
	headingy = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->bath[i] = 0.0;
		mb_io_ptr->bath_acrosstrack[i] = 0.0;
		mb_io_ptr->bath_alongtrack[i] = 0.0;
		mb_io_ptr->bath_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->amp[i] = 0.0;
		mb_io_ptr->amp_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->ss[i] = 0.0;
		mb_io_ptr->ss_acrosstrack[i] = 0.0;
		mb_io_ptr->ss_alongtrack[i] = 0.0;
		mb_io_ptr->ss_num[i] = 0;
		}

	/* read the data */
	done = MB_NO;
	while (done == MB_NO)
		{

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg2  About to read ping in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       need_new_ping: %d\n",
				mb_io_ptr->need_new_ping);
			fprintf(stderr,"dbg2       ping_count:    %d\n",
				mb_io_ptr->ping_count);
			fprintf(stderr,"dbg2       pings_read:    %d\n",
				mb_io_ptr->pings_read);
			fprintf(stderr,"dbg2       status:        %d\n",status);
			fprintf(stderr,"dbg2       error:         %d\n",*error);
			}

		/* get next ping */
		if (mb_io_ptr->need_new_ping)
			{
			status = mb_read_ping(verbose,mbio_ptr,NULL,error);

			/* set errors if not survey data */
			if (status == MB_SUCCESS)
				{
				mb_io_ptr->need_new_ping = MB_NO;
				if (mb_io_ptr->new_kind == MB_DATA_DATA)
					mb_io_ptr->ping_count++;
				else if (mb_io_ptr->new_kind == MB_DATA_COMMENT)
					{
					mb_io_ptr->comment_count++;
					status = MB_FAILURE;
					*error = MB_ERROR_COMMENT;
					mb_io_ptr->new_error = *error;
					}
				else
					{
					status = MB_FAILURE;
					*error = MB_ERROR_OTHER;
					mb_io_ptr->new_error = *error;
					}
				}
			}
		else
			{
			*error = mb_io_ptr->new_error;
			if (*error == MB_ERROR_NO_ERROR)
				status = MB_SUCCESS;
			else
				status = MB_FAILURE;
			}

		/* if not a fatal error, increment ping counter */
		if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA)
			mb_io_ptr->pings_read++;

		/* if first ping read set "old" navigation values */
		if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA
			&& mb_io_ptr->ping_count == 1)
			{
			mb_io_ptr->old_time_d = mb_io_ptr->new_time_d;
			mb_io_ptr->old_lon = mb_io_ptr->new_lon;
			mb_io_ptr->old_lat = mb_io_ptr->new_lat;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg2  New ping read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       need_new_ping: %d\n",
				mb_io_ptr->need_new_ping);
			fprintf(stderr,"dbg2       ping_count:    %d\n",
				mb_io_ptr->ping_count);
			fprintf(stderr,"dbg2       comment_count: %d\n",
				mb_io_ptr->comment_count);
			fprintf(stderr,"dbg2       pings_read:    %d\n",
				mb_io_ptr->pings_read);
			fprintf(stderr,"dbg2       status:        %d\n",
				status);
			fprintf(stderr,"dbg2       error:         %d\n",
				*error);
			fprintf(stderr,"dbg2       new_error:     %d\n",
				mb_io_ptr->new_error);
			}

		/* check for out of location or time bounds */
		if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		if (mb_io_ptr->new_lon < mb_io_ptr->bounds[0] 
			|| mb_io_ptr->new_lon > mb_io_ptr->bounds[1] 
			|| mb_io_ptr->new_lat < mb_io_ptr->bounds[2] 
			|| mb_io_ptr->new_lat > mb_io_ptr->bounds[3])
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_BOUNDS;
			}
		else if (mb_io_ptr->new_time_d > mb_io_ptr->etime_d 
			|| mb_io_ptr->new_time_d < mb_io_ptr->btime_d)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
			}
		}

		/* check for time gap */
		if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA
			&& mb_io_ptr->ping_count > 1)
		{
		if ((mb_io_ptr->new_time_d - mb_io_ptr->last_time_d) 
			> 60*mb_io_ptr->timegap)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_TIME_GAP;
			}
		}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping checked by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       ping_count:    %d\n",
				mb_io_ptr->ping_count);
			fprintf(stderr,"dbg4       comment_count: %d\n",
				mb_io_ptr->comment_count);
			fprintf(stderr,"dbg4       pings_avg:     %d\n",
				mb_io_ptr->pings_avg);
			fprintf(stderr,"dbg4       pings_read:    %d\n",
				mb_io_ptr->pings_read);
			fprintf(stderr,"dbg4       error:         %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       status:        %d\n",
				status);
			}
		if (verbose >= 4 
			&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
			{
			fprintf(stderr,"dbg4       comment:     \ndbg4       %s\n",
				mb_io_ptr->new_comment);
			}
		else if (verbose >= 4 
			&& mb_io_ptr->new_kind == MB_DATA_DATA
			&& *error <= MB_ERROR_NO_ERROR
			&& *error > MB_ERROR_COMMENT)
			{
			fprintf(stderr,"dbg4       time_i[0]:     %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:     %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:     %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:     %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:     %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:     %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:     %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:        %f\n",
				mb_io_ptr->new_time_d);
			fprintf(stderr,"dbg4       longitude:     %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:      %f\n",
				mb_io_ptr->new_lat);
			fprintf(stderr,"dbg4       speed:         %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:       %f\n",
				mb_io_ptr->new_heading);
			fprintf(stderr,"dbg4       beams_bath:    %d\n",
				mb_io_ptr->beams_bath);
			if (mb_io_ptr->beams_bath > 0)
			  {
			  fprintf(stderr,"dbg4       beam   bath  crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->beams_bath;i++)
			    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
				i,mb_io_ptr->new_bath[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			  }
			fprintf(stderr,"dbg4       beams_amp:     %d\n",
				mb_io_ptr->beams_amp);
			if (mb_io_ptr->beams_amp > 0)
			  {
			  fprintf(stderr,"dbg4       beam    amp  crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->beams_bath;i++)
			    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
				i,mb_io_ptr->new_amp[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			  }
			fprintf(stderr,"dbg4       pixels_ss:     %d\n",
				mb_io_ptr->pixels_ss);
			if (mb_io_ptr->pixels_ss > 0)
			  {
			  fprintf(stderr,"dbg4       pixel sidescan crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->pixels_ss;i++)
			    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
				i,mb_io_ptr->new_ss[i],
				mb_io_ptr->new_ss_acrosstrack[i],mb_io_ptr->new_ss_alongtrack[i]);
			  }
			}

		/* now bin the data if appropriate */
		if (mb_io_ptr->new_kind == MB_DATA_DATA &&

			/* if data is ok */
			(status == MB_SUCCESS

			/* or if nonfatal error and only one ping read, 
				bin the ping */
			|| (*error < MB_ERROR_NO_ERROR 
			&& *error > MB_ERROR_COMMENT
			&& mb_io_ptr->pings_read == 1))
			)

			{
			/* bin the values */
			mb_io_ptr->pings_binned++;
			mb_io_ptr->time_d = mb_io_ptr->time_d 
				+ mb_io_ptr->new_time_d;
			mb_io_ptr->lon = mb_io_ptr->lon 
				+ mb_io_ptr->new_lon;
			mb_io_ptr->lat = mb_io_ptr->lat 
				+ mb_io_ptr->new_lat;
			mb_io_ptr->speed = mb_io_ptr->speed 
				+ mb_io_ptr->new_speed;
			mb_io_ptr->heading = mb_io_ptr->heading 
				+ mb_io_ptr->new_heading;
			headingx = headingx + sin(DTR*mb_io_ptr->new_heading);
			headingy = headingy + cos(DTR*mb_io_ptr->new_heading);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  {
			  if (mb_io_ptr->new_bath[i] > 0 
			      || mb_io_ptr->pings == 1)
			    {
			    mb_io_ptr->bath[i] = mb_io_ptr->bath[i] 
			                         + mb_io_ptr->new_bath[i];
			    mb_io_ptr->bath_acrosstrack[i] = mb_io_ptr->bath_acrosstrack[i] 
			                         + mb_io_ptr->new_bath_acrosstrack[i];
			    mb_io_ptr->bath_alongtrack[i] = mb_io_ptr->bath_alongtrack[i] 
			                         + mb_io_ptr->new_bath_alongtrack[i];
			    mb_io_ptr->bath_num[i]++;
			    }
			  }
			for (i=0;i<mb_io_ptr->beams_amp;i++)
			  {
			  if (mb_io_ptr->new_amp[i] > 0 
			      || mb_io_ptr->pings == 1)
			    {
			    mb_io_ptr->amp[i] = mb_io_ptr->amp[i] 
			                         + mb_io_ptr->new_amp[i];
			    mb_io_ptr->amp_num[i]++;
			    }
			  }
			for (i=0;i<mb_io_ptr->pixels_ss;i++)
			  {
			  if (mb_io_ptr->new_ss[i] > 0 
			      || mb_io_ptr->pings == 1)
			    {
			    mb_io_ptr->ss[i] = mb_io_ptr->ss[i] 
			                         + mb_io_ptr->new_ss[i];
			    mb_io_ptr->ss_acrosstrack[i] = mb_io_ptr->ss_acrosstrack[i] 
			                         + mb_io_ptr->new_ss_acrosstrack[i];
			    mb_io_ptr->ss_alongtrack[i] = mb_io_ptr->ss_alongtrack[i] 
			                         + mb_io_ptr->new_ss_alongtrack[i];
			    mb_io_ptr->ss_num[i]++;
			    }
			  }
			}

		/* print debug statements */
		if (verbose >= 4 
			&& mb_io_ptr->new_kind == MB_DATA_DATA 
			&& (status == MB_SUCCESS
			|| (*error < MB_ERROR_NO_ERROR 
			&& *error > MB_ERROR_COMMENT
			&& mb_io_ptr->pings_read == 1)))
			{
			fprintf(stderr,"\ndbg4  New ping binned by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Current binned ping values:\n");
			fprintf(stderr,"dbg4       pings_binned: %d\n",
				mb_io_ptr->pings_binned);
			fprintf(stderr,"dbg4       time_d:       %f\n",
				mb_io_ptr->time_d);
			fprintf(stderr,"dbg4       longitude:    %f\n",
				mb_io_ptr->lon);
			fprintf(stderr,"dbg4       latitude:     %f\n",
				mb_io_ptr->lat);
			fprintf(stderr,"dbg4       speed:        %f\n",
				mb_io_ptr->speed);
			fprintf(stderr,"dbg4       heading:      %f\n",
				mb_io_ptr->heading);
			fprintf(stderr,"dbg4       beams_bath:    %d\n",
				mb_io_ptr->beams_bath);
			if (mb_io_ptr->beams_bath > 0)
			  {
			  fprintf(stderr,"dbg4       beam   nbath bath  crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->beams_bath;i++)
			    fprintf(stderr,"dbg4       %4d   %4d  %f    %f     %f\n",
				i,mb_io_ptr->bath_num[i],mb_io_ptr->bath[i],
				mb_io_ptr->bath_acrosstrack[i],
				mb_io_ptr->bath_alongtrack[i]);
			  }
			fprintf(stderr,"dbg4       beams_amp:    %d\n",
				mb_io_ptr->beams_amp);
			if (mb_io_ptr->beams_amp > 0)
			  {
			  fprintf(stderr,"dbg4       beam    namp  amp  crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->beams_amp;i++)
			    fprintf(stderr,"dbg4       %4d   %4d  %f    %f     %f\n",
				i,mb_io_ptr->amp_num[i],mb_io_ptr->amp[i],
				mb_io_ptr->bath_acrosstrack[i],
				mb_io_ptr->bath_alongtrack[i]);
			  }
			fprintf(stderr,"dbg4       pixels_ss:     %d\n",
				mb_io_ptr->pixels_ss);
			if (mb_io_ptr->pixels_ss > 0)
			  {
			  fprintf(stderr,"dbg4       pixel nss  sidescan crosstrack alongtrack\n");
			  for (i=0;i<mb_io_ptr->pixels_ss;i++)
			    fprintf(stderr,"dbg4       %4d   %4d   %f    %f     %f\n",
				i,mb_io_ptr->ss_num[i],mb_io_ptr->ss[i],
				mb_io_ptr->ss_acrosstrack[i],
				mb_io_ptr->ss_alongtrack[i]);
			  }
			}

		/* if data is ok but more pings needed keep reading */
		if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA 
			&& mb_io_ptr->pings_binned < mb_io_ptr->pings_avg)
			{
			done = MB_NO;
			mb_io_ptr->need_new_ping = MB_YES;
			reset_last = MB_YES;
			}

		/* if data is ok and enough pings binned then done */
		else if (status == MB_SUCCESS 
			&& mb_io_ptr->new_kind == MB_DATA_DATA 
			&& mb_io_ptr->pings_binned >= mb_io_ptr->pings_avg)
			{
			done = MB_YES;
			mb_io_ptr->need_new_ping = MB_YES;
			reset_last = MB_YES;
			}

		/* if data gap and only one ping read and more
			pings needed set error save flag and keep reading */
		else if (*error == MB_ERROR_TIME_GAP
			&& mb_io_ptr->new_kind == MB_DATA_DATA 
			&& mb_io_ptr->pings_read == 1
			&& mb_io_ptr->pings_avg > 1)
			{
			done = MB_NO;
			mb_io_ptr->need_new_ping = MB_YES;
			mb_io_ptr->error_save = *error;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = MB_YES;
			}

		/* if other kind of data and need more pings
			then keep reading */
		else if ((*error == MB_ERROR_OTHER
			|| *error == MB_ERROR_UNINTELLIGIBLE)
			&& mb_io_ptr->pings_binned < mb_io_ptr->pings_avg)
			{
			done = MB_NO;
			mb_io_ptr->need_new_ping = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = MB_NO;
			}

		/* if error and only one ping read then done */
		else if (*error != MB_ERROR_NO_ERROR
			&& mb_io_ptr->pings_read <= 1)
			{
			done = MB_YES;
			mb_io_ptr->need_new_ping = MB_YES;
			if (*error == MB_ERROR_TIME_GAP
				|| *error == MB_ERROR_OUT_BOUNDS)
				reset_last = MB_YES;
			else
				reset_last = MB_NO;
			}

		/* if error and more than one ping read, 
			then done but save the ping */
		else if (*error != MB_ERROR_NO_ERROR)
			{
			done = MB_YES;
			mb_io_ptr->need_new_ping = MB_NO;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = MB_NO;
			}

		/* if needed reset "last" pings */
		if (reset_last == MB_YES)
			{
			mb_io_ptr->last_time_d = mb_io_ptr->new_time_d;
			mb_io_ptr->last_lon = mb_io_ptr->new_lon;
			mb_io_ptr->last_lat = mb_io_ptr->new_lat;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  End of reading loop in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Current status values:\n");
			fprintf(stderr,"dbg4       done:          %d\n",done);
			fprintf(stderr,"dbg4       need_new_ping: %d\n",
					mb_io_ptr->need_new_ping);
			fprintf(stderr,"dbg4       pings_binned:  %d\n",
					mb_io_ptr->pings_binned);
			fprintf(stderr,"dbg4       error:         %d\n",*error);			fprintf(stderr,"dbg4       status:        %d\n",status);
			}
		}

	/* set output number of pings */
	*pings = mb_io_ptr->pings_binned;

	/* set data kind */
	if (mb_io_ptr->pings_binned > 0)
		*kind = MB_DATA_DATA;
	else if (*error == MB_ERROR_COMMENT)
		*kind = MB_DATA_COMMENT;
	else
		*kind = mb_io_ptr->new_kind;

	/* get output time */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT)
		{
		if (mb_io_ptr->pings_binned == 1)
			{
			for (i=0;i<7;i++)
				time_i[i] = mb_io_ptr->new_time_i[i];
			*time_d = mb_io_ptr->new_time_d;
			}
		else if (mb_io_ptr->pings_binned > 1)
			{
			*time_d = mb_io_ptr->time_d/mb_io_ptr->pings_binned;
			mb_get_date(verbose,*time_d,time_i);
			}
		else
			{
			*error = MB_ERROR_NO_PINGS_BINNED;
			}
		}

	/* get other output values */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT)
		{
		/* get navigation values */
		*navlon = mb_io_ptr->lon/mb_io_ptr->pings_binned;
		*navlat = mb_io_ptr->lat/mb_io_ptr->pings_binned;
		headingx = headingx/mb_io_ptr->pings_binned;
		headingy = headingy/mb_io_ptr->pings_binned;
		denom = sqrt(headingx*headingx + headingy*headingy);
		if (denom > 0.0)
			{
			headingx = headingx/denom;
			headingy = headingy/denom;
			*heading = RTD*atan2(headingx,headingy);
			}
		else
			*heading = mb_io_ptr->heading/mb_io_ptr->pings_binned;
		if (*heading < 0.0)
			*heading = *heading + 360.0;

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
		if (mb_io_ptr->speed > 0.0)
			{
			*speed = mb_io_ptr->speed/mb_io_ptr->pings_binned;
			delta_time = 0.0;
			}
		else if (mb_io_ptr->old_time_d > 0.0)
			{
			delta_time = 0.000277778*
				(*time_d - mb_io_ptr->old_time_d); /* hours */
			if (delta_time > 0.0)
				*speed = *distance/delta_time; /* km/hr */
			else
				*speed = 0.0;
			}
		else
			*speed = 0.0;

		/* check for less than minimum speed */
		if (*error == MB_ERROR_NO_ERROR 
			|| *error == MB_ERROR_TIME_GAP) 
		{
		if (mb_io_ptr->ping_count > 1 
			&& *speed < mb_io_ptr->speedmin)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_SPEED_TOO_SMALL;
			}
		}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Distance and Speed Calculated in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Speed and Distance Related Values:\n");
			fprintf(stderr,"dbg4       binned speed: %f\n",
					mb_io_ptr->speed);
			fprintf(stderr,"dbg4       pings_binned: %d\n",
					mb_io_ptr->pings_binned);
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
			fprintf(stderr,"dbg4       speed:        %f\n",
					*speed);
			fprintf(stderr,"dbg4       error:        %d\n",*error);
			fprintf(stderr,"dbg4       status:       %d\n",status);
			}

		/* get swath data */
		*nbath = mb_io_ptr->beams_bath;
		*namp = mb_io_ptr->beams_amp;
		*nss = mb_io_ptr->pixels_ss;
		for (i=0;i<*nbath;i++)
			{
			if (mb_io_ptr->bath_num[i] > 0)
				{
				bath[i] = (mb_io_ptr->bath[i])
					/(mb_io_ptr->bath_num[i]);
				bathacrosstrack[i] = (mb_io_ptr->bath_acrosstrack[i])
					/(mb_io_ptr->bath_num[i]);
				bathalongtrack[i] = (mb_io_ptr->bath_alongtrack[i])
					/(mb_io_ptr->bath_num[i]);
				}
			}
		for (i=0;i<*namp;i++)
			{
			if (mb_io_ptr->amp_num[i] > 0)
				{
				amp[i] = (mb_io_ptr->amp[i])
					/(mb_io_ptr->amp_num[i]);
				}
			}
		for (i=0;i<*nss;i++)
			{
			if (mb_io_ptr->ss_num[i] > 0)
				{
				ss[i] = mb_io_ptr->ss[i]
					/mb_io_ptr->ss_num[i];
				ssacrosstrack[i] = mb_io_ptr->ss_acrosstrack[i]
					/mb_io_ptr->ss_num[i];
				ssalongtrack[i] = mb_io_ptr->ss_alongtrack[i]
					/mb_io_ptr->ss_num[i];
				}
			}
		}

	/* get output comment */
	if (*error == MB_ERROR_COMMENT)
		{
		strcpy(comment,mb_io_ptr->new_comment);
		}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT)
		{
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
		}

	/* get saved error flag if needed */
	if (*error == MB_ERROR_NO_ERROR 
		&& mb_io_ptr->error_save != MB_ERROR_NO_ERROR)
		{
		*error = mb_io_ptr->error_save;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       pings:      %d\n",*pings);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       navlon:     %f\n",*navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",*speed);
		fprintf(stderr,"dbg2       heading:    %f\n",*heading);
		fprintf(stderr,"dbg2       distance:   %f\n",*distance);
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		if (verbose >= 3 && mb_io_ptr->beams_bath > 0)
		  {
		  fprintf(stderr,"dbg3       beam   nbath bath  crosstrack alongtrack\n");
		  for (i=0;i<mb_io_ptr->beams_bath;i++)
		    fprintf(stderr,"dbg3       %4d   %4d  %f    %f     %f\n",
			i,mb_io_ptr->bath_num[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       namp:       %d\n",*namp);
		if (verbose >= 3 && mb_io_ptr->beams_amp > 0)
		  {
		  fprintf(stderr,"dbg3       beam    namp  amp  crosstrack alongtrack\n");
		  for (i=0;i<mb_io_ptr->beams_amp;i++)
		    fprintf(stderr,"dbg3       %4d   %4d  %f    %f     %f\n",
			i,mb_io_ptr->amp_num[i],amp[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       nss:      %d\n",*nss);
		if (verbose >= 3 && mb_io_ptr->pixels_ss > 0)
		  {
		  fprintf(stderr,"dbg3       pixel nss  sidescan crosstrack alongtrack\n");
		  for (i=0;i<mb_io_ptr->pixels_ss;i++)
		    fprintf(stderr,"dbg3       %4d   %4d   %f    %f     %f\n",
			i,mb_io_ptr->ss_num[i],ss[i],
			ssacrosstrack[i],ssalongtrack[i]);
		  }
		}
	else if (verbose >= 2 && *kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
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
