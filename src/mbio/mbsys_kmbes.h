/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_kmbes.h	5/25/2018
 *
 *    Copyright (c) 2018-2023 by
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
 * mbsys_kmbes.h defines the MBIO data structures for handling data from
 * the following data formats:
 *    MBSYS_KMBES formats (code in mbsys_kmbes.c and mbsys_kmbes.h):
 *      MBF_KEMMBES : MBIO ID 261 (code in mbr_kemmbes.c)
 *
 * Author:	B. Y. Raanan
 * Date:	May 25, 2018
 *
 *
 */
/*
 * Notes on the mbsys_kmbes data structure and associated format:
 *   1. This MBIO format supports the generic interface message output
 *      by the Konsberg EM datagram format.
 *   2. Reference: Konsberg EM datagram format Reg.no. 410224 rev F, November 2018
 *   3. The Konsberg EM datagram stream consists of several different data
 *      records that can vary among models and installations
 *   4. The Konsberg EM datagram format supports the following multibeam models (September 2016):
 *      EM 710,   40-100 kHz
 *      EM 712,   40-100 kHz
 *      EM 2040,  200/300/400 kHz
 *      EM 2040C, 180-400 kHz
 *   5. Each datagram begins with a 20 byte sequence:
 *      unsigned int numBytesDgm;       // Datagram length in bytes.
 *      mb_u_char dgmType[4];           // Multi beam datagram type definition, e.g. #AAA
 *      mb_u_char dgmVersion;           // Datagram version.
 *      mb_u_char systemID;             // System ID.
 *      unsigned short echoSounderID;   // Echo sounder identity.
 *      unsigned int time_sec;          // Time in second.
 *      unsigned int time_nanosec;      // Nano seconds remainder.
 *   5. Each datagram ends with:
 *      unsigned int numBytesDgm;       // Datagram length in bytes.
 *   6. All data are in little-endian form.
 *   7. The data record names include:
 *      // I - datagrams (Installation and Runtime)
 *      IIP, // Info Installation PU
 *      IOP, // Runtime datagram
 *      IBE, // BIST error report
 *      IBR, // BIST reply
 *      IBS, // BIST short reply
 *
 *      // S-datagrams (Sensors)
 *      SPO, // Sensor POsition data
 *      SKM, // KM binary sensor data
 *      SVP, // Sound Velocity Profile
 *      SVT, // Sensor data for sound Velocity at Transducer
 *      SCL, // Sensor CLock data
 *      SDE, // Sensor DEpth data
 *      SHI, // Sensor HeIght data
 *      SHA, // Sensor HeAding data
 *
 *      // M-datagrams (Multibeam)
 *      MRZ, // Multibeam data for raw range, depth, reflectivity, seabed image(SI) etc.
 *      MWC, // Water column multibeam data
 *
 *      // C-datagrams (Compatibility)
 *      CHE, // Compatibility heave data - store raw sensor data without modification
 *      CPO, // Compatibility position data - store raw sensor data without modification
 *
 *      // F-datagrams (Files)
 *      FCF, // Backscatter calibration (C) file (F) datagram
 *
 *      // X-datagrams (eXtra - defined only for MB-System)
 *      XMB, // The presence of this datagram indicates this file/stream has been
 *           // written by MB-System.
 *           // - this means that pings include a sidescan datagram XMS after the MRZ datagrams
 *           // - this means that MB-System beamflags are embedded in the MRZ datagram soundings
 *      XMC, // Comment datagram (MB-System only)
 *      XMS, // MB-System multibeam pseudosidescan derived from multibeam backscatter (MB-System only)
 */

#ifndef MBSYS_KMBES_H_
#define MBSYS_KMBES_H_

#include "mb_define.h"

/*---------------------------------------------------------------*/
/* Datagram ID definitions */
/* I - datagrams */
#define MBSYS_KMBES_I_INSTALLATION_PARAM        "#IIP" // Installation parameters and sensor setup.
#define MBSYS_KMBES_I_OP_RUNTIME                "#IOP" // Runtime parameters as chosen by operator.
#define MBSYS_KMBES_I_BE_BIST                   "#IBE" // Built in test (BIST) error report.
#define MBSYS_KMBES_I_BR_BIST                   "#IBR" // Built in test (BIST) reply.
#define MBSYS_KMBES_I_BS_BIST                   "#IBS" // Built in test (BIST) short reply.

/* S-datagrams */
#define MBSYS_KMBES_S_POSITION                  "#SPO" // Sensor POsition data
#define MBSYS_KMBES_S_KM_BINARY                 "#SKM" // KM binary sensor data
#define MBSYS_KMBES_S_SOUND_VELOCITY_PROFILE    "#SVP" // Sound Velocity Profile
#define MBSYS_KMBES_S_SOUND_VELOCITY_TRANSDUCER "#SVT" // Sensor data for sound Velocity at Transducer
#define MBSYS_KMBES_S_CLOCK                     "#SCL" // Sensor CLock datagram
#define MBSYS_KMBES_S_DEPTH                     "#SDE" // Sensor DEpth data
#define MBSYS_KMBES_S_HEIGHT                    "#SHI" // Sensor HeIght data
#define MBSYS_KMBES_S_HEADING                   "#SHA" // Sensor HeAding

/* M-datagrams */
#define MBSYS_KMBES_M_RANGE_AND_DEPTH           "#MRZ" // Multibeam data for raw range, depth, reflectivity, seabed image(SI) etc.
#define MBSYS_KMBES_M_WATER_COLUMN              "#MWC" // Multibeam water column datagram.

/* C-datagrams */
#define MBSYS_KMBES_C_POSITION                  "#CPO"
#define MBSYS_KMBES_C_HEAVE                     "#CHE"

/* F-datagrams */
#define MBSYS_KMBES_F_BSCALIBRATIONFILE         "#FCF"

/* X-datagrams */
#define MBSYS_KMBES_X_MBSYSTEM                  "#XMB" // Indicates these data written by MB-System (MB-System only)
#define MBSYS_KMBES_X_COMMENT                   "#XMC" // Comment datagram (MB-System only)
#define MBSYS_KMBES_X_EXTENSION                 "#XMT" // MB-System corrected navigation, attitude, beam travel times, and angles (MB-System only)
#define MBSYS_KMBES_X_PSEUDOSIDESCAN            "#XMS" // Multibeam pseudosidescan derived from multibeam backscatter (MB-System only)

#define MBSYS_KMBES_SYNC_CHAR 0x23  // ascii "#"
#define MBSYS_KMBES_QUAL_FACTOR_THRESHOLD 50

/*---------------------------------------------------------------*/
/* Record size definitions (if needed for use in data reading and writing) */
#define MBSYS_KMBES_START_BUFFER_SIZE 64000 // udp packet max is 64kbyte, but the KMBES K-Controller may concat a number of packets
#define MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE 4096
#define MBSYS_KMBES_HEADER_SIZE 20
#define MBSYS_KMBES_PARITION_SIZE 4
#define MBSYS_KMBES_END_SIZE 4
#define MBSYS_KMBES_MAX_SPO_DATALENGTH 250
#define MBSYS_KMBES_MAX_ATT_DATALENGTH 250
#define MBSYS_KMBES_MAX_SVT_DATALENGTH 64
#define MBSYS_KMBES_MAX_SCL_DATALENGTH 64
#define MBSYS_KMBES_MAX_SDE_DATALENGTH 32
#define MBSYS_KMBES_MAX_SHI_DATALENGTH 32
#define MBSYS_KMBES_MAX_SHA_DATALENGTH 32
#define MBSYS_KMBES_MAX_CPO_DATALENGTH 250
#define MBSYS_KMBES_MAX_CHE_DATALENGTH 64
#define MBSYS_KMBES_MAX_IIP_DATALENGTH 4096
#define MBSYS_KMBES_MAX_IOP_DATALENGTH 4096
#define MBSYS_KMBES_SPO_VAR_OFFSET 72
#define MBSYS_KMBES_SCL_VAR_OFFSET 36
#define MBSYS_KMBES_SDE_VAR_OFFSET 40
#define MBSYS_KMBES_SHI_VAR_OFFSET 40
#define MBSYS_KMBES_CPO_VAR_OFFSET 72
#define MBSYS_KMBES_IIP_VAR_OFFSET 30
#define MBSYS_KMBES_IOP_VAR_OFFSET 30
#define MBSYS_KMBES_XMT_PINGINFO_DATALENGTH 60
#define MBSYS_KMBES_XMT_SOUNDING_DATALENGTH 24

/*---------------------------------------------------------------*/
/* Array size definitions (if needed for use in data reading and writing) */
#define MBSYS_KMBES_MAX_NUM_BEAMS 1024
#define MBSYS_KMBES_MAX_PIXELS 2048
#define MBSYS_KMBES_MAX_EXTRA_DET 1024
#define MBSYS_KMBES_MAX_EXTRA_DET_CLASSES 11
#define MBSYS_KMBES_MAX_SIDESCAN_SAMP 60000
#define MBSYS_KMBES_MAX_SIDESCAN_EXTRA_SAMP 15000
#define MBSYS_KMBES_MAX_NUM_TX_PULSES 9
#define MBSYS_KMBES_MAX_ATT_SAMPLES  148
#define MBSYS_KMBES_MAX_SVP_POINTS 2000
#define MBSYS_KMBES_MAX_SVT_SAMPLES 1
#define MBSYS_KMBES_MAX_HEADING_SAMPLES 1000
#define MBSYS_KMBES_MAX_NUM_MST_DGMS 256
#define MBSYS_KMBES_MAX_NUM_MWC_DGMS 256
#define MBSYS_KMBES_MAX_NUM_MRZ_DGMS 32
#define MBSYS_KMBES_MAX_F_FILENAME_LENGTH 64
#define MBSYS_KMBES_MAX_F_FILE_SIZE 63000

// number of datagrams to add when index table size limit reached (dynamic allocation)

/*---------------------------------------------------------------*/
/* Other definitions */
#define MBSYS_KMBES_UNAVAILABLE_POSFIX 0xffff
#define MBSYS_KMBES_UNAVAILABLE_LATITUDE 200.0f
#define MBSYS_KMBES_UNAVAILABLE_LONGITUDE 200.0f
#define MBSYS_KMBES_UNAVAILABLE_SPEED -1.0f
#define MBSYS_KMBES_UNAVAILABLE_COURSE -4.0f
#define MBSYS_KMBES_UNAVAILABLE_ELLIPSOIDHEIGHT -999.0f

#define MBSYS_KMBES_NANO pow(10.0, -9.0)

/* invalid value flags */
#define MBSYS_KMBES_INVALID_AMP     0x7F
#define MBSYS_KMBES_INVALID_SS      0x7FFF
#define MBSYS_KMBES_INVALID_CHAR    0xFF
#define MBSYS_KMBES_INVALID_SHORT   0xFFFF
#define MBSYS_KMBES_INVALID_U_INT   0xFFFFFFFF
#define MBSYS_KMBES_INVALID_INT     0x7FFFFFFF

/*---------------------------------------------------------------*/

