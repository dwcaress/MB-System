/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_atlas.h	6/22/01
 *	$Id: mbsys_atlas.h,v 5.5 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 2001, 2002 by
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
 * mbsys_atlas.h defines the MBIO data structures for handling data from 
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
 *
 * Author:	D. W. Caress
 * Author:	D. N. Chayes
 * Date:	June 22, 2001
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.3  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.2  2001/08/10 22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.1  2001-07-19 17:32:54-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2001/06/29  22:49:07  caress
 * Added support for HSDS2RAW
 *
 *
 */
/*
 * Notes on the MBSYS_ATLAS data structure:
 *
 * 1) STN Atlas Marine Electronics (aka SAM) sonars write raw data in real-time
 *    as binary XDR encoded data. Files are stored on disk by the HYDROMAP 
 *    Online workstation. The workstation on the Ewing is an HP Vectra
 *    running SuSe Linux (2.2 kernel.)
 *
 * 2) Files are 10 minutes long.
 *
 * 3) Multiple (parallel files are created). For example:
 *
 *  64k 00010529004010.adt
 *  28k 00010529004010.ang      Angle data
 *  28k 00010529004010.cdt      Sound speed profile(s)
 * 8.0k 00010529004010.evt      Event data, comments, etc.
 * 344k 00010529004010.fsw      Sounding (travel time, sidescan, backscatter...
 * 148k 00010529004010.lev      Level (tide gage) data
 * 664k 00010529004010.lot	center beam (telegram 26) and lots of zeros
 *  60k 00010529004010.mot      Motion sensor (attitude)
 * 388k 00010529004010.nav      Navigation sensors
 *
 * lot file:    26 (center beam) and lots of zeros
 *
 * For starters (June, 2001) we are assuming fixed beam spacing w/ a 90 or 
 * 120 degree swath so all of the swath mapping data we need is in the  ".fsw"
 * data file. 
 *
 * 4) Data record type ids:
 *    MB-System critical records are:
 *
 *      Sounding data     10   T0, nav, attitude, 
 *      Depths            11   travel time+amp/beam 
 *      Sidescan          12   Multiple packets of sidescan data
 *      Tracking windows  19   
 *      Amplitude         28   TX and RX data necessary for calculating true amp
 *
 *    Other potentially interesting things:
 *      "Center beam"     26
 *
 * 5) Time values are in Unix seconds (seconds since 1/1/1970 00:00:00
 */
 
/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* sonar models */
#define	MBSYS_ATLAS_UNKNOWN	0
#define	MBSYS_ATLAS_HSDS2	1
#define	MBSYS_ATLAS_FANSWEEP15	2
#define	MBSYS_ATLAS_FANSWEEP20	3

/* maximum number of beams and pixels */
#define	MBSYS_ATLAS_MAXBEAMS		1440
#define	MBSYS_ATLAS_MAXPIXELS		4096
#define	MBSYS_ATLAS_MAXBEAMTELEGRAM	200
#define	MBSYS_ATLAS_MAXPIXELTELEGRAM	1024
#define	MBSYS_ATLAS_MAXWINDOWS		100
#define	MBSYS_ATLAS_HSDS2_PFB_NUM	59 
#define	MBSYS_ATLAS_HSDS2_RX_PAR		20 
#define	MBSYS_ATLAS_HSDS2_TX_PAR		10 
#define	MBSYS_ATLAS_COMMENT_LENGTH	256
#define	MBSYS_ATLAS_HSDS2_MAXBEAMS	140

/* datagram type id's */
#define	MBSYS_ATLAS_TELEGRAM_NONE		0
#define	MBSYS_ATLAS_TELEGRAM_START		10
#define	MBSYS_ATLAS_TELEGRAM_TRAVELTIMES		11
#define	MBSYS_ATLAS_TELEGRAM_SIDESCAN		12
#define	MBSYS_ATLAS_TELEGRAM_TRACKINGWINDOWS	19
#define	MBSYS_ATLAS_TELEGRAM_CENTERBEAM		26
#define	MBSYS_ATLAS_TELEGRAM_BACKSCATTER		28
#define	MBSYS_ATLAS_TELEGRAM_SYSTEM		40
#define	MBSYS_ATLAS_TELEGRAM_HSDS2LAM		1179799367
#define	MBSYS_ATLAS_TELEGRAM_COMMENTLAM		1129270605

