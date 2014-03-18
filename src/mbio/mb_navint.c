/*--------------------------------------------------------------------
 *    The MB-system:	mb_time.c	10/30/2000
 *    $Id$
 *
 *    Copyright (c) 2000-2014 by
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
 * mb_navint.c includes the "mb_" functions used to interpolate
 * navigation for data formats using asynchronous nav.
 *
 * Author:	D. W. Caress
 * Date:	October 30, 2000
 *
 * $Log: mb_navint.c,v $
 * Revision 5.14  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.13  2006/03/14 01:41:52  caress
 * Improved debug messages.
 *
 * Revision 5.12  2005/06/15 15:17:51  caress
 * Added some useful debug statements.
 *
 * Revision 5.11  2005/03/25 04:31:23  caress
 * Minor changes to code comments.
 *
 * Revision 5.10  2004/07/15 19:25:04  caress
 * Progress in supporting Reson 7k data.
 *
 * Revision 5.9  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.8  2003/01/15 20:51:48  caress
 * Release 5.0.beta28
 *
 * Revision 5.7  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.6  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.5  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.4  2001/10/12 21:08:37  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.3  2001/08/10 22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.2  2001-07-19 17:31:11-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/06/08 21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"

/*  #define MB_NAVINT_DEBUG 1
    #define MB_ATTINT_DEBUG 1
    #define MB_HEDINT_DEBUG 1
    #define MB_DEPINT_DEBUG 1
    #define MB_ALTINT_DEBUG 1 */

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
/* 	function mb_navint_add adds a nav fix to the internal
		list used for interpolation/extrapolation. */
int mb_navint_add(int verbose, void *mbio_ptr, double time_d, double lon, double lat, int *error)
{
	char	*function_name = "mb_navint_add";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       lon:        %f\n",lon);
		fprintf(stderr,"dbg2       lat:        %f\n",lat);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Current nav fix values:\n");
		for (i=0;i<mb_io_ptr->nfix;i++)
			fprintf(stderr,"dbg2       nav fix[%2d]:   %f %f %f\n",
						i, mb_io_ptr->fix_time_d[i],
						mb_io_ptr->fix_lon[i],
						mb_io_ptr->fix_lat[i]);
		}

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nfix == 0
		|| (time_d > mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]))
		{
		/* if list if full make room for another nav fix */
		if (mb_io_ptr->nfix >= MB_ASYNCH_SAVE_MAX)
			{
			mb_io_ptr->nfix = MB_ASYNCH_SAVE_MAX - 1;
			for (i=0;i<mb_io_ptr->nfix;i++)
				{
				mb_io_ptr->fix_time_d[i]
					= mb_io_ptr->fix_time_d[i+1];
				mb_io_ptr->fix_lon[i] = mb_io_ptr->fix_lon[i+1];
				mb_io_ptr->fix_lat[i] = mb_io_ptr->fix_lat[i+1];
				}
			}

		/* add new fix to list */
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] = time_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] = lon;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] = lat;
		mb_io_ptr->nfix++;
#ifdef MB_NAVINT_DEBUG
	fprintf(stderr, "mb_navint_add:    Nav fix %d %f %f added\n", mb_io_ptr->nfix, lon, lat);
#endif

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Nav fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nfix:       %d\n",
				mb_io_ptr->nfix);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]);
			fprintf(stderr,"dbg4       fix_lon:    %f\n",
				mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]);
			fprintf(stderr,"dbg4       fix_lat:    %f\n",
				mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		fprintf(stderr,"\ndbg2  Current nav fix values:\n");
		for (i=0;i<mb_io_ptr->nfix;i++)
			fprintf(stderr,"dbg2       nav fix[%2d]:   %f %f %f\n",
						i, mb_io_ptr->fix_time_d[i],
						mb_io_ptr->fix_lon[i],
						mb_io_ptr->fix_lat[i]);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_navint_interp interpolates or extrapolates a
		nav fix from the internal list. */
int mb_navint_interp(int verbose, void *mbio_ptr,
		double time_d, double heading, double rawspeed,
		double *lon, double *lat, double *speed,
		int *error)
{
	char	*function_name = "mb_navint_interp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	double	mtodeglon, mtodeglat;
	double	dx, dy, dt, dd;
	double	factor, headingx, headingy;
	double  speed_mps;
	int	ifix;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       rawspeed:   %f\n",rawspeed);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Current nav fix values:\n");
		for (i=0;i<mb_io_ptr->nfix;i++)
			fprintf(stderr,"dbg2       nav fix[%2d]:   %f %f %f\n",
						i, mb_io_ptr->fix_time_d[i],
						mb_io_ptr->fix_lon[i],
						mb_io_ptr->fix_lat[i]);
		}

	/* get degrees to meters conversion if fix available */
	if (mb_io_ptr->nfix > 0)
		{
		mb_coor_scale(verbose,
			mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
			&mtodeglon,&mtodeglat);
		}

	/* use raw speed if available */
	if (rawspeed > 0.0)
	  *speed = rawspeed; /* km/hr */

	/* else get speed averaged over all available fixes */
	else if (mb_io_ptr->nfix > 1)
		{
		dx = (mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]
			- mb_io_ptr->fix_lon[0])/mtodeglon;
		dy = (mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]
			- mb_io_ptr->fix_lat[0])/mtodeglat;
		dt = mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
			- mb_io_ptr->fix_time_d[0];
		*speed = 3.6 * sqrt(dx*dx + dy*dy)/dt; /* km/hr */
		}

	/* else speed unknown */
	else
		*speed = 0.0;

	/* get speed in m/s */
	speed_mps = *speed / 3.6;

	/* interpolate if possible */
	if (mb_io_ptr->nfix > 1
		&& (mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
			>= time_d)
		&& (mb_io_ptr->fix_time_d[0]
			<= time_d))
		{
		/* get interpolated position */
		ifix = (mb_io_ptr->nfix - 1) * (time_d - mb_io_ptr->fix_time_d[0])
			/(mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1] - mb_io_ptr->fix_time_d[0]);
		while (time_d > mb_io_ptr->fix_time_d[ifix])
			{
			ifix++;
			}
		while (time_d < mb_io_ptr->fix_time_d[ifix-1])
			{
			ifix--;
			}

		factor = (time_d - mb_io_ptr->fix_time_d[ifix-1])
			/(mb_io_ptr->fix_time_d[ifix] - mb_io_ptr->fix_time_d[ifix-1]);
		*lon = mb_io_ptr->fix_lon[ifix-1]
			+ factor*(mb_io_ptr->fix_lon[ifix] - mb_io_ptr->fix_lon[ifix-1]);
		*lat = mb_io_ptr->fix_lat[ifix-1]
			+ factor*(mb_io_ptr->fix_lat[ifix] - mb_io_ptr->fix_lat[ifix-1]);
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
	fprintf(stderr, "mb_navint_interp: Nav  %f %f interpolated at fix %d of %d with factor:%f\n",
		*lon, *lat, ifix, mb_io_ptr->nfix, factor);
#endif
		}

	/* extrapolate from last fix - note zero speed
	    results in just using the last fix */
	else if (mb_io_ptr->nfix > 1
		&& (mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
			< time_d))
		{
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			* speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]
			+ headingx * mtodeglon * dd;
		*lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]
			+ headingy * mtodeglat * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
	fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from last fix of %d with distance:%f and speed:%f\n",
		*lon, *lat, mb_io_ptr->nfix, dd, speed_mps);
#endif
		}

	/* extrapolate from first fix - note zero speed
	    results in just using the first fix */
	else if (mb_io_ptr->nfix >= 1)
		{
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[0])
			* speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*lon = mb_io_ptr->fix_lon[0]
			+ headingx * mtodeglon * dd;
		*lat = mb_io_ptr->fix_lat[0]
			+ headingy * mtodeglat * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
	fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from first fix of %d with distance %f and speed:%f\n",
		*lon, *lat, mb_io_ptr->nfix, dd, speed_mps);
#endif
		}

	/* else no fix */
	else
		{
		*lon = 0.0;
		*lat = 0.0;
		*speed = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_NAVINT_DEBUG
	fprintf(stderr, "mb_navint_interp: Nav zeroed\n");
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       lon:        %f\n",*lon);
		fprintf(stderr,"dbg2       lat:        %f\n",*lat);
		fprintf(stderr,"dbg2       speed:      %f\n",*speed);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_add adds a attitude fix to the internal
		list used for interpolation/extrapolation. */
int mb_attint_add(int verbose, void *mbio_ptr,
			double time_d, double heave,
			double roll, double pitch, int *error)
{
	char	*function_name = "mb_attint_add";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nattitude == 0
		|| (time_d > mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1]))
		{
		/* if list if full make room for another attitude fix */
		if (mb_io_ptr->nattitude >= MB_ASYNCH_SAVE_MAX)
			{
			mb_io_ptr->nattitude = MB_ASYNCH_SAVE_MAX - 1;
			for (i=0;i<mb_io_ptr->nattitude;i++)
				{
				mb_io_ptr->attitude_time_d[i]
					= mb_io_ptr->attitude_time_d[i+1];
				mb_io_ptr->attitude_heave[i] = mb_io_ptr->attitude_heave[i+1];
				mb_io_ptr->attitude_roll[i] = mb_io_ptr->attitude_roll[i+1];
				mb_io_ptr->attitude_pitch[i] = mb_io_ptr->attitude_pitch[i+1];
				}
			}

		/* add new fix to list */
		mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude] = time_d;
		mb_io_ptr->attitude_heave[mb_io_ptr->nattitude] = heave;
		mb_io_ptr->attitude_roll[mb_io_ptr->nattitude] = roll;
		mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude] = pitch;
		mb_io_ptr->nattitude++;
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_add:    Attitude fix %d time_d:%f roll:%f pitch:%f heave:%f added\n",
				mb_io_ptr->nattitude, time_d, roll, pitch, heave);
