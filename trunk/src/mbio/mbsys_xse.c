/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_xse.c	3/27/2000
 *	$Id: mbsys_xse.c,v 5.11 2001-12-30 20:32:12 caress Exp $
 *
 *    Copyright (c) 2000, 2001 by 
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_xse.h contains the functions for handling 
 * the data structure used by MBIO functions 
 * to store swath sonar data in the XSE Data Exchange Format
 * developed by L-3 Communications ELAC Nautik.
 * This format is used for data from ELAC Bottomchart multibeam sonars
 * and SeaBeam 2100 multibeam sonars (made by L-3 Communications
 * SeaBeam Instruments).
 * The data format associated with XSE is:
 *      MBF_L3XSERAW : MBIO ID 94
 *
 * Author:	D. W. Caress
 * Date:	August 1,  1999
 * Additional Authors:	P. A. Cohen and S. Dzurenko
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.10  2001/12/20 20:48:51  caress
 * Release 5.0.beta11
 *
 * Revision 5.9  2001/08/25  00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.8  2001/08/23  20:50:24  caress
 * Fixed problems with SB2120 data.
 *
 * Revision 5.7  2001/07/27  19:07:16  caress
 * Fixed handling 1180 data.
 *
 * Revision 5.6  2001/07/22 21:17:01  caress
 * Fixed bug initialized too much in ttimes function.
 *
 * Revision 5.5  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.4  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.3  2001/04/06  22:05:59  caress
 * Consolidated xse formats into one format.
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
 * Revision 4.2  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/08/08  04:14:35  caress
 * Initial revision.
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
#include "../../include/mbsys_xse.h"

