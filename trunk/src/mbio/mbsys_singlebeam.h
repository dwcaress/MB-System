/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_singlebeam.h	2/17/93
 *	$Id: mbsys_singlebeam.h,v 4.0 1999-04-16 01:27:33 caress Exp $
 *
 *    Copyright (c) 1999 by 
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_singlebeam.h defines the data structures used by MBIO functions
 * to store single beam echosounder or navigation data.
 * The data formats which are commonly used to store single beam
 * data in files include
 *      MBF_MGD77DAT : MBIO ID 161
 *      MBF_SIOUWMRG : MBIO ID 162
 *      MBF_LDEOUWDT : MBIO ID 163
 *      MBF_MBARINAV : MBIO ID 164
 *      MBF_MBARIROV : MBIO ID 165
 *
 * Author:	D. W. Caress
 * Date:	April 13,  1999
 *
 * $Log: not supported by cvs2svn $
 *
 */
/*
 * Notes on the MBSYS_SINGLEBEAM data structure:
 *   1. The single beam formats are intended to support
 *      true single beam formats, marine geophysical underway
 *      data formats, and navigation formats. These formats
 *      in some cases support magnetics and gravity data.
 */

/* maximum line length in characters */
#define MBSYS_SINGLEBEAM_MAXLINE 120

struct mbsys_singlebeam_struct
	{
	/* type of data record */
	int	kind;
	
	/* survey id */
	char	survey_id[24];
			    /* Identifier supplied by the contributing       
				 organization, else given by NGDC in
				 a manner which represents the data. */+

	/* time stamp */
	double	time_d;
	int	time_i[7];
	int	timezone;   /* Corrects time (in characters 13-27)
				 to GMT when added: equals zero when 
				 time is GMT.  Timezone normally falls
				 between -13 and +12 inclusively. */	
	/* navigation */
	double	longitude;
	double	latitude;
	double	heading;    /* degrees */
	double	speed;	    /* km/hr */
	int	nav_type;   /* Indicates how lat/lon was obtained:
				1 = Observed fix
				3 = Interpolated
				9 = Unspecified	*/
	int	nav_quality;
			    /* QUALITY CODE FOR NAVIGATION -
                             5 - Suspected, by the
                                 originating institution
                             6 - Suspected, by the data
                                 center
                             9 - No identifiable problem
                                 found */
				
	/* motion sensor data */
	double	roll;
	double	pitch;
	double	heave;

	/* bathymetry */
	double	tt;	    /* two way travel time in sec */
	double	bath;	    /* corrected depth in m */
	double	altitude;   /* altitude of transducer above seafloor in m */
	double	tide;	    /* tidal correction in m */
	int	bath_corr;  /* BATHYMETRIC CORRECTION CODE
			       This code details the procedure
			       used for determining the sound
			       velocity correction to depth:
				01-55  Matthews' Zones with zone
				59     Matthews' Zones, no zone
				60     S. Kuwahara Formula
				61     Wilson Formula
				62     Del Grosso Formula
				63     Carter's Tables
				88     Other (see Add. Doc.)
				99     Unspecified */
	int	bath_type;  /* BATHYMETRIC TYPE CODE
				 Indicates how the data record's
				 bathymetric value was obtained:
				 1 =    Observed
				 3 =    Interpolated 
				 9 =    Unspecified */

	/* magnetics */
	double	mag_tot_1;  /* MAGNETICS TOTAL FIELD, 1ST SENSOR
				In tenths of nanoteslas (gammas). 
				For leading sensor.  Use this field
				for single sensor. */
	double	mag_tot_2;  /* MAGNETICS TOTAL FIELD, 2ND SENSOR
				In tenths of nanoteslas (gammas). 
				For trailing sensor. */
	double	mag_res;    /* MAGNETICS RESIDUAL FIELD
				In tenths of nanoteslas (gammas). */
	int	mag_res_sensor;
			    /* SENSOR FOR RESIDUAL FIELD
				1 = 1st or leading sensor
				2 = 2nd or trailing sensor
				9 = Unspecified */
	double	mag_diurnal;
			    /* MAGNETICS DIURNAL CORRECTION -
				In tenths of nanoteslas (gammas). 
				(In nanoteslas) if 9-filled
				(i.e., set to "+9999"), total
				and residual fields are assumed
				to be uncorrected; if used,
				total and residuals are assumed
				to have been already corrected. */
	double	mag_altitude;
			    /* DEPTH OR ALTITUDE OF MAGNETICS SENSOR
				In meters.
				+ = Below sealevel
				- = Above sealevel */
				
	/* gravity */
	double	gravity;    /* OBSERVED GRAVITY
                             In milligals.
                             Corrected for Eotvos, drift, and
                             tares */
	double	eotvos;	    /* EOTVOS CORRECTION
                             In milligals.
                             E = 7.5 V cos phi sin alpha + 
                             0.0042 V*V */
	double	free_air;   /* FREE-AIR ANOMALY
                             In milligals.
                             Free-air Anomaly = G(observed) -
                             G(theoretical) */
	
	/* seismic */
	int	seismic_line;
			    /* SEISMIC LINE NUMBER
                             Used for cross referencing with
                             seismic data. */
	int	seismic_shot;
			    /* SEISMIC SHOT-POINT NUMBER */
 
	/* comment */
	char	comment[MBSYS_SINGLEBEAM_MAXLINE];
	};	
