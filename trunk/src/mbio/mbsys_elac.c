/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_elac.c	3.00	8/20/94
 *	$Id: mbsys_elac.c,v 4.12 1997-09-15 19:06:40 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_elac.c contains the functions for handling the data structure
 * used by MBIO functions to store data from Reson SEABAT 9001
 * multibeam sonar systems.
 * The data formats which are commonly used to store EM-12
 * data in files include
 *      MBF_BCHRTUNB : MBIO ID 91
 *
 * These functions include:
 *   mbsys_elac_alloc	  - allocate memory for mbsys_elac_struct structure
 *   mbsys_elac_deall	  - deallocate memory for mbsys_elac_struct structure
 *   mbsys_elac_extract - extract basic data from mbsys_elac_struct 
 *				structure
 *   mbsys_elac_insert  - insert basic data into mbsys_elac_struct structure
 *   mbsys_elac_ttimes  - extract travel time data from
 *				mbsys_elac_struct structure
 *   mbsys_elac_extract_nav - extract navigation data from
 *                          mbsys_elac_struct structure
 *   mbsys_elac_insert_nav - insert navigation data into
 *                          mbsys_elac_struct structure
 *   mbsys_elac_copy	  - copy data in one mbsys_elac_struct structure
 *   				into another mbsys_elac_struct structure
 *
 * Author:	D. W. Caress
 * Date:	August 20, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.11  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.10  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.9  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.9  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/04/22  10:57:09  caress
 * DTR define now in mb_io.h
 *
 * Revision 4.6  1995/11/27  21:50:06  caress
 * New version of mb_ttimes with ssv and angles_null.
 *
 * Revision 4.5  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.4  1995/08/17  14:41:09  caress
 * Revision for release 4.3.
 *
 * Revision 4.3  1995/07/13  19:13:36  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.2  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1994/11/09  21:40:34  caress
 * Changed ttimes extraction routines to handle forward beam angles
 * so that alongtrack distances can be calculated.
 *
 * Revision 4.0  1994/10/21  12:34:59  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
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
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_elac.h"

