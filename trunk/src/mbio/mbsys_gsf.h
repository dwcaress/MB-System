/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_gsf.h	6/10/97
 *	$Id: mbsys_gsf.h,v 4.0 1998-10-05 19:16:02 caress Exp $
 *
 *    Copyright (c) 1998 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_gsf.h defines the data structures used by MBIO functions
 * to store data from the Generic Sensor Format (GSF).
 * The MBIO representation of GSF is:
 *      MBF_GSFGENMB : MBIO ID 121
 *
 *
 * Author:	D. W. Caress
 * Date:	March 5, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1998/10/05  18:32:27  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBF_GSFGENMB data format:
 *   1. The underlying data format is the Generic Sensor Format (GSF)
 *      developed by Shannon Byrne of SAIC. The GSF format stores swath 
 *      bathymetry, single beam bathymetry, and other data.
 *   2. This MBIO i/o module accesses swath bathymtry data stored in
 *      the GSF format using the gsflib library also written at SAIC.
 *      The gsflib calls translate the data from scaled short integers
 *      (big endian) stored in the file to double values. The sensor
 *      specific values held in the GSF data stream are not
 *      accessed by this module. However, all of the GSF records
 *      and the included information are passed when mb_get_all, 
 *      mb_put_all, and the mb_buffer routines are used for
 *      reading and writing.
 *
 */

/* internal data structure */
struct mbsys_gsf_struct
	{
	int	    kind;
	gsfDataID   dataID;
	gsfRecords  records;
	};

