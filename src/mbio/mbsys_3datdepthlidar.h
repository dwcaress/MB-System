/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_3datdepthlidar.h	11/29/2013
 *	$Id$
 *
 *    Copyright (c) 2013-2014 by
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
 * mbsys_3datdepthlidar.h defines the MBIO data structures for handling data from
 * 3DatDepth LIDAR laser scanning system: 
 *      MBF_3DDEPTHP : MBIO ID 231 - SWATHplus processed format
 *
 * Author:	David W. Caress
 * Date:	November 29, 2013
 *
 * $Log: mbsys_3datdepthlidar.c,v $
 *
 */
/*
 * Notes on the MBSYS_3DATDEPTHLIDAR data structure:
 *
 * Vendor processed format from 3DatDepth, produced from raw LIDAR
 * time series files by proprietary 3DatDepth software
 *
 *--------------------------------------------------------------------------------
 * Processing Tool Data Output
 * 
 * The 3D at Depth Processing Tool reads binary sensor data and
 * processes it to provide corresponding range angle data, saved
 * to a binary file. A timestamp is provided for each scan recorded
 * in the file. This timestamp is the time of the first laser pulse
 * for that scan. The μsec time value listed for each laser pulse
 * is the relative time between each successive pulse, offset from
 * the timestamp. The value for the first pulse should be subtracted
 * from all pulses in that scan (including itself), such that the
 * first pulse would be 0 μsec accordingly.
 * 
 * ---------------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 *              Item	                                Value	            Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record	
 *           File magic number	                        0x3D46	            2   (1 UINT16)
 *           Parameter record id	                0x3D07	            2   (1 UINT16)
 *           File version	                        1	            2   (1 UINT16)
 *           File sub version	                        1	            2   (1 UINT16)
 * Scan Information		
 *           Scan type (AZ raster, AZEL raster, bowtie)	2, 3, 4             2   (1 UINT16)
 *           Cross track angle start (deg)		                    4   (1 float32)
 *           Cross track angle end (deg)		                    4   (1 float32)
 *           Forward track angle start (deg)		                    4   (1 float32)
 *           Forward track angle end (deg)		                    4   (1 float32)
 *           Counts per Scan (AZ raster and bowtie)		            2   (1 UINT16)
 *           Counts per cross track (AZEL raster)		            2   (1 UINT16)
 *           Counts per forward track (AZEL raster)		            2   (1 UINT16)
 *           Scanner Efficiency		                                    2   (1 UINT16)
 *           Scans per File		                                    2   (1 UINT16)
 *           Scan count		                                            4   (1 UINT32)
 *           
 * ---------------------------------------------------------------------------------------
 * Lidar Scan Record	
 *           Lidar scan record id	                0x3D52	            2   (1 UINT16)
 * First Pulse Timestamp ( 1 to n Scans )		
 *           Timestamp year		                                    2   (1 UINT16)
 *           Timestamp month		                                    1   (1 UINT8)
 *           Timestamp day		                                    1   (1 UINT8)
 *           Timestamp days since Jan 1		                            2   (1 UINT16)
 *           Timestamp hour		                                    2   (1 UINT16)
 *           Timestamp minutes		                                    1   (1 UINT8)
 *           Timestamp seconds		                                    1   (1 UINT8)
 *           Timestamp nano seconds		                            4   (1 UINT32)
 * Laser Pulse Data ( 1 to m pulses per scan )		
 *           Range ( from glass front ) meters                              4   (1 float32)
 *           Amplitude / peak of signal	                         	    2   (1 short int)
 *           SNR of signal return		                            4   (1 float32)
 *           Cross track angle (deg)		                            4   (1 float32)
 *           Forward track angle (deg)		                            4   (1 float32)
 *           Cross track offset (m)		                            4   (1 float32)
 *           Forward track offset (m)		                            4   (1 float32)
 *           Pulse time offset (µsec)		                            4   (1 UINT32)
 *           Saturated (0/1)    		                            1   (1 UINT8)
 *           
 * For each scan per file, a “First Pulse Timestamp” and “m” sets of
 * “Laser Pulse Data” will exist.  For example, for a Continuous Scan
 * file with 500 scans per file, and 200 pts per scan, the following
 * data would be present:
 * 	File Header
 * 	Scan Information
 *      (1) First Pulse Timestamp
 * 		200	 Laser Pulse Data sets
 *      (2) First Pulse Timestamp
 *              200	Laser Pulse Data sets
 * 		…
 *      (500) First Pulse Timestamp
 * 	        200	 Laser Pulse Data sets
 * 
 * A Bowtie scan file would be the same as above.  A Full scan file
 * will only contain one scan of data but with “Counts per Cross track”
 * multiplied by “Counts per Forward track” sets of laser pulse data.
 * 
 * The timestamp is the time of the first laser pulse for that scan.
 * The pulse time offset (µsec) value listed for each laser pulse is
 * the relative time between each successive pulse.  The value for the
 * first pulse should be subtracted from all pulses in that scan, such
 * that the first pulse would be 0 µsec accordingly.  The Pt Valid term
 * is provided to specify range validity.  If “valid” is given as 0, a
 * range could not be determined for that laser pulse, or it fell
 * outside of the processing limits.
 * 
 * Note, if processing is interrupted by hitting the “Cancel Processing”
 * button, an indeterminate number of records may exist in the current
 * processing file.  All processed files will be written to the same
 * folder location of the raw file.  The same file name is used for
 * processed files, but with a “.csv” or “.bin” file extension.  If data
 * is reprocessed in the same folder location, previously processed data
 * files will be overwritten.
 *
 *--------------------------------------------------------------------------------
 * INS Data Format
 * 
 * INU serial packets are archived to data files located at %RAW_DATA_PATH%/INU/.
 * %RAW_DATA_PATH% is set in the sensor configuration file. Data files are
 * named INU_MMDDYYYY_HHMMSS.bin. 3000 navigation messages are stored per
 * each archived data file, representing approximately 5 minutes data storage
 * if collecting messages at 10 Hz. The binary data file storage is provided
 * below, for the first record. Additional records follow to 3000 per file.
 * The latency from first character received to the timestamp is approximately
 * 100 msec. If an error occurs, or the first byte read from the serial port is
 * not equal to the INU magic character, the software will begin reading byte
 * by byte in an attempt to resync.
 *              Item                                Value                   Bytes
 *           Header Magic Number                    0xE32F                  2 (1 UINT16)
 *           Timestamp year                                                 2 (1 UINT16)
 *           Timestamp month                                                1 (1 UINT8)
 *           Timestamp day                                                  1 (1 UINT8)
 *           Timestamp days since Jan 1                                     2 (1 UINT16)
 *           Timestamp hour                                                 2 (1 UINT16)
 *           Timestamp minutes                                              1 (1 UINT8)
 *           Timestamp seconds                                              1 (1 UINT8)
 *           Timestamp nano seconds                                         4 (1 UINT32)
 *           Latency (μseconds) – latency from first                        4 (1 float32)
 *                                 character received to timestamp
 *           INU Nav message 1
 *                      complete message as received from INU               Len / packet ‐ reference the INU specification
 *                      1 magic, 1 ID, 1 data len, N NAV bytes                          
 * 
 *--------------------------------------------------------------------------------
 * CTD Ethernet Interface
 * 
 * The 3D at Depth laser sensor listens for CTD data packets available on port
 * 2003 of the static IP address for the sensor. The sensor reads and timestamps
 * these packets and saves the data to a binary file, located at %RAW_DATA_PATH%/CTD/.
 * %RAW_DATA_PATH% is set in the sensor configuration file.
 * Data files are named CTD_MMDDYYYY_HHMMSS.bin. The file format is as follows.
 * CTD Packets
 *              Item                                Value                   Bytes
 *           Header Magic Number                    0x3D07                  2 (1 UINT16)
 *           Timestamp year                                                 2 (1 UINT16)
 *           Timestamp month                                                1 (1 UINT8)
 *           Timestamp day                                                  1 (1 UINT8)
 *           Timestamp days since Jan 1                                     2 (1 UINT16)
 *           Timestamp hour                                                 2 (1 UINT16)
 *           Timestamp minutes                                              1 (1 UINT8)
 *           Timestamp seconds                                              1 (1 UINT8)
 *           Timestamp nano seconds                                         4 (1 UINT32)
 *           Header project code                    0x3D03                  2 (1 UINT16)
 *           Header version                         1                       1 (1 UINT8)
 *           Water Temperature C                                            8 (1 double)
 *           Water Salinity psu                                             8 (1 double)
 *           Water Pressure dbar                                            8 (1 double)
 *--------------------------------------------------------------------------------
 *
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* defines */
#define MBF_3DDEPTHP_MAGICNUMBER                    0x3D46    /* '=''F' */
#define MBF_3DDEPTHP_RECORD_PARAMETER               0x3D07    /* '=', Bell */
#define MBF_3DDEPTHP_RECORD_RAWLIDAR                0x3D52    /* '=''R' */
#define MBF_3DDEPTHP_RECORD_COMMENT                 0x3D43    /* '=''C' */
#define MBF_3DDEPTHP_RECORD_LIDAR                   0x3D4C    /* '=''L' */
#define MBF_3DDEPTHP_RECORD_POSITION                0x3D50    /* '=''P' */
#define MBF_3DDEPTHP_RECORD_ATTITUDE                0x3D41    /* '=''A' */
#define MBF_3DDEPTHP_RECORD_HEADING                 0x3D48    /* '=''H' */
#define MBF_3DDEPTHP_RECORD_SENSORDEPTH             0x3D5A    /* '=''Z' */
#define MBF_3DDEPTHP_VERSION_1_0_PARAMETER_SIZE     36
#define MBF_3DDEPTHP_VERSION_1_1_PARAMETER_SIZE     38
#define MBF_3DDEPTHP_VERSION_1_0_SCANHEADER_SIZE    14
#define MBF_3DDEPTHP_VERSION_1_1_RAWSCANHEADER_SIZE 18
#define MBF_3DDEPTHP_VERSION_1_1_SCANHEADER_SIZE    66
#define MBF_3DDEPTHP_VERSION_1_0_PULSE_SIZE         31
#define MBF_3DDEPTHP_VERSION_1_1_RAWPULSE_SIZE      31
#define MBF_3DDEPTHP_VERSION_1_1_PULSE_SIZE         100