#endif

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Attitude fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nattitude:       %d\n",
				mb_io_ptr->nattitude);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_heave:    %f\n",
				mb_io_ptr->attitude_heave[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_roll:     %f\n",
				mb_io_ptr->attitude_roll[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_pitch:    %f\n",
				mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_nadd adds multiple attitude fixes to the internal
		list used for interpolation/extrapolation. */
int mb_attint_nadd(int verbose, void *mbio_ptr,
			int nsamples, double *time_d, double *heave,
			double *roll, double *pitch, int *error)
{
	char	*function_name = "mb_attint_nadd";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	shift;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       nsamples:   %d\n",nsamples);
		for (i=0;i<nsamples;i++)
			{
			fprintf(stderr,"dbg2       %d time_d:%f heave:%f roll:%f pitch:%f\n",
				i,time_d[i],heave[i],roll[i],pitch[i]);
			}
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* if necessary make room for attitude fixes */
	if (mb_io_ptr->nattitude + nsamples >= MB_ASYNCH_SAVE_MAX)
		{
		shift = mb_io_ptr->nattitude + nsamples - MB_ASYNCH_SAVE_MAX;
		for (i=0;i<mb_io_ptr->nattitude-shift;i++)
			{
			mb_io_ptr->attitude_time_d[i]
				= mb_io_ptr->attitude_time_d[i+shift];
			mb_io_ptr->attitude_heave[i] = mb_io_ptr->attitude_heave[i+shift];
			mb_io_ptr->attitude_roll[i] = mb_io_ptr->attitude_roll[i+shift];
			mb_io_ptr->attitude_pitch[i] = mb_io_ptr->attitude_pitch[i+shift];
			}
		mb_io_ptr->nattitude = mb_io_ptr->nattitude - shift;
		}

	/* add fixes */
	for (i=0;i<nsamples;i++)
		{
		/* add new fix to list */
		mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude] = time_d[i];
		mb_io_ptr->attitude_heave[mb_io_ptr->nattitude] = heave[i];
		mb_io_ptr->attitude_roll[mb_io_ptr->nattitude] = roll[i];
		mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude] = pitch[i];
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_add:    Attitude fix %d of %d: time:%f roll:%f pitch:%f heave:%f added\n",
				i, mb_io_ptr->nattitude, time_d[i], roll[i], pitch[i], heave[i]);
#endif
		mb_io_ptr->nattitude++;

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Attitude fixes added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nattitude:       %d\n",
				mb_io_ptr->nattitude);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_heave:    %f\n",
				mb_io_ptr->attitude_heave[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_roll:     %f\n",
				mb_io_ptr->attitude_roll[mb_io_ptr->nattitude-1]);
			fprintf(stderr,"dbg4       attitude_pitch:    %f\n",
				mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_interp interpolates or extrapolates a
		attitude fix from the internal list. */
int mb_attint_interp(int verbose, void *mbio_ptr,
		double time_d, double *heave,
		double *roll, double *pitch,
		int *error)
{
	char	*function_name = "mb_attint_interp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	double	factor;
	int	ifix;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* interpolate if possible */
	if (mb_io_ptr->nattitude > 1
		&& (mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1]
			>= time_d)
		&& (mb_io_ptr->attitude_time_d[0]
			<= time_d))
		{
		/* get interpolated position */
		ifix = (mb_io_ptr->nattitude - 1) * (time_d - mb_io_ptr->attitude_time_d[0])
			/(mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1] - mb_io_ptr->attitude_time_d[0]);
		while (time_d > mb_io_ptr->attitude_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->attitude_time_d[ifix-1])
			ifix--;

		factor = (time_d - mb_io_ptr->attitude_time_d[ifix-1])
			/(mb_io_ptr->attitude_time_d[ifix] - mb_io_ptr->attitude_time_d[ifix-1]);
		*heave = mb_io_ptr->attitude_heave[ifix-1]
			+ factor*(mb_io_ptr->attitude_heave[ifix] - mb_io_ptr->attitude_heave[ifix-1]);
		*roll = mb_io_ptr->attitude_roll[ifix-1]
			+ factor*(mb_io_ptr->attitude_roll[ifix] - mb_io_ptr->attitude_roll[ifix-1]);
		*pitch = mb_io_ptr->attitude_pitch[ifix-1]
			+ factor*(mb_io_ptr->attitude_pitch[ifix] - mb_io_ptr->attitude_pitch[ifix-1]);
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f interpolated at fix %d of %d with factor:%f\n",
		time_d, *roll, *pitch, *heave, ifix, mb_io_ptr->nattitude, factor);
#endif
		}

	/* extrapolate from last fix */
	else if (mb_io_ptr->nattitude > 1
		&& (mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1]
			< time_d))
		{
		/* extrapolated position using average speed */
		*heave = mb_io_ptr->attitude_heave[mb_io_ptr->nattitude-1];
		*roll = mb_io_ptr->attitude_roll[mb_io_ptr->nattitude-1];
		*pitch = mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude-1];
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f extrapolated from last fix of %d\n",
		time_d, *roll, *pitch, *heave, mb_io_ptr->nattitude);
#endif
		}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nattitude >= 1)
		{
		*heave = mb_io_ptr->attitude_heave[0];
		*roll = mb_io_ptr->attitude_roll[0];
		*pitch = mb_io_ptr->attitude_pitch[0];
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f extrapolated from first fix of %d\n",
		time_d, *roll, *pitch, *heave, mb_io_ptr->nattitude);
