/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad2.c	3.00	10/9/98
 *	$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $
 *
 *    Copyright (c) 1998, 2001 by
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
 * mbsys_simrad2.c contains the MBIO functions for handling data from 
 * new (post-1997) Simrad multibeam sonars (e.g. EM120, EM300, EM3000).
 * The data formats associated with Simrad multibeams 
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
 * Date:	October 9, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.9  2001/08/25  00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.8  2001/08/04  01:00:02  caress
 * Applied mods from Gordon Keith.
 *
 * Revision 5.7  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.6  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.5  2001/06/01  00:14:06  caress
 * Redid support for current Simrad multibeam data.
 *
 * Revision 5.4  2001/05/30  17:57:26  caress
 * Fixed New Simrad data handling, plus removed use of
 * intermediate data structure. Still need to reduce use
 * of #ifdefs related to byteswapping.
 *
 * Revision 5.3  2001/05/24  23:18:07  caress
 * Fixed handling of Revelle EM120 data (first cut).
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
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
int mbsys_simrad2_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $";
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
	strcpy(store->par_p1g, "WGS_84");
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

	/* pointer to ssv data structure */
	store->ssv = NULL;

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
			void *mbio_ptr, void *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $";
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
		ping->png_latitude = EM2_INVALID_INT;
				/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
		ping->png_longitude = EM2_INVALID_INT;
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
		    
		/* raw beam record */
		ping->png_raw_read = MB_NO;	
				/* flag indicating actual reading of rawbeam record */
		ping->png_nrawbeams = 0;	
				/* number of raw travel times and angles
				    - nonzero only if raw beam record read */
		for (i=0;i<MBSYS_SIMRAD2_MAXBEAMS;i++)
		    {
		    ping->png_rawpointangle[i] = 0;
				/* Raw beam pointing angles in 0.01 degree,
					positive to port. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
		    ping->png_rawtiltangle[i] = 0;
				/* Raw transmit tilt angles in 0.01 degree,
					positive forward. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
		    ping->png_rawrange[i] = 0;
				/* Ranges as raw two way travel times in time 
					units defined as one-fourth the inverse 
					sampling rate. These values have not 
					been corrected for changes in the
					heave during the ping cycle. */
		    ping->png_rawamp[i] = 0;		
				/* 0.5 dB */
		    ping->png_rawbeam_num[i] = 0;	
				/* beam 128 is first beam on 
				    second head of EM3000D */
		    }
	
		/* sidescan */
		ping->png_ss_read = MB_NO;	
				/* flag indicating actual reading of sidescan record */
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
			void *mbio_ptr, void *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $";
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
			void *mbio_ptr, void *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $";
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
int mbsys_simrad2_ssv_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error)
{
 static char res_id[]="$Id: mbsys_simrad2.c,v 5.10 2001-09-12 19:27:57 caress Exp $";
	char	*function_name = "mbsys_simrad2_ssv_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ssv_struct *ssv;
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
	if (store->ssv == NULL)
		status = mb_malloc(verbose,
			sizeof(struct mbsys_simrad2_ssv_struct),
			&(store->ssv),error);
			
	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		ssv = (struct mbsys_simrad2_ssv_struct *) store->ssv;

		/* initialize everything */
		ssv->ssv_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ssv->ssv_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		ssv->ssv_count = 0;	
				/* sequential counter or input identifier */
		ssv->ssv_serial = 0;	
				/* system 1 or system 2 serial number */
		ssv->ssv_ndata = 0;	
				/* number of ssv data */
		for (i=0;i<MBSYS_SIMRAD2_MAXSSV;i++)
		    {
		    ssv->ssv_time[i] = 0;
				/* time since record start (msec) */
		    ssv->ssv_ssv[i] = 0;
				/* ssv (0.1 m/s) */
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
int mbsys_simrad2_deall(int verbose, void *mbio_ptr, void **store_ptr, 
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

	/* deallocate memory for ssv data structure */
	if (store->ssv != NULL)
		status = mb_free(verbose,&(store->ssv),error);

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
int mbsys_simrad2_zero_ss(int verbose, void *store_ptr, int *error)
{
	char	*function_name = "mbsys_simrad2_zero_ss";
	int	status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to data descriptor */
	store = (struct mbsys_simrad2_struct *) store_ptr;
	if (store != NULL)
	    ping = (struct mbsys_simrad2_ping_struct *) store->ping;

	/* initialize all sidescan stuff to zeros */
	if (store->ping != NULL)
		{
		ping->png_ss_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
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
		ping->png_npixels = 0;	
				/* number of pixels of sidescan */
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
				/* the sidescan ordered port to starboard */
		    }
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
		    {
		    ping->png_ss[i] = 0;
				/* the sidescan ordered port to starboard */
		    ping->png_ssalongtrack[i] = 0;
				/* the sidescan ordered port to starboard */
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
int mbsys_simrad2_extract(int verbose, void *mbio_ptr, void *store_ptr, 
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
	double	dacrscale, daloscale, reflscale;
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
		if (ping->png_longitude != EM2_INVALID_INT)
		    *navlon = 0.0000001 * ping->png_longitude;
		else
		    *navlon = 0.0;
		if (ping->png_latitude != EM2_INVALID_INT)
		    *navlat = 0.00000005 * ping->png_latitude;
		else
		    *navlat = 0.0;
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
			
		/* set beamwidths in mb_io structure */
		if (store->run_tran_beam > 0)
		    mb_io_ptr->beamwidth_ltrack 
			= 0.1 * store->run_tran_beam;
		else if (ping->png_tx > 0)
		    {
		    mb_io_ptr->beamwidth_ltrack 
			= 0.1 * ping->png_tx;
		    }
		else if (store->sonar == MBSYS_SIMRAD2_EM120)
		    mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM300)
		    mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM1002)
		    mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM2000)
		    mb_io_ptr->beamwidth_ltrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_1 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_2 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_3 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_4 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_5 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_6 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_7)
		    mb_io_ptr->beamwidth_ltrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM1000)
		    mb_io_ptr->beamwidth_ltrack = 3.3;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S
			|| store->sonar == MBSYS_SIMRAD2_EM12D)
		    mb_io_ptr->beamwidth_ltrack = 1.7;
		else if (store->sonar == MBSYS_SIMRAD2_EM121)
		    {
		    mb_io_ptr->beamwidth_ltrack = 1.0;
		    }
		if (store->run_rec_beam > 0)
		    {
		    mb_io_ptr->beamwidth_xtrack 
			= 0.1 * store->run_rec_beam;
		    }
		else if (store->sonar == MBSYS_SIMRAD2_EM120)
		    mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM300)
		    mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM1002)
		    mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM2000)
		    mb_io_ptr->beamwidth_xtrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_1 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_2 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_3 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_4 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_5 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_6 
			|| store->sonar == MBSYS_SIMRAD2_EM3000D_7)
		    mb_io_ptr->beamwidth_xtrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM1000)
		    mb_io_ptr->beamwidth_xtrack = 3.3;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S
			|| store->sonar == MBSYS_SIMRAD2_EM12D)
		    mb_io_ptr->beamwidth_xtrack = 3.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM121)
		    mb_io_ptr->beamwidth_xtrack = mb_io_ptr->beamwidth_ltrack;

		/* read distance and depth values into storage arrays */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		dacrscale  = 0.01 * ping->png_distance_res;
		daloscale  = 0.01 * ping->png_distance_res;
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
			}
		*nbath = ping->png_nbeams_max;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD2_MAXPIXELS;
		pixel_size = 0.01 * ping->png_pixel_size;
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
			{
			ss[i] = 0.01 * ping->png_ss[i];
			ssacrosstrack[i] = pixel_size 
					* (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
			ssalongtrack[i] = daloscale * ping->png_ssalongtrack[i];
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
			fprintf(stderr,"dbg4        nss:      %d\n",
				*nss);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
			}

		/* done translating values */

		}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV)
		{
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
		if (store->pos_longitude != EM2_INVALID_INT)
		    *navlon = 0.0000001 * store->pos_longitude;
		else
		    *navlon = 0.0;
		if (store->pos_latitude != EM2_INVALID_INT)
		    *navlat = 0.00000005 * store->pos_latitude;
		else
		    *navlat = 0.0;
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
		*heading = 0.01 * store->pos_heading;

		/* get speed  */
		if (store->pos_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

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
int mbsys_simrad2_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
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
	double	depthscale, dacrscale,daloscale,reflscale;
	double	depthoffset;
	double	depthmax, distancemax;
	int	png_depth_res, png_distance_res;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

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
		ping->png_heading = (int) rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int) rint(speed / 0.036);		

		/* get guesses at sonar and resolutions if needed  */
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
			
		/* set initial values for resolutions */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		dacrscale  = 0.01 * ping->png_distance_res;
		daloscale  = 0.01 * ping->png_distance_res;
		reflscale  = 0.5;
		
		/* Figure out depth and distance scaling on
		 * the fly. Using the existing scaling got us
		 * into trouble with Revelle data in August-September 2001.
		 * Use calculated values only if needed to fit
		 * new depths into short int's.
		 */
		if (status == MB_SUCCESS)
			{
			/* get max depth and distance values */
			depthmax = 0.0;
			distancemax = 0.0;
			for (i=0;i<nbath;i++)
			    {
			    if (beamflag[i] != MB_FLAG_NULL)
				{
				depthmax = MAX(depthmax, fabs(bath[i] - depthoffset));
				distancemax = MAX(depthmax, fabs(bathacrosstrack[i]));
				}
			    }
			    
			/* figure out best scaling */
			if (store->sonar == MBSYS_SIMRAD2_EM120
				|| store->sonar == MBSYS_SIMRAD2_EM300)
				png_depth_res = (int)(depthmax / 655.36) + 1;
			else
				png_depth_res = (int)(depthmax / 327.68) + 1;
			png_distance_res = (int)(distancemax / 327.68) + 1;
				
			/* Change scaling if needed */
			if (png_depth_res > ping->png_depth_res)
				{
				ping->png_depth_res = png_depth_res;
				depthscale = 0.01 * ping->png_depth_res;
				}
			if (png_distance_res > ping->png_distance_res)
				{
				ping->png_distance_res = png_distance_res;
				dacrscale  = 0.01 * ping->png_distance_res;
				daloscale  = 0.01 * ping->png_distance_res;
				}
			}

		if (status == MB_SUCCESS && ping->png_nbeams == 0)
			{
			for (i=0;i<nbath;i++)
			    if (beamflag[i] != MB_FLAG_NULL)
				{
				j = ping->png_nbeams;
				ping->png_beam_num[j] = i + 1;
				ping->png_depth[j] = (int) rint((bath[i] - depthoffset)
							/ depthscale);
				ping->png_beamflag[j] = beamflag[i];
				ping->png_acrosstrack[j]
				        = (int) rint(bathacrosstrack[i] / dacrscale);
				ping->png_alongtrack[j] 
					= (int) rint(bathalongtrack[i] / daloscale);
				if (amp[i] != 0.0)
					ping->png_amp[j] = (int) rint((amp[i] - 64) 
						/ reflscale);
				else
					ping->png_amp[j] = 0;
				ping->png_nbeams++;
				}
			ping->png_nbeams_max = nbath;
			}
		else if (status == MB_SUCCESS)
			{
			for (j=0;j<ping->png_nbeams;j++)
				{
				i = ping->png_beam_num[j] - 1;
				ping->png_depth[j] = (int) rint((bath[i] - depthoffset)
							/ depthscale);
				ping->png_beamflag[j] = beamflag[i];
				ping->png_acrosstrack[j]
					= (int) rint(bathacrosstrack[i] / dacrscale);
				ping->png_alongtrack[j] 
					= (int) rint(bathalongtrack[i] / daloscale);
				if (amp[i] != 0.0)
					ping->png_amp[j] = (int) rint((amp[i] - 64) 
						/ reflscale);
				else
					ping->png_amp[j] = 0;
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<nss;i++)
				{
				ping->png_ss[i] = (int) rint(100 * ss[i]);
				ping->png_ssalongtrack[i] = (int) rint(ssalongtrack[i] / daloscale);
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
		store->pos_heading = (int) rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int) rint(speed / 0.036);

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
int mbsys_simrad2_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
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
		if (store->sonar == MBSYS_SIMRAD2_EM120 
		    || store->sonar == MBSYS_SIMRAD2_EM300 
		    || store->sonar == MBSYS_SIMRAD2_EM1002 
		    || store->sonar == MBSYS_SIMRAD2_EM2000 
		    || store->sonar == MBSYS_SIMRAD2_EM3000)
		    ttscale = 0.5 / ping->png_sample_rate;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_2 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_3 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_4 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_5 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_6 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_7)
		    ttscale = 0.5 / 14000;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S
		    || store->sonar == MBSYS_SIMRAD2_EM12D
		    || store->sonar == MBSYS_SIMRAD2_EM121
		    || store->sonar == MBSYS_SIMRAD2_EM1000)
		    {
		    ttscale = 1.0 / ping->png_sample_rate;
		    }
		*nbeams = ping->png_nbeams_max;
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
			if (store->sonar == MBSYS_SIMRAD2_EM120 
				|| store->sonar == MBSYS_SIMRAD2_EM300 
				|| store->sonar == MBSYS_SIMRAD2_EM1002 
				|| store->sonar == MBSYS_SIMRAD2_EM2000 
				|| store->sonar == MBSYS_SIMRAD2_EM3000
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_1 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_2 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_3 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_4 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_5 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_6 
				|| store->sonar == MBSYS_SIMRAD2_EM3000D_7)
			    angles_null[i] = angles[i];
			else if (store->sonar == MBSYS_SIMRAD2_EM1000)
			    angles_null[i] = angles[i];
			else if (store->sonar == MBSYS_SIMRAD2_EM12S
				|| store->sonar == MBSYS_SIMRAD2_EM12D
				|| store->sonar == MBSYS_SIMRAD2_EM121)
			    angles_null[i] = 0.0;
			heave[j] = heave_use;
			alongtrack_offset[j] = 0.0;
			}
		
		/* reset null angles for EM1000 outer beams */
		if (store->sonar == MBSYS_SIMRAD2_EM1000
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
int mbsys_simrad2_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_simrad2_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	double	depthscale, dacrscale;
	double	bath_best;
	double	xtrack_min;
	int	found;
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
		found = MB_NO;
		bath_best = 0.0;
		xtrack_min = 99999999.9;
		for (i=0;i<ping->png_nbeams;i++)
		    {
		    if (mb_beam_ok(ping->png_beamflag[i])
			&& fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min)
			{
			xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
			bath_best = depthscale * ping->png_depth[i];
			found = MB_YES;
			}
		    }		
		if (found == MB_NO)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<ping->png_nbeams;i++)
			{
			if (ping->png_quality[i] > 0
			    && fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
			    bath_best = depthscale * ping->png_depth[i];
			    found = MB_YES;
			    }
			}		
		    }
		if (found == MB_YES)
		    *altitude = bath_best - *transducer_depth;
		else
		    *altitude = 0.0;

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
int mbsys_simrad2_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
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
		if (ping->png_longitude != EM2_INVALID_INT)
		    *navlon = 0.0000001 * ping->png_longitude;
		else
		    *navlon = 0.0;
		if (ping->png_latitude != EM2_INVALID_INT)
		    *navlat = 0.00000005 * ping->png_latitude;
		else
		    *navlat = 0.0;
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
		if (store->pos_longitude != EM2_INVALID_INT)
		    *navlon = 0.0000001 * store->pos_longitude;
		else
		    *navlon = 0.0;
		if (store->pos_latitude != EM2_INVALID_INT)
		    *navlat = 0.00000005 * store->pos_latitude;
		else
		    *navlat = 0.0;
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
int mbsys_simrad2_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
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
		ping->png_heading = (int) rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int) rint(speed / 0.036);

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
		store->pos_heading = (int) rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int) rint(speed / 0.036);

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
int mbsys_simrad2_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad2_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
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
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;
		
		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = 0.01 * store->svp_depth_res * store->svp_depth[i];
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
int mbsys_simrad2_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad2_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_SIMRAD2_MAXSVP);
		store->svp_depth_res = 1;
		
		/* get profile */
		for (i=0;i<store->svp_num;i++)
			{
			store->svp_depth[i] = (int) (100 * depth[i] / store->svp_depth_res);
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
int mbsys_simrad2_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
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
	struct mbsys_simrad2_ssv_struct *ssv_store;
	struct mbsys_simrad2_ssv_struct *ssv_copy;
	char	*ssv_save;

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
	
	/* check if ssv data needs to be copied */
	if (store->ssv != NULL)
		{
		/* make sure a ssv data structure exists to
			be copied into */
		if (copy->ssv == NULL)
			{
			status = mbsys_simrad2_ssv_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}
			
		/* save pointer value */
		ssv_save = (char *)copy->ssv;
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
	
	/* if needed copy the ssv data structure */
	if (store->ssv != NULL && status == MB_SUCCESS)
		{
		copy->ssv = (struct mbsys_simrad2_ssv_struct *) ssv_save;
		ssv_store = (struct mbsys_simrad2_ssv_struct *) store->ssv;
		ssv_copy = (struct mbsys_simrad2_ssv_struct *) copy->ssv;
		*ssv_copy = *ssv_store;
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
int mbsys_simrad2_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size, 
		int swath_width_set, double *swath_width, 
		int pixel_int, 
		int *error)
{
	char	*function_name = "mbsys_simrad2_makess";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	double	ss[MBSYS_SIMRAD2_MAXPIXELS];
	int	ss_cnt[MBSYS_SIMRAD2_MAXPIXELS];
	double	ssacrosstrack[MBSYS_SIMRAD2_MAXPIXELS];
	double	ssalongtrack[MBSYS_SIMRAD2_MAXPIXELS];
	mb_s_char *beam_ss;
	int	nbathsort;
	double	bathsort[MBSYS_SIMRAD2_MAXBEAMS];
	double	depthscale, depthoffset;
	double	dacrscale, daloscale;
	double	reflscale;
	double	ssoffset;
	double  pixel_size_calc;
	double	ss_spacing, ss_spacing_use;
	int	pixel_int_use;
	double	angle, depth, xtrack, ltrack, xtrackss;
	double	range, beam_foot, beamwidth, sint;
	int	first, last, k1, k2;
	int	i, j, k, jj, kk, l, m;

	/* compare function for qsort */
	int mb_double_compare();

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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get pointer to raw data structure */
		ping = (struct mbsys_simrad2_ping_struct *) store->ping;

		/* zero the sidescan */
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
			}

		/* set scaling parameters */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth
				+ 655.36 * ping->png_offset_multiplier;
		dacrscale  = 0.01 * ping->png_distance_res;
		daloscale  = 0.01 * ping->png_distance_res;
		reflscale  = 0.5;
		ssoffset = 64.0;
		if (store->sonar == MBSYS_SIMRAD2_EM300
		    && store->run_mode == 4)
		    {
		    if (depthscale * ping->png_depth[ping->png_nbeams/2] > 3500.0
			&& ping->png_max_range > 19000
			&& ping->png_bsn + ping->png_bso < -60)
			{
			ssoffset = 64.0 - 0.6 * (ping->png_bsn + ping->png_bso + 60);
			}
		    }

		/* get raw pixel size */
		if (store->sonar == MBSYS_SIMRAD2_EM120 
		    || store->sonar == MBSYS_SIMRAD2_EM300 
		    || store->sonar == MBSYS_SIMRAD2_EM1002 
		    || store->sonar == MBSYS_SIMRAD2_EM2000 
		    || store->sonar == MBSYS_SIMRAD2_EM3000)
		    ss_spacing = 750.0 / ping->png_sample_rate;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_2 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_3 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_4 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_5 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_6 
		    || store->sonar == MBSYS_SIMRAD2_EM3000D_7)
		    ss_spacing = 750.0 / 14000;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S
		    || store->sonar == MBSYS_SIMRAD2_EM12D
		    || store->sonar == MBSYS_SIMRAD2_EM121
		    || store->sonar == MBSYS_SIMRAD2_EM1000)
		    {
		    ss_spacing = 0.01 * ping->png_max_range;
		    }

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD2_EM1000)
		    {
		    beamwidth = 2.5;
		    }
		else
		    {
		    beamwidth = 0.1 * ((double) ping->png_tx);
		    }

		/* get median depth */
		nbathsort = 0;
		for (i=0;i<ping->png_nbeams;i++)
		    {
		    if (mb_beam_ok(ping->png_beamflag[i]))
			{
			bathsort[nbathsort] = depthscale 
				* ping->png_depth[i]
				    + depthoffset;
			nbathsort++;
			}
		    }
	
		/* get sidescan pixel size */
		if (swath_width_set == MB_NO
		    && nbathsort > 0)
		    {
		    (*swath_width) = 2.5 + MAX(90.0 - 0.01 * ping->png_depression[0], 
				    90.0 - 0.01 * ping->png_depression[ping->png_nbeams-1]);
		    (*swath_width) = MAX((*swath_width), 60.0);
		    }
		if (pixel_size_set == MB_NO
		    && nbathsort > 0)
		    {
		    qsort((char *)bathsort, nbathsort, sizeof(double),mb_double_compare);
		    pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort/2] 
					/ MBSYS_SIMRAD2_MAXPIXELS;
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

		/* loop over raw sidescan, putting each raw pixel into
			the binning arrays */
		for (i=0;i<ping->png_nbeams_ss;i++)
			{
			beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
			j = ping->png_beam_num[i] - 1;
			if (mb_beam_ok(ping->png_beamflag[i]))
			    {
			    if (ping->png_beam_samples[i] > 0)
				{
				depth = depthscale * ping->png_depth[i];
				xtrack = dacrscale * ping->png_acrosstrack[i];
				ltrack = daloscale * ping->png_alongtrack[i];
				range = sqrt(depth * depth + xtrack * xtrack);
				angle = 90.0 - 0.01 * ping->png_depression[i];
				beam_foot = range * sin(DTR * beamwidth)
							/ cos(DTR * angle);
				sint = fabs(sin(DTR * angle));
				if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
				    ss_spacing_use = beam_foot / ping->png_beam_samples[i];
				else
				    ss_spacing_use = ss_spacing / sint;
/*fprintf(stderr, "spacing: %f %f n:%d sint:%f angle:%f range:%f foot:%f factor:%f\n", 
ss_spacing, ss_spacing_use, 
ping->png_beam_samples[i], sint, angle, range, beam_foot, 
ping->png_beam_samples[i] * ss_spacing / beam_foot);*/
				}
			    for (k=0;k<ping->png_beam_samples[i];k++)
				{
				if (beam_ss[k] != EM2_INVALID_AMP)
					{
					/* locate based on range */
					if (k == ping->png_center_sample[i])
					    {
					    xtrackss = xtrack;
					    }
					else if (i == ping->png_nbeams_ss - 1 
					    || (k <= ping->png_center_sample[i]
						&& i != 0))
					    {
					    if (ping->png_range[i] != ping->png_range[i-1])
						{
						jj = ping->png_beam_num[i-1] - 1;
						xtrackss = dacrscale * ping->png_acrosstrack[i]
						    + (dacrscale * ping->png_acrosstrack[i]
							- dacrscale * ping->png_acrosstrack[i-1])
						    * 2 *((double)(k - ping->png_center_sample[i]))
						    / fabs((double)(ping->png_range[i] - ping->png_range[i-1]));
						}
					    else
						{
						xtrackss = xtrack
						    + ss_spacing_use * (k - ping->png_center_sample[i]);
						}
					    }
					else
					    {
					    if (ping->png_range[i] != ping->png_range[i+1])
						{
						jj = ping->png_beam_num[i+1] - 1;
						    
						xtrackss = dacrscale * ping->png_acrosstrack[i]
						    + (dacrscale * ping->png_acrosstrack[i+1]
							- dacrscale * ping->png_acrosstrack[i])
						    * 2 *((double)(k - ping->png_center_sample[i]))
						    / fabs((double)(ping->png_range[i+1] - ping->png_range[i]));
						}
					    else
						{
						xtrackss = xtrack
						    + ss_spacing_use * (k - ping->png_center_sample[i]);
						}
					    }
					xtrackss = xtrack
						    + ss_spacing_use * (k - ping->png_center_sample[i]);
					kk = MBSYS_SIMRAD2_MAXPIXELS / 2 
					    + (int)(xtrackss / (*pixel_size));
					if (kk > 0 && kk < MBSYS_SIMRAD2_MAXPIXELS)
					    {
					    ss[kk]  += reflscale*((double)beam_ss[k]) + ssoffset;
					    ssalongtrack[kk] 
						    += daloscale * ping->png_alongtrack[i];
					    ss_cnt[kk]++;
					    }
					}
				}
			    }
			}
			
		/* average the sidescan */
		first = MBSYS_SIMRAD2_MAXPIXELS;
		last = -1;
		for (k=0;k<MBSYS_SIMRAD2_MAXPIXELS;k++)
			{
			if (ss_cnt[k] > 0)
				{
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] 
					= (k - MBSYS_SIMRAD2_MAXPIXELS / 2)
						* (*pixel_size);
				first = MIN(first, k);
				last = k;
				}
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
				    = (k - MBSYS_SIMRAD2_MAXPIXELS / 2)
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
		ping->png_pixel_size = (int) (100 * (*pixel_size));
		if (last > first)
		    ping->png_pixels_ss = MBSYS_SIMRAD2_MAXPIXELS;
		else 
		    ping->png_pixels_ss = 0;
		for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
		    {
		    ping->png_ss[i] = (short) rint(100 * ss[i]);
		    ping->png_ssalongtrack[i] 
			    = (short) rint(ssalongtrack[i] / daloscale);
		    }

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Sidescan regenerated in <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       png_nbeams_ss: %d\n",
				ping->png_nbeams_ss);
			for (i=0;i<ping->png_nbeams_ss;i++)
			  fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				ping->png_beam_num[i],
				ping->png_beamflag[i],
				ping->png_depth[i],
				ping->png_amp[i],
				ping->png_acrosstrack[i],
				ping->png_alongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				MBSYS_SIMRAD2_MAXPIXELS);
			for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n",
				i,ss_cnt[i],ss[i],
				ssacrosstrack[i],
				ssalongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				ping->png_pixels_ss);
			for (i=0;i<MBSYS_SIMRAD2_MAXPIXELS;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n",
				i,ping->png_ss[i],ping->png_ssalongtrack[i]);
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
