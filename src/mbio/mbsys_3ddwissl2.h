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
#define SRIAT_HEADER_VERSION CRIAAT_FILE_VERSION 5
#define SRIAT_RECORD_ID_RANGE 	    1
#define SRIAT_RECORD_ID_THERMAL     2
#define SRIAT_RECORD_ID_MBARI       3
#define SRIAT_RECORD_ID_FILEHEADER  4
#define SRIAT_RECORD_ID_TAIL        5
#define SRIAT_RECORD_ID_PIA         6
#define SRIAT_RECORD_SIZE_FILEHEADER  177
#define SRIAT_RECORD_SIZE_RANGE_HEADER  63

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

/*--------------------------------------------------------------------------------------*/
/* file header record: 177 bytes */
/*--------------------------------------------------------------------------------------*/
typedef struct
{

   unsigned char     PacketID;            // The packet type / ID = PACKET_ID_HEADER
   unsigned char     Version;             // packet version number
   unsigned short    SizeBytes;           // define size of header in bytes.
   // --- above is common header format ---

   unsigned int      ScanSizeBytes;       // define size of Data (does not include header) - Fill in at end.

   /////////////////////////////////////////////////////////////////////////////
   // --- Time Stamp - use TimeSpec: (Sec and nSec from Epic)-------------------
   // This file covers from time start to time end. 
   int               TimeStart_Sec ;       // Rollover in 2038 if this is signed.
   int               TimeStart_nSec;       //   signed allows math to add or subtract right.
    
   int               TimeEnd_Sec ;         // Rollover in 2038 if this is signed.
   int               TimeEnd_nSec;         //   signed allows math to add or subtract right.
   // --- Time Stamp End -------------------------------------------------------
   /////////////////////////////////////////////////////////////////////////////

   unsigned char     SL_GEN   ;             // SL Generation The "#" {SL"2", SL"3", SL"4", SL"5", SL"6"} or {MBARI, PIPE, SL4, SL5, SL6}
   unsigned char     SL_Letter;             // SL#L-xxxx The L = "N"uclear, "D"eep, "P"ipeline, "M"bari part.
   unsigned char     SL_X     ;             // SL XXX number of the name SL4(N)-XXX
   unsigned char     nPtsToAverage;         // default 1, only EP will set to a different value.

   char              cJobName[24];          // Job folder
   char              cScanPos[24];          // Position Folder.
   char              cfileTag[24];          // file name.

   unsigned short    nScanNum;              // pulled from fileTag or auto counts each scan (UDP).

   unsigned int      rawbit1;
   unsigned int      nPtsPerScanLine;       // bitfield : 14; // 10 - 10,000 <= nPtsPerLine (actual number of Range points in scan).
   unsigned int      AzCmdStart;            // bitfield : 18; // Full resolution raw counts. Start taking data here.

   unsigned int      rawbit2;
   unsigned int      AzCmdEnd;              // bitfield : 18; // Full resolution raw counts. Stop  taking data here.
   unsigned int      nScanLinesPerScan;     // bitfield : 12; // 10 - 3800 (4095 bit limit)
   unsigned int      Spare1;                // bitfield : 2;

   unsigned int      rawbit3;
   unsigned int      nPtsPerLine;           // bitfield : 14; // 10 - 10,000 define a full line of points (full rotation count)
   unsigned int      Mode;                  // bitfield :  3; // scan mode { RA, FA, AZ, EL, EP } 
   unsigned int      nTPtsPerScanLine;      // bitfield : 14; // 10 - 10,000 <= nPtsPerLine (actual number of Thermal points in scan).
   unsigned int      Spare2;                // bitfield : 1;

   unsigned int      ShotCnt;               // Max is 14,500,000 (24 bits), don't include averages. Number in SCAN, windowed down by Az Start/Stop angles.
   //unsigned short    Conductivity_uS_cm;  // Range 0-65535 uSiemens per cm (note uS/Cm * 1/10,000 = S/m)
   unsigned short    WaterSalinity_psu ;    // Range 0-42 PSU. psu = #*42/65535;  Calculated from CTD.
   unsigned short    WaterPressure_dbar;    // Range 0-6000 (13 bits) 1 dbar = 1 meter.
   
   unsigned int      rawbit4;
   unsigned int      WaterTemperature_C;    // bitfield : 13; // Range -2 to 35. Cal is C = #*37/8191 - 2;
   unsigned int      PRF_Hz;                // bitfield : 19; // current laser shot rate. 500,000 max, 19 bits. 

   unsigned char     DigitizerTemperature_C; // C = #*100/255;
   
   //unsigned int      Motor_Hz    ;      // current motor spin rate - pts/line and prf can calculate.

   float             RScale_m_per_cnt;      // Need exact range per count for accuracy.

   unsigned short    ThBinStart_cnt;        // Bin Count start, AzCmdStart for thermal. Az angle/TempAzCnt = bin
   unsigned short    ThBinEnd_cnts ;        // Bin Count end,   AzCmdEnd   for thermal.

   // Temperature Profile definition ----------------------
   unsigned char     TempAzCnt ; // number of shots to average in Az   for a Temperature Bin. 1-255. Default 10.
   unsigned char     TempRowCnt; // number of shots to average in Rows for a Temperature Bin. 1-255. Default 10.  
   // Is range average fixed or some sort of profile? Note: Only reports <128 bins and <AvAz*AvEl count.
   //        [0] = 0    //  0 averages of 1  in range Skip? Did not implement just single sample.
   //        [1] = 8    //  8 averages of 2  in range for the fist 8*2   = 16  (must be divisable by 16)
   //        [2] = 12   // 12 averages of 4  in range for the next 12*4  = 48  (must be divisable by 16)
   //        [3] = 28   // 28 averages of 8  in range for the next 28*8  = 224 (must be divisable by 16)
   //        [4] = 52   // 51 averages of 16 in range for the next 52*16 = 832 (by definition is divisable by 16)
   unsigned int      rawbit5;
   unsigned int      TempRCnt_av2;   // bitfield : 8; // number of shots to average in Range for a Temperature Range Bin. 1-128 
   unsigned int      TempRCnt_av4;   // bitfield : 8; // number of shots to average in Range for a Temperature Range Bin. 1-128
   unsigned int      TempRCnt_av8;   // bitfield : 8; // number of shots to average in Range for a Temperature Range Bin. 1-128
   unsigned int      TempRCnt_av16;  // bitfield : 8; // number of shots to average in Range for a Temperature Range Bin. 1-128
   // end Temperature Profile definition ----------------

   unsigned short     ScannerShift_mDeg;  // 1 = 0.001 degrees.

   // Used when converting file from Range to XYZ.
   float       Shift_m[3];           // Additional shift to apply
   // Rotation with respect to sensitive point. order is {X, Y, then Z}.
   float       Rotate_deg[3];        // Additional rotation to apply  
   unsigned char       EC_Version[4];        // Capture Software version info.
   unsigned char       InstaCloud_Version[4];// [0]= MSB, [3]=LSB, Example: 7.1.1.255
   short       ElDeg_cnts;           // cal is 90 degees /0xFFFF, 0 is perpendicular to output cap
} mbsys_3ddwissl2_sriat_fileheader_struct; 

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
// Class[num shots * Points_per_LOS]- always 2 points/shot ... Classification { 0=Unclass, Ignore, LowRange, HiRange, LowIndex, HiIndex, Clutter }
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

