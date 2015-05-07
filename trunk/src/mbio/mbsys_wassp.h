/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_wassp.h	1/28/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2015 by
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
 * mbsys_wassp.h defines the MBIO data structures for handling data from
 * the following data formats:
 *    MBSYS_WASSP formats (code in mbsys_wassp.c and mbsys_wassp.h):
 *      MBF_WASSPENL : MBIO ID 241 (code in mbr_wasspenl.c)
 *
 * Author:	D. W. Caress
 * Date:	January 28, 2014
 *
 *
 */
/*
 * Notes on the mbsys_wassp data structure and associated format:
 *   1. This MBIO format supports the generic interface message output
 *      by the ENL WASSP multibeam sonars.
 *   2. Reference: WASSP Generic ICD 2.2, 15 October 2013.
 *   2.            WASSP Generic ICD 2.4, 17 February 2015.
 *   3. The WASSP data stream consists of several different data
 *      records that can vary among models and installations.
 *   4. The WASSP multibeam models as of January 2014 incude:
 *        WMB-3230, 160 kHz, 112 beams, 120 degree swath
 *        WMB-5230, 80 kHz, 112 beams, 120 degree swath
 *        WMB-3250, 160 kHz, 224 beams, 120 degree swath
 *   5. The alongtrack beamwidths are 4 degrees, the acrosstrack
 *      beamwidths are estimated to be 2 degrees since the transducer
 *      arrays are about two times wider than long.
 *   6. Each data record begins with a 16 byte sequence:
 *        unsigned int sync;        \\ 0x77F9345A
 *        unsigned int size;        \\ Size in bytes of this record from start
 *                                  \\     of sync pattern to end of checksum
 *        char         header[8];   \\ Data record name
 *   7. All data are in little-endian form.
 *   8. The data record names include:
 *        GENBATHY - Uncorrected Bathymetry
 *        GEN_SENS - External Sensor Data
 *        RAWSONAR - Raw water column data (roll stabilized sonar)
 *        CORBATHY - Corrected Bathymetry
 *        NVUPDATE - Nav Data Message
 *        WCD_NAVI - Water Column Information
 *        SYS_CFG1 - Unknown record at start of file
 *                     (3224 bytes including sync and checksum)
 *   9. Some but not all sample files logged by the ENL datalogger have an 804 byte UTF-8
 *      header that looks like:
 *      ********************************************************************************
 *      **************************************** PM Version: 2.4.1.288****************
 *      **************************************** GUI Version: 2.4.1.125****************
 *      **************************************** Mity Verion: 20060****************
 *      ********************************************************************************
 *      ********************************************************************************
 *      **************************************** PM Version: 2.4.1.288****************
 *      **************************************** GUI Version: 2.4.1.125****************
 *      **************************************** Mity Verion: 20060****************
 *      ********************************************************************************
 *      The reading code will search for valid sync values and so should ignore similar
 *      headers when encountered.
 *      A sample file logged by Jonathon Beaudoin of the University of New Hampshire
 *      Center for Coastal and Ocean Mapping does not contain this header, so it is a
 *      feature of the ENL datalogger and not part of the data stream output by the
 *      WASSP sonars.
 *  10. The CORBATHY records do not have a full time stamp, and thus do not stand alone
 *      as survey data. Both a GENBATHY and a CORBATHY record are required to form a useful
 *      survey record.
 *  11. A survey record may or may not include a RAWSONAR record and a WCD_NAVI record.
 *  12. The UNH/CCOM data samples contain adjacent GENBATHY and CORBATHY records for each ping.
 *      The ENL-logged data have separate groups of GENBATHY records and CORBATHY records,
 *      with each CORBATHY record occuring 10 to 30 records after the corresponding
 *      GENBATHY record. Consequently, parsing these data would require buffering the GENBATHY
 *      records to match them with the right CORBATHY record. The current I/O module is
 *      not implemented with GENBATHY buffering, and will not work with the ENL data sample.
 *  13. The contents of the GEN_SENS records are not specified and are presently unknown.
 *  14. The NVUPDATE record does not include a time stamp and thus does not usefully serve
 *      as asynchronous navigation and attitude.
 *  15. The maximum number of beams is known, and can be statically dimensioned for the 
 *      GENBATHY and CORBATHY records. However, the maximum numbers of samples in the RAWSONAR 
 *      and WCD_NAVI records are not defined, and so these structures must allow for 
 *      dynamic allocation of memory.
 *     
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------*/
/* Record ID definitions (if needed for use in data reading and writing) */
#define MBSYS_WASSP_SYNC                0x77F9345A
#define MBSYS_WASSP_RECORD_GENBATHY     1   // Uncorrected Bathymetry
#define MBSYS_WASSP_RECORD_GEN_SENS     2   // External Sensor Data
#define MBSYS_WASSP_RECORD_RAWSONAR     3   // Raw water column data (roll stabilized sonar)
#define MBSYS_WASSP_RECORD_CORBATHY     4   // Corrected Bathymetry
#define MBSYS_WASSP_RECORD_NVUPDATE     4   // Nav Data Message
#define MBSYS_WASSP_RECORD_WCD_NAVI     5   // Water Column Information
#define MBSYS_WASSP_RECORD_SENSPROP     6   // Sensor Properties
#define MBSYS_WASSP_RECORD_SYS_PROP     7   // System Properties
#define MBSYS_WASSP_RECORD_MCOMMENT     8   // Comment (MB-System only)
#define MBSYS_WASSP_RECORD_SYS_CFG1     9   // Unknown record at start of some files

