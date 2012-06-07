/*--------------------------------------------------------------------
 *    The MB-system:	mbr_xtfb1624.c	3/29/2011
 *	$Id$
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
 * mbr_xtfb1624.c contains the functions for reading and writing
 * sidescan data in the XTFSIS1624 format.
 * These functions include:
 *   mbr_alm_xtfb1624	- allocate read/write memory
 *   mbr_dem_xtfb1624	- deallocate read/write memory
 *   mbr_rt_xtfb1624	- read and translate data
 *   mbr_wt_xtfb1624	- translate and write data
 *
 * Author:	Jens Renken (MARUM/University of Bremen)
 * Date:	March 29, 2011
 *
 * Author:	D. W. Caress
 * Date:	2 May 2012 (when the code was brought into the MB-System archive as a read-only i/o module)
 *
 * $Log: $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_benthos.h"

/* turn on debug statements here */
/* #define MBR_XTFB1624_DEBUG 1 */

/* maximum number of beams and pixels */
#define	MBF_XTFB1624_MAXBEAMS		1
#define	MBF_XTFB1624_MAXRAWPIXELS	15360
#define	MBF_XTFB1624_COMMENT_LENGTH	200
#define	MBF_XTFB1624_MAXLINE		16384
#define	MBF_XTFB1624_FILEHEADERLEN	1024
#define XTF_MAGIC_NUMBER		0xFACE
#define XTF_DATA_SIDESCAN		0
#define XTF_DATA_ANNOTATION		1
#define XTF_DATA_BATHYMETRY		2
#define XTF_DATA_ATTITUDE		3
#define XTF_DATA_POSITION		100
#define XTF_HEADER_SONAR                     0 // sidescan and subbottom
#define XTF_HEADER_NOTES                     1 // notes - text annotation
#define XTF_HEADER_BATHY                     2 // bathymetry (Seabat, Odom)
#define XTF_HEADER_ATTITUDE                  3 // TSS or MRU attitude (pitch, roll, heave, yaw)
#define XTF_HEADER_FORWARD                   4 // forward-look sonar (polar display)
#define XTF_HEADER_ELAC                      5 // Elac multibeam
#define XTF_HEADER_RAW_SERIAL                6 // Raw data from serial port
#define XTF_HEADER_EMBED_HEAD                7 // Embedded header structure
#define XTF_HEADER_HIDDEN_SONAR              8 // hidden (non-displayable) ping
#define XTF_HEADER_SEAVIEW_ANGLES            9 // Bathymetry (angles) for Seaview
#define XTF_HEADER_SEAVIEW_DEPTHS           10 // Bathymetry from Seaview data (depths)
#define XTF_HEADER_HIGHSPEED_SENSOR         11 // used by Klein (Cliff Chase) 0=roll, 1=yaw
#define XTF_HEADER_ECHOSTRENGTH             12 // Elac EchoStrength (10 values)
#define XTF_HEADER_GEOREC                   13 // Used to store mosaic params
#define XTF_HEADER_K5000_BATHYMETRY         14 // Bathymetry data from the Klein 5000
#define XTF_HEADER_HIGHSPEED_SENSOR2        15 // High speed sensor from Klein 5000
#define XTF_HEADER_RAW_CUSTOM               199 // Raw Custom Header

struct mbf_xtfb1624_xtfchaninfo
	{
	char		TypeOfChannel;
	char		SubChannelNumber;
	unsigned short	CorrectionFlags;
	unsigned short	UniPolar;
	unsigned short	BytesPerSample;
	unsigned int	SamplesPerChannel;
	char		ChannelName[16];
	float		VoltScale;		/* maximum value (V) */
	float		Frequency;		/* Hz */
	float		HorizBeamAngle;		/* degrees */
	float		TiltAngle;
	float		BeamWidth;
	float		OffsetX;
	float		OffsetY;
	float		OffsetZ;
	float		OffsetYaw;
	float		OffsetPitch;
	float		OffsetRoll;
	char		ReservedArea[56];
	};

struct mbf_xtfb1624_xtffileheader
	{
	char		FileFormat;		/* Set to 123 (0x7B */
	char		SystemType;		/* Set to 1 */
	char		RecordingProgramName[8];
	char		RecordingProgramVersion[8];
	char		SonarName[16];
	unsigned short	SonarType;		/* 	0 = Reserved
							1 = JAMSTEC
							2 = Analog_c31
							3 = SIS1000
							4 = Analog_32chan
							5 = Klein2000
							6 = RWS
							7 = DF1000
							8 = SeaBat 9001
							9 = Klein595
							10 = EGG260
							11 = Sonatech_DDS
							12 = Echoscan
							13 = Elac Bottomchart 1180
							14 = Klein 5000
							15 = Reson SeaBat 8101
							16 = Imagenex model 858
							17 = USN SILOS with 3-channel analog
							18 = Sonatech super high res sidescan sonar
							19 = Delph AU32 Analog input (2 channel)
							20 = Generic sonar using the memory-mapped
								file interface
							21 = Simrad SM2000
							22 = Standard multibedia audio
							23 = Edgetech ACI card for 260 sonar throug PC31 card
							24 = Edgetech black box
							25 = Fugro deeptow
							26 = C&C Edgetech chirp conversion program
							27 = DTI SAS synthetic aperture processor (mmap file)
							*/
	char		NoteString[64];
	char		ThisFileName[64];
	unsigned short	NavUnits;		/* 0 = meters, 3 = degrees */
	unsigned short	NumberOfSonarChannels;	/* if <= 6 use 1024 byte header,
							if > 6 use 2048 byte header */
	unsigned short	NumberOfBathymetryChannels;
	unsigned short	Reserved1;
	unsigned short	Reserved2;
	unsigned short	Reserved3;
	unsigned short	Reserved4;
	unsigned short	Reserved5;
	unsigned short	Reserved6;
	char		ProjectionType[12];	/* not currently used */
	char		SpheroidType[10];	/* not currently used */
	int		NavigationLatency;	/* GPS_time_received - GPS_time_sent (msec) */
	float		OriginY;		/* not currently used */
	float		OriginX; 		/* not currently used */
	float		NavOffsetY;		/* Multibeam nav offset (m) */
	float		NavOffsetX;		/* Multibeam nav offset (m) */
	float		NavOffsetZ; 		/* Multibeam nav z offset (m) */
	float		NavOffsetYaw;		/* Multibeam heading offset (m) */
	float		MRUOffsetY;		/* Multibeam MRU y offset (m) */
	float		MRUOffsetX;		/* Multibeam MRU x offset (m) */
	float		MRUOffsetZ; 		/* Multibeam MRU z offset (m) */
	float		MRUOffsetYaw;		/* Multibeam MRU heading offset (m) */
	float		MRUOffsetPitch; 	/* Multibeam MRU pitch offset (degrees) */
	float		MRUOffsetRoll;		/* Multibeam MRU roll offset (degrees) */
	struct mbf_xtfb1624_xtfchaninfo chaninfo[6];
	};

struct mbf_xtfpacketheader
	{
	mb_u_char	MagicNumber[2];		/* 0xFACE */
	mb_u_char	HeaderType;		/* 0 = sonar ping, 1 = annotation, 2 = bathymetry, 3 = attitude */
	mb_u_char	SubChannelNumber;	/* which multibeam head */
	unsigned short	NumChansToFollow;	/* Number of beams to follow */
	unsigned short	Reserved1[2];
	unsigned int	NumBytesThisRecord;	/* total byte count including this header */
	};

struct mbf_xtfattitudeheader
	{
	struct mbf_xtfpacketheader packetheader;
	unsigned int	Reserved2[4];
	float		Pitch;
	float		Roll;
	float		Heave;
	float		Yaw;
	unsigned int	TimeTag;		/* time tag (msec) */
	float		Heading;
	char		Reserved3[10];
	};