// Enumerate EM datagram types
typedef enum {
    /* unknown datagram */
    UNKNOWN,

    /* I - datagrams */
    IIP, // EM_DGM_I_INSTALLATION_PARAM
    IOP, // EM_DGM_I_OP_RUNTIME
    IBE, // EM_DGM_I_BE_BIST
    IBR, // EM_DGM_I_BR_BIST
    IBS, // EM_DGM_I_BS_BIST

    /* S-datagrams */
    SPO, // EM_DGM_S_POSITION
    SKM, // EM_DGM_S_KM_BINARY
    SVP, // EM_DGM_S_SOUND_VELOCITY_PROFILE
    SVT, // EM_DGM_S_SOUND_VELOCITY_TRANSDUCER
    SCL, // EM_DGM_S_CLOCK
    SDE, // EM_DGM_S_DEPTH
    SHI, // EM_DGM_S_HEIGHT
    SHA, // EM_DGM_S_HEADING

    /* M-datagrams */
    MRZ, // EM_DGM_M_RANGE_AND_DEPTH
    MWC, // EM_DGM_M_WATER_COLUMN

    /* C-datagrams */
    CPO, // EM_DGM_C_POSITION
    CHE, // EM_DGM_C_HEAVE

    /* F-datagrams */
    FCF, // EM_DGM_F_CALIBRATION

    // X-datagrams (extra - defined only for MB-System)
    XMB, // The presence of this datagram indicates this file/stream has been
         // written by MB-System.
         // - this means that pings include a corrected navigation, attitude, travel
         //   time, and pointing angle datagram XMT after rthe MRZ datagrams
         // - this means that pings include a sidescan datagram XMS after the MRZ datagrams
         // - this means that MB-System beamflags are embedded in the MRZ datagram soundings
    XMT, // Corrected/interpolated navigation, attitude, travel time, and
         //   pointing angle datagram (MB-System only)
    XMC, // Comment datagram (MB-System only)
    XMS  // MB-System multibeam pseudosidescan derived from multibeam backscatter (MB-System only)

} mbsys_kmbes_emdgm_type;

/*---------------------------------------------------------------*/
/* Structure size definitions (if needed because there are dynamically allocated substructures) */

/*********************************************

   General datagram header

 *********************************************/

struct mbsys_kmbes_header {
    /* Definition of general datagram header */
    unsigned int numBytesDgm;       /* Datagram length in bytes.
                                     * The length field at the start (4 bytes) and end of the datagram (4 bytes)
                                     * are included in the length count. */
    mb_u_char dgmType[4];           /* Multi beam datagram type definition, e.g. #AAA */
    mb_u_char dgmVersion;           /* Datagram version. */
    mb_u_char systemID;             /* System ID. Parameter used for separating datagrams from different echosounders
                                     * if more than one system is connected to SIS/K-Controller. */
    unsigned short echoSounderID;   /* Echo sounder identity, e.g. 122, 302, 710, 712, 2040, 2045, 850. */
    unsigned int time_sec;          /* Time in second. Epoch 1970-01-01. Ignoring leap second. time_nanosec part to
                                     * be added for more exact time. */
    unsigned int time_nanosec;      /* Nano seconds remainder. time_nanosec part to be added to time_sec for more exact time. */
};

/*********************************************

    Sensor datagrams

  *********************************************/

struct mbsys_kmbes_s_common {
    /* Sensor (S) output datagram - common part for all external sensors. */
    unsigned short numBytesCmnPart;     /* Size in bytes of current struct.
                                         * Used for denoting size of rest of datagram in cases where only one datablock is attached. */
    unsigned short sensorSystem;        /* Sensor system number, as indicated when setting up the system in K-Controller installation menu. */
    unsigned short sensorStatus;        /* Sensor quality status. To indicate if sensor data is valid or invalid.
                                         * Numerical or bit code with varying content according to type of sensor.
                                         * See specification under each main datagrams struct. */
    unsigned short padding;
};

struct mbsys_kmbes_s_data_info {
    /* Information of repeated sensor data in one datagram.
     * Info about data from sensor. Part included if data from sensor appears multiple times in a datagram. */
    unsigned short numBytesInfoPart;        /* Size in bytes of current struct. */
    unsigned short numSamplesArray;         /* Number of sensor samples added in datagram. */
    unsigned short numBytesPerSample;       /* Length in bytes of one whole sample (decoded and raw data). */
    unsigned short numBytesRawSensorData;   /* Length in bytes of raw sensor data. */
};

/************************************
    #SPO - Sensor POsition data
  ************************************/

struct mbsys_kmbes_spo_data_block {
    /* #SPO - Sensor position data block. Data given both decoded and corrected,
     * and raw as received from sensor in text string. */
    unsigned int timeFromSensor_sec;        /* Time from position sensor. Unit seconds. Epoch 1970-01-01.
                                             * Ignoring leap seconds. Nanosec part to be added for more exact time. */
    unsigned int timeFromSensor_nanosec;    /* Time from position sensor. Unit nano seconds remainder. */
    float posFixQuality_m;                  /* Only if available as input from sensor. Calculation according to format. */
    double correctedLat_deg;                /* Motion corrected (if enabled in K-Controller) data as used in depth
                                             * calculations. Referred to vessel reference point. Unit decimal degree. */
    double correctedLong_deg;               /* Motion corrected (if enabled in K-Controller) data as used in depth
                                             * calculations. Referred to vessel reference point. Unit decimal degree.*/
    float speedOverGround_mPerSec;          /* Speed over ground. Unit m/s. If unavailable, value set to 0xffff = 65535. */
    float courseOverGround_deg;             /* Course over ground. Unit degree. If unavailable, value set to 0xffff = 65535. */
    float ellipsoidHeightReRefPoint_m;      /* Height of vessel reference point above the ellipsoid. Data from GGA/GGK
                                             * is corrected data for position system installation parameters.
                                             * Data is also corrected for motion ( roll and pitch only) if enabled by
                                             * K-Controller operator. Unit meter. */
    char posDataFromSensor[MBSYS_KMBES_MAX_SPO_DATALENGTH]; /* Position data as received from sensor, i.e.
                                                             * uncorrected for motion etc. */
};

struct mbsys_kmbes_spo {
    /* #SPO - Struct of position sensor datagram. */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_spo_data_block sensorData;
};

 #define MBSYS_KMBES_SPO_VERSION 0

/************************************
   #SKM - KM binary sensor data
 ************************************/

struct mbsys_kmbes_skm_info {
    /* Sensor (S) output datagram - info of KMB datagrams. */
    unsigned short numBytesInfoPart;    /* Size in bytes of current struct. Used for denoting size of rest of datagram
                                         * in cases where only one datablock is attached. */
    mb_u_char sensorSystem;             /* Attitude system number, as numbered in installation parameters.
                                         * System 1 or system 2. */
    mb_u_char sensorStatus;             /* Sensor status. */
    unsigned short sensorInputFormat;   /* Format of raw data from input sensor, given in numerical code:
                                         * 1	KM binary Sensor Input
                                         * 2	EM 3000 data
                                         * 3	Sagem
                                         * 4	Seapath binary 11
                                         * 5	Seapath binary 23
                                         * 6	Seapath binary 26
                                         * 7	POS M/V GRP 102/103
                                         * 8	Coda Octopus MCOM */
    unsigned short numSamplesArray;     /* Number of KM binary sensor samples added in this datagram. */
    unsigned short numBytesPerSample;   /* Length in bytes of one whole KM binary sensor sample. */
    unsigned short sensorDataContents;  /* Field to indicate which information is available from the input sensor,
                                         * at the given sensor format.
                                         *      0 = not available
                                         *      1 = data is available
                                         * The bit pattern is used to determine sensorStatus from status field in
                                         * #KMB samples. Only data available from sensor is check up against
                                         * invalid/reduced performance in status, and summaries in sensorStatus. */
};


struct mbsys_kmbes_skm_binary {
    /* #SKM - Sensor attitude data block. Data given timestamped, not corrected. */
    mb_u_char dgmType[4];
    unsigned short numBytesDgm;     /* Datagram length in bytes. The length field at the start (4 bytes) and end of
                                     * the datagram (4 bytes) are included in the length count. */
    unsigned short dgmVersion;      /* Datagram version. */
    unsigned int time_sec;          /* Time from inside KM sensor data. Unit second. Epoch 1970-01-01 time.
                                     * Ignoring leap second. Nanosec part to be added for more exact time.
                                     * If time is unavailable from attitude sensor input, time of reception on serial
                                     * port is added to this field. */
    unsigned int time_nanosec;      /* Nano seconds remainder. Nanosec part to be added to time_sec for more exact time.
                                     * If time is unavailable from attitude sensor input, time of reception on serial
                                     * port is added to this field. */
    unsigned int status;            /* Bit pattern for indicating validity of sensor data, and reduced performance.
                                     * The status word consists of 32 single bit flags numbered from 0 to 31, where 0
                                     * is the least significant bit.
                                     * Bit number 0-7 indicate if from a sensor data is invalid.
                                     *   0 = valid data, 1 = invalid data.
                                     *        Bit number 	Sensor data --—
                                     *        0 	Horizontal position and velocity
                                     *        1 	Roll and pitch
                                     *        2 	Heading
                                     *        3 	Heave and vertical velocity
                                     *        4 	Acceleration
                                     *        5 	Error fields
                                     *        6 	Delayed heave

                                     * Bit number 16-> indicate if data from sensor has reduced performance.
                                     *   0 = valid data, 1 = reduced performance.
                                     *        Bit number 	Sensor data --—
                                     *        16 	Horizontal position and velocity
                                     *        17 	Roll and pitch
                                     *        18 	Heading
                                     *        19 	Heave and vertical velocity
                                     *        20 	Acceleration
                                     *        21 	Error fields
                                     *        22 	Delayed heave */

    /* Position */
    double latitude_deg;        /* Position in decimal degrees. */
    double longitude_deg;       /* Position in decimal degrees. */
    float ellipsoidHeight_m;    /* Height of sensor reference point above the ellipsoid. Positive above ellipsoid.
                                 * ellipsoidHeight_m is not corrected for motion and installation offsets of the
                                 * position sensor. */

    /* Attitude */
    float roll_deg;     /* Roll. Unit degree. */
    float pitch_deg;    /* Pitch. Unit degree. */
    float heading_deg;  /* Heading of vessel. Unit degree. Relative to the fixed coordinate system, i.e. true north. */
    float heave_m;      /* Heave. Unit meter. Positive downwards. */

    /* Rates */
    float rollRate;     /* Roll rate. Unit degree/s */
    float pitchRate;    /* Pitch rate. Unit degree/s */
    float yawRate;      /* Yaw (heading) rate. Unit degree/s */

    /* Velocities */
    float velNorth;     /* Velocity North (X). Unit m/s */
    float velEast;      /* Velocity East (Y). Unit m/s */
    float velDown;      /* Velocity downwards (Z). Unit m/s */

