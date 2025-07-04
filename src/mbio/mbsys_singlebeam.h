/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_singlebeam.h	4/13/93
 *
 *    Copyright (c) 1999-2025 by
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
 * mbsys_singlebeam.h defines the data structures used by MBIO functions
 * to store single beam echosounder or navigation data.
 * The data formats which are commonly used to store single beam
 * data in files include
 *      MBF_MGD77DAT : MBIO ID 161
 *      MBF_SIOUWMRG : MBIO ID 162
 *      MBF_LDEOUWDT : MBIO ID 163
 *      MBF_MBARINAV : MBIO ID 164
 *      MBF_MBARIROV : MBIO ID 165
 *      MBF_MBPRONAV : MBIO ID 166
 *      MBF_MBARROV2 : MBIO ID 170
 *
 * Author:	D. W. Caress
 * Date:	April 13,  1999
 *
 *
 *
 */
/*
 * Notes on the MBSYS_SINGLEBEAM data structure:
 *   1. The single beam formats are intended to support
 *      true single beam formats, marine geophysical underway
 *      data formats, and navigation formats. These formats
 *      in some cases support magnetics and gravity data.
 *   2. With MB-System 5.1, the mbf_mbpronav format has been
 *      extended to include min and max acrosstrack distances
 *      of non-null data for both bathymetry beams and
 *      sidescan pixels. This allows these values to be included
 *      in the *.fnv files and supports mbgrdviz and mbproject.
 *      These values are accessed by a special function
 *      mbsys_singlebeam_swathbounds().
 */

#ifndef MBSYS_SINGLEBEAM_H_
#define MBSYS_SINGLEBEAM_H_

struct mbsys_singlebeam_struct {
	/* type of data record */
	int kind;

	/* survey id */
	char survey_id[8];
	/* Identifier supplied by the contributing
	 organization, else given by NGDC in
	 a manner which represents the data. */

	/* time stamp */
	double time_d;
	int time_i[7];
	int timezone; /* Corrects time (in characters 13-27)
	           to GMT when added: equals zero when
	           time is GMT.  Timezone normally falls
	           between -13 and +12 inclusively. */
	/* navigation */
	double longitude;
	double latitude;
	double easting;
	double northing;
	double heading; /* degrees */
	double speed;   /* km/hr */
	int nav_type; /* Indicates how lat/lon was obtained:
	          1 = Observed fix
	          3 = Interpolated
	          9 = Unspecified	*/
	int nav_quality; /* R2Rnav QUALITY CODE FOR NAVIGATION -
	             5 - Suspected, by the
	                 originating institution
	             6 - Suspected, by the data
	                 center
	             9 - No identifiable problem
	                 found */
	int gps_quality;     /* R2Rnav GPS quality using NMEA-0183 definition:
	                 The National Marine Electronics Association
	                 has defined the following indicator:
	                     0 = fix not available or invalid
	                     1 = GPS Standard Positioning Service (SPS) mode, fix valid
	                     2 = differential GPS, SPS mode, fix valid
	                     3 = GPS Precise Positioning Service (PPS) mode, fix valid
	                         values for the GPS quality
	                     4 = Real Time Kinematic (RTK). Satellite system used in
	                         RTK mode with fixed integers
	                     5 = Float RTK. Satellite system used in RTK mode with
	                         floating integers
	                     6 = Estimated (dead reckoning) mode
	                     7 = Manual input mode
	                     8 = Simulator mode */
	int gps_nsat;        /* R2Rnav number of satellites */
	double gps_dilution; /* R2Rnav GPS horizontal dilution of position (hdop) */
	int gps_height;      /* R2Rnav GPS height (m) */
	
	/* SOI USBL tracking */
	double gps_time;     /* time since start of day */
	// int gps_quality - same as above for R2R
	// int gps_nsat - same as above for R2R
	// double gps_dilution - same as above for R2R
	// double sonar_depth - same as below submersible/ROV data
	
	/* SOI ROV INS Navigation */
	// double roll; - same as below
	// double pitch; - same as below
	// double heading; - same as below
	int orientation_status;
	// double longitude; - same as below
	// double latitude; - same as below
	int position_status;
	double velocity_fwd;
	double velocity_stbd;
	double velocity_down;
	// double altitude; - same as rov_altitude below
	int altitude_status;
	// double depth; - same as sonar_depth below
	int depth_used;

	/* motion sensor data */
	double roll;
	double pitch;
	double heave; /* heave or rov depth in m */

	/* submersible/ROV data */
	double sonar_depth;  /* platform depth in m */
	double rov_pressure; /* platform pressure in decibar */
	double rov_altitude; /* altitude above seafloor in m */