struct mbf_xtfpingheader
	{
	struct mbf_xtfpacketheader packetheader;
	unsigned short	Year;
	mb_u_char	Month;
	mb_u_char	Day;
	mb_u_char	Hour;
	mb_u_char	Minute;
	mb_u_char	Second;
	mb_u_char	HSeconds;
	unsigned short	JulianDay;
	unsigned short	CurrentLineID;
	unsigned short	EventNumber;
	unsigned int	PingNumber;
	float		SoundVelocity;		/* Half sound speed (e.g. 750 m/sec instead of 1500 m/sec) */
	float		OceanTide;		/* (m) */
	unsigned int	Reserved2;
	float		ConductivityFreq;	/* Raw CTD conductivity frequency (Hz) */
	float		TemperatureFreq;	/* Raw CTD temperature frequency (Hz) */
	float		PressureFreq;		/* Raw CTD pressure frequency (Hz) */
	float		PressureTemp;		/* Raw CTD pressure temperature (deg C) */
	float		Conductivity;		/* computed CTD conductivity (siemens/m) */
	float		WaterTemperature;	/* computed CTD temperature (deg C) */
	float		Pressure;		/* computed CTD water pressure (psia) */
	float		ComputedSoundVelocity;	/* water sound velocity (m/sec) */
	float		MagX;			/* X-axis magnetometer (mGauss) */
	float		MagY;			/* Y-axis magnetometer (mGauss) */
	float		MagZ;			/* Z-axis magnetometer (mGauss) */
	float		AuxVal1;
	float		AuxVal2;
	float		AuxVal3;
	float		AuxVal4;
	float		AuxVal5;
	float		AuxVal6;
	float		SpeedLog;	/* towfish speed (kts) */
	float		Turbidity;	/* turbity (0-5V * 10000) */
	float		ShipSpeed;	/* ship speed (kts) */
	float		ShipGyro;	/* ship heading (deg) */
	double		ShipYcoordinate;/* ship latitude or northing */
	double		ShipXcoordinate;/* ship longitude or easting */
	short		ShipAltitude;	/* ship altitude (decimeters) */
	short		ShipDepth;	/* ship depth (decimeters) */
	mb_u_char	FixTimeHour;	/* last nav fix time (hour) */
	mb_u_char	FixTimeMinute;	/* last nav fix time (min) */
	mb_u_char	FixTimeSecond;	/* last nav fix time (sec) */
	char		Reserved4;
	float		SensorSpeed; 	/* towfish speed (kts) */
	float		KP;		/* kilometers pipe (km) */
	double		SensorYcoordinate; /* towfish latitude or northing */
	double		SensorXcoordinate; /* towfish longitude or easting */
	short		Reserved6;
	short		RangeToSensor;	/* slant range to towfish * 10 */
	short		BearingToSensor;/* bearing to towfish * 100 */
	short		CableOut;	/* cable out (m) */
	float		Layback;	/* distance from ship to sensor (m) */
	float		CableTension;	/* cable tension */
	float		SensorDepth;	/* towfish depth (m) */
	float		SensorPrimaryAltitude; /* towfish altitude (m) */
	float		SensorAuxAltitude; /* towfish altitude (m) */
	float		SensorPitch;	/* sensor pitch (deg) */
	float		SensorRoll;	/* sensor roll (deg) */
	float		SensorHeading;	/* sensor heading (deg) */
	float		Heave;		/* sensor heave (m) */
	float		Yaw;		/* sensor yaw (deg) */
	int		AttitudeTimeTag;/* time tag from MRU */
	float		DOT;		/* distance off track */
	char		ReservedSpace[20];
	};

struct mbf_xtfpingchanheader
        {
        unsigned short	ChannelNumber;	/* Typically,  */
					/* 0=port (low frequency) */
					/* 1=stbd (low frequency) */
					/* 2=port (high frequency) */
					/* 3=stbd (high frequency) */
        unsigned short	DownsampleMethod;   /* 2=MAX, 4=RMS */
        float		SlantRange;	/* Slant range of the data in meters */
        float		GroundRange;	/* Ground range of the data in meters */
                                             /*   (SlantRange^2 - Altitude^2) */
        float		TimeDelay;	/* Amount of time (in seconds) to the start of recorded data */
                                             /*   almost always 0.0 */
        float		TimeDuration;	/* Amount of time (in seconds) recorded */
        float		SecondsPerPing;	/* Amount of time (in seconds) from ping to ping */

        unsigned short	ProcessingFlags;/* 4=TVG, 8=BAC&GAC, 16=Filter, etc... */
                                             /*   almost always 0 */
        unsigned short	Frequency;	/* Center transmit frequency for this channel. */
                                             /*   when non-zero, replaces value found in file */
                                             /*   header CHANINFO struct ChanInfo->SamplesPerChannel. */
                                             /*   This allows samples per channel to change on the fly. */

        unsigned short	InitialGainCode;/* Settings as transmitted by sonar */
        unsigned short	GainCode;
        unsigned short	BandWidth;

        /* Contact information - updated when contacts are saved through Target.exe */
        unsigned int		ContactNumber;
        unsigned short	ContactClassification;
        mb_u_char		ContactSubNumber;
        mb_u_char		ContactType;


        unsigned int		NumSamples;	/* Number of samples that will follow this structure.  The */
                                             /* number of bytes will be this value multipied by the */
                                             /* number of bytes per sample (given in the file header) */

        unsigned short	Reserved;	/* Obsolete. */
        float		ContactTimeOffTrack;	/* Time off track to this contact (stored in milliseconds) */
        mb_u_char	ContactCloseNumber;
        mb_u_char	Reserved2;

        float		FixedVSOP;	/* Fixed along-track size of each ping, stored in cm. */
                                             /*  on multibeam system with zero beam spread, this value */
                                             /*  needs to be filled in to prevent Isis from calculating */
                                             /*  along-track ground coverage based on beam spread and  */
                                             /*  speed over ground.  In order for */
                                             /* Target to use this number, "223" */
                                             /* or later must be set in */
                                             /* XTFFILEHEADER.RecordingProgramVersion */

        mb_u_char	ReservedSpace[6];   /* reserved for future expansion */
};

struct mbf_xftnotesheader {
	struct mbf_xtfpacketheader packetheader;
	//
	// Date and time of the annotation
	//
	unsigned short	Year;
	mb_u_char	Month;
	mb_u_char	Day;
	mb_u_char	Hour;
	mb_u_char	Minute;
	mb_u_char	Second;
	mb_u_char	HSeconds;

	char  NotesText[256-56];
};

struct mbf_xtfrawcustomheader {
	mb_u_char		MagicNumber[2];	/* 0xFACE */
	mb_u_char		HeaderType;		/* 199 = XTFRAWCUSTOMHEADER*/
	mb_u_char		ManufacturerID;	/* 1 = Benthos */
	unsigned short 	SonarID;		/* 1624 = 1624	*/
	unsigned short 	PacketID;	/* TBD */
	unsigned short 	Reserved1[1];
	unsigned int 	NumBytesThisRecord; // Total byte count for this update

	unsigned short	Year;
	mb_u_char	Month;
	mb_u_char	Day;
	mb_u_char	Hour;
	mb_u_char	Minute;
	mb_u_char	Second;
	mb_u_char	HSeconds;

	unsigned short JulianDay;
	unsigned short Reserved2[2];
	unsigned int   PingNumber;
	unsigned int   TimeTag;
	unsigned int   NumCustomerBytes;
	mb_u_char Reserved3[24]; 		// Padding to make the structure 64 bytes
};

struct mbf_xtfb1624_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Benthos sonar */

	/* xtf file header */
	struct mbf_xtfb1624_xtffileheader fileheader;

	/* xtf attitude record */
	struct mbf_xtfattitudeheader attitudeheader;

	/* xtf raw custom record */
	struct mbf_xtfrawcustomheader rawcustomheader;

	/* xtf Benthos sidescan ping record */
	struct mbf_xtfpingheader pingheader;
	struct mbf_xtfpingchanheader pingchanportheader;
	unsigned short ssrawport[MBF_XTFB1624_MAXRAWPIXELS];
	struct mbf_xtfpingchanheader pingchanstbdheader;
	unsigned short ssrawstbd[MBF_XTFB1624_MAXRAWPIXELS];

	/* comment */
	char comment[MBF_XTFB1624_COMMENT_LENGTH];
	};

