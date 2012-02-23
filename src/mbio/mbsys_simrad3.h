/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad3.h	2/22/2008
 *	$Id$
 *
 *    Copyright (c) 2008-2012 by
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
 * mbsys_simrad3.h defines the MBIO data structures for handling data from 
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
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	February 22, 2008
 *
 * $Log: mbsys_simrad3.h,v $
 * Revision 5.2  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.1  2008/07/10 06:40:34  caress
 * Fixed support for EM122
 *
 * Revision 5.0  2008/03/01 09:11:35  caress
 * Added support for Simrad EM710 multibeam in new formats 58 and 59.
 *
 *
 */
/*
 * Notes on the MBSYS_SIMRAD3 data structure:
 *   1. Simrad multibeam systems output datagrams which are
 *      a combination of ascii and binary. This code has been written
 *      using data format specifications found in Simrad document
 *	16092ai EM Series Data Formats (rev I January 20, 2006).
 *   2. Simrad multibeam sonars output both bathymetry
 *      and amplitude information for beams and sidescan information
 *      with a higher resolution than the bathymetry and amplitude.
 *   3. This code and formats 58 and 59 support data from the 
 *         EM-710 multibeam sonar and later models. Data from older
 *         Simrad multibeams are supported by formats 51, 56, and 57.
  *   4. Each telegram is preceded by a two byte start code 0x02 and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not check the checksums
 *      on input, but does calculate the checksums for output.
 *   5. The Kongsberg Simrad datagram format manual lists a large number
 *      of datagram types. The complete list of telegram start codes, 
 *      types, and sizes is given below. Datagram listings preceded
 *      by an "*" are recognized by MB-System. Unrecognized datagrams
 *      will be skipped on input and not included in output files.
 *        *0x0230: Parameter - Stop                       variable size
 *        *0x0231: Parameter - Data out off               variable size
 *        *0x0232: Parameter - Data out on                variable size
 *        *0x0233: Extra Parameter                        variable size
 *        *0x0241: Attitude Output                        1222 bytes
 *        *0x0243: Clock Output                           28 bytes
 *        *0x0244: Bathymetry                             48-4092 bytes
 *         0x0245: Single beam echosounder depth          32 bytes
 *        *0x0246: Raw range and beam angle               24-2056 bytes
 *        *0x0247: Surface sound speed                    variable size
 *        *0x0248: Heading Output                         422 bytes
 *        *0x0249: Parameter - Start                      variable size
 *        *0x024A: Mechanical transducer tilt             variable size
 *         0x024B: Central beams echogram                 variable size
 *         0x024E: Raw range and beam angle (EM710 and later) variable size
 *        *0x0250: Position                               100-134 bytes
 *        *0x0252: Runtime Parameter                      52 bytes
 *        *0x0253: Sidescan                               48->5K bytes
 *        *0x0254: Tide Output                            30 bytes
 *        *0x0255: Sound velocity profile (new)           variable size
 *        *0x0256: Sound velocity profile (old)           variable size
 *         0x0257: SSP input                              variable size
 *         0x0258: Bathymetry  (EM710 and later)          variable size
 *         0x0259: Sidescan (EM710 and later)             variable size
 *        *0x0265: Raw range and beam angle               112-1658 bytes
 *        *0x0266: Raw range and beam angle               44-1658 bytes
 *        *0x0268: Height Output                          24 bytes
 *        *0x0269: Parameter - Stop                       variable size
 *        *0x026B: Water column                           variable size
 *        *0x026E: Network attitude                       variable size
 *         0x0270: Parameter - Remote                     variable size
 *         0x0273: Surface sound speed                    variable size
 *        *0x02E1: Bathymetry (MBARI format 57)           48-4092 bytes
 *        *0x02E2: Sidescan (MBARI format 57)             48->5K bytes
 *        *0x02E3: Bathymetry (MBARI format 59)           48-4092 bytes
 *        *0x02E4: Sidescan (MBARI format 59)             48->5K bytes
 *   6. Simrad systems record navigation fixes using the position 
 *      datagram; no navigation is included in the per ping data.  Thus,
 *      it is necessary to extrapolate the navigation for each ping
 *      at read time from the last navigation fix.  The frequency of
 *      GPS fixes generally assures that this is not a problem, but
 *      we offer no guarentees that this will always be the case.
 *      In this format the navigation fix datagrams include copies of
 *      the ASCII data records (typically NMEA 0183) input into the
 *      sonar by the navigation system.
 *   7. The beam depths are give relative to the transmit transducer 
 *      or sonar head depth and the horizontal location of the active 
 *      positioning system's antenna (or reference point). Heave,  
 *      roll, pitch, sound speed at the transducer depth and ray 
 *      bending have been applied. 
 *   8. The new Simrad sonars record the heading and attitude sensor
 *      data streams input into the sonar, usually at a sampling 
 *      frequency of 100 Hz.
\*   9. The beam angles and travel times are reported in the raw range 
 *      and angle datagrams. The raw times and angles are recorded 
 *      without correction for the vessel motion. The raw ranges are 
 *      given as two-way travel times.
 *  10. The sidescan is structured in terms of a certain number of samples
 *      per beam. The range sampling rate for the sidescan is the same as
 *      that specified in the depth datagram and the ranges in the sidescan
 *      datagram (seabed image datagram) are all two way travel times.
 *  11. The attitude data is output asynchronously with respect to the
 *      ping output datagrams. Typical motion sensors give data at a 100 Hz
 *      rate. The attitude datagrams are given when the number of measurements
 *      is 100, or usually at 1 second intervals. The attitude data timing
 *      is corrected for the sensor time delay entered by the operator. If
 *      roll is input with respect to the horizontal, then the sonar 
 *      recalculates the roll so that the output values are in the plane
 *      defined by the heading and pitch axis. The entered sensor offsets
 *      (roll bias, pitch bias, heading bias). Extra heave at the transducer
 *      due to roll and pitch when the sensor does not give its data at the
 *      transducer position is also included and heave is positive downwards.
 *      The sensor status is copied from the input datagram's two sync bytes 
 *      with the second byte always set to 0x90. The first byte is either
 *      zero or in the 0x90-0xAF range. If the latter is true, then 0x90
 *      indicates valid data with full accuracy, 0x91-0x99 indicates valid
 *      data with gradually reduced accuracy, 0x9A-0x9F indicates invalid data
 *      from an operating sensor,  and 0xA0-0xAF indicates invalid data 
 *      from a faulty sensor. This interpretation may be dependent on the
 *      attitude sensor type.
 *  12. The heading data is output asynchronously with respect to the
 *      ping output datagrams. Typical heading sensors give data at a 10 Hz
 *      rate. The attitude datagrams are given when the number of measurements
 *      is 100, or usually at 10 second intervals. The heading data is
 *      corrected for the heading offset entered by the operator.
 *  13. The raw vendor format (format 58) does not support flagging of bathymetry
 *      values nor does it store navigation in the bathymetry data records.
 *      MB-System also supports a processing format (59) which includes
 *      beamflags and navigation in the bathymetry records. This format
 *      is identical to the vendor format except for the use of a
 *      slightly different bathymetry record.
 *
 */
 