/* Kearfoot INS position and attitude data structure */

/* 3DatDepth LIDAR data structure */
struct mbsys_3datdepthlidar_pulse_struct
	{
        /* Laser Pulse Data (1 to counts_per_scan pulses per scan) */
        float           range;                      /* meters from glass front */
        short           amplitude;                  /* peak of signal - to 1023 */
        float           snr;                        /* SNR of signal return */
        float           cross_track_angle;          /* degrees */
        float           forward_track_angle;        /* degrees */
        float           cross_track_offset;         /* m */
        float           forward_track_offset;       /* m */
        unsigned int    pulse_time_offset;          /* time offset relative to start of scan (usec) */
        mb_u_char       saturated;                  /* boolean */
        
        /* processed information */
        double          time_d;                     /* epoch time */
        mb_u_char       beamflag;                   /* MB-System beam flag */
        double          acrosstrack;                /* acrosstrack distance relative to survey system reference point (meters) */
        double          alongtrack;                 /* alongtrack distance relative to survey system reference point (meters) */
        double          depth;                      /* m relative to survey system reference point */
        double          navlon;                     /* absolute position longitude (degrees) */
        double          navlat;                     /* absolute position latitude (degrees) */
        double          sensordepth;                 /* absolute position depth below sea surface (meters), includes any tide correction */
        float           heading;                    /* lidar heading (degrees) */
        float           roll;                       /* lidar roll (degrees) */
        float           pitch;                      /* lidar pitch (degrees) */
	};

