/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_3ddwissl2.h  4/29/2025
 *
 *    Copyright (c) 2025-2025 by
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
 * mbsys_3ddwissl2.h defines the MBIO data structures for handling data from
 * the 3DatDepth WiSSL2 (2nd generation wide swath subsea lidar):
 *      MBF_3DWISSL2 : MBIO ID 232 - 3DatDepth WiSSL2 vendor format with MB-System extensions
 *
 * Author:  David W. Caress
 * Date:  April 29, 2025
 *
 */
/*
 * Notes on the MBSYS_3DDWISSL2 data structure:
 *
 * Vendor format from 3D at Depth for the WiSSL2 (2nd generation wide swath subsea lidar) 
 * system delivered to MBARI in April 2025.
 *
 * Initial coding done using example C++ code supplied by 3D at Depth in April 2025.
 * 
 * The WiSSL2 data are supported by an i/o module associated with a single format id. 
 * The WiSSL2 data are logged in (or converted to) files with a *.sriat suffix that are
 * read as format MBF_3DWISSL2 (format id 134). Format 134 files written by MB-System 
 * use data records that are extended from the initial records.
 */

#ifndef MBSYS_3DDWISSL2_H_
#define MBSYS_3DDWISSL2_H_

#include "mb_define.h"

/* defines */
#define SRIAT_SYNC_WORD 0xC0DEFACE // unswapped byte order: CE FA DE C0
#define SRIAT_HEADER_VERSION CRIAAT_FILE_VERSION 6
#define SRIAT_RECORD_ID_RANGE 	    1
#define SRIAT_RECORD_ID_THERMAL     2
#define SRIAT_RECORD_ID_MBARI       3
#define SRIAT_RECORD_ID_FILEHEADER  4
#define SRIAT_RECORD_ID_TAIL        5
#define SRIAT_RECORD_ID_PIA         6
#define SRIAT_RECORD_ID_COMMENT     7

#define SRIAT_RECORD_SIZE_FILEHEADER  187
#define SRIAT_RECORD_SIZE_RANGE_HEADER  72
#define SRIAT_RECORD_SIZE_MBARI_HEADER  160
#define SRIAT_RECORD_SIZE_MBARI_SOUNDING  60
#define SRIAT_RECORD_SIZE_COMMENT_HEADER 12

enum
{
   PACKET_ID_0 = 0,       // reserved for not yet defined?
   PACKET_ID_RANGE = 1,   // Range data, 2 point return. Scan line header.
   PACKET_ID_THERMAL = 2, // Thermal data, MBARI Raman data. Scan line header.
   PACKET_ID_MBARI = 3,   // Special MBARI data packet (wrapped and inserted into data stream)
   PACKET_ID_HEADER = 4,  // File Header info - first one. Will not need pointer or info about this one.
   PACKET_ID_TAIL = 5,    // File Tail   info - last  one. Will not need pointer or info about this one.
   PACKET_ID_PIA = 6,     // Packet informational area - end of file of where data is at.
};

#define MBSYS_3DDWISSL2_DEFAULT_AMPLITUDE_THRESHOLD 	1.5
#define MBSYS_3DDWISSL2_DEFAULT_TARGET_ALTITUDE        3.0

/*--------------------------------------------------------------------------------------*/
/* file header record: 187 bytes */
/*--------------------------------------------------------------------------------------*/
struct mbsys_3ddwissl2_sriat_fileheader_struct
{
   unsigned int SyncWord;
   mb_u_char PacketID;            // The packet type / ID = PACKET_ID_HEADER
   mb_u_char Version;             // packet version number
   unsigned int SizeBytes;           // define size of header in bytes.
   // --- above is common header format ---

   unsigned int ScanSizeBytes;       // define size of Data AND this header - Fill in at end.

   /////////////////////////////////////////////////////////////////////////////
   // --- Time Stamp - use TimeSpec: (Sec and nSec from Epic)-------------------
   // This file covers from time start to time end. 
   int TimeStart_Sec ;       // Rollover in 2038 if this is signed.
   int TimeStart_nSec;       //   signed allows math to add or subtract right.
    
   int TimeEnd_Sec ;         // Rollover in 2038 if this is signed.
   int TimeEnd_nSec;         //   signed allows math to add or subtract right.
   // --- Time Stamp End -------------------------------------------------------
   /////////////////////////////////////////////////////////////////////////////