/* SRIAT range record structure */
typedef struct
{ 
  unsigned char   PacketID;           // The packet type / ID = PACKET_ID_RANGE
  unsigned char   Version;
  unsigned short  SizeBytes;           // define size of header in bytes.
  // ---
  unsigned int    DataSizeBytes;       // The Variable Range data size AND this header (total size Range Packet)
  
  ///////////////////////////////////
  int             TimeStart_Sec;       // Rollover in 2038 if this is signed.
  int             TimeStart_nSec;      //   signed allows math to add or subtract right.
  //unsigned int DeltaTimeStamp_S    : 12; // 4095 (real max is probably 180 seconds)
  //unsigned int DeltaTimeStamp_uS   : 20; // 1,000,000
  ///////////////////////////////////

  unsigned short  NumPtsRow;          // (14 bits - max is 16,383) Number of shots per rotation of motor.
  unsigned short  NumPtsPkt;          // (14 bits - max is 16,383) Number of shots in this packet <= NumPtsRow.
  unsigned int    LineLaserPower;     // Fraction of max power output - both gain setting and attenuator, 20 bits full res 
  
  unsigned int    rawbit1;
  unsigned int    PRF_Hz;             // bitfield : 19; // current laser shot rate. Used with timestamp for point spacing. Max 500,000.
  unsigned int    Spare1;             // bitfield : 7; // ballence
  unsigned int    Points_per_LOS;     // bitfield : 2; // 0=good data only, 1=1 (always 1), 2=2 (always 2 entries)
  unsigned int    ScannerType;        // bitfield : 4; // {MBARI, PIPE, SL4, SL5, SL6} - TBD.  

  short           lineAccelX;         // 16 bits - signed acceleration, full res
  short           lineAccelY;         // 16 bits - signed acceleration, full res
  short           lineAccelZ;         // 16 bits - signed acceleration, full res  
  unsigned short  lineIndex;          // U16.15 Water index
  
  unsigned short  RowNumber;          // 0-3800 (12 bits), data is for this row.
  // ---------------------------------------------------------------------
  // keep Max R / Max I?
  unsigned int    rawbit2;
  unsigned int	   R_Max;              // bitfield : 20;     // represented in raw counts,    20.14 m 
  unsigned int    I_Max;              // bitfield : 12;     // represented in raw counts,    12 bits
  // Send AutoR / AutoI? -> needed for LAS CCI
  unsigned int    rawbit3;
  unsigned int    R_Auto;             // bitfield : 20;     // represented in raw counts,    20.14 m 
  unsigned int    I_Auto;             // bitfield : 12;     // represented in raw counts,    12 bits
  // Strongest mode? - all returns (AGC - scan info)
  unsigned int    rawbit4;
  unsigned int    R_Mode;             // bitfield : 20;     // represented in raw counts,    20.14 m 
  unsigned int    I_Mode;             // bitfield : 12;     // represented in raw counts,    12 bits
  // Info about the scan line distribution for Laser Power, AGC:
  unsigned char   I_Good;             // Percent in good range: 30% to 90% of full scale.
  unsigned char   I_Low;              // Percent in low  range: <30%  of full scale.
  unsigned char   I_High;             // Percent in high range: >90%  of full scale.
  // Monitor laser output amplitude for thermal control of SHG TEC.
  unsigned short  SHGAmplitudeAv;     // 12 bit average for line.
  // ---------------------------------------------------------------------

  unsigned int    rawbit5;
  unsigned int    R_offset;           // bitfield : 20; // represented in raw counts,    20 bits
  unsigned int    I_offset;           // bitfield : 12; // represented in raw counts,    12 bits

  unsigned int    rawbit6;
  unsigned int    AZ_offset;          // bitfield : 18; // represented in raw counts,    18 bits                        
  unsigned int    R_nbits;            // bitfield : 5; // number of bits each 0-20
  unsigned int    I_nbits;            // bitfield : 4; // number of bits each 0-12
  unsigned int    AZ_nbits;           // bitfield : 5; // number of bits each 0-18
  // ---------------------------------------------------------------------
   
  //unsigned char* PointData[variable];
  //U*: AZ[NumPtsPkt] - only 1 per shot (but only if always setting 2 per shot)
  //U*: R [NumPtsPkt * Points_per_LOS]  - always 2 points/shot
  //U*: I [NumPtsPkt * Points_per_LOS]  - always 2 points/shot
  //U4: Class[NumPtsPkt * Points_per_LOS]- always 2 points/shot ... Classification { 0=Unclass, Ignore, LowRange, HiRange, LowIndex, HiIndex, Clutter }
  size_t sriat_num_samples_alloc;
  unsigned int *sriat_Az;            // U18 cal = 360 deg / 0x3FFFF
  unsigned int *sriat_Range1;         // U20 at .1 mm per count.
  unsigned int *sriat_Range2;         // U20 at .1 mm per count.
  unsigned short *sriat_Intensity1;   // U12 raw value
  unsigned short *sriat_Intensity2;   // U12 raw value
  unsigned char *sriat_ClassR1;       // U4 Range class
  unsigned char *sriat_ClassR2;       // U4 Range class

   
} mbsys_3ddwissl2_sriatrange_struct; // row info, one for each row

