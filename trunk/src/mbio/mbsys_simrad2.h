/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad2.h	10/9/98
 *	$Id: mbsys_simrad2.h,v 5.17 2006-02-02 19:42:09 caress Exp $
 *
 *    Copyright (c) 1998, 2001, 2002, 2003 by
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
 * mbsys_simrad2.h defines the MBIO data structures for handling data from 
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
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	October 9, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.16  2006/01/27 20:09:47  caress
 * Added support for EM3002
 *
 * Revision 5.15  2006/01/06 18:27:18  caress
 * Working towards 5.0.8
 *
 * Revision 5.14  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.13  2005/04/07 04:24:34  caress
 * 5.0.7 Release.
 *
 * Revision 5.12  2003/11/24 19:04:21  caress
 * Added remaining unsupported Simrad datagrams to defines so that all are listed and can be recognized by the parsing code.
 *
 * Revision 5.11  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.10  2002/08/21 00:55:46  caress
 * Release 5.0.beta22
 *
 * Revision 5.9  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.8  2002/05/29 23:41:49  caress
 * Release 5.0.beta18
 *
 * Revision 5.7  2001/07/20 00:32:54  caress
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
 * Revision 4.3  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  2000/07/20  20:24:59  caress
 * First cut at supporting both EM120 and EM1002.
 *
 * Revision 4.1  2000/07/19  03:54:23  caress
 * Added support for EM120.
 *
 * Revision 4.0  1998/12/17  22:59:14  caress
 * MB-System version 4.6beta4
 *
 *
 */
