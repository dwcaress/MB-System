/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad2.c	3.00	10/9/98
 *	$Id: mbsys_simrad2.c,v 4.4 2000-10-11 01:03:21 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * mbsys_simrad2.c contains the functions for handling the data structure
 * used by MBIO functions to store data in the new (post 1997) Simrad 
 * multibeam data formats. The sonars which produce data in the new
 * formats include the EM300 and the EM3000. The associated data
 * formats include
 *      MBF_EM300RAW : MBIO ID 56
 *      MBF_EM300MBA : MBIO ID 57
 *
 * These functions include:
 *   mbsys_simrad2_alloc       - allocate memory for mbsys_simrad2_struct structure
 *   mbsys_simrad2_deall       - deallocate memory for mbsys_simrad2_struct structure
 *   mbsys_simrad2_extract     - extract basic data from mbsys_simrad2_struct 
 *				 structure
 *   mbsys_simrad2_insert      - insert basic data into mbsys_simrad2_struct structure
 *   mbsys_simrad2_ttimes      - extract travel time data from
 *				 mbsys_simrad2_struct structure
 *   mbsys_simrad2_altitude    - extract transducer depth and altitude from
 *				 mbsys_simrad2_struct structure
 *   mbsys_simrad2_extract_nav - extract navigation data from
 *                               mbsys_simrad2_struct structure
 *   mbsys_simrad2_insert_nav  - insert navigation data into
 *                               mbsys_simrad2_struct structure
 *   mbsys_simrad2_copy	       - copy data in one mbsys_simrad2_struct structure
 *   				 into another mbsys_simrad2_struct structure
 *
 * Author:	D. W. Caress
 * Date:	October 9, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  2000/07/20  20:24:59  caress
 * First cut at supporting both EM120 and EM1002.
 *
 * Revision 4.1  2000/07/19  04:01:41  caress
 * Supported EM120.
 *
 * Revision 4.0  1998/12/17  22:59:14  caress
 * MB-System version 4.6beta4
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

