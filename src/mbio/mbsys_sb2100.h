/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2100.h	2/4/94
 *
 *    Copyright (c) 1994-2025 by
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
 * mbsys_sb2100.h defines the data structures used by MBIO functions
 * to store data from SeaBeam 2100 and 1000 multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam 1000/2100
 * data in files include
 *      MBF_SB2100RW : MBIO ID 41
 *      MBF_SB2100BN : MBIO ID 42
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1994
 *
 */
/*
 * Notes on the MBSYS_SB2100 data structure:
 *   1. SeaBeam 2100 multibeam sonars output raw data in two
 *      formats,  with a third created in post processing.
 *      The older format is mostly ascii with binary
 *      sidescan (format 41).  The newer formats are entirely
 *      binary. Format 42 contains all information; format 43
 *      is identical to format 42 except that the sidescan
 *      data has been removed.
 *   2. The SeaBeam 2100 sonars output up to 151 beams of
 *      bathymetry and 2000 pixels of sidescan, along with a
 *      plethora of other information.
 *   3. The records all include navigation and time stamp information.
 *      The record types are for format 41 are:
 *        PR:  sonar parameter record (roll bias, pitch bias, sound velocity profile)
 *        TR:  sonar text record (comments)
 *        DR:  bathymetry data record (bathymetry and per-beam amplitudes)
 *        SS:  side scan data record
 *      The record types are for format 42 are:
 *        PR:  sonar parameter record (roll bias, pitch bias, sound velocity profile)
 *        TR:  sonar text record (comments)
 *        DH:  ping data header (one per ping)
 *        BR:  bathymetry data record (bathymetry and per-beam amplitudes)
 *        SR:  side scan data record
 *   4. For format 41 a single ping usually results in both DR and SS
 *      records.  The PR record occurs every 30 minutes or when
 *      the sound velocity profile or bias parameters are changed.
 *   5. For format 42 a single ping results in DH, BR, and SR records.
 *      Format 42 files created directly by the sonars will have
 *      PR records at the beginning; format 42 files created by
 *      translating format 41 files will have the same frequency of
 *      PR records as in the original files.
 *   5. The kind value in the mbsys_sb2k_struct indicates whether the
 *      mbsys_sb2k_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a ping
 *                   (DR + SS) or (DH + BR + SR)
 *        kind = 2 : comment (TR)
 *        kind = 8 : sonar parameter (PR)
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2100 records.
 */
/*
 * Notes on the MBF_SB2100B1 data format:
 *   1. SeaBeam 2100 multibeam sonars currently generate
 *      raw data in an hybrid ascii/binary format (41).
 *      This is a replacement fully binary (excepting file header)
 *      format which has significantly faster i/o during processing.
 *   2. The SeaBeam 2100 sonars output up to 151 beams of bathymetry
 *      and 2000 pixels of sidescan measurements, along with a plethora
 *      of other information.
 *   3. The record types are:
 *        SB21BIFH:  file header with data format description
 *                   (beginning of file only)
 *        SB21BIPR:  sonar parameter record
 *                   (roll bias, pitch bias, sound velocity profile)
 *        SB21BITR:  sonar text record (comments)
 *        SB21BIDH:  sonar data header (one for each ping)
 *        SB21BIBR:  bathymetry data record (one for each ping)
 *        SB21BISR:  sidescan data record (one for each ping)
 *   4. The file header record occurs at the beginning of each file.
 *      this is a fully ASCII record with line feeds and null
 *      termination so that uninformed users can figure out the
 *      contents of the file without additional documentation.
 *      There is no analog to this header in format 41.
 *   5. The parameter record should be generated at the beginning of
 *      every file (after the header); new files with new parameter
 *      records should be generated any time the roll bias, pitch
 *      bias, or sound velocity profile values change.
 *      The existing SeaBeam 2100 sonars output parameter records when
 *      the sonar begins logging and every 30 minutes thereafter,
 *      regardless of where it appears in files.
 *      The parameter also includes values for navigation offsets
 *      due to the offset between the transducers and the GPS antenna.
 *      SeaBeam sonars do not presently make use of such parameters.
 *   6. Individual comment records are limited to lengths of
 *      1920 characters.
 *      Each file should begin with comment records stating
 *      the sonar and sonar control software version used to
 *      generate the data. This does not occur at present.
 *   7. Each ping generates three data records in the following
 *      order:
 *        SB21BIDH:  sonar data header
 *        SB21BIBR:  bathymetry data record
 *        SB21BISR:  sidescan data record
 *   8. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2100 records.
 *
 *
 */
