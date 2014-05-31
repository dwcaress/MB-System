/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_ldeoih.c	2/26/93
 *	$Id$
 *
 *    Copyright (c) 1993-2014 by
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
 * mbsys_ldeoih.c contains the functions for handling the data structure
 * used by MBIO functions to store data from a generic multibeam
 * format which handles data with arbitrary numbers of bathymetry,
 * amplitude, and sidescan data.  This generic format is
 *      MBF_MBLDEOIH : MBIO ID 61
 *
 * Author:	D. W. Caress
 * Date:	February 26, 1993
 * $Log: mbsys_ldeoih.c,v $
 * Revision 5.17  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.16  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.15  2008/07/10 18:02:39  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.12  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.11  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.10  2005/03/25 04:26:49  caress
 * Fixed problem with occasional incorrect scaling of sonar depth in mbldeoih format (71) data.
 *
 * Revision 5.9  2004/12/02 06:33:32  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.8  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.7  2003/04/16 16:47:41  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.5  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.4  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.3  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.2  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.15  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.14  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.13  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.12  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.11  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.10  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.10  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.9  1995/11/27  21:51:35  caress
 * New version of mb_ttimes with ssv and angles_null.
 *
 * Revision 4.8  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.7  1995/08/17  14:41:09  caress
 * Revision for release 4.3.
 *
 * Revision 4.6  1995/07/13  19:13:36  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.5  1995/03/22  19:44:26  caress
 * Added explicit casts to shorts divided by doubles for
 * ansi C compliance.
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1994/11/09  21:40:34  caress
 * Changed ttimes extraction routines to handle forward beam angles
 * so that alongtrack distances can be calculated.
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/04/11  23:34:41  caress
 * Added function to extract travel time and beam angle data
 * from multibeam data in an internal data structure.
 *
 * Revision 4.1  1994/04/11  23:34:41  caress
 * Added function to extract travel time and beam angle data
 * from multibeam data in an internal data structure.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/20  04:55:11  caress
 * First cut at new version.  Now handles both amplitude
 * and sidescan data.
 *
 * Revision 3.0  1993/05/14  23:04:29  sohara
 * initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_ldeoih.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_ldeoih_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_ldeoih_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mbsys_ldeoih_struct),
				(void **)store_ptr,error);

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) *store_ptr;

	/* initialize values in structure */
	store->kind = MB_DATA_NONE;
	store->time_d = 0.0;
	store->longitude = 0.0;
	store->latitude = 0.0;
	store->sonardepth = 0.0;
	store->altitude = 0.0;
	store->heading = 0.0;
	store->speed = 0.0;
	store->roll = 0.0;
	store->pitch = 0.0;
	store->heave = 0.0;
	store->beam_xwidth = 0.0;
	store->beam_lwidth = 0.0;
	store->beams_bath = 0;
	store->beams_amp = 0;
	store->pixels_ss = 0;
	store->spare1 = 0;
	store->beams_bath_alloc = 0;
	store->beams_amp_alloc = 0;
	store->pixels_ss_alloc = 0;
	store->depth_scale = 0.0;
	store->distance_scale = 0.0;
	store->ss_scalepower = 0;
	store->ss_type = 0;
	store->spare3 = 0;
	store->sonartype = MB_SONARTYPE_UNKNOWN;
	store->beamflag = NULL;
	store->bath = NULL;
	store->amp = NULL;
	store->bath_acrosstrack = NULL;
	store->bath_alongtrack = NULL;
	store->ss = NULL;
	store->ss_acrosstrack = NULL;
	store->ss_alongtrack = NULL;
	memset(store->comment, 0, MBSYS_LDEOIH_MAXLINE);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_ldeoih_deall";
	int	status = MB_SUCCESS;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		}

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) *store_ptr;

	/* deallocate memory for data structures */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->beamflag,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_acrosstrack,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_alongtrack,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->amp,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_acrosstrack,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_alongtrack,error);

	/* deallocate memory for data structure */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)store_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_ldeoih_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		}
	else
		{
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_ldeoih_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error)
{
	char	*function_name = "mbsys_ldeoih_sidescantype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get sidescan type */
	*sonartype = store->sonartype;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sonartype:  %d\n",*sonartype);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
		int *ss_type, int *error)
{
	char	*function_name = "mbsys_ldeoih_sidescantype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get sidescan type */
	*ss_type = store->ss_type;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ss_type:    %d\n",*ss_type);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_ldeoih_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	double	ss_scale;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->time_d;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get heading */
		*heading = store->heading;

		/* get speed */
		*speed = store->speed;

		/* set beamwidths in mb_io structure */
		if (store->beam_lwidth > 0)
		    mb_io_ptr->beamwidth_ltrack = store->beam_lwidth;
		else
		    mb_io_ptr->beamwidth_ltrack = 2.0;
		if (store->beam_xwidth > 0)
		    mb_io_ptr->beamwidth_xtrack = store->beam_xwidth;
		else
		    mb_io_ptr->beamwidth_xtrack = 2.0;

		/* read distance, depth, and backscatter
			values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		for (i=0;i<*nbath;i++)
			{
			beamflag[i] = store->beamflag[i];
			if (beamflag[i] != MB_FLAG_NULL)
				{
				bath[i] = store->depth_scale * store->bath[i] + store->sonardepth;
				bathacrosstrack[i] = store->distance_scale * store->bath_acrosstrack[i];
				bathalongtrack[i] = store->distance_scale * store->bath_alongtrack[i];
				}
			else
				{
				bath[i] = 0.0;
				bathacrosstrack[i] = 0.0;
				bathalongtrack[i] = 0.0;
				}
			}
		for (i=0;i<*namp;i++)
			{
			amp[i] = store->amp[i];
			}
		ss_scale = pow(2.0, (double)(store->ss_scalepower));
		for (i=0;i<*nss;i++)
			{
			if (store->ss[i] != 0)
				ss[i] = ss_scale * store->ss[i];
			else
				ss[i] = MB_SIDESCAN_NULL;
			ssacrosstrack[i] = store->distance_scale * store->ss_acrosstrack[i];
			ssalongtrack[i] = store->distance_scale * store->ss_alongtrack[i];
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				*navlon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				*navlat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				*speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				*heading);
			fprintf(stderr,"dbg4       nbath:      %d\n",
				*nbath);
			for (i=0;i<*nbath;i++)
			  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
				i,beamflag[i],bath[i],bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        namp:     %d\n",
				*namp);
			for (i=0;i<*namp;i++)
			  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        nss:      %d\n",
				*nss);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strcpy(comment,store->comment);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_ldeoih_insert(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_ldeoih_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	double	depthmax, distmax;
	double	ssmax, ss_scale;
	int	ngood;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
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
		fprintf(stderr,"dbg2       nbath:      %d\n",
			nbath);
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

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;
		store->speed = speed;

		/* if needed reset numbers of beams and allocate
		   memory for store arrays */
		if (nbath > store->beams_bath_alloc)
		    {
		    store->beams_bath_alloc = nbath;
		    if (store->beamflag != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->beamflag, error);
		    if (store->bath != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath, error);
		    if (store->bath_acrosstrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_acrosstrack, error);
		    if (store->bath_alongtrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_alongtrack, error);
		    status = mb_mallocd(verbose,__FILE__, __LINE__,
				store->beams_bath_alloc * sizeof(char),
				(void **)&store->beamflag,error);
		    status = mb_mallocd(verbose,__FILE__, __LINE__,
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath,error);
		    status = mb_mallocd(verbose,__FILE__, __LINE__,
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath_acrosstrack,error);
		    status = mb_mallocd(verbose,__FILE__, __LINE__,
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath_alongtrack,error);

		    /* deal with a memory allocation failure */
		    if (status == MB_FAILURE)
			{
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->beamflag, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_acrosstrack, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->bath_alongtrack, error);
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  MBcopy function <%s> terminated with error\n",
					function_name);
				fprintf(stderr,"dbg2  Return values:\n");
				fprintf(stderr,"dbg2       error:      %d\n",*error);
				fprintf(stderr,"dbg2  Return status:\n");
				fprintf(stderr,"dbg2       status:  %d\n",status);
				}
			return(status);
			}
		    }
		if (namp > store->beams_amp_alloc)
		    {
		    store->beams_amp_alloc = namp;
		    if (store != NULL)
			{
			if (store->amp != NULL)
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->amp, error);
			status = mb_mallocd(verbose,__FILE__, __LINE__,
				    store->beams_amp_alloc * sizeof(short),
				    (void **)&store->amp,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->amp, error);
			    status = MB_FAILURE;
			    *error = MB_ERROR_MEMORY_FAIL;
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					    function_name);
				    fprintf(stderr,"dbg2  Return values:\n");
				    fprintf(stderr,"dbg2       error:      %d\n",*error);
				    fprintf(stderr,"dbg2  Return status:\n");
				    fprintf(stderr,"dbg2       status:  %d\n",status);
				    }
			    return(status);
			    }
			}
		    }
		if (nss > store->pixels_ss_alloc)
		    {
		    store->pixels_ss_alloc = nss;
		    if (store != NULL)
			{
			if (store->ss != NULL)
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss, error);
			if (store->ss_acrosstrack != NULL)
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_acrosstrack, error);
			if (store->ss_alongtrack != NULL)
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_alongtrack, error);
			status = mb_mallocd(verbose,__FILE__, __LINE__,
				    store->pixels_ss_alloc * sizeof(short),
				    (void **)&store->ss,error);
			status = mb_mallocd(verbose,__FILE__, __LINE__,
				    store->pixels_ss_alloc * sizeof(short),
				    (void **)&store->ss_acrosstrack,error);
			status = mb_mallocd(verbose,__FILE__, __LINE__,
				    store->pixels_ss_alloc * sizeof(short),
				    (void **)&store->ss_alongtrack,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss, error);
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_acrosstrack, error);
			    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&store->ss_alongtrack, error);
			    status = MB_FAILURE;
			    *error = MB_ERROR_MEMORY_FAIL;
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					    function_name);
				    fprintf(stderr,"dbg2  Return values:\n");
				    fprintf(stderr,"dbg2       error:      %d\n",*error);
				    fprintf(stderr,"dbg2  Return status:\n");
				    fprintf(stderr,"dbg2       status:  %d\n",status);
				    }
			    return(status);
			    }
			}
		    }

		/* get scaling */
		depthmax = 0.0;
		distmax = 0.0;
		ssmax = 0.0;
		for (i=0;i<nbath;i++)
		    {
		    if (beamflag[i] != MB_FLAG_NULL)
			{
			depthmax = MAX(depthmax, fabs(bath[i] - store->sonardepth));
			distmax = MAX(distmax, fabs(bathacrosstrack[i]));
			distmax = MAX(distmax, fabs(bathalongtrack[i]));
			}
		    }
		for (i=0;i<nss;i++)
		    {
		    if (ss[i] > MB_SIDESCAN_NULL)
			{
			distmax = MAX(distmax, fabs(ssacrosstrack[i]));
			distmax = MAX(distmax, fabs(ssalongtrack[i]));
			ssmax = MAX(ssmax, fabs(ss[i]));
			}
		    }
		if (depthmax > 0.0)
		    store->depth_scale = 0.001 * (float)(MAX((depthmax / 30.0), 1.0));
		if (distmax > 0.0)
		    store->distance_scale = 0.001 * (float)(MAX((distmax / 30.0), 1.0));
		if (ssmax > 0.0)
			{
			store->ss_scalepower = (mb_s_char)(log2(ssmax / 32767.0)) + 1;
			ss_scale = pow(2.0, (double)(store->ss_scalepower));
			}
		else
			{
			store->ss_scalepower = 0;
			ss_scale = 1.0;
			}

		/* set beam widths */
		if (store->beam_xwidth == 0.0)
		    store->beam_xwidth = mb_io_ptr->beamwidth_xtrack;
		if (store->beam_lwidth == 0.0)
		    store->beam_lwidth = mb_io_ptr->beamwidth_ltrack;

		/* put distance, depth, and backscatter values
			into data structure */
		store->beams_bath = nbath;
		for (i=0;i<nbath;i++)
			{
			if (beamflag[i] != MB_FLAG_NULL)
				{
				store->beamflag[i] = beamflag[i];
				store->bath[i] = (bath[i] - store->sonardepth) / store->depth_scale;
				store->bath_acrosstrack[i] = bathacrosstrack[i] / store->distance_scale;
				store->bath_alongtrack[i] = bathalongtrack[i] / store->distance_scale;
				}
			else
				{
				store->beamflag[i] = beamflag[i];
				store->bath[i] = 0;
				store->bath_acrosstrack[i] = 0;
				store->bath_alongtrack[i] = 0;
				}
			}
		store->beams_amp = namp;
		for (i=0;i<namp;i++)
			{
			store->amp[i] = amp[i];
			}
		store->pixels_ss = nss;
		ngood = 0;
		for (i=0;i<nss;i++)
			{
			if (ss[i] > MB_SIDESCAN_NULL)
				{
				store->ss[i] = (short)(ss[i] / ss_scale);
				ngood++;
				}
			else
				store->ss[i] = 0;
			store->ss_acrosstrack[i] = ssacrosstrack[i] / store->distance_scale;
			store->ss_alongtrack[i] = ssalongtrack[i] / store->distance_scale;
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->comment,comment,MBSYS_LDEOIH_MAXLINE-1);
		if (strlen(comment) > MBSYS_LDEOIH_MAXLINE-2)
			store->comment[MBSYS_LDEOIH_MAXLINE-1] = '\0';
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles,
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset,
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_ldeoih_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %p\n",(void *)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%p\n",(void *)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%p\n",(void *)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%p\n",(void *)angles_null);
		fprintf(stderr,"dbg2       heave:      %p\n",(void *)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %p\n",(void *)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get travel times, angles */
		for (i=0;i<store->beams_bath;i++)
			{
			ttimes[i] = 0.0;
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
			}

		/* get ssv */
		*ssv = 0.0;
		*draft = 0.0;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */

		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       draft:      %f\n",*draft);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
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
int mbsys_ldeoih_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	int *detects, int *error)
{
	char	*function_name = "mbsys_ldeoih_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       detects:    %p\n",(void *)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get detects */
		for (i=0;i<*nbeams;i++)
			{
			detects[i] = MB_DETECT_UNKNOWN;
			}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */

		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_ldeoih_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude,
	int *error)
{
	char	*function_name = "mbsys_ldeoih_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	double	bath_best;
	double	xtrack_min;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		*transducer_depth = store->sonardepth;
		if (store->altitude <= 0.0 && store->beams_bath > 0)
		    {
		    bath_best = 0.0;
		    if (store->bath[store->beams_bath/2] > 0.0)
			bath_best = store->depth_scale * store->bath[store->beams_bath/2] + (*transducer_depth);
		    else
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->beams_bath;i++)
			    {
			    if (store->bath[i] > 0
				&& fabs(store->distance_scale * store->bath_acrosstrack[i]) < xtrack_min)
				{
				xtrack_min = fabs(store->distance_scale * store->bath_acrosstrack[i]);
				bath_best = store->depth_scale * store->bath[i] + (*transducer_depth);
				}
			    }
			}
		    if (bath_best <= 0.0)
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->beams_bath;i++)
			    {
			    if (store->bath[i] < 0.0
				&& fabs(store->distance_scale * store->bath_acrosstrack[i]) < xtrack_min)
				{
				xtrack_min = fabs(store->distance_scale * store->bath_acrosstrack[i]);
				bath_best = -store->depth_scale * store->bath[i] + (*transducer_depth);
				}
			    }
			}
		    *altitude = bath_best - *transducer_depth;
		    }
		else
		    *altitude = store->altitude;
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_ldeoih_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude,
	int *error)
{
	char	*function_name = "mbsys_ldeoih_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
		{
		store->sonardepth = transducer_depth;
		store->altitude = altitude;
		}

	/* deal with comment */
	else if (store->kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mbsys_ldeoih_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */;
		*time_d = store->time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get heading */
		*heading = store->heading;

		/* set speed */
		*speed = store->speed;

		/* set draft */
		*draft = store->sonardepth + store->heave;

		/* get roll pitch and heave */
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				*navlon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				*navlat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				*speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				*heading);
			fprintf(stderr,"dbg4       draft:      %f\n",
				*draft);
			fprintf(stderr,"dbg4       roll:       %f\n",
				*roll);
			fprintf(stderr,"dbg4       pitch:      %f\n",
				*pitch);
			fprintf(stderr,"dbg4       heave:      %f\n",
				*heave);
			}

		/* done translating values */

		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR
		&& *kind == MB_DATA_DATA)
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
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
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
int mbsys_ldeoih_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_ldeoih_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
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
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;
		store->speed = speed;

		/* get draft */
		store->sonardepth = draft - heave;

		/* get roll pitch and heave */
		store->roll = roll;
		store->pitch = pitch;
		store->heave = heave;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_ldeoih_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	struct mbsys_ldeoih_struct *copy;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %p\n",(void *)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_ldeoih_struct *) store_ptr;
	copy = (struct mbsys_ldeoih_struct *) copy_ptr;

	if (copy->beamflag != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->beamflag, error);
	if (copy->bath != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath, error);
	if (copy->bath_acrosstrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath_acrosstrack, error);
	if (copy->bath_alongtrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath_alongtrack, error);
	if (copy->amp != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->amp, error);
	if (copy->ss != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss, error);
	if (copy->ss_acrosstrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_acrosstrack, error);
	if (copy->ss_alongtrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_alongtrack, error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->beams_bath * sizeof(char),
		    (void **)&copy->beamflag,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->beams_bath * sizeof(short),
		    (void **)&copy->bath,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->beams_bath * sizeof(short),
		    (void **)&copy->bath_acrosstrack,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->beams_bath * sizeof(short),
		    (void **)&copy->bath_alongtrack,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->beams_amp * sizeof(short),
		    (void **)&copy->amp,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->pixels_ss * sizeof(short),
		    (void **)&copy->ss,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->pixels_ss * sizeof(short),
		    (void **)&copy->ss_acrosstrack,error);
	status = mb_mallocd(verbose,__FILE__, __LINE__,
		    store->pixels_ss * sizeof(short),
		    (void **)&copy->ss_alongtrack,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
	    {
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->beamflag, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath_acrosstrack, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->bath_alongtrack, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->amp, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_acrosstrack, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_alongtrack, error);
	    status = MB_FAILURE;
	    *error = MB_ERROR_MEMORY_FAIL;
	    }

	/* copy the data */
	if (status == MB_SUCCESS)
	    {
	    copy->kind = store->kind;
	    copy->time_d = store->time_d;
	    copy->longitude = store->longitude;
	    copy->latitude = store->latitude;
	    copy->sonardepth = store->sonardepth;
	    copy->altitude = store->altitude;
	    copy->heading = store->heading;
	    copy->speed = store->speed;
	    copy->roll = store->roll;
	    copy->pitch = store->pitch;
	    copy->heave = store->heave;
	    copy->beam_xwidth = store->beam_xwidth;
	    copy->beam_lwidth = store->beam_lwidth;
	    copy->beams_bath = store->beams_bath;
	    copy->beams_amp = store->beams_amp;
	    copy->pixels_ss = store->pixels_ss;
	    copy->spare1 = store->spare1;
	    copy->depth_scale = store->depth_scale;
	    copy->distance_scale = store->distance_scale;
	    copy->ss_type = store->ss_type;
	    copy->ss_scalepower = store->ss_scalepower;
	    for (i=0;i<copy->beams_bath;i++)
		    {
		    copy->beamflag[i] = store->beamflag[i];
		    copy->bath[i] = store->bath[i];
		    copy->bath_acrosstrack[i] = store->bath_acrosstrack[i];
		    copy->bath_alongtrack[i] = store->bath_alongtrack[i];
		    }
	    for (i=0;i<copy->beams_amp;i++)
		    {
		    copy->amp[i] = store->amp[i];
		    }
	    for (i=0;i<copy->pixels_ss;i++)
		    {
		    copy->ss[i] = store->ss[i];
		    copy->ss_acrosstrack[i] = store->ss_acrosstrack[i];
		    copy->ss_alongtrack[i] = store->ss_alongtrack[i];
		    }
	    strcpy(copy->comment,store->comment);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
