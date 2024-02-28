/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbpronav.h	5/20/99
 *
 *    Copyright (c) 1999-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbf_mbpronav.h defines the data structures used by MBIO functions
 * to store navigation data read from the MBF_MBPRONAV format.
 *
 * Author:	D. W. Caress
 * Date:	May 20, 1999
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

#ifndef MBF_MBPRONAV_H_
#define MBF_MBPRONAV_H_

#define MBF_MBPRONAV_MAXLINE 256

struct mbf_mbpronav_struct {
	/* type of data record */
	int kind;

	/* time stamp */
	double time_d;
	int time_i[7];

	/* navigation */
	double longitude;
	double latitude;
	double heading;
	double speed;
	double sensordepth;
	double roll;
	double pitch;
	double heave;

	/* swathbounds */
	double portlon;
	double portlat;
	double stbdlon;
	double stbdlat;

	/* comment */
	char comment[MBF_MBPRONAV_MAXLINE];
};

#endif  /* MBF_MBPRONAV_H_ */