/*
 * Notes on the MBSYS_SIMRAD2 data structure:
 *   1. Simrad multibeam systems output datagrams which are
 *      a combination of ascii and binary. This code has been written
 *      using data format specifications found in an April 28, 1998
 *      technical note from Simrad.
 *   2. Simrad multibeam sonars output both bathymetry
 *      and amplitude information for beams and sidescan information
 *      with a higher resolution than the bathymetry and amplitude.
 *   3. There are three systems of interest:
 *         EM-3000:  Single array 300 kHz shallow water system with up to 127
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam.
 *         EM-3000D: Double array 300 kHz shallow water system with up to 254
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam.
 *         EM-2000:  Single array 200 kHz shallow water system with up to 87
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam.
 *         EM-1002:  Single array 95 kHz shallow water system with up to 111
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam.
 *         EM-300:   Single array 30 kHz mid water system with up to 135
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam. This system applies
 *                   pitch and yaw compensation to achieve
 *                   more uniform coverage of the seafloor.
 *         EM-120:   Single array 12 kHz full ocean system with up to 191
 *                   beams of bathymetry and a variable number of sidescan
 *                   samples per bathymetry beam. This system applies
 *                   pitch and yaw compensation to achieve
 *                   more uniform coverage of the seafloor.
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
 *        *0x0231: Parameter - Data out off               variable size
 *        *0x0232: Parameter - Data out on                variable size
 *        *0x0230: Parameter - Stop                       variable size
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
 *        *0x0250: Position                               100-134 bytes
 *        *0x0252: Runtime Parameter                      52 bytes
 *        *0x0253: Sidescan                               48->5K bytes
 *        *0x0254: Tide Output                            30 bytes
 *        *0x0255: Sound velocity profile (new)           variable size
 *        *0x0256: Sound velocity profile (old)           variable size
 *         0x0257: SSP input                              variable size
 *        *0x0265: Raw range and beam angle               112-1658 bytes
 *        *0x0266: Raw range and beam angle               44-1658 bytes
 *        *0x0268: Height Output                          24 bytes
 *        *0x0269: Parameter - Stop                       variable size
 *         0x0270: Parameter - Remote                     variable size
 *         0x0273: Surface sound speed                    variable size
 *        *0x02E1: Bathymetry (MBARI format 57)           48-4092 bytes
 *        *0x02E2: Sidescan (MBARI format 57)             48->5K bytes
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
 *      bending have been applied. In the EM3000 the beam depths must 
 *      be regarded as signed values to take into account beams 
 *      which may be going upwards. On the EM300 the beam depths are
 *      always positive and the values are therefore unsigned.
 *      To obtain depths relative to the water line, the
 *      raw depths must be added to the transmit transducer depth plus
 *      the depth offset multiplier times 65536 cm. The depth offset
 *      multiplier will be zero except when 
 *        1) the EM3000 sonar head is on an underwater vehicle 
 *           at a depth greater than 650 m, or
 *        2) when the heave is large enough to bring the transmit
 *           transducer above the water line (the depth offset 
 *           multiplier is -1 in this case).
 *   8. The new Simrad sonars record the heading and attitude sensor
 *      data streams input into the sonar, usually at a sampling 
 *      frequency of 100 Hz.
 *   9. Although this new format started out as a fairly clean conception, 
 *      later revisions adding new information have been implemented in
 *      an unnecessarily complicated fashion. For example, 
 *      if the sonar is an EM300 or EM3000, then the sample rate
 *      value contains the sample rate in Hz as an unsigned short.
 *      The range resolution in time is half the inverse of the
 *      sampling rate, or approximately 2.5 cm for an EM3000.
 *      HOWEVER, if the sonar is an EM3000D, then the sample rate value
 *      contains the depth difference between the two sonar heads
 *      (the units are not specified in the spec, but are presumably in
 *      cm like the transmit transducer depth). In this case, the transmit
 *      transducer depth for the second sonar head is the transmit
 *      transducer depth plus the depth difference. The sample rate for the
 *      EM3000D is derived from the sonar model,  as follows:
 *        Sonar Model     Head 1 Sample Rate     Head 2 Sample Rate 
 *        -----------     ------------------     ------------------
 *           3002              13956 Hz               14621 Hz
 *           3003              13956 Hz               14621 Hz
 *           3004              14293 Hz               14621 Hz
 *           3005              13956 Hz               14293 Hz
 *           3006              14621 Hz               14293 Hz
 *           3007              14293 Hz               13956 Hz
 *           3008              14621 Hz               13956 Hz
 *      In the case of sonar model 3002, the depth difference should be
 *      taken as zero, regardless of the value found in the data.
 *  10. An additional complication involves the beam angles and travel
 *      times reported in the data stream. In the original format 
 *      specification, the beam range is given as a one way travel time, 
 *      the beam depression angle is positive downwards (90 deg at vertical)
 *      and the beam azimuth angle gives the orientation of the sounding
 *      relative to the heading of the vessel. These values include the
 *      effects of the vessels motion during the ping cycle (heave, pitch, 
 *      roll, and yaw), and are sufficient for recalculating bathymetry 
 *      by raytracing (an improvement over older Simrad data formats).
 *      HOWEVER, the sonar operator can specify a data recording mode in
 *      which the raw times and angles are recorded without correction
 *      for the vessel motion. In this case, the raw two-way travel times
 *      are recorded instead of one-way travel times. Also, the beam
 *      depression angles are replaced by beam pointing angles, which are
 *      the angles relative to the sonar heads before correction for roll
 *      and roll bias. The beam azimuth angle is replaced by a combination
 *      of a flag signaling the raw data mode (angle values greater than 35999
 *      indicates storage of uncorrected range and angle data) and the 
 *      transmitter tilt angle (angle value minus 54000 in 0.01 deg). The
 *      purpose of the uncorrected data mode is to allow users to
 *      fully recalculate the data following revision of the heading and
 *      attitude data streams.
 *      NOTE: As of May 2002, Simrad seems to have implemented a different
 *      scheme for the EM300 and EM120 sonars. There is a new raw range and
 *      angle datagram (0x0265) that is output in addition to the depth and
 *      seafloor image datagrams. This would seem to make obsolete the notion
 *      of placing uncorrected ranges and angles in the depth datagram.
 *  11. The sidescan is structured in terms of a certain number of samples
 *      per beam. The range sampling rate for the sidescan is the same as
 *      that specified in the depth datagram and the ranges in the sidescan
 *      datagram (seabed image datagram) are all two way travel times.
 *  12. The attitude data is output asynchronously with respect to the
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
 *  13. The heading data is output asynchronously with respect to the
 *      ping output datagrams. Typical heading sensors give data at a 10 Hz
 *      rate. The attitude datagrams are given when the number of measurements
 *      is 100, or usually at 10 second intervals. The heading data is
 *      corrected for the heading offset entered by the operator.
 *  14. The raw vendor format (format 56) does not support flagging of bathymetry
 *      values nor does it store navigation in the bathymetry data records.
 *      MB-System also supports a processing format (57) which includes
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
#define	MBSYS_SIMRAD2_UNKNOWN	0
#define	MBSYS_SIMRAD2_EM120	120
#define	MBSYS_SIMRAD2_EM300	300
#define	MBSYS_SIMRAD2_EM1002	1002
#define	MBSYS_SIMRAD2_EM2000	2000
#define	MBSYS_SIMRAD2_EM3000	3000
#define	MBSYS_SIMRAD2_EM3000D_1	3001
#define	MBSYS_SIMRAD2_EM3000D_2	3002
#define	MBSYS_SIMRAD2_EM3000D_3	3003
#define	MBSYS_SIMRAD2_EM3000D_4	3004
#define	MBSYS_SIMRAD2_EM3000D_5	3005
#define	MBSYS_SIMRAD2_EM3000D_6	3006
#define	MBSYS_SIMRAD2_EM3000D_7	3007
#define	MBSYS_SIMRAD2_EM3000D_8	3008
#define	MBSYS_SIMRAD2_EM3002	3020

#define	MBSYS_SIMRAD2_EM12S	9901
#define	MBSYS_SIMRAD2_EM12D	9902
#define	MBSYS_SIMRAD2_EM121	9903
#define	MBSYS_SIMRAD2_EM100	9904
#define	MBSYS_SIMRAD2_EM1000	9905

/* maximum number of beams and pixels */
#define	MBSYS_SIMRAD2_MAXBEAMS		254
#define	MBSYS_SIMRAD2_MAXPIXELS		1024
#define	MBSYS_SIMRAD2_MAXRAWPIXELS	32000
#define MBSYS_SIMRAD2_MAXTX		19
#define	MBSYS_SIMRAD2_MAXSVP		1024
#define	MBSYS_SIMRAD2_MAXATTITUDE	256
#define	MBSYS_SIMRAD2_MAXHEADING	256
#define	MBSYS_SIMRAD2_MAXSSV		256
#define	MBSYS_SIMRAD2_MAXTILT		256
#define	MBSYS_SIMRAD2_COMMENT_LENGTH	256
#define	MBSYS_SIMRAD2_BUFFER_SIZE	2048

