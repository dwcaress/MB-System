/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em300mba.c	10/16/98
 *	$Id: mbr_em300mba.c,v 4.0 1998-12-17 22:59:14 caress Exp $
 *
 *    Copyright (c) 1998 by 
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
 * mbr_em300mba.c contains the functions for reading and writing
 * multibeam data in the EM300MBA format.  
 * These functions include:
 *   mbr_alm_em300mba	- allocate read/write memory
 *   mbr_dem_em300mba	- deallocate read/write memory
 *   mbr_rt_em300mba	- read and translate data
 *   mbr_wt_em300mba	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 16,  1998
 * $Log: not supported by cvs2svn $
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
#include "../../include/mbsys_simrad2.h"
#include "../../include/mbf_em300mba.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/
int mbr_alm_em300mba(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_em300mba.c,v 4.0 1998-12-17 22:59:14 caress Exp $";
	char	*function_name = "mbr_alm_em300mba";
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_em300mba_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_simrad2_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_em300mba(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_em300mba(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_em300mba";
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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_simrad2_deall(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

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
int mbr_zero_em300mba(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_em300mba";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* data identifiers */
		data->kind = MB_DATA_NONE;
		data->type = EM2_NONE;
		data->sonar = MBSYS_SIMRAD2_UNKNOWN;
	
		/* time stamp */
		data->date = 0;
		data->msec = 0;
	
		/* installation parameter values */
		data->par_date = 0;	/* installation parameter date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		data->par_msec = 0;	/* installation parameter time since midnight in msec
					    08:12:51.234 = 29570234 */
		data->par_line_num = 0;	/* survey line number */
		data->par_serial_1 = 0;	/* system 1 serial number */
		data->par_serial_2 = 0;	/* system 2 serial number */
		data->par_wlz = 0.0;	/* water line vertical location (m) */
		data->par_smh = 0;	/* system main head serial number */
		data->par_s1z = 0.0;	/* transducer 1 vertical location (m) */
		data->par_s1x = 0.0;	/* transducer 1 along location (m) */
		data->par_s1y = 0.0;	/* transducer 1 athwart location (m) */
		data->par_s1h = 0.0;	/* transducer 1 heading (deg) */
		data->par_s1r = 0.0;	/* transducer 1 roll (m) */
		data->par_s1p = 0.0;	/* transducer 1 pitch (m) */
		data->par_s1n = 0;	/* transducer 1 number of modules */
		data->par_s2z = 0.0;	/* transducer 2 vertical location (m) */
		data->par_s2x = 0.0;	/* transducer 2 along location (m) */
		data->par_s2y = 0.0;	/* transducer 2 athwart location (m) */
		data->par_s2h = 0.0;	/* transducer 2 heading (deg) */
		data->par_s2r = 0.0;	/* transducer 2 roll (m) */
		data->par_s2p = 0.0;	/* transducer 2 pitch (m) */
		data->par_s2n = 0;	/* transducer 2 number of modules */
		data->par_go1 = 0.0;	/* system (sonar head 1) gain offset */
		data->par_go2 = 0.0;	/* sonar head 2 gain offset */
		for (i=0;i<16;i++)
		    {
		    data->par_tsv[i] = '\0';	/* transmitter (sonar head 1) software version */
		    data->par_rsv[i] = '\0';	/* receiver (sonar head 2) software version */
		    data->par_bsv[i] = '\0';	/* beamformer software version */
		    data->par_psv[i] = '\0';	/* processing unit software version */
		    data->par_osv[i] = '\0';	/* operator station software version */
		    }
		data->par_dsd = 0.0;	/* depth sensor time delay (msec) */
		data->par_dso = 0.0;	/* depth sensor offset */
		data->par_dsf = 0.0;	/* depth sensor scale factor */
		data->par_dsh[0] = 'I';	/* depth sensor heave (IN or NI) */
		data->par_dsh[1] = 'N';	/* depth sensor heave (IN or NI) */
		data->par_aps = 0;	/* active position system number */
		data->par_p1m = 0;	/* position system 1 motion compensation (boolean) */
		data->par_p1t = 0;	/* position system 1 time stamp used 
					    (0=system time, 1=position input time) */
		data->par_p1z = 0.0;	/* position system 1 vertical location (m) */
		data->par_p1x = 0.0;	/* position system 1 along location (m) */
		data->par_p1y = 0.0;	/* position system 1 athwart location (m) */
		data->par_p1d = 0.0;	/* position system 1 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    data->par_p1g[i] = '\0';	/* position system 1 geodetic datum */
		    }
		data->par_p2m = 0;	/* position system 2 motion compensation (boolean) */
		data->par_p2t = 0;	/* position system 2 time stamp used 
					    (0=system time, 1=position input time) */
		data->par_p2z = 0.0;	/* position system 2 vertical location (m) */
		data->par_p2x = 0.0;	/* position system 2 along location (m) */
		data->par_p2y = 0.0;	/* position system 2 athwart location (m) */
		data->par_p2d = 0.0;	/* position system 2 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    data->par_p2g[i] = '\0';	/* position system 2 geodetic datum */
		    }
		data->par_p3m = 0;	/* position system 3 motion compensation (boolean) */
		data->par_p3t = 0;	/* position system 3 time stamp used 
					    (0=system time, 1=position input time) */
		data->par_p3z = 0.0;	/* position system 3 vertical location (m) */
		data->par_p3x = 0.0;	/* position system 3 along location (m) */
		data->par_p3y = 0.0;	/* position system 3 athwart location (m) */
		data->par_p3d = 0.0;	/* position system 3 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    data->par_p3g[i] = '\0';	/* position system 3 geodetic datum */
		    }
		data->par_msz = 0.0;	/* motion sensor vertical location (m) */
		data->par_msx = 0.0;	/* motion sensor along location (m) */
		data->par_msy = 0.0;	/* motion sensor athwart location (m) */
		data->par_mrp[0] = 'H';	/* motion sensor roll reference plane (HO or RP) */
		data->par_mrp[1] = 'O';	/* motion sensor roll reference plane (HO or RP) */
		data->par_msd = 0.0;	/* motion sensor time delay (sec) */
		data->par_msr = 0.0;	/* motion sensor roll offset (deg) */
		data->par_msp = 0.0;	/* motion sensor pitch offset (deg) */
		data->par_msg = 0.0;	/* motion sensor heading offset (deg) */
		data->par_gcg = 0.0;	/* gyro compass heading offset (deg) */
		for (i=0;i<4;i++)
		    {
		    data->par_cpr[i] = '\0';	/* cartographic projection */
		    }
		for (i=0;i<MBF_EM300MBA_COMMENT_LENGTH;i++)
		    {
		    data->par_rop[i] = '\0';	/* responsible operator */
		    data->par_sid[i] = '\0';	/* survey identifier */
		    data->par_pll[i] = '\0';	/* survey line identifier (planned line number) */
		    data->par_com[i] = '\0';	/* comment */
		    }
	
		/* runtime parameter values */
		data->run_date = 0;		/* runtime parameter date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->run_msec = 0;		/* runtime parameter time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->run_ping_count = 0;	/* ping counter */
		data->run_serial = 0;		/* system 1 or 2 serial number */
		data->run_status = 0;		/* system status */
		data->run_mode = 0;		/* system mode:
						    0 : nearfield (EM3000) or very shallow (EM300)
						    1 :	normal (EM3000) or shallow (EM300)
						    2 : medium (EM300)
						    3 : deep (EM300)
						    4 : very deep (EM300) */
		data->run_filter_id = 0;	/* filter identifier - the two lowest bits
						    indicate spike filter strength:
							00 : off
							01 : weak
							10 : medium
							11 : strong 
						    bit 2 is set if the slope filter is on
						    bit 3 is set if the sidelobe filter is on
						    bit 4 is set if the range windows are expanded
						    bit 5 is set if the smoothing filter is on
						    bit	6 is set if the interference filter is on */
		data->run_min_depth = 0;	/* minimum depth (m) */
		data->run_max_depth = 0;	/* maximum depth (m) */
		data->run_absorption = 0;	/* absorption coefficient (0.01 dB/km) */
	
		data->run_tran_pulse = 0;	/* transmit pulse length (usec) */
		data->run_tran_beam = 0;	/* transmit beamwidth (0.1 deg) */
		data->run_tran_pow = 0;		/* transmit power reduction (dB) */
		data->run_rec_beam = 0;		/* receiver beamwidth (0.1 deg) */
		data->run_rec_band = 0;		/* receiver bandwidth (50 hz) */
		data->run_rec_gain = 0;		/* receiver fixed gain (dB) */
		data->run_tvg_cross = 0;	/* TVG law crossover angle (deg) */
		data->run_ssv_source = 0;	/* source of sound speed at transducer:
						    0 : from sensor
						    1 : manual
						    2 : from profile */
		data->run_max_swath = 0;	/* maximum swath width (m) */
		data->run_beam_space = 0;	/* beam spacing:
						    0 : determined by beamwidth (EM3000)
						    1 : equidistant
						    2 : equiangle */
		data->run_swath_angle = 0;	/* coverage sector of swath (deg) */
		data->run_stab_mode = 0;	/* yaw and pitch stabilization mode:
						    The upper bit (bit 7) is set if pitch
						    stabilization is on.
						    The two lower bits are used to show yaw
						    stabilization mode as follows:
							00 : none
							01 : to survey line heading
							10 : to mean vessel heading
							11 : to manually entered heading */
		for (i=0;i<4;i++)
		    {
		    data->run_spare[i] = '\0';
		    }
	
		/* sound velocity profile */
		data->svp_use_date = 0;		/* date at start of use
						    date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->svp_use_msec = 0;		/* time at start of use since midnight in msec
						    08:12:51.234 = 29570234 */
		data->svp_count = 0;		/* sequential counter or input identifier */
		data->svp_serial = 0;		/* system 1 serial number */
		data->svp_origin_date = 0;	/* date at svp origin
						    date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->svp_origin_msec = 0;	/* time at svp origin since midnight in msec
						    08:12:51.234 = 29570234 */
		data->svp_num = 0;		/* number of svp entries */
		data->svp_depth_res = 0;	/* depth resolution (cm) */
		for (i=0;i<MBF_EM300MBA_MAXSVP;i++)
		    {
		    data->svp_depth[i] = 0;	/* depth of svp entries (according to svp_depth_res) */
		    data->svp_vel[i] = 0;	/* sound speed of svp entries (0.1 m/sec) */
		    }
		    
		/* position */
		data->pos_date = 0;		/* position date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->pos_msec = 0;		/* position time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->pos_count = 0;		/* sequential counter */
		data->pos_serial = 0;		/* system 1 serial number */
		data->pos_latitude = 0;		/* latitude in decimal degrees * 20000000
						    (negative in southern hemisphere) 
						    if valid, invalid = 0x7FFFFFFF */
		data->pos_longitude = 0;	/* longitude in decimal degrees * 10000000
						    (negative in western hemisphere) 
						    if valid, invalid = 0x7FFFFFFF */
		data->pos_quality = 0;		/* measure of position fix quality (cm) */
		data->pos_speed = 0;		/* speed over ground (cm/sec) if valid,
						    invalid = 0xFFFF */
		data->pos_course = 0;		/* course over ground (0.01 deg) if valid,
						    invalid = 0xFFFF */
		data->pos_heading = 0;	/* heading (0.01 deg) if valid,
						    invalid = 0xFFFF */
		data->pos_system = 0;		/* position system number, type, and realtime use
						    - position system number given by two lowest bits
						    - fifth bit set means position must be derived
							from input Simrad 90 datagram
						    - sixth bit set means valid time is that of
							input datagram */
		data->pos_input_size = 0;	/* number of bytes in input position datagram */
		for (i=0;i<256;i++)
		    {
		    data->pos_input[i] = 0;	/* position input datagram as received, minus
						    header and tail (such as NMEA 0183 $ and CRLF) */
		    }
		    
		/* height */
		data->hgt_date = 0;		/* height date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->hgt_msec = 0;		/* height time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->hgt_count = 0;		/* sequential counter */
		data->hgt_serial = 0;		/* system 1 serial number */
		data->hgt_height = 0;		/* height (0.01 m) */
		data->hgt_type = 0;		/* height type as given in input datagram or if
						    zero the height is derived from the GGK datagram
						    and is the height of the water level re the
						    vertical datum */
		
		/* tide */
		data->tid_date = 0;		/* tide date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->tid_msec = 0;		/* tide time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->tid_count = 0;		/* sequential counter */
		data->tid_serial = 0;		/* system 1 serial number */
		data->tid_origin_date = 0;	/* tide input date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->tid_origin_msec = 0;	/* tide input time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->tid_tide = 0;		/* tide offset (0.01 m) */	
		
		/* clock */
		data->clk_date = 0;		/* system date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->clk_msec = 0;		/* system time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->clk_count = 0;		/* sequential counter */
		data->clk_serial = 0;		/* system 1 serial number */
		data->clk_origin_date	= 0;	/* external clock date = year*10000 + month*100 + day
						    Feb 26, 1995 = 19950226 */
		data->clk_origin_msec = 0;	/* external clock time since midnight in msec
						    08:12:51.234 = 29570234 */
		data->clk_1_pps_use = 0;	/* if 1 then the internal clock is synchronized
						    to an external 1 PPS signal, if 0 then not */
	
		/* attitude data */
		data->att_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		data->att_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		data->att_count = 0;	
				/* sequential counter or input identifier */
		data->att_serial = 0;	
				/* system 1 or system 2 serial number */
		data->att_ndata = 0;	
				/* number of attitude data */
		for (i=0;i<MBF_EM300MBA_MAXATTITUDE;i++)
		    {
		    data->att_time[i] = 0;
				/* time since record start (msec) */
		    data->att_sensor_status[i] = 0;
				/* see note 12 above */
		    data->att_roll[i] = 0;
				/* roll (0.01 degree) */
		    data->att_pitch[i] = 0;
				/* pitch (0.01 degree) */
		    data->att_heave[i] = 0;
				/* heave (cm) */
		    data->att_heading[i] = 0;
				/* heading (0.01 degree) */
		    }
		data->att_heading_status = 0;
				/* heading status (0=inactive) */
	
		/* heading data */
		data->hed_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		data->hed_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		data->hed_count = 0;	
				/* sequential counter or input identifier */
		data->hed_serial = 0;	
				/* system 1 or system 2 serial number */
		data->hed_ndata = 0;	
				/* number of heading data */
		for (i=0;i<MBF_EM300MBA_MAXHEADING;i++)
		    {
		    data->hed_time[i] = 0;
				/* time since record start (msec) */
		    data->hed_heading[i] = 0;
				/* heading (0.01 degree) */
		    }
		data->hed_heading_status = 0;
				/* heading status (0=inactive) */
	
		/* survey data */
		data->png_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		data->png_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		data->png_count = 0;	
				/* sequential counter or input identifier */
		data->png_serial = 0;	
				/* system 1 or system 2 serial number */
		data->png_latitude = 0;		/* latitude in decimal degrees * 20000000
						    (negative in southern hemisphere) 
						    if valid, invalid = 0x7FFFFFFF */
		data->png_longitude = 0;	/* longitude in decimal degrees * 10000000
						    (negative in western hemisphere) 
						    if valid, invalid = 0x7FFFFFFF */
		data->png_speed = 0;		/* speed over ground (cm/sec) if valid,
						    invalid = 0xFFFF */
		data->png_heading = 0;	
				/* heading (0.01 deg) */
		data->png_ssv = 0;	
				/* sound speed at transducer (0.1 m/sec) */
		data->png_xducer_depth = 0;   
				/* transmit transducer depth (0.01 m) 
				    - The transmit transducer depth plus the
					depth offset multiplier times 65536 cm
					should be added to the beam depths to 
					derive the depths re the water line.
					The depth offset multiplier will usually
					be zero, except when the EM3000 sonar
					head is on an underwater vehicle at a
					depth greater than about 650 m. Note that
					the offset multiplier will be negative
					(-1) if the actual heave is large enough
					to bring the transmit transducer above 
					the water line. This may represent a valid
					situation,  but may also be due to an 
					erroneously set installation depth of 
					the either transducer or the water line. */
		data->png_offset_multiplier = 0;	
				/* transmit transducer depth offset multiplier
				   - see note 7 above */ 
				   
		/* beam data */
		data->png_nbeams_max = 0;	
				/* maximum number of beams possible */
		data->png_nbeams = 0;	
				/* number of valid beams */
		data->png_depth_res = 0;	
				/* depth resolution (0.01 m) */
		data->png_distance_res = 0;	
				/* x and y resolution (0.01 m) */
		data->png_sample_rate = 0;	
				/* sampling rate (Hz) OR depth difference between
				    sonar heads in EM3000D - see note 9 above */
		for (i=0;i<MBF_EM300MBA_MAXBEAMS;i++)
		    {
		    data->png_depth[i] = 0;	
				/* depths in depth resolution units */
		    data->png_acrosstrack[i] = 0;
				/* acrosstrack distances in distance resolution units */
		    data->png_alongtrack[i] = 0;
				/* alongtrack distances in distance resolution units */
		    data->png_depression[i] = 0;
				/* Primary beam angles in one of two formats (see note 10 above)
				   1: Corrected format - gives beam depression angles
				        in 0.01 degree. These are the takeoff angles used
					in raytracing calculations.
				   2: Uncorrected format - gives beam pointing angles
				        in 0.01 degree. These values are relative to
					the transducer array and have not been corrected
					for vessel motion. */
		    data->png_azimuth[i] = 0;
				/* Secondary beam angles in one of two formats (see note 10 above)
				   1: Corrected format - gives beam azimuth angles
				        in 0.01 degree. These values used to rotate sounding
					position relative to the sonar after raytracing.
				   2: Uncorrected format - combines a flag indicating that
				        the angles are in the uncorrected format with
					beam tilt angles. Values greater than
					35999 indicate the uncorrected format is in use. The
					beam tilt angles are given as (value - 54000) in
					0.01 degree; the tilt angles give the tilt of the
					transmitted ping due to compensation for vessel
					motion. */
		    data->png_range[i] = 0;
				/* Ranges in one of two formats (see note 10 above):
				   1: Corrected format - the ranges are one way 
				        travel times in time units defined as half 
					the inverse sampling rate.
				   2: Uncorrected format - the ranges are raw two
				        way travel times in time units defined as
					half the inverse sampling rate. These values
					have not been corrected for changes in the
					heave during the ping cycle. */
		    data->png_quality[i] = 0;	
				/* 0-254 */
		    data->png_window[i] = 0;		
				/* samples/4 */
		    data->png_amp[i] = 0;		
				/* 0.5 dB */
		    data->png_beam_num[i] = 0;	
				/* beam 128 is first beam on 
				    second head of EM3000D */
		    data->png_beamflag[i] = MB_FLAG_NULL;	
				/* uses standard MB-System beam flags */
		    }
		data->png_ss_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		data->png_ss_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		data->png_max_range = 0;  
				/* max range of ping in number of samples */
		data->png_r_zero = 0;	
				/* range to normal incidence used in TVG
				    (R0 predicted) in samples */
		data->png_r_zero_corr = 0;
				/* range to normal incidence used to correct
				    sample amplitudes in number of samples */
		data->png_tvg_start = 0;	
				/* start sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		data->png_tvg_stop = 0;	\
				/* stop sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		data->png_bsn = 0;	
				/* normal incidence backscatter (BSN) in dB */
		data->png_bso = 0;	
				/* oblique incidence backscatter (BSO) in dB */
		data->png_tx = 0;	
				/* Tx beamwidth in 0.1 degree */
		data->png_tvg_crossover = 0;	
				/* TVG law crossover angle in degrees */
		data->png_nbeams_ss = 0;	
				/* number of beams with sidescan */
		data->png_npixels = 0;	
				/* number of pixels of sidescan */
		data->png_pixel_size = 0;	
				/* processed sidescan pixel size in cm */
		data->png_pixels_ss = 0;	
				/* number of pixels of processed sidescan */
		for (i=0;i<MBF_EM300MBA_MAXBEAMS;i++)
		    {
		    data->png_beam_index[i] = 0;	
				/* beam index number */
		    data->png_sort_direction[i] = 0;	
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
		    data->png_beam_samples[i] = 0;	
				/* number of sidescan samples derived from
					each beam */
		    data->png_start_sample[i] = 0;	
				/* start sample number */
		    data->png_center_sample[i] = 0;	
				/* center sample number */
		    }
		for (i=0;i<MBF_EM300MBA_MAXRAWPIXELS;i++)
		    {
		    data->png_ssraw[i] = EM2_INVALID_AMP;
				/* the raw sidescan ordered port to starboard */
		    }
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
		    {
		    data->png_ss[i] = 0;
				/* the processed sidescan ordered port to starboard */
		    data->png_ssalongtrack[i] = 0;
				/* the processed sidescan alongtrack positions */
		    }
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_zero_ss_em300mba(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_ss_em300mba";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* initialize all sidescan stuff to zeros */
	if (data != NULL)
		{
		data->png_ss_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		data->png_ss_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		data->png_max_range = 0;  
				/* max range of ping in number of samples */
		data->png_r_zero = 0;	
				/* range to normal incidence used in TVG
				    (R0 predicted) in samples */
		data->png_r_zero_corr = 0;
				/* range to normal incidence used to correct
				    sample amplitudes in number of samples */
		data->png_tvg_start = 0;	
				/* start sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		data->png_tvg_stop = 0;	\
				/* stop sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		data->png_bsn = 0;	
				/* normal incidence backscatter (BSN) in dB */
		data->png_bso = 0;	
				/* oblique incidence backscatter (BSO) in dB */
		data->png_tx = 0;	
				/* Tx beamwidth in 0.1 degree */
		data->png_tvg_crossover = 0;	
				/* TVG law crossover angle in degrees */
		data->png_nbeams_ss = 0;	
				/* number of beams with sidescan */
		data->png_npixels = 0;	
				/* number of pixels of sidescan */
		data->png_pixel_size = 0;	
				/* processed sidescan pixel size in cm */
		data->png_pixels_ss = 0;	
				/* number of pixels of processed sidescan */
		for (i=0;i<MBF_EM300MBA_MAXBEAMS;i++)
		    {
		    data->png_beam_index[i] = 0;	
				/* beam index number */
		    data->png_sort_direction[i] = 0;	
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
		    data->png_beam_samples[i] = 0;	
				/* number of sidescan samples derived from
					each beam */
		    data->png_start_sample[i] = 0;	
				/* start sample number */
		    data->png_center_sample[i] = 0;	
				/* center sample number */
		    }
		for (i=0;i<MBF_EM300MBA_MAXRAWPIXELS;i++)
		    {
		    data->png_ssraw[i] = EM2_INVALID_AMP;
				/* the raw sidescan ordered port to starboard */
		    }
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
		    {
		    data->png_ss[i] = 0;
				/* the processed sidescan ordered port to starboard */
		    data->png_ssalongtrack[i] = 0;
				/* the processed sidescan alongtrack positions */
		    }
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_rt_em300mba(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_em300mba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em300mba_struct *data;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_attitude_struct *attitude;
	struct mbsys_simrad2_heading_struct *heading;
	struct mbsys_simrad2_ping_struct *ping;
	mb_s_char *beam_ss;
	int	time_i[7];
	double	bath_time_d, ss_time_d;
	double	dd, dt, dx, dy, speed;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	depthoffset, depthscale;
	double	dacrscale, daloscale;
	double	ttscale, reflscale;
	int	ifix;
	int	i, j, k;

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

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_em300mba_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* reset values in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_NONE;
	mb_io_ptr->new_time_i[0] = 0;
	mb_io_ptr->new_time_i[1] = 0;
	mb_io_ptr->new_time_i[2] = 0;
	mb_io_ptr->new_time_i[3] = 0;
	mb_io_ptr->new_time_i[4] = 0;
	mb_io_ptr->new_time_i[5] = 0;
	mb_io_ptr->new_time_i[6] = 0;
	mb_io_ptr->new_time_d = 0.0;
	mb_io_ptr->new_lon = 0.0;
	mb_io_ptr->new_lat = 0.0;
	mb_io_ptr->new_heading = 0.0;
	mb_io_ptr->new_speed = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->new_bath[i] = 0.0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
		mb_io_ptr->new_bath_alongtrack[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->new_amp[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = 0.0;
		mb_io_ptr->new_ss_acrosstrack[i] = 0.0;
		mb_io_ptr->new_ss_alongtrack[i] = 0.0;
		}

	/* read next data from file */
	status = mbr_em300mba_rd_data(verbose,mbio_ptr,error);
	
	/* check that bath and sidescan data record time stamps
	   match for survey data - we can have bath without
	   sidescan but not sidescan without bath */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA)
		{
		/* get times of bath and sidescan records */
		time_i[0] = data->png_date / 10000;
		time_i[1] = (data->png_date % 10000) / 100;
		time_i[2] = data->png_date % 100;
		time_i[3] = data->png_msec / 3600000;
		time_i[4] = (data->png_msec % 3600000) / 60000;
		time_i[5] = (data->png_msec % 60000) / 1000;
		time_i[6] = (data->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = data->png_ss_date / 10000;
		time_i[1] = (data->png_ss_date % 10000) / 100;
		time_i[2] = data->png_ss_date % 100;
		time_i[3] = data->png_ss_msec / 3600000;
		time_i[4] = (data->png_ss_msec % 3600000) / 60000;
		time_i[5] = (data->png_ss_msec % 60000) / 1000;
		time_i[6] = (data->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
		
		/* check for time match - if bath newer than
		   sidescan then zero sidescan,  if sidescan
		   newer than bath then set error,  if ok then
		   check that beam ids are the same */
		if (bath_time_d > ss_time_d)
		    {
		    status = mbr_zero_ss_em300mba(verbose,mb_io_ptr->raw_data,error);
		    }
		else if (bath_time_d < ss_time_d)
		    {
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    status = MB_FAILURE;
		    }
		else
		    {
		    /* check for some indicators of broken records */
		    if (data->png_nbeams != data->png_nbeams_ss)
			    {
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    status = MB_FAILURE;
			    }
		    else
			    {
			    for (i=0;i<data->png_nbeams;i++)
				{
				if (data->png_beam_num[i] != 
				    data->png_beam_index[i] + 1)
				    {
				    *error = MB_ERROR_UNINTELLIGIBLE;
				    status = MB_FAILURE;
				    }
				}
			    }
		    }
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS)
		{
		/* get time */
		if (data->kind == MB_DATA_DATA)
			{
			mb_io_ptr->new_time_i[0] = data->png_date / 10000;
			mb_io_ptr->new_time_i[1] = (data->png_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = data->png_date % 100;
			mb_io_ptr->new_time_i[3] = data->png_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (data->png_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (data->png_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (data->png_msec % 1000) * 1000;
			}
		else if (data->kind == MB_DATA_COMMENT
			|| data->kind == MB_DATA_START
			|| data->kind == MB_DATA_STOP)
			{
			mb_io_ptr->new_time_i[0] = data->par_date / 10000;
			mb_io_ptr->new_time_i[1] = (data->par_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = data->par_date % 100;
			mb_io_ptr->new_time_i[3] = data->par_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (data->par_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (data->par_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (data->par_msec % 1000) * 1000;
			}
		else if (data->kind == MB_DATA_VELOCITY_PROFILE)
			{
			mb_io_ptr->new_time_i[0] = data->svp_use_date / 10000;
			mb_io_ptr->new_time_i[1] = (data->svp_use_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = data->svp_use_date % 100;
			mb_io_ptr->new_time_i[3] = data->svp_use_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (data->svp_use_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (data->svp_use_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (data->svp_use_msec % 1000) * 1000;
			}
		else if (data->kind == MB_DATA_NAV)
			{
			mb_io_ptr->new_time_i[0] = data->pos_date / 10000;
			mb_io_ptr->new_time_i[1] = (data->pos_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = data->pos_date % 100;
			mb_io_ptr->new_time_i[3] = data->pos_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (data->pos_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (data->pos_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (data->pos_msec % 1000) * 1000;
			}
		else if (data->kind == MB_DATA_ATTITUDE)
			{
			mb_io_ptr->new_time_i[0] = data->att_date / 10000;
			mb_io_ptr->new_time_i[1] = (data->att_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = data->att_date % 100;
			mb_io_ptr->new_time_i[3] = data->att_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (data->att_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (data->att_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (data->att_msec % 1000) * 1000;
			}
		else if (data->kind == MB_DATA_RUN_PARAMETER)
			{
			if (data->run_date != 0)
			    {
			    mb_io_ptr->new_time_i[0] = data->run_date / 10000;
			    mb_io_ptr->new_time_i[1] = (data->run_date % 10000) / 100;
			    mb_io_ptr->new_time_i[2] = data->run_date % 100;
			    mb_io_ptr->new_time_i[3] = data->run_msec / 3600000;
			    mb_io_ptr->new_time_i[4] = (data->run_msec % 3600000) / 60000;
			    mb_io_ptr->new_time_i[5] = (data->run_msec % 60000) / 1000;
			    mb_io_ptr->new_time_i[6] = (data->run_msec % 1000) * 1000;
			    }
			else
			    {
			    mb_io_ptr->new_time_i[0] = data->date / 10000;
			    mb_io_ptr->new_time_i[1] = (data->date % 10000) / 100;
			    mb_io_ptr->new_time_i[2] = data->date % 100;
			    mb_io_ptr->new_time_i[3] = data->msec / 3600000;
			    mb_io_ptr->new_time_i[4] = (data->msec % 3600000) / 60000;
			    mb_io_ptr->new_time_i[5] = (data->msec % 60000) / 1000;
			    mb_io_ptr->new_time_i[6] = (data->msec % 1000) * 1000;
			    }
			}
		if (mb_io_ptr->new_time_i[0] < 1970)
			mb_io_ptr->new_time_d = 0.0;
		else
			mb_get_time(verbose,mb_io_ptr->new_time_i,
				&mb_io_ptr->new_time_d);
				
		/* save fix if nav data */
		if (data->kind == MB_DATA_NAV
			&& data->pos_longitude != EM2_INVALID_INT
			&& data->pos_latitude != EM2_INVALID_INT)
			{
			/* make room for latest fix */
			if (mb_io_ptr->nfix >= MB_NAV_SAVE_MAX)
				{
				for (i=0;i<mb_io_ptr->nfix-1;i++)
					{
					mb_io_ptr->fix_time_d[i]
					    = mb_io_ptr->fix_time_d[i+1];
					mb_io_ptr->fix_lon[i]
					    = mb_io_ptr->fix_lon[i+1];
					mb_io_ptr->fix_lat[i]
					    = mb_io_ptr->fix_lat[i+1];
					}
				mb_io_ptr->nfix--;
				}
			
			/* add latest fix */
			mb_io_ptr->fix_time_d[mb_io_ptr->nfix] 
				= mb_io_ptr->new_time_d;
			mb_io_ptr->fix_lon[mb_io_ptr->nfix] 
				= 0.0000001 * data->pos_longitude;
			mb_io_ptr->fix_lat[mb_io_ptr->nfix] 
				= 0.00000005 * data->pos_latitude;
			mb_io_ptr->nfix++;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       kind:       %d\n",
				mb_io_ptr->new_kind);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			}
		}

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* first get speed - try value in bath record first */
		if (data->png_speed != EM2_INVALID_SHORT)
			mb_io_ptr->new_speed = 0.036 * data->png_speed;
			
		/* else try last position record */
		else if (data->pos_speed != EM2_INVALID_SHORT)
			mb_io_ptr->new_speed = 0.036 * data->pos_speed;
			
		/* else try to interpolate from saved nav */
		else if (mb_io_ptr->nfix > 1)
			{
			mb_coor_scale(verbose,
			    mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
			    &mtodeglon,&mtodeglat);
			dx = (mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_lon[0])/mtodeglon;
			dy = (mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_lat[0])/mtodeglat;
			dt = mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
			    - mb_io_ptr->fix_time_d[0];
			mb_io_ptr->new_speed = 3.6 * sqrt(dx*dx + dy*dy)/dt; /* km/hr */
			}
			
		/* else give up */
		else
			mb_io_ptr->new_speed = 0.0;
			
		/* don't report an unreasonable speed */
		if (mb_io_ptr->new_speed > 100.0)
			mb_io_ptr->new_speed = 0.0;
			
		/* if bath record was missing speed, put it in */
		if (data->png_speed == EM2_INVALID_SHORT)
			data->png_speed = mb_io_ptr->new_speed / 0.036;
		
		/* now get nav - from bath record if possible */
		if (data->png_latitude != EM2_INVALID_INT
			&& data->png_longitude != EM2_INVALID_INT)
			{
			mb_io_ptr->new_lon = 0.0000001 * data->png_longitude;
			mb_io_ptr->new_lat = 0.00000005 * data->png_latitude;
			}

		/* else interpolate from saved nav if possible */
		else if (mb_io_ptr->nfix > 1)
			{
			/* interpolation possible */
			if (mb_io_ptr->new_time_d 
				>= mb_io_ptr->fix_time_d[0]
			    && mb_io_ptr->new_time_d
				<= mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    ifix = 0;
			    while (mb_io_ptr->new_time_d
				> mb_io_ptr->fix_time_d[ifix+1])
				ifix++;
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (mb_io_ptr->new_time_d 
				< mb_io_ptr->fix_time_d[0]
				&& mb_io_ptr->new_speed > 0.0)
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[0])
				* mb_io_ptr->new_speed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(0.01 * data->png_heading));
			    headingy = cos(DTR*(0.01 * data->png_heading));
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (mb_io_ptr->new_time_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
				&& mb_io_ptr->new_speed > 0.0)
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* mb_io_ptr->new_speed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(0.01 * data->png_heading));
			    headingy = cos(DTR*(0.01 * data->png_heading));
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* use last fix */
			else
			    {
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			    }
			}
			
		/* else extrapolate from only fix */
		else if (mb_io_ptr->nfix == 1
			&& mb_io_ptr->new_speed > 0.0)
			{
			dd = (mb_io_ptr->new_time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* mb_io_ptr->new_speed / 3.6;
			mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			headingx = sin(DTR*(0.01 * data->png_heading));
			headingy = cos(DTR*(0.01 * data->png_heading));
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			}
		/* else just take last position */
		else if (mb_io_ptr->nfix == 1)
			{
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			}
		else
			{
			mb_io_ptr->new_lon = 0.0;
			mb_io_ptr->new_lat = 0.0;
			}
		
		/* deal with lon flipping */
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		
		/* if no nav in bath record put it in */
		if (data->png_latitude == EM2_INVALID_INT
			|| data->png_longitude == EM2_INVALID_INT)
			{
			data->png_longitude = (int) (10000000 * mb_io_ptr->new_lon);
			data->png_latitude = (int) (20000000 * mb_io_ptr->new_lat);
			}

		/* get heading */
		mb_io_ptr->new_heading = 0.01*data->png_heading;

		/* read beam and pixel values into storage arrays */
		depthscale = 0.01 * data->png_depth_res;
		depthoffset = 0.01 * data->png_xducer_depth
				+ 655.36 * data->png_offset_multiplier;
		dacrscale  = 0.01 * data->png_distance_res;
		daloscale  = 0.01 * data->png_distance_res;
		reflscale  = 0.5;
		mb_io_ptr->beams_bath = 0;
		for (i=0;i<data->png_nbeams;i++)
			{
			j = data->png_beam_num[i] - 1;
			mb_io_ptr->new_bath[j] 
				= depthscale * data->png_depth[i]
				    + depthoffset;
			mb_io_ptr->new_beamflag[j] 
				= data->png_beamflag[i];
			mb_io_ptr->new_bath_acrosstrack[j] 
				= dacrscale * data->png_acrosstrack[i];
			mb_io_ptr->new_bath_alongtrack[j] 
				= daloscale * data->png_alongtrack[i];
			if (data->png_quality[i] != 0)
				mb_io_ptr->new_amp[j] 
					= reflscale * data->png_amp[i] + 64;
			else
				mb_io_ptr->new_amp[j] = 0;
			mb_io_ptr->beams_bath = MAX(j + 1, mb_io_ptr->beams_bath);
			}
		mb_io_ptr->beams_amp = mb_io_ptr->beams_bath;
		mb_io_ptr->pixels_ss = MBF_EM300MBA_MAXPIXELS;
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			mb_io_ptr->new_ss[i] = 0.01 * data->png_ss[i];
			mb_io_ptr->new_ss_acrosstrack[i] = 0.01 * data->png_pixel_size 
					* (i - MBF_EM300MBA_MAXPIXELS / 2);
			mb_io_ptr->new_ss_alongtrack[i] = daloscale * data->png_ssalongtrack[i];
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
			fprintf(stderr,"dbg4       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			fprintf(stderr,"dbg4       beams_amp:  %d\n",
				mb_io_ptr->beams_amp);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,mb_io_ptr->new_beamflag[i],
				mb_io_ptr->new_bath[i],
				mb_io_ptr->new_amp[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			fprintf(stderr,"dbg4       pixels_ss:  %d\n",
				mb_io_ptr->pixels_ss);
			for (i=0;i<mb_io_ptr->pixels_ss;i++)
			  fprintf(stderr,"dbg4       pixel:%d  ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,mb_io_ptr->new_ss[i],
				mb_io_ptr->new_ss_acrosstrack[i],
				mb_io_ptr->new_ss_alongtrack[i]);
			}
		}

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_NAV)
		{
		mb_io_ptr->new_lon = 0.0000001 * data->pos_longitude;
		mb_io_ptr->new_lat = 0.00000005 * data->pos_latitude;
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* get heading */
		if (data->pos_heading != EM2_INVALID_SHORT)
			mb_io_ptr->new_heading = 0.01 * data->pos_heading;
		else
			mb_io_ptr->new_heading = 0.0;

		/* get speed  */
		if (data->pos_speed != EM2_INVALID_SHORT)
			mb_io_ptr->new_speed = 0.036 * data->pos_speed;
		else
			mb_io_ptr->new_speed = 0.0;
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->par_com,
			MBF_EM300MBA_COMMENT_LENGTH);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				mb_io_ptr->new_comment);
			}
		}

	/* translate values to simrad data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* data identifiers */
		store->kind = data->kind;
		store->type = data->type;
		store->sonar = data->sonar;
	
		/* time stamp */
		store->date = data->date;
		store->msec = data->msec;
	
		/* installation parameter values */
		store->par_date = data->par_date;
		store->par_msec = data->par_msec;
		store->par_line_num = data->par_line_num;
		store->par_serial_1 = data->par_serial_1;
		store->par_serial_2 = data->par_serial_2;
		store->par_wlz = data->par_wlz;
		store->par_smh = data->par_smh;
		store->par_s1z = data->par_s1z;
		store->par_s1x = data->par_s1x;
		store->par_s1y = data->par_s1y;
		store->par_s1h = data->par_s1h;
		store->par_s1r = data->par_s1r;
		store->par_s1p = data->par_s1p;
		store->par_s1n = data->par_s1n;
		store->par_s2z = data->par_s2z;
		store->par_s2x = data->par_s2x;
		store->par_s2y = data->par_s2y;
		store->par_s2h = data->par_s2h;
		store->par_s2r = data->par_s2r;
		store->par_s2p = data->par_s2p;
		store->par_s2n = data->par_s2n;
		store->par_go1 = data->par_go1;
		store->par_go2 = data->par_go2;
		for (i=0;i<16;i++)
		    {
		    store->par_tsv[i] = data->par_tsv[i];
		    store->par_rsv[i] = data->par_rsv[i];
		    store->par_bsv[i] = data->par_bsv[i];
		    store->par_psv[i] = data->par_psv[i];
		    store->par_osv[i] = data->par_osv[i];
		    }
		store->par_dsd = data->par_dsd;
		store->par_dso = data->par_dso;
		store->par_dsf = data->par_dsf;
		store->par_dsh[0] = data->par_dsh[0];
		store->par_dsh[1] = data->par_dsh[1];
		store->par_aps = data->par_aps;
		store->par_p1m = data->par_p1m;
		store->par_p1t = data->par_p1t;
		store->par_p1z = data->par_p1z;
		store->par_p1x = data->par_p1x;
		store->par_p1y = data->par_p1y;
		store->par_p1d = data->par_p1d;
		for (i=0;i<16;i++)
		    {
		    store->par_p1g[i] = data->par_p1g[i];
		    }
		store->par_p2m = data->par_p2m;
		store->par_p2t = data->par_p2t;
		store->par_p2z = data->par_p2z;
		store->par_p2x = data->par_p2x;
		store->par_p2y = data->par_p2y;
		store->par_p2d = data->par_p2d;
		for (i=0;i<16;i++)
		    {
		    store->par_p2g[i] = data->par_p2g[i];
		    }
		store->par_p3m = data->par_p3m;
		store->par_p3t = data->par_p3t;
		store->par_p3z = data->par_p3z;
		store->par_p3x = data->par_p3x;
		store->par_p3y = data->par_p3y;
		store->par_p3d = data->par_p3d;
		for (i=0;i<16;i++)
		    {
		    store->par_p3g[i] = data->par_p3g[i];
		    }
		store->par_msz = data->par_msz;
		store->par_msx = data->par_msx;
		store->par_msy = data->par_msy;
		store->par_mrp[0] = data->par_mrp[0];
		store->par_mrp[1] = data->par_mrp[1];
		store->par_msd = data->par_msd;
		store->par_msr = data->par_msr;
		store->par_msp = data->par_msp;
		store->par_msg = data->par_msg;
		store->par_gcg = data->par_gcg;
		for (i=0;i<4;i++)
		    {
		    store->par_cpr[i] = data->par_cpr[i];
		    }
		for (i=0;i<MBSYS_SIMRAD2_COMMENT_LENGTH;i++)
		    {
		    store->par_rop[i] = data->par_rop[i];
		    store->par_sid[i] = data->par_sid[i];
		    store->par_pll[i] = data->par_pll[i];
		    store->par_com[i] = data->par_com[i];
		    }
	
		/* runtime parameter values */
		store->run_date = data->run_date;
		store->run_msec = data->run_msec;
		store->run_ping_count = data->run_ping_count;
		store->run_serial = data->run_serial;
		store->run_status = data->run_status;
		store->run_mode = data->run_mode;
		store->run_filter_id = data->run_filter_id;
		store->run_min_depth = data->run_min_depth;
		store->run_max_depth = data->run_max_depth;
		store->run_absorption = data->run_absorption;
		store->run_tran_pulse = data->run_tran_pulse;
		store->run_tran_beam = data->run_tran_beam;
		store->run_tran_pow = data->run_tran_pow;
		store->run_rec_beam = data->run_rec_beam;
		store->run_rec_band = data->run_rec_band;
		store->run_rec_gain = data->run_rec_gain;
		store->run_tvg_cross = data->run_tvg_cross;
		store->run_ssv_source = data->run_ssv_source;
		store->run_max_swath = data->run_max_swath;
		store->run_beam_space = data->run_beam_space;
		store->run_swath_angle = data->run_swath_angle;
		store->run_stab_mode = data->run_stab_mode;
		for (i=0;i<4;i++)
		    store->run_spare[i] = data->run_spare[i];
	
		/* sound velocity profile */
		store->svp_use_date = data->svp_use_date;
		store->svp_use_msec = data->svp_use_msec;
		store->svp_count = data->svp_count;
		store->svp_serial = data->svp_serial;
		store->svp_origin_date = data->svp_origin_date;
		store->svp_origin_msec = data->svp_origin_msec;
		store->svp_num = data->svp_num;
		store->svp_depth_res = data->svp_depth_res;
		for (i=0;i<MBF_EM300MBA_MAXSVP;i++)
		    {
		    store->svp_depth[i] = data->svp_depth[i];
		    store->svp_vel[i] = data->svp_vel[i];
		    }
		    
		/* position */
		store->pos_date = data->pos_date;
		store->pos_msec = data->pos_msec;
		store->pos_count = data->pos_count;
		store->pos_serial = data->pos_serial;
		store->pos_latitude = data->pos_latitude;
		store->pos_longitude = data->pos_longitude;
		store->pos_quality = data->pos_quality;
		store->pos_speed = data->pos_speed;
		store->pos_course = data->pos_course;
		store->pos_heading = data->pos_heading;
		store->pos_system = data->pos_system;
		store->pos_input_size = data->pos_input_size;
		for (i=0;i<256;i++)
		    store->pos_input[i] = data->pos_input[i];
		    
		/* height */
		store->hgt_date = data->hgt_date;
		store->hgt_msec = data->hgt_msec;
		store->hgt_count = data->hgt_count;
		store->hgt_serial = data->hgt_serial;
		store->hgt_height = data->hgt_height;
		store->hgt_type = data->hgt_type;
		
		/* tide */
		store->tid_date = data->tid_date;
		store->tid_msec = data->tid_msec;
		store->tid_count = data->tid_count;
		store->tid_serial = data->tid_serial;
		store->tid_origin_date = data->tid_origin_date;
		store->tid_origin_msec = data->tid_origin_msec;
		store->tid_tide = data->tid_tide;
		
		/* clock */
		store->clk_date = data->clk_date;
		store->clk_msec = data->clk_msec;
		store->clk_count = data->clk_count;
		store->clk_serial = data->clk_serial;
		store->clk_origin_date = data->clk_origin_date;
		store->clk_origin_msec = data->clk_origin_msec;
		store->clk_1_pps_use = data->clk_1_pps_use;
	
		
		/* allocate secondary data structure for
			attitude data if needed */
		if (data->att_ndata > 0
			&& store->attitude == NULL)
			{
			status = mbsys_simrad2_attitude_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting attitude data into
		    secondary data structure */
		if (status == MB_SUCCESS 
			&& data->att_ndata > 0)
			{
			/* get data structure pointer */
			attitude = (struct mbsys_simrad2_attitude_struct *) 
				store->attitude;
		
			/* attitude data */
			attitude->att_date = data->att_date;
			attitude->att_msec = data->att_msec;
			attitude->att_count = data->att_count;
			attitude->att_serial = data->att_serial;
			attitude->att_ndata = data->att_ndata;	
			for (i=0;i<MBF_EM300MBA_MAXATTITUDE;i++)
			    {
			    attitude->att_time[i] = data->att_time[i];
			    attitude->att_sensor_status[i] = data->att_sensor_status[i];
			    attitude->att_roll[i] = data->att_roll[i];
			    attitude->att_pitch[i] = data->att_pitch[i];
			    attitude->att_heave[i] = data->att_heave[i];
			    attitude->att_heading[i] = data->att_heading[i];
			    }
			attitude->att_heading_status = data->att_heading_status;
			}
		
		/* allocate secondary data structure for
			heading data if needed */
		if (data->hed_ndata > 0
			&& store->heading == NULL)
			{
			status = mbsys_simrad2_heading_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting heading data into
		    secondary data structure */
		if (status == MB_SUCCESS 
			&& data->hed_ndata > 0)
			{
			/* get data structure pointer */
			heading = (struct mbsys_simrad2_heading_struct *) 
				store->heading;
		
			/* heading data */
			heading->hed_date = data->hed_date;	
			heading->hed_msec = data->hed_msec;	
			heading->hed_count = data->hed_count;	
			heading->hed_serial = data->hed_serial;	
			heading->hed_ndata = data->hed_ndata;	
			for (i=0;i<MBF_EM300MBA_MAXHEADING;i++)
			    {
			    heading->hed_time[i] = data->hed_time[i];
			    heading->hed_heading[i] = data->hed_heading[i];
			    }
			heading->hed_heading_status = data->hed_heading_status;
			}
		
		/* allocate secondary data structure for
			survey data if needed */
		if (data->kind == MB_DATA_DATA
			&& store->ping == NULL)
			{
			status = mbsys_simrad2_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting survey data into
		    secondary data structure */
		if (status == MB_SUCCESS 
			&& data->kind == MB_DATA_DATA)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad2_ping_struct *) 
				store->ping;

			/* survey data */
			ping->png_date = data->png_date;	
			ping->png_msec = data->png_msec;
			ping->png_count = data->png_count;	
			ping->png_serial = data->png_serial;	
			ping->png_longitude = data->png_longitude;
			ping->png_latitude = data->png_latitude;
			ping->png_speed = data->png_speed;
			ping->png_heading = data->png_heading;	
			ping->png_ssv = data->png_ssv;	
			ping->png_xducer_depth = data->png_xducer_depth;   
			ping->png_offset_multiplier = data->png_offset_multiplier;	
					   
			/* beam data */
			ping->png_nbeams_max = data->png_nbeams_max;	
			ping->png_nbeams = data->png_nbeams;	
			ping->png_depth_res = data->png_depth_res;	
			ping->png_distance_res = data->png_distance_res;	
			ping->png_sample_rate = data->png_sample_rate;	
			for (i=0;i<ping->png_nbeams;i++)
			    {
			    ping->png_depth[i] = data->png_depth[i];	
			    ping->png_acrosstrack[i] = data->png_acrosstrack[i];
			    ping->png_alongtrack[i] = data->png_alongtrack[i];
			    ping->png_depression[i] = data->png_depression[i];
			    ping->png_azimuth[i] = data->png_azimuth[i];
			    ping->png_range[i] = data->png_range[i];
			    ping->png_quality[i] = data->png_quality[i];	
			    ping->png_window[i] = data->png_window[i];		
			    ping->png_amp[i] = data->png_amp[i];		
			    ping->png_beam_num[i] = data->png_beam_num[i];	
			    ping->png_beamflag[i] = data->png_beamflag[i];
			    }
			ping->png_max_range = data->png_max_range;  
			ping->png_r_zero = data->png_r_zero;	
			ping->png_r_zero_corr = data->png_r_zero_corr;
			ping->png_tvg_start = data->png_tvg_start;	
			ping->png_tvg_stop = data->png_tvg_stop;	\
			ping->png_bsn = data->png_bsn;	
			ping->png_bso = data->png_bso;	
			ping->png_tx = data->png_tx;	
			ping->png_tvg_crossover = data->png_tvg_crossover;	
			ping->png_nbeams_ss = data->png_nbeams_ss;	
			ping->png_npixels = data->png_npixels;	
			for (i=0;i<ping->png_nbeams_ss;i++)
			    {
			    ping->png_beam_index[i] = data->png_beam_index[i];	
			    ping->png_sort_direction[i] = data->png_sort_direction[i];	
			    ping->png_beam_samples[i] = data->png_beam_samples[i];	
			    ping->png_start_sample[i] = data->png_start_sample[i];	
			    ping->png_center_sample[i] = data->png_center_sample[i];	
			    }
			for (i=0;i<ping->png_npixels;i++)
			    {
			    ping->png_ssraw[i] = data->png_ssraw[i];
			    }
			ping->png_pixel_size = data->png_pixel_size;	
			ping->png_pixels_ss = data->png_pixels_ss;	
			for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			    {
			    ping->png_ss[i] = data->png_ss[i];
			    ping->png_ssalongtrack[i] = data->png_ssalongtrack[i];
			    }
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_em300mba(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_em300mba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em300mba_struct *data;
	char	*data_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_attitude_struct *attitude;
	struct mbsys_simrad2_heading_struct *heading;
	struct mbsys_simrad2_ping_struct *ping;
	double	scalefactor;
	int	time_j[5];
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	double	depthoffset;
	int	iss;
	mb_s_char *beam_ss;
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
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* data identifiers */
		data->kind = store->kind;
		data->type = store->type;
		data->sonar = store->sonar;
	
		/* time stamp */
		data->date = store->date;
		data->msec = store->msec;
	
		/* installation parameter values */
		data->par_date = store->par_date;
		data->par_msec = store->par_msec;
		data->par_line_num = store->par_line_num;
		data->par_serial_1 = store->par_serial_1;
		data->par_serial_2 = store->par_serial_2;
		data->par_wlz = store->par_wlz;
		data->par_smh = store->par_smh;
		data->par_s1z = store->par_s1z;
		data->par_s1x = store->par_s1x;
		data->par_s1y = store->par_s1y;
		data->par_s1h = store->par_s1h;
		data->par_s1r = store->par_s1r;
		data->par_s1p = store->par_s1p;
		data->par_s1n = store->par_s1n;
		data->par_s2z = store->par_s2z;
		data->par_s2x = store->par_s2x;
		data->par_s2y = store->par_s2y;
		data->par_s2h = store->par_s2h;
		data->par_s2r = store->par_s2r;
		data->par_s2p = store->par_s2p;
		data->par_s2n = store->par_s2n;
		data->par_go1 = store->par_go1;
		data->par_go2 = store->par_go2;
		for (i=0;i<16;i++)
		    {
		    data->par_tsv[i] = store->par_tsv[i];
		    data->par_rsv[i] = store->par_rsv[i];
		    data->par_bsv[i] = store->par_bsv[i];
		    data->par_psv[i] = store->par_psv[i];
		    data->par_osv[i] = store->par_osv[i];
		    }
		data->par_dsd = store->par_dsd;
		data->par_dso = store->par_dso;
		data->par_dsf = store->par_dsf;
		data->par_dsh[0] = store->par_dsh[0];
		data->par_dsh[1] = store->par_dsh[1];
		data->par_aps = store->par_aps;
		data->par_p1m = store->par_p1m;
		data->par_p1t = store->par_p1t;
		data->par_p1z = store->par_p1z;
		data->par_p1x = store->par_p1x;
		data->par_p1y = store->par_p1y;
		data->par_p1d = store->par_p1d;
		for (i=0;i<16;i++)
		    {
		    data->par_p1g[i] = store->par_p1g[i];
		    }
		data->par_p2m = store->par_p2m;
		data->par_p2t = store->par_p2t;
		data->par_p2z = store->par_p2z;
		data->par_p2x = store->par_p2x;
		data->par_p2y = store->par_p2y;
		data->par_p2d = store->par_p2d;
		for (i=0;i<16;i++)
		    {
		    data->par_p2g[i] = store->par_p2g[i];
		    }
		data->par_p3m = store->par_p3m;
		data->par_p3t = store->par_p3t;
		data->par_p3z = store->par_p3z;
		data->par_p3x = store->par_p3x;
		data->par_p3y = store->par_p3y;
		data->par_p3d = store->par_p3d;
		for (i=0;i<16;i++)
		    {
		    data->par_p3g[i] = store->par_p3g[i];
		    }
		data->par_msz = store->par_msz;
		data->par_msx = store->par_msx;
		data->par_msy = store->par_msy;
		data->par_mrp[0] = store->par_mrp[0];
		data->par_mrp[1] = store->par_mrp[1];
		data->par_msd = store->par_msd;
		data->par_msr = store->par_msr;
		data->par_msp = store->par_msp;
		data->par_msg = store->par_msg;
		data->par_gcg = store->par_gcg;
		for (i=0;i<4;i++)
		    {
		    data->par_cpr[i] = store->par_cpr[i];
		    }
		for (i=0;i<MBSYS_SIMRAD2_COMMENT_LENGTH;i++)
		    {
		    data->par_rop[i] = store->par_rop[i];
		    data->par_sid[i] = store->par_sid[i];
		    data->par_pll[i] = store->par_pll[i];
		    data->par_com[i] = store->par_com[i];
		    }
	
		/* runtime parameter values */
		data->run_date = store->run_date;
		data->run_msec = store->run_msec;
		data->run_ping_count = store->run_ping_count;
		data->run_serial = store->run_serial;
		data->run_status = store->run_status;
		data->run_mode = store->run_mode;
		data->run_filter_id = store->run_filter_id;
		data->run_min_depth = store->run_min_depth;
		data->run_max_depth = store->run_max_depth;
		data->run_absorption = store->run_absorption;
		data->run_tran_pulse = store->run_tran_pulse;
		data->run_tran_beam = store->run_tran_beam;
		data->run_tran_pow = store->run_tran_pow;
		data->run_rec_beam = store->run_rec_beam;
		data->run_rec_band = store->run_rec_band;
		data->run_rec_gain = store->run_rec_gain;
		data->run_tvg_cross = store->run_tvg_cross;
		data->run_ssv_source = store->run_ssv_source;
		data->run_max_swath = store->run_max_swath;
		data->run_beam_space = store->run_beam_space;
		data->run_swath_angle = store->run_swath_angle;
		data->run_stab_mode = store->run_stab_mode;
		for (i=0;i<4;i++)
		    data->run_spare[i] = store->run_spare[i];
	
		/* sound velocity profile */
		data->svp_use_date = store->svp_use_date;
		data->svp_use_msec = store->svp_use_msec;
		data->svp_count = store->svp_count;
		data->svp_serial = store->svp_serial;
		data->svp_origin_date = store->svp_origin_date;
		data->svp_origin_msec = store->svp_origin_msec;
		data->svp_num = store->svp_num;
		data->svp_depth_res = store->svp_depth_res;
		for (i=0;i<MBF_EM300MBA_MAXSVP;i++)
		    {
		    data->svp_depth[i] = store->svp_depth[i];
		    data->svp_vel[i] = store->svp_vel[i];
		    }
		    
		/* position */
		data->pos_date = store->pos_date;
		data->pos_msec = store->pos_msec;
		data->pos_count = store->pos_count;
		data->pos_serial = store->pos_serial;
		data->pos_latitude = store->pos_latitude;
		data->pos_longitude = store->pos_longitude;
		data->pos_quality = store->pos_quality;
		data->pos_speed = store->pos_speed;
		data->pos_course = store->pos_course;
		data->pos_heading = store->pos_heading;
		data->pos_system = store->pos_system;
		data->pos_input_size = store->pos_input_size;
		for (i=0;i<256;i++)
		    data->pos_input[i] = store->pos_input[i];
		    
		/* height */
		data->hgt_date = store->hgt_date;
		data->hgt_msec = store->hgt_msec;
		data->hgt_count = store->hgt_count;
		data->hgt_serial = store->hgt_serial;
		data->hgt_height = store->hgt_height;
		data->hgt_type = store->hgt_type;
		
		/* tide */
		data->tid_date = store->tid_date;
		data->tid_msec = store->tid_msec;
		data->tid_count = store->tid_count;
		data->tid_serial = store->tid_serial;
		data->tid_origin_date = store->tid_origin_date;
		data->tid_origin_msec = store->tid_origin_msec;
		data->tid_tide = store->tid_tide;
		
		/* clock */
		data->clk_date = store->clk_date;
		data->clk_msec = store->clk_msec;
		data->clk_count = store->clk_count;
		data->clk_serial = store->clk_serial;
		data->clk_origin_date = store->clk_origin_date;
		data->clk_origin_msec = store->clk_origin_msec;
		data->clk_1_pps_use = store->clk_1_pps_use;
		
		/* deal with putting attitude data into
		    secondary data structure */
		if (store->attitude != NULL)
			{
			/* get data structure pointer */
			attitude = (struct mbsys_simrad2_attitude_struct *) 
				store->attitude;
		
			/* attitude data */
			data->att_date = attitude->att_date;
			data->att_msec = attitude->att_msec;
			data->att_count = attitude->att_count;
			data->att_serial = attitude->att_serial;
			data->att_ndata = attitude->att_ndata;	
			for (i=0;i<MBF_EM300MBA_MAXATTITUDE;i++)
			    {
			    data->att_time[i] = attitude->att_time[i];
			    data->att_sensor_status[i] = attitude->att_sensor_status[i];
			    data->att_roll[i] = attitude->att_roll[i];
			    data->att_pitch[i] = attitude->att_pitch[i];
			    data->att_heave[i] = attitude->att_heave[i];
			    data->att_heading[i] = attitude->att_heading[i];
			    }
			data->att_heading_status = attitude->att_heading_status;
			}
		
		/* deal with putting heading data into
		    secondary data structure */
		if (store->heading != NULL)
			{
			/* get data structure pointer */
			heading = (struct mbsys_simrad2_heading_struct *) 
				store->heading;
		
			/* heading data */
			data->hed_date = heading->hed_date;	
			data->hed_msec = heading->hed_msec;	
			data->hed_count = heading->hed_count;	
			data->hed_serial = heading->hed_serial;	
			data->hed_ndata = heading->hed_ndata;	
			for (i=0;i<MBF_EM300MBA_MAXHEADING;i++)
			    {
			    data->hed_time[i] = heading->hed_time[i];
			    data->hed_heading[i] = heading->hed_heading[i];
			    }
			data->hed_heading_status = heading->hed_heading_status;
			}
		
		/* deal with putting survey data into
		    secondary data structure */
		if (store->ping != NULL)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad2_ping_struct *) 
				store->ping;

			/* survey data */
			data->png_date = ping->png_date;	
			data->png_msec = ping->png_msec;	
			data->png_count = ping->png_count;	
			data->png_serial = ping->png_serial;	
			data->png_longitude = ping->png_longitude;
			data->png_latitude = ping->png_latitude;
			data->png_speed = ping->png_speed;
			data->png_heading = ping->png_heading;	
			data->png_ssv = ping->png_ssv;	
			data->png_xducer_depth = ping->png_xducer_depth;   
			data->png_offset_multiplier = ping->png_offset_multiplier;	
					   
			/* beam data */
			data->png_nbeams_max = ping->png_nbeams_max;	
			data->png_nbeams = ping->png_nbeams;	
			data->png_depth_res = ping->png_depth_res;	
			data->png_distance_res = ping->png_distance_res;	
			data->png_sample_rate = ping->png_sample_rate;	
			for (i=0;i<data->png_nbeams;i++)
			    {
			    data->png_depth[i] = ping->png_depth[i];	
			    data->png_acrosstrack[i] = ping->png_acrosstrack[i];
			    data->png_alongtrack[i] = ping->png_alongtrack[i];
			    data->png_depression[i] = ping->png_depression[i];
			    data->png_azimuth[i] = ping->png_azimuth[i];
			    data->png_range[i] = ping->png_range[i];
			    data->png_quality[i] = ping->png_quality[i];	
			    data->png_window[i] = ping->png_window[i];		
			    data->png_amp[i] = ping->png_amp[i];		
			    data->png_beam_num[i] = ping->png_beam_num[i];	
			    data->png_beamflag[i] = ping->png_beamflag[i];	
			    }
			data->png_ss_date = ping->png_date;	
			data->png_ss_msec = ping->png_msec;	
			data->png_max_range = ping->png_max_range;  
			data->png_r_zero = ping->png_r_zero;	
			data->png_r_zero_corr = ping->png_r_zero_corr;
			data->png_tvg_start = ping->png_tvg_start;	
			data->png_tvg_stop = ping->png_tvg_stop;	\
			data->png_bsn = ping->png_bsn;	
			data->png_bso = ping->png_bso;	
			data->png_tx = ping->png_tx;	
			data->png_tvg_crossover = ping->png_tvg_crossover;	
			data->png_nbeams_ss = ping->png_nbeams_ss;	
			data->png_npixels = ping->png_npixels;	
			for (i=0;i<data->png_nbeams_ss;i++)
			    {
			    data->png_beam_index[i] = ping->png_beam_index[i];	
			    data->png_sort_direction[i] = ping->png_sort_direction[i];	
			    data->png_beam_samples[i] = ping->png_beam_samples[i];	
			    data->png_start_sample[i] = ping->png_start_sample[i];	
			    data->png_center_sample[i] = ping->png_center_sample[i];	
			    }
			for (i=0;i<data->png_npixels;i++)
			    {
			    data->png_ssraw[i] = ping->png_ssraw[i];
			    }
			data->png_pixel_size = ping->png_pixel_size;	
			data->png_pixels_ss = ping->png_pixels_ss;	
			for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			    {
			    data->png_ss[i] = ping->png_ss[i];
			    data->png_ssalongtrack[i] = ping->png_ssalongtrack[i];
			    }
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* set times from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		{
		/* get time */
		data->date = 10000 * mb_io_ptr->new_time_i[0]
				    + 100 * mb_io_ptr->new_time_i[1]
				    + mb_io_ptr->new_time_i[2];
		data->msec = 3600000 * mb_io_ptr->new_time_i[3]
				    + 60000 * mb_io_ptr->new_time_i[4]
				    + 1000 * mb_io_ptr->new_time_i[5]
				    + 0.001 * mb_io_ptr->new_time_i[6];
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		data->par_date = data->date;
		data->par_msec = data->msec;
		strncpy(data->par_com,mb_io_ptr->new_comment,
			MBF_EM300MBA_COMMENT_LENGTH);
		}

	/* check for parameter to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_PARAMETER)
		{
		data->par_date = data->date;
		data->par_msec = data->msec;
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		data->png_date = data->date;
		data->png_msec = data->msec;
		data->png_ss_date = data->date;
		data->png_ss_msec = data->msec;

		/* get nav  */
		data->png_longitude = (int) (10000000 * mb_io_ptr->new_lon);
		data->png_latitude = (int) (20000000 * mb_io_ptr->new_lat);

		/* get speed  */
		data->png_speed = (int) mb_io_ptr->new_speed / 0.036;

		/* get heading */
		data->png_heading = (int) (mb_io_ptr->new_heading * 100);

		/* insert distance and depth values into storage arrays */
		if (data->sonar == MBSYS_SIMRAD2_UNKNOWN)
			{
			if (mb_io_ptr->beams_bath <= 127)
				{
				data->sonar = MBSYS_SIMRAD2_EM3000;
				if (data->png_depth_res == 0)
				    data->png_depth_res = 1; /* kluge */
				if (data->png_distance_res == 0)
				    data->png_distance_res = 1; /* kluge */
				}
			else if (mb_io_ptr->beams_bath <= 135)
				{
				data->sonar = MBSYS_SIMRAD2_EM300;
				if (data->png_depth_res == 0)
				    data->png_depth_res = 10; /* kluge */
				if (data->png_distance_res == 0)
				    data->png_distance_res = 10; /* kluge */
				}
			else if (mb_io_ptr->beams_bath <= 254)
				{
				store->sonar = MBSYS_SIMRAD2_EM3000D_2;
				if (data->png_depth_res == 0)
				    data->png_depth_res = 1; /* kluge */
				if (data->png_distance_res == 0)
				    data->png_distance_res = 1; /* kluge */
				}
			else
				{
				*error = MB_ERROR_DATA_NOT_INSERTED;
				status = MB_FAILURE;
				}
			}
		depthscale = 0.01 * data->png_depth_res;
		depthoffset = 0.01 * data->png_xducer_depth
				+ 655.36 * data->png_offset_multiplier;
		dacrscale  = 0.01 * data->png_distance_res;
		daloscale  = 0.01 * data->png_distance_res;
		if (data->sonar == 300 || data->sonar == 3000)
		    ttscale = 0.5 / data->png_sample_rate;
		else
		    ttscale = 0.5 / 14000;
		reflscale  = 0.5;
		if (status == MB_SUCCESS && data->png_nbeams == 0)
			{
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			    if (mb_io_ptr->new_beamflag[i] != MB_FLAG_NULL)
				{
				j = data->png_nbeams;
				data->png_beam_num[j] = i + 1;
				data->png_depth[j] = (mb_io_ptr->new_bath[i] - depthoffset)
							/ depthscale;
				data->png_acrosstrack[j]
					= mb_io_ptr->new_bath_acrosstrack[i] / dacrscale;
				data->png_alongtrack[j] 
					= mb_io_ptr->new_bath_alongtrack[i] / daloscale;
				if (mb_io_ptr->new_amp[i] != 0.0)
					data->png_amp[j] = (mb_io_ptr->new_amp[i] - 64) 
						/ reflscale;
				else
					data->png_amp[j] = 0;
				data->png_beamflag[j] = mb_io_ptr->new_beamflag[i];
				data->png_nbeams++;
				}
			data->png_nbeams_max = data->png_nbeams;
			}
		else if (status == MB_SUCCESS)
			{
			for (j=0;j<data->png_nbeams;j++)
				{
				i = data->png_beam_num[j] - 1;
				data->png_depth[j] = (mb_io_ptr->new_bath[i] - depthoffset)
							/ depthscale;
				data->png_acrosstrack[j]
					= mb_io_ptr->new_bath_acrosstrack[i] / dacrscale;
				data->png_alongtrack[j] 
					= mb_io_ptr->new_bath_alongtrack[i] / daloscale;
				if (mb_io_ptr->new_amp[i] != 0.0)
					data->png_amp[j] = (mb_io_ptr->new_amp[i] - 64) 
						/ reflscale;
				else
					data->png_amp[j] = 0;
				data->png_beamflag[j] = mb_io_ptr->new_beamflag[i];
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<mb_io_ptr->pixels_ss;i++)
				{
				data->png_ss[i] = 100 * mb_io_ptr->new_ss[i];
				data->png_ssalongtrack[i] = mb_io_ptr->new_ss_alongtrack[i] / daloscale;
				}
			}
		}

	/* else check for nav data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_NAV)
		{
		/* get time */
		data->pos_date = data->date;
		data->pos_msec = data->msec;

		/* get navigation */
		data->pos_longitude = 10000000 * mb_io_ptr->new_lon;
		data->pos_latitude = 20000000 * mb_io_ptr->new_lat;

		/* get heading */
		data->pos_heading = (int) (mb_io_ptr->new_heading * 100);

		/* get speed  */
		data->pos_speed = (int)(mb_io_ptr->new_speed / 0.036);

		/* get roll pitch and heave */
		}

	/* write next data to file */
	status = mbr_em300mba_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_em300mba_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em300mba_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	char	*label;
	int	*label_save_flag;
	short	expect;
	short	*type;
	short	*sonar;
	short	first_type;
	short	*expect_save;
	int	*expect_save_flag;
	short	*first_type_save;
	int	match;
	int	read_len;
	int	i;
	
/*#define MBR_EM300MBA_DEBUG 1*/

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	label = (char *) mb_io_ptr->save_label;
	type = (short *) mb_io_ptr->save_label;
	sonar = (short *) (&mb_io_ptr->save_label[2]);
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	expect_save_flag = (int *) &mb_io_ptr->save_flag;
	expect_save = (short *) &mb_io_ptr->save1;
	first_type_save = (short *) &mb_io_ptr->save2;
	if (*expect_save_flag == MB_YES)
		{
		expect = *expect_save;
		first_type = *first_type_save;
		*expect_save_flag = MB_NO;
		}
	else
		{
		expect = EM2_NONE;
		first_type = EM2_NONE;
		}

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		if (*label_save_flag == MB_NO)
			{
			/* look for label */
			if ((read_len = fread(label,
				1,4,mb_io_ptr->mbfp)) != 4)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			while (status == MB_SUCCESS
				&& mbr_em300mba_chk_label(verbose, 
					*type, *sonar) != MB_SUCCESS)
			    {
			    /* get next byte */
			    for (i=0;i<3;i++)
				label[i] = label[i+1];
			    if ((read_len = fread(&label[3],
				    1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}				
			    }
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short) mb_swap_short(*type);
		*sonar = (short) mb_swap_short(*sonar);
#endif

#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"\nstart of mbr_em300mba_rd_data loop:\n");
	fprintf(stderr,"done:%d\n",done);
	fprintf(stderr,"expect:%x\n",expect);
	fprintf(stderr,"type:%x\n",*type);
	fprintf(stderr,"sonar:%d\n",*sonar);
	fprintf(stderr,"first_type:%x\n",first_type);
#endif

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM2_NONE)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call nothing, read failure, no expect\n");
#endif
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != EM2_NONE)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call nothing, read failure, expect %x\n",expect);
#endif
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (*type != EM2_START
			&& *type != EM2_STOP
			&& *type != EM2_STOP2
			&& *type != EM2_OFF
			&& *type != EM2_ON
			&& *type != EM2_RUN_PARAMETER
			&& *type != EM2_CLOCK
			&& *type != EM2_TIDE
			&& *type != EM2_HEIGHT
			&& *type != EM2_HEADING
			&& *type != EM2_ATTITUDE
			&& *type != EM2_POS
			&& *type != EM2_SVP
			&& *type != EM2_BATH_MBA
			&& *type != EM2_SS_MBA)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call nothing, try again\n");
#endif
			done = MB_NO;
			}
		else if (*type == EM2_START
			|| *type == EM2_STOP
			|| *type == EM2_STOP2
			|| *type == EM2_OFF
			|| *type == EM2_ON)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_start type %x\n",*type);
#endif
			status = mbr_em300mba_rd_start(
				verbose,mbfp,data,*type,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_RUN_PARAMETER)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_run_parameter type %x\n",*type);
#endif
			status = mbr_em300mba_rd_run_parameter(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_CLOCK)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_clock type %x\n",*type);
#endif
			status = mbr_em300mba_rd_clock(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_TIDE)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_tide type %x\n",*type);
#endif
			status = mbr_em300mba_rd_tide(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_HEIGHT)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_height type %x\n",*type);
#endif
			status = mbr_em300mba_rd_height(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_HEADING)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_heading type %x\n",*type);
#endif
			status = mbr_em300mba_rd_heading(
				5,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_ATTITUDE)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_attitude type %x\n",*type);
#endif
			status = mbr_em300mba_rd_attitude(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}	
		else if (*type == EM2_POS)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_pos type %x\n",*type);
#endif
			status = mbr_em300mba_rd_pos(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_SVP)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_svp type %x\n",*type);
#endif
			status = mbr_em300mba_rd_svp(
				verbose,mbfp,data,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_BATH_MBA 
			&& expect != EM2_NONE 
			&& expect != EM2_BATH_MBA)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			done = MB_YES;
			expect = EM2_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM2_BATH_MBA)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_bath type %x\n",*type);
#endif
			status = mbr_em300mba_rd_bath(
				verbose,mbfp,data,&match,*sonar,error);
			if (status == MB_SUCCESS)
				{
				if (first_type == EM2_NONE
					|| match == MB_NO)
					{
					done = MB_NO;
					first_type = EM2_BATH_MBA;
					expect = EM2_SS_MBA;
					}
				else
					{
					done = MB_YES;
					expect = EM2_NONE;
					}
				}
			}
		else if (*type == EM2_SS_MBA 
			&& expect != EM2_NONE 
			&& expect != EM2_SS_MBA)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			done = MB_YES;
			expect = EM2_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM2_SS_MBA)
			{
#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"call mbr_em300mba_rd_ss type %x\n",*type);
#endif
			status = mbr_em300mba_rd_ss(
				verbose,mbfp,data,*sonar,&match,error);
			if (status == MB_SUCCESS)
				{
				if (first_type == EM2_NONE
					|| match == MB_NO)
					{
					done = MB_NO;
					first_type = EM2_SS_MBA;
					expect = EM2_BATH_MBA;
					}
				else
					{
					done = MB_YES;
					expect = EM2_NONE;
					}
				}
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;

#ifdef MBR_EM300MBA_DEBUG
	fprintf(stderr,"end of mbr_em300mba_rd_data loop:\n");
	fprintf(stderr,"status:%d error:%d\n",status, *error);
	fprintf(stderr,"done:%d\n",done);
	fprintf(stderr,"expect:%x\n",expect);
	fprintf(stderr,"type:%x\n",*type);
	fprintf(stderr,"sonar:%x\n",*sonar);
#endif
		}
		
	/* get file position */
	if (*label_save_flag == MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp) - 2;
	else if (*expect_save_flag != MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp);

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
int mbr_em300mba_chk_label(verbose,type,sonar)
int	verbose;
short	type;
short	sonar;
{
	char	*function_name = "mbr_em300mba_chk_label";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short) mb_swap_short(*type);
		*sonar = (short) mb_swap_short(*sonar);
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2  Input values byte swapped:\n");
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
#endif

	/* check for valid label */
	if (type != EM2_START
		&& type != EM2_STOP
		&& type != EM2_STOP2
		&& type != EM2_OFF
		&& type != EM2_ON
		&& type != EM2_RUN_PARAMETER
		&& type != EM2_CLOCK
		&& type != EM2_TIDE
		&& type != EM2_HEIGHT
		&& type != EM2_HEADING
		&& type != EM2_ATTITUDE
		&& type != EM2_POS
		&& type != EM2_SVP
		&& type != EM2_BATH_MBA
		&& type != EM2_SS_MBA)
		{
		status = MB_FAILURE;
		}
		
	/* check for valid sonar model */
	if (sonar != EM2_EM300
		&& sonar != EM2_EM3000
		&& sonar != EM2_EM3000D_1
		&& sonar != EM2_EM3000D_2
		&& sonar != EM2_EM3000D_3
		&& sonar != EM2_EM3000D_4
		&& sonar != EM2_EM3000D_5
		&& sonar != EM2_EM3000D_6
		&& sonar != EM2_EM3000D_7)
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_rd_start(verbose,mbfp,data,type,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	type;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_start";
	int	status = MB_SUCCESS;
	char	line[MBF_EM300MBA_BUFFER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	char	*comma_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* make sure comment is initialized */
	data->par_com[0] = '\0';
	
	/* set type value */
	data->type = type;
	data->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_START_HEADER_SIZE,mbfp);
	if (read_len == EM2_START_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->par_date = (int) mb_swap_int(*int_ptr);
		data->date = data->par_date;
		int_ptr = (int *) &line[4];
		data->par_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->par_msec;
		short_ptr = (short *) &line[8];
		data->par_line_num = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->par_serial_1 = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		data->par_serial_2 = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->par_date = (int) *int_ptr;
		data->date = data->par_date;
		int_ptr = (int *) &line[4];
		data->par_msec = (int) *int_ptr;
		data->msec = data->par_msec;
		short_ptr = (short *) &line[8];
		data->par_line_num = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->par_serial_1 = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		data->par_serial_2 = (unsigned short) *short_ptr;
#endif
		}
		
	/* now loop over reading individual characters to 
	    handle ASCII parameter values */
	done = MB_NO;
	len = 0;
	while (status == MB_SUCCESS && done == MB_NO)
		{
		read_len = fread(&line[len],1,1,mbfp);
		if (read_len == 1)
			{
			status = MB_SUCCESS;
			len++;
			}
		else
			{
			done = MB_YES;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
			
		if (status == MB_SUCCESS 
			&& line[len-1] == ','
			&& len > 5)
			{
			line[len] = 0;
			if (strncmp("WLZ=", line, 4) == 0)
			    mb_get_double(&(data->par_wlz), &line[4], len-5);
			else if (strncmp("SMH=", line, 4) == 0)
			    mb_get_int(&(data->par_smh), &line[4], len-5);
			else if (strncmp("S1Z=", line, 4) == 0)
			    mb_get_double(&(data->par_s1z), &line[4], len-5);
			else if (strncmp("S1X=", line, 4) == 0)
			    mb_get_double(&(data->par_s1x), &line[4], len-5);
			else if (strncmp("S1Y=", line, 4) == 0)
			    mb_get_double(&(data->par_s1y), &line[4], len-5);
			else if (strncmp("S1H=", line, 4) == 0)
			    mb_get_double(&(data->par_s1h), &line[4], len-5);
			else if (strncmp("S1R=", line, 4) == 0)
			    mb_get_double(&(data->par_s1r), &line[4], len-5);
			else if (strncmp("S1P=", line, 4) == 0)
			    mb_get_double(&(data->par_s1p), &line[4], len-5);
			else if (strncmp("S1N=", line, 4) == 0)
			    mb_get_int(&(data->par_s1n), &line[4], len-5);
			else if (strncmp("S2Z=", line, 4) == 0)
			    mb_get_double(&(data->par_s2z), &line[4], len-5);
			else if (strncmp("S2X=", line, 4) == 0)
			    mb_get_double(&(data->par_s2x), &line[4], len-5);
			else if (strncmp("S2Y=", line, 4) == 0)
			    mb_get_double(&(data->par_s2y), &line[4], len-5);
			else if (strncmp("S2H=", line, 4) == 0)
			    mb_get_double(&(data->par_s2h), &line[4], len-5);
			else if (strncmp("S2R=", line, 4) == 0)
			    mb_get_double(&(data->par_s2r), &line[4], len-5);
			else if (strncmp("S2P=", line, 4) == 0)
			    mb_get_double(&(data->par_s2p), &line[4], len-5);
			else if (strncmp("S2N=", line, 4) == 0)
			    mb_get_int(&(data->par_s2n), &line[4], len-5);
			else if (strncmp("GO1=", line, 4) == 0)
			    mb_get_double(&(data->par_go1), &line[4], len-5);
			else if (strncmp("GO2=", line, 4) == 0)
			    mb_get_double(&(data->par_go2), &line[4], len-5);
			else if (strncmp("TSV=", line, 4) == 0)
			    strncpy(data->par_tsv, &line[4], MIN(len-5, 15));
			else if (strncmp("RSV=", line, 4) == 0)
			    strncpy(data->par_rsv, &line[4], MIN(len-5, 15));
			else if (strncmp("BSV=", line, 4) == 0)
			    strncpy(data->par_bsv, &line[4], MIN(len-5, 15));
			else if (strncmp("PSV=", line, 4) == 0)
			    strncpy(data->par_psv, &line[4], MIN(len-5, 15));
			else if (strncmp("OSV=", line, 4) == 0)
			    strncpy(data->par_osv, &line[4], MIN(len-5, 15));
			else if (strncmp("DSD=", line, 4) == 0)
			    mb_get_double(&(data->par_dsd), &line[4], len-5);
			else if (strncmp("DSO=", line, 4) == 0)
			    mb_get_double(&(data->par_dso), &line[4], len-5);
			else if (strncmp("DSF=", line, 4) == 0)
			    mb_get_double(&(data->par_dsf), &line[4], len-5);
			else if (strncmp("DSH=", line, 4) == 0)
			    {
			    data->par_dsh[0] = line[4];
			    data->par_dsh[1] = line[5];
			    }
			else if (strncmp("APS=", line, 4) == 0)
			    mb_get_int(&(data->par_aps), &line[4], len-5);
			else if (strncmp("P1M=", line, 4) == 0)
			    mb_get_int(&(data->par_p1m), &line[4], len-5);
			else if (strncmp("P1T=", line, 4) == 0)
			    mb_get_int(&(data->par_p1t), &line[4], len-5);
			else if (strncmp("P1Z=", line, 4) == 0)
			    mb_get_double(&(data->par_p1z), &line[4], len-5);
			else if (strncmp("P1X=", line, 4) == 0)
			    mb_get_double(&(data->par_p1x), &line[4], len-5);
			else if (strncmp("P1Y=", line, 4) == 0)
			    mb_get_double(&(data->par_p1y), &line[4], len-5);
			else if (strncmp("P1D=", line, 4) == 0)
			    mb_get_double(&(data->par_p1d), &line[4], len-5);
			else if (strncmp("P1G=", line, 4) == 0)
			    strncpy(data->par_p1g, &line[4], MIN(len-5, 15));
			else if (strncmp("P2M=", line, 4) == 0)
			    mb_get_int(&(data->par_p2m), &line[4], len-5);
			else if (strncmp("P2T=", line, 4) == 0)
			    mb_get_int(&(data->par_p2t), &line[4], len-5);
			else if (strncmp("P2Z=", line, 4) == 0)
			    mb_get_double(&(data->par_p2z), &line[4], len-5);
			else if (strncmp("P2X=", line, 4) == 0)
			    mb_get_double(&(data->par_p2x), &line[4], len-5);
			else if (strncmp("P2Y=", line, 4) == 0)
			    mb_get_double(&(data->par_p2y), &line[4], len-5);
			else if (strncmp("P2D=", line, 4) == 0)
			    mb_get_double(&(data->par_p2d), &line[4], len-5);
			else if (strncmp("P2G=", line, 4) == 0)
			    strncpy(data->par_p2g, &line[4], MIN(len-5, 15));
			else if (strncmp("P3M=", line, 4) == 0)
			    mb_get_int(&(data->par_p3m), &line[4], len-5);
			else if (strncmp("P3T=", line, 4) == 0)
			    mb_get_int(&(data->par_p3t), &line[4], len-5);
			else if (strncmp("P3Z=", line, 4) == 0)
			    mb_get_double(&(data->par_p3z), &line[4], len-5);
			else if (strncmp("P3X=", line, 4) == 0)
			    mb_get_double(&(data->par_p3x), &line[4], len-5);
			else if (strncmp("P3Y=", line, 4) == 0)
			    mb_get_double(&(data->par_p3y), &line[4], len-5);
			else if (strncmp("P3D=", line, 4) == 0)
			    mb_get_double(&(data->par_p3d), &line[4], len-5);
			else if (strncmp("P3G=", line, 4) == 0)
			    strncpy(data->par_p3g, &line[4], MIN(len-5, 15));
			else if (strncmp("MSZ=", line, 4) == 0)
			    mb_get_double(&(data->par_msz), &line[4], len-5);
			else if (strncmp("MSX=", line, 4) == 0)
			    mb_get_double(&(data->par_msx), &line[4], len-5);
			else if (strncmp("MSY=", line, 4) == 0)
			    mb_get_double(&(data->par_msy), &line[4], len-5);
			else if (strncmp("MRP=", line, 4) == 0)
			    {
			    data->par_mrp[0] = line[4];
			    data->par_mrp[1] = line[5];
			    }
			else if (strncmp("MSD=", line, 4) == 0)
			    mb_get_double(&(data->par_msd), &line[4], len-5);
			else if (strncmp("MSR=", line, 4) == 0)
			    mb_get_double(&(data->par_msr), &line[4], len-5);
			else if (strncmp("MSP=", line, 4) == 0)
			    mb_get_double(&(data->par_msp), &line[4], len-5);
			else if (strncmp("MSG=", line, 4) == 0)
			    mb_get_double(&(data->par_msg), &line[4], len-5);
			else if (strncmp("GCG=", line, 4) == 0)
			    mb_get_double(&(data->par_gcg), &line[4], len-5);
			else if (strncmp("CPR=", line, 4) == 0)
			    strncpy(data->par_cpr, &line[4], MIN(len-5, 3));
			else if (strncmp("ROP=", line, 4) == 0)
			    strncpy(data->par_rop, &line[4], MIN(len-5, MBF_EM300MBA_COMMENT_LENGTH-1));
			else if (strncmp("SID=", line, 4) == 0)
			    strncpy(data->par_sid, &line[4], MIN(len-5, MBF_EM300MBA_COMMENT_LENGTH-1));
			else if (strncmp("PLL=", line, 4) == 0)
			    strncpy(data->par_pll, &line[4], MIN(len-5, MBF_EM300MBA_COMMENT_LENGTH-1));
			else if (strncmp("COM=", line, 4) == 0)
			    {
			    strncpy(data->par_com, &line[4], MIN(len-5, MBF_EM300MBA_COMMENT_LENGTH-1));
			    data->par_com[MIN(len-5, MBF_EM300MBA_COMMENT_LENGTH-1)] = 0;
			    /* replace caret (^) values with commas (,) to circumvent
			       the format's inability to store commas in comments */
			    while ((comma_ptr = strchr(data->par_com, '^')) != NULL)
				{
				comma_ptr[0] = ',';
				}
			    }
			len = 0;
			}
		else if (status == MB_SUCCESS 
			&& line[len-1] == ','
			&& len == 5)
			{
			len = 0;
			}
		else if (status == MB_SUCCESS 
			&& line[len-1] == ','
			&& len < 5)
			{
			done = MB_YES;
			}
		}
		
	/* now set the data kind */
	if (status == MB_SUCCESS)
		{
		if (strlen(data->par_com) > 0)
		    data->kind = MB_DATA_COMMENT;
		else if (data->type == EM2_START)
		    data->kind = MB_DATA_START;
		else if (data->type == EM2_STOP)
		    data->kind = MB_DATA_STOP;
		else if (data->type == EM2_STOP2)
		    data->kind = MB_DATA_STOP;
		else if (data->type == EM2_OFF)
		    data->kind = MB_DATA_STOP;
		else if (data->type == EM2_ON)
		    data->kind = MB_DATA_START;
		}
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM2_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[0],2,1,mbfp);
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       par_date:        %d\n",data->par_date);
		fprintf(stderr,"dbg5       par_msec:        %d\n",data->par_msec);
		fprintf(stderr,"dbg5       par_line_num:    %d\n",data->par_line_num);
		fprintf(stderr,"dbg5       par_serial_1:    %d\n",data->par_serial_1);
		fprintf(stderr,"dbg5       par_serial_2:    %d\n",data->par_serial_2);
		fprintf(stderr,"dbg5       par_wlz:         %f\n",data->par_wlz);
		fprintf(stderr,"dbg5       par_smh:         %d\n",data->par_smh);
		fprintf(stderr,"dbg5       par_s1z:         %f\n",data->par_s1z);
		fprintf(stderr,"dbg5       par_s1x:         %f\n",data->par_s1x);
		fprintf(stderr,"dbg5       par_s1y:         %f\n",data->par_s1y);
		fprintf(stderr,"dbg5       par_s1h:         %f\n",data->par_s1h);
		fprintf(stderr,"dbg5       par_s1r:         %f\n",data->par_s1r);
		fprintf(stderr,"dbg5       par_s1p:         %f\n",data->par_s1p);
		fprintf(stderr,"dbg5       par_s1n:         %d\n",data->par_s1n);
		fprintf(stderr,"dbg5       par_s2z:         %f\n",data->par_s2z);
		fprintf(stderr,"dbg5       par_s2x:         %f\n",data->par_s2x);
		fprintf(stderr,"dbg5       par_s2y:         %f\n",data->par_s2y);
		fprintf(stderr,"dbg5       par_s2h:         %f\n",data->par_s2h);
		fprintf(stderr,"dbg5       par_s2r:         %f\n",data->par_s2r);
		fprintf(stderr,"dbg5       par_s2p:         %f\n",data->par_s2p);
		fprintf(stderr,"dbg5       par_s2n:         %d\n",data->par_s2n);
		fprintf(stderr,"dbg5       par_go1:         %f\n",data->par_go1);
		fprintf(stderr,"dbg5       par_go2:         %f\n",data->par_go2);
		fprintf(stderr,"dbg5       par_tsv:         %s\n",data->par_tsv);
		fprintf(stderr,"dbg5       par_rsv:         %s\n",data->par_rsv);
		fprintf(stderr,"dbg5       par_bsv:         %s\n",data->par_bsv);
		fprintf(stderr,"dbg5       par_psv:         %s\n",data->par_psv);
		fprintf(stderr,"dbg5       par_osv:         %s\n",data->par_osv);
		fprintf(stderr,"dbg5       par_dsd:         %f\n",data->par_dsd);
		fprintf(stderr,"dbg5       par_dso:         %f\n",data->par_dso);
		fprintf(stderr,"dbg5       par_dsf:         %f\n",data->par_dsf);
		fprintf(stderr,"dbg5       par_dsh:         %c%c\n",
			data->par_dsh[0],data->par_dsh[1]);
		fprintf(stderr,"dbg5       par_aps:         %d\n",data->par_aps);
		fprintf(stderr,"dbg5       par_p1m:         %d\n",data->par_p1m);
		fprintf(stderr,"dbg5       par_p1t:         %d\n",data->par_p1t);
		fprintf(stderr,"dbg5       par_p1z:         %f\n",data->par_p1z);
		fprintf(stderr,"dbg5       par_p1x:         %f\n",data->par_p1x);
		fprintf(stderr,"dbg5       par_p1y:         %f\n",data->par_p1y);
		fprintf(stderr,"dbg5       par_p1d:         %f\n",data->par_p1d);
		fprintf(stderr,"dbg5       par_p1g:         %s\n",data->par_p1g);
		fprintf(stderr,"dbg5       par_p2m:         %d\n",data->par_p2m);
		fprintf(stderr,"dbg5       par_p2t:         %d\n",data->par_p2t);
		fprintf(stderr,"dbg5       par_p2z:         %f\n",data->par_p2z);
		fprintf(stderr,"dbg5       par_p2x:         %f\n",data->par_p2x);
		fprintf(stderr,"dbg5       par_p2y:         %f\n",data->par_p2y);
		fprintf(stderr,"dbg5       par_p2d:         %f\n",data->par_p2d);
		fprintf(stderr,"dbg5       par_p2g:         %s\n",data->par_p2g);
		fprintf(stderr,"dbg5       par_p3m:         %d\n",data->par_p3m);
		fprintf(stderr,"dbg5       par_p3t:         %d\n",data->par_p3t);
		fprintf(stderr,"dbg5       par_p3z:         %f\n",data->par_p3z);
		fprintf(stderr,"dbg5       par_p3x:         %f\n",data->par_p3x);
		fprintf(stderr,"dbg5       par_p3y:         %f\n",data->par_p3y);
		fprintf(stderr,"dbg5       par_p3d:         %f\n",data->par_p3d);
		fprintf(stderr,"dbg5       par_p3g:         %s\n",data->par_p3g);
		fprintf(stderr,"dbg5       par_msz:         %f\n",data->par_msz);
		fprintf(stderr,"dbg5       par_msx:         %f\n",data->par_msx);
		fprintf(stderr,"dbg5       par_msy:         %f\n",data->par_msy);
		fprintf(stderr,"dbg5       par_mrp:         %c%c\n",
			data->par_mrp[0],data->par_mrp[1]);
		fprintf(stderr,"dbg5       par_msd:         %f\n",data->par_msd);
		fprintf(stderr,"dbg5       par_msr:         %f\n",data->par_msr);
		fprintf(stderr,"dbg5       par_msp:         %f\n",data->par_msp);
		fprintf(stderr,"dbg5       par_msg:         %f\n",data->par_msg);
		fprintf(stderr,"dbg5       par_gcg:         %f\n",data->par_gcg);
		fprintf(stderr,"dbg5       par_cpr:         %s\n",data->par_cpr);
		fprintf(stderr,"dbg5       par_rop:         %s\n",data->par_rop);
		fprintf(stderr,"dbg5       par_sid:         %s\n",data->par_sid);
		fprintf(stderr,"dbg5       par_pll:         %s\n",data->par_pll);
		fprintf(stderr,"dbg5       par_com:         %s\n",data->par_com);
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
int mbr_em300mba_rd_run_parameter(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_run_parameter";
	int	status = MB_SUCCESS;
	char	line[EM2_RUN_PARAMETER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_RUN_PARAMETER;
	data->type = EM2_RUN_PARAMETER;
	data->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_RUN_PARAMETER_SIZE-4,mbfp);
	if (read_len == EM2_RUN_PARAMETER_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->run_date = (int) mb_swap_int(*int_ptr);
		if (data->run_date != 0) data->date = data->run_date;
		int_ptr = (int *) &line[4];
		data->run_msec = (int) mb_swap_int(*int_ptr);
		if (data->run_date != 0) data->msec = data->run_msec;
		short_ptr = (short *) &line[8];
		data->run_ping_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->run_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->run_status = (int) mb_swap_int(*int_ptr);
		data->run_mode = (mb_u_char) line[16];
		data->run_filter_id = (mb_u_char) line[17];
		short_ptr = (short *) &line[18];
		data->run_min_depth = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[20];
		data->run_max_depth = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		data->run_absorption = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[24];
		data->run_tran_pulse = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[26];
		data->run_tran_beam = (unsigned short) mb_swap_short(*short_ptr);
		data->run_tran_pow = (mb_u_char) line[28];
		data->run_rec_beam = (mb_u_char) line[29];
		data->run_rec_band = (mb_u_char) line[30];
		data->run_rec_gain = (mb_u_char) line[31];
		data->run_tvg_cross = (mb_u_char) line[32];
		data->run_ssv_source = (mb_u_char) line[33];
		short_ptr = (short *) &line[34];
		data->run_max_swath = (unsigned short) mb_swap_short(*short_ptr);
		data->run_beam_space = (mb_u_char) line[36];
		data->run_swath_angle = (mb_u_char) line[37];
		data->run_stab_mode = (mb_u_char) line[38];
		for (i=0;i<6;i++)
		    data->run_spare[i] = line[39+i];
#else
		int_ptr = (int *) &line[0];
		data->run_date = (int) *int_ptr;
		if (data->run_date != 0) data->date = data->run_date;
		int_ptr = (int *) &line[4];
		data->run_msec = (int) *int_ptr;
		if (data->run_date != 0) data->msec = data->run_msec;
		short_ptr = (short *) &line[8];
		data->run_ping_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->run_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->run_status = (int) *int_ptr;
		data->run_mode = (mb_u_char) line[16];
		data->run_filter_id = (mb_u_char) line[17];
		short_ptr = (short *) &line[18];
		data->run_min_depth = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[20];
		data->run_max_depth = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		data->run_absorption = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[24];
		data->run_tran_pulse = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[26];
		data->run_tran_beam = (unsigned short) *short_ptr;
		data->run_tran_pow = (mb_u_char) line[28];
		data->run_rec_beam = (mb_u_char) line[29];
		data->run_rec_band = (mb_u_char) line[30];
		data->run_rec_gain = (mb_u_char) line[31];
		data->run_tvg_cross = (mb_u_char) line[32];
		data->run_ssv_source = (mb_u_char) line[33];
		short_ptr = (short *) &line[34];
		data->run_max_swath = (unsigned short) *short_ptr;
		data->run_beam_space = (mb_u_char) line[36];
		data->run_swath_angle = (mb_u_char) line[37];
		data->run_stab_mode = (mb_u_char) line[38];
		for (i=0;i<6;i++)
		    data->run_spare[i] = line[39+i];
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       run_date:        %d\n",data->run_date);
		fprintf(stderr,"dbg5       run_msec:        %d\n",data->run_msec);
		fprintf(stderr,"dbg5       run_ping_count:  %d\n",data->run_ping_count);
		fprintf(stderr,"dbg5       run_serial:      %d\n",data->run_serial);
		fprintf(stderr,"dbg5       run_status:      %d\n",data->run_status);
		fprintf(stderr,"dbg5       run_mode:        %d\n",data->run_mode);
		fprintf(stderr,"dbg5       run_filter_id:   %d\n",data->run_filter_id);
		fprintf(stderr,"dbg5       run_min_depth:   %d\n",data->run_min_depth);
		fprintf(stderr,"dbg5       run_max_depth:   %d\n",data->run_max_depth);
		fprintf(stderr,"dbg5       run_absorption:  %d\n",data->run_absorption);
		fprintf(stderr,"dbg5       run_tran_pulse:  %d\n",data->run_tran_pulse);
		fprintf(stderr,"dbg5       run_tran_beam:   %d\n",data->run_tran_beam);
		fprintf(stderr,"dbg5       run_tran_pow:    %d\n",data->run_tran_pow);
		fprintf(stderr,"dbg5       run_rec_beam:    %d\n",data->run_rec_beam);
		fprintf(stderr,"dbg5       run_rec_band:    %d\n",data->run_rec_band);
		fprintf(stderr,"dbg5       run_rec_gain:    %d\n",data->run_rec_gain);
		fprintf(stderr,"dbg5       run_tvg_cross:   %d\n",data->run_tvg_cross);
		fprintf(stderr,"dbg5       run_ssv_source:  %d\n",data->run_ssv_source);
		fprintf(stderr,"dbg5       run_max_swath:   %d\n",data->run_max_swath);
		fprintf(stderr,"dbg5       run_beam_space:  %d\n",data->run_beam_space);
		fprintf(stderr,"dbg5       run_swath_angle: %d\n",data->run_swath_angle);
		fprintf(stderr,"dbg5       run_stab_mode:   %d\n",data->run_stab_mode);
		for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       run_spare[%d]:    %d\n",i,data->run_spare[i]);
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
int mbr_em300mba_rd_clock(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_clock";
	int	status = MB_SUCCESS;
	char	line[EM2_CLOCK_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_CLOCK;
	data->type = EM2_CLOCK;
	data->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_CLOCK_SIZE-4,mbfp);
	if (read_len == EM2_CLOCK_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->clk_date = (int) mb_swap_int(*int_ptr);
		data->date = data->clk_date;
		int_ptr = (int *) &line[4];
		data->clk_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->clk_msec;
		short_ptr = (short *) &line[8];
		data->clk_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->clk_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->clk_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->clk_origin_msec = (int) mb_swap_int(*int_ptr);
		data->clk_1_pps_use = (mb_u_char) line[20];
#else
		int_ptr = (int *) &line[0];
		data->clk_date = (int) *int_ptr;
		data->date = data->clk_date;
		int_ptr = (int *) &line[4];
		data->clk_msec = (int) *int_ptr;
		data->msec = data->clk_msec;
		short_ptr = (short *) &line[8];
		data->clk_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->clk_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->clk_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		data->clk_origin_msec = (int) *int_ptr;
		data->clk_1_pps_use = (mb_u_char) line[20];
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       clk_date:        %d\n",data->clk_date);
		fprintf(stderr,"dbg5       clk_msec:        %d\n",data->clk_msec);
		fprintf(stderr,"dbg5       clk_count:       %d\n",data->clk_count);
		fprintf(stderr,"dbg5       clk_serial:      %d\n",data->clk_serial);
		fprintf(stderr,"dbg5       clk_origin_date: %d\n",data->clk_origin_date);
		fprintf(stderr,"dbg5       clk_origin_msec: %d\n",data->clk_origin_msec);
		fprintf(stderr,"dbg5       clk_1_pps_use:   %d\n",data->clk_1_pps_use);
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
int mbr_em300mba_rd_tide(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_tide";
	int	status = MB_SUCCESS;
	char	line[EM2_TIDE_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_TIDE;
	data->type = EM2_TIDE;
	data->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_TIDE_SIZE-4,mbfp);
	if (read_len == EM2_TIDE_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->tid_date = (int) mb_swap_int(*int_ptr);
		data->date = data->tid_date;
		int_ptr = (int *) &line[4];
		data->tid_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->tid_msec;
		short_ptr = (short *) &line[8];
		data->tid_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->tid_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->tid_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->tid_origin_msec = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		data->tid_tide = mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->tid_date = (int) *int_ptr;
		data->date = data->tid_date;
		int_ptr = (int *) &line[4];
		data->tid_msec = (int) *int_ptr;
		data->msec = data->tid_msec;
		short_ptr = (short *) &line[8];
		data->tid_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->tid_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->tid_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		data->tid_origin_msec = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		data->tid_tide = *short_ptr;
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       tid_date:        %d\n",data->tid_date);
		fprintf(stderr,"dbg5       tid_msec:        %d\n",data->tid_msec);
		fprintf(stderr,"dbg5       tid_count:       %d\n",data->tid_count);
		fprintf(stderr,"dbg5       tid_serial:      %d\n",data->tid_serial);
		fprintf(stderr,"dbg5       tid_origin_date: %d\n",data->tid_origin_date);
		fprintf(stderr,"dbg5       tid_origin_msec: %d\n",data->tid_origin_msec);
		fprintf(stderr,"dbg5       tid_tide:        %d\n",data->tid_tide);
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
int mbr_em300mba_rd_height(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_height";
	int	status = MB_SUCCESS;
	char	line[EM2_HEIGHT_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_HEIGHT;
	data->type = EM2_HEIGHT;
	data->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_HEIGHT_SIZE-4,mbfp);
	if (read_len == EM2_HEIGHT_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->hgt_date = (int) mb_swap_int(*int_ptr);
		data->date = data->hgt_date;
		int_ptr = (int *) &line[4];
		data->hgt_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->hgt_msec;
		short_ptr = (short *) &line[8];
		data->hgt_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->hgt_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->hgt_height = (int) mb_swap_int(*int_ptr);
		data->hgt_type = (mb_u_char) line[16];
#else
		int_ptr = (int *) &line[0];
		data->hgt_date = (int) *int_ptr;
		data->date = data->hgt_date;
		int_ptr = (int *) &line[4];
		data->hgt_msec = (int) *int_ptr;
		data->msec = data->hgt_msec;
		short_ptr = (short *) &line[8];
		data->hgt_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->hgt_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->hgt_height = (int) *int_ptr;
		data->hgt_type = (mb_u_char) line[16];
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       hgt_date:        %d\n",data->hgt_date);
		fprintf(stderr,"dbg5       hgt_msec:        %d\n",data->hgt_msec);
		fprintf(stderr,"dbg5       hgt_count:       %d\n",data->hgt_count);
		fprintf(stderr,"dbg5       hgt_serial:      %d\n",data->hgt_serial);
		fprintf(stderr,"dbg5       hgt_height:      %d\n",data->hgt_height);
		fprintf(stderr,"dbg5       hgt_type:        %d\n",data->hgt_type);
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
int mbr_em300mba_rd_heading(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_heading";
	int	status = MB_SUCCESS;
	char	line[16];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_HEADING;
	data->type = EM2_HEADING;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_HEADING_HEADER_SIZE,mbfp);
	if (read_len == EM2_HEADING_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->hed_date = (int) mb_swap_int(*int_ptr);
		data->date = data->hed_date;
		int_ptr = (int *) &line[4];
		data->hed_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->hed_msec;
		short_ptr = (short *) &line[8];
		data->hed_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->hed_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		data->hed_ndata = (int) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->hed_date = (int) *int_ptr;
		data->date = data->hed_date;
		int_ptr = (int *) &line[4];
		data->hed_msec = (int) *int_ptr;
		data->msec = data->hed_msec;
		short_ptr = (short *) &line[8];
		data->hed_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->hed_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		data->hed_ndata = (int) *short_ptr;
#endif
		}

	/* read binary heading values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<data->hed_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_HEADING_SLICE_SIZE,mbfp);
		if (read_len == EM2_HEADING_SLICE_SIZE 
			&& i < MBF_EM300MBA_MAXHEADING)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			data->hed_time[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			data->hed_heading[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			data->hed_time[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			data->hed_heading[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    data->hed_ndata = MIN(data->hed_ndata, MBF_EM300MBA_MAXHEADING);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			data->hed_heading_status = (mb_u_char) line[0];
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       hed_date:        %d\n",data->hed_date);
		fprintf(stderr,"dbg5       hed_msec:        %d\n",data->hed_msec);
		fprintf(stderr,"dbg5       hed_count:       %d\n",data->hed_count);
		fprintf(stderr,"dbg5       hed_serial:      %d\n",data->hed_serial);
		fprintf(stderr,"dbg5       hed_ndata:       %d\n",data->hed_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<data->hed_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, data->hed_time[i], data->hed_heading[i]);
		fprintf(stderr,"dbg5       hed_heading_status: %d\n",data->hed_heading_status);
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
int mbr_em300mba_rd_attitude(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_attitude";
	int	status = MB_SUCCESS;
	char	line[16];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_ATTITUDE;
	data->type = EM2_ATTITUDE;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_ATTITUDE_HEADER_SIZE,mbfp);
	if (read_len == EM2_ATTITUDE_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->att_date = (int) mb_swap_int(*int_ptr);
		data->date = data->att_date;
		int_ptr = (int *) &line[4];
		data->att_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->att_msec;
		short_ptr = (short *) &line[8];
		data->att_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->att_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		data->att_ndata = (int) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->att_date = (int) *int_ptr;
		data->date = data->att_date;
		int_ptr = (int *) &line[4];
		data->att_msec = (int) *int_ptr;
		data->msec = data->att_msec;
		short_ptr = (short *) &line[8];
		data->att_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->att_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		data->att_ndata = (int) *short_ptr;
#endif
		}

	/* read binary attitude values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<data->att_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_ATTITUDE_SLICE_SIZE,mbfp);
		if (read_len == EM2_ATTITUDE_SLICE_SIZE 
			&& i < MBF_EM300MBA_MAXATTITUDE)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			data->att_time[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			data->att_sensor_status[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[4];
			data->att_roll[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[6];
			data->att_pitch[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[8];
			data->att_heave[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[10];
			data->att_heading[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			data->att_time[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			data->att_sensor_status[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[4];
			data->att_roll[i] = (short) *short_ptr;
			short_ptr = (short *) &line[6];
			data->att_pitch[i] = (short) *short_ptr;
			short_ptr = (short *) &line[8];
			data->att_heave[i] = (short) *short_ptr;
			short_ptr = (short *) &line[10];
			data->att_heading[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    data->att_ndata = MIN(data->att_ndata, MBF_EM300MBA_MAXATTITUDE);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			data->att_heading_status = (mb_u_char) line[0];
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       att_date:        %d\n",data->att_date);
		fprintf(stderr,"dbg5       att_msec:        %d\n",data->att_msec);
		fprintf(stderr,"dbg5       att_count:       %d\n",data->att_count);
		fprintf(stderr,"dbg5       att_serial:      %d\n",data->att_serial);
		fprintf(stderr,"dbg5       att_ndata:       %d\n",data->att_ndata);
		fprintf(stderr,"dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr,"dbg5       -------------------------------------\n");
		for (i=0;i<data->att_ndata;i++)
			fprintf(stderr,"dbg5        %3d  %d  %d %d %d %d\n",
				i, data->att_time[i], data->att_roll[i], 
				data->att_pitch[i], data->att_heave[i], 
				data->att_heading[i]);
		fprintf(stderr,"dbg5       att_heading_status: %d\n",data->att_heading_status);
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
int mbr_em300mba_rd_pos(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_pos";
	int	status = MB_SUCCESS;
	char	line[256];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_NAV;
	data->type = EM2_POS;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_POS_HEADER_SIZE,mbfp);
	if (read_len == EM2_POS_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->pos_date = (int) mb_swap_int(*int_ptr);
		data->date = data->pos_date;
		int_ptr = (int *) &line[4];
		data->pos_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->pos_msec;
		short_ptr = (short *) &line[8];
		data->pos_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->pos_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->pos_latitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->pos_longitude = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		data->pos_quality = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		data->pos_speed = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[24];
		data->pos_course = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[26];
		data->pos_heading = (unsigned short) mb_swap_short(*short_ptr);
		data->pos_system = (mb_u_char) line[28];
		data->pos_input_size = (mb_u_char) line[29];
#else
		int_ptr = (int *) &line[0];
		data->pos_date = (int) *int_ptr;
		data->date = data->pos_date;
		int_ptr = (int *) &line[4];
		data->pos_msec = (int) *int_ptr;
		data->msec = data->pos_msec;
		short_ptr = (short *) &line[8];
		data->pos_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->pos_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->pos_latitude = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		data->pos_longitude = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		data->pos_quality = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		data->pos_speed = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[24];
		data->pos_course = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[26];
		data->pos_heading = (unsigned short) *short_ptr;
		data->pos_system = (mb_u_char) line[28];
		data->pos_input_size = (mb_u_char) line[29];
#endif
		}

	/* read input position string */
	if (status == MB_SUCCESS && data->pos_input_size < 256)
		{
		read_len = fread(data->pos_input,1,data->pos_input_size,mbfp);
		if (read_len == data->pos_input_size)
			{
			status = MB_SUCCESS;
			data->pos_input[data->pos_input_size] = '\0';
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM2_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[0],2,1,mbfp);
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       pos_date:        %d\n",data->pos_date);
		fprintf(stderr,"dbg5       pos_msec:        %d\n",data->pos_msec);
		fprintf(stderr,"dbg5       pos_count:       %d\n",data->pos_count);
		fprintf(stderr,"dbg5       pos_serial:      %d\n",data->pos_serial);
		fprintf(stderr,"dbg5       pos_latitude:    %d\n",data->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:   %d\n",data->pos_longitude);
		fprintf(stderr,"dbg5       pos_quality:     %d\n",data->pos_quality);
		fprintf(stderr,"dbg5       pos_speed:       %d\n",data->pos_speed);
		fprintf(stderr,"dbg5       pos_course:      %d\n",data->pos_course);
		fprintf(stderr,"dbg5       pos_heading:     %d\n",data->pos_heading);
		fprintf(stderr,"dbg5       pos_system:      %d\n",data->pos_system);
		fprintf(stderr,"dbg5       pos_input_size:  %d\n",data->pos_input_size);
		fprintf(stderr,"dbg5       pos_input:\ndbg5            %s\n",data->pos_input);
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
int mbr_em300mba_rd_svp(verbose,mbfp,data,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_svp";
	int	status = MB_SUCCESS;
	char	line[256];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_VELOCITY_PROFILE;
	data->type = EM2_SVP;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SVP_HEADER_SIZE,mbfp);
	if (read_len == EM2_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->svp_use_date = (int) mb_swap_int(*int_ptr);
		data->date = data->svp_use_date;
		int_ptr = (int *) &line[4];
		data->svp_use_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->svp_use_msec;
		short_ptr = (short *) &line[8];
		data->svp_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->svp_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->svp_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->svp_origin_msec = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		data->svp_num = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		data->svp_depth_res = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->svp_use_date = (int) *int_ptr;
		data->date = data->svp_use_date;
		int_ptr = (int *) &line[4];
		data->svp_use_msec = (int) *int_ptr;
		data->msec = data->svp_use_msec;
		short_ptr = (short *) &line[8];
		data->svp_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->svp_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->svp_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		data->svp_origin_msec = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		data->svp_num = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		data->svp_depth_res = (unsigned short) *short_ptr;
#endif
		}

	/* read binary svp values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<data->svp_num && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,4,mbfp);
		if (read_len != 4)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (i < MBF_EM300MBA_MAXSVP)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			data->svp_depth[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			data->svp_vel[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			data->svp_depth[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			data->svp_vel[i] = (unsigned short) *short_ptr;
#endif
			}
		}
	    data->svp_num = MIN(data->svp_num, MBF_EM300MBA_MAXSVP);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",data->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",data->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",data->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",data->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",data->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",data->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",data->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",data->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, data->svp_depth[i], data->svp_vel[i]);
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
int mbr_em300mba_rd_bath(verbose,mbfp,data,match,sonar,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
int	*match;
short	sonar;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_bath";
	int	status = MB_SUCCESS;
	char	line[EM2_BATH_MBA_HEADER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_DATA;
	data->type = EM2_BATH_MBA;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_BATH_MBA_HEADER_SIZE,mbfp);
	if (read_len == EM2_BATH_MBA_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->png_date = (int) mb_swap_int(*int_ptr);
		data->date = data->png_date;
		int_ptr = (int *) &line[4];
		data->png_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->png_msec;
		short_ptr = (short *) &line[8];
		data->png_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->png_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		data->png_latitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->png_longitude = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		data->png_speed = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		data->png_heading = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[24];
		data->png_ssv = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[26];
		data->png_xducer_depth = (unsigned short) mb_swap_short(*short_ptr);
		data->png_nbeams_max = (mb_u_char) line[28];
		data->png_nbeams = (mb_u_char) line[29];
		data->png_depth_res = (mb_u_char) line[30];
		data->png_distance_res = (mb_u_char) line[31];
		short_ptr = (short *) &line[32];
		data->png_sample_rate = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->png_date = (int) *int_ptr;
		data->date = data->png_date;
		int_ptr = (int *) &line[4];
		data->png_msec = (int) *int_ptr;
		data->msec = data->png_msec;
		short_ptr = (short *) &line[8];
		data->png_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->png_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		data->png_latitude = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		data->png_longitude = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		data->png_speed = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		data->png_heading = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[24];
		data->png_ssv = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[26];
		data->png_xducer_depth = (unsigned short) *short_ptr;
		data->png_nbeams_max = (mb_u_char) line[28];
		data->png_nbeams = (mb_u_char) line[29];
		data->png_depth_res = (mb_u_char) line[30];
		data->png_distance_res = (mb_u_char) line[31];
		short_ptr = (short *) &line[32];
		data->png_sample_rate = (unsigned short) *short_ptr;
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (data->png_nbeams > data->png_nbeams_max
			|| data->png_nbeams < 0
			|| data->png_nbeams_max < 0
			|| data->png_nbeams > MBF_EM300MBA_MAXBEAMS
			|| data->png_nbeams_max > MBF_EM300MBA_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<data->png_nbeams && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_BATH_MBA_BEAM_SIZE,mbfp);
		if (read_len == EM2_BATH_MBA_BEAM_SIZE 
			&& i < MBF_EM300MBA_MAXBEAMS)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			if (data->sonar == 300)
			    data->png_depth[i] = (unsigned short) mb_swap_short(*short_ptr);
			else
			    data->png_depth[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			data->png_acrosstrack[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[4];
			data->png_alongtrack[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[6];
			data->png_depression[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[8];
			data->png_azimuth[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[10];
			data->png_range[i] = (unsigned short) mb_swap_short(*short_ptr);
			data->png_quality[i] = (mb_u_char) line[12];
			data->png_window[i] = (mb_u_char) line[13];
			short_ptr = (short *) &line[14];
			data->png_amp[i] = (short) mb_swap_short(*short_ptr);
			data->png_beam_num[i] = (mb_u_char) line[16];
			data->png_beamflag[i] = (char) line[17];
#else
			short_ptr = (short *) &line[0];
			if (data->sonar == 300)
			    data->png_depth[i] = (unsigned short) *short_ptr;
			else
			    data->png_depth[i] = (short) *short_ptr;
			short_ptr = (short *) &line[2];
			data->png_acrosstrack[i] = (short) *short_ptr;
			short_ptr = (short *) &line[4];
			data->png_alongtrack[i] = (short) *short_ptr;
			short_ptr = (short *) &line[6];
			data->png_depression[i] = (short) *short_ptr;
			short_ptr = (short *) &line[8];
			data->png_azimuth[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[10];
			data->png_range[i] = (unsigned short) *short_ptr;
			data->png_quality[i] = (mb_u_char) line[12];
			data->png_window[i] = (mb_u_char) line[13];
			short_ptr = (short *) &line[14];
			data->png_amp[i] = (short) *short_ptr;
			data->png_beam_num[i] = (mb_u_char) line[16];
			data->png_beamflag[i] = (char) line[17];
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			data->png_offset_multiplier = (mb_u_char) line[0];
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (data->png_beam_num[0] > data->png_nbeams_max)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		for (i=1;i<data->png_nbeams;i++)
			{
			if (data->png_beam_num[i] < data->png_beam_num[i-1]
				|| data->png_beam_num[i] > data->png_nbeams_max)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			}
		}
		
	/* check if bath and sidescan time tags agree 
	   - we cannot pair bath 
	   and sidescan records from different pings */
	if (status == MB_SUCCESS)
		{
		if (data->png_date == data->png_ss_date
		    && data->png_msec == data->png_ss_msec)
		    *match = MB_YES;
		else
		    *match = MB_NO;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",data->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",data->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",data->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",data->png_serial);
		fprintf(stderr,"dbg5       png_latitude:    %d\n",data->png_latitude);
		fprintf(stderr,"dbg5       png_longitude:   %d\n",data->png_longitude);
		fprintf(stderr,"dbg5       png_speed:       %d\n",data->png_speed);
		fprintf(stderr,"dbg5       png_heading:     %d\n",data->png_heading);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",data->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %d\n",data->png_xducer_depth);
		fprintf(stderr,"dbg5       png_offset_multiplier: %d\n",data->png_offset_multiplier);
		fprintf(stderr,"dbg5       png_nbeams_max:        %d\n",data->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",data->png_nbeams);
		fprintf(stderr,"dbg5       png_depth_res:         %d\n",data->png_depth_res);
		fprintf(stderr,"dbg5       png_distance_res:      %d\n",data->png_distance_res);
		fprintf(stderr,"dbg5       png_sample_rate:       %d\n",data->png_sample_rate);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd  amp num flag\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<data->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %4d %3d %4d\n",
				i, data->png_depth[i], data->png_acrosstrack[i], 
				data->png_alongtrack[i], data->png_depression[i], 
				data->png_azimuth[i], data->png_range[i], 
				data->png_quality[i], data->png_window[i], 
				data->png_amp[i], data->png_beam_num[i], 
				data->png_beamflag[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       match:      %d\n",*match);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_rd_ss(verbose,mbfp,data,sonar,match,error)
int	verbose;
FILE	*mbfp;
struct mbf_em300mba_struct *data;
short	sonar;
int	*match;
int	*error;
{
	char	*function_name = "mbr_em300mba_rd_ss";
	int	status = MB_SUCCESS;
	char	line[2*MBF_EM300MBA_MAXPIXELS];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, read_size, len;
	int	done;
	int	junk_bytes;
	int	offset;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	data->kind = MB_DATA_DATA;
	data->type = EM2_SS_MBA;
	data->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SS_MBA_HEADER_SIZE,mbfp);
	if (read_len == EM2_SS_MBA_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		data->png_ss_date = (int) mb_swap_int(*int_ptr);
		data->date = data->png_ss_date;
		int_ptr = (int *) &line[4];
		data->png_ss_msec = (int) mb_swap_int(*int_ptr);
		data->msec = data->png_ss_msec;
		short_ptr = (short *) &line[8];
		data->png_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		data->png_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		data->png_max_range = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[14];
		data->png_r_zero = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[16];
		data->png_r_zero_corr = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[18];
		data->png_tvg_start = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[20];
		data->png_tvg_stop = (unsigned short) mb_swap_short(*short_ptr);
		data->png_bsn = (mb_s_char) line[22];
		data->png_bso = (mb_s_char) line[23];
		short_ptr = (short *) &line[24];
		data->png_tx = (unsigned short) mb_swap_short(*short_ptr);
		data->png_tvg_crossover = (mb_u_char) line[26];
		data->png_nbeams_ss = (mb_u_char) line[27];
		short_ptr = (short *) &line[28];
		data->png_pixel_size = (short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[30];
		data->png_pixels_ss = (short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		data->png_ss_date = (int) *int_ptr;
		data->date = data->png_ss_date;
		int_ptr = (int *) &line[4];
		data->png_ss_msec = (int) *int_ptr;
		data->msec = data->png_ss_msec;
		short_ptr = (short *) &line[8];
		data->png_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		data->png_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		data->png_max_range = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[14];
		data->png_r_zero = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[16];
		data->png_r_zero_corr = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[18];
		data->png_tvg_start = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[20];
		data->png_tvg_stop = (unsigned short) *short_ptr;
		data->png_bsn = (mb_s_char) line[22];
		data->png_bso = (mb_s_char) line[23];
		short_ptr = (short *) &line[24];
		data->png_tx = (unsigned short) *short_ptr;
		data->png_tvg_crossover = (mb_u_char) line[26];
		data->png_nbeams_ss = (mb_u_char) line[27];
		short_ptr = (short *) &line[28];
		data->png_pixel_size = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[30];
		data->png_pixels_ss = (unsigned short) *short_ptr;
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (data->png_nbeams_ss < 0
			|| data->png_nbeams_ss > MBF_EM300MBA_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    data->png_npixels = 0;
	    for (i=0;i<data->png_nbeams_ss && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_SS_MBA_BEAM_SIZE,mbfp);
		if (read_len == EM2_SS_MBA_BEAM_SIZE 
			&& i < MBF_EM300MBA_MAXBEAMS)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			data->png_beam_index[i] = (mb_u_char) line[0];
			data->png_sort_direction[i] = (mb_s_char) line[1];
			short_ptr = (short *) &line[2];
			data->png_beam_samples[i] = (unsigned short) mb_swap_short(*short_ptr);
			data->png_start_sample[i] = data->png_npixels;
			short_ptr = (short *) &line[4];
			data->png_center_sample[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			data->png_beam_index[i] = (mb_u_char) line[0];
			data->png_sort_direction[i] = (mb_s_char) line[1];
			short_ptr = (short *) &line[2];
			data->png_beam_samples[i] = (unsigned short) *short_ptr;
			data->png_start_sample[i] = data->png_npixels;
			short_ptr = (short *) &line[4];
			data->png_center_sample[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		data->png_npixels += data->png_beam_samples[i];
		if (data->png_npixels > MBF_EM300MBA_MAXRAWPIXELS)
			{
			data->png_beam_samples[i] 
				-= (data->png_npixels 
					- MBF_EM300MBA_MAXRAWPIXELS);
			if (data->png_beam_samples[i] < 0)
				data->png_beam_samples[i] = 0;
			}
		}
	    if (data->png_npixels > MBF_EM300MBA_MAXRAWPIXELS)
		{
fprintf(stderr, "WARNING: EM300/3000 sidescan pixels %d exceed maximum %d!\n", 
data->png_npixels, MBF_EM300MBA_MAXRAWPIXELS);
		junk_bytes = data->png_npixels - MBF_EM300MBA_MAXRAWPIXELS;
		data->png_npixels = MBF_EM300MBA_MAXRAWPIXELS;
		}
	    else
		junk_bytes = 0;
	    }
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (data->png_beam_index[0] > MBF_EM300MBA_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		for (i=1;i<data->png_nbeams_ss;i++)
			{
			if (data->png_beam_index[i] < data->png_beam_index[i-1]
				|| data->png_beam_index[0] > MBF_EM300MBA_MAXBEAMS)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			}
		}

	/* read binary sidescan values */
	if (status == MB_SUCCESS)
		{
		read_size = data->png_npixels + 1 - (data->png_npixels % 2);
		read_len = fread(data->png_ssraw,1,read_size, mbfp);
		if (read_len == read_size)
			{
			status = MB_SUCCESS;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* read any leftover binary sidescan values */
	if (status == MB_SUCCESS && junk_bytes > 0)
		{
		for (i=0;i<junk_bytes;i++)
		    read_len = fread(&line[0],1,1,mbfp);
		}
		
	/* read processed sidescan data */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			{
			data->png_ss[i] = 0;
			}
		read_len = fread(line,1,
				    data->png_pixels_ss * sizeof(short),mbfp);
		if (read_len == data->png_pixels_ss * sizeof(short))
			status = MB_SUCCESS;
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* get processed sidescan data */
	if (status == MB_SUCCESS)
		{
		offset = (MBF_EM300MBA_MAXPIXELS - data->png_pixels_ss) / 2;
		for (i=0;i<data->png_pixels_ss;i++)
		    {
#ifdef BYTESWAPPED
		    short_ptr = (short *) &line[2*i];
		    data->png_ss[offset+i] = (short) mb_swap_short(*short_ptr);
#else
		    short_ptr = (short *) &line[2*i];
		    data->png_ss[offset+i] = (short) *short_ptr;
#endif
		    }
		}
		
	/* read processed sidescan alongtrack data */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			{
			data->png_ssalongtrack[i] = 0;
			}
		read_len = fread(line,1,
				    data->png_pixels_ss * sizeof(short),mbfp);
		if (read_len == data->png_pixels_ss * sizeof(short))
			status = MB_SUCCESS;
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* get processed sidescan alongtrack data */
	if (status == MB_SUCCESS)
		{
		offset = (MBF_EM300MBA_MAXPIXELS - data->png_pixels_ss) / 2;
		for (i=0;i<data->png_pixels_ss;i++)
		    {
#ifdef BYTESWAPPED
		    short_ptr = (short *) &line[2*i];
		    data->png_ssalongtrack[offset+i] = (short) mb_swap_short(*short_ptr);
#else
		    short_ptr = (short *) &line[2*i];
		    data->png_ssalongtrack[offset+i] = (short) *short_ptr;
#endif
		    }
		}
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM2_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[0],2,1,mbfp);
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
	    }
		
	/* check if bath and sidescan time tags agree 
	   - we cannot pair bath 
	   and sidescan records from different pings */
	if (status == MB_SUCCESS)
		{
		if (data->png_date == data->png_ss_date
		    && data->png_msec == data->png_ss_msec)
		    *match = MB_YES;
		else
		    *match = MB_NO;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",data->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",data->png_msec);
		fprintf(stderr,"dbg5       png_ss_date:     %d\n",data->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:     %d\n",data->png_ss_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",data->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",data->png_serial);
		fprintf(stderr,"dbg5       png_max_range:   %d\n",data->png_max_range);
		fprintf(stderr,"dbg5       png_r_zero:      %d\n",data->png_r_zero);
		fprintf(stderr,"dbg5       png_r_zero_corr: %d\n",data->png_r_zero_corr);
		fprintf(stderr,"dbg5       png_tvg_start:   %d\n",data->png_tvg_start);
		fprintf(stderr,"dbg5       png_tvg_stop:    %d\n",data->png_tvg_stop);
		fprintf(stderr,"dbg5       png_bsn:         %d\n",data->png_bsn);
		fprintf(stderr,"dbg5       png_bso:         %d\n",data->png_bso);
		fprintf(stderr,"dbg5       png_tx:          %d\n",data->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover: %d\n",data->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:     %d\n",data->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:       %d\n",data->png_npixels);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<data->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d\n",
				i, data->png_beam_index[i], data->png_sort_direction[i], 
				data->png_beam_samples[i], data->png_start_sample[i], 
				data->png_center_sample[i]);
		fprintf(stderr,"dbg5       cnt  ssraw\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<data->png_npixels;i++)
			fprintf(stderr,"dbg5        %d %d\n",
				i, data->png_ssraw[i]);
		fprintf(stderr,"dbg5       png_pixel_size:    %d\n",data->png_pixel_size);
		fprintf(stderr,"dbg5       png_pixels_ss:     %d\n",data->png_pixels_ss);
		fprintf(stderr,"dbg5       pixel  ss  ltrack\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			fprintf(stderr,"dbg5        %4d %6d %6d\n",
				i, data->png_ss[i], data->png_ssalongtrack[i]);
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       match:      %d\n",*match);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em300mba_struct *data;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_COMMENT
		|| data->kind == MB_DATA_START
		|| data->kind == MB_DATA_STOP)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_start kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_start(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_RUN_PARAMETER)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_run_parameter kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_run_parameter(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_CLOCK)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_clock kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_clock(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_TIDE)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_tide kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_tide(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_HEIGHT)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_height kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_height(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_HEADING)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_heading kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_heading(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_ATTITUDE)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_attitude kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_attitude(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_pos kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_pos(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_svp kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_svp(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
	/*fprintf(stderr,"call mbr_em300mba_wr_bath kind:%d type %x\n",data->kind,data->type);*/
		status = mbr_em300mba_wr_bath(verbose,mbfp,data,error);
		if (data->png_nbeams_ss > 0)
		    {
	/*fprintf(stderr,"call mbr_em300mba_wr_ss kind:%d type %x\n",data->kind,data->type);*/
		    status = mbr_em300mba_wr_ss(verbose,mbfp,data,error);
		    }
		}
	else
		{
	/*fprintf(stderr,"call nothing bad kind: %d type %x\n", data->kind, data->type);*/
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
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
int mbr_em300mba_wr_start(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_start";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[MBF_EM300MBA_BUFFER_SIZE], *buff;
	int	buff_len, write_len;
	short	*label;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	char	*comma_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       par_date:        %d\n",data->par_date);
		fprintf(stderr,"dbg5       par_msec:        %d\n",data->par_msec);
		fprintf(stderr,"dbg5       par_line_num:    %d\n",data->par_line_num);
		fprintf(stderr,"dbg5       par_serial_1:    %d\n",data->par_serial_1);
		fprintf(stderr,"dbg5       par_serial_2:    %d\n",data->par_serial_2);
		fprintf(stderr,"dbg5       par_wlz:         %f\n",data->par_wlz);
		fprintf(stderr,"dbg5       par_smh:         %d\n",data->par_smh);
		fprintf(stderr,"dbg5       par_s1z:         %f\n",data->par_s1z);
		fprintf(stderr,"dbg5       par_s1x:         %f\n",data->par_s1x);
		fprintf(stderr,"dbg5       par_s1y:         %f\n",data->par_s1y);
		fprintf(stderr,"dbg5       par_s1h:         %f\n",data->par_s1h);
		fprintf(stderr,"dbg5       par_s1r:         %f\n",data->par_s1r);
		fprintf(stderr,"dbg5       par_s1p:         %f\n",data->par_s1p);
		fprintf(stderr,"dbg5       par_s1n:         %d\n",data->par_s1n);
		fprintf(stderr,"dbg5       par_s2z:         %f\n",data->par_s2z);
		fprintf(stderr,"dbg5       par_s2x:         %f\n",data->par_s2x);
		fprintf(stderr,"dbg5       par_s2y:         %f\n",data->par_s2y);
		fprintf(stderr,"dbg5       par_s2h:         %f\n",data->par_s2h);
		fprintf(stderr,"dbg5       par_s2r:         %f\n",data->par_s2r);
		fprintf(stderr,"dbg5       par_s2p:         %f\n",data->par_s2p);
		fprintf(stderr,"dbg5       par_s2n:         %d\n",data->par_s2n);
		fprintf(stderr,"dbg5       par_go1:         %f\n",data->par_go1);
		fprintf(stderr,"dbg5       par_go2:         %f\n",data->par_go2);
		fprintf(stderr,"dbg5       par_tsv:         %s\n",data->par_tsv);
		fprintf(stderr,"dbg5       par_rsv:         %s\n",data->par_rsv);
		fprintf(stderr,"dbg5       par_bsv:         %s\n",data->par_bsv);
		fprintf(stderr,"dbg5       par_psv:         %s\n",data->par_psv);
		fprintf(stderr,"dbg5       par_osv:         %s\n",data->par_osv);
		fprintf(stderr,"dbg5       par_dsd:         %f\n",data->par_dsd);
		fprintf(stderr,"dbg5       par_dso:         %f\n",data->par_dso);
		fprintf(stderr,"dbg5       par_dsf:         %f\n",data->par_dsf);
		fprintf(stderr,"dbg5       par_dsh:         %c%c\n",
			data->par_dsh[0],data->par_dsh[1]);
		fprintf(stderr,"dbg5       par_aps:         %d\n",data->par_aps);
		fprintf(stderr,"dbg5       par_p1m:         %d\n",data->par_p1m);
		fprintf(stderr,"dbg5       par_p1t:         %d\n",data->par_p1t);
		fprintf(stderr,"dbg5       par_p1z:         %f\n",data->par_p1z);
		fprintf(stderr,"dbg5       par_p1x:         %f\n",data->par_p1x);
		fprintf(stderr,"dbg5       par_p1y:         %f\n",data->par_p1y);
		fprintf(stderr,"dbg5       par_p1d:         %f\n",data->par_p1d);
		fprintf(stderr,"dbg5       par_p1g:         %s\n",data->par_p1g);
		fprintf(stderr,"dbg5       par_p2m:         %d\n",data->par_p2m);
		fprintf(stderr,"dbg5       par_p2t:         %d\n",data->par_p2t);
		fprintf(stderr,"dbg5       par_p2z:         %f\n",data->par_p2z);
		fprintf(stderr,"dbg5       par_p2x:         %f\n",data->par_p2x);
		fprintf(stderr,"dbg5       par_p2y:         %f\n",data->par_p2y);
		fprintf(stderr,"dbg5       par_p2d:         %f\n",data->par_p2d);
		fprintf(stderr,"dbg5       par_p2g:         %s\n",data->par_p2g);
		fprintf(stderr,"dbg5       par_p3m:         %d\n",data->par_p3m);
		fprintf(stderr,"dbg5       par_p3t:         %d\n",data->par_p3t);
		fprintf(stderr,"dbg5       par_p3z:         %f\n",data->par_p3z);
		fprintf(stderr,"dbg5       par_p3x:         %f\n",data->par_p3x);
		fprintf(stderr,"dbg5       par_p3y:         %f\n",data->par_p3y);
		fprintf(stderr,"dbg5       par_p3d:         %f\n",data->par_p3d);
		fprintf(stderr,"dbg5       par_p3g:         %s\n",data->par_p3g);
		fprintf(stderr,"dbg5       par_msz:         %f\n",data->par_msz);
		fprintf(stderr,"dbg5       par_msx:         %f\n",data->par_msx);
		fprintf(stderr,"dbg5       par_msy:         %f\n",data->par_msy);
		fprintf(stderr,"dbg5       par_mrp:         %c%c\n",
			data->par_mrp[0],data->par_mrp[1]);
		fprintf(stderr,"dbg5       par_msd:         %f\n",data->par_msd);
		fprintf(stderr,"dbg5       par_msr:         %f\n",data->par_msr);
		fprintf(stderr,"dbg5       par_msp:         %f\n",data->par_msp);
		fprintf(stderr,"dbg5       par_msg:         %f\n",data->par_msg);
		fprintf(stderr,"dbg5       par_gcg:         %f\n",data->par_gcg);
		fprintf(stderr,"dbg5       par_cpr:         %s\n",data->par_cpr);
		fprintf(stderr,"dbg5       par_rop:         %s\n",data->par_rop);
		fprintf(stderr,"dbg5       par_sid:         %s\n",data->par_sid);
		fprintf(stderr,"dbg5       par_pll:         %s\n",data->par_pll);
		fprintf(stderr,"dbg5       par_com:         %s\n",data->par_com);
		}
		
	/* zero checksum */
	checksum = 0;
		
	/* if data type not set - use start */
	if (data->type == EM2_NONE)
	    data->type = EM2_START;
	    
	/* if sonar not set use EM300 */
	if (data->sonar == 0)
	    data->sonar = 300;
		
	/* set up start of output buffer - we handle this
	   record differently because of the ascii data */
	memset(line, 0, MBF_EM300MBA_BUFFER_SIZE);
	label = (short *) &line[4];
	*label = data->type;
#ifdef BYTESWAPPED
	*label = (short) mb_swap_short(*label);
#endif

	/* put binary header data into buffer */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[6];
		*short_ptr = (unsigned short) mb_swap_short(data->sonar);
		int_ptr = (int *) &line[8];
		*int_ptr = (int) mb_swap_int(data->par_date);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->par_msec);
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) mb_swap_short(data->par_line_num);
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(data->par_serial_1);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->par_serial_2);
#else
		short_ptr = (short *) &line[6];
		*short_ptr = (unsigned short) data->sonar;
		int_ptr = (int *) &line[8];
		*int_ptr = (int) data->par_date;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->par_msec;
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) data->par_line_num;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) data->par_serial_1;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->par_serial_2;
#endif
		}
		
	/* construct ASCII parameter buffer */
	buff = &line[22];
	sprintf(&buff[0], "WLZ=%.2f,", data->par_wlz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "SMH=%d,", data->par_smh);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Z=%.2f,", data->par_s1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1X=%.2f,", data->par_s1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Y=%.2f,", data->par_s1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1H=%.2f,", data->par_s1h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1R=%.2f,", data->par_s1r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1P=%.2f,", data->par_s1p);
	buff_len = strlen(buff);
	if (data->par_s1n > 0)
	    {
	    sprintf(&buff[buff_len], "S1N=%d,", data->par_s1n);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "S2Z=%.2f,", data->par_s2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2X=%.2f,", data->par_s2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2Y=%.2f,", data->par_s2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2H=%.2f,", data->par_s2h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2R=%.2f,", data->par_s2r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2P=%.2f,", data->par_s2p);
	buff_len = strlen(buff);
	if (data->par_s2n > 0)
	    {
	    sprintf(&buff[buff_len], "S2N=%d,", data->par_s2n);
	    buff_len = strlen(buff);
	    }
	if (data->par_go1 != 0.0)
	    {
	    sprintf(&buff[buff_len], "GO1=%.2f,", data->par_go1);
	    buff_len = strlen(buff);
	    }
	if (data->par_go2 != 0.0)
	    {
	    sprintf(&buff[buff_len], "GO2=%.2f,", data->par_go2);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "TSV=%s,", data->par_tsv);
	buff_len = strlen(buff);
	if (strlen(data->par_rsv) > 0)
	    {
	    sprintf(&buff[buff_len], "RSV=%s,", data->par_rsv);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "BSV=%s,", data->par_bsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "PSV=%s,", data->par_tsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "OSV=%s,", data->par_osv);
	buff_len = strlen(buff);
	if (data->par_dsd != 0.0)
	    {
	    sprintf(&buff[buff_len], "DSD=%.1f,", data->par_dsd);
	    buff_len = strlen(buff);
	    }
	else
	    {
	    sprintf(&buff[buff_len], "DSD=,");
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "DSO=%.6f,", data->par_dso);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSF=%.6f,", data->par_dsf);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSH=%c%c,", 
		data->par_dsh[0], data->par_dsh[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "APS=%d,",data->par_aps);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1M=%d,",data->par_p1m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1T=%d,",data->par_p1t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Z=%.2f,", data->par_p1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1X=%.2f,", data->par_p1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Y=%.2f,", data->par_p1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1D=%.1f,", data->par_p1d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1G=%s,", data->par_p1g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2M=%d,",data->par_p2m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2T=%d,",data->par_p2t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Z=%.2f,", data->par_p2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2X=%.2f,", data->par_p2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Y=%.2f,", data->par_p2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2D=%.1f,", data->par_p2d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2G=%s,", data->par_p2g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3M=%d,",data->par_p3m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3T=%d,",data->par_p3t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Z=%.2f,", data->par_p3z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3X=%.2f,", data->par_p3x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Y=%.2f,", data->par_p3y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3D=%.1f,", data->par_p3d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3G=%s,", data->par_p3g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSZ=%.2f,", data->par_msz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSX=%.2f,", data->par_msx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSY=%.2f,", data->par_msy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MRP=%c%c,", 
		data->par_mrp[0], data->par_mrp[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSD=%.2f,", data->par_msd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSR=%.2f,", data->par_msr);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSP=%.2f,", data->par_msp);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSG=%.2f,", data->par_msg);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "GCG=%.2f,", data->par_gcg);
	buff_len = strlen(buff);
	if (strlen(data->par_cpr) > 0)
	    {
	    sprintf(&buff[buff_len], "CPR=%s,", data->par_cpr);
	    buff_len = strlen(buff);
	    }
	if (strlen(data->par_rop) > 0)
	    {
	    sprintf(&buff[buff_len], "ROP=%s,", data->par_rop);
	    buff_len = strlen(buff);
	    }
	if (strlen(data->par_sid) > 0)
	    {
	    sprintf(&buff[buff_len], "SID=%s,", data->par_sid);
	    buff_len = strlen(buff);
	    }
	if (strlen(data->par_pll) > 0)
	    {
	    sprintf(&buff[buff_len], "PLL=%s,", data->par_pll);
	    buff_len = strlen(buff);
	    }
	if (strlen(data->par_com) > 0)
	    {
	    /* replace commas (,) with caret (^) values to circumvent
	       the format's inability to store commas in comments */
	    while ((comma_ptr = strchr(data->par_com, ',')) != NULL)
		{
		comma_ptr[0] = '^';
		}
	    sprintf(&buff[buff_len], "COM=%s,", data->par_com);
	    buff_len = strlen(buff);
	    }
	buff[buff_len] = ',';
	buff_len++;
	if (buff_len % 2 == 0)
	    buff_len++;
	    
	/* put end of record in buffer */
	line[buff_len + 22] = EM2_END;
		
	/* get size of record */
	write_size = 25 + buff_len;
	int_ptr = (int *) &line[0];
	*int_ptr = write_size;
#ifdef BYTESWAPPED
	*int_ptr = (short) mb_swap_int(*int_ptr);
#endif
		
	/* compute checksum */
	uchar_ptr = (mb_u_char *) line;
	for (j=5;j<write_size-3;j++)
	    checksum += uchar_ptr[j];
    
	/* set checksum */
	short_ptr = (short *) &line[buff_len + 23];
#ifdef BYTESWAPPED
	*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
	*short_ptr = (unsigned short) checksum;
#endif

	/* finally write out the data */
	write_len = fwrite(&line,1,write_size,mbfp);
	if (write_len != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

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
int mbr_em300mba_wr_run_parameter(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_run_parameter";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_RUN_PARAMETER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       run_date:        %d\n",data->run_date);
		fprintf(stderr,"dbg5       run_msec:        %d\n",data->run_msec);
		fprintf(stderr,"dbg5       run_ping_count:  %d\n",data->run_ping_count);
		fprintf(stderr,"dbg5       run_serial:      %d\n",data->run_serial);
		fprintf(stderr,"dbg5       run_status:      %d\n",data->run_status);
		fprintf(stderr,"dbg5       run_mode:        %d\n",data->run_mode);
		fprintf(stderr,"dbg5       run_filter_id:   %d\n",data->run_filter_id);
		fprintf(stderr,"dbg5       run_min_depth:   %d\n",data->run_min_depth);
		fprintf(stderr,"dbg5       run_max_depth:   %d\n",data->run_max_depth);
		fprintf(stderr,"dbg5       run_absorption:  %d\n",data->run_absorption);
		fprintf(stderr,"dbg5       run_tran_pulse:  %d\n",data->run_tran_pulse);
		fprintf(stderr,"dbg5       run_tran_beam:   %d\n",data->run_tran_beam);
		fprintf(stderr,"dbg5       run_tran_pow:    %d\n",data->run_tran_pow);
		fprintf(stderr,"dbg5       run_rec_beam:    %d\n",data->run_rec_beam);
		fprintf(stderr,"dbg5       run_rec_band:    %d\n",data->run_rec_band);
		fprintf(stderr,"dbg5       run_rec_gain:    %d\n",data->run_rec_gain);
		fprintf(stderr,"dbg5       run_tvg_cross:   %d\n",data->run_tvg_cross);
		fprintf(stderr,"dbg5       run_ssv_source:  %d\n",data->run_ssv_source);
		fprintf(stderr,"dbg5       run_max_swath:   %d\n",data->run_max_swath);
		fprintf(stderr,"dbg5       run_beam_space:  %d\n",data->run_beam_space);
		fprintf(stderr,"dbg5       run_swath_angle: %d\n",data->run_swath_angle);
		fprintf(stderr,"dbg5       run_stab_mode:   %d\n",data->run_stab_mode);
		for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       run_spare[%d]:    %d\n",i,data->run_spare[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_RUN_PARAMETER_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_RUN_PARAMETER;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->run_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->run_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->run_ping_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->run_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->run_status);
		line[16] = data->run_mode;
		line[17] = data->run_filter_id;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(data->run_min_depth);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->run_max_depth);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(data->run_absorption);
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(data->run_tran_pulse);
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) mb_swap_short(data->run_tran_beam);
		line[28] = data->run_tran_pow;
		line[29] = data->run_rec_beam;
		line[30] = data->run_rec_band;
		line[31] = data->run_rec_gain;
		line[32] = data->run_tvg_cross;
		line[33] = data->run_ssv_source;
		short_ptr = (short *) &line[34];
		*short_ptr = (unsigned short) mb_swap_short(data->run_max_swath);
		line[36] = data->run_beam_space;
		line[37] = data->run_swath_angle;
		line[38] = data->run_stab_mode;
		for (i=0;i<6;i++)
		    line[39+i] = data->run_spare[i];
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->run_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->run_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->run_ping_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->run_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->run_status;
		line[16] = data->run_mode;
		line[17] = data->run_filter_id;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) data->run_min_depth;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->run_max_depth;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) data->run_absorption;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) data->run_tran_pulse;
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) data->run_tran_beam;
		line[28] = data->run_tran_pow;
		line[29] = data->run_rec_beam;
		line[30] = data->run_rec_band;
		line[31] = data->run_rec_gain;
		line[32] = data->run_tvg_cross;
		line[33] = data->run_ssv_source;
		short_ptr = (short *) &line[34];
		*short_ptr = (unsigned short) data->run_max_swath;
		line[36] = data->run_beam_space;
		line[37] = data->run_swath_angle;
		line[38] = data->run_stab_mode;
		for (i=0;i<6;i++)
		    line[39+i] = data->run_spare[i];
#endif
		line[EM2_RUN_PARAMETER_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_RUN_PARAMETER_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_RUN_PARAMETER_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_RUN_PARAMETER_SIZE-4,mbfp);
		if (write_len != EM2_RUN_PARAMETER_SIZE-4)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_clock(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_clock";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_CLOCK_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       clk_date:        %d\n",data->clk_date);
		fprintf(stderr,"dbg5       clk_msec:        %d\n",data->clk_msec);
		fprintf(stderr,"dbg5       clk_count:       %d\n",data->clk_count);
		fprintf(stderr,"dbg5       clk_serial:      %d\n",data->clk_serial);
		fprintf(stderr,"dbg5       clk_origin_date: %d\n",data->clk_origin_date);
		fprintf(stderr,"dbg5       clk_origin_msec: %d\n",data->clk_origin_msec);
		fprintf(stderr,"dbg5       clk_1_pps_use:   %d\n",data->clk_1_pps_use);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_CLOCK_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_CLOCK;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->clk_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->clk_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->clk_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->clk_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->clk_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->clk_origin_msec);
		line[20] = data->clk_1_pps_use;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->clk_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->clk_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->clk_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->clk_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->clk_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) data->clk_origin_msec;
		line[20] = data->clk_1_pps_use;
#endif
		line[EM2_CLOCK_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_CLOCK_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_CLOCK_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_CLOCK_SIZE,mbfp);
		if (write_len != EM2_CLOCK_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_tide(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_tide";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_TIDE_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       tid_date:        %d\n",data->tid_date);
		fprintf(stderr,"dbg5       tid_msec:        %d\n",data->tid_msec);
		fprintf(stderr,"dbg5       tid_count:       %d\n",data->tid_count);
		fprintf(stderr,"dbg5       tid_serial:      %d\n",data->tid_serial);
		fprintf(stderr,"dbg5       tid_origin_date: %d\n",data->tid_origin_date);
		fprintf(stderr,"dbg5       tid_origin_msec: %d\n",data->tid_origin_msec);
		fprintf(stderr,"dbg5       tid_tide:        %d\n",data->tid_tide);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_TIDE_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_TIDE;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->tid_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->tid_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->tid_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->tid_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->tid_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->tid_origin_msec);
		short_ptr = (short *) &line[20];
		*short_ptr = (short) mb_swap_short(data->tid_tide);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->tid_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->tid_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->tid_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->tid_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->tid_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) data->tid_origin_msec;
		short_ptr = (short *) &line[20];
		*short_ptr = (short) data->tid_tide;
#endif
		line[EM2_TIDE_SIZE-8] = '\0';
		line[EM2_TIDE_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_TIDE_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_TIDE_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_TIDE_SIZE,mbfp);
		if (write_len != EM2_TIDE_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_height(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_height";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_HEIGHT_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       hgt_date:        %d\n",data->hgt_date);
		fprintf(stderr,"dbg5       hgt_msec:        %d\n",data->hgt_msec);
		fprintf(stderr,"dbg5       hgt_count:       %d\n",data->hgt_count);
		fprintf(stderr,"dbg5       hgt_serial:      %d\n",data->hgt_serial);
		fprintf(stderr,"dbg5       hgt_height:      %d\n",data->hgt_height);
		fprintf(stderr,"dbg5       hgt_type:        %d\n",data->hgt_type);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_HEIGHT_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_HEIGHT;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->hgt_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->hgt_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->hgt_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->hgt_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->hgt_height);
		line[16] = (mb_u_char) data->hgt_type;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->hgt_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->hgt_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->hgt_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->hgt_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->hgt_height;
		line[16] = (mb_u_char) data->hgt_type;
#endif
		line[EM2_HEIGHT_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEIGHT_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_HEIGHT_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_HEIGHT_SIZE,mbfp);
		if (write_len != EM2_HEIGHT_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_heading(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_heading";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_HEADING_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       hed_date:        %d\n",data->hed_date);
		fprintf(stderr,"dbg5       hed_msec:        %d\n",data->hed_msec);
		fprintf(stderr,"dbg5       hed_count:       %d\n",data->hed_count);
		fprintf(stderr,"dbg5       hed_serial:      %d\n",data->hed_serial);
		fprintf(stderr,"dbg5       hed_ndata:       %d\n",data->hed_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<data->hed_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, data->hed_time[i], data->hed_heading[i]);
		fprintf(stderr,"dbg5       hed_heading_status: %d\n",data->hed_heading_status);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_HEADING_HEADER_SIZE 
			+ EM2_HEADING_SLICE_SIZE * data->hed_ndata + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_HEADING;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->hed_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->hed_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->hed_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->hed_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(data->hed_ndata);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->hed_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->hed_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->hed_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->hed_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) data->hed_ndata;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEADING_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_HEADING_HEADER_SIZE,mbfp);
		if (write_len != EM2_HEADING_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output binary heading data */
	if (status == MB_SUCCESS)
	    for (i=0;i<data->hed_ndata;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(data->hed_time[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(data->hed_heading[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) data->hed_time[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) data->hed_heading[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEADING_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_HEADING_SLICE_SIZE,mbfp);
		if (write_len != EM2_HEADING_SLICE_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = (mb_u_char) data->hed_heading_status;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_attitude(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_ATTITUDE_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       att_date:        %d\n",data->att_date);
		fprintf(stderr,"dbg5       att_msec:        %d\n",data->att_msec);
		fprintf(stderr,"dbg5       att_count:       %d\n",data->att_count);
		fprintf(stderr,"dbg5       att_serial:      %d\n",data->att_serial);
		fprintf(stderr,"dbg5       att_ndata:       %d\n",data->att_ndata);
		fprintf(stderr,"dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr,"dbg5       -------------------------------------\n");
		for (i=0;i<data->att_ndata;i++)
			fprintf(stderr,"dbg5        %3d  %d  %d %d %d %d\n",
				i, data->att_time[i], data->att_roll[i], 
				data->att_pitch[i], data->att_heave[i], 
				data->att_heading[i]);
		fprintf(stderr,"dbg5       att_heading_status: %d\n",data->att_heading_status);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_ATTITUDE_HEADER_SIZE 
			+ EM2_ATTITUDE_SLICE_SIZE * data->att_ndata + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_ATTITUDE;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->att_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->att_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->att_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->att_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(data->att_ndata);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->att_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->att_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->att_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->att_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) data->att_ndata;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_ATTITUDE_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_ATTITUDE_HEADER_SIZE,mbfp);
		if (write_len != EM2_ATTITUDE_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output binary heading data */
	if (status == MB_SUCCESS)
	    for (i=0;i<data->att_ndata;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(data->att_time[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(data->att_sensor_status[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(data->att_roll[i]);
		short_ptr = (short *) &line[6];
		*short_ptr = (short) mb_swap_short(data->att_pitch[i]);
		short_ptr = (short *) &line[8];
		*short_ptr = (short) mb_swap_short(data->att_heave[i]);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->att_heading[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) data->att_time[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) data->att_sensor_status[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) data->att_roll[i];
		short_ptr = (short *) &line[6];
		*short_ptr = (short) data->att_pitch[i];
		short_ptr = (short *) &line[8];
		*short_ptr = (short) data->att_heave[i];
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->att_heading[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_ATTITUDE_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_ATTITUDE_SLICE_SIZE,mbfp);
		if (write_len != EM2_ATTITUDE_SLICE_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = (mb_u_char) data->att_heading_status;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_pos(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_pos";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_POS_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       pos_date:        %d\n",data->pos_date);
		fprintf(stderr,"dbg5       pos_msec:        %d\n",data->pos_msec);
		fprintf(stderr,"dbg5       pos_count:       %d\n",data->pos_count);
		fprintf(stderr,"dbg5       pos_serial:      %d\n",data->pos_serial);
		fprintf(stderr,"dbg5       pos_latitude:    %d\n",data->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:   %d\n",data->pos_longitude);
		fprintf(stderr,"dbg5       pos_quality:     %d\n",data->pos_quality);
		fprintf(stderr,"dbg5       pos_speed:       %d\n",data->pos_speed);
		fprintf(stderr,"dbg5       pos_course:      %d\n",data->pos_course);
		fprintf(stderr,"dbg5       pos_heading:     %d\n",data->pos_heading);
		fprintf(stderr,"dbg5       pos_system:      %d\n",data->pos_system);
		fprintf(stderr,"dbg5       pos_input_size:  %d\n",data->pos_input_size);
		fprintf(stderr,"dbg5       pos_input:\ndbg5            %s\n",data->pos_input);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_POS_HEADER_SIZE 
			+ data->pos_input_size 
			- (data->pos_input_size % 2) + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_POS;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->pos_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->pos_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->pos_latitude);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->pos_longitude);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_quality);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_speed);
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_course);
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) mb_swap_short(data->pos_heading);
		line[28] = (mb_u_char) data->pos_system;
		line[29] = (mb_u_char) data->pos_input_size;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->pos_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->pos_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->pos_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->pos_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->pos_latitude;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) data->pos_longitude;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->pos_quality;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) data->pos_speed;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) data->pos_course;
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) data->pos_heading;
		line[28] = (mb_u_char) data->pos_system;
		line[29] = (mb_u_char) data->pos_input_size;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_POS_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_POS_HEADER_SIZE,mbfp);
		if (write_len != EM2_POS_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output original ascii heading data */
	if (status == MB_SUCCESS)
		{
		write_size = data->pos_input_size 
				- (data->pos_input_size % 2) + 1;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) data->pos_input;
		for (j=0;j<write_size;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(data->pos_input,1,write_size,mbfp);
		if (write_len != write_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[1] = 0x03;
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(&line[1],1,3,mbfp);
		if (write_len != 3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_svp(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_SVP_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",data->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",data->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",data->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",data->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",data->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",data->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",data->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",data->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, data->svp_depth[i], data->svp_vel[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_SVP_HEADER_SIZE 
			+ EM2_SVP_SLICE_SIZE * data->svp_num + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_SVP;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->svp_use_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->svp_use_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->svp_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->svp_origin_msec);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_num);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_depth_res);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->svp_use_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->svp_use_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->svp_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->svp_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->svp_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) data->svp_origin_msec;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->svp_num;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) data->svp_depth_res;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SVP_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SVP_HEADER_SIZE,mbfp);
		if (write_len != EM2_SVP_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output binary svp data */
	if (status == MB_SUCCESS)
	    for (i=0;i<data->svp_num;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_depth[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(data->svp_vel[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) data->svp_depth[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) data->svp_vel[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SVP_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SVP_SLICE_SIZE,mbfp);
		if (write_len != EM2_SVP_SLICE_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = '\0';
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_bath(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_bath";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[EM2_BATH_MBA_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",data->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",data->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",data->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",data->png_serial);
		fprintf(stderr,"dbg5       png_latitude:    %d\n",data->png_latitude);
		fprintf(stderr,"dbg5       png_longitude:   %d\n",data->png_longitude);
		fprintf(stderr,"dbg5       png_speed:       %d\n",data->png_speed);
		fprintf(stderr,"dbg5       png_heading:     %d\n",data->png_heading);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",data->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %d\n",data->png_xducer_depth);
		fprintf(stderr,"dbg5       png_offset_multiplier: %d\n",data->png_offset_multiplier);
		fprintf(stderr,"dbg5       png_nbeams_max:        %d\n",data->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",data->png_nbeams);
		fprintf(stderr,"dbg5       png_depth_res:         %d\n",data->png_depth_res);
		fprintf(stderr,"dbg5       png_distance_res:      %d\n",data->png_distance_res);
		fprintf(stderr,"dbg5       png_sample_rate:       %d\n",data->png_sample_rate);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd  amp num flag\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<data->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %4d %3d %4d\n",
				i, data->png_depth[i], data->png_acrosstrack[i], 
				data->png_alongtrack[i], data->png_depression[i], 
				data->png_azimuth[i], data->png_range[i], 
				data->png_quality[i], data->png_window[i], 
				data->png_amp[i], data->png_beam_num[i], 
				data->png_beamflag[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_BATH_MBA_HEADER_SIZE 
			+ EM2_BATH_MBA_BEAM_SIZE * data->png_nbeams + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_BATH_MBA;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->png_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->png_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->png_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->png_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->png_latitude);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->png_longitude);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->png_apeed);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(data->png_heading);
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(data->png_ssv);
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) mb_swap_short(data->png_xducer_depth);
		line[28] = (mb_u_char) data->png_nbeams_max;
		line[29] = (mb_u_char) data->png_nbeams;
		line[30] = (mb_u_char) data->png_depth_res;
		line[31] = (mb_u_char) data->png_distance_res;
		short_ptr = (short *) &line[32];
		*short_ptr = (unsigned short) mb_swap_short(data->png_sample_rate);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->png_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->png_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->png_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->png_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) data->png_latitude;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) data->png_longitude;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->png_speed;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) data->png_heading;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) data->png_ssv;
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) data->png_xducer_depth;
		line[28] = (mb_u_char) data->png_nbeams_max;
		line[29] = (mb_u_char) data->png_nbeams;
		line[30] = (mb_u_char) data->png_depth_res;
		line[31] = (mb_u_char) data->png_distance_res;
		short_ptr = (short *) &line[32];
		*short_ptr = (unsigned short) data->png_sample_rate;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_BATH_MBA_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_BATH_MBA_HEADER_SIZE,mbfp);
		if (write_len != EM2_BATH_MBA_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    for (i=0;i<data->png_nbeams;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		if (data->sonar == 300)
		    *short_ptr = (unsigned short) mb_swap_short(data->png_depth[i]);
		else
		    *short_ptr = (short) mb_swap_short(data->png_depth[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (short) mb_swap_short(data->png_acrosstrack[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(data->png_alongtrack[i]);
		short_ptr = (short *) &line[6];
		*short_ptr = (short) mb_swap_short(data->png_depression[i]);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->png_azimuth[i]);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->png_range[i]);
		line[12] = (mb_u_char) data->png_quality[i];
		line[13] = (mb_u_char) data->png_window[i];
		short_ptr = (short *) &line[14];
		*short_ptr = (short) mb_swap_short(data->png_amp[i]);
		line[16] = (mb_u_char) data->png_beam_num[i];
		line[17] = (char) data->png_beamflag[i];
#else
		short_ptr = (short *) &line[0];
		if (data->sonar == 300)
		    *short_ptr = (unsigned short) data->png_depth[i];
		else
		    *short_ptr = (short) data->png_depth[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) data->png_acrosstrack[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) data->png_alongtrack[i];
		short_ptr = (short *) &line[6];
		*short_ptr = (short) data->png_depression[i];
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->png_azimuth[i];
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->png_range[i];
		line[12] = (mb_u_char) data->png_quality[i];
		line[13] = (mb_u_char) data->png_window[i];
		short_ptr = (short *) &line[14];
		*short_ptr = (short) data->png_amp[i];
		line[16] = (mb_u_char) data->png_beam_num[i];
		line[17] = (char) data->png_beamflag[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_BATH_MBA_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_BATH_MBA_BEAM_SIZE,mbfp);
		if (write_len != EM2_BATH_MBA_BEAM_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = (mb_u_char) data->png_offset_multiplier;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300mba_wr_ss(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em300mba_wr_ss";
	int	status = MB_SUCCESS;
	struct mbf_em300mba_struct *data;
	char	line[2 * MBF_EM300MBA_MAXPIXELS];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	offset;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_em300mba_struct *) data_ptr;
	
	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",data->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",data->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",data->date);
		fprintf(stderr,"dbg5       msec:            %d\n",data->msec);
		fprintf(stderr,"dbg5       png_ss_date:     %d\n",data->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:     %d\n",data->png_ss_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",data->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",data->png_serial);
		fprintf(stderr,"dbg5       png_max_range:   %d\n",data->png_max_range);
		fprintf(stderr,"dbg5       png_r_zero:      %d\n",data->png_r_zero);
		fprintf(stderr,"dbg5       png_r_zero_corr: %d\n",data->png_r_zero_corr);
		fprintf(stderr,"dbg5       png_tvg_start:   %d\n",data->png_tvg_start);
		fprintf(stderr,"dbg5       png_tvg_stop:    %d\n",data->png_tvg_stop);
		fprintf(stderr,"dbg5       png_bsn:         %d\n",data->png_bsn);
		fprintf(stderr,"dbg5       png_bso:         %d\n",data->png_bso);
		fprintf(stderr,"dbg5       png_tx:          %d\n",data->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover: %d\n",data->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:     %d\n",data->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:       %d\n",data->png_npixels);
		fprintf(stderr,"dbg5       png_pixel_size:    %d\n",data->png_pixel_size);
		fprintf(stderr,"dbg5       png_pixels_ss:     %d\n",data->png_pixels_ss);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<data->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d\n",
				i, data->png_beam_index[i], data->png_sort_direction[i], 
				data->png_beam_samples[i], data->png_start_sample[i], 
				data->png_center_sample[i]);
		fprintf(stderr,"dbg5       cnt  ssraw\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<data->png_npixels;i++)
			fprintf(stderr,"dbg5        %d %d\n",
				i, data->png_ssraw[i]);
		fprintf(stderr,"dbg5       pixel  ss  ltrack\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<MBF_EM300MBA_MAXPIXELS;i++)
			fprintf(stderr,"dbg5        %4d %6d %6d\n",
				i, data->png_ss[i], data->png_ssalongtrack[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_SS_MBA_HEADER_SIZE 
			+ EM2_SS_MBA_BEAM_SIZE * data->png_nbeams_ss 
			+ data->png_npixels - (data->png_npixels % 2) + 8
			+ 2 * data->png_pixels_ss * sizeof(short);
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		label = EM2_SS_MBA;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	if (status == MB_SUCCESS)
		{
		label = data->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
			
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(data->png_ss_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(data->png_ss_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(data->png_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(data->png_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(data->png_max_range);
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) mb_swap_short(data->png_r_zero);
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) mb_swap_short(data->png_r_zero_corr);
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(data->png_tvg_start);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(data->png_tvg_stop);
		line[22] = (mb_s_char) data->png_bsn;
		line[23] = (mb_s_char) data->png_bso;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(data->png_tx);
		line[26] = (mb_u_char) data->png_tvg_crossover;
		line[27] = (mb_u_char) data->png_nbeams_ss;
		short_ptr = (short *) &line[28];
		*short_ptr = (unsigned short) mb_swap_short(data->png_pixel_size);
		short_ptr = (short *) &line[30];
		*short_ptr = (unsigned short) mb_swap_short(data->png_pixels_ss);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) data->png_ss_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) data->png_ss_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) data->png_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) data->png_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) data->png_max_range;
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) data->png_r_zero;
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) data->png_r_zero_corr;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) data->png_tvg_start;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) data->png_tvg_stop;
		line[22] = (mb_s_char) data->png_bsn;
		line[23] = (mb_s_char) data->png_bso;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) data->png_tx;
		line[26] = (mb_u_char) data->png_tvg_crossover;
		line[27] = (mb_u_char) data->png_nbeams_ss;
		short_ptr = (short *) &line[28];
		*short_ptr = (short) data->png_pixel_size;
		short_ptr = (short *) &line[30];
		*short_ptr = (short) data->png_pixels_ss;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SS_MBA_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SS_MBA_HEADER_SIZE,mbfp);
		if (write_len != EM2_SS_MBA_HEADER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    for (i=0;i<data->png_nbeams_ss;i++)
		{
#ifdef BYTESWAPPED
		line[0] = (mb_u_char) data->png_beam_index[i];
		line[1] = (mb_s_char) data->png_sort_direction[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) mb_swap_short(data->png_beam_samples[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(data->png_center_sample[i]);
#else
		line[0] = (mb_u_char) data->png_beam_index[i];
		line[1] = (mb_s_char) data->png_sort_direction[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) data->png_beam_samples[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) data->png_center_sample[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SS_MBA_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SS_MBA_BEAM_SIZE,mbfp);
		if (write_len != EM2_SS_MBA_BEAM_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output raw sidescan data */
	if (status == MB_SUCCESS)
		{
		write_size = data->png_npixels + 1 - (data->png_npixels % 2);
		if (data->png_npixels % 2 == 0)
		    data->png_ssraw[data->png_npixels] = 0;

		/* compute checksum */
		uchar_ptr = (mb_u_char *) data->png_ssraw;
		for (j=0;j<write_size;j++)
		    checksum += uchar_ptr[j];

		write_len = fwrite(data->png_ssraw,1,write_size,mbfp);
		if (write_len != write_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* output processed sidescan data */
	if (status == MB_SUCCESS)
	    {
	    offset = (MBF_EM300MBA_MAXPIXELS - data->png_pixels_ss) / 2;
	    for (i=0;i<data->png_pixels_ss;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[2*i];
		*short_ptr = (short) mb_swap_short(data->png_ss[offset+i]);
#else
		short_ptr = (short *) &line[2*i];
		*short_ptr = (short) data->png_ss[offset+i];
#endif
		}
		
	    /* compute checksum */
	    write_size = data->png_pixels_ss * sizeof(short);
	    uchar_ptr = (mb_u_char *) &line[0];
	    for (j=0;j<write_size;j++)
		checksum += uchar_ptr[j];
    
	    /* write out data */
	    write_len = fwrite(line,1,write_size,mbfp);
	    if (write_len != write_size)
		    {
		    *error = MB_ERROR_WRITE_FAIL;
		    status = MB_FAILURE;
		    }
	    else
		    {
		    *error = MB_ERROR_NO_ERROR;
		    status = MB_SUCCESS;
		    }
	    }

	/* output processed sidescan alongtrack data */
	if (status == MB_SUCCESS)
	    {
	    offset = (MBF_EM300MBA_MAXPIXELS - data->png_pixels_ss) / 2;
	    for (i=0;i<data->png_pixels_ss;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[sizeof(short)*i];
		*short_ptr = (short) mb_swap_short(data->png_ssalongtrack[offset+i]);
#else
		short_ptr = (short *) &line[sizeof(short)*i];
		*short_ptr = (short) data->png_ssalongtrack[offset+i];
#endif
		}
		
	    /* compute checksum */
	    write_size = data->png_pixels_ss * sizeof(short);
	    uchar_ptr = (mb_u_char *) &line[0];
	    for (j=0;j<write_size;j++)
		checksum += uchar_ptr[j];
    
	    /* write out data */
	    write_len = fwrite(line,1,write_size,mbfp);
	    if (write_len != write_size)
		    {
		    *error = MB_ERROR_WRITE_FAIL;
		    status = MB_FAILURE;
		    }
	    else
		    {
		    *error = MB_ERROR_NO_ERROR;
		    status = MB_SUCCESS;
		    }
	    }

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[1] = 0x03;
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(&line[1],1,3,mbfp);
		if (write_len != 3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