    /* Errors in data. Sensor data quality, as standard deviations. */
    float latitudeError_m;          /* Latitude error. Unit meter. */
    float longitudeError_m;         /* Longitude error. Unit meter. */
    float ellipsoidHeightError_m;   /* Ellipsoid height error. Unit meter. */
    float rollError_deg;            /* Roll error. Unit degree. */
    float pitchError_deg;           /* Pitch error. Unit degree. */
    float headingError_deg;         /* Heading error. Unit degree. */
    float heaveError_m;             /* Heave error. Unit degree. */

    /* Acceleration */
    float northAcceleration;    /* Unit m/s^2 */
    float eastAcceleration;     /* Unit m/s^2 */
    float downAcceleration;     /* Unit m/s^2 */
};

struct mbsys_kmbes_skm_delayed_heave {
    /* #SKM - delayed heave. Included if available from sensor. */
    unsigned int time_sec;
    unsigned int time_nanosec;
    float delayedHeave_m;       /* Delayed heave. Unit meter. */
};

struct mbsys_kmbes_skm_sample {
    /* #SKM - all available data. */
    struct mbsys_kmbes_skm_binary KMdefault;
    struct mbsys_kmbes_skm_delayed_heave delayedHeave;
};

struct mbsys_kmbes_skm {
    /* #SKM - data from attitude and attitude velocity sensors. */
    /* Datagram may contain several sensor measurements. The number of samples in datagram is listed in
     * numSamplesArray in the struct mbsys_kmbes_skm_info. Time given in datagram header, is time of arrival
     * of data on serial line or on network. Time inside #KMB sample is time from the sensors data. If input
     * is other than KM binary sensor input format, the data are converted to the KM binary format by the PU.
     * All parameters are uncorrected.
     * For processing of data, installation offsets, installation angles and attitude values are needed to
     * correct the data for motion. */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_skm_info infoPart;
    struct mbsys_kmbes_skm_sample sample[MBSYS_KMBES_MAX_ATT_SAMPLES];
};

 #define MBSYS_KMBES_SKM_VERSION 1

/************************************
     #SVP - Sound Velocity Profile
  ************************************/

struct mbsys_kmbes_svp_point {
    /* #SVP - Sound Velocity Profile. Data from sound velocity profile or from CTD profile.
     * Sound velocity is measured directly or estimated, respectively. */
    float depth_m;                  /* Depth at which measurement is taken. Unit m.
                                     * Valid range from 0.00 m to 12000 m. */
    float soundVelocity_mPerSec;    /* Measured sound velocity from profile. Unit m/s.
                                     * For a CTD profile, this will be the calculated sound velocity.*/
    unsigned int padding;
    float temp_C;                   /* Water temperature at given depth. Unit Celsius.
                                     * For a Sound velocity profile (S00), this will be set to 0.00. */
    float salinity;                 /* Salinity of water at given depth.
                                     * For a Sound velocity profile (S00), this will be set to 0.00. */
};

struct mbsys_kmbes_svp {
    /* #SVP - Sound Velocity Profile. */
    /* Data from sound velocity profile or from CTD profile.
     * Sound velocity is measured directly or estimated, respectively. */
    struct mbsys_kmbes_header header;
    unsigned short numBytesCmnPart; /* Size in bytes of body part struct. Used for denoting size of rest of datagram. */
    unsigned short numSamples;      /* Number of sound velocity samples. */
    mb_u_char sensorFormat[4];      /* Sound velocity profile format:
                                     * 'S00' = sound velocity profile
                                     * 'S01' = CTD profile */
    unsigned int time_sec;          /* Time extracted from the Sound Velocity Profile.
                                     * Parameter is set to zero if not found. */
    double latitude_deg;            /* Latitude in degrees. Negative if southern hemisphere.
                                     * Position extracted from the Sound Velocity Profile.
                                     * Parameter is set to 200 if not found. */
    double longitude_deg;           /* Longitude in degrees. Negative if western hemisphere.
                                     * Position extracted from the Sound Velocity Profile.
                                     * Parameter is set to 200 if not found. */
    struct mbsys_kmbes_svp_point sensorData[MBSYS_KMBES_MAX_SVP_POINTS];
    /* SVP point samples, repeated numSamples times. */
};

 #define MBSYS_KMBES_SVP_VERSION 1

/************************************
 * #SVT - Sensor sound Velocity measured at Transducer
 ************************************/
struct mbsys_kmbes_svt_info
{
    /* Part of Sound Velocity at Transducer datagram. */
    unsigned short numBytesInfoPart;    /* Size in bytes of current struct. */
    unsigned short sensorStatus;        /* Sensor status. To indicate quality of sensor data is valid or invalid.
                                         * Quality may be invalid even if sensor is active and the PU receives data.
                                         * Bit code vary according to type of sensor.
                                         *
                                         * Bits 0 -7 common to all sensors and #MRZ sensor status:
                                         *
                                         *      Bit number	Sensor data ---
                                         *      0	        0 = Data OK
                                         *                  1 = Data OK and sensor is chosen as active
                                         *                  #SCL only: 1 = Valid data and 1PPS OK
                                         *      1	        0
                                         *      2	        0 = Data OK
                                         *                  1 = Reduced performance
                                         *                  #SCL only: 1 = Reduced performance, no time synchronisation of PU
                                         *      3	        0
                                         *      4	        0 = Data OK
                                         *                  1 = Invalid data
                                         *      5	        0
                                         *      6	        0 = Velocity from sensor
                                         *                  1 = Velocity calculated by PU */
    unsigned short sensorInputFormat;   /* Format of raw data from input sensor, given in numerical code according to
                                         * table below:
                                         *      Code	Sensor format
                                         *      1	    AML NMEA
                                         *      2	    AML SV
                                         *      3	    AML SVT
                                         *      4	    AML SVP
                                         *      5	    Micro SV
                                         *      6	    Micro SVT
                                         *      7	    Micro SVP
                                         *      8	    Valeport MiniSVS
                                         *      9 	  KSSIS 80
                                         *      10 	  KSSIS 43 */
    unsigned short numSamplesArray;     /* Number of sensor samples added in this datagram. */
    unsigned short numBytesPerSample;   /* Length in bytes of one whole SVT sensor sample. */
    unsigned short sensorDataContents;  /* Field to indicate which information is available from the input sensor, at
                                         * the given sensor format:
                                         *      0 = not available
                                         *      1 = data is available
                                         * Expected data field in sensor input:
                                         *      Bit number	Sensor data ---
                                         *      0	        Sound Velocity
                                         *      1	        Temperature
                                         *      2	        Pressure
                                         *      3	        Salinity */
    float filterTime_sec;               /* Time parameter for moving median filter. Unit seconds */
    float soundVelocity_mPerSec_offset; /* Offset for measured sound velocity set in K-Controller. Unit m/s */
};

struct mbsys_kmbes_svt_sample
{
    unsigned int time_sec;              /* Time in second. Epoch 1970-01-01. */
    unsigned int time_nanosec;          /* Nano seconds remainder. Add to time_sec for more exact time. */
    float soundVelocity_mPerSec;        /* Measured sound velocity from sound velocity probe. Unit m/s. */
    float temp_C;                       /* Water temperature from sound velocity probe. Unit Celsius. */
    float pressure_Pa;                  /* Pressure. Unit Pascal. */
    float salinity;                     /* Salinity of water. Measured in g salt/kg sea water */
};

struct mbsys_kmbes_svt
{
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_svt_info infoPart;
    struct mbsys_kmbes_svt_sample sensorData[MBSYS_KMBES_MAX_SVT_SAMPLES];
};

#define MBSYS_KMBES_SVT_VERSION 0

/************************************
     #SCL - Sensor CLock datagram
  ************************************/

struct mbsys_kmbes_scl_data_from_sensor {
    /* Part of clock datagram giving offsets and the raw input in text format. */
    float offset_sec;           /* Offset in seconds from K-Controller operator input. */
    int clockDevPU_nanosec;     /* Clock deviation from PU. Difference between time stamp at receive of sensor
                                 * data and time in the clock source. Unit nanoseconds.
                                 * Difference smaller than +/- 1 second if 1PPS is active and sync from ZDA. */
    mb_u_char dataFromSensor[MBSYS_KMBES_MAX_SCL_DATALENGTH];   /* Clock data as received from sensor, in text format.
                                                                 * Data are uncorrected for offsets. */
};

struct mbsys_kmbes_scl {
    /* #SCL - CLock datagram. */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_scl_data_from_sensor sensorData;
};

 #define MBSYS_KMBES_SCL_VERSION 0

/************************************
     #SDE - Sensor DEpth data
  ************************************/

struct mbsys_kmbes_sde_data_from_sensor {
    /* Part of depth datagram giving depth as used, offsets, scale factor and data as
     * received from sensor (uncorrected). */
    float depthUsed_m;      /* Depth as used. Corrected with installation parameters. Unit meter. */
    float offset;           /* Offset used measuring this sample. */
    float scale;            /* Scaling factor for depth. */
    double latitude_deg;    /* Latitude in degrees. Negative if southern hemisphere. Position extracted from the
                             * Sound Velocity Profile. Parameter is set to 200 if not available from sensor. */
    double longitude_deg;   /* Longitude in degrees. Negative if western hemisphere. Position extracted from the
                             * Sound Velocity Profile. Parameter is set to 200 if not if not available from sensor. */
    mb_u_char dataFromSensor[MBSYS_KMBES_MAX_SDE_DATALENGTH];
};

struct mbsys_kmbes_sde {
    /* #SDE - DEpth datagram. */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_sde_data_from_sensor sensorData;
};

 #define MBSYS_KMBES_SDE_VERSION 0

/************************************
    #SHI - Sensor HeIght data
 ************************************/

struct mbsys_kmbes_shi_data_from_sensor {
    /* Part of Height datagram, giving corrected and uncorrected data as received from sensor. */
    unsigned short sensorType;
    float heigthUsed_m;         /* Height corrected using installation parameters, if any. Unit meter. */
    mb_u_char dataFromSensor[MBSYS_KMBES_MAX_SHI_DATALENGTH];
};

struct mbsys_kmbes_shi {
    /* #SHI - Height datagram. */
    struct mbsys_kmbes_header  header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_shi_data_from_sensor sensorData;
};

 #define MBSYS_KMBES_SHI_VERSION 0

/************************************
    #SHA - Sensor HeAding
 ************************************/

struct mbsys_kmbes_sha_data_from_sensor {
    /* Part of Heading datagram, giving corrected and uncorrected data as received from sensor. */
    unsigned int timeSinceRecStart_nanosec; /* Offset between heading data sample and time in datagram header.
                                             * To get absolute time, add header time and timeSinceRecStart_nanosec.
                                             * Time in header is uncorrected for offsets. */
    float headingCorrected_deg;             /* Heading corrected using installation parameters. Unit degree. */
    mb_u_char dataFromSensor[MBSYS_KMBES_MAX_SHA_DATALENGTH];   /* Heading as received from sensor. */
};

struct mbsys_kmbes_sha {
    /* #SHA - Heading */
    /* Heading from separated heading sensor, e.g. gyro compass. */
    struct mbsys_kmbes_header  header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_s_data_info dataInfo;
    struct mbsys_kmbes_sha_data_from_sensor sensorData[MBSYS_KMBES_MAX_HEADING_SAMPLES];
};

 #define MBSYS_KMBES_SHA_VERSION 0