/*--------------------------------------------------------------------*/
int mbsys_simrad2_alloc(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 4.4 2000-10-11 01:03:21 caress Exp $";
	char	*function_name = "mbsys_simrad2_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_simrad2_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_simrad2_struct *) *store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->type = EM2_NONE;
	store->sonar = MBSYS_SIMRAD2_UNKNOWN;

	/* time stamp */
	store->date = 0;
	store->msec = 0;

	/* installation parameter values */
	store->par_date = 0;	/* installation parameter date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->par_msec = 0;	/* installation parameter time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->par_line_num = 0;/* survey line number */
	store->par_serial_1 = 0;/* system 1 serial number */
	store->par_serial_2 = 0;/* system 2 serial number */
	store->par_wlz = 0.0;	/* water line vertical location (m) */
	store->par_smh = 0;	/* system main head serial number */
	store->par_s1z = 0.0;	/* transducer 1 vertical location (m) */
	store->par_s1x = 0.0;	/* transducer 1 along location (m) */
	store->par_s1y = 0.0;	/* transducer 1 athwart location (m) */
	store->par_s1h = 0.0;	/* transducer 1 heading (deg) */
	store->par_s1r = 0.0;	/* transducer 1 roll (m) */
	store->par_s1p = 0.0;	/* transducer 1 pitch (m) */
	store->par_s1n = 0;	/* transducer 1 number of modules */
	store->par_s2z = 0.0;	/* transducer 2 vertical location (m) */
	store->par_s2x = 0.0;	/* transducer 2 along location (m) */
	store->par_s2y = 0.0;	/* transducer 2 athwart location (m) */
	store->par_s2h = 0.0;	/* transducer 2 heading (deg) */
	store->par_s2r = 0.0;	/* transducer 2 roll (m) */
	store->par_s2p = 0.0;	/* transducer 2 pitch (m) */
	store->par_s2n = 0;	/* transducer 2 number of modules */
	store->par_go1 = 0.0;	/* system (sonar head 1) gain offset */
	store->par_go2 = 0.0;	/* sonar head 2 gain offset */
	for (i=0;i<16;i++)
	    {
	    store->par_tsv[i] = '\0';	/* transmitter (sonar head 1) software version */
	    store->par_rsv[i] = '\0';	/* receiver (sonar head 2) software version */
	    store->par_bsv[i] = '\0';	/* beamformer software version */
	    store->par_psv[i] = '\0';	/* processing unit software version */
	    store->par_osv[i] = '\0';	/* operator station software version */
	    }
	store->par_dsd = 0.0;	/* depth sensor time delay (msec) */
	store->par_dso = 0.0;	/* depth sensor offset */
	store->par_dsf = 0.0;	/* depth sensor scale factor */
	store->par_dsh[0] = 'I';	/* depth sensor heave (IN or NI) */
	store->par_dsh[1] = 'N';	/* depth sensor heave (IN or NI) */
	store->par_aps = 0;	/* active position system number */
	store->par_p1m = 0;	/* position system 1 motion compensation (boolean) */
	store->par_p1t = 0;	/* position system 1 time stamp used 
				    (0=system time, 1=position input time) */
	store->par_p1z = 0.0;	/* position system 1 vertical location (m) */
	store->par_p1x = 0.0;	/* position system 1 along location (m) */
	store->par_p1y = 0.0;	/* position system 1 athwart location (m) */
	store->par_p1d = 0.0;	/* position system 1 time delay (sec) */
	for (i=0;i<16;i++)
	    {
	    store->par_p1g[i] = '\0';	/* position system 1 geodetic datum */
	    }
	store->par_p2m = 0;	/* position system 2 motion compensation (boolean) */
	store->par_p2t = 0;	/* position system 2 time stamp used 
				    (0=system time, 1=position input time) */
	store->par_p2z = 0.0;	/* position system 2 vertical location (m) */
	store->par_p2x = 0.0;	/* position system 2 along location (m) */
	store->par_p2y = 0.0;	/* position system 2 athwart location (m) */
	store->par_p2d = 0.0;	/* position system 2 time delay (sec) */
	for (i=0;i<16;i++)
	    {
	    store->par_p2g[i] = '\0';	/* position system 2 geodetic datum */
	    }
	store->par_p3m = 0;	/* position system 3 motion compensation (boolean) */
	store->par_p3t = 0;	/* position system 3 time stamp used 
				    (0=system time, 1=position input time) */
	store->par_p3z = 0.0;	/* position system 3 vertical location (m) */
	store->par_p3x = 0.0;	/* position system 3 along location (m) */
	store->par_p3y = 0.0;	/* position system 3 athwart location (m) */
	store->par_p3d = 0.0;	/* position system 3 time delay (sec) */
	for (i=0;i<16;i++)
	    {
	    store->par_p3g[i] = '\0';	/* position system 3 geodetic datum */
	    }
	store->par_msz = 0.0;	/* motion sensor vertical location (m) */
	store->par_msx = 0.0;	/* motion sensor along location (m) */
	store->par_msy = 0.0;	/* motion sensor athwart location (m) */
	store->par_mrp[0] = 'H';	/* motion sensor roll reference plane (HO or RP) */
	store->par_mrp[1] = 'O';	/* motion sensor roll reference plane (HO or RP) */
	store->par_msd = 0.0;	/* motion sensor time delay (sec) */
	store->par_msr = 0.0;	/* motion sensor roll offset (deg) */
	store->par_msp = 0.0;	/* motion sensor pitch offset (deg) */
	store->par_msg = 0.0;	/* motion sensor heading offset (deg) */
	store->par_gcg = 0.0;	/* gyro compass heading offset (deg) */
	for (i=0;i<4;i++)
	    {
	    store->par_cpr[i] = '\0';	/* cartographic projection */
	    }
	for (i=0;i<MBSYS_SIMRAD2_COMMENT_LENGTH;i++)
	    {
	    store->par_rop[i] = '\0';	/* responsible operator */
	    store->par_sid[i] = '\0';	/* survey identifier */
	    store->par_pll[i] = '\0';	/* survey line identifier (planned line number) */
	    store->par_com[i] = '\0';	/* comment */
	    }

	/* runtime parameter values */
	store->run_date = 0;		/* runtime parameter date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->run_msec = 0;		/* runtime parameter time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->run_ping_count = 0;	/* ping counter */
	store->run_serial = 0;		/* system 1 or 2 serial number */
	store->run_status = 0;		/* system status */
	store->run_mode = 0;		/* system mode:
				    0 : nearfield (EM3000) or very shallow (EM300)
				    1 :	normal (EM3000) or shallow (EM300)
				    2 : medium (EM300)
				    3 : deep (EM300)
				    4 : very deep (EM300) */
	store->run_filter_id = 0;	/* filter identifier - the two lowest bits
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
	store->run_min_depth = 0;	/* minimum depth (m) */
	store->run_max_depth = 0;	/* maximum depth (m) */
	store->run_absorption = 0;	/* absorption coefficient (0.01 dB/km) */

	store->run_tran_pulse = 0;	/* transmit pulse length (usec) */
	store->run_tran_beam = 0;	/* transmit beamwidth (0.1 deg) */
	store->run_tran_pow = 0;	/* transmit power reduction (dB) */
	store->run_rec_beam = 0;	/* receiver beamwidth (0.1 deg) */
	store->run_rec_beam = 0;	/* receiver bandwidth (50 hz) */
	store->run_rec_gain = 0;	/* receiver fixed gain (dB) */
	store->run_tvg_cross = 0;	/* TVG law crossover angle (deg) */
	store->run_ssv_source = 0;	/* source of sound speed at transducer:
				    0 : from sensor
				    1 : manual
				    2 : from profile */
	store->run_max_swath = 0;	/* maximum swath width (m) */
	store->run_beam_space = 0;	/* beam spacing:
				    0 : determined by beamwidth (EM3000)
				    1 : equidistant
				    2 : equiangle */
	store->run_swath_angle = 0;	/* coverage sector of swath (deg) */
	store->run_stab_mode = 0;	/* yaw and pitch stabilization mode:
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
	    store->run_spare[i] = '\0';
	    }

	/* sound velocity profile */
	store->svp_use_date = 0;	/* date at start of use
				    date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->svp_use_msec = 0;	/* time at start of use since midnight in msec
				    08:12:51.234 = 29570234 */
	store->svp_count = 0;		/* sequential counter or input identifier */
	store->svp_serial = 0;		/* system 1 serial number */
	store->svp_origin_date = 0;	/* date at svp origin
				    date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->svp_origin_msec = 0;	/* time at svp origin since midnight in msec
				    08:12:51.234 = 29570234 */
	store->svp_num = 0;		/* number of svp entries */
	store->svp_depth_res = 0;	/* depth resolution (cm) */
	for (i=0;i<MBSYS_SIMRAD2_MAXSVP;i++)
	    {
	    store->svp_depth[i] = 0;	/* depth of svp entries (according to svp_depth_res) */
	    store->svp_vel[i] = 0;	/* sound speed of svp entries (0.1 m/sec) */
	    }
	    
	/* position */
	store->pos_date = 0;		/* position date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->pos_msec = 0;		/* position time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->pos_count = 0;		/* sequential counter */
	store->pos_serial = 0;		/* system 1 serial number */
	store->pos_latitude = 0;	/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	store->pos_longitude = 0;	/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	store->pos_quality = 0;	/* measure of position fix quality (cm) */
	store->pos_speed = 0;		/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
	store->pos_course = 0;		/* course over ground (0.01 deg) if valid,
				    invalid = 0xFFFF */
	store->pos_heading = 0;	/* heading (0.01 deg) if valid,
				    invalid = 0xFFFF */
	store->pos_system = 0;		/* position system number, type, and realtime use
				    - position system number given by two lowest bits
				    - fifth bit set means position must be derived
					from input Simrad 90 datagram
				    - sixth bit set means valid time is that of
					input datagram */
	store->pos_input_size = 0;	/* number of bytes in input position datagram */
	for (i=0;i<256;i++)
	    {
	    store->pos_input[i] = 0;	/* position input datagram as received, minus
				    header and tail (such as NMEA 0183 $ and CRLF) */
	    }
	    
	/* height */
	store->hgt_date = 0;		/* height date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->hgt_msec = 0;		/* height time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->hgt_count = 0;		/* sequential counter */
	store->hgt_serial = 0;		/* system 1 serial number */
	store->hgt_height = 0;		/* height (0.01 m) */
	store->hgt_type = 0;		/* height type as given in input datagram or if
				    zero the height is derived from the GGK datagram
				    and is the height of the water level re the
				    vertical datum */
	
	/* tide */
	store->tid_date = 0;		/* tide date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->tid_msec = 0;		/* tide time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->tid_count = 0;		/* sequential counter */
	store->tid_serial = 0;		/* system 1 serial number */
	store->tid_origin_date = 0;	/* tide input date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->tid_origin_msec = 0;	/* tide input time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->tid_tide = 0;		/* tide offset (0.01 m) */	
	
	/* clock */
	store->clk_date = 0;		/* system date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->clk_msec = 0;		/* system time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->clk_count = 0;		/* sequential counter */
	store->clk_serial = 0;		/* system 1 serial number */
	store->clk_origin_date	= 0;	/* external clock date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->clk_origin_msec = 0;	/* external clock time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->clk_1_pps_use = 0;	/* if 1 then the internal clock is synchronized
				    to an external 1 PPS signal, if 0 then not */

	/* pointer to attitude data structure */
	store->attitude = NULL;

	/* pointer to heading data structure */
	store->heading = NULL;

	/* pointer to survey data structure */
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
int mbsys_simrad2_survey_alloc(int verbose, 
			char *mbio_ptr, char *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 4.4 2000-10-11 01:03:21 caress Exp $";
	char	*function_name = "mbsys_simrad2_survey_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->ping == NULL)
		status = mb_malloc(verbose,
			sizeof(struct mbsys_simrad2_ping_struct),
			&(store->ping),error);
			
	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* initialize everything */
		ping->png_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ping->png_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		ping->png_count = 0;	
				/* sequential counter or input identifier */
		ping->png_serial = 0;	
				/* system 1 or system 2 serial number */
		ping->png_latitude = 0;
				/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
		ping->png_longitude = 0;
				/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
		ping->png_speed = 0;
				/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
		ping->png_heading = 0;	
				/* heading (0.01 deg) */
		ping->png_ssv = 0;	
				/* sound speed at transducer (0.1 m/sec) */
		ping->png_xducer_depth = 0;   
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
		ping->png_offset_multiplier = 0;	
				/* transmit transducer depth offset multiplier
				   - see note 7 above */ 
				   
		/* beam data */
		ping->png_nbeams_max = 0;	
				/* maximum number of beams possible */
		ping->png_nbeams = 0;	
				/* number of valid beams */
		ping->png_depth_res = 0;	
				/* depth resolution (0.01 m) */
		ping->png_distance_res = 0;	
				/* x and y resolution (0.01 m) */
		ping->png_sample_rate = 0;	
				/* sampling rate (Hz) OR depth difference between
				    sonar heads in EM3000D - see note 9 above */
		for (i=0;i<MBSYS_SIMRAD2_MAXBEAMS;i++)
		    {
		    ping->png_depth[i] = 0;	
				/* depths in depth resolution units */
		    ping->png_acrosstrack[i] = 0;
				/* acrosstrack distances in distance resolution units */
		    ping->png_alongtrack[i] = 0;
				/* alongtrack distances in distance resolution units */
		    ping->png_depression[i] = 0;
				/* Primary beam angles in one of two formats (see note 10 above)
				   1: Corrected format - gives beam depression angles
				        in 0.01 degree. These are the takeoff angles used
					in raytracing calculations.
				   2: Uncorrected format - gives beam pointing angles
				        in 0.01 degree. These values are relative to
					the transducer array and have not been corrected
					for vessel motion. */
		    ping->png_azimuth[i] = 0;
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
		    ping->png_range[i] = 0;
				/* Ranges in one of two formats (see note 10 above):
				   1: Corrected format - the ranges are one way 
				        travel times in time units defined as half 
					the inverse sampling rate.
				   2: Uncorrected format - the ranges are raw two
				        way travel times in time units defined as
					half the inverse sampling rate. These values
					have not been corrected for changes in the
					heave during the ping cycle. */
		    ping->png_quality[i] = 0;	
				/* 0-254 */
		    ping->png_window[i] = 0;		
				/* samples/4 */
		    ping->png_amp[i] = 0;		
				/* 0.5 dB */
		    ping->png_beam_num[i] = 0;	
				/* beam 128 is first beam on 
				    second head of EM3000D */
		    ping->png_beamflag[i] = MB_FLAG_NULL;	
				/* uses standard MB-System beamflags */
		    }
	
		/* sidescan */
		ping->png_max_range = 0;  
				/* max range of ping in number of samples */
		ping->png_r_zero = 0;	
				/* range to normal incidence used in TVG
				    (R0 predicted) in samples */
		ping->png_r_zero_corr = 0;
				/* range to normal incidence used to correct
				    sample amplitudes in number of samples */
		ping->png_tvg_start = 0;	
				/* start sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		ping->png_tvg_stop = 0;	\
				/* stop sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		ping->png_bsn = 0;	
				/* normal incidence backscatter (BSN) in dB */
		ping->png_bso = 0;	
				/* oblique incidence backscatter (BSO) in dB */
		ping->png_tx = 0;	
				/* Tx beamwidth in 0.1 degree */
		ping->png_tvg_crossover = 0;	
				/* TVG law crossover angle in degrees */
		ping->png_nbeams_ss = 0;	
				/* number of beams with sidescan */
		for (i=0;i<MBSYS_SIMRAD2_MAXBEAMS;i++)
		    {
		    ping->png_beam_index[i] = 0;	
				/* beam index number */
		    ping->png_sort_direction[i] = 0;	
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
		    ping->png_beam_samples[i] = 0;	
				/* number of sidescan samples derived from
					each beam */
		    ping->png_start_sample[i] = 0;	
				/* start sample number */
		    ping->png_center_sample[i] = 0;	
				/* center sample number */
		    }
		for (i=0;i<MBSYS_SIMRAD2_MAXRAWPIXELS;i++)
		    {
		    ping->png_ssraw[i] = EM2_INVALID_AMP;
				/* the raw sidescan ordered port to starboard */
		    }
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
		    {
		    ping->png_ss[i] = 0;
				/* the processed sidescan ordered port to starboard */
		    ping->png_ssalongtrack[i] = 0;
				/* the processed sidescan alongtrack distances 
					in distance resolution units */
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
int mbsys_simrad2_attitude_alloc(int verbose, 
			char *mbio_ptr, char *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 4.4 2000-10-11 01:03:21 caress Exp $";
	char	*function_name = "mbsys_simrad2_attitude_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_attitude_struct *attitude;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->attitude == NULL)
		status = mb_malloc(verbose,
			sizeof(struct mbsys_simrad2_attitude_struct),
			&(store->attitude),error);
			
	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		attitude = (struct mbsys_simrad2_attitude_struct *) store->attitude;

		/* initialize everything */
		attitude->att_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		attitude->att_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		attitude->att_count = 0;	
				/* sequential counter or input identifier */
		attitude->att_serial = 0;	
				/* system 1 or system 2 serial number */
		attitude->att_ndata = 0;	
				/* number of attitude data */
		for (i=0;i<MBSYS_SIMRAD2_MAXATTITUDE;i++)
		    {
		    attitude->att_time[i] = 0;
				/* time since record start (msec) */
		    attitude->att_sensor_status[i] = 0;
				/* see note 12 above */
		    attitude->att_roll[i] = 0;
				/* roll (0.01 degree) */
		    attitude->att_pitch[i] = 0;
				/* pitch (0.01 degree) */
		    attitude->att_heave[i] = 0;
				/* heave (cm) */
		    attitude->att_heading[i] = 0;
				/* heading (0.01 degree) */
		    }
		attitude->att_heading_status = 0;
				/* heading status (0=inactive) */
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
int mbsys_simrad2_heading_alloc(int verbose, 
			char *mbio_ptr, char *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 4.4 2000-10-11 01:03:21 caress Exp $";
	char	*function_name = "mbsys_simrad2_heading_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_heading_struct *heading;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->heading == NULL)
		status = mb_malloc(verbose,
			sizeof(struct mbsys_simrad2_heading_struct),
			&(store->heading),error);
			
	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		heading = (struct mbsys_simrad2_heading_struct *) store->heading;

		/* initialize everything */
		heading->hed_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		heading->hed_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		heading->hed_count = 0;	
				/* sequential counter or input identifier */
		heading->hed_serial = 0;	
				/* system 1 or system 2 serial number */
		heading->hed_ndata = 0;	
				/* number of heading data */
		for (i=0;i<MBSYS_SIMRAD2_MAXHEADING;i++)
		    {
		    heading->hed_time[i] = 0;
				/* time since record start (msec) */
		    heading->hed_heading[i] = 0;
				/* heading (0.01 degree) */
		    }
		heading->hed_heading_status = 0;
				/* heading status (0=inactive) */
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
int mbsys_simrad2_deall(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_simrad2_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;

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
	store = (struct mbsys_simrad2_struct *) *store_ptr;

	/* deallocate memory for survey data structure */
	if (store->ping != NULL)
		status = mb_free(verbose,&(store->ping),error);

	/* deallocate memory for attitude data structure */
	if (store->attitude != NULL)
		status = mb_free(verbose,&(store->attitude),error);

	/* deallocate memory for heading data structure */
	if (store->heading != NULL)
		status = mb_free(verbose,&(store->heading),error);

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
int mbsys_simrad2_extract(int verbose, char *mbio_ptr, char *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad2_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	int	ntime_i[7];
	double	ntime_d;
	mb_s_char	*beam_ss;
	double	depthscale, depthoffset;
	double	dacrscale, daloscale, ttscale, reflscale;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = 0.0000001 * ping->png_longitude;
		*navlat = 0.00000005 * ping->png_latitude;
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
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* read distance and depth values into storage arrays */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		dacrscale  = 0.01 * ping->png_distance_res;
		daloscale  = 0.01 * ping->png_distance_res;
		if (store->sonar <= EM2_EM3000)
		    ttscale = 0.5 / ping->png_sample_rate;
		else
		    ttscale = 0.5 / 14000;
		reflscale  = 0.5;
		*nbath = 0;
		for (j=0;j<MBSYS_SIMRAD2_MAXBEAMS;j++)
			{
			bath[j] = 0.0;
			beamflag[j] = MB_FLAG_NULL;
			amp[j] = 0.0;
			bathacrosstrack[j] = 0.0;
			bathalongtrack[j] = 0.0;
			}
		for (i=0;i<ping->png_nbeams;i++)
			{
			j = ping->png_beam_num[i] - 1;
			bath[j] = depthscale * ping->png_depth[i]
				    + depthoffset;
			beamflag[j] = ping->png_beamflag[i];
			bathacrosstrack[j] 
				= dacrscale * ping->png_acrosstrack[i];
			bathalongtrack[j] 
				= daloscale * ping->png_alongtrack[i];
			if (ping->png_quality[i] != 0)
				amp[j] = reflscale * ping->png_amp[i] + 64;
			else
				amp[j] = 0;
			*nbath = MAX(j + 1, *nbath);
			}
		*nss = MBSYS_SIMRAD2_MAXPIXELS;
		pixel_size = 0.01 * ping->png_pixel_size;
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
			{
			ss[i] = 0.01 * ping->png_ss[i];
			ssacrosstrack[i] = pixel_size 
					* (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
			ssalongtrack[i] = daloscale * ping->png_ssalongtrack[i];
			}
		mb_io_ptr->beams_bath = *nbath;
		*namp = *nbath;
		mb_io_ptr->beams_amp = *nbath;
		mb_io_ptr->pixels_ss = *nss;

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

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(comment,store->par_com,
			MBSYS_SIMRAD2_COMMENT_LENGTH);

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
int mbsys_simrad2_insert(int verbose, char *mbio_ptr, char *store_ptr, 
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad2_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	int	kind;
	int	time_j[5];
	double	depthscale, dacrscale,daloscale,ttscale,reflscale;
	double	depthoffset;
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
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* allocate secondary data structure for
			survey data if needed */
		if (store->ping == NULL)
			{
			status = mbsys_simrad2_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
		/* get time */
		ping->png_date = 10000 * time_i[0]
				    + 100 * time_i[1]
				    + time_i[2];
		ping->png_msec = 3600000 * time_i[3]
				    + 60000 * time_i[4]
				    + 1000 * time_i[5]
				    + 0.001 * time_i[6];
		store->date = ping->png_date;
		store->msec = ping->png_msec;
		
		/* get navigation */
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int) (heading * 100);

		/* get speed  */
		ping->png_speed = (int)(speed / 0.036);

		/* insert distance and depth values into storage arrays */
		if (store->sonar == MBSYS_SIMRAD2_UNKNOWN)
			{
			if (nbath <= 87)
				{
				store->sonar = MBSYS_SIMRAD2_EM2000;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 1; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 1; /* kluge */
				}
			else if (nbath <= 111)
				{
				store->sonar = MBSYS_SIMRAD2_EM1002;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 1; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 1; /* kluge */
				}
			else if (nbath <= 127)
				{
				store->sonar = MBSYS_SIMRAD2_EM3000;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 1; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 1; /* kluge */
				}
			else if (nbath <= 135)
				{
				store->sonar = MBSYS_SIMRAD2_EM300;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 10; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 10; /* kluge */
				}
			else if (nbath <= 191)
				{
				store->sonar = MBSYS_SIMRAD2_EM120;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 10; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 10; /* kluge */
				}
			else if (nbath <= 254)
				{
				store->sonar = MBSYS_SIMRAD2_EM3000D_2;
				if (ping->png_depth_res == 0)
				    ping->png_depth_res = 1; /* kluge */
				if (ping->png_distance_res == 0)
				    ping->png_distance_res = 1; /* kluge */
				}
			else
				{
				*error = MB_ERROR_DATA_NOT_INSERTED;
				status = MB_FAILURE;
				}
			}
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		dacrscale  = 0.01 * ping->png_distance_res;
		daloscale  = 0.01 * ping->png_distance_res;
		if (store->sonar <= EM2_EM3000)
		    ttscale = 0.5 / ping->png_sample_rate;
		else
		    ttscale = 0.5 / 14000;
		reflscale  = 0.5;
		if (status == MB_SUCCESS && ping->png_nbeams == 0)
			{
			for (i=0;i<nbath;i++)
			    if (beamflag[i] != MB_FLAG_NULL)
				{
				j = ping->png_nbeams;
				ping->png_beam_num[j] = i + 1;
				ping->png_depth[j] = (bath[i] - depthoffset)
							/ depthscale;
				ping->png_beamflag[j] = beamflag[i];
				ping->png_acrosstrack[j]
					= bathacrosstrack[i] / dacrscale;
				ping->png_alongtrack[j] 
					= bathalongtrack[i] / daloscale;
				if (amp[i] != 0.0)
					ping->png_amp[j] = (amp[i] - 64) 
						/ reflscale;
				else
					ping->png_amp[j] = 0;
				ping->png_nbeams++;
				}
			ping->png_nbeams_max = ping->png_nbeams;
			}
		else if (status == MB_SUCCESS)
			{
			for (j=0;j<ping->png_nbeams;j++)
				{
				i = ping->png_beam_num[j] - 1;
				ping->png_depth[j] = (bath[i] - depthoffset)
							/ depthscale;
				ping->png_beamflag[j] = beamflag[i];
				ping->png_acrosstrack[j]
					= bathacrosstrack[i] / dacrscale;
				ping->png_alongtrack[j] 
					= bathalongtrack[i] / daloscale;
				if (amp[i] != 0.0)
					ping->png_amp[j] = (amp[i] - 64) 
						/ reflscale;
				else
					ping->png_amp[j] = 0;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<nss;i++)
				{
				ping->png_ss[i] = 100 * ss[i];
				ping->png_ssalongtrack[i] = ssalongtrack[i] / daloscale;
				}
			}
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV)
		{

		/* get time */
		store->pos_date = 10000 * time_i[0]
				    + 100 * time_i[1]
				    + time_i[2];
		store->pos_msec = 3600000 * time_i[3]
				    + 60000 * time_i[4]
				    + 1000 * time_i[5]
				    + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;
		
		/* get navigation */
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int) (heading * 100);

		/* get speed  */
		store->pos_speed = (int)(speed / 0.036);

		/* get roll pitch and heave */
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->par_com,comment,
			MBSYS_SIMRAD2_COMMENT_LENGTH);
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
int mbsys_simrad2_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_simrad2_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	double	ttscale;
	double	heave_use;
	double	*angles_simrad;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* get depth offset (heave + heave offset) */
		heave_use =  0.0;
		*ssv = 0.1 * ping->png_ssv;
		*draft = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;

		/* get travel times, angles */
		if (store->sonar <= EM2_EM3000)
		    ttscale = 0.5 / ping->png_sample_rate;
		else
		    ttscale = 0.5 / 14000;
		*nbeams = 0;
		for (j=0;j<ping->png_nbeams_max;j++)
			{
			ttimes[j] = 0.0;
			angles[j] = 0.0;
			angles_forward[j] = 0.0;
			angles_null[j] = 0.0;
			heave[j] = 0.0;
			alongtrack_offset[j] = 0.0;
			}
		for (i=0;i<ping->png_nbeams;i++)
			{
			j = ping->png_beam_num[i] - 1;
			ttimes[j] = ttscale * ping->png_range[i];
			angles[j] = 90.0 - 0.01 * ping->png_depression[i];
			angles_forward[j] = 90 - 0.01 * ping->png_azimuth[i];
			if (angles_forward[j] < 0.0) angles_forward[j] += 360.0;
			angles_null[j] = 0.0;
			heave[j] = heave_use;
			alongtrack_offset[j] = 0.0;
			*nbeams = MAX(j + 1, *nbeams);
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
int mbsys_simrad2_altitude(int verbose, char *mbio_ptr, char *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_simrad2_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* get transducer depth and altitude */
		*transducer_depth = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		depthscale = 0.01 * ping->png_depth_res;
		dacrscale  = 0.01 * ping->png_distance_res;
		bath_best = 0.0;
		if (ping->png_depth[mb_io_ptr->beams_bath/2] > 0)
		    bath_best = depthscale * ping->png_depth[mb_io_ptr->beams_bath/2];
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (ping->png_quality[i] > 10
			    && fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
			    bath_best = depthscale * ping->png_depth[i];
			    }
			}		
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (ping->png_quality[i] > 0
			    && fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
			    bath_best = depthscale * ping->png_depth[i];
			    }
			}		
		    }
		*altitude = bath_best;

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
int mbsys_simrad2_extract_nav(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_simrad2_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	int	ntime_i[7];
	double	ntime_d;
	mb_s_char	*beam_ss;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = 0.0000001 * ping->png_longitude;
		*navlat = 0.00000005 * ping->png_latitude;
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
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;

		/* get roll pitch and heave */
		*roll = 0.0; /* kluge must get from attitude datagrams */
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

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV)
		{
                /* get survey data structure */
		if (store->ping != NULL)
                	ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = 0.0000001 * store->pos_longitude;
		*navlat = 0.00000005 * store->pos_latitude;
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
		if (store->pos_heading != EM2_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (store->ping != NULL)
			*draft = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.0; /* kluge must get from attitude datagrams */
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
int mbsys_simrad2_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_simrad2_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
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
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* allocate secondary data structure for
			survey data if needed */
		if (store->ping == NULL)
			{
			status = mbsys_simrad2_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
		/* get time */
		ping->png_date = 10000 * time_i[0]
				    + 100 * time_i[1]
				    + time_i[2];
		ping->png_msec = 3600000 * time_i[3]
				    + 60000 * time_i[4]
				    + 1000 * time_i[5]
				    + 0.001 * time_i[6];
		store->msec = ping->png_msec;
		store->date = ping->png_date;
		
		/* get navigation */
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int) (heading * 100);

		/* get speed  */
		ping->png_speed = (int) (speed / 0.036);

		/* get draft  */
		ping->png_offset_multiplier = (int)(draft / 655.36);
		ping->png_xducer_depth 
			= 100 * (draft - 655.36 * ping->png_offset_multiplier);

		/* get roll pitch and heave */
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV)
		{

		/* get time */
		store->pos_date = 10000 * time_i[0]
				    + 100 * time_i[1]
				    + time_i[2];
		store->pos_msec = 3600000 * time_i[3]
				    + 60000 * time_i[4]
				    + 1000 * time_i[5]
				    + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;
		
		/* get navigation */
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int) (heading * 100);

		/* get speed  */
		store->pos_speed = (int)(speed / 0.036);

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
int mbsys_simrad2_copy(int verbose, char *mbio_ptr, 
			char *store_ptr, char *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad2_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_struct *copy;
	struct mbsys_simrad2_ping_struct *ping_store;
	struct mbsys_simrad2_ping_struct *ping_copy;
	char	*ping_save;
	struct mbsys_simrad2_attitude_struct *attitude_store;
	struct mbsys_simrad2_attitude_struct *attitude_copy;
	char	*attitude_save;
	struct mbsys_simrad2_heading_struct *heading_store;
	struct mbsys_simrad2_heading_struct *heading_copy;
	char	*heading_save;

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
	store = (struct mbsys_simrad2_struct *) store_ptr;
	copy = (struct mbsys_simrad2_struct *) copy_ptr;
	
	/* check if survey data needs to be copied */
	if (store->kind == MB_DATA_DATA 
		&& store->ping != NULL)
		{
		/* make sure a survey data structure exists to
			be copied into */
		if (copy->ping == NULL)
			{
			status = mbsys_simrad2_survey_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}
			
		/* save pointer value */
		ping_save = (char *)copy->ping;
		}
	else
		ping_save = NULL;
	
	/* check if attitude data needs to be copied */
	if (store->attitude != NULL)
		{
		/* make sure a attitude data structure exists to
			be copied into */
		if (copy->attitude == NULL)
			{
			status = mbsys_simrad2_attitude_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}
			
		/* save pointer value */
		attitude_save = (char *)copy->attitude;
		}
	
	/* check if heading data needs to be copied */
	if (store->heading != NULL)
		{
		/* make sure a heading data structure exists to
			be copied into */
		if (copy->heading == NULL)
			{
			status = mbsys_simrad2_heading_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}
			
		/* save pointer value */
		heading_save = (char *)copy->heading;
		}

	/* copy the main structure */
	*copy = *store;
	
	/* if needed copy the survey data structure */
	if (store->kind == MB_DATA_DATA 
		&& store->ping != NULL 
		&& status == MB_SUCCESS)
		{
		copy->ping = (struct mbsys_simrad2_ping_struct *) ping_save;
		ping_store = (struct mbsys_simrad2_ping_struct *) store->ping;
		ping_copy = (struct mbsys_simrad2_ping_struct *) copy->ping;
		*ping_copy = *ping_store;
		}
	else
		copy->ping = NULL;
	
	/* if needed copy the attitude data structure */
	if (store->attitude != NULL && status == MB_SUCCESS)
		{
		copy->attitude = (struct mbsys_simrad2_attitude_struct *) attitude_save;
		attitude_store = (struct mbsys_simrad2_attitude_struct *) store->attitude;
		attitude_copy = (struct mbsys_simrad2_attitude_struct *) copy->attitude;
		*attitude_copy = *attitude_store;
		}
	
	/* if needed copy the heading data structure */
	if (store->heading != NULL && status == MB_SUCCESS)
		{
		copy->heading = (struct mbsys_simrad2_heading_struct *) heading_save;
		heading_store = (struct mbsys_simrad2_heading_struct *) store->heading;
		heading_copy = (struct mbsys_simrad2_heading_struct *) copy->heading;
		*heading_copy = *heading_store;
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