/*--------------------------------------------------------------------*/
int mbsys_xse_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_xse.c,v 5.11 2001-12-30 20:32:12 caress Exp $";
	char	*function_name = "mbsys_xse_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_xse_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_xse_struct *) *store_ptr;

	/* initialize everything */
	/* type of data record */
	store->kind = MB_DATA_NONE;  /* Survey, nav, Comment */
	
	/* parameter (ship frames) */
	store->par_source = 0;		/* sensor id */
	store->par_sec = 0;		/* sec since 1/1/1901 00:00 */
	store->par_usec = 0;		/* microseconds */
	store->par_roll_bias = 0.0;		/* radians */
	store->par_pitch_bias = 0.0;		/* radians */
	store->par_heading_bias = 0.0;	/* radians */
	store->par_time_delay = 0.0;		/* nav time lag, seconds */
	store->par_trans_x_port = 0.0;	/* port transducer x position, meters */
	store->par_trans_y_port = 0.0;	/* port transducer y position, meters */
	store->par_trans_z_port = 0.0;	/* port transducer z position, meters */
	store->par_trans_x_stbd = 0.0;	/* starboard transducer x position, meters */
	store->par_trans_y_stbd = 0.0;	/* starboard transducer y position, meters */
	store->par_trans_z_stbd = 0.0;	/* starboard transducer z position, meters */
	store->par_trans_err_port = 0.0;	/* port transducer rotation in roll direction, radians */
	store->par_trans_err_stbd = 0.0;	/* starboard transducer rotation in roll direction, radians */
	store->par_nav_x = 0.0;		/* navigation antenna x position, meters */
	store->par_nav_y = 0.0;		/* navigation antenna y position, meters */
	store->par_nav_z = 0.0;		/* navigation antenna z position, meters */
	store->par_hrp_x = 0.0;		/* motion sensor x position, meters */
	store->par_hrp_y = 0.0;		/* motion sensor y position, meters */
	store->par_hrp_z = 0.0;		/* motion sensor z position, meters */

	/* svp (sound velocity frames) */
	store->svp_source = 0;		/* sensor id */
	store->svp_sec = 0;		/* sec since 1/1/1901 00:00 */
	store->svp_usec = 0;		/* microseconds */
	store->svp_nsvp = 0;		/* number of depth values */
	store->svp_nctd = 0;		/* number of ctd values */
	store->svp_ssv = 0.0;				/* m/s */
	for (i=0;i<MBSYS_XSE_MAXSVP;i++)
	    {
	    store->svp_depth[i] = 0.0;		/* m */
	    store->svp_velocity[i] = 0.0;	/* m/s */
	    store->svp_conductivity[i] = 0.0;	/* mmho/cm */
	    store->svp_salinity[i] = 0.0;	/* o/oo */
	    store->svp_temperature[i] = 0.0;	/* degree celcius */
	    store->svp_pressure[i] = 0.0;	/* bar */
	    }

	/* position (navigation frames) */
	store->nav_source = 0;		/* sensor id */
	store->nav_sec = 0;		/* sec since 1/1/1901 00:00 */
	store->nav_usec = 0;		/* microseconds */
	store->nav_quality = 0;
	store->nav_status = 0;
	store->nav_description_len = 0;
	for (i=0;i<MBSYS_XSE_DESCRIPTION_LENGTH;i++)
	    store->nav_description[i] = 0;
	store->nav_x = 0.0;			/* eastings (m) or 
					    longitude (radians) */
	store->nav_y = 0.0;			/* northings (m) or 
					    latitude (radians) */
	store->nav_z = 0.0;			/* height (m) or 
					    ellipsoidal height (m) */
	store->nav_speed_ground = 0.0;	/* m/s */
	store->nav_course_ground = 0.0;	/* radians */
	store->nav_speed_water = 0.0;	/* m/s */
	store->nav_course_water = 0.0;	/* radians */
	
	/* survey depth (multibeam frames) */
	store->mul_frame = MB_NO;	/* boolean flag - multibeam frame read */
	store->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
	store->mul_group_tt = MB_NO;	/* boolean flag - tt group read */
	store->mul_group_quality = MB_NO;/* boolean flag - quality group read */
	store->mul_group_amp = MB_NO;	/* boolean flag - amp group read */
	store->mul_group_delay = MB_NO;	/* boolean flag - delay group read */
	store->mul_group_lateral = MB_NO;/* boolean flag - lateral group read */
	store->mul_group_along = MB_NO;	/* boolean flag - along group read */
	store->mul_group_depth = MB_NO;	/* boolean flag - depth group read */
	store->mul_group_angle = MB_NO;	/* boolean flag - angle group read */
	store->mul_group_heave = MB_NO;	/* boolean flag - heave group read */
	store->mul_group_roll = MB_NO;	/* boolean flag - roll group read */
	store->mul_group_pitch = MB_NO;	/* boolean flag - pitch group read */
	store->mul_group_gates = MB_NO;	/* boolean flag - gates group read */
	store->mul_group_noise = MB_NO;	/* boolean flag - noise group read */
	store->mul_group_length = MB_NO;/* boolean flag - length group read */
	store->mul_group_hits = MB_NO;	/* boolean flag - hits group read */
	store->mul_source = 0;		/* sensor id */
	store->mul_sec = 0;		/* sec since 1/1/1901 00:00 */
	store->mul_usec = 0;		/* microseconds */
	store->mul_x = 0.0;		/* interpolated longitude in degrees */
	store->mul_y = 0.0;		/* interpolated latitude in degrees */
	store->mul_ping = 0;		/* ping number */
	store->mul_frequency = 0.0;	/* transducer frequency (Hz) */
	store->mul_pulse = 0.0;		/* transmit pulse length (sec) */
	store->mul_power = 0.0;		/* transmit power (dB) */
	store->mul_bandwidth = 0.0;	/* receive bandwidth (Hz) */
	store->mul_sample = 0.0;		/* receive sample interval (sec) */
	store->mul_swath = 0.0;		/* swath width (radians) */
	store->mul_num_beams = 0;	/* number of beams */
	for (i=0;i<MBSYS_XSE_MAXBEAMS;i++)
	    {
	    store->beams[i].tt = 0.0;
	    store->beams[i].delay = 0.0;
	    store->beams[i].lateral = 0.0;
	    store->beams[i].along = 0.0;
	    store->beams[i].depth = 0.0;
	    store->beams[i].angle = 0.0;
	    store->beams[i].heave = 0.0;
	    store->beams[i].roll = 0.0;
	    store->beams[i].pitch = 0.0;
	    store->beams[i].beam = i + 1;
	    store->beams[i].quality = 0;
	    store->beams[i].amplitude = 0;		    
	    store->beams[i].gate_angle = 0.0;
	    store->beams[i].gate_start = 0.0;
	    store->beams[i].gate_stop = 0.0;
	    store->beams[i].noise = 0.0;
	    store->beams[i].length = 0.0;
	    store->beams[i].hits = 0;
	    }
	
	/* survey sidescan (sidescan frames) */
	store->sid_frame = MB_NO;	/* boolean flag - sidescan frame read */
	store->sid_source = 0;		/* sensor id */
	store->sid_sec = 0;		/* sec since 1/1/1901 00:00 */
	store->sid_usec = 0;		/* microseconds */
	store->sid_ping = 0;		/* ping number */
	store->sid_frequency = 0.0;		/* transducer frequency (Hz) */
	store->sid_pulse = 0.0;		/* transmit pulse length (sec) */
	store->sid_power = 0.0;		/* transmit power (dB) */
	store->sid_bandwidth = 0.0;		/* receive bandwidth (Hz) */
	store->sid_sample = 0.0;		/* receive sample interval (sec) */
	store->sid_bin_size = 0;		/* bin size in mm */
	store->sid_offset = 0;		/* lateral offset in mm */
	store->sid_num_pixels = 0;		/* number of pixels */
	for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
	    store->ss[i] = 0; /* sidescan amplitude in dB */

	/* seabeam (seabeam frames) */
	store->sbm_properties = MB_NO;		/* boolean flag - sbm properties group read */
	store->sbm_hrp = MB_NO;			/* boolean flag - sbm hrp group read */
	store->sbm_center = MB_NO;		/* boolean flag - sbm center group read */
	store->sbm_message = MB_NO;		/* boolean flag - sbm message group read */
	store->sbm_source = 0;			/* sensor id */
	store->sbm_sec = 0;			/* sec since 1/1/1901 00:00 */
	store->sbm_usec = 0;			/* microseconds */
	store->sbm_ping = 0;			/* ping number */
	store->sbm_ping_gain = 0.0;		/* ping gain (dB) */
	store->sbm_pulse_width = 0.0;		/* pulse width (s) */
	store->sbm_transmit_power = 0.0;	/* transmit power (dB) */
	store->sbm_pixel_width = 0.0;		/* pixel width (m) */
	store->sbm_swath_width = 0.0;		/* swath width (radians) */
	store->sbm_time_slice = 0.0;		/* time slice (s) */
	store->sbm_depth_mode = 0;		/* depth mode (1=shallow, 2=deep) */
	store->sbm_beam_mode = 0;		/* focused beam mode (0=off, 1=on) */
	store->sbm_ssv = 0.0;			/* surface sound velocity (m/s) */
	store->sbm_frequency = 0.0;		/* sonar frequency (kHz) */
	store->sbm_bandwidth = 0.0;		/* receiver bandwidth (kHz) */
	store->sbm_heave = 0.0;			/* heave (m) */
	store->sbm_roll = 0.0;			/* roll (radians) */
	store->sbm_pitch = 0.0;			/* pitch (radians) */
	store->sbm_center_beam = 0;		/* beam number for center beam profile */
	store->sbm_center_count = 0;		/* number of samples in center beam profile */
	for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
	    store->sbm_center_amp[i] = 0.0;	/* center beam profile values */
	store->sbm_message_id = 0;			/* seabeam message id */
	store->sbm_message_len = 0;			/* seabeam message length */
	for (i=0;i<MBSYS_XSE_COMMENT_LENGTH;i++)
	    store->sbm_message_txt[i] = 0; /* seabeam message */

	/* comment */
	for (i=0;i<MBSYS_XSE_COMMENT_LENGTH;i++)
	    store->comment[i] = 0;

	/* unsupported frame */
	store->rawsize = 0;
	for (i=0;i<MBSYS_XSE_COMMENT_LENGTH;i++)
	    store->raw[i] = 0;

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
int mbsys_xse_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_xse_deall";
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
int mbsys_xse_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_xse_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	double	xtrackmin, xtrackmax;
	int	ixtrackmin, ixtrackmax;
	double	dsign;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mul_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->mul_usec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = store->mul_x;
		*navlat = store->mul_y;
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
		*heading = RTD * store->nav_course_ground;

		/* get speed  */
		*speed  = 3.6 * store->nav_speed_ground;
			
		/* set beamwidths in mb_io structure */
		if (store->mul_frequency >= 50.0
		    || store->mul_frequency <= 0.0)
		    {
		    mb_io_ptr->beamwidth_ltrack 
			= 2.8;
		    mb_io_ptr->beamwidth_xtrack 
			= 1.5;
		    }
		else
		    {
		    mb_io_ptr->beamwidth_ltrack 
			= 1.0;
		    mb_io_ptr->beamwidth_xtrack 
			= 1.0;
		    }

		/* get distance and depth values */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		if (store->mul_frame == MB_YES)
		    {
		    /* set number of beams */
		    *nbath = store->mul_num_beams;
		    if (store->mul_group_amp == MB_YES)
			    *namp = store->mul_num_beams;
			    
		    /* determine whether beams are ordered 
			port to starboard or starboard to port */
		    xtrackmin = 0.0;
		    xtrackmax = 0.0;
		    ixtrackmin = 0;
		    ixtrackmax = 0;
		    for (i=0;i<store->mul_num_beams;i++)
			{
			if (store->beams[i].lateral < xtrackmin)
			    {
			    xtrackmin = store->beams[i].lateral;
			    ixtrackmin = i;
			    }
			if (store->beams[i].lateral > xtrackmax)
			    {
			    xtrackmax = store->beams[i].lateral;
			    ixtrackmax = i;
			    }
			}
		    if (ixtrackmax > ixtrackmin)
			dsign = -1.0;
		    else
			dsign = 1.0;
/*
fprintf(stderr, "itrack: %d %d   freq:%f\n", 
ixtrackmin, ixtrackmax, store->mul_frequency);
*/

		    /* now extract the bathymetry */		  
		    for (i=0;i<store->mul_num_beams;i++)
			{
			j = store->mul_num_beams - store->beams[i].beam;
			if (store->beams[i].quality == 1)
			    beamflag[j] = MB_FLAG_NONE;
			else if (store->beams[i].quality < 8)
			    beamflag[j] = MB_FLAG_SONAR + MB_FLAG_FLAG;
			else if (store->beams[i].quality == 8)
			    beamflag[j] = MB_FLAG_NULL;
			else if (store->beams[i].quality == 10)
			    beamflag[j] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
			else if (store->beams[i].quality == 20)
			    beamflag[j] = MB_FLAG_FILTER + MB_FLAG_FLAG;
			else
			    beamflag[j] = MB_FLAG_NULL;
			bath[j] = store->beams[i].depth
				    + store->beams[i].heave;
			if (store->beams[i].lateral < 0.0)
			    bath[j] += store->par_trans_z_port;
			else
			    bath[j] += store->par_trans_z_stbd;
			bathacrosstrack[j] 
				= dsign * store->beams[i].lateral;
			bathalongtrack[j] 
				= store->beams[i].along;
			amp[j] = store->beams[i].amplitude;
			}
		    }
		if (store->sid_frame == MB_YES)
		    {
		    *nss = store->sid_num_pixels;
		    for (i=0;i<store->sid_num_pixels;i++)
			{
			j = store->sid_num_pixels - i - 1;
			ss[j] = store->ss[i];
			ssacrosstrack[j] 
			    = -0.001 * store->sid_bin_size 
				* (i - store->sid_num_pixels / 2);
			if (store->mul_frame == MB_YES)
			    ssalongtrack[j] 
				= 0.5 * store->nav_speed_ground 
				    * (store->sid_sec + 0.000001 * store->sid_usec
					- store->mul_sec + 0.000001 * store->mul_usec);
			else
			    ssalongtrack[j] = 0.0;
			}
		    }

		/* done translating values */

		}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get time */
		*time_d = store->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->nav_usec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = RTD * store->nav_x;
		*navlat = RTD * store->nav_y;
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
		*heading = RTD * store->nav_course_ground;

		/* get speed  */
		*speed  = 3.6 * store->nav_speed_ground;

		/* get distance and depth values */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