/*********************************************

   Multibeam datagrams

 *********************************************/

struct mbsys_kmbes_m_partition {
    /* Multibeam (M) datagrams - data partition information. General for all M datagrams. */
    /* If a multibeam depth datagram (or any other large datagram) exceeds the limit of an UPD package (64 kB),
     * the datagram is split into several datagrams =< 64 kB before sending from the PU. The parameters in
     * this struct will give information of the partitioning of datagrams. K-controller/SIS re-joins all UDP
     * packets/datagram parts to one datagram, and store it as one datagram in the .kmall files. Datagrams stored
     * in .kmall files will therefore always have numOfDgm = 1 and dgmNum = 1, and may have size > 64 kB. */
    unsigned short numOfDgms;   /* Number of datagram parts to re-join to get one Multibeam datagram. E.g. 3. */
    unsigned short dgmNum;      /* Datagram part number, e.g. 2 (of 3). */
};

struct mbsys_kmbes_m_body {
    /* Multibeam (M) datagrams - body part. Start of body of all M datagrams. */
    /* Contains information of transmitter and receiver used to find data in datagram. */
    unsigned short numBytesCmnPart; /* Used for denoting size of current struct, mbsys_kmbes_m_body. */
    unsigned short pingCnt;         /* A ping is made of one or more RX fans and one or more TX pulses transmitted
                                     * at approximately the same time. Ping counter is incremented at every set of
                                     * TX pulses (one or more pulses transmitted at approximately the same time). */
    mb_u_char rxFansPerPing;        /* Number of rx fans per ping gives information of how many #MRZ datagrams are
                                     * generated per ping. Combined with swathsPerPing, number of datagrams to join
                                     * for a complete swath can be found. */
    mb_u_char rxFanIndex;           /* Index 0 is the aft swath, port side. */
    mb_u_char swathsPerPing;        /* Number of swaths per ping. A swath is a complete set of across track data.
                                     * A swath may contain several transmit sectors and RX fans. */
    mb_u_char swathAlongPosition;   /* Alongship index for the location of the swath in multi swath mode.
                                     * Index 0 is the aftmost swath. */
    mb_u_char txTransducerInd;      /* Transducer used in this rx fan. Index:
                                     * 0 = TRAI_TX1
                                     * 1 = TRAI_TX2 etc. */
    mb_u_char rxTransducerInd;      /* Transducer used in this rx fan. Index:
                                     * 0 = TRAI_RX1
                                     * 1 = TRAI_RX2 etc. */
    mb_u_char numRxTransducers;     /* Total number of receiving units. */
    mb_u_char algorithmType;        /* For future use. 0 - current algorithm, >0 - future algorithms. */
};

/************************************
    #MRZ - multibeam data for raw range,
    depth, reflectivity, seabed image(SI) etc.
      Datagram Version (header->dgmVersion):
        Format Spec Rev < F ==> 0
        Format Spec Rev < G ==> 1
        Format Spec Rev < H ==> 2
      MB-System writes MRZ datagrams in version 2 form.
      Fields added for version 1 are marked with <G> and version 2 with <H>
 ************************************/

struct mbsys_kmbes_mrz_ping_info {
    /* #MRZ - ping info. Information on vessel/system level, i.e. information common to all beams in the current ping. */
    unsigned short numBytesInfoData;    /* Number of bytes in current struct. */
    unsigned short padding0;            /* Byte alignment. */
    float pingRate_Hz;                  /* Ping rate. Filtered/averaged. */
    mb_u_char beamSpacing;              /* 0 = Eqidistance
                                         * 1 = Equiangle
                                         * 2 = High density */
    mb_u_char depthMode;                /* Depth mode.
                                         * Describes setting of depth in K-Controller. Depth mode influences the PUs
                                         * choice of pulse length and pulse type. If operator has manually chosen the
                                         * depth mode to use, this is flagged by adding 1000 to the mode index.
                                         *
                                         * Number	Auto setting	Number	Manual setting
                                         * 0	    Very shallow	1000	Very shallow
                                         * 1	    Shallow	        1001	Shallow
                                         * 2	    Medium	        1002	Medium
                                         * 3	    Deep	        1003	Deep
                                         * 4	    Very deep	    1004	Very deep
                                         * 5	    Extra deep	    1005	Extra deep */
    mb_u_char subDepthMode;             /* For advanced use when depth mode is set manually.
                                         * 0 = Sub depth mode is not used (when depth mode is auto). */
    mb_u_char distanceBtwSwath;         /* Achieved distance between swaths, in percent relative to required swath distance.
                                         * 0 = function is not used
                                         * 100 = achieved swath distance equals required swath distance. */
    mb_u_char detectionMode;            /* Detection mode. Bottom detection algorithm used.
                                         * 0 = normal
                                         * 1 = waterway
                                         * 2 = tracking
                                         * 3 = minimum depth */
    mb_u_char pulseForm;                /* Pulse forms used for current swath.
                                         * 0 = CW
                                         * 1 = mix
                                         * 2 = FM */
    unsigned short padding1;            /* Ping rate. Filtered/averaged. */
    float frequencyMode_Hz;             /* Ping frequency in hertz. E.g. for EM 2040: 200 000 Hz, 300 000 Hz,
                                         * 400 000 Hz, 600 000 Hz or 700 000 Hz. If values is less than 100,
                                         * it refers to a code defined in the table below.
                                         * Value	    Frequency	    Valid for EM model
                                         * -1	        Not used	    -
                                         * 0	        40 - 100 kHz	EM 710, EM 712
                                         * 1	        50 - 100 kHz	EM 710, EM 712
                                         * 2	        70 - 100 kHz	EM 710, EM 712
                                         * 3	        50 kHz	        EM 710, EM 712
                                         * 4	        40 kHz	        EM 710, EM 712
                                         * 180k - 400k	180 - 400 kHz	EM 2040C (10 kHz steps)
                                         * 200k	        200 kHz	        EM 2040, EM 2040P
                                         * 300k	        300 kHz	        EM 2040, EM 2040P
                                         * 400k	        400 kHz	        EM 2040, EM 2040P
                                         * 600k	        600 kHz	        EM 2040, EM 2040P
                                         * 700k	        700 kHz	        EM 2040, EM 2040P */
    float freqRangeLowLim_Hz;           /* Lowest centre frequency of all sectors in this swath. Unit hertz.
                                         * E.g. for EM 2040: 260 000 Hz. */
    float freqRangeHighLim_Hz;          /* Highest centre frequency of all sectors in this swath. Unit hertz.
                                         * E.g. for EM 2040: 320 000 Hz. */
    float maxTotalTxPulseLength_sec;    /* Total signal length of the sector with longest tx pulse. Unit second. */
    float maxEffTxPulseLength_sec;      /* Effective signal length (-3dB envelope) of the sector with longest effective
                                         * tx pulse. Unit second. */
    float maxEffTxBandWidth_Hz;         /* Effective bandwidth (-3dB envelope) of the sector with highest bandwidth. */
    float absCoeff_dBPerkm;             /* Average absorption coefficient, in dB/km, for vertical beam at current depth. */
    float portSectorEdge_deg;           /* Port sector edge, used by beamformer, Coverage is refered to z of SCS. Unit degree. */
    float starbSectorEdge_deg;          /* Starboard sector edge, used by beamformer. Coverage is referred to z of SCS. Unit degree. */
    float portMeanCov_deg;              /* Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit degree. */
    float starbMeanCov_deg;             /* Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit degree. */
    short portMeanCov_m;                /* Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit degree. */
    short starbMeanCov_m;               /* Coverage achieved, corrected for raybending. Unit meter. */
    mb_u_char modeAndStabilisation;     /* Modes and stabilisation settings as chosen by operator.
                                         * Each bit refers to one setting in K-Controller.
                                         * Unless otherwise stated, default: 0 = off, 1 = on/auto.
                                         * Bit	Mode
                                         * 1	Pitch stabilisation
                                         * 2	Yaw stabilisation
                                         * 3	Sonar mode
                                         * 4	Angular coverage mode
                                         * 5	Sector mode
                                         * 6	Swath along position (0 = fixed, 1 = dynamic)
                                         * 7-8	Future use */
    mb_u_char runtimeFilter1;           /* Filter settings as chosen by operator.
                                         * Refers to settings in runtime display of K-Controller.
                                         * Each bit refers to one filter setting. 0 = off, 1 = on/auto.
                                         * Bit	Filter choice
                                         * 1	Slope filter
                                         * 2	Aeration filer
                                         * 3	Sector filter
                                         * 4	Interference filter
                                         * 5	Special amplitude detect
                                         * 6-8	Future use */

    unsigned short runtimeFilter2;      /* Filter settings as chosen by operator.
                                         * Refers to settings in runtime display of K-Controller. 4 bits used per filter.
                                         * Bit	    Filter choice	        Setting
                                         * 1-4	    Range gate size	        0 = small, 1 = normal, 2 = large
                                         * 5-8	    Spike filter strength	0 = off, 1= weak, 2 = medium, 3 = strong
                                         * 9-12	    Penetration filter	    0 = off, 1 = weak, 2 = medium, 3 = strong
                                         * 13-16	Phase ramp	            0 = short, 1 = normal, 2 = long */

