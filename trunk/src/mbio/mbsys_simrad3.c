/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad3.c	3.00	2/22/2008
 *	$Id$
 *
 *    Copyright (c) 2008-2013 by
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
 * mbsys_simrad3.c contains the MBIO functions for handling data from
 * new (post-2006) Simrad multibeam sonars (e.g. EM710, EM3002, EM302, EM122).
 * The data formats associated with Simrad multibeams
 * (both old and new) include:
 *    MBSYS_SIMRAD formats (code in mbsys_simrad.c and mbsys_simrad.h):
 *      MBF_EMOLDRAW : MBIO ID 51 - Vendor EM1000, EM12S, EM12D, EM121
 *                   : MBIO ID 52 - aliased to 51
 *      MBF_EM12IFRM : MBIO ID 53 - IFREMER EM12S and EM12D
 *      MBF_EM12DARW : MBIO ID 54 - NERC EM12S
 *                   : MBIO ID 55 - aliased to 51
 *    MBSYS_SIMRAD3 formats (code in mbsys_simrad2.c and mbsys_simrad2.h):
 *      MBF_EM300RAW : MBIO ID 56 - Vendor EM3000, EM300, EM120
 *      MBF_EM300MBA : MBIO ID 57 - MBARI EM3000, EM300, EM120 for processing
 *    MBSYS_SIMRAD3 formats (code in mbsys_simrad3.c and mbsys_simrad3.h):
 *      MBF_EM710RAW : MBIO ID 58 - Vendor EM710
 *      MBF_EM710MBA : MBIO ID 59 - MBARI EM710 for processing
 *
 * Author:	D. W. Caress
 * Date:	February 22, 2008
 *
 * $Log: mbsys_simrad3.c,v $
 * Revision 5.4  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.3  2009/02/06 19:12:43  caress
 * Fixed description in mb_angle.c and angle extraction in mb_simrad3.c
 *
 * Revision 5.2  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.1  2008/07/10 06:40:34  caress
 * Fixed support for EM122
 *
 * Revision 5.0  2008/03/01 09:11:35  caress
 * Added support for Simrad EM710 multibeam in new formats 58 and 59.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_simrad3.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_simrad3_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad3_struct),
				(void **)store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) *store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->type = EM3_NONE;
	store->sonar = MBSYS_SIMRAD3_UNKNOWN;

	/* time stamp */
	store->date = 0;
	store->msec = 0;
	store->sts_date = 0;	/* status date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	store->sts_msec = 0;	/* status time since midnight in msec
				    08:12:51.234 = 29570234 */
	store->sts_status_count = 0; 	/* status datagram counter */
	store->sts_serial = 0;		/* system 1 or 2 serial number */
	store->sts_pingrate = 0;		/* ping rate (0.01 Hz) */
	store->sts_ping_count = 0;		/* ping counter - latest ping */
	store->sts_load = 0;		/* processing unit load (%) */
	store->sts_udp_status = 0;		/* sensor input status, UDP port 2 */
	store->sts_serial1_status = 0;	/* sensor input status, serial port 1 */
	store->sts_serial2_status = 0;	/* sensor input status, serial port 2 */
	store->sts_serial3_status = 0;	/* sensor input status, serial port 3 */
	store->sts_serial4_status = 0;	/* sensor input status, serial port 4 */
	store->sts_pps_status = 0;		/* sensor input status, pps, >0 ok */
	store->sts_position_status = 0;	/* sensor input status, position, >0 ok */
	store->sts_attitude_status = 0;	/* sensor input status, attitude, >0 ok */
	store->sts_clock_status = 0;	/* sensor input status, clock, >0 ok */
	store->sts_heading_status = 0;	/* sensor input status, heading, >0 ok */
	store->sts_pu_status = 0;		/* sensor input status, processing unit
						(0=off, 1-on, 2=simulator) */
	store->sts_last_heading = 0;	/* last received heading (0.01 deg) */
	store->sts_last_roll = 0;		/* last received roll (0.01 deg) */
	store->sts_last_pitch = 0;		/* last received pitch (0.01 deg) */
	store->sts_last_heave = 0;		/* last received heave (0.01 m) */
	store->sts_last_ssv = 0;		/* last received sound speed (0.1 m/s) */
	store->sts_last_depth = 0;		/* last received depth (0.01 m) */
	store->sts_spare = 0;		/* spare */
	store->sts_bso = 0;		/* backscatter at oblique angle (dB) */
	store->sts_bsn = 0;		/* backscatter at normal incidence (dB) */
	store->sts_gain = 0;		/* fixed gain (dB) */
	store->sts_dno = 0;		/* depth to normal incidence (m) */
	store->sts_rno = 0;		/* range to normal incidence (m) */
	store->sts_port = 0;		/* port coverange (deg) */
	store->sts_stbd = 0;		/* starboard coverange (deg) */
	store->sts_ssp = 0;		/* sound speed at transducer from profile (0.1 m/s) */
	store->sts_yaw = 0;		/* yaw stabilization (0.01 deg) */
	store->sts_port2 = 0;		/* port coverange for second em3002 head (deg) */
	store->sts_stbd2 = 0;		/* starboard coverange for second em3002 head (deg) */
	store->sts_spare2 = 0;		/* spare */

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
	for (i=0;i<MBSYS_SIMRAD3_COMMENT_LENGTH;i++)
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
	for (i=0;i<MBSYS_SIMRAD3_MAXSVP;i++)
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
	store->pos_heave = 0;	/* heave from interpolation (0.01 m) */
	store->pos_roll = 0;	/* roll from interpolation (0.01 deg) */
	store->pos_pitch = 0;	/* pitch from interpolation (0.01 deg) */
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

        /* pointer to extra parameters data structure */
        store->extraparameters = NULL;

	/* pointer to attitude data structure */
	store->attitude = NULL;

	/* pointer to network attitude data structure */
	store->netattitude = NULL;

	/* pointer to heading data structure */
	store->heading = NULL;

	/* pointer to ssv data structure */
	store->ssv = NULL;

	/* pointer to tilt data structure */
	store->tilt = NULL;

	/* pointer to survey data structure */
	store->ping = NULL;

	/* pointer to water column data structure */
	store->wc = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbsys_simrad3_survey_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_survey_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->ping == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_ping_struct),
			(void **)&(store->ping),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* initialize everything */
		ping->png_date = 0;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ping->png_msec = 0;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		ping->png_count = 0;	/* sequential counter or input identifier */
		ping->png_serial = 0;	/* system 1 or system 2 serial number */
		ping->png_latitude = 0;	/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere)
				    if valid, invalid = 0x7FFFFFFF */
		ping->png_longitude = 0;	/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere)
				    if valid, invalid = 0x7FFFFFFF */
		ping->png_heading = 0;	/* heading (0.01 deg) */
		ping->png_heave = 0;	/* heave from interpolation (0.01 m) */
		ping->png_roll = 0;	/* roll from interpolation (0.01 deg) */
		ping->png_pitch = 0;	/* pitch from interpolation (0.01 deg) */
		ping->png_speed = 0;	/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
		ping->png_ssv = 0;	/* sound speed at transducer (0.1 m/sec) */
		ping->png_xducer_depth = 0.0;
				/* transmit transducer depth (m)
					The transmit transducer depth should be
					added to the beam depths to derive the
					depths re the water line. Note that the
					transducer depth will be negative if the
					actual heave is large enough to bring the
					transmit transducer above the water line.
					This may represent a valid situation, but
					may also be due to an erroneously set
					installation depth of either the transducer
					or the water line. */

		ping->png_nbeams = 0;	/* maximum number of beams possible */
		ping->png_nbeams_valid = 0;	/* number of valid beams */
		ping->png_sample_rate = 0.0; /* sampling rate (Hz) */
		ping->png_spare = 0; /* sampling rate (Hz) */
		for (i=0;i<MBSYS_SIMRAD3_MAXBEAMS;i++)
			{
			ping->png_depth[i] = 0.0;
					/* depths relative to sonar (m)
						The beam data are given re the transmit
						transducer or sonar head depth and the
						horizontal location (x,y) of the active
						positioning systemÕs reference point.
						Heave, roll, pitch, sound speed at the
						transducer depth and ray bending through
						the water column have been applied. */
			ping->png_acrosstrack[i] = 0.0;
					/* acrosstrack distances (m) */
			ping->png_alongtrack[i] = 0.0;
					/* alongtrack distances (m) */
			ping->png_window[i] = 0;
					/* samples */
			ping->png_quality[i] = 0;
					/* 0-254 Scaled standard deviation (sd) of the
						range detection divided by
						the detected range (dr):
						Quality factor = 250*sd/dr. */
			ping->png_iba[i] = 0;
					/* beam incidence angle adjustment (IBA) (0.1 deg)
						Due to raybending, the beam incidence angle at the bottom hit
						will usually differ from the beam launch angle at the transducer
						and also from the angle given by a straight line between the
						transducer and the bottom hit. The difference from the latter is
						given by the beam incidence angle adjustment (IBA). The beam
						incidence angle re the horizontal, corrected for the ray bending,
						can be calculated as follows:
							BAC = atan( z / abs(y) ) + IBA.
						BAC is positive downwards and IBA will be positive when the
						beam is bending towards the bottom. This parameter can be
						helpful for correcting seabed imagery data and in seabed
						classification. */
			ping->png_detection[i] = 0;
					/* Detection info:
					   This datagram may contain data for beams with and without a
					   valid detection. Eight bits (0-7) gives details about the detection:
						A) If the most significant bit (bit7) is zero, this beam has a valid
							detection. Bit 0-3 is used to specify how the range for this beam
							is calculated
							0: Amplitude detect
							1: Phase detect
							2-15: Future use
						B) If the most significant bit is 1, this beam has an invalid
							detection. Bit 4-6 is used to specify how the range (and x,y,z
							parameters) for this beam is calculated
							0: Normal detection
							1: Interpolated or extrapolated from neighbour detections
							2: Estimated
							3: Rejected candidate
							4: No detection data is available for this beam (all parameters
								are set to zero)
							5-7: Future use
						The invalid range has been used to fill in amplitude samples in
						the seabed image datagram. */
			ping->png_clean[i] = 0;
					/* realtime cleaning info:
						For future use. A real time data cleaning module may flag out
						beams. Bit 7 will be set to 1 if the beam is flagged out. Bit 0-6
						will contain a code telling why the beam is flagged out. */
			ping->png_amp[i] = 0;
					/* 0.5 dB */
			ping->png_beamflag[i] = 0;
					/* uses standard MB-System beamflags */
			ping->png_depression[i] = 0.0;
				/* beam depression angles (deg) */
			ping->png_azimuth[i] = 0.0;
				/* beam azimuth angles (deg) */
			ping->png_range[i] = 0.0;
				/* Two-way travel times (sec). */
			ping->png_bheave[i] = 0.0;
				/* Average of heave at transmit and receive time for each beam */
			}

		/* raw travel time and angle data version 4 */
		ping->png_raw4_read = 0; /* flag indicating actual reading of raw beam record */
		ping->png_raw_date = 0;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ping->png_raw_msec = 0;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		ping->png_raw_count = 0;	/* sequential counter or input identifier */
		ping->png_raw_serial = 0;	/* system 1 or system 2 serial number */
		ping->png_raw_ssv = 0;		/* sound speed at transducer (0.1 m/sec) */
		ping->png_raw_ntx = 0;		/* number of TX pulses (1 to 9) */
		ping->png_raw_nbeams = 0;		/* number of raw travel times and angles
					    - nonzero only if raw beam record read */
		ping->png_raw_detections = 0;	/* number of valid detections */
		ping->png_raw_sample_rate = 0.0;	/* sampling rate (Hz) */
		ping->png_raw_spare = 0;
		for (i=0;i<MBSYS_SIMRAD3_MAXTX;i++)
			{
			ping->png_raw_txtiltangle[i] = 0;/* tilt angle (0.01 deg) */
			ping->png_raw_txfocus[i] = 0;   /* focus range (0.1 m)
									0 = no focus */
			ping->png_raw_txsignallength[i] = 0.0;	/* signal length (sec) */
			ping->png_raw_txoffset[i] = 0.0;	/* transmit time offset (sec) */
			ping->png_raw_txcenter[i] = 0.0;	/* center frequency (Hz) */
			ping->png_raw_txabsorption[i] = 0;	/* mean absorption coeff. (0.01 dB/km) */

			ping->png_raw_txwaveform[i] = 0;	/* signal waveform identifier
										0 = CW, 1 = FM upsweep, 2 = FM downsweep */
			ping->png_raw_txsector[i] = 0;	/* transmit sector number (0-19) */
			ping->png_raw_txbandwidth[i] = 0.0;	/* bandwidth (Hz) */
			}
		for (i=0;i<MBSYS_SIMRAD3_MAXBEAMS;i++)
			{
			ping->png_raw_rxpointangle[i] = 0;
					/* Raw beam pointing angles in 0.01 degree,
						positive to port.
						These values are relative to the transducer
						array and have not been corrected
						for vessel motion. */
			ping->png_raw_rxsector[i] = 0;	/* transmit sector number (0-19) */
			ping->png_raw_rxdetection[i] = 0; /* Detection info:
								   This datagram may contain data for beams with and without a
								   valid detection. Eight bits (0-7) gives details about the detection:
									A) If the most significant bit (bit7) is zero, this beam has a valid
										detection. Bit 0-3 is used to specify how the range for this beam
										is calculated
										0: Amplitude detect
										1: Phase detect
										2-15: Future use
									B) If the most significant bit is 1, this beam has an invalid
										detection. Bit 4-6 is used to specify how the range (and x,y,z
										parameters) for this beam is calculated
										0: Normal detection
										1: Interpolated or extrapolated from neighbour detections
										2: Estimated
										3: Rejected candidate
										4: No detection data is available for this beam (all parameters
											are set to zero)
										5-7: Future use
									The invalid range has been used to fill in amplitude samples in
									the seabed image datagram.
										bit 7: 0 = good detection
										bit 7: 1 = bad detection
										bit 3: 0 = amplitude detect
										bit 3: 1 = phase detect
										bits 4-6: 0 = normal detection
										bits 4-6: 1 = interpolated from neighbor detections
										bits 4-6: 2 = estimated
										bits 4-6: 3 = rejected
										bits 4-6: 4 = no detection available
										other bits : future use */
			ping->png_raw_rxwindow[i] = 0;	/* length of detection window */
			ping->png_raw_rxquality[i] = 0;	/* beam quality flag
								   0-254 Scaled standard deviation (sd) of the
									range detection divided by
									the detected range (dr):
									Quality factor = 250*sd/dr. */
			ping->png_raw_rxspare1[i] = 0;	/* spare */
			ping->png_raw_rxrange[i] = 0.0;	/* range as two-way travel time (s) */
			ping->png_raw_rxamp[i] = 0;		/* 0.5 dB */
			ping->png_raw_rxcleaning[i] = 0;	/* Real time cleaning info */
					/* realtime cleaning info:
						For future use. A real time data cleaning module may flag out
						beams. Bit 7 will be set to 1 if the beam is flagged out. Bit 0-6
						will contain a code telling why the beam is flagged out. */
			ping->png_raw_rxspare2[i] = 0;	/* spare */
			}


		/* sidescan */
		ping->png_ss2_read = 0;	/* flag indicating actual reading of sidescan record */
		ping->png_ss_date = 0;	/* date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;	/* time since midnight in msec
					    08:12:51.234 = 29570234 */
		ping->png_ss_count = 0;	/* sequential counter or input identifier */
		ping->png_ss_serial = 0;	/* system 1 or system 2 serial number */
		ping->png_ss_sample_rate = 0.0;	/* sampling rate (Hz) */
		ping->png_r_zero = 0;	/* range to normal incidence used in TVG
					    (R0 predicted) in samples */
		ping->png_bsn = 0;	/* normal incidence backscatter (BSN) (0.1 dB) */
		ping->png_bso = 0;	/* oblique incidence backscatter (BSO) (0.1 dB) */
		ping->png_tx = 0;		/* Tx beamwidth (0.1 deg) */
		ping->png_tvg_crossover = 0;
					/* TVG law crossover angle (0.1 deg) */
		ping->png_nbeams_ss = 0;	/* number of beams with sidescan */
		ping->png_npixels = 0;	/* number of pixels of sidescan */
		for (i=0;i<MBSYS_SIMRAD3_MAXBEAMS;i++)
			{
			ping->png_sort_direction[i] = 0;
					/* sorting direction - The first sample in a beam
						has lowest range if 1, highest if -- 1. Note
						that the ranges in the seabed image datagram
						are all two-- way from time of transmit to
						time of receive. */
			ping->png_beam_samples[i] = 0;
					/* number of sidescan samples derived from
						each beam */
			ping->png_start_sample[i] = 0;
					/* start sample number */
			ping->png_ssdetection[i] = 0; /* Detection info:
								   This datagram may contain data for beams with and without a
								   valid detection. Eight bits (0-7) gives details about the detection:
									A) If the most significant bit (bit7) is zero, this beam has a valid
										detection. Bit 0-3 is used to specify how the range for this beam
										is calculated
										0: Amplitude detect
										1: Phase detect
										2-15: Future use
									B) If the most significant bit is 1, this beam has an invalid
										detection. Bit 4-6 is used to specify how the range (and x,y,z
										parameters) for this beam is calculated
										0: Normal detection
										1: Interpolated or extrapolated from neighbour detections
										2: Estimated
										3: Rejected candidate
										4: No detection data is available for this beam (all parameters
											are set to zero)
										5-7: Future use
									The invalid range has been used to fill in amplitude samples in
									the seabed image datagram.
										bit 7: 0 = good detection
										bit 7: 1 = bad detection
										bit 3: 0 = amplitude detect
										bit 3: 1 = phase detect
										bits 4-6: 0 = normal detection
										bits 4-6: 1 = interpolated from neighbor detections
										bits 4-6: 2 = estimated
										bits 4-6: 3 = rejected
										bits 4-6: 4 = no detection available
										other bits : future use */
			ping->png_center_sample[i] = 0;
					/* center sample number */
			}
		for (i=0;i<MBSYS_SIMRAD3_MAXRAWPIXELS;i++)
			{
			ping->png_ssraw[i] = 0; /* the raw sidescan ordered port to starboard */
			}
		ping->png_pixel_size = 0.0;	/* processed sidescan pixel size (m) */
		ping->png_pixels_ss = 0;	/* number of processed sidescan pixels stored */
		for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			{
			ping->png_ss[i] = 0;	/* the processed sidescan ordered port to starboard */
			ping->png_ssalongtrack[i] = 0; /* the processed sidescan alongtrack distances (0.01 m) */
			}
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
int mbsys_simrad3_extraparameters_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_extraparameters_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_extraparameters_struct *extraparameters;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->extraparameters == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_extraparameters_struct),
			(void **)&(store->extraparameters),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		extraparameters = (struct mbsys_simrad3_extraparameters_struct *) store->extraparameters;

		/* initialize everything */
		extraparameters->xtr_date = 0;	/* extra parameters date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		extraparameters->xtr_msec = 0;	/* extra parameters time since midnight in msec
				    08:12:51.234 = 29570234 */
		extraparameters->xtr_count = 0;	/* ping counter */
		extraparameters->xtr_serial = 0;	/* system 1 or 2 serial number */
		extraparameters->xtr_id = 0;	        /* content identifier:
                                    1:  Calib.txt file for angle offset
                                    2:  Log all heights
                                    3:  Sound velocity at transducer
                                    4:  Sound velocity profile
                                    5:  Multicast RX status */
		extraparameters->xtr_data_size = 0;
		extraparameters->xtr_nalloc = 0;
		extraparameters->xtr_data = NULL;          /* variable array following from content identifier and record size */
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
int mbsys_simrad3_wc_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_wc_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_watercolumn_struct *wc;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->wc == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_watercolumn_struct),
			(void **)&(store->wc),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		wc = (struct mbsys_simrad3_watercolumn_struct *) store->wc;

		/* initialize everything */
		wc->wtc_date = 0;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		wc->wtc_msec = 0;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		wc->wtc_count = 0;	/* sequential counter or input identifier */
		wc->wtc_serial = 0;	/* system 1 or system 2 serial number */
		wc->wtc_ndatagrams = 0;	/* number of datagrams used to represent
						the water column for this ping */
		wc->wtc_datagram = 0;	/* number this datagram */
		wc->wtc_ntx = 0;	/* number of transmit sectors */
		wc->wtc_nrx = 0;	/* number of receive beams */
		wc->wtc_nbeam = 0;	/* number of beams in this datagram */
		wc->wtc_ssv = 0;	/* sound speed at transducer (0.1 m/sec) */
		wc->wtc_sfreq = 0;	/* sampling frequency (0.01 Hz) */
		wc->wtc_heave = 0;	/* tx time heave at transducer (0.01 m) */
		wc->wtc_spare1 = 0;	/* spare */
		wc->wtc_spare2 = 0;	/* spare */
		wc->wtc_spare3 = 0;	/* spare */
		for (i=0;i<MBSYS_SIMRAD3_MAXTX;i++)
			{
			wc->wtc_txtiltangle[i] = 0;	/* tilt angle (0.01 deg) */
			wc->wtc_txcenter[i] = 0;	/* center frequency (Hz) */
			wc->wtc_txsector[i] = 0;	/* transmit sector number (0-19) */
			}
		for (i=0;i<MBSYS_SIMRAD3_MAXBEAMS;i++)
			{
			wc->beam[i].wtc_rxpointangle = 0;	/* Beam pointing angles in 0.01 degree,
									positive to port. These values are roll stabilized. */
			wc->beam[i].wtc_start_sample = 0;	/* start sample number */
			wc->beam[i].wtc_beam_samples = 0;	/* number of water column samples derived from
									each beam */
			wc->beam[i].wtc_sector = 0;		/* transmit sector identifier */
			wc->beam[i].wtc_beam = 0;  		/* beam 128 is first beam on
				  	  				second head of EM3000D */
			for (j=0;j<MBSYS_SIMRAD3_MAXRAWPIXELS;j++)
				wc->beam[i].wtc_amp[j] = 0;	/* water column amplitude (dB) */
			}
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
int mbsys_simrad3_attitude_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_attitude_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_attitude_struct *attitude;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->attitude == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_attitude_struct),
			(void **)&(store->attitude),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		attitude = (struct mbsys_simrad3_attitude_struct *) store->attitude;

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
		for (i=0;i<MBSYS_SIMRAD3_MAXATTITUDE;i++)
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
		attitude->att_sensordescriptor = 0;
				/* heading status (0=inactive) */
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
int mbsys_simrad3_netattitude_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_netattitude_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_netattitude_struct *netattitude;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->netattitude == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_netattitude_struct),
			(void **)&(store->netattitude),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		netattitude = (struct mbsys_simrad3_netattitude_struct *) store->netattitude;

		/* initialize everything */
		netattitude->nat_date = 0;
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		netattitude->nat_msec = 0;
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		netattitude->nat_count = 0;
				/* sequential counter or input identifier */
		netattitude->nat_serial = 0;
				/* system 1 or system 2 serial number */
		netattitude->nat_ndata = 0;
				/* number of attitude data */
		netattitude->nat_sensordescriptor = 0;	/* sensor system descriptor */
		for (i=0;i<MBSYS_SIMRAD3_MAXATTITUDE;i++)
		    {
		    netattitude->nat_time[i] = 0;
				/* time since record start (msec) */
		    netattitude->nat_roll[i] = 0;
				/* roll (0.01 degree) */
		    netattitude->nat_pitch[i] = 0;
				/* pitch (0.01 degree) */
		    netattitude->nat_heave[i] = 0;
				/* heave (cm) */
		    netattitude->nat_heading[i] = 0;
				/* heading (0.01 degree) */
		    netattitude->nat_nbyte_raw[i] = 0;	/* number of bytes in input datagram (Nd) */
		    for (j=0;j<MBSYS_SIMRAD3_BUFFER_SIZE;j++)
		    	netattitude->nat_raw[i*MBSYS_SIMRAD3_BUFFER_SIZE+j] = 0;	/* network attitude input datagram as received by datalogger */
		    }
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
int mbsys_simrad3_heading_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_heading_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_heading_struct *heading;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->heading == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_heading_struct),
			(void **)&(store->heading),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		heading = (struct mbsys_simrad3_heading_struct *) store->heading;

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
		for (i=0;i<MBSYS_SIMRAD3_MAXHEADING;i++)
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
int mbsys_simrad3_ssv_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_ssv_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ssv_struct *ssv;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->ssv == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_ssv_struct),
			(void **)&(store->ssv),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		ssv = (struct mbsys_simrad3_ssv_struct *) store->ssv;

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
		for (i=0;i<MBSYS_SIMRAD3_MAXTILT;i++)
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
int mbsys_simrad3_tilt_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_tilt_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_tilt_struct *tilt;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* allocate memory for data structure if needed */
	if (store->tilt == NULL)
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			sizeof(struct mbsys_simrad3_tilt_struct),
			(void **)&(store->tilt),error);

	if (status == MB_SUCCESS)
		{

		/* get data structure pointer */
		tilt = (struct mbsys_simrad3_tilt_struct *) store->tilt;

		/* initialize everything */
		tilt->tlt_date = 0;
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		tilt->tlt_msec = 0;
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		tilt->tlt_count = 0;
				/* sequential counter or input identifier */
		tilt->tlt_serial = 0;
				/* system 1 or system 2 serial number */
		tilt->tlt_ndata = 0;
				/* number of tilt data */
		for (i=0;i<MBSYS_SIMRAD3_MAXTILT;i++)
		    {
		    tilt->tlt_time[i] = 0;
				/* time since record start (msec) */
		    tilt->tlt_tilt[i] = 0;
				/* tilt + forward (0.01 deg) */
		    }
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
int mbsys_simrad3_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_deall";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)*store_ptr);
		}

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) *store_ptr;

	/* deallocate memory for survey data structure */
	if (store->ping != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->ping),error);

	/* deallocate memory for extraparameters data structure */
	if (store->extraparameters != NULL)
		{
		if (store->extraparameters->xtr_data != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->extraparameters->xtr_data),error);
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->extraparameters),error);
		}

	/* deallocate memory for water column data structure */
	if (store->wc != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->wc),error);

	/* deallocate memory for attitude data structure */
	if (store->attitude != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->attitude),error);

	/* deallocate memory for network attitude data structure */
	if (store->netattitude != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->netattitude),error);

	/* deallocate memory for heading data structure */
	if (store->heading != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->heading),error);

	/* deallocate memory for ssv data structure */
	if (store->ssv != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->ssv),error);

	/* deallocate memory for tilt data structure */
	if (store->tilt != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->tilt),error);

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
int mbsys_simrad3_zero_ss(int verbose, void *store_ptr, int *error)
{
	char	*function_name = "mbsys_simrad3_zero_ss";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to data descriptor */
	store = (struct mbsys_simrad3_struct *) store_ptr;
	if (store != NULL)
	    ping = (struct mbsys_simrad3_ping_struct *) store->ping;

	/* initialize all sidescan stuff to zeros */
	if (store->ping != NULL)
		{
		ping->png_ss2_read = 0;	/* flag indicating actual reading of sidescan record */
		ping->png_ss_date = 0;	/* date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;	/* time since midnight in msec
					    08:12:51.234 = 29570234 */
		ping->png_ss_count = 0;	/* sequential counter or input identifier */
		ping->png_ss_serial = 0;	/* system 1 or system 2 serial number */
		ping->png_ss_sample_rate = 0.0;	/* sampling rate (Hz) */
		ping->png_r_zero = 0;	/* range to normal incidence used in TVG
					    (R0 predicted) in samples */
		ping->png_bsn = 0;	/* normal incidence backscatter (BSN) (0.1 dB) */
		ping->png_bso = 0;	/* oblique incidence backscatter (BSO) (0.1 dB) */
		ping->png_tx = 0;		/* Tx beamwidth (0.1 deg) */
		ping->png_tvg_crossover = 0;
					/* TVG law crossover angle (0.1 deg) */
		ping->png_nbeams_ss = 0;	/* number of beams with sidescan */
		ping->png_npixels = 0;	/* number of pixels of sidescan */
		for (i=0;i<MBSYS_SIMRAD3_MAXBEAMS;i++)
			{
			ping->png_sort_direction[i] = 0;
					/* sorting direction - The first sample in a beam
						has lowest range if 1, highest if -- 1. Note
						that the ranges in the seabed image datagram
						are all two-- way from time of transmit to
						time of receive. */
			ping->png_beam_samples[i] = 0;
					/* number of sidescan samples derived from
						each beam */
			ping->png_start_sample[i] = 0;
					/* start sample number */
			ping->png_ssdetection[i] = 0; /* Detection info:
								   This datagram may contain data for beams with and without a
								   valid detection. Eight bits (0-7) gives details about the detection:
									A) If the most significant bit (bit7) is zero, this beam has a valid
										detection. Bit 0-3 is used to specify how the range for this beam
										is calculated
										0: Amplitude detect
										1: Phase detect
										2-15: Future use
									B) If the most significant bit is 1, this beam has an invalid
										detection. Bit 4-6 is used to specify how the range (and x,y,z
										parameters) for this beam is calculated
										0: Normal detection
										1: Interpolated or extrapolated from neighbour detections
										2: Estimated
										3: Rejected candidate
										4: No detection data is available for this beam (all parameters
											are set to zero)
										5-7: Future use
									The invalid range has been used to fill in amplitude samples in
									the seabed image datagram.
										bit 7: 0 = good detection
										bit 7: 1 = bad detection
										bit 3: 0 = amplitude detect
										bit 3: 1 = phase detect
										bits 4-6: 0 = normal detection
										bits 4-6: 1 = interpolated from neighbor detections
										bits 4-6: 2 = estimated
										bits 4-6: 3 = rejected
										bits 4-6: 4 = no detection available
										other bits : future use */
			ping->png_center_sample[i] = 0;
					/* center sample number */
			}
		for (i=0;i<MBSYS_SIMRAD3_MAXRAWPIXELS;i++)
			{
			ping->png_ssraw[i] = 0; /* the raw sidescan ordered port to starboard */
			}
		ping->png_pixel_size = 0.0;	/* processed sidescan pixel size (m) */
		ping->png_pixels_ss = 0;	/* number of processed sidescan pixels stored */
		for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			{
			ping->png_ss[i] = 0; /* the processed sidescan ordered port to starboard */
			ping->png_ssalongtrack[i] = 0; /* the processed sidescan alongtrack distances (0.01 m) */
			}
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_simrad3_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;
		*nbath = ping->png_nbeams;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD3_MAXPIXELS;
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
int mbsys_simrad3_pingnumber(int verbose, void *mbio_ptr,
		int *pingnumber, int *error)
{
	char	*function_name = "mbsys_simrad3_pingnumber";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) mb_io_ptr->store_data;

	/* extract data from structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
	*pingnumber = ping->png_count;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_simrad3_extract(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad3_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	double	reflscale;
	double	pixel_size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

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
		if (ping->png_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * ping->png_longitude;
		else
		    *navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * ping->png_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* set beamwidths in mb_io structure */
		if (store->run_rec_beam > 0)
		    mb_io_ptr->beamwidth_xtrack
			= 0.1 * store->run_rec_beam;
		if (ping->png_tx > 0)
		    {
		    mb_io_ptr->beamwidth_ltrack
			= 0.1 * ping->png_tx;
		    }
		else if (store->run_tran_beam > 0)
		    {
		    mb_io_ptr->beamwidth_ltrack
			= 0.1 * store->run_tran_beam;
		    }

		/* read distance and depth values into storage arrays */
		reflscale  = 0.1;
		*nbath = 0;
		for (i=0;i<ping->png_nbeams;i++)
			{
			bath[i] = ping->png_depth[i] + ping->png_xducer_depth;
			beamflag[i] = ping->png_beamflag[i];
			bathacrosstrack[i] = ping->png_acrosstrack[i];
			bathalongtrack[i] = ping->png_alongtrack[i];
			amp[i] = reflscale * ping->png_amp[i];
			}
		*nbath = ping->png_nbeams;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD3_MAXPIXELS;
		pixel_size = ping->png_pixel_size;
		for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			{
			if (ping->png_ss[i] == EM3_INVALID_SS
				|| (ping->png_ss[i] == EM3_INVALID_AMP && ping->png_ssalongtrack[i] == 0))
				{
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = pixel_size
						* (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
				ssalongtrack[i] = 0.0;
				}
			else
				{
				ss[i] = 0.01 * ping->png_ss[i];
				ssacrosstrack[i] = pixel_size
						* (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
				ssalongtrack[i] = 0.01 * ping->png_ssalongtrack[i];
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
	else if (*kind == MB_DATA_NAV
		|| *kind == MB_DATA_NAV1
		|| *kind == MB_DATA_NAV2
		|| *kind == MB_DATA_NAV3)
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
		if (store->pos_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * store->pos_longitude;
		else
		    *navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * store->pos_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		*heading = 0.01 * store->pos_heading;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
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
			MBSYS_SIMRAD3_COMMENT_LENGTH);

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
int mbsys_simrad3_insert(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_simrad3_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	double	reflscale;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
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
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* allocate secondary data structure for
			survey data if needed */
		if (store->ping == NULL)
			{
			status = mbsys_simrad3_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

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
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int) rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int) rint(speed / 0.036);

		/* insert bathymetry and amplitude */
		reflscale  = 0.1;
		if (status == MB_SUCCESS && ping->png_nbeams == 0)
			{
			ping->png_nbeams_valid = 0;
			for (i=0;i<nbath;i++)
			    {
			    if (beamflag[i] != MB_FLAG_NULL)
				{
				ping->png_depth[i] = bath[i] - ping->png_xducer_depth;
				ping->png_beamflag[i] = beamflag[i];
				ping->png_acrosstrack[i] = bathacrosstrack[i];
				ping->png_alongtrack[i] = bathalongtrack[i];
				ping->png_amp[i] = (int) rint(amp[i] / reflscale);
				ping->png_nbeams++;
				ping->png_nbeams_valid++;
				}
			    else
				{
				ping->png_depth[i] = 0.0;
				ping->png_beamflag[i] = MB_FLAG_NULL;
				ping->png_acrosstrack[i] = 0.0;
				ping->png_alongtrack[i] = 0.0;
				ping->png_amp[i] = 0;
				ping->png_nbeams++;
				}
			    }
			ping->png_nbeams = nbath;
			}
		else if (status == MB_SUCCESS)
			{
			for (i=0;i<ping->png_nbeams;i++)
				{
				ping->png_depth[i] = bath[i] - ping->png_xducer_depth;
				ping->png_beamflag[i] = beamflag[i];
				ping->png_acrosstrack[i] = bathacrosstrack[i];
				ping->png_alongtrack[i] = bathalongtrack[i];
				ping->png_amp[i] = (int) rint(amp[i] / reflscale);
				}
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<nss;i++)
				{
				if (ss[i] > MB_SIDESCAN_NULL)
					{
					ping->png_ss[i] = (short) rint(100 * ss[i]);
					ping->png_ssalongtrack[i] = (short) rint(100 * ssalongtrack[i]);
					}
				else
					{
					ping->png_ss[i] = EM3_INVALID_SS;
					ping->png_ssalongtrack[i] = 0;
					}
				}
			}
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV
		|| store->kind == MB_DATA_NAV1
		|| store->kind == MB_DATA_NAV2
		|| store->kind == MB_DATA_NAV3)
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
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int) rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int) rint(speed / 0.036);

		/* get roll pitch and heave */

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV)
		    {
		    store->pos_system = store->pos_system | 128;
		    }

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1)
		    {
		    store->pos_system = store->pos_system | 1;
		    }
		else if (store->kind == MB_DATA_NAV2)
		    {
		    store->pos_system = store->pos_system | 2;
		    }
		else if (store->kind == MB_DATA_NAV3)
		    {
		    store->pos_system = store->pos_system | 3;
		    }
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->par_com,comment,
			MBSYS_SIMRAD3_COMMENT_LENGTH);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles,
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset,
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_simrad3_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	int	time_i[7];
	double	ptime_d;
	double	soundspeed;
	double	lever_x, lever_y, lever_z, offset_x, offset_y, offset_z;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %lu\n",(size_t)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%lu\n",(size_t)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%lu\n",(size_t)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%lu\n",(size_t)angles_null);
		fprintf(stderr,"dbg2       heave:      %lu\n",(size_t)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %lu\n",(size_t)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* get ping time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ptime_d);

		/* obtain lever arm offset for sonar relative to the position sensor */
		mb_lever(verbose, store->par_s1y, store->par_s1x, store->par_s1z - store->par_wlz,
				store->par_p1y, store->par_p1x, store->par_p1z,
				store->par_msy, store->par_msx, store->par_msz,
				-0.01 * ping->png_pitch + store->par_msp, -0.01 * ping->png_roll + store->par_msr,
				&lever_x, &lever_y, &lever_z, error);

		/* obtain position offset for beam */
		offset_x = store->par_s1y - store->par_p1y + lever_x;
		offset_y = store->par_s1x - store->par_p1x + lever_y;
		offset_z =  lever_z;

		/* get depth offset (heave + heave offset) */
		*ssv = 0.1 * ping->png_ssv;
		*draft = ping->png_xducer_depth + offset_z;

		/* get travel times, angles */
		*nbeams = ping->png_nbeams;
		soundspeed = 0.1 * ((double)ping->png_ssv);
		for (i=0;i<ping->png_nbeams;i++)
			{
			ttimes[i] = ping->png_range[i];
			angles[i] = ping->png_depression[i];
			angles_forward[i] = 180.0 - ping->png_azimuth[i];
			if (angles_forward[i] < 0.0) angles_forward[i] += 360.0;
			angles_null[i] = 0.0;
			heave[i] = -ping->png_bheave[i] + lever_z;
			alongtrack_offset[i] = (0.01 * ((double)ping->png_speed))
						* ((double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]])
						+ offset_y;
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
int mbsys_simrad3_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_simrad3_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       detects:    %lu\n",(size_t)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		*nbeams = ping->png_nbeams;
		for (j=0;j<ping->png_nbeams;j++)
			{
			detects[j] = MB_DETECT_UNKNOWN;
			}
		for (i=0;i<ping->png_nbeams;i++)
			{
			if (ping->png_detection[i] & 1)
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
int mbsys_simrad3_pulses(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *pulses, int *error)
{
	char	*function_name = "mbsys_simrad3_pulses";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       pulses:     %lu\n",(size_t)pulses);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		*nbeams = ping->png_nbeams;
		for (j=0;j<ping->png_nbeams;j++)
			{
			pulses[j] = MB_PULSE_UNKNOWN;
			}
		for (i=0;i<ping->png_nbeams;i++)
			{
			if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 0)
				pulses[i] = MB_PULSE_CW;
			else if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 1)
				pulses[i] = MB_PULSE_UPCHIRP;
			else if (ping->png_raw_txwaveform[ping->png_raw_rxsector[i]] == 2)
				pulses[i] = MB_PULSE_DOWNCHIRP;
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
			fprintf(stderr,"dbg2       beam %d: pulses:%d\n",
				i,pulses[i]);
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
int mbsys_simrad3_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error)
{
	char	*function_name = "mbsys_simrad3_gains";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_survey_struct *ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_survey_struct *) store->ping;

		/* get transmit_gain (dB) */
		*transmit_gain = (double)store->run_tran_pow;

		/* get pulse_length (sec) */
		*pulse_length = 0.000001 * (double)store->run_tran_pulse;

		/* get receive_gain (dB) */
		*receive_gain = (double)store->run_rec_gain;

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
		fprintf(stderr,"dbg2       transmit_gain: %f\n",*transmit_gain);
		fprintf(stderr,"dbg2       pulse_length:  %f\n",*pulse_length);
		fprintf(stderr,"dbg2       receive_gain:  %f\n",*receive_gain);
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
int mbsys_simrad3_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude,
	int *error)
{
	char	*function_name = "mbsys_simrad3_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	double	altitude_best;
	double	xtrack_min;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* get transducer depth and altitude */
		*transducer_depth = ping->png_xducer_depth;
		found = MB_NO;
		altitude_best = 0.0;
		xtrack_min = 99999999.9;
		for (i=0;i<ping->png_nbeams;i++)
		    {
		    if (mb_beam_ok(ping->png_beamflag[i])
			&& fabs(ping->png_acrosstrack[i]) < xtrack_min)
			{
			xtrack_min = fabs(ping->png_acrosstrack[i]);
			altitude_best = ping->png_depth[i];
			found = MB_YES;
			}
		    }
		if (found == MB_NO)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<ping->png_nbeams;i++)
			{
			if (ping->png_quality[i] > 0
			    && fabs(ping->png_acrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(ping->png_acrosstrack[i]);
			    altitude_best = ping->png_depth[i];
			    found = MB_YES;
			    }
			}
		    }
		if (found == MB_YES)
		    *altitude = altitude_best;
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
int mbsys_simrad3_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error)
{
	char	*function_name = "mbsys_simrad3_extract_nnav";
	int	status = MB_SUCCESS;
	int	interp_error = MB_ERROR_NO_ERROR;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	struct mbsys_simrad3_attitude_struct *attitude;
	struct mbsys_simrad3_netattitude_struct *netattitude;
	double	atime_d;
	int	atime_i[7];
	int	i, inav;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       nmax:       %d\n",nmax);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* just one navigation value */
		*n = 1;

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
		if (ping->png_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * ping->png_longitude;
		else
		    *navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * ping->png_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = ping->png_xducer_depth;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		/* done translating values */

		}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV
		|| *kind == MB_DATA_NAV1
		|| *kind == MB_DATA_NAV2
		|| *kind == MB_DATA_NAV3)
		{
                /* get survey data structure */
		if (store->ping != NULL)
                	ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* just one navigation value */
		*n = 1;

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
		if (store->pos_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * store->pos_longitude;
		else
		    *navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * store->pos_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM3_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (store->ping != NULL)
			*draft = ping->png_xducer_depth;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

		/* done translating values */

		}

	/* extract data from attitude structure */
	else if (store->type == EM3_ATTITUDE
		&& store->attitude != NULL)
		{
                /* get attitude data structure */
		attitude = (struct mbsys_simrad3_attitude_struct *) store->attitude;

		/* get n */
		*n = MIN(attitude->att_ndata, MB_ASYNCH_SAVE_MAX);

		/* get attitude time */
		atime_i[0] = attitude->att_date / 10000;
		atime_i[1] = (attitude->att_date % 10000) / 100;
		atime_i[2] = attitude->att_date % 100;
		atime_i[3] = attitude->att_msec / 3600000;
		atime_i[4] = (attitude->att_msec % 3600000) / 60000;
		atime_i[5] = (attitude->att_msec % 60000) / 1000;
		atime_i[6] = (attitude->att_msec % 1000) * 1000;
		mb_get_time(verbose, atime_i, &atime_d);

		/* loop over the data */
		for (i=0;i<*n;i++)
			{
			/* get time from the data record */
			time_d[i] = (double)(atime_d + 0.001 * attitude->att_time[i]);
			mb_get_date(verbose,time_d[i],&(time_i[7*i]));

			/* get attitude from the data record */
			heave[i] = (double)(0.01 * attitude->att_heave[i]);
			roll[i] = (double)(0.01 * attitude->att_roll[i]);
			pitch[i] = (double)(0.01 * attitude->att_pitch[i]);

			/* interpolate the heading */
			mb_hedint_interp(verbose, mbio_ptr, time_d[i],
				    		&heading[i], &interp_error);

			/* interpolate the navigation */
			mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], 0.0,
						&navlon[i], &navlat[i], &speed[i], &interp_error);

			/* interpolate the sonar depth */
			mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], &interp_error);
			}

		/* done translating values */

		}

	/* extract data from netattitude structure */
	else if (store->type == EM3_NETATTITUDE
		&& store->netattitude != NULL)
		{
                /* get attitude data structure */
		netattitude = (struct mbsys_simrad3_netattitude_struct *) store->netattitude;

		/* get n */
		*n = MIN(netattitude->nat_ndata, MB_ASYNCH_SAVE_MAX);

		/* get attitude time */
		atime_i[0] = netattitude->nat_date / 10000;
		atime_i[1] = (netattitude->nat_date % 10000) / 100;
		atime_i[2] = netattitude->nat_date % 100;
		atime_i[3] = netattitude->nat_msec / 3600000;
		atime_i[4] = (netattitude->nat_msec % 3600000) / 60000;
		atime_i[5] = (netattitude->nat_msec % 60000) / 1000;
		atime_i[6] = (netattitude->nat_msec % 1000) * 1000;
		mb_get_time(verbose, atime_i, &atime_d);

		/* loop over the data */
		for (i=0;i<*n;i++)
			{
			/* get time from the data record */
			time_d[i] = (double)(atime_d + 0.001 * netattitude->nat_time[i]);
			mb_get_date(verbose,time_d[i],&(time_i[7*i]));

			/* get attitude from the data record */
			heave[i] = (double)(0.01 * netattitude->nat_heave[i]);
			roll[i] = (double)(0.01 * netattitude->nat_roll[i]);
			pitch[i] = (double)(0.01 * netattitude->nat_pitch[i]);

			/* interpolate the heading */
			mb_hedint_interp(verbose, mbio_ptr, time_d[i],
				    		&heading[i], &interp_error);

			/* interpolate the navigation */
			mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], 0.0,
						&navlon[i], &navlat[i], &speed[i], &interp_error);

			/* interpolate the sonar depth */
			mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], &interp_error);
			}

		/* done translating values */

		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*n = 0;
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*n = 0;
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_simrad3_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mbsys_simrad3_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

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
		if (ping->png_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * ping->png_longitude;
		else
		    *navlon = 0.0;
		if (ping->png_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * ping->png_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = ping->png_xducer_depth;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		/* done translating values */

		}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV
		|| *kind == MB_DATA_NAV1
		|| *kind == MB_DATA_NAV2
		|| *kind == MB_DATA_NAV3)
		{
                /* get survey data structure */
		if (store->ping != NULL)
                	ping = (struct mbsys_simrad3_ping_struct *) store->ping;

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
		if (store->pos_longitude != EM3_INVALID_INT)
		    *navlon = 0.0000001 * store->pos_longitude;
		else
		    *navlon = 0.0;
		if (store->pos_latitude != EM3_INVALID_INT)
		    *navlat = 0.00000005 * store->pos_latitude;
		else
		    *navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM3_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM3_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (store->ping != NULL)
			*draft = ping->png_xducer_depth;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

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
int mbsys_simrad3_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_simrad3_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
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
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* allocate secondary data structure for
			survey data if needed */
		if (store->ping == NULL)
			{
			status = mbsys_simrad3_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

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
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int) rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int) rint(speed / 0.036);

		/* get draft  */
		ping->png_xducer_depth = draft;

		/* get roll pitch and heave */
		ping->png_roll = (int) rint(roll / 0.01);
		ping->png_pitch = (int) rint(pitch / 0.01);
		ping->png_heave = (int) rint(heave / 0.01);
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV
		|| store->kind == MB_DATA_NAV1
		|| store->kind == MB_DATA_NAV2
		|| store->kind == MB_DATA_NAV3)
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
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int) rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int) rint(speed / 0.036);

		/* get roll pitch and heave */
		store->pos_roll = (int) rint(roll / 0.01);
		store->pos_pitch = (int) rint(pitch / 0.01);
		store->pos_heave = (int) rint(heave / 0.01);

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV)
		    {
		    store->pos_system = store->pos_system | 128;
		    }

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1)
		    {
		    store->pos_system = store->pos_system | 1;
		    }
		else if (store->kind == MB_DATA_NAV2)
		    {
		    store->pos_system = store->pos_system | 2;
		    }
		else if (store->kind == MB_DATA_NAV3)
		    {
		    store->pos_system = store->pos_system | 3;
		    }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad3_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_simrad3_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_simrad3_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_SIMRAD3_MAXSVP);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad3_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_simrad3_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_struct *copy;
	struct mbsys_simrad3_ping_struct *ping_store;
	struct mbsys_simrad3_ping_struct *ping_copy;
	char	*ping_save;
	struct mbsys_simrad3_attitude_struct *attitude_store;
	struct mbsys_simrad3_attitude_struct *attitude_copy;
	char	*attitude_save;
	struct mbsys_simrad3_netattitude_struct *netattitude_store;
	struct mbsys_simrad3_netattitude_struct *netattitude_copy;
	char	*netattitude_save;
	struct mbsys_simrad3_heading_struct *heading_store;
	struct mbsys_simrad3_heading_struct *heading_copy;
	char	*heading_save;
	struct mbsys_simrad3_ssv_struct *ssv_store;
	struct mbsys_simrad3_ssv_struct *ssv_copy;
	char	*ssv_save;
	struct mbsys_simrad3_tilt_struct *tilt_store;
	struct mbsys_simrad3_tilt_struct *tilt_copy;
	char	*tilt_save;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %lu\n",(size_t)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_simrad3_struct *) store_ptr;
	copy = (struct mbsys_simrad3_struct *) copy_ptr;

	/* check if survey data needs to be copied */
	if (store->kind == MB_DATA_DATA
		&& store->ping != NULL)
		{
		/* make sure a survey data structure exists to
			be copied into */
		if (copy->ping == NULL)
			{
			status = mbsys_simrad3_survey_alloc(
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
			status = mbsys_simrad3_attitude_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}

		/* save pointer value */
		attitude_save = (char *)copy->attitude;
		}

	/* check if netattitude data needs to be copied */
	if (store->netattitude != NULL)
		{
		/* make sure a netattitude data structure exists to
			be copied into */
		if (copy->netattitude == NULL)
			{
			status = mbsys_simrad3_netattitude_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}

		/* save pointer value */
		netattitude_save = (char *)copy->netattitude;
		}

	/* check if heading data needs to be copied */
	if (store->heading != NULL)
		{
		/* make sure a heading data structure exists to
			be copied into */
		if (copy->heading == NULL)
			{
			status = mbsys_simrad3_heading_alloc(
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
			status = mbsys_simrad3_ssv_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}

		/* save pointer value */
		ssv_save = (char *)copy->ssv;
		}

	/* check if tilt data needs to be copied */
	if (store->tilt != NULL)
		{
		/* make sure a tilt data structure exists to
			be copied into */
		if (copy->tilt == NULL)
			{
			status = mbsys_simrad3_tilt_alloc(
					verbose,mbio_ptr,
					copy_ptr,error);
			}

		/* save pointer value */
		tilt_save = (char *)copy->tilt;
		}

	/* copy the main structure */
	*copy = *store;

	/* if needed copy the survey data structure */
	if (store->kind == MB_DATA_DATA
		&& store->ping != NULL
		&& status == MB_SUCCESS)
		{
		copy->ping = (struct mbsys_simrad3_ping_struct *) ping_save;
		ping_store = (struct mbsys_simrad3_ping_struct *) store->ping;
		ping_copy = (struct mbsys_simrad3_ping_struct *) copy->ping;
		*ping_copy = *ping_store;
		}
	else
		copy->ping = NULL;

	/* if needed copy the attitude data structure */
	if (store->attitude != NULL && status == MB_SUCCESS)
		{
		copy->attitude = (struct mbsys_simrad3_attitude_struct *) attitude_save;
		attitude_store = (struct mbsys_simrad3_attitude_struct *) store->attitude;
		attitude_copy = (struct mbsys_simrad3_attitude_struct *) copy->attitude;
		*attitude_copy = *attitude_store;
		}

	/* if needed copy the netattitude data structure */
	if (store->netattitude != NULL && status == MB_SUCCESS)
		{
		copy->netattitude = (struct mbsys_simrad3_netattitude_struct *) netattitude_save;
		netattitude_store = (struct mbsys_simrad3_netattitude_struct *) store->netattitude;
		netattitude_copy = (struct mbsys_simrad3_netattitude_struct *) copy->netattitude;
		*netattitude_copy = *netattitude_store;
		}

	/* if needed copy the heading data structure */
	if (store->heading != NULL && status == MB_SUCCESS)
		{
		copy->heading = (struct mbsys_simrad3_heading_struct *) heading_save;
		heading_store = (struct mbsys_simrad3_heading_struct *) store->heading;
		heading_copy = (struct mbsys_simrad3_heading_struct *) copy->heading;
		*heading_copy = *heading_store;
		}

	/* if needed copy the ssv data structure */
	if (store->ssv != NULL && status == MB_SUCCESS)
		{
		copy->ssv = (struct mbsys_simrad3_ssv_struct *) ssv_save;
		ssv_store = (struct mbsys_simrad3_ssv_struct *) store->ssv;
		ssv_copy = (struct mbsys_simrad3_ssv_struct *) copy->ssv;
		*ssv_copy = *ssv_store;
		}

	/* if needed copy the tilt data structure */
	if (store->tilt != NULL && status == MB_SUCCESS)
		{
		copy->tilt = (struct mbsys_simrad3_tilt_struct *) tilt_save;
		tilt_store = (struct mbsys_simrad3_tilt_struct *) store->tilt;
		tilt_copy = (struct mbsys_simrad3_tilt_struct *) copy->tilt;
		*tilt_copy = *tilt_store;
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
int mbsys_simrad3_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size,
		int swath_width_set, double *swath_width,
		int pixel_int,
		int *error)
{
	char	*function_name = "mbsys_simrad3_makess";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	double	ss[MBSYS_SIMRAD3_MAXPIXELS];
	int	ss_cnt[MBSYS_SIMRAD3_MAXPIXELS];
	double	ssacrosstrack[MBSYS_SIMRAD3_MAXPIXELS];
	double	ssalongtrack[MBSYS_SIMRAD3_MAXPIXELS];
	short *beam_ss;
	int	nbathsort;
	double	bathsort[MBSYS_SIMRAD3_MAXBEAMS];
	double	depthoffset;
	double	reflscale;
	double  pixel_size_calc;
	double	pixel_size_max_swath;
	double	ss_spacing, ss_spacing_use;
	int	pixel_int_use;
	double	angle, xtrackss;
	double	range, beam_foot, beamwidth, sint;
	int	time_i[7];
	double	bath_time_d, ss_time_d;
	int	ss_ok;
	int	first, last, k1, k2;
	int	i, k, kk;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:        %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:       %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       pixel_size_set:  %d\n",pixel_size_set);
		fprintf(stderr,"dbg2       pixel_size:      %f\n",*pixel_size);
		fprintf(stderr,"dbg2       swath_width_set: %d\n",swath_width_set);
		fprintf(stderr,"dbg2       swath_width:     %f\n",*swath_width);
		fprintf(stderr,"dbg2       pixel_int:       %d\n",pixel_int);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get pointer to raw data structure */
		ping = (struct mbsys_simrad3_ping_struct *) store->ping;

		/* zero the sidescan */
		for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
			}

		/* set scaling parameters */
		depthoffset = ping->png_xducer_depth;
		reflscale  = 0.1;

		/* get raw pixel size */
		ss_spacing = 750.0 / ping->png_sample_rate;

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD3_EM1000)
		    {
		    beamwidth = 2.5;
		    }
		else if (ping->png_tx > 0)
		    {
		    beamwidth = 0.1 * ping->png_tx;
		    }
		else if (store->run_tran_beam > 0)
		    {
		    beamwidth = 0.1 * store->run_tran_beam;
		    }

		/* get median depth */
		nbathsort = 0;
		for (i=0;i<ping->png_nbeams;i++)
		    {
		    if (mb_beam_ok(ping->png_beamflag[i]))
			{
			bathsort[nbathsort] = ping->png_depth[i] + depthoffset;
			nbathsort++;
			}
		    }

		/* get sidescan pixel size */
		if (swath_width_set == MB_NO)
		    {
		    if (store->run_swath_angle > 0)
			*swath_width = (double)store->run_swath_angle;
		    else
			*swath_width = 2.5 + MAX(90.0 - ping->png_depression[0],
						90.0 - ping->png_depression[ping->png_nbeams-1]);
		    }
		if (pixel_size_set == MB_NO
		    && nbathsort > 0)
		    {
		    qsort((char *)bathsort, nbathsort, sizeof(double),(void *)mb_double_compare);
		    pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort/2]
					/ MBSYS_SIMRAD3_MAXPIXELS;
		    if (store->run_max_swath > 0)
			{
			pixel_size_max_swath = 2 * ((double) store->run_max_swath) / ((double)MBSYS_SIMRAD3_MAXPIXELS);
			if (pixel_size_max_swath < pixel_size_calc)
			    pixel_size_calc = pixel_size_max_swath;
			}
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

		/* check that sidescan can be used */
		/* get times of bath and sidescan records */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
		ss_ok = MB_YES;
		if (ping->png_nbeams < ping->png_nbeams_ss
		    || ping->png_nbeams > ping->png_nbeams_ss + 1)
		    {
		    ss_ok = MB_NO;
		    if (verbose > 0)
			    fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
				    function_name, time_i[0], time_i[1], time_i[2],
				    time_i[3], time_i[4], time_i[5], time_i[6],
				    ping->png_nbeams, ping->png_nbeams_ss);
		    }
