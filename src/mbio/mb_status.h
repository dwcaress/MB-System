/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	3.00	2/1/93
 *    $Id: mb_status.h,v 3.0 1993-04-23 18:56:22 dale Exp $
 *
 *    Copyright (c) 1993 by 
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
 *
 */

/* MB-system version id */
#define	MB_VERSION	"3.00"

/* MBIO function boolean convention */
#define	MB_YES	1
#define	MB_NO	0

/* MBIO data type ("kind") convention */
#define	MB_DATA_KINDS			9
#define	MB_DATA_NONE			0
#define	MB_DATA_DATA			1
#define	MB_DATA_COMMENT			2
#define	MB_DATA_CALIBRATE		3
#define	MB_DATA_MEAN_VELOCITY		4
#define	MB_DATA_VELOCITY_PROFILE	5
#define	MB_DATA_STANDBY			6
#define	MB_DATA_NAV_SOURCE		7
#define	MB_DATA_RAW_LINE		8

/* MBIO function status convention */
#define	MB_SUCCESS			1
#define	MB_FAILURE			0

/* MBIO minimum and maximum error values */
#define	MB_ERROR_MIN			-14
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
	"No data was dumped from the buffer"
	};
static char *unknown_error_msg[] =
	{
	"Unknown error identifier"
	};