/* MBARI range record structure */
typedef struct
{ 
  unsigned char   PacketID;           // The packet type / ID = PACKET_ID_RANGE
  unsigned char   Version;
  unsigned short  SizeBytes;           // define size of header in bytes.
  // ---
  unsigned int    DataSizeBytes;       // The Variable Range data size AND this header (total size Range Packet)
  
  ///////////////////////////////////
  int             TimeStart_Sec;       // Rollover in 2038 if this is signed.
  int             TimeStart_nSec;      //   signed allows math to add or subtract right.
  //unsigned int DeltaTimeStamp_S    : 12; // 4095 (real max is probably 180 seconds)
  //unsigned int DeltaTimeStamp_uS   : 20; // 1,000,000
  ///////////////////////////////////

  unsigned short  NumPtsRow;          // (14 bits - max is 16,383) Number of shots per rotation of motor.
  unsigned short  NumPtsPkt;          // (14 bits - max is 16,383) Number of shots in this packet <= NumPtsRow.
  unsigned int    LineLaserPower;     // Fraction of max power output - both gain setting and attenuator, 20 bits full res 
   
} mbsys_3ddwissl2_mbarirange_struct; 

/* 3DatDepth LIDAR data structure */
struct mbsys_3ddwissl2_struct
  {
  /* Type of data record */
  int kind;  /* MB-System record ID */
  
  /* File header */
  mbsys_3ddwissl2_sriat_fileheader_struct fileheader;
  
  /* SRIAT form lidar scan */
  mbsys_3ddwissl2_sriatrange_struct sriatrange;
  
  /* MBARI processing lidar scan */
  mbsys_3ddwissl2_mbarirange_struct mbarirange;

  /* comment */
  unsigned short comment_len;          /* comment length in bytes */
  char comment[MB_COMMENT_MAXLINE];  /* comment string */
  };