/* datagram start and end byte */
#define	EM2_START_BYTE		0x02
#define	EM2_END_BYTE		0x03
#define	EM2_END			0x03

/* datagram types including start byte */
#define	EM2_NONE		0
#define	EM2_STOP2		0x0230
#define	EM2_OFF			0x0231
#define	EM2_ON			0x0232
#define	EM2_ATTITUDE		0x0241
#define	EM2_CLOCK		0x0243
#define	EM2_BATH		0x0244
#define	EM2_SBDEPTH		0x0245
#define	EM2_RAWBEAM		0x0246
#define	EM2_SSV			0x0247
#define	EM2_HEADING		0x0248
#define	EM2_START		0x0249
#define	EM2_TILT		0x024A
#define	EM2_CBECHO		0x024B
#define	EM2_POS			0x0250
#define	EM2_RUN_PARAMETER	0x0252
#define	EM2_SS			0x0253
#define	EM2_TIDE		0x0254
#define	EM2_SVP2		0x0255
#define	EM2_SVP			0x0256
#define	EM2_SSPINPUT		0x0257
#define	EM2_RAWBEAM2		0x0265
#define	EM2_RAWBEAM3		0x0266
#define	EM2_HEIGHT		0x0268
#define	EM2_STOP		0x0269
#define	EM2_REMOTE		0x0270
#define	EM2_SSP			0x0273
#define	EM2_BATH_MBA		0x02E1
#define	EM2_SS_MBA		0x02E2

/* datagram types */
#define	EM2_ID_STOP2		0x30
#define	EM2_ID_OFF		0x31
#define	EM2_ID_ON		0x32
#define	EM2_ID_ATTITUDE		0x41
#define	EM2_ID_CLOCK		0x43
#define	EM2_ID_BATH		0x44
#define	EM2_ID_SBDEPTH		0x45
#define	EM2_ID_RAWBEAM		0x46
#define	EM2_ID_SSV		0x47
#define	EM2_ID_HEADING		0x48
#define	EM2_ID_START		0x49
#define	EM2_ID_TILT		0x4A
#define	EM2_ID_CBECHO		0x4B
#define	EM2_ID_POS		0x50
#define	EM2_ID_RUN_PARAMETER	0x52
#define	EM2_ID_SS		0x53
#define	EM2_ID_TIDE		0x54
#define	EM2_ID_SVP2		0x55
#define	EM2_ID_SVP		0x56
#define	EM2_ID_SSPINPUT		0x57
#define	EM2_ID_RAWBEAM2		0x65
#define	EM2_ID_RAWBEAM3		0x66
#define	EM2_ID_HEIGHT		0x68
#define	EM2_ID_STOP		0x69
#define	EM2_ID_REMOTE		0x70
#define	EM2_ID_SSP		0x73
#define	EM2_ID_BATH_MBA		0xE1
#define	EM2_ID_SS_MBA		0xE2