#endif
		}

	/* else no fix */
	else
		{
		*heave = 0.0;
		*roll = 0.0;
		*pitch = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_ATTINT_DEBUG
	fprintf(stderr, "mb_attint_interp: Attitude zeroed\n");
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       heave:        %f\n",*heave);
		fprintf(stderr,"dbg2       roll:         %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:        %f\n",*pitch);
		fprintf(stderr,"dbg2       error:        %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_add adds a heading fix to the internal
		list used for interpolation/extrapolation. */
int mb_hedint_add(int verbose, void *mbio_ptr, double time_d, double heading, int *error)
{
	char	*function_name = "mb_hedint_add";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nheading == 0
		|| (time_d > mb_io_ptr->heading_time_d[mb_io_ptr->nheading-1]))
		{
		/* if list if full make room for another heading fix */
		if (mb_io_ptr->nheading >= MB_ASYNCH_SAVE_MAX)
			{
			mb_io_ptr->nheading = MB_ASYNCH_SAVE_MAX - 1;
			for (i=0;i<mb_io_ptr->nheading;i++)
				{
				mb_io_ptr->heading_time_d[i]
					= mb_io_ptr->heading_time_d[i+1];
				mb_io_ptr->heading_heading[i] = mb_io_ptr->heading_heading[i+1];
				}
			}

		/* add new fix to list */
		mb_io_ptr->heading_time_d[mb_io_ptr->nheading] = time_d;
		mb_io_ptr->heading_heading[mb_io_ptr->nheading] = heading;
		mb_io_ptr->nheading++;
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_add:    Heading fix %d %f added\n", mb_io_ptr->nheading, heading);
#endif

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Heading fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nheading:       %d\n",
				mb_io_ptr->nheading);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->heading_time_d[mb_io_ptr->nheading-1]);
			fprintf(stderr,"dbg4       heading_heading:  %f\n",
				mb_io_ptr->heading_heading[mb_io_ptr->nheading-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_nadd adds multiple heading fixes to the internal
		list used for interpolation/extrapolation. */
int mb_hedint_nadd(int verbose, void *mbio_ptr,
			int nsamples, double *time_d, double *heading, int *error)
{
	char	*function_name = "mb_hedint_nadd";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	shift;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       nsamples:   %d\n",nsamples);
		for (i=0;i<nsamples;i++)
			{
			fprintf(stderr,"dbg2       %d time_d:%f heading:%f\n",
				i,time_d[i],heading[i]);
			}
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* if necessary make room for heading fixes */
	if (mb_io_ptr->nheading + nsamples >= MB_ASYNCH_SAVE_MAX)
		{
		shift = mb_io_ptr->nheading + nsamples - MB_ASYNCH_SAVE_MAX;
		for (i=0;i<mb_io_ptr->nheading-shift;i++)
			{
			mb_io_ptr->heading_time_d[i]
				= mb_io_ptr->heading_time_d[i+shift];
			mb_io_ptr->heading_heading[i] = mb_io_ptr->heading_heading[i+shift];
			}
		mb_io_ptr->nheading = mb_io_ptr->nheading - shift;
		}

	/* add fixes */
	for (i=0;i<nsamples;i++)
		{
		/* add new fix to list */
		mb_io_ptr->heading_time_d[mb_io_ptr->nheading] = time_d[i];
		mb_io_ptr->heading_heading[mb_io_ptr->nheading] = heading[i];
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_nadd:    Heading fix %d of %d: %f added\n",
				i, mb_io_ptr->nheading, heading[i]);
#endif
		mb_io_ptr->nheading++;

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Heading fixes added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nheading:       %d\n",
				mb_io_ptr->nheading);
			fprintf(stderr,"dbg4       time_d:          %f\n",
				mb_io_ptr->heading_time_d[mb_io_ptr->nheading-1]);
			fprintf(stderr,"dbg4       heading_heading: %f\n",
				mb_io_ptr->heading_heading[mb_io_ptr->nheading-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_interp interpolates or extrapolates a
		heading fix from the internal list. */
int mb_hedint_interp(int verbose, void *mbio_ptr,
		double time_d, double *heading,
		int *error)
{
	char	*function_name = "mb_hedint_interp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	double	factor;
	int	ifix;
	double	heading1, heading2;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* interpolate if possible */
	if (mb_io_ptr->nheading > 1
		&& (mb_io_ptr->heading_time_d[mb_io_ptr->nheading-1]
			>= time_d)
		&& (mb_io_ptr->heading_time_d[0]
			<= time_d))
		{
		/* get interpolated heading */
		ifix = (mb_io_ptr->nheading - 1) * (time_d - mb_io_ptr->heading_time_d[0])
			/(mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1] - mb_io_ptr->heading_time_d[0]);
		while (time_d > mb_io_ptr->heading_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->heading_time_d[ifix-1])
			ifix--;

		factor = (time_d - mb_io_ptr->heading_time_d[ifix-1])
			/(mb_io_ptr->heading_time_d[ifix] - mb_io_ptr->heading_time_d[ifix-1]);
		heading1 = mb_io_ptr->heading_heading[ifix-1];
		heading2 = mb_io_ptr->heading_heading[ifix];
		if (heading2 - heading1 > 180.0)
			heading2 -= 360.0;
		else if (heading2 - heading1 < -180.0)
			heading2 += 360.0;
		*heading = heading1 + factor*(heading2 - heading1);
		if (*heading < 0.0)
			*heading += 360.0;
		else if (*heading > 360.0)
			*heading -= 360.0;
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_interp: Heading %f interpolated at value %d of %d with factor:%f\n",
		*heading, ifix, mb_io_ptr->nheading, factor);
#endif
		}

	/* extrapolate from last fix */
	else if (mb_io_ptr->nheading > 1
		&& (mb_io_ptr->heading_time_d[mb_io_ptr->nheading-1]
			< time_d))
		{
		/* extrapolated heading using average speed */
		*heading = mb_io_ptr->heading_heading[mb_io_ptr->nheading-1];
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_interp: Heading %f taken from last value of %d\n",
		*heading, mb_io_ptr->nheading);
#endif
		}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nheading >= 1)
		{
		*heading = mb_io_ptr->heading_heading[0];
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_interp: Heading %f taken from first value of %d\n",
		*heading, mb_io_ptr->nheading);
#endif
		}

	/* else no fix */
	else
		{
		*heading = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_HEDINT_DEBUG
	fprintf(stderr, "mb_hedint_interp: Heading zeroed\n");
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       heading:      %f\n",*heading);
		fprintf(stderr,"dbg2       error:        %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_depint_add adds a sonar depth fix to the internal
		list used for interpolation/extrapolation. */
int mb_depint_add(int verbose, void *mbio_ptr, double time_d, double sonardepth, int *error)
{
	char	*function_name = "mb_depint_add";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       sonardepth: %f\n",sonardepth);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nsonardepth == 0
		|| (time_d > mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth-1]))
		{
		/* if list if full make room for another sonardepth fix */
		if (mb_io_ptr->nsonardepth >= MB_ASYNCH_SAVE_MAX)
			{
			mb_io_ptr->nsonardepth = MB_ASYNCH_SAVE_MAX - 1;
			for (i=0;i<mb_io_ptr->nsonardepth;i++)
				{
				mb_io_ptr->sonardepth_time_d[i]
					= mb_io_ptr->sonardepth_time_d[i+1];
				mb_io_ptr->sonardepth_sonardepth[i] = mb_io_ptr->sonardepth_sonardepth[i+1];
				}
			}

		/* add new fix to list */
		mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth] = time_d;
		mb_io_ptr->sonardepth_sonardepth[mb_io_ptr->nsonardepth] = sonardepth;
		mb_io_ptr->nsonardepth++;
#ifdef MB_DEPINT_DEBUG
	fprintf(stderr, "mb_depint_add:    sonardepth fix %d %f added\n", mb_io_ptr->nsonardepth, sonardepth);
#endif

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Sonar depth fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       nsonardepth:       %d\n",
				mb_io_ptr->nsonardepth);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth-1]);
			fprintf(stderr,"dbg4       sonardepth_sonardepth:  %f\n",
				mb_io_ptr->sonardepth_sonardepth[mb_io_ptr->nsonardepth-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_depint_interp interpolates or extrapolates a
		sonar depth fix from the internal list. */
int mb_depint_interp(int verbose, void *mbio_ptr,
		double time_d, double *sonardepth,
		int *error)
{
	char	*function_name = "mb_depint_interp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	double	factor;
	int	ifix;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* interpolate if possible */
	if (mb_io_ptr->nsonardepth > 1
		&& (mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth-1]
			>= time_d)
		&& (mb_io_ptr->sonardepth_time_d[0]
			<= time_d))
		{
		/* get interpolated position */
		ifix = (mb_io_ptr->nsonardepth - 1) * (time_d - mb_io_ptr->sonardepth_time_d[0])
			/(mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth - 1] - mb_io_ptr->sonardepth_time_d[0]);
		while (time_d > mb_io_ptr->sonardepth_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->sonardepth_time_d[ifix-1])
			ifix--;

		factor = (time_d - mb_io_ptr->sonardepth_time_d[ifix-1])
			/(mb_io_ptr->sonardepth_time_d[ifix] - mb_io_ptr->sonardepth_time_d[ifix-1]);
		*sonardepth = mb_io_ptr->sonardepth_sonardepth[ifix-1]
			+ factor*(mb_io_ptr->sonardepth_sonardepth[ifix] - mb_io_ptr->sonardepth_sonardepth[ifix-1]);
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
	fprintf(stderr, "mb_depint_interp: sonardepth %f interpolated at fix %d of %d with factor:%f\n",
		*sonardepth, ifix, mb_io_ptr->nsonardepth, factor);
#endif
		}

	/* extrapolate from last value */
	else if (mb_io_ptr->nsonardepth > 1
		&& (mb_io_ptr->sonardepth_time_d[mb_io_ptr->nsonardepth-1]
			< time_d))
		{
		/* extrapolated depth using last value */
		*sonardepth = mb_io_ptr->sonardepth_sonardepth[mb_io_ptr->nsonardepth-1];
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
	fprintf(stderr, "mb_depint_interp: sonardepth %f extrapolated from last fix of %d\n",
		*sonardepth, mb_io_ptr->nsonardepth);
#endif
		}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nsonardepth >= 1)
		{
		*sonardepth = mb_io_ptr->sonardepth_sonardepth[0];
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
	fprintf(stderr, "mb_depint_interp: sonardepth %f extrapolated from first fix of %d\n",
		*sonardepth, mb_io_ptr->nsonardepth);
#endif
		}

	/* else no fix */
	else
		{
		*sonardepth = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_DEPINT_DEBUG
	fprintf(stderr, "mb_depint_interp: sonardepth zeroed\n");
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       sonardepth:   %f\n",*sonardepth);
		fprintf(stderr,"dbg2       error:        %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_altint_add adds a heading fix to the internal
		list used for interpolation/extrapolation. */
int mb_altint_add(int verbose, void *mbio_ptr, double time_d, double altitude, int *error)
{
	char	*function_name = "mb_altint_add";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       altitude:   %f\n",altitude);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->naltitude == 0
		|| (time_d > mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude-1]))
		{
		/* if list if full make room for another altitude fix */
		if (mb_io_ptr->naltitude >= MB_ASYNCH_SAVE_MAX)
			{
			mb_io_ptr->naltitude = MB_ASYNCH_SAVE_MAX - 1;
			for (i=0;i<mb_io_ptr->naltitude;i++)
				{
				mb_io_ptr->altitude_time_d[i]
					= mb_io_ptr->altitude_time_d[i+1];
				mb_io_ptr->altitude_altitude[i] = mb_io_ptr->altitude_altitude[i+1];
				}
			}

		/* add new fix to list */
		mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude] = time_d;
		mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude] = altitude;
		mb_io_ptr->naltitude++;
#ifdef MB_ALTINT_DEBUG
	fprintf(stderr, "mb_altint_add:    altitude fix %d %f added\n", mb_io_ptr->naltitude, altitude);