/*--------------------------------------------------------------------*/
int mbsys_elac_alloc(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
 static char res_id[]="$Id: mbsys_elac.c,v 4.12 1997-09-15 19:06:40 caress Exp $";
	char	*function_name = "mbsys_elac_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	int	i, j;

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
	status = mb_malloc(verbose,sizeof(struct mbsys_elac_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_elac_struct *) *store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->sonar = MBSYS_ELAC_UNKNOWN;

	/* parameter telegram */
	store->par_year = 0;
	store->par_month = 0;
	store->par_day = 0;
	store->par_hour = 0;
	store->par_minute = 0;
	store->par_second = 0;
	store->par_hundredth_sec = 0;
	store->par_thousandth_sec = 0;
	store->roll_offset = 0;		/* roll offset (degrees) */
	store->pitch_offset = 0;	/* pitch offset (degrees) */
	store->heading_offset = 0;	/* heading offset (degrees) */
	store->time_delay = 0;		/* positioning system delay (sec) */
	store->transducer_port_height = 0;
	store->transducer_starboard_height = 0;
	store->transducer_port_depth = 0;
	store->transducer_starboard_depth = 0;
	store->transducer_port_x = 0;
	store->transducer_starboard_x = 0;
	store->transducer_port_y = 0;
	store->transducer_starboard_y = 0;
	store->transducer_port_error = 0;
	store->transducer_starboard_error = 0;
	store->antenna_height = 0;
	store->antenna_x = 0;
	store->antenna_y = 0;
	store->vru_height = 0;
	store->vru_x = 0;
	store->vru_y = 0;
	store->heave_offset = 0;
	store->line_number = 0;
	store->start_or_stop = 0;
	store->transducer_serial_number = 0;
	for (i=0;i<MBSYS_ELAC_COMMENT_LENGTH;i++)
		store->comment[i] = '\0';

	/* position (position telegrams) */
	store->pos_year = 0;
	store->pos_month = 0;
	store->pos_day = 0;
	store->pos_hour = 0;
	store->pos_minute = 0;
	store->pos_second = 0;
	store->pos_hundredth_sec = 0;
	store->pos_thousandth_sec = 0;
	store->pos_latitude = 0;
	store->pos_longitude = 0;
	store->utm_northing = 0;
	store->utm_easting = 0;
	store->utm_zone_lon = 0;
	store->utm_zone = 0;
	store->hemisphere = 0;
	store->ellipsoid = 0;
	store->pos_spare = 0;
	store->semi_major_axis = 0;
	store->other_quality = 0;

	/* sound velocity profile */
	store->svp_year = 0;
	store->svp_month = 0;
	store->svp_day = 0;
	store->svp_hour = 0;
	store->svp_minute = 0;
	store->svp_second = 0;
	store->svp_hundredth_sec = 0;
	store->svp_thousandth_sec = 0;
	store->svp_num = 0;
	for (i=0;i<500;i++)
		{
		store->svp_depth[i] = 0; /* 0.1 meters */
		store->svp_vel[i] = 0;	/* 0.1 meters/sec */
		}

	/* depth telegram */
	store->ping_num = 0;
	store->sound_vel = 0;
	store->mode = 0;
	store->pulse_length = 0;
	store->source_power = 0;
	store->receiver_gain = 0;
	store->profile_num = 0;
	store->beams_bath = 0;
	for (i=0;i<7;i++)
		{
		store->profile[i].year = 0;
		store->profile[i].month = 0;
		store->profile[i].day = 0;
		store->profile[i].hour = 0;
		store->profile[i].minute = 0;
		store->profile[i].second = 0;
		store->profile[i].hundredth_sec = 0;
		store->profile[i].thousandth_sec = 0;
		store->profile[i].roll = 0;
		store->profile[i].pitch = 0;
		store->profile[i].heading = 0;
		store->profile[i].heave = 0;
		for (j=0;j<8;j++)
			{
			store->profile[i].bath[j] = 0;
			store->profile[i].bath_acrosstrack[j] = 0;
			store->profile[i].bath_alongtrack[j] = 0;
			store->profile[i].tt[j] = 0;
			store->profile[i].angle[j] = 0;
			store->profile[i].quality[j] = 0;
			store->profile[i].amp[j] = 0;
			}
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
int mbsys_elac_deall(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
	char	*function_name = "mbsys_elac_deall";
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
int mbsys_elac_extract(verbose,mbio_ptr,store_ptr,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
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
	char	*function_name = "mbsys_elac_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	double	depthscale, dacrscale, daloscale,reflscale;
	int	ibeam;
	int	i, j;

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
	store = (struct mbsys_elac_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_i[0] = store->profile[0].year + 1900;
		time_i[1] = store->profile[0].month;
		time_i[2] = store->profile[0].day;
		time_i[3] = store->profile[0].hour;
		time_i[4] = store->profile[0].minute;
		time_i[5] = store->profile[0].second;
		time_i[6] = 10000*store->profile[0].hundredth_sec 
			+ 100*store->profile[0].thousandth_sec;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = store->profile[0].longitude*0.00000009;
		*navlat = store->profile[0].latitude*0.00000009;
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

		/* get heading */
		*heading = 0.01*store->profile[0].heading;

		/* get speed  */
		*speed = 0.0;

		/* read distance and depth values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_bath;
		*nss = 0;
		depthscale = 0.01;
		dacrscale  = -0.02;
		daloscale  = 0.01;
		reflscale  = 1.0;
		for (i=0;i<store->profile_num;i++)
		  for (j=0;j<8;j++)
			{
			ibeam = (store->profile_num - 1 - i)
					+ store->profile_num*(7 - j);
			bath[ibeam] = depthscale*store->profile[i].bath[j];
			bathacrosstrack[ibeam] = dacrscale
				*store->profile[i].bath_acrosstrack[j];
			bathalongtrack[ibeam] = daloscale
				*store->profile[i].bath_alongtrack[j];
			amp[ibeam] = reflscale*store->profile[i].amp[j];
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
			  fprintf(stderr,"dbg4       beam:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
				i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        namp:     %d\n",
				*namp);
			for (i=0;i<*namp;i++)
			  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
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
		  fprintf(stderr,"dbg2       beam:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        namp:     %d\n",
			*namp);
		for (i=0;i<*namp;i++)
		  fprintf(stderr,"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
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
int mbsys_elac_insert(verbose,mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
int	nbath;
int	namp;
int	nss;
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
	char	*function_name = "mbsys_elac_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	int	kind;
	double	depthscale, dacrscale,daloscale,reflscale;
	int	ibeam;
	int	i, j;

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
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3) 
		 for (i=0;i<namp;i++)
		  fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_elac_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->profile[0].year = time_i[0] - 1900;
		store->profile[0].month = time_i[1];
		store->profile[0].day = time_i[2];
		store->profile[0].hour = time_i[3];
		store->profile[0].minute = time_i[4];
		store->profile[0].second = time_i[5];
		store->profile[0].hundredth_sec = time_i[6]/10000;
		store->profile[0].thousandth_sec 
			= (time_i[6] 
			- 10000*store->profile[0].hundredth_sec)/100;

		/*get navigation */
		store->profile[0].longitude = (int) (navlon*11111111.0);
		store->profile[0].latitude = (int) (navlat*11111111.0);

		/* get heading */
		store->profile[0].heading = (int) (heading * 100);

		/* insert distance and depth values into storage arrays */
		if (store->beams_bath == nbath)
			{
			depthscale = 0.01;
			dacrscale  = -0.02;
			daloscale  = 0.01;
			reflscale  = 1.0;
			for (i=0;i<store->profile_num;i++)
			  for (j=0;j<8;j++)
				{
				ibeam = (store->profile_num - 1 - i)
					+ store->profile_num*(7 - j);
				store->profile[i].bath[j] 
					= bath[ibeam]/depthscale;
				store->profile[i].bath_acrosstrack[j] 
					= bathacrosstrack[ibeam]/dacrscale;
				store->profile[i].bath_alongtrack[j] 
					= bathalongtrack[ibeam]/daloscale;
				store->profile[i].amp[j] = amp[ibeam]/reflscale;
				}

			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->comment,comment,199);
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
int mbsys_elac_ttimes(verbose,mbio_ptr,store_ptr,kind,nbeams,
	ttimes,angles,angles_forward,angles_null,
	depth_offset,alongtrack_offset,flags,ssv,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	*nbeams;
double	*ttimes;
double	*angles;
double	*angles_forward;
double	*angles_null;
double	*depth_offset;
double	*alongtrack_offset;
int	*flags;
double	*ssv;
int	*error;
{
	char	*function_name = "mbsys_elac_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	double	daloscale, ttscale, angscale;
	double	depthadd;
	int	ibeam;
	int	i, j;

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
		fprintf(stderr,"dbg2       depth_off:  %d\n",depth_offset);
		fprintf(stderr,"dbg2       ltrk_off:   %d\n",alongtrack_offset);
		fprintf(stderr,"dbg2       flags:      %d\n",flags);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_elac_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get depth offset (heave + heave offset) */
		/*depthadd = 0.005 * (store->transducer_starboard_depth
				    + store->transducer_port_depth)
				+ 0.001 * store->heave_offset;*/
		depthadd = 0.0; /* parameter records only found
				    sporadically */

		/* get ssv */
		*ssv = store->sound_vel;
		if (*ssv < 1400.0 || *ssv > 1600.0)
			*ssv = 1480.0; /* hard wired for UNB MB course 96 */

		/* get travel times, angles, and flags */
		daloscale  = 0.01;
		ttscale = 0.0001;
		angscale = 0.005;
		for (i=0;i<store->profile_num;i++)
		  for (j=0;j<8;j++)
			{
			ibeam = (store->profile_num - 1 - i)
					+ store->profile_num*(7 - j);
			ttimes[ibeam] = ttscale*store->profile[i].tt[j];
			angles[ibeam] = angscale*store->profile[i].angle[j];
			if (angles[ibeam] < 0.0)
				{
				angles[ibeam] = -angles[ibeam];
				angles_forward[ibeam] = 0.0;
				angles_null[ibeam] = 30.0 
					+ angscale 
					* store->transducer_port_error;
				}
			else
				{
				angles_forward[ibeam] = 180.0;
				angles_null[ibeam] = 30.0 
					+ angscale 
					* store->transducer_starboard_error;
				}
			depth_offset[ibeam] = depthadd 
				+ 0.001 * store->profile[i].heave;
			alongtrack_offset[ibeam] = daloscale
				*store->profile[i].bath_alongtrack[j];
			if (store->profile[i].bath[j] < 0)
				flags[ibeam] = MB_YES;
			else
				flags[ibeam] = MB_NO;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f  flag:%d\n",
				i,ttimes[i],angles[i],
				angles_forward[i],angles_null[i],
				depth_offset[i],alongtrack_offset[i],
				flags[i]);
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
int mbsys_elac_extract_nav(verbose,mbio_ptr,store_ptr,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		roll,pitch,heave,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*roll;
double	*pitch;
double	*heave;
int	*error;
{
	char	*function_name = "mbsys_elac_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	int	ibeam;
	int	i, j;

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
	store = (struct mbsys_elac_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_i[0] = store->profile[0].year + 1900;
		time_i[1] = store->profile[0].month;
		time_i[2] = store->profile[0].day;
		time_i[3] = store->profile[0].hour;
		time_i[4] = store->profile[0].minute;
		time_i[5] = store->profile[0].second;
		time_i[6] = 10000*store->profile[0].hundredth_sec 
			+ 100*store->profile[0].thousandth_sec;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = store->profile[0].longitude*0.00000009;
		*navlat = store->profile[0].latitude*0.00000009;
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

		/* get heading */
		*heading = 0.01*store->profile[0].heading;

		/* get speed  */
		*speed = 0.0;

		/* get roll pitch and heave */
		*roll = 0.005 * store->profile[0].roll;
		*pitch = 0.005 * store->profile[0].pitch;
		*heave = 0.001 * store->profile[0].heave;

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
int mbsys_elac_insert_nav(verbose,mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,speed,heading,
		roll,pitch,heave,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	roll;
double	pitch;
double	heave;
int	*error;
{
	char	*function_name = "mbsys_elac_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	int	kind;
	int	ibeam;
	int	i, j;

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
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_elac_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->profile[0].year = time_i[0] - 1900;
		store->profile[0].month = time_i[1];
		store->profile[0].day = time_i[2];
		store->profile[0].hour = time_i[3];
		store->profile[0].minute = time_i[4];
		store->profile[0].second = time_i[5];
		store->profile[0].hundredth_sec = time_i[6]/10000;
		store->profile[0].thousandth_sec 
			= (time_i[6] 
			- 10000*store->profile[0].hundredth_sec)/100;

		/*get navigation */
		store->profile[0].longitude = (int) (navlon*11111111.0);
		store->profile[0].latitude = (int) (navlat*11111111.0);

		/* get heading */
		store->profile[0].heading = (int) (heading *100);

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
int mbsys_elac_copy(verbose,mbio_ptr,store_ptr,copy_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
char	*copy_ptr;
int	*error;
{
	char	*function_name = "mbsys_elac_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_elac_struct *store;
	struct mbsys_elac_struct *copy;

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
	store = (struct mbsys_elac_struct *) store_ptr;
	copy = (struct mbsys_elac_struct *) copy_ptr;

	/* copy the data */
	*copy = *store;

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