typedef struct
{  
   unsigned char       Packet_ID;           // The packet type / ID = PACKET_ID_THERMAL
   unsigned char			Version  ;
   unsigned short      SizeBytes;           // define size of header in bytes.
   // ---
   unsigned int      DataSizeBytes;       // The Variable Thermal data size AND this header (total size Thermal Packet)
   // Timestamp - absolute or relitive?
   int      TimeStart_Sec ;       // Rollover in 2038 if this is signed.
   int      TimeStart_nSec;       //   signed allows math to add or subtract right.
   //unsigned int DeltaTimeStamp_S    : 12; // 4095 (real max is probably 180 seconds)
   //unsigned int DeltaTimeStamp_uS   : 20; // 1,000,000
   ///////////////////////////////////

   unsigned short NumPtsRow        ; // (14 bits - max is 16,383) Number of shots per rotation of motor.
   unsigned short NumPtsPkt        ; // (14 bits - max is 16,383) Number of shots in this packet <= NumPtsRow.

   unsigned short ScannerType    :4; // {MBARI, PIPE, SL4, SL5, SL6} - also sets what is in header.
   unsigned short RBinEnd        :8; // Last range bin in line (255 max)
   unsigned short Spare1         :2; // Ballence
   unsigned short Points_per_LOS :2; // 0=good data only, 1=1 (always 1), 2=2 (always 2 entries)

   // In header - will not change for each line.
   //unsigned char  TempAzCnt        ; // number of shots to average in Az   for a Temperature Bin. 1-255. Default 10.
   //unsigned char  TempRowCnt       ; // number of shots to average in Rows for a Temperature Bin. 1-255. Default 10.

   unsigned int AzBinStart   : 13; // limit to 8191           = # columns / TempAzCnt 
   unsigned int ElBin        : 11; // limit to 2047 (11 bits) = # Rows    / TempElCnt
   unsigned int RBinStart    :  8; // First range bin in line (255 max), a line is TempAzCnt ranges at a time.

   unsigned int R0_offset    : 20; // represented in raw counts,    20 bits
   unsigned int SNR_offset   : 12; // represented in raw counts,    12 bits

   unsigned int Ratio_offset     ; // represented in raw counts,    32 bits
           
   unsigned int R0_nbits     :  6; // number of bits each 0-32
   unsigned int SNR_nbits    :  6; // number of bits each 0-32
   unsigned int Ratio_nbits  :  6; // number of bits each 0-32
   unsigned int AzBinStop    : 13; // limit to 8191           = # columns / TempAzCnt
   unsigned int Spare2       : 1; // ballence
} mbsys_3ddwissl2_StructLineThermalPacket; // row info, one for each row
//unsigned char* PointData[variable];
//U*: R0     [NumPtsPkt] - Red 0 signal             
//U*: Ratio  [NumPtsPkt] - Red 1/ Red 0 signal      
//U*: SNR    [NumPtsPkt] - Red 0/1 SNR measure 
//U4: Class  [NumPtsPkt] - Classification { 0=Unclass, Ignore, LowSNR, Clutter, HiSNR, Good, Glow }