   mb_u_char SL_GEN   ;           // SL Generation The "#" {SL"2", SL"3", SL"4", SL"5", SL"6"}, the number not letter
   mb_u_char SL_Letter;           // SL#L-xxxx The L = "N"uclear, "D"eep, "P"ipeline, "M"bari part.
   mb_u_char SL_X     ;           // SL XXX number of the name SL4(N)-XXX
   mb_u_char nPtsToAverage;       // default 1, only EP will set to a different value.
      
   char cJobName[24]; // Job folder
   char cScanPos[24]; // Position Folder.
   char cfileTag[24]; // file name.
   unsigned short nScanNum;     // pulled from fileTag or auto counts each scan (UDP).

   int AzCmdStart            ;// Full resolution raw counts. Start taking data here.
   int AzCmdEnd              ;// Full resolution raw counts. Stop  taking data here.

	 unsigned int rawbit1;
   unsigned int      nPtsPerScanLine   : 14;// 10 - 10,000 <= nPtsPerLine (actual number of Laser shots in angle per row).
   unsigned int      nScanLinesPerScan : 12;// 10 - 3800 (4095 bit limit)
   unsigned int      Spare1            : 6;

	 unsigned int rawbit2;
   unsigned int      nPtsPerLine       : 14;// 10 - 10,000 define a full line of points (full 360 rotation count)
   unsigned int      Mode              :  3;// scan mode { RA, FA, AZ, EL, EP } 
   unsigned int      nTPtsPerScanLine  : 14;// 10 - 10,000 <= nPtsPerLine (actual number of Thermal points in scan).
   unsigned int      bHaveThermal      :  1;// 1 = Have Thermal Data, 0 = no Thermal Data or Packets.

   unsigned int ShotCnt;           // Max is 14,500,000 (24 bits), don't include averages. Number in SCAN, windowed down by Az Start/Stop angles.
   //unsigned short Conductivity_uS_cm;// Range 0-65535 uSiemens per cm (note uS/Cm * 1/10,000 = S/m)
   unsigned short WaterSalinity_psu ;  // Range 0-42 PSU. psu = #*42/65535;  Calculated from CTD.
   unsigned short WaterPressure_dbar;  // Range 0-6000 (13 bits) 1 dbar = 1 meter.
   
	 unsigned int rawbit3;
   unsigned int WaterTemperature_C : 13;// Range -2 to 35. Cal is C = #*37/8191 - 2;
   unsigned int PRF_Hz             : 19;// current laser shot rate. 500,000 max, 19 bits. 

   mb_u_char DigitizerTemperature_C; // C = #*100/255;
   
   //unsigned int Motor_Hz    ;      // current motor spin rate - pts/line and prf can calculate.

   float RScale_m_per_cnt; // Need exact range per count for accuracy.

   short ThBinStart_cnt; // Bin Count start, AzCmdStart for thermal. Az angle/TempAzCnt = bin
   short ThBinEnd_cnts ; // Bin Count end,   AzCmdEnd   for thermal.

   // Temperature Profile definition ----------------------
   mb_u_char TempAzCnt ; // number of shots to average in Az   for a Temperature Bin. 1-255. Default 10.
   mb_u_char TempRowCnt; // number of shots to average in Rows for a Temperature Bin. 1-255. Default 10.  
   // Is range average fixed or some sort of profile? Note: Only reports <128 bins and <AvAz*AvEl count.
   //        [0] = 0    //  0 averages of 1  in range Skip? Did not implement just single sample.
   //        [1] = 8    //  8 averages of 2  in range for the fist 8*2   = 16  (must be divisable by 16)
   //        [2] = 12   // 12 averages of 4  in range for the next 12*4  = 48  (must be divisable by 16)
   //        [3] = 28   // 28 averages of 8  in range for the next 28*8  = 224 (must be divisable by 16)
   //        [4] = 52   // 51 averages of 16 in range for the next 52*16 = 832 (by definition is divisable by 16)
	 unsigned int rawbit4;
   unsigned int TempRCnt_av2  : 8;  // number of shots to average in Range for a Temperature Range Bin. 1-128 
   unsigned int TempRCnt_av4  : 8;  // number of shots to average in Range for a Temperature Range Bin. 1-128
   unsigned int TempRCnt_av8  : 8;  // number of shots to average in Range for a Temperature Range Bin. 1-128
   unsigned int TempRCnt_av16  : 8;  // number of shots to average in Range for a Temperature Range Bin. 1-128
   // end Temperature Profile definition ----------------

