/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_geoswath.h	8/20/2012
 *	$Id: mbsys_geoswath.h 1982 2012-08-15 16:52:53Z caress $
 *
 *    Copyright (c) 2012-2012 by
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
 * mbsys_geoswath.h defines the MBIO data structures for handling data from
 * Kongsberg GeoAcoustics GeoSwath Plus interferometric sonars:
 *      MBF_GEOSWATH : MBIO ID 221 - Kongsberg GeoAcoustics GeoSwath Plus data in the rdf file format
 *
 * Author:	D. W. Caress
 * Date:	August 20, 2012
 *
 * $Log: mbsys_geoswath.h,v $
 *
 *
 */
/*
 * Notes on the mbsys_geoswath data structure:
 *   1. This format is defined by the GeoSwath Plus
 *      Raw Data File Format & Broadcast Raw Data File Format + Command Specification
 *      Document ID: 9-GS+ -6063/BB
 *      Document Date: 22-04-2009
 *   2. The source code here and in mbr_geoswath.c has been modified from that included
 *      in the specification.
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------*/
/* Record ID definitions */

typedef struct
    {
    unsigned short time;    //Time in wavelengths
    short sine; //Sine of return angle
    unsigned short amplitude;   //16bit amplitude value
    } RAA;



typedef struct
{
double x;
double y;
float z;
double time; double time_stamp; char quality;
//Easting
//Northing
//Antenna height
//GPS time
//Time stamp
//GPS quality indicator
unsigned
} NAV;
typedef struct
{
float roll; float pitch; float heave; double time_stamp;
//Roll in degrees
//Pitch in degrees
//Heave in degrees
//Time stamp
} MRU;
typedef struct
9-GS+ -6063/BB
Page 21 of 24
??Raw Data File Format & Broadcast Raw Data File Format + Command Specification
GeoSwath Plus
{
float heading; double time_stamp;
//Heading in degrees
//Time stamp
} GYRO;
typedef struct
{
float depth1; float depth2; double time_stamp;
//Depth1
//Depth2
//Time stamp
} ECHO;
typedef struct
{
float velocity; double time_stamp;
//MiniSVS velocity
//Time stamp
} MINISVS;
typedef struct
{
} AUXDATA;
              type;
char          spare[3];
//Aux type
//Value;
unsigned    char
            float         value;
typedef struct
{
unsigned int
short raw_header_size;
short raw_ping_header_size; char filename[512];
int frequency;
short echo_type;
creation;
unsigned char
char version[8];
file_mode;
} RAWFILEHEADER;
char pps_mode; char spare[8];
typedef struct
{
Page 22 of 24
9-GS+ -6063/BB
??GeoSwath Plus
Raw Data File Format & Broadcast Raw Data File Format + Command Specification
int ping_number; double ping_time;
unsigned
unsigned
unsigned
unsigned
unsigned
unsigned
unsigned
int previous_ping_position; int ping_size;
char navigation_number; char attitude_number;
unsigned
unsigned
unsigned
char heading_number;
char echosounder_number;
char miniSVS_number;
char aux1_number;
char aux2_number;
short ping_length;
char pulse_length;
char power;
char sidescan_gain;
int sample_number;
char side;
short navigation_strings_size; short attitude_strings_size; short heading_strings_size; short echosounder_strings_size; short miniSVS_strings_size; short aux1_strings_size;
short aux2_strings_size;
short ping_delay;
double pps_time;
char source;
short sample_interval;
unsigned
unsigned
        unsigned
}RAWPINGHEADER;
//////////////////////////////////////////////////////////////////////////// //
// function prototypes
// ///////////////////////////////////////////////////////////////////////////
BOOL read_raw_file_header(RAWFILEHEADER *, FILE *);
BOOL read_raw_ping_header(RAWPINGHEADER *, FILE *);
BOOL read_raw_ping_data(char *, int, FILE *);
void get_ping_data(RAA **, NAV **, MRU **, GYRO **, ECHO **, MINISVS **, double **, double **, RAWPINGHEADER, char *);
9-GS+ -6063/BB Page 23 of 24
??Raw Data File Format & Broadcast Raw Data File Format + Command Specification GeoSwath Plus
void get_raw_strings(char **, char **, char **, char **, char **, char **, char **, AUXDATA **, AUXDATA **, RAWPINGHEADER, char *);
BOOL write_raw_file_header(RAWFILEHEADER *, FILE *);
BOOL write_raw_ping_header(RAWPINGHEADER *, FILE *);
BOOL write_raw_ping_data(char *, int, FILE *);