    unsigned int pipeTrackingStatus;    /* Pipe tracking status. Describes how angle and range of top of pipe is determined.
                                        * 0 = for future use
                                        * 1 = PU uses guidance from SIS. */
    float transmitArraySizeUsed_deg;    /* Transmit array size used. Direction along ship. Unit degree. */
    float receiveArraySizeUsed_deg;     /* Receiver array size used. Direction across ship. Unit degree. */
    float transmitPower_dB;             /* Operator selected tx power level re maximum. Unit dB. */
    unsigned short SLrampUpTimeRemaining; /* For marine mammal protection.
                                           * The parameters describes time remaining until max source level (SL) is
                                           * achieved. Unit %. */
    unsigned short padding2;            /* Byte alignment. */
    float yawAngle_deg;                 /* Yaw correction angle applied. Unit degree. */
    unsigned short numTxSectors;        /* Number of transmit sectors.
                                        * Also called Ntx in documentation. Denotes how many times the struct
                                        * mbsys_kmbes_mrz_tx_sector_info is repeated in the datagram. */
    unsigned short numBytesPerTxSector; /* Number of bytes in the struct mbsys_kmbes_mrz_tx_sector_info, containing tx
                                         * sector specific information. The struct is repeated numTxSectors times. */
    float headingVessel_deg;            /* Heading of vessel at time of midpoint of first tx pulse.
                                         * From active heading sensor. */
    float soundSpeedAtTxDepth_mPerSec;  /* At time of midpoint of first tx pulse. Value as used in depth calculations.
                                         * Source of sound speed defined by user in K-Controller. */
    float txTransducerDepth_m;          /* Tx transducer depth in meters below waterline, at time of midpoint of first
                                         * tx pulse. For the tx array (head) used by this RX-fan. Use depth of TX1 to
                                         * move depth point (XYZ) from vessel reference point to transducer (old datagram format). */
    float z_waterLevelReRefPoint_m;     /* Distance between water line and vessel reference point in meters.
                                         * At time of midpoint of first tx pulse. Measured in the surface coordinate
                                         * system (SCS).See Coordinate systems 'Coordinate systems' for definition.
                                         * Used this to move depth point (XYZ) from vessel reference point to transducer (old datagram format). */
    float x_kmallToall_m;               /* Distance between transducer (reference point) and vessel reference point in
                                         * meters, in the surface coordinate system, at time of midpoint of first tx pulse.
                                         * Used this to move depth point (XYZ) from vessel reference point to transducer (old datagram format). */
    float y_kmallToall_m;               /* Distance between transducer (reference point) and vessel reference point in
                                         * meters, in the surface coordinate system, at time of midpoint of first tx pulse.
                                         * Used this to move depth point (XYZ) from vessel reference point to transducer (old datagram format). */
    mb_u_char latLongInfo;              /* Method of position determination from position sensor data:
                                         * 0 = last position received
                                         * 1 = interpolated
                                         * 2 = processed */
    mb_u_char posSensorStatus;          /* Status/quality for data from active position sensor.
                                         * 0 = valid data, 1 = invalid data, 2 = reduced performance. */
    mb_u_char attitudeSensorStatus;     /* Status/quality for data from active attitude sensor. */
    mb_u_char padding3;                 /* Byte alignment. */
    double latitude_deg;                /* Latitude (decimal degrees) of vessel reference point at time of midpoint of
                                         * first tx pulse. Negative on southern hemisphere. */
    double longitude_deg;               /* Longitude (decimal degrees) of vessel reference point at time of midpoint of
                                         * first tx pulse. Negative on western hemisphere. */
    float ellipsoidHeightReRefPoint_m;  /* Height of vessel reference point above the ellipsoid, derived from active GGA
                                         * sensor. ellipsoidHeightReRefPoint_m is GGA height corrected for motion and
                                         * installation offsets of the position sensor. */
    float bsCorrectionOffset_dB;        /* <G> Backscatter offset set in the installation menu. */
    mb_u_char lambertsLawApplied;       /* <G> Beam intensity data corrected as seabed image data (Lambert and normal incidence corrections).  */
    mb_u_char iceWindow;                /* <G> Ice window installed. */
    unsigned short activeModes;         /* <H> Showing active modes. Currently used for EM MultiFrequency Mode only.
                                           <G> Was 2-Byte alignment padding4 in version 1. */
};

struct mbsys_kmbes_mrz_tx_sector_info
{
    /* #MRZ - sector information. */
    /* Information specific to each transmitting sector. sectorInfo is repeated numTxSectors (Ntx)- times in datagram. */
    mb_u_char txSectorNumb;         /* Tx sector index number, used in the sounding section. Starts at 0. */
    mb_u_char txArrNumber;          /* TX arry number. Single Tx, txArrNumber = 0. */
    mb_u_char txSubArray;           /* Default = 0. E.g. for EM2040, the transmitted pulse consists of three sectors,
                                     * each transmitted from separate txSubArrays. Orientation and numbers are relative
                                     * the array coordinate system. Sub array installation offsets can be found in the
                                     * installation datagram, #IIP.
                                     * 0 = Port subarray
                                     * 1 = middle subarray
                                     * 2 = starboard subarray */
    mb_u_char padding0;             /* Byte alignment. */
    float sectorTransmitDelay_sec;  /* Transmit delay of the current sector/subarray. Delay is the time from the
                                     * midpoint of the current transmission to midpoint of the first transmitted pulse
                                     * of the ping, i.e. relative to the time used in the datagram header. */
    float tiltAngleReTx_deg;        /* Along ship steering angle of the TX beam (main lobe of transmitted pulse), angle
                                     * referred to transducer array coordinate system. Unit degree. */
    float txNominalSourceLevel_dB;  /* Actual SL = txNominalSourceLevel_dB + highVoltageLevel_dB. Unit dB re 1 microPascal.  */
    float txFocusRange_m;           /* 0 = no focusing applied. */
    float centreFreq_Hz;            /* Centre frequency. Unit hertz. */
    float signalBandWidth_Hz;       /* FM mode: effective bandwidth
                                     * CW mode: 1/(effective tx pulse length) */
    float totalSignalLength_sec;    /* Also called pulse length. Unit second. */
    mb_u_char pulseShading;         /* Transmit pulse is shaded in time (tapering).
                                     * Amplitude shading in %. cos2- function used for shading the tx pulse in time. */
    mb_u_char signalWaveForm;       /* Transmit signal wave form.
                                     * 0 = CW
                                     * 1 = FM upsweep
                                     * 2 = FM downsweep. */
    unsigned short padding1;        /* <G> Byte alignment. */
    float highVoltageLevel_dB;      /* <G> 20log(Measured high voltage power level at TX pulse or Nominal high voltage
                                     * power level). This parameter will also include the effect of user selected
                                     * transmit power reduction (transmitPower_dB) and mammal protection.
                                     * Actual SL = txNominalSourceLevel_dB + highVoltageLevel_dB. Unit dB. */
    float sectorTrackingCorr_dB;    /* <G> Backscatter correction added in sector tracking mode. Unit dB. */
    float effectiveSignalLength_sec;/* <G> Signal length used for backscatter footprint calculation.
                                     * This compensates for the TX pulse tapering and the RX filter bandwidths.
                                     * Unit second. */
};

struct mbsys_kmbes_mrz_rx_info {
    /* #MRZ - receiver specific information. */
    /* Information specific to the receiver unit used in this swath. */
    unsigned short numBytesRxInfo;              /* Bytes in current struct. */
    unsigned short numSoundingsMaxMain;         /* Maximum number of main soundings (bottom soundings) in this datagram,
                                                * extra detections (soundings in water column) excluded. Also referred
                                                * to as Nrx. Denotes how many bottom points (or loops) given in the struct
                                                * mbsys_kmbes_mrz_sounding. */
    unsigned short numSoundingsValidMain;       /* Number of main soundings of valid quality.
                                                * Extra detections not included. */
    unsigned short numBytesPerSounding;         /* Bytes per loop of sounding (per depth point), i.e. bytes per loops of
                                                * the struct mbsys_kmbes_mrz_sounding. */
    float WCSampleRate;                         /* Sample frequency divided by water column decimation factor. Unit Hz. */
    float seabedImageSampleRate;                /* Sample frequency divided by seabed image decimation factor. Unit Hz. */
    float BSnormal_dB;                          /* Backscatter level, normal incidence. Unit dB */
    float BSoblique_dB;                         /* Backscatter level, oblique incidence. Unit dB */
    unsigned short extraDetectionAlarmFlag;     /* Sum of alarm flags. Range 0-10. */
    unsigned short numExtraDetections;          /* Sum of extradetection from all classes. Also refered to as Nd. */
    unsigned short numExtraDetectionClasses;    /* Range 0-10. */
    unsigned short numBytesPerClass;            /* Number of bytes in the struct mbsys_kmbes_mrz_extra_det_class_info */
};

struct mbsys_kmbes_mrz_extra_det_class_info {
    /* #MRZ - Extra detection class information. */
    /* To be entered in loop numExtraDetectionClasses - times. */
    unsigned short numExtraDetInClass;  /* Number of extra detection in this class. */
    signed char padding;                /* Byte alignment. */
    mb_u_char alarmFlag;                /* 0 = no alarm, 1 = alarm. */
};

struct mbsys_kmbes_mrz_sounding {
    /* #MRZ - Data for each sounding, e.g. XYZ, reflectivity, two way travel time etc. */
    /* Also contains information necessary to read seabed image following this datablock
     * (number of samples in SI etc.). To be entered in loop (numSoundingsMaxMain + numExtraDetections)
     * times. */

    unsigned short soundingIndex;       /* Sounding index. Cross reference for seabed image.
                                        * Valid range: 0 to (numSoundingsMaxMain+numExtraDetections)-1,
                                        * i.e. 0 - (Nrx+Nd)-1. */
    mb_u_char txSectorNumb;             /* Transmitting sector number. Valid range: 0-(Ntx-1), where Ntx is numTxSectors. */

    /* Detection info */
    mb_u_char detectionType;            /* Bottom detection type. Normal bottom detection, extra detection, or rejected.
                                        * 0 = normal detection
                                        * 1 = extra detection
                                        * 2 = rejected detection
                                        * In case 2, the estimated range has been used to fill in amplitude samples in
                                        * the seabed image datagram. */
    mb_u_char detectionMethod;          /* Method for determining bottom detection, e.g. amplitude or phase.
                                        * 0 = no valid detection
                                        * 1 = amplitude detection
                                        * 2 = phase detection
                                        * 3-15 for future use. */
    mb_u_char rejectionInfo1;           /* For Kongsberg use. */
    mb_u_char rejectionInfo2;           /* For Kongsberg use. */
    mb_u_char postProcessingInfo;       /* For Kongsberg use. */
    mb_u_char detectionClass;           /* Detection class based on detected range. */
    mb_u_char detectionConfidenceLevel; /* Detection confidence level. */
    /* unsigned short padding; */       /* These two bytes specified as padding
                                           in the Kongsberg specification but are
                                           here used for the MB-System beam flag
                                           - if the first mb_u_char == 1 then the
                                             second byte is an MB-System beamflag */
    char beamflag_enabled;              /* MB-system beamflag enabled iff == 1 */
    char beamflag;                      /* MB-system beamflag value */

    float rangeFactor;                  /* Unit %. rangeFactor = 100 if main detection. */
    float qualityFactor;                /* Estimated standard deviation as % of the detected depth.
                                         * Quality Factor (QF) is calculated from IFREMER Quality Factor (IFQ):
                                         * QF=Est(dz)/z=100*10^-IQF */
    float detectionUncertaintyVer_m;    /* Vertical uncertainty, based on quality factor (QF, qualityFactor). */
    float detectionUncertaintyHor_m;    /* Horizontal uncertainty, based on quality factor (QF, qualityFactor). */
    float detectionWindowLength_sec;    /* Detection window length. Unit second. Sample data range used in
                                         * final detection. */
    float echoLength_sec;               /* Measured echo length. Unit second. */

    /* Water column paramters */
    unsigned short WCBeamNumb;          /* Water column beam number.
                                         * Info for plotting soundings together with water column data. */
    unsigned short WCrange_samples;     /* Water column range. Range of bottom detection, in samples. */
    float WCNomBeamAngleAcross_deg;     /* Water column nominal beam angle across. Re vertical. */