/* internal data structure for survey data */
struct mbsys_atlas_struct
	{
	int		kind;
	
	/* navigation telegram */
	double		sys_pos_lat;		/* latitude (radians) */
	double		sys_pos_lon;		/* longitude (radians) */
	char		sys_pos_sensor[8];	/* position sensor name */
	double		sys_pos_time_d;		/* position fix time (s) */
	
	/* start telegram */
	unsigned int	start_ping_no;		/* ping number */
	double		start_transmit_time_d;	/* ping timestamp */
	mb_u_char	start_opmode[32];	/* 32 single byte mode indicators:			*/
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

	double		start_heave;		/* heave at transmit (m) */
	double		start_roll;		/* roll at transmit (radians) */
	double		start_pitch;		/* pitch at transmit (radians) */
	double		start_heading;		/* heading at transmit (radians) */
	double		start_ckeel;		/* water sound speed at transducer (m/s) */
	double		start_cmean;		/* mean water sound speed (m/s) */
	double		start_depth_min;	/* minimum depth from GUI (m) */
	double		start_depth_max;	/* maximum depth from GUI (m) */
	
	/* travel times telegrams */
	unsigned int	tt_ping_no;		/* ping number */
	double		tt_transmit_time_d;	/* ping timestamp */
	int		tt_beam_table_index;	/* index to beam angle table	*/
						/*	tt_beam_table_index = 1 : ds2_ang_120[] */
						/*	tt_beam_table_index = 2 : ds2_ang_90[] */
	int		tt_beam_cnt;		/* number of beam values in this ping (max 1440) */
	int		tt_long1;		/* reserve */
	int		tt_long2;		/* reserve */
	int		tt_long3;		/* reserve */
	int		tt_xdraught;		/* draft flag: */
						/*	tt_xdraught = 0 : inst-draft	*/
						/*	tt_xdraught = 1 : system-draft	*/
	double		tt_double1;		/* DS2: backscatter TVG (dB?) */
						/* FS10: period of time */
	double		tt_double2;		/* FS10: data age */
	double		tt_sensdraught;		/* sens/inst draft */
	double		tt_draught;		/* system draft (m) */
	float		tt_lruntime[MBSYS_ATLAS_MAXBEAMS];	/* array of beam traveltimes with   */
								/* each entry related to the beam   */
								/* angle in the actual angle table  */
	mb_u_char	tt_lamplitude[MBSYS_ATLAS_MAXBEAMS];	/* array of beam amplitudes:	    */
	mb_u_char	tt_lstatus[MBSYS_ATLAS_MAXBEAMS];	/* array of beam states:	    */
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
								
	/* processed bathymetry */
	double		pr_navlon;					/* longitude (degrees) */
	double		pr_navlat;					/* latitude (degrees) */
	double		pr_speed;					/* speed made good (m/s) */
	double		pr_bath[MBSYS_ATLAS_MAXBEAMS];			/* bathymetry (m) */
	double		pr_bathacrosstrack[MBSYS_ATLAS_MAXBEAMS];	/* acrosstrack distance (m) */
	double		pr_bathalongtrack[MBSYS_ATLAS_MAXBEAMS];		/* alongtrack distance (m) */
	char		pr_beamflag[MBSYS_ATLAS_MAXBEAMS];		/* beam edit/status flags */
	
	/* sidescan telegrams */
	unsigned int	ss_ping_no;		/* ping number */
	double		ss_transmit_time_d;	/* ping timestamp */
	double		ss_timedelay;		/* time from transmit to first sidescan value (s) */
	double		ss_timespacing;		/* time spacing between sidescan values (s) */
	int		ss_max_side_bb_cnt;	/* total number of values to port */
	int		ss_max_side_sb_cnt;	/* total number of values to starboard */
	mb_u_char		ss_sidescan[MBSYS_ATLAS_MAXPIXELS];

	/* tracking windows telegram */
	double		tr_transmit_time_d;	/* ping timestamp */
	unsigned int	tr_ping_no;		/* ping number */
	int		tr_window_mode;		/* window mode */
	int		tr_no_of_win_groups;	/* number of window groups  */
						/* DS2 & MD => 8	    */
						/* Fansweep => 20	    */
	int		tr_repeat_count[MBSYS_ATLAS_MAXWINDOWS];	    /* this window repeats n times  */
								    /* DS2 => 6,8,8,8,8,8,8,5	    */
								    /* MD => 5,5,5,5,5,5,5,5	    */
	float		tr_start[MBSYS_ATLAS_MAXWINDOWS];	    /* start time (s) - two way	    */
	float		tr_stop[MBSYS_ATLAS_MAXWINDOWS];		    /* stop time (s) - two way	    */
	
	/* backscatter telegram */	
	double		bs_transmit_time_d;	/* ping timestamp */
	int		bs_ping_no;		/* ping number */
	unsigned short	bs_nrActualGainSets;	/* 10 to 20 gain sets */
	float		bs_rxGup;		/* DS2: -175.0 dB relative to 1 V/uPa */
						/* MD: -185.0 dB relative to 1 V/uPa */
	float		bs_rxGain;		/* scale : dB */
	float		bs_ar;			/* scale : dB/m */
	float		bs_TvgRx_time[MBSYS_ATLAS_HSDS2_RX_PAR];	    /* two way time (s) */
	float		bs_TvgRx_gain[MBSYS_ATLAS_HSDS2_RX_PAR];	    /* receiver gain (dB) */
	unsigned short	bs_nrTxSets;				    /* number of transmit sets (1, 3, 5) */
	unsigned int	bs_txBeamIndex[MBSYS_ATLAS_HSDS2_TX_PAR];    /* code of external beamshape table */
	float		bs_txLevel[MBSYS_ATLAS_HSDS2_TX_PAR];	    /* transmit level: dB relative to 1 uPa */
	float		bs_txBeamAngle[MBSYS_ATLAS_HSDS2_TX_PAR];    /* transmit beam angle (radians) */
	float		bs_pulseLength[MBSYS_ATLAS_HSDS2_TX_PAR];    /* transmit pulse length (s) */
	unsigned short	bs_nrBsSets;				    /* number of backscatter sets */
	float		bs_m_tau[MBSYS_ATLAS_HSDS2_PFB_NUM];	    /* echo duration (s) */
	char		bs_eff_ampli[MBSYS_ATLAS_HSDS2_PFB_NUM];	    /* effective amplitude */
	char		bs_nis[MBSYS_ATLAS_HSDS2_PFB_NUM];	    /* noise isotropic */
	
	/* comment */
	char		comment[MBSYS_ATLAS_COMMENT_LENGTH];
	};
	

