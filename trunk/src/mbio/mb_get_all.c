/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_all.c	1/26/93
 *    $Id: mb_get_all.c,v 5.5 2002-09-18 23:32:59 caress Exp $
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
 * mb_get_all.c reads swath data from a file
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
 * Revision 5.4  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.3  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.12  2000/10/16  21:52:54  caress
 * No longer initializes some null arrays.
 *
 * Revision 4.11  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.10  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.9  2000/07/19  03:28:02  caress
 * Fixed handling of some nonsurvey data records.
 *
 * Revision 4.8  1999/08/08  04:12:45  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.7  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.3  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.2  1994/02/23  00:32:27  caress
 * Fixed several debug messages plus a couple real bugs.
 *
 * Revision 4.1  1994/02/20  03:37:05  caress
 * Fixed a number of bad variable names.
 *
 * Revision 4.0  1994/02/20  01:50:47  caress
 * First cut of new version.  Includes new handling of
 * sidescan and amplitude data.
 *
 * Revision 3.1  1993/05/14  22:35:26  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  15:57:18  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"

static char rcs_id[]="$Id: mb_get_all.c,v 5.5 2002-09-18 23:32:59 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_get_all(int verbose, void *mbio_ptr, void **store_ptr, int *kind,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, 
		double *distance, double *altitude, double *sonardepth,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mb_get_all";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;
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
	status = mb_read_ping(verbose,mbio_ptr,*store_ptr,kind,error);
	
	/* if survey data read into storage array */
	if (status == MB_SUCCESS
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_NAV
		    || *kind == MB_DATA_COMMENT))
		{
		/* initialize return values */
		*kind = MB_DATA_NONE;
		for (i=0;i<7;i++)
			time_i[i] = 0;
		*time_d = 0.0;
		*navlon = 0.0;
		*navlat = 0.0;
		*speed = 0.0;
		*heading = 0.0;
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		for (i=0;i<mb_io_ptr->beams_bath_max;i++)
			{
			beamflag[i] = MB_FLAG_NULL;
			bath[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			}
		for (i=0;i<mb_io_ptr->beams_amp_max;i++)
			{
			amp[i] = 0.0;
			}
		for (i=0;i<mb_io_ptr->pixels_ss_max;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			}
		strcpy(comment,"\0");

		/* get the data */
		status = mb_extract(verbose, 
			mbio_ptr, *store_ptr, kind,
			time_i, time_d,
			navlon, navlat, speed, heading,
			nbath, namp, nss, 
			beamflag, bath, amp, bathacrosstrack, bathalongtrack, 
			ss, ssacrosstrack, ssalongtrack, 
			comment, error);
		if (status == MB_SUCCESS
			&& *kind == MB_DATA_DATA)
			{
			status = mb_extract_altitude(verbose, 
				mbio_ptr, *store_ptr, 
				kind,
				sonardepth, 
				altitude, 
				error);
			}
		}

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

	/* increment counters */
	if (status == MB_SUCCESS)
		{
		if (*kind == MB_DATA_DATA)
			mb_io_ptr->ping_count++;
		else if (*kind == MB_DATA_NAV)
			mb_io_ptr->nav_count++;
		else if (*kind == MB_DATA_COMMENT)
			mb_io_ptr->comment_count++;
		}

	/* if first ping read set "old" navigation values */
	if (status == MB_SUCCESS 
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_NAV
		    || *kind == MB_DATA_CALIBRATE)
		&& mb_io_ptr->ping_count == 1)
		{
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
		}

	/* if first nav read set "old" navigation values */
	if (status == MB_SUCCESS 
		&& (*kind == MB_DATA_NAV)
		&& mb_io_ptr->nav_count == 1)
		{
		mb_io_ptr->old_ntime_d = *time_d;
		mb_io_ptr->old_nlon = *navlon;
		mb_io_ptr->old_nlat = *navlat;
		}

	/* calculate speed and distance for ping data */
	if (status == MB_SUCCESS
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_CALIBRATE))
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
			delta_time = 0.000277778*
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
			fprintf(stderr,"dbg4       altitude:     %f\n",
					*altitude);
			fprintf(stderr,"dbg4       sonardepth:   %f\n",
					*sonardepth);
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

	/* calculate speed and distance for nav data */
	else if (status == MB_SUCCESS
		&& (*kind == MB_DATA_NAV))
		{
		/* get coordinate scaling */
		mb_coor_scale(verbose,*navlat,&mtodeglon,&mtodeglat);

		/* get distance value */
		if (mb_io_ptr->old_ntime_d > 0.0)
			{
			dx = (*navlon - mb_io_ptr->old_nlon)/mtodeglon;
			dy = (*navlat - mb_io_ptr->old_nlat)/mtodeglat;
			*distance = 0.001*sqrt(dx*dx + dy*dy); /* km */
			}
		else
			*distance = 0.0;

		/* get speed value */
		if (*speed <= 0.0 && mb_io_ptr->old_ntime_d > 0.0)
			{
			delta_time = 0.000277778*
				(*time_d - mb_io_ptr->old_ntime_d); /* hours */
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
					mb_io_ptr->old_ntime_d);
			fprintf(stderr,"dbg4       old lon:      %f\n",
					mb_io_ptr->old_nlon);
			fprintf(stderr,"dbg4       old lat:      %f\n",
					mb_io_ptr->old_lat);
			fprintf(stderr,"dbg4       distance:     %f\n",
					*distance);
			fprintf(stderr,"dbg4       altitude:     %f\n",
					*altitude);
			fprintf(stderr,"dbg4       sonardepth:   %f\n",
					*sonardepth);
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
		
	/* else set nav values to zero */
	else
		{
		*navlon = 0.0;
		*navlat = 0.0;
		*distance = 0.0;
		*altitude = 0.0;
		*sonardepth = 0.0;
		*speed = 0.0;
		}

	/* check for out of location or time bounds */
	if (status == MB_SUCCESS 
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_NAV
		    || *kind == MB_DATA_CALIBRATE))
		{
		if (*navlon < mb_io_ptr->bounds[0] 
			|| *navlon > mb_io_ptr->bounds[1] 
			|| *navlat < mb_io_ptr->bounds[2] 
			|| *navlat > mb_io_ptr->bounds[3])
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_BOUNDS;
			}
		else if (mb_io_ptr->etime_d > mb_io_ptr->btime_d
			&& *time_d > MB_TIME_D_UNKNOWN 
			&& (*time_d > mb_io_ptr->etime_d 
				|| *time_d < mb_io_ptr->btime_d))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
			}
		else if (mb_io_ptr->etime_d < mb_io_ptr->btime_d
			&& *time_d > MB_TIME_D_UNKNOWN 
			&& (*time_d > mb_io_ptr->etime_d 
				&& *time_d < mb_io_ptr->btime_d))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
			}
		}

	/* check for time gap */
	if (status == MB_SUCCESS 
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_NAV
		    || *kind == MB_DATA_CALIBRATE) 
		&& mb_io_ptr->ping_count > 1)
		{
		if ((*time_d - mb_io_ptr->old_ntime_d) 
			> 60*mb_io_ptr->timegap)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_TIME_GAP;
			}
		}

	/* check for less than minimum speed */
	if ((*error == MB_ERROR_NO_ERROR 
		|| *error == MB_ERROR_TIME_GAP)
		&& (((*kind == MB_DATA_DATA
			|| *kind == MB_DATA_CALIBRATE) 
			&& mb_io_ptr->ping_count > 1)
		    || (*kind == MB_DATA_NAV
			&& mb_io_ptr->nav_count > 1)))
		{
		if (*speed < mb_io_ptr->speedmin)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_SPEED_TOO_SMALL;
			}
		}
			
	/* log errors */
	if (*error < MB_ERROR_NO_ERROR)
		mb_notice_log_error(verbose, mbio_ptr, *error);

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
		fprintf(stderr,"dbg4       nav_count:     %d\n",
			mb_io_ptr->nav_count);
		fprintf(stderr,"dbg4       comment_count: %d\n",
			mb_io_ptr->comment_count);
		fprintf(stderr,"dbg4       error:         %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg4       status:        %d\n",
			status);
		}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT
		&& (*kind == MB_DATA_DATA
		    || *kind == MB_DATA_CALIBRATE))
		{
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
		}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR 
		&& *error > MB_ERROR_COMMENT
		&& *kind == MB_DATA_NAV)
		{
		mb_io_ptr->old_ntime_d = *time_d;
		mb_io_ptr->old_nlon = *navlon;
		mb_io_ptr->old_nlat = *navlat;
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
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg2       distance:      %f\n",*distance);
		fprintf(stderr,"dbg2       altitude:      %f\n",*altitude);
		fprintf(stderr,"dbg2       sonardepth:    %f\n",*sonardepth);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		if (verbose >= 3 && *nbath > 0)
		  {
		  fprintf(stderr,"dbg3       beam   flag  bath  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg3       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       namp:      %d\n",*namp);
		if (verbose >= 3 && *namp > 0)
		  {
		  fprintf(stderr,"dbg3       beam   amp  crosstrack alongtrack\n");
		  for (i=0;i<*namp;i++)
		    fprintf(stderr,"dbg3       %4d   %f    %f     %f\n",
			i,amp[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       nss:      %d\n",*nss);
		if (verbose >= 3 && *nss > 0)
		  {
		  fprintf(stderr,"dbg3       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<*nss;i++)
		    fprintf(stderr,"dbg3       %4d   %f    %f     %f\n",
			i,ss[i],
			ssacrosstrack[i],ssalongtrack[i]);
		  }
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
