/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbarrov2.h	10/3/2006
 *
 *    Copyright (c) 1999-2023 by
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
 * mbf_mbarrov2.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_MBARROV2 format (MBIO id 170).
 *
 * Author:	D. W. Caress
 * Date:	October 3, 2006
 *
 */
/*
 * Notes on the MBF_MBARROV2 data format:
 *   1. MBARI ROV navigation is stored as ascii tables with
 *      commas separating the fields.
 *   2. The MB-System implementation includes the support of
 *      an arbitrary number of comment records at the beginning
 *      of each file. The comment records begin with the character '#'.
 *
 */

#ifndef MBF_MBARROV2_H_
#define MBF_MBARROV2_H_

#define MBF_MBARROV2_MAXLINE 1024

struct mbf_mbarrov2_struct {
	/* type of data record */
	int kind;

	/* RovName */
	char rovname[8];
	int divenumber;

	/* time stamp */
	double time_d;
	int time_i[7];

	/* navigation */
	double longitude;      /* degrees */
	double latitude;       /* degrees */
	double rov_depth;      /* m */
	double rov_pressure;   /* decibars */
	double rov_heading;    /* degrees */
	double rov_altitude;   /* m */
	double rov_pitch;      /* degrees */
	double rov_roll;       /* degrees */
	double ship_longitude; /* degrees */
	double ship_latitude;  /* degrees */
	double ship_heading;   /* degrees */

	/* flags */
	int qc_flag;

	/* comment */
	char comment[MBF_MBARROV2_MAXLINE];
};

#endif  /* MBF_MBARROV2_H_ */