/*---------------------------------------------------------------*/
/* Record size definitions (if needed for use in data reading and writing) */

/*---------------------------------------------------------------*/
/* Array size definitions (if needed for use in data reading and writing) */
#define MBSYS_WASSP_MAX_BEAMS           512
#define MBSYS_WASSP_MAX_PIXELS          0
#define MBSYS_WASSP_MAX_MESSAGE         128
#define MBSYS_WASSP_BUFFER_STARTSIZE    32768
/*---------------------------------------------------------------*/

/* Structure size definitions (if needed because there are dynamically allocated substructures) */

/* Individual data record structures */
struct mbsys_wassp_genbathy_struct
	{
        /* GENBATHY Record */
        /* Supported Products: - WMB-3250 */
        /* Uncorrected Bathymetry
         * All the bottom detection points will be supplied as range and angle values.
         * The length of the output message is variable, dependant on the number of beam data.
         * In addition to the Flags the sample number will be set to zero when detection is invalid. */
        /* unsigned int sync; */               /* 0x77F9345A */
        /* unsigned int size; */               /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */          /* "GENBATHY" */
        unsigned int    version;               /* 3 */
        double          msec;                  /* A millisecond time stamp of rising edge
                                                * of Transmit pulse in UTC time (UTC time
                                                * is calculated from the timestamp of the
                                                * ZDA sentence and or a PPS signal when
                                                * available) No local time zone correction
                                                * is applied.
                                                * On systems not supporting UTC time, this
                                                * will be system referenced time. */
        mb_u_char       day;                   /* UTC time from NMEA ZDA time message, if available */
        mb_u_char       month;                 /* UTC time from NMEA ZDA time message, if available */
        unsigned short  year;                  /* UTC time from NMEA ZDA time message, if available */
        unsigned int    ping_number;           /* Sequential number. */
        unsigned int    sonar_model;           /* unused in ICD 2.4 */
        unsigned long   transducer_serial_number;
        unsigned int    number_beams;
        unsigned int    modeflags;             /* Bit field:
                                                *   Bit 0: Roll information is valid (not implemented)
                                                *   Bit 1: Backscatter information is valid in beam data
                                                *   Bits 2-31: Reserved for future use */
        float           sampling_frequency;    /* Sonar sampling frequency (Hz) */
        float           acoustic_frequency;    /* Sonar nominal acoustic frequency (Hz) */
        float           tx_power;              /* Voltage (volts) rms applied to
                                                * transmitter in dB. If "Sample Type" field is set
                                                * to "un-calibrated" this will be the nominal
                                                * power level */
        float           pulse_width;           /* Pulse length in milliseconds */
        float           absorption_loss;       /* Configurable value applied by WASSP. */
        float           spreading_loss;        /* 0, 30 or 40 as selected by WASSP GUI,
                                                         * units dB (as function of target range
                                                         * in metres) */
        unsigned int    sample_type;           /* Set to 0 if un-calibrated. Set to 1 if calibrated.
                                                * All dB values will be relative rather than absolute
                                                * if ‘Sample Type’ is ‘un-calibrated’ */
        float           sound_velocity;        /* Sound velocity at the sonar head in m/s
                                                         * (that was used in beam forming) */
        float           detection_point[MBSYS_WASSP_MAX_BEAMS];
                                                /* Non-corrected fractional sample number
                                                 * with the reference to the receiver’s
                                                 * acoustic centre with the zero sample at
                                                 * the transmit time. */
        float           rx_angle[MBSYS_WASSP_MAX_BEAMS];
                                                /* Beam steering angle with reference to
                                                 * receiver’s acoustic centre in the sonar
                                                 * reference frame, at the detection point;
                                                 * in degrees. */
        unsigned int    flags[MBSYS_WASSP_MAX_BEAMS];
                                                /* Bit fields:
                                                    Bit 0: Detection Success
                                                    Bit 1: Detection Valid
                                                    Bit 2: WMT Detection
                                                    Bit 3: SAC Detection
                                                    Bits 4-31: Reserved for future use */
        float           backscatter[MBSYS_WASSP_MAX_BEAMS];
                                                /* Target strength (dB) at seafloor 
                                                 * detection point on this beam. */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_gen_sens_struct
	{
        /* GEN_SENS Record */
        /* Supported Products: - WMB-3250 */
        /* External Sensor Data
         * The External Sensor Data would be a WASSP time-stamped replica of the External Sensor Data
         * (e.g. Attitude, NMEA data) received by the WASSP. This data may be required to provide a
         * facility to undo the sensor stabilization performed by WASSP.
         */
        unsigned int    version;               /* 2 */
        double          msec;                  /* A millisecond time stamp of rising edge
                                                         * of Transmit pulse in UTC time (UTC time
                                                         * is calculated from the timestamp of the
                                                         * ZDA sentence and or a PPS signal when
                                                         * available) No local time zone correction
                                                         * is applied. */
        unsigned int    port_number;           /* Serial port stream number (1-10) */
        mb_u_char       message_length;        /* length of message in characters */
        char            message[MBSYS_WASSP_MAX_MESSAGE];
                                                        /* Copy of Sensor Data
                                                         * Length of this field = Size – 33 (followed by
                                                         * line feed and or carriage return characters)
                                                         * e.g. $HEHDT,45.2,,*67 */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_rawsonar_struct
	{
        /* RAWSONAR Record */
        /* Supported Products: - WMB-3250 */
        /* Raw water column data (roll stabilized sonar)
         * This packet is only roll stabilized if the WASSP system has valid roll information available.
         * The data contained in this packet is to be dB signal levels received by time and angle. Some
         * filtering of the data may be applied to remove side lobes and noise. This data is likely to be
         * a subset of the full sampling resolution of the system – less beams and less samples. The first
         * sample of raw data is the first sampling period starting from the rising edge of the transmit
         * pulse and ending at the end of the sampling period determined by the Sample Rate.
         * The Spreading Loss value applied to this data can be found in the preceding GENBATHY packet.
         */
        unsigned int    version;                /* 2 */
        double          msec;                   /* A millisecond time stamp of rising edge
                                                 * of Transmit pulse in UTC time (UTC time
                                                 * is calculated from the timestamp of the
                                                 * ZDA sentence and or a PPS signal when
                                                 * available) No local time zone correction
                                                 * is applied. 
                                                 * On systems not supporting UTC time, this
                                                 * will be system referenced time. */
        unsigned int    ping_number;            /* Sequential number. */
        float           sample_rate;            /* Frequency (Hz) of raw data in this packet */
        unsigned int    n;                      /* Number of beams of raw data in this packet */
        unsigned int    m;                      /* Number of samples (per beam) of raw data in this packet */
        float           tx_power;               /* Voltage (volts) rms applied to
                                                         * transmitter in dB.*/
        float           pulse_width;            /* Pulse length in milliseconds */
        unsigned int    sample_type;            /* Set to 0 if un-calibrated.
                                                         * Set to 1 if calibrated. */
        unsigned short  spare[MBSYS_WASSP_MAX_BEAMS];
                                                /* Unused */
        unsigned short  beam_index[MBSYS_WASSP_MAX_BEAMS];
                                                /* Equivalent beam Index into uncorrected bathy
                                                 * (GENBATHY) record of each beam. */
        unsigned int    detection_point[MBSYS_WASSP_MAX_BEAMS];
                                                /* Index of sample which most closely matches
                                                 * seafloor detection. 0 = not valid. */
        float           beam_angle[MBSYS_WASSP_MAX_BEAMS];
                                                /* Beam angle for this beam in degrees (negative
                                                 * port side of nadir) */
        size_t          rawdata_alloc;          /* Number of shorts allocated for rawdata array */
        short           *rawdata;               /* If Sample Type = 0 then Signal Levels at
                                                 * sample/beam in dB*100 (divide by 100 to get
                                                 * actual signal level dB). The order is
                                                 *   sample 1 x [0, 1, 2, 3, ... N] 
                                                 *   sample 2 x [0, 1, 2, 3, ... N] 
                                                 *   .....
                                                 *   sample M x [0, 1, 2, 3, ... N] 
                                                 * If Sample Type = 1 then calibrated db*100. */
        unsigned int    checksum;               /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };


struct mbsys_wassp_corbathy_struct
	{
        /* CORBATHY Record */
        /* Supported Products:
         *   WMB-3250
         *   WMB-3230
         *   WMB-5230 */
        /* Corrected Bathymetry
         * Use this data for use as fully corrected bathymetry data. The ships sensors
         * integrated into the WASSP correct this information for leaver arm, pitch, roll, yaw,
         * heave, tide etc. Each total message contains the detections data for a single ping.
         * NOTES:
         *      1) Sign of Latitude (N = +ve)
         *      2) Sign of Longitude (E = +ve)
         *      3) All points are sent for every beam even if they contain no detection data.
         *         So you can check if it is valid by checking the Y value, if this is 0 then the
         *         detection is not valid and should not be used.
         *      4) The X,Y,Z positions are relative to the position in Table 5 Corrected Bathy.
         *         This is based on the fully corrected output using lever arm,
         *         sensor data and sound speed information available. This means if the X, Y, Z
         *         offsets in the WASSP application are correct, there is no need to account for
         *         the distance between GPS antenna and transducer or any pitch/roll/heave inclination.
         *      5) Sign of Longitude is normal (East = positive)
         *      6) Depths are tide corrected unless tides are disabled in the WASSP system.
         */
        /* unsigned int sync; */               /* 0x77F9345A */
        /* unsigned int size; */               /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */          /* "CORBATHY" */
        unsigned int    version;               /* 3 for ICD 2.2, 4 for ICD 2.4 */
        double          msec;                  /* Version 4: unused
                                                * Version 3:
                                                * A millisecond time stamp of rising edge
                                                * of Transmit pulse in UTC time (UTC time
                                                * is calculated from the timestamp of the
                                                * ZDA sentence and or a PPS signal when
                                                * available) No local time zone correction
                                                * is applied.
                                                * On systems not supporting UTC time, this
                                                * will be system referenced time. */
        unsigned int    num_beams;             /* Fixed by software. Invalid points have depth set to 0.0. */
        unsigned int    ping_number;           /* Ping sequence number */
        double          latitude;              /* Latitude at transducer in degrees*/
        double          longitude;             /* Longitude at transducer in degrees */
        float           bearing;               /* Bearing/Heading of vessel on transmit in degrees */
        float           roll;                  /* Roll of vessel on transmit in degrees */
        float           pitch;                 /* Pitch of vessel on transmit in degrees */
        float           heave;                 /* Heave of vessel on transmit at transducer in meters */
        unsigned int    sample_type;           /* Set to 0 if un-calibrated. Set to 1 if calibrated. */
        float           tide;                  /* Tide adjustment applied in meters */
        unsigned int    spare[5];
        unsigned int    beam_index[MBSYS_WASSP_MAX_BEAMS];
                                                /* Beam index number */
        float           x[MBSYS_WASSP_MAX_BEAMS];
                                                /* Distance to detection point in metres laterally along
                                                 * west/east axis. East of position is positive. */
        float           y[MBSYS_WASSP_MAX_BEAMS];
                                                /* Distance to detection point in metres laterally along
                                                 * north/south axis. South of position is positive. */
        float           z[MBSYS_WASSP_MAX_BEAMS];
                                                /* Depth in meters for the detection point. –ve = down.
                                                 * 0 = not valid */
        float           beam_angle[MBSYS_WASSP_MAX_BEAMS];
                                                /* Angle of the beam this detection appears on in radians
                                                 * positive for starboard side of vessel. */
        float           backscatter[MBSYS_WASSP_MAX_BEAMS];
                                                /* Post-processed target strength (dB) at seafloor on this beam. */
        mb_u_char       quality[MBSYS_WASSP_MAX_BEAMS];
                                                /* Detection information - (0=none, 1=WMT, 2=SAC) */
        mb_u_char       fish[MBSYS_WASSP_MAX_BEAMS];
                                                /* Fish intensity value for all fish targets vertically above
                                                 * detection point. */
        mb_u_char       roughness[MBSYS_WASSP_MAX_BEAMS]; /* Unused */
        mb_u_char       empty[MBSYS_WASSP_MAX_BEAMS];   /* Unused */
        unsigned int    pad[MBSYS_WASSP_MAX_BEAMS];     /* Unused */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_nvupdate_struct
	{
        /* NVUPDATE Record */
        /* Supported Products:
         *   WMB-3250
         *   WMB-3230
         *   WMB-5230 */
        /* Nav Data Message
         * This message is used to update the external application with the most current navigation data.
         * This is output at approximately 100ms intervals. This data DOES NOT relate to the detection data or
         * water column data and is just the most recent navigation data the WASSP system is processing.
         * Data shown will be that configured to be used by the WASSP system. All serial data is also
         * sent via the raw data format.
         * Please Note: MAG value has no units at this time. The value is the intensity of the returned
         * echo at the detection point, this value is affected by transmission losses and power levels.
         * NOTES:
         *      1) Sign of Longitude is E positive, W negative
         *      2) Sign of Latitude is N positive, S negative
         */
        /* unsigned int sync; */               /* 0x77F9345A */
        /* unsigned int size; */               /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */          /* "NVUPDATE" */
        unsigned int    version;               /* 4 */
        double          latitude;              /* Latitude from GPS sensor in decimal degrees.
                                                * Set to “-999” if no valid latitude is received. */
        double          longitude;             /* Longitude from GPS sensor in decimal degrees.
                                                * Set to “-999” if no valid longitude is received. */
        float           sog;                   /* Speed over ground in knots.
                                                * Set to “-999” if no valid speed is received. */
        float           cog;                   /* Course over ground in degrees.
                                                * Set to “-999” if no valid course is received. */
        float           heading;               /* Vessel heading in degrees.
                                                * Set to “-999” if no valid heading is received. */
        float           roll;                  /* Vessel roll in degrees.
                                                * Set to “-999” if no valid roll is received. */
        float           pitch;                 /* Vessel pitch in degrees.
                                                * Set to “-999” if no valid pitch is received. */
        float           heave;                 /* Vessel heave in meters.
                                                * Set to “-999” if no valid heave is received. */
        float           nadir_depth;           /* Roll corrected depth below transducer in meters.
                                                * Set to “-999” if no valid depth is received. */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_wcd_navi_struct
	{
        /* WCD_NAVI Record */
        /* Supported Products:
         *   WMB-3250
         *   WMB-3230
         *   WMB-5230 */
        /* Water Column Information
         * This message is sent over the network after each detection message is sent, thus the water
         * column data is valid for the previous ping that has just been received.
         * Notes:
         *      1. The X positions are relative to the vessel position 
         *      2. The Y depths are tide corrected unless tides are disabled in the WASSP system
         *      3. MAG value has no units at this time. The value is the intensity of the returned
         *      echo at the detection point, this value is affected by transmission losses and
         *      power levels. */
        /* unsigned int sync; */               /* 0x77F9345A */
        /* unsigned int size; */               /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */          /* "WCD_NAVI" */
        unsigned int    version;               /* 3 for ICD 2.2, 4 for ICD 2.4 */
        double          latitude;              /* Latitude from GPS sensor in decimal degrees */
        double          longitude;             /* Longitude from GPS sensor in decimal degrees */
        unsigned int    num_points;            /* Number of water column points to follow */
        float           bearing;               /* Bearing of vessel for fish targets, degrees */
        double          msec;                  /* Version 4: unused
                                                * Version 3:
                                                * A millisecond time stamp of rising edge
                                                * of Transmit pulse in UTC time (UTC time
                                                * is calculated from the timestamp of the
                                                * ZDA sentence and or a PPS signal when
                                                * available) No local time zone correction
                                                * is applied.
                                                * On systems not supporting UTC time, this
                                                * will be system referenced time. */
        unsigned int    ping_number;           /* Ping sequence number */
        float           sample_rate;           /* Sampling frequency in Hz for the Water Column Information */
        size_t          wcdata_alloc;          /* Number of points allocated for wcdata arrays */
        float           *wcdata_x;             /* Distance in meters to water column point port/stbd
                                                         * from vessels heading. Negative value is port. */
        float           *wcdata_y;             /* Depth in meters for the water column point. */
        float           *wcdata_mag;           /* Intensity value for water column point, not referenced */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_sensprop_struct
	{
        /* SENSPROP Record */
        /* Supported Products:
         *   WMB-3250
         *   WMB-3230
         *   WMB-5230 */
        /* Sensor Properties
         * These data packets contain WASSP properties.
         * Packets are sent on connection and if any properties are changed.
         * All positions are relative to the vessel’s reference point
         * - see WASSP installation manual. */
        /* unsigned int sync; */                /* 0x77F9345A */
        /* unsigned int size; */                /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */           /* "SENSPROP" */
        unsigned int    version;                /* 1 */
        unsigned int    flags;                  /* Bit field:
                                                 *      Bit 0: Pitch Compensation
                                                 *      Bit 1: Heave Compensation
                                                 *      Bit 2: Roll Correction
                                                 *      Bit 3: Swap Roll
                                                 *      Bit 4: Swap Array
                                                 *      Bit 5: Invert Roll
                                                 *      Bit 6: Induced Heave
                                                 *      Bit 7: PPS applied
                                                 *      Bit 8: Auto Power by signal
                                                 *      Bits 9-31: Reserved for future use */
        float           sea_level_reference;    /* Sea Level Reference
                                                 *  Meters from ship’s reference to sea level,
                                                 *  down is positive */
        float           element_spacing;        /* Element spacing in millimeters */
        unsigned int    spare[8];               /* Spare */
        unsigned int    n;                      /* Number of sensors */
        size_t          n_alloc;                /* Number of sensors allocated */
        struct mbsys_wassp_sensor_struct *sensors; /* Array of sensor details */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_sensor_struct
	{
        unsigned int    sensor_type;            /* Sensor type:
                                                 *      1: Roll
                                                 *      2: Pitch
                                                 *      4: Heave
                                                 *      8: Heading
                                                 *      16: Transducer
                                                 *      32: Position */
        unsigned int    flags;                  /* Bit field:
                                                 *      Bit 0: Device active
                                                 *      Bit 1: Offsets set
                                                 *      Bit 2: Roll Bias set
                                                 *      Bit 3: Pitch Bias set
                                                 *      Bit 4: Yaw Bias set
                                                 *      Bits 5-31: Reserved for future use */
        mb_u_char       port_number;            /* Port number */
        mb_u_char       device;                 /* Device Type which the port number applies to:
                                                 *      0: Unknown
                                                 *      1: WASSP Serial Transfer Task */
        mb_u_char       sentence;               /* Sentence used:
                                                 *      0: n/a
                                                 *      10: GGA
                                                 *      11: GGL
                                                 *      12: GNS
                                                 *      13: RMC
                                                 *      14: GGK (Trimble)
                                                 *      15: ZDA
                                                 *      20: HDM
                                                 *      21: HDT
                                                 *      22: HDG
                                                 *      23: VTG
                                                 *      31: PSXN (Kongsberg)
                                                 *      32: PASHR (Applanix)
                                                 *      33: PFEC,ATT (Furuno)
                                                 *      34: PFEC,HVE (Furuno)
                                                 *      35: RCD (JRC)
                                                 *      100: TSS1
                                                 *      101: Minisense */
        mb_u_char       sensor_model;           /* Sensor model:
                                                 *      0: Unknown
                                                 *      10: WASSP 160
                                                 *      11: WASSP 80
                                                 *      12: WASSP 160 Vs
                                                 *      13: WASSP 160 Vl
                                                 *      40: Applanix POS MV V4
                                                 *      41: Furuno SC30
                                                 *      42: Furuno SC50
                                                 *      43: JRC JLR20
                                                 *      44: CDL Minisense
                                                 *      45: Kongsberg MRU
                                                 *      46: Teledyne TSS
                                                 *      47: Maretron SSC200
                                                 *      48: SMC IMU-108 */
        float           latency;                /* Latency in seconds */
        float           roll_bias;              /* Roll offset, degrees, positive stbd down */
        float           pitch_bias;             /* Pitch offset, degrees, positive bow up */
        float           yaw_bias;               /* Heading offset, degrees, positive stbd */
        float           offset_x;               /* Meters from ship’s reference, fwd is positive */
        float           offset_y;               /* Meters from ship’s reference, stbd is positive */
        float           offset_z;               /* Meters from ship’s reference, down is positive */
        };

struct mbsys_wassp_sys_prop_struct
	{
        /* SYS_PROP Record */
        /* Supported Products:
         *   WMB-3250
         *   WMB-3230
         *   WMB-5230 */
        /* System Properties
         * These data packets contain WASSP properties.
         * Packets are sent on connection and if any properties are changed.
         * All positions are relative to the vessel’s reference point
         * - see WASSP installation manual. */
        /* unsigned int sync; */                /* 0x77F9345A */
        /* unsigned int size; */                /* Size in bytes of this record from start of sync pattern to end of checksum */
        /* char         header[8]; */           /* "SYS_PROP" */
        unsigned int    version;                /* 1 */
        unsigned int    product_type;           /* Product type:
                                                 *      1: WMB-3230
                                                 *      2: WMB-5230
                                                 *      3: WMB-3250 */
        unsigned int    protocol_version;       /* Protocol version x100, currently 240 */
        unsigned int    sw_version[4];          /* Software version:
                                                 * Product Version, Major Version, Minor Version, Build */
        unsigned int    fw_version;             /* Firmware version */
        unsigned int    hw_version;             /* Hardware version */
        unsigned int    transducer_sn;          /* Transducer serial number in ASCII */
        unsigned int    transceiver_sn;         /* Transceiver serial number in ASCII */
        unsigned int    spare[8];               /* Spare */
        unsigned int    checksum;               /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_sys_cfg1_struct
	{
        /* SYS_CFG1 Record */
        /* Undocumented record, probably installation parameters
         * Raw bytes stored and passed on for now */
        size_t          sys_cfg1_data_alloc;    /* Number of bytes allocated to hold the sys_cfg1 record */
        size_t          sys_cfg1_len;           /* Number of bytes stored from the sys_cfg1 record */
        char            *sys_cfg1_data;         /* SYS_CFG1 record stored as raw bytes */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
        };

struct mbsys_wassp_mcomment_struct
	{
        /* MCOMMENT Record */
        /* Comment message
         * This record is defined only for MB-System */
        unsigned int    comment_length;         /* Comment length in bytes */
        char            comment_message[MB_COMMENT_MAXLINE];
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
       };

struct mbsys_wassp_unknown1_struct
	{
        /* Unknown record
         * Raw bytes stored and passed on for now */
        size_t          unknown1_data_alloc;    /* Number of bytes allocated to hold the unknown1 record */
        size_t          unknown1_len;           /* Number of bytes stored from the unknown1 record */
        char            *unknown1_data;         /* unknown1 record stored as raw bytes */
        unsigned int    checksum;              /* 0x8806CBA5 (not really a checksum, actually a sync value) */
       };

/* Data system structure */
struct mbsys_wassp_struct
	{
	/* Type of most recently read data record */
	int		kind;			/* MB-System record ID */

	/* MB-System time stamp of most recently read record */
	double		time_d;
	int		time_i[7];

        /* GENBATHY record */
        struct mbsys_wassp_genbathy_struct genbathy;

        /* RAWSONAR record */
        struct mbsys_wassp_rawsonar_struct rawsonar;

        /* GEN_SENS record */
        struct mbsys_wassp_gen_sens_struct gen_sens;

        /* CORBATHY record */
        struct mbsys_wassp_corbathy_struct corbathy;

        /* NVUPDATE record */
        struct mbsys_wassp_nvupdate_struct nvupdate;

        /* WCD_NAVI record */
        struct mbsys_wassp_wcd_navi_struct wcd_navi;

        /* SENSPROP record */
        struct mbsys_wassp_sensprop_struct sensprop;

        /* SYS_PROP record */
        struct mbsys_wassp_sys_prop_struct sys_prop;

        /* SYS_CFG1 record */
        struct mbsys_wassp_sys_cfg1_struct sys_cfg1;
        
        /* MCOMMENT Record */
        struct mbsys_wassp_mcomment_struct mcomment;

        /* unknown record */
        struct mbsys_wassp_unknown1_struct unknown1;
 	};
        
/*---------------------------------------------------------------*/

/* System specific function prototypes */
int mbsys_wassp_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_wassp_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_wassp_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_wassp_pingnumber(int verbose, void *mbio_ptr,
			int *pingnumber, int *error);
int mbsys_wassp_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
                        int *sonartype, int *error);
//int mbsys_wassp_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *ss_type, int *error);
//int mbsys_wassp_preprocess(int verbose, void *mbio_ptr, void *store_ptr,
//                        double time_d, double navlon, double navlat,
//                        double speed, double heading, double sonardepth,
//                        double roll, double pitch, double heave,
//                        int *error);
int mbsys_wassp_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_wassp_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_wassp_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
//int mbsys_wassp_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
//			int nmax, int *kind, int *n,
//			int *time_i, double *time_d,
//			double *navlon, double *navlat,
//			double *speed, double *heading, double *draft,
//			double *roll, double *pitch, double *heave,
//			int *error);
int mbsys_wassp_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_wassp_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
//int mbsys_wassp_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
//                        double transducer_depth, double altitude,
//                        int *error);
int mbsys_wassp_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_wassp_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_wassp_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_wassp_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
//int mbsys_wassp_pulses(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *kind, int *nbeams, int *pulses, int *error);
int mbsys_wassp_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
//int mbsys_wassp_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_wassp_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_wassp_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind,
//			void *segytraceheader_ptr,
//			int *error);
//int mbsys_wassp_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int *sampleformat,
//			int *kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_wassp_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_wassp_ctd(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nctd, double *time_d,
//			double *conductivity, double *temperature,
//			double *depth, double *salinity, double *soundspeed, int *error);
//int mbsys_wassp_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nsensor, double *time_d,
//			double *sensor1, double *sensor2, double *sensor3,
//			double *sensor4, double *sensor5, double *sensor6,
//			double *sensor7, double *sensor8, int *error);
int mbsys_wassp_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
/*---------------------------------------------------------------*/