#endif

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Altitude fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New fix values:\n");
			fprintf(stderr,"dbg4       naltitude:       %d\n",
				mb_io_ptr->naltitude);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude-1]);
			fprintf(stderr,"dbg4       altitude_altitude:  %f\n",
				mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude-1]);
			}
		}

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_altint_interp interpolates or extrapolates a
		altitude fix from the internal list. */
int mb_altint_interp(int verbose, void *mbio_ptr,
		double time_d, double *altitude,
		int *error)
{
	char	*function_name = "mb_altint_interp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	double	factor;
	int	ifix;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* interpolate if possible */
	if (mb_io_ptr->naltitude > 1
		&& (mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude-1]
			>= time_d)
		&& (mb_io_ptr->altitude_time_d[0]
			<= time_d))
		{
		/* get interpolated position */
		ifix = (mb_io_ptr->naltitude - 1) * (time_d - mb_io_ptr->altitude_time_d[0])
			/(mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1] - mb_io_ptr->altitude_time_d[0]);
		while (time_d > mb_io_ptr->altitude_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->fix_time_d[ifix-1])
			ifix--;

		factor = (time_d - mb_io_ptr->altitude_time_d[ifix-1])
			/(mb_io_ptr->altitude_time_d[ifix] - mb_io_ptr->altitude_time_d[ifix-1]);
		*altitude = mb_io_ptr->altitude_altitude[ifix-1]
			+ factor*(mb_io_ptr->altitude_altitude[ifix] - mb_io_ptr->altitude_altitude[ifix-1]);
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
	fprintf(stderr, "mb_altint_interp: altitude %f interpolated at fix %d of %d with factor:%f\n",
		*altitude, ifix, mb_io_ptr->naltitude, factor);
#endif
		}

	/* extrapolate from last fix */
	else if (mb_io_ptr->naltitude > 1
		&& (mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude-1]
			< time_d))
		{
		/* extrapolated position using average speed */
		*altitude = mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude-1];
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
	fprintf(stderr, "mb_altint_interp: altitude %f extrapolated from last fix of %d\n",
		*altitude, mb_io_ptr->naltitude);
