/********************************************************************
 *
 * Module Name : GSF.H
 *
 * Author/Date : J. S. Byrne / Jan 25,  1994
 *
 * Description : This is the header file for the GSF ToolKit
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the type short is 16 bits, and that the type
 *    int is 32 bits.
 *
 * Change Descriptions :
 * who when      what
 * --- ----      ----
 * jsb 10/17/94  Added support for Reson Seabat data
 * jsb 01/29/95  Added support for em121a nominal depth array
 * jsb 08/14/95  Scale factors and pointers to dynamically allocated memory
 *               now maintained by the library, not the caller.  Consolidated
 *               APIs for sequential and direct access, now both use gsfRead
 *               and gsfWrite. This is version GSF-v01.01.
 * jsb 11/01/95  Completed modifications to indexing to support increase in
 *               gsf file size after initial index file creation.  The size
 *               of the file is now stored in the index file header. Index
 *               files without the expected header are recreated on the first
 *               open. This is still version GSF-v01.01. Also added a unique
 *               sensor specific subrecord for Simrad EM1000.
 * jsb 12/22/95  Added gsfGetMBParams, gsfPutMBParams, gsfIsStarboardPing,
 *               and gsfGetSwathBathyBeamWidths. Also added GSF_APPEND as
 *               a file access mode, and modifed GSF_CREATE access mode so
 *               that files can be updated (read and written). This is gsf
 *               library version GSF-v01.02.
 * hem 08/20/96  Added gsfSingleBeamPing Record structure; added Type III
 *               Seabeam, Echotrac, Bathy200, MGD77, BDB, & NOS HDB subrecord
 *               IDs & subrecords; added gsfStringError.  This is gsf library
 *               version GSF-v1.03.
 * jsb 09/27/96  Added support for SeaBeam with amplitude data.
 * jsb 03/24/97  Added gsfSeaBatIISpecific data structure to replace
 *               the gsfSeaBatSpecific data structure, for the Reson 900x
 *               series sonar systems.  Also added gsfSeaBat8101Specific
 *               data structure for the Reson 8101 series sonar system.
 *               Increased the macro GSF_MAX_RECORD_SIZE from 4k to 32k.
 *               This is GSF library version GSF-v1.04.
 * bac 10/27/97  Added a sensor specific subrecord for the SeaBeam 2112/36.
 * dwc 1/9/98    Added a sensor specific subrecord for the Elac Bottomchart MkII.
 * jsb 9/28/98   Added new navigation error record definition. gsfHVNavigationError.
 *               This record is intended to replace the gsfNavigationError record.
 *               This change addresses CRs: GSF-98-001, and GSF-98-002. Also added
 *               new ping array subrecords: horizontal_error, and vertical_error. This
 *               change address CR: GSF-98-003. These new subrecords are intended to
 *               replace the depth_errror, along_track_error and across_track_error
 *               subrecords. In a future release, new file support for these three
 *               error subrecords will be dropped. This is library version GSF-v1.07.
 * jsb 12/19/98  Added support for em3000. Also increased number of entries available
 *               for processing and sensor parameter records. This is library version
 *               GSF-v1.08.
 * wkm 4/1/99    Added CMP_SASS subrecord for Compressed SASS (BOSDAT) data.  This
 *               subrecord should be used in place of the SASS subrecord (TypeIII).
 *               The original has been left in tactc so as to not break existing code.
 * jsb 07/20/99  Completed work on GSF version 1.08.  Added new functions gsfGetSwathBathyArrayMinMax,
 *               and gsfLoadDepthScaleFactorAutoOffset in support of signed depth.
 *               This release addresses the following CRs: GSF-99-002, GSF-99-006, GSF-99-007,
 *               GSF-99-008, GSF-99-009, GSF-99-010, GSF-99-011, GSF-99-012,
 * wkm 7/30/99   Updated SASS specific data subrecord to include 'lntens' and renamed
 *               surface_velocity to 'lfreq'.  These are the original SASS data filed
 *               names and were requested by NAVO to remane in tact.  Added commet block
 *               to document mapping of SASS data fields to GSF.
 * bac 12-22-99  Applications built with GCC that use GSF.DLL require any global data to be explicitly
 *               defined as imported from a DLL.  When this is the case, gsfError gets redefined
 *               as *__imp_gsfError.  gsfError is unchanged for other compilers or static linking with GCC.
 *               Programs built with the GCC and using gsf.dll must define gsf_USE_DLL
 *               (-Dgsf_USE_DLL) when building. This is version GSF_1.10
 * bac 10-24-00  Updated gsfEM3RunTime structure to include data fields from updated
 *               EM series runtime parameter datagram.  Also added an additional check
 *               for the LINUX define of timespec.
 * bac 07-18-01  Added a sensor specific subrecord for the Reson 8100 series of sonars.  Also
 *               made modifications for use with C++ code.  The typedef for each sensor specific
 *               structure has been modfied to have a different name than the element of the SensorSpecific
 *               union.  Also removed the useage of C++ reserved words "class" and "operator".  These
 *               modifications will potentially require some changes to application code.
 * bac 10-12-01  Added a new attitude record definition.  The attitude record provides a method for
 *               logging full time-series attitude measurements in the GSF file, instead of attitude
 *               samples only at ping time.  Each attitude record contains arrays of attitude
 *               measurements for time, roll, pitch, heave and heading.  The number of measurements
 *               is user-definable, but because of the way in which measurement times are stored, a
 *               single attitude record should never contain more than sixty seconds worth of
 *               data.
 * bac 11-09-01  Added motion sensor offsets to the gsfMBOffsets structure.  Added support for these
 *               new offsets in the gsfPutMBParams and gsfGetMBParams functions, so these offsets are
 *               encoded in the process_parameters record.
 * jsb 01-16-02  Added support for Simrad EM120.
 * bac 06-19-03  Added support for bathymetric receive beam time series intensity data (i.e., Simrad
 *               "Seabed image" and Reson "snippets").  Inlcluded RWL updates of 12-19-02 for adding
 *               sensor-specific singlebeam information to the MB sensor specific subrecords.
 * bac 12-28-04  Added support for Reson Navisound, EM3000D, EM3002, and EM3002D.  Renumbered
 *               singlebeam subrecord IDs to be less than 256, as previous version did not save
 *               these sensors IDs correctly.  Added beam spacing to Reson 8100 sensor-specific
 *               subrecord.  Added definitions for RTG position types in gsfHVNavigationError record.
 * bac 06-28-06  Added sensor ID for EM121A data received via Kongsberg SIS, mapped to existing
 *               EM3 series sensor specific data structure.  Added __APPLE__ macro definition to
 *               support compile switch steering around definition of struct timespec on MAC OSX.
 *               Changed structure elements of type long to int, for compilation on 64-bit architectures.
 * dhg 09-27-06  Added sensor ID for gsfGeoSwathPlusSpecific data received via GeoSwath interferometric
 *               250 Khz sonar.
 * dhg 10-04-06  Added more sensor specific fields for the GeoSwathPlus interferometric sonar.
 * jsb 07-24-07  GSFv2.07 includes some additional parameters in the imagery sensor specific structure
 *                to support calculating back to the exact original intensity values received from the
 *                sonar.
 * bsl 11-05-07  Added sensor ID and structure for gsfGeoSwathPlusSpecific data received via Klein 5410
 *                bathymetric sidescan sonar.  Also added an imagery sensor specific structure.
 * jsb 11-06-07  Updates to utilize the subrecord size in termining the field size for the array subrecords
 *                that support more than one field size.  Also replaced use of strstr with strcmp in gsfGetMBParams
 *                to resolve potential problem where one keyword name may be fully contained in another.
 * DHG 2008/12/18 Add "VESSEL_TYPE" to Processing Parameters for AUV vs Surface Ship discrimination.
 * mab 02-01-09  Updates to support Reson 7125. Added new subrecord IDs and subrecord definitions for Kongsberg
 *                sonar systems where TWTT and angle are populated from raw range and beam angle datagram. Added
 *                new subrecord definition for EM2000.  Bug fixes in gsfOpen and gsfPercent.
 * mab 06-11-09  Moved GSF_MAX_RECORD_SIZE from 256KB to 512KB to accomodate Reson7125 imagery.
 * jsb 01-14-10  Added new function prototypes to return various status information about the opened GSF file.
 * clb 04-21-11  Changed version from 03.02 to 03.03
 * clb 05-11-11  Changed the value of GSF_NULL_DEPTH_CORRECTOR (STR 19142)
 * clb 05-11-11  Added depth sensor and receiver array offsets to gsfMBOffsets structure
 * clb 05-27-11  Added __MINGW64__ references
 * clb 06-21-11  Added t_gsfEM12Specific structure
 * clb 09-20-11  Added R2Sonic support
 * clb 10-04-11  Added GSF_PARTIAL_RECORD_AT_END_OF_FILE error code
 * clb 11-09-11  Added gsfInitializeMBParams() prototype
 * clb 11-10-11  Changed value of GSF_NULL_DEPTH_CORRECTOR back to 99.99 for backwards compatibility
 * clb 09-13-11  Added check for HAVE_STRUCT_TIMESPEC when defining timespec; updated version to 03.05
 * jhp 02-10-14  Added GSF_SWATH_BATHY_SUBRECORD_SONAR_VERT_UNCERT_ARRAY
 * jhp 03-31-14  Added support for R2Sonic 2020.
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 * copyright 2014 Leidos, Inc.
 * There is no charge to use the library, and it may be accessed at: https://www.leidos.com/maritime/gsf
 * This library may be redistributed and/or modified under the terms of the GNU Lesser General Public License
 * version 2.1, as published by the Free Software Foundation.  A copy of the LGPL 2.1 license is included
 * with the GSF distribution and is avaialbe at: http://opensource.org/licenses/LGPL-2.1.
 *
 * Leidos, Inc. configuration manages GSF, and provides GSF releases. Users are strongly encouraged to
 * communicate change requests and change proposals to Leidos, Inc.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 ********************************************************************/

#ifndef  __GSF_H__
#define __GSF_H__

/* Get the required standard C library includes */
#include <stdio.h>
#include <time.h>

/* Get the required system include files */
#include <float.h>
#ifdef __OS2__
#include <types.h>
#include <utils.h>
/* specify OS/2 Optlink linkage in the function prototype */
#ifndef OPTLK
#define OPTLK _Optlink
#endif
#else
#include <sys/types.h>
#define OPTLK
#endif

#ifdef WIN32
#	define R_OK 04
#	define W_OK 02
#	define X_OK 01
#	define F_OK 00
#	ifndef __WINDOWS__              /* I, Joaquim Luis, understand nothing of this mess on the ways to detect if we are on Windows */
#		define __WINDOWS__
#	endif
#	ifndef _WIN32
#		define _WIN32
#	endif
#endif