Appendix C: Sample Source Code (rdf.h)
/**************************************************************************** /
/ (c) Copyright 2009 GeoAcoustics Limited
/
/****************************************************************************/
////////////////////////////////////////////////////////////////////////////
//
// Structures
// ///////////////////////////////////////////////////////////////////////////
typedef struct
{
} RAA;
//Time in wavelengths
//Sine of return angle
//16bit amplitude value
unsigned
unsigned
short time; short sine; short amplitude;
typedef struct
{
double x;
double y;
float z;
double time; double time_stamp; char quality;
//Easting
//Northing
//Antenna height
//GPS time
//Time stamp
//GPS quality indicator
unsigned
} NAV;
typedef struct
{
float roll; float pitch; float heave; double time_stamp;
//Roll in degrees
//Pitch in degrees
//Heave in degrees
//Time stamp
} MRU;
typedef struct
9-GS+ -6063/BB
Page 21 of 24
??Raw Data File Format & Broadcast Raw Data File Format + Command Specification
GeoSwath Plus
{
float heading; double time_stamp;
//Heading in degrees
//Time stamp
} GYRO;
typedef struct
{
float depth1; float depth2; double time_stamp;
//Depth1
//Depth2
//Time stamp
} ECHO;
typedef struct
{
float velocity; double time_stamp;
//MiniSVS velocity
//Time stamp
} MINISVS;
typedef struct
{
} AUXDATA;
              type;
char          spare[3];
//Aux type
//Value;
unsigned    char
            float         value;
typedef struct
{
unsigned int
short raw_header_size;
short raw_ping_header_size; char filename[512];
int frequency;
short echo_type;
creation;
unsigned char
char version[8];
file_mode;
} RAWFILEHEADER;
char pps_mode; char spare[8];
typedef struct
{
Page 22 of 24
9-GS+ -6063/BB
??GeoSwath Plus
Raw Data File Format & Broadcast Raw Data File Format + Command Specification
int ping_number; double ping_time;
unsigned
unsigned
unsigned
unsigned
unsigned
unsigned
unsigned
int previous_ping_position; int ping_size;
char navigation_number; char attitude_number;
unsigned
unsigned
unsigned
char heading_number;
char echosounder_number;
char miniSVS_number;
char aux1_number;
char aux2_number;
short ping_length;
char pulse_length;
char power;
char sidescan_gain;
int sample_number;
char side;
short navigation_strings_size; short attitude_strings_size; short heading_strings_size; short echosounder_strings_size; short miniSVS_strings_size; short aux1_strings_size;
short aux2_strings_size;
short ping_delay;
double pps_time;
char source;
short sample_interval;
unsigned
unsigned
        unsigned
}RAWPINGHEADER;
//////////////////////////////////////////////////////////////////////////// //
// function prototypes
// ///////////////////////////////////////////////////////////////////////////
BOOL read_raw_file_header(RAWFILEHEADER *, FILE *);
BOOL read_raw_ping_header(RAWPINGHEADER *, FILE *);
BOOL read_raw_ping_data(char *, int, FILE *);
void get_ping_data(RAA **, NAV **, MRU **, GYRO **, ECHO **, MINISVS **, double **, double **, RAWPINGHEADER, char *);
9-GS+ -6063/BB Page 23 of 24
??Raw Data File Format & Broadcast Raw Data File Format + Command Specification GeoSwath Plus
void get_raw_strings(char **, char **, char **, char **, char **, char **, char **, AUXDATA **, AUXDATA **, RAWPINGHEADER, char *);
BOOL write_raw_file_header(RAWFILEHEADER *, FILE *);
BOOL write_raw_ping_header(RAWPINGHEADER *, FILE *);
BOOL write_raw_ping_data(char *, int, FILE *);


