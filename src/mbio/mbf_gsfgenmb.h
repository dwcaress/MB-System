/*--------------------------------------------------------------------
 *    The MB-system:	mbf_gsfgenmb.h	2/27/98
 *	$Id: mbf_gsfgenmb.h,v 4.1 2000-09-30 06:34:20 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * mbf_gsfgenmb.h defines the data structures used by MBIO functions
 * to store multibeam data read from the  MBF_GSFGENMB format (MBIO id 121).  
 *
 * Author:	D. W. Caress
 * Date:	February 27, 1998
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1998/10/05  19:16:02  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:32:27  caress
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
 */

struct mbf_gsfgenmb_struct
	{
	int	    kind;
	gsfDataID   dataID;
	gsfRecords  records;
	};

