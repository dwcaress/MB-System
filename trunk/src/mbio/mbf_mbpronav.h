/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbpronav.h	5/20/99
 *	$Id$
 *
 *    Copyright (c) 1999-2011 by
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
 * mbf_mbpronav.h defines the data structures used by MBIO functions
 * to store navigation data read from the MBF_MBPRONAV format.  
 *
 * Author:	D. W. Caress
 * Date:	May 20, 1999
 *
 * $Log: mbf_mbpronav.h,v $
 * Revision 5.3  2006/10/05 18:58:28  caress
 * Changes for 5.1.0beta4
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
 * Revision 4.0  1999/10/21  22:39:24  caress
 * Added MBPRONAV format.
 *
 *
 *
 */
/*
 * Notes on the MBF_MBPRONAV data format:
 *   1. This is a simple ascii navigation format which includes
 *      time, longitude, latitude, heading, and speed.
 *   2. The MB-System implementation includes the support of
 *      an arbitrary number of comment records at the beginning
 *      of each file. The comment records begin with the character '#'.
 *   3. Navigation files in the this format may be generated using
 *      mblist with the -OtMXYHS option.
 *   
 */
 
#define	MBF_MBPRONAV_MAXLINE	256

struct mbf_mbpronav_struct
	{
	/* type of data record */
	int	kind;
	
	/* time stamp */
	double	time_d;
	int	time_i[7];

	/* navigation */
	double	longitude;
	double	latitude;
        double  heading;
	double  speed;
	double  sonardepth;
	double  roll;
	double  pitch;
	double  heave;
	
	/* swathbounds */
	double	portlon;
	double	portlat;
	double	stbdlon;
	double	stbdlat;
 
	/* comment */
	char	comment[MBF_MBPRONAV_MAXLINE];
	};	
