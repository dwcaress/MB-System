/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	2/1/93
 *    $Id: mb_status.h,v 5.99 2009/03/13 07:05:58 caress Exp $
 *
 *    Copyright (c) 1993-2009 by
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
 * mbio_status.h defines version, status and error codes used 
 * by MBIO functions and programs
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 * $Log: mb_status.h,v $
 * Revision 5.99  2009/03/13 07:05:58  caress
 * Release 5.1.2beta02
 *
 * Revision 5.98  2009/03/09 15:11:31  caress
 * Release 5.1.2beta01
 *
 * Revision 5.97  2009/03/06 21:46:47  caress
 * Release 5.1.2beta01 March 6 2009
 *
 * Revision 5.96  2008/12/31 08:53:32  caress
 * Updates towards release 5.1.1
 *
 * Revision 5.95  2008/11/18 21:26:39  caress
 * Release 5.1.1beta26
 *
 * Revision 5.94  2008/10/17 07:30:22  caress
 * Added format 26 supporting Hydrosweep DS data used by SOPAC.
 *
 * Revision 5.93  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.92  2008/09/20 00:57:41  caress
 * Release 5.1.1beta23
 *
 * Revision 5.91  2008/07/20 15:41:35  caress
 * Release 5.1.1beta21
 *
 * Revision 5.90  2008/07/19 07:41:14  caress
 * Added formats 191 and 192 to support Imagenex Delta T multibeam data.
 *
 * Revision 5.89  2008/07/10 18:25:41  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.88  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.87  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.86  2008/03/14 19:00:45  caress
 * Updated version to 5.1.1beta16.
 *
 * Revision 5.85  2008/03/01 09:12:52  caress
 * Added support for Simrad EM710 multibeam in new formats 58 and 59.
 *
 * Revision 5.84  2007/11/02 22:39:17  caress
 * Distribution 5.1.beta12
 *
 * Revision 5.83  2007/10/17 20:26:03  caress
 * Release 5.1.1beta11
 *
 * Revision 5.82  2007/10/08 17:20:20  caress
 * New MB-System version number.
 *
 * Revision 5.81  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.80  2007/07/05 16:42:40  caress
 * Release 5.1.1beta5
 *
 * Revision 5.79  2007/06/18 01:19:48  caress
 * Changes as of 17 June 2007.
 *
 * Revision 5.78  2007/05/14 06:23:44  caress
 * Fixed list of error message notices.
 *
 * Revision 5.77  2006/11/26 09:37:09  caress
 * Making distribution 5.1.0.
 *
 * Revision 5.76  2006/10/05 18:58:28  caress
 * Changes for 5.1.0beta4
 *
 * Revision 5.75  2006/09/11 19:04:41  caress
 * Setting up release 5.1.0beta3.
 *
 * Revision 5.74  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.73  2006/08/10 00:12:42  caress
 * Working towards 5.1.0beta2
 *
 * Revision 5.72  2006/07/05 19:50:21  caress
 * Working towards 5.1.0beta
 *
 * Revision 5.71  2006/06/22 04:45:42  caress
 * Working towards 5.1.0
 *
 * Revision 5.70  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.69  2006/02/20 05:29:08  caress
 * Release 5.0.9
 *
 * Revision 5.68  2006/02/16 21:25:45  caress
 * Release 5.0.9
 *
 * Revision 5.67  2006/02/08 17:49:59  caress
 * Release 5.0.8. Finally.
 *
 * Revision 5.66  2006/02/06 06:25:30  caress
 * Version 5.0.8
 *
 * Revision 5.65  2006/02/03 21:58:01  caress
 * Set version for 5.0.8beta5.
 *
 * Revision 5.64  2006/02/03 21:08:51  caress
 * Working on supporting water column datagrams in Simrad formats.
 *
 * Revision 5.63  2006/02/01 18:30:55  caress
 * Release 5.0.8beta4
 *
 * Revision 5.62  2006/01/27 19:09:38  caress
 * Version 5.0.8beta2
 *
 * Revision 5.61  2006/01/24 19:11:17  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.60  2006/01/20 19:40:04  caress
 * Working towards 5.0.8
 *
 * Revision 5.59  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.58  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.57  2005/06/15 15:19:00  caress
 * Fixed typo.
 *
 * Revision 5.56  2005/04/07 04:25:55  caress
 * 5.0.7 Release.
 *
 * Revision 5.55  2005/03/26 22:05:17  caress
 * Release 5.0.7.
 *
 * Revision 5.54  2005/02/20 07:32:12  caress
 * Release 5.0.6
 *
 * Revision 5.53  2005/02/19 07:03:14  caress
 * Release 5.0.6
 *
 * Revision 5.52  2005/02/08 23:00:14  caress
 * Heading towards 5.0.6 release.
 *
 * Revision 5.51  2004/11/06 03:55:17  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.50  2004/10/06 19:04:24  caress
 * Release 5.0.5 update.
 *
 * Revision 5.49  2004/05/22 07:07:26  caress
 * Release 5.0.4
 *
 * Revision 5.48  2004/04/27 01:46:13  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.47  2004/02/26 22:43:25  caress
 * Release 5.0.3
 *
 * Revision 5.46  2003/07/27 13:16:01  caress
 * Release 5.0.0
 *
 * Revision 5.45  2003/07/03 14:19:20  caress
 * Release 5.0.0
 *
 * Revision 5.44  2003/04/29 20:32:24  caress
 * Release 5.0.beta31
 *
 * Revision 5.43  2003/04/25 23:19:45  caress
 * Release 5.0.beta30
 *
 * Revision 5.42  2003/04/22 16:29:52  caress
 * Release 5.0.beta30 again.
 *
 * Revision 5.41  2003/04/17 22:16:39  caress
 * Release 5.0.beta30
 *
 * Revision 5.40  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.39  2003/04/16 16:47:41  caress
 * Release 5.0.beta30
 *
 * Revision 5.38  2003/01/15 20:55:53  caress
 * Release 5.0.beta28
 *
 * Revision 5.37  2002/11/14 03:51:15  caress
 * Release 5.0.beta27
 *
 * Revision 5.36  2002/11/12 07:32:15  caress
 * Release 5.0.beta27
 *
 * Revision 5.35  2002/11/04 21:24:42  caress
 * Release 5.0.beta26
 *
 * Revision 5.34  2002/10/15 18:36:43  caress
 * Release 5.0.beta25
 *
 * Revision 5.33  2002/10/04 21:29:01  caress
 * Release 5.0.beta24.
 *
 * Revision 5.32  2002/09/20 22:40:43  caress
 * Real release 5.0.beta23
 *
 * Revision 5.31  2002/09/19 17:33:10  caress
 * Release 5.0.beta23
 *
 * Revision 5.30  2002/09/19 00:56:20  caress
 * Release 5.0.beta23
 *
 * Revision 5.29  2002/08/30 19:27:52  caress
 * Release 5.0.beta22
 *
 * Revision 5.28  2002/08/28 01:41:03  caress
 * Release 5.0.beta22
 *
 * Revision 5.27  2002/08/21 00:58:23  caress
 * Release 5.0.beta22
 *
 * Revision 5.26  2002/07/25 19:09:04  caress
 * Release 5.0.beta21
 *
 * Revision 5.25  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.24  2002/05/31 20:00:24  caress
 * Release 5.0.beta18
 *
 * Revision 5.23  2002/05/29 23:40:48  caress
 * Release 5.0.beta18
 *
 * Revision 5.22  2002/05/02 04:02:51  caress
 * Release 5.0.beta17
 *
 * Revision 5.21  2002/04/08 21:10:50  caress
 * Release 5.0.beta17
 *
 * Revision 5.20  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.19  2002/02/26 08:01:48  caress
 * Release 5.0.beta14
 *
 * Revision 5.18  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.17  2001/12/30 20:36:13  caress
 * Release 5.0.beta12
 *
 * Revision 5.16  2001/12/20 20:48:51  caress
 * Release 5.0.beta11
 *
 * Revision 5.15  2001/11/20  22:00:01  caress
 * Real 5.0.beta10 release
 *
 * Revision 5.14  2001/11/17  07:29:18  caress
 * Release 5.0.beta10
 *
 * Revision 5.13  2001/11/16 20:15:55  caress
 * Set release version 5.0.beta10
 *
 * Revision 5.12  2001/11/06 22:29:14  caress
 * Changed version to 5.0.beta09
 *
 * Revision 5.11  2001/10/19  19:41:09  caress
 * Now uses relative paths.
 *
 * Revision 5.10  2001/10/12  21:10:41  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.9  2001/08/10  22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.8  2001-07-30 17:52:16-07  caress
 * Release 5.0.beta06
 *
 * Revision 5.7  2001/07/27  19:20:01  caress
 * Release 5.0.beta06
 *
 * Revision 5.6  2001/07/20  16:11:01  caress
 * Release 5.0.beta04
 *
 * Revision 5.5  2001/07/20  00:37:46  caress
 * Release 5.0.beta03
 *
 * Revision 5.4  2001/06/08  21:50:13  caress
 * Version 5.0.beta01
 *
 * Revision 5.3  2001/06/01  00:14:06  caress
 * Added support for metadata insertion.
 *
 * Revision 5.2  2001/03/22  22:32:47  caress
 * Setting version for release 5.0.beta00.
 *
 * Revision 5.1  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.20  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.19  2000/07/19  03:54:23  caress
 * Added new beam flagging macro.
 *
 * Revision 4.18  2000/03/06  21:57:03  caress
 * Set version to 4.6.10
 *
 * Revision 4.17  2000/01/20  00:09:04  caress
 * Updated version and build date tags.
 *
 * Revision 4.16  1999/09/15  21:03:17  caress
 * Version strings now set in mb_format.h
 *
 * Revision 4.15  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.14  1999/05/06  23:45:04  caress
 * Release 4.6a.
 *
 * Revision 4.13  1999/01/01  23:41:06  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.12  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.11  1998/10/20 05:00:37  caress
 * Added some new data record types.
 *
 * Revision 4.10  1998/10/07  22:48:34  caress
 * Fixed typo.
 *
 * Revision 4.9  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.8  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.7  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.6  1996/08/26  17:31:10  caress
 * Release 4.4 revision.
 *
 * Revision 4.5  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.4  1995/08/17  14:43:23  caress
 * Revision for release 4.3.
 *
 * Revision 4.3  1995/08/14  12:38:44  caress
 * Changed version to 4.3.
 *
 * Revision 4.2  1995/01/25  17:16:56  caress
 * Changed MB_VERSION to 4.2
 *
 * Revision 4.1  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.3  1994/03/05  02:14:41  caress
 * Altered to accomodate MBF_SB2100RW format.
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/02/17  21:01:39  caress
 * Changed MB_VERSION to 4.00
 *
 * Revision 4.0  1994/02/17  20:27:46  caress
 * First cut at new version.  Added data kind MB_DATA_PARAMETER
 * for SeaBeam 2100 data.
 *
 * Revision 3.0  1993/04/23  18:56:22  dale
 * Initial version
 *
 *
 */