	/* bathymetry */
	int flag;      /* MB-System style beamflag */
	double tt;     /* two way travel time in sec */
	double bath;   /* corrected depth in m */
	double tide;   /* tidal correction in m */
	int bath_corr; /* BATHYMETRIC CORRECTION CODE
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
	int bath_type; /* BATHYMETRIC TYPE CODE
	            Indicates how the data record's
	            bathymetric value was obtained:
	            1 =    Observed
	            3 =    Interpolated
	            9 =    Unspecified */

	/* magnetics */
	double mag_tot_1; /* MAGNETICS TOTAL FIELD, 1ST SENSOR
	          In tenths of nanoteslas (gammas).
	          For leading sensor.  Use this field
	          for single sensor. */
	double mag_tot_2; /* MAGNETICS TOTAL FIELD, 2ND SENSOR
	          In tenths of nanoteslas (gammas).
	          For trailing sensor. */
	double mag_res; /* MAGNETICS RESIDUAL FIELD
	        In tenths of nanoteslas (gammas). */
	int mag_res_sensor;
	/* SENSOR FOR RESIDUAL FIELD
	1 = 1st or leading sensor
	2 = 2nd or trailing sensor
	9 = Unspecified */
	double mag_diurnal;
	/* MAGNETICS DIURNAL CORRECTION -
	In tenths of nanoteslas (gammas).
	(In nanoteslas) if 9-filled
	(i.e., set to "+9999"), total
	and residual fields are assumed
	to be uncorrected; if used,
	total and residuals are assumed
	to have been already corrected. */
	double mag_altitude;
	/* DEPTH OR ALTITUDE OF MAGNETICS SENSOR
	In meters.
	+ = Below sealevel
	- = Above sealevel */
	int mag_qualco; /* quality code for magnetics
	                    1 – good
	                    2 – fair
	                    3 – poor
	                    4 – bad
	                    5 – suspected bad by contributor
	                    6 – suspected bad by data center
	                    nil - unspecified  */

	/* gravity */
	double gravity;  /* OBSERVED GRAVITY
	                      In milligals.
	                      Corrected for Eotvos, drift, and
	                      tares */
	double eotvos;   /* EOTVOS CORRECTION
	                      In milligals.
	                      E = 7.5 V cos phi sin alpha +
	                      0.0042 V*V */
	double free_air; /* FREE-AIR ANOMALY
	                      In milligals.
	                      Free-air Anomaly = G(observed) -
	                      G(theoretical) */
	int gra_qualco;  /* quality code for gravity
	                     1 – good
	                     2 – fair
	                     3 – poor
	                     4 – bad
	                     5 – suspected bad by contributor
	                     6 – suspected bad by data center
	                     nil - unspecified  */

	/* seismic */
	int seismic_line;
	/* SEISMIC LINE NUMBER
	             Used for cross referencing with
	             seismic data. */
	int seismic_shot;
	/* SEISMIC SHOT-POINT NUMBER */
	int seismic_cdp;
	/* SEISMIC CDP-POINT NUMBER */

	/* ship navigation */
	double ship_longitude; /* degrees */
	double ship_latitude;  /* degrees */
	double ship_heading;   /* degrees */

	/* flags */
	int position_flag;
	int pressure_flag;
	int heading_flag;
	int altitude_flag;
	int attitude_flag;
	int qc_flag;

	/* swathbounds */
	double portlon;
	double portlat;
	double stbdlon;
	double stbdlat;

	/* comment */
	char comment[MB_COMMENT_MAXLINE];
};

/* system specific function prototypes */
int mbsys_singlebeam_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_singlebeam_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_singlebeam_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                                int *error);
int mbsys_singlebeam_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_singlebeam_segynumber(int verbose, void *mbio_ptr, unsigned int *line, unsigned int *shot, unsigned int *cdp, int *error);
int mbsys_singlebeam_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                             double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                             char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                             double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_singlebeam_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                            double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag,
                            double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                            double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_singlebeam_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                            double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                            double *ssv, int *error);
int mbsys_singlebeam_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_singlebeam_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                      double *altitude, int *error);
int mbsys_singlebeam_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                                 double *pitch, double *heave, int *error);
int mbsys_singlebeam_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                                double navlat, double speed, double heading, double draft, double roll, double pitch,
                                double heave, int *error);
int mbsys_singlebeam_swathbounds(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *portlon, double *portlat,
                                 double *stbdlon, double *stbdlat, int *error);
int mbsys_singlebeam_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_singlebeam_pressuredepth(int verbose, double pressure, double latitude, double *depth, int *error);

#endif  /* MBSYS_SINGLEBEAM_H_ */