   unsigned short ScannerShift_mDeg;  // 1 = 0.001 degrees.

   // Used when converting file from Range to XYZ.
   float Shift_m[3];           // Additional shift to apply
   // Rotation with respect to sensitive point. order is {X, Y, then Z}.
   float Rotate_deg[3];        // Additional rotation to apply  
   mb_u_char EC_Version[4];        // Capture Software version info.
   mb_u_char InstaCloud_Version[4];// [0]= MSB, [3]=LSB, Example: 7.1.1.255
   short ElDeg_cnts;           // cal is 90 degees /0xFFFF, 0 is perpendicular to output cap

}; 

/*--------------------------------------------------------------------------------------* 
 * Range record
 *--------------------------------------------------------------------------------------*/

// Line Data
// StructLineCriaat[PointsRows]
//
// Point Data
// AZ[num shots] - only 1 per shot
// R [num shots * Points_per_LOS]  - always 2 points/shot
// I [num shots * Points_per_LOS]  - always 2 points/shot
// Class[num shots * Points_per_LOS]- always 2 points/shot ... 
//   Classification { 0=Unclass, Ignore, LowRange, HiRange, LowIndex, HiIndex, Clutter }
//
// ------ only MBARI has this second part ----
// R0     [num shots] - Red 0 signal             
// Ratio  [num shots] - Red 1/ Red 0 signal      
// R0_SNR [num shots] - Red 0 SNR measure 
// TClass [num shots] - Classification { 0=Unclass, Ignore, LowSNR, Clutter, HiSNR, Good, Glow }
//
// ---- SL4 has this
// EL[num shots] - only 1 per shot
//

// Classification defines:
enum {
   CLASS_Unclass   = 0,
   CLASS_Ignore    = 1, // outside of Az acceptance window
   CLASS_LowRange  = 2,
   CLASS_HiRange   = 3,
   CLASS_LowReturn = 4,
   CLASS_HiReturn  = 5, // ie - Saturated
   CLASS_Clutter   = 6,
   CLASS_Bad       = 7, // generic bad data?
   // Everything below ... 7 is dropped data?
   CLASS_Good      = 8,
   CLASS_Glow      = 9, // Thermal measure beyond range return.
   // Values 10 - 15 available.
};

// ---------------------------------------------------------------------

/* SRIAT range record structure 
		- This represents the structure of the raw SRIAT range record 
		- the MBARI WiSSL2 SRIAT i/o module can read the raw SRIAT range record but store it
			into an extended structure that will be written out as an MBARI range record
		- Format 234 files written by MB-System will therefore always consist of the extended
			MBARI range records, represented in the mbarirange structure defined below. */
struct mbsys_3ddwissl2_sriatrange_struct
{ 
   unsigned int SyncWord;
   mb_u_char PacketID;            // The packet type / ID = PACKET_ID_RANGE
   mb_u_char Version  ;
   unsigned int SizeBytes;           // The Variable Range data size AND this header (total size Range Packet)
   // ---
   unsigned short HdrSizeBytes;        // define size of header in bytes.
   
   ////// Timestamp - absolute ///////
   unsigned int TimeStart_Sec ;       // Rollover in 2038 if this is signed.
   unsigned int TimeStart_nSec;       //   signed allows math to add or subtract right.
   ///////////////////////////////////

   unsigned short NumPtsRow          ; // (14 bits - max is 16,383) Number of shots per rotation of motor.
   unsigned short NumPtsPkt          ; // (14 bits - max is 16,383) Number of shots in this packet <= NumPtsRow.
   unsigned int LineLaserPower     ; // Fraction of max power output - both gain setting and attinuator, 20 bits full res 
   
   unsigned int rawbit1;
   unsigned int PRF_Hz         : 19; // current laser shot rate. Used with timestamp for point spacing. Max 500,000.
   unsigned int Spare1         :  7; // balance
   unsigned int Points_per_LOS :  2; // 0=good data only, 1=1 (always 1), 2=2 (always 2 entries)
   unsigned int ScannerType    :  4; // {MBARI, PIPE, SL4, SL5, SL6} - TBD.  