/* include this code only once */
#ifndef MB_STATUS_DEF
#define MB_STATUS_DEF

/* MB-system version id */
#define	MB_VERSION	"5.1.2beta11"
#define	MB_BUILD_DATE	"26 August 2009"

/* MBIO function boolean convention */
#define	MB_YES	1
#define	MB_NO	0

/* MBIO data type ("kind") convention */
#define	MB_DATA_KINDS			45
#define	MB_DATA_NONE			0
#define	MB_DATA_DATA			1	/* general survey data */
#define	MB_DATA_COMMENT			2	/* general comment */
#define	MB_DATA_HEADER			3	/* general header */
#define	MB_DATA_CALIBRATE		4	/* Hydrosweep DS */
#define	MB_DATA_MEAN_VELOCITY		5	/* Hydrosweep DS */
#define	MB_DATA_VELOCITY_PROFILE	6	/* general */
#define	MB_DATA_STANDBY			7	/* Hydrosweep DS */
#define	MB_DATA_NAV_SOURCE		8	/* Hydrosweep DS */
#define	MB_DATA_PARAMETER		9	/* general */
#define	MB_DATA_START			10	/* Simrad */
#define	MB_DATA_STOP			11	/* Simrad */
#define	MB_DATA_NAV			12	/* Simrad, Reson 7k */
#define	MB_DATA_RUN_PARAMETER		13	/* Simrad */
#define	MB_DATA_CLOCK			14	/* Simrad */
#define	MB_DATA_TIDE			15	/* Simrad, Reson 7k */
#define	MB_DATA_HEIGHT			16	/* Simrad */
#define	MB_DATA_HEADING			17	/* Simrad, Hypack */
#define	MB_DATA_ATTITUDE		18	/* Simrad, Hypack, Reson 7k */
#define	MB_DATA_SSV			19	/* Simrad */
#define	MB_DATA_ANGLE			20	/* HSMD */
#define	MB_DATA_EVENT			21	/* HSMD */
#define	MB_DATA_HISTORY			22	/* GSF */
#define	MB_DATA_SUMMARY			23	/* GSF */
#define	MB_DATA_PROCESSING_PARAMETERS	24	/* GSF */
#define	MB_DATA_SENSOR_PARAMETERS	25	/* GSF */
#define	MB_DATA_NAVIGATION_ERROR	26	/* GSF */
#define	MB_DATA_RAW_LINE		27	/* uninterpretable line for ascii formats */
#define	MB_DATA_NAV1			28	/* ancillary nav system 1 */
#define	MB_DATA_NAV2			29	/* ancillary nav system 2 */
#define	MB_DATA_NAV3			30	/* ancillary nav system 3 */
#define	MB_DATA_TILT			31	/* Simrad */
#define	MB_DATA_MOTION			32	/* Reson 7k */
#define	MB_DATA_CTD			33	/* Reson 7k */
#define	MB_DATA_SUBBOTTOM_MCS		34	/* Reson 7k */
#define	MB_DATA_SUBBOTTOM_CNTRBEAM	35	/* Simrad */
#define	MB_DATA_SUBBOTTOM_SUBBOTTOM	36	/* Reson 7k, XTF */
#define	MB_DATA_SIDESCAN2		37	/* Reson 7k, XTF */
#define	MB_DATA_SIDESCAN3		38	/* Reson 7k, XTF */
#define	MB_DATA_IMAGE			39	/* Reson 7k */
#define	MB_DATA_ROLL			40	/* Reson 7k */
#define	MB_DATA_PITCH			41	/* Reson 7k */
#define	MB_DATA_ABSORPTIONLOSS		42	/* Reson 7k */
#define	MB_DATA_SPREADINGLOSS		43	/* Reson 7k */
#define	MB_DATA_INSTALLATION		44	/* Reson 7k */
#define	MB_DATA_WATER_COLUMN		45	/* Simrad */
#define	MB_DATA_STATUS			46	/* Simrad */

