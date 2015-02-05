/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	1/19/93
 *    $Id$
 *
 *    Copyright (c) 1993-2015 by
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
 * mb_io.h defines data structures used by MBIO "mb_" functions
 * to store parameters relating to reading data from or writing
 * data to a single multibeam data file.
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 */

/* include this code only once */
#ifndef MB_IO_DEF
#define MB_IO_DEF

#include "mb_config.h"
#include "mb_define.h"
#include "mb_status.h"
        
/* survey platform definition structures */
struct mb_sensor_offsets
        {
        
        int     time_latency_mode;
        double  time_latency_static;
        int     time_latency_n;
        int     time_latency_nalloc;
        double  *time_latency_time_d;
        double  *time_latency_value;
        
        int     position_offset_mode;
        double  position_offset_x;
        double  position_offset_y;
        double  position_offset_z;
        
        int     angular_offset_mode;
        double  angular_offset_azimuth;
        double  angular_offset_roll;
        double  angular_offset_pitch;
        
        };

#define MB_SENSOR_TYPE_SONAR_NONE                       0
#define MB_SENSOR_TYPE_SONAR_ECHOSOUNDER                1
#define MB_SENSOR_TYPE_SONAR_MULTIECHOSOUNDER           2
#define MB_SENSOR_TYPE_SONAR_SIDESCAN                   3
#define MB_SENSOR_TYPE_SONAR_INTERFEROMETRY             4
#define MB_SENSOR_TYPE_SONAR_MULTIBEAM                  5
#define MB_SENSOR_TYPE_SONAR_SUBBOTTOM                  6
#define MB_SENSOR_TYPE_CAMERA_MONO                      21
#define MB_SENSOR_TYPE_CAMERA_STEREO                    22
#define MB_SENSOR_TYPE_CAMERA_VIDEO                     23
#define MB_SENSOR_TYPE_LIDAR_SCAN                       31
#define MB_SENSOR_TYPE_LIDAR_SWATH                      32
#define MB_SENSOR_TYPE_POSITION                         51
#define MB_SENSOR_TYPE_COMPASS                          61
#define MB_SENSOR_TYPE_VRU                              71
#define MB_SENSOR_TYPE_IMU                              81
#define MB_SENSOR_TYPE_CTD                              91
#define MB_SENSOR_TYPE_SOUNDSPEED                       101
struct mb_sensor
        {
        int     type;
        int     sensor_class;
        mb_longname sensor_model;
        mb_longname sensor_manufacturer;
        mb_longname sensor_serialnumber;
        int     capability;
        int     special_capability;
        
        int     num_offsets;
        int     num_offsets_alloc;
        struct mb_sensor_offsets    *offsets;
        
        };
        
#define MB_PLATFORM_NONE                0
#define MB_PLATFORM_SURFACE_VESSEL      1
#define MB_PLATFORM_TOW_BODY            2
#define MB_PLATFORM_ROV                 3
#define MB_PLATFORM_AUV                 4
#define MB_PLATFORM_AIRPLANE            5
#define MB_PLATFORM_SATELLITE           6
struct mb_platform
        {
        int             type;
        mb_longname     name;
        mb_longname     organization;
        
        int             num_sensors;
        int             num_sensors_alloc;
        struct mb_sensor       *sensors;
        };

struct mb_io_ping_struct
	{
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	int	nbath;
	int	namp;
	int	nss;
	char	*beamflag;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	};