/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* sonar models */
#define	MBSYS_SIMRAD3_UNKNOWN	0

#define	MBSYS_SIMRAD3_EM2040	 2040
#define	MBSYS_SIMRAD3_EM710	 710
#define	MBSYS_SIMRAD3_EM302	 302
#define	MBSYS_SIMRAD3_EM122	 122

#define	MBSYS_SIMRAD3_EM120	120
#define	MBSYS_SIMRAD3_EM300	300
#define	MBSYS_SIMRAD3_EM1002	1002
#define	MBSYS_SIMRAD3_EM2000	2000
#define	MBSYS_SIMRAD3_EM3000	3000
#define	MBSYS_SIMRAD3_EM3000D_1	3001
#define	MBSYS_SIMRAD3_EM3000D_2	3002
#define	MBSYS_SIMRAD3_EM3000D_3	3003
#define	MBSYS_SIMRAD3_EM3000D_4	3004
#define	MBSYS_SIMRAD3_EM3000D_5	3005
#define	MBSYS_SIMRAD3_EM3000D_6	3006
#define	MBSYS_SIMRAD3_EM3000D_7	3007
#define	MBSYS_SIMRAD3_EM3000D_8	3008
#define	MBSYS_SIMRAD3_EM3002	3020

#define	MBSYS_SIMRAD3_EM12S	9901
#define	MBSYS_SIMRAD3_EM12D	9902
#define	MBSYS_SIMRAD3_EM121	9903
#define	MBSYS_SIMRAD3_EM100	9904
#define	MBSYS_SIMRAD3_EM1000	9905

/* maximum number of beams and pixels */
#define	MBSYS_SIMRAD3_MAXBEAMS		512
#define	MBSYS_SIMRAD3_MAXPIXELS		1024
#define	MBSYS_SIMRAD3_MAXRAWPIXELS	65535
#define MBSYS_SIMRAD3_MAXTX		19
#define	MBSYS_SIMRAD3_MAXSVP		1024
#define	MBSYS_SIMRAD3_MAXATTITUDE	256
#define	MBSYS_SIMRAD3_MAXHEADING	256
#define	MBSYS_SIMRAD3_MAXSSV		256
#define	MBSYS_SIMRAD3_MAXTILT		256
#define	MBSYS_SIMRAD3_COMMENT_LENGTH	256
#define	MBSYS_SIMRAD3_BUFFER_SIZE	2048

/* datagram start and end byte */
#define	EM3_START_BYTE		0x02
#define	EM3_END_BYTE		0x03
#define	EM3_END			0x03

/* datagram types including start byte */
#define	EM3_NONE		0
#define	EM3_STOP2		0x0230
#define	EM3_STATUS		0x0231
#define	EM3_OFF			0x0231
#define	EM3_ON			0x0232
#define	EM3_ATTITUDE		0x0241
#define	EM3_CLOCK		0x0243
#define	EM3_BATH		0x0244
#define	EM3_SBDEPTH		0x0245
#define	EM3_RAWBEAM		0x0246
#define	EM3_SSV			0x0247
#define	EM3_HEADING		0x0248
#define	EM3_START		0x0249
#define	EM3_TILT		0x024A
#define	EM3_CBECHO		0x024B
#define	EM3_RAWBEAM4		0x024E
#define	EM3_POS			0x0250
#define	EM3_RUN_PARAMETER	0x0252
#define	EM3_SS			0x0253
#define	EM3_TIDE		0x0254
#define	EM3_SVP2		0x0255
#define	EM3_SVP			0x0256
#define	EM3_SSPINPUT		0x0257
#define	EM3_BATH2		0x0258
#define	EM3_SS2			0x0259
#define	EM3_RAWBEAM2		0x0265
#define	EM3_RAWBEAM3		0x0266
#define	EM3_HEIGHT		0x0268
#define	EM3_STOP		0x0269
#define	EM3_WATERCOLUMN		0x026B
#define	EM3_NETATTITUDE		0x026E
#define	EM3_REMOTE		0x0270
#define	EM3_SSP			0x0273
#define	EM3_BATH_MBA		0x02E1
#define	EM3_SS_MBA		0x02E2
#define	EM3_BATH2_MBA		0x02E3
#define	EM3_SS2_MBA		0x02E4
#define	EM3_BATH3_MBA		0x02E5

