/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbarirov.h	5/20/99
 *
 *    Copyright (c) 1999-2020 by
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
 * mbf_mbarirov.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_MBARIROV format (MBIO id 21).
 *
 * Author:	D. W. Caress
 * Date:	May 20, 1999
 *
 *
 */
/*
 * Notes on the MBF_MBARIROV data format:
 *   1. MBARI ROV navigation is stored as ascii tables with
 *      commas separating the fields.
 *   2. The MB-System implementation includes the support of
 *      an arbitrary number of comment records at the beginning
 *      of each file. The comment records begin with the character '#'.
 *
 */

#ifndef MBF_MBARIRO_H_
#define MBF_MBARIRO_H_

#define MBF_MBARIROV_MAXLINE 256

struct mbf_mbarirov_struct {
	/* type of data record */
	int kind;

	/* time stamp */
	double time_d;
	int time_i[7];

	/* navigation */
	double longitude;
	double latitude;
	double easting;
	double northing;
	double rov_depth;    /* m */
	double rov_pressure; /* decibars */
	double rov_heading;  /* degrees */
	double rov_altitude; /* m */
	double rov_pitch;    /* degrees */
	double rov_roll;     /* degrees */

	/* flags */
	int position_flag;
	int pressure_flag;
	int heading_flag;
	int altitude_flag;
	int attitude_flag;

	/* comment */
	char comment[MBF_MBARIROV_MAXLINE];
};

#endif  /* MBF_MBARIRO_H_ */