/*fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d   bathbeams:%d   ssbeams:%d\n",
function_name, time_i[0], time_i[1], time_i[2],
time_i[3], time_i[4], time_i[5], time_i[6],
ping->png_nbeams, ping->png_nbeams_ss);*/

		/* loop over raw sidescan, putting each raw pixel into
			the binning arrays */
		if (ss_ok == MB_YES)
		for (i=0;i<ping->png_nbeams_ss;i++)
			{
			beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
			if (mb_beam_ok(ping->png_beamflag[i]))
			    {
			    if (ping->png_beam_samples[i] > 0)
				{
				range = sqrt(ping->png_depth[i] * ping->png_depth[i]
						+ ping->png_acrosstrack[i] * ping->png_acrosstrack[i]);
				angle = 90.0 - ping->png_depression[i];
				beam_foot = range * sin(DTR * beamwidth)
							/ cos(DTR * angle);
				sint = fabs(sin(DTR * angle));
				if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
				    ss_spacing_use = beam_foot / ping->png_beam_samples[i];
				else
				    ss_spacing_use = ss_spacing / sint;
/* fprintf(stderr, "spacing: %f %f n:%d sint:%f beamwidth:%f angle:%f range:%f foot:%f factor:%f\n",
ss_spacing, ss_spacing_use,
ping->png_beam_samples[i], sint, beamwidth, angle, range, beam_foot,
ping->png_beam_samples[i] * ss_spacing / beam_foot);*/
				}
			    for (k=0;k<ping->png_beam_samples[i];k++)
				{
				if (beam_ss[k] != EM3_INVALID_AMP)
					{
					xtrackss = ping->png_acrosstrack[i]
						    + ss_spacing_use * (k - ping->png_center_sample[i]);
					kk = MBSYS_SIMRAD3_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
					if (kk > 0 && kk < MBSYS_SIMRAD3_MAXPIXELS)
					    {
					    ss[kk]  += reflscale*((double)beam_ss[k]);
					    ssalongtrack[kk] += ping->png_alongtrack[i];
					    ss_cnt[kk]++;
					    }
					}
				}
			    }
			}

		/* average the sidescan */
		first = MBSYS_SIMRAD3_MAXPIXELS;
		last = -1;
		for (k=0;k<MBSYS_SIMRAD3_MAXPIXELS;k++)
			{
			if (ss_cnt[k] > 0)
				{
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k]
					= (k - MBSYS_SIMRAD3_MAXPIXELS / 2)
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
				    = (k - MBSYS_SIMRAD3_MAXPIXELS / 2)
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
		ping->png_pixel_size = *pixel_size;
		if (last > first)
		    ping->png_pixels_ss = MBSYS_SIMRAD3_MAXPIXELS;
		else
		    ping->png_pixels_ss = 0;
		for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
		    {
		    if (ss[i] > MB_SIDESCAN_NULL)
		    	{
		    	ping->png_ss[i] = (short)(100 * ss[i]);
		    	ping->png_ssalongtrack[i] = (short)(100 * ssalongtrack[i]);
			}
		    else
		    	{
		    	ping->png_ss[i] = EM3_INVALID_SS;
		    	ping->png_ssalongtrack[i] = 0;
			}
		    }

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Sidescan regenerated in <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       png_nbeams_ss: %d\n",
				ping->png_nbeams_ss);
			for (i=0;i<ping->png_nbeams_ss;i++)
			  fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  amp:%d  acrosstrack:%f  alongtrack:%f\n",
				i,
				ping->png_beamflag[i],
				ping->png_depth[i],
				ping->png_amp[i],
				ping->png_acrosstrack[i],
				ping->png_alongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				MBSYS_SIMRAD3_MAXPIXELS);
			for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n",
				i,ss_cnt[i],ss[i],
				ssacrosstrack[i],
				ssalongtrack[i]);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n",
				ping->png_pixels_ss);
			for (i=0;i<MBSYS_SIMRAD3_MAXPIXELS;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n",
				i,ping->png_ss[i],ping->png_ssalongtrack[i]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
