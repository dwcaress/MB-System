/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad.c	3.00	8/5/94
 *	$Id: mbsys_simrad.c,v 4.8 1996-04-22 13:21:19 caress Exp $
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
 * mbsys_simrad.c contains the functions for handling the data structure
 * used by MBIO functions to store data from Simrad EM-12 and EM-1000
 * multibeam sonar systems.
 * The data formats which are commonly used to store EM-12
 * data in files include
 *      MBF_EM1000   : MBIO ID 51
 *      MBF_EM12S    : MBIO ID 52
 *      MBF_EM12D    : MBIO ID 53
 *      MBF_EM12DARW : MBIO ID 54
 *
 * These functions include:
 *   mbsys_simrad_alloc	  - allocate memory for mbsys_simrad_struct structure
 *   mbsys_simrad_deall	  - deallocate memory for mbsys_simrad_struct structure
 *   mbsys_simrad_extract - extract basic data from mbsys_simrad_struct 
 *				structure
 *   mbsys_simrad_insert  - insert basic data into mbsys_simrad_struct structure
 *   mbsys_simrad_ttimes  - extract travel time data from
 *				mbsys_simrad_struct structure
 *   mbsys_simrad_extract_nav - extract navigation data from
 *                          mbsys_simrad_struct structure
 *   mbsys_simrad_insert_nav - insert navigation data into
 *                          mbsys_simrad_struct structure
 *   mbsys_simrad_copy	  - copy data in one mbsys_simrad_struct structure
 *   				into another mbsys_simrad_struct structure
 *
 * Author:	D. W. Caress
 * Date:	August 5, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.7  1996/04/22  10:57:09  caress
 * DTR define now in mb_io.h
 *
 * Revision 4.6  1995/11/27  21:53:53  caress
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
 * Revision 4.0  1994/10/21  12:35:02  caress
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
#include "../../include/mbsys_simrad.h"

