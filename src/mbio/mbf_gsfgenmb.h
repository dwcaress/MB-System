/*--------------------------------------------------------------------
 *    The MB-system:	mbf_gsfgenmb.h	2/27/98
 *	$Id$
 *
 *    Copyright (c) 1998-2013 by
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
 * $Log: mbf_gsfgenmb.h,v $
 * Revision 5.3  2009/03/13 07:05:58  caress
 * Release 5.1.2beta02
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
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

#ifndef __GSF_H__
#include "gsf.h"
#endif

struct mbf_gsfgenmb_struct
	{
	int	    kind;
	gsfDataID   dataID;
	gsfRecords  records;
	};