/* MBIO function status convention */
#define	MB_SUCCESS			1
#define	MB_FAILURE			0

/* MBIO minimum and maximum error values */
#define	MB_ERROR_MIN			-23
#define	MB_ERROR_MAX			15

/* MBIO function fatal error values */
#define	MB_ERROR_NO_ERROR		0
#define	MB_ERROR_MEMORY_FAIL		1
#define	MB_ERROR_OPEN_FAIL		2
#define	MB_ERROR_BAD_FORMAT		3
#define	MB_ERROR_EOF			4
#define	MB_ERROR_WRITE_FAIL		5
#define	MB_ERROR_NONE_IN_BOUNDS		6
#define	MB_ERROR_NONE_IN_TIME		7
#define	MB_ERROR_BAD_DESCRIPTOR		8
#define	MB_ERROR_BAD_USAGE		9
#define	MB_ERROR_NO_PINGS_BINNED	10
#define	MB_ERROR_BAD_KIND		11
#define MB_ERROR_BAD_PARAMETER		12
#define	MB_ERROR_BAD_BUFFER_ID		13
#define	MB_ERROR_BAD_SYSTEM		14
#define	MB_ERROR_BAD_DATA		15

/* MBIO function nonfatal error values */
#define	MB_ERROR_TIME_GAP		-1
#define	MB_ERROR_OUT_BOUNDS		-2
#define	MB_ERROR_OUT_TIME		-3
#define	MB_ERROR_SPEED_TOO_SMALL	-4
#define	MB_ERROR_COMMENT		-5
#define	MB_ERROR_SUBBOTTOM		-6
#define	MB_ERROR_WATER_COLUMN		-7
#define	MB_ERROR_OTHER			-8
#define	MB_ERROR_UNINTELLIGIBLE		-9
#define	MB_ERROR_IGNORE			-10
#define	MB_ERROR_NO_DATA_REQUESTED	-11
#define	MB_ERROR_BUFFER_FULL		-12
#define	MB_ERROR_NO_DATA_LOADED		-13
#define	MB_ERROR_BUFFER_EMPTY		-14
#define	MB_ERROR_NO_DATA_DUMPED		-15
#define	MB_ERROR_NO_MORE_DATA		-16
#define	MB_ERROR_DATA_NOT_INSERTED	-17
#define	MB_ERROR_BAD_PROJECTION		-18
#define	MB_ERROR_MISSING_PROJECTIONS	-19
#define	MB_ERROR_MISSING_NAVATTITUDE	-20
#define	MB_ERROR_NOT_ENOUGH_DATA	-21
#define	MB_ERROR_FILE_NOT_FOUND		-22
#define	MB_ERROR_FILE_LOCKED		-23