/*
 * These two arrays represent the angle from the vertical for each of the 
 * two modes of operation of the HS-DS2 on the Ewing in June, 2001.
 * 
 * The values were extracted from a sample "raw" angle data file.
 *
 */
#ifndef MBSYS_ATLAS_C
extern double ds2_ang_120d_59b[];
extern double ds2_ang_90d_59b[];
extern double ds2_ang_120d_140b[];
extern double ds2_ang_90d_140b[];
#else
double ds2_ang_120d_59b[] = 
	{
	-1.040042,
	-1.004178,
	-0.968315,
	-0.932451,
	-0.896588,
	-0.860724,
	-0.824861,
	-0.788997,
	-0.753134,
	-0.717270,
	-0.681407,
	-0.645543,
	-0.609680,
	-0.573816,
	-0.537953,
	-0.502089,
	-0.466226,
	-0.430362,
	-0.394499,
	-0.358635,
	-0.322772,
	-0.286908,
	-0.251045,
	-0.215181,
	-0.179318,
	-0.143454,
	-0.107591,
	-0.071727,
	-0.035864,
	0.000000,
	0.035864,
	0.071727,
	0.107591,
	0.143454,
	0.179318,
	0.215181,
	0.251045,
	0.286908,
	0.322772,
	0.358635,
	0.394499,
	0.430362,
	0.466226,
	0.502089,
	0.537953,
	0.573816,
	0.609680,
	0.645543,
	0.681407,
	0.717270,
	0.753134,
	0.788997,
	0.824861,
	0.860724,
	0.896588,
	0.932451,
	0.968315,
	1.004178,
	1.040042
	};
	