/* essential function prototypes */
int mbr_register_xtfb1624(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_xtfb1624(int verbose,
			int *system,
			int *beams_bath_max,
			int *beams_amp_max,
			int *pixels_ss_max,
			char *format_name,
			char *system_name,
			char *format_description,
			int *numfile,
			int *filetype,
			int *variable_beams,
			int *traveltime,
			int *beam_flagging,
			int *nav_source,
			int *heading_source,
			int *vru_source,
			int *svp_source,
			double *beamwidth_xtrack,
			double *beamwidth_ltrack,
			int *error);
int mbr_alm_xtfb1624(int verbose, void *mbio_ptr, int *error);
int mbr_dem_xtfb1624(int verbose, void *mbio_ptr, int *error);
int mbr_zero_xtfb1624(int verbose, char *data_ptr, int *error);
int mbr_rt_xtfb1624(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_xtfb1624(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_xtfb1624_rd_data(int verbose, void *mbio_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_xtfb1624(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_xtfb1624";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_xtfb1624(verbose,
			&mb_io_ptr->system,
			&mb_io_ptr->beams_bath_max,
			&mb_io_ptr->beams_amp_max,
			&mb_io_ptr->pixels_ss_max,
			mb_io_ptr->format_name,
			mb_io_ptr->system_name,
			mb_io_ptr->format_description,
			&mb_io_ptr->numfile,
			&mb_io_ptr->filetype,
			&mb_io_ptr->variable_beams,
			&mb_io_ptr->traveltime,
			&mb_io_ptr->beam_flagging,
			&mb_io_ptr->nav_source,
			&mb_io_ptr->heading_source,
			&mb_io_ptr->vru_source,
			&mb_io_ptr->svp_source,
			&mb_io_ptr->beamwidth_xtrack,
			&mb_io_ptr->beamwidth_ltrack,
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_xtfb1624;
	mb_io_ptr->mb_io_format_free = &mbr_dem_xtfb1624;
	mb_io_ptr->mb_io_store_alloc = &mbsys_benthos_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_benthos_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_xtfb1624;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_xtfb1624;
	mb_io_ptr->mb_io_dimensions = &mbsys_benthos_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_benthos_extract;
	mb_io_ptr->mb_io_insert = &mbsys_benthos_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_benthos_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_benthos_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_benthos_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;		// &mbsys_benthos_extract_svp;
	mb_io_ptr->mb_io_insert_svp = NULL; 		// &mbsys_benthos_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_benthos_ttimes;
	mb_io_ptr->mb_io_copyrecord = &mbsys_benthos_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %lu\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %lu\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %lu\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %lu\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %lu\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %lu\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %lu\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %lu\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %lu\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %lu\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %lu\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %lu\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %lu\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %lu\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_xtfb1624(int verbose,
			int *system,
			int *beams_bath_max,
			int *beams_amp_max,
			int *pixels_ss_max,
			char *format_name,
			char *system_name,
			char *format_description,
			int *numfile,
			int *filetype,
			int *variable_beams,
			int *traveltime,
			int *beam_flagging,
			int *nav_source,
			int *heading_source,
			int *vru_source,
			int *svp_source,
			double *beamwidth_xtrack,
			double *beamwidth_ltrack,
			int *error)
{
	char	*function_name = "mbr_info_xtfb1624";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_BENTHOS;
	*beams_bath_max = MBSYS_BENTHOS_MAXBEAMS;
	*beams_amp_max = MBSYS_BENTHOS_MAXBEAMS;
	*pixels_ss_max = MBSYS_BENTHOS_MAXPIXELS;
	strncpy(format_name, "XTFB1624", MB_NAME_LENGTH);
	strncpy(system_name, "BENTHOS", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_XTFB1624\nInformal Description: XTF format Benthos Sidescan SIS1624\nAttributes:           variable pixels, dual frequency sidescan and subbottom,\n                      xtf variant, single files,\n                      low frequency sidescan returned as\n                      survey data, Benthos. \n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_xtfb1624(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_xtfb1624";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*fileheaderread;
	double	*pixel_size;
	double	*swath_width;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_xtfb1624_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_benthos_struct),
				&mb_io_ptr->store_data,error);

	/* set saved flags */
	fileheaderread = (int *) &(mb_io_ptr->save1);
	pixel_size = &mb_io_ptr->saved1;
	swath_width = &mb_io_ptr->saved2;
	*fileheaderread = MB_NO;
	*pixel_size = 0.0;
	*swath_width = 0.0;

	/* initialize everything to zeros */
	mbr_zero_xtfb1624(verbose,mb_io_ptr->raw_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_xtfb1624(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_xtfb1624";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_xtfb1624(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_xtfb1624";
	int	status = MB_SUCCESS;
	struct mbf_xtfb1624_struct *data;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %lu\n",(size_t)data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_xtfb1624_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_BENTHOS_UNKNOWN;
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_xtfb1624(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_xtfb1624";
	int	status = MB_SUCCESS;

	struct mb_io_struct *mb_io_ptr;
	struct mbf_xtfb1624_struct *data;
	struct mbsys_benthos_struct *store;
	int	nchan;
	int	time_i[7];
	double	time_d, ntime_d, dtime;
	double	*pixel_size, *swath_width;
	int	badtime;
	double	lon, lat;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_xtfb1624_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_benthos_struct *) store_ptr;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;

	/* read next data from file */
	status = mbr_xtfb1624_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* handle navigation fix delay */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		/* get ping time */
		time_i[0] = data->pingheader.Year;
		time_i[1] = data->pingheader.Month;
		time_i[2] = data->pingheader.Day;
		time_i[3] = data->pingheader.Hour;
		time_i[4] = data->pingheader.Minute;
		time_i[5] = data->pingheader.Second;
		time_i[6] = 10000 * data->pingheader.HSeconds;
		mb_get_time(verbose, time_i, &time_d);

		/* do check on time here - we sometimes get a bad fix */
		badtime = MB_NO;
		if (time_i[0] < 1970 && time_i[0] > 2100 ) badtime = MB_YES;
		if (time_i[1] < 0 && time_i[1] > 12) badtime = MB_YES;
		if (time_i[2] < 0 && time_i[2] > 31 ) badtime = MB_YES;
		if (badtime == MB_YES)
			{
			if (verbose > 0)
				fprintf(stderr," Bad time from XTF in ping header\n");
			data->kind = MB_DATA_NONE;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}

		/* get nav time */
		ntime_d = time_d;
		/* Check if fixtime is valid */
		if (data->pingheader.FixTimeHour || data->pingheader.FixTimeMinute
										 || data->pingheader.FixTimeSecond) {
			dtime = 3600.0 * (data->pingheader.FixTimeHour - data->pingheader.Hour)
				+ 60.0 * (data->pingheader.FixTimeMinute - data->pingheader.Minute)
				+ data->pingheader.FixTimeSecond - data->pingheader.Second
				- 0.01 * data->pingheader.HSeconds;
			if (data->pingheader.FixTimeHour - data->pingheader.Hour > 1)
				dtime -= 3600.0 * 24;
			ntime_d = time_d + dtime;
		}
		/* check for use of projected coordinates
			XTF allows projected coordinates like UTM but the format spec
			lists the projection specification values as unused!
			Assume UTM zone 1N as we have to assume something */
		if (mb_io_ptr->projection_initialized == MB_YES)
			{
			mb_proj_inverse(verbose, mb_io_ptr->pjptr,
							data->pingheader.SensorXcoordinate,
							data->pingheader.SensorYcoordinate,
							&lon, &lat,
							error);
			}
		else
			{
			lon = data->pingheader.SensorXcoordinate;
			lat = data->pingheader.SensorYcoordinate;
			}

		/* add latest fix to list */
		mb_navint_add(verbose, mbio_ptr, ntime_d, lon, lat, error);
		}

	/* translate values to benthos data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* type of sonar */
		store->sonar = data->sonar;			/* Type of Reson sonar */

		/* parameter info */
		nchan = data->fileheader.NumberOfSonarChannels
			+ data->fileheader.NumberOfBathymetryChannels;
		for (i=0;i<nchan;i++)
			{
			if (data->fileheader.chaninfo[i].TypeOfChannel == 3)
			    {
			    store->MBOffsetX = data->fileheader.chaninfo[i].OffsetX;
			    store->MBOffsetY = data->fileheader.chaninfo[i].OffsetY;
			    store->MBOffsetZ = data->fileheader.chaninfo[i].OffsetZ;
			    }
			}
		store->NavLatency = data->fileheader.NavigationLatency;		/* GPS_time_received - GPS_time_sent (sec) */
		store->NavOffsetY = data->fileheader.NavOffsetY;		/* Nav offset (m) */
		store->NavOffsetX = data->fileheader.NavOffsetX;		/* Nav offset (m) */
		store->NavOffsetZ = data->fileheader.NavOffsetZ; 		/* Nav z offset (m) */
		store->NavOffsetYaw = data->fileheader.NavOffsetZ;		/* Heading offset (m) */
		store->MRUOffsetY = data->fileheader.MRUOffsetY;		/* Multibeam MRU y offset (m) */
		store->MRUOffsetX = data->fileheader.MRUOffsetX;		/* Multibeam MRU x offset (m) */
		store->MRUOffsetZ = data->fileheader.MRUOffsetZ; 		/* Multibeam MRU z offset (m) */
		store->MRUOffsetPitch = data->fileheader.MRUOffsetPitch; 		/* Multibeam MRU pitch offset (degrees) */
		store->MRUOffsetRoll = data->fileheader.MRUOffsetRoll;		/* Multibeam MRU roll offset (degrees) */

		/* attitude data */
		store->att_timetag = data->pingheader.AttitudeTimeTag;
		store->att_heading = data->pingheader.SensorHeading;
		store->att_heave = data->pingheader.Heave;
		store->att_roll = data->pingheader.SensorRoll;
		store->att_pitch = data->pingheader.SensorPitch;

		/* comment */
		for (i=0;i<MBSYS_BENTHOS_COMMENT_LENGTH;i++)
			store->comment[i] = data->comment[i];

		/* survey data */
		time_i[0] = data->pingheader.Year;
		time_i[1] = data->pingheader.Month;
		time_i[2] = data->pingheader.Day;
		time_i[3] = data->pingheader.Hour;
		time_i[4] = data->pingheader.Minute;
		time_i[5] = data->pingheader.Second;
		time_i[6] = 10000 * data->pingheader.HSeconds;
		mb_get_time(verbose, time_i,  &(store->png_time_d));
		store->png_time_d -= store->png_latency;
		store->png_longitude = data->pingheader.SensorXcoordinate;
		store->png_latitude = data->pingheader.SensorYcoordinate;
		store->png_speed = data->pingheader.SensorSpeed;

		/* interpolate attitude if possible */
		if (mb_io_ptr->nattitude > 1)
		    {
#ifdef JRBENTH
                    /* time tag is on receive;  average reception is closer
		    	to the midpoint of the two way travel time
		        but will vary on beam angle and water depth
		        set the receive time delay to the average
			( 0 to 60 deg)  two way travel time for a seabed
		        located at 80% of the maximum range
		        Old code:
		    timetag = 0.001 * data->bathheader.AttitudeTimeTag
				    - store->png_latency
				    + 2.0 * ((double)data->reson8100rit.range_set)
					    / ((double)data->reson8100rit.velocity); */
		    timetag = 0.001 * data->pingheader.AttitudeTimeTag
				    - store->png_latency
				    + 1.4*((double)data->reson8100rit.range_set)
					    / ((double)data->reson8100rit.velocity);
		    mb_attint_interp(verbose, mbio_ptr, timetag,
			    &(store->png_heave), &(store->png_roll),
			    &(store->png_pitch), error);
		    mb_hedint_interp(verbose, mbio_ptr, timetag,
			    &(store->png_heading), error);
#ifdef MBR_XTFB1624_DEBUG
			fprintf(stderr, "roll: %d %f %f %f %f   latency:%f time:%f %f roll:%f\n",
			mb_io_ptr->nattitude,
			mb_io_ptr->attitude_time_d[0],
			mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1],
			mb_io_ptr->attitude_roll[0],
			mb_io_ptr->attitude_roll[mb_io_ptr->nattitude-1],
			store->png_latency, (double)(0.001 * data->pingheader.AttitudeTimeTag),
			timetag, store->png_roll);
#endif
#endif
		    }
		else
		    {
		    store->png_roll = data->pingheader.SensorRoll;
		    store->png_pitch = data->pingheader.SensorPitch;
		    store->png_heading = data->pingheader.SensorHeading;
		    store->png_heave = data->pingheader.Heave;
		    }

		/* interpolate nav if possible */
		if (mb_io_ptr->nfix > 0)
			{
			mb_navint_interp(verbose, mbio_ptr, store->png_time_d, store->png_heading, 0.0,
			    &(store->png_longitude), &(store->png_latitude), &(store->png_speed), error);

			/* now deal with odd case where original nav is in eastings and northings
				- since the projection is initialized, it will be applied when data
				are extracted using mb_extract(), mb_extract_nav(), etc., so we have
				to reproject the lon lat values to eastings northings for now */
			if (mb_io_ptr->projection_initialized == MB_YES)
				{
				mb_proj_forward(verbose, mb_io_ptr->pjptr,
							store->png_longitude, store->png_latitude,
							&(store->png_longitude), &(store->png_latitude),
							error);
				}
			}

		store->png_rtsv = data->pingheader.SoundVelocity;
		if (data->pingheader.ComputedSoundVelocity > 1000)
			store->png_computedsv = data->pingheader.ComputedSoundVelocity;
		else
			store->png_computedsv = data->pingheader.SoundVelocity * 2.0;
		store->png_pressure = data->pingheader.Pressure;
		store->png_depth = data->pingheader.SensorDepth;

#ifdef JRBENTH
		store->ping_number = data->reson8100rit.ping_number;			/* sequential ping number from sonar startup/reset */
		store->sonar_id = data->reson8100rit.sonar_id;				/* least significant four bytes of Ethernet address */
		store->sonar_model = data->reson8100rit.sonar_model;			/* coded model number of sonar */
		store->frequency = data->reson8100rit.frequency;			/* sonar frequency in KHz */
		store->velocity = data->reson8100rit.velocity;         			/* programmed sound velocity (LSB = 1 m/sec) */
		store->sample_rate = data->reson8100rit.sample_rate;      		/* A/D sample rate (samples per second) */
		store->ping_rate = data->reson8100rit.ping_rate;        		/* Ping rate (pings per second * 1000) */
		store->range_set = data->reson8100rit.range_set;        		/* range setting for SeaBat (meters ) */
		store->power = data->reson8100rit.power;            			/* power setting for SeaBat  	 */
											/* bits	0-4 -	power (0 - 8) */
		store->gain = data->reson8100rit.gain;             			/* gain setting for SeaBat */
											/* bits	0-6 -	gain (1 - 45) */
											/* bit 	14	(0 = fixed, 1 = tvg) */
											/* bit	15	(0 = manual, 1 = auto) */
		store->pulse_width = data->reson8100rit.pulse_width;      		/* transmit pulse width (microseconds) */
		store->tvg_spread = data->reson8100rit.tvg_spread;			/* spreading coefficient for tvg * 4  */
											/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
		store->tvg_absorp = data->reson8100rit.tvg_absorp;			/* absorption coefficient for tvg */
		store->projector_type = data->reson8100rit.projector_type;      	/* bits 0-4 = projector type */
							/* 0 = stick projector */
							/* 1 = array face */
							/* 2 = ER projector */
							/* bit 7 - pitch steering (1=enabled, 0=disabled) */
		store->projector_beam_width = data->reson8100rit.projector_beam_width;	/* along track transmit beam width (degrees * 10) */
		store->beam_width_num = data->reson8100rit.beam_width_num;   	/* cross track receive beam width numerator */
		store->beam_width_denom = data->reson8100rit.beam_width_denom; 	/* cross track receive beam width denominator */
							/* beam width degrees = numerator / denominator */
		store->projector_angle = data->reson8100rit.projector_angle;		/* projector pitch steering angle (degrees * 100) */
		store->min_range = data->reson8100rit.min_range;		/* sonar filter settings */
		store->max_range = data->reson8100rit.max_range;
		store->min_depth = data->reson8100rit.min_depth;
		store->max_depth = data->reson8100rit.max_depth;
		store->filters_active = data->reson8100rit.filters_active;		/* range/depth filters active  */
							/* bit 0 - range filter (0 = off, 1 = active) */
							/* bit 1 - depth filter (0 = off, 1 = active) */
		store->temperature = data->reson8100rit.temperature;		/* temperature at sonar head (deg C * 10) */
		store->beam_count = data->reson8100rit.beam_count;       		/* number of sets of beam data in packet */
		for (i=0;i<store->beam_count;i++)
			store->range[i] = data->reson8100rit.range[i]; 		/* range for beam where n = Beam Count */
							/* range units = sample cells * 4 */
		for (i=0;i<store->beam_count/2+1;i++)
			store->quality[i] = data->reson8100rit.quality[i];   		/* packed quality array (two 4 bit values/char) */
							/* cnt = n/2 if beam count even, n/2+1 if odd */
							/* cnt then rounded up to next even number */
							/* e.g. if beam count=101, cnt=52  */
							/* unused trailing quality values set to zero */
							/* bit 0 - brightness test (0=failed, 1=passed) */
							/* bit 1 - colinearity test (0=failed, 1=passed) */
							/* bit 2 - amplitude bottom detect used */
							/* bit 3 - phase bottom detect used */
							/* bottom detect can be amplitude, phase or both */
		intensity_max = 0;
		for (i=0;i<store->beam_count;i++)
			{
			store->intensity[i] = data->reson8100rit.intensity[i];   		/* intensities at bottom detect  */
			intensity_max = MAX(intensity_max, (int)store->intensity[i]);
			}

		store->beams_bath = data->reson8100rit.beam_count;
		if (intensity_max > 0)
			store->beams_amp = store->beams_bath;
		else
			store->beams_amp = 0;

		/* ttscale in seconds per range count ( 4 counts per time interval) */
		ttscale = 0.25 / store->sample_rate;
		icenter = store->beams_bath / 2;
		angscale = ((double)store->beam_width_num)
			/ ((double)store->beam_width_denom);
		for (i=0;i<store->beams_bath;i++)
			{
			/* get beamflag */
			if (i % 2 == 0)
				quality = ((store->quality[i/2]) & 15) & 3;
			else
				quality = ((store->quality[i/2] >> 4) & 15) & 3;
			if (quality == 0)
				store->beamflag[i] = MB_FLAG_NULL;
			else if (quality < 3)
				store->beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
			else
				store->beamflag[i] = MB_FLAG_NONE;

			if (store->beamflag[i] == MB_FLAG_NULL)
				{
				store->bath[i] = 0.0;		/* bathymetry (m) */
				store->bath_acrosstrack[i] = 0.0;/* acrosstrack distance (m) */
				store->bath_alongtrack[i] = 0.0;	/* alongtrack distance (m) */
				}
			else
				{
				angle = 90.0 + (icenter - i) * angscale + store->png_roll;
				mb_rollpitch_to_takeoff(
					verbose,
					store->png_pitch, angle,
					&theta, &phi,
					error);
				rr = 0.5 * store->velocity * ttscale * store->range[i];
				xx = rr * sin(DTR * theta);
				zz = rr * cos(DTR * theta);
				store->bath_acrosstrack[i]
					= xx * cos(DTR * phi);
				store->bath_alongtrack[i]
					= xx * sin(DTR * phi);
				store->bath[i] = zz - store->png_heave
						+ store->MBOffsetZ;
/*if (i==store->beams_bath/2 && timetag > 1.0)
fprintf(stderr,"%f %f %f %f %f\n",timetag,zz,store->png_heave,lever_z,store->bath[i]);*/
				}
			}
		gain_correction = 2.2 * (store->gain & 63) + 6 * store->power;
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = (double)(40.0 * log10(store->intensity[i])- gain_correction);
			}
#endif

		store->beams_bath = 1;
		store->bath[0] = data->pingheader.SensorPrimaryAltitude;		/* bathymetry (m) */

		store->ssrawtimedelay = data->pingchanportheader.TimeDelay;
		store->ssrawtimeduration = data->pingchanportheader.TimeDuration;
		store->ssrawbottompick = data->pingheader.SensorPrimaryAltitude
					    / data->pingheader.SoundVelocity;

		store->ssrawslantrange = data->pingchanportheader.SlantRange;
		store->ssrawgroundrange = data->pingchanportheader.GroundRange;
		store->ssfrequency = data->pingchanportheader.Frequency;

		store->ssportinitgain = data->pingchanportheader.InitialGainCode;
		store->ssstbdinitgain = data->pingchanstbdheader.InitialGainCode;
		store->ssportgain = data->pingchanportheader.GainCode;
		store->ssstbdgain = data->pingchanstbdheader.GainCode;

		store->ssrawportsamples = data->pingchanportheader.NumSamples;
		store->ssrawstbdsamples = data->pingchanstbdheader.NumSamples;
		for (i=0;i<store->ssrawportsamples;i++)
		    store->ssrawport[i] = data->ssrawport[ /* store->ssrawportsamples - 1 -  */ i];
		for (i=0;i<store->ssrawstbdsamples;i++)
		    store->ssrawstbd[i] = data->ssrawstbd[i];

		/* generate processed sidescan */
		store->pixel_size = 0.0;
		store->pixels_ss = store->ssrawportsamples + store->ssrawstbdsamples;
		status = mbsys_benthos_makess(verbose,
				mbio_ptr, store_ptr,
				MB_NO, pixel_size,
				MB_NO, swath_width,
				error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */

	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_xtfb1624(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_xtfb1624";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set error as this is a read only format */
	status = MB_FAILURE;
	*error = MB_ERROR_WRITE_FAIL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_xtfb1624_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_xtfb1624_rd_data";
	int	status = MB_SUCCESS;

	struct mb_io_struct *mb_io_ptr;
	struct mbf_xtfb1624_struct *data;
	char	line[MBF_XTFB1624_MAXLINE];
	int	*fileheaderread;
	struct mbf_xtfb1624_xtffileheader *fileheader;
	struct mbf_xtfpacketheader packetheader;
	struct mbf_xtfattitudeheader *attitudeheader;
	struct mbf_xtfrawcustomheader *rawcustomheader;
	struct mbf_xtfpingheader *pingheader;
	struct mbf_xtfpingchanheader *pingchanportheader;
	struct mbf_xtfpingchanheader *pingchanstbdheader;
	int	index;
	int	ichan;
	int	done, found;
	int	read_len, read_bytes;
	int	skip;
	mb_u_char *mb_u_char_ptr;
	double	timetag, heave, roll, pitch, heading;
	int	utm_zone;
	char	projection[MB_NAME_LENGTH];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set saved flags */
	fileheaderread = (int *) &(mb_io_ptr->save1);
	/* get pointer to raw data structure */
	data = (struct mbf_xtfb1624_struct *) mb_io_ptr->raw_data;
	fileheader = (struct mbf_xtfb1624_xtffileheader *) &(data->fileheader);
	attitudeheader = (struct mbf_xtfattitudeheader *) &(data->attitudeheader);
	rawcustomheader = (struct mbf_xtfrawcustomheader *) &(data->rawcustomheader);
	pingheader = (struct mbf_xtfpingheader *) &(data->pingheader);
	pingchanportheader = (struct mbf_xtfpingchanheader *) &(data->pingchanportheader);
	pingchanstbdheader = (struct mbf_xtfpingchanheader *) &(data->pingchanstbdheader);

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* read file header if required */
	if (*fileheaderread == MB_NO)
	    {
	    read_len = fread(line,1,MBF_XTFB1624_FILEHEADERLEN,mb_io_ptr->mbfp);
	    if (read_len == MBF_XTFB1624_FILEHEADERLEN)
		{
		/* extract data from buffer */
		*fileheaderread = MB_YES;
		status = MB_SUCCESS;
		index = 0;
		fileheader->FileFormat = line[index];
		index++;
		fileheader->SystemType = line[index];
		index++;
		for (i=0;i<8;i++)
			fileheader->RecordingProgramName[i] = line[index+i];
		index += 8;
		for (i=0;i<8;i++)
			fileheader->RecordingProgramVersion[i] = line[index+i];
		index += 8;
		for (i=0;i<16;i++)
			fileheader->SonarName[i] = line[index+i];
		index += 16;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->SonarType));
		index += 2;
		for (i=0;i<64;i++)
			fileheader->NoteString[i] = line[index+i];
		index += 64;
		for (i=0;i<64;i++)
			fileheader->ThisFileName[i] = line[index+i];
		index += 64;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NavUnits));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NumberOfSonarChannels));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NumberOfBathymetryChannels));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved1));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved2));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved3));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved4));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved5));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved6));
		index += 2;
		for (i=0;i<12;i++)
			fileheader->ProjectionType[i] = line[index+i];
		index += 12;
		for (i=0;i<10;i++)
			fileheader->SpheroidType[i] = line[index+i];
		index += 10;
		mb_get_binary_int(MB_YES, &line[index], (int *)&(fileheader->NavigationLatency));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->OriginY));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->OriginX));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetY));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetX));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetZ));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetYaw));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetY));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetX));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetZ));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetYaw));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetPitch));
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetRoll));
		index += 4;
		for (ichan=0;ichan<6;ichan++)
			{
			fileheader->chaninfo[ichan].TypeOfChannel = line[index];
			index++;
			fileheader->chaninfo[ichan].SubChannelNumber = line[index];
			index++;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].CorrectionFlags));
			index += 2;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].UniPolar));
			index += 2;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].BytesPerSample));
			index += 2;
			mb_get_binary_int(MB_YES, &line[index], (int *)&(fileheader->chaninfo[ichan].SamplesPerChannel));
			index += 4;
			for (i=0;i<16;i++)
				fileheader->chaninfo[ichan].ChannelName[i] = line[index+i];
			index += 16;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].VoltScale));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].Frequency));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].HorizBeamAngle));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].TiltAngle));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].BeamWidth));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetX));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetY));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetZ));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetYaw));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetPitch));
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetRoll));
			index += 4;
			for (i=0;i<56;i++)
				fileheader->chaninfo[ichan].ReservedArea[i] = line[index+i];
			index += 56;
			}

		/* if NavUnits indicates use of projected coordinates (the format spec
			indicates the projection parameters are unused!) assume UTM zone 1N
			and set up the projection */
		if (fileheader->NavUnits == 0 && mb_io_ptr->projection_initialized == MB_NO)
			{
			/* initialize UTM projection */
			utm_zone = (int)(((RTD * 0.0 + 183.0)
					/ 6.0) + 0.5);
			sprintf(projection,"UTM%2.2dN", utm_zone);
			mb_proj_init(verbose, projection, &(mb_io_ptr->pjptr), error);
			mb_io_ptr->projection_initialized = MB_YES;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       FileFormat:                 %d\n",fileheader->FileFormat);
			fprintf(stderr,"dbg5       SystemType:                 %d\n",fileheader->SystemType);
			fprintf(stderr,"dbg5       RecordingProgramName:       %s\n",fileheader->RecordingProgramName);
			fprintf(stderr,"dbg5       RecordingProgramVersion:    %s\n",fileheader->RecordingProgramVersion);
			fprintf(stderr,"dbg5       SonarName:                  %s\n",fileheader->SonarName);
			fprintf(stderr,"dbg5       SonarType:                  %d\n",fileheader->SonarType);
			fprintf(stderr,"dbg5       NoteString:                 %s\n",fileheader->NoteString);
			fprintf(stderr,"dbg5       ThisFileName:               %s\n",fileheader->ThisFileName);
			fprintf(stderr,"dbg5       NavUnits:                   %d\n",fileheader->NavUnits);
			fprintf(stderr,"dbg5       NumberOfSonarChannels:      %d\n",fileheader->NumberOfSonarChannels);
			fprintf(stderr,"dbg5       NumberOfBathymetryChannels: %d\n",fileheader->NumberOfBathymetryChannels);
			fprintf(stderr,"dbg5       Reserved1:                  %d\n",fileheader->Reserved1);
			fprintf(stderr,"dbg5       Reserved2:                  %d\n",fileheader->Reserved2);
			fprintf(stderr,"dbg5       Reserved3:                  %d\n",fileheader->Reserved3);
			fprintf(stderr,"dbg5       Reserved4:                  %d\n",fileheader->Reserved4);
			fprintf(stderr,"dbg5       Reserved5:                  %d\n",fileheader->Reserved5);
			fprintf(stderr,"dbg5       Reserved6:                  %d\n",fileheader->Reserved6);
			fprintf(stderr,"dbg5       ProjectionType:             %s\n",fileheader->ProjectionType);
			fprintf(stderr,"dbg5       SpheroidType:               %s\n",fileheader->SpheroidType);
			fprintf(stderr,"dbg5       NavigationLatency:          %d\n",fileheader->NavigationLatency);
			fprintf(stderr,"dbg5       OriginY:                    %f\n",fileheader->OriginY);
			fprintf(stderr,"dbg5       OriginX:                    %f\n",fileheader->OriginX);
			fprintf(stderr,"dbg5       NavOffsetY:                 %f\n",fileheader->NavOffsetY);
			fprintf(stderr,"dbg5       NavOffsetX:                 %f\n",fileheader->NavOffsetX);
			fprintf(stderr,"dbg5       NavOffsetZ:                 %f\n",fileheader->NavOffsetZ);
			fprintf(stderr,"dbg5       NavOffsetYaw:               %f\n",fileheader->NavOffsetYaw);
			fprintf(stderr,"dbg5       MRUOffsetY:                 %f\n",fileheader->MRUOffsetY);
			fprintf(stderr,"dbg5       MRUOffsetX:                 %f\n",fileheader->MRUOffsetX);
			fprintf(stderr,"dbg5       MRUOffsetZ:                 %f\n",fileheader->MRUOffsetZ);
			fprintf(stderr,"dbg5       MRUOffsetYaw:               %f\n",fileheader->MRUOffsetYaw);
			fprintf(stderr,"dbg5       MRUOffsetPitch:             %f\n",fileheader->MRUOffsetPitch);
			fprintf(stderr,"dbg5       MRUOffsetRoll:              %f\n",fileheader->MRUOffsetRoll);
			for (i=0;i<fileheader->NumberOfSonarChannels
				+ fileheader->NumberOfBathymetryChannels;i++)
			    {
			    fprintf(stderr,"dbg5       TypeOfChannel:              %d\n",fileheader->chaninfo[i].TypeOfChannel);
			    fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",fileheader->chaninfo[i].SubChannelNumber);
			    fprintf(stderr,"dbg5       CorrectionFlags:            %d\n",fileheader->chaninfo[i].CorrectionFlags);
			    fprintf(stderr,"dbg5       UniPolar:                   %d\n",fileheader->chaninfo[i].UniPolar);
			    fprintf(stderr,"dbg5       BytesPerSample:             %d\n",fileheader->chaninfo[i].BytesPerSample);
			    fprintf(stderr,"dbg5       SamplesPerChannel:          %d\n",fileheader->chaninfo[i].SamplesPerChannel);
			    fprintf(stderr,"dbg5       ChannelName:                %s\n",fileheader->chaninfo[i].ChannelName);
			    fprintf(stderr,"dbg5       VoltScale:                  %f\n",fileheader->chaninfo[i].VoltScale);
			    fprintf(stderr,"dbg5       Frequency:                  %f\n",fileheader->chaninfo[i].Frequency);
			    fprintf(stderr,"dbg5       HorizBeamAngle:             %f\n",fileheader->chaninfo[i].HorizBeamAngle);
			    fprintf(stderr,"dbg5       TiltAngle:                  %f\n",fileheader->chaninfo[i].TiltAngle);
			    fprintf(stderr,"dbg5       BeamWidth:                  %f\n",fileheader->chaninfo[i].BeamWidth);
			    fprintf(stderr,"dbg5       OffsetX:                    %f\n",fileheader->chaninfo[i].OffsetX);
			    fprintf(stderr,"dbg5       OffsetY:                    %f\n",fileheader->chaninfo[i].OffsetY);
			    fprintf(stderr,"dbg5       OffsetZ:                    %f\n",fileheader->chaninfo[i].OffsetZ);
			    fprintf(stderr,"dbg5       OffsetYaw:                  %f\n",fileheader->chaninfo[i].OffsetYaw);
			    fprintf(stderr,"dbg5       OffsetPitch:                %f\n",fileheader->chaninfo[i].OffsetPitch);
			    fprintf(stderr,"dbg5       OffsetRoll:                 %f\n",fileheader->chaninfo[i].OffsetRoll);
			    fprintf(stderr,"dbg5       ReservedArea:               %s\n",fileheader->chaninfo[i].ReservedArea);
			    }
			}
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    }

	/* look for next recognizable record */
	done = MB_NO;
	while (status == MB_SUCCESS && done == MB_NO)
	    {
	    /* find the next packet beginning */
	    found = MB_NO;
	    skip = 0;
	    read_len = fread(line,1,2,mb_io_ptr->mbfp);
	    if (read_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    else if (((mb_u_char)line[0]) == 0xce && ((mb_u_char)line[1] == 0xfa))
		found = MB_YES;
	    while (status == MB_SUCCESS
		&& found == MB_NO)
		{
		line[0] = line[1];
		read_len = fread(&(line[1]),1,1,mb_io_ptr->mbfp);
		skip++;
		if (read_len != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (((mb_u_char)line[0]) == 0xce && ((mb_u_char)line[1] == 0xfa))
			found = MB_YES;
		}

	    /* read the next packet header */
	    read_len = fread(&(line[2]),1,12,mb_io_ptr->mbfp);
	    if (read_len == 12)
		{
		/* extract data from buffer */
		index = 0;
		packetheader.MagicNumber[0] = line[index];
		index++;
		packetheader.MagicNumber[1] = line[index];
		index++;
		packetheader.HeaderType = line[index];
		index++;
		packetheader.SubChannelNumber = line[index];
		index++;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.NumChansToFollow));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.Reserved1[0]));
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.Reserved1[1]));
		index += 2;
		mb_get_binary_int(MB_YES, &line[index], (int *)&(packetheader.NumBytesThisRecord));
		index += 4;

		/* check packet header details */
		if( packetheader.NumChansToFollow > 20)
			{
			if (verbose > 0)
				fprintf(stderr,"Bad packet header in xtf - skip this record\n");
			packetheader.NumBytesThisRecord = 0;
			packetheader.HeaderType = 99;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       Bytes Skipped:              %d\n",skip);
			fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
					packetheader.MagicNumber[0],packetheader.MagicNumber[1],
					packetheader.MagicNumber[0],packetheader.MagicNumber[1]);
			fprintf(stderr,"dbg5       HeaderType:                 %d\n",packetheader.HeaderType);
			fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",packetheader.SubChannelNumber);
			fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",packetheader.NumChansToFollow);
			fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",packetheader.Reserved1[0],packetheader.Reserved1[1]);
			fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",packetheader.NumBytesThisRecord);
			}
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		done = MB_YES;
		}

	    /* read rest of attitude packet */
	    if (status == MB_SUCCESS
		&& packetheader.HeaderType == XTF_DATA_ATTITUDE
		&& packetheader.NumBytesThisRecord == 64)
		{
#ifdef MBR_xtfb1624_DEBUG
fprintf(stderr,"Reading attitude packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		attitudeheader->packetheader = packetheader;
		read_len = fread(line,1,50,mb_io_ptr->mbfp);
		if (read_len == 50)
		    {
		    /* parse the rest of the attitude record */
		    index = 0;
		    for (i=0;i<4;i++)
			{
			mb_get_binary_int(MB_YES, &line[index], (int *)&(attitudeheader->Reserved2[i]));
			index += 4;
			}
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Pitch));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Roll));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Heave));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Yaw));
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(attitudeheader->TimeTag));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Heading));
		    index += 4;
		    for (i=0;i<10;i++)
			{
			attitudeheader->Reserved3[i] = line[index];
			index++;
			}

		    /* add attitude to list for interpolation */
		    timetag = 0.001 *  attitudeheader->TimeTag;
		    heave = attitudeheader->Heave;
		    roll = attitudeheader->Roll;
		    pitch = attitudeheader->Pitch;
		    heading = attitudeheader->Heading;

		    /* add latest attitude to list */
		    mb_attint_add(verbose, mbio_ptr,
				    timetag, heave, roll, pitch,
				    error);
		    mb_hedint_add(verbose, mbio_ptr,
				    timetag, heading, error);

		    /* print debug statements */
		    if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
					attitudeheader->packetheader.MagicNumber[0],attitudeheader->packetheader.MagicNumber[1],
					attitudeheader->packetheader.MagicNumber[0],attitudeheader->packetheader.MagicNumber[1]);
			fprintf(stderr,"dbg5       HeaderType:                 %d\n",attitudeheader->packetheader.HeaderType);
			fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",attitudeheader->packetheader.SubChannelNumber);
			fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",attitudeheader->packetheader.NumChansToFollow);
			fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",attitudeheader->packetheader.Reserved1[0],attitudeheader->packetheader.Reserved1[1]);
			fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",attitudeheader->packetheader.NumBytesThisRecord);
			fprintf(stderr,"dbg5       Reserved2[0]:               %d\n",attitudeheader->Reserved2[0]);
			fprintf(stderr,"dbg5       Reserved2[1]:               %d\n",attitudeheader->Reserved2[1]);
			fprintf(stderr,"dbg5       Reserved2[2]:               %d\n",attitudeheader->Reserved2[2]);
			fprintf(stderr,"dbg5       Reserved2[3]:               %d\n",attitudeheader->Reserved2[3]);
			fprintf(stderr,"dbg5       Pitch:                      %f\n",attitudeheader->Pitch);
			fprintf(stderr,"dbg5       Roll:                       %f\n",attitudeheader->Roll);
			fprintf(stderr,"dbg5       Heave:                      %f\n",attitudeheader->Heave);
			fprintf(stderr,"dbg5       Yaw:                        %f\n",attitudeheader->Yaw);
			fprintf(stderr,"dbg5       TimeTag:                    %d\n",attitudeheader->TimeTag);
			fprintf(stderr,"dbg5       Heading:                    %f\n",attitudeheader->Heading);
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
		}

	    /* read rest of sidescan packet */
	    else if (status == MB_SUCCESS
		&& packetheader.HeaderType == XTF_DATA_SIDESCAN)
		{
#ifdef MBR_xtfb1624_DEBUG
fprintf(stderr,"Reading sidescan packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		/* read and parse the the sidescan header */
		data->kind = MB_DATA_DATA;
		pingheader->packetheader = packetheader;
		read_len = fread(line,1,242,mb_io_ptr->mbfp);
		if (read_len == 242)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->Year));
		    index += 2;
		    pingheader->Month = line[index];
		    index++;
		    pingheader->Day = line[index];
		    index++;
		    pingheader->Hour = line[index];
		    index++;
		    pingheader->Minute = line[index];
		    index++;
		    pingheader->Second = line[index];
		    index++;
		    pingheader->HSeconds = line[index];
		    index++;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->JulianDay));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->CurrentLineID));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->EventNumber));
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingheader->PingNumber));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SoundVelocity));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->OceanTide));
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingheader->Reserved2));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->ConductivityFreq));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->TemperatureFreq));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->PressureFreq));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->PressureTemp));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Conductivity));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->WaterTemperature));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Pressure));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->ComputedSoundVelocity));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->MagX));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->MagY));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->MagZ));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal1));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal2));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal3));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal4));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal5));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->AuxVal6));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SpeedLog));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Turbidity));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->ShipSpeed));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->ShipGyro));
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(pingheader->ShipYcoordinate));
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(pingheader->ShipXcoordinate));
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->ShipAltitude));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingheader->ShipDepth));
		    index += 2;
		    pingheader->FixTimeHour = line[index];
		    index++;
		    pingheader->FixTimeMinute = line[index];
		    index++;
		    pingheader->FixTimeSecond = line[index];
		    index++;
		    pingheader->Reserved4 = line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorSpeed));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->KP));
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(pingheader->SensorYcoordinate));
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(pingheader->SensorXcoordinate));
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], &(pingheader->Reserved6));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(pingheader->RangeToSensor));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(pingheader->BearingToSensor));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(pingheader->CableOut));
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Layback));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->CableTension));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorDepth));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorPrimaryAltitude));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorAuxAltitude));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorPitch));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorRoll));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->SensorHeading));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Heave));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->Yaw));
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], &(pingheader->AttitudeTimeTag));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingheader->DOT));
		    index += 4;
		    for (i=0;i<20;i++)
			{
			pingheader->ReservedSpace[i] = line[index];
			index++;
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* read and parse the port sidescan channel header */
		if (status == MB_SUCCESS)
		    read_len = fread(line,1,64,mb_io_ptr->mbfp);
		if (status == MB_SUCCESS && read_len == 64)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ChannelNumber));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->DownsampleMethod));
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->SlantRange));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->GroundRange));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->TimeDelay));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->TimeDuration));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->SecondsPerPing));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ProcessingFlags));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->Frequency));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->InitialGainCode));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->GainCode));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->BandWidth));
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanportheader->ContactNumber));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ContactClassification));
		    index += 2;
		    pingchanportheader->ContactSubNumber = (mb_u_char) line[index];
		    index++;
		    pingchanportheader->ContactType = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanportheader->NumSamples));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->Reserved));
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->ContactTimeOffTrack));
		    index += 4;
		    pingchanportheader->ContactCloseNumber = (mb_u_char) line[index];
		    index++;
		    pingchanportheader->Reserved2 = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->FixedVSOP));
		    index += 4;
		    for (i=0;i<6;i++)
			{
			pingchanportheader->ReservedSpace[i] = (mb_u_char) line[index];
			index++;
			}

		    /* fix up on time duration if needed */
		    if ( pingchanportheader->TimeDuration == 0.0)
		    	{
		    	pingchanportheader->TimeDuration
				= pingchanportheader->SlantRange
					/ pingheader->SoundVelocity;
		    	}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* check for corrupted record */
		if (pingchanportheader->ChannelNumber
			> (fileheader->NumberOfSonarChannels
				+ fileheader->NumberOfBathymetryChannels - 1))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