/* MBIO problem values */
#define	MB_PROBLEM_MAX			6
#define	MB_PROBLEM_NO_DATA		1
#define	MB_PROBLEM_ZERO_NAV		2
#define	MB_PROBLEM_TOO_FAST		3
#define	MB_PROBLEM_AVG_TOO_FAST		4
#define	MB_PROBLEM_TOO_DEEP		5
#define	MB_PROBLEM_BAD_DATAGRAM		6

/* processing status values returned by mb_datalist_read2() */
#define MB_PROCESSED_NONE	0
#define MB_PROCESSED_EXIST	1
#define MB_PROCESSED_USE	2

/* MBIO function error messages */
static char *fatal_error_msg[] = 
	{
	"No error",
	"Unable to allocate memory, initialization failed",
	"Unable to open file, initialization failed",
	"Illegal format identifier, initialization failed",
	"Read error, probably end-of-file",
	"Write error",
	"No data in specified location bounds",
	"No data in specified time interval",
	"Invalid mbio i/o descriptor",
	"Inconsistent usage of mbio i/o descriptor",
	"No pings binned but no fatal error - this should not happen!",
	"Invalid data record type specified for writing",
	"Invalid control parameter specified by user",
	"Invalid buffer id",
	"Invalid system id - this should not happen!", 
	"This data file is not in the specified format!"
	};