double ds2_ang_90d_59b[] = 
	{
	-0.779988,
	-0.753092,
	-0.726195,
	-0.699299,
	-0.672403,
	-0.645507,
	-0.618611,
	-0.591715,
	-0.564819,
	-0.537923,
	-0.511026,
	-0.484130,
	-0.457234,
	-0.430338,
	-0.403442,
	-0.376546,
	-0.349650,
	-0.322754,
	-0.295857,
	-0.268961,
	-0.242065,
	-0.215169,
	-0.188273,
	-0.161377,
	-0.134481,
	-0.107584,
	-0.080688,
	-0.053792,
	-0.026896,
	0.000000,
	0.026896,
	0.053792,
	0.080688,
	0.107584,
	0.134481,
	0.161377,
	0.188273,
	0.215169,
	0.242065,
	0.268961,
	0.295857,
	0.322754,
	0.349650,
	0.376546,
	0.403442,
	0.430338,
	0.457234,
	0.484130,
	0.511026,
	0.537923,
	0.564819,
	0.591715,
	0.618611,
	0.645507,
	0.672403,
	0.699299,
	0.726195,
	0.753092,
	0.779988
	};
	
double ds2_ang_120d_140b[] = 
	{
	-1.040042,
	-1.025077,
	-1.010112,
	-0.995148,
	-0.980183,
	-0.965219,
	-0.950254,
	-0.935289,
	-0.920325,
	-0.905360,
	-0.890395,
	-0.875431,
	-0.860466,
	-0.845502,
	-0.830537,
	-0.815572,
	-0.800608,
	-0.785643,
	-0.770678,
	-0.755714,
	-0.740749,
	-0.725784,
	-0.710820,
	-0.695855,
	-0.680891,
	-0.665926,
	-0.650961,
	-0.635997,
	-0.621032,
	-0.606067,
	-0.591103,
	-0.576138,
	-0.561174,
	-0.546209,
	-0.531244,
	-0.516280,
	-0.501315,
	-0.486350,
	-0.471386,
	-0.456421,
	-0.441457,
	-0.426492,
	-0.411527,
	-0.396563,
	-0.381598,
	-0.366633,
	-0.351669,
	-0.336704,
	-0.321740,
	-0.306775,
	-0.291810,
	-0.276846,
	-0.261881,
	-0.246916,
	-0.231952,
	-0.216987,
	-0.202022,
	-0.187058,
	-0.172093,
	-0.157129,
	-0.142164,
	-0.127199,
	-0.112235,
	-0.097270,
	-0.082305,
	-0.067341,
	-0.052376,
	-0.037412,
	-0.022447,
	-0.007482,
	0.007482,
	0.022447,
	0.037412,
	0.052376,
	0.067341,
	0.082305,
	0.097270,
	0.112235,
	0.127199,
	0.142164,
	0.157129,
	0.172093,
	0.187058,
	0.202022,
	0.216987,
	0.231952,
	0.246916,
	0.261881,
	0.276846,
	0.291810,
	0.306775,
	0.321740,
	0.336704,
	0.351669,
	0.366633,
	0.381598,
	0.396563,
	0.411527,
	0.426492,
	0.441457,
	0.456421,
	0.471386,
	0.486350,
	0.501315,
	0.516280,
	0.531244,
	0.546209,
	0.561174,
	0.576138,
	0.591103,
	0.606067,
	0.621032,
	0.635997,
	0.650961,
	0.665926,
	0.680891,
	0.695855,
	0.710820,
	0.725784,
	0.740749,
	0.755714,
	0.770678,
	0.785643,
	0.800608,
	0.815572,
	0.830537,
	0.845502,
	0.860466,
	0.875431,
	0.890395,
	0.905360,
	0.920325,
	0.935289,
	0.950254,
	0.965219,
	0.980183,
	0.995148,
	1.010112,
	1.025077,
	1.040042
	};