/* datagram types */
#define	EM3_ID_STOP2		0x30
#define	EM3_ID_STATUS		0x31
#define	EM3_ID_OFF		0x31
#define	EM3_ID_ON		0x32
#define	EM3_ID_ATTITUDE		0x41
#define	EM3_ID_CLOCK		0x43
#define	EM3_ID_BATH		0x44
#define	EM3_ID_SBDEPTH		0x45
#define	EM3_ID_RAWBEAM		0x46
#define	EM3_ID_SSV		0x47
#define	EM3_ID_HEADING		0x48
#define	EM3_ID_START		0x49
#define	EM3_ID_TILT		0x4A
#define	EM3_ID_CBECHO		0x4B
#define	EM3_ID_RAWBEAM4		0x4E
#define	EM3_ID_POS		0x50
#define	EM3_ID_RUN_PARAMETER	0x52
#define	EM3_ID_SS		0x53
#define	EM3_ID_TIDE		0x54
#define	EM3_ID_SVP2		0x55
#define	EM3_ID_SVP		0x56
#define	EM3_ID_SSPINPUT		0x57
#define	EM3_ID_BATH2		0x58
#define	EM3_ID_SS2		0x59
#define	EM3_ID_RAWBEAM2		0x65
#define	EM3_ID_RAWBEAM3		0x66
#define	EM3_ID_HEIGHT		0x68
#define	EM3_ID_STOP		0x69
#define	EM3_ID_WATERCOLUMN	0x6B
#define	EM3_ID_NETATTITUDE	0x6E
#define	EM3_ID_REMOTE		0x70
#define	EM3_ID_SSP		0x73
#define	EM3_ID_BATH_MBA		0xE1
#define	EM3_ID_SS_MBA		0xE2
#define	EM3_ID_BATH2_MBA	0xE3
#define	EM3_ID_SS2_MBA		0xE4
#define	EM3_ID_BATH3_MBA	0xE5

/* datagram sizes where constant */
#define	EM3_STATUS_SIZE			88
#define	EM3_RUN_PARAMETER_SIZE		52
#define	EM3_CLOCK_SIZE			28
#define	EM3_TIDE_SIZE			30
#define	EM3_HEIGHT_SIZE			24
#define	EM3_START_HEADER_SIZE		14
#define	EM3_HEADING_HEADER_SIZE		14
#define	EM3_HEADING_SLICE_SIZE		4
#define	EM3_SSV_HEADER_SIZE		14
#define	EM3_SSV_SLICE_SIZE		4
#define	EM3_TILT_HEADER_SIZE		14
#define	EM3_TILT_SLICE_SIZE		4
#define	EM3_ATTITUDE_HEADER_SIZE	14
#define	EM3_ATTITUDE_SLICE_SIZE		12
#define	EM3_POS_HEADER_SIZE		30
#define	EM3_SVP_HEADER_SIZE		24
#define	EM3_SVP_SLICE_SIZE		4
#define	EM3_SVP2_HEADER_SIZE		24
#define	EM3_SVP2_SLICE_SIZE		8
#define	EM3_BATH_HEADER_SIZE		24
#define	EM3_BATH_BEAM_SIZE		16
#define	EM3_BATH_MBA_HEADER_SIZE	34
#define	EM3_BATH_MBA_BEAM_SIZE		18
#define	EM3_BATH2_HEADER_SIZE		32
#define	EM3_BATH2_BEAM_SIZE		20
#define	EM3_BATH2_MBA_HEADER_SIZE	48
#define	EM3_BATH2_MBA_BEAM_SIZE		34
#define	EM3_RAWBEAM4_HEADER_SIZE	28
#define	EM3_RAWBEAM4_TX_SIZE		24
#define	EM3_RAWBEAM4_BEAM_SIZE		16
#define	EM3_BATH3_MBA_HEADER_SIZE	48
#define	EM3_BATH3_MBA_BEAM_SIZE		38
#define	EM3_SS2_HEADER_SIZE		28
#define	EM3_SS2_BEAM_SIZE		6
#define	EM3_SS2_MBA_HEADER_SIZE		36
#define	EM3_SS2_MBA_BEAM_SIZE		6
#define	EM3_NETATTITUDE_HEADER_SIZE	16
#define	EM3_NETATTITUDE_SLICE_SIZE	11
#define	EM3_WC_HEADER_SIZE		36
#define	EM3_WC_TX_SIZE			6
#define	EM3_WC_BEAM_SIZE		10

/* invalid value flags */
#define	EM3_INVALID_AMP			0x7F
#define	EM3_INVALID_CHAR		0xFF
#define	EM3_INVALID_SHORT		0xFFFF
#define EM3_INVALID_U_INT		0xFFFFFFFF
#define EM3_INVALID_INT			0x7FFFFFFF
     