/*		else if (pingchanportheader->NumSamples
			> fileheader->chaninfo[pingchanportheader->ChannelNumber].SamplesPerChannel)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
jrenken: SamplesPerChannel not used anymore, value can change dependend on range
*/
		/* read port sidescan data */
		if (status == MB_SUCCESS)
		    {
		    read_bytes = pingchanportheader->NumSamples
				    * fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample;
		    read_len = fread(line,1,read_bytes,mb_io_ptr->mbfp);
		    }
		if (status == MB_SUCCESS && read_len == read_bytes)
		    {
		    if (fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample == 1)
			{
			for (i=0;i<pingchanportheader->NumSamples;i++)
			    {
			    mb_u_char_ptr = (mb_u_char *) &line[i];
			    data->ssrawport[i] = (unsigned short) (*mb_u_char_ptr);
			    }
			}
		    else if (fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample == 2)
			{
			index = 0;
			for (i=0;i<pingchanportheader->NumSamples;i++)
			    {
			    mb_get_binary_short(MB_YES, &line[index], (short *)&(data->ssrawport[i]));
			    index += 2;
			    }
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* read and parse the starboard sidescan channel header */
		if (status == MB_SUCCESS)
		    read_len = fread(line,1,64,mb_io_ptr->mbfp);
		if (status == MB_SUCCESS && read_len == 64)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ChannelNumber));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->DownsampleMethod));
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->SlantRange));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->GroundRange));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->TimeDelay));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->TimeDuration));
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->SecondsPerPing));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ProcessingFlags));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->Frequency));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->InitialGainCode));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->GainCode));
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->BandWidth));
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanstbdheader->ContactNumber));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ContactClassification));
		    index += 2;
		    pingchanstbdheader->ContactSubNumber = (mb_u_char) line[index];
		    index++;
		    pingchanstbdheader->ContactType = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanstbdheader->NumSamples));
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->Reserved));
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->ContactTimeOffTrack));
		    index += 4;
		    pingchanstbdheader->ContactCloseNumber = (mb_u_char) line[index];
		    index++;
		    pingchanstbdheader->Reserved2 = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->FixedVSOP));
		    index += 4;
		    for (i=0;i<6;i++)
			{
			pingchanstbdheader->ReservedSpace[i] = (mb_u_char) line[index];
			index++;
			}

		    /* fix up on time duration if needed */
		    if ( pingchanstbdheader->TimeDuration == 0.0)
		    	{
		    	pingchanstbdheader->TimeDuration
				= pingchanstbdheader->SlantRange
					/ pingheader->SoundVelocity;
		    	}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* check for corrupted record */
		if (pingchanstbdheader->ChannelNumber
			> (fileheader->NumberOfSonarChannels
				+ fileheader->NumberOfBathymetryChannels - 1))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
