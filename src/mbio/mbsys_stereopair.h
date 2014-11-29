/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_stereopair.h	11/22/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * mbsys_stereopair.h defines the MBIO data structures for handling data from
 * the following data formats:
 *    MBSYS_STEREOPAIR formats (code in mbsys_stereopair.c and mbsys_stereopair.h):
 *      MBF_PHOTGRAM : MBIO ID 251 (code in mbr_photgram.c)
 *
 * Author:	D. W. Caress
 * Date:	November 22, 2014
 *
 *
 */
/*
 * Notes on the mbsys_stereopair data structure and associated format:
 *   1. This is an MB-System i/o module to read and write topography
 *      calculated by photogrammetry from stereo pair photographs.
 *   2. The structure in mbsys_stereopair.h defines the internal
 *      representation of photogrammetric topography data.
 *   3. The functions in mbsys_stereopair.c allow for extracting
 *      data from or inserting data into this internal
 *      representation. These functions are called by the
 *      MBIO API functions found in mbio/mb_access.c.
 *   4. The functions in mbr_photgram.c actually read and
 *      write the mbf_photgram format.
 *   5. Prototypes for all of the functions in mbsys_stereopair.c
 *      are provided in mbsys_stereopair.h.
 *   6. This list of functions corresponds to the function pointers
 *      that are included in the structure mb_io_struct that is defined
 *      in the file mbsystem/src/mbio/mb_io.h
 *      Not all of these functions are required - some only make sense to
 *      define if the relevant data type is part of the format. For instance,
 *      do not define mbsys_stereopair_extract_segy() if there are no subbottom
 *      profiler data supported by this data system
 *   7. The data are structured as deriving from a series of stereo pairs.
 *      The position and attitude of the camera rig are included, as is the
 *      position (relative to the camera) of each sounding derived from the
 *      stereo pair.
 *   8. Files in format mbf_photgram begin with the characters:
 *          ##PHOTGRAM##V001
 *      Following the 16-byte file header, the individual data records follow
 *      in any order. The defined record types include survey (MB_DATA_DATA),
 *      comment (MB_DATA_COMMENT), and INS (MB_DATA_NAV) which includes
 *      navigation, sensor depth, heading, and attitude sampled more frequently
 *      than the stereo photography.
 *
 *      Survey data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x44445047 = "DDPG" = 1145327687)
 *              Time stamp (MB-System time_d)       8F      Decimal seconds since 1970/1/1/ 00:00:00
 *              Longitude                           8F      Decimal degrees
 *              Lattitude                           8F      Decimal degrees
 *              Sensor depth                        8F      Meters
 *              Heading                             4F      Decimal degrees
 *              Roll                                4F      Decimal degrees
 *              Pitch                               4F      Decimal degrees
 *              Speed                               4F      Decimal degrees
 *              Altitude                            4F      Decimal degrees
 *              N (Number of soundings)             4U
 *              ------------------------------------------------------------
 *              Repeat N times:
 *              ------------------------------------------------------------
 *              acrosstrack                         8F      meters
 *              alongtrack                          8F      meters
 *              depth                               8F      meters
 *              beamflag                            1U      beamflag
 *              red                                 1U      0-255
 *              green                               1U      0-255
 *              blue                                1U      0-255
 *              ------------------------------------------------------------
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
 *
 *      INS data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x4444494E = "DDIN" = 1145325902)
 *              Time stamp (MB-System time_d)       8F      Decimal seconds since 1970/1/1/ 00:00:00
 *              Longitude                           8F      Decimal degrees
 *              Lattitude                           8F      Decimal degrees
 *              Sensor depth                        8F      Meters
 *              Heading                             4F      Decimal degrees
 *              Roll                                4F      Decimal degrees
 *              Pitch                               4F      Decimal degrees
 *              Speed                               4F      Decimal degrees
 *              Altitude                            4F      Decimal degrees
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
 *
 *      Comment data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x4444434D = "DDCM" = 1145324365)
 *              Number of characters in comment     4U      Includes at least one terminating
 *                                                          null character, multiple of 4.
 *              Comment                             NC      
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------*/
/* Record ID definitions (if needed for use in data reading and writing) */

/*---------------------------------------------------------------*/
/* Record size definitions (if needed for use in data reading and writing) */