double ds2_ang_90d_140b[] = 
	{
	-0.77998,
	-0.76876,
	-0.75754,
	-0.74631,
	-0.73509,
	-0.72387,
	-0.71265,
	-0.70142,
	-0.69020,
	-0.67898,
	-0.66775,
	-0.65653,
	-0.64531,
	-0.63409,
	-0.62286,
	-0.61164,
	-0.60042,
	-0.58919,
	-0.57797,
	-0.56675,
	-0.55553,
	-0.54430,
	-0.53308,
	-0.52186,
	-0.51063,
	-0.49941,
	-0.48819,
	-0.47697,
	-0.46574,
	-0.45452,
	-0.44330,
	-0.43207,
	-0.42085,
	-0.40963,
	-0.39841,
	-0.38718,
	-0.37596,
	-0.36474,
	-0.35352,
	-0.34229,
	-0.33107,
	-0.31985,
	-0.30862,
	-0.29740,
	-0.28618,
	-0.27496,
	-0.26373,
	-0.25251,
	-0.24129,
	-0.23006,
	-0.21884,
	-0.20762,
	-0.19640,
	-0.18517,
	-0.17395,
	-0.16273,
	-0.15150,
	-0.14028,
	-0.12906,
	-0.11784,
	-0.10661,
	-0.09539,
	-0.08417,
	-0.07294,
	-0.06172,
	-0.05050,
	-0.03928,
	-0.02805,
	-0.01683,
	-0.00561,
	0.005611,
	0.016834,
	0.028057,
	0.039280,
	0.050503,
	0.061726,
	0.072948,
	0.084171,
	0.095394,
	0.106617,
	0.117840,
	0.129063,
	0.140286,
	0.151508,
	0.162731,
	0.173954,
	0.185177,
	0.196400,
	0.207623,
	0.218845,
	0.230068,
	0.241291,
	0.252514,
	0.263737,
	0.274960,
	0.286183,
	0.297405,
	0.308628,
	0.319851,
	0.331074,
	0.342297,
	0.353520,
	0.364742,
	0.375965,
	0.387188,
	0.398411,
	0.409634,
	0.420857,
	0.432079,
	0.443302,
	0.454525,
	0.465748,
	0.476971,
	0.488194,
	0.499417,
	0.510639,
	0.521862,
	0.533085,
	0.544308,
	0.555531,
	0.566754,
	0.577976,
	0.589199,
	0.600422,
	0.611645,
	0.622868,
	0.634091,
	0.645314,
	0.656536,
	0.667759,
	0.678982,
	0.690205,
	0.701428,
	0.712651,
	0.723873,
	0.735096,
	0.746319,
	0.757542,
	0.768765,
	0.779988
	};

/*-----------------------------------------------------------------------------*/
/* Hydrosweep DS2 Transmit Instant Correction Tables 
 *
 * Mode: Medium Depth 1 ( 100 -  450 m), additive corr. values for runtimes [msec]
 *       Opmode[6] = 1
 */
double DS2_TimeCorrMedium1[] = {
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-24.000,
		-23.973,
		-23.907,
		-23.760,
		-23.520,
		-23.200,
		-22.840,
		-22.453,
		-22.067,
		-21.680,
		-21.320,
		-21.000,
		-20.760,
		-20.600,
		-20.533,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.507,
		-20.493,
		-20.453,
		-20.373,
		-20.253,
		-20.080,
		-19.880,
		-19.680,
		-19.467,
		-19.253,
		-19.053,
		-18.840,
		-18.627,
		-18.440,
		-18.267,
		-18.133,
		-18.053,
		-18.013,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-18.000,
		-17.920,
		-17.693,
		-17.240,
		-16.493,
		-15.507,
		-14.387,
		-13.173,
		-11.973,
		-10.760,
		-9.547,
		-8.347,
		-7.133,
		-6.013,
		-5.027,
		-4.267,
		-3.813,
		-3.587,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.507,
		-3.480,
		-3.413,
		-3.267,
		-3.027,
		-2.707,
		-2.347,
		-1.960,
		-1.560,
		-1.173,
		-0.813,
		-0.493,
		-0.253,
		-0.093,
		-0.027,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000};

/*
 * 
 * Mode: Medium Depth 2 ( 400 - 1000 m), additive corr. values for runtimes [msec]
 *       Opmode[6] = 1
 */
double DS2_TimeCorrMedium2[] = {
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-34.000,
		-33.960,
		-33.867,
		-33.653,
		-33.307,
		-32.853,
		-32.333,
		-31.787,
		-31.227,
		-30.667,
		-30.147,
		-29.693,
		-29.347,
		-29.147,
		-29.040,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-29.000,
		-28.987,
		-28.933,
		-28.827,
		-28.640,
		-28.400,
		-28.133,
		-27.840,
		-27.547,
		-27.253,
		-26.973,
		-26.680,
		-26.387,
		-26.107,
		-25.880,
		-25.693,
		-25.587,
		-25.520,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.507,
		-25.400,
		-25.093,
		-24.453,
		-23.373,
		-21.987,
		-20.400,
		-18.680,
		-16.973,
		-15.267,
		-13.560,
		-11.840,
		-10.133,
		-8.533,
		-7.147,
		-6.080,
		-5.440,
		-5.107,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-5.000,
		-4.960,
		-4.867,
		-4.653,
		-4.307,
		-3.853,
		-3.333,
		-2.787,
		-2.227,
		-1.667,
		-1.147,
		-0.693,
		-0.347,
		-0.147,
		-0.040,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000};