typedef struct {
   unsigned int AzCmdStart_deg_cnt; // in 360./0x3FFFF per count - default resolution
   unsigned int AzCmdEnd_deg_cnt  ; // in 360./0x3FFFF per count - default resolution
   unsigned int RangeMin_m_cnt    ; // in default resolution U20.14, need to change with m_iRScale_Bits for direct compare.
   unsigned int RangeMax_m_cnt    ; // in default resolution U20.14, need to change with m_iRScale_Bits for direct compare.
   unsigned short ThBinStart_cnts   ; // Thermal az bin start (include) deg to cnts is 5000/360.0 = 1/m_fAzBin_deg_per_cnt
   unsigned short ThBinEnd_cnts     ; // Thermal az bin end   (include)
   unsigned int IntensityMin      ;
   unsigned int IntensityMax      ;
   unsigned int SNRMin            ; // Thermal SNR U12.0
   unsigned int SNRMax            ;
   unsigned int R0Min             ; // Thermal Ro  U20
   unsigned int R0Max             ;
   unsigned int RatioMin          ; // Thermal Ratio U32.31
   unsigned int RatioMax          ;
   bool   SendGoodOnly      ; // Only Class good - includes Az window.
   bool   SendAzWindow      ; // All points in the Az range acceptance window, good or bad.
} mbsys_3ddwissl2_SriatProcessingCFG;

typedef struct {
   unsigned int  Az_cnts;       // U18 cal = 360 deg / 0x3FFFF ... but only used for window selection.
   unsigned int  Red0_cnts ;    // U20 Sum red0
   unsigned int  Ratio_cnts;    // U32.31 Red0/Red1
   unsigned short  SNR_cnts  ;    // U12 goodness measure
   unsigned int  Time_uS;       // uSeconds from start of scan (Center Time ~200 ms each if 10x10 at 50 Hz)
   unsigned char   ClassT    ; // Thermal class
   unsigned short  AzBin;
   unsigned short  ElBin;
   unsigned char   RBin;
} mbsys_3ddwissl2_TempDataPoint;

typedef struct {
   unsigned int  Az_cnts;        // U18 cal = 360 deg / 0x3FFFF
   unsigned int  Range_cnts;     // U20 at .1 mm per count.
   unsigned short  Intensity_cnts; // U12 raw value
   unsigned int  Time_uS;        // uSeconds from start of scan.
   unsigned char   ClassR;         // Range   class
} mbsys_3ddwissl2_RangeDataPoint; // will have 2x number of ranges for every 1 temperature.







// unpacked range data
struct RiaatArray
{

   unsigned int   m_iSize   ; // current size of populated data.
   unsigned int   m_iMaxSize; // Maximum size of populated data, array size.

   unsigned int*  m_piAz_Deg   ; // Az Angle (spin axis measurement) ... Speed + Index replace?
   int*     m_piRange_m  ; // Range to data point ... from glass?
   unsigned short*  m_piIntensity; // Signal intensity
   unsigned int*  m_pnTime_uS  ; // time from start of scan in uS
   unsigned char*   m_iClass     ; // classification of data point.   