/* 3DatDepth LIDAR data structure */
struct mbsys_3datdepthlidar_struct
	{
        /* Type of data record */
        int             kind;                       /* MB-System record ID */

        /* File Header */
        unsigned short  file_version;               /* 1 */
        unsigned short  sub_version;                /* 0 = initial version from 3DatDepth,
                                                       1 = first extended version for MB-System */
        
        /* Scan Information */
        unsigned short  scan_type;                  /* (2: AZ raster, 3: AZEL raster, 4: bowtie) */
        float           cross_track_angle_start;    /* across track angle start relative to lidar (degrees) */
        float           cross_track_angle_end;      /* across track angle end relative to lidar (degrees) */
        float           forward_track_angle_start;  /* across track angle end relative to lidar (degrees) */
        float           forward_track_angle_end;    /* across track angle end relative to lidar (degrees) */
        unsigned short  counts_per_scan;            /* pulse count for scan types AZ raster and bowtie*/
        unsigned short  counts_per_cross_track;     /* across track pulse count for scan type AZEL raster */
        unsigned short  counts_per_forward_track;   /* along track pulse count for scan type AZEL raster */
        unsigned short  scanner_efficiency;         /* */
        unsigned short  scans_per_file;             /* number of scans in this file */
        unsigned int    scan_count;                 /* global scan count */
        
        /* Id of most recently read record */
        unsigned short  record_id;                  /* MBF_3DDEPTHP_RECORD_RAWLIDAR     0x0000          */
                                                    /* MBF_3DDEPTHP_RECORD_COMMENT      0x3D43   'R''C' */
                                                    /* MBF_3DDEPTHP_RECORD_LIDAR        0x3D4C   'R''L' */
                                                    /* MBF_3DDEPTHP_RECORD_POSITION     0x3D50   'R''P' */
                                                    /* MBF_3DDEPTHP_RECORD_ATTITUDE     0x3D41   'R''A' */
                                                    /* MBF_3DDEPTHP_RECORD_HEADING      0x3D48   'R''H' */
                                                    /* MBF_3DDEPTHP_RECORD_SENSORDEPTH  0x3D5A   'R''Z' */

        /* Number of Current Laser Scan Data (1 to scans_per_file Scans) */
        int             current_scan;               /* sequential count of current scan, starting with 0 */
        
        /* First Pulse Timestamp (1 to n Scans) */
        unsigned short  year;
        mb_u_char       month;
        mb_u_char       day;
        unsigned short  days_since_jan_1;           /* day of year minus 1 */
        unsigned short  hour;
        mb_u_char       minutes;
        mb_u_char       seconds;
        unsigned int    nanoseconds;
        
        /* position and attitude per first pulse */
        double          time_d;                     /* epoch time */
        double          navlon;                     /* lidar reference position longitude (degrees) */
        double          navlat;                     /* lidar reference position latitude (degrees) */
        double          sensordepth;                 /* lidar reference position depth (meters) */
        float           heading;                    /* heading (degrees) */
        float           roll;                       /* roll (degrees) */
        float           pitch;                      /* pitch (degrees) */
        float           speed;                      /* speed (degrees) */

        /* Laser Scan Data (1 to counts_per_scan pulses per scan) */
        int             bathymetry_calculated;      /* boolean flag re calculation of bathymetry from ranges and angles */
        int             num_pulses;                 /* number of pulses */
        int             num_pulses_alloc;           /* array allocated for this number of pulses */
        struct mbsys_3datdepthlidar_pulse_struct *pulses;
        
        /* comment */
        unsigned short  comment_len;                    /* comment length in bytes */
        char            comment[MB_COMMENT_MAXLINE];    /* comment string */
 
        /* position data */
        double  pos_time_d;
        double  pos_longitude;
        double  pos_latitude;
        
        /* attitude data */
        double  att_time_d;
        double  att_roll;
        double  att_pitch;
        double  att_heave;
        
        /* heading data */
        double  hdg_time_d;
        double  hdg_heading;
        
        /* sensordepth data */
        double  sdp_time_d;
        double  sdp_sensordepth;
	};


