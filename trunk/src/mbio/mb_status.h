/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	2/1/93
 *    $Id: mb_status.h,v 4.6 1996-08-26 17:31:10 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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

/* MB-system version id */
#define	MB_VERSION	"4.4"

/* MBIO function boolean convention */
#define	MB_YES	1
#define	MB_NO	0

/* MBIO data type ("kind") convention */
#define	MB_DATA_KINDS			14
#define	MB_DATA_NONE			0
#define	MB_DATA_DATA			1	/* general survey data */
#define	MB_DATA_COMMENT			2	/* general comment */
#define	MB_DATA_CALIBRATE		3	/* Hydrosweep DS */
#define	MB_DATA_MEAN_VELOCITY		4	/* Hydrosweep DS */
#define	MB_DATA_VELOCITY_PROFILE	5	/* general */
#define	MB_DATA_STANDBY			6	/* Hydrosweep DS */
#define	MB_DATA_NAV_SOURCE		7	/* Hydrosweep DS */
#define	MB_DATA_PARAMETER		8	/* SeaBeam 2100 */
#define	MB_DATA_START			9	/* Simrad */
#define	MB_DATA_STOP			10	/* Simrad */
#define	MB_DATA_NAV			11	/* Simrad */
#define	MB_DATA_ANGLE			12	/* HSMD */
#define	MB_DATA_EVENT			13	/* HSMD */
#define	MB_DATA_RAW_LINE		14	/* uninterpretable line
							for ascii formats */

/* MBIO function status convention */
#define	MB_SUCCESS			1
#define	MB_FAILURE			0

/* MBIO minimum and maximum error values */
#define	MB_ERROR_MIN			-15
#define	MB_ERROR_MAX			14

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
	"Invalid system id - this should not happen!"
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