/* internal data structure for survey data */
struct mbsys_simrad3_ping_struct
	{
	int	png_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_count;	/* sequential counter or input identifier */
	int	png_serial;	/* system 1 or system 2 serial number */
	double	png_latitude;	/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	double	png_longitude;	/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	int	png_heading;	/* heading (0.01 deg) */
	int	png_heave;	/* heave from interpolation (0.01 m) */
	int	png_roll;	/* roll from interpolation (0.01 deg) */
	int	png_pitch;	/* pitch from interpolation (0.01 deg) */
	int	png_speed;	/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
	int	png_ssv;	/* sound speed at transducer (0.1 m/sec) */
	float	png_xducer_depth;   
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

	int	png_nbeams;	/* maximum number of beams possible */
	int	png_nbeams_valid;	/* number of valid beams */
	float	png_sample_rate; /* sampling rate (Hz) */
	int	png_spare; /* sampling rate (Hz) */
	float	png_depth[MBSYS_SIMRAD3_MAXBEAMS];	
				/* depths relative to sonar (m)
					The beam data are given re the transmit 
					transducer or sonar head depth and the 
					horizontal location (x,y) of the active 
					positioning systemÕs reference point. 
					Heave, roll, pitch, sound speed at the 
					transducer depth and ray bending through 
					the water column have been applied. */
	float	png_acrosstrack[MBSYS_SIMRAD3_MAXBEAMS];
				/* acrosstrack distances (m) */
	float	png_alongtrack[MBSYS_SIMRAD3_MAXBEAMS];
				/* alongtrack distances (m) */
	int	png_window[MBSYS_SIMRAD3_MAXBEAMS];		
				/* samples */
	int	png_quality[MBSYS_SIMRAD3_MAXBEAMS];	
				/* 0-254 Scaled standard deviation (sd) of the 
					range detection divided by 
					the detected range (dr): 
					Quality factor = 250*sd/dr. */
	int	png_iba[MBSYS_SIMRAD3_MAXBEAMS];	
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
	int	png_detection[MBSYS_SIMRAD3_MAXBEAMS];	
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
	int	png_clean[MBSYS_SIMRAD3_MAXBEAMS];	
				/* realtime cleaning info:
					For future use. A real time data cleaning module may flag out 
					beams. Bit 7 will be set to 1 if the beam is flagged out. Bit 0-6 
					will contain a code telling why the beam is flagged out. */
	int	png_amp[MBSYS_SIMRAD3_MAXBEAMS];		
				/* 0.5 dB */
	char	png_beamflag[MBSYS_SIMRAD3_MAXBEAMS];	
				/* uses standard MB-System beamflags */
	float	png_depression[MBSYS_SIMRAD3_MAXBEAMS];
				/* beam depression angles (deg) */
	float	png_azimuth[MBSYS_SIMRAD3_MAXBEAMS];
				/* beam azimuth angles (deg) */
	float	png_range[MBSYS_SIMRAD3_MAXBEAMS];
				/* Two-way travel times (sec). */
	float	png_bheave[MBSYS_SIMRAD3_MAXBEAMS];
				/* Heave correction to the sonar depth for each beam - this is half the difference
				   between the sonar depth at ping time and the sonar depth at receive time,
				   as measured by heave  */

	/* raw travel time and angle data version 4 */
	int	png_raw4_read;	/* flag indicating actual reading of raw beam record */
	int	png_raw_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_raw_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_raw_count;	/* sequential counter or input identifier */
	int	png_raw_serial;	/* system 1 or system 2 serial number */
	int	png_raw_ssv;		/* sound speed at transducer (0.1 m/sec) */
	int	png_raw_ntx;		/* number of TX pulses (1 to 9) */
	int	png_raw_nbeams;		/* number of raw travel times and angles
					    - nonzero only if raw beam record read */
	int	png_raw_detections;	/* number of valid detections */
	float	png_raw_sample_rate;	/* sampling rate (Hz) */
	int	png_raw_spare;
	int	png_raw_txtiltangle[MBSYS_SIMRAD3_MAXTX];/* tilt angle (0.01 deg) */
	int	png_raw_txfocus[MBSYS_SIMRAD3_MAXTX];   /* focus range (0.1 m)
								0 = no focus */
	float	png_raw_txsignallength[MBSYS_SIMRAD3_MAXTX];	/* signal length (sec) */
	float	png_raw_txoffset[MBSYS_SIMRAD3_MAXTX];	/* transmit time offset (sec) */
	float	png_raw_txcenter[MBSYS_SIMRAD3_MAXTX];	/* center frequency (Hz) */
	int	png_raw_txabsorption[MBSYS_SIMRAD3_MAXTX];	/* mean absorption coeff. (0.01 dB/km) */

	int	png_raw_txwaveform[MBSYS_SIMRAD3_MAXTX];	/* signal waveform identifier 
									0 = CW, 1 = FM upsweep, 2 = FM downsweep */
	int	png_raw_txsector[MBSYS_SIMRAD3_MAXTX];	/* transmit sector number (0-19) */
	float	png_raw_txbandwidth[MBSYS_SIMRAD3_MAXTX];	/* bandwidth (Hz) */

	int	png_raw_rxpointangle[MBSYS_SIMRAD3_MAXBEAMS];
				/* Raw beam pointing angles in 0.01 degree,
					positive to port. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
	int	png_raw_rxsector[MBSYS_SIMRAD3_MAXBEAMS];	/* transmit sector number (0-19) */
	int	png_raw_rxdetection[MBSYS_SIMRAD3_MAXBEAMS]; /* Detection info:
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
	int	png_raw_rxwindow[MBSYS_SIMRAD3_MAXBEAMS];	/* length of detection window */
	int	png_raw_rxquality[MBSYS_SIMRAD3_MAXBEAMS];	/* beam quality flag 
							   0-254 Scaled standard deviation (sd) of the 
								range detection divided by 
								the detected range (dr): 
								Quality factor = 250*sd/dr. */
	int	png_raw_rxspare1[MBSYS_SIMRAD3_MAXBEAMS];	/* spare */
	float	png_raw_rxrange[MBSYS_SIMRAD3_MAXBEAMS];	/* range as two-way travel time (s) */
	int	png_raw_rxamp[MBSYS_SIMRAD3_MAXBEAMS];		/* 0.5 dB */
	int	png_raw_rxcleaning[MBSYS_SIMRAD3_MAXBEAMS];	/* Real time cleaning info */
				/* realtime cleaning info:
					For future use. A real time data cleaning module may flag out 
					beams. Bit 7 will be set to 1 if the beam is flagged out. Bit 0-6 
					will contain a code telling why the beam is flagged out. */
	int	png_raw_rxspare2[MBSYS_SIMRAD3_MAXBEAMS];	/* spare */
				

	/* sidescan */
	int	png_ss2_read;	/* flag indicating actual reading of sidescan record */
	int	png_ss_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_ss_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_ss_count;	/* sequential counter or input identifier */
	int	png_ss_serial;	/* system 1 or system 2 serial number */
	float	png_ss_sample_rate;	/* sampling rate (Hz) */
	int	png_r_zero;	/* range to normal incidence used in TVG
				    (R0 predicted) in samples */
	int	png_bsn;	/* normal incidence backscatter (BSN) (0.1 dB) */
	int	png_bso;	/* oblique incidence backscatter (BSO) (0.1 dB) */
	int	png_tx;		/* Tx beamwidth (0.1 deg) */
	int	png_tvg_crossover;	
				/* TVG law crossover angle (0.1 deg) */
	int	png_nbeams_ss;	/* number of beams with sidescan */
	int	png_npixels;	/* number of pixels of sidescan */
	int	png_sort_direction[MBSYS_SIMRAD3_MAXBEAMS];	
				/* sorting direction - The first sample in a beam 
					has lowest range if 1, highest if -- 1. Note 
					that the ranges in the seabed image datagram 
					are all two-- way from time of transmit to 
					time of receive. */
	int	png_ssdetection[MBSYS_SIMRAD3_MAXBEAMS]; /* Detection info:
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
	int	png_beam_samples[MBSYS_SIMRAD3_MAXBEAMS];	
				/* number of sidescan samples derived from
					each beam */
	int	png_start_sample[MBSYS_SIMRAD3_MAXBEAMS];	
				/* start sample number */
	int	png_center_sample[MBSYS_SIMRAD3_MAXBEAMS]; 	/* center sample number */
	short	png_ssraw[MBSYS_SIMRAD3_MAXRAWPIXELS];		/* the raw sidescan ordered port to starboard */
	float	png_pixel_size;					/* processed sidescan pixel size (m) */
	int	png_pixels_ss;					/* number of processed sidescan pixels stored */
	short	png_ss[MBSYS_SIMRAD3_MAXPIXELS]; 		/* the processed sidescan ordered port to starboard */
	short	png_ssalongtrack[MBSYS_SIMRAD3_MAXPIXELS]; 	/* the processed sidescan alongtrack distances (0.01 m) */
	};