/* System specific function prototypes */
int mbsys_3datdepthlidar_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3datdepthlidar_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3datdepthlidar_dimensions(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbath,
	int *namp,
	int *nss,
	int *error);
int mbsys_3datdepthlidar_pingnumber(int verbose, void *mbio_ptr, int *pingnumber, int *error);
int mbsys_3datdepthlidar_preprocess(int verbose,
        void *mbio_ptr,
        void *store_ptr,
	double time_d,
        double navlon,
        double navlat,
	double speed,
        double heading,
        double sensordepth,
	double roll,
        double pitch,
        double heave,
	int *error);
int mbsys_3datdepthlidar_extract(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int time_i[7],
	double *time_d,
	double *navlon,
	double *navlat,
	double *speed,
	double *heading,
	int *nbath,
	int *namp,
	int *nss,
	char *beamflag,
	double *bath,
	double *amp,
	double *bathacrosstrack,
	double *bathalongtrack,
	double *ss,
	double *ssacrosstrack,
	double *ssalongtrack,
	char *comment,
	int *error);
int mbsys_3datdepthlidar_insert(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int kind,
	int time_i[7],
	double time_d,
	double navlon,
	double navlat,
	double speed,
	double heading,
	int nbath,
	int namp,
	int nss,
	char *beamflag,
	double *bath,
	double *amp,
	double *bathacrosstrack,
	double *bathalongtrack,
	double *ss,
	double *ssacrosstrack,
	double *ssalongtrack,
	char *comment,
	int *error);