/*---------------------------------------------------------------*/
/* Array size definitions (if needed for use in data reading and writing) */
#define MBSYS_STEREOPAIR_MAX_BEAMS 400
#define MBSYS_STEREOPAIR_MAX_PIXELS 400

#define MBSYS_STEREOPAIR_SOUNDING_SIZE  28
#define MBSYS_STEREOPAIR_INS_SIZE       52
#define MBSYS_STEREOPAIR_HEADER_SIZE    56

/*---------------------------------------------------------------*/

/* Structure size definitions (if needed because there are dynamically allocated substructures) */
struct mbsys_stereopair_sounding_struct
        {
        double          acrosstrack;
        double          alongtrack;
        double          depth;
        mb_u_char       beamflag;
        mb_u_char       red;
        mb_u_char       green;
        mb_u_char       blue;
        };

/* Internal data structure */
struct mbsys_stereopair_struct
	{
	/* Type of most recently read data record */
	int		kind;			/* MB-System record ID */

	/* MB-System time stamp of most recently read record */
	double		time_d;
	int		time_i[7];
        
        /* Navigation */
        double          longitude;      /* degrees */
        double          latitude;       /* degrees */
        double          sensordepth;    /* meters */
        float           heading;        /* degrees */
        float           roll;           /* degrees */
        float           pitch;          /* degrees */
        float           speed;          /* m/sec */
        float           altitude;       /* meters */
        
        /* Photogrammetric soundings */
        int             num_soundings;
        int             num_soundings_alloc;
        struct mbsys_stereopair_sounding_struct *soundings;
        
        /* Comment */
        int             comment_len;
        char            comment[MB_COMMENT_MAXLINE];
	};
        
/*---------------------------------------------------------------*/

/* System specific function prototypes */
/* Note: this list of functions corresponds to the function pointers
 * that are included in the structure mb_io_struct that is defined
 * in the file mbsystem/src/mbio/mb_io.h
 * Not all of these functions are required - some only make sense to
 * define if the relevant data type is part of the format. For instance,
 * do not define mbsys_stereopair_extract_segy() if there are no subbottom
 * profiler data supported by this data system.
 * The function prototypes that are not required for all data systems
 * are commented out below. When using this example as the basis for
 * for coding a new MB-System I/O module, uncomment any non-required
 * functions that will be useful. */
int mbsys_stereopair_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_stereopair_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_stereopair_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_stereopair_pingnumber(int verbose, void *mbio_ptr,
			int *pingnumber, int *error);
int mbsys_stereopair_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
                        int *sonartype, int *error);
int mbsys_stereopair_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
                        int *ss_type, int *error);
//int mbsys_stereopair_preprocess(int verbose, void *mbio_ptr, void *store_ptr,
//                        double time_d, double navlon, double navlat,
//                        double speed, double heading, double sonardepth,
//                        double roll, double pitch, double heave,
//                        int *error);
int mbsys_stereopair_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_stereopair_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_stereopair_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_stereopair_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_stereopair_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_stereopair_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
//int mbsys_stereopair_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
//                        double transducer_depth, double altitude,
//                        int *error);
//int mbsys_stereopair_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind,
//			int *nsvp,
//			double *depth, double *velocity,
//			int *error);
//int mbsys_stereopair_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
//			int nsvp,
//			double *depth, double *velocity,
//			int *error);
int mbsys_stereopair_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
//int mbsys_stereopair_detects(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nbeams, int *detects, int *error);
//int mbsys_stereopair_pulses(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *kind, int *nbeams, int *pulses, int *error);
//int mbsys_stereopair_gains(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, double *transmit_gain, double *pulse_length,
//			double *receive_gain, int *error);
//int mbsys_stereopair_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_stereopair_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_stereopair_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind,
//			void *segytraceheader_ptr,
//			int *error);
//int mbsys_stereopair_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int *sampleformat,
//			int *kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_stereopair_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_stereopair_ctd(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nctd, double *time_d,
//			double *conductivity, double *temperature,
//			double *depth, double *salinity, double *soundspeed, int *error);
//int mbsys_stereopair_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nsensor, double *time_d,
//			double *sensor1, double *sensor2, double *sensor3,
//			double *sensor4, double *sensor5, double *sensor6,
//			double *sensor7, double *sensor8, int *error);
int mbsys_stereopair_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
/*---------------------------------------------------------------*/