/* Reson 7k Spreading Loss (record 7612) */
typedef struct s7kr_spreadingloss_struct
{
	s7k_header	header;
	float		spreadingloss;		/* dB (0 - 60) */
}
s7kr_spreadingloss;

/* internal data structure */
struct mbsys_geoswath_struct
	{
	/* Type of data record */
	int		kind;			/* MB-System record ID */
	int		type;			/* Reson record ID */
	int		sstype;			/* If type == R7KRECID_FSDWsidescan
							sstype: 0 = low frequency sidescan
							 	1 = high frequency sidescan */

	/* ping record id's */
	int		current_ping_number;
	int		read_volatilesettings;
	int		read_matchfilter;
	int		read_beamgeometry;
	int		read_remotecontrolsettings;
	int		read_bathymetry;
	int		read_backscatter;
	int		read_beam;
	int		read_verticaldepth;
	int		read_image;
	int		read_v2pingmotion;
	int		read_v2detectionsetup;
	int		read_v2beamformed;
	int		read_v2detection;
	int		read_v2rawdetection;
	int		read_v2snippet;

	/* MB-System time stamp */
	double		time_d;
	int		time_i[7];

	/* Reference point information (record 1000) */
	/*  Note: these offsets should be zero for submersible vehicles */
	s7kr_reference	reference;

	/* Sensor uncalibrated offset position information (record 1001) */
	s7kr_sensoruncal	sensoruncal;

	/* Sensor calibrated offset position information (record 1002) */
	s7kr_sensorcal	sensorcal;

	/* Position (record 1003) */
	s7kr_position	position;

	/* Custom attitude (record 1004) */
	s7kr_customattitude	customattitude;

	/* Tide (record 1005) */
	s7kr_tide	tide;

	/* Altitude (record 1006) */
	s7kr_altitude	altitude;

	/* Motion over ground (record 1007) */
	s7kr_motion	motion;

	/* Depth (record 1008) */
	s7kr_depth	depth;

	/* Sound velocity profile (record 1009) */
	s7kr_svp	svp;

	/* CTD (record 1010) */
	s7kr_ctd	ctd;

	/* Geodesy (record 1011) */
	s7kr_geodesy	geodesy;

	/* Roll pitch heave (record 1012) */
	s7kr_rollpitchheave	rollpitchheave;

	/* Heading (record 1013) */
	s7kr_heading	heading;

	/* Survey line (record 1014) */
	s7kr_surveyline	surveyline;

	/* Navigation (record 1015) */
	s7kr_navigation	navigation;

	/* Attitude (record 1016) */
	s7kr_attitude	attitude;

	/* Unknown record 1022 (record 1022) */
	s7kr_rec1022	rec1022;

	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsslo;

	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsshi;

	/* Edgetech FS-DW subbottom (record 3001) */
	s7kr_fsdwsb	fsdwsb;

	/* Bluefin data frames (record 3100) */
	s7kr_bluefin	bluefin;

	/* Reson 7k volatile sonar settings (record 7000) */
	s7kr_volatilesettings	volatilesettings;

	/* Reson 7k configuration (record 7001) */
	s7kr_configuration	configuration;

	/* Reson 7k match filter (record 7002) */
	s7kr_matchfilter	matchfilter;

	/* Reson 7k firmware and hardware configuration (record 7003) */
	s7kr_v2firmwarehardwareconfiguration	v2firmwarehardwareconfiguration;

	/* Reson 7k beam geometry (record 7004) */
	s7kr_beamgeometry	beamgeometry;

	/* Reson 7k calibration (record 7005) */
	s7kr_calibration	calibration;

	/* Reson 7k bathymetry (record 7006) */
	s7kr_bathymetry		bathymetry;

	/* Reson 7k backscatter imagery data (record 7007) */
	s7kr_backscatter	backscatter;

	/* Reson 7k beam data (record 7008) */
	s7kr_beam		beam;

	/* Reson 7k vertical depth (record 7009) */
	s7kr_verticaldepth	verticaldepth;

	/* Reson 7k image data (record 7011) */
	s7kr_image		image;

	/* Ping motion (record 7012) */
	s7kr_v2pingmotion	v2pingmotion;

	/* Detection setup (record 7017) */
	s7kr_v2detectionsetup	v2detectionsetup;

	/* Reson 7k beamformed magnitude and phase data (record 7018) */
	s7kr_v2beamformed	v2beamformed;

	/* Reson 7k BITE (record 7021) */
	s7kr_v2bite	v2bite;

	/* Reson 7k center version (record 7022) */
	s7kr_v27kcenterversion	v27kcenterversion;

	/* Reson 7k 8k wet end version (record 7023) */
	s7kr_v28kwetendversion	v28kwetendversion;

	/* Reson 7k version 2 detection (record 7026) */
	s7kr_v2detection	v2detection;

	/* Reson 7k version 2 raw detection (record 7027) */
	s7kr_v2rawdetection	v2rawdetection;

	/* Reson 7k version 2 snippet (record 7028) */
	s7kr_v2snippet	v2snippet;

	/* Reson 7k sonar installation parameters (record 7030) */
	s7kr_installation	installation;

	/* Reson 7k system event (record 7051) */
	s7kr_systemeventmessage	systemeventmessage;

	/* Reson 7k file header (record 7200) */
	s7kr_fileheader		fileheader;

	/* Reson 7k remote control sonar settings (record 7503) */
	s7kr_remotecontrolsettings	remotecontrolsettings;

	/* Reson 7k Reserved (well, unknown really...) (record 7504) */
	s7kr_reserved		reserved;

	/* Reson 7k Roll (record 7600) */
	s7kr_roll		roll;

	/* Reson 7k Pitch (record 7601) */
	s7kr_pitch		pitch;

	/* Reson 7k Sound Velocity (record 7610) */
	s7kr_soundvelocity	soundvelocity;

	/* Reson 7k Absorption Loss (record 7611) */
	s7kr_absorptionloss	absorptionloss;

	/* Reson 7k Spreading Loss (record 7612) */
	s7kr_spreadingloss	spreadingloss;

	};