#endif
		}

	/* extrapolate from first fix */
	else if (mb_io_ptr->naltitude >= 1)
		{
		*altitude = mb_io_ptr->altitude_altitude[0];
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
	fprintf(stderr, "mb_altint_interp: altitude %f extrapolated from first fix of %d\n",
		*altitude, mb_io_ptr->naltitude);
#endif
		}

	/* else no fix */
	else
		{
		*altitude = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_ALTINT_DEBUG
	fprintf(stderr, "mb_altint_interp: altitude zeroed\n");
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       altitude:     %f\n",*altitude);
		fprintf(stderr,"dbg2       error:        %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/

int mb_loadnavdata(int verbose, char *merge_nav_file, int merge_nav_format, int merge_nav_lonflip,
                int *merge_nav_num, int *merge_nav_alloc,
                double **merge_nav_time_d, double **merge_nav_lon,
                double **merge_nav_lat, double **merge_nav_speed, int *error)
	{
	char	*function_name = "mb_loadnavdata";
	int	status = MB_SUCCESS;
	char	buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result, *bufftmp;
	int	nrecord;
	int	nchar, nget;
	size_t	size;
	FILE	*tfp;
	int	nav_ok;
	int	time_i[7], time_j[6], ihr, ioff;
	char	NorS[2], EorW[2];
	double	mlon, llon, mlat, llat;
	int	degree, time_set;
	double	sec, hr, dminute;
	double	time_d, heading, sensordepth, roll, pitch, heave;
	int	len;
	int	quality, nsatellite, dilution, gpsheight;
	double	*n_time_d;
	double	*n_lon;
	double	*n_lat;
	double	*n_speed;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:                 %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:                %d\n",verbose);
		fprintf(stderr,"dbg2       merge_nav_file:         %s\n",merge_nav_file);
		fprintf(stderr,"dbg2       merge_nav_format:       %d\n",merge_nav_format);
		fprintf(stderr,"dbg2       merge_nav_lonflip:      %d\n",merge_nav_lonflip);
		fprintf(stderr,"dbg2       merge_nav_num *:        %p\n",merge_nav_num);
		fprintf(stderr,"dbg2       merge_nav_num:          %d\n",*merge_nav_num);
		fprintf(stderr,"dbg2       merge_nav_alloc *:      %p\n",merge_nav_alloc);
		fprintf(stderr,"dbg2       merge_nav_alloc:        %d\n",*merge_nav_alloc);
		fprintf(stderr,"dbg2       merge_nav_time_d **:    %p\n",merge_nav_time_d);
		fprintf(stderr,"dbg2       merge_nav_time_d *:     %p\n",*merge_nav_time_d);
		fprintf(stderr,"dbg2       merge_nav_lon **:       %p\n",merge_nav_lon);
		fprintf(stderr,"dbg2       merge_nav_lon *:        %p\n",*merge_nav_lon);
		fprintf(stderr,"dbg2       merge_nav_lat **:       %p\n",merge_nav_lat);
		fprintf(stderr,"dbg2       merge_nav_lat *:        %p\n",*merge_nav_lat);
		fprintf(stderr,"dbg2       merge_nav_speed **:     %p\n",merge_nav_speed);
		fprintf(stderr,"dbg2       merge_nav_speed *:      %p\n",*merge_nav_speed);
		}
	
	/* set max number of characters to be read at a time */
	if (merge_nav_format == 8)
		nchar = 96;
	else
		nchar = MBP_FILENAMESIZE-1;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_nav_file, "r")) != NULL)
		{
		/* loop over reading the records */
		while ((result = fgets(buffer,nchar,tfp)) == buffer)
			nrecord++;
			
		/* close the file */
		fclose(tfp);
		tfp = NULL;
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_nav_alloc < nrecord)
		{
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_lon, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_lat, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_speed, error);
		if (status == MB_SUCCESS)
			*merge_nav_alloc = nrecord;
		n_time_d = *merge_nav_time_d;
		n_lon = *merge_nav_lon;
		n_lat = *merge_nav_lat;
		n_speed = *merge_nav_speed;
		}

	/* read the records */
	if (status == MB_SUCCESS)
		{
		nrecord = 0;
		if ((tfp = fopen(merge_nav_file, "r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer,nchar,tfp)) == buffer)
				{
				nav_ok = MB_NO;

				/* deal with nav in form: time_d lon lat */
				if (merge_nav_format == 1)
					{
					nget = sscanf(buffer,"%lf %lf %lf",
						&n_time_d[nrecord],&n_lon[nrecord],&n_lat[nrecord]);
					n_speed[nrecord] = 0.0;
					if (nget == 3)
						nav_ok = MB_YES;
					}
		
				/* deal with nav in form: yr mon day hour min sec lon lat */
				else if (merge_nav_format == 2)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_lon[nrecord],&n_lat[nrecord]);
					time_i[5] = (int) sec;
					time_i[6] = 1000000*(sec - time_i[5]);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 8)
						nav_ok = MB_YES;
					}
		
				/* deal with nav in form: yr jday hour min sec lon lat */
				else if (merge_nav_format == 3)
					{
					nget = sscanf(buffer,"%d %d %d %d %lf %lf %lf",
						&time_j[0],&time_j[1],&ihr,
						&time_j[2],&sec,
						&n_lon[nrecord],&n_lat[nrecord]);
					time_j[2] = time_j[2] + 60*ihr;
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 7)
						nav_ok = MB_YES;
					}
		
				/* deal with nav in form: yr jday daymin sec lon lat */
				else if (merge_nav_format == 4)
					{
					nget = sscanf(buffer,"%d %d %d %lf %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,
						&n_lon[nrecord],&n_lat[nrecord]);
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 6)
						nav_ok = MB_YES;
					}
		
				/* deal with nav in L-DEO processed nav format */
				else if (merge_nav_format == 5)
					{
					strncpy(dummy,"\0",128);
					if (buffer[2] == '+')
						{
						time_j[0] = atoi(strncpy(dummy,buffer,2));
						mb_fix_y2k(verbose, time_j[0], &time_j[0]);
						ioff = 3;
						}
					else
						{
						time_j[0] = atoi(strncpy(dummy,buffer,4));
						ioff = 5;
						}
					strncpy(dummy,"\0",128);
					time_j[1] = atoi(strncpy(dummy,buffer+ioff,3));
					strncpy(dummy,"\0",128);
					ioff += 4;
					hr = atoi(strncpy(dummy,buffer+ioff,2));
					strncpy(dummy,"\0",128);
					ioff += 3;
					time_j[2] = atoi(strncpy(dummy,buffer+ioff,2))
						+ 60*hr;
					strncpy(dummy,"\0",128);
					ioff += 3;
					time_j[3] = atoi(strncpy(dummy,buffer+ioff,2));
					time_j[4] = 0;
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
		
					strncpy(NorS,"\0",sizeof(NorS));
					ioff += 7;
					NorS[0] = buffer[ioff];
					ioff += 1;
					strncpy(dummy,"\0",128);
					mlat = atof(strncpy(dummy,buffer+ioff,3));
					strncpy(dummy,"\0",128);
					ioff += 3;
					llat = atof(strncpy(dummy,buffer+ioff,8));
					strncpy(EorW,"\0",sizeof(EorW));
					ioff += 9;
					EorW[0] = buffer[ioff];
					strncpy(dummy,"\0",128);
					ioff += 1;
					mlon = atof(strncpy(dummy,buffer+ioff,4));
					strncpy(dummy,"\0",128);
					ioff += 4;
					llon = atof(strncpy(dummy,buffer+ioff,8));
					n_lon[nrecord] = mlon + llon/60.;
					if (strncmp(EorW,"W",1) == 0)
						n_lon[nrecord] = -n_lon[nrecord];
					n_lat[nrecord] = mlat + llat/60.;
					if (strncmp(NorS,"S",1) == 0)
						n_lat[nrecord] = -n_lat[nrecord];
					n_speed[nrecord] = 0.0;
					nav_ok = MB_YES;
					}
		
				/* deal with nav in real and pseudo NMEA 0183 format */
				else if (merge_nav_format == 6 || merge_nav_format == 7)
					{
					/* check if real sentence */
					len = strlen(buffer);
					if (strncmp(buffer,"$",1) == 0)
					    {
					    if (strncmp(&buffer[3],"DAT",3) == 0
						&& len > 15)
						{
						time_set = MB_NO;
						strncpy(dummy,"\0",128);
						time_i[0] = atoi(strncpy(dummy,buffer+7,4));
						time_i[1] = atoi(strncpy(dummy,buffer+11,2));
						time_i[2] = atoi(strncpy(dummy,buffer+13,2));
						}
					    else if ((strncmp(&buffer[3],"ZDA",3) == 0
						    || strncmp(&buffer[3],"UNX",3) == 0)
						    && len > 14)
						{
						time_set = MB_NO;
						/* find start of ",hhmmss.ss" */
						if ((bufftmp = strchr(buffer, ',')) != NULL)
						    {
						    strncpy(dummy,"\0",128);
						    time_i[3] = atoi(strncpy(dummy,bufftmp+1,2));
						    strncpy(dummy,"\0",128);
						    time_i[4] = atoi(strncpy(dummy,bufftmp+3,2));
						    strncpy(dummy,"\0",128);
						    time_i[5] = atoi(strncpy(dummy,bufftmp+5,2));
						    if (bufftmp[7] == '.')
							{
							strncpy(dummy,"\0",128);
							time_i[6] = 10000*
							    atoi(strncpy(dummy,bufftmp+8,2));
							}
						    else
							time_i[6] = 0;
						    /* find start of ",dd,mm,yyyy" */
						    if ((bufftmp = strchr(&bufftmp[1], ',')) != NULL)
							{
							strncpy(dummy,"\0",128);
							time_i[2] = atoi(strncpy(dummy,bufftmp+1,2));
							strncpy(dummy,"\0",128);
							time_i[1] = atoi(strncpy(dummy,bufftmp+4,2));
							strncpy(dummy,"\0",128);
							time_i[0] = atoi(strncpy(dummy,bufftmp+7,4));
							time_set = MB_YES;
							}
						    }
						}
					    else if (((merge_nav_format == 6 && strncmp(&buffer[3],"GLL",3) == 0)
						|| (merge_nav_format == 7 && strncmp(&buffer[3],"GGA",3) == 0))
						&& time_set == MB_YES && len > 26)
						{
						time_set = MB_NO;
						/* find start of ",ddmm.mm,N,ddmm.mm,E" */
						if ((bufftmp = strchr(buffer, ',')) != NULL)
						    {
						    if (merge_nav_format == 7)
							bufftmp = strchr(&bufftmp[1], ',');
						    strncpy(dummy,"\0",128);
						    degree = atoi(strncpy(dummy,bufftmp+1,2));
						    strncpy(dummy,"\0",128);
						    dminute = atof(strncpy(dummy,bufftmp+3,5));
						    strncpy(NorS,"\0",sizeof(NorS));
						    bufftmp = strchr(&bufftmp[1], ',');
						    strncpy(NorS,bufftmp+1,1);
						    n_lat[nrecord] = degree + dminute/60.;
						    if (strncmp(NorS,"S",1) == 0)
							n_lat[nrecord] = -n_lat[nrecord];
						    bufftmp = strchr(&bufftmp[1], ',');
						    strncpy(dummy,"\0",128);
						    degree = atoi(strncpy(dummy,bufftmp+1,3));
						    strncpy(dummy,"\0",128);
						    dminute = atof(strncpy(dummy,bufftmp+4,5));
						    bufftmp = strchr(&bufftmp[1], ',');
						    strncpy(EorW,"\0",sizeof(EorW));
						    strncpy(EorW,bufftmp+1,1);
						    n_lon[nrecord] = degree + dminute/60.;
						    if (strncmp(EorW,"W",1) == 0)
							n_lon[nrecord] = -n_lon[nrecord];
						    mb_get_time(verbose,time_i,&time_d);
						    n_time_d[nrecord] = time_d;
						    nav_ok = MB_YES;
						    }
						}
					    }
					n_speed[nrecord] = 0.0;
					}
		
				/* deal with nav in Simrad 90 format */
				else if (merge_nav_format == 8)
					{
					mb_get_int(&(time_i[2]), buffer+2,  2);
					mb_get_int(&(time_i[1]), buffer+4,  2);
					mb_get_int(&(time_i[0]), buffer+6,  2);
					mb_fix_y2k(verbose, time_i[0], &time_i[0]);
					mb_get_int(&(time_i[3]), buffer+9,  2);
					mb_get_int(&(time_i[4]), buffer+11, 2);
					mb_get_int(&(time_i[5]), buffer+13, 2);
					mb_get_int(&(time_i[6]), buffer+15, 2);
					time_i[6] = 10000 * time_i[6];
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
		
					mb_get_double(&mlat,    buffer+18,   2);
					mb_get_double(&llat, buffer+20,   7);
					NorS[0] = buffer[27];
					n_lat[nrecord] = mlat + llat/60.0;
					if (NorS[0] == 'S' || NorS[0] == 's')
						n_lat[nrecord] = -n_lat[nrecord];
					mb_get_double(&mlon,    buffer+29,   3);
					mb_get_double(&llon, buffer+32,   7);
					EorW[0] = buffer[39];
					n_lon[nrecord] = mlon + llon/60.0;
					if (EorW[0] == 'W' || EorW[0] == 'w')
						n_lon[nrecord] = -n_lon[nrecord];
					n_speed[nrecord] = 0.0;
					nav_ok = MB_YES;
					}
		
				/* deal with nav in form: yr mon day hour min sec time_d lon lat heading speed sensordepth*/
				else if (merge_nav_format == 9)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_time_d[nrecord],
						&n_lon[nrecord],&n_lat[nrecord],
						&heading,&n_speed[nrecord],&sensordepth,
						&roll,&pitch,&heave);
					if (nget >= 9)
						nav_ok = MB_YES;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1])
						nav_ok = MB_NO;
					}
		
				/* deal with nav in r2rnav form:
					yyyy-mm-ddThh:mm:ss.sssZ decimalLongitude decimalLatitude quality nsat dilution height */
				else if (merge_nav_format == 10)
					{
					nget = sscanf(buffer,"%d-%d-%dT%d:%d:%lfZ %lf %lf %d %d %d %d",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_lon[nrecord],&n_lat[nrecord],
						&quality,&nsatellite,&dilution,&gpsheight);
					if (nget != 12)
						{
						quality = 0;
						nsatellite = 0;
						dilution = 0;
						gpsheight = 0;
						}
					time_i[5] = (int) floor(sec);
					time_i[6] = (int)((sec - time_i[5]) * 1000000);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget >= 8)
						nav_ok = MB_YES;
					}
		
		
				/* make sure longitude is defined according to lonflip */
				if (nav_ok == MB_YES)
					{
					if (merge_nav_lonflip == -1 && n_lon[nrecord] > 0.0)
						n_lon[nrecord] = n_lon[nrecord] - 360.0;
					else if (merge_nav_lonflip == 0 && n_lon[nrecord] < -180.0)
						n_lon[nrecord] = n_lon[nrecord] + 360.0;
					else if (merge_nav_lonflip == 0 && n_lon[nrecord] > 180.0)
						n_lon[nrecord] = n_lon[nrecord] - 360.0;
					else if (merge_nav_lonflip == 1 && n_lon[nrecord] < 0.0)
						n_lon[nrecord] = n_lon[nrecord] + 360.0;
					}
		
				/* output some debug values */
				if (verbose >= 5 && nav_ok == MB_YES)
					{
					fprintf(stderr,"\ndbg5  New navigation point read in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
						nrecord,n_time_d[nrecord],n_lon[nrecord],n_lat[nrecord]);
					}
				else if (verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Error parsing line in navigation file in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       line: %s\n",buffer);
					}
		
				/* check for reverses or repeats in time */
				if (nav_ok == MB_YES)
					{
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord-1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1]
						&& verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  Navigation time error in function <%s>\n",function_name);
						fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
							nrecord-1,n_time_d[nrecord-1],n_lon[nrecord-1],
							n_lat[nrecord-1]);
						fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
							nrecord,n_time_d[nrecord],n_lon[nrecord],
							n_lat[nrecord]);
						}
					}
				strncpy(buffer,"\0",sizeof(buffer));
				}
			
			/* get the good record count */
			*merge_nav_num = nrecord;
			
			/* close the file */
			fclose(tfp);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       merge_nav_num:          %d\n",*merge_nav_num);
		fprintf(stderr,"dbg2       merge_nav_alloc:        %d\n",*merge_nav_alloc);
		fprintf(stderr,"dbg2       merge_nav_time_d *:     %p\n",*merge_nav_time_d);
		fprintf(stderr,"dbg2       merge_nav_lon *:        %p\n",*merge_nav_lon);
		fprintf(stderr,"dbg2       merge_nav_lat *:        %p\n",*merge_nav_lat);
		fprintf(stderr,"dbg2       merge_nav_speed *:      %p\n",*merge_nav_speed);
		fprintf(stderr,"dbg2       error:                  %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                 %d\n",status);
		}

	/* return success */
	return(status);
	}