   // Calculated variables and parameters 
   float m_fExpectedDeltaTime_uS; // calculated based on PRF and line count, used to stuff.
   float m_fExpectedDeltaAz_deg ; // calculated based on PRF and line count, used to stuff.
   float m_fScannerShift_deg    ; // = shift count * m_fExpectedDeltaAz_deg
   bool  m_bSimpleAngle         ; // range is from Start to End if true (does not pass through zero).
   // running tally
   int   m_iPtsGoodR            ; // the number of good points (by classification).
   // Limits
   unsigned int m_iRmin_cnt ; // 20 bits
   unsigned int m_iRmax_cnt ; // 20 bits
   unsigned short m_iImin_cnt ; // 12 bits
   unsigned short m_iImax_cnt ; // 12 bits
   unsigned int m_iAzmin_cnt; // 18 bits
   unsigned int m_iAzmax_cnt; // 18 bits
};

// unpacked thermal data
struct ThermArray
{

   unsigned int   m_iSize   ; // current size of populated data.
   unsigned int   m_iMaxSize; // Maximum size of populated data, array size.
   //float    m_fLaserPrf; // calculate timestamp from / fill array if missing data.

   unsigned int*  m_piRatio;  // Red 2 / Red 1 U32.31
   unsigned int*  m_piR0   ;  // Red 1
   unsigned short*  m_piSNR  ;  // Red 1 Intensity.
   unsigned int*  m_pnTime_uS; // time from start of scan in uS - probably will not need. Array position implies.
   unsigned char*   m_iClass ;  // classification of data point.

   //float* GetThermalProfile(unsigned short az_bin, unsigned short el_bin);
   //float* GetRangeProfile_m(unsigned short az_bin, unsigned short el_bin);
   
   //// Set the AZ bin range and El bin value when adding points
   unsigned short   m_iAzBinLast; // pt zero baised Az Bin
   unsigned short   m_iElBinLast; // pt zero baised EL bin
   unsigned short   m_iRBinLast ; // pt zero baised R bin
   
   // Used to determine if the Az bin is in the Az capture window.
   float    m_AzBinAngle_deg;// calculate what a bin size is in deg (PreHeader fills out).

   // Limits
   unsigned short m_iSNRmin_cnt  ; // 12 bits
   unsigned short m_iSNRmax_cnt  ; // 12 bits
   unsigned int m_iR0min_cnt   ; // 20 bits
   unsigned int m_iR0max_cnt   ; // 20 bits
   unsigned int m_iRatioMin_cnt; // 32 bits
   unsigned int m_iRatioMax_cnt; // 32 bits
   unsigned short  m_iAzBinMin; // 13 bits The min AZ    bin for thermal 
   unsigned short  m_iElBinMin; // 11 bits The min EL    bin for thermal
   unsigned char   m_iRBinMin ; //  8 bits The min Range bin for thermal
   unsigned short  m_iAzBinMax; // 13 bits The max AZ    bin for thermal
   unsigned short  m_iElBinMax; // 11 bits The max EL    bin for thermal
   unsigned char   m_iRBinMax ; //  8 bits The max Range bin for thermal

   // Parameters
   int      m_iPtsGoodT;
   bool     m_bSimpleAngle; // range is from Start to End if true (does not pass through zero).
   unsigned char    m_TempAzCnt ; // number of shots to average in Az   for a Temperature Bin. 1-255. Default 10.
   unsigned char    m_TempRowCnt; // number of shots to average in Rows for a Temperature Bin. 1-255. Default 10. 
};

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

/* functions called by mbpreprocess to fix first generation WiSSL timestamp errors */
int mbsys_3ddwissl2_indextablefix(int verbose,
  void *mbio_ptr,
  int num_indextable,
  void *indextable_ptr,
  int *error);
int mbsys_3ddwissl2_indextableapply(int verbose,
  void *mbio_ptr,
  int num_indextable,
  void *indextable_ptr,
  int n_file,
  int *error);

#endif  /* MBSYS_3DDWISSL2_H_ */
