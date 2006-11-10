/*--------------------------------------------------------------------
 *    The MB-system:	mb_access.c	11/1/00
 *    $Id: mb_access.c,v 5.14 2006-11-10 22:36:04 caress Exp $

 *    Copyright (c) 2000, 2002, 2003 by
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
 * This source file includes the functions used to extract data from
 * and insert data into sonar specific structures.
 *
 * Author:	D. W. Caress
 * Date:	October 1, 2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.13  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.12  2004/12/02 06:33:32  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.11  2004/09/16 19:02:33  caress
 * Changes to better support segy data.
 *
 * Revision 5.10  2004/06/18 03:07:15  caress
 * Adding support for segy i/o and working on support for Reson 7k format 88.
 *
 * Revision 5.9  2004/05/21 23:46:22  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 * Revision 5.8  2004/04/27 01:46:13  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.7  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.5  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.4  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.3  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22 20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
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
#include "../../include/mb_segy.h"

static char rcs_id[]="$Id: mb_access.c,v 5.14 2006-11-10 22:36:04 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_alloc(int verbose, void *mbio_ptr,
		    void **store_ptr, int *error)
{
	char	*function_name = "mb_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call appropriate memory allocation routine */
	if (mb_io_ptr->mb_io_store_alloc != NULL)
		{
		status = (*mb_io_ptr->mb_io_store_alloc)
				(verbose,mbio_ptr,store_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_deall(int verbose, void *mbio_ptr,
			void **store_ptr, int *error)
{
	char	*function_name = "mb_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call appropriate memory deallocation routine */
	if (mb_io_ptr->mb_io_store_free != NULL)
		{
		status = (*mb_io_ptr->mb_io_store_free)
				(verbose,mbio_ptr,store_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
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
int mb_get_store(int verbose, void *mbio_ptr,
		    void **store_ptr, int *error)
{
	char	*function_name = "mb_get_store";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store pointer */
	*store_ptr = (void *) mb_io_ptr->store_data;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mb_dimensions";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract != NULL)
		{
		status = (*mb_io_ptr->mb_io_dimensions)
				(verbose, mbio_ptr, store_ptr, 
				kind, nbath, namp, nss, error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		fprintf(stderr,"dbg2       namp:       %d\n",*namp);
		fprintf(stderr,"dbg2       nss:        %d\n",*nss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_pingnumber(int verbose, void *mbio_ptr, 
		int *pingnumber, int *error)
{
	char	*function_name = "mb_pingnumber";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

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

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_pingnumber != NULL)
		{
		status = (*mb_io_ptr->mb_io_pingnumber)
				(verbose, mbio_ptr, 
				pingnumber, error);
		}
	else
		{
		*pingnumber = mb_io_ptr->ping_count;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pingnumber: %d\n",*pingnumber);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mb_extract";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract)
				(verbose, mbio_ptr, store_ptr, 
				kind, time_i, time_d,
				navlon, navlat,
				speed, heading,
				nbath, namp, nss,
				beamflag, bath, amp, 
				bathacrosstrack, bathalongtrack,
				ss, ssacrosstrack, ssalongtrack,
				comment, error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}
		
	/* apply projection and lonflip if necessary */
	if (status == MB_SUCCESS)
		{
		/* apply inverse projection if required */
		if (mb_io_ptr->projection_initialized == MB_YES)
			{
			easting = *navlon;
			northing = *navlat;
			mb_proj_inverse(verbose, mb_io_ptr->pjptr, 
							easting, northing,
							navlon, navlat,
							error);
			}
		
		/* apply lonflip */
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
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
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",
			*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        namp:     %d\n",
			*namp);
		for (i=0;i<*namp;i++)
		  fprintf(stderr,"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:      %d\n",
			*nss);
		for (i=0;i<*nss;i++)
		  fprintf(stderr,"dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
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
int mb_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mb_insert";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV))
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		}
	if (verbose >= 2 && kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3) 
		 for (i=0;i<namp;i++)
		  fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3) 
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	
	/* check that io arrays are large enough, allocate larger arrays if necessary */
	if (nbath > mb_io_ptr->beams_bath_alloc
		|| namp > mb_io_ptr->beams_amp_alloc
		|| nss > mb_io_ptr->pixels_ss_alloc)
		{
		status = mb_update_arrays(verbose, mbio_ptr,
			nbath, namp, nss, error);
		}
	mb_io_ptr->beams_bath_max = MAX(mb_io_ptr->beams_bath_max, nbath);
	mb_io_ptr->beams_amp_max = MAX(mb_io_ptr->beams_amp_max, namp);
	mb_io_ptr->pixels_ss_max = MAX(mb_io_ptr->pixels_ss_max, nss);
		
	/* apply inverse projection if required */
	if (mb_io_ptr->projection_initialized == MB_YES)
		{
		mb_proj_forward(verbose, mb_io_ptr->pjptr, 
						navlon, navlat,
						&easting, &northing,
						error);
		navlon = easting;
		navlat = northing;
		}

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert)
				(verbose, mbio_ptr, store_ptr, 
				kind, time_i, time_d,
				navlon, navlat,
				speed, heading,
				nbath, namp, nss,
				beamflag, bath, amp, 
				bathacrosstrack, bathalongtrack,
				ss, ssacrosstrack, ssalongtrack,
				comment, error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind,
	int time_i[7], double *time_d, 
	double *navlon, double *navlat,
	double *speed, double *heading, double *draft, 
	double *roll, double *pitch, double *heave, 
	int *error)
{
	char	*function_name = "mb_extract_nav";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_nav != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_nav)
				(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}
		
	/* apply projection and lonflip if necessary */
	if (status == MB_SUCCESS)
		{
		/* apply inverse projection if required */
		if (mb_io_ptr->projection_initialized == MB_YES)
			{
			easting = *navlon;
			northing = *navlat;
			mb_proj_inverse(verbose, mb_io_ptr->pjptr, 
							easting, northing,
							navlon, navlat,
							error);
			}
		
		/* apply lonflip */
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
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
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, 
	int nmax, int *kind, int *n,
	int *time_i, double *time_d, 
	double *navlon, double *navlat,
	double *speed, double *heading, double *draft, 
	double *roll, double *pitch, double *heave, 
	int *error)
{
	char	*function_name = "mb_extract_nnav";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;
	int	i, inav;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_nnav != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_nnav)
				(verbose,mbio_ptr,store_ptr,
				nmax,kind,n,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (mb_io_ptr->mb_io_extract_nav != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_nav)
				(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		if (status == MB_SUCCESS)
			*n = 1;
		else
			*n = 0;
		}
	else
		{
		*n = 0;
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}
		
	/* apply projection and lonflip if necessary */
	if (status == MB_SUCCESS)
		{
		for (inav=0;inav<*n;inav++)
			{
			/* apply inverse projection if required */
			if (mb_io_ptr->projection_initialized == MB_YES)
				{
				easting = navlon[inav];
				northing = navlat[inav];
				mb_proj_inverse(verbose, mb_io_ptr->pjptr, 
								easting, northing,
								&(navlon[inav]), &(navlat[inav]),
								error);
				}

			/* apply lonflip */
			if (mb_io_ptr->lonflip < 0)
				{
				if (navlon[inav] > 0.) 
					navlon[inav] = navlon[inav] - 360.;
				else if (navlon[inav] < -360.)
					navlon[inav] = navlon[inav] + 360.;
				}
			else if (mb_io_ptr->lonflip == 0)
				{
				if (navlon[inav] > 180.) 
					navlon[inav] = navlon[inav] - 360.;
				else if (navlon[inav] < -180.)
					navlon[inav] = navlon[inav] + 360.;
				}
			else
				{
				if (navlon[inav] > 360.) 
					navlon[inav] = navlon[inav] - 360.;
				else if (navlon[inav] < 0.)
					navlon[inav] = navlon[inav] + 360.;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       n:          %d\n",*n);
		for (inav=0;inav<*n;inav++)
			{
			for (i=0;i<7;i++)
				fprintf(stderr,"dbg2       %d time_i[%d]:     %d\n",inav,i,time_i[inav * 7 + i]);
			fprintf(stderr,"dbg2       %d time_d:        %f\n",inav,time_d[inav]);
			fprintf(stderr,"dbg2       %d longitude:     %f\n",inav,navlon[inav]);
			fprintf(stderr,"dbg2       %d latitude:      %f\n",inav,navlat[inav]);
			fprintf(stderr,"dbg2       %d speed:         %f\n",inav,speed[inav]);
			fprintf(stderr,"dbg2       %d heading:       %f\n",inav,heading[inav]);
			fprintf(stderr,"dbg2       %d draft:         %f\n",inav,draft[inav]);
			fprintf(stderr,"dbg2       %d roll:          %f\n",inav,roll[inav]);
			fprintf(stderr,"dbg2       %d pitch:         %f\n",inav,pitch[inav]);
			fprintf(stderr,"dbg2       %d heave:         %f\n",inav,heave[inav]);
			}
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
	int time_i[7], double time_d, 
	double navlon, double navlat,
	double speed, double heading, double draft, 
	double roll, double pitch, double heave, 
	int *error)
{
	char	*function_name = "mb_insert_nav";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	double	easting, northing;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:        %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:     %d\n",store_ptr);
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",speed);
		fprintf(stderr,"dbg2       heading:       %f\n",heading);
		fprintf(stderr,"dbg2       draft:         %f\n",draft);
		fprintf(stderr,"dbg2       roll:          %f\n",roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
		
	/* apply inverse projection if required */
	if (mb_io_ptr->projection_initialized == MB_YES)
		{
		mb_proj_forward(verbose, mb_io_ptr->pjptr, 
						navlon, navlat,
						&easting, &northing,
						error);
		navlon = easting;
		navlat = northing;
		}

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert_nav != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert_nav)
				(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error)
{
	char	*function_name = "mb_extract_altitude";
	int	status;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_altitude != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_altitude)
				(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",*transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",*altitude);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
		double transducer_depth, double altitude,
		int *error)
{
	char	*function_name = "mb_insert_altitude";
	int	status;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert_altitude != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert_altitude)
				(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mb_extract_svp";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_svp != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_svp)
				(verbose,mbio_ptr,store_ptr,
				kind,nsvp,depth,velocity,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       nsvp:              %d\n",*nsvp);
		for (i=0;i<*nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mb_insert_svp";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
		fprintf(stderr,"dbg2       nsvp:              %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert_svp != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert_svp)
				(verbose,mbio_ptr,store_ptr,
				nsvp,depth,velocity,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * mb_ttimes() calls the appropriate mbsys_ routine for 
 * extracting travel times and  beam angles from
 * a stored survey data ping.
 * 
 * The coordinates of the beam angles can be a bit confusing.
 * The angles are returned in "takeoff angle coordinates"
 * appropriate for raytracing. The array angles contains the
 * angle from vertical (theta below) and the array angles_forward
 * contains the angle from acrosstrack (phi below). This 
 * coordinate system is distinct from the roll-pitch coordinates
 * appropriate for correcting roll and pitch values. The following
 * is a description of these relevent coordinate systems:
 * 
 * Notes on Coordinate Systems used in MB-System
 * 
 * David W. Caress
 * April 22, 1996
 * R/V Maurice Ewing, EW9602
 * 
 * I. Introduction
 * ---------------
 * The coordinate systems described below are used
 * within MB-System for calculations involving
 * the location in space of depth, amplitude, or
 * sidescan data. In all cases the origin of the
 * coordinate system is at the center of the sonar 
 * transducers.
 * 
 * II. Cartesian Coordinates
 * -------------------------
 * The cartesian coordinate system used in MB-System
 * is a bit odd because it is left-handed, as opposed
 * to the right-handed x-y-z space conventionally
 * used in most circumstances. With respect to the
 * sonar (or the ship on which the sonar is mounted),
 * the x-axis is athwartships with positive to starboard
 * (to the right if facing forward), the y-axis is
 * fore-aft with positive forward, and the z-axis is
 * positive down.
 * 
 * III. Spherical Coordinates
 * --------------------------
 * There are two non-traditional spherical coordinate 
 * systems used in MB-System. The first, referred to here 
 * as takeoff angle coordinates, is useful for raytracing.
 * The second, referred to here as roll-pitch 
 * coordinates, is useful for taking account of 
 * corrections to roll and pitch angles.
 * 
 * 1. Takeoff Angle Coordinates
 * ----------------------------
 * The three parameters are r, theta, and phi, where
 * r is the distance from the origin, theta is the
 * angle from vertical down (that is, from the 
 * positive z-axis), and phi is the angle from 
 * acrosstrack (the positive x-axis) in the x-y plane.
 * Note that theta is always positive; the direction
 * in the x-y plane is given by phi.
 * Raytracing is simple in these coordinates because
 * the ray takeoff angle is just theta. However,
 * applying roll or pitch corrections is complicated because
 * roll and pitch have components in both theta and phi.
 * 
 * 	0 <= theta <= PI/2
 * 	-PI/2 <= phi <= 3*PI/2
 * 
 * 	x = r * SIN(theta) * COS(phi) 
 * 	y = r * SIN(theta) * SIN(phi)
 * 	z = r * COS(theta) 
 * 	
 * 	theta = 0    ---> vertical, along positive z-axis
 * 	theta = PI/2 ---> horizontal, in x-y plane
 * 	phi = -PI/2  ---> aft, in y-z plane with y negative
 * 	phi = 0      ---> port, in x-z plane with x positive
 * 	phi = PI/2   ---> forward, in y-z plane with y positive
 * 	phi = PI     ---> starboard, in x-z plane with x negative
 * 	phi = 3*PI/2 ---> aft, in y-z plane with y negative
 * 
 * 2. Roll-Pitch Coordinates
 * -------------------------
 * The three parameters are r, alpha, and beta, where
 * r is the distance from the origin, alpha is the angle 
 * forward (effectively pitch angle), and beta is the
 * angle from horizontal in the x-z plane (effectively
 * roll angle). Applying a roll or pitch correction is 
 * simple in these coordinates because pitch is just alpha 
 * and roll is just beta. However, raytracing is complicated 
 * because deflection from vertical has components in both 
 * alpha and beta.
 * 
 * 	-PI/2 <= alpha <= PI/2
 * 	0 <= beta <= PI
 * 	
 * 	x = r * COS(alpha) * COS(beta) 
 * 	y = r * SIN(alpha)
 * 	z = r * COS(alpha) * SIN(beta) 
 * 	
 * 	alpha = -PI/2 ---> horizontal, in x-y plane with y negative
 * 	alpha = 0     ---> ship level, zero pitch, in x-z plane
 * 	alpha = PI/2  ---> horizontal, in x-y plane with y positive
 * 	beta = 0      ---> starboard, along positive x-axis
 * 	beta = PI/2   ---> in y-z plane rotated by alpha
 * 	beta = PI     ---> port, along negative x-axis
 * 
 * IV. SeaBeam Coordinates
 * ----------------------
 * The per-beam parameters in the SB2100 data format include
 * angle-from-vertical and angle-forward. Angle-from-vertical
 * is the same as theta except that it is signed based on
 * the acrosstrack direction (positive to starboard, negative 
 * to port). The angle-forward values are also defined 
 * slightly differently from phi, in that angle-forward is 
 * signed differently on the port and starboard sides. The 
 * SeaBeam 2100 External Interface Specifications document 
 * includes both discussion and figures illustrating the 
 * angle-forward value. To summarize:
 * 
 *     Port:
 *     
 * 	theta = absolute value of angle-from-vertical
 * 	
 * 	-PI/2 <= phi <= PI/2  
 * 	is equivalent to 
 * 	-PI/2 <= angle-forward <= PI/2
 * 	
 * 	phi = -PI/2 ---> angle-forward = -PI/2 (aft)
 * 	phi = 0     ---> angle-forward = 0     (starboard)
 * 	phi = PI/2  ---> angle-forward = PI/2  (forward)
 * 
 *     Starboard:
 * 	
 * 	theta = angle-from-vertical
 *     
 * 	PI/2 <= phi <= 3*PI/2 
 * 	is equivalent to 
 * 	-PI/2 <= angle-forward <= PI/2
 * 	
 * 	phi = PI/2   ---> angle-forward = -PI/2 (forward)
 * 	phi = PI     ---> angle-forward = 0     (port)
 * 	phi = 3*PI/2 ---> angle-forward = PI/2  (aft)
 * 
 * V. Usage of Coordinate Systems in MB-System
 * ------------------------------------------
 * Some sonar data formats provide angle values along with
 * travel times. The angles are converted to takoff-angle 
 * coordinates regardless of the  storage form of the 
 * particular data format. Currently, most data formats
 * do not contain an alongtrack component to the position
 * values; in these cases the conversion is trivial since
 * phi = beta = 0 and theta = alpha. The angle and travel time 
 * values can be accessed using the MBIO function mb_ttimes.
 * All angle values passed by MB-System functions are in
 * degrees rather than radians.
 * 
 * The programs mbbath and mbvelocitytool use angles in
 * take-off angle coordinates to do the raytracing. If roll
 * and/or pitch corrections are to be made, the angles are
 * converted to roll-pitch coordinates, corrected, and then
 * converted back prior to raytracing.
 * 
 * The SeaBeam patch test tool SeaPatch calculates angles
 * in roll-pitch coordinates from the initial bathymetry
 * and then applies whatever roll and pitch corrections are
 * set interactively by the user.
 * 
*
 */
/*--------------------------------------------------------------------*/
int mb_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double	*angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mb_ttimes";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_ttimes != NULL)
		{
		status = (*mb_io_ptr->mb_io_ttimes)
				(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				heave,alongtrack_offset,
				draft,ssv,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       draft:      %f\n",*draft);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n",
				i,ttimes[i],angles[i],
				angles_forward[i],angles_null[i],
				heave[i],alongtrack_offset[i]);
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
int mb_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mb_detects";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_detects != NULL)
		{
		status = (*mb_io_ptr->mb_io_detects)
				(verbose,mbio_ptr,store_ptr,
				kind,nbeams,detects,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: detects:%d\n",
				i,detects[i]);
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
int mb_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error)
{
	char	*function_name = "mb_extract_rawss";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_rawss != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_rawss)
				(verbose,mbio_ptr,store_ptr,
				kind,nrawss,rawss,
				rawssacrosstrack,rawssalongtrack,
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       nrawss:            %d\n",*nrawss);
		for (i=0;i<*nrawss;i++)
		    fprintf(stderr,"dbg2       sample: %d  rawss:%f  %f  %f\n",
			    i, rawss[i], rawssacrosstrack[i], rawssalongtrack[i]);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
		int nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error)
{
	char	*function_name = "mb_insert_rawss";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
		fprintf(stderr,"dbg2       nrawss:            %d\n",nrawss);
		for (i=0;i<nrawss;i++)
		    fprintf(stderr,"dbg2       sample: %d  rawss:%f  %f  %f\n",
			    i, rawss[i], rawssacrosstrack[i], rawssalongtrack[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert_rawss != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert_rawss)
				(verbose,mbio_ptr,store_ptr,
				nrawss,rawss,
				rawssacrosstrack,rawssalongtrack,
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		void *segytraceheader_ptr, 
		int *error)
{
	char	*function_name = "mb_extract_segytraceheader";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       segytraceheader_ptr: %d\n",segytraceheader_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segytraceheader_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_segy != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_segytraceheader)
				(verbose,mbio_ptr,store_ptr,
				kind,segytraceheader_ptr,
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       seq_num:           %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:          %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:          %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:           %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:              %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:            %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:             %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:            %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:          %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:          %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:               %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:             %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:          %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:          %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:         %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:         %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:         %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:           %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:           %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:       %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:      %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:          %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:           %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:          %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:           %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:       %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:              %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:             %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:        %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:        %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:        %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:        %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:        %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:              %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:        %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:        %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:        %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:            %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:         %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:              %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:         %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:              %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:               %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:               %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:              %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:         %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:             %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:         %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:         %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:           %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:          %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:         %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:            %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:            %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:            %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:            %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:            %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:            %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:            %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:            %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       heading:           %d\n",mb_segytraceheader_ptr->heading);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int *sampleformat,
		int *kind,
		void *segytraceheader_ptr, 
		float *segydata, 
		int *error)
{
	char	*function_name = "mb_extract_segy";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       sampleformat:   %d\n",*sampleformat);
		fprintf(stderr,"dbg2       segytraceheader_ptr: %d\n",segytraceheader_ptr);
		fprintf(stderr,"dbg2       segydata:       %d\n",segydata);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segytraceheader_ptr;

	/* call the appropriate mbsys_ extraction routine */
	if (mb_io_ptr->mb_io_extract_segy != NULL)
		{
		status = (*mb_io_ptr->mb_io_extract_segy)
				(verbose,mbio_ptr,store_ptr,sampleformat,
				kind,segytraceheader_ptr,segydata,
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sampleformat:   %d\n",*sampleformat);
		fprintf(stderr,"dbg2       kind:           %d\n",*kind);
		fprintf(stderr,"dbg2       seq_num:        %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:       %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:       %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:        %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:           %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:         %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:          %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:         %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:       %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:       %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:            %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:          %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:       %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:       %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:      %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:      %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:      %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:        %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:        %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:    %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:   %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:       %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:        %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:       %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:        %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:    %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:           %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:          %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:     %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:     %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:     %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:     %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:     %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:           %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:     %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:     %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:     %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:         %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:      %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:           %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:      %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:           %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:            %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:            %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:           %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:      %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:          %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:      %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:      %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:        %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:       %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:      %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:         %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:         %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:         %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:         %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:         %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:         %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:         %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:         %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       heading:        %d\n",mb_segytraceheader_ptr->heading);
		for (i=0;i<mb_segytraceheader_ptr->nsamps;i++)
		    fprintf(stderr,"dbg2       sample:%d  data:%f\n", i, segydata[i]);
		fprintf(stderr,"dbg2       error:          %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int kind,
		void *segytraceheader_ptr, 
		float *segydata, 
		int *error)
{
	char	*function_name = "mb_insert_segy";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:         %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:      %d\n",store_ptr);
		fprintf(stderr,"dbg2       segytraceheader_ptr: %d\n",segytraceheader_ptr);
		fprintf(stderr,"dbg2       segydata:       %d\n",segydata);
		fprintf(stderr,"dbg2       kind:           %d\n",kind);
		fprintf(stderr,"dbg2       seq_num:        %d\n",mb_segytraceheader_ptr->seq_num);
		fprintf(stderr,"dbg2       seq_reel:       %d\n",mb_segytraceheader_ptr->seq_reel);
		fprintf(stderr,"dbg2       shot_num:       %d\n",mb_segytraceheader_ptr->shot_num);
		fprintf(stderr,"dbg2       shot_tr:        %d\n",mb_segytraceheader_ptr->shot_tr);
		fprintf(stderr,"dbg2       espn:           %d\n",mb_segytraceheader_ptr->espn);
		fprintf(stderr,"dbg2       rp_num:         %d\n",mb_segytraceheader_ptr->rp_num);
		fprintf(stderr,"dbg2       rp_tr:          %d\n",mb_segytraceheader_ptr->rp_tr);
		fprintf(stderr,"dbg2       trc_id:         %d\n",mb_segytraceheader_ptr->trc_id);
		fprintf(stderr,"dbg2       num_vstk:       %d\n",mb_segytraceheader_ptr->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:       %d\n",mb_segytraceheader_ptr->cdp_fold);
		fprintf(stderr,"dbg2       use:            %d\n",mb_segytraceheader_ptr->use);
		fprintf(stderr,"dbg2       range:          %d\n",mb_segytraceheader_ptr->range);
		fprintf(stderr,"dbg2       grp_elev:       %d\n",mb_segytraceheader_ptr->grp_elev);
		fprintf(stderr,"dbg2       src_elev:       %d\n",mb_segytraceheader_ptr->src_elev);
		fprintf(stderr,"dbg2       src_depth:      %d\n",mb_segytraceheader_ptr->src_depth);
		fprintf(stderr,"dbg2       grp_datum:      %d\n",mb_segytraceheader_ptr->grp_datum);
		fprintf(stderr,"dbg2       src_datum:      %d\n",mb_segytraceheader_ptr->src_datum);
		fprintf(stderr,"dbg2       src_wbd:        %d\n",mb_segytraceheader_ptr->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:        %d\n",mb_segytraceheader_ptr->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:    %d\n",mb_segytraceheader_ptr->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:   %d\n",mb_segytraceheader_ptr->coord_scalar);
		fprintf(stderr,"dbg2       src_long:       %d\n",mb_segytraceheader_ptr->src_long);
		fprintf(stderr,"dbg2       src_lat:        %d\n",mb_segytraceheader_ptr->src_lat);
		fprintf(stderr,"dbg2       grp_long:       %d\n",mb_segytraceheader_ptr->grp_long);
		fprintf(stderr,"dbg2       grp_lat:        %d\n",mb_segytraceheader_ptr->grp_lat);
		fprintf(stderr,"dbg2       coord_units:    %d\n",mb_segytraceheader_ptr->coord_units);
		fprintf(stderr,"dbg2       wvel:           %d\n",mb_segytraceheader_ptr->wvel);
		fprintf(stderr,"dbg2       sbvel:          %d\n",mb_segytraceheader_ptr->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:     %d\n",mb_segytraceheader_ptr->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:     %d\n",mb_segytraceheader_ptr->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:     %d\n",mb_segytraceheader_ptr->src_static);
		fprintf(stderr,"dbg2       grp_static:     %d\n",mb_segytraceheader_ptr->grp_static);
		fprintf(stderr,"dbg2       tot_static:     %d\n",mb_segytraceheader_ptr->tot_static);
		fprintf(stderr,"dbg2       laga:           %d\n",mb_segytraceheader_ptr->laga);
		fprintf(stderr,"dbg2       delay_mils:     %d\n",mb_segytraceheader_ptr->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:     %d\n",mb_segytraceheader_ptr->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:     %d\n",mb_segytraceheader_ptr->emute_mils);
		fprintf(stderr,"dbg2       nsamps:         %d\n",mb_segytraceheader_ptr->nsamps);
		fprintf(stderr,"dbg2       si_micros:      %d\n",mb_segytraceheader_ptr->si_micros);
		for (i=0;i<19;i++)
		fprintf(stderr,"dbg2       other_1[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_1[i]);
		fprintf(stderr,"dbg2       year:           %d\n",mb_segytraceheader_ptr->year);
		fprintf(stderr,"dbg2       day_of_yr:      %d\n",mb_segytraceheader_ptr->day_of_yr);
		fprintf(stderr,"dbg2       hour:           %d\n",mb_segytraceheader_ptr->hour);
		fprintf(stderr,"dbg2       min:            %d\n",mb_segytraceheader_ptr->min);
		fprintf(stderr,"dbg2       sec:            %d\n",mb_segytraceheader_ptr->sec);
		fprintf(stderr,"dbg2       mils:           %d\n",mb_segytraceheader_ptr->mils);
		fprintf(stderr,"dbg2       tr_weight:      %d\n",mb_segytraceheader_ptr->tr_weight);
		for (i=0;i<5;i++)
		fprintf(stderr,"dbg2       other_2[%2d]:       %d\n",i,mb_segytraceheader_ptr->other_2[i]);
		fprintf(stderr,"dbg2       delay:          %f\n",mb_segytraceheader_ptr->delay);
		fprintf(stderr,"dbg2       smute_sec:      %f\n",mb_segytraceheader_ptr->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:      %f\n",mb_segytraceheader_ptr->emute_sec);
		fprintf(stderr,"dbg2       si_secs:        %f\n",mb_segytraceheader_ptr->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:       %f\n",mb_segytraceheader_ptr->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:      %d\n",mb_segytraceheader_ptr->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:         %d\n",mb_segytraceheader_ptr->dummy1);
		fprintf(stderr,"dbg2       dummy2:         %d\n",mb_segytraceheader_ptr->dummy2);
		fprintf(stderr,"dbg2       dummy3:         %d\n",mb_segytraceheader_ptr->dummy3);
		fprintf(stderr,"dbg2       dummy4:         %d\n",mb_segytraceheader_ptr->dummy4);
		fprintf(stderr,"dbg2       dummy5:         %d\n",mb_segytraceheader_ptr->dummy5);
		fprintf(stderr,"dbg2       dummy6:         %d\n",mb_segytraceheader_ptr->dummy6);
		fprintf(stderr,"dbg2       dummy7:         %d\n",mb_segytraceheader_ptr->dummy7);
		fprintf(stderr,"dbg2       dummy8:         %d\n",mb_segytraceheader_ptr->dummy8);
		fprintf(stderr,"dbg2       heading:        %d\n",mb_segytraceheader_ptr->heading);
		for (i=0;i<mb_segytraceheader_ptr->nsamps;i++)
		    fprintf(stderr,"dbg2       sample:%d  data:%f\n", i, segydata[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *) segytraceheader_ptr;

	/* call the appropriate mbsys_ insertion routine */
	if (mb_io_ptr->mb_io_insert_segy != NULL)
		{
		status = (*mb_io_ptr->mb_io_insert_segy)
				(verbose,mbio_ptr,store_ptr,
				kind,segytraceheader_ptr,segydata,
				error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_copyrecord(int verbose, void *mbio_ptr,
		    void *store_ptr, void *copy_ptr, int *error)
{
	char	*function_name = "mb_copy_record";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call appropriate memory copy routine */
	if (mb_io_ptr->mb_io_copyrecord != NULL)
		{
		status = (*mb_io_ptr->mb_io_copyrecord)
				(verbose,mbio_ptr,
				store_ptr,copy_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
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
