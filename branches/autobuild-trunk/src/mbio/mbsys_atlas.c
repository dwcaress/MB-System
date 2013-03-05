/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_atlas.c	3.00	6/25/01
 *	$Id: mbsys_atlas.c 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 2001-2012 by
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
 * mbsys_atlas.c contains the MBIO functions for handling data from 
 * STN Atlas Marine Electronics multibeam sonars.
 * The relevant sonars include Hydrosweep DS2 and Fansweep sonars.
 * The older  Hydrosweep DS and MD sonars produce data in different 
 * formats (e.g. 21-24 and 101-102).
 * The data formats associated with (newer) STN Atlas sonars
 * include:
 *    MBSYS_ATLAS formats (code in mbsys_atlas.c and mbsys_atlas.h):
 *      MBF_HSDS2RAW : MBIO ID 182 - Vendor raw HSDS2 and Fansweep format
 *      MBF_HSDS2LAM : MBIO ID 183 - L-DEO HSDS2 and Fansweep processing format
 *
 * Author:	D. W. Caress
 * Author:	D. N. Chayes
 * Date:	June 25, 2001
 *
 * $Log: mbsys_atlas.c,v $
 * Revision 5.11  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.10  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.9  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.8  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.7  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.6  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.5  2001/08/10  22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.4  2001-07-30 17:40:52-07  caress
 * Fixed typos.
 *
 * Revision 5.3  2001/07/26  03:40:56  caress
 * Fixed handling of sidescan.
 *
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/06/30  17:40:14  caress
 * Release 5.0.beta02
 *
 * Revision 5.0  2001/06/29  22:49:07  caress
 * Added support for HSDS2RAW
 *
 *
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
#define MBSYS_ATLAS_C
#include "mbsys_atlas.h"

static char rcs_id[]="$Id: mbsys_atlas.c 1917 2012-01-10 19:25:33Z caress $";