int mbsys_3datdepthlidar_ttimes(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbeams,
	double *ttimes,
	double *angles,
	double *angles_forward,
	double *angles_null,
	double *heave,
	double *alongtrack_offset,
	double *draft,
	double *ssv,
	int *error);
int mbsys_3datdepthlidar_detects(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbeams,
	int *detects,
	int *error);
int mbsys_3datdepthlidar_pulses(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbeams,
	int *pulses,
	int *error);
int mbsys_3datdepthlidar_gains(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	double *transmit_gain,
	double *pulse_length,
	double *receive_gain,
	int *error);
int mbsys_3datdepthlidar_extract_altitude(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	double *transducer_depth,
	double *altitude,
	int *error);
int mbsys_3datdepthlidar_extract_nnav(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int nmax,
	int *kind,
	int *n,
	int *time_i,
	double *time_d,
	double *navlon,
	double *navlat,
	double *speed,
	double *heading,
	double *draft,
	double *roll,
	double *pitch,
	double *heave,
	int *error);
int mbsys_3datdepthlidar_extract_nav(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int time_i[7],
	double *time_d,
	double *navlon,
	double *navlat,
	double *speed,
	double *heading,
	double *draft,
	double *roll,
	double *pitch,
	double *heave,
	int *error);
int mbsys_3datdepthlidar_insert_nav(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int time_i[7],
	double time_d,
	double navlon,
	double navlat,
	double speed,
	double heading,
	double draft,
	double roll,
	double pitch,
	double heave,
	int *error);
int mbsys_3datdepthlidar_extract_svp(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nsvp,
	double *depth,
	double *velocity,
	int *error);
int mbsys_3datdepthlidar_insert_svp(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int nsvp,
	double *depth,
	double *velocity,
	int *error);
int mbsys_3datdepthlidar_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_3datdepthlidar_print_store(int verbose, void *store_ptr, int *error);
int mbsys_3datdepthlidar_calculatebathymetry(int verbose, void *mbio_ptr, void *store_ptr, int *error);