/* MBIO input/output control structure */
struct mb_io_struct
	{
	/* system byte swapping */
	int	byteswapped;	/* 0 = unswapped, 1 = swapped (Intel byte order) */

	/* format parameters */
	int	format;		/* data format id */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	int	beams_bath_alloc;	/* allocated number of bathymetry beams */
	int	beams_amp_alloc;	/* allocated number of amplitude beams */
	int	pixels_ss_alloc;	/* allocated number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, single normal, xdr, or gsf) */
	int	filemode;	/* file mode (read or write) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	int	svp_source;	/* data record types containing the primary svp */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* control parameters - see mbio manual pages for explanation */
	int	pings;		/* controls ping averaging */
	int	lonflip;	/* controls longitude range */
	double	bounds[4];	/* locations bounds of acceptable data */
	int	btime_i[7];	/* beginning time of acceptable data */
	int	etime_i[7];	/* ending time of acceptable data */
	double	btime_d;	/* beginning time of acceptable data
					in "_d" format (unix seconds) */
	double	etime_d;	/* ending time of acceptable data
					in "_d" format (unix seconds) */
	double	speedmin;	/* minimum ship speed of acceptable data
					in km/hr */
	double	timegap;	/* maximum time between pings without
					a data gap */

	/* file descriptor, file name, and usage flag */
	FILE	*mbfp;		/* file descriptor */
	mb_path	file;	        /* file name */
	long	file_pos;	/* file position at start of
				    last record read */
	long	file_bytes;	/* number of bytes read from file */
        char    *file_iobuffer;  /* file i/o buffer for fread() and fwrite() calls */
	FILE	*mbfp2;		/* file descriptor #2 */
	char	file2[MB_PATH_MAXLINE];	/* file name #2 */
	long	file2_pos;	/* file position #2 at start of
				    last record read */
	long	file2_bytes;	/* number of bytes read from file */
	FILE	*mbfp3;		/* file descriptor #3 */
	char	file3[MB_PATH_MAXLINE];	/* file name #3 */
	long	file3_pos;	/* file position #3 at start of
				    last record read */
	long	file3_bytes;	/* number of bytes read from file */
	int	ncid;		/* netCDF datastream ID */
	int	gsfid;		/* GSF datastream ID */
	void	*xdrs;		/* XDR stream handle */
	void	*xdrs2;		/* XDR stream handle #2 */
	void	*xdrs3;		/* XDR stream handle #2 */

	/* read or write history */
	int	fileheader;	/* indicates whether file header has
					been read or written */
	int	hdr_comment_size;/* number of characters in
					header_comment string */
	int	hdr_comment_loc;/* number of characters already extracted
					from header_comment string */
	char	*hdr_comment;	/* placeholder for long comment strings
					for formats using a single
					comment string in a file header */
	int	irecord_count;	/* counting variable used for VMS derived
					data formats to remove extra
					bytes (e.g. sburivax format) */
	int	orecord_count;	/* counting variable used for VMS derived
					data formats to insert extra
					bytes (e.g. sburivax format) */

	/* pointer to structure containing raw data (could be any format) */
	int	structure_size;
	int	data_structure_size;
	int	header_structure_size;
	void	*raw_data;
	void	*store_data;

	/* working variables */
	int	ping_count;	/* number of pings read or written so far */
	int	nav_count;	/* number of nav records read or written so far */
	int	comment_count;	/* number of comments read or written so far */
	int	pings_avg;	/* number of pings currently averaged */
	int	pings_read;	/* number of pings read this binning cycle */
	int	error_save;	/* saves time gap error to end of binning */
	double	last_time_d;
	double	last_lon;
	double	last_lat;
	double	old_time_d;
	double	old_lon;
	double	old_lat;
	double	old_ntime_d;
	double	old_nlon;
	double	old_nlat;

	/* data binning variables */
	int	pings_binned;
	double	time_d;
	double	lon;
	double	lat;
	double	speed;
	double	heading;
	char	*beamflag;
	double	*bath;
	double	*amp;
	double	*bath_acrosstrack;
	double	*bath_alongtrack;
	int	*bath_num;
	int	*amp_num;
	double	*ss;
	double	*ss_acrosstrack;
	double	*ss_alongtrack;
	int	*ss_num;

	/* current ping variables */
	int	need_new_ping;
	int	new_kind;
	int	new_error;
	char	new_comment[MB_COMMENT_MAXLINE];
	int	new_time_i[7];
	double	new_time_d;
	double	new_lon;
	double	new_lat;
	double	new_speed;
	double	new_heading;
	int	new_beams_bath;	/* number of bathymetry beams */
	int	new_beams_amp;	/* number of amplitude beams
					- either 0 or = beams_bath */
	int	new_pixels_ss;	/* number of sidescan pixels */
	char	*new_beamflag;
	double	*new_bath;
	double	*new_amp;
	double	*new_bath_acrosstrack;
	double	*new_bath_alongtrack;
	double	*new_ss;
	double	*new_ss_acrosstrack;
	double	*new_ss_alongtrack;

	/* variables for projections to and from projected coordinates */
	int	projection_initialized;
        char	projection_id[MB_NAME_LENGTH];
	void 	*pjptr;

	/* variables for interpolating/extrapolating navigation
		for formats containing nav as asynchronous
		position records separate from ping data */
	int nfix;
	double fix_time_d[MB_ASYNCH_SAVE_MAX];
	double fix_lon[MB_ASYNCH_SAVE_MAX];
	double fix_lat[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating attitude
		for formats containing attitude as asynchronous
		data records separate from ping data */
	int nattitude;
	double attitude_time_d[MB_ASYNCH_SAVE_MAX];
	double attitude_heave[MB_ASYNCH_SAVE_MAX];
	double attitude_roll[MB_ASYNCH_SAVE_MAX];
	double attitude_pitch[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating heading
		for formats containing heading as asynchronous
		data records separate from ping data */
	int nheading;
	double heading_time_d[MB_ASYNCH_SAVE_MAX];
	double heading_heading[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating sonar depth
		for formats containing sonar depth as asynchronous
		data records separate from ping data */
	int nsonardepth;
	double sonardepth_time_d[MB_ASYNCH_SAVE_MAX];
	double sonardepth_sonardepth[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating altitude
		for formats containing altitude as asynchronous
		data records separate from ping data */
	int naltitude;
	double altitude_time_d[MB_ASYNCH_SAVE_MAX];
	double altitude_altitude[MB_ASYNCH_SAVE_MAX];

	/* variables for accumulating MBIO notices */
	int	notice_list[MB_NOTICE_MAX];

	/* variable for registering and maintaining application i/o arrays */
	int	bath_arrays_reallocated;
	int	amp_arrays_reallocated;
	int	ss_arrays_reallocated;
	int	n_regarray;
	int	n_regarray_alloc;
	void	**regarray_handle;
	void	**regarray_ptr;
	void	**regarray_oldptr;
	int	*regarray_type;
	size_t	*regarray_size;

	/* variables for saving information */
	char	save_label[12];
	int	save_label_flag;
	int	save_flag;
	int	save1;
	int	save2;
	int	save3;
	int	save4;
	int	save5;
	int	save6;
	int	save7;
	int	save8;
	int	save9;
	int	save10;
	int	save11;
	int	save12;
	int	save13;
	int	save14;
	double	saved1;
	double	saved2;
	double	saved3;
	double	saved4;
	double	saved5;
	void	*saveptr1;
	void	*saveptr2;

	/* function pointers for allocating and deallocating format
		specific structures */
	int (*mb_io_format_alloc)(int verbose, void *mbio_ptr, int *error);
	int (*mb_io_format_free)(int verbose, void *mbio_ptr, int *error);
	int (*mb_io_store_alloc)(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);
	int (*mb_io_store_free)(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);

	/* function pointers for reading and writing records */
	int (*mb_io_read_ping)(int verbose, void *mbio_ptr,
		void *store_ptr, int *error);
	int (*mb_io_write_ping)(int verbose, void *mbio_ptr,
		void *store_ptr, int *error);

	/* function pointers for extracting and inserting data */
	int (*mb_io_dimensions)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error);
	int (*mb_io_pingnumber)(int verbose, void *mbio_ptr,
		int *pingnumber, int *error);
	int (*mb_io_segynumber)(int verbose, void *mbio_ptr,
		int *line, int *shot, int *cdp, int *error);
	int (*mb_io_sonartype)(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error);
	int (*mb_io_sidescantype)(int verbose, void *mbio_ptr, void *store_ptr,
		int *ss_type, int *error);
        int (*mb_io_preprocess)(int verbose, void *mbio_ptr, void *store_ptr,
		double time_d, double navlon, double navlat,
		double speed, double heading, double sonardepth,
		double roll, double pitch, double heave,
		int *error);
	int (*mb_io_extract)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
	int (*mb_io_insert)(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
	int (*mb_io_extract_nav)(int verbose, void *mbio_ptr, void *store_ptr, int *kind,
		int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error);
	int (*mb_io_extract_nnav)(int verbose, void *mbio_ptr, void *store_ptr,
		int nmax, int *kind, int *n,
		int *time_i, double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error);
	int (*mb_io_insert_nav)(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error);
	int (*mb_io_extract_altitude)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error);
	int (*mb_io_insert_altitude)(int verbose, void *mbio_ptr, void *store_ptr,
		double transducer_depth, double altitude,
		int *error);
	int (*mb_io_extract_svp)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nsvp,
		double *depth, double *velocity,
		int *error);
	int (*mb_io_insert_svp)(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error);
	int (*mb_io_ttimes)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams,
		double *ttimes, double	*angles,
		double *angles_forward, double *angles_null,
		double *heave, double *alongtrack_offset,
		double *draft, double *ssv, int *error);
	int (*mb_io_detects)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams, int *detects, int *error);
	int (*mb_io_pulses)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams, int *pulses, int *error);
	int (*mb_io_gains)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, double *transmit_gain, double *pulse_length,
		double *receive_gain, int *error);
	int (*mb_io_extract_rawssdimensions)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, double *sample_interval,
		int *num_samples_port, int *num_samples_stbd, int *error);
	int (*mb_io_extract_rawss)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *sidescan_type, double *sample_interval,
                double *beamwidth_xtrack, double *beamwidth_ltrack,
		int *num_samples_port, double *rawss_port, 
		int *num_samples_stbd, double *rawss_stbd, int *error);
	int (*mb_io_insert_rawss)(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int sidescan_type, double sample_interval,
		double beamwidth_xtrack, double beamwidth_ltrack,
		int num_samples_port, double *rawss_port,
		int num_samples_stbd, double *rawss_stbd, int *error);
	int (*mb_io_extract_segytraceheader)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		void *segytraceheader_ptr,
		int *error);
	int (*mb_io_extract_segy)(int verbose, void *mbio_ptr, void *store_ptr,
		int *sampleformat,
		int *kind,
		void *segytraceheader_ptr,
		float *segydata,
		int *error);
	int (*mb_io_insert_segy)(int verbose, void *mbio_ptr, void *store_ptr,
		int kind,
		void *segytraceheader_ptr,
		float *segydata,
		int *error);
	int (*mb_io_ctd)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nctd, double *time_d,
		double *conductivity, double *temperature,
		double *depth, double *salinity, double *soundspeed, int *error);
	int (*mb_io_ancilliarysensor)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsensor, double *time_d,
		double *sensor1, double *sensor2, double *sensor3,
		double *sensor4, double *sensor5, double *sensor6,
		double *sensor7, double *sensor8, int *error);
	int (*mb_io_copyrecord)(int verbose, void *mbio_ptr,
		void *store_ptr, void *copy_ptr, int *error);

	};

/* MBIO buffer control structure */
struct mb_buffer_struct
	{
	void *buffer[MB_BUFFER_MAX];
	int buffer_kind[MB_BUFFER_MAX];
	int nbuffer;
	};

/* MBIO datalist control structure */
#define MB_DATALIST_RECURSION_MAX 	25
struct mb_datalist_struct {
	int	open;
	int	recursion;
	int	look_processed;
	int	local_weight;
	int	weight_set;
	double	weight;
	FILE	*fp;
	char	path[MB_PATH_MAXLINE];
	struct mb_datalist_struct *datalist;
	};

/* end conditional include */
#endif