/*--------------------------------------------------------------------*/

int mb_loadsensordepthdata(int verbose, char *merge_sensordepth_file, int merge_sensordepth_format,
                int *merge_sensordepth_num, int *merge_sensordepth_alloc,
                double **merge_sensordepth_time_d, double **merge_sensordepth_sensordepth,
		int *error)
	{
	char	*function_name = "mb_loadsensordepthdata";
	int	status = MB_SUCCESS;
	char	buffer[MBP_FILENAMESIZE], *result;
	int	nrecord;
	int	nchar, nget;
	size_t	size;
	FILE	*tfp;
	int	sensordepth_ok;
	int	time_i[7], time_j[6], ihr;
	double	sec;
	double	time_d, lon, lat, heading, speed, roll, pitch, heave;
	double	*n_time_d;
	double	*n_sensordepth;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:                           %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:                          %d\n",verbose);
		fprintf(stderr,"dbg2       merge_sensordepth_file:           %s\n",merge_sensordepth_file);
		fprintf(stderr,"dbg2       merge_sensordepth_format:         %d\n",merge_sensordepth_format);
		fprintf(stderr,"dbg2       merge_sensordepth_num *:          %p\n",merge_sensordepth_num);
		fprintf(stderr,"dbg2       merge_sensordepth_num:            %d\n",*merge_sensordepth_num);
		fprintf(stderr,"dbg2       merge_sensordepth_alloc *:        %p\n",merge_sensordepth_alloc);
		fprintf(stderr,"dbg2       merge_sensordepth_alloc:          %d\n",*merge_sensordepth_alloc);
		fprintf(stderr,"dbg2       merge_sensordepth_time_d **:      %p\n",merge_sensordepth_time_d);
		fprintf(stderr,"dbg2       merge_sensordepth_time_d *:       %p\n",*merge_sensordepth_time_d);
		fprintf(stderr,"dbg2       merge_sensordepth_sensordepth **: %p\n",merge_sensordepth_sensordepth);
		fprintf(stderr,"dbg2       merge_sensordepth_sensordepth *:  %p\n",*merge_sensordepth_sensordepth);
		}
	
	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE-1;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_sensordepth_file, "r")) != NULL)
		{
		/* loop over reading the records */
		while ((result = fgets(buffer,nchar,tfp)) == buffer)
			nrecord++;
			
		/* close the file */
		fclose(tfp);
		tfp = NULL;
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_sensordepth_alloc < nrecord)
		{
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_sensordepth_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_sensordepth_sensordepth, error);
		if (status == MB_SUCCESS)
			*merge_sensordepth_alloc = nrecord;
		n_time_d = *merge_sensordepth_time_d;
		n_sensordepth = *merge_sensordepth_sensordepth;
		}

	/* read the records */
	if (status == MB_SUCCESS)
		{
		nrecord = 0;
		if ((tfp = fopen(merge_sensordepth_file, "r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer,nchar,tfp)) == buffer)
				{
				sensordepth_ok = MB_NO;

				/* deal with sensordepth in form: time_d sensordepth */
				if (merge_sensordepth_format == 1)
					{
					nget = sscanf(buffer,"%lf %lf",
						&n_time_d[nrecord],&n_sensordepth[nrecord]);
					if (nget == 2)
						sensordepth_ok = MB_YES;
					}
		
				/* deal with sensordepth in form: yr mon day hour min sec sensordepth */
				else if (merge_sensordepth_format == 2)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_sensordepth[nrecord]);
					time_i[5] = (int) sec;
					time_i[6] = 1000000*(sec - time_i[5]);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						sensordepth_ok = MB_YES;
					}
		
				/* deal with sensordepth in form: yr jday hour min sec sensordepth */
				else if (merge_sensordepth_format == 3)
					{
					nget = sscanf(buffer,"%d %d %d %d %lf %lf",
						&time_j[0],&time_j[1],&ihr,
						&time_j[2],&sec,
						&n_sensordepth[nrecord]);
					time_j[2] = time_j[2] + 60*ihr;
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						sensordepth_ok = MB_YES;
					}
		
				/* deal with sensordepth in form: yr jday daymin sec sensordepth */
				else if (merge_sensordepth_format == 4)
					{
					nget = sscanf(buffer,"%d %d %d %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,
						&n_sensordepth[nrecord]);
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						sensordepth_ok = MB_YES;
					}
		
				/* deal with sensordepth in form: yr mon day hour min sec time_d lon lat heading speed draft*/
				else if (merge_sensordepth_format == 9)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_time_d[nrecord],
						&lon,&lat,
						&heading,&speed,&n_sensordepth[nrecord],
						&roll,&pitch,&heave);
					if (nget >= 9)
						sensordepth_ok = MB_YES;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1])
						sensordepth_ok = MB_NO;
					}
		
				/* output some debug values */
				if (verbose >= 5 && sensordepth_ok == MB_YES)
					{
					fprintf(stderr,"\ndbg5  New sensordepth point read in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       sensordepth[%d]: %f %f\n",
						nrecord,n_time_d[nrecord],n_sensordepth[nrecord]);
					}
				else if (verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Error parsing line in sensordepth file in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       line: %s\n",buffer);
					}
		
				/* check for reverses or repeats in time */
				if (sensordepth_ok == MB_YES)
					{
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord-1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1]
						&& verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  sensordepth time error in function <%s>\n",function_name);
						fprintf(stderr,"dbg5       sensordepth[%d]: %f %f\n",
							nrecord-1,n_time_d[nrecord-1],n_sensordepth[nrecord-1]);
						fprintf(stderr,"dbg5       sensordepth[%d]: %f %f\n",
							nrecord,n_time_d[nrecord],n_sensordepth[nrecord]);
						}
					}
				strncpy(buffer,"\0",sizeof(buffer));
				}
			
			/* get the good record count */
			*merge_sensordepth_num = nrecord;
			
			/* close the file */
			fclose(tfp);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       merge_sensordepth_num:            %d\n",*merge_sensordepth_num);
		fprintf(stderr,"dbg2       merge_sensordepth_alloc:          %d\n",*merge_sensordepth_alloc);
		fprintf(stderr,"dbg2       merge_sensordepth_time_d *:       %p\n",*merge_sensordepth_time_d);
		fprintf(stderr,"dbg2       merge_sensordepth_sensordepth *:  %p\n",*merge_sensordepth_sensordepth);
		fprintf(stderr,"dbg2       error:                            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                           %d\n",status);
		}

	/* return success */
	return(status);
	}

/*--------------------------------------------------------------------*/