/* datagram sizes where constant */
#define	EM2_RUN_PARAMETER_SIZE		52
#define	EM2_CLOCK_SIZE			28
#define	EM2_TIDE_SIZE			30
#define	EM2_HEIGHT_SIZE			24
#define	EM2_START_HEADER_SIZE		14
#define	EM2_HEADING_HEADER_SIZE		14
#define	EM2_HEADING_SLICE_SIZE		4
#define	EM2_SSV_HEADER_SIZE		14
#define	EM2_SSV_SLICE_SIZE		4
#define	EM2_TILT_HEADER_SIZE		14
#define	EM2_TILT_SLICE_SIZE		4
#define	EM2_ATTITUDE_HEADER_SIZE	14
#define	EM2_ATTITUDE_SLICE_SIZE		12
#define	EM2_POS_HEADER_SIZE		30
#define	EM2_SVP_HEADER_SIZE		24
#define	EM2_SVP_SLICE_SIZE		4
#define	EM2_SVP2_HEADER_SIZE		24
#define	EM2_SVP2_SLICE_SIZE		8
#define	EM2_BATH_HEADER_SIZE		24
#define	EM2_BATH_BEAM_SIZE		16
#define	EM2_BATH_MBA_HEADER_SIZE	34
#define	EM2_BATH_MBA_BEAM_SIZE		18
#define	EM2_RAWBEAM_HEADER_SIZE		16
#define	EM2_RAWBEAM_BEAM_SIZE		8
#define	EM2_RAWBEAM2_HEADER_SIZE	42
#define	EM2_RAWBEAM2_BEAM_SIZE		16
#define	EM2_RAWBEAM2_TX_SIZE		12
#define	EM2_RAWBEAM3_HEADER_SIZE	32
#define	EM2_RAWBEAM3_BEAM_SIZE		12
#define	EM2_RAWBEAM3_TX_SIZE		20
#define	EM2_SS_HEADER_SIZE		28
#define	EM2_SS_BEAM_SIZE		6
#define	EM2_SS_MBA_HEADER_SIZE		32
#define	EM2_SS_MBA_BEAM_SIZE		6

/* invalid value flags */
#define	EM2_INVALID_AMP			0x7F
#define	EM2_INVALID_CHAR		0xFF
#define	EM2_INVALID_SHORT		0xFFFF
#define EM2_INVALID_U_INT		0xFFFFFFFF
#define EM2_INVALID_INT			0x7FFFFFFF
     