/*
 * Notes on the MBF_SB2100B2 data format:
 *   1. SeaBeam 2100 multibeam sonars currently generate
 *      raw data in an hybrid ascii/binary format (41).
 *      Format 42 is a replacement fully binary (excepting file header)
 *      format which has significantly faster i/o during processing.
 *      This format is a bathymetry-only variant of format 42 which,
 *      due to the lack of sidescan data, has much smaller data files
 *      and much faster i/o.
 *   2. The SeaBeam 2100 sonars output up to 151 beams of bathymetry
 *      and 2000 pixels of sidescan measurements, along with a plethora
 *      of other information.
 *   3. The record types are:
 *        SB21BIFH:  file header with data format description
 *                   (beginning of file only)
 *        SB21BIPR:  sonar parameter record
 *                   (roll bias, pitch bias, sound velocity profile)
 *        SB21BITR:  sonar text record (comments)
 *        SB21BIDH:  sonar data header (one for each ping)
 *        SB21BIBR:  bathymetry data record (one for each ping)
 *        SB21BISR:  sidescan data record (one for each ping)
 *   4. The file header record occurs at the beginning of each file.
 *      this is a fully ASCII record with line feeds and null
 *      termination so that uninformed users can figure out the
 *      contents of the file without additional documentation.
 *      There is no analog to this header in format 41.
 *   5. The parameter record should be generated at the beginning of
 *      every file (after the header); new files with new parameter
 *      records should be generated any time the roll bias, pitch
 *      bias, or sound velocity profile values change.
 *      The existing SeaBeam 2100 sonars output parameter records when
 *      the sonar begins logging and every 30 minutes thereafter,
 *      regardless of where it appears in files.
 *      The parameter also includes values for navigation offsets
 *      due to the offset between the transducers and the GPS antenna.
 *      SeaBeam sonars do not presently make use of such parameters.
 *   6. Individual comment records are limited to lengths of
 *      1920 characters.
 *      Each file should begin with comment records stating
 *      the sonar and sonar control software version used to
 *      generate the data. This does not occur at present.
 *   7. Each ping generates three data records in the following
 *      order:
 *        SB21BIDH:  sonar data header
 *        SB21BIBR:  bathymetry data record
 *        SB21BISR:  sidescan data record
 *      The sidescan records are present in format 43, but contain
 *      no pixels. This allows format 43 files to be read as
 *      format 42 without breaking anything.
 *   8. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2100 records.
 *
 */

#ifndef MBSYS_SB2100_H_
#define MBSYS_SB2100_H_

/* maximum number of depth-velocity pairs */
#define MBSYS_SB2100_MAXVEL 30

/* maximum line length in characters */
#define MBSYS_SB2100_MAXLINE 1944

/* maximum number of formed beams for SeaBeam 1000/2100 */
#define MBSYS_SB2100_BEAMS 151

/* maximum number of sidescan pixels for SeaBeam 1000/2100 */
#define MBSYS_SB2100_PIXELS 2000

struct mbsys_sb2100_svp_struct {
	float depth;    /* m */
	float velocity; /* m/sec */
};

struct mbsys_sb2100_beam_struct {
	float depth;           /* m */
	float acrosstrack;     /* m */
	float alongtrack;      /* m */
	float range;           /* seconds */
	float angle_across;    /* degrees */
	float angle_forward;   /* degrees */
	short amplitude;       /* 0.25 dB */
	short signal_to_noise; /* dB */
	short echo_length;     /* samples */
	char quality;          /* 0=no data,
	                   Q=poor quality,
	                   blank otherwise */
	char source;           /* B=BDI, W=WMT */
};