int mb_loadheadingdata(int verbose, char *merge_heading_file, int merge_heading_format,
                int *merge_heading_num, int *merge_heading_alloc,
                double **merge_heading_time_d, double **merge_heading_heading,
		int *error)
	{
	char	*function_name = "mb_loadheadingdata";
	int	status = MB_SUCCESS;
	char	buffer[MBP_FILENAMESIZE], *result;
	int	nrecord;
	int	nchar, nget;
	size_t	size;
	FILE	*tfp;
	int	heading_ok;
	int	time_i[7], time_j[6], ihr;
	double	sec;
	double	time_d, lon, lat, sensordepth, speed, roll, pitch, heave;
	double	*n_time_d;
	double	*n_heading;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:                           %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:                          %d\n",verbose);
		fprintf(stderr,"dbg2       merge_heading_file:               %s\n",merge_heading_file);
		fprintf(stderr,"dbg2       merge_heading_format:             %d\n",merge_heading_format);
		fprintf(stderr,"dbg2       merge_heading_num *:              %p\n",merge_heading_num);
		fprintf(stderr,"dbg2       merge_heading_num:                %d\n",*merge_heading_num);
		fprintf(stderr,"dbg2       merge_heading_alloc *:            %p\n",merge_heading_alloc);
		fprintf(stderr,"dbg2       merge_heading_alloc:              %d\n",*merge_heading_alloc);
		fprintf(stderr,"dbg2       merge_heading_time_d **:          %p\n",merge_heading_time_d);
		fprintf(stderr,"dbg2       merge_heading_time_d *:           %p\n",*merge_heading_time_d);
		fprintf(stderr,"dbg2       merge_heading_heading **:         %p\n",merge_heading_heading);
		fprintf(stderr,"dbg2       merge_heading_heading *:          %p\n",*merge_heading_heading);
		}
	
	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE-1;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_heading_file, "r")) != NULL)
		{
		/* loop over reading the records */
		while ((result = fgets(buffer,nchar,tfp)) == buffer)
			nrecord++;
			
		/* close the file */
		fclose(tfp);
		tfp = NULL;
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_heading_alloc < nrecord)
		{
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_heading_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_heading_heading, error);
		if (status == MB_SUCCESS)
			*merge_heading_alloc = nrecord;
		n_time_d = *merge_heading_time_d;
		n_heading = *merge_heading_heading;
		}

	/* read the records */
	if (status == MB_SUCCESS)
		{
		nrecord = 0;
		if ((tfp = fopen(merge_heading_file, "r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer,nchar,tfp)) == buffer)
				{
				heading_ok = MB_NO;

				/* deal with heading in form: time_d heading */
				if (merge_heading_format == 1)
					{
					nget = sscanf(buffer,"%lf %lf",
						&n_time_d[nrecord],&n_heading[nrecord]);
					if (nget == 2)
						heading_ok = MB_YES;
					}
		
				/* deal with heading in form: yr mon day hour min sec heading */
				else if (merge_heading_format == 2)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_heading[nrecord]);
					time_i[5] = (int) sec;
					time_i[6] = 1000000*(sec - time_i[5]);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						heading_ok = MB_YES;
					}
		
				/* deal with heading in form: yr jday hour min sec heading */
				else if (merge_heading_format == 3)
					{
					nget = sscanf(buffer,"%d %d %d %d %lf %lf",
						&time_j[0],&time_j[1],&ihr,
						&time_j[2],&sec,
						&n_heading[nrecord]);
					time_j[2] = time_j[2] + 60*ihr;
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						heading_ok = MB_YES;
					}
		
				/* deal with heading in form: yr jday daymin sec heading */
				else if (merge_heading_format == 4)
					{
					nget = sscanf(buffer,"%d %d %d %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,
						&n_heading[nrecord]);
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						heading_ok = MB_YES;
					}
		
				/* deal with heading in form: yr mon day hour min sec time_d lon lat heading speed draft*/
				else if (merge_heading_format == 9)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_time_d[nrecord],
						&lon,&lat,
						&n_heading[nrecord],&speed,&sensordepth,
						&roll,&pitch,&heave);
					if (nget >= 9)
						heading_ok = MB_YES;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1])
						heading_ok = MB_NO;
					}
		
				/* output some debug values */
				if (verbose >= 5 && heading_ok == MB_YES)
					{
					fprintf(stderr,"\ndbg5  New heading point read in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       heading[%d]: %f %f\n",
						nrecord,n_time_d[nrecord],n_heading[nrecord]);
					}
				else if (verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Error parsing line in heading file in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       line: %s\n",buffer);
					}
		
				/* check for reverses or repeats in time */
				if (heading_ok == MB_YES)
					{
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord-1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1]
						&& verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  heading time error in function <%s>\n",function_name);
						fprintf(stderr,"dbg5       heading[%d]: %f %f\n",
							nrecord-1,n_time_d[nrecord-1],n_heading[nrecord-1]);
						fprintf(stderr,"dbg5       heading[%d]: %f %f\n",
							nrecord,n_time_d[nrecord],n_heading[nrecord]);
						}
					}
				strncpy(buffer,"\0",sizeof(buffer));
				}
			
			/* get the good record count */
			*merge_heading_num = nrecord;
			
			/* close the file */
			fclose(tfp);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       merge_heading_num:                %d\n",*merge_heading_num);
		fprintf(stderr,"dbg2       merge_heading_alloc:              %d\n",*merge_heading_alloc);
		fprintf(stderr,"dbg2       merge_heading_time_d *:           %p\n",*merge_heading_time_d);
		fprintf(stderr,"dbg2       merge_heading_heading *:          %p\n",*merge_heading_heading);
		fprintf(stderr,"dbg2       error:                            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                           %d\n",status);
		}

	/* return success */
	return(status);
	}

/*--------------------------------------------------------------------*/

int mb_loadattitudedata(int verbose, char *merge_attitude_file, int merge_attitude_format,
                int *merge_attitude_num, int *merge_attitude_alloc,
                double **merge_attitude_time_d, double **merge_attitude_roll,
		double **merge_attitude_pitch, double **merge_attitude_heave,
		int *error)
	{
	char	*function_name = "mb_loadattitudedata";
	int	status = MB_SUCCESS;
	char	buffer[MBP_FILENAMESIZE], *result;
	int	nrecord;
	int	nchar, nget;
	size_t	size;
	FILE	*tfp;
	int	attitude_ok;
	int	time_i[7], time_j[6], ihr;
	double	sec;
	double	time_d, lon, lat, sensordepth, heading, speed;
	double	*n_time_d;
	double	*n_roll;
	double	*n_pitch;
	double	*n_heave;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:                           %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:                          %d\n",verbose);
		fprintf(stderr,"dbg2       merge_attitude_file:              %s\n",merge_attitude_file);
		fprintf(stderr,"dbg2       merge_attitude_format:            %d\n",merge_attitude_format);
		fprintf(stderr,"dbg2       merge_attitude_num *:             %p\n",merge_attitude_num);
		fprintf(stderr,"dbg2       merge_attitude_num:               %d\n",*merge_attitude_num);
		fprintf(stderr,"dbg2       merge_attitude_alloc *:           %p\n",merge_attitude_alloc);
		fprintf(stderr,"dbg2       merge_attitude_alloc:             %d\n",*merge_attitude_alloc);
		fprintf(stderr,"dbg2       merge_attitude_time_d **:         %p\n",merge_attitude_time_d);
		fprintf(stderr,"dbg2       merge_attitude_time_d *:          %p\n",*merge_attitude_time_d);
		fprintf(stderr,"dbg2       merge_attitude_roll **:           %p\n",merge_attitude_roll);
		fprintf(stderr,"dbg2       merge_attitude_roll *:            %p\n",*merge_attitude_roll);
		fprintf(stderr,"dbg2       merge_attitude_pitch **:          %p\n",merge_attitude_pitch);
		fprintf(stderr,"dbg2       merge_attitude_pitch *:           %p\n",*merge_attitude_pitch);
		fprintf(stderr,"dbg2       merge_attitude_heave **:          %p\n",merge_attitude_heave);
		fprintf(stderr,"dbg2       merge_attitude_heave *:           %p\n",*merge_attitude_heave);
		}
	
	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE-1;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_attitude_file, "r")) != NULL)
		{
		/* loop over reading the records */
		while ((result = fgets(buffer,nchar,tfp)) == buffer)
			nrecord++;
			
		/* close the file */
		fclose(tfp);
		tfp = NULL;
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_attitude_alloc < nrecord)
		{
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_roll, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_pitch, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_heave, error);
		if (status == MB_SUCCESS)
			*merge_attitude_alloc = nrecord;
		n_time_d = *merge_attitude_time_d;
		n_roll = *merge_attitude_roll;
		n_pitch = *merge_attitude_pitch;
		n_heave = *merge_attitude_heave;
		}

	/* read the records */
	if (status == MB_SUCCESS)
		{
		nrecord = 0;
		if ((tfp = fopen(merge_attitude_file, "r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer,nchar,tfp)) == buffer)
				{
				attitude_ok = MB_NO;

				/* deal with attitude in form: time_d roll pitch heave */
				if (merge_attitude_format == 1)
					{
					nget = sscanf(buffer,"%lf %lf %lf %lf",
						&n_time_d[nrecord],&n_roll[nrecord],&n_pitch[nrecord],&n_heave[nrecord]);
					if (nget == 4)
						attitude_ok = MB_YES;
					}
		
				/* deal with attitude in form: yr mon day hour min sec roll pitch heave */
				else if (merge_attitude_format == 2)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_roll[nrecord],&n_pitch[nrecord],&n_heave[nrecord]);
					time_i[5] = (int) sec;
					time_i[6] = 1000000*(sec - time_i[5]);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 9)
						attitude_ok = MB_YES;
					}
		
				/* deal with attitude in form: yr jday hour min sec roll pitch heave */
				else if (merge_attitude_format == 3)
					{
					nget = sscanf(buffer,"%d %d %d %d %lf %lf %lf %lf",
						&time_j[0],&time_j[1],&ihr,
						&time_j[2],&sec,
						&n_roll[nrecord],&n_pitch[nrecord],&n_heave[nrecord]);
					time_j[2] = time_j[2] + 60*ihr;
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 8)
						attitude_ok = MB_YES;
					}
		
				/* deal with attitude in form: yr jday daymin sec roll pitch heave */
				else if (merge_attitude_format == 4)
					{
					nget = sscanf(buffer,"%d %d %d %lf %lf %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,&n_roll[nrecord],&n_pitch[nrecord],&n_heave[nrecord]);
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						attitude_ok = MB_YES;
					}
		
				/* deal with attitude in form: yr mon day hour min sec time_d lon lat heading speed sensordepth roll pitch heave */
				else if (merge_attitude_format == 9)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_time_d[nrecord],
						&lon,&lat,
						&heading,&speed,&sensordepth,
						&n_roll[nrecord],&n_pitch[nrecord],&n_heave[nrecord]);
					if (nget >= 9)
						attitude_ok = MB_YES;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1])
						attitude_ok = MB_NO;
					}
		
				/* output some debug values */
				if (verbose >= 5 && attitude_ok == MB_YES)
					{
					fprintf(stderr,"\ndbg5  New attitude point read in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       attitude[%d]: %f %f %f %f\n",
						nrecord,n_time_d[nrecord],n_roll[nrecord],n_pitch[nrecord],n_heave[nrecord]);
					}
				else if (verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Error parsing line in attitude file in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       line: %s\n",buffer);
					}
		
				/* check for reverses or repeats in time */
				if (attitude_ok == MB_YES)
					{
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord-1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1]
						&& verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  attitude time error in function <%s>\n",function_name);
						fprintf(stderr,"dbg5       attitude[%d]: %f %f %f %f\n",
							nrecord-1,n_time_d[nrecord-1],n_roll[nrecord-1],n_pitch[nrecord-1],n_heave[nrecord-1]);
						fprintf(stderr,"dbg5       attitude[%d]: %f %f %f %f\n",
							nrecord,n_time_d[nrecord],n_roll[nrecord],n_pitch[nrecord],n_heave[nrecord]);
						}
					}
				strncpy(buffer,"\0",sizeof(buffer));
				}
			
			/* get the good record count */
			*merge_attitude_num = nrecord;
			
			/* close the file */
			fclose(tfp);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       merge_attitude_num:               %d\n",*merge_attitude_num);
		fprintf(stderr,"dbg2       merge_attitude_alloc:             %d\n",*merge_attitude_alloc);
		fprintf(stderr,"dbg2       merge_attitude_time_d *:          %p\n",*merge_attitude_time_d);
		fprintf(stderr,"dbg2       merge_attitude_roll *:            %p\n",*merge_attitude_roll);
		fprintf(stderr,"dbg2       merge_attitude_pitch *:           %p\n",*merge_attitude_pitch);
		fprintf(stderr,"dbg2       merge_attitude_heave *:           %p\n",*merge_attitude_heave);
		fprintf(stderr,"dbg2       error:                            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                           %d\n",status);
		}

	/* return success */
	return(status);	
	}