/* internal data structure for survey data */
struct mbsys_simrad2_ping_struct
	{
	int	png_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_count;	/* sequential counter or input identifier */
	int	png_serial;	/* system 1 or system 2 serial number */
	int	png_latitude;	/* latitude in decimal degrees * 20000000
				    (negative in southern hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	int	png_longitude;	/* longitude in decimal degrees * 10000000
				    (negative in western hemisphere) 
				    if valid, invalid = 0x7FFFFFFF */
	int	png_speed;	/* speed over ground (cm/sec) if valid,
				    invalid = 0xFFFF */
	int	png_heading;	/* heading (0.01 deg) */
	int	png_heave;	/* heave from interpolation (0.01 m) */
	int	png_roll;	/* roll from interpolation (0.01 deg) */
	int	png_pitch;	/* pitch from interpolation (0.01 deg) */
	int	png_ssv;	/* sound speed at transducer (0.1 m/sec) */
	int	png_xducer_depth;   
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
	int	png_offset_multiplier;	
				/* transmit transducer depth offset multiplier
				   - see note 7 above */ 
	int	png_nbeams_max;	/* maximum number of beams possible */
	int	png_nbeams;	/* number of valid beams */
	int	png_depth_res;	/* depth resolution (0.01 m) */
	int	png_distance_res;	
				/* x and y resolution (0.01 m) */
	int	png_sample_rate;	
				/* sampling rate (Hz) OR depth difference between
				    sonar heads in EM3000D - see note 9 above */
	int	png_depth[MBSYS_SIMRAD2_MAXBEAMS];	
				/* depths in depth resolution units */
	int	png_acrosstrack[MBSYS_SIMRAD2_MAXBEAMS];
				/* acrosstrack distances in distance resolution units */
	int	png_alongtrack[MBSYS_SIMRAD2_MAXBEAMS];
				/* alongtrack distances in distance resolution units */
	int	png_depression[MBSYS_SIMRAD2_MAXBEAMS];
				/* Primary beam angles in one of two formats (see note 10 above)
				   1) Corrected format - gives beam depression angles
				        in 0.01 degree. These are the takeoff angles used
					in raytracing calculations.
				   2) Uncorrected format - gives beam pointing angles
				        in 0.01 degree. These values are relative to
					the transducer array and have not been corrected
					for vessel motion. */
	int	png_azimuth[MBSYS_SIMRAD2_MAXBEAMS];
				/* Secondary beam angles in one of two formats (see note 10 above)
				   1) Corrected format - gives beam azimuth angles
				        in 0.01 degree. These values used to rotate sounding
					position relative to the sonar after raytracing.
				   2) Uncorrected format - combines a flag indicating that
				        the angles are in the uncorrected format with
					beam tilt angles. Values greater than
					35999 indicate the uncorrected format is in use. The
					beam tilt angles are given as (value - 54000) in
					0.01 degree; the tilt angles give the tilt of the
					transmitted ping due to compensation for vessel
					motion. */
	int	png_range[MBSYS_SIMRAD2_MAXBEAMS];
				/* Ranges in one of two formats (see note 10 above):
				   1) Corrected format - the ranges are one way 
				        travel times in time units defined as half 
					the inverse sampling rate.
				   2) Uncorrected format - the ranges are raw two
				        way travel times in time units defined as
					half the inverse sampling rate. These values
					have not been corrected for changes in the
					heave during the ping cycle. */
	int	png_quality[MBSYS_SIMRAD2_MAXBEAMS];	
				/* 0-254 */
	int	png_window[MBSYS_SIMRAD2_MAXBEAMS];		
				/* samples/4 */
	int	png_amp[MBSYS_SIMRAD2_MAXBEAMS];		
				/* 0.5 dB */
	int	png_beam_num[MBSYS_SIMRAD2_MAXBEAMS];	
				/* beam 128 is first beam on 
				    second head of EM3000D */
	char	png_beamflag[MBSYS_SIMRAD2_MAXBEAMS];	
				/* uses standard MB-System beamflags */
				
	/* raw travel time and angle data */
	int	png_raw1_read;	/* flag indicating actual reading of old rawbeam record */
	int	png_raw2_read;	/* flag indicating actual reading of new rawbeam record */
	int	png_raw_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_raw_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_raw_count;	/* sequential counter or input identifier */
	int	png_raw_serial;	/* system 1 or system 2 serial number */
	int	png_raw_heading;	/* heading (0.01 deg) */
	int	png_raw_ssv;		/* sound speed at transducer (0.1 m/sec) */
	int	png_raw_xducer_depth;	/* transmit transducer depth (0.01 m) */
	int	png_raw_nbeams_max;	/* maximum number of beams possible */
	int	png_raw_nbeams;		/* number of raw travel times and angles
					    - nonzero only if raw beam record read */
	int	png_raw_depth_res;	/* depth resolution (0.01 m) */
	int	png_raw_distance_res;	/* x and y resolution (0.01 m) */
	int	png_raw_sample_rate;	/* sampling rate (Hz or 0.01 Hz) */
	int	png_raw_status;		/* status from PU/TRU */
	int	png_raw_rangenormal;	/* normal incidence range (meters) */
	int	png_raw_normalbackscatter; /* normal incidence backscatter (dB) (-60 to +9) */
	int	png_raw_obliquebackscatter; /* oblique incidence backscatter (dB) (-60 to +9) */
	int	png_raw_fixedgain;	/* fixed gain (dB) (0 to 30) */
	int	png_raw_txpower;	/* transmit power (dB) (0, -10, or -20) */
	int	png_raw_mode;		/* sonar mode: 
						0 : very shallow
						1 : shallow
						2 : medium
						3 : deep
						4 : very deep
						5 : extra deep */
	int	png_raw_coverage;	/* swath width (degrees) (10 to 150 degrees) */
	int	png_raw_yawstabheading; /* yaw stabilization heading (0.01 degrees) */
	int	png_raw_ntx;		/* number of TX pulses (1 to 9) */
	int	png_raw_txlastbeam[MBSYS_SIMRAD2_MAXTX];/* last beam number in this TX pulse */
	int	png_raw_txtiltangle[MBSYS_SIMRAD2_MAXTX];/* tilt angle (0.01 deg) */
	int	png_raw_txheading[MBSYS_SIMRAD2_MAXTX];	/* heading (0.01 deg) */
	int	png_raw_txroll[MBSYS_SIMRAD2_MAXTX];	/* roll (0.01 deg) */
	int	png_raw_txpitch[MBSYS_SIMRAD2_MAXTX];	/* pitch angle (0.01 deg) */
	int	png_raw_txheave[MBSYS_SIMRAD2_MAXTX];	/* heave (0.01 m) */
	int	png_raw_rxrange[MBSYS_SIMRAD2_MAXBEAMS];
				/* Ranges as raw two way travel times in time 
					units defined as one-fourth the inverse 
					sampling rate. These values have not 
					been corrected for changes in the
					heave during the ping cycle. */
	int	png_raw_rxquality[MBSYS_SIMRAD2_MAXBEAMS];	/* beam quality flag */
	int	png_raw_rxwindow[MBSYS_SIMRAD2_MAXBEAMS];	/* length of detection window */
	int	png_raw_rxamp[MBSYS_SIMRAD2_MAXBEAMS];		/* 0.5 dB */
	int	png_raw_rxbeam_num[MBSYS_SIMRAD2_MAXBEAMS];	
				/* beam 128 is first beam on 
				    second head of EM3000D */
	int	png_raw_rxpointangle[MBSYS_SIMRAD2_MAXBEAMS];
				/* Raw beam pointing angles in 0.01 degree,
					positive to port. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
	int	png_raw_rxtiltangle[MBSYS_SIMRAD2_MAXBEAMS];
				/* Raw transmit tilt angles in 0.01 degree,
					positive forward. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
	int	png_raw_rxheading[MBSYS_SIMRAD2_MAXBEAMS];	/* heading (0.01 deg) */
	int	png_raw_rxroll[MBSYS_SIMRAD2_MAXBEAMS];	/* roll (0.01 deg) */
	int	png_raw_rxpitch[MBSYS_SIMRAD2_MAXBEAMS];	/* pitch angle (0.01 deg) */
	int	png_raw_rxheave[MBSYS_SIMRAD2_MAXBEAMS];	/* heave (0.01 m) */
				
	/* raw travel time and angle data version 3 */
	int	png_raw3_read;	/* flag indicating actual reading of newer rawbeam record */
	int	png_raw3_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_raw3_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_raw3_count;	/* sequential counter or input identifier */
	int	png_raw3_serial;	/* system 1 or system 2 serial number */
	int	png_raw3_ntx;		/* number of TX pulses (1 to 9) */
	int	png_raw3_nbeams;		/* number of raw travel times and angles
					    - nonzero only if raw beam record read */
	int	png_raw3_sample_rate;	/* sampling rate (Hz or 0.01 Hz) */
	int	png_raw3_xducer_depth;	/* transmit transducer depth (0.01 m) */
	int	png_raw3_ssv;		/* sound speed at transducer (0.1 m/sec) */
	int	png_raw3_nbeams_max;	/* maximum number of beams possible */
	int	png_raw3_txtiltangle[MBSYS_SIMRAD2_MAXTX];/* tilt angle (0.01 deg) */
	int	png_raw3_txfocus[MBSYS_SIMRAD2_MAXTX];   /* focus range (0.1 m)
								0 = no focus */
	int	png_raw3_txsignallength[MBSYS_SIMRAD2_MAXTX];	/* signal length (usec) */
	int	png_raw3_txoffset[MBSYS_SIMRAD2_MAXTX];	/* transmit time offset (usec) */
	int	png_raw3_txcenter[MBSYS_SIMRAD2_MAXTX];	/* center frequency (Hz) */
	int	png_raw3_txbandwidth[MBSYS_SIMRAD2_MAXTX];	/* bandwidth (10 Hz) */
	int	png_raw3_txwaveform[MBSYS_SIMRAD2_MAXTX];	/* signal waveform identifier 
									0 = CW, 1 = FM */
	int	png_raw3_txsector[MBSYS_SIMRAD2_MAXTX];	/* transmit sector number (0-19) */
	int	png_raw3_rxpointangle[MBSYS_SIMRAD2_MAXBEAMS];
				/* Raw beam pointing angles in 0.01 degree,
					positive to port. 
					These values are relative to the transducer 
					array and have not been corrected
					for vessel motion. */
	int	png_raw3_rxrange[MBSYS_SIMRAD2_MAXBEAMS];	/* Ranges (0.25 samples) */
	int	png_raw3_rxsector[MBSYS_SIMRAD2_MAXBEAMS];	/* transmit sector identifier */
	int	png_raw3_rxamp[MBSYS_SIMRAD2_MAXBEAMS];		/* 0.5 dB */
	int	png_raw3_rxquality[MBSYS_SIMRAD2_MAXBEAMS];	/* beam quality flag */
	int	png_raw3_rxwindow[MBSYS_SIMRAD2_MAXBEAMS];	/* length of detection window */
	int	png_raw3_rxbeam_num[MBSYS_SIMRAD2_MAXBEAMS];	
				/* beam 128 is first beam on 
				    second head of EM3000D */
	int	png_raw3_rxspare[MBSYS_SIMRAD2_MAXBEAMS];	/* spare */

	/* sidescan */
	int	png_ss_read;	/* flag indicating actual reading of sidescan record */
	int	png_ss_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	png_ss_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	png_ss_count;	/* sequential counter or input identifier */
	int	png_ss_serial;	/* system 1 or system 2 serial number */
	int	png_max_range;	/* old: max range of ping in number of samples */
				/* current: mean absorption coefficient in 0.01 db/km */
				/* also: used to store sidescan pixel size in range (m)
				    for old Simrad multibeams (e.g. EM12, EM1000) */
	int	png_r_zero;	/* old: range to normal incidence used in TVG
				    (R0 predicted) in samples */
				/* current: pulse length in usec */
	int	png_r_zero_corr;/* range to normal incidence used to correct
				    sample amplitudes in number of samples */
	int	png_tvg_start;	/* start sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
	int	png_tvg_stop;	/* stop sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
	int	png_bsn;	/* normal incidence backscatter (BSN) in dB */
	int	png_bso;	/* oblique incidence backscatter (BSO) in dB */
	int	png_tx;		/* Tx beamwidth in 0.1 degree */
	int	png_tvg_crossover;	
				/* TVG law crossover angle in degrees */
	int	png_nbeams_ss;	/* number of beams with sidescan */
	int	png_npixels;	/* number of pixels of sidescan */
	int	png_beam_index[MBSYS_SIMRAD2_MAXBEAMS];	
				/* beam index number */
	int	png_sort_direction[MBSYS_SIMRAD2_MAXBEAMS];	
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
	int	png_beam_samples[MBSYS_SIMRAD2_MAXBEAMS];	
				/* number of sidescan samples derived from
					each beam */
	int	png_start_sample[MBSYS_SIMRAD2_MAXBEAMS];	
				/* start sample number */
	int	png_center_sample[MBSYS_SIMRAD2_MAXBEAMS];	
				/* center sample number */
	mb_s_char png_ssraw[MBSYS_SIMRAD2_MAXRAWPIXELS];
				/* the raw sidescan ordered port to starboard */
	int	png_pixel_size;	/* processed sidescan pixel size in cm */
	int	png_pixels_ss;	/* number of processed sidescan pixels stored */
	short	png_ss[MBSYS_SIMRAD2_MAXPIXELS];
				/* the processed sidescan ordered port to starboard */
	short	png_ssalongtrack[MBSYS_SIMRAD2_MAXPIXELS];
				/* the processed sidescan alongtrack distances 
					in distance resolution units */
	};