static char *nonfatal_error_msg[] =
	{
	"No error",
	"Time gap in data",
	"Data outside specified location bounds",
	"Data outside specified time interval",
	"Ship speed too small",
	"Comment record",
	"Subbottom record",
	"Water column record",
	"Neither a data record nor a comment record",
	"Unintelligible data record",
	"Ignore these data",
	"No data requested for buffer load",
	"Data buffer is full",
	"No data was loaded into the buffer",
	"Data buffer is empty",
	"No data was dumped from the buffer",
	"No more survey data records in buffer", 
	"Data inconsistencies prevented inserting data into storage structure",
	"UTM projection initialization failed",
	"Projection database cannot be read", 
	"Missing navigation and/or attitude data", 
	"Not enough data available to perform operation", 
	"Requested file not found", 
	"Requested file locked", 
	};
static char *unknown_error_msg[] =
	{
	"Unknown error identifier"
	};

/* MBIO maximum notice value */
#define	MB_NOTICE_MAX	(MB_DATA_KINDS - MB_ERROR_MIN + MB_PROBLEM_MAX + 1)

/* MBIO function notice messages */
static char *notice_msg[] = 
	{
	"Unknown notice identifier junk", 

	/* notices for data record types */
	"MB_DATA_DATA (ID=1): survey data", 
	"MB_DATA_COMMENT (ID=2): comment", 
	"MB_DATA_HEADER (ID=3): general header", 
	"MB_DATA_CALIBRATE (ID=4): Hydrosweep DS calibration ping", 
	"MB_DATA_MEAN_VELOCITY (ID=5): Hydrosweep DS mean sound speed", 
	"MB_DATA_VELOCITY_PROFILE (ID=6): SVP", 
	"MB_DATA_STANDBY (ID=7): Hydrosweep DS standby record", 
	"MB_DATA_NAV_SOURCE (ID=8): Hydrosweep DS nav source record", 
	"MB_DATA_PARAMETER (ID=9): Parameter record", 
	"MB_DATA_START (ID=10): Simrad start datagram", 
	"MB_DATA_STOP (ID=11): Simrad stop datagram", 
	"MB_DATA_NAV (ID=12): Navigation record", 
	"MB_DATA_RUN_PARAMETER (ID=13): Simrad runtime parameter datagram", 
	"MB_DATA_CLOCK (ID=14): Simrad clock datagram", 
	"MB_DATA_TIDE (ID=15): Tide record", 
	"MB_DATA_HEIGHT (ID=16): Simrad height datagram", 
	"MB_DATA_HEADING (ID=17): Heading record", 
	"MB_DATA_ATTITUDE (ID=18): Attitude record", 
	"MB_DATA_SSV (ID=19): Surface sound speed record", 
	"MB_DATA_ANGLE (ID=20): Beam angle record", 
	"MB_DATA_EVENT (ID=21): Hydrosweep MD event record", 
	"MB_DATA_HISTORY (ID=22): GSF history record", 
	"MB_DATA_SUMMARY (ID=23): GSF summary record", 
	"MB_DATA_PROCESSING_PARAMETERS (ID=24): GSF processing parameters record", 
	"MB_DATA_SENSOR_PARAMETERS (ID=25): GSF sensor parameter record", 
	"MB_DATA_NAVIGATION_ERROR (ID=26): GSF navigation error record", 
	"MB_DATA_RAW_LINE (ID=27): uninterpretable ASCII line", 
	"MB_DATA_NAV1 (ID=28): Auxilliary nav system 1", 
	"MB_DATA_NAV2 (ID=29): Auxilliary nav system 2", 
	"MB_DATA_NAV3 (ID=30): Auxilliary nav system 3", 
	"MB_DATA_TILT (ID=31): Mechanical tilt record", 
	"MB_DATA_MOTION (ID=32): Motion (DVL) sensor record", 
	"MB_DATA_CTD (ID=33): CTD record", 
	"MB_DATA_SUBBOTTOM_MCS (ID=34): MCS subbottom record", 
	"MB_DATA_SUBBOTTOM_CNTRBEAM (ID=35): Centerbeam subbottom record", 
	"MB_DATA_SUBBOTTOM_SUBBOTTOM (ID=36): Subbottom record", 
	"MB_DATA_SIDESCAN2 (ID=37): Secondary sidescan record", 
	"MB_DATA_SIDESCAN3 (ID=38): Tertiary sidescan record", 
	"MB_DATA_IMAGE (ID=39): Sonar image record", 
	"MB_DATA_ROLL (ID=40): Roll record", 
	"MB_DATA_PITCH (ID=41): Pitch record", 
	"MB_DATA_ABSORPTIONLOSS (ID=42): Absorption loss record", 
	"MB_DATA_SPREADINGLOSS (ID=43): Spreading loss record", 
	"MB_DATA_INSTALLATION (ID=44): Installation parameter record", 
	"MB_DATA_WATER_COLUMN (ID=45): Water column record", 
	
	/* notices for nonfatal error messages */
	"MB_ERROR_TIME_GAP (ID=-1): Time gap in data",
	"MB_ERROR_OUT_BOUNDS (ID=-2): Data outside specified location bounds",
	"MB_ERROR_OUT_TIME (ID=-3): Data outside specified time interval",
	"MB_ERROR_SPEED_TOO_SMALL (ID=-4): Ship speed too small",
	"MB_ERROR_COMMENT (ID=-5): Comment record",
	"MB_ERROR_SUBBOTTOM (ID=-6): Subbottom record",
	"MB_ERROR_WATER_COLUMN (ID=-7): Water column record",
	"MB_ERROR_OTHER (ID=-8): Neither a data record nor a comment record",
	"MB_ERROR_UNINTELLIGIBLE (ID=-9): Unintelligible data record",
	"MB_ERROR_IGNORE (ID=-10): Ignore these data",
	"MB_ERROR_NO_DATA_REQUESTED (ID=-11): No data requested for buffer load",
	"MB_ERROR_BUFFER_FULL (ID=-12): Data buffer is full",
	"MB_ERROR_NO_DATA_LOADED (ID=-13): No data was loaded into the buffer",
	"MB_ERROR_BUFFER_EMPTY (ID=-14): Data buffer is empty",
	"MB_ERROR_NO_DATA_DUMPED (ID=-15): No data was dumped from the buffer",
	"MB_ERROR_NO_MORE_DATA (ID=-16): No more survey data records in buffer", 
	"MB_ERROR_DATA_NOT_INSERTED (ID=-17): Data inconsistencies prevented inserting data into storage structure", 
	"MB_ERROR_BAD_PROJECTION (ID=-18): UTM projection initialization failed", 
	"MB_ERROR_MISSING_PROJECTIONS (ID=-19): Projection database cannot be read", 
	"MB_ERROR_MISSING_NAVATTITUDE (ID=-20): Attitude data are missing for this ping",
	"MB_ERROR_NOT_ENOUGH_DATA (ID=-21): Not enough data to perform spline interpolation",
	"MB_ERROR_FILE_NOT_FOUND (ID=-22): Required file cannot be found",
	"MB_ERROR_FILE_LOCKED (ID=-23): Required file locked",
	
	/* problem notices */
	"DATA PROBLEM (ID=1): No survey data found",
	"DATA PROBLEM (ID=2): Zero longitude or latitude in survey data",
	"DATA PROBLEM (ID=3): Instantaneous speed exceeds 25 km/hr",
	"DATA PROBLEM (ID=4): Average speed exceeds 25 km/hr",
	"DATA PROBLEM (ID=5): Sounding depth exceeds 11000 m",
	"DATA PROBLEM (ID=6): Unsupported datagram or record",
	};