   short lineAccelX         ; // 16 bits - signed acceleration, full res
   short lineAccelY         ; // 16 bits - signed acceleration, full res
   short lineAccelZ         ; // 16 bits - signed acceleration, full res  
   unsigned short lineIndex          ; // U16.15 Water index
   
   unsigned short RowNumber          ; // 0-3800 (12 bits), data is for this row.
   // ---------------------------------------------------------------------
   // Monitor laser output amplitude for thermal control of SHG TEC.
   unsigned short      SHGAmplitudeAv; // 12 bit average for line.
   // keep Max R / Max I?
   unsigned int rawbit2;
   unsigned int		R_Max : 20;     // represented in raw counts,    20.14 m 
   unsigned int		I_Max : 12;     // represented in raw counts,    12 bits
   // Send AutoR / AutoI? -> needed for LAS CCI
   unsigned int rawbit3;
   unsigned int		R_Auto: 20;     // represented in raw counts,    20.14 m 
   unsigned int		I_Auto: 12;     // represented in raw counts,    12 bits
   // Strongest mode? - all returns (AGC - scan info)
   unsigned int rawbit4;
   unsigned int		R_Mode: 20;     // represented in raw counts,    20.14 m 
   unsigned int		I_Mode: 12;     // represented in raw counts,    12 bits
   // Info about the scan line distribution for Laser Power, AGC:
   mb_u_char       I_Good    ;     // Percent in good range: 30% to 90% of full scale.
   mb_u_char       I_Low     ;     // Percent in low  range: <30%  of full scale.
   mb_u_char       I_High    ;     // Percent in high range: >90%  of full scale.
   mb_u_char       Spare3    ;     // header sized for 32 bit alignment.
   // ---------------------------------------------------------------------

   unsigned int rawbit5;
   unsigned int		R_offset  : 20; // represented in raw counts,    20 bits
   unsigned int		I_offset  : 12; // represented in raw counts,    12 bits

	 int            AZ_offset     ; // represented in raw counts,    s18 bits

   unsigned int rawbit6;
   unsigned int      R_nbits   :  5; // number of bits each 0-20
   unsigned int      I_nbits   :  4; // number of bits each 0-12
   unsigned int      AZ_nbits  :  5; // number of bits each 0-18
   unsigned int      Spare2    :  18;
  // ---------------------------------------------------------------------
   
  //mb_u_char* PointData[variable];
  //U*: AZ[NumPtsPkt] - only 1 per shot (but only if always setting 2 per shot)
  //U*: R [NumPtsPkt * Points_per_LOS]  - always 2 points/shot
  //U*: I [NumPtsPkt * Points_per_LOS]  - always 2 points/shot
  //U4: Class[NumPtsPkt * Points_per_LOS]- always 2 points/shot ... 
  //		Classification { 0=Unclass, Ignore, LowRange, HiRange, LowIndex, HiIndex, Clutter }
  size_t sriat_num_samples_alloc;
  unsigned int *sriat_Az;            // U18 cal = 360 deg / 0x3FFFF
  unsigned int *sriat_Range1;         // U20 at .1 mm per count.
  unsigned int *sriat_Range2;         // U20 at .1 mm per count.
  unsigned short *sriat_Intensity1;   // U12 raw value
  unsigned short *sriat_Intensity2;   // U12 raw value
  mb_u_char *sriat_ClassR1;       // U4 Range class
  mb_u_char *sriat_ClassR2;       // U4 Range class
  
}; // row info, one for each row

// ---------------------------------------------------------------------