/* 7K Macros */
int mbsys_geoswath_checkheader(s7k_header header);

/* system specific function prototypes */
int mbsys_geoswath_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_geoswath_survey_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error);
int mbsys_geoswath_attitude_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error);
int mbsys_geoswath_heading_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error);
int mbsys_geoswath_ssv_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error);
int mbsys_geoswath_tlt_alloc(int verbose,
			void *mbio_ptr, void *store_ptr,
			int *error);
int mbsys_geoswath_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_geoswath_zero_ss(int verbose, void *store_ptr, int *error);
int mbsys_geoswath_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_geoswath_pingnumber(int verbose, void *mbio_ptr,
			int *pingnumber, int *error);
int mbsys_geoswath_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_geoswath_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_geoswath_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_geoswath_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_geoswath_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
int mbsys_geoswath_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_geoswath_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_geoswath_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_geoswath_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_geoswath_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_geoswath_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_geoswath_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			void *segyheader_ptr,
			int *error);
int mbsys_geoswath_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
			int *sampleformat,
			int *kind,
			void *segyheader_ptr,
			float *segydata,
			int *error);
int mbsys_geoswath_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
			int kind,
			void *segyheader_ptr,
			float *segydata,
			int *error);
int mbsys_geoswath_ctd(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nctd, double *time_d,
			double *conductivity, double *temperature,
			double *depth, double *salinity, double *soundspeed, int *error);
