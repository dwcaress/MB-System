/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	2/1/93
 *    $Id: mb_status.h,v 5.11 2001-10-19 19:41:09 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * $Log: not supported by cvs2svn $
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
#define	MB_VERSION	"5.0.beta08"
#define	MB_BUILD_DATE	"September 19, 2001"

/* MBIO function boolean convention */
#define	MB_YES	1
#define	MB_NO	0

/* MBIO data type ("kind") convention */
#define	MB_DATA_KINDS			19
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
#define	MB_DATA_NAV			12	/* Simrad */
#define	MB_DATA_RUN_PARAMETER		13	/* Simrad */
#define	MB_DATA_CLOCK			14	/* Simrad */
#define	MB_DATA_TIDE			15	/* Simrad */
#define	MB_DATA_HEIGHT			16	/* Simrad */
#define	MB_DATA_HEADING			17	/* Simrad, Hypack */
#define	MB_DATA_ATTITUDE		18	/* Simrad, Hypack */
#define	MB_DATA_SSV			19	/* Simrad */
#define	MB_DATA_ANGLE			20	/* HSMD */
#define	MB_DATA_EVENT			21	/* HSMD */
#define	MB_DATA_HISTORY			22	/* GSF */
#define	MB_DATA_SUMMARY			23	/* GSF */
#define	MB_DATA_PROCESSING_PARAMETERS	24	/* GSF */
#define	MB_DATA_SENSOR_PARAMETERS	25	/* GSF */
#define	MB_DATA_NAVIGATION_ERROR	26	/* GSF */
#define	MB_DATA_RAW_LINE		27	/* uninterpretable line
							for ascii formats */

/* MBIO function status convention */
#define	MB_SUCCESS			1
#define	MB_FAILURE			0

/* MBIO minimum and maximum error values */
#define	MB_ERROR_MIN			-15
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
#define	MB_ERROR_OTHER			-6
#define	MB_ERROR_UNINTELLIGIBLE		-7
#define	MB_ERROR_IGNORE			-8
#define	MB_ERROR_NO_DATA_REQUESTED	-9
#define	MB_ERROR_BUFFER_FULL		-10
#define	MB_ERROR_NO_DATA_LOADED		-11
#define	MB_ERROR_BUFFER_EMPTY		-12
#define	MB_ERROR_NO_DATA_DUMPED		-13
#define	MB_ERROR_NO_MORE_DATA		-14
#define	MB_ERROR_DATA_NOT_INSERTED	-15

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
	"Neither a data record nor a comment record",
	"Unintelligible data record",
	"Ignore this data",
	"No data requested for buffer load",
	"Data buffer is full",
	"No data was loaded into the buffer",
	"Data buffer is empty",
	"No data was dumped from the buffer",
	"No more survey data records in buffer", 
	"Data inconsistencies prevented inserting data into storage structure"
	};
static char *unknown_error_msg[] =
	{
	"Unknown error identifier"
	};

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

/* end conditional include */
#endif