/* internal data structure for water column time series */
struct mbsys_simrad3_wcbeam_struct
	{
	int	wtc_rxpointangle;	/* Beam pointing angles in 0.01 degree,
						positive to port. These values are roll stabilized. */
	int	wtc_start_sample;	/* start sample number */
	int	wtc_beam_samples;	/* number of water column samples derived from
						each beam */
	int	wtc_beam_spare;		/* unknown */
	int	wtc_sector;		/* transmit sector identifier */
	int	wtc_beam;  		/* beam 128 is first beam on 
				  	  	second head of EM3000D */
	mb_s_char wtc_amp[MBSYS_SIMRAD3_MAXRAWPIXELS]; /* water column amplitude (dB) */
	};
	
/* internal data structure for water column data */
struct mbsys_simrad3_watercolumn_struct
	{
	int	wtc_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	wtc_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	wtc_count;	/* sequential counter or input identifier */
	int	wtc_serial;	/* system 1 or system 2 serial number */
	int	wtc_ndatagrams;	/* number of datagrams used to represent 
						the water column for this ping */
	int	wtc_datagram;	/* number this datagram */
	int	wtc_ntx;	/* number of transmit sectors */
	int	wtc_nrx;	/* number of receive beams */
	int	wtc_nbeam;	/* number of beams in this datagram */
	int	wtc_ssv;	/* sound speed at transducer (0.1 m/sec) */
	int	wtc_sfreq;	/* sampling frequency (0.01 Hz) */
	int	wtc_heave;	/* tx time heave at transducer (0.01 m) */
	int	wtc_spare1;	/* spare */
	int	wtc_spare2;	/* spare */
	int	wtc_spare3;	/* spare */
	int	wtc_txtiltangle[MBSYS_SIMRAD3_MAXTX];	/* tilt angle (0.01 deg) */
	int	wtc_txcenter[MBSYS_SIMRAD3_MAXTX];	/* center frequency (Hz) */
	int	wtc_txsector[MBSYS_SIMRAD3_MAXTX];	/* transmit sector number (0-19) */
	struct mbsys_simrad3_wcbeam_struct beam[MBSYS_SIMRAD3_MAXBEAMS];
	};

/* internal data structure for attitude data */
struct mbsys_simrad3_attitude_struct
	{
	int	att_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	att_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	att_count;	/* sequential counter or input identifier */
	int	att_serial;	/* system 1 or system 2 serial number */
	int	att_ndata;	/* number of attitude data */
	int	att_time[MBSYS_SIMRAD3_MAXATTITUDE];
				/* time since record start (msec) */
	int	att_sensor_status[MBSYS_SIMRAD3_MAXATTITUDE];
				/* see note 12 above */
	int	att_roll[MBSYS_SIMRAD3_MAXATTITUDE];
				/* roll (0.01 degree) */
	int	att_pitch[MBSYS_SIMRAD3_MAXATTITUDE];
				/* pitch (0.01 degree) */
	int	att_heave[MBSYS_SIMRAD3_MAXATTITUDE];
				/* heave (cm) */
	int	att_heading[MBSYS_SIMRAD3_MAXATTITUDE];
				/* heading (0.01 degree) */
	int	att_sensordescriptor;
				/* sensor system descriptor - indicates
				   which motion sensor is source of these
				   data, and which values have been
				   used in realtime processing:
				   	xx00 xxxx - motion sensor number 1
				   	xx01 xxxx - motion sensor number 2
				   	xx10 xxxx - motion sensor number 3 (in netattitude record)
				   	xxxx xxx1 - heading from this system is active
				   	xxxx xx0x - roll from this system is active
				   	xxxx x0xx - pitch from this system is active
				   	xxxx 0xxx - heading from this system is active */
	};

