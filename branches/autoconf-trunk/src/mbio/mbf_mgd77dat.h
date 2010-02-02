/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mgd77dat.h	5/19/99
 *	$Id$
 *
 *    Copyright (c) 1999-2009 by
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
 * mbf_mgd77dat.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_MGD77DAT format (MBIO id 21).  
 *
 * Author:	D. W. Caress
 * Date:	May 19, 1999
 *
 * $Log: mbf_mgd77dat.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/07/16  19:29:09  caress
 * First revision.
 *
 * Revision 1.1  1999/07/16  19:24:15  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBF_MGD77DAT data format:
 *   1. The MGD77 format is is an exchange format for marine 
 *	geophysical data (bathymetry, magnetics, and gravity).
 *      The format standard is maintained by the National 
 *      Geophysical Data Center of NOAA.
 *   2. The standard MGD77 format includes a 1920 byte header
 *      followed by 120 byte data records. The header consists
 *      of 24 80 byte records. The first character of the first 
 *      header record is either 1 (pre-Y2K fix) or 4 (post-Y2K fix).
 *      MB-System treats the header as 16 120 byte records and
 *      provides no means of modifying the header.
 *   3. The data records are each 120 bytes long. The first
 *      character of each data record is either 
 *      3 (pre-Y2K fix) or 5 (post-Y2K fix).
 *   4. The MB-System implementation includes the support of
 *      an arbitrary number of comment records at the beginning
 *      of each file. The comment records are 120 bytes each
 *      and begin with the character '#'.
 *   
 */

/* header and data record in bytes */
#define MBF_MGD77DAT_HEADER_NUM	    16
#define MBF_MGD77DAT_DATA_LEN	    120

struct mbf_mgd77dat_struct
	{
	/* type of data record */
	int	kind;
	
	/* survey id */
	char	survey_id[8];
			    /* Identifier supplied by the contributing       
				 organization, else given by NGDC in
				 a manner which represents the data. */

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
	int	flag;	    /* MB-System style beamflag */
	double	tt;	    /* two way travel time in sec */
	double	bath;	    /* corrected depth in m */
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
	char	comment[MBF_MGD77DAT_DATA_LEN];
	};	