static char *unknown_notice_msg[] =
	{
	"Unknown notice identifier detritus"
	};
	
/* MBIO sidescan types 
	- sidescan values can be logarithmic (dB) or linear (usually voltage) */
#define MB_SIDESCAN_LOGARITHMIC 0
#define MB_SIDESCAN_LINEAR	1

/* MBIO null sidescan:
	- value used to flag sidescan values as undefined */
#define MB_SIDESCAN_NULL -1000000000.0
	
/* MBIO unknown time flag:
	- time_d value used to flag unknown time tag
	- e.g. for xyz soundings */
#define MB_TIME_D_UNKNOWN -2209075200.000000

/*
 * The following defines the values used to flag or 
 * select individual bathymetry values (soundings). This scheme
 * is very similar to the convention used in the HMPS 
 * hydrographic data processing package and the SAIC Hydrobat 
 * package. The values passed in MBIO functions are single 
 * byte characters.
 * 
 * Macros used to identify the flags are also defined here.
 * 
 * The flagging scheme is as follows:
 * 
 * Beams cannot be both flagged and selected. However, more than
 * one "reason bit" can be set for either flagging or selection.
 * 
 * The flag and select bits:
 *   xxxxxx00 => This beam is neither flagged nor selected.
 *   xxxxxx01 => This beam is flagged as bad and should be ignored.
 *   xxxxxx10 => This beam has been selected.
 * 
 * Flagging modes:
 *   00000001 => Flagged because no detection was made by the sonar.
 *   xxxxx101 => Flagged by manual editing.
 *   xxxx1x01 => Flagged by automatic filter.
 *   xxx1xx01 => Flagged because uncertainty exceeds 1 X IHO standard.
 *   xx1xxx01 => Flagged because uncertainty exceeds 2 X IHO standard.
 *   x1xxxx01 => Flagged because footprint is too large
 *   1xxxxx01 => Flagged by sonar as unreliable.
 * 
 * Selection modes:
 *   00000010 => Selected, no reason specified.
 *   xxxxx110 => Selected as least depth.
 *   xxxx1x10 => Selected as average depth.
 *   xxx1xx10 => Selected as maximum depth.
 *   xx1xxx10 => Selected as location of sidescan contact.
 *   x1xxxx10 => Selected, spare.
 *   1xxxxx10 => Selected, spare.
 *
 */