/* internal data structure for network attitude data */
struct mbsys_simrad3_netattitude_struct
	{
	int	nat_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	nat_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	nat_count;	/* sequential counter or input identifier */
	int	nat_serial;	/* system 1 or system 2 serial number */
	int	nat_ndata;	/* number of attitude data */
	int	nat_sensordescriptor;	/* sensor system descriptor */
	int	nat_time[MBSYS_SIMRAD3_MAXATTITUDE];
				/* time since record start (msec) */
	int	nat_roll[MBSYS_SIMRAD3_MAXATTITUDE];
				/* roll (0.01 degree) */
	int	nat_pitch[MBSYS_SIMRAD3_MAXATTITUDE];
				/* pitch (0.01 degree) */
	int	nat_heave[MBSYS_SIMRAD3_MAXATTITUDE];
				/* heave (cm) */
	int	nat_heading[MBSYS_SIMRAD3_MAXATTITUDE];
				/* heading (0.01 degree) */
	int	nat_nbyte_raw[MBSYS_SIMRAD3_MAXATTITUDE];	/* number of bytes in input datagram (Nd) */
	char	nat_raw[MBSYS_SIMRAD3_MAXATTITUDE*MBSYS_SIMRAD3_BUFFER_SIZE];	/* network attitude input datagram as received by datalogger */
	};

/* internal data structure for heading data */
struct mbsys_simrad3_heading_struct
	{
	int	hed_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	hed_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	hed_count;	/* sequential counter or input identifier */
	int	hed_serial;	/* system 1 or system 2 serial number */
	int	hed_ndata;	/* number of heading data */
	int	hed_time[MBSYS_SIMRAD3_MAXHEADING];
				/* time since record start (msec) */
	int	hed_heading[MBSYS_SIMRAD3_MAXHEADING];
				/* heading (0.01 degree) */
	int	hed_heading_status;
				/* heading status (0=inactive) */
	};

/* internal data structure for ssv data */
struct mbsys_simrad3_ssv_struct
	{
	int	ssv_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	ssv_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	ssv_count;	/* sequential counter or input identifier */
	int	ssv_serial;	/* system 1 or system 2 serial number */
	int	ssv_ndata;	/* number of ssv data */
	int	ssv_time[MBSYS_SIMRAD3_MAXSSV];
				/* time since record start (msec) */
	int	ssv_ssv[MBSYS_SIMRAD3_MAXSSV];
				/* ssv (0.1 m/s) */
	};

/* internal data structure for tilt data */
struct mbsys_simrad3_tilt_struct
	{
	int	tlt_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	tlt_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	tlt_count;	/* sequential counter or input identifier */
	int	tlt_serial;	/* system 1 or system 2 serial number */
	int	tlt_ndata;	/* number of tilt data */
	int	tlt_time[MBSYS_SIMRAD3_MAXTILT];
				/* time since record start (msec) */
	int	tlt_tilt[MBSYS_SIMRAD3_MAXTILT];
				/* tilt + forward (0.01 degree) */
	};
	
