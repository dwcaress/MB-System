/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbldeoih.h	1/20/93
 *	$Id: mbf_mbldeoih.h,v 5.4 2004-09-16 18:59:42 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003, 2004 by
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
 * mbf_mbldeoih.h defines the data structures used by MBIO functions
 * to store swath data read from the MBF_MBLDEOIH format (MBIO id 71).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.1  2000/12/10 20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/02/21  03:23:28  caress
 * Changed location of amp array pointer in
 * struct mbf_mbldeoih_data_struct .
 *
 * Revision 4.0  1994/02/17  21:11:32  caress
 * First cut at new version.  Recast format to include both
 * beam amplitude and sidescan data.  I hope no one has used
 * the old version of this format, as the files will be
 * orphaned!!!
 *
 * Revision 3.0  1993/05/14  22:52:30  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_MBLDEOIH data format:
 *   1. This data format is used to store swath bathymetry
 *      and/or backscatter data with arbitrary numbers of beams
 *      and pixels. This format was created by the 
 *      Lamont-Doherty Earth Observatory and the Monterey Bay  
 *      Aquarium Research Institute to serve as general  
 *      purpose archive formats for processed swath data.
 *   2. The format stores bathymetry, amplitude, and sidescan data.
 *   3. Each data record has a header section and a data section.
 *      The beginning of each header is a two byte identifier.
 *      The size of the header depends on the identifier:
 *           "##" =  8995 : Old comment - 30 byte header
 *           "dd" = 25700 : Old data - 30 byte header
 *           "cc" = 25443 : New comment - 36 byte header
 *           "nn" = 28270 : New data - 2 byte header
 *      In the case of data records, the header contains the time stamp,
 *      navigation, and the numbers of depth, beam amplitude, and
 *      sidescan values.  The data section contains the depth and
 *      backscatter values.  The number of depth and beam amplitude
 *      values is generally different from the number of sidescan
 *      values, so the length of the data section must be calculated
 *      from the numbers of beams and pixels. In the case of comment
 *      records, the header contains no information other than the
 *      identifier whether it is old (30 byte) or new (2 byte). The
 *      data section of the comment record is always 128 bytes. 
 *   4. The data headers have changed and now include beam angle
 *      widths to allow beam footprint calculation. Older data 
 *      is read without complaint, and the beam widths are passed
 *      as zero.
 *   5. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   6. All data arrays are centered.
 *
 * The kind value in the mbsys_ldeoih_struct indicates whether the
 * structure holds data (kind = 1) or an
 * ascii comment record (kind = 0).
 *
 * The structures used to represent the binary data in the MBF_MBLDEOIH format
 * are documented in the mbsys_ldeoih.h file.
 */