struct mbsys_sb2100_ss_struct {
	float amplitude;  /* sidescan value */
	float alongtrack; /* m */
};

struct mbsys_sb2100_struct {
	/* type of data record */
	int kind;

	/* sonar parameters (SB21BIPR) */
	float roll_bias_port;      /* deg */
	float roll_bias_starboard; /* deg */
	float pitch_bias;          /* deg */
	float ship_draft;          /* m */
	float offset_x;            /* m */
	float offset_y;            /* m */
	float offset_z;            /* m */
	int num_svp;
	struct mbsys_sb2100_svp_struct svp[MBSYS_SB2100_MAXVEL];

	/* sonar data header (SB21BIDH) */
	short year;
	short jday;
	short hour;
	short minute;
	short sec;
	short msec;
	short spare1;
	short spare2;
	double longitude;             /* degrees */
	double latitude;              /* degrees */
	float heading;                /* degrees */
	float speed;                  /* m/sec */
	float roll;                   /* degrees */
	float pitch;                  /* degrees */
	float heave;                  /* m */
	float ssv;                    /* m/sec */
	char frequency;               /* L=12kHz; H=36kHz */
	char depth_gate_mode;         /* A=Auto, M=Manual */
	char ping_gain;               /* dB */
	char ping_pulse_width;        /* msec */
	char transmitter_attenuation; /* dB */
	char ssv_source;              /* V=Velocimeter, M=Manual,
	                          T=Temperature */
	char svp_correction; /* 0=None; A=True Xtrack
	             and Apparent Depth;
	             T=True Xtrack and True Depth */
	char pixel_algorithm; /* pixel intensity algorithm
	              D = logarithm, L = linear */
	float pixel_size;     /* m */
	int nbeams;           /* up to 151 */
	int npixels;          /* up to 2000 */
	short spare3;
	short spare4;
	short spare5;
	short spare6;

	/* bathymetry record (SB21BIBR) */
	struct mbsys_sb2100_beam_struct beams[MBSYS_SB2100_BEAMS];

	/* sidescan record (SB21BISR) */
	struct mbsys_sb2100_ss_struct pixels[MBSYS_SB2100_PIXELS];

	/* parameters unique to MBF_SB2100RW format */
	char range_scale; /* D = m; I = 0.1 m; S = 0.01 m */
	char spare_dr[2];
	int num_algorithms; /* If 1 then only "best" algorithm
	                recorded, else multiple
	                algorithm results recorded */
	char algorithm_order[4]; /* blank if num_algorithms=1;
	                 W=WMT and B=BDI */
	char svp_corr_ss;        /* 0=off; 1=on */
	int ss_data_length;      /* number of bytes of sidescan data */
	char pixel_size_scale;
	char spare_ss;

	/* comment (SB21BITR) - comments are stored by
	    recasting pointer to roll_bias_port to a char ptr
	    and writing over up to MBSYS_SB2100_MAXLINE
	    bytes in structure */
	char *comment;
};

/* system specific function prototypes */
int mbsys_sb2100_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_sb2100_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_sb2100_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_sb2100_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                         double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                         double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                         double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_sb2100_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                        double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                        double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                        double *ssalongtrack, char *comment, int *error);
int mbsys_sb2100_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                        double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                        double *ssv, int *error);
int mbsys_sb2100_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_sb2100_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                       double *receive_gain, int *error);
int mbsys_sb2100_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                  double *altitude, int *error);
int mbsys_sb2100_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                             double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                             double *pitch, double *heave, int *error);
int mbsys_sb2100_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                            double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                            int *error);
int mbsys_sb2100_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                             int *error);
int mbsys_sb2100_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
int mbsys_sb2100_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_SB2100_H_ */