/*--------------------------------------------------------------------*/
int mbsys_atlas_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_atlas_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_atlas_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_atlas_struct *) *store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	
	/* start telegram */
	store->start_ping_no = 0;		/* ping number */
	store->start_transmit_time_d = 0.0;	/* ping timestamp */
	for (i=0;i<32;i++)
	    store->start_opmode[i] = 0;		/* 32 single byte mode indicators:			*/
						/*	start_opmode[ 0] = OPMODE_SOUNDING = 0		*/
						/*		OM_SOUNDING_OFF = 0			*/
						/*		OM_SOUNDING_ON = 1			*/
						/*	start_opmode[ 1] = OPMODE_SEARCH = 1		*/
						/*		OM_SEARCH_OFF = 0			*/
						/*		OM_SEARCH_ON = 1			*/
						/*	start_opmode[ 2] = OPMODE_SIMULATION = 2	*/
						/*		OM_SIMULATION_OFF = 0			*/
						/*		OM_SIMULATION_ON = 1			*/
						/*	start_opmode[ 3] = OPMODE_COVERAGE = 3		*/
						/*		OM_COVERAGE_90_DEG = 0			*/
						/*		OM_COVERAGE_120_DEG = 1			*/
						/*	start_opmode[ 4] = OPMODE_SUBRANGE = 4		*/
						/*		OM_SUBRANGE_0 = 0			*/
						/*		OM_SUBRANGE_1 = 1			*/
						/*		OM_SUBRANGE_2 = 2			*/
						/*		OM_SUBRANGE_3 = 3			*/
						/*	start_opmode[ 5] = OPMODE_DUMMY1 = 5		*/
						/*	start_opmode[ 6] = OPMODE_RANGE = 6		*/
						/*		OM_RANGE_SHALLOW_WATER = 0		*/
						/*		OM_RANGE_MEDIUM_DEPTH = 1		*/
						/*		OM_RANGE_DEEP_SEA = 2			*/
						/*	start_opmode[ 7] = OPMODE_DUMMY2 = 7		*/
						/*	start_opmode[ 8] = OPMODE_SWATH = 8		*/
						/*		OM_SWATH_FULL = 0			*/
						/*		OM_SWATH_HALF = 1			*/
						/*	start_opmode[ 9] = OPMODE_SIDE = 9		*/
						/*		OM_SIDE_PORT = 0			*/
						/*		OM_SIDE_STAR = 1			*/
						/*	start_opmode[10] = OPMODE_HOPPING = 10		*/
						/*		OM_HOPPING_OFF = 0			*/
						/*		OM_HOPPING_ON = 1			*/
						/*	start_opmode[11] = OPMODE_SEQUENCE = 11		*/
						/*		OM_SEQUENCE_NORMAL = 0			*/
						/*		OM_SEQUENCE_REVERSE = 1			*/
						/*	start_opmode[12] = OPMODE_CALIBR = 12		*/
						/*		OM_CALIBRATION_OFF = 0			*/
						/*		OM_CALIBRATION_ON = 1			*/
						/*	start_opmode[13] = OPMODE_TEST = 13		*/
						/*		OM_TEST_OFF = 0				*/
						/*		OM_TEST_TRANSMITTER_FULL = 1		*/
						/*		OM_TEST_TRANSMITTER_1_GROUP = 2		*/
						/*		OM_TEST_RECEIVER = 3			*/
						/*	start_opmode[14] = OPMODE_SONARTYPE = 14	*/
						/*		OM_SONAR_FS20 = 0			*/
						/*		OM_SONAR_FS10 = 1			*/
						/*		OM_SONAR_Boma = 2			*/
						/*		OM_SONAR_MD = 3				*/
						/*		OM_SONAR_MD2 = 4			*/
						/*		OM_SONAR_DS = 5				*/
						/*		OM_SONAR_DS2 = 6			*/
						/*		OM_SONAR_VLOT = 7			*/
						/*		OM_SONAR_VLOT2 = 8			*/
						/*	start_opmode[15] = OPMODE_EXTENSION = 15	*/
						/*		OM_EXTENSION_NOT_USED = 0		*/
						/*	start_opmode[16] = OPMODE_FREQUENCY = 16	*/
						/*		OM_FREQUENCY_HIGH = 0			*/
						/*		OM_FREQUENCY_LOW = 1			*/
						/*	start_opmode[17] = OPMODE_TRANS_MODE = 17	*/
						/*		OM_TRANSMISSION_MODE_0 = 0		*/
						/*		OM_TRANSMISSION_MODE_1 = 1		*/
						/*		OM_TRANSMISSION_MODE_2 = 2		*/
						/*		OM_TRANSMISSION_MODE_3 = 3		*/
						/*		OM_TRANSMISSION_MODE_4 = 4		*/
						/*		OM_TRANSMISSION_MODE_5 = 5		*/
						/*		OM_TRANSMISSION_MODE_6 = 6		*/
						/*		OM_TRANSMISSION_MODE_7 = 7		*/
						/*	start_opmode[18] = OPMODE_RESERVED_18 = 18	*/
						/*	start_opmode[19] = OPMODE_RESERVED_19 = 19	*/
						/*	start_opmode[20] = OPMODE_RESERVED_20 = 20	*/
						/*	start_opmode[21] = OPMODE_RESERVED_21 = 21	*/
						/*	start_opmode[22] = OPMODE_RESERVED_22 = 22	*/
						/*	start_opmode[23] = OPMODE_RESERVED_23 = 23	*/
						/*	start_opmode[24] = OPMODE_RESERVED_24 = 24	*/
						/*	start_opmode[25] = OPMODE_RESERVED_25 = 25	*/
						/*	start_opmode[26] = OPMODE_RESERVED_26 = 26	*/
						/*	start_opmode[27] = OPMODE_RESERVED_27 = 27	*/
						/*	start_opmode[28] = OPMODE_RESERVED_28 = 28	*/
						/*	start_opmode[29] = OPMODE_RESERVED_29 = 29	*/
						/*	start_opmode[30] = OPMODE_RESERVED_30 = 30	*/
						/*	start_opmode[31] = OPMODE_RESERVED_31 = 31	*/

	store->start_heave = 0.0;		/* heave at transmit (m) */
	store->start_roll = 0.0;		/* roll at transmit (radians) */
	store->start_pitch = 0.0;		/* pitch at transmit (radians) */
	store->start_heading = 0.0;		/* heading at transmit (radians) */
	store->start_ckeel = 0.0;		/* water sound speed at transducer (m/s) */
	store->start_cmean = 0.0;		/* mean water sound speed (m/s) */
	store->start_depth_min = 0.0;	/* minimum depth from GUI (m) */
	store->start_depth_max = 0.0;	/* maximum depth from GUI (m) */
	
	/* travel times telegrams */
	store->tt_ping_no = 0;		/* ping number */
	store->tt_transmit_time_d = 0.0;/* ping timestamp */
	store->tt_beam_table_index = 0;	/* index to beam angle table	*/
					/* tt_beam_table_index = 1 : ds2_ang_120[] */
					/* tt_beam_table_index = 2 : ds2_ang_90[] */
	store->tt_beam_cnt = 0;		/* number of beam values in this ping (max 1440) */
	store->tt_long1 = 0;		/* reserve */
	store->tt_long2 = 0;		/* reserve */
	store->tt_long3 = 0;		/* reserve */
	store->tt_xdraught = 0;		/* draft flag: */
						/* tt_xdraught = 0 : inst-draft	*/
						/* tt_xdraught = 1 : system-draft	*/
	store->tt_double1 = 0.0;		/* DS2: backscatter TVG (dB?) */
						/* FS10: period of time */
	store->tt_double2 = 0.0;		/* FS10: data age */
	store->tt_sensdraught = 0.0;		/* sens/inst draft */
	store->tt_draught = 0.0;		/* system draft (m) */
	for (i=0;i<MBSYS_ATLAS_MAXBEAMS;i++)
	    {
	    store->tt_lruntime[i] = 0.0;	/* array of beam traveltimes with   */
						/* each entry related to the beam   */
						/* angle in the actual angle table  */
	    store->tt_lamplitude[i] = 0;	/* array of beam amplitudes:	    */
	    store->tt_lstatus[i] = 0;		/* array of beam states:	    */
						/*	DS2: NIS data		    */
						/*	FS:			    */
						/*	    bit 0 => beamside	    */
						/*		0 = port	    */
						/*		1 = starboard	    */
						/*	    bit 1 => lobe	    */
						/*		0 = cond. lobe	    */
						/*		1 = wide lobe	    */
						/*	    bit 2 => valid	    */
						/*		0 = unvalid	    */
						/*		1 = valid	    */
						/*	    bits 3-7 unused	    */
	    }
	    
	/* processed bathymetry */
	store->pr_navlon = 0.0;			/* longitude (degrees) */
	store->pr_navlat = 0.0;			/* latitude (degrees) */
	store->pr_speed = 0.0;			/* speed made good (m/s) */
	for (i=0;i<MBSYS_ATLAS_MAXBEAMS;i++)
	    {
	    store->pr_bath[i] = 0.0;			/* bathymetry (m) */
	    store->pr_bathacrosstrack[i] = 0.0;		/* acrosstrack distance (m) */
	    store->pr_bathalongtrack[i] = 0.0;		/* alongtrack distance (m) */
	    store->pr_beamflag[i] = MB_FLAG_NULL;	/* beam edit/status flags */
	    }
	
	/* sidescan telegrams */
	store->ss_ping_no = 0;		/* ping number */
	store->ss_transmit_time_d = 0.0;	/* ping timestamp */
	store->ss_timedelay = 0.0;		/* time from transmit to first sidescan value (s) */
	store->ss_timespacing = 0.0;		/* time spacing between sidescan values (s) */
	store->ss_max_side_bb_cnt = 0;	/* total number of values to port */
	store->ss_max_side_sb_cnt = 0;	/* total number of values to starboard */
	for (i=0;i<MBSYS_ATLAS_MAXPIXELS;i++)
	    store->ss_sidescan[i] = 0;

	/* tracking windows telegram */
	store->tr_transmit_time_d = 0.0;	/* ping timestamp */
	store->tr_ping_no = 0;		/* ping number */
	store->tr_window_mode = 0;		/* window mode */
	store->tr_no_of_win_groups = 0;	/* number of window groups  */
						/* DS2 & MD => 8	    */
						/* Fansweep => 20	    */
	for (i=0;i<MBSYS_ATLAS_MAXWINDOWS;i++)
	    {
	    store->tr_repeat_count[i] = 0;	/* this window repeats n times  */
						/* DS2 => 6,8,8,8,8,8,8,5	    */
						/* MD => 5,5,5,5,5,5,5,5	    */
	    store->tr_start[i] = 0.0;		/* start time (s) - two way	    */
	    store->tr_stop[i] = 0.0;		/* stop time (s) - two way	    */
	    }
	
	/* backscatter telegram */	
	store->bs_transmit_time_d = 0.0;	/* ping timestamp */
	store->bs_ping_no = 0;		/* ping number */
	store->bs_nrActualGainSets = 0;	/* 10 to 20 gain sets */
	store->bs_rxGup = 0.0;		/* DS2: -175.0 dB relative to 1 V/uPa */
					/* MD: -185.0 dB relative to 1 V/uPa */
	store->bs_rxGain = 0.0;		/* scale : dB */
	store->bs_ar = 0.0;		/* scale : dB/m */
	for (i=0;i<MBSYS_ATLAS_HSDS2_RX_PAR;i++)
	    {
	    store->bs_TvgRx_time[i] = 0.0;	/* two way time (s) */
	    store->bs_TvgRx_gain[i] = 0.0;	/* receiver gain (dB) */
	    }
	store->bs_nrTxSets = 0;			/* number of transmit sets (1, 3, 5) */
	for (i=0;i<MBSYS_ATLAS_HSDS2_TX_PAR;i++)
	    {
	    store->bs_txBeamIndex[i] = 0;	/* code of external beamshape table */
	    store->bs_txLevel[i] = 0.0;		/* transmit level: dB relative to 1 uPa */
	    store->bs_txBeamAngle[i] = 0.0;	/* transmit beam angle (radians) */
	    store->bs_pulseLength[i] = 0.0;	/* transmit pulse length (s) */
	    }
	store->bs_nrBsSets = 0;			/* number of backscatter sets */
	for (i=0;i<MBSYS_ATLAS_HSDS2_PFB_NUM;i++)
	    {
	    store->bs_m_tau[i] = 0.0;		/* echo duration (s) */
	    store->bs_eff_ampli[i] = 0;		/* effective amplitude */
	    store->bs_nis[i] = 0;		/* noise isotropic */
	    }
	    
	/* comment */
	for (i=0;i<MBSYS_ATLAS_COMMENT_LENGTH;i++)
	    {
	    store->comment[i] = '\0';
	    }

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
int mbsys_atlas_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_atlas_deall";
	int	status = MB_SUCCESS;
	struct mbsys_atlas_struct *store;

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
	store = (struct mbsys_atlas_struct *) *store_ptr;

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

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
int mbsys_atlas_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_atlas_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = store->tt_beam_cnt;
		*namp = *nbath;
		*nss = store->ss_max_side_bb_cnt 
			+ store->ss_max_side_sb_cnt;
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
int mbsys_atlas_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_atlas_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	double	pixel_size;
	double	range, tt, ttmin, ssdepth;
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
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{		
		/* get time */
		*time_d = store->start_transmit_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->pr_navlon;
		*navlat = store->pr_navlat;

		/* get heading */
		*heading = RTD * store->start_heading;

		/* get speed  */
		*speed = 3.6 * store->pr_speed;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.3;
		mb_io_ptr->beamwidth_xtrack = 2.3;

		/* read distance and depth values into storage arrays */
		*nbath = store->tt_beam_cnt;
		for (i=0;i<MBSYS_ATLAS_MAXBEAMS;i++)
			{
			bath[i] = 0.0;
			beamflag[i] = MB_FLAG_NULL;
			amp[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			}
		ttmin = 999999.9;
		for (i=0;i<store->tt_beam_cnt;i++)
			{
			bath[i] = store->pr_bath[i];
			beamflag[i] = store->pr_beamflag[i];
			bathacrosstrack[i] = store->pr_bathacrosstrack[i];
			bathalongtrack[i] = store->pr_bathalongtrack[i];
			amp[i] = store->tt_lamplitude[i];
			if (beamflag[i] != MB_FLAG_NULL
				&& store->tt_lruntime[i] < ttmin)
				ttmin = store->tt_lruntime[i];
			}
		*namp = *nbath;
		*nss = store->ss_max_side_bb_cnt 
			+ store->ss_max_side_sb_cnt;
		pixel_size = store->start_cmean * store->ss_timespacing;
		ssdepth = store->start_cmean * ttmin / 2.0;
		for (i=0;i<*nss;i++)
			{
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			}
		for (i=0;i<store->ss_max_side_bb_cnt;i++)
			{
			j = store->ss_max_side_bb_cnt - i;
			tt = store->ss_timedelay + store->ss_timespacing * (i - 1);
			if (tt > ttmin)
				{ 
				ss[j] = store->ss_sidescan[i];
				range = store->start_cmean * tt / 2.0;
				ssacrosstrack[j] = -sqrt(range * range - ssdepth * ssdepth);
				ssalongtrack[j] = 0.0;
				}
			}
		for (i=store->ss_max_side_bb_cnt;i<*nss;i++)
			{
			tt = store->ss_timedelay + store->ss_timespacing * (i - store->ss_max_side_bb_cnt);
			if (tt > ttmin)
				{ 
				ss[i] = store->ss_sidescan[i];
				range = store->start_cmean * tt / 2.0;
				ssacrosstrack[i] = sqrt(range * range - ssdepth * ssdepth);
				ssalongtrack[i] = 0.0;
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

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(comment,store->comment,
			MBSYS_ATLAS_COMMENT_LENGTH);

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
int mbsys_atlas_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_atlas_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	double	xtrackmin;
	int	centerpixel;
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
	if (verbose >= 2 && kind == MB_DATA_DATA)
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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{		
		/* get time */
		store->start_transmit_time_d = time_d;

		/* get navigation */
		store->pr_navlon = navlon;
		store->pr_navlat = navlat;

		/* get heading */
		store->start_heading = DTR * heading;

		/* get speed  */
		store->pr_speed = speed / 3.6;

		/* read distance and depth values into storage arrays */
		store->tt_beam_cnt = nbath;
		for (i=0;i<store->tt_beam_cnt;i++)
			{
			store->pr_bath[i] = bath[i];
			store->pr_beamflag[i] = beamflag[i];
			store->pr_bathacrosstrack[i] = bathacrosstrack[i];
			store->pr_bathalongtrack[i] = bathalongtrack[i];
			store->tt_lamplitude[i] = amp[i];
			}
		if (store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt != nss)
			{
			xtrackmin = 99999.9;
			centerpixel = 0;
			for (i=0;i<nss;i++)
				{
				if (ss[i] > 0.0 && fabs(ssacrosstrack[i]) < xtrackmin)
					{
					xtrackmin = fabs(ssacrosstrack[i]);
					centerpixel = i;
					}
				}
			if (centerpixel > 0)
				{
				store->ss_max_side_bb_cnt = centerpixel;
				store->ss_max_side_sb_cnt = nss - centerpixel;
				}
			else
				{
				store->ss_max_side_bb_cnt = nss / 2;
				store->ss_max_side_sb_cnt = nss / 2;
				}
			}
		for (i=0;i<store->ss_max_side_bb_cnt;i++)
			{
			store->ss_sidescan[i] = ss[store->ss_max_side_bb_cnt - i];
			}
		for (i=store->ss_max_side_bb_cnt;i<nss;i++)
			{
			store->ss_sidescan[i] = ss[i];
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->comment,comment,
			MBSYS_ATLAS_COMMENT_LENGTH);
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
int mbsys_atlas_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_atlas_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	double	*angle_table;
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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get angle_table for 90 degree coverage */
		if (store->start_opmode[3] == 0)
		    {
		    if (store->tt_beam_cnt == 140)
			angle_table = (double *) ds2_ang_90d_140b;
		    else if (store->tt_beam_cnt == 59)
			angle_table = (double *) ds2_ang_90d_59b;
		    }

		/* get angle_table for 120 degree coverage */
		else if (store->start_opmode[3] == 1)
		    {
		    if (store->tt_beam_cnt == 140)
			angle_table = (double *) ds2_ang_120d_140b;
		    else if (store->tt_beam_cnt == 59)
			angle_table = (double *) ds2_ang_120d_59b;
		    }

		/* get travel times */
		*nbeams = store->tt_beam_cnt;
		for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = 0.0;
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
			}
		for (i=0;i<store->tt_beam_cnt;i++)
			{
			ttimes[i] = store->tt_lruntime[i];
			angles[i] = RTD * fabs(angle_table[i]);
			if (angle_table[i] < 0.0)
			    angles_forward[i] = 180.0;
			else
			    angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = store->start_heave;
			alongtrack_offset[i] = 0.0;
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
int mbsys_atlas_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	int *detects, int *error)
{
	char	*function_name = "mbsys_atlas_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	int	detect;
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
		fprintf(stderr,"dbg2       detects:    %lu\n",(size_t)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get sonar type */
		if (store->start_opmode[14] <= 1)
			detect = MB_DETECT_PHASE;
		else
			detect = MB_DETECT_AMPLITUDE;
			
		/* get detects */
		*nbeams = store->tt_beam_cnt;
		for (i=0;i<store->tt_beam_cnt;i++)
			{
			detects[i] = detect;
			}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
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
int mbsys_atlas_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_atlas_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	double	bath_best;
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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transducer depth and altitude */
		*transducer_depth = store->tt_draught + store->start_heave;
		found = MB_NO;
		bath_best = 0.0;
		xtrack_min = 99999999.9;
		for (i=0;i<store->tt_beam_cnt;i++)
		    {
		    if (mb_beam_ok(store->pr_beamflag[i])
			&& fabs(store->pr_bathacrosstrack[i]) < xtrack_min)
			{
			xtrack_min = fabs(store->pr_bathacrosstrack[i]);
			bath_best = store->pr_bath[i];
			found = MB_YES;
			}
		    }		
		if (found == MB_NO)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<store->tt_beam_cnt;i++)
			{
			if (store->pr_beamflag[i]!= MB_FLAG_NULL
			    && fabs(store->pr_bathacrosstrack[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(store->pr_bathacrosstrack[i]);
			    bath_best = store->pr_bath[i];
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
int mbsys_atlas_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_atlas_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->start_transmit_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->pr_navlon;
		*navlat = store->pr_navlat;

		/* get heading */
		*heading = RTD * store->start_heading;

		/* get speed  */
		*speed = 3.6 * store->pr_speed;

		/* get draft  */
		*draft = store->tt_draught;

		/* get roll pitch and heave */
		*roll = RTD * store->start_roll;
		*pitch = RTD * store->start_pitch;
		*heave = store->start_heave;

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
int mbsys_atlas_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_atlas_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

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
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{		
		/* get time */
		store->start_transmit_time_d = time_d;

		/* get navigation */
		store->pr_navlon = navlon;
		store->pr_navlat = navlat;

		/* get heading */
		store->start_heading = DTR * heading;

		/* get speed  */
		store->pr_speed = speed / 3.6;

		/* get draft  */
		store->tt_draught = draft;

		/* get roll pitch and heave */
		store->start_roll = DTR * roll;
		store->start_pitch = DTR * pitch;
		store->start_heave = heave;
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
int mbsys_atlas_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_atlas_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	struct mbsys_atlas_struct *copy;

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
	store = (struct mbsys_atlas_struct *) store_ptr;
	copy = (struct mbsys_atlas_struct *) copy_ptr;

	/* copy the main structure */
	*copy = *store;

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
int mbsys_atlas_ttcorr(int verbose, void *mbio_ptr, 
			void *store_ptr,
			int *error)
{
	char	*function_name = "mbsys_atlas_ttcorr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
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

	/* get data structure pointers */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* check for correct kind of data - hsd2 */
	if (store->start_opmode[14] == 6
		&& store->kind == MB_DATA_DATA
		&& store->tt_beam_cnt == 140)
		{
		if (store->start_opmode[6] == 1)
			{
			for (i=0;i<store->tt_beam_cnt;i++)
				{
				store->tt_lruntime[i] += 0.001 * DS2_TimeCorrMedium1[i];
				}
			}
		else if (store->start_opmode[6] == 2)
			{
			for (i=0;i<store->tt_beam_cnt;i++)
				{
				store->tt_lruntime[i] -= DS2_TimeCorrDeep3[i];
				}
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
