/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad.c	3.00	8/5/94
 *	$Id: mbsys_simrad.c,v 5.15 2007-10-08 15:59:34 caress Exp $
 *
 *    Copyright (c) 1994, 2000, 2002, 2003 by
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
 * mbsys_simrad.c contains the MBIO functions for handling data from 
 * old (pre-1997) Simrad multibeam sonars (e.g. EM950, EM1000, EM12S, 
 * EM12D). The data formats associated with Simrad multibeams 
 * (both old and new) include:
 *    MBSYS_SIMRAD formats (code in mbsys_simrad.c and mbsys_simrad.h):
 *      MBF_EMOLDRAW : MBIO ID 51 - Vendor EM1000, EM12S, EM12D, EM121
 *                   : MBIO ID 52 - aliased to 51
 *      MBF_EM12IFRM : MBIO ID 53 - IFREMER EM12S and EM12D
 *      MBF_EM12DARW : MBIO ID 54 - NERC EM12S
 *                   : MBIO ID 55 - aliased to 51
 *    MBSYS_SIMRAD2 formats (code in mbsys_simrad2.c and mbsys_simrad2.h):
 *      MBF_EM300RAW : MBIO ID 56 - Vendor EM3000, EM300, EM120 
 *      MBF_EM300MBA : MBIO ID 57 - MBARI EM3000, EM300, EM120
 *
 * Author:	D. W. Caress
 * Date:	August 5, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.14  2007/06/18 01:19:48  caress
 * Changes as of 17 June 2007.
 *
 * Revision 5.13  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.12  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.11  2003/12/04 23:10:24  caress
 * Fixed problems with format 54 EM12DARW due to old code assuming how internal structure was packed. Also changed handling of beamflags for formats that don't support beamflags. Now flagged beams will always be nulled in such cases.
 *
 * Revision 5.10  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.9  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.8  2002/08/21 00:55:46  caress
 * Release 5.0.beta22
 *
 * Revision 5.7  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.6  2002/05/29 23:40:48  caress
 * Release 5.0.beta18
 *
 * Revision 5.5  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.4  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.2  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.21  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.20  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.19  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.18  1998/12/18  20:49:54  caress
 * MB-System version 4.6beta5
 *
 * Revision 4.17  1998/12/18  01:39:32  caress
 * MB-System version 4.6beta5
 *
 * Revision 4.16  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.15  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.14  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.13  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.12  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.12  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.11  1996/08/26  18:33:50  caress
 * Changed "signed char" to "char" for SunOs 4.1 compiler compatibility.
 *
 * Revision 4.10  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.9  1996/07/26  21:06:00  caress
 * Version after first cut of handling em12s and em121 data.
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
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
#define MBSYS_SIMRAD_C
#include "../../include/mbsys_simrad.h"

static char res_id[]="$Id: mbsys_simrad.c,v 5.15 2007-10-08 15:59:34 caress Exp $";