#ifdef __cplusplus
extern          "C"
{
#endif

/* Define this version of the GSF library (MAXIMUM 11 characters) */
#define GSF_VERSION       "GSF-v03.06"

/* Define largest ever expected record size */
#define GSF_MAX_RECORD_SIZE    524288

/* Define the maximum number of files which may be open at once */
#define GSF_MAX_OPEN_FILES     4

/* Define the GSF data file access flags */
#define GSF_CREATE             1
#define GSF_READONLY           2
#define GSF_UPDATE             3
#define GSF_READONLY_INDEX     4
#define GSF_UPDATE_INDEX       5
#define GSF_APPEND             6

/* Define options for sequential access gsf file pointer manipulation */
#define GSF_REWIND             1
#define GSF_END_OF_FILE        2
#define GSF_PREVIOUS_RECORD    3

/* Typedefs for GSF short and long integers */
typedef unsigned short gsfuShort;      /* an unsigned 16 bit integer */
typedef unsigned int   gsfuLong;       /* an unsigned 32 bit integer */
typedef short          gsfsShort;      /* a signed 16 bit integer */
typedef int            gsfsLong;       /* a signed 32 bit integer */

#define GSF_SHORT_SIZE 2
#define GSF_LONG_SIZE  4

/* redefine gsfError for MINGW applications using gsf.dll, harmless for other compilers */
#if defined (__MINGW32__) || defined (__MINGW64__)
  #if __GNUC__ < 3
     #ifdef gsf_USE_DLL
      #define gsfError *__imp_gsfError
     #endif
  #endif
#endif

/* Define the gsf Data Identifier structure */
typedef struct t_gsfDataID
{
    int             checksumFlag;       /* boolean */
    int             reserved;           /* up to 9 bits */
    unsigned int    recordID;           /* bits 00-11 => data type number */
                                        /* bits 12-22 => registry number */
    int             record_number;      /* specifies the nth occurance of */
                                        /* record type specified by recordID */
                                        /* relavent only for direct access */
                                        /* the record_number counts from 1 */
}
gsfDataID;

/* Specify a key to allow reading the next record, no matter what it is */
#define GSF_NEXT_RECORD 0

/* Specify the GSF record data type numbers, for registry number zero */
#define GSF_RECORD_HEADER                                   1u
#define GSF_RECORD_SWATH_BATHYMETRY_PING                    2u
#define GSF_RECORD_SOUND_VELOCITY_PROFILE                   3u
#define GSF_RECORD_PROCESSING_PARAMETERS                    4u
#define GSF_RECORD_SENSOR_PARAMETERS                        5u
#define GSF_RECORD_COMMENT                                  6u
#define GSF_RECORD_HISTORY                                  7u
#define GSF_RECORD_NAVIGATION_ERROR                         8u /* 10/19/98 This record is obsolete */
#define GSF_RECORD_SWATH_BATHY_SUMMARY                      9u
#define GSF_RECORD_SINGLE_BEAM_PING                         10u
#define GSF_RECORD_HV_NAVIGATION_ERROR                      11u /* This record replaces GSF_RECORD_NAVIGATION_ERROR */
#define GSF_RECORD_ATTITUDE                                 12u

/* Number of currently defined record data types (including 0 which is used
 *  in the indexing for ping records which contain scale factor subrecords).
 */
#define             NUM_REC_TYPES  13

/* Put a ceiling on the maximum number of swath bathymetry ping array
 * subrecords allowed in a gsf file.  This define dimensions the scale
 * factors structure.
 */
#define GSF_MAX_PING_ARRAY_SUBRECORDS 27

/* Specify the GSF swath bathymetry ping subrecord identifiers. The beam
 *  data definitions specify the index into the scale factor table, and
 *  define the subrecord id put down on the disk with the subrecord.
 */
#define GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY                  1u
#define GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY           2u
#define GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY            3u
#define GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY            4u
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY             5u
#define GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY     6u
#define GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY     7u
#define GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY             8u
#define GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY         9u
#define GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY          10u
#define GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY            11u /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY     12u /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY      13u /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY          14u
#define GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY          15u
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY             16u
#define GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY        17u
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY     18u
#define GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY         19u /* This record replaces DEPTH_ERROR_ARRAY */
#define GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY       20u /* This record replaces ACROSS_TRACK_ERROR_ARRAY and ALONG_TRACK_ERROR_ARRAY */
#define GSF_SWATH_BATHY_SUBRECORD_INTENSITY_SERIES_ARRAY       21u
#define GSF_SWATH_BATHY_SUBRECORD_SECTOR_NUMBER_ARRAY          22u
#define GSF_SWATH_BATHY_SUBRECORD_DETECTION_INFO_ARRAY         23u
#define GSF_SWATH_BATHY_SUBRECORD_INCIDENT_BEAM_ADJ_ARRAY      24u
#define GSF_SWATH_BATHY_SUBRECORD_SYSTEM_CLEANING_ARRAY        25u
#define GSF_SWATH_BATHY_SUBRECORD_DOPPLER_CORRECTION_ARRAY     26u
#define GSF_SWATH_BATHY_SUBRECORD_SONAR_VERT_UNCERT_ARRAY      27u

/* Define the additional swath bathymetry subrecords, to which the scale
 * factors do not apply.
 */
#define GSF_SWATH_BATHY_SUBRECORD_UNKNOWN                     0u
#define GSF_SWATH_BATHY_SUBRECORD_SCALE_FACTORS             100u
#define GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC          102u
#define GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC             103u
#define GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC            104u
#define GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC            105u
#define GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC           106u
#define GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC            107u
#define GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC             108u /* 03-30-99 wkm/dbj Typeiii SASS ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC           109u
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC           110u
#define GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC           111u
#define GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC  112u  /* 03-30-99 wkm/dbj Typeiii Seabeam ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC           113u
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC        114u
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC      115u
#define GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC     116u
#define GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC        117u
#define GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC           118u
#define GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC           119u
#define GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC            120u
#define GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC         121u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC       122u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC       123u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC       124u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC       125u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC       126u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC       127u
#define GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC            128u
#define GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC           129u
#define GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC          130u
#define GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC          131u
#define GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC       132u
#define GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC            133u
#define GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC            134u
#define GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC            135u
#define GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC    136u /* 2006-09-27: dhg: GeoAcoustics GeoSwath+ */
#define GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC   137u
#define GSF_SWATH_BATHY_SUBRECORD_RESON_7125_SPECIFIC       138u /* 2008-07-17: mab: Reson7p series sonars */
#define GSF_SWATH_BATHY_SUBRECORD_EM2000_SPECIFIC           139u
#define GSF_SWATH_BATHY_SUBRECORD_EM300_RAW_SPECIFIC        140u /* 2009-02-10: mab: EM3 sonars with raw range and beam angles */
#define GSF_SWATH_BATHY_SUBRECORD_EM1002_RAW_SPECIFIC       141u
#define GSF_SWATH_BATHY_SUBRECORD_EM2000_RAW_SPECIFIC       142u
#define GSF_SWATH_BATHY_SUBRECORD_EM3000_RAW_SPECIFIC       143u
#define GSF_SWATH_BATHY_SUBRECORD_EM120_RAW_SPECIFIC        144u
#define GSF_SWATH_BATHY_SUBRECORD_EM3002_RAW_SPECIFIC       145u
#define GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC      146u
#define GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC      147u
#define GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_RAW_SPECIFIC   148u
#define GSF_SWATH_BATHY_SUBRECORD_EM2040_SPECIFIC           149u
#define GSF_SWATH_BATHY_SUBRECORD_DELTA_T_SPECIFIC          150u
#define GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2022_SPECIFIC     151u
#define GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2024_SPECIFIC     152u
#define GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2020_SPECIFIC     153u

#define GSF_SINGLE_BEAM_SUBRECORD_UNKNOWN                     0u
#define GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC         201u
#define GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC        202u
#define GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC            203u
#define GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC              204u
#define GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC           205u
#define GSF_SWATH_BATHY_SB_SUBRECORD_ECHOTRAC_SPECIFIC      206u
#define GSF_SWATH_BATHY_SB_SUBRECORD_BATHY2000_SPECIFIC     207u
#define GSF_SWATH_BATHY_SB_SUBRECORD_MGD77_SPECIFIC         208u
#define GSF_SWATH_BATHY_SB_SUBRECORD_BDB_SPECIFIC           209u
#define GSF_SWATH_BATHY_SB_SUBRECORD_NOSHDB_SPECIFIC        210u
#define GSF_SWATH_BATHY_SB_SUBRECORD_PDD_SPECIFIC           211u
#define GSF_SWATH_BATHY_SB_SUBRECORD_NAVISOUND_SPECIFIC     212u

/* Define null values to be used for missing data */
#define GSF_NULL_LATITUDE               91.0
#define GSF_NULL_LONGITUDE             181.0
#define GSF_NULL_HEADING               361.0
#define GSF_NULL_COURSE                361.0
#define GSF_NULL_SPEED                  99.0
#define GSF_NULL_PITCH                  99.0
#define GSF_NULL_ROLL                   99.0
#define GSF_NULL_HEAVE                  99.0
#define GSF_NULL_DRAFT                   0.0
#define GSF_NULL_DEPTH_CORRECTOR        99.99
#define GSF_NULL_TIDE_CORRECTOR         99.99
#define GSF_NULL_SOUND_SPEED_CORRECTION 99.99
#define GSF_NULL_HORIZONTAL_ERROR       -1.00
#define GSF_NULL_VERTICAL_ERROR         -1.00
#define GSF_NULL_HEIGHT               9999.99
#define GSF_NULL_SEP                  9999.99
#define GSF_NULL_SEP_UNCERTAINTY         0.0

/* Define null values for the swath bathymetry ping array types. Note that
 * these zero values do not necessarily indicate a non-valid value.  The
 * beam flags array should be used to determine data validity.
 */
#define GSF_NULL_DEPTH                 0.0
#define GSF_NULL_ACROSS_TRACK          0.0
#define GSF_NULL_ALONG_TRACK           0.0
#define GSF_NULL_TRAVEL_TIME           0.0
#define GSF_NULL_BEAM_ANGLE            0.0
#define GSF_NULL_MC_AMPLITUDE          0.0
#define GSF_NULL_MR_AMPLITUDE          0.0
#define GSF_NULL_ECHO_WIDTH            0.0
#define GSF_NULL_QUALITY_FACTOR        0.0
#define GSF_NULL_RECEIVE_HEAVE         0.0
#define GSF_NULL_DEPTH_ERROR           0.0
#define GSF_NULL_ACROSS_TRACK_ERROR    0.0
#define GSF_NULL_ALONG_TRACK_ERROR     0.0
#define GSF_NULL_NAV_POS_ERROR         0.0

/* Define macros used to indicate that a certain parameter is not known */
#define GSF_BEAM_WIDTH_UNKNOWN        -1.0

/* define Posix.4 proposed structure for internal storage of time */
/* timespec is defined in __MINGW64__ but it appears that they don't set a macro for it.  JCD  */
#if (!defined (_STRUCT_TIMESPEC_) && !defined (_TIMESPEC_T) && !defined (_STRUCT_TIMESPEC) && !defined (_SYS_TIMESPEC_H) && !defined (__timespec_defined) && !defined (__MINGW64__) && !defined (HAVE_STRUCT_TIMESPEC) && !defined _TIMESPEC_DEFINED)
#define HAVE_STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC_
#define _TIMESPEC_T
#define _STRUCT_TIMESPEC
#define _SYS_TIMESPEC_H
#define __timespec_defined
#define _TIMESPEC_DEFINED
/* MAC OSX is a different bird, and while doesn't have the structures defined */
/* above, does have the timespec structure defined. __APPLE__ will be set when  */
/* compiled with Apple's gcc on OSX and other third party compilers, so we use it to */
/* insure the definition below does not conflict. */
 #ifndef __APPLE__

    struct timespec
    {
        time_t          tv_sec;
        long            tv_nsec;
    };
 #endif
#endif

/* Define a structure for the gsf header record */
#define GSF_VERSION_SIZE 12
typedef struct t_gsfHeader
{
    char            version[GSF_VERSION_SIZE];
}
gsfHeader;

/* Define the data structure for the swath bathymety summary record */
typedef struct t_gsfSwathBathySummary
{
    struct timespec start_time;
    struct timespec end_time;
    double          min_latitude;
    double          min_longitude;
    double          max_latitude;
    double          max_longitude;
    double          min_depth;
    double          max_depth;
}
gsfSwathBathySummary;


/* Define the typeIII specific data structure */
typedef struct t_gsfTypeIIISpecific               /* 03-30-99 wkm/dbj: Obsolete replaced with t_gsfTypeCmpSassSpecific */
{
    unsigned short  leftmost_beam;  /* 0 - leftmost possible beam */
    unsigned short  rightmost_beam;
    unsigned short  total_beams;
    unsigned short  nav_mode;
    unsigned short  ping_number;
    unsigned short  mission_number;
}
t_gsfTypeIIISpecific;


/* Define the CMP (Compressed) SASS specific data structure (from sass.h) */
typedef struct t_gsfCmpSassSpecific
{
    /************************************************************************************
     *
     *   Mapping from Compressed SASS (BOSDAT) to GSF record
     *
     *    from          to                   comment
     *    ===============================================================================
     *
     *    lntens        ping.heave           mapped only when year is post 1991 or
     *                                       user has elected to force mapping.
     *    lfreq         not-mapped
     *    ldraft        comment              APPLIED_DRAFT comment record
     *    svp.svel      svp.sound_velocity   at <= 1000 ... FATHOMS
     *                                       at <= 2500 ... METERS
     *                                       otherwise  ... FEET
     *    svp.deptl     svp.depth            (see sound_velocity)
     *    lmishn        comment              MISSION_NUMBER comment record
     *    luyr          ping_time            GSF time record from 1960 to 1970 base
     *    pitchl        ping.pitch
     *    rolll         ping.roll
     *    lbear         ping.heading         SASS specific (not Seabeam)
     *    pinhd         ping.heading         Seabeam specific (not SASS)
     *    depth         ping.nominal_depth   FATHOMS_TO_METERS_NOMINAL
     *    pslatl        ping.across_track    YARDS_TO_METERS_EXACT
     *    bltime        ping.travel_time
     *    ampl          ping.mr_amplitude
     *    <ftaf file>   ping.beam_flags      HMPS_FLAGS
     *    alpos         ping.along_track     SASS specific YARDS_TO_METERS_EXACT
     *
     ************************************************************************************/

    double lfreq;  /* sea-surface sound velocity in feet/sec from bosdat(lfreq) */
    double lntens; /* post 1992 heave, before 1992 unclear what field contained. */
}
t_gsfCmpSassSpecific;

/* Define the 16 Beam SeaBeam specific data structure */
typedef struct t_gsfSeabeamSpecific
{
    unsigned short  EclipseTime; /* In 10ths of seconds */
}
t_gsfSeaBeamSpecific;

typedef struct t_gsfSBAmpSpecific
{
    unsigned char   hour;
    unsigned char   minute;
    unsigned char   second;
    unsigned char   hundredths;
    unsigned int    block_number;
    short           avg_gate_depth;
}
t_gsfSBAmpSpecific;

/* Define the Seamap specific data structure */
typedef struct t_gsfSeamapSpecific
{
    double          portTransmitter[2];
    double          stbdTransmitter[2];
    double          portGain;
    double          stbdGain;
    double          portPulseLength;
    double          stbdPulseLength;
    double          pressureDepth;
    double          altitude;
    double          temperature;
}
t_gsfSeamapSpecific;

/* Define the EM950/EM1000 specific data structure */
typedef struct t_gsfEM950Specific
{
    int             ping_number;
    int             mode;
    int             ping_quality;
    double          ship_pitch;
    double          transducer_pitch;
    double          surface_velocity;
}
t_gsfEM950Specific;

/* Define the EM100 specific data structure */
typedef struct t_gsfEM100Specific
{
    double          ship_pitch;
    double          transducer_pitch;
    int             mode;
    int             power;
    int             attenuation;
    int             tvg;
    int             pulse_length;
    int             counter;
}
t_gsfEM100Specific;

/* Define the EM121A specific data structure */
typedef struct t_gsfEM121ASpecific
{
    int             ping_number;
    int             mode;
    int             valid_beams;
    int             pulse_length;
    int             beam_width;
    int             tx_power;
    int             tx_status;
    int             rx_status;
    double          surface_velocity;
}
t_gsfEM121ASpecific;

/* Define the Reson SeaBat specific data structure */
typedef struct t_gsfSeaBatSpecific
{
    int             ping_number;
    double          surface_velocity;
    int             mode;
    int             sonar_range;
    int             transmit_power;
    int             receive_gain;
}
t_gsfSeaBatSpecific;

/* The gsfSeaBatIISpecific data structure is intended to replace the
 * gsfSeaBatSpecific data structure as of GSF_1.04.
 */
typedef struct t_gsfSeaBatIISpecific
{
    int             ping_number;            /* 1 - 32767 */
    double          surface_velocity;       /* meters/second */
    int             mode;                   /* bit mapped, see macros below */
    int             sonar_range;            /* meters */
    int             transmit_power;
    int             receive_gain;
    double          fore_aft_bw;            /* fore/aft beam width in degrees */
    double          athwart_bw;             /* athwartships beam width in degrees */
    char            spare[4];               /* Four bytes of spare space, for future use */
}
t_gsfSeaBatIISpecific;

/* Macro definitions for the SeaBatSpecific and SeaBatIISpecific mode field */
#define GSF_SEABAT_WIDE_MODE         0x01   /* if set 10 deg fore-aft */
#define GSF_SEABAT_9002              0x02   /* if set two sonar heads */
#define GSF_SEABAT_STBD_HEAD         0x04   /* if set starboard ping (seabat head 2) */
#define GSF_SEABAT_9003              0x08   /* if set 9003 series sonar (40 beams) */

/* Define the Reson SeaBat specific data structure */
typedef struct t_gsfSeaBat8101Specific
{
    int             ping_number;            /* 1 - 65535 */
    double          surface_velocity;       /* meters/second */
    int             mode;                   /* bit mapped, see macros below */
    int             range;                  /* meters */
    int             power;                  /* 0-8 + status bits */
    int             gain;                   /* 1-45 + status bits */
    int             pulse_width;            /* in microseconds */
    int             tvg_spreading;          /* tvg spreading coefficient * 4 */
    int             tvg_absorption;         /* tvg absorption coefficient */
    double          fore_aft_bw;            /* fore/aft beam width in degrees */
    double          athwart_bw;             /* athwartships beam width in degrees */
    double          range_filt_min;         /* range filter, minimum value, meters (future use) */
    double          range_filt_max;         /* range filter, maximum value, meters (future use) */
    double          depth_filt_min;         /* depth filter, minimum value, meters (future use) */
    double          depth_filt_max;         /* depth filter, maximum value, meters (future use) */
    int             projector;              /* projector type (future use) */
    char            spare[4];               /* Four bytes of spare space, for future use */
}
t_gsfSeaBat8101Specific;

/* Macro definitions for the SeaBat8101Specific and SeaBat8101Specific mode field */
#define GSF_8101_WIDE_MODE         0x01   /* set if transmit on receiver */
#define GSF_8101_TWO_HEADS         0x02   /* set if two sonar heads */
#define GSF_8101_STBD_HEAD         0x04   /* set if starboard ping (seabat head 2) */
#define GSF_8101_AMPLITUDE         0x08   /* set if beam amplitude is available (RITHETA packet) */

/* Define the Reson SeaBat specific data structure */
typedef struct t_gsfReson7100Specific
{
    unsigned int       protocol_version;                       /* Obtained from the Data Record Frame (DRF) */
    unsigned int       device_id;                              /* i.e. 7101, 7111, 7125, etc. Obtained from the DRF */
    unsigned char      reserved_1[16];                         /* Placeholder for growth of fields from DRF */
    unsigned int       major_serial_number;                    /* high order 4 bytes of sonar serial number, from record 7000 */
    unsigned int       minor_serial_number;                    /* low order 4 bytes of sonar serial number, from record 7000 */
    unsigned int       ping_number;                            /* sequential number, unique for each ping, wraps at boundary */
    unsigned int       multi_ping_seq;                         /* 0 if not in multi-ping mode, otherwise number of pings in a multi-ping sequence */
    double             frequency;                              /* Sonar operating frequency in Hz. From record 7000 */
    double             sample_rate;                            /* Sonar system sampling rate in Hz. From record 7000 */
    double             receiver_bandwdth;                      /* Sonar system signal bandwidth in Hz. From record 7000 */
    double             tx_pulse_width;                         /* Transmit pulse length in seconds. From record 7000 */
    unsigned int       tx_pulse_type_id;                       /* 0=CW, 1=Linear chirp, from record 7000 */
    unsigned int       tx_pulse_envlp_id;                      /* 0=Tapered rectangular, 1=Tukey, from record 7000 */
    unsigned int       tx_pulse_envlp_param;                   /* four byte field containing envelope parameter, no definition or units available, from record 7000 */
    unsigned int       tx_pulse_reserved;                      /* four byte field reserved for future growth, from record 7000 */
    double             max_ping_rate;                          /* Maximum ping rate in pings per second, from record 7000 */
    double             ping_period;                            /* seconds since last ping, from record 7000 */
    double             range;                                  /* Sonar range selection in meters, from record 7000 */
    double             power;                                  /* Power selection in dB re 1 microPa, from record 7000 */
    double             gain;                                   /* Gain selection in dB, from record 7000 */
    unsigned int       control_flags;                          /*   0-3: Auto range method
                                                                    4-7: Auto bottom detect filter method
                                                                      8: Bottom detect range filter
                                                                      9: Bottom detect depth filter
                                                                  10-14: Auto receiver gain method
                                                                  15-31: Reserved */
    unsigned int       projector_id;                           /* projector selection, from record 7000 */
    double             projector_steer_angl_vert;              /* degrees, from record 7000 */
    double             projector_steer_angl_horz;              /* degrees, from record 7000 */
    double             projector_beam_wdth_vert;               /* degrees, from record 7000 */
    double             projector_beam_wdth_horz;               /* degrees, from record 7000 */
    double             projector_beam_focal_pt;                /* meters, from record 7000 */
    unsigned int       projector_beam_weighting_window_type;   /* 0-Rectangular, 1-Chebychhev, from record 7000 */
    unsigned int       projector_beam_weighting_window_param;  /* four byte projector weighting parameter, no definition or units available, from record 7000 */
    unsigned int       transmit_flags;                         /* 0-3: Pitch stabilization method
                                                                  4-6: Yaw stabilization method
                                                                  8-31: Reserved */
    unsigned int       hydrophone_id;                          /* hydrophone selection, from record 7000 */
    unsigned int       receiving_beam_weighting_window_type;   /* 0-Chebychev, 1-Kaiser, from record 7000 */
    unsigned int       receiving_beam_weighting_window_param;  /* four byte receiver weighting parameter, no definition or units available, from record 7000 */
    unsigned int       receive_flags;                          /*  0-3: Roll stabilization method
                                                                   4-7: Dynamic focusing method
                                                                   8-11: Doppler compensation method
                                                                  12-15: Match filtering method
                                                                  16-19: TVG method
                                                                  20-23: Multi-Ping Mode
                                                                  24-31: Reserved */
    double             receive_beam_width;                     /* angle in degrees, from record 7000 */
    double             range_filt_min;                         /* range filter, minimum value, meters, from record 7000 */
    double             range_filt_max;                         /* range filter, maximum value, meters, from record 7000 */
    double             depth_filt_min;                         /* depth filter, minimum value, meters, from record 7000 */
    double             depth_filt_max;                         /* depth filter, maximum value, meters, from record 7000 */
    double             absorption;                             /* absorption in dB/km, from record 7000 */
    double             sound_velocity;                         /* sound speed in m/s at transducer, from record 7006 */
    double             spreading;                              /* spreading loss in dB from record 7000 */
    char               reserved_2[16];                         /* spare space, for future use */
    unsigned char      sv_source;                              /* (0: measured, 1: manual), from record 7006 */
    unsigned char      layer_comp_flag;                        /* (0: off, 1: on), from record 7006 */
    char               reserved_3[8];                          /* spare space, for future use */
}
t_gsfReson7100Specific;

#define GSF_7100_PITCH_STAB             0x0001 /* set if pitch stabilized */
#define GSF_7100_ROLL_STAB              0x0001 /* set if roll stabilized */

/* Define the SeaBeam 2112/36 specific data structure */
typedef struct t_gsfSeaBeam2112Specific
{
    int             mode;                       /* bit mapped, see macros below */
    double          surface_velocity;           /* meters/second */
    char            ssv_source;                 /* (V)elocimiter, (M)anual, (T)emperature,
                                                   (E)xternal, or (U)nknown */
    int             ping_gain;                  /* dB */
    int             pulse_width;                /* in milliseconds */
    int             transmitter_attenuation;    /* dB */
    int             number_algorithms;          /* algorithms per beam (1-4) */
    char            algorithm_order[5];         /* null terminated string, each char will be either
                                                   a space, W(MT), or B(DI). if number_algorithms
                                                   equals one, this will be four spaces */
    char            spare[2];                   /* Two bytes of spare space, for future use */
}
t_gsfSeaBeam2112Specific;

/* Macro definitions for the SeaBeam2112Specific mode field */
#define GSF_2112_SVP_CORRECTION   0x01    /* set if true depth, true position corrections are used */
#define GSF_2112_LOW_FREQUENCY    0x02    /* set if using 12kHz frequecy - 36kHz if not set */
#define GSF_2112_AUTO_DEPTH_GATE  0x04    /* set if depth gate mode is automatic - manual if not set */

/* SeaBeam 2112 specific macro definitions for the quality factor array */
#define GSF_2112_POOR_QUALITY     0x01    /* set if the beam was flagged by the SeaBeam as poor quality */
#define GSF_2112_DATA_SOURCE_WMT  0x10    /* set if the data source is WMT - source is BDI if not set */

/* Define the Elac MkII specific data structure */
typedef struct t_gsfElacMkIISpecific
{
    int             mode;                       /* bit mapped, see macros below */
    int             ping_num;
    int             sound_vel;                  /* 0.1 m/s */
    int             pulse_length;               /* 0.01 ms */
    int             receiver_gain_stbd;         /* db */
    int             receiver_gain_port;         /* db */
    int             reserved;
}
t_gsfElacMkIISpecific;

/* Macro definitions for the ElacMkIISpecific mode field */
#define GSF_MKII_LOW_FREQUENCY    0x01    /* set if using 12kHz frequecy - 36kHz if not set */
#define GSF_MKII_SOURCE_MODE      0x02    /* set if RDT transmit used, otherwise omni */
#define GSF_MKII_SOURCE_POWER     0x04    /* set if transmit high power - low power if not set */
#define GSF_MKII_STBD_HEAD        0x08    /* set if starboard ping */

/* Define a data structure to hold the Simrad EM3000 series run time parameters. */
typedef struct t_gsfEM3RunTime
{
    int             model_number;               /* from the run-time parameter datagram */
    struct timespec dg_time;                    /* from the run-time parameter datagram */
    int             ping_number;                /* sequential counter 0 - 65535 */
    int             serial_number;              /* The sonar head serial number */
    int             system_status;              /* normally = 0 */
    int             mode;                       /* 0=nearfield, 1=normal, 2=target, 3=deep, 4=very deep */
    int             filter_id;
    double          min_depth;                  /* meters */
    double          max_depth;                  /* meters */
    double          absorption;                 /* dB/km */
    double          pulse_length;               /* micro seconds */
    double          transmit_beam_width;        /* degrees */
    int             power_reduction;            /* dB */
    double          receive_beam_width;         /* degrees */
    int             receive_bandwidth;          /* Hz */
    int             receive_gain;               /* dB */
    int             cross_over_angle;           /* degrees */
    int             ssv_source;                 /* 0=sensor, 1=manual, 2=profile */
    int             swath_width;                /* total swath width in meters */
    int             beam_spacing;               /* 0=beamwidth, 1=equiangle, 2=equidistant, 3=intermediate */
    int             coverage_sector;            /* total coverage in degrees */
    int             stabilization;
    int             port_swath_width;           /* maximum port swath width in meters */
    int             stbd_swath_width;           /* maximum starboard swath width in meters */
    int             port_coverage_sector;       /* maximum port coverage in degrees */
    int             stbd_coverage_sector;       /* maximum starboard coverage in degrees */
    int             hilo_freq_absorp_ratio;
    int             spare1;                     /* four spare bytes */
}
gsfEM3RunTime;

/* Define the Simrad EM3000 series specific data structure */
typedef struct t_gsfEM3Specific
{
    /* The first nine values are updated with each depth datagram */
    int             model_number;               /* ie: 3000, ... */
    int             ping_number;                /* 0 - 65535 */
    int             serial_number;              /* 100 - 65535 */
    double          surface_velocity;           /* in m/s */
    double          transducer_depth;           /* transmit transducer depth in meters */
    int             valid_beams;                /* number of valid beams for this ping */
    int             sample_rate;                /* in Hz */
    double          depth_difference;           /* in meters between sonar heads in em3000d configuration */
    int             offset_multiplier;          /* transducer depth offset multiplier */
    /* The gsfEM3RunTime data structure is updated with each run-time parameter datagram */
    gsfEM3RunTime   run_time[2];                /* A two element array is needed to support em3000d */
}
t_gsfEM3Specific;

/* Define the Reson 8100 specific data structure */
typedef struct t_gsfReson8100Specific
{
    int             latency;                /* time from ping to output (milliseconds) */
    int             ping_number;            /* 4 byte ping number */
    int             sonar_id;               /* least significant 4 bytes of ethernet address */
    int             sonar_model;            /*  */
    int             frequency;              /* KHz */
    double          surface_velocity;       /* meters/second */
    int             sample_rate;            /* A/D samples per second */
    int             ping_rate;              /* pings per second * 1000 */
    int             mode;                   /* bit mapped, see macros below */
    int             range;                  /* meters */
    int             power;                  /* 0-8 + status bits */
    int             gain;                   /* 1-45 + status bits */
    int             pulse_width;            /* in microseconds */
    int             tvg_spreading;          /* tvg spreading coefficient * 4 */
    int             tvg_absorption;         /* tvg absorption coefficient */
    double          fore_aft_bw;            /* fore/aft beam width in degrees */
    double          athwart_bw;             /* athwartships beam width in degrees */
    int             projector_type;         /* projector type */
    int             projector_angle;        /* projector pitch steering angle (degrees * 100) */
    double          range_filt_min;         /* range filter, minimum value, meters */
    double          range_filt_max;         /* range filter, maximum value, meters */
    double          depth_filt_min;         /* depth filter, minimum value, meters */
    double          depth_filt_max;         /* depth filter, maximum value, meters */
    int             filters_active;         /* bit 0 - range filter, bit 1 - depth filter */
    int             temperature;            /* temperature at sonar head (deg C * 10) */
    double          beam_spacing;           /* across track receive beam angular spacing */
    char            spare[2];               /* Two remaining bytes of spare space, for future use */
}
t_gsfReson8100Specific;

/* Macro definitions for the SeaBat8100Specific mode field */
#define GSF_8100_WIDE_MODE              0x01 /* set if transmit on receiver */
#define GSF_8100_TWO_HEADS              0x02 /* set if two sonar heads */
#define GSF_8100_STBD_HEAD              0x04 /* set if starboard ping (seabat head 2) */
#define GSF_8100_AMPLITUDE              0x08 /* set if beam amplitude is available (RITHETA packet) */
#define GSF_8100_PITCH_STAB             0x10 /* set if pitch stabilized */
#define GSF_8100_ROLL_STAB              0x20 /* set if roll stabilized */

/* Define the Echotrac Single-Beam sensor specific data structure. */

#define GSF_SB_MPP_SOURCE_UNKNOWN       0x00 /* Unknown MPP source */
#define GSF_SB_MPP_SOURCE_GPS_3S        0x01 /* GPS 3S */
#define GSF_SB_MPP_SOURCE_GPS_TASMAN    0x02 /* GPS Tasman */
#define GSF_SB_MPP_SOURCE_DGPS_TRIMBLE  0x03 /* DGPS Trimble */
#define GSF_SB_MPP_SOURCE_DGPS_TASMAN   0x04 /* DGPS Tasman */
#define GSF_SB_MPP_SOURCE_DGPS_MAG      0x05 /* DGPS MagMPPox */
#define GSF_SB_MPP_SOURCE_RANGE_MFIX    0x06 /* Range/Azimauth - Microfix */
#define GSF_SB_MPP_SOURCE_RANGE_TRIS    0x07 /* Range/Azimauth - Trisponder */
#define GSF_SB_MPP_SOURCE_RANGE_OTHER   0x08 /* Range/Azimauth - Other */

typedef struct t_gsfSBEchotracSpecific
{
    int             navigation_error;
    unsigned short  mpp_source;     /* Flag To determine mpp source - See above */
    unsigned short  tide_source;    /* in GSF Version 2.02+ this is in ping flags */
    double          dynamic_draft;  /* speed induced draft im meters */
    char            spare[4];       /* four bytes of reserved space */

}
t_gsfSBEchotracSpecific;

/* Define the MGD77 Single-Beam sensor specific data structure. */
typedef struct t_gsfSBMGD77Specific
{
    unsigned short  time_zone_corr;
    unsigned short  position_type_code;
    unsigned short  correction_code;
    unsigned short  bathy_type_code;
    unsigned short  quality_code;
    double          travel_time;
    char            spare[4];                  /* four bytes of reserved space */
}
t_gsfSBMGD77Specific;

/* Define the BDB sensor specific data structure */
typedef struct t_gsfSBBDBSpecific
{
    int   doc_no;         /* Document number (5 digits) */
    char  eval;           /* Evaluation (1-best, 4-worst) */
    char  classification; /* Classification ((U)nclass, (C)onfidential,
                                             (S)ecret, (P)roprietary/Unclass,
                                             (Q)Proprietary/Class) */
    char  track_adj_flag; /* Track Adjustment Flag (Y,N) */
    char  source_flag;    /* Source Flag ((S)urvey, (R)andom, (O)cean Survey) */
    char  pt_or_track_ln; /* Discrete Point (D) or Track Line (T) Flag */
    char  datum_flag;     /* Datum Flag ((W)GS84, (D)atumless) */
    char  spare[4];       /* four bytes of reserved space */
}
t_gsfSBBDBSpecific;

/* Define the NOS HDB sensor specific data structure */
typedef struct t_gsfSBNOSHDBSpecific
{
    unsigned short  type_code;    /*  Depth type code  */
    unsigned short  carto_code;   /*  Cartographic code  */
    char            spare[4];     /* four bytes of reserved space */
}
t_gsfSBNOSHDBSpecific;

/* Define the Navisound sensor specific data structure */
typedef struct t_gsfSBNavisoundSpecific
{
    double          pulse_length;    /*  pulse length in cm  */
    char            spare[8];     /* eight bytes of reserved space */
}
t_gsfSBNavisoundSpecific;

/* Macro definitions for EM4 series sector data details */
#define  GSF_MAX_EM4_SECTORS     9
#define  GSF_MAX_EM3_SECTORS     20

/* Define sub-structure for the transmit sectors */
#define  GSF_EM_WAVEFORM_CW      0
#define  GSF_EM_WAVEFORM_FM_UP   1
#define  GSF_EM_WAVEFORM_FM_DOWN 2

typedef struct t_gsfEM4TxSector
{
    double          tilt_angle;                     /* transmitter tilt angle in degrees */
    double          focus_range;                    /* focusing range, 0.0 for no focusing */
    double          signal_length;                  /* transmit signal duration in seconds */
    double          transmit_delay;                 /* Sector transmit delay from first transmission in seconds */
    double          center_frequency;               /* center frequency in Hz */
    double          mean_absorption;                /* mean absorption coefficient in 0.01 dB/kilometer */
    int             waveform_id;                    /* signal waveform ID 0=CW; 1=FM upsweep; 2=FM downsweep */
    int             sector_number;                  /* transmit sector number */
    double          signal_bandwidth;               /* signal bandwidth in Hz */
    unsigned char   spare[16];                      /* spare space */
}
t_gsfEM4TxSector;

typedef struct t_gsfEM3RawTxSector
{
    double          tilt_angle;                     /* transmitter tilt angle in degrees */
    double          focus_range;                    /* focusing range, 0.0 for no focusing */
    double          signal_length;                  /* transmit signal duration in seconds */
    double          transmit_delay;                 /* Sector transmit delay from first transmission in seconds */
    double          center_frequency;               /* center frequency in Hz */
    int             waveform_id;                    /* signal waveform ID 0=CW; 1=FM upsweep; 2=FM downsweep */
    int             sector_number;                  /* transmit sector number */
    double          signal_bandwidth;               /* signal bandwidth in Hz */
    unsigned char   spare[16];                      /* spare space */
}
t_gsfEM3RawTxSector;

/* The following macro definitions are to aid in interpretation of the sonar mode field */
#define GSF_EM_MODE_VERY_SHALLOW 0x00               /* Bits 2,1,0 cleared means very shallow mode */
#define GSF_EM_MODE_SHALLOW      0x01               /* Bit zero set means shallow mode */
#define GSF_EM_MODE_MEDIUM       0x02               /* Bit one set means medium mode */
#define GSF_EM_MODE_DEEP         0x03               /* Bits one and zero set means deep mode */
#define GSF_EM_MODE_VERY_DEEP    0x04               /* Bit two set means very deep mode */
#define GSF_EM_MODE_EXTRA_DEEP   0x05               /* Bits two and one set means extra deep mode */
#define GSF_EM_MODE_MASK         0x07               /* Mask off bits 2,1,0 to determine just the mode */
                                                    /* Exact definition of bits 5,4,3 not clear from document rev J. */
#define GSF_EM_MODE_DS_OFF       0xC0               /* bits 7 and 6 cleared means dual swath off */
#define GSF_EM_MODE_DS_FIXED     0x40               /* bit 6 set means dual swath in fixed mode */
#define GSF_EM_MODE_DS_DYNAMIC   0x80               /* bit 7 set means dual swath in dynamic mode */

/* Define a data structure to hold the Simrad EM series run time parameters per datagram document rev I. */
typedef struct t_gsfEMRunTime
{
    int              model_number;                  /* from the run-time parameter datagram */
    struct timespec  dg_time;                       /* from the run-time parameter datagram */
    int              ping_counter;                  /* sequential counter 0 - 65535 */
    int              serial_number;                 /* The primary sonar head serial number */
    unsigned char    operator_station_status;       /* Bit mask of status information for operator station */
    unsigned char    processing_unit_status;        /* Bit mask of status information for sonar processor unit */
    unsigned char    bsp_status;                    /* Bit mask of status information for BSP status */
    unsigned char    head_transceiver_status;       /* Bit mask of status information for sonar head or sonar transceiver */
    unsigned char    mode;                          /* Bit mask of sonar operating information, see mode bit mask definitions */
    unsigned char    filter_id;                     /* one byte tit mask for various sonar processing filter settings */
    double           min_depth;                     /* meters */
    double           max_depth;                     /* meters */
    double           absorption;                    /* dB/km */
    double           tx_pulse_length;               /* in micro seconds */
    double           tx_beam_width;                 /* degrees */
    double           tx_power_re_max;               /* The transmit power referenced to maximum power in dB */
    double           rx_beam_width;                 /* degrees */
    double           rx_bandwidth;                  /* Hz */
    double           rx_fixed_gain;                 /* dB */
    double           tvg_cross_over_angle;          /* degrees */
    unsigned char    ssv_source;                    /* one byte bit mask defining SSSV source -> 0=sensor, 1=manual, 2=profile */
    int              max_port_swath_width;          /* total swath width to port side in meters */
    unsigned char    beam_spacing;                  /* one byte bit mask -> 0=beamwidth, 1=equiangle, 2=equidistant, 3=intermediate */
    int              max_port_coverage;             /* coverage to port side in degrees */
    unsigned char    stabilization;                 /* one byte bit mask defining yaw and pitch stabilization mode */
    int              max_stbd_coverage;             /* coverage to starboard side in degrees */
    int              max_stbd_swath_width;          /* total swath width to starboard side in meters */
    double           durotong_speed;                /* Sound speed in Durotong (SSD). Field only valid for the EM 1002 */
    double           hi_low_absorption_ratio;       /* HiLo frequency absorption coefficient ratio, field valid only for EM3 sonars */
    double           tx_along_tilt;                 /* Transmit fan along track tilt angle in degrees, field valid only for EM4 sonars */
    unsigned char    filter_id_2;                   /* two lowest order bits define the penetration filter setting: off, weak, medium, or strong */
    unsigned char    spare[16];                     /* 16 spare bytes */
}
t_gsfEMRunTime;

/* Macro definitions for bits of pu_status field */
#define GSF_EM_VALID_1_PPS      0x0001              /* If set, then 1 PPS timing is valid */
#define GSF_EM_VALID_POSITION   0x0002              /* If set, then position input is valid */
#define GSF_EM_VALID_ATTITUDE   0x0004              /* If set, then attitude input is valid */
#define GSF_EM_VALID_CLOCK      0x0008              /* If set, then clock status is valid */
#define GSF_EM_VALID_HEADING    0x0010              /* If set, then heading status is valid */
#define GSF_EM_PU_ACTIVE        0x0020              /* If set, then PU is active (i.e. pinging) */

/* Define a data structure to hold the Simrad EM series PU status values per datagram document rev I. */
typedef struct t_gsfEMPUStatus
{
    double           pu_cpu_load;                   /* Percent CPU load in the processor unit */
    unsigned short   sensor_status;                 /* Bit mask containing status of sensor inputs */
    int              achieved_port_coverage;        /* Achieved coverage to port in degrees */
    int              achieved_stbd_coverage;        /* Achieved coverage to starboard in degrees */
    double           yaw_stabilization;             /* in degrees */
    unsigned char    spare[16];
}
t_gsfEMPUStatus;

/* Define sensor specific data structures for the Kongsberg 710/302/122 */
typedef struct t_gsfEM4Specific
{
    /* values from the XYZ datagram and raw range datagram */
    int              model_number;                  /* 122, or 302, or 710, or ... */
    int              ping_counter;                  /* Sequential ping counter, 1 through 65535 */
    int              serial_number;                 /* System unique serial number, 100 - ? */
    double           surface_velocity;              /* Measured sound speed near the surface in m/s */
    double           transducer_depth;              /* The transmit transducer depth in meters re water level at ping time */
    int              valid_detections;              /* number of beams with a valid bottom detection for this ping */
    double           sampling_frequency;            /* The system digitizing rate in Hz */
    unsigned int     doppler_corr_scale;            /* Scale factor value to be applied to Doppler correction field prior to applying corrections */
    double           vehicle_depth;                 /* From 0x66 datagram, non-zero when sonar head is mounted on a sub-sea platform */
    unsigned char    spare_1[16];
    int              transmit_sectors;              /* The number of transmit sectors for this ping */
    t_gsfEM4TxSector sector[GSF_MAX_EM4_SECTORS];   /* Array of structures with transmit sector information */
    unsigned char    spare_2[16];

    /* Values from the run-time parameters datagram */
    t_gsfEMRunTime   run_time;

    /* Values from the PU status datagram */
    t_gsfEMPUStatus  pu_status;
}
t_gsfEM4Specific;

/* Define sensor specific data structures for the Kongsberg 3000, etc which use raw range and beam angle */
typedef struct t_gsfEM3RawSpecific
{
    /* values from the XYZ datagram and raw range datagram */
    int              model_number;                  /* ie 3000 ... */
    int              ping_counter;                  /* Sequential ping counter, 0 through 65535 */
    int              serial_number;                 /* System unique serial number, 100 - ? */
    double           surface_velocity;              /* Measured sound speed near the surface in m/s */
    double           transducer_depth;              /* The transmit transducer depth in meters re water level at ping time */
    int              valid_detections;              /* number of beams with a valid bottom detection for this ping */
    double           sampling_frequency;            /* The system digitizing rate in Hz */
    double           vehicle_depth;                 /* vechicle depth in 0.01 m */
    double           depth_difference;              /* in meters between sonar heads in em3000d configuration */
    int              offset_multiplier;             /* transducer depth offset multiplier */
    unsigned char    spare_1[16];
    int              transmit_sectors;              /* The number of transmit sectors for this ping */
    t_gsfEM3RawTxSector sector[GSF_MAX_EM3_SECTORS];   /* Array of structures with transmit sector information */
    unsigned char    spare_2[16];

    /* Values from the run-time parameters datagram */
    t_gsfEMRunTime   run_time;

    /* Values from the PU status datagram */
    t_gsfEMPUStatus  pu_status;
}
t_gsfEM3RawSpecific;

/*DHG 2006/09/27 Added support for GeoSwath interferometric 250 Khz sonar */
/* Define the GeoSwath sensor specific data structure */
typedef struct t_gsfGeoSwathPlusSpecific
{
    int             data_source;             /* 0 = CBF, 1 = RDF */
    int             side;                    /* 0 = port, 1 = stbd */
    int             model_number;            /* ie: 100, 250, 500, ... */
    double          frequency;               /* Hz */
    int             echosounder_type;        /* ? */
    long            ping_number;             /* 0 - 4,294,967,295 */
    int             num_nav_samples;         /* number of navigation samples in this ping */
    int             num_attitude_samples;    /* number of attitude samples in this ping */
    int             num_heading_samples;     /* number of heading samples in this ping */
    int             num_miniSVS_samples;     /* number of miniSVS samples in this ping */
    int             num_echosounder_samples; /* number of echosounder samples in ping */
    int             num_raa_samples;         /* number of RAA (Range/Angle/Amplitude) samples in ping */
    double          mean_sv;                 /* meters per second */
    double          surface_velocity;        /* in m/s */
    int             valid_beams;             /* number of valid beams for this ping */
    double          sample_rate;             /* Hz */
    double          pulse_length;            /* micro seconds */
    int             ping_length;             /* meters */
    int             transmit_power;          /* ? */
    int             sidescan_gain_channel;   /* RDF documentation = 0 - 3  */
    int             stabilization;           /* 0 or 1 */
    int             gps_quality;             /* ? */
    double          range_uncertainty;       /* meters */
    double          angle_uncertainty;       /* degrees */
    char            spare[32];               /* 32 bytes of reserved space */
}
t_gsfGeoSwathPlusSpecific;

#define PORT_PING 0
#define STBD_PING 1

#define GSF_GEOSWATH_PLUS_PORT_PING PORT_PING
#define GSF_GEOSWATH_PLUS_STBD_PING STBD_PING

/* Define the Klein 5410 Bathy Sidescan sensor specific data structure */
typedef struct t_gsfKlein5410BssSpecific
{
    int             data_source;             /* 0 = SDF */
    int             side;                    /* 0 = port, 1 = stbd */
    int             model_number;            /* ie: 5410 */
    double          acoustic_frequency;      /* system frequency in Hz */
    double          sampling_frequency;      /* sampling frequency in Hz */
    unsigned int    ping_number;             /* 0 - 4,294,967,295 */
    unsigned int    num_samples;             /* total number of samples in this ping */
    unsigned int    num_raa_samples;         /* number of valid range, angle, amplitude samples in ping */
    unsigned int    error_flags;             /* error flags for this ping */
    unsigned int    range;                   /* sonar range setting */
    double          fish_depth;              /* reading from the towfish pressure sensor in Volts */
    double          fish_altitude;           /* towfish altitude in m */
    double          sound_speed;             /* speed of sound at the transducer face in m/sec */
    int             tx_waveform;             /* transmit pulse: 0 = 132 microsec CW; 1 = 132 microsec FM; */
                                             /* 2 = 176 microsec CW; 3 = 176 microsec FM */
    int             altimeter;               /* altimeter status: 0 = passive, 1 = active */
    unsigned int    raw_data_config;         /* raw data configuration */
    char            spare[32];               /* 32 bytes of reserved space */
}
t_gsfKlein5410BssSpecific;

/* Define the Imagenex Delta T sensor specific data structure */
typedef struct t_gsfDeltaTSpecific
{
    char            decode_file_type[4];     /* contains the decoded files extension. */
    char            version;                 /* contains the minor version number of the delta t */
    int             ping_byte_size;          /* size in bytes of this ping (256 + ((((byte 117[1 or 0])*2) + 2) * number of beams)) */
    struct timespec interrogation_time;      /* The sonar interrogation time */
    int             samples_per_beam;        /* number of samples per beam */
    double          sector_size;             /* size of the sector in degrees */
    double          start_angle;             /* the angle that beam 0 starts at in degrees. */
    double          angle_increment;         /* the number of degrees the angle increments per beam */
    int             acoustic_range;          /* acoustic range in meters */
    int             acoustic_frequency;      /* acoustic frequency in kHz */
    double          sound_velocity;          /* the velocity of sound at the transducer face in m/s */
    double          range_resolution;        /* range resolution in centimeters (documentation says mm but all example data is in cm) */
    double          profile_tilt_angle;      /* the mounting offset */
    double          repetition_rate;         /* time between pings in milliseconds */
    unsigned long   ping_number;             /* the current ping number of this ping.  */
    unsigned char   intensity_flag;          /* this tells whether the GSF will have intensity data (1=true) */
    double          ping_latency;            /* time from sonar ping interrogation to actual ping in seconds */
    double          data_latency;            /* time from sonar ping interrogation to 83P UDP datagram in seconds */
    unsigned char   sample_rate_flag;        /* sampling rate 0 = (1 in 500); 1 = (1 in 5000) */
    unsigned char   option_flags;            /* this flag states whether the data is roll corrected or raybend corrected (1 = roll, 2 = raybend, 3 = both) */
    int             num_pings_avg;           /* number of pings averaged 1 - 25 */
    double          center_ping_time_offset; /* the time difference in seconds between the center ping interrogation and the current ping interrogation */
    unsigned char   user_defined_byte;       /* contains a user defined byte */
    double          altitude;                /* the height of the fish above the ocean floor.  */
    char            external_sensor_flags;   /* this flag is a bit mask where (1 = external heading, 2 = external roll, 4 = external pitch, 8 = external heave) */
    double          pulse_length;            /* acoustic pulse length in seconds */
    double          fore_aft_beamwidth;      /* Effective f/a beam width in degrees */
    double          athwartships_beamwidth;  /* Effective athwartships beam width in degrees */
    unsigned char   spare[32];               /* room to grow */
}
t_gsfDeltaTSpecific;

/* Define sensor specific data structures for the EM12 */
typedef struct t_gsfEM12Specific
{
    int              ping_number;          /* 0 to 65535 */
    int              resolution;           /* 1 = high, 2 = low */
    int              ping_quality;         /* 21 to 81; number of beams with accepted bottom detections */
    double           sound_velocity;       /* m/s */
    int              mode;                 /* 1 to 8; shallow, deep, type of beam spacing */
    unsigned char    spare[32];            /* room to grow */
} t_gsfEM12Specific;

/* Define the R2Sonic sensor specific data structure */
typedef struct t_gsfR2SonicSpecific
{
    unsigned char   model_number[12];   /* Model number, e.g. "2024".  Unused chars are nulls */
    unsigned char   serial_number[12];  /* Serial number, e.g. "100017".  Unused chars are nulls */
    struct timespec dg_time;            /* Ping time, re 00:00:00, Jan 1, 1970 ("Unix time") */
    unsigned int    ping_number;        /* Sequential ping counter relative to power up or reboot */
    double          ping_period;        /* Time interval between two most recent pings, seconds */
    double          sound_speed;        /* Sound speed at transducer face, m/s */
    double          frequency;          /* Sonar center frequency (Hz) */
    double          tx_power;           /* TX source level, dB re 1uPa at 1 meter */
    double          tx_pulse_width;     /* pulse width, seconds */
    double          tx_beamwidth_vert;  /* fore-aft beamwidth, degrees */
    double          tx_beamwidth_horiz; /* athwartship beamwidth, degrees */
    double          tx_steering_vert;   /* fore-aft beam steering angle, degrees */
    double          tx_steering_horiz;  /* athwartship beam steering angle, degrees */
    unsigned int    tx_misc_info;       /* reserved for future use */
    double          rx_bandwidth;       /* receiver bandwidth, Hz */
    double          rx_sample_rate;     /* receiver sample rate, Hz */
    double          rx_range;           /* receiver range setting, seconds in docs but I think it's meters  */
    double          rx_gain;            /* receiver gain setting, 2dB increments between steps */
    double          rx_spreading;       /* TVG spreading law coefficient, e.g. 20log10(range) */
    double          rx_absorption;      /* TVG absorption coefficient, dB/km */
    double          rx_mount_tilt;      /* degrees */
    unsigned int    rx_misc_info;       /* reserved for future use */
    unsigned short  reserved;           /* reserved for future use */
    unsigned short  num_beams;          /* number of beams in this ping */

    /* These fields are from the BTH0 packet only */
    double          A0_more_info[6];     /* Additional fields associated with equi-angular mode; first element of array is roll  */
    double          A2_more_info[6];     /* Additional fields associated with equi-distant mode; first element of array is roll */
    double          G0_depth_gate_min;   /* global minimum gate in seconds (twtt) */
    double          G0_depth_gate_max;   /* global maximum gate in seconds (twtt) */
    double          G0_depth_gate_slope; /* slope of depth gate, degrees */
    unsigned char   spare[32];           /* saved for future expansion */
}
t_gsfR2SonicSpecific;

/* Define a union of the known sensor specific ping subrecords */
typedef union t_gsfSensorSpecific
{
    t_gsfSeaBeamSpecific      gsfSeaBeamSpecific;
    t_gsfEM100Specific        gsfEM100Specific;
    t_gsfEM121ASpecific       gsfEM121ASpecific;
    t_gsfEM121ASpecific       gsfEM121Specific;
    t_gsfSeaBatSpecific       gsfSeaBatSpecific;
    t_gsfEM950Specific        gsfEM950Specific;
    t_gsfEM950Specific        gsfEM1000Specific;
    t_gsfSeamapSpecific       gsfSeamapSpecific;

    #if 1
    /* 03-30-99 wkm/dbj: Obsolete replaced with gsfCmpSassSpecific */
    t_gsfTypeIIISpecific      gsfTypeIIISeaBeamSpecific;
    t_gsfTypeIIISpecific      gsfSASSSpecific;
    #endif

    t_gsfCmpSassSpecific      gsfCmpSassSpecific;

    t_gsfSBAmpSpecific        gsfSBAmpSpecific;
    t_gsfSeaBatIISpecific     gsfSeaBatIISpecific;
    t_gsfSeaBat8101Specific   gsfSeaBat8101Specific;
    t_gsfSeaBeam2112Specific  gsfSeaBeam2112Specific;
    t_gsfElacMkIISpecific     gsfElacMkIISpecific;
    t_gsfEM3Specific          gsfEM3Specific;          /* used for EM120, EM300, EM1002, EM3000, EM3002, and EM121A_SIS */
    t_gsfEM3RawSpecific       gsfEM3RawSpecific;       /* used for EM120, EM300, EM1002, EM3000, EM3002, and EM121A_SIS with raw range and beam angle */
    t_gsfReson8100Specific    gsfReson8100Specific;
    t_gsfReson7100Specific    gsfReson7100Specific;
    t_gsfEM4Specific          gsfEM4Specific;          /* used for EM710, EM302, EM122, and EM2040 */
    t_gsfGeoSwathPlusSpecific gsfGeoSwathPlusSpecific; /* DHG 2006/09/27 Use for GeoSwath+ interferometer */
    t_gsfKlein5410BssSpecific gsfKlein5410BssSpecific; /* Use for Klein 5410 Bathy Sidescan. */
    t_gsfDeltaTSpecific       gsfDeltaTSpecific;
    t_gsfEM12Specific         gsfEM12Specific;
    t_gsfR2SonicSpecific      gsfR2SonicSpecific;

        /* Single beam sensors added */
    t_gsfSBEchotracSpecific   gsfSBEchotracSpecific;
    t_gsfSBEchotracSpecific   gsfSBBathy2000Specific;
    t_gsfSBMGD77Specific      gsfSBMGD77Specific;
    t_gsfSBBDBSpecific        gsfSBBDBSpecific;
    t_gsfSBNOSHDBSpecific     gsfSBNOSHDBSpecific;
    t_gsfSBEchotracSpecific   gsfSBPDDSpecific;
    t_gsfSBNavisoundSpecific  gsfSBNavisoundSpecific;
} gsfSensorSpecific;

/* Define the Echotrac Single-Beam sensor specific data structure. */
typedef struct t_gsfEchotracSpecific
{
    int             navigation_error;
    unsigned short  mpp_source; /* Flag To determine if nav was mpp */
    unsigned short  tide_source;
}
t_gsfEchotracSpecific;

/* Define the MGD77 Single-Beam sensor specific data structure. */
typedef struct t_gsfMGD77Specific
{
    unsigned short  time_zone_corr;
    unsigned short  position_type_code;
    unsigned short  correction_code;
    unsigned short  bathy_type_code;
    unsigned short  quality_code;
    double travel_time;
}
t_gsfMGD77Specific;

/* Define the BDB sensor specific data structure */
typedef struct t_gsfBDBSpecific
{
    int   doc_no;         /* Document number (5 digits) */
    char  eval;           /* Evaluation (1-best, 4-worst) */
    char  classification; /* Classification ((U)nclass, (C)onfidential,
                                             (S)ecret, (P)roprietary/Unclass,
                                             (Q)Proprietary/Class) */
    char  track_adj_flag; /* Track Adjustment Flag (Y,N) */
    char  source_flag;    /* Source Flag ((S)urvey, (R)andom, (O)cean Survey) */
    char  pt_or_track_ln; /* Discrete Point (D) or Track Line (T) Flag */
    char  datum_flag;     /* Datum Flag ((W)GS84, (D)atumless) */
}
t_gsfBDBSpecific;

/* Define the NOS HDB sensor specific data structure */
typedef struct t_gsfNOSHDBSpecific
{
   unsigned short  type_code;    /*  Depth type code  */
   unsigned short  carto_code;   /*  Cartographic code  */
}
t_gsfNOSHDBSpecific;

/* Define a union of the known sensor specific
 * single beam ping subrecords
 */
typedef union t_gsfSBSensorSpecific
{
    t_gsfEchotracSpecific    gsfEchotracSpecific;
    t_gsfEchotracSpecific    gsfBathy2000Specific;
    t_gsfMGD77Specific       gsfMGD77Specific;
    t_gsfBDBSpecific         gsfBDBSpecific;
    t_gsfNOSHDBSpecific      gsfNOSHDBSpecific;
} gsfSBSensorSpecific;

/* Define the bit flags for the "ping_flags" field of the swath bathymetry
 *  ping record.
 * GSF_IGNORE_PING may be set to indicate to an application to ignore this ping
 * GSF_PING_USER_FLAGS 01-15 may be set/read by application specific software
 */
#define GSF_IGNORE_PING       0x0001u
#define GSF_PING_USER_FLAG_01 0x0002u
#define GSF_PING_USER_FLAG_02 0x0004u
#define GSF_PING_USER_FLAG_03 0x0008u
#define GSF_PING_USER_FLAG_04 0x0010u
#define GSF_PING_USER_FLAG_05 0x0020u
#define GSF_PING_USER_FLAG_06 0x0040u
#define GSF_PING_USER_FLAG_07 0x0080u
#define GSF_PING_USER_FLAG_08 0x0100u
#define GSF_PING_USER_FLAG_09 0x0200u
#define GSF_PING_USER_FLAG_10 0x0400u
#define GSF_PING_USER_FLAG_11 0x0800u
#define GSF_PING_USER_FLAG_12 0x1000u
#define GSF_PING_USER_FLAG_13 0x2000u
#define GSF_PING_USER_FLAG_14 0x4000u
#define GSF_PING_USER_FLAG_15 0x8000u

/* Define a set of macros to set, clear, and test the state of the
 *  ping status flags.
 *  Where:
 *     ping_flags: The ping flags field of the gsfSwathBathyPing structure.
 *     usflag:     The definition of the flag to test, set, or clear.
 */
#define gsfTestPingStatus(ping_flags, usflag)    (((ping_flags) & (usflag)) ? 1 : 0)
#define gsfSetPingStatus(ping_flags, usflag)      ((ping_flags) |= (usflag))
#define gsfClearPingStatus(ping_flags, usflag)    ((ping_flags) &= (~(usflag)))

/* Define the GSF bit flags flags for the beam status array.
 * The GSF_IGNORE_BEAM flag may be set to indicate that this beam should
 *  not be used by any processing/display software.  The flags
 *  GSF_BEAM_USER_FLAG_01-07 may be set/read by application specific software
 */
#define GSF_IGNORE_BEAM       0x01u
#define GSF_BEAM_USER_FLAG_01 0x02u
#define GSF_BEAM_USER_FLAG_02 0x04u
#define GSF_BEAM_USER_FLAG_03 0x08u
#define GSF_BEAM_USER_FLAG_04 0x10u
#define GSF_BEAM_USER_FLAG_05 0x20u
#define GSF_BEAM_USER_FLAG_06 0x40u
#define GSF_BEAM_USER_FLAG_07 0x80u

/* Define the internal form of the array subrecord scale factor information,
 * which is used to scale the swath bathymetry ping record to/from
 * internal/external form. The subrecord id is specified by the index into
 * the scaleTable array.
 */
typedef struct t_gsfScaleInfo
{
    unsigned char   compressionFlag;    /* Specifies bytes of storage in high order nibble and type of compression in low order nibble */
    double          multiplier;         /* the scale factor (millionths)for the array */
    double          offset;             /* dc offset to scale data by */
} gsfScaleInfo;
typedef struct t_gsfScaleFactors
{
    int             numArraySubrecords; /* the number of scaling factors we actually have */
    gsfScaleInfo    scaleTable[GSF_MAX_PING_ARRAY_SUBRECORDS];
} gsfScaleFactors;

/* Macro definitions for populating the compression flag (c_flag) argument passed through the
 * gsfLoadScaleFactors(), gsfGetScaleFactor, and gsfLoadDepthScaleFactorAutoOffset() APIs
 */

/* The high order 4 bits are used to define the field size for this array */
#define GSF_FIELD_SIZE_DEFAULT  0x00  /* Default values for field size are used used for all beam arrays */
#define GSF_FIELD_SIZE_ONE      0x10  /* value saved as a one byte value after applying scale and offset */
#define GSF_FIELD_SIZE_TWO      0x20  /* value saved as a two byte value after applying scale and offset */
#define GSF_FIELD_SIZE_FOUR     0x40  /* value saved as a four byte value after applying scale and offset */

/* The low order 4 bits are used to define the compression approach */
#define GSF_DISABLE_COMPRESSION 0x00  /* no compression to be applied to the beam array data. */
                                      /* no compression supported in this version, but may be added in a future release */


typedef struct t_gsfEM3ImagerySpecific
{
    unsigned short range_norm;          /* range to normal incidence used to correct sample amplitudes (in samples) */
    unsigned short start_tvg_ramp;      /* start range sample of TVG ramp if not enough dynamic range (0 else) */
    unsigned short stop_tvg_ramp;       /* stop range sample of TVG ramp if not enough dynamic range (0 else) */
    char           bsn;                 /* normal incidence BS in dB */
    char           bso;                 /* oblique BS in dB */
    double         mean_absorption;     /* mean absorption coeffiecient in dB/km, resolution of 0.01 dB/km) */
    short          offset;              /* Value that has been added to all imagery samples to convert to a positive value */
    short          scale;               /* Manufacturer's specified scale value for each sample. This value is 2 for data from EM3000/EM3002/EM1002/EM300/EM120 */
    unsigned char  spare[4];            /* spare sensor specific subrecord space, reserved for future expansion */
} t_gsfEM3ImagerySpecific;

typedef struct t_gsfReson7100ImagerySpecific
{
    unsigned short size;
    unsigned char  spare[64];            /* spare sensor specific subrecord space, reserved for future expansion */
} t_gsfReson7100ImagerySpecific;

typedef struct t_gsfReson8100ImagerySpecific
{
    unsigned char  spare[8];            /* spare sensor specific subrecord space, reserved for future expansion */
} t_gsfReson8100ImagerySpecific;

typedef struct t_gsfEM4ImagerySpecific
{
    double         sampling_frequency;  /* The system digitizing rate in Hz, value retrieved from the imagery datagram */
    double         mean_absorption;     /* mean absorption coefficient in dB/km, from 0x53 datagram, 0 if data is from 0x59  */
    double         tx_pulse_length;     /* transmit pulse length in microseconds from imagery datagram 0x53, or 0x59 */
    int            range_norm;          /* range to normal incidence used to correct sample amplitudes (in samples) */
    int            start_tvg_ramp;      /* start range (in samples) of TVG ramp if not enough dynamic range 0 means not used */
    int            stop_tvg_ramp;       /* stop range (in samples) of TVG ramp if not enough dynamic range 0 means not used */
    double         bsn;                 /* normal incidence BS in dB */
    double         bso;                 /* oblique incidence BS in dB */
    double         tx_beam_width;       /* transmit beam width in degrees from imagery datagram */
    double         tvg_cross_over;      /* The TVG law crossover angle in degrees */
    short          offset;              /* Value that has been added to all imagery samples to convert to a positive value */
    short          scale;               /* Manufacturer's specified scale value for each sample. This value is 10 for data from EM710/EM302/EM122 */
    unsigned char  spare[20];           /* spare sensor specific subrecord space, reserved for future expansion */
} t_gsfEM4ImagerySpecific;

typedef struct t_gsfKlein5410BssImagerySpecific
{
    unsigned int   res_mode;            /* Descriptor for resolution mode: 0 = normal; 1 = high */
    unsigned int   tvg_page;            /* TVG page number */
    unsigned int   beam_id[5];          /* array of identifiers for five sidescan beam magnitude time series, starting with beam id 1 as the forward-most */
    unsigned char  spare[4];            /* spare sensor specific subrecord space, reserved for future expansion */
} t_gsfKlein5410BssImagerySpecific;

/* Define the R2Sonic sensor imagery data structure.  Very similar to gsfR2SonicSpecific structure */
typedef struct t_gsfR2SonicImagerySpecific
{
    unsigned char   model_number[12];   /* Model number, e.g. "2024".  Unused chars are nulls */
    unsigned char   serial_number[12];  /* Serial number, e.g. "100017".  Unused chars are nulls */
    struct timespec dg_time;            /* Ping time, re 00:00:00, Jan 1, 1970 ("Unix time") */
    unsigned int    ping_number;        /* Sequential ping counter relative to power up or reboot */
    double          ping_period;        /* Time interval between two most recent pings, seconds */
    double          sound_speed;        /* Sound speed at transducer face, m/s */
    double          frequency;          /* Sonar center frequency (Hz) */
    double          tx_power;           /* TX source level, dB re 1uPa at 1 meter */
    double          tx_pulse_width;     /* pulse width, seconds */
    double          tx_beamwidth_vert;  /* fore-aft beamwidth, degrees */
    double          tx_beamwidth_horiz; /* athwartship beamwidth, degreess */
    double          tx_steering_vert;   /* fore-aft beam steering angle, degrees */
    double          tx_steering_horiz;  /* athwartship beam steering angle, degrees */
    unsigned int    tx_misc_info;       /* reserved for future use */
    double          rx_bandwidth;       /* receiver bandwidth, Hz */
    double          rx_sample_rate;     /* receiver sample rate, Hz */
    double          rx_range;           /* receiver range setting, seconds in docs but I think it's meters  */
    double          rx_gain;            /* receiver gain setting, 2dB increments between steps */
    double          rx_spreading;       /* TVG spreading law coefficient, e.g. 20log10(range) */
    double          rx_absorption;      /* TVG absorption coefficient, dB/km */
    double          rx_mount_tilt;      /* degrees */
    unsigned int    rx_misc_info;       /* reserved for future use */
    unsigned short  reserved;           /* reserved for future use */
    unsigned short  num_beams;          /* number of beams in this ping */
    double          more_info[6];       /* reserved for future use, from SNI0 datagram */
    unsigned char   spare[32];          /* saved for future expansion */
}
t_gsfR2SonicImagerySpecific;

typedef union t_gsfSensorImagery
{
    t_gsfEM3ImagerySpecific          gsfEM3ImagerySpecific;          /* used for EM120, EM300, EM1002, EM3000 */
    t_gsfReson7100ImagerySpecific    gsfReson7100ImagerySpecific;    /* For Reson 71P "snippet" imagery */
    t_gsfReson8100ImagerySpecific    gsfReson8100ImagerySpecific;    /* For Reson 81P "snippet" imagery */
    t_gsfEM4ImagerySpecific          gsfEM4ImagerySpecific;          /* used for EM122, EM302, EM710 */
    t_gsfKlein5410BssImagerySpecific gsfKlein5410BssImagerySpecific; /* used for Klein 5410 Bathy Sidescan */
    t_gsfR2SonicImagerySpecific      gsfR2SonicImagerySpecific;      /* used for R2Sonic */
} gsfSensorImagery;

typedef struct gsfTimeSeriesIntensity
{
    unsigned short sample_count;       /* number of amplitude samples per beam */
    unsigned short detect_sample;      /* index of bottom detection sample for the beam */
    unsigned char  spare[8];           /* for future use */
    unsigned int  *samples;            /* Array of per-beam time series intensity samples  */
} gsfTimeSeriesIntensity;

#define GSF_INTENSITY_LINEAR     0x01u
#define GSF_INTENSITY_CALIBRATED 0x02u
#define GSF_INTENSITY_POWER      0x04u
#define GSF_INTENSITY_GAIN       0x08u

typedef struct t_gsfBRBIntensity
{
    unsigned char           bits_per_sample;       /* bits per intensity sample */
    unsigned int            applied_corrections;   /* flags to describe corrections applied to intensity values */
    unsigned char           spare[16];             /* spare header space */
    gsfSensorImagery        sensor_imagery;        /* sensor specific per-ping imagery information */
    gsfTimeSeriesIntensity *time_series;           /* array of per-beam time series intensity values */
} gsfBRBIntensity;

/* Define the data structure for a ping from a swath bathymetric system */
typedef struct t_gsfSwathBathyPing
{
    struct timespec    ping_time;          /* seconds and nanoseconds */
    double             latitude;           /* in degrees, positive going north */
    double             longitude;          /* in degrees, positive going east */
    double             height;             /* height above ellipsoid, positive value defines a point above ellipsoid */
    double             sep;                /* distance from ellipsoid to vertical datum, positive value indicates datum above ellipsoid */
    short              number_beams;       /* in this ping */
    short              center_beam;        /* offset into array (0 = portmost outer) */
    unsigned short     ping_flags;         /* flags to mark status of this ping */
    short              reserved;           /* for future use */
    double             tide_corrector;     /* in meters */
    double             gps_tide_corrector; /* in meters */
    double             depth_corrector;    /* in meters */
    double             heading;            /* in degrees */
    double             pitch;              /* in degrees */
    double             roll;               /* in degrees */
    double             heave;              /* in meters    */
    double             course;             /* in degrees */
    double             speed;              /* in knots */
    gsfScaleFactors    scaleFactors;       /* The array scale factors for this data */
    double            *depth;              /* depth array (meters) */
    double            *nominal_depth;      /* Array of depth relative to 1500 m/s */
    double            *across_track;       /* across track array (meters) */
    double            *along_track;        /* along track array (meters) */
    double            *travel_time;        /* roundtrip travel time array (seconds) */
    double            *beam_angle;         /* beam angle array (degrees from vertical) */
    double            *mc_amplitude;       /* mean, calibrated beam amplitude array (dB re 1V/micro pascal at 1 meter) */
    double            *mr_amplitude;       /* mean, relative beam amplitude array (dB re 1V/micro pascal at 1 meter) */
    double            *echo_width;         /* echo width array (seconds) */
    double            *quality_factor;     /* quality factor array (dimensionless) */
    double            *receive_heave;      /* Array of heave data (meters) */
    double            *depth_error;        /* Array of estimated vertical error (meters) */
    double            *across_track_error; /* Array of estimated across track error (meters) */
    double            *along_track_error;  /* Array of estimated along track error (meters) */
    unsigned char     *quality_flags;      /* Two bit beam detection flags provided by Reson sonar */
    unsigned char     *beam_flags;         /* Array of beam status flags */
    double            *signal_to_noise;    /* signal to noise ratio (dB) */
    double            *beam_angle_forward; /* beam angle forward array (degrees counterclockwise from stbd.) */
    double            *vertical_error;     /* Array of estimated vertical error (meters, at 95% confidence) */
    double            *horizontal_error;   /* Array of estimated horizontal error (meters, at 95% confidence) */
    unsigned short    *sector_number;      /* Array of values that specify the transit sector for this beam */
    unsigned short    *detection_info;     /* Array of values that specify the method of bottom detection */
    double            *incident_beam_adj;  /* Array of values that specify incident beam angle adjustment from beam_angle */
    unsigned short    *system_cleaning;    /* Array of values that specify data cleaning information from the sensor system */
    double            *doppler_corr;       /* Array of values used to correct the travel times for Doppler when transmission is FM */
    double            *sonar_vert_uncert;  /* vertical uncertainty provided by the sonar */
    int                sensor_id;          /* a definition which specifies the sensor */
    gsfSensorSpecific  sensor_data;        /* union of known sensor specific data */
    gsfBRBIntensity   *brb_inten;          /* Structure containing bathymetric receive beam time series intensities */
}
gsfSwathBathyPing;

/* Define a single beam record structure. */
typedef struct t_gsfSingleBeamPing
{
    struct timespec ping_time;          /* Time the sounding was made */
    double          latitude;           /* latitude (degrees) of sounding */
    double          longitude;          /* longitude (degrees) of sounding */
    double          tide_corrector;     /* in meters */
    double          depth_corrector;    /* in meters draft corrector for sensor */
    double          heading;            /* in degrees */
    double          pitch;              /* in meters */
    double          roll;               /* in meters */
    double          heave;              /* in meters */
    double          depth;              /* in meters */
    double          sound_speed_correction;  /* in meters */
    unsigned short  positioning_system_type;
    int             sensor_id;
    gsfSBSensorSpecific sensor_data;
}
gsfSingleBeamPing;

/* Define the sound velocity profile structure */
typedef struct t_gsfSVP
{
    struct timespec observation_time;   /* time the SVP measurement was made            */
    struct timespec application_time;   /* time the SVP was used by the sonar           */
    double          latitude;           /* latitude (degrees) of SVP measurement        */
    double          longitude;          /* longitude (degrees) of SVP measurement       */
    int             number_points;      /* number of data points in the profile         */
    double         *depth;              /* array of profile depth values in meters      */
    double         *sound_speed;        /* array of profile sound velocity values in m/s*/
}
gsfSVP;

/* Define the internal record structure for processing parameters */
#define GSF_MAX_PROCESSING_PARAMETERS 128
typedef struct t_gsfProcessingParameters
{
    struct timespec param_time;
    int             number_parameters;
    short           param_size[GSF_MAX_PROCESSING_PARAMETERS];  /* array of sizes of param text */
    char           *param[GSF_MAX_PROCESSING_PARAMETERS];       /* array of parameters: "param_name=param_value" */
}
gsfProcessingParameters;

/* Define the sensor parameters record structure */
#define GSF_MAX_SENSOR_PARAMETERS 128
typedef struct t_gsfSensorParameters
{
    struct timespec param_time;
    int             number_parameters;
    short           param_size[GSF_MAX_SENSOR_PARAMETERS];      /* array of sizes of param text */
    char           *param[GSF_MAX_SENSOR_PARAMETERS];   /* array of parameters: "param_name=param_value" */
}
gsfSensorParameters;

/* Define the comment record structure */
typedef struct t_gsfComment
{
    struct timespec comment_time;
    int             comment_length;
    char           *comment;
}
gsfComment;

/* Define the history record */
#define GSF_OPERATOR_LENGTH  64
#define GSF_HOST_NAME_LENGTH 64
typedef struct t_gsfHistory
{
    struct timespec history_time;
    char            host_name[GSF_HOST_NAME_LENGTH + 1];
    char            operator_name[GSF_OPERATOR_LENGTH + 1];
    char           *command_line;
    char           *comment;
}
gsfHistory;

/* Define the navigation error record
 * jsb As of GSF v1.07, this record is replaced by gsfHVNavigationError.
 * All newly created files should be written using gsfHVNavigationError,
 * instead of gsfNavigationError.
 */
typedef struct t_gsfNavigationError
{
    struct timespec nav_error_time;
    int             record_id;          /* Containing nav with these errors */
    double          latitude_error;     /* 90% CE in meters */
    double          longitude_error;    /* 90% CE in meters */
}
gsfNavigationError;

/* jsb As of GSF v1.07, This new navigation error record replaces gsfNavigationError.
 *  The definition of gsfNavigationError will remain in the specification for several
 *  release of GSF for backwards compatability. (The HV stands for Horizontal and Vertical)
 */
typedef struct t_gsfHVNavigationError
{
    struct timespec nav_error_time;
    int             record_id;                 /* Containing nav with these errors */
    double          horizontal_error;          /* RMS error in meters */
    double          vertical_error;            /* RMS error in meters */
    double          SEP_uncertainty;           /* RMS uncertainty of SEP in meters. Set to 0.0 if not applicable */
    char            spare[2];                  /* Two bytes of reserved space */
    char           *position_type;             /* A character string code which specifies the type of positioning system */
}
gsfHVNavigationError;

/* Define a set of macros which may to used to set the position_type field */
#define GSF_POS_TYPE_UNKN "UNKN"               /* Unknown positioning system type */
#define GSF_POS_TYPE_GPSU "GPSU"               /* GPS Position, unknown positioning service */
#define GSF_POS_TYPE_PPSD "PPSD"               /* Precise positioning service - differential */
#define GSF_POS_TYPE_PPSK "PPSK"               /* Precise positioning service - kinematic */
#define GSF_POS_TYPE_PPSS "PPSS"               /* Precise positioning service - standalone */
#define GSF_POS_TYPE_PPSG "PPSG"               /* Precise positioning service - gypsy */
#define GSF_POS_TYPE_SPSD "SPSD"               /* Standard positioning service - differential */
#define GSF_POS_TYPE_SPSK "SPSK"               /* Standard positioning service - kinematic */
#define GSF_POS_TYPE_SPSS "SPSS"               /* Standard positioning service - standalone */
#define GSF_POS_TYPE_SPSG "SPSG"               /* Standard positioning service - gypsy */
#define GSF_POS_TYPE_GPPP "GPPP"               /* Post Processing - Precise Point Positioning */
#define GPS_POS_TYPE_GPPK "GPPK"               /* Post Processing - Post Processed Kinematic */

#define GSF_POS_TYPE_INUA "INUA"   /* Inertial measurements only, unaided */
#define GSF_POS_TYPE_INVA "INVA"   /* Inertial measurements with absolute velocity aiding */
#define GSF_POS_TYPE_INWA "INWA"   /* Inertial measurements with water-relative velocity aiding */
#define GSF_POS_TYPE_LBLN "LBLN"   /* One or more long-baseline acoustic navigation lines of position */
#define GSF_POS_TYPE_USBL "USBL"   /* ultra-short baseline acoustic navigation */

#define GSF_POS_TYPE_PIUA "PIUA"   /* Post-processed inertial measurements only, unaided */
#define GSF_POS_TYPE_PIVA "PIVA"   /* Post-processed Inertial measurements with absolute velocity aiding */
#define GSF_POS_TYPE_PIWA "PIWA"   /* Post-processed Inertial measurements with water-relative velocity aiding */
#define GSF_POS_TYPE_PLBL "PLBL"   /* Post-processed One or more long-baseline acoustic navigation lines of position */
#define GSF_POS_TYPE_PSBL "PSBL"   /* Post-processed ultra-short baseline acoustic navigation */

/* Define the data structure for a ping from a swath bathymetric system */
typedef struct t_gsfAttitude
{
    short            num_measurements;      /* number of attitude measurements in this record */
    struct timespec *attitude_time;         /* seconds and nanoseconds */
    double          *pitch;                 /* in degrees */
    double          *roll;                  /* in degrees */
    double          *heave;                 /* in meters */
    double          *heading;               /* in degrees */
}
gsfAttitude;

/* Define a structure to encapsulate the known gsf records, this
 * simplifies the number of arguments to gsfRead, and gsfWrite.
 */
typedef struct t_gsfRecords
{
    gsfHeader               header;
    gsfSwathBathySummary    summary;
    gsfSwathBathyPing       mb_ping;
    gsfSingleBeamPing       sb_ping;
    gsfSVP                  svp;
    gsfProcessingParameters process_parameters;
    gsfSensorParameters     sensor_parameters;
    gsfComment              comment;
    gsfHistory              history;
    gsfNavigationError      nav_error;
    gsfHVNavigationError    hv_nav_error;
    gsfAttitude             attitude;
} gsfRecords;

/* Define a data structure to hold offsets needed to correct multibeam
 * bathymetric data. Currently gsf supports tracking of up to two pairs
 * of each of the relavent offsets.  This is required for systems such as
 * HydroChart II and Reson 9002 which have two pairs of transmit/receive
 * arrays per installation.
 */
#define GSF_MAX_OFFSETS                2
#define GSF_COMPENSATED                1
#define GSF_UNCOMPENSATED              0
#define GSF_TRUE_DEPTHS                1
#define GSF_DEPTHS_RE_1500_MS          2
#define GSF_DEPTH_CALC_UNKNOWN         3
#define GSF_UNKNOWN_PARAM_VALUE  DBL_MIN        /* defined in <float.h> */
#define GSF_UNKNOWN_PARAM_INT        -99
#define GSF_TRUE                       1
#define GSF_FALSE                      0
#define GSF_NUMBER_PROCESSING_PARAMS  49

/* Macro definitions for type of platform */
#define GSF_PLATFORM_TYPE_SURFACE_SHIP  0              /*DHG 2008/12/22 Add for AUV vs Surface Ship discrimination */
#define GSF_PLATFORM_TYPE_AUV           1              /*DHG 2008/12/22 Add for AUV vs Surface Ship discrimination */
#define GSF_PLATFORM_TYPE_ROTV          2

typedef struct t_gsfMBOffsets
{
    double           draft[GSF_MAX_OFFSETS];                     /* meters */
    double           pitch_bias[GSF_MAX_OFFSETS];                /* pitch bias in degrees, results from patch test */
    double           roll_bias[GSF_MAX_OFFSETS];                 /* roll bias in degrees, results from patch test */
    double           gyro_bias[GSF_MAX_OFFSETS];                 /* gyro bias in degrees, results from patch test */
    double           position_x_offset;                          /* meters */
    double           position_y_offset;                          /* meters */
    double           position_z_offset;                          /* meters */
    double           antenna_x_offset;                           /* meters */
    double           antenna_y_offset;                           /* meters */
    double           antenna_z_offset;                           /* meters */
    double           transducer_x_offset[GSF_MAX_OFFSETS];       /* sonar X installation offset in meters, from ship alignment survey */
    double           transducer_y_offset[GSF_MAX_OFFSETS];       /* sonar Y installation offset in meters, from ship alignment survey */
    double           transducer_z_offset[GSF_MAX_OFFSETS];       /* sonar Z installation offset in meters, from ship alignment survey */
    double           transducer_pitch_offset[GSF_MAX_OFFSETS];   /* sonar pitch installation angle in degrees, from ship alignment survey */
    double           transducer_roll_offset[GSF_MAX_OFFSETS];    /* sonar roll installation angle in degrees, from ship alignment survey */
    double           transducer_heading_offset[GSF_MAX_OFFSETS]; /* sonar heading installation  angle in degrees, from ship alignment survey */
    double           mru_pitch_bias;                             /* MRU installation pitch angle in degrees, from ship alignment survey */
    double           mru_roll_bias;                              /* MRU installation roll angle in degrees, from ship alignment survey */
    double           mru_heading_bias;                           /* MRU installation heading angle in degrees, from ship alignment survey */
    double           mru_x_offset;                               /* MRU X installation offset in meters, from ship alignment survey */
    double           mru_y_offset;                               /* MRU Y installation offset in meters, from ship alignment survey meters */
    double           mru_z_offset;                               /* MUR Z installation offset in meters, from ship alignment survey meters */
    double           center_of_rotation_x_offset;                /* meters */
    double           center_of_rotation_y_offset;                /* meters */
    double           center_of_rotation_z_offset;                /* meters */
    double           position_latency;                           /* seconds */
    double           attitude_latency;                           /* seconds */
    double           depth_sensor_latency;                       /* seconds */
    double           depth_sensor_x_offset;                      /* for subsurface vehicles; distance of pressure sensor from MRP in meters */
    double           depth_sensor_y_offset;                      /* for subsurface vehicles; distance of pressure sensor from MRP in meters */
    double           depth_sensor_z_offset;                      /* for subsurface vehicles; distance of pressure sensor from MRP in meters */
    double           rx_transducer_x_offset[GSF_MAX_OFFSETS];      /* If the receive array is in a different location than   */
    double           rx_transducer_y_offset[GSF_MAX_OFFSETS];      /* the transmit array these "rx" offsets will be used for */
    double           rx_transducer_z_offset[GSF_MAX_OFFSETS];      /* the receive array and the transducer offsets defined   */
    double           rx_transducer_pitch_offset[GSF_MAX_OFFSETS];  /* above will be used for the transmit array              */
    double           rx_transducer_roll_offset[GSF_MAX_OFFSETS];
    double           rx_transducer_heading_offset[GSF_MAX_OFFSETS];
} gsfMBOffsets;

/* Macro definitions for roll_reference type */
#define GSF_HORIZONTAL_PITCH_AXIS          1
#define GSF_ROTATED_PITCH_AXIS             2


/* Define a data structure to hold multibeam sonar processing parameters */
typedef struct t_gsfMBParams
{
    /* These parameters define reference points */
    char start_of_epoch[64];
    int horizontal_datum;
    int vertical_datum;

    int utc_offset;                 /* = The offset in hours from UTC to the local time when the GSF file was created */

    /* these parameters define the installed hardware */
    int number_of_transmitters;      /* = The number of transmitters installed */
    int number_of_receivers;         /* = The number of receivers installed */

    /* These parameters specify what corrections have been applied to the data */
    int roll_reference;             /* = flag to indicate whether roll is horizontal pitch axis or rotated pitch axis */
    int roll_compensated;           /* = GSF_COMPENSATED if the depth data has been corrected for roll */
    int pitch_compensated;          /* = GSF_COMPENSATED if the depth data has been corrected for pitch */
    int heave_compensated;          /* = GSF_COMPENSATED if the depth data has been corrected for heave */
    int tide_compensated;           /* = GSF_COMPENSATED if the depth data has been corrected for tide */
    int ray_tracing;                /* = GSF_COMPENSATED if the travel time/angle pairs are compensated for ray tracing */
    int depth_calculation;          /* = GSF_TRUE_DEPTHS, or GSF_DEPTHS_RE_1500_MS, applicable to the depth field */
    int vessel_type;                /* DHG 2008/12/18 Add "VESSEL_TYPE" to Processing Parameters */
    int full_raw_data;              /* = GSF_TRUE if this GSF file has sufficient information to support full recalculation of X,Y,Z from raw measurements, otherwise = GSF_FALSE */
    int msb_applied_to_attitude;    /* = GSF_TRUE if the motion sensor bias values (from patch test) have been added to the attitude values in the ping record and attitude record */
    int heave_removed_from_gps_tc;  /* = GSF_TRUE if the heave has been removed from the gps_tide_corrector */

    /* These parameters specify known offsets which have NOT been corrected.
     * If each of these values are zero, then all known offsets have been
     * corrected for.
     */
    gsfMBOffsets to_apply;

    /* These parameters specify offsets which have already been corrected. */
    gsfMBOffsets applied;
} gsfMBParams;

/* Macro definitions for approved horizontal datums. Note that as of
 * 12/20/95 only WGS-84 is supported by GSF.
 */
#define GSF_H_DATUM_ADI   1   /* Adinan */
#define GSF_H_DATUM_ARF   2   /* Arc 1950 */
#define GSF_H_DATUM_ARS   3   /* Arc 1960 */
#define GSF_H_DATUM_AUA   4   /* Australian Geodetic */
#define GSF_H_DATUM_BAT   5   /* Djakarta (Batavia) */
#define GSF_H_DATUM_BID   6   /* Bissau Base Northwest End Pillar, Portugese Guinea */
#define GSF_H_DATUM_BUR   7   /* Bukit Rimpah */
#define GSF_H_DATUM_CAI   8   /* Campo Inchauspe */
#define GSF_H_DATUM_CAM   9   /* Camacupa Base SW End */
#define GSF_H_DATUM_CAP  10   /* Cape, South Africa */
#define GSF_H_DATUM_CAA  11   /* Campo Area Astro */
#define GSF_H_DATUM_CHO  12   /* Chatham Island Astro */
#define GSF_H_DATUM_CHU  13   /* Chua Astro */
#define GSF_H_DATUM_COA  14   /* Corrego Alegre */
#define GSF_H_DATUM_ENB  15   /* European 79 */
#define GSF_H_DATUM_EUR  16   /* European */
#define GSF_H_DATUM_GDA  17   /* German */
#define GSF_H_DATUM_GEO  18   /* Geodetic Datum 1949 */
#define GSF_H_DATUM_GHA  19   /* Ghana */
#define GSF_H_DATUM_GSB  20   /* G. Segara */
#define GSF_H_DATUM_GSF  21   /* G. Serindung */
#define GSF_H_DATUM_GUA  22   /* Guam 1963 */
#define GSF_H_DATUM_HEN  23   /* Herat North */
#define GSF_H_DATUM_HER  24   /* Hermannskogel */
#define GSF_H_DATUM_HJO  25   /* Hjorsey 1955 */
#define GSF_H_DATUM_HTN  26   /* Hu-Tu-Shan */
#define GSF_H_DATUM_IDA  27   /* Italian */
#define GSF_H_DATUM_IND  28   /* Indian */
#define GSF_H_DATUM_IRE  29   /* Ireland 1965 */
#define GSF_H_DATUM_KEA  30   /* Kertau */
#define GSF_H_DATUM_LIB  31   /* Liberia 1964 (Robertsfield Astro) */
#define GSF_H_DATUM_LOC  32   /* Local Astro */
#define GSF_H_DATUM_LUZ  33   /* Luzon */
#define GSF_H_DATUM_MER  34   /* Merchich */
#define GSF_H_DATUM_MET  35   /* Mercury */
#define GSF_H_DATUM_MOL  36   /* Montjong Lowe */
#define GSF_H_DATUM_NAN  37   /* Nanking 1960 */
#define GSF_H_DATUM_NAR  38   /* North American 1983 */
#define GSF_H_DATUM_NAS  39   /* North American 1927 */
#define GSF_H_DATUM_NIG  40   /* Nigeria */
#define GSF_H_DATUM_OGB  41   /* Ordnance Survey of Great Britain 1936 */
#define GSF_H_DATUM_OHA  42   /* Old Hawaiian */
#define GSF_H_DATUM_OSI  43   /* Ordnance Survey of Ireland */
#define GSF_H_DATUM_PLN  44   /* Pico de las Nieves, Gran Canaria, Canary Islands */
#define GSF_H_DATUM_PRP  45   /* Provisional South American 1956 */
#define GSF_H_DATUM_QUO  46   /* Qornoq */
#define GSF_H_DATUM_SIB  47   /* Sierra leone 1960 */
#define GSF_H_DATUM_TAN  48   /* Tananarive Obsv 1925 */
#define GSF_H_DATUM_TIL  49   /* Timbalai */
#define GSF_H_DATUM_TOK  50   /* Tokyo */
#define GSF_H_DATUM_UND  51   /* Undetermined */
#define GSF_H_DATUM_VOI  52   /* Voirol */
#define GSF_H_DATUM_WGA  53   /* World Geodetic System 1960 */
#define GSF_H_DATUM_WGB  54   /* World Geodetic System 1966 */
#define GSF_H_DATUM_WGC  55   /* World Geodetic System 1972 */
#define GSF_H_DATUM_WGD  56   /* World Geodetic System 1980 */
#define GSF_H_DATUM_WGE  57   /* World Geodetic System 1984 */
#define GSF_H_DATUM_WGS  58   /* World Geodetic System (year unknown) */
#define GSF_H_DATUM_XXX  59   /* Multiple datums */
#define GSF_H_DATUM_YAC  60   /* Yacare */

/* Macro definitions for supported vertical datums */
#define GSF_V_DATUM_UNKNOWN  1  /* Unknown vertical datum */
#define GSF_V_DATUM_MLLW     2  /* Mean lower low water */
#define GSF_V_DATUM_MLW      3  /* Mean low water */
#define GSF_V_DATUM_ALAT     4  /* Aprox Lowest Astronomical Tide */
#define GSF_V_DATUM_ESLW     5  /* Equatorial Springs Low Water */
#define GSF_V_DATUM_ISLW     6  /* Indian Springs Low Water */
#define GSF_V_DATUM_LAT      7  /* Lowest Astronomical Tide */
#define GSF_V_DATUM_LLW      8  /* Lowest Low Water */
#define GSF_V_DATUM_LNLW     9  /* Lowest Normal Low Water */
#define GSF_V_DATUM_LWD     10  /* Low Water Datum */
#define GSF_V_DATUM_MLHW    11  /* Mean Lower High Water */
#define GSF_V_DATUM_MLLWS   12  /* Mean Lower Low Water Springs */
#define GSF_V_DATUM_MLWN    13  /* Mean Low Water Neap */
#define GSF_V_DATUM_MSL     14  /* Mean Sea Level */

/* Define the error codes which gsfError may be set to */
#define GSF_NORMAL                                 0
#define GSF_FOPEN_ERROR                           -1
#define GSF_UNRECOGNIZED_FILE                     -2
#define GSF_BAD_ACCESS_MODE                       -3
#define GSF_READ_ERROR                            -4
#define GSF_WRITE_ERROR                           -5
#define GSF_INSUFFICIENT_SIZE                     -6
#define GSF_RECORD_SIZE_ERROR                     -7
#define GSF_CHECKSUM_FAILURE                      -8
#define GSF_FILE_CLOSE_ERROR                      -9
#define GSF_TOO_MANY_ARRAY_SUBRECORDS            -10
#define GSF_TOO_MANY_OPEN_FILES                  -11
#define GSF_MEMORY_ALLOCATION_FAILED             -12
#define GSF_UNRECOGNIZED_RECORD_ID               -13
#define GSF_STREAM_DECODE_FAILURE                -14
#define GSF_BAD_SEEK_OPTION                      -15
#define GSF_FILE_SEEK_ERROR                      -16
#define GSF_UNRECOGNIZED_SENSOR_ID               -17
#define GSF_UNRECOGNIZED_DATA_RECORD             -18
#define GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID      -19
#define GSF_UNRECOGNIZED_SUBRECORD_ID            -20
#define GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER      -21
#define GSF_CANNOT_REPRESENT_PRECISION           -22
#define GSF_READ_TO_END_OF_FILE                  -23
#define GSF_BAD_FILE_HANDLE                      -24
#define GSF_HEADER_RECORD_DECODE_FAILED          -25
#define GSF_MB_PING_RECORD_DECODE_FAILED         -26
#define GSF_SVP_RECORD_DECODE_FAILED             -27
#define GSF_PROCESS_PARAM_RECORD_DECODE_FAILED   -28
#define GSF_SENSOR_PARAM_RECORD_DECODE_FAILED    -29
#define GSF_COMMENT_RECORD_DECODE_FAILED         -30
#define GSF_HISTORY_RECORD_DECODE_FAILED         -31
#define GSF_NAV_ERROR_RECORD_DECODE_FAILED       -32
#define GSF_HEADER_RECORD_ENCODE_FAILED          -25
#define GSF_MB_PING_RECORD_ENCODE_FAILED         -26
#define GSF_SVP_RECORD_ENCODE_FAILED             -27
#define GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED   -28
#define GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED    -29
#define GSF_COMMENT_RECORD_ENCODE_FAILED         -30
#define GSF_HISTORY_RECORD_ENCODE_FAILED         -31
#define GSF_NAV_ERROR_RECORD_ENCODE_FAILED       -32
#define GSF_SETVBUF_ERROR                        -33
#define GSF_FLUSH_ERROR                          -34
#define GSF_FILE_TELL_ERROR                      -35
#define GSF_INDEX_FILE_OPEN_ERROR                -36
#define GSF_CORRUPT_INDEX_FILE_ERROR             -37
#define GSF_SCALE_INDEX_CALLOC_ERROR             -38
#define GSF_RECORD_TYPE_NOT_AVAILABLE            -39
#define GSF_SUMMARY_RECORD_DECODE_FAILED         -40
#define GSF_SUMMARY_RECORD_ENCODE_FAILED         -41
#define GSF_INVALID_NUM_BEAMS                    -42
#define GSF_INVALID_RECORD_NUMBER                -43
#define GSF_INDEX_FILE_READ_ERROR                -44
#define GSF_PARAM_SIZE_FIXED                     -45
#define GSF_SINGLE_BEAM_ENCODE_FAILED            -46
#define GSF_HV_NAV_ERROR_RECORD_ENCODE_FAILED    -47
#define GSF_HV_NAV_ERROR_RECORD_DECODE_FAILED    -48
#define GSF_ATTITUDE_RECORD_ENCODE_FAILED        -49
#define GSF_ATTITUDE_RECORD_DECODE_FAILED        -50
#define GSF_OPEN_TEMP_FILE_FAILED                -51
#define GSF_PARTIAL_RECORD_AT_END_OF_FILE        -52
#define GSF_QUALITY_FLAGS_DECODE_ERROR           -53

typedef struct t_gsf_gp{
    double lon;            /* degrees */
    double lat;            /* degrees */
    double z;              /* meters */
} GSF_POSITION;

/*
    note: the coordinate system is:
    +x forward, +y starboard, + z down, +hdg cw from north
*/

typedef struct t_gsf_pos_offsets{
    double x;              /* meters */
    double y;              /* meters */
    double z;              /* meters */
} GSF_POSITION_OFFSETS;

/* Global external data defined in this module */

/* The following are the function protoytpes for all functions intended
 * to be exported by the library.

    Fugro modification - Mitch Ames - 2012-02-15
    The original exported functions did not use const pointers everywhere that they could or should.
    I've added const where appropriate.
 */

int OPTLK       gsfOpen(const char *filename, const int mode, int *handle);
/*
 * Description : This function attempts to open a gsf data file.  If the
 *  file exits and is opened readonly or update the gsf header is read
 *  to confirm that this is a gsf data file.  If the file is opened create,
 *  the GSF header containing the version number of the software library is
 *  written into the header.  This function passes an integer handle back to
 *  the calling application.  The handle is used for all further access to the
 *  file. gsfOpen explicitly sets stream bufferring to the value specified
 *  by GSF_STREAM_BUF_SIZE.  The internal file table is searched for an
 *  available entry whose name matches that specified in the argument list, if
 *  no match is found, then the first available entry is used.  Up to
 *  GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file to open
 *  mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading an writing
 *     GSF_CREATE   = create a new gsf file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading an writing with index
 *  handle = a pointer to an integer to be assigned a handle which will be
 *     reference for all future file access.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_BAD_ACCESS_MODE
 *     GSF_TOO_MANY_OPEN_FILES
 *     GSF_FOPEN_ERROR
 *     GSF_SETVBUF_ERROR
 *     GSF_UNRECOGNIZED_FILE
 */

int OPTLK       gsfOpenBuffered(const char *filename, const int mode, int *handle, int buf_size);
/*
 * Description : This function attempts to open a gsf data file.  If the
 *  file exits and is opened readonly or update the gsf header is read
 *  to confirm that this is a gsf data file.  If the file is opened create,
 *  the GSF header containing the version number of the software library is
 *  written into the header.  This function passes an integer handle back to
 *  the calling application.  The handle is used for all further access to the
 *  file. gsfOpenBufferd explicitly sets stream bufferring to the value
 *  specified by the buf_size argument. The internal file table is searched
 *  for an available entry whose name matches that specified in the argument
 *  list, if no match is found, then the first available entry is used.  Up
 *  to GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *  gsfOpenBuffered performs identical processing to gsfOpen, except here,
 *  the caller is allowed to explicitly set the standard system library level
 *  I/O buffer size.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file to open
 *  mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading an writing
 *     GSF_CREATE   = create a new gsf file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading an writing with index
 *  handle = a pointer to an integer to be assigned a handle which will be
 *     reference for all future file access.
 *  buf_size = an integer buffer size in bytes.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_BAD_ACCESS_MODE
 *     GSF_TOO_MANY_OPEN_FILES
 *     GSF_FOPEN_ERROR
 *     GSF_SETVBUF_ERROR
 *     GSF_UNRECOGNIZED_FILE
 */

int OPTLK       gsfClose(const int handle);
/*
 * Description : This function closes a gsf file previously openned
 *  using gsfOpen.
 *
 * Inputs :
 *  handle = the handle of the gsf file to be closed.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_CLOSE_ERROR
 */

int OPTLK       gsfSeek(int handle, int option);
/*
 * Description : This function may be used to move the file pointer
 *  for a previously openned gsf file.
 *
 * Inputs :
 *  handle = the integer handle returned from gsf Open
 *  option = the desired action for moving the file pointer, where:
 *    GSF_REWIND, move pointer to first record in the file.
 *    GSF_END_OF_FILE, move pointer to the end of the file.
 *    GSF_PREVIOUS_RECORD, backup to the beginning of the record just
 *     written or just read.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_SEEK_ERROR
 *    GSF_BAD_SEEK_OPTION
 */

int OPTLK       gsfRead(int handle, int desiredRecord, gsfDataID * dataID, gsfRecords * rec, unsigned char *stream, int max_size);
/* Description : gsfRead supports both direct and sequential access. If the
 *  file is opened for sequential access, this function reads the desired
 *  record from the gsf data file specified by handle.  The "desiredRecord"
 *  argument may be set to GSF_NEXT_RECORD to read the next record in the
 *  data file, or "desiredRecord" record may be set to the id of the record
 *  of interest, in which case the file will be read, skipping past
 *  intermediary records until the desired record is found.  When the desired
 *  record is found, it is read and then decoded from external to internal
 *  form. If the optional checksum is found with the data it will be verified.
 *  All of the fields of the gsfDataID structure, with the exception of the
 *  record_number field will be loaded with the values contained in the GSF
 *  record byte stream.  The record_number field will be undefined.  The
 *  stream and max_size arguments are normally set to NULL, unless the
 *  calling application is interested in a copy of the GSF byte stream.
 *
 *  If the file is opened for direct access, then the combination of the
 *  recordID and the record_number fields of the dataID structure are used
 *  to uniquely identify the record of interest.  The address for this record
 *  is retrieved from the index file, which was created on a previous call
 *  to gsfOpen or gsfOpenBuffered.  If the record of interest is a ping record
 *  for which we need to retrieve new scale factors, then the ping record
 *  containing the scale factors needed is read first, and then the ping
 *  record of interest is read.  Direct access applications should set the
 *  desiredRecord argument equal to the recordID field in the gsfDataID
 *  structure.
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *    dataID = a pointer to a gsfDataID structure to be populated for the
 *             input record.
 *    rptr = a pointer to a gsfRecords structure to be populated with the
 *           data from the input record in internal form.
 *    stream = an optional pointer to caller memory to be populated with a copy
 *          of the gsf byte stream for this record.
 *    max_size = an optional maximum size to copy into buf
 *
 * Returns :
 *  This function returns the number of bytes read if successful,
 *  or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_SEEK_ERROR
 *    GSF_FLUSH_ERROR
 *    GSF_READ_TO_END_OF_FILE
 *    GSF_READ_ERROR
 *    GSF_RECORD_SIZE_ERROR
 *    GSF_INSUFFICIENT_SIZE
 *    GSF_CHECKSUM_FAILURE
 *    GSF_UNRECOGNIZED_RECORD_ID
 *    GSF_HEADER_RECORD_DECODE_FAILED
 *    GSF_SVP_RECORD_DECODE_FAILED
 *    GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *    GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *    GSF_COMMENT_RECORD_DECODE_FAILED
 *    GSF_HISTORY_RECORD_DECODE_FAILED
 *    GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int OPTLK       gsfWrite(int handle, gsfDataID * id, gsfRecords * record);
/* Description : gsfWrite encodes the data from internal to external form,
 *  and then writes the requested record into the file specified by handle,
 *  where handle is the value retured by gsfOpen.  The record is written to
 *  the current file pointer for handle.  An optional checksum may be computed
 *  and encoded with the data.
 *
 *  If the file is opened for sequential access (GSF_CREATE, or GSF_UPDATE)
 *  then the recordID field of the gsfDataID structure is used to specify
 *  the record to be written.  The record is written at the current location
 *  in the file.
 *
 *  If the file is opened for direct access (GSF_UPDATE_INDEX), then the
 *  combination of the recordID and the record_number fields of the gsfDataID
 *  structure are used to uniquely identify the record to be written.  The
 *  address of the record of interest is read from the index file and the file
 *  pointer is moved to this offset before the record is encoded and written
 *  to disk.
 *
 * Inputs :
 *  handle = the handle for this file as returned by gsfOpen
 *  id = a pointer to a gsfDataID containing the record id information for
 *       the record to write.
 *  rptr = a pointer to a gsfRecords structure from which to get the internal
 *         form of the record to be written to the file.
 *
 * Returns :
 *  This function returns the number of bytes written if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_UNRECOGNIZED_RECORD_ID
 *    GSF_FILE_SEEK_ERROR
 *    GSF_WRITE_ERROR
 *    GSF_HEADER_RECORD_ENCODE_FAILED
 *    GSF_SVP_RECORD_ENCODE_FAILED
 *    GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED
 *    GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED
 *    GSF_COMMENT_RECORD_ENCODE_FAILED
 *    GSF_HISTORY_RECORD_ENCODE_FAILED
 *    GSF_NAV_ERROR_RECORD_ENCODE_FAILED
 *    GSF_FLUSH_ERROR
 *    GSF_SINGLE_BEAM_ENCODE_FAILED
 */

int OPTLK       gsfLoadScaleFactor(gsfScaleFactors *sf, unsigned int subrecordID, char c_flag, double precision, int offset);
/*
 * Description : gsfLoadScaleFactors should be used to load the swath
 *  bathymetry ping record scale factor structure.  This function assures
 *  that the multiplier and offset fields of the scale factor structure
 *  have a precision equal to that which will be stored in the gsf data file.
 *  This function should be called once for each beam array data type
 *  contained in your data.
 *
 * Inputs :
 *  sf = a pointer to the gsfScaleFactors structure to be loaded
 *  subrecordID = the subrecord id for the beam array data
 *  c_flag = the compression flag for the beam array
 *  precision = the presision to which the beam array data are to be stored
 *              (a value of 0.1 would indicate decimeter precision for depth)
 *  offset = the "DC" offset to scale the data by.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 */

int OPTLK gsfGetScaleFactor(int handle, unsigned int subrecordID, unsigned char *c_flag, double *multiplier, double *offset);
/*
 * Description : gsfGetScaleFactor may be used to obtain the multiplier
 *  and DC offset values by which each swath bathymetry ping array subrecord
 *  is be scaled. gsfGetScalesFactor must be called once for each array
 *  subrecord of interest.  At leat one swath bathymetry ping record
 *  must have been read from, or written to the file specified by handle.
 *
 * Inputs :
 *  handle = the integer value set by a call to gsfOpen.
 *  subrecordID = an integer value containing the subrecord id of the requested scale factors
 *  c_flag = the address of an unsigned character to contain the the compression flag
 *  multiplier = the address of a double to contain the scaling multiplier
 *  offset = the address of a double to contain the scaling DC offset.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 */

void OPTLK      gsfFree(gsfRecords *rec);
/*
 * Description : This function frees all dynamically allocated memory
 *    from a gsfRecords data structure, and it then clears all the
 *    data elements in the structure.
 *
 * Inputs :
 *    gsfRecords *rec = a pointer to ta gsfRecords data structure
 *
 * Returns : none
 *
 * Error Conditions : none
 */

void OPTLK      gsfPrintError(FILE * fp);
/* Description : This function is used to print a short message describing
 *  the most recent error encountered.  This function need only be called if
 *  a -1 is returned from one of the gsf functions.
 *
 * Inputs :
 *  fp = a pointer to a FILE to which to write the message.
 *
 * Returns : none
 *
 * Error Conditions : none
 */

int gsfIntError(void);
/* Description : This function is used to return the most recent
 *  error encountered.  This function need only be called if
 *  a -1 is returned from one of the gsf functions.
 *
 * Inputs : none
 *
 * Returns : constant integer value representing the most recent error
 *
 * Error Conditions : none
 */

const char *gsfStringError(void);
/* Description : This function is used to return a short message describing
 *  the most recent error encountered.  This function need only be called if
 *  a -1 is returned from one of the gsf functions.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 */

int OPTLK       gsfIndexTime(int, int, int, time_t *, long *);
/* Description : This function returns the time (Posix.4) associated with
 *  a specified record number and type.  It also returns the record number
 *  that was read.
 *
 * Inputs :
 *  handle = gsf file handle assigned by gsfOpen or gsfOpenBuffered
 *  record_type = record type to be retrieved
 *  record_number = record number to be retrieved (-1 will get the time
 *                  and record number of the last record of this type)
 *  sec = Posix.4 seconds
 *  nsec = Posix.4 nanoseconds
 *
 * Returns :
 *  This function returns the record number if successful, or -1 if an
 *  error occured.
 *
 * Error Conditions :
 *    GSF_RECORD_TYPE_NOT_AVAILABLE
 */

int OPTLK       gsfPercent (int handle);
/* Description : This function returns an integer value representing
 *  the location of the file pointer as a percentage of the total file
 *  size.  It may be used to obtain an indication of how far along a
 *  program is in reading a gsf data file.  The file size is obtained
 *  when the file is opened.
 *
 * Inputs :
 *  handle = gsf file handle assigned by gsfOpen or gsfOpenBuffered
 *
 * Returns :
 *  This function returns the current file position as a percentage of
 *  the file size, or -1 if an error occurred. gsfError will be set to
 *  indicate the error.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_TELL_ERROR
 */

int OPTLK       gsfGetNumberRecords (int handle, int desiredRecord);
/* Description : This function will return the number of records of a
 *  given type to the caller. The number of records is retreived from
 *  the index file, so the file must have been opened for direct
 *  access (GSF_READONLY_INDEX, or GSF_UPDATE_INDEX).
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *
 * Returns :
 *  This function returns the number of records of type desiredRecord
 *  contained in the GSF file designated by handle, or -1 if an error
 *  occured.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_BAD_ACCESS_MODE
 */

int OPTLK       gsfCopyRecords (gsfRecords *target, const gsfRecords *source);
/* Description : This function will copy all of the data contained in the
 *  source gsfRecords data structure to the target gsfRecords data
 *  structure. The target MUST be memset to zero before the first call to
 *  gsfCopyRecords.  This function allocates dynmanic memory which is NOT
 *  maintained by the library.  It is up to the calling application to
 *  release the memory allocated.  This may be done by maintaining the
 *  target data structure as static data, or by using gsfFree to release
 *  the memory.
 *
 * Inputs :
 *  target = a pointer to a gsfRecords data structure allocated by the
 *      calling application, into which the source data is to be copied.
 *  source = a pointer to a gsfRecords data structure allocated by the
 *      calling application, from which data is to be copied.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  GSF_MEMORY_ALLOCATION_FAILED
 */

int OPTLK       gsfPutMBParams(const gsfMBParams *p, gsfRecords *rec, int handle, int numArrays);
/* Description : This function moves swath bathymetry sonar processing
 *    parameters from internal form to "KEYWORD=VALUE" form.  The internal
 *    form parameters are read from an MB_PARAMETERS data structure maintained
 *    by the caller.  The "KEYWORD=VALUE" form parameters are written into the
 *    processing_parameters structure of the gsfRecords data structure
 *    maitained by the caller. Parameters for up to two transmitters and two receivers
 *    are supported.  If the user sets the number_of_transmitters and number_of_receivers
 *    elements in the gsfMBParams data structure in addition to the numArrays command line
 *    argument, the numArrays value will be ignored.  If number_of_transmitters and
 *    number_of_receivers are equal to 0, then numArrays will be used to populate both
 *    these values in the GSF processing parameters record.
 *
 * Inputs :
 *     p = a pointer to the gsfMBParams data structure which contains
 *         the parameters in internal form.
 *     rec = a pointer to the gsfRecords data structure into which the
 *         parameters are to be written in the "KEYWORK=VALUE" form.
 *     handle = the integer handle to the file set by gsfOpen.
 *     numArrays = the integer value specifying the number of pairs of
 *         arrays which need to have seperate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *     GSF_MEMORY_ALLOCATION_FAILED
 *     GSF_PARAM_SIZE_FIXED
 */

int OPTLK       gsfGetMBParams(const gsfRecords *rec, gsfMBParams *p, int *numArrays);
/* Description : This function moves swath bathymetry sonar processing
 *    parameters from external, form to internal form.  The external
 *    "KEYWORD=VALUE" format parameters are read from a processing_params
 *    structure of a gsfRecords data structure maintained by the caller.
 *    The internal form parameters are written into a gsfMBParams data
 *    structure maintained by the caller. Parameters for up to two transmitters
 *    and two receivers are supported.  The number_of_transmitters and
 *    number_of_receivers elements of the gsfMBParams data structure are set by
 *    determining the number of fields in the parameters for the transmitter(s)
 *    and receiver(s), respectively.  The numArrays argument is set from the
 *    number of fields for the transmitter(s). Any parameter not described in a
 *    "KEYWORD=VALUE" format will be set to "GSF_UNKNOWN_PARAM_VALUE".
 *
 * Inputs :
 *     rec = a pointer to the gsfRecords data structure from which the
 *         parameters in "KEYWORK=VALUE" form are to be read.
 *     p = a pointer to the gsfMBParams data structure which will be populated.
 *     numArrays = the integer value specifying the number of pairs of
 *         arrays which need to have seperate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  none.
 */

int OPTLK       gsfGetSwathBathyBeamWidths(const gsfRecords *data, double *fore_aft, double *athwartship);
/* Description : This function returns to the caller the fore-aft and
 *    the port-starboard beam widths in degrees for a swath bathymetry
 *    multibeam sonar, given a gsfRecords data structure which contains
 *    a populated gsfSwathBathyPing structure.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *     fore_aft = The address of a double allocated by the caller which will
 *         be loaded with the sonar's fore/aft beam width in degrees. A value of
 *         GSF_BEAM_WIDTH_UNKNOWN is used when the beam width is not known.
 *     athwartship = The address of a double allocated by the caller which will
 *         be loaded with the sonar's athwartship beam width in degrees.  A
 *         value of GSF_BEAM_WIDTH_UNKNOWN is used when the beam width is not known.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *     occured.
 *
 * Error Conditions : unrecognized sonar id or mode.
 */

int OPTLK gsfIsStarboardPing(const gsfRecords *data);
/* Description : This function uses the sonar specific data union
 *     of a gsfSwathBathymetry ping structure to determine if the ping
 *     is from the starboard arrays of a multibeam installation with
 *     dual transmit receive sonar arrays.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *
 * Returns : This function returns non-zero if the ping contained in the
 *     passed data represents a starboard looking ping from a dual headed
 *     sonar installation. Otherwise, zero is returned.
 *
 * Error Conditions : unrecognized sonar id or mode.
 */

int OPTLK gsfLoadDepthScaleFactorAutoOffset(gsfSwathBathyPing *ping, unsigned int subrecordID, int reset, double min_depth, double max_depth, double *last_corrector, char c_flag, double precision);
/* Description : gsfLoadDepthScaleFactorAutoOffset should be used to load
 *  the scale factors for the depth subrecords of the swath bathymetry ping
 *  record scale factor structure. The approach uses the tide and depth
 *  correction fields to help establish the offset component of the scale
 *  factor such that negative depth values may be supported.  Negative
 *  depth values may be encountered when surveying above the tidal datum.
 *  In addition, systems mounted on subsea platforms may support their
 *  native precision even in deep water.
 *
 * Inputs :
 *  ping = A pointer to the gsfSwathBathyPing which contains the depht
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = the subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  c_flag = The compression flag for the beam array
 *  precision = The presision to which the beam array data are to be stored
 *      (a value of 0.1 would indicate decimeter precision for depth)
 *  reset = An integer value which will cause the internal logic to be
 *      refreshed when the value is non-zero.  The first call to this function
 *      should use a non-zero reset, from then on, this value may be passed
 *      as zero.
 *  last_corrector = The address of a double value stored as permanent memory.
 *      Successive calls to this function must pass the same address for this
 *      argument.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 *
 */

int OPTLK gsfGetSwathBathyArrayMinMax(const gsfSwathBathyPing *ping, unsigned int subrecordID, double *min_value, double *max_value);
/* Description : This function may be used to obtain the minimum and maximum
 *  supportable values for each of the swath bathymetry arrays.  The minimum
 *  and maximum values are determined based on the scale factors and the array
 *  type.
 *
 * Inputs :
 *  ping = A pointer to the gsfSwathBathyPing which contains the depht
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = The subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  min_value = The address of a double value allocated by the caller into
 *      which will be placed the minimum value which may be represented for
 *      this array type.
 *  max_value = The address of a double value allocated by the caller into
 *      which will be placed the maximum value which may be represented for
 *      this array type.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 */

const char *gsfGetSonarTextName(const gsfSwathBathyPing *ping);
/* Description : This function is used to return the text of the sonar name.
 *
 * Inputs : The GSF ping
 *
 * Returns : A text string of the sensors name, or "Unknown" if the sensor id is not found
 *
 * Error Conditions : none
 */

int gsfFileSupportsRecalculateXYZ(int handle, int *status);
/* Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a full recalculation
 *  of the platform relative XYZ values from raw measurements. This function
 *  rewinds the file to the first record and reads through the file looking for
 *  the information required to support a recalculation. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the
 *            function result is placed. *status is assigned a value of 1
 *            if this file provides sufficient information to support full
 *            recalculation of the platform relative XYZ values, otherwise
 *            *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions : none
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int gsfFileSupportsRecalculateTPU(int handle, int *status);
/* Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a recalculation
 *  of the total propagated uncertainty (TPU) estimates. This function
 *  rewinds the file to the first record and reads through the file looking for
 *  the information required to support TPU estimation. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the
 *           function result is placed. *status is assigned a value of 1
 *           if this file provides sufficient information to support TPU
 *           estimation, otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int gsfFileSupportsRecalculateNominalDepth(int handle, int *status);
/* Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a recalculation
 *  of the nominal depth array. This function rewinds the file to the first
 *  record and reads through the file looking for the information required
 *  to support calculation of the nominal depth values. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the
 *           function result is placed. *status is assigned a value of 1
 *           if this file provides sufficient information to support
 *           nominal depth calculation, otherwise *status is assigned
 *           a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int gsfFileContainsMBAmplitude(int handle, int *status);
/* Function Name : gsfFileContainsMBAmplitude
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains the average per receive beam amplitude data.
 *  This function rewinds the file to the first record and reads through
 *  the file up to and including the first ping record. If amplitude data
 *  are contained in the first ping record it is assumed that amplitude
 *  data are contained with all ping records in this file. On success,
 *  the file pointer is reset to the beginning of the file before the
 *  function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the
 *           function result is placed. *status is assigned a value of 1
 *           if this file contains the per receive beam amplitude data,
 *           otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int gsfFileContainsMBImagery(int handle, int *status);
/* Function Name : gsfFileContainsMBImagery
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains the per receive beam imagery time series data.
 *  This function rewinds the file to the first record and reads through
 *  the file up to and including the first ping record. If MB imagery data
 *  are contained in the first ping record it is assumed that MB imagery
 *  data are contained with all ping records in this file. On success,
 *  the file pointer is reset to the beginning of the file before the
 *  function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the
 *           function result is placed. *status is assigned a value of 1
 *           if this file contains the per receive beam imagery time
 *           series data, otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int gsfIsNewSurveyLine(int handle, const gsfRecords *rec, double azimuth_change, double *last_heading);
/* Function Name : gsfIsNewSurveyLine
 *
 * Description : This function provides an approach for calling applications
 *  to determine if the last ping read from a GSF file is from the same survey
 *  transect line, or if the last ping is from a newly started survey line. The
 *  implementation looks for a change in platform heading to determine that the
 *  last ping read is from a new survey line. External to this function, calling
 *  applications can decide on their own if the first ping read from a newly opened
 *  GSF file should be considered to be from a new survey transect line or not.
 *  This function assumes that the GSF file is read in chronological order from
 *  the beginning of the file, file access can be either direct or sequential
 *
 * Inputs :
 *  handle         = The handle to the file as provided by gsfOpen
 *  rec            = A pointer to a gsfRecords structure containing the data from the most
 *                    recent call to gsfRead
 *  azimuth_change = The trigger value specifying the change in platform heading that
 *                    must be exceeded for a new survey transect line to be determined
 *  last_heading   = A pointer to a double allocated by the caller and into which this
 *                    function will place the heading value for each detected line. The
 *                    value must be allocated as permanent memory that persists through
 *                    all calls to this function. Startup or reset events can be handled
 *                    by the caller by placing a negative value in this memory location.
 *
 * Returns :
 *  This function returns 1 if this ping is considered to be the first ping of a new
 *   survey transect line, otherwise, 0 is returned.
 *
 * Error Conditions :
 *  none
 */

void gsfInitializeMBParams (gsfMBParams *p);
/********************************************************************
 *
 * Function Name : gsfInitializeMBParams
 *
 * Description : This function provides a way to initialize all the
 *    sonar processing parameters to "unknown"
 *
 * Inputs :
 *    p = a pointer to the gsfMBParams data structure that needs initializing
 *
 * Returns :
 *    None
 *
 * Error Conditions :
 *    None
 *
 ********************************************************************/


int gsfStat (const char *filename, long long *sz);
/********************************************************************
 *
 * Function Name : gsfStat
 *
 * Description : This function attempts to stat a GSF file.
 *               Supports 64 bit file size.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file
 *  sz       = pointer to an 8 byte long long for return
 *             of the GSF file size from the stat64 system call.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_FOPEN_ERROR
 *     GSF_UNRECOGNIZED_FILE
 *
 ********************************************************************/

int gsfSetDefaultScaleFactor(gsfSwathBathyPing *mb_ping);
/********************************************************************
 *
 * Function Name : gsfSetDefaultScaleFactor
 *
 * Description : This function is used to estimate and set scale
 *               factors for a ping record
 *
 * Inputs :
 *    mb_ping - a pointer to a ping record.  The scale factors
 *              will be set in this record.
 *
 * Returns : This function returns 0.
 *
 * Error Conditions : none
 *
 ********************************************************************/

/********************************************************************
 *
 * Function Name : gsfGetPositionDestination
 *
 * Description : compute a new position from an existing one.
 *
 * Inputs : ref pos, offsets from ref (+x forward, +y starboard, + z down), ref heading (+hdg cw from north), maximum distance step.
 *
 * Returns : new position
 *
 * Error Conditions :
 *
 ********************************************************************/

GSF_POSITION *gsfGetPositionDestination(GSF_POSITION gp, GSF_POSITION_OFFSETS offsets, double hdg, double dist_step);

/********************************************************************
 *
 * Function Name : gsfGetPositionOffsets
 *
 * Description : compute offsets between two positions.
 *
 * Inputs : ref pos, new pos, ref heading (+hdg cw from north), maximum distance_step.
 *
 * Returns : offsets from ref (+x forward, +y starboard, + z down)
 *
 * Error Conditions :
 *
 ********************************************************************/

GSF_POSITION_OFFSETS *gsfGetPositionOffsets(GSF_POSITION gp_from, GSF_POSITION gp_to, double hdg, double dist_step);

#ifdef __cplusplus
}
#endif

#endif /* __GSF_H__ */