/* internal data structure for attitude data */
struct mbsys_simrad2_attitude_struct
	{
	int	att_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	att_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	att_count;	/* sequential counter or input identifier */
	int	att_serial;	/* system 1 or system 2 serial number */
	int	att_ndata;	/* number of attitude data */
	int	att_time[MBSYS_SIMRAD2_MAXATTITUDE];
				/* time since record start (msec) */
	int	att_sensor_status[MBSYS_SIMRAD2_MAXATTITUDE];
				/* see note 12 above */
	int	att_roll[MBSYS_SIMRAD2_MAXATTITUDE];
				/* roll (0.01 degree) */
	int	att_pitch[MBSYS_SIMRAD2_MAXATTITUDE];
				/* pitch (0.01 degree) */
	int	att_heave[MBSYS_SIMRAD2_MAXATTITUDE];
				/* heave (cm) */
	int	att_heading[MBSYS_SIMRAD2_MAXATTITUDE];
				/* heading (0.01 degree) */
	int	att_heading_status;
				/* heading status (0=inactive) */
	};

/* internal data structure for heading data */
struct mbsys_simrad2_heading_struct
	{
	int	hed_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	hed_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	hed_count;	/* sequential counter or input identifier */
	int	hed_serial;	/* system 1 or system 2 serial number */
	int	hed_ndata;	/* number of heading data */
	int	hed_time[MBSYS_SIMRAD2_MAXHEADING];
				/* time since record start (msec) */
	int	hed_heading[MBSYS_SIMRAD2_MAXHEADING];
				/* heading (0.01 degree) */
	int	hed_heading_status;
				/* heading status (0=inactive) */
	};