struct mbsys_3ddwissl2_sounding_struct
  {
  unsigned short int pulse_id;		/* pulse number in the original reported scan */
  unsigned short int sounding_id;					/* 0 or 1 */
  float time_offset;							/* time offset relative to start of scan (seconds) */

  /* lidar reference point navigation relative to the start of the scan */
  float acrosstrack_offset;  	/* m */
  float alongtrack_offset;  		/* m */
  float sensordepth_offset;  	/* m */
  float heading_offset;    			/* deg */
  float roll_offset;      			/* deg */
  float pitch_offset;      			/* deg */

  /* raw information */
  float range;      /* meters from glass front */
  float angle_az;  /* Acrosstrack angle, zero = vertical down, positive to starboard (deg) */
  float angle_el;  /* Alongtrack angle, zero = vertical down, positive forward, (deg) */
  short intensity;    /* peak of signal - to 1023 */
  mb_u_char class;  /* diagnostic value - unknown meaning, >= V1.2 only */

  /* processed information incorporating pulse offsets and head offsets */
  mb_u_char beamflag;  	/* MB-System beam flag */
  float acrosstrack;  	/* acrosstrack distance relative lidar reference point (meters) */
  float alongtrack;    	/* alongtrack distance relative to lidar reference point (meters) */
  float depth;      		/* depth relative to lidar reference point (meters) */
  };

/* MBARI range record structure  
		- This represents the structure of the extended MBARI range record 
		- The MBARI WiSSL2 SRIAT i/o module can read the raw SRIAT range record structured
			as defined above, but stores it into an extended structure that will be written out 
			as an MBARI range record.
		- Format 234 files written by MB-System will therefore always consist of the extended
			MBARI range records, represented in the mbarirange structure defined here. */
struct mbsys_3ddwissl2_mbarirange_struct
{ 
   unsigned int SyncWord;
   mb_u_char PacketID;            // The packet type / ID = PACKET_ID_RANGE
   mb_u_char Version  ;
   unsigned int SizeBytes;           // The Variable Range data size AND this header (total size Range Packet)
   // ---------------------------------------------------------------------
   unsigned short HdrSizeBytes;        // define size of header in bytes.
   unsigned int TimeStart_Sec;       // Rollover in 2038 if this is signed.
   unsigned int TimeStart_nSec;       //   signed allows math to add or subtract right.
   unsigned short NumPtsRow; // (14 bits - max is 16,383) Number of shots per rotation of motor.
   unsigned short NumPtsPkt; // (14 bits - max is 16,383) Number of shots in this packet <= NumPtsRow.
   unsigned int LineLaserPower; // Fraction of max power output - both gain setting and attinuator, 20 bits full res 
   unsigned int PRF_Hz; // current laser shot rate. Used with timestamp for point spacing. Max 500,000.
   unsigned short Points_per_LOS; // 0=good data only, 1=1 (always 1), 2=2 (always 2 entries)
   unsigned short ScannerType; // {MBARI, PIPE, SL4, SL5, SL6} - TBD.  
   short lineAccelX; // 16 bits - signed acceleration, full res
   short lineAccelY; // 16 bits - signed acceleration, full res
   short lineAccelZ; // 16 bits - signed acceleration, full res  
   unsigned short lineIndex; // U16.15 Water index
   unsigned short RowNumber; // 0-3800 (12 bits), data is for this row.
   // ---------------------------------------------------------------------
   // Monitor laser output amplitude for thermal control of SHG TEC.
   unsigned short      SHGAmplitudeAv; // 12 bit average for line.

   // keep Max R / Max I?
   unsigned int		R_Max;     // represented in raw counts,    20.14 m 
   unsigned int		I_Max;     // represented in raw counts,    12 bits

   // Send AutoR / AutoI? -> needed for LAS CCI
   unsigned int		R_Auto;     // represented in raw counts,    20.14 m 
   unsigned int		I_Auto;     // represented in raw counts,    12 bits

   // Strongest mode? - all returns (AGC - scan info)
   unsigned int		R_Mode;     // represented in raw counts,    20.14 m 
   unsigned int		I_Mode;     // represented in raw counts,    12 bits

   // Info about the scan line distribution for Laser Power, AGC:
   mb_u_char       I_Good;     // Percent in good range: 30% to 90% of full scale.
   mb_u_char       I_Low;      // Percent in low  range: <30%  of full scale.
   mb_u_char       I_High;     // Percent in high range: >90%  of full scale.
   mb_u_char       I_Spare;    // header sized for 32 bit alignment.
   // ---------------------------------------------------------------------

   unsigned int		R_offset; // represented in raw counts,    20 bits
   unsigned int		I_offset; // represented in raw counts,    12 bits
	 int            AZ_offset; // represented in raw counts,    s18 bits
  // ---------------------------------------------------------------------