int mbsys_xse_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_xse_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	double	maxoffset, xtrackmin, xtrackmax;
	int	imaxoffset, ixtrackmin, ixtrackmax;
	double	dsign;
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
			fprintf(stderr,"dbg4       nss:        %d\n",
				nss);
			for (i=0;i<nss;i++)
			  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->mul_sec = (unsigned int) (time_d + MBSYS_XSE_TIME_OFFSET);
		store->mul_usec = (time_d 
				    - ((int) time_d)) * 1000000;
		store->sid_sec = store->mul_sec;
		store->sid_usec = store->mul_usec;

		/*get navigation */
		store->mul_x = navlon;
		store->mul_y = navlat;

		/* get heading */
		store->nav_course_ground = DTR * heading;

		/* get speed */
		store->nav_speed_ground = speed / 3.6;

		/* insert distance and depth values into storage arrays */
		xtrackmin = 0.0;
		xtrackmax = 0.0;
		ixtrackmin = 0;
		ixtrackmax = 0;
		if (store->mul_frame == MB_YES)
		    {
		    /* determine whether beams are ordered 
			port to starboard or starboard to port */
		    for (i=0;i<store->mul_num_beams;i++)
			{
			if (store->beams[i].lateral < xtrackmin)
			    {
			    xtrackmin = store->beams[i].lateral;
			    ixtrackmin = i;
			    }
			if (store->beams[i].lateral > xtrackmax)
			    {
			    xtrackmax = store->beams[i].lateral;
			    ixtrackmax = i;
			    }
			}
		    if (ixtrackmax > ixtrackmin)
			dsign = -1.0;
		    else
			dsign = 1.0;

		    /* now insert the bathymetry */		  
		    for (i=0;i<store->mul_num_beams;i++)
			{
			j = store->mul_num_beams - store->beams[i].beam;
			if (j < nbath)
			    {
			    if (mb_beam_check_flag(beamflag[j]))
				{
				if (mb_beam_check_flag_null(beamflag[j]))
				    store->beams[i].quality = 8;
				else if (mb_beam_check_flag_manual(beamflag[j]))
				    store->beams[i].quality = 10;
				else if (mb_beam_check_flag_filter(beamflag[j]))
				    store->beams[i].quality = 20;
				else if (store->beams[i].quality == 1)
				    store->beams[i].quality = 7;
				}
			    else 
				store->beams[i].quality = 1;
			    store->beams[i].depth 
				    = bath[j] - store->beams[i].heave;
			    store->beams[i].lateral = dsign * bathacrosstrack[j];
			    store->beams[i].along = bathalongtrack[j];
			    store->beams[i].amplitude = (int) (amp[j]);
			    }
			}
		    }

		/* now insert the sidescan */		  
		if (store->sid_frame == MB_YES)
		    {
		    if (nss != store->sid_num_pixels)
			{
			store->sid_num_pixels = nss;
			maxoffset = 0.0;
			imaxoffset = -1;
			for (i=0;i<nss;i++)
			    {
			    if (fabs(ssacrosstrack[i]) > maxoffset)
				{
				maxoffset = fabs(ssacrosstrack[i]);
				imaxoffset = i - (nss / 2);
				}
			    }
			if (maxoffset > 0.0 && imaxoffset != 0)
			    store->sid_bin_size = (int)(1000 * maxoffset / imaxoffset);
			}
		    for (i=0;i<store->sid_num_pixels;i++)
			{
			j = store->sid_num_pixels - i - 1;
			if (j < nss)
			    {
			    store->ss[i]= ss[j];
			    }
			}
		    }
		}
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		store->nav_sec = (unsigned int) (time_d + MBSYS_XSE_TIME_OFFSET);
		store->nav_usec = (time_d 
				    - ((int) time_d)) * 1000000;

		/*get navigation */
		store->nav_x = DTR * navlon;
		store->nav_y = DTR * navlat;

		/* get heading */
		store->nav_course_ground = DTR * heading;

		/* get speed */
		store->nav_speed_ground = speed / 3.6;
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
int mbsys_xse_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_xse_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	double	maxoffset, xtrackmin, xtrackmax;
	int	imaxoffset, ixtrackmin, ixtrackmax;
	double	dsign;
	double	alpha, beta;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get draft */
		*draft = 0.5 * (store->par_trans_z_port
				    + store->par_trans_z_stbd);

		/* get ssv */
		if (store->sbm_ssv > 0.0)
		    *ssv = store->sbm_ssv;
		else
		    *ssv = store->svp_ssv;

		/* get travel times, angles */
		*nbeams = 0;
		if (store->mul_frame == MB_YES)
		    {
		    /* determine whether beams are ordered 
			port to starboard or starboard to port */
		    xtrackmin = 0.0;
		    xtrackmax = 0.0;
		    ixtrackmin = 0;
		    ixtrackmax = 0;
		    for (i=0;i<store->mul_num_beams;i++)
			{
			if (store->beams[i].lateral < xtrackmin)
			    {
			    xtrackmin = store->beams[i].lateral;
			    ixtrackmin = i;
			    }
			if (store->beams[i].lateral > xtrackmax)
			    {
			    xtrackmax = store->beams[i].lateral;
			    ixtrackmax = i;
			    }
			}
		    if (ixtrackmax > ixtrackmin)
			dsign = -1.0;
		    else
			dsign = 1.0;

		    /* loop over beams */
		    for (i=0;i<store->mul_num_beams;i++)
			{
			j = store->mul_num_beams - store->beams[i].beam;
			*nbeams = MAX(store->beams[i].beam, *nbeams);
			ttimes[j] = store->beams[i].tt;
			beta = 90.0 + RTD * store->beams[i].angle;
			alpha = RTD * store->beams[i].pitch;
			mb_rollpitch_to_takeoff(verbose, 
			    alpha, beta, &angles[j], 
			    &angles_forward[j], error);
			if (store->mul_frequency >= 50.0
				|| store->mul_frequency <= 0.0)
			    {
			    if (store->beams[j].angle < 0.0)
				{
				angles_null[j] = 37.5 
					+ RTD 
					* store->par_trans_err_port;
				}
			    else
				{
				angles_null[j] = 37.5 
					+ RTD 
					* store->par_trans_err_stbd;
				}
			    }
			else
			    {
			    angles_null[j] = 0.0;
			    }
			heave[j] = store->beams[i].heave;
			alongtrack_offset[j] 
			    = 0.5 * store->nav_speed_ground 
				    * store->beams[i].delay;
			}
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
int mbsys_xse_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_xse_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get draft */
		*transducer_depth = 0.5 * (store->par_trans_z_port
				    + store->par_trans_z_stbd);
		bath_best = 0.0;
		if (store->mul_num_beams > 0)
		    {
		    *transducer_depth -= store->beams[store->mul_num_beams/2].heave;
		    if (store->beams[store->mul_num_beams/2].quality == 1)
			bath_best = store->beams[store->mul_num_beams/2].depth;
		    else
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->mul_num_beams;i++)
			    {
			    if (store->beams[i].quality == 1
				&& fabs(store->beams[i].lateral) 
				    < xtrack_min)
				{
				xtrack_min = fabs(store->beams[i].lateral);
				bath_best = store->beams[i].depth;
				}
			    }		
			}
		    if (bath_best <= 0.0)
			{
			xtrack_min = 99999999.9;
			for (i=0;i<store->mul_num_beams;i++)
			    {
			    if (store->beams[i].quality < 8
				&& fabs(store->beams[i].lateral) 
				    < xtrack_min)
				{
				xtrack_min = fabs(store->beams[i].lateral);
				bath_best = store->beams[i].depth;
				}
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
int mbsys_xse_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_xse_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mul_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->mul_usec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = store->mul_x;
		*navlat = store->mul_y;
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
		*heading = RTD * store->nav_course_ground;

		/* get speed  */
		*speed  = 3.6 * store->nav_speed_ground;

		/* get draft */
		*draft = 0.5 * (store->par_trans_z_port
				    + store->par_trans_z_stbd);

		/* get roll pitch and heave */
		if (store->mul_num_beams > 0)
			{
			*roll = RTD * store->beams[store->mul_num_beams/2].roll;
			*pitch = RTD * store->beams[store->mul_num_beams/2].pitch;
			*heave = store->beams[store->mul_num_beams/2].heave;
			}
		else
			{
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
			}

		/* done translating values */

		}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
		/* get time */
		*time_d = store->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->nav_usec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = RTD * store->nav_x;
		*navlat = RTD * store->nav_y;
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
		*heading = RTD * store->nav_course_ground;

		/* get speed  */
		*speed  = 3.6 * store->nav_speed_ground;

		/* get draft */
		*draft = 0.5 * (store->par_trans_z_port
				    + store->par_trans_z_stbd);

		/* get roll pitch and heave */
		if (store->mul_num_beams > 0)
			{
			*roll = RTD * store->beams[store->mul_num_beams/2].roll;
			*pitch = RTD * store->beams[store->mul_num_beams/2].pitch;
			*heave = store->beams[store->mul_num_beams/2].heave;
			}
		else
			{
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
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
int mbsys_xse_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_xse_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->mul_sec = (unsigned int) (time_d + MBSYS_XSE_TIME_OFFSET);
		store->mul_usec = (time_d 
				    - ((int) time_d)) * 1000000;
		store->sid_sec = store->mul_sec;
		store->sid_usec = store->mul_usec;

		/*get navigation */
		store->mul_x = navlon;
		store->mul_y = navlat;

		/* get heading */
		store->nav_course_ground = DTR * heading;

		/* get speed */
		store->nav_speed_ground = speed / 3.6;

		/* get draft */
		store->par_trans_z_port = draft;
		store->par_trans_z_stbd = draft;

		/* get roll pitch and heave */
		}

	/* insert data in structure */
	else if (store->kind == MB_DATA_NAV)
		{
		/* get time */
		store->nav_sec = (unsigned int) (time_d + MBSYS_XSE_TIME_OFFSET);
		store->nav_usec = (time_d 
				    - ((int) time_d)) * 1000000;

		/*get navigation */
		store->nav_x = DTR * navlon;
		store->nav_y = DTR * navlat;

		/* get heading */
		store->nav_course_ground = DTR * heading;

		/* get speed */
		store->nav_speed_ground = speed / 3.6;

		/* get draft */
		store->par_trans_z_port = draft;
		store->par_trans_z_stbd = draft;

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
int mbsys_xse_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_xse_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_nsvp;
		
		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = store->svp_depth[i];
			velocity[i] = store->svp_velocity[i];
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
int mbsys_xse_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_xse_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
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
	store = (struct mbsys_xse_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		store->svp_nsvp = MIN(nsvp, MBSYS_XSE_MAXSVP);
		
		/* get profile */
		for (i=0;i<store->svp_nsvp;i++)
			{
			store->svp_depth[i] = depth[i];
			store->svp_velocity[i] = velocity[i];
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
int mbsys_xse_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_xse_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	struct mbsys_xse_struct *copy;

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
	store = (struct mbsys_xse_struct *) store_ptr;
	copy = (struct mbsys_xse_struct *) copy_ptr;

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