/* internal data structure for ssv data */
struct mbsys_simrad2_ssv_struct
	{
	int	ssv_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	ssv_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	ssv_count;	/* sequential counter or input identifier */
	int	ssv_serial;	/* system 1 or system 2 serial number */
	int	ssv_ndata;	/* number of ssv data */
	int	ssv_time[MBSYS_SIMRAD2_MAXSSV];
				/* time since record start (msec) */
	int	ssv_ssv[MBSYS_SIMRAD2_MAXSSV];
				/* ssv (0.1 m/s) */
	};

/* internal data structure for tilt data */
struct mbsys_simrad2_tilt_struct
	{
	int	tlt_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
	int	tlt_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
	int	tlt_count;	/* sequential counter or input identifier */
	int	tlt_serial;	/* system 1 or system 2 serial number */
	int	tlt_ndata;	/* number of tilt data */
	int	tlt_time[MBSYS_SIMRAD2_MAXTILT];
				/* time since record start (msec) */
	int	tlt_tilt[MBSYS_SIMRAD2_MAXTILT];
				/* tilt + forward (0.01 degree) */
	};
	
/* internal data structure */
struct mbsys_simrad2_struct
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
	char	par_rop[MBSYS_SIMRAD2_COMMENT_LENGTH];	
				/* responsible operator */
	char	par_sid[MBSYS_SIMRAD2_COMMENT_LENGTH];	
				/* survey identifier */
	char	par_pll[MBSYS_SIMRAD2_COMMENT_LENGTH];	
				/* survey line identifier (planned line number) */
	char	par_com[MBSYS_SIMRAD2_COMMENT_LENGTH];	
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
	int	svp_depth[MBSYS_SIMRAD2_MAXSVP]; 
				/* depth of svp entries (according to svp_depth_res) */
	int	svp_vel[MBSYS_SIMRAD2_MAXSVP];	
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
	struct mbsys_simrad2_attitude_struct *attitude;

	/* pointer to heading data structure */
	struct mbsys_simrad2_heading_struct *heading;

	/* pointer to ssv data structure */
	struct mbsys_simrad2_ssv_struct *ssv;

	/* pointer to tilt data structure */
	struct mbsys_simrad2_tilt_struct *tilt;

	/* pointer to survey data structure */
	struct mbsys_simrad2_ping_struct *ping;
	};
	
	
/* system specific function prototypes */
int mbsys_simrad2_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_simrad2_survey_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad2_attitude_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad2_heading_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad2_ssv_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad2_tlt_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_simrad2_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_simrad2_zero_ss(int verbose, void *store_ptr, int *error);
int mbsys_simrad2_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_simrad2_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_simrad2_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_simrad2_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_simrad2_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_simrad2_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_simrad2_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_simrad2_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_simrad2_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_simrad2_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_simrad2_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_simrad2_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size, 
		int swath_width_set, double *swath_width, 
		int pixel_int, 
		int *error);

