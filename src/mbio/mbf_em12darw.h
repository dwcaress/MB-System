/*--------------------------------------------------------------------
 *    The MB-system:	mbf_em12darw.h	1/20/93
 *
 *    Copyright (c) 1994-2024 by
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
 * mbf_em12darw.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_EM12DARW format (MBIO id 51).
 *
 * Authors:	D. W. Caress (L-DEO)
 *		R. B. Owens (University of Oxford)
 * Date:	January 20, 1994
 *
 *
 */
/*
 * Notes on the MBF_EM12DARW data format:
 *   1. This data format is used to store 81 beam SIMRAD EM12 bathymetry
 *      data.  This format was created and used by R.B.Owens for
 *      SIMRAD EM12 data collected on the RRS Charles Darwin;
 *   2. The data consist of 1056-byte records including 1-byte characters,
 *      2-byte and 4-byte integers, 4-byte floats and 8-byte doubles.
 *   3. The 81 depth values are stored centered in 81 value arrays.  The
 *      center beam is in word 40 of the depth and distance arrays.
 *   4. The "range" is the two-way pulse travel time for each beam (81 values)
 *   5. The "reflectivity" is the mean backscattering strength (in dB)
 *      over each beam, corrected for beam patterns, echo sounder
 *      parameters and Lambert's law.  In the context of MB-System, the
 *	backscatter values are considered to be amplitudes rather than
 *	sidescan because they are coincident with the bathymetry values.
 *   6. Comments can be embedded in the data as N-byte ascii strings,
 *	where "func" = 100. Comment lines are stored as 1-byte characters
 *      in the "depth" array. For data, "func" = 150.
 *   7. Mode; 1 = Shallow, 2 = Deep. Controls scaling factors, e.g. Depth is
 *      scaled 1/0.1 for mode 1 and scaled 1/0.2 for mode 2.
 *
 */

#ifndef MBF_EM12DARW_H_
#define MBF_EM12DARW_H_

/* record length in bytes */
#define MBF_EM12DARW_RECORD_LENGTH 1056

/* number of beams for EM12 */
#define MBF_EM12DARW_BEAMS 81

struct mbf_em12darw_struct {
	short func;       /* record type; 100=comment, 150=data */
	short year;       /* TWO-digit year */
	short jday;       /* Julian day  */
	short minute;     /* minute of day */
	short secs;       /* seconds x 100 */
	double latitude;  /* latitude (for units see corflag) */
	double longitude; /* longitude (for units see corflag) */
	short corflag;    /* Co-ordinate flag: 0=Lat/Long, 1=UTM North,
	              2=UTM South  */
	float utm_merd;   /* UTM Meridian if corflag=1,2  */
	short utm_zone;   /* UTM Zone if corflag=1,2  */
	short posq;       /* Quality factor of Position data */
	int pingno;       /* ping number */
	short mode;       /* resolution mode */
	float depthl;     /* depth of centrebeam */
	float speed;      /* Ship's speed */
	float gyro;       /* Ship's Heading */
	float roll;       /* Ship's Roll */
	float pitch;      /* Ship's Pitch */
	float heave;      /* Ship's Heave */
	float sndval;     /* Sound Velocity */
	short depth[MBF_EM12DARW_BEAMS];
	/* Beam Depth scaled (0.1/0.2) */
	short distacr[MBF_EM12DARW_BEAMS];
	/* Beam Across Distance, scaled (0.2/0.5) */
	short distalo[MBF_EM12DARW_BEAMS];
	/* Beam Along Distance, scaled (0.2/0.5) */
	short range[MBF_EM12DARW_BEAMS];
	/* Beam Range, scaled (0.2/0.8) */
	short refl[MBF_EM12DARW_BEAMS];
	/* Beam Reflectivity, scaled 0.5 */
	short beamq[MBF_EM12DARW_BEAMS];
	/* Beam Quality, unscaled */
};

#endif  /* MBF_EM12DARW_H_ */