  /* merged navigation and attitude per each scan */
  double time_d;                /* epoch time of the first pulse in this scan */
  double navlon;                /* lidar reference point position longitude (degrees) */
  double navlat;                /* lidar reference point  position latitude (degrees) */
  double sensordepth;   				/* lidar reference point  position depth below sea surface (meters), includes any tide correction */
  double speed;                	/* lidar speed (m/s) */
  double heading;               /* lidar heading (degrees) */
  double roll;                  /* lidar roll (degrees) */
  double pitch;                	/* lidar pitch (degrees) */
  unsigned int num_soundings; 	/* number of soundings stored in this record 
  																	- originally the scan included results from a subset
  																	  of pulses over a limited angle range, with two 
  																	  soundings per each pulse. Some picks are null and
  																	  others are valid. 
  																	- the output file with processed bathymetry may contain
  																	  all of the original soundings (null and valid), or
  																	  a subset of the soundings.
  																  - The most common subsets will be:
  																  		- all valid soundings
  																  		- all first soundings 
  																  		- all valid first soundings */
  unsigned int num_soundings_alloc; /* number of soundings allocated to be stored in this record */

  /* soundings */
  struct mbsys_3ddwissl2_sounding_struct *soundings;
     
}; 

/* MBARI comment record */
struct mbsys_3ddwissl2_comment_struct
{ 
   unsigned int SyncWord;
   mb_u_char PacketID;                // The packet type / ID = PACKET_ID_RANGE
   mb_u_char Version;
   unsigned int SizeBytes;            // Total size comment record
   unsigned short comment_len;        // comment length in bytes, including one or more 
   																		//   end-of-string null bytes, 
   																		//   e.g. comment_len = strlen(comment->comment) + 1;
   char comment[MB_COMMENT_MAXLINE];  // comment string 
     
}; 
// ---------------------------------------------------------------------

/* Data structure for 3DatDepth WiSSL2 Lidar data */
struct mbsys_3ddwissl2_struct
  {
  /* Type of data record */
  int kind;  /* MB-System record ID */
  
  /* File header */
  struct mbsys_3ddwissl2_sriat_fileheader_struct fileheader;
  
  /* MBARI processing lidar scan */
  bool bathymetry_calculated;
  struct mbsys_3ddwissl2_mbarirange_struct mbarirange;

  /* comment */
  struct mbsys_3ddwissl2_comment_struct comment;
  };
// ---------------------------------------------------------------------

/* System specific function prototypes */
int mbsys_3ddwissl2_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl2_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl2_dimensions(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbath,
  int *namp,
  int *nss,
  int *error);
int mbsys_3ddwissl2_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_3ddwissl2_preprocess(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  void *platform_ptr,
  void *preprocess_pars_ptr,
  int *error);
int mbsys_3ddwissl2_sensorhead(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *sensorhead,
  int *error);
int mbsys_3ddwissl2_extract(int verbose,
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
int mbsys_3ddwissl2_insert(int verbose,
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
int mbsys_3ddwissl2_ttimes(int verbose,
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
int mbsys_3ddwissl2_detects(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbeams,
  int *detects,
  int *error);
int mbsys_3ddwissl2_pulses(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbeams,
  int *pulses,
  int *error);
int mbsys_3ddwissl2_gains(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  double *transmit_gain,
  double *pulse_length,
  double *receive_gain,
  int *error);
int mbsys_3ddwissl2_extract_altitude(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  double *transducer_depth,
  double *altitude,
  int *error);
int mbsys_3ddwissl2_extract_nnav(int verbose,
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
int mbsys_3ddwissl2_extract_nav(int verbose,
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
int mbsys_3ddwissl2_insert_nav(int verbose,
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
int mbsys_3ddwissl2_extract_svp(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nsvp,
  double *depth,
  double *velocity,
  int *error);
int mbsys_3ddwissl2_insert_svp(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int nsvp,
  double *depth,
  double *velocity,
  int *error);
int mbsys_3ddwissl2_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_3ddwissl2_print_store(int verbose, void *store_ptr, int *error);
int mbsys_3ddwissl2_calculatebathymetry(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  double amplitude_threshold,
  double target_altitude,
  int *error);

#endif  /* MBSYS_3DDWISSL2_H_ */
