/*--------------------------------------------------------------------
 *    The MB-system:	mb_time.c	10/30/2000
 *    $Id: mb_navint.c,v 5.14 2008/09/27 03:27:10 caress Exp $
 *
 *    Copyright (c) 2000-2009 by
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
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"

/*  #define MB_NAVINT_DEBUG 1
    #define MB_ATTINT_DEBUG 1
    #define MB_HEDINT_DEBUG 1
    #define MB_DEPINT_DEBUG 1 
    #define MB_ALTINT_DEBUG 1 */

static char rcs_id[]="$Id: mb_navint.c,v 5.14 2008/09/27 03:27:10 caress Exp $";

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	nshift;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
			nshift++;
			}
		while (time_d < mb_io_ptr->fix_time_d[ifix-1])
			{
			ifix--;
			nshift++;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
	fprintf(stderr, "mb_attint_add:    Attitude fix %d %f %f %f added\n", 
				mb_io_ptr->nattitude, roll, pitch, heave);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
	fprintf(stderr, "mb_attint_add:    Attitude fix %d of %d: %f %f %f added\n", 
				i, mb_io_ptr->nattitude, roll[i], pitch[i], heave[i]);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
	fprintf(stderr, "mb_attint_interp: Attitude %f %f %f interpolated at fix %d of %d with factor:%f\n", 
		*roll, *pitch, *heave, ifix, mb_io_ptr->nattitude, factor);
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
	fprintf(stderr, "mb_attint_interp: Attitude %f %f %f extrapolated from last fix of %d\n", 
		*roll, *pitch, *heave, mb_io_ptr->nattitude);
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
	fprintf(stderr, "mb_attint_interp: Attitude %f %f %f extrapolated from first fix of %d\n", 
		*roll, *pitch, *heave, mb_io_ptr->nattitude);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       rcs_id:     %s\n",rcs_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