    /* Reflectivity data (backscatter (BS) data) */
    float meanAbsCoeff_dBPerkm;         /* Mean absorption coefficient, alfa. Used for TVG calculations.
                                         * Value as used. Unit dB/km.*/
    float reflectivity1_dB;             /* Beam intensity (BS), using TVG = X log(R) + 2 alpha R. X (operator selected)
                                         * is common to all beams in datagram.
                                         * Alpha (variabel meanAbsCoeff_dBPerkm) is given for each beam (current
                                         * struct). BS = EL – SL – M + TVG + BScorr, where EL= detected echo level
                                         * (not recorded in datagram), and the rest of the parameters are
                                         * found below.*/
    float reflectivity2_dB;             /* Beam intensity, using the traditional KM special TVG. */
    float receiverSensitivityApplied_dB;    /* Receiver sensitivity (M), in dB, compensated for RX beampattern at
                                             * actual transmit frequency at current vessel attitude.*/
    float sourceLevelApplied_dB;        /* Source level (SL) applied (dB): SL = SLnom + SLcorr
                                         * where SLnom = Nominal maximum SL, recorded per TX sector (variabel
                                         * txNominalSourceLevel_dB in struct EMdgmMRZ_txSectorInfo_def) and
                                         * SLcorr = SL correction relative to nominal TX power based on measured high
                                         * voltage power level and any use of digital power control. SL is corrected
                                         * for TX beampattern along and across at actual transmit frequency at current
                                         * vessel attitude.*/
    float BScalibration_dB;             /* Backscatter (BScorr) calibration offset applied (default = 0 dB). */
    float TVG_dB;                       /* Time Varying Gain (TVG) used when correcting reflectivity. */

    /* Travel time and angle data) */
    float beamAngleReRx_deg;            /* Angle relative to the RX transducer array, except for ME70, where the
                                         * angles are relative to the horizontal plane. */
    float beamAngleCorrection_deg;      /* Applied beam pointing angle correction. */
    float twoWayTravelTime_sec;         /* Two way travel time (also called range). Unit second. */
    float twoWayTravelTimeCorrection_sec;   /* Applied two way travel time correction. Unit second. */

    /* Georeferenced depth points */
    float deltaLatitude_deg;            /* Distance from vessel reference point at time of first tx pulse in ping,
                                         * to depth point. Measured in the surface coordinate system (SCS), see
                                         * Coordinate systems for definition. Unit decimal degrees. */
    float deltaLongitude_deg;           /* Distance from vessel reference point at time of first tx pulse in ping,
                                         * to depth point. Measured in the surface coordinate system (SCS), see
                                         * Coordinate systems for definition. Unit decimal degree. */
    float z_reRefPoint_m;               /* Vertical distance z. Distance from vessel reference point at time of first
                                         * tx pulse in ping, to depth point. Measured in the surface coordinate system
                                         * (SCS), see Coordinate systems for definition. */
    float y_reRefPoint_m;               /* Horizontal distance y. Distance from vessel reference point at time of first
                                         * tx pulse in ping, to depth point. Measured in the surface coordinate system
                                         * (SCS), see Coordinate systems for definition. */
    float x_reRefPoint_m;               /* Horizontal distance x. Distance from vessel reference point at time of first
                                         * tx pulse in ping, to depth point. Measured in the surface coordinate system
                                         * (SCS), see Coordinate systems for definition. */
    float beamIncAngleAdj_deg;          /* Beam incidence angle adjustment (IBA) unit degree. */
    unsigned short realTimeCleanInfo;   /* For future use. */

    /* Seabed image */
    unsigned short SIstartRange_samples;    /* Seabed image start range, in sample number from transducer.
                                             * Valid only for the current beam. */
    unsigned short SIcentreSample;          /* Seabed image. Number of the centre seabed image sample for the
                                             * current beam. */
    unsigned short SInumSamples;            /* Seabed image. Number of range samples from the current beam, used to
                                             * form the seabed image. */

};

struct mbsys_kmbes_mrz_extra_si {
    /* #MRZ - Extra seabed image samples. */
    unsigned short portStartRange_samples;  /* Start range of port side extra seabed image samples. Range in samples. */
    unsigned short numPortSamples;          /* Number of extra seabed image samples on port side. */
    short portSIsample_desidB[MBSYS_KMBES_MAX_SIDESCAN_EXTRA_SAMP]; /* Port side extra seabed image samples, as
                                                                     * amplitudes in 0.1 dB. */
    unsigned short starbStartRange_samples; /* Start range of starboard side extra seabed image samples.
                                             * Range in samples. */
    unsigned short numStarbSamples;         /* Number of extra seabed image samples on starboard side. */
    short starbSIsample_desidB[MBSYS_KMBES_MAX_SIDESCAN_EXTRA_SAMP]; /* Starboard side extra seabed image samples, as
                                                                      * amplitudes in 0.1 dB. */
};

struct mbsys_kmbes_mrz {
    /* #MRZ - Multibeam Raw Range and Depth datagram. The datagram also contains seabed image data. */
    /* Depths points (x,y,z) are calculated in meters, georeferred to the position of the vessel reference point
     * at the time of the first transmitted pulse of the ping. The depth point coordinates x and y are in the surface
     * coordinate system (SCS), and are also given as delta latitude and delta longitude, referred to origin of the
     * VCS/SCS, at the time of the midpoint of the first transmitted pulse of the ping (equals time used in the
     * datagram header timestamp). See Coordinate systems for introduction to spatial reference points and coordinate
     * systems. Reference points are also described in Reference points and offsets. Explanation of the xyz reference
     * points is also illustrated in the figure below. */

    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_m_partition partition;
    struct mbsys_kmbes_m_body cmnPart;
    struct mbsys_kmbes_mrz_ping_info pingInfo;
    struct mbsys_kmbes_mrz_tx_sector_info sectorInfo[MBSYS_KMBES_MAX_NUM_TX_PULSES];
    struct mbsys_kmbes_mrz_rx_info rxInfo;
    struct mbsys_kmbes_mrz_extra_det_class_info extraDetClassInfo[MBSYS_KMBES_MAX_EXTRA_DET_CLASSES];
    struct mbsys_kmbes_mrz_sounding sounding[MBSYS_KMBES_MAX_NUM_BEAMS+MBSYS_KMBES_MAX_EXTRA_DET];
    short SIsample_desidB[MBSYS_KMBES_MAX_SIDESCAN_SAMP];
    /* Seabed image sample amplitude, in 0.1 dB. Actual number of seabed image samples (SIsample_desidB) to be
     * found by summing parameter SInumSamples in struct mbsys_kmbes_mrz_sounding for all beams. Seabed image data are
     * raw beam sample data taken from the RX beams. The data samples are selected based on the bottom detection
     * ranges. First sample for each beam is the one with the lowest range. The centre sample from each beam is geo
     * referenced (x, y, z data from the detections). The BS corrections applied at the centre sample are the same as
     * used for reflectivity2_dB (struct mbsys_kmbes_mrz_sounding).*/
};

 #define MBSYS_KMBES_MRZ_VERSION 2

/************************************
    #MWC - water column datagram
      Datagram Version (header->dgmVersion):
        Format Spec Rev < F ==> 0
        Format Spec Rev = G ==> 1
      MB-System writes MWC datagrams in version 1 form.
      Fields added for version 1 are marked with <G>
 ************************************/

struct mbsys_kmbes_mwc_tx_info {
    /* #MWC - data block 1: transmit sectors, general info for all sectors */
    unsigned short numBytesTxInfo;      /* Number of bytes in current struct. */
    unsigned short numTxSectors;        /* Number of transmitting sectors (Ntx). Denotes the number of times the
                                         * struct EMdgmMWCtxSectorData is repeated in the datagram. */
    unsigned short numBytesPerTxSector; /* Number of bytes in EMdgmMWCtxSectorData. */
    short padding;
    float heave_m;  /* Heave at vessel reference point, at time of ping, i.e. at midpoint of first tx pulse in rxfan. */
};

struct mbsys_kmbes_mwc_tx_sector_data {
    /* #MWC - data block 1: transmit sector data, loop for all i = numTxSectors. */
    float tiltAngleReTx_deg;        /* Along ship steering angle of the TX beam (main lobe of transmitted pulse),
                                     * angle referred to transducer face. Angle as used by beamformer (includes
                                     * stabilisation). Unit degree. */
    float centreFreq_Hz;            /* Centre frequency of current sector. Unit Hz. */
    float txBeamWidthAlong_deg;     /* Corrected for frequency, sound velocity and tilt angle. Unit degree. */
    unsigned short txSectorNum;     /* Transmitting sector number. */
    short padding;
};

struct mbsys_kmbes_mwc_rx_info {
    /* #MWC - data block 2: receiver, general info */
    unsigned short numBytesRxInfo;      /* Number of bytes in current struct. */
    unsigned short numBeams;            /* Number of beams in this datagram (Nrx). */
    mb_u_char numBytesPerBeamEntry;     /* Bytes in EMdgmMWCrxBeamData struct, excluding sample amplitudes (which
                                         * have varying lengths) */
    mb_u_char phaseFlag;                /* 0 = off
                                         * 1 = low resolution
                                         * 2 = high resolution. */
    mb_u_char TVGfunctionApplied;       /* Time Varying Gain function applied (X). X log R + 2 Alpha R + OFS + C,
                                         * where X and C is documented in #MWC datagram. OFS is gain offset to
                                         * compensate for TX source level, receiver sensitivity etc. */
    signed char TVGoffset_dB;           /* Time Varying Gain offset used (OFS), unit dB. X log R + 2 Alpha R + OFS + C,
                                         * where X and C is documented in #MWC datagram. OFS is gain offset to
                                         * compensate for TX source level, receiver sensitivity etc. */
    float sampleFreq_Hz;                /* The sample rate is normally decimated to be approximately the same as the
                                         * bandwidth of the transmitted pulse. Unit hertz. */
    float soundVelocity_mPerSec;        /* Sound speed at transducer, unit m/s. */
};

struct mbsys_kmbes_mwc_rx_beam_data {
    /* #MWC - data block 2: receiver, specific info for each beam. */
    float beamPointAngReVertical_deg;
    unsigned short startRangeSampleNum;
    unsigned short detectedRangeInSamples;
    /* Two way range in samples. Approximation to calculated distance from tx to bottom detection
     * [meters] = soundVelocity_mPerSec * detectedRangeInSamples / (sampleFreq_Hz * 2). The detected range
     * is set to zero when the beam has no bottom detection */
    unsigned short beamTxSectorNum;
    unsigned short numSampleData;           /* Number of sample data for current beam. Also denoted Ns. */
    float detectedRangeInSamplesHighResolution;  /* <G> The same information as in detectedRangeInSamples
                                                    with higher resolution. Two way range in samples.
                                                    Approximation to calculated distance from tx to
                                                    bottom detection [meters] = soundVelocity_mPerSec
                                                    * detectedRangeInSamples / (sampleFreq_Hz * 2).
                                                    The detected range is set to zero when the beam
                                                    has no bottom detection.*/
    size_t sampleAmplitude05dB_p_alloc_size;
    signed char  *sampleAmplitude05dB_p;    /* Pointer to start of array with Water Column data. Length of
                                             * array = numSampleData. Sample amplitudes in 0.5 dB resolution. */
    /* If rxInfo->phaseFlag == 0 then the sampleAmplitude time series is not followed
       by phase information. If rxInfo->phaseFlag == 1 then the sampleAmplitude time series
       is followed immediately by a time series of numSampleData signed char values.
       If rxInfo->phaseFlag == 2 then the sampleAmplitude time series
       is followed immediately by a time series of numSampleData signed short values. */
    size_t samplePhase8bit_alloc_size;
    signed char *samplePhase8bit;  /* Rx beam phase in 180/128 degree resolution. */
    size_t samplePhase16bit_alloc_size;
    short *samplePhase16bit;       /* Rx beam phase in 0.01 degree resolution. */
};