/*		else if (pingchanstbdheader->NumSamples
			> fileheader->chaninfo[pingchanstbdheader->ChannelNumber].SamplesPerChannel)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
jrenken: SamplesPerChannel not used anymore, value can change depending on range
*/
		/* read starboard sidescan data */
		if (status == MB_SUCCESS)
		    {
		    read_bytes = pingchanstbdheader->NumSamples
				    * fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample;
		    read_len = fread(line,1,read_bytes,mb_io_ptr->mbfp);
		    }
		if (status == MB_SUCCESS && read_len == read_bytes)
		    {
		    if (fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample == 1)
			{
			for (i=0;i<pingchanstbdheader->NumSamples;i++)
			    {
			    mb_u_char_ptr = (mb_u_char *) &line[i];
			    data->ssrawstbd[i] = (unsigned short) (*mb_u_char_ptr);
			    }
			}
		    else if (fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample == 2)
			{
			index = 0;
			for (i=0;i<pingchanstbdheader->NumSamples;i++)
			    {
			    mb_get_binary_short(MB_YES, &line[index], (short *)&(data->ssrawstbd[i]));
			    index += 2;
			    }
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		    fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
				    pingheader->packetheader.MagicNumber[0],pingheader->packetheader.MagicNumber[1],
				    pingheader->packetheader.MagicNumber[0],pingheader->packetheader.MagicNumber[1]);
		    fprintf(stderr,"dbg5       HeaderType:                 %d\n",pingheader->packetheader.HeaderType);
		    fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",pingheader->packetheader.SubChannelNumber);
		    fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",pingheader->packetheader.NumChansToFollow);
		    fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",pingheader->packetheader.Reserved1[0],pingheader->packetheader.Reserved1[1]);
		    fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",pingheader->packetheader.NumBytesThisRecord);
		    fprintf(stderr,"dbg5       Year:                       %d\n",pingheader->Year);
		    fprintf(stderr,"dbg5       Month:                      %d\n",pingheader->Month);
		    fprintf(stderr,"dbg5       Day:                        %d\n",pingheader->Day);
		    fprintf(stderr,"dbg5       Hour:                       %d\n",pingheader->Hour);
		    fprintf(stderr,"dbg5       Minute:                     %d\n",pingheader->Minute);
		    fprintf(stderr,"dbg5       Second:                     %d\n",pingheader->Second);
		    fprintf(stderr,"dbg5       HSeconds:                   %d\n",pingheader->HSeconds);
		    fprintf(stderr,"dbg5       JulianDay:                  %d\n",pingheader->JulianDay);
		    fprintf(stderr,"dbg5       CurrentLineID:              %d\n",pingheader->CurrentLineID);
		    fprintf(stderr,"dbg5       EventNumber:                %d\n",pingheader->EventNumber);
		    fprintf(stderr,"dbg5       PingNumber:                 %d\n",pingheader->PingNumber);
		    fprintf(stderr,"dbg5       SoundVelocity:              %f\n",pingheader->SoundVelocity);
		    fprintf(stderr,"dbg5       OceanTide:                  %f\n",pingheader->OceanTide);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",pingheader->Reserved2);
		    fprintf(stderr,"dbg5       ConductivityFreq:           %f\n",pingheader->ConductivityFreq);
		    fprintf(stderr,"dbg5       TemperatureFreq:            %f\n",pingheader->TemperatureFreq);
		    fprintf(stderr,"dbg5       PressureFreq:               %f\n",pingheader->PressureFreq);
		    fprintf(stderr,"dbg5       PressureTemp:               %f\n",pingheader->PressureTemp);
		    fprintf(stderr,"dbg5       Conductivity:               %f\n",pingheader->Conductivity);
		    fprintf(stderr,"dbg5       WaterTemperature:           %f\n",pingheader->WaterTemperature);
		    fprintf(stderr,"dbg5       Pressure:                   %f\n",pingheader->Pressure);
		    fprintf(stderr,"dbg5       ComputedSoundVelocity:      %f\n",pingheader->ComputedSoundVelocity);
		    fprintf(stderr,"dbg5       MagX:                       %f\n",pingheader->MagX);
		    fprintf(stderr,"dbg5       MagY:                       %f\n",pingheader->MagY);
		    fprintf(stderr,"dbg5       MagZ:                       %f\n",pingheader->MagZ);
		    fprintf(stderr,"dbg5       AuxVal1:                    %f\n",pingheader->AuxVal1);
		    fprintf(stderr,"dbg5       AuxVal2:                    %f\n",pingheader->AuxVal2);
		    fprintf(stderr,"dbg5       AuxVal3:                    %f\n",pingheader->AuxVal3);
		    fprintf(stderr,"dbg5       AuxVal4:                    %f\n",pingheader->AuxVal4);
		    fprintf(stderr,"dbg5       AuxVal5:                    %f\n",pingheader->AuxVal5);
		    fprintf(stderr,"dbg5       AuxVal6:                    %f\n",pingheader->AuxVal6);
		    fprintf(stderr,"dbg5       SpeedLog:                   %f\n",pingheader->SpeedLog);
		    fprintf(stderr,"dbg5       Turbidity:                  %f\n",pingheader->Turbidity);
		    fprintf(stderr,"dbg5       ShipSpeed:                  %f\n",pingheader->ShipSpeed);
		    fprintf(stderr,"dbg5       ShipGyro:                   %f\n",pingheader->ShipGyro);
		    fprintf(stderr,"dbg5       ShipYcoordinate:            %f\n",pingheader->ShipYcoordinate);
		    fprintf(stderr,"dbg5       ShipXcoordinate:            %f\n",pingheader->ShipXcoordinate);
		    fprintf(stderr,"dbg5       ShipAltitude:               %d\n",pingheader->ShipAltitude);
		    fprintf(stderr,"dbg5       ShipDepth:                  %d\n",pingheader->ShipDepth);
		    fprintf(stderr,"dbg5       FixTimeHour:                %d\n",pingheader->FixTimeHour);
		    fprintf(stderr,"dbg5       FixTimeMinute:              %d\n",pingheader->FixTimeMinute);
		    fprintf(stderr,"dbg5       FixTimeSecond:              %d\n",pingheader->FixTimeSecond);
		    fprintf(stderr,"dbg5       Reserved4:                  %d\n",pingheader->Reserved4);
		    fprintf(stderr,"dbg5       SensorSpeed:                %f\n",pingheader->SensorSpeed);
		    fprintf(stderr,"dbg5       KP:                         %f\n",pingheader->KP);
		    fprintf(stderr,"dbg5       SensorYcoordinate:          %f\n",pingheader->SensorYcoordinate);
		    fprintf(stderr,"dbg5       SensorXcoordinate:          %f\n",pingheader->SensorXcoordinate);
		    fprintf(stderr,"dbg5       Reserved6:                  %d\n",pingheader->Reserved6);
		    fprintf(stderr,"dbg5       RangeToSensor:              %d\n",pingheader->RangeToSensor);
		    fprintf(stderr,"dbg5       BearingToSensor:            %d\n",pingheader->BearingToSensor);
		    fprintf(stderr,"dbg5       CableOut:                   %d\n",pingheader->CableOut);
		    fprintf(stderr,"dbg5       Layback:                    %f\n",pingheader->Layback);
		    fprintf(stderr,"dbg5       CableTension:               %f\n",pingheader->CableTension);
		    fprintf(stderr,"dbg5       SensorDepth:                %f\n",pingheader->SensorDepth);
		    fprintf(stderr,"dbg5       SensorPrimaryAltitude:      %f\n",pingheader->SensorPrimaryAltitude);
		    fprintf(stderr,"dbg5       SensorAuxAltitude:          %f\n",pingheader->SensorAuxAltitude);
		    fprintf(stderr,"dbg5       SensorPitch:                %f\n",pingheader->SensorPitch);
		    fprintf(stderr,"dbg5       SensorRoll:                 %f\n",pingheader->SensorRoll);
		    fprintf(stderr,"dbg5       SensorHeading:              %f\n",pingheader->SensorHeading);
		    fprintf(stderr,"dbg5       Heave:                      %f\n",pingheader->Heave);
		    fprintf(stderr,"dbg5       Yaw:                        %f\n",pingheader->Yaw);
		    fprintf(stderr,"dbg5       AttitudeTimeTag:            %d\n",pingheader->AttitudeTimeTag);
		    fprintf(stderr,"dbg5       DOT:                        %f\n",pingheader->DOT);
		    for (i=0;i<20;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,pingheader->ReservedSpace[i]);
		    fprintf(stderr,"dbg5       ChannelNumber:              %d\n",pingchanportheader->ChannelNumber);
		    fprintf(stderr,"dbg5       DownsampleMethod:           %d\n",pingchanportheader->DownsampleMethod);
		    fprintf(stderr,"dbg5       SlantRange:                 %f\n",pingchanportheader->SlantRange);
		    fprintf(stderr,"dbg5       GroundRange:                %f\n",pingchanportheader->GroundRange);
		    fprintf(stderr,"dbg5       TimeDelay:                  %f\n",pingchanportheader->TimeDelay);
		    fprintf(stderr,"dbg5       TimeDuration:               %f\n",pingchanportheader->TimeDuration);
		    fprintf(stderr,"dbg5       SecondsPerPing:             %f\n",pingchanportheader->SecondsPerPing);
		    fprintf(stderr,"dbg5       ProcessingFlags:            %d\n",pingchanportheader->ProcessingFlags);
		    fprintf(stderr,"dbg5       Frequency:                  %d\n",pingchanportheader->Frequency);
		    fprintf(stderr,"dbg5       InitialGainCode:            %d\n",pingchanportheader->InitialGainCode);
		    fprintf(stderr,"dbg5       GainCode:                   %d\n",pingchanportheader->GainCode);
		    fprintf(stderr,"dbg5       BandWidth:                  %d\n",pingchanportheader->BandWidth);
		    fprintf(stderr,"dbg5       ContactNumber:              %d\n",pingchanportheader->ContactNumber);
		    fprintf(stderr,"dbg5       ContactClassification:      %d\n",pingchanportheader->ContactClassification);
		    fprintf(stderr,"dbg5       ContactSubNumber:           %d\n",pingchanportheader->ContactSubNumber);
		    fprintf(stderr,"dbg5       ContactType:                %d\n",pingchanportheader->ContactType);
		    fprintf(stderr,"dbg5       NumSamples:                 %d\n",pingchanportheader->NumSamples);
		    fprintf(stderr,"dbg5       Reserved:                   %d\n",pingchanportheader->Reserved);
		    fprintf(stderr,"dbg5       ContactTimeOffTrack:        %f\n",pingchanportheader->ContactTimeOffTrack);
		    fprintf(stderr,"dbg5       ContactCloseNumber:         %d\n",pingchanportheader->ContactCloseNumber);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",pingchanportheader->Reserved2);
		    fprintf(stderr,"dbg5       FixedVSOP:                  %f\n",pingchanportheader->FixedVSOP);
		    for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,pingchanportheader->ReservedSpace[i]);
		    fprintf(stderr,"dbg5       ChannelNumber:              %d\n",pingchanstbdheader->ChannelNumber);
		    fprintf(stderr,"dbg5       DownsampleMethod:           %d\n",pingchanstbdheader->DownsampleMethod);
		    fprintf(stderr,"dbg5       SlantRange:                 %f\n",pingchanstbdheader->SlantRange);
		    fprintf(stderr,"dbg5       GroundRange:                %f\n",pingchanstbdheader->GroundRange);
		    fprintf(stderr,"dbg5       TimeDelay:                  %f\n",pingchanstbdheader->TimeDelay);
		    fprintf(stderr,"dbg5       TimeDuration:               %f\n",pingchanstbdheader->TimeDuration);
		    fprintf(stderr,"dbg5       SecondsPerPing:             %f\n",pingchanstbdheader->SecondsPerPing);
		    fprintf(stderr,"dbg5       ProcessingFlags:            %d\n",pingchanstbdheader->ProcessingFlags);
		    fprintf(stderr,"dbg5       Frequency:                  %d\n",pingchanstbdheader->Frequency);
		    fprintf(stderr,"dbg5       InitialGainCode:            %d\n",pingchanstbdheader->InitialGainCode);
		    fprintf(stderr,"dbg5       GainCode:                   %d\n",pingchanstbdheader->GainCode);
		    fprintf(stderr,"dbg5       BandWidth:                  %d\n",pingchanstbdheader->BandWidth);
		    fprintf(stderr,"dbg5       ContactNumber:              %d\n",pingchanstbdheader->ContactNumber);
		    fprintf(stderr,"dbg5       ContactClassification:      %d\n",pingchanstbdheader->ContactClassification);
		    fprintf(stderr,"dbg5       ContactSubNumber:           %d\n",pingchanstbdheader->ContactSubNumber);
		    fprintf(stderr,"dbg5       ContactType:                %d\n",pingchanstbdheader->ContactType);
		    fprintf(stderr,"dbg5       NumSamples:                 %d\n",pingchanstbdheader->NumSamples);
		    fprintf(stderr,"dbg5       Reserved:                   %d\n",pingchanstbdheader->Reserved);
		    fprintf(stderr,"dbg5       ContactTimeOffTrack:        %f\n",pingchanstbdheader->ContactTimeOffTrack);
		    fprintf(stderr,"dbg5       ContactCloseNumber:         %d\n",pingchanstbdheader->ContactCloseNumber);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",pingchanstbdheader->Reserved2);
		    fprintf(stderr,"dbg5       FixedVSOP:                  %f\n",pingchanstbdheader->FixedVSOP);
		    for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,pingchanstbdheader->ReservedSpace[i]);
		    for (i=0;i<MAX(pingchanportheader->NumSamples,pingchanstbdheader->NumSamples);i++)
			fprintf(stderr,"dbg5       sidescan[%4.4d]: %d %d\n",i,data->ssrawport[i], data->ssrawstbd[i]);
		    }
	    /* set success */
	    status = MB_SUCCESS;
	    *error = MB_ERROR_NO_ERROR;
	    done = MB_YES;

		}


	    /* else read rest of unknown packet */
	    else if (status == MB_SUCCESS)
		{
		if (((int)packetheader.NumBytesThisRecord) > 14)
			{
			for (i=0;i<((int)packetheader.NumBytesThisRecord)-14;i++)
				{
				read_len = fread(line,1,1,mb_io_ptr->mbfp);
				}
			if (read_len != 1)
		    		{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				}
			}
#ifdef MBR_xtfb1624_DEBUG
fprintf(stderr,"Reading unknown packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		}
	    }

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