/* Definitions for FLAG category */
#define MB_FLAG_NONE			0x00
#define MB_FLAG_FLAG			0x01
#define MB_FLAG_NULL			0x01
#define MB_FLAG_MANUAL			0x04
#define MB_FLAG_FILTER			0x08
#define MB_FLAG_FILTER2			0x10
#define MB_FLAG_GT_1X_IHO		0x10
#define MB_FLAG_GT_2X_IHO		0x20
#define MB_FLAG_FOOTPRINT       	0x40
#define MB_FLAG_SONAR			0x80

/* Definitions for the SELECT category */
#define MB_SELECT_SELECT		0x02
#define MB_SELECT_LEAST			0x04
#define MB_SELECT_MAXIMUM		0x08
#define MB_SELECT_AVERAGE		0x10
#define MB_SELECT_CONTACT		0x20
#define MB_SELECT_SPARE_1		0x40
#define MB_SELECT_SPARE_2		0x80

/* Definitions for macros applying and testing flags */
#define mb_beam_ok(F)				((int)(!(F & MB_FLAG_FLAG)))
#define mb_beam_check_flag(F)			((int)(F & MB_FLAG_FLAG))
#define mb_beam_check_flag_null(F)		((int)(F == MB_FLAG_NULL))
#define mb_beam_check_flag_manual(F)		((int)((F & MB_FLAG_MANUAL   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter(F)		((int)((F & MB_FLAG_FILTER   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter2(F)		((int)((F & MB_FLAG_FILTER2   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_gt_1x_iho(F)		((int)((F & MB_FLAG_GT_1X_IHO) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_gt_2x_iho(F)		((int)((F & MB_FLAG_GT_2X_IHO) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_footprint(F)		((int)((F & MB_FLAG_FOOTPRINT) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_sonar(F)		((int)((F & MB_FLAG_SONAR    ) && (F & MB_FLAG_FLAG)))
#define mb_beam_set_flag_null(F)		(MB_FLAG_NULL)
#define mb_beam_set_flag_manual(F)		(F | 0x05)
#define mb_beam_set_flag_filter(F)		(F | 0x09)
#define mb_beam_set_flag_filter2(F)		(F | 0x11)
#define mb_beam_set_flag_gt_1x_iho(F)		(F | 0x11)
#define mb_beam_set_flag_gt_2x_iho(F)		(F | 0x21)
#define mb_beam_set_flag_footprint(F)		(F | 0x41)
#define mb_beam_check_select(F)			((int)(F & MB_SELECT_SELECT))
#define mb_beam_check_select_least(F)		((int)((F & MB_SELECT_LEAST  ) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_maximum(F)		((int)((F & MB_SELECT_MAXIMUM) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_average(F)		((int)((F & MB_SELECT_AVERAGE) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_contact(F)		((int)((F & MB_SELECT_CONTACT) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_1(F)		((int)((F & MB_SELECT_SPARE_1) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_2(F)		((int)((F & MB_SELECT_SPARE_2) && (F & MB_SELECT_SELECT)))
#define mb_beam_set_select(F)			(F | 0x02)
#define mb_beam_set_select_least(F)		(F | 0x06)
#define mb_beam_set_select_maximum(F)		(F | 0x0a)
#define mb_beam_set_select_average(F)		(F | 0x12)
#define mb_beam_set_select_contact(F)		(F | 0x22)
#define mb_beam_set_select_spare_1(F)		(F | 0x42)
#define mb_beam_set_select_spare_2(F)		(F | 0x82)

/* Bottom detect flags */
#define MB_DETECT_UNKNOWN	0
#define MB_DETECT_AMPLITUDE	1
#define MB_DETECT_PHASE		2
static char *detect_name[] = 
	{	"Unknown",
		"Amplitude",
		"Phase"
	};

/* end conditional include */
#endif