int mbsys_geoswath_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nsamples, double *time_d,
			double *sensor1, double *sensor2, double *sensor3,
			double *sensor4, double *sensor5, double *sensor6,
			double *sensor7, double *sensor8, int *error);
int mbsys_geoswath_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_geoswath_checkheader(s7k_header header);
int mbsys_geoswath_makess(int verbose, void *mbio_ptr, void *store_ptr,
			int pixel_size_set, double *pixel_size,
			int swath_width_set, double *swath_width,
			int pixel_int,
			int *nss, double *ss,
			double *ssacrosstrack, double *ssalongtrack,
			int *error);
int mbsys_geoswath_print_header(int verbose,
			s7k_header *header,
			int *error);
int mbsys_geoswath_print_reference(int verbose,
			s7kr_reference *reference,
			int *error);
int mbsys_geoswath_print_sensoruncal(int verbose,
			s7kr_sensoruncal *sensoruncal,
			int *error);
int mbsys_geoswath_print_sensorcal(int verbose,
			s7kr_sensorcal *sensorcal,
			int *error);
int mbsys_geoswath_print_position(int verbose,
			s7kr_position *position,
			int *error);
int mbsys_geoswath_print_customattitude(int verbose,
			s7kr_customattitude *customattitude,
			int *error);
int mbsys_geoswath_print_tide(int verbose,
			s7kr_tide *tide,
			int *error);
int mbsys_geoswath_print_altitude(int verbose,
			s7kr_altitude *altitude,
			int *error);
int mbsys_geoswath_print_motion(int verbose,
			s7kr_motion *motion,
			int *error);
int mbsys_geoswath_print_depth(int verbose,
			s7kr_depth *depth,
			int *error);
int mbsys_geoswath_print_svp(int verbose,
			s7kr_svp *svp,
			int *error);
int mbsys_geoswath_print_ctd(int verbose,
			s7kr_ctd *ctd,
			int *error);
int mbsys_geoswath_print_geodesy(int verbose,
			s7kr_geodesy *geodesy,
			int *error);
int mbsys_geoswath_print_rollpitchheave(int verbose,
			s7kr_rollpitchheave *rollpitchheave,
			int *error);
int mbsys_geoswath_print_heading(int verbose,
			s7kr_heading *heading,
			int *error);
int mbsys_geoswath_print_surveyline(int verbose,
			s7kr_surveyline *surveyline,
			int *error);
int mbsys_geoswath_print_navigation(int verbose,
			s7kr_navigation *navigation,
			int *error);
int mbsys_geoswath_print_attitude(int verbose,
			s7kr_attitude *attitude,
			int *error);
int mbsys_geoswath_print_rec1022(int verbose,
			s7kr_rec1022 *rec1022,
			int *error);
int mbsys_geoswath_print_fsdwchannel(int verbose, int data_format,
			s7k_fsdwchannel *fsdwchannel,
			int *error);
int mbsys_geoswath_print_fsdwssheader(int verbose,
			s7k_fsdwssheader *fsdwssheader,
			int *error);
int mbsys_geoswath_print_fsdwsegyheader(int verbose,
			s7k_fsdwsegyheader *fsdwsegyheader,
			int *error);
int mbsys_geoswath_print_fsdwss(int verbose,
			s7kr_fsdwss *fsdwss,
			int *error);
int mbsys_geoswath_print_fsdwsb(int verbose,
			s7kr_fsdwsb *fsdwsb,
			int *error);
int mbsys_geoswath_print_bluefin(int verbose,
			s7kr_bluefin *bluefin,
			int *error);
int mbsys_geoswath_print_volatilesettings(int verbose,
			s7kr_volatilesettings *volatilesettings,
			int *error);