struct mbsys_kmbes_mwc {
    /* #MWC - Multibeam Water Column Datagram. Entire datagram containing several sub structs. */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_m_partition partition;
    struct mbsys_kmbes_m_body cmnPart;
    struct mbsys_kmbes_mwc_tx_info txInfo;
    struct mbsys_kmbes_mwc_tx_sector_data sectorData[MBSYS_KMBES_MAX_NUM_TX_PULSES];
    struct mbsys_kmbes_mwc_rx_info rxInfo;
    size_t beamData_p_alloc_size;
    struct mbsys_kmbes_mwc_rx_beam_data *beamData_p;
};

 #define MBSYS_KMBES_MWC_VERSION 1

/*********************************************

 Compatibility datagrams for .all to .kmall conversion support

 *********************************************/

/************************************
 #CPO - Compatibility position sensor data
 ************************************/
struct mbsys_kmbes_cpo_data_block
{
    unsigned int timeFromSensor_sec;        /* UTC time from position sensor. Unit seconds. Epoch 1970-01-01. */
    unsigned int timeFromSensor_nanosec;    /* UTC time from position sensor. Unit nano seconds remainder. */
    float posFixQuality_m;                  /* Only if available as input from sensor. */
    double correctedLat_deg;                /* Motion corrected (if enabled in K-Controller) data as used in depth
                                             * calculations. Referred to antenna footprint at water level. Unit decimal
                                             * degree. Parameter is set to define MBSYS_KMBES_UNAVAILABLE_LATITUDE if
                                             * sensor inactive. */
    double correctedLong_deg;               /* Motion corrected (if enabled in K-Controller) data as used in depth
                                             * calculations. Referred to antenna footprint at water level. Unit decimal
                                             * degree. Parameter is set to define MBSYS_KMBES_UNAVAILABLE_LONGITUDE if
                                             * sensor inactive. */
    float speedOverGround_mPerSec;          /* Speed over ground. Unit m/s. Motion corrected (if enabled in
                                             * K-Controller) data as used in depth calculations. If unavailable or from
                                             * inactive sensor, value set to define MBSYS_KMBES_UNAVAILABLE_SPEED. */
    float courseOverGround_deg;             /* Course over ground. Unit degree. Motion corrected (if enabled in
                                             * K-Controller) data as used in depth calculations. If unavailable or from
                                             * inactive sensor, value set to define MBSYS_KMBES_UNAVAILABLE_COURSE. */
    float ellipsoidHeightReRefPoint_m;      /* Height of antenna footprint above the ellipsoid. Unit meter. Motion
                                             * corrected (if enabled in K-Controller) data as used in depth
                                             * calculations. If unavailable or from inactive sensor, value set to
                                             * define MBSYS_KMBES_UNAVAILABLE_ELLIPSOIDHEIGHT. */
    char posDataFromSensor[MBSYS_KMBES_MAX_CPO_DATALENGTH];    /* Position data as received from sensor, i.e.
                                                                * uncorrected for motion etc. */
};

struct mbsys_kmbes_cpo
{
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_s_common cmnPart;
    struct mbsys_kmbes_cpo_data_block sensorData;
};

#define MBSYS_KMBES_CPO_VERSION 0

/************************************
 #CHE - Compatibility heave data
 ************************************/
struct mbsys_kmbes_che_data
{
    /* Heave compatibility data part. Heave reference point at transducer instead of at vessel reference point */
    float heave_m; /* Heave. Unit meter. Positive downwards. */
};

struct mbsys_kmbes_che
{
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_m_body cmnPart;
    struct mbsys_kmbes_che_data data;
};

#define MBSYS_KMBES_CHE_VERSION 0

/*********************************************

   Installation and runtime datagrams

 *********************************************/

/************************************
    #IIP - Info Installation PU
 ************************************/

struct mbsys_kmbes_iip {
    /* Definition of #IIP datagram containing installation parameters and sensor format settings. */
    struct mbsys_kmbes_header header;
    unsigned short numBytesCmnPart; /* Size in bytes of body part struct.
                                     * Used for denoting size of rest of the datagram. */
    unsigned short info;            /* Information. For future use. */
    unsigned short status;          /* Status. For future use. */
    mb_u_char install_txt[MBSYS_KMBES_MAX_IIP_DATALENGTH];          /* Installation settings as text format.
                                     * Parameters separated by ; and lines separated by , delimiter. */
};

 #define MBSYS_KMBES_IIP_VERSION 0

/************************************
    #IOP -  Runtime datagram
 ************************************/

struct mbsys_kmbes_iop {
    /* Definition of #IOP datagram containing runtime parameters, exactly as chosen by operator in the
     * K-Controller/SIS menus. */
    struct mbsys_kmbes_header header;
    unsigned short numBytesCmnPart; /* Size in bytes of body part struct. Used for denoting size of rest of the datagram. */
    unsigned short info;            /* Information. For future use. */
    unsigned short status;          /* Status. For future use. */
    mb_u_char runtime_txt[MBSYS_KMBES_MAX_IOP_DATALENGTH];          /* Runtime paramters as text format.
                                     * Parameters separated by ; and lines separated by , delimiter.
                                     * Text strings refer to names in menues of the K-Controller/SIS.
                                     * For detailed description of text strings, see the separate document
                                     * Runtime parameters set by operator */
};

 #define MBSYS_KMBES_IOP_VERSION 0

/************************************
    #IB - BIST Error Datagrams
 ************************************/

struct mbsys_kmbes_ib {
    /* #IB - Results from online built in test (BIST). Definition used for three different BIST datagrams,
     * i.e. #IBE (BIST Error report), #IBR (BIST reply) or #IBS (BIST short reply).*/
    struct mbsys_kmbes_header header;
    unsigned short numBytesCmnPart; /* Size in bytes of body part struct.
                                     * Used for denoting size of rest of the datagram. */
    mb_u_char BISTInfo;             /* 0 = last subset of the message
                                     * 1 = more messages to come */
    mb_u_char BISTStyle;            /* 0 = plain text
                                     * 1 = use style sheet */
    mb_u_char BISTNumber;           /* The BIST number executed. */
    signed char BISTStatus;         /* 0 = BIST executed with no errors
                                     * positive number = warning
                                     * negative number = error */
    mb_u_char BISTText;             /* Result of the BIST. Starts with a synopsis of the result, followed by
                                     * detailed descriptions. */
};

 #define MBSYS_KMBES_BIST_VERSION 0

/*********************************************

    File datagrams

 *********************************************/

struct mbsys_kmbes_f_common
{
    uint16_t numBytesCmnPart;               /* Size in bytes of common part struct.  */
    int8_t fileStatus;                      /* -1 = No file found, 0 = OK, 1 = File too large (cropped) */
    uint8_t padding1;                       /* padding */
    unsigned int numBytesFile;              /* File size in bytes. */
    char fileName[MBSYS_KMBES_MAX_F_FILENAME_LENGTH];   /* Name of file. */
};

/********************************************
     #FCF - Backscatter calibration file datagram
 ********************************************/
struct mbsys_kmbes_fcf {
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_m_partition partition;
    struct mbsys_kmbes_f_common cmnPart;
    mb_u_char bsCalibrationFile[MBSYS_KMBES_MAX_F_FILE_SIZE];
};

#define MBSYS_KMBES_FCF_VERSION 0

/*********************************************

   X-datagrams (extra - defined only for MB-System)

 *********************************************/

/************************************

    XMB, // The presence of this datagram indicates this file/stream has been
         // written by MB-System.
         // - this means that pings include a sidescan datagram MMS after the MRZ datagrams
         // - this means that MB-System beamflags are embedded in the MRZ datagram soundings

 ************************************/

struct mbsys_kmbes_xmb {
    /* Definition of #XMB datagram indicating these data have been written by MB-System */
    struct mbsys_kmbes_header header;
    int mbsystem_extensions;      /* Flag indicating ping data includes a pseudosidescan XMS datagram
                                    following the the MRZ datagrams */
    int watercolumn;              /* Flag indicating ping data includes water column MWC datagrams
                                    following the the MRZ datagrams */
    char unused[24];
    char version[MB_COMMENT_MAXLINE];          /* MB-System version string */
};

 #define MBSYS_KMBES_XMB_VERSION 1

/************************************

    XMC, // Comment datagram (MB-System only)

 ************************************/

struct mbsys_kmbes_xmc {
    /* Definition of #XMC datagram containing comment inserted by MB-System */
    struct mbsys_kmbes_header header;
    char unused[32];
    char comment[MB_COMMENT_MAXLINE];          /* Comment string (null terminated) */
};

 #define MBSYS_KMBES_XMC_VERSION 0

/************************************

    XMT, // MB-System multibeam ping navigation, attitude, travel time, and pointing angle (MB-System only)

 ************************************/

struct mbsys_kmbes_xmt_ping_info {
    /* #XMTZ - ping info. Information on vessel/system level, i.e. information common to all beams in the current ping. */
    unsigned short numBytesInfoData;     /* Number of bytes in current struct (60). */
    unsigned short numBytesPerSounding;  /* Number of bytes per sounding entry (16) */
    int padding0;                        /* padding field */
    double longitude;                   /* Sonar longtitude (degrees) */
    double latitude;                     /* Sonar latitude (degrees) */
    double sensordepth;                  /* Sonar depth (meters) */
    double heading;                       /* Sonar heading (degrees) */
    float speed;                         /* Sonar speed (m/s) */
    float roll;                          /* Sonar roll (degrees) */
    float pitch;                         /* Sonar pitch (degrees) */
    float heave;                         /* Sonar heave (meters) */
    int numSoundings;                    /* number of soundings */
};

struct mbsys_kmbes_xmt_sounding {
    /* #XMT - Corrected travel times and pointing angles ready for raytracing */

    unsigned short soundingIndex;       /* Sounding index. Cross reference for seabed image.
                                        * Valid range: 0 to (numSoundingsMaxMain+numExtraDetections)-1,
                                        * i.e. 0 - (Nrx+Nd)-1. */
    unsigned short padding0;            /* Byte alignment. */
    float twtt;                         /* Corrected two way travel time (seconds) */
    float angle_vertical;               /* Vertical beam angle for raytracing
                                            (degrees, zero = vertical down) */
    float angle_azimuthal;              /* Azimuthal beam angle for raytracing
                                            (degrees, zero = forward, positive clockwise) */
    float beam_heave;                   /* Difference in sensordepth between ping
                                            time and beam receive time */
    float alongtrack_offset;            /* Alongtrack offset in the direction of
                                            platform travel between ping time and beam receive time */

};
struct mbsys_kmbes_xmt {
    /* Definition of #XMT datagram containing corrected/interpolated navigation,
       attitude, travel time, and pointing angle data resolved to ping time
       for each MRZ datagram (MB-System only) */
    struct mbsys_kmbes_header header;
    struct mbsys_kmbes_m_partition partition;
    struct mbsys_kmbes_m_body cmnPart;
    struct mbsys_kmbes_xmt_ping_info xmtPingInfo;
    struct mbsys_kmbes_xmt_sounding xmtSounding[MBSYS_KMBES_MAX_NUM_BEAMS+MBSYS_KMBES_MAX_EXTRA_DET];
};

 #define MBSYS_KMBES_XMT_VERSION 0