/*--------------------------------------------------------------------*/
int mbsys_simrad_alloc(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
 static char res_id[]="$Id: mbsys_simrad.c,v 4.8 1996-04-22 13:21:19 caress Exp $";
	char	*function_name = "mbsys_simrad_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	i;

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
	status = mb_malloc(verbose,sizeof(struct mbsys_simrad_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) *store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->sonar = MBSYS_SIMRAD_UNKNOWN;

	/* parameter datagram */
	store->par_year = 0;
	store->par_month = 0;
	store->par_day = 0;
	store->par_hour = 0;
	store->par_minute = 0;
	store->par_second = 0;
	store->par_centisecond = 0;
	store->pos_type = 0;	/* positioning system type */
	store->pos_delay = 0.0;	/* positioning system delay (sec) */
	store->roll_offset = 0.0;	/* roll offset (degrees) */
	store->pitch_offset = 0.0;	/* pitch offset (degrees) */
	store->heading_offset = 0.0;	/* heading offset (degrees) */
	store->em100_td = 0.0;	/* EM-100 tranducer depth (meters) */
	store->em100_tx = 0.0;	/* EM-100 tranducer fore-aft 
					offset (meters) */
	store->em100_ty = 0.0;	/* EM-100 tranducer athwartships 
					offset (meters) */
	store->em12_td = 0.0;	/* EM-12 tranducer depth (meters) */
	store->em12_tx = 0.0;	/* EM-12 tranducer fore-aft 
					offset (meters) */
	store->em12_ty = 0.0;	/* EM-12 tranducer athwartships 
					offset (meters) */
	store->em1000_td = 0.0;	/* EM-1000 tranducer depth (meters) */
	store->em1000_tx = 0.0;	/* EM-1000 tranducer fore-aft 
					offset (meters) */
	store->em1000_ty = 0.0;	/* EM-1000 tranducer athwartships 
					offset (meters) */
	for (i=0;i<128;i++)
		store->spare_parameter[i] = '\0';
	store->survey_line = 0;
	for (i=0;i<80;i++)
		store->comment[i] = '\0';

	/* position (position datagrams) */
	store->pos_year = 0;
	store->pos_month = 0;
	store->pos_day = 0;
	store->pos_hour = 0;
	store->pos_minute = 0;
	store->pos_second = 0;
	store->pos_centisecond = 0;
	store->latitude = 0.0;
	store->longitude = 0.0;
	store->utm_northing = 0.0;
	store->utm_easting = 0.0;
	store->utm_zone = 0;
	store->utm_zone_lon = 0.0;
	store->utm_system = 0;
	store->pos_quality = 0;
	store->speed = 0.0;			/* meters/second */
	store->line_heading = 0.0;		/* degrees */

	/* sound velocity profile */
	store->svp_year = 0;
	store->svp_month = 0;
	store->svp_day = 0;
	store->svp_hour = 0;
	store->svp_minute = 0;
	store->svp_second = 0;
	store->svp_centisecond = 0;
	store->svp_num = 0;
	for (i=0;i<100;i++)
		{
		store->svp_depth[i] = 0; /* meters */
		store->svp_vel[i] = 0;	/* 0.1 meters/sec */
		}

	/* time stamp */
	store->year = 0;
	store->month = 0;
	store->day = 0;
	store->hour = 0;
	store->minute = 0;
	store->second = 0;
	store->centisecond = 0;
	store->ping_number = 0;
	store->beams_bath = MBSYS_SIMRAD_MAXBEAMS;
	store->bath_mode = 0;
	store->bath_res = 0;
	store->bath_quality = 0;
	store->keel_depth = 0;
	store->heading = 0;
	store->roll = 0;
	store->pitch = 0;
	store->xducer_pitch = 0;
	store->ping_heave = 0;
	store->sound_vel = 0;
	store->pixels_ss = 0;
	store->ss_mode = 0;
	for (i=0;i<MBSYS_SIMRAD_MAXBEAMS;i++)
		{
		store->bath[i] = 0;
		store->bath_acrosstrack[i] = 0;
		store->bath_alongtrack[i] = 0;
		store->tt[i] = 0;
		store->amp[i] = 0;
		store->quality[i] = 0;
		store->heave[i] = 0;
		store->beam_frequency[i] = 0;
		store->beam_samples[i] = 0;
		store->beam_center_sample[i] = 0;
		store->beam_start_sample[i] = 0;
		}
	for (i=0;i<MBSYS_SIMRAD_MAXPIXELS;i++)
		{
		store->ss[i] = 0;
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
int mbsys_simrad_deall(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
	char	*function_name = "mbsys_simrad_deall";
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
int mbsys_simrad_extract(verbose,mbio_ptr,store_ptr,kind,
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
	char	*function_name = "mbsys_simrad_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	ntime_i[7];
	double	ntime_d;
	short int *beam_ss;
	double	ss_spacing;
	double	dd;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
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
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_i[0] = store->year + 1900;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000*store->centisecond;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		ntime_i[0] = store->pos_year + 1900;
		ntime_i[1] = store->pos_month;
		ntime_i[2] = store->pos_day;
		ntime_i[3] = store->pos_hour;
		ntime_i[4] = store->pos_minute;
		ntime_i[5] = store->pos_second;
		ntime_i[6] = 10000*store->pos_centisecond;
		mb_get_time(verbose,ntime_i,&ntime_d);
		dd = 0.001*(*time_d - ntime_d)*store->speed;
		mb_coor_scale(verbose,store->latitude,&mtodeglon,&mtodeglat);
		headingx = sin(DTR*store->line_heading);
		headingy = cos(DTR*store->line_heading);
		*navlon = store->longitude + headingx*mtodeglon*dd;
		*navlat = store->latitude + headingy*mtodeglat*dd;
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
		*heading = 0.1*store->heading;

		/* get speed  */
		*speed = 3.6*store->speed;

		/* read distance and depth values into storage arrays */
		*nbath = store->beams_bath;
		mb_io_ptr->beams_bath = store->beams_bath;
		*namp = store->beams_bath;
		mb_io_ptr->beams_amp = store->beams_bath;
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			depthscale = 0.02;
			dacrscale  = 0.1;
			daloscale  = 0.1;
			ttscale    = 0.05;
			reflscale  = 0.5;
			}
		else if (store->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		if (store->ss_mode == 1)
			ss_spacing = 0.6;
		else if (store->ss_mode == 2)
			ss_spacing = 2.4;
		else if (store->ss_mode == 3)
			ss_spacing = 0.3;
		else if (store->ss_mode == 4)
			ss_spacing = 0.3;
		else
			ss_spacing = 0.15;
		for (i=0;i<*nbath;i++)
			{
			bath[i] = depthscale*store->bath[i];
			bathacrosstrack[i] 
				= dacrscale*store->bath_acrosstrack[i];
			bathalongtrack[i] 
				= daloscale*store->bath_alongtrack[i];
			}
		for (i=0;i<*namp;i++)
			{
			amp[i] = reflscale*store->amp[i] + 200;
			}
		*nss = 0;
		for (i=0;i<*nbath;i++)
			{
			beam_ss = &store->ss[store->beam_start_sample[i]];
			for (j=0;j<store->beam_samples[i];j++)
				{
				ss[*nss] = beam_ss[j];
				ssacrosstrack[*nss] = dacrscale*bathacrosstrack[i] 
					+ ss_spacing*
					(j - store->beam_center_sample[i]);
				ssalongtrack[*nss] = daloscale*bathalongtrack[i];
				(*nss)++;
				}
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
		  fprintf(stderr,"dbg2       beam:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
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
int mbsys_simrad_insert(verbose,mbio_ptr,store_ptr,
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
	char	*function_name = "mbsys_simrad_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	kind;
	int	time_j[5];
	double	depthscale, dacrscale,daloscale,ttscale,reflscale;
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
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3) 
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->year = time_i[0] - 1900;
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = time_i[6]/10000;

		/* get heading */
		store->heading = (int) (heading * 10);

		/* get speed  */
		store->speed = speed/3.6;

		/* insert distance and depth values into storage arrays */
		store->beams_bath = nbath;
		if (store->sonar == MBSYS_SIMRAD_UNKNOWN)
			{
			if (nbath <= 60)
				{
				store->sonar = MBSYS_SIMRAD_EM1000;
				store->bath_mode = 0;
				}
			else if (nbath <= 81)
				{
				store->sonar = MBSYS_SIMRAD_EM12S;
				store->bath_mode = 0;
				store->bath_res = 2;
				}
			else if (nbath <= 162)
				{
				store->sonar = MBSYS_SIMRAD_EM12D;
				store->bath_mode = 0;
				store->bath_res = 2;
				}
			else
				{
				*error = MB_ERROR_DATA_NOT_INSERTED;
				status = MB_FAILURE;
				}
			}
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			depthscale = 0.02;
			dacrscale  = 0.1;
			daloscale  = 0.1;
			ttscale    = 0.05;
			reflscale  = 0.5;
			}
		else if (store->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else
			{
			*error = MB_ERROR_DATA_NOT_INSERTED;
			status = MB_FAILURE;
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<nbath;i++)
				{
				store->bath[i] = bath[i]/depthscale;
				store->bath_acrosstrack[i]
					= bathacrosstrack[i]/dacrscale;
				store->bath_alongtrack[i] 
					= bathalongtrack[i]/daloscale;
				}
			for (i=0;i<namp;i++)
				{
				store->amp[i] = amp[i]/reflscale - 200;
				}
			}
		if (status == MB_SUCCESS && nss == store->pixels_ss)
			{
			for (i=0;i<store->pixels_ss;i++)
				{
				store->ss[i] = ss[i];
				}
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->comment,comment,79);
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
int mbsys_simrad_ttimes(verbose,mbio_ptr,store_ptr,kind,nbeams,
	ttimes,angles,angles_forward,angles_null,flags,
	depthadd,ssv,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	*nbeams;
double	*ttimes;
double	*angles;
double	*angles_forward;
double	*angles_null;
int	*flags;
double	*depthadd;
double	*ssv;
int	*error;
{
	char	*function_name = "mbsys_simrad_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	double	ttscale;
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
		fprintf(stderr,"dbg2       flags:      %d\n",flags);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = mb_io_ptr->beams_bath;

		/* get travel times, angles, and flags */
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			ttscale = 0.05;
		else if (store->bath_res == 1)
			ttscale = 0.2;
		else if (store->bath_res == 2)
			ttscale = 0.8;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			ttimes[i] = ttscale*store->tt[i];
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = angles[i];
			if (store->bath[i] < 0)
				flags[i] = MB_YES;
			else
				flags[i] = MB_NO;
			}

		/* get depth offset (heave + heave offset) */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			*depthadd = 100.0*store->ping_heave + store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			*depthadd = 100.0*store->ping_heave + store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			*depthadd = 100.0*store->ping_heave + store->em100_td;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			*depthadd = 100.0*store->ping_heave + store->em1000_td;
		*ssv = 0.1 * store->sound_vel;

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
		fprintf(stderr,"dbg2       depthadd:   %f\n",*depthadd);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  flag:%d\n",
				i,ttimes[i],angles[i],
				angles_forward[i],angles_null[i],flags[i]);
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
int mbsys_simrad_extract_nav(verbose,mbio_ptr,store_ptr,kind,
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
	char	*function_name = "mbsys_simrad_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	ntime_i[7];
	double	ntime_d;
	short int *beam_ss;
	double	ss_spacing;
	double	dd;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
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
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_i[0] = store->year + 1900;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000*store->centisecond;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		ntime_i[0] = store->pos_year + 1900;
		ntime_i[1] = store->pos_month;
		ntime_i[2] = store->pos_day;
		ntime_i[3] = store->pos_hour;
		ntime_i[4] = store->pos_minute;
		ntime_i[5] = store->pos_second;
		ntime_i[6] = 10000*store->pos_centisecond;
		mb_get_time(verbose,ntime_i,&ntime_d);
		dd = 0.001*(*time_d - ntime_d)*store->speed;
		mb_coor_scale(verbose,store->latitude,&mtodeglon,&mtodeglat);
		headingx = sin(DTR*store->line_heading);
		headingy = cos(DTR*store->line_heading);
		*navlon = store->longitude + headingx*mtodeglon*dd;
		*navlat = store->latitude + headingy*mtodeglat*dd;
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
		*heading = 0.1*store->heading;

		/* get speed  */
		*speed = 3.6*store->speed;

		/* get roll pitch and heave */
		*roll = 0.01*store->roll;
		*pitch = 0.01*store->pitch;
		*heave = 0.01*store->ping_heave;

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
int mbsys_simrad_insert_nav(verbose,mbio_ptr,store_ptr,
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
	char	*function_name = "mbsys_simrad_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	kind;
	int	time_j[5];
	double	depthscale, dacrscale,daloscale,ttscale,reflscale;
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
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->year = time_i[0] - 1900;
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = time_i[6]/10000;

		/* get heading */
		store->heading = (int) (heading * 10);

		/* get speed  */
		store->speed = speed/3.6;

		/* get roll pitch and heave */
		store->roll = roll*100.0;
		store->pitch = pitch*100.0;
		store->ping_heave = heave*100.0;
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
int mbsys_simrad_copy(verbose,mbio_ptr,store_ptr,copy_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
char	*copy_ptr;
int	*error;
{
	char	*function_name = "mbsys_simrad_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_struct *copy;

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
	store = (struct mbsys_simrad_struct *) store_ptr;
	copy = (struct mbsys_simrad_struct *) copy_ptr;

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