int mbsys_geoswath_print_device(int verbose,
			s7k_device *device,
			int *error);
int mbsys_geoswath_print_configuration(int verbose,
			s7kr_configuration *configuration,
			int *error);
int mbsys_geoswath_print_matchfilter(int verbose,
			s7kr_matchfilter *matchfilter,
			int *error);
int mbsys_geoswath_print_v2firmwarehardwareconfiguration(int verbose,
			s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration,
			int *error);
int mbsys_geoswath_print_beamgeometry(int verbose,
			s7kr_beamgeometry *beamgeometry,
			int *error);
int mbsys_geoswath_print_calibration(int verbose,
			s7kr_calibration *calibration,
			int *error);
int mbsys_geoswath_print_bathymetry(int verbose,
			s7kr_bathymetry *bathymetry,
			int *error);
int mbsys_geoswath_print_backscatter(int verbose,
			s7kr_backscatter *backscatter,
			int *error);
int mbsys_geoswath_print_beam(int verbose,
			s7kr_beam *beam,
			int *error);
int mbsys_geoswath_print_verticaldepth(int verbose,
			s7kr_verticaldepth *verticaldepth,
			int *error);
int mbsys_geoswath_print_image(int verbose,
			s7kr_image *image,
			int *error);
int mbsys_geoswath_print_v2pingmotion(int verbose,
			s7kr_v2pingmotion *v2pingmotion,
			int *error);
int mbsys_geoswath_print_v2detectionsetup(int verbose,
			s7kr_v2detectionsetup *v2detectionsetup,
			int *error);
int mbsys_geoswath_print_v2beamformed(int verbose,
			s7kr_v2beamformed *v2beamformed,
			int *error);
int mbsys_geoswath_print_v2bite(int verbose,
			s7kr_v2bite *v2bite,
			int *error);
int mbsys_geoswath_print_v27kcenterversion(int verbose,
			s7kr_v27kcenterversion *v27kcenterversion,
			int *error);
int mbsys_geoswath_print_v28kwetendversion(int verbose,
			s7kr_v28kwetendversion *v28kwetendversion,
			int *error);
int mbsys_geoswath_print_v2detection(int verbose,
			s7kr_v2detection *v2detection,
			int *error);
int mbsys_geoswath_print_v2rawdetection(int verbose,
			s7kr_v2rawdetection *v2rawdetection,
			int *error);
int mbsys_geoswath_print_v2snippet(int verbose,
			s7kr_v2snippet *v2snippet,
			int *error);
int mbsys_geoswath_print_installation(int verbose,
			s7kr_installation *installation,
			int *error);
int mbsys_geoswath_print_systemeventmessage(int verbose,
			s7kr_systemeventmessage *systemeventmessage,
			int *error);
int mbsys_geoswath_print_subsystem(int verbose,
			s7kr_subsystem *subsystem,
			int *error);
int mbsys_geoswath_print_fileheader(int verbose,
			s7kr_fileheader *fileheader,
			int *error);
int mbsys_geoswath_print_remotecontrolsettings(int verbose,
			s7kr_remotecontrolsettings *remotecontrolsettings,
			int *error);
int mbsys_geoswath_print_reserved(int verbose,
			s7kr_reserved *reserved,
			int *error);
int mbsys_geoswath_print_roll(int verbose,
			s7kr_roll *roll,
			int *error);
int mbsys_geoswath_print_pitch(int verbose,
			s7kr_pitch *pitch,
			int *error);
int mbsys_geoswath_print_soundvelocity(int verbose,
			s7kr_soundvelocity *soundvelocity,
			int *error);
int mbsys_geoswath_print_absorptionloss(int verbose,
			s7kr_absorptionloss *absorptionloss,
			int *error);
int mbsys_geoswath_print_spreadingloss(int verbose,
			s7kr_spreadingloss *spreadingloss,
			int *error);