/*
 * 
 * Mode: Deep Sea 3 ( 800 - 3700 m), additive corr. values for runtimes [msec]
 *       Opmode[6] = 2
 */
double DS2_TimeCorrDeep3[] = {
		-64.000,
		-64.000,
		-64.000,
		-64.000,
		-64.000,
		-64.000,
		-64.000,
		-63.960,
		-63.840,
		-63.587,
		-63.173,
		-62.627,
		-62.000,
		-61.333,
		-60.667,
		-60.000,
		-59.333,
		-58.667,
		-58.000,
		-57.333,
		-56.667,
		-56.000,
		-55.373,
		-54.840,
		-54.413,
		-54.173,
		-54.040,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-54.000,
		-53.973,
		-53.920,
		-53.787,
		-53.587,
		-53.307,
		-53.000,
		-52.667,
		-52.333,
		-52.000,
		-51.667,
		-51.333,
		-51.000,
		-50.667,
		-50.333,
		-50.000,
		-49.667,
		-49.333,
		-49.000,
		-48.693,
		-48.413,
		-48.213,
		-48.080,
		-48.027,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-48.000,
		-47.853,
		-47.387,
		-46.467,
		-44.947,
		-42.960,
		-40.653,
		-38.213,
		-35.760,
		-33.307,
		-30.867,
		-28.413,
		-25.960,
		-23.520,
		-21.067,
		-18.613,
		-16.307,
		-14.253,
		-12.600,
		-11.493,
		-10.787,
		-10.400,
		-10.200,
		-10.080,
		-10.027,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-10.000,
		-9.960,
		-9.840,
		-9.613,
		-9.227,
		-8.720,
		-8.133,
		-7.507,
		-6.880,
		-6.253,
		-5.627,
		-5.000,
		-4.387,
		-3.760,
		-3.133,
		-2.507,
		-1.880,
		-1.293,
		-0.787,
		-0.387,
		-0.160,
		-0.040,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000};

/*
 * 
 * Mode: Deep Sea 4 (3300 - max. m), additive corr. values for runtimes [msec]
 *       Opmode[6] = 2
 */
double DS2_TimeCorrDeep4[] = {
		-88.000,
		-88.000,
		-88.000,
		-88.000,
		-88.000,
		-88.000,
		-88.000,
		-87.947,
		-87.787,
		-87.453,
		-86.920,
		-86.213,
		-85.400,
		-84.533,
		-83.667,
		-82.800,
		-81.933,
		-81.067,
		-80.200,
		-79.333,
		-78.467,
		-77.600,
		-76.787,
		-76.080,
		-75.547,
		-75.213,
		-75.053,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-75.000,
		-74.973,
		-74.880,
		-74.693,
		-74.373,
		-73.973,
		-73.507,
		-73.000,
		-72.507,
		-72.000,
		-71.507,
		-71.000,
		-70.507,
		-70.000,
		-69.507,
		-69.000,
		-68.507,
		-68.000,
		-67.507,
		-67.040,
		-66.627,
		-66.320,
		-66.120,
		-66.027,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-66.000,
		-65.827,
		-65.307,
		-64.267,
		-62.520,
		-60.253,
		-57.640,
		-54.853,
		-52.067,
		-49.267,
		-46.493,
		-43.693,
		-40.907,
		-38.120,
		-35.333,
		-32.533,
		-29.760,
		-26.960,
		-24.173,
		-21.387,
		-18.773,
		-16.493,
		-14.747,
		-13.707,
		-13.173,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-13.000,
		-12.947,
		-12.800,
		-12.493,
		-11.987,
		-11.333,
		-10.573,
		-9.760,
		-8.947,
		-8.133,
		-7.320,
		-6.507,
		-5.693,
		-4.880,
		-4.067,
		-3.253,
		-2.440,
		-1.680,
		-1.013,
		-0.507,
		-0.200,
		-0.053,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000,
		0.000};

#endif
	
/* system specific function prototypes */
int mbsys_atlas_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_atlas_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_atlas_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_atlas_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_atlas_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_atlas_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_atlas_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_atlas_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_atlas_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_atlas_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