/*--------------------------------------------------------------------*/

int mb_loadtimeshiftdata(int verbose, char *merge_timeshift_file, int merge_timeshift_format,
                int *merge_timeshift_num, int *merge_timeshift_alloc,
                double **merge_timeshift_time_d, double **merge_timeshift_timeshift,
		int *error)
	{
	char	*function_name = "mb_loadtimeshiftdata";
	int	status = MB_SUCCESS;
	char	buffer[MBP_FILENAMESIZE], *result;
	int	nrecord;
	int	nchar, nget;
	size_t	size;
	FILE	*tfp;
	int	timeshift_ok;
	int	time_i[7], time_j[6], ihr;
	double	sec;
	double	time_d;
	double	*n_time_d;
	double	*n_timeshift;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:                           %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:                          %d\n",verbose);
		fprintf(stderr,"dbg2       merge_timeshift_file:             %s\n",merge_timeshift_file);
		fprintf(stderr,"dbg2       merge_timeshift_format:           %d\n",merge_timeshift_format);
		fprintf(stderr,"dbg2       merge_timeshift_num *:            %p\n",merge_timeshift_num);
		fprintf(stderr,"dbg2       merge_timeshift_num:              %d\n",*merge_timeshift_num);
		fprintf(stderr,"dbg2       merge_timeshift_alloc *:          %p\n",merge_timeshift_alloc);
		fprintf(stderr,"dbg2       merge_timeshift_alloc:            %d\n",*merge_timeshift_alloc);
		fprintf(stderr,"dbg2       merge_timeshift_time_d **:        %p\n",merge_timeshift_time_d);
		fprintf(stderr,"dbg2       merge_timeshift_time_d *:         %p\n",*merge_timeshift_time_d);
		fprintf(stderr,"dbg2       merge_timeshift_timeshift **:     %p\n",merge_timeshift_timeshift);
		fprintf(stderr,"dbg2       merge_timeshift_timeshift *:      %p\n",*merge_timeshift_timeshift);
		}
	
	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE-1;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_timeshift_file, "r")) != NULL)
		{
		/* loop over reading the records */
		while ((result = fgets(buffer,nchar,tfp)) == buffer)
			nrecord++;
			
		/* close the file */
		fclose(tfp);
		tfp = NULL;
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_timeshift_alloc < nrecord)
		{
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_timeshift_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_timeshift_timeshift, error);
		if (status == MB_SUCCESS)
			*merge_timeshift_alloc = nrecord;
		n_time_d = *merge_timeshift_time_d;
		n_timeshift = *merge_timeshift_timeshift;
		}

	/* read the records */
	if (status == MB_SUCCESS)
		{
		nrecord = 0;
		if ((tfp = fopen(merge_timeshift_file, "r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer,nchar,tfp)) == buffer)
				{
				timeshift_ok = MB_NO;

				/* deal with timeshift in form: time_d timeshift */
				if (merge_timeshift_format == 1)
					{
					nget = sscanf(buffer,"%lf %lf",
						&n_time_d[nrecord],&n_timeshift[nrecord]);
					if (nget == 2)
						timeshift_ok = MB_YES;
					}
		
				/* deal with timeshift in form: yr mon day hour min sec timeshift */
				else if (merge_timeshift_format == 2)
					{
					nget = sscanf(buffer,"%d %d %d %d %d %lf %lf",
						&time_i[0],&time_i[1],&time_i[2],
						&time_i[3],&time_i[4],&sec,
						&n_timeshift[nrecord]);
					time_i[5] = (int) sec;
					time_i[6] = 1000000*(sec - time_i[5]);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						timeshift_ok = MB_YES;
					}
		
				/* deal with timeshift in form: yr jday hour min sec timeshift */
				else if (merge_timeshift_format == 3)
					{
					nget = sscanf(buffer,"%d %d %d %d %lf %lf",
						&time_j[0],&time_j[1],&ihr,
						&time_j[2],&sec,
						&n_timeshift[nrecord]);
					time_j[2] = time_j[2] + 60*ihr;
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						timeshift_ok = MB_YES;
					}
		
				/* deal with timeshift in form: yr jday daymin sec timeshift */
				else if (merge_timeshift_format == 4)
					{
					nget = sscanf(buffer,"%d %d %d %lf %lf",
						&time_j[0],&time_j[1],&time_j[2],
						&sec,
						&n_timeshift[nrecord]);
					time_j[3] = (int) sec;
					time_j[4] = 1000000*(sec - time_j[3]);
					mb_get_itime(verbose,time_j,time_i);
					mb_get_time(verbose,time_i,&time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						timeshift_ok = MB_YES;
					}
		
				/* output some debug values */
				if (verbose >= 5 && timeshift_ok == MB_YES)
					{
					fprintf(stderr,"\ndbg5  New timeshift point read in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       timeshift[%d]: %f %f\n",
						nrecord,n_time_d[nrecord],n_timeshift[nrecord]);
					}
				else if (verbose >= 5)
					{
					fprintf(stderr,"\ndbg5  Error parsing line in timeshift file in function <%s>\n",function_name);
					fprintf(stderr,"dbg5       line: %s\n",buffer);
					}
		
				/* check for reverses or repeats in time */
				if (timeshift_ok == MB_YES)
					{
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord-1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord-1]
						&& verbose >= 5)
						{
						fprintf(stderr,"\ndbg5  timeshift time error in function <%s>\n",function_name);
						fprintf(stderr,"dbg5       timeshift[%d]: %f %f\n",
							nrecord-1,n_time_d[nrecord-1],n_timeshift[nrecord-1]);
						fprintf(stderr,"dbg5       timeshift[%d]: %f %f\n",
							nrecord,n_time_d[nrecord],n_timeshift[nrecord]);
						}
					}
				strncpy(buffer,"\0",sizeof(buffer));
				}
			
			/* get the good record count */
			*merge_timeshift_num = nrecord;
			
			/* close the file */
			fclose(tfp);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       merge_timeshift_num:              %d\n",*merge_timeshift_num);
		fprintf(stderr,"dbg2       merge_timeshift_alloc:            %d\n",*merge_timeshift_alloc);
		fprintf(stderr,"dbg2       merge_timeshift_time_d *:         %p\n",*merge_timeshift_time_d);
		fprintf(stderr,"dbg2       merge_timeshift_timeshift *:      %p\n",*merge_timeshift_timeshift);
		fprintf(stderr,"dbg2       error:                            %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                           %d\n",status);
		}

	/* return success */
	return(status);
	}

/*--------------------------------------------------------------------*/
