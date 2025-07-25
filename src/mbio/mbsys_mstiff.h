/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_mstiff.h	4/10/98
 *
 *    Copyright (c) 1998-2025 by
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
 * mbsys_mstiff.h  defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_MSTIFFSS format (MBIO id 131).
 *
 * Author:	D. W. Caress
 * Date:	April 10, 1988
 *
 */
/*
 * Notes on the MBSYS_MSTIFF data structure:
 *   1. The MSTIFF data format is used to store raw sidescan data from
 *      Sea Scan sidescan sonars. This format is a variant of
 *      the TIFF image format with navigation and other information
 *      embedded within the file.
 *   2. The file structure consists of a bunch of pointers to
 *      data objects at various arbitrary locations within the
 *      file. The header contains a pointer to the location of
 *      the "image file directory", which in turn contains
 *      pointers to the locations of data arrays within the file.
 *   3. As far as MB-System is concerned,  this is a read-only
 *      data format.
 *   4. The raw sidescan data in the file consists of 1000 pings.
 *      Each ping produces two 512 sample arrays - one for
 *      each side (port and starboard).
 *   5. The sidescan data is not slant range corrected - the
 *      bottom detect and slant range correction is done on
 *      input by MBIO. The data stored internally by MBIO
 *      is slant range corrected.
 *   6. The MSTIFF files contain lots of information not used by
 *      MBIO,  including images of the data derived from a
 *      realtime display.
 *   7. Comments are not supported in this format.
 *   8. Each of the possible data fields is identified by a
 *      unique tag:
 *          Tag        Field Type                Default
 *          ---        ----------                -------
 *          273        Annotation
 *          272        AnnotationCount
 *          279        Annotation2
 *          278        Annotation2Count
 *          281        Annotation3
 *          280        Annotation3Count
 *          260        BinsPerChannel
 *          258        BitsPerBin                8 bits per sample (2 MSB = 0)
 *          254        Compression               1 (no compression)
 *          255        CondensedImage
 *          301        CreatorVersion
 *          256        Description
 *          287        Fathometer
 *          296        Fathometer2
 *          286        FathometerCount
 *          257        History
 *          263        LeftChannel
 *          299        LeftChannel2
 *          289        Magnetometer
 *          288        MagnetometerCount
 *          291        MagnetometerParms
 *          303        MagnetometerParms2
 *          269        Marker
 *          268        MarkerCount
 *          277        Marker2
 *          276        Marker2Count
 *          284        Marker3
 *          283        Marker3Count
 *          295        Marker4
 *          294        Marker4Count
 *          307        Marker5
 *          306        Marker5Count
 *          267        NavInfo
 *          275        NavInfo2
 *          282        NavInfo3
 *          293        NavInfo4
 *          297        NavInfo5
 *          308        NavInfo6
 *          266        NavInfoCount
 *          304        NavInterpolationTimeout
 *          274        PingNavInfo
 *          264        RightChannel
 *          300        RightChannel2
 *          261        ScrollDirection
 *          265        SonarDataInfo
 *          292        SonarDataInfo2
 *          298        SonarDataInfo3
 *          259        SonarLines
 *          271        SurveyPlotterImage
 *          270        SurveyPlotterParms
 *          290        SurveyPlotterParms2
 *          302        SurveyPlotterParms3
 *          305        SurveyPlotterParms4
 *          262        TimeCorrelation
 *          285        Y2KTimeCorrelation
 */

#ifndef MBSYS_MSTIFF_H_
#define MBSYS_MSTIFF_H_

/* number of sidescan pixels for Sea Scan sidescan sonars */
#define MBSYS_MSTIFF_PIXELS 1024

struct mbsys_mstiff_struct {
	/* time stamp */
	double time_d; /* unix time */

	/* position */
	double lat; /* latitude in degrees */
	double lon; /* longitude in degrees */

	/* other values */
	double heading;         /* heading in degrees */
	double course;          /* course made good in degrees */
	double speed;           /* fore-aft speed in km/hr */
	double altitude;        /* altitude in m */
	double slant_range_max; /* meters */
	double range_delay;     /* meters */
	double sample_interval; /* meters */
	double sonar_depth;     /* sonar depth in meters */
	double frequency;       /* sonar frequency in Hz */

	/* sidescan data */
	int pixels_ss; /* number of pixels */
	unsigned char ss[MBSYS_MSTIFF_PIXELS];
	double ssacrosstrack[MBSYS_MSTIFF_PIXELS];
};

/* system specific function prototypes */
int mbsys_mstiff_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_mstiff_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_mstiff_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_mstiff_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                         double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                         double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                         double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_mstiff_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                        double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                        double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                        double *ssalongtrack, char *comment, int *error);
int mbsys_mstiff_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_mstiff_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                        double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                        double *ssv, int *error);
int mbsys_mstiff_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                  double *altitude, int *error);
int mbsys_mstiff_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                             double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                             double *pitch, double *heave, int *error);
int mbsys_mstiff_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                            double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                            int *error);
int mbsys_mstiff_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_MSTIFF_H_ */