/*--------------------------------------------------------------------*/
int mbsys_simrad_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
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
	store->pos_latitude = 0.0;
	store->pos_longitude = 0.0;
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
	
	/* survey data structure not allocated yet */
	store->ping = NULL;

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
int mbsys_simrad_survey_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_simrad_survey_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
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
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->ping == NULL)
		status = mb_malloc(verbose,
			sizeof(struct mbsys_simrad_survey_struct),
			&(store->ping),error);
			
	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* initialize everything */
		ping->swath_id = EM_SWATH_CENTER;
		ping->longitude = 0.0;
		ping->latitude = 0.0;
		ping->ping_number = 0;
		ping->beams_bath = MBSYS_SIMRAD_MAXBEAMS;
		ping->bath_mode = 0;
		ping->bath_res = 0;
		ping->bath_quality = 0;
		ping->bath_num = 0;
		ping->pulse_length = 0;
		ping->beam_width = 0;
		ping->power_level = 0;
		ping->tx_status = 0;
		ping->rx_status = 0;
		ping->along_res = 0;
		ping->across_res = 0;
		ping->depth_res = 0;
		ping->range_res = 0;
		ping->keel_depth = 0;
		ping->heading = 0;
		ping->roll = 0;
		ping->pitch = 0;
		ping->xducer_pitch = 0;
		ping->ping_heave = 0;
		ping->sound_vel = 0;
		ping->ss_status = EM_SS_NONE;
		ping->pixels_ssraw = 0;
		ping->ss_mode = 0;
		for (i=0;i<MBSYS_SIMRAD_MAXBEAMS;i++)
			{
			ping->bath[i] = 0;
			ping->bath_acrosstrack[i] = 0;
			ping->bath_alongtrack[i] = 0;
			ping->tt[i] = 0;
			ping->amp[i] = 0;
			ping->quality[i] = 0;
			ping->heave[i] = 0;
			ping->beamflag[i] = MB_FLAG_NULL;
			ping->beam_frequency[i] = 0;
			ping->beam_samples[i] = 0;
			ping->beam_center_sample[i] = 0;
			ping->beam_start_sample[i] = 0;
			}
		ping->pixel_size = 0.0;
		ping->pixels_ss = 0;
		for (i=0;i<MBSYS_SIMRAD_MAXRAWPIXELS;i++)
			{
			ping->ssraw[i] = 0;
			ping->ssp[i] = 0;
			}
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
int mbsys_simrad_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_simrad_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;

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

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) *store_ptr;

	/* deallocate memory for survey data structure */
	if (store->ping != NULL)
		status = mb_free(verbose,&(store->ping),error);

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
int mbsys_simrad_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_simrad_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;

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
		/* get beam and pixel numbers */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		*nbath = ping->beams_bath;
		*namp = ping->beams_bath;
		*nss = MBSYS_SIMRAD_MAXPIXELS;
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		fprintf(stderr,"dbg2        namp:      %d\n",*namp);
		fprintf(stderr,"dbg2        nss:       %d\n",*nss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	mb_s_char	*beam_ss;
	double	ss_spacing;
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	double	pixel_size;
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		
		/* get time */
		mb_fix_y2k(verbose, store->year, &time_i[0]);
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000*store->centisecond;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ping->longitude;
		*navlat = ping->latitude;

		/* get heading */
		if (store->sonar == MBSYS_SIMRAD_EM121)
			*heading = 0.01 * ping->heading;
		else
			*heading = 0.1 * ping->heading;

		/* get speed  */
		*speed = 3.6*store->speed;
			
		/* set beamwidths in mb_io structure */
		if (store->sonar == MBSYS_SIMRAD_EM1000)
		    {
		    mb_io_ptr->beamwidth_ltrack = 3.3;
		    mb_io_ptr->beamwidth_xtrack = 3.3;
		    }
		else if (store->sonar == MBSYS_SIMRAD_EM12S
			|| store->sonar == MBSYS_SIMRAD_EM12D)
		    {
		    mb_io_ptr->beamwidth_ltrack = 1.7;
		    mb_io_ptr->beamwidth_xtrack = 3.5;
		    }
		else if (store->sonar == MBSYS_SIMRAD_EM121)
		    {
		    if (ping->bath_mode == 3)
			{
			mb_io_ptr->beamwidth_ltrack = 4.0;
			mb_io_ptr->beamwidth_xtrack = 4.0;
			}
		    else if (ping->bath_mode == 2)
			{
			mb_io_ptr->beamwidth_ltrack = 2.0;
			mb_io_ptr->beamwidth_xtrack = 2.0;
			}
		    else
			{
			mb_io_ptr->beamwidth_ltrack = 1.0;
			mb_io_ptr->beamwidth_xtrack = 1.0;
			}
		    }

		/* read distance and depth values into storage arrays */
		*nbath = ping->beams_bath;
		*namp = ping->beams_bath;
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			depthscale = 0.02;
			dacrscale  = 0.1;
			daloscale  = 0.1;
			ttscale    = 0.05;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			depthscale = 0.01 * ping->depth_res;
			dacrscale  = 0.01 * ping->across_res;
			daloscale  = 0.01 * ping->along_res;
			ttscale    = 0.1 * ping->range_res;
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
		if (ping->ss_mode == 1)
			ss_spacing = 0.6;
		else if (ping->ss_mode == 2)
			ss_spacing = 2.4;
		else if (ping->ss_mode == 3)
			ss_spacing = 0.3;
		else if (ping->ss_mode == 4)
			ss_spacing = 0.3;
		else
			ss_spacing = 0.15;
		for (i=0;i<*nbath;i++)
			{
			beamflag[i] = ping->beamflag[i];
			bath[i] = depthscale*ping->bath[i];
			bathacrosstrack[i] 
				= dacrscale*ping->bath_acrosstrack[i];
			bathalongtrack[i] 
				= daloscale*ping->bath_alongtrack[i];
			}
		for (i=0;i<*namp;i++)
			{
			amp[i] = reflscale*ping->amp[i];
			}
		if (ss != NULL)
			{
			*nss = MBSYS_SIMRAD_MAXPIXELS;
			pixel_size = 0.01 * ping->pixel_size;
			for (i=0;i<MBSYS_SIMRAD_MAXPIXELS;i++)
				{
				ss[i] = 0.01 * ping->ss[i];
				ssacrosstrack[i] = pixel_size 
						* (i - MBSYS_SIMRAD_MAXPIXELS / 2);
				ssalongtrack[i] = daloscale * ping->ssalongtrack[i];
				}
			}
		else
			{
			*nss = 0;
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
				i,beamflag[i],bath[i],
				bathacrosstrack[i],bathalongtrack[i]);
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

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* get time */
		mb_fix_y2k(verbose, store->pos_year, &time_i[0]);
		time_i[1] = store->pos_month;
		time_i[2] = store->pos_day;
		time_i[3] = store->pos_hour;
		time_i[4] = store->pos_minute;
		time_i[5] = store->pos_second;
		time_i[6] = 10000*store->pos_centisecond;
		mb_get_time(verbose,time_i,time_d);
		*navlon = store->pos_longitude;
		*navlat = store->pos_latitude;

		/* get heading */
		*heading = store->line_heading;

		/* get speed  */
		*speed = 3.6*store->speed;

		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
int mbsys_simrad_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
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
		  fprintf(stderr,"dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
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
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* allocate secondary data structure for
			survey data if needed */
		if (store->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->year);
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = time_i[6]/10000;
		
		/* get nav */
		ping->longitude = navlon;
		ping->latitude = navlat;

		/* get heading */
		if (store->sonar == MBSYS_SIMRAD_EM121)
			ping->heading = (int) (heading * 100);
		else
			ping->heading = (int) (heading * 10);

		/* get speed  */
		store->speed = speed/3.6;

		/* insert distance and depth values into storage arrays */
		ping->beams_bath = nbath;
		if (store->sonar == MBSYS_SIMRAD_UNKNOWN)
			{
			if (nbath <= 60)
				{
				store->sonar = MBSYS_SIMRAD_EM1000;
				ping->bath_mode = 0;
				}
			else if (nbath <= 81)
				{
				store->sonar = MBSYS_SIMRAD_EM12S;
				ping->bath_mode = 0;
				ping->bath_res = 2;
				}
			else if (nbath <= 121)
				{
				store->sonar = MBSYS_SIMRAD_EM121;
				ping->bath_mode = 0;
				ping->bath_res = 2;
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
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			ttscale    = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			ttscale    = 0.8;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			depthscale = 0.01 * ping->depth_res;
			dacrscale  = 0.01 * ping->across_res;
			daloscale  = 0.01 * ping->along_res;
			ttscale    = 0.1 * ping->range_res;
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
				ping->bath[i] = bath[i]/depthscale;
				ping->bath_acrosstrack[i]
					= bathacrosstrack[i]/dacrscale;
				ping->bath_alongtrack[i] 
					= bathalongtrack[i]/daloscale;
				ping->beamflag[i] = beamflag[i];
				if (beamflag[i] == MB_FLAG_NULL)
				    ping->bath[i] = 0;
				}
			for (i=0;i<namp;i++)
				{
				ping->amp[i] = amp[i] / reflscale;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<nss;i++)
				{
				ping->ss[i] = 100 * ss[i];
				ping->ssalongtrack[i] = ssalongtrack[i] / daloscale;
				}
			}
		}

	/* insert nav in structure */
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->pos_year);
		store->pos_month = time_i[1];
		store->pos_day = time_i[2];
		store->pos_hour = time_i[3];
		store->pos_minute = time_i[4];
		store->pos_second = time_i[5];
		store->pos_centisecond = time_i[6]/10000;
		
		/* get nav */
		store->pos_longitude = navlon;
		store->pos_latitude = navlat;

		/* get heading */
		store->line_heading = heading;

		/* get speed  */
		store->speed = speed/3.6;
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
int mbsys_simrad_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_simrad_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	ttscale;
	double	heave_use;
	double	*angles_simrad;
	double	alpha, beta;
	int	istep = 0;
	int	interleave = 0;
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
		fprintf(stderr,"dbg2       heave:      %d\n",heave);
		fprintf(stderr,"dbg2       ltrk_off:   %d\n",alongtrack_offset);
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* get nbeams */
		*nbeams = ping->beams_bath;

		/* get depth offset (heave + heave offset) */
		heave_use =  0.01 * ping->ping_heave;
		*ssv = 0.1 * ping->sound_vel;
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			*draft = store->em100_td;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			*draft = store->em1000_td;

		/* get travel times, angles */
		interleave = MB_NO;
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			if (ping->bath_mode == 1)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_60_2_MS_48_FAIS;
			    interleave = MB_NO;
			    }
			else if (ping->bath_mode == 2)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_120_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 3)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_150_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 4)
			    {
			    angles_simrad = angles_EM1000_CHANNEL_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 5)
			    {
			    angles_simrad = angles_EM1000_150_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 6)
			    {
			    angles_simrad = angles_EM1000_140_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 7)
			    {
			    angles_simrad = angles_EM1000_128_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 8)
			    {
			    angles_simrad = angles_EM1000_120_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 9)
			    {
			    angles_simrad = angles_EM1000_104_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 10)
			    {
			    angles_simrad = angles_EM1000_88_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 11)
			    {
			    angles_simrad = angles_EM1000_70_2_MS_48_FAIS;
			    interleave = MB_NO;
			    }
			else if (ping->bath_mode == 12)
			    {
			    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 13)
			    {
			    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12S_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12S_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12S_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12S_120;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12S_105;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12S_90;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D
			&& ping->swath_id == EM_SWATH_PORT)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12DP_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12DP_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12DP_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12DP_150;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12DP_140;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12DP_128;			
			else if (ping->bath_mode == 7)
			    angles_simrad = angles_EM12DP_114;			
			else if (ping->bath_mode == 8)
			    angles_simrad = angles_EM12DP_98;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D
			&& ping->swath_id == EM_SWATH_STARBOARD)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12DS_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12DS_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12DS_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12DS_150;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12DS_140;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12DS_128;			
			else if (ping->bath_mode == 7)
			    angles_simrad = angles_EM12DS_114;			
			else if (ping->bath_mode == 8)
			    angles_simrad = angles_EM12DS_98;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			angles_simrad = angles_EM121_GUESS;
			}
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			ttscale = 0.00005;
			}
		else if ((store->sonar == MBSYS_SIMRAD_EM12S
			    || store->sonar == MBSYS_SIMRAD_EM12D)
			&& ping->bath_res == 1)
			{
			ttscale    = 0.0002;
			}
		else if ((store->sonar == MBSYS_SIMRAD_EM12S
			    || store->sonar == MBSYS_SIMRAD_EM12D)
			&& ping->bath_res == 2)
			ttscale    = 0.0008;
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			ttscale    = 0.0001 * ping->range_res;
		else
			ttscale    = 0.0002;
			
		/* if interleaved get center beam */
		if (interleave == MB_YES)
			{
			if (ping->bath_mode == 12
			    && fabs(ping->bath_acrosstrack[28])
				< fabs(ping->bath_acrosstrack[29]))
			    istep = 1;
			else if (ping->bath_mode == 13
			    && fabs(ping->bath_acrosstrack[31])
				< fabs(ping->bath_acrosstrack[30]))
			    istep = 1;
			else if (fabs(ping->bath_acrosstrack[*nbeams/2-1])
			    < fabs(ping->bath_acrosstrack[*nbeams/2]))
			    istep = 1;
			else
			    istep = 0;
			}
		
		/* get travel times and angles */
		for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = ttscale * ping->tt[i];
			alpha = 0.01 * ping->pitch;
			if (store->sonar == MBSYS_SIMRAD_EM1000
			    && ping->bath_mode == 13)
			    {
			    beta = 90.0 - angles_simrad[*nbeams-1-(2*i+istep)];
			    }
			else if (store->sonar == MBSYS_SIMRAD_EM1000
			    && interleave == MB_YES)
			    {
			    beta = 90.0 + angles_simrad[2*i+istep];
			    }
			else if (store->sonar == MBSYS_SIMRAD_EM1000)
			    {
			    beta = 90.0 + angles_simrad[i];
			    }
			else
			    {
			    beta = 90.0 + angles_simrad[i];
			    }
			mb_rollpitch_to_takeoff(verbose, 
				alpha, beta, &angles[i], 
				&angles_forward[i], error);
			if (store->sonar == MBSYS_SIMRAD_EM1000)
			    angles_null[i] = angles[i];
			else if (store->sonar == MBSYS_SIMRAD_EM1000)
			    angles_null[i] = angles[i];
			else if (store->sonar == MBSYS_SIMRAD_EM12S)
			    angles_null[i] = 0.0;
			else if (store->sonar == MBSYS_SIMRAD_EM12D)
			    angles_null[i] = 0.0; /* wrong for sure */
			heave[i] = heave_use;
			alongtrack_offset[i] = 0.0;
			}
		
		/* reset null angles for EM1000 outer beams */
		if (store->sonar == MBSYS_SIMRAD_EM1000
		    && *nbeams == 60)
			{
			for (i=0;i<6;i++)
			    angles_null[i] = angles_null[6];
			for (i=55;i<=60;i++)
			    angles_null[i] = angles_null[54];
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
		fprintf(stderr,"dbg2       draft:      %f\n",*draft);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
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
int mbsys_simrad_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_simrad_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       detects:    %d\n",detects);
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		*nbeams = ping->beams_bath;
		for (i=0;i<ping->beams_bath;i++)
			{
			if (ping->bath[i] == 0)
				detects[i] = MB_DETECT_UNKNOWN;
			else if (ping->quality[i] & 128)
				detects[i] = MB_DETECT_PHASE;
			else
				detects[i] = MB_DETECT_AMPLITUDE;
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
int mbsys_simrad_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_simrad_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	depthscale, dacrscale;
	double	bath_best;
	double	xtrack_min;
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* get transducer depth and altitude */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			*transducer_depth = 0.01*ping->ping_heave + store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			*transducer_depth = 0.01*ping->ping_heave + store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			*transducer_depth = 0.01*ping->ping_heave + store->em100_td;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			*transducer_depth = 0.01*ping->ping_heave + store->em1000_td;
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			*transducer_depth = 0.01*ping->ping_heave + store->em12_td;
		else
			*transducer_depth = 0.0;
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			depthscale = 0.02;
			dacrscale  = 0.1;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			depthscale = 0.01 * ping->depth_res;
			dacrscale  = 0.01 * ping->across_res;
			}
		else
			{
			depthscale = 0.1;
			dacrscale  = 0.1;
			}
		bath_best = 0.0;
		if (ping->bath[ping->beams_bath/2] > 0)
		    bath_best = depthscale * ping->bath[ping->beams_bath/2];
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<ping->beams_bath;i++)
			{
			if (ping->bath[i] > 0.0
			    && fabs(dacrscale * ping->bath_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(dacrscale * ping->bath_acrosstrack[i]);
			    bath_best = depthscale * ping->bath[i];
			    }
			}		
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<ping->beams_bath;i++)
			{
			if (ping->bath[i] < 0.0
			    && fabs(dacrscale * ping->bath_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(dacrscale * ping->bath_acrosstrack[i]);
			    bath_best = -depthscale * ping->bath[i];
			    }
			}		
		    }
		*altitude = bath_best - *transducer_depth;

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
int mbsys_simrad_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_simrad_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* get time */
		mb_fix_y2k(verbose, store->year, &time_i[0]);
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000*store->centisecond;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = ping->longitude;
		*navlat = ping->latitude;

		/* get heading */
		if (store->sonar == MBSYS_SIMRAD_EM121)
			*heading = 0.01 * ping->heading;
		else
			*heading = 0.1 * ping->heading;

		/* get speed  */
		*speed = 3.6*store->speed;

		/* get draft  */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			*draft = store->em100_td;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			*draft = store->em1000_td;

		/* get roll pitch and heave */
		*roll = 0.01*ping->roll;
		*pitch = 0.01*ping->pitch;
		*heave = 0.01*ping->ping_heave;

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

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get time */
		mb_fix_y2k(verbose, store->pos_year, &time_i[0]);
		time_i[1] = store->pos_month;
		time_i[2] = store->pos_day;
		time_i[3] = store->pos_hour;
		time_i[4] = store->pos_minute;
		time_i[5] = store->pos_second;
		time_i[6] = 10000*store->pos_centisecond;
		mb_get_time(verbose,time_i,time_d);
		*navlon = store->pos_longitude;
		*navlat = store->pos_latitude;

		/* get heading */
		*heading = store->line_heading;

		/* get speed  */
		*speed = 3.6 * store->speed;

		/* get draft  */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			*draft = store->em12_td;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			*draft = store->em100_td;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			*draft = store->em1000_td;

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
int mbsys_simrad_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_simrad_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	int	kind;
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
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
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
		/* get survey data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->year);
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = time_i[6]/10000;
		
		/* get nav */
		ping->longitude = navlon;
		ping->latitude = navlat;

		/* get heading */
		if (store->sonar == MBSYS_SIMRAD_EM121)
			ping->heading = (int) (heading * 100);
		else
			ping->heading = (int) (heading * 10);

		/* get speed  */
		store->speed = speed/3.6;

		/* get draft  */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			store->em12_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			store->em12_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			store->em100_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			store->em1000_td = draft;

		/* get roll pitch and heave */
		ping->roll = roll*100.0;
		ping->pitch = pitch*100.0;
		ping->ping_heave = heave*100.0;
		}

	/* insert nav in structure */
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->pos_year);
		store->pos_month = time_i[1];
		store->pos_day = time_i[2];
		store->pos_hour = time_i[3];
		store->pos_minute = time_i[4];
		store->pos_second = time_i[5];
		store->pos_centisecond = time_i[6]/10000;
		
		/* get nav */
		store->pos_longitude = navlon;
		store->pos_latitude = navlat;

		/* get heading */
		store->line_heading = heading;

		/* get speed  */
		store->speed = speed/3.6;

		/* get draft  */
		if (store->sonar == MBSYS_SIMRAD_EM12S)
			store->em12_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM12D)
			store->em12_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM100)
			store->em100_td = draft;
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
			store->em1000_td = draft;
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
int mbsys_simrad_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
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
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;
		
		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = store->svp_depth[i];
			velocity[i] = 0.1 * store->svp_vel[i];
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
int mbsys_simrad_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	int	kind;
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
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_SIMRAD_MAXSVP);
		
		/* get profile */
		for (i=0;i<store->svp_num;i++)
			{
			store->svp_depth[i] = (int) depth[i];
			store->svp_vel[i] = (int) (10 * velocity[i]);
			}
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
int mbsys_simrad_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_struct *copy;
	struct mbsys_simrad_survey_struct *ping_store;
	struct mbsys_simrad_survey_struct *ping_copy;
	char	*ping_save;

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
	
	/* check if survey data needs to be copied */
	if (store->kind == MB_DATA_DATA 
		&& store->ping != NULL)
		{
		/* make sure a survey data structure exists to
			be copied into */
		if (copy->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}
			
		/* save pointer value */
		ping_save = (char *)copy->ping;
		}
	else
		ping_save = NULL;

	/* copy the main structure */
	*copy = *store;
	
	/* if needed copy the survey data structure */
	if (store->kind == MB_DATA_DATA 
		&& store->ping != NULL 
		&& status == MB_SUCCESS)
		{
		copy->ping = (struct mbsys_simrad_survey_struct *) ping_save;
		ping_store = (struct mbsys_simrad_survey_struct *) store->ping;
		ping_copy = (struct mbsys_simrad_survey_struct *) copy->ping;
		*ping_copy = *ping_store;
		}
	else
		copy->ping = NULL;

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
int mbsys_simrad_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size, 
		int swath_width_set, double *swath_width, 
		int pixel_int, 
		int *error)
{
	char	*function_name = "mbsys_simrad_makess";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	ss[MBSYS_SIMRAD_MAXPIXELS];
	int	ss_cnt[MBSYS_SIMRAD_MAXPIXELS];
	double	ssacrosstrack[MBSYS_SIMRAD_MAXPIXELS];
	double	ssalongtrack[MBSYS_SIMRAD_MAXPIXELS];
	mb_s_char *beam_ss;
	int	nbathsort;
	double	bathsort[MBSYS_SIMRAD_MAXBEAMS];
	double	depthscale, depthoffset;
	double	dacrscale, daloscale;
	double	reflscale;
	double  pixel_size_calc;
	double	ss_spacing, ss_spacing_use;
	double	*angles_simrad;
	int	pixel_int_use;
	double	depth, xtrack, ltrack, xtrackss;
	double	range, beam_foot, beamwidth, sint;
	double	angle;
	int	interleave, istep;
	int	first, last, k1, k2;
	int	i, j, k, kk, l, m;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:        %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:       %d\n",store_ptr);
		fprintf(stderr,"dbg2       pixel_size_set:  %d\n",pixel_size_set);
		fprintf(stderr,"dbg2       pixel_size:      %f\n",*pixel_size);
		fprintf(stderr,"dbg2       swath_width_set: %d\n",swath_width_set);
		fprintf(stderr,"dbg2       swath_width:     %f\n",*swath_width);
		fprintf(stderr,"dbg2       pixel_int:       %d\n",pixel_int);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get pointer to raw data structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;

		/* zero the sidescan */
		for (i=0;i<MBSYS_SIMRAD_MAXPIXELS;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
			}

		/* set scaling parameters */
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			depthscale = 0.02;
			dacrscale  = 0.1;
			daloscale  = 0.1;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D 
			&& ping->bath_res == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			reflscale  = 0.5;
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			depthscale = 0.01 * ping->depth_res;
			dacrscale  = 0.01 * ping->across_res;
			daloscale  = 0.01 * ping->along_res;
			reflscale  = 0.5;
			}
		else
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			reflscale  = 0.5;
			}
		depthoffset = 0.0;

		/* get angles */
		interleave = MB_NO;
		if (store->sonar == MBSYS_SIMRAD_EM1000)
			{
			if (ping->bath_mode == 1)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_60_2_MS_48_FAIS;
			    interleave = MB_NO;
			    }
			else if (ping->bath_mode == 2)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_120_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 3)
			    {
			    angles_simrad = angles_EM1000_ISO_ANG_150_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 4)
			    {
			    angles_simrad = angles_EM1000_CHANNEL_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 5)
			    {
			    angles_simrad = angles_EM1000_150_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 6)
			    {
			    angles_simrad = angles_EM1000_140_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 7)
			    {
			    angles_simrad = angles_EM1000_128_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 8)
			    {
			    angles_simrad = angles_EM1000_120_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 9)
			    {
			    angles_simrad = angles_EM1000_104_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 10)
			    {
			    angles_simrad = angles_EM1000_88_07_MS_48_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 11)
			    {
			    angles_simrad = angles_EM1000_70_2_MS_48_FAIS;
			    interleave = MB_NO;
			    }
			else if (ping->bath_mode == 12)
			    {
			    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			else if (ping->bath_mode == 13)
			    {
			    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
			    interleave = MB_YES;
			    }
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12S)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12S_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12S_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12S_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12S_120;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12S_105;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12S_90;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D
			&& ping->swath_id == EM_SWATH_PORT)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12DP_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12DP_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12DP_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12DP_150;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12DP_140;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12DP_128;			
			else if (ping->bath_mode == 7)
			    angles_simrad = angles_EM12DP_114;			
			else if (ping->bath_mode == 8)
			    angles_simrad = angles_EM12DP_98;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM12D
			&& ping->swath_id == EM_SWATH_STARBOARD)
			{
			if (ping->bath_mode == 1)
			    angles_simrad = angles_EM12DS_ISO_ANG_SHALLOW;
			else if (ping->bath_mode == 2)
			    angles_simrad = angles_EM12DS_ISO_ANG_DEEP;
			else if (ping->bath_mode == 3)
			    angles_simrad = angles_EM12DS_SHALLOW;
			else if (ping->bath_mode == 4)
			    angles_simrad = angles_EM12DS_150;
			else if (ping->bath_mode == 5)
			    angles_simrad = angles_EM12DS_140;
			else if (ping->bath_mode == 6)
			    angles_simrad = angles_EM12DS_128;			
			else if (ping->bath_mode == 7)
			    angles_simrad = angles_EM12DS_114;			
			else if (ping->bath_mode == 8)
			    angles_simrad = angles_EM12DS_98;			
			}
		else if (store->sonar == MBSYS_SIMRAD_EM121)
			{
			angles_simrad = angles_EM121_GUESS;
			}
			
		/* if interleaved get center beam */
		if (interleave == MB_YES)
			{
			if (ping->bath_mode == 12
			    && fabs(ping->bath_acrosstrack[28])
				< fabs(ping->bath_acrosstrack[29]))
			    istep = 1;
			else if (ping->bath_mode == 13
			    && fabs(ping->bath_acrosstrack[31])
				< fabs(ping->bath_acrosstrack[30]))
			    istep = 1;
			else if (fabs(ping->bath_acrosstrack[ping->beams_bath/2-1])
			    < fabs(ping->bath_acrosstrack[ping->beams_bath/2]))
			    istep = 1;
			else
			    istep = 0;
			}

		/* get raw pixel size */
		if (store->sonar == MBSYS_SIMRAD_EM12D
		    || store->sonar == MBSYS_SIMRAD_EM12S
		    || store->sonar == MBSYS_SIMRAD_EM121)
		    {
		    if (ping->ss_mode == 1)
			ss_spacing = 0.6;
		    else if (ping->ss_mode == 2)
			ss_spacing = 2.4;
		    else if (ping->bath_mode == 1
				|| ping->bath_mode == 3)
			ss_spacing = 0.6;
		    else
			ss_spacing = 2.4;
		    }
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
		    {
		    if (ping->ss_mode == 3)
			ss_spacing = 0.3;
		    else if (ping->ss_mode == 4)
			ss_spacing = 0.3;
		    else if (ping->ss_mode == 5)
			ss_spacing = 0.15;
		    else
			ss_spacing = 0.15;
		    }

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD_EM12D
		    || store->sonar == MBSYS_SIMRAD_EM12S)
		    {
		    beamwidth = 2.00;
		    }
		else if (store->sonar == MBSYS_SIMRAD_EM121)
		    {
		    beamwidth = ping->beam_width;
		    }
		else if (store->sonar == MBSYS_SIMRAD_EM1000)
		    {
		    beamwidth = 2.5;
		    }

		/* get median depth */
		nbathsort = 0;
		for (i=0;i<ping->beams_bath;i++)
		    {
		    if (ping->bath[i] > 0.0)
			{
			bathsort[nbathsort] = depthscale 
				* ping->bath[i];
			nbathsort++;
			}
		    }
	
		/* get sidescan pixel size */
		if (swath_width_set == MB_NO
		    && nbathsort > 0)
		    {
		    (*swath_width) = 2.5 + angles_simrad[0];
		    (*swath_width) = MAX((*swath_width), 60.0);
		    }
		if (pixel_size_set == MB_NO
		    && nbathsort > 0)
		    {
		    qsort((char *)bathsort, nbathsort, sizeof(double),(void *)mb_double_compare);
		    pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort/2] 
					/ MBSYS_SIMRAD_MAXPIXELS;
		    pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort/2] * sin(DTR * 0.1));
		    if ((*pixel_size) <= 0.0)
			(*pixel_size) = pixel_size_calc;
		    else if (0.95 * (*pixel_size) > pixel_size_calc)
			(*pixel_size) = 0.95 * (*pixel_size);
		    else if (1.05 * (*pixel_size) < pixel_size_calc)
			(*pixel_size) = 1.05 * (*pixel_size);
		    else
			(*pixel_size) = pixel_size_calc;
		    }
		    
		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Sidescan regenerated in <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       depthscale:    %f\n",depthscale);
			fprintf(stderr,"dbg2       dacrscale:     %f\n",dacrscale);
			fprintf(stderr,"dbg2       daloscale:     %f\n",daloscale);
			fprintf(stderr,"dbg2       reflscale:     %f\n",reflscale);
			fprintf(stderr,"dbg2       depthscale:    %f\n",depthscale);
			fprintf(stderr,"dbg2       depthoffset:   %f\n",depthoffset);
			fprintf(stderr,"dbg2       depthscale:    %f\n",depthscale);
			fprintf(stderr,"dbg2       ss_spacing:    %f\n",ss_spacing);
			fprintf(stderr,"dbg2       pixel_size:    %f\n",*pixel_size);
			fprintf(stderr,"dbg2       swath_width:   %f\n",*swath_width);
			for (i=0;i<ping->beams_bath;i++)
			  fprintf(stderr,"dbg2       beam:%d  bath: %d %d %d freq:%d nsamp:%d center:%d start:%d\n",
				i,
				ping->bath[i],ping->bath_acrosstrack[i],ping->bath_alongtrack[i], 
				ping->beam_frequency[i],ping->beam_samples[i],
				ping->beam_center_sample[i],ping->beam_start_sample[i]);
			}
			
		/* loop over raw sidescan, putting each raw pixel into
			the binning arrays */
		for (i=0;i<ping->beams_bath;i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			if (ping->bath[i] > 0.0)
			    {
			    if (ping->beam_samples[i] > 0)
				{
				depth = depthscale * ping->bath[i];
				xtrack = dacrscale * ping->bath_acrosstrack[i];
				ltrack = daloscale * ping->bath_alongtrack[i];
				range = sqrt(depth * depth + xtrack * xtrack);
				if (store->sonar == MBSYS_SIMRAD_EM1000
				    && ping->bath_mode == 13)
				    {
				    angle = angles_simrad[ping->beams_bath-1-(2*i+istep)];
				    }
				else if (store->sonar == MBSYS_SIMRAD_EM1000
				    && interleave == MB_YES)
				    {
				    angle = -angles_simrad[2*i+istep];
				    }
				else if (store->sonar == MBSYS_SIMRAD_EM1000)
				    {
				    angle = -angles_simrad[i];
				    }
				else
				    {
				    angle = -angles_simrad[i];
				    }
				beam_foot = range * sin(DTR * beamwidth)
							/ cos(DTR * angle);
				sint = fabs(sin(DTR * angle));
				if (sint < ping->beam_samples[i] * ss_spacing / beam_foot)
				    ss_spacing_use = beam_foot / ping->beam_samples[i];
				else
				    ss_spacing_use = ss_spacing / sint;
/*fprintf(stderr, "spacing: %f %f n:%d sint:%f angle:%f range:%f foot:%f factor:%f\n", 
ss_spacing, ss_spacing_use, 
ping->beam_samples[i], sint, angle, range, beam_foot, 
ping->beam_samples[i] * ss_spacing / beam_foot);*/
				}
			    for (k=0;k<ping->beam_samples[i];k++)
				{
				xtrackss = xtrack
				    + ss_spacing_use * (k - ping->beam_center_sample[i]);
				kk = MBSYS_SIMRAD_MAXPIXELS / 2 
				    + (int)(xtrackss / (*pixel_size));
				if (kk > 0 && kk < MBSYS_SIMRAD_MAXPIXELS)
				    {
				    ss[kk]  += reflscale*((double)beam_ss[k]);
				    ssalongtrack[kk] 
					    += ltrack;
				    ss_cnt[kk]++;
				    }
				}
			    }
			}
			
		/* average the sidescan */
		first = MBSYS_SIMRAD_MAXPIXELS;
		last = -1;
		for (k=0;k<MBSYS_SIMRAD_MAXPIXELS;k++)
			{
			if (ss_cnt[k] > 0)
				{
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] 
					= (k - MBSYS_SIMRAD_MAXPIXELS / 2)
						* (*pixel_size);
				first = MIN(first, k);
				last = k;
				}
			else
				ss[k] = MB_SIDESCAN_NULL;	
			}
			
		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k=first+1;k<last;k++)
		    {
		    if (ss_cnt[k] <= 0)
			{
			if (k2 <= k)
			    {
			    k2 = k+1;
			    while (ss_cnt[k2] <= 0 && k2 < last)
				k2++;
			    }
			if (k2 - k1 <= pixel_int_use)
			    {
			    ss[k] = ss[k1]
				+ (ss[k2] - ss[k1])
				    * ((double)(k - k1)) / ((double)(k2 - k1));
			    ssacrosstrack[k] 
				    = (k - MBSYS_SIMRAD_MAXPIXELS / 2)
					    * (*pixel_size);
			    ssalongtrack[k] = ssalongtrack[k1]
				+ (ssalongtrack[k2] - ssalongtrack[k1])
				    * ((double)(k - k1)) / ((double)(k2 - k1));
			    }
			}
		    else
			{
			k1 = k;
			}
		    }
			
		/* insert the new sidescan into store */
		ping->pixel_size = (int) (100 * (*pixel_size));
		if (last > first)
		    ping->pixels_ss = MBSYS_SIMRAD_MAXPIXELS;
		else 
		    ping->pixels_ss = 0;
		for (i=0;i<MBSYS_SIMRAD_MAXPIXELS;i++)
		    {
		    if (ss[i] > MB_SIDESCAN_NULL)
		    	{
		    	ping->ss[i] = (short)(100 * ss[i]);
		    	ping->ssalongtrack[i] 
			    	= (short)(ssalongtrack[i] / daloscale);
			}
		    else
		    	{
		    	ping->ss[i] = 0;
		    	ping->ssalongtrack[i] = 0;
			}
		    }

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Sidescan regenerated in <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       beams_bath:    %d\n",
				ping->beams_bath);
			for (i=0;i<ping->beams_bath;i++)
			  fprintf(stderr,"dbg2       beam:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,
				ping->bath[i],
				ping->amp[i],
				ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				MBSYS_SIMRAD_MAXPIXELS);
			for (i=0;i<MBSYS_SIMRAD_MAXPIXELS;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n",
				i,ss_cnt[i],ss[i],
				ssacrosstrack[i],
				ssalongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				ping->pixels_ss);
			for (i=0;i<ping->pixels_ss;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n",
				i,ping->ss[i],ping->ssalongtrack[i]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       pixel_size:      %f\n",*pixel_size);
		fprintf(stderr,"dbg2       swath_width:     %f\n",*swath_width);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