/************************************

    XMS, // MB-System multibeam pseudosidescan derived from multibeam backscatter (MB-System only)

 ************************************/

struct mbsys_kmbes_xms {
    /* Definition of #XMS datagram containing multibeam pseudosidescan calculated by MB-System */
    struct mbsys_kmbes_header header;
    unsigned short pingCnt;
    unsigned short spare;
    float pixel_size;                                  /* Pseudosidescan pixel width (meters) */
    int pixels_ss;                                     /* Number of pseudosidescan pixels */
    mb_u_char unused[32];
    float ss[MBSYS_KMBES_MAX_PIXELS];           /* the processed sidescan values ordered port to starboard */
    float ss_alongtrack[MBSYS_KMBES_MAX_PIXELS]; /* the processed sidescan alongtrack distances (meters) */
};

 #define MBSYS_KMBES_XMS_VERSION 0

/*********************************************

   Unknown datagram format

 *********************************************/


struct mbsys_kmbes_unknown_struct {
    /* Unknown record
     * Raw bytes stored and passed on for now */
    size_t unknown_data_alloc; /* Number of bytes allocated to hold the unknown record */
    size_t unknown_len;        /* Number of bytes stored from the unknown record */
    char *unknown_data;        /* unknown record stored as raw bytes */
};


/*********************************************

   File indexing structures

 *********************************************/

/* EM dgm index data structure */
struct mbsys_kmbes_index
{
    double time_d;                     // MB-System time stamp of datagram
    double ping_time_d;                // MB-System time stamp of ping start
    mbsys_kmbes_emdgm_type emdgm_type; // EM datagram type enumeration
    struct mbsys_kmbes_header header;  // EM datagram header
    long file_pos;                     // EM datagram file pointer position
    int index_org;                     // original order in file
    int  ping_num;
    mb_u_char  rx_per_ping;            // Number of rx fans per ping (# of datagrams generated per ping)
    mb_u_char  rx_index;               // Index 0 is the aft swath, port side.
    mb_u_char  swaths_per_ping;        // Number of swath fans per ping
};

/* EM dgm index data structure */
struct mbsys_kmbes_index_table {
    size_t dgm_count;                       // count of indexed datagrams (dgmID)
    size_t num_alloc;                       // allocated capacity
    struct mbsys_kmbes_index *indextable;   // points to index table array head
};


/*********************************************

   Full data storage structure

 *********************************************/

/* Internal data structure */
struct mbsys_kmbes_struct {
    /* Type of most recently read data record */
    /* Translation of kind values:
        MB_DATA_DATA              =  1: Set of MRZ datagrams associated with a single ping
        MB_DATA_COMMENT           =  2:
        MB_DATA_NAV               = 12: #SPO datagram
        MB_DATA_ATTITUDE          = 18: #SKM datagram
        MB_DATA_VELOCITY_PROFILE  =  6: #SVP datagram
        MB_DATA_SSV               = 19: #SVT datagram
        MB_DATA_CLOCK             = 14: #SCL datagram
        MB_DATA_SENSORDEPTH       = 59: #SDE datagram
        MB_DATA_HEIGHT            = 16: #SHI datagram
        MB_DATA_HEADING           = 17: #SHA datagram
        MB_DATA_DATA              = 12: #MRZ datagram
        MB_DATA_WATER_COLUMN      = 46: #MWC datagram
        MB_DATA_NAV1              = 29: #CPO datagram
        MB_DATA_HEAVE             = 64: #CHE datagram
        MB_DATA_INSTALLATION      = 45: #IIP datagram
        MB_DATA_RUN_PARAMETER     = 13: #IOP datagram
        MB_DATA_BIST              = 65: #IBE datagram
        MB_DATA_BIST1             = 66: #IBR datagram
        MB_DATA_BIST2             = 67: #IBS datagram */

    int kind; /* MB-System record ID */

    /* MB-System time stamp of most recently read record */
    double time_d;
    int time_i[7];

    /* Beam and pixel count totals for ping data (multiple MRZ datagrams) */
    int num_soundings;
    int num_backscatter_samples;
    int num_pixels;

    /* #SPO - Sensor POsition data */
    struct mbsys_kmbes_spo spo;

    /* #SKM - KM binary sensor data (attitude)*/
    struct mbsys_kmbes_skm skm;

    /* #SVP - Sound Velocity Profile data */
    struct mbsys_kmbes_svp svp;

    /* #SVT - Sensor sound Velocity measured at Transducer */
    struct mbsys_kmbes_svt svt;

    /* #SCL - Sensor CLock datagram */
    struct mbsys_kmbes_scl scl;

    /* #SDE - Sensor DEpth data */
    struct mbsys_kmbes_sde sde;

    /* #SHI - Sensor HeIght data */
    struct mbsys_kmbes_shi shi;

    /* #SHA - Sensor HeAding */
    struct mbsys_kmbes_sha sha;

    /* #MRZ - Multibeam data for raw range,
     * depth, reflectivity, seabed image(SI) etc. */
    int n_mrz_read;
    int n_mrz_needed;  // Number of MRZ datagrams for the current ping = mrz[mrz.cmnPart.rxFanIndex].cmnPart.rxFansPerPing
    struct mbsys_kmbes_mrz mrz[MBSYS_KMBES_MAX_NUM_MRZ_DGMS];

    /* #XMT - MB-System corrected navigation, attitude, beam travel times, and angles */
    struct mbsys_kmbes_xmt xmt[MBSYS_KMBES_MAX_NUM_MRZ_DGMS];

    /* #XMS - MB-System pseudosidescan (included after MRZ datagrams when written by MB-System) */
    struct mbsys_kmbes_xms xms;

    /* #MWC - Multibeam Water Column data */
    int n_mwc_read;
    int n_mwc_needed;  // Number of MWC datagrams for the current ping = mrz[mrz.cmnPart.rxFanIndex].cmnPart.rxFansPerPing
    struct mbsys_kmbes_mwc mwc[MBSYS_KMBES_MAX_NUM_MWC_DGMS];

    /* #CPO - Compatibility position sensor data */
    struct mbsys_kmbes_cpo cpo;

    /* #CHE - Compatibility heave data */
    struct mbsys_kmbes_che che;

    /* #IIP - Info Installation PU */
    struct mbsys_kmbes_iip iip;

    /* #IOP -  Runtime datagram */
    struct mbsys_kmbes_iop iop;

    /* #IBE - BIST error report datagram */
    struct mbsys_kmbes_ib ibe;

    /* #IBR - BIST reply datagram */
    struct mbsys_kmbes_ib ibr;

    /* #IBS - BIST short reply datagram */
    struct mbsys_kmbes_ib ibs;

    /* #FCF - Backscatter Calibration File Datagram */
    struct mbsys_kmbes_fcf fcf;

    /* #XMB - datagram indicating these data have been written by MB-System */
    struct mbsys_kmbes_xmb xmb;

    /* #XMC - datagram containing comment inserted by MB-System */
    struct mbsys_kmbes_xmc xmc;

    /* Unknown format */
    struct mbsys_kmbes_unknown_struct unknown;

};


/*---------------------------------------------------------------*/

/* System specific function prototypes */
/* Note: this list of functions corresponds to the function pointers
 * that are included in the structure mb_io_struct that is defined
 * in the file mbsystem/src/mbio/mb_io.h
 * Not all of these functions are required - some only make sense to
 * define if the relevant data type is part of the format. For instance,
 * do not define mbsys_kmbes_extract_segy() if there are no subbottom
 * profiler data supported by this data system.
 * The function prototypes that are not required for all data systems
 * are commented out below. When using this example as the basis for
 * for coding a new MB-System I/O module, uncomment any non-required
 * functions that will be useful. */

#ifdef __cplusplus
extern "C" {
#endif

int mbsys_kmbes_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_kmbes_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_kmbes_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_kmbes_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_kmbes_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
int mbsys_kmbes_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error);
int mbsys_kmbes_preprocess(int verbose, void *mbio_ptr, void *store_ptr,
                            void *platform_ptr, void *preprocess_pars_ptr, int *error);
// int mbsys_kmbes_extract_platform(int verbose, void *mbio_ptr, void *store_ptr,
//		int *kind, void **platform_ptr, int *error);
int mbsys_kmbes_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                                 char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                 double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_kmbes_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d,
                                double navlon, double navlat, double speed, double heading, int nbath, int namp, int nss,
                                char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_kmbes_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                     double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                                     double *pitch, double *heave, int *error);
// int mbsys_kmbes_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
//			int nmax, int *kind, int *n,
//			int *time_i, double *time_d,
//			double *navlon, double *navlat,
//			double *speed, double *heading, double *draft,
//			double *roll, double *pitch, double *heave,
//			int *error);
int mbsys_kmbes_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                                    double navlat, double speed, double heading, double draft, double roll, double pitch,
                                    double heave, int *error);
int mbsys_kmbes_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                          double *altitude, int *error);
// int mbsys_kmbes_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
//                        double transducer_depth, double altitude,
//                        int *error);
int mbsys_kmbes_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth,
                          double *velocity, int *error);
int mbsys_kmbes_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                          int *error);
int mbsys_kmbes_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes,
                          double *angles, double *angles_forward, double *angles_null, double *heave,
                          double *alongtrack_offset, double *draft, double *ssv, int *error);
int mbsys_kmbes_detects(int verbose, void *mbio_ptr, void *store_ptr,
			                    int *kind, int *nbeams, int *detects, int *error);
int mbsys_kmbes_pulses(int verbose, void *mbio_ptr, void *store_ptr,
                          int *kind, int *nbeams, int *pulses, int *error);
int mbsys_kmbes_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
// int mbsys_kmbes_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
// int mbsys_kmbes_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
// int mbsys_kmbes_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind,
//			void *segytraceheader_ptr,
//			int *error);
// int mbsys_kmbes_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int *sampleformat,
//			int *kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
// int mbsys_kmbes_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
// int mbsys_kmbes_ctd(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nctd, double *time_d,
//			double *conductivity, double *temperature,
//			double *depth, double *salinity, double *soundspeed, int *error);
// int mbsys_kmbes_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nsensor, double *time_d,
//			double *sensor1, double *sensor2, double *sensor3,
//			double *sensor4, double *sensor5, double *sensor6,
//			double *sensor7, double *sensor8, int *error);
int mbsys_kmbes_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_kmbes_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MBSYS_KMBES_H_ */