/* internal data structure */
struct mbsys_simrad3_struct
	{
	/* type of data record */
	int	kind;		/* MB-System record ID */
	int	type;		/* Simrad datagram ID */

	/* type of sonar */
	int	sonar;		/* Type of Simrad sonar */
	
	/* time stamp */
	int	date;		/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	msec;		/* time since midnight in msec
				    08:12:51.234 = 29570234 */

	/* status parameter values */
	int	sts_date;	/* status date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	sts_msec;	/* status time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	sts_status_count; 	/* status datagram counter */
	int	sts_serial;		/* system 1 or 2 serial number */
	int	sts_pingrate;		/* ping rate (0.01 Hz) */
	int	sts_ping_count;		/* ping counter - latest ping */
	int	sts_load;		/* processing unit load (%) */
	int	sts_udp_status;		/* sensor input status, UDP port 2 */
	int	sts_serial1_status;	/* sensor input status, serial port 1 */
	int	sts_serial2_status;	/* sensor input status, serial port 2 */
	int	sts_serial3_status;	/* sensor input status, serial port 3 */
	int	sts_serial4_status;	/* sensor input status, serial port 4 */
	int	sts_pps_status;		/* sensor input status, pps, >0 ok */
	int	sts_position_status;	/* sensor input status, position, >0 ok */
	int	sts_attitude_status;	/* sensor input status, attitude, >0 ok */
	int	sts_clock_status;	/* sensor input status, clock, >0 ok */
	int	sts_heading_status;	/* sensor input status, heading, >0 ok */
	int	sts_pu_status;		/* sensor input status, processing unit 
						(0=off, 1-on, 2=simulator) */
	int	sts_last_heading;	/* last received heading (0.01 deg) */
	int	sts_last_roll;		/* last received roll (0.01 deg) */
	int	sts_last_pitch;		/* last received pitch (0.01 deg) */
	int	sts_last_heave;		/* last received heave (0.01 m) */
	int	sts_last_ssv;		/* last received sound speed (0.1 m/s) */
	int	sts_last_depth;		/* last received depth (0.01 m) */
	int	sts_spare;		/* spare */
	int	sts_bso;		/* backscatter at oblique angle (dB) */
	int	sts_bsn;		/* backscatter at normal incidence (dB) */
	int	sts_gain;		/* fixed gain (dB) */
	int	sts_dno;		/* depth to normal incidence (m) */
	int	sts_rno;		/* range to normal incidence (m) */
	int	sts_port;		/* port coverage (deg) */
	int	sts_stbd;		/* starboard coverage (deg) */
	int	sts_ssp;		/* sound speed at transducer from profile (0.1 m/s) */
	int	sts_yaw;		/* yaw stabilization (0.01 deg) */
	int	sts_port2;		/* port coverage for second em3002 head (deg) */
	int	sts_stbd2;		/* starboard coverage for second em3002 head (deg) */
	int	sts_spare2;		/* spare */

	/* installation parameter values */
	int	par_date;	/* installation parameter date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	par_msec;	/* installation parameter time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	par_line_num;	/* survey line number */
	int	par_serial_1;	/* system 1 serial number */
	int	par_serial_2;	/* system 2 serial number */
	double	par_wlz;	/* water line vertical location (m) */
	int	par_smh;	/* system main head serial number */
	double	par_s1z;	/* transducer 1 vertical location (m) */
	double	par_s1x;	/* transducer 1 along location (m) */
	double	par_s1y;	/* transducer 1 athwart location (m) */
	double	par_s1h;	/* transducer 1 heading (deg) */
	double	par_s1r;	/* transducer 1 roll (m) */
	double	par_s1p;	/* transducer 1 pitch (m) */
	int	par_s1n;	/* transducer 1 number of modules */
	double	par_s2z;	/* transducer 2 vertical location (m) */
	double	par_s2x;	/* transducer 2 along location (m) */
	double	par_s2y;	/* transducer 2 athwart location (m) */
	double	par_s2h;	/* transducer 2 heading (deg) */
	double	par_s2r;	/* transducer 2 roll (m) */
	double	par_s2p;	/* transducer 2 pitch (m) */
	int	par_s2n;	/* transducer 2 number of modules */
	double	par_go1;	/* system (sonar head 1) gain offset */
	double	par_go2;	/* sonar head 2 gain offset */
	char	par_tsv[16];	/* transmitter (sonar head 1) software version */
	char	par_rsv[16];	/* receiver (sonar head 2) software version */
	char	par_bsv[16];	/* beamformer software version */
	char	par_psv[16];	/* processing unit software version */
	char	par_osv[16];	/* operator station software version */
	double	par_dsd;	/* depth sensor time delay (msec) */
	double	par_dso;	/* depth sensor offset */
	double	par_dsf;	/* depth sensor scale factor */
	char	par_dsh[2];	/* depth sensor heave (IN or NI) */
	int	par_aps;	/* active position system number */
	int	par_p1m;	/* position system 1 motion compensation (boolean) */
	int	par_p1t;	/* position system 1 time stamp used 
				    (0=system time, 1=position input time) */
	double	par_p1z;	/* position system 1 vertical location (m) */
	double	par_p1x;	/* position system 1 along location (m) */
	double	par_p1y;	/* position system 1 athwart location (m) */
	double	par_p1d;	/* position system 1 time delay (sec) */
	char	par_p1g[16];	/* position system 1 geodetic datum */
	int	par_p2m;	/* position system 2 motion compensation (boolean) */
	int	par_p2t;	/* position system 2 time stamp used 
				    (0=system time, 1=position input time) */
	double	par_p2z;	/* position system 2 vertical location (m) */
	double	par_p2x;	/* position system 2 along location (m) */
	double	par_p2y;	/* position system 2 athwart location (m) */
	double	par_p2d;	/* position system 2 time delay (sec) */
	char	par_p2g[16];	/* position system 2 geodetic datum */
	int	par_p3m;	/* position system 3 motion compensation (boolean) */
	int	par_p3t;	/* position system 3 time stamp used 
				    (0=system time, 1=position input time) */
	double	par_p3z;	/* position system 3 vertical location (m) */
	double	par_p3x;	/* position system 3 along location (m) */
	double	par_p3y;	/* position system 3 athwart location (m) */
	double	par_p3d;	/* position system 3 time delay (sec) */
	char	par_p3g[16];	/* position system 3 geodetic datum */
	double	par_msz;	/* motion sensor vertical location (m) */
	double	par_msx;	/* motion sensor along location (m) */
	double	par_msy;	/* motion sensor athwart location (m) */
	char	par_mrp[2];	/* motion sensor roll reference plane (HO or RP) */
	double	par_msd;	/* motion sensor time delay (sec) */
	double	par_msr;	/* motion sensor roll offset (deg) */
	double	par_msp;	/* motion sensor pitch offset (deg) */
	double	par_msg;	/* motion sensor heading offset (deg) */
	double	par_gcg;	/* gyro compass heading offset (deg) */
	char	par_cpr[4];	/* cartographic projection */
	char	par_rop[MBSYS_SIMRAD3_COMMENT_LENGTH];	
				/* responsible operator */
	char	par_sid[MBSYS_SIMRAD3_COMMENT_LENGTH];	
				/* survey identifier */
	char	par_pll[MBSYS_SIMRAD3_COMMENT_LENGTH];	
				/* survey line identifier (planned line number) */
	char	par_com[MBSYS_SIMRAD3_COMMENT_LENGTH];	
				/* comment */

	/* runtime parameter values */
	int	run_date;	/* runtime parameter date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	run_msec;	/* runtime parameter time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	run_ping_count;	/* ping counter */
	int	run_serial;	/* system 1 or 2 serial number */
	int	run_status;	/* system status */
	int	run_mode;	/* system mode:
				    0 : nearfield (EM3000) or very shallow (EM300)
				    1 :	normal (EM3000) or shallow (EM300)
				    2 : medium (EM300)
				    3 : deep (EM300)
				    4 : very deep (EM300) */
	int	run_filter_id;	/* filter identifier - the two lowest bits
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
	int	run_min_depth;	/* minimum depth (m) */
	int	run_max_depth;	/* maximum depth (m) */
	int	run_absorption;	/* absorption coefficient (0.01 dB/km) */

	int	run_tran_pulse;	/* transmit pulse length (usec) */
	int	run_tran_beam;	/* transmit beamwidth (0.1 deg) */
	int	run_tran_pow;	/* transmit power reduction (dB) */
	int	run_rec_beam;	/* receiver beamwidth (0.1 deg) */
	int	run_rec_band;	/* receiver bandwidth (50 hz) */
	int	run_rec_gain;	/* receiver fixed gain (dB) */
	int	run_tvg_cross;	/* TVG law crossover angle (deg) */
	int	run_ssv_source;	/* source of sound speed at transducer:
				    0 : from sensor
				    1 : manual
				    2 : from profile */
	int	run_max_swath;	/* maximum swath width (m) */
	int	run_beam_space;	/* beam spacing:
				    0 : determined by beamwidth (EM3000)
				    1 : equidistant
				    2 : equiangle */
	int	run_swath_angle;/* coverage sector of swath (deg) */
	int	run_stab_mode;	/* yaw and pitch stabilization mode:
				    The upper bit (bit 7) is set if pitch
				    stabilization is on.
				    The two lower bits are used to show yaw
				    stabilization mode as follows:
					00 : none
					01 : to survey line heading
					10 : to mean vessel heading
					11 : to manually entered heading */
	char	run_spare[6];

	/* sound velocity profile */
	int	svp_use_date;	/* date at start of use
				    date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	svp_use_msec;	/* time at start of use since midnight in msec
				    08:12:51.234 = 29570234 */
	int	svp_count;	/* sequential counter or input identifier */
	int	svp_serial;	/* system 1 serial number */
	int	svp_origin_date;/* date at svp origin
				    date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	svp_origin_msec;/* time at svp origin since midnight in msec
				    08:12:51.234 = 29570234 */
	int	svp_num;	/* number of svp entries */
	int	svp_depth_res;	/* depth resolution (cm) */
	int	svp_depth[MBSYS_SIMRAD3_MAXSVP]; 
				/* depth of svp entries (according to svp_depth_res) */
	int	svp_vel[MBSYS_SIMRAD3_MAXSVP];	
				/* sound speed of svp entries (0.1 m/sec) */

	/* position */
	int	pos_date;	/* position date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	pos_msec;	/* position time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	pos_count;	/* sequential counter */
	int	pos_serial;	/* system 1 serial number */
	int	pos_latitude;	/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	int	pos_longitude;	/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	int	pos_quality;	/* measure of position fix quality (cm) */
	int	pos_speed;	/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
	int	pos_course;	/* course over ground (0.01 deg) if valid,
				    invalid = 0xFFFF */
	int	pos_heading;	/* heading (0.01 deg) if valid,
				    invalid = 0xFFFF */
	int	pos_heave;	/* heave from interpolation (0.01 m) */
	int	pos_roll;	/* roll from interpolation (0.01 deg) */
	int	pos_pitch;	/* pitch from interpolation (0.01 deg) */
	int	pos_system;	/* position system number, type, and realtime use
				    - position system number given by two lowest bits
				    - fifth bit set means position must be derived
					from input Simrad 90 datagram
				    - sixth bit set means valid time is that of
					input datagram */
	int	pos_input_size;	/* number of bytes in input position datagram */
	char	pos_input[256];	/* position input datagram as received, minus
				    header and tail (such as NMEA 0183 $ and CRLF) */

	/* height */
	int	hgt_date;	/* height date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	hgt_msec;	/* height time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	hgt_count;	/* sequential counter */
	int	hgt_serial;	/* system 1 serial number */
	int	hgt_height;	/* height (0.01 m) */
	int	hgt_type;	/* height type as given in input datagram or if
				    zero the height is derived from the GGK datagram
				    and is the height of the water level re the
				    vertical datum */
	
	/* tide */
	int	tid_date;	/* tide date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	tid_msec;	/* tide time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	tid_count;	/* sequential counter */
	int	tid_serial;	/* system 1 serial number */
	int	tid_origin_date;/* tide input date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	tid_origin_msec;/* tide input time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	tid_tide;	/* tide offset (0.01 m) */	
	
	/* clock */
	int	clk_date;	/* system date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	clk_msec;	/* system time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	clk_count;	/* sequential counter */
	int	clk_serial;	/* system 1 serial number */
	int	clk_origin_date;/* external clock date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	clk_origin_msec;/* external clock time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	clk_1_pps_use;	/* if 1 then the internal clock is synchronized
				    to an external 1 PPS signal, if 0 then not */

	/* pointer to attitude data structure */
	struct mbsys_simrad3_attitude_struct *attitude;

	/* pointer to network attitude data structure */
	struct mbsys_simrad3_netattitude_struct *netattitude;

	/* pointer to heading data structure */
	struct mbsys_simrad3_heading_struct *heading;

	/* pointer to ssv data structure */
	struct mbsys_simrad3_ssv_struct *ssv;

	/* pointer to tilt data structure */
	struct mbsys_simrad3_tilt_struct *tilt;

	/* pointer to survey data structure */
	struct mbsys_simrad3_ping_struct *ping;

	/* pointer to water column data structure */
	struct mbsys_simrad3_watercolumn_struct *wc;
	};
	
	
/* system specific function prototypes */
int mbsys_simrad3_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_simrad3_survey_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_wc_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_attitude_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_netattitude_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_heading_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_ssv_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_tilt_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad3_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_simrad3_zero_ss(int verbose, void *store_ptr, int *error);
int mbsys_simrad3_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_simrad3_pingnumber(int verbose, void *mbio_ptr, 
			int *pingnumber, int *error);
int mbsys_simrad3_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_simrad3_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_simrad3_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_simrad3_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_simrad3_pulses(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *pulses, int *error);
int mbsys_simrad3_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length, 
			double *receive_gain, int *error);
int mbsys_simrad3_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_simrad3_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_simrad3_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_simrad3_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_simrad3_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_simrad3_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_simrad3_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_simrad3_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size, 
		int swath_width_set, double *swath_width, 
		int pixel_int, 
		int *error);

