/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_ldeoih.c	2/26/93
 *	$Id: mbsys_ldeoih.c,v 5.2 2001-07-20 00:32:54 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * $Log: not supported by cvs2svn $
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
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_ldeoih.h"

/*--------------------------------------------------------------------*/
int mbsys_ldeoih_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_ldeoih.c,v 5.2 2001-07-20 00:32:54 caress Exp $";
	char	*function_name = "mbsys_ldeoih_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

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

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_ldeoih_struct),
				store_ptr,error);

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) *store_ptr;

	/* initialize values in structure */
	store->kind = MB_DATA_NONE;
	store->lon2u = 0;
	store->lon2b = 0;
	store->lat2u = 0;
	store->lat2b = 0;
	store->year = 0;
	store->day = 0;
	store->min = 0;
	store->sec = 0;
	store->msec = 0;
	store->heading = 0;
	store->speed = 0;
	store->beams_bath = 0;
	store->beams_amp = 0;
	store->pixels_ss = 0;
	store->beams_bath_alloc = 0;
	store->beams_amp_alloc = 0;
	store->pixels_ss_alloc = 0;
	store->beamflag = NULL;
	store->bath = NULL;
	store->bath_acrosstrack = NULL;
	store->bath_alongtrack = NULL;
	store->amp = NULL;
	store->ss = NULL;
	store->ss_acrosstrack = NULL;
	store->ss_alongtrack = NULL;
	memset(store->comment, 0, MBSYS_LDEOIH_MAXLINE);

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
int mbsys_ldeoih_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_ldeoih_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

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

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) *store_ptr;

	/* deallocate memory for data structures */
	status = mb_free(verbose,&store->beamflag,error);
	status = mb_free(verbose,&store->bath,error);
	status = mb_free(verbose,&store->bath_acrosstrack,error);
	status = mb_free(verbose,&store->bath_alongtrack,error);
	status = mb_free(verbose,&store->amp,error);
	status = mb_free(verbose,&store->ss,error);
	status = mb_free(verbose,&store->ss_acrosstrack,error);
	status = mb_free(verbose,&store->ss_alongtrack,error);

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

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
	int	time_j[5];
	double	depthscale;
	double	distscale;
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

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = store->year;
		time_j[1] = store->day;
		time_j[2] = store->min;
		time_j[3] = store->sec;
		time_j[4] = 1000 * store->msec;
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ((double) store->lon2u)/60. 
			+ ((double) store->lon2b)/600000.;
		*navlat = ((double) store->lat2u)/60. 
			+ ((double) store->lat2b)/600000. - 90.;
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

		/* get heading (360 degrees = 65536) */
		*heading = store->heading*0.0054932;

		/* set speed to zero */
		*speed = 0.01 * store->speed;

		/* read distance, depth, and backscatter 
			values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		depthscale = 0.001 * store->depth_scale;
		distscale = 0.001 * store->distance_scale;
		for (i=0;i<*nbath;i++)
			{
			beamflag[i] = store->beamflag[i];
			bath[i] = depthscale * store->bath[i];
			bathacrosstrack[i] = distscale * store->bath_acrosstrack[i];
			bathalongtrack[i] = distscale * store->bath_alongtrack[i];
			}
		for (i=0;i<*namp;i++)
			{
			amp[i] = store->amp[i];
			}
		for (i=0;i<*nss;i++)
			{
			ss[i] = store->ss[i];
			ssacrosstrack[i] = distscale * store->ss_acrosstrack[i];
			ssalongtrack[i] = distscale * store->ss_alongtrack[i];
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
				error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
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
	int	time_j[5];
	double	depthscale;
	double	distscale;
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
		mb_get_jtime(verbose,time_i,time_j);
		store->year = time_j[0];
		store->day = time_j[1];
		store->min = time_j[2];
		store->sec = time_j[3];
		store->msec = time_j[4] / 1000;

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->lon2u = (short int) 60.0*navlon;
		store->lon2b = (short int) (600000.0*(navlon 
			- ((double) store->lon2u)/60.0));
		navlat = navlat + 90.0;
		store->lat2u = (short int) 60.0*navlat;
		store->lat2b = (short int) (600000.0*(navlat 
			- ((double) store->lat2u)/60.0));

		/* get heading (360 degrees = 65536) */
		store->heading = 182.044444*heading;

		/* put distance, depth, and backscatter values 
			into data structure */
		depthscale = 0.001 * store->depth_scale;
		distscale = 0.001 * store->distance_scale;
		store->beams_bath = nbath;
		for (i=0;i<nbath;i++)
			{
			store->beamflag[i] = beamflag[i];
			store->bath[i] = bath[i] / depthscale;
			store->bath_acrosstrack[i] = bathacrosstrack[i]
							/ distscale;
			store->bath_alongtrack[i] = bathalongtrack[i]
							/distscale;
			}
		store->beams_amp = namp;
		for (i=0;i<namp;i++)
			{
			store->amp[i] = amp[i];
			}
		store->pixels_ss = nss;
		for (i=0;i<nss;i++)
			{
			store->ss[i] = ss[i];
			store->ss_acrosstrack[i] = ssacrosstrack[i]
							/ distscale;
			store->ss_alongtrack[i] = ssalongtrack[i]
							/ distscale;
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strcpy(store->comment,comment);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %d\n",ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%d\n",angles);
		fprintf(stderr,"dbg2       angles_ltrk:%d\n",angles_forward);
		fprintf(stderr,"dbg2       angles_null:%d\n",angles_null);
		fprintf(stderr,"dbg2       heave:      %d\n",heave);
		fprintf(stderr,"dbg2       ltrk_off:   %d\n",alongtrack_offset);
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
int mbsys_ldeoih_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_ldeoih_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	double	depthscale;
	double	distscale;
	double	bath_best;
	double	xtrack_min;
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

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		depthscale = 0.001 * store->depth_scale;
		distscale = 0.001 * store->distance_scale;
		*transducer_depth = depthscale * store->transducer_depth;
		if (store->altitude <= 0 && store->beams_bath > 0)
		    {		
		    bath_best = 0.0;
		    if (store->bath[store->beams_bath/2] > 0.0)
			bath_best = depthscale * store->bath[store->beams_bath/2];
		    else
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->beams_bath;i++)
			    {
			    if (store->bath[i] > 0.0
				&& fabs(distscale * store->bath_acrosstrack[i]) < xtrack_min)
				{
				xtrack_min = fabs(distscale * store->bath_acrosstrack[i]);
				bath_best = depthscale * store->bath[i];
				}
			    }		
			}
		    if (bath_best <= 0.0)
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->beams_bath;i++)
			    {
			    if (store->bath[i] < 0.0
				&& fabs(distscale * store->bath_acrosstrack[i]) < xtrack_min)
				{
				xtrack_min = fabs(distscale * store->bath_acrosstrack[i]);
				bath_best = -depthscale * store->bath[i];
				}
			    }		
			}
		    *altitude = bath_best - *transducer_depth;
		    }
		else
		    *altitude = depthscale * store->altitude;
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
int mbsys_ldeoih_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_ldeoih_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	double	depthscale;
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
		depthscale = 0.001 * store->depth_scale;
		store->transducer_depth = transducer_depth / depthscale;
		store->altitude = altitude / depthscale;
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
	int	time_j[5];
	double	depthscale;
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

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = store->year;
		time_j[1] = store->day;
		time_j[2] = store->min;
		time_j[3] = store->sec;
		time_j[4] = 1000 * store->msec;
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ((double) store->lon2u)/60. 
			+ ((double) store->lon2b)/600000.;
		*navlat = ((double) store->lat2u)/60. 
			+ ((double) store->lat2b)/600000. - 90.;
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

		/* get heading (360 degrees = 65536) */
		*heading = store->heading*0.0054932;

		/* set speed to zero */
		*speed = 0.01 * store->speed;

		/* set draft to zero */
		depthscale = 0.001 * store->depth_scale;
		*draft = depthscale * store->transducer_depth;

		/* get roll pitch and heave */
		*roll = 0.0;
		*pitch = 0.0;
		*heave = 0.0;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
	int	kind;
	int	time_j[5];
	double	depthscale;
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
		mb_get_jtime(verbose,time_i,time_j);
		store->year = time_j[0];
		store->day = time_j[1];
		store->min = time_j[2];
		store->sec = time_j[3];
		store->msec = time_j[4] / 1000;

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->lon2u = (short int) 60.0*navlon;
		store->lon2b = (short int) (600000.0*(navlon 
			- ((double) store->lon2u)/60.0));
		navlat = navlat + 90.0;
		store->lat2u = (short int) 60.0*navlat;
		store->lat2b = (short int) (600000.0*(navlat 
			- ((double) store->lat2u)/60.0));

		/* get heading (360 degrees = 65536) */
		store->heading = 182.044444*heading;

		/* get speed */
		store->speed = 100 * speed;

		/* get draft */
		depthscale = 0.001 * store->depth_scale;
		store->transducer_depth = draft / depthscale;

		/* get roll pitch and heave */
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

	/* get data structure pointers */
	store = (struct mbsys_ldeoih_struct *) store_ptr;
	copy = (struct mbsys_ldeoih_struct *) copy_ptr;
	
	if (copy->beamflag != NULL)
	    status = mb_free(verbose, &copy->beamflag, error);
	if (copy->bath != NULL)
	    status = mb_free(verbose, &copy->bath, error);
	if (copy->bath_acrosstrack != NULL)
	    status = mb_free(verbose, &copy->bath_acrosstrack, error);
	if (copy->bath_alongtrack != NULL)
	    status = mb_free(verbose, &copy->bath_alongtrack, error);
	if (copy->amp != NULL)
	    status = mb_free(verbose, &copy->amp, error);
	if (copy->ss != NULL)
	    status = mb_free(verbose, &copy->ss, error);
	if (copy->ss_acrosstrack != NULL)
	    status = mb_free(verbose, &copy->ss_acrosstrack, error);
	if (copy->ss_alongtrack != NULL)
	    status = mb_free(verbose, &copy->ss_alongtrack, error);
	status = mb_malloc(verbose, 
		    store->beams_bath * sizeof(char),
		    &copy->beamflag,error);
	status = mb_malloc(verbose, 
		    store->beams_bath * sizeof(short),
		    &copy->bath,error);
	status = mb_malloc(verbose, 
		    store->beams_bath * sizeof(short),
		    &copy->bath_acrosstrack,error);
	status = mb_malloc(verbose, 
		    store->beams_bath * sizeof(short),
		    &copy->bath_alongtrack,error);
	status = mb_malloc(verbose, 
		    store->beams_amp * sizeof(short),
		    &copy->amp,error);
	status = mb_malloc(verbose, 
		    store->pixels_ss * sizeof(short),
		    &copy->ss,error);
	status = mb_malloc(verbose, 
		    store->pixels_ss * sizeof(short),
		    &copy->ss_acrosstrack,error);
	status = mb_malloc(verbose, 
		    store->pixels_ss * sizeof(short),
		    &copy->ss_alongtrack,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
	    {
	    status = mb_free(verbose, &copy->beamflag, error);
	    status = mb_free(verbose, &copy->bath, error);
	    status = mb_free(verbose, &copy->bath_acrosstrack, error);
	    status = mb_free(verbose, &copy->bath_alongtrack, error);
	    status = mb_free(verbose, &copy->amp, error);
	    status = mb_free(verbose, &copy->ss, error);
	    status = mb_free(verbose, &copy->ss_acrosstrack, error);
	    status = mb_free(verbose, &copy->ss_alongtrack, error);
	    status = MB_FAILURE;
	    *error = MB_ERROR_MEMORY_FAIL;
	    }

	/* copy the data */
	if (status == MB_SUCCESS)
	    {
	    copy->kind = store->kind;
	    copy->lon2u = store->lon2u;
	    copy->lon2b = store->lon2b;
	    copy->lat2u = store->lat2u;
	    copy->lat2b = store->lat2b;
	    copy->year = store->year;
	    copy->day = store->day;
	    copy->min = store->min;
	    copy->sec = store->sec;
	    copy->msec = store->msec;
	    copy->heading = store->heading;
	    copy->speed = store->speed;
	    copy->beams_bath = store->beams_bath;
	    copy->beams_amp = store->beams_amp;
	    copy->pixels_ss = store->pixels_ss;
	    copy->depth_scale = store->depth_scale;
	    copy->distance_scale = store->distance_scale;
	    copy->altitude = store->altitude;
	    copy->transducer_depth = store->transducer_depth;
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
