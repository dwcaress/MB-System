/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_3ddwissl.c  3.00  4/29/2025
 *
 *    Copyright (c) 2017-2025 by
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
 * mbsys_3ddwissl.h defines the MBIO data structures for handling data from
 * the 3DatDepth WiSSL (wide swath lidar) submarine lidar:
 *      MBF_3DWISSLR : MBIO ID 232 - 3DatDepth WiSSL vendor format
 *      MBF_3DWISSLP : MBIO ID 233 - 3DatDepth WiSSL processing format
 *
 * Author:  David W. Caress
 * Date:  December 19, 2017
 * Date:  July 25, 2019 (updated)
 *
 */
/*
 * Notes on the MBSYS_3DATDEPTHWISSL data structure:
 *
 * Vendor format from 3D at Depth for the WiSSL (wide swath lidar) submarine
 * lidar system delivered to MBARI in December 2017.
 *
 * Initial coding done using the WiSSL Wide Swath Subsea LiDAR Software User
 * Manual version 1.2 from December 2017.
 * Modifications made using the WiSSL Wide Swath Subsea LiDAR Software User
 * Manual version 1.4 from May 2019.
 *
 * Two data formats are supported: all variations of the *.raa files logged by
 * the WiSSL are supported as MB-System format 232 (MBF_3DWISSLR), and the
 * processed WiSSL data written by MB-System are supported as format 233 (MBF_3DWISSLP).
 * The logged data have variants 1.1 and 1.2, with the newer variant changing
 * the calibration block size from 450 bytes to 407 bytes, and adding a 1-byte
 * value named diagnostic to each sounding. MB-System has defined an additional
 * variant 1.3 in which the calibration block of 1.2 is lengthened to 450 bytes
 * so that the file header size is the same as 1.1. When a logged *.raa file
 * in the 1.2 variant is read, the data are converted to 1.3. MB-System only
 * writes newer data in the 1.3 variant. Data in the original 1.1 form remain
 * in that form.
 *
 *--------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 * 3D at Depth raw WiSSL data format
 * Supported as format 232 in MB-System
 *              Item                                  Value              Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record (923 bytes for 1.1 and 1.3, 837 bytes for 1.2):
 * ------------------
 *
 * File and version ID (8 bytes)
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D08   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1 to 3   2 (1 UINT16)
 *
 * Scan Information (15 bytes)
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Np, Number pulses per cross track scan line            2 (1 UINT16)
 *           Nl, Number LOS (soundings) per pulse                   1 (1 UINT8)
 *           Na, Scan lines per this File, Head A                   2 (1 UINT16)
 *           Nb, Scan lines per this File, Head B                   2 (1 UINT16)
 *
 * Calibration Information (900 bytes for 1.1 & 1.3, 814 bytes for 1.2)
 *           Calibration Structure, Head A                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *           Calibration Structure, Head B                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *
 * ---------------------------------------------------------------------------------------
 * Scan Record (1 to N = Na + Nb scans):
 * -----------
 *
 * Scan Header (49 bytes)
 *           Record ID – Head A or B              0x3D53, 0x3D54    2 (1 UINT16)
 *           Timestamp (year (true year))                           2 (1 UINT16)
 *           Timestamp (month (1-12))                               1 (1 UINT8)
 *           Timestamp (day)                                        1 (1 UINT8)
 *           Timestamp (days since Jan 1)                           2 (1 UINT16)
 *           Timestamp (hours)                                      2 (1 UINT16)
 *           Timestamp (minutes)                                    1 (1 UINT8)
 *           Timestamp (seconds)                                    1 (1 UINT8)
 *           Timestamp (nano seconds)                               4 (1 UINT32)
 *           Gain (laser power)                                     1 (1 UINT8)
 *           Digitizer temperature (degrees C)                      4 (1 float32)
 *           CTD temperature (degrees C)                            4 (1 float32)
 *           CTD salinity (psu)                                     4 (1 float32)
 *           CTD pressure (dbar)                                    4 (1 float32)
 *           Index of refraction                                    4 (1 float32)
 *           Start processing range (meters)                        4 (1 float32)
 *           End processing range  (meters)                         4 (1 float32)
 *           Np, Number of pulses in this scan                      4 (1 UINT32)
 *
 * Laser Pulse Data ( 1 to Np pulses per scan, each pulse has Nl soundings )
 *   Pulse Header (20 bytes)
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *   Values for LOS (6 bytes per sounding for 1.1, 7 bytes per sounding >= 1.2)
 *           Sounding 1 Range ( from glass front ) meters           4 (1 float32)
 *           ...
 *           Sounding Nl Range ( from glass front ) meters          4 (1 float32)
 *           Sounding 1 Amplitude / peak of signal                  2 (1 UINT16)
 *           ...
 *           Sounding Nl Amplitude / peak of signal                 2 (1 UINT16)
 *           Sounding 1 Diagnostic value (if version >= 1.2):       1 (1 UINT8)
 *           ...
 *           Sounding Nl Diagnostic value (if version >= 1.2):      1 (1 UINT8)
 *
 * Each RAA file begins with a File Header record that includes a “Scan Information”
 * block and two “Calibration Information” blocks, one for each optical head.
 * There may be an arbitrary number of comment records following the File Header,
 * but before the scan data. The scan line data interleaves the Head A and Head B
 * scans with no guarentee that the order is consistent with increasing time stamp.
 * Also, differing scan counts will generally exist for Head A and B.
 * The data for each scan line contains: a Record ID (head designator), a full
 * timestamp, and a” Laser Pulse Data” collection of data.
 * The file suffix of files logged by the WiSSL will be *.raa. Files written by
 * MB-System will typically have a ".mb232" suffix. Either suffix is valid.
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, the following data would be present in
 * the RAA file:
 *      File Header Record
 *        Scan Information
 *        Calibration Information Head A (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *        Calibration Information Head B (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *      Comment Record 0
 *      ......
 *      Comment Record Nc - 1
 *      Scan Record 1 (A or B)
 *          Scan Timestamp and characteristics
 *          Pulse count this scan line
 *              (1) Laser Pulse Data:
 *                  AZ angle
 *                  EL angle
 *                  AZ offset
 *                  EL offset
 *                  Pulse time offset
 *                  Range Data:
 *                      Sounding 1 Range
 *                      Sounding 2 Range
 *                      Sounding 3 Range
 *                  Intensity Data:
 *                      Sounding 1 Intensity
 *                      Sounding 2 Intensity
 *                      Sounding 3 Intensity
 *                  Diagnostic Data (if version >= 1.2):
 *                      Sounding 1 Diagnostic
 *                      Sounding 2 Diagnostic
 *                      Sounding 3 Diagnostic
 *                  ...
 *              (250) Laser Pulse Data:
 *                  AZ angle
 *                  EL angle
 *                  AZ offset
 *                  EL offset
 *                  Pulse time offset
 *                  Range Data:
 *                      Sounding 1 Range
 *                      Sounding 2 Range
 *                      Sounding 3 Range
 *                  Intensity Data:
 *                      Sounding 1 Intensity
 *                      Sounding 2 Intensity
 *                      Sounding 3 Intensity
 *                  Diagnostic Data (if version >= 1.2):
 *                      Sounding 1 Diagnostic
 *                      Sounding 2 Diagnostic
 *                      Sounding 3 Diagnostic
 *
 * Note: based on laser head performance, differing counts of data sets may
 * exist for Head A and B. The “.raa” file extension is used for the binary file.
 *
 * ---------------------------------------------------------------------------------------
 * Comment Record:
 * --------------
 *           Record ID – Comment                          0x3D43    2 (1 UINT16)
 *           Comment length (Nc bytes)                              2 (1 UINT16)
 *           Comment (null terminated C string)                    Nc (Nc CHAR)
 *--------------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------------
 * Processing WiSSL Data Format
 * ----------------------------
 * Supported as format 233 in MB-System
 * The file header and survey data records differ from those of the raw format
 * in several respects:
 *   1) The file magic number is 0x3D09
 *   2) The file header is 923 bytes long, always.
 *   3) The scan record id's are 0x3D73, 0x3D74 rather than 0x3D53, 0x3D54
 *   4) The size in bytes of the scan record, minus 4, is stored as an unsigned
 *      short int immediately following the record id.
 *   5) Only non-null soundings as defined by preprocessing are stored - many
 *      low amplitude picks may be discarded.
 *   7) The valid pulse headers are stored in a sequential list.
 *   8) The valid soundings are stored in a separate sequential list following pulses.
 *   9) The soundings include calculated bathymetry values and the pulse and LOS
 *      pick numbers.
 *--------------------------------------------------------------------------------
 *              Item                                  Value              Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record (923 bytes):
 * ------------------
 *
 * File and version ID (8 bytes)
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D09   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1 to 3   2 (1 UINT16)
 *
 * Scan Information (15 bytes)
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Np, Number pulses per cross track scan line            2 (1 UINT16)
 *           Nl, Number LOS (soundings) per pulse                   1 (1 UINT8)
 *           Na, Scan lines per this File, Head A                   2 (1 UINT16)
 *           Nb, Scan lines per this File, Head B                   2 (1 UINT16)
 *
 * Calibration Information (900 bytes)
 *           Calibration Structure, Head A                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *           Calibration Structure, Head B                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *
 * ---------------------------------------------------------------------------------------
 * Scan Record (0 to N-1 where N = Na + Nb scans):
 * -----------
 *
 * Scan Header (100 bytes)
 *           Record ID – Head A or B              0x3D73, 0x3D74    2 (1 UINT16)
 *           Timestamp (year (true year))                           2 (1 UINT16)
 *           Timestamp (month (1-12))                               1 (1 UINT8)
 *           Timestamp (day)                                        1 (1 UINT8)
 *           Timestamp (days since Jan 1)                           2 (1 UINT16)
 *           Timestamp (hours)                                      2 (1 UINT16)
 *           Timestamp (minutes)                                    1 (1 UINT8)
 *           Timestamp (seconds)                                    1 (1 UINT8)
 *           Timestamp (nano seconds)                               4 (1 UINT32)
 *           Gain (laser power)                                     1 (1 UINT8)
 *           Unused                                                 1 (1 UINT8)
 *           Digitizer temperature (degrees C)                      4 (1 float32)
 *           CTD temperature (degrees C)                            4 (1 float32)
 *           CTD salinity (psu)                                     4 (1 float32)
 *           CTD pressure (dbar)                                    4 (1 float32)
 *           Index of refraction                                    4 (1 float32)
 *           Start processing range (meters)                        4 (1 float32)
 *           End processing range  (meters)                         4 (1 float32)
 *           Np, Number of pulses in this scan                      4 (1 UINT32)
 *           Epoch time (seconds since 1970)                        8 (1 double64)
 *           Scan longitude (degrees)                               8 (1 double64)
 *           Scan latitude (degrees)                                8 (1 double64)
 *           Scan sensordepth (meters)                              8 (1 double64)
 *           Scan speed (m/sec)                                     4 (1 float32)
 *           Scan heading (degrees)                                 4 (1 float32)
 *           Scan roll (degrees)                                    4 (1 float32)
 *           Scan pitch (degrees)                                   4 (1 float32)
 *           Nv, Number of valid pulses                             2 (1 UINT16)
 *           Ns, Number of valid soundings                          2 (1 UINT16)
 *
 * Pulse Data ( 0 to Nv-1 pulses in this scan, 66 bytes per pulse, each valid pulse has at least 1 valid sounding )
 *           Ip, pulse id in scan, counted from 0                   2 (1 UINT16)
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *           Epoch time (seconds since 1970)                        8 (1 double64)
 *           Navigation Acrosstrack offset (meters)                 8 (1 double64)
 *           Navigation Alongtrack offset (meters)                  8 (1 double64)
 *           Navigation Sensordepth offset (meters)                 8 (1 double64)
 *           Heading offset (degrees)                               4 (1 float32)
 *           Roll offset (degrees)                                  4 (1 float32)
 *           Pitch offset (degrees)                                 4 (1 float32)
 *
 * Sounding Data ( 0 to Ns-1 soundings in this scan, 22 bytes per sounding for 1.1, 23 bytes per sounding >= 1.2)
 *           Ip, pulse id in scan, counted from 0                   2 (1 UINT16)
 *           Is, LOS id in pulse, counted from 0                    1 (1 UINT8)
 *           Range ( from glass front ) meters                      4 (1 float32)
 *           Amplitude / peak of signal                             2 (1 UINT16)
 *           Diagnostic value (if version >= 1.2):                  1 (1 UINT8)
 *           MB-System beamflag                                     1 (1 UINT8)
 *           Acrosstrack (meters)                                   4 (1 float32)
 *           Alongtrack (meters)                                    4 (1 float32)
 *           Sensordepth (meters)                                   4 (1 float32)
 *
 * Each MB-System format 233 file begins with a File Header record that includes
 * a “Scan Information” block and two “Calibration Information” blocks, one for each
 * optical head. There may be an arbitrary number of comment records following the
 * File Header, but before the scan data. The scan line data interleaves the
 * Head A and Head B scans with no guarentee that the order is consistent with
 * increasing time stamp. The data for each scan line contains: a Record ID
 * (head designator), timestamp and navigation, a list of valid pulses (e.g.
 * pulses with at least one valid sounding), and a list of valid soundings.
 * The file suffix will typically be ".mb232".
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, and 245 pulses had valid soundings resulting
 * in a list 247 valid soundings, the following data would be present in
 * the format 233 file:
 *      File Header Record
 *        Scan Information
 *        Calibration Information Head A (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *        Calibration Information Head B (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *      Comment Record 0
 *      ......
 *      Comment Record Nc - 1
 *      Scan Record 1 (A or B)
 *        Scan header
 *        Pulse 0
 *        ....
 *        Pulse Nv - 1
 *        Sounding 0
 *        ....
 *        Sounding Ns - 1
 *
 * ---------------------------------------------------------------------------------------
 * Comment Record:
 * --------------
 *           Record ID – Comment                          0x3D43    2 (1 UINT16)
 *           Comment length (Nc bytes)                              2 (1 UINT16)
 *           Comment (null terminated C string)                    Nc (Nc CHAR)
 *--------------------------------------------------------------------------------
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_3ddwissl2.h"

// #define MBF_3DDEPTHP_DEBUG 1

/*-------------------------------------------------------------------- */
int mbsys_3ddwissl2_alloc
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,              /* in: see mb_io.h:/^struct mb_io_struct/ */
  void **store_ptr,              /* in: see mbsys_3ddwissl.h:/^struct
                           mbsys_3ddwissl2_struct/ */
  int *error                /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  int status = mb_mallocd(verbose,
    __FILE__,
    __LINE__,
    sizeof(struct mbsys_3ddwissl2_struct),
    (void **)store_ptr,
    error);
  /*mb_io_ptr->structure_size = 0; */

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)*store_ptr;

  /* initialize everything */
  memset(*store_ptr, 0, sizeof(struct mbsys_3ddwissl2_struct));

  /* Type of data record */
  store->kind = MB_DATA_NONE;  /* MB-System record ID */


  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_alloc */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_deall
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,              /* in: see mb_io.h:/^struct mb_io_struct/ */
  void **store_ptr,              /* in: see mbsys_3ddwissl.h:/^struct
                           mbsys_3ddwissl2_struct/ */
  int *error                /* out: see mb_status.h:/error values/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
    }

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)*store_ptr;

  int status = MB_SUCCESS;
  
	struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	if (mbarirange->soundings != NULL && mbarirange->num_soundings_alloc > 0) 
		{
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mbarirange->soundings), error);
		mbarirange->num_soundings_alloc = 0;
		}

  /* deallocate memory for data structure */
  status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_deall */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_dimensions
(
  int verbose,
  void *mbio_ptr,                      /* in: verbosity level set on command
                                 line 0..N */
  void *store_ptr,                /* in: see mbsys_3ddwissl.h:/^struct
                             mbsys_3ddwissl2_struct/ */
  int *kind,                    /* in: see mb_status.h:0+/MBIO data type/ */
  int *nbath,                    /* out: number of bathymetric samples
                             0..MBSYS_SWPLS_MAX_BEAMS */
  int *namp,                    /* out: number of amplitude samples
                             0..MBSYS_SWPLS_MAX_BEAMS */
  int *nss,                    /* out: number of sidescan samples
                             0..MBSYS_SWPLS_MAX_BEAMS */
  int *error                    /* out: see mb_status.h:/error values/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract beam and pixel numbers from structure */
  if (*kind == MB_DATA_DATA)
    {
    *nbath = store->mbarirange.num_soundings;
    *namp = *nbath;
    *nss = 0;
    }
  else
    {
    *nbath = 0;
    *namp = 0;
    *nss = 0;
    }

  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
    fprintf(stderr, "dbg2        namp:      %d\n", *namp);
    fprintf(stderr, "dbg2        nss:       %d\n", *nss);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_dimensions */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_pingnumber
(
  int verbose,                    /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                 /* in: see mb_io.h:/^struct mb_io_struct/ */
  unsigned int *pingnumber,       /* out: ping number */
  int *error                      /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)mb_io_ptr->store_data;

  /* extract ping number from structure */
  if (store->kind == MB_DATA_DATA)
    {
    *pingnumber = store->mbarirange.RowNumber;
    }
  else
    {
    *pingnumber = 0;
    }

  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_pingnumber */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_preprocess
(
  int verbose,                /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,             /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,            /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl2_struct/ */
  void *platform_ptr,
  void *preprocess_pars_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
    }

  *error = MB_ERROR_NO_ERROR;

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(preprocess_pars_ptr != NULL);

  /* get mbio descriptor */
  //struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get preprocessing parameters */
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

  /* get data structure pointers */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;
  //struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

  /* get kluges */
  bool kluge_beampatternsnell = false;
  double kluge_beampatternsnellfactor = 1.0;
  for (int i = 0; i < pars->n_kluge; i++) {
    if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
      kluge_beampatternsnell = true;
      kluge_beampatternsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       target_sensor:              %d\n", pars->target_sensor);
    fprintf(stderr, "dbg2       timestamp_changed:          %d\n", pars->timestamp_changed);
    // fprintf(stderr, "dbg2       time_d:                     %f\n", pars->time_d);
    fprintf(stderr, "dbg2       n_nav:                      %d\n", pars->n_nav);
    fprintf(stderr, "dbg2       nav_time_d:                 %p\n", pars->nav_time_d);
    fprintf(stderr, "dbg2       nav_lon:                    %p\n", pars->nav_lon);
    fprintf(stderr, "dbg2       nav_lat:                    %p\n", pars->nav_lat);
    fprintf(stderr, "dbg2       nav_speed:                  %p\n", pars->nav_speed);
    fprintf(stderr, "dbg2       n_sensordepth:              %d\n", pars->n_sensordepth);
    fprintf(stderr, "dbg2       sensordepth_time_d:         %p\n", pars->sensordepth_time_d);
    fprintf(stderr, "dbg2       sensordepth_sensordepth:    %p\n",
      pars->sensordepth_sensordepth);
    fprintf(stderr, "dbg2       n_heading:                  %d\n", pars->n_heading);
    fprintf(stderr, "dbg2       heading_time_d:             %p\n", pars->heading_time_d);
    fprintf(stderr, "dbg2       heading_heading:            %p\n", pars->heading_heading);
    fprintf(stderr, "dbg2       n_altitude:                 %d\n", pars->n_altitude);
    fprintf(stderr, "dbg2       altitude_time_d:            %p\n", pars->altitude_time_d);
    fprintf(stderr, "dbg2       altitude_altitude:          %p\n", pars->altitude_altitude);
    fprintf(stderr, "dbg2       n_attitude:                 %d\n", pars->n_attitude);
    fprintf(stderr, "dbg2       attitude_time_d:            %p\n", pars->attitude_time_d);
    fprintf(stderr, "dbg2       attitude_roll:              %p\n", pars->attitude_roll);
    fprintf(stderr, "dbg2       attitude_pitch:             %p\n", pars->attitude_pitch);
    fprintf(stderr, "dbg2       attitude_heave:             %p\n", pars->attitude_heave);
    fprintf(stderr, "dbg2       n_kluge:                    %d\n", pars->n_kluge);
    for (int i = 0; i < pars->n_kluge; i++) {
      fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
      if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
        fprintf(stderr, "dbg2       kluge_beampatternsnell:        %d\n", kluge_beampatternsnell);
        fprintf(stderr, "dbg2       kluge_beampatternsnellfactor:  %f\n", kluge_beampatternsnellfactor);
      }
    }
  }


  /* always successful */
  int status = MB_SUCCESS;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

	/* deal with a survey record */
	else if (store->kind == MB_DATA_DATA) {
	
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* change timestamp if indicated */
    if (pars->timestamp_changed)
      {
      mbarirange->time_d = pars->time_d;
      mbarirange->TimeStart_Sec = (int) floor(pars->time_d);
   		mbarirange->TimeStart_nSec = (int)(1.0e9 * (pars->time_d - mbarirange->TimeStart_Sec));
      }

    /* interpolate navigation and attitude */

    /* get nav sensordepth heading attitude values for scan timestamp
       - this will generally conform to the first pulse of the scan */
    // int interp_status = MB_SUCCESS;
    int interp_error = MB_ERROR_NO_ERROR;
		int jnav = 0;
		int jsensordepth = 0;
		int jheading = 0;
		int jattitude = 0;
    if (pars->n_nav > 0)
      {
      /* interp_status = */ mb_linear_interp_longitude(verbose,
        pars->nav_time_d - 1,
        pars->nav_lon - 1,
        pars->n_nav,
        mbarirange->time_d,
        &mbarirange->navlon,
        &jnav,
        &interp_error);
      /* interp_status = */ mb_linear_interp_latitude(verbose,
        pars->nav_time_d - 1,
        pars->nav_lat - 1,
        pars->n_nav,
        mbarirange->time_d,
        &mbarirange->navlat,
        &jnav,
        &interp_error);
      /* interp_status = */ mb_linear_interp(verbose,
        pars->nav_time_d - 1,
        pars->nav_speed - 1,
        pars->n_nav,
        mbarirange->time_d,
        &mbarirange->speed,
        &jnav,
        &interp_error);
      }
    if (pars->n_sensordepth > 0)
    	{
      /* interp_status = */ mb_linear_interp(verbose,
        pars->sensordepth_time_d - 1,
        pars->sensordepth_sensordepth - 1,
        pars->n_sensordepth,
        mbarirange->time_d,
        &mbarirange->sensordepth,
        &jsensordepth,
        &interp_error);
    	}
    if (pars->n_heading > 0)
      {
      /* interp_status = */ mb_linear_interp_heading(verbose,
        pars->heading_time_d - 1,
        pars->heading_heading - 1,
        pars->n_heading,
        mbarirange->time_d,
        &mbarirange->heading,
        &jheading,
        &interp_error);
      }
    if (pars->n_attitude > 0)
      {
      /* interp_status = */ mb_linear_interp(verbose,
        pars->attitude_time_d - 1,
        pars->attitude_roll - 1,
        pars->n_attitude,
        mbarirange->time_d,
        &mbarirange->roll,
        &jattitude,
        &interp_error);
      /* interp_status = */ mb_linear_interp(verbose,
        pars->attitude_time_d - 1,
        pars->attitude_pitch - 1,
        pars->n_attitude,
        mbarirange->time_d,
        &mbarirange->pitch,
        &jattitude,
        &interp_error);
      }

    /* do lever arm correction */
    if (platform_ptr != NULL)
      {

      /* calculate sonar position */
      status = mb_platform_position(verbose,
        platform_ptr,
        pars->target_sensor,
        0,
        mbarirange->navlon,
        mbarirange->navlat,
        mbarirange->sensordepth,
        mbarirange->heading,
        mbarirange->roll,
        mbarirange->pitch,
        &mbarirange->navlon,
        &mbarirange->navlat,
        &mbarirange->sensordepth,
        error);

      /* calculate sonar attitude */
      status = mb_platform_orientation_target(verbose,
        platform_ptr,
        pars->target_sensor,
        0,
        mbarirange->heading,
        mbarirange->roll,
        mbarirange->pitch,
        &mbarirange->heading,
        &mbarirange->roll,
        &mbarirange->pitch,
        error);
      }

    /* get scaling */
    double mtodeglon, mtodeglat;
    mb_coor_scale(verbose, mbarirange->navlat, &mtodeglon, &mtodeglat);
    double headingx = sin(mbarirange->heading * DTR);
    double headingy = cos(mbarirange->heading * DTR);

    /* loop over all soundings */
    for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++)
      {
			struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);

			/* get nav sensordepth heading attitude offset values relative to start of scan for each sounding */
			// int interp_status = MB_SUCCESS;
			int interp_error = MB_ERROR_NO_ERROR;
      double time_d = mbarirange->time_d + (double)sounding->time_offset;
      double navlon = mbarirange->navlon;
      double navlat = mbarirange->navlat;
      double speed = mbarirange->speed;
      double sensordepth = mbarirange->sensordepth;
      double heading = mbarirange->heading;
      double roll = mbarirange->roll;
      double pitch = mbarirange->pitch;
			if (pars->n_nav > 0)
				{
				/* interp_status = */ mb_linear_interp_longitude(verbose,
					pars->nav_time_d - 1,
					pars->nav_lon - 1,
					pars->n_nav,
					time_d,
					&navlon,
					&jnav,
					&interp_error);
				/* interp_status = */ mb_linear_interp_latitude(verbose,
					pars->nav_time_d - 1,
					pars->nav_lat - 1,
					pars->n_nav,
					time_d,
					&navlat,
					&jnav,
					&interp_error);
				/* interp_status = */ mb_linear_interp(verbose,
					pars->nav_time_d - 1,
					pars->nav_speed - 1,
					pars->n_nav,
					time_d,
					&speed,
					&jnav,
					&interp_error);
				}
			if (pars->n_sensordepth > 0)
				{
				/* interp_status = */ mb_linear_interp(verbose,
					pars->sensordepth_time_d - 1,
					pars->sensordepth_sensordepth - 1,
					pars->n_sensordepth,
					time_d,
					&sensordepth,
					&jsensordepth,
					&interp_error);
				}
			if (pars->n_heading > 0)
				{
				/* interp_status = */ mb_linear_interp_heading(verbose,
					pars->heading_time_d - 1,
					pars->heading_heading - 1,
					pars->n_heading,
					time_d,
					&heading,
					&jheading,
					&interp_error);
				}
			if (pars->n_attitude > 0)
				{
				/* interp_status = */ mb_linear_interp(verbose,
					pars->attitude_time_d - 1,
					pars->attitude_roll - 1,
					pars->n_attitude,
					time_d,
					&roll,
					&jattitude,
					&interp_error);
				/* interp_status = */ mb_linear_interp(verbose,
					pars->attitude_time_d - 1,
					pars->attitude_pitch - 1,
					pars->n_attitude,
					time_d,
					&pitch,
					&jattitude,
					&interp_error);
				}

      /* do lever arm correction */
      if (platform_ptr != NULL)
        {
        /* calculate sensor position position */
        status = mb_platform_position(verbose,
          platform_ptr,
          pars->target_sensor,
          0,
          navlon,
          navlat,
          sensordepth,
          heading,
          roll,
          pitch,
          &navlon,
          &navlat,
          &sensordepth,
          error);
        double dlonm = (navlon - mbarirange->navlon) / mtodeglon;
        double dlatm = (navlat - mbarirange->navlat) / mtodeglat;
        sounding->acrosstrack_offset = dlonm * headingx + dlatm * headingy;
        sounding->alongtrack_offset = dlonm * headingy - dlatm * headingx;
        sounding->sensordepth_offset = (float)(sensordepth - mbarirange->sensordepth);

        /* calculate sensor attitude */
        status = mb_platform_orientation_target(verbose,
          platform_ptr,
          pars->target_sensor,
          0,
          heading,
          roll,
          pitch,
          &heading,
          &roll,
          &pitch,
          error);
        sounding->heading_offset = (float)(heading - mbarirange->heading);
        sounding->roll_offset = (float)(roll - mbarirange->roll);
        sounding->pitch_offset = (float)(pitch - mbarirange->pitch);
        }

        /* if requested apply kluge scaling of rx beam angles */
        if (kluge_beampatternsnell) {
          sounding->angle_az = RTD * asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor * sin(DTR * sounding->angle_az))));
        }
			}
			
    /* calculate the bathymetry using the newly inserted values */
    double amplitude_threshold;
    if (pars->sounding_amplitude_filter)
      amplitude_threshold = pars->sounding_amplitude_threshold;
    else
      amplitude_threshold = MBSYS_3DDWISSL2_DEFAULT_AMPLITUDE_THRESHOLD;
    double target_altitude;
    if (pars->sounding_altitude_filter)
      target_altitude = pars->sounding_target_altitude;
    else
      target_altitude = MBSYS_3DDWISSL2_DEFAULT_TARGET_ALTITUDE;
    status = mbsys_3ddwissl2_calculatebathymetry(verbose, mbio_ptr, store_ptr,
                  amplitude_threshold, target_altitude, error);
  }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:         %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
    }

  return status;
} /* mbsys_3ddwissl2_preprocess */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_sensorhead
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *sensorhead,
  int *error
)
{
  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* only one lidar head for the WiSSL2 */
  *sensorhead = 0;
  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sensorhead: %d\n", *sensorhead);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
} /* mbsys_3ddwissl2_sensorhead */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_extract
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                  /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,                /* in: see mbsys_3ddwissl.h:/^struct
                             mbsys_3ddwissl2_struct/ */
  int *kind,                /* out: MBIO data type; see mb_status.h:0+/MBIO data
                         type/ */
  int time_i[7],              /* out: MBIO time array; see mb_time.c:0+/mb_get_time/
                         */
  double *time_d,                  /* out: MBIO time (seconds since 1,1,1970) */
  double *navlon,                  /* out: transducer longitude -180.0..+180.0 */
  double *navlat,                  /* out: transducer latitude -180.0..+180.0 */
  double *speed,              /* out: vessel speed (km/hr) */
  double *heading,                /* out: vessel heading -180.0..+180.0 */
  int *nbath,                /* out: number of bathymetry samples (beams) */
  int *namp,                /* out: number of amplitude samples, usually namp =
                         nbath */
  int *nss,                /* out: number of side scan pixels */
  char *beamflag,                  /* out: array[nbath] of beam flags; see
                             mb_status.h:/FLAG category/ */
  double *bath,              /* out: array[nbath] of depth values (m) positive down
                         */
  double *amp,              /* out: array[namp] of amplitude values */
  double *bathacrosstrack,                /* out: array[nbath] bathy across-track
                                 offsets from transducer (m) */
  double *bathalongtrack,                  /* out: array[nbath] bathy along-track
                                 offsets from transducer (m) */
  double *ss,                /* out: array[nss] sidescan pixel values */
  double *ssacrosstrack,          /* out: array[nss] sidescan across-track offsets from transducer (m) */
  double *ssalongtrack,          /* out: array[nss] sidescan along-track offsets from transducer (m) */
  char *comment,              /* out: comment string */
  int *error                /* out: see mb_status.h:/MB_ERROR/ */
)
{
  (void)ss;  // Unused arg
  (void)ssacrosstrack;  // Unused arg
  (void)ssalongtrack;  // Unused arg

  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from store and copy into mb-system slots */
  if (*kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	  
    /* get the timestamp */
		*time_d = mbarirange->time_d;
		mb_get_date(verbose, *time_d, time_i);

    /* get the navigation */
    *navlon = mbarirange->navlon;
    *navlat = mbarirange->navlat;
    *speed = mbarirange->speed;
    *heading = mbarirange->heading;

    /* get the number of soundings */
    *nbath = mbarirange->num_soundings;
    *namp = *nbath;
    *nss = 0;

    /* we are poking into the mb_io_ptr to change the beamwidth here
        350 microradians for the LIDAR laser */
    mb_io_ptr->beamwidth_xtrack = 0.02;
    mb_io_ptr->beamwidth_ltrack = 0.02;

    /* get the bathymetry */
    for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++) 
    	{
			struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
			beamflag[isounding] = sounding->beamflag;
			bath[isounding] = sounding->depth + mbarirange->sensordepth + sounding->sensordepth_offset;
			amp[isounding] = (double) sounding->intensity;
			bathacrosstrack[isounding] = sounding->acrosstrack + sounding->acrosstrack_offset;
			bathalongtrack[isounding] = sounding->alongtrack + sounding->alongtrack_offset;
    	}

    /* always successful */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }

  else if (*kind == MB_DATA_COMMENT)
    {
    memset((void *)store->comment.comment, 0, MB_COMMENT_MAXLINE);
		strncpy(store->comment.comment, comment, MB_COMMENT_MAXLINE - 1);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_extract */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_insert
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,              /* in: see mbsys_3ddwissl.h:/^struct
                           mbsys_3ddwissl2_struct/ */
  int kind,                /* in: see mb_status.h:0+/MBIO data type/ */
  int time_i[7],              /* in: see mb_time.c:0+/mb_get_time/ */
  double time_d,              /* in: time in seconds since 1,1,1970) */
  double navlon,              /* in: transducer longitude -180.0..+180.0 */
  double navlat,              /* in: transducer latitude -180.0..+180.0 */
  double speed,              /* in: vessel speed (km/hr) */
  double heading,                /* in: vessel heading -180.0..+180.0 */
  int nbath,                /* in: number of bathymetry samples/beams */
  int namp,                /* in: number of amplitude samples, usually namp ==
                         nbath */
  int nss,                /* in: number of sidescan pixels */
  char *beamflag,                /* in: array[nbath] of beam flags; see
                           mb_status.h:/FLAG category/ */
  double *bath,              /* in: array[nbath] of depth values (m) positive down */
  double *amp,              /* in: array[namp] of amplitude values */
  double *bathacrosstrack,              /* in: array[nbath] bathy across-track
                               offsets from transducer (m) */
  double *bathalongtrack,                /* in: array[nbath] bathy along-track
                               offsets from transducer (m) */
  double *ss,                /* in: array[nss] sidescan pixel values */
  double *ssacrosstrack,          /* in: array[nss] sidescan across-track offsets from
                         transducer (m) */
  double *ssalongtrack,          /* in: array[nss] sidescan along-track offsets from
                         transducer (m) */
  char *comment,              /* in: comment string */
  int *error                /* out: see mb_status.h:/MB_ERROR/ */
)
{
  (void)ss;  // Unused arg
  (void)ssacrosstrack;  // Unused arg
  (void)ssalongtrack;  // Unused arg

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(time_i != NULL);
  assert(0 <= nbath);
  assert(0 <= namp);
  assert(namp == nbath);
  assert(0 <= nss);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       kind:       %d\n", kind);
    }

  int status = MB_SUCCESS;

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  store->kind = kind;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* set the timestamp */
		mbarirange->time_d = time_d;
    mbarirange->TimeStart_Sec = (int) floor(time_d);
   	mbarirange->TimeStart_nSec = (int)(1.0e9 * (time_d - mbarirange->TimeStart_Sec));

    /* set the navigation */
    mbarirange->navlon = navlon;
    mbarirange->navlat = navlat;
    mbarirange->speed = speed;
    mbarirange->heading = heading;

    /* check for allocation of space */
		if (status == MB_SUCCESS && mbarirange->num_soundings_alloc < nbath)
			{
			size_t alloc_size = nbath * sizeof(struct mbsys_3ddwissl2_sounding_struct);
			status = mb_reallocd(verbose, __FILE__, __LINE__, 
								alloc_size, (void **)(&mbarirange->soundings), error);
			if (status == MB_SUCCESS)
				{
				memset(mbarirange->soundings, 0, alloc_size);
				mbarirange->num_soundings_alloc = mbarirange->num_soundings;
				}
			else
				{
				store->kind = MB_DATA_NONE;
				int freestatus = mb_freed(verbose, __FILE__, __LINE__, 
								(void **)(&mbarirange->soundings), error);
				}
			}

    /* set the bathymetry */
    if (status == MB_SUCCESS) 
    	{
    	mbarirange->num_soundings = nbath;
			for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++) 
				{
				struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
        sounding->beamflag = beamflag[isounding];
        sounding->depth = bath[isounding] - mbarirange->sensordepth - sounding->sensordepth_offset;
        sounding->intensity = (int) amp[isounding];
        sounding->acrosstrack = bathacrosstrack[isounding] - sounding->acrosstrack_offset;
        sounding->alongtrack = bathalongtrack[isounding] - sounding->alongtrack_offset;
				}
			}
				
    /* insert the sidescan pixel data */
    }

  /* deal with comments */
  else if (store->kind == MB_DATA_COMMENT)
    {
    store->comment.comment_len = MIN(strlen(comment) + 1, MB_COMMENT_MAXLINE);
    memset((void *)store->comment.comment, 0, MB_COMMENT_MAXLINE);
    strncpy(store->comment.comment, comment, MB_COMMENT_MAXLINE - 1);
    }

  /* deal with other records types  */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 4)
    mbsys_3ddwissl2_print_store(verbose, store, error);
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_insert */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_ttimes
(
  int verbose,                  /* in: verbosity level set on command line 0..N
                             */
  void *mbio_ptr,                  /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,                /* in: see mbsys_3ddwissl.h:/^struct
                             mbsys_3ddwissl2_struct/ */
  int *kind,                    /* out: MBIO data type; see mb_status.h:0+/MBIO
                             data type/ */
  int *nbeams,                  /* out: number of beams (samples) in this ping
                             */
  double *ttimes,                  /* out: array[nbeams] travel time of beam (secs)
                             */
  double *angles,                  /* out: array[nbeams] across-track angle of beam
                             (deg) */
  double *angles_forward,                /* out: array[nbeams] along-track angle of
                               beam (deg) */
  double *angles_null,              /* out: array[nbeams] ?? */
  double *heave,                  /* out: array[nbeams] heave for each beam ?? */
  double *alongtrack_offset,                /* out: array[nbeams] ?? */
  double *draft,                  /* out: draft of transducer below waterline ??
                             (m) */
  double *ssv,                  /* out: sound velocity at head (m/s) */
  int *error                    /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(ttimes != NULL);
  assert(angles != NULL);
  assert(angles_forward != NULL);
  assert(angles_null != NULL);
  assert(heave != NULL);
  assert(alongtrack_offset != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract travel time data */
  if (*kind == MB_DATA_DATA)
    {
    /* get the number of soundings */
    *nbeams = store->mbarirange.num_soundings;

    /* get travel times, angles */
    for (int i = 0; i < *nbeams; i++)
      {
      ttimes[i] = 0.0;
      angles[i] = 0.0;
      angles_forward[i] = 0.0;
      angles_null[i] = 0.0;
      heave[i] = 0.0;
      alongtrack_offset[i] = 0.0;
      }

    /* get ssv */
    *ssv = 0.0;
    *draft = 0.0;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
    }
  /* deal with comment record type */
  else if (*kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }
  /* deal with other record types */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  /* print output debu statements */
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_ttimes */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_detects
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                  /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,                /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl2_struct/ */
  int *kind,                /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
  int *nbeams,              /* out: number of beams (samples) in this ping */
  int *detects,                /* out: array[nbeams] detection flag;
                            see mb_status.h:/Bottom detect flags/ */
  int *error                /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", detects);
    }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* get the number of soundings */
    *nbeams = mbarirange->num_soundings;

    /* WiSSL2 detects */
		for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++) 
			{
			// Bits 8-11 are used for multi-detect sounding priority, with highest == 0
			// A sounding flagged as secondary has a priority of 1, else the priority is 0
			struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
			if (mb_beam_check_flag_multipick(sounding->beamflag))
				detects[isounding] = MB_DETECT_LIDAR | 0x100;
			else
				detects[isounding] = MB_DETECT_LIDAR;
			}			

    /* always successful */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* deal with other record type */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    }
  if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
    {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
    }
  if (verbose >= 2)
    {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_detects */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_pulses
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,              /* in: see mbsys_3ddwissl.h:/^struct
                           mbsys_3ddwissl2_struct/ */
  int *kind,                /* out: MBIO data type; see mb_status.h:0+/MBIO data
                         type/ */
  int *nbeams,              /* out: number of beams (samples) in this ping */
  int *pulses,              /* out: array[nbeams] pulse type; see
                         mb_status.h:/Source pulse/ */
  int *error                /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       pulses:     %p\n", pulses);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* get the number of soundings */
    *nbeams = mbarirange->num_soundings;

    /* get pulse type */
    for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++)
      pulses[isounding] = MB_PULSE_LIDAR;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }

  /* deal with comments */
  else if (*kind == MB_DATA_COMMENT)
    {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* deal with other record type */
  else
    {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    }
  if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
    {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
    }
  if (verbose >= 2)
    {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_pulses */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_gains
(
  int verbose,                /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,              /* in: see mbsys_3ddwissl.h:/^struct
                           mbsys_3ddwissl2_struct/ */
  int *kind,                  /* in: MBIO data type; see mb_status.h:0+/MBIO data
                           type/ */
  double *transmit_gain,                /* out: transmit gain (dB) */
  double *pulse_length,                /* out: pulse width (usec) */
  double *receive_gain,                /* out: receive gain (dB) */
  int *error                  /* out: see mb_status.h:/MB_ERROR/ */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA)
    {
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* get laser power */
    *transmit_gain = (double)mbarirange->LineLaserPower;

    /* get pulse_length */
    *pulse_length = 0.0;

    /* get receive_gain (dB) */
    *receive_gain = 0.0;
    }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* deal with other record types */
  else
    {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    }
  if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
    {
    fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
    fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
    fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
    }
  if (verbose >= 2)
    {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_gains */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_extract_altitude
(
  int verbose,        /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,        /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,      /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl2_struct/ */
  int *kind,          /* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
  double *transducer_depth,  /* out: transducer depth below water line (m) */
  double *altitude,      /* out: transducer altitude above seafloor (m) */
  int *error          /* out: see mb_status.h:/MB_ERROR/ */
)
{
  struct mbsys_3ddwissl2_pulse_struct *pulse;
  struct mbsys_3ddwissl2_sounding_struct *sounding;

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA)
    {
	  	struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* get sonar depth */
    *transducer_depth = mbarirange->sensordepth;

    /* loop over all soundings looking for most nadir */
    double rmin = 9999999.9;
		for (int isounding=0; isounding < mbarirange->num_soundings; isounding++) 
			{
			struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
			if (mb_beam_ok(sounding->beamflag))
				{
				double r = sqrt(sounding->acrosstrack * sounding->acrosstrack);
				if (r < rmin)
					{
					rmin = r;
					*altitude = sounding->depth;
    			*transducer_depth = mbarirange->sensordepth + sounding->sensordepth_offset;
					}
				}
			}
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* deal with other record type */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
    fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_extract_altitude */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_extract_nnav
(
  int verbose,                /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                    /* in: see mb_io.h:/^struct mb_io_struct/ */
  void *store_ptr,                  /* in: see mbsys_3ddwissl.h:/^struct
                               mbsys_3ddwissl2_struct/ */
  int nmax,                  /* in: maximum size available to n; e.g., n < nmax
                           */
  int *kind,                  /* out: MBIO data type; see mb_status.h:0+/MBIO data
                           type/ */
  int *n,                    /* out: number of navigation values extracted */
  int *time_i,                /* out: array[n] time_i[7] values; see
                           mb_time.c:0+/mb_get_time/ */
  double *time_d,                    /* out: array[n] time_d values; seconds
                               since 1,1,1970 */
  double *navlon,                    /* out: array[n] longitude (degrees);
                               -180.0..+180.0 */
  double *navlat,                    /* out: array[n] latitude (degree); -90..+90
                               */
  double *speed,                /* out: array[n] speed (m/s) */
  double *heading,                  /* out: array[n] heading (degree): 0..360 */
  double *draft,                /* out: array[n] txer depth below datum (m) */
  double *roll,                /* out: array[n] roll (degrees) */
  double *pitch,                /* out: array[n] pitch (degrees) */
  double *heave,                /* out: array[n] heave (m) */
  int *error                  /* out: see mb_status.h:/MB_ERROR/ */
)
{
  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(nmax > 0);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
    }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from ping structure */
  if (*kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

    /* just one navigation value */
    *n = 1;
	  
    /* get the timestamp */
		*time_d = mbarirange->time_d;

    /* get navigation and heading */
    navlon[0] = mbarirange->navlon;
    navlat[0] = mbarirange->navlat;
    speed[0] = mbarirange->speed;
    heading[0] = mbarirange->heading;

    /* get draft */
    draft[0] = mbarirange->sensordepth;

    /* get roll pitch and heave. In SXP heave is included in height. */
    roll[0] = mbarirange->roll;
    pitch[0] = mbarirange->pitch;
    heave[0] = 0.0;

    /* done translating values */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }
    
  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT)
    {
    *n = 0;
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }
  /* deal with other record type */
  else
    {
    *n = 0;
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       n:          %d\n", *n);
    for (int inav = 0; inav < *n; inav++)
      {
      for (int i = 0; i < 7; i++)
        fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i,
          time_i[inav * 7 + i]);
      fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
      fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
      fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
      fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
      fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
      fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
      fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
      fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
      fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
      }
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_extract_nnav */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_extract_nav
(
  int verbose,
  void *mbio_ptr,                      /* in: verbosity level set on command
                                 line 0..N */
  void *store_ptr,                  /* in: see mb_io.h:/^struct mb_io_struct/ */
  int *kind,                      /* out: see mbsys_3ddwissl.h:/^struct
                               mbsys_3ddwissl2_struct/ */
  int time_i[7],                    /* out: time_i[7] values; see mb_time.c */
  double *time_d,                    /* out: time in seconds since 1,1,1970 */
  double *navlon,                    /* out: longitude (degrees) -180..+180.0 */
  double *navlat,                    /* out: latittude (degrees) -90..+90 */
  double *speed,                    /* out: speed (km/s) */
  double *heading,                  /* out: heading (degrees) 0..360 */
  double *draft,                    /* out: draft (m) */
  double *roll,                    /* out: roll (degrees) */
  double *pitch,                    /* out: pitch (degrees) */
  double *heave,                    /* out: heave (degrees) */
  int *error                      /* out: see mb_status.h:MB_ERROR */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* extract data from structure */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from ping structure */
  if (*kind == MB_DATA_DATA)
    {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	  
    /* get the timestamp */
		*time_d = mbarirange->time_d;

    /* get navigation and heading */
    *navlon = mbarirange->navlon;
    *navlat = mbarirange->navlat;
    *speed = mbarirange->speed;
    *heading = mbarirange->heading;

    /* get draft */
    *draft = mbarirange->sensordepth;

    /* get roll pitch and heave. In SXP heave is included in height. */
    *roll = mbarirange->roll;
    *pitch = mbarirange->pitch;
    *heave = 0.0;

    /* done translating values */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }
  /* deal with other record type */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    }
  if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) && (*kind == MB_DATA_DATA))
    {
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
    fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", *speed);
    fprintf(stderr, "dbg2       heading:       %f\n", *heading);
    fprintf(stderr, "dbg2       draft:         %f\n", *draft);
    fprintf(stderr, "dbg2       roll:          %f\n", *roll);
    fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
    fprintf(stderr, "dbg2       heave:         %f\n", *heave);
    }
  if (verbose >= 2)
    {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_extract_nav */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_insert_nav
(
  int verbose,
  void *mbio_ptr,                      /* in: verbosity level set on command
                                 line */
  void *store_ptr,                      /* in: see mb_io.h:mb_io_struct */
  int time_i[7],                        /* in: time_i struct; see mb_time.c
                                   */
  double time_d,                        /* in: time in seconds since
                                   1,1,1970 */
  double navlon,                        /* in: longitude in degrees
                                   -180..+180 */
  double navlat,                        /* in: latitude in degrees -90..+90
                                   */
  double speed,                        /* in: speed (m/s) */
  double heading,                        /* in: heading (degrees) */
  double draft,                        /* in: draft (m) */
  double roll,                        /* in: roll (degrees) */
  double pitch,                        /* in: pitch (degreees) */
  double heave,                        /* in: heave (m) */
  int *error                          /* out: see mb_status.h:MB_ERROR */
)
{
  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(time_i != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
    fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
    fprintf(stderr, "dbg2       speed:      %f\n", speed);
    fprintf(stderr, "dbg2       heading:    %f\n", heading);
    fprintf(stderr, "dbg2       draft:      %f\n", draft);
    fprintf(stderr, "dbg2       roll:       %f\n", roll);
    fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
    fprintf(stderr, "dbg2       heave:      %f\n", heave);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  const int status = MB_SUCCESS;

  /* insert data in data structure */
  if (store->kind == MB_DATA_DATA) {
	  struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	  
    /* get the timestamp */
		mbarirange->time_d = time_d;

    /* get navigation and heading */
    mbarirange->navlon = navlon;
    mbarirange->navlat = navlat;
    mbarirange->speed = speed;
    mbarirange->heading = heading;

    /* get draft */
    mbarirange->sensordepth = draft;

    /* get roll pitch and heave. In SXP heave is included in height. */
    mbarirange->roll = roll;
    mbarirange->pitch = pitch;
    // heave;

    /* done translating values */
    *error = MB_ERROR_NO_ERROR;
    // status = MB_SUCCESS;
  }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_insert_nav */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_extract_svp
(
  int verbose,                /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                /* in: see mb_io.h:mb_io_struct */
  void *store_ptr,                  /* in: see
                               mbsys_3ddwissl.h:mbsys_3ddwissl2_struct */
  int *kind,                  /* out: see mb_status.h:MBIO data type */
  int *nsvp,                  /* out: number of svp measurements */
  double *depth,                /* out: array[nsvp] depths (m) */
  double *velocity,                  /* out: array[nsvp] velocity (m) */
  int *error                  /* out: see: mb_status.h:MB_ERROR */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_COMMENT)
    {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* deal with other record type */
  else
    {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
    for (int i = 0; i < *nsvp; i++)
      fprintf(stderr,
        "dbg2       depth[%d]: %f   velocity[%d]: %f\n",
        i,
        depth[i],
        i,
        velocity[i]);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_extract_svp */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_insert_svp
(
  int verbose,                /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,                /* in: mbio.h:mb_io_struct */
  void *store_ptr,                  /* in: mbsys_3ddwissl2_struct */
  int nsvp,                  /* in: number of svp records to insert */
  double *depth,                /* in: array[nsvp] depth records (m) */
  double *velocity,                  /* in: array[nsvp] sound velocity records
                               (m/s) */
  int *error                  /* out: see mb_status.h:MB_ERROR */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(nsvp > 0);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
    for (int i = 0; i < nsvp; i++)
      fprintf(stderr,
        "dbg2       depth[%d]: %f   velocity[%d]: %f\n",
        i,
        depth[i],
        i,
        velocity[i]);
    }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_COMMENT)
    {
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
    }

  /* handle other types */
  else
    {
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_insert_svp */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_copy
(
  int verbose,            /* in: verbosity level set on command line */
  void *mbio_ptr,                /* in: see mb_io.h:mb_io_struct */
  void *store_ptr,              /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl2_struct */
  void *copy_ptr,                /* out: see mbsys_3ddwissl.h:mbsys_3ddwissl2_struct
                           */
  int *error              /* out: see mb_status.h:MB_ERROR */
)
{
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(copy_ptr != NULL);
  assert(store_ptr != copy_ptr);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", copy_ptr);
    }

  /* set error status */
  *error = MB_ERROR_NO_ERROR;

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointers */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;
  struct mbsys_3ddwissl2_struct *copy = (struct mbsys_3ddwissl2_struct *)copy_ptr;

  /* copy structure */
  const int num_soundings_alloc = copy->mbarirange.num_soundings_alloc;
  struct mbsys_3ddwissl2_sounding_struct *soundings = copy->mbarirange.soundings;
  memcpy(copy_ptr, store_ptr, sizeof(struct mbsys_3ddwissl2_struct));
  copy->mbarirange.num_soundings_alloc = num_soundings_alloc;
  copy->mbarirange.soundings = soundings;

  int status = MB_SUCCESS;

  /* allocate memory for soundings if needed */
  if ((copy->mbarirange.num_soundings > copy->mbarirange.num_soundings_alloc) 
  			|| ( copy->mbarirange.soundings == NULL) )
    {
    status =
      mb_reallocd(verbose, __FILE__, __LINE__,
      copy->mbarirange.num_soundings * sizeof(struct mbsys_3ddwissl2_sounding_struct),
      (void **)&copy->mbarirange.soundings, error);
    if (status == MB_SUCCESS)
      copy->mbarirange.num_soundings_alloc = copy->mbarirange.num_soundings;
    }

  /* copy pulses */
  if (status == MB_SUCCESS) 
  	{
  	memcpy((void *)copy->mbarirange.soundings, (void *)store->mbarirange.soundings,
    				copy->mbarirange.num_soundings * sizeof(struct mbsys_3ddwissl2_sounding_struct));
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_copy */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl2_print_store
(
  int verbose,              /* in: verbosity level set on command line 0..N */
  void *store_ptr,          /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl2_struct */
  int *error                /* out: see mb_status.h:MB_ERROR */
)
{
  struct mbsys_3ddwissl2_pulse_struct *pulse;
  struct mbsys_3ddwissl2_sounding_struct *sounding;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2         store:    %p\n", store_ptr);
    }

  /* check for non-null data */
  assert(store_ptr != NULL);

  /* always successful */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  /* get data structure pointers */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* print 3DDWISSL2 store structure contents */
  static const char debug_str[] = "dbg2  ";
  static const char nodebug_str[] = "  ";
  const char *first;
  if (verbose >= 2)
    {
    first = debug_str;
    }
  else
    {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
    }
  fprintf(stderr, "%s struct mbsys_3ddwissls contents:\n", first);
  fprintf(stderr, "%s     kind:                          %d\n", first, store->kind);

  if (store->kind == MB_DATA_PARAMETER)
    {
	  struct mbsys_3ddwissl2_sriat_fileheader_struct *fileheader = &store->fileheader;
		fprintf(stderr, "\n%s  File header record\n", first);
		fprintf(stderr, "%s     fileheader->SyncWord:                    %u\n", first, fileheader->SyncWord);
		fprintf(stderr, "%s     fileheader->PacketID:                    %u\n", first, fileheader->PacketID);
		fprintf(stderr, "%s     fileheader->Version:                     %u\n", first, fileheader->Version);
		fprintf(stderr, "%s     fileheader->SizeBytes:                   %u\n", first, fileheader->SizeBytes);
		fprintf(stderr, "%s     fileheader->ScanSizeBytes:               %u\n", first, fileheader->ScanSizeBytes);
		fprintf(stderr, "%s     fileheader->TimeStart_Sec:               %d\n", first, fileheader->TimeStart_Sec);
		fprintf(stderr, "%s     fileheader->TimeStart_nSec:              %d\n", first, fileheader->TimeStart_nSec);
		fprintf(stderr, "%s     fileheader->TimeEnd_Sec:                 %d\n", first, fileheader->TimeEnd_Sec);
		fprintf(stderr, "%s     fileheader->TimeEnd_nSec:                %d\n", first, fileheader->TimeEnd_nSec);
		fprintf(stderr, "%s     fileheader->SL_GEN:                      %u\n", first, fileheader->SL_GEN);
		fprintf(stderr, "%s     fileheader->SL_Letter:                   %u\n", first, fileheader->SL_Letter);
		fprintf(stderr, "%s     fileheader->SL_X:                        %u\n", first, fileheader->SL_X);
		fprintf(stderr, "%s     fileheader->nPtsToAverage:               %u\n", first, fileheader->nPtsToAverage);
		fprintf(stderr, "%s     fileheader->cJobName:                    %s\n", first, fileheader->cJobName);
		fprintf(stderr, "%s     fileheader->cScanPos:                    %s\n", first, fileheader->cScanPos);
		fprintf(stderr, "%s     fileheader->cfileTag:                    %s\n", first, fileheader->cfileTag);
		fprintf(stderr, "%s     fileheader->nScanNum:                    %u\n", first, fileheader->nScanNum);
		fprintf(stderr, "%s     fileheader->AzCmdStart:                  %u\n", first, fileheader->AzCmdStart);
		fprintf(stderr, "%s     fileheader->AzCmdEnd:                    %u\n", first, fileheader->AzCmdEnd);
		fprintf(stderr, "%s     fileheader->nScanNum:                    %u\n", first, fileheader->nScanNum);
		fprintf(stderr, "%s     fileheader->rawbit1:                     %u\n", first, fileheader->rawbit1);
		fprintf(stderr, "%s     -fileheader->nPtsPerScanLine:             %u\n", first, fileheader->nPtsPerScanLine);
		fprintf(stderr, "%s     -fileheader->nScanLinesPerScan:           %u\n", first, fileheader->nScanLinesPerScan);
		fprintf(stderr, "%s     -fileheader->Spare1:                      %u\n", first, fileheader->Spare1);
		fprintf(stderr, "%s     fileheader->rawbit2:                     %u\n", first, fileheader->rawbit2);
		fprintf(stderr, "%s     -fileheader->nPtsPerLine:                 %u\n", first, fileheader->nPtsPerLine);
		fprintf(stderr, "%s     -fileheader->Mode:                        %u\n", first, fileheader->Mode);
		fprintf(stderr, "%s     -fileheader->nTPtsPerScanLine:            %u\n", first, fileheader->nTPtsPerScanLine);
		fprintf(stderr, "%s     -fileheader->bHaveThermal:                %u\n", first, fileheader->bHaveThermal);
		fprintf(stderr, "%s     fileheader->ShotCnt:                     %u\n", first, fileheader->ShotCnt);
		fprintf(stderr, "%s     fileheader->WaterSalinity_psu:           %u  %.3f\n", first, fileheader->WaterSalinity_psu, fileheader->WaterSalinity_psu * 42.0 / 65535.0 - 2.0);
		fprintf(stderr, "%s     fileheader->WaterPressure_dbar:          %u\n", first, fileheader->WaterPressure_dbar);
		fprintf(stderr, "%s     fileheader->rawbit3:                     %u\n", first, fileheader->rawbit3);
		fprintf(stderr, "%s     -fileheader->WaterTemperature_C:          %u  %.3f\n", first, fileheader->WaterTemperature_C, fileheader->WaterTemperature_C * 37.0 / 8191.0 - 2.0);
		fprintf(stderr, "%s     -fileheader->PRF_Hz:                      %u\n", first, fileheader->PRF_Hz);
		fprintf(stderr, "%s     fileheader->DigitizerTemperature_C:      %u  %.3f\n", first, fileheader->DigitizerTemperature_C, fileheader->DigitizerTemperature_C * 100.0 / 255.0);
		fprintf(stderr, "%s     fileheader->RScale_m_per_cnt:            %f\n", first, fileheader->RScale_m_per_cnt);
		fprintf(stderr, "%s     fileheader->ThBinStart_cnt:              %u\n", first, fileheader->ThBinStart_cnt);
		fprintf(stderr, "%s     fileheader->ThBinEnd_cnts:               %u\n", first, fileheader->ThBinEnd_cnts);
		fprintf(stderr, "%s     fileheader->TempAzCnt:                   %u\n", first, fileheader->TempAzCnt);
		fprintf(stderr, "%s     fileheader->TempRowCnt:                  %u\n", first, fileheader->TempRowCnt);
		fprintf(stderr, "%s     fileheader->rawbit4:                     %u\n", first, fileheader->rawbit4);
		fprintf(stderr, "%s     -fileheader->TempRCnt_av2:                %u\n", first, fileheader->TempRCnt_av2);
		fprintf(stderr, "%s     -fileheader->TempRCnt_av4:                %u\n", first, fileheader->TempRCnt_av4);
		fprintf(stderr, "%s     -fileheader->TempRCnt_av8:                %u\n", first, fileheader->TempRCnt_av8);
		fprintf(stderr, "%s     -fileheader->TempRCnt_av16:               %u\n", first, fileheader->TempRCnt_av16);
		fprintf(stderr, "%s     fileheader->ScannerShift_mDeg:           %u\n", first, fileheader->ScannerShift_mDeg);
		fprintf(stderr, "%s     fileheader->Shift_m[0]:                  %f\n", first, fileheader->Shift_m[0]);
		fprintf(stderr, "%s     fileheader->Shift_m[1]:                  %f\n", first, fileheader->Shift_m[1]);
		fprintf(stderr, "%s     fileheader->Shift_m[2]:                  %f\n", first, fileheader->Shift_m[2]);
		fprintf(stderr, "%s     fileheader->Rotate_deg[0]:               %f\n", first, fileheader->Rotate_deg[0]);
		fprintf(stderr, "%s     fileheader->Rotate_deg[1]:               %f\n", first, fileheader->Rotate_deg[1]);
		fprintf(stderr, "%s     fileheader->Rotate_deg[2]:               %f\n", first, fileheader->Rotate_deg[2]);
		fprintf(stderr, "%s     fileheader->EC_Version:                  %d.%d.%d.%d\n", 
						first, fileheader->EC_Version[0], fileheader->EC_Version[1], 
						fileheader->EC_Version[2], fileheader->EC_Version[3]);
		fprintf(stderr, "%s     fileheader->InstaCloud_Version:          %d.%d.%d.%d\n", 
						first, fileheader->InstaCloud_Version[0], fileheader->InstaCloud_Version[1], 
						fileheader->InstaCloud_Version[2], fileheader->InstaCloud_Version[3]);
		fprintf(stderr, "%s     fileheader->ElDeg_cnts:                  %d  %.3f\n", first, fileheader->ElDeg_cnts, fileheader->ElDeg_cnts * 90.0 / 65535.0);
    }
  else if (store->kind == MB_DATA_COMMENT)
    {
	  struct mbsys_3ddwissl2_comment_struct *comment = &store->comment;
		fprintf(stderr, "%s     comment->SyncWord:                       %u\n", first, comment->SyncWord);
		fprintf(stderr, "%s     comment->PacketID:                       %u\n", first, comment->PacketID);
		fprintf(stderr, "%s     comment->Version:                        %u\n", first, comment->Version);
		fprintf(stderr, "%s     comment->SizeBytes:                      %u\n", first, comment->SizeBytes);
		fprintf(stderr, "%s     comment->comment_len:                    %u\n", first, comment->comment_len);
		fprintf(stderr, "%s     comment->comment:                        %s\n", first, comment->comment);
    }
  else if (store->kind == MB_DATA_DATA)
    {
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_print_store */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl2_calculatebathymetry
(
  int verbose,        /* in: verbosity level set on command line 0..N */
  void *mbio_ptr,     /* in: see mb_io.h:mb_io_struct */
  void *store_ptr,    /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl2_struct */
  double amplitude_threshold,   /* used to determine valid soundings */
  double target_altitude, /* used to prioritize picks close to a desired standoff */
  int *error              /* out: see mb_status.h:MB_ERROR */
)
{
  (void)mbio_ptr;  // Unused arg

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
    fprintf(stderr, "dbg2         mbio_ptr:            %p\n", mbio_ptr);
    fprintf(stderr, "dbg2         store:               %p\n", store_ptr);
    fprintf(stderr, "dbg2         amplitude_threshold: %f\n", amplitude_threshold);
    fprintf(stderr, "dbg2         target_altitude:     %f\n", target_altitude);
    }

  assert(store_ptr != NULL);

  /* always successful */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  /* get data structure pointers */
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* recalculate bathymetry from LIDAR data */
  if (store->kind == MB_DATA_DATA)
    {
    struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
  	
    /* get scaling */
  	double mtodeglon, mtodeglat;
    mb_coor_scale(verbose, mbarirange->navlat, &mtodeglon, &mtodeglat);

    /* loop over all soundings */
    for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++)
      {
  		struct mbsys_3ddwissl2_sounding_struct *sounding = &mbarirange->soundings[isounding];
  		
  		if (sounding->range > 0.0) 
  			{

				/* apply pitch and roll */  // TODO fix rotation w/ Giancarlo's functions
				double alpha = sounding->angle_el + mbarirange->pitch + sounding->pitch_offset;
				double beta = 90.0 - sounding->angle_az + mbarirange->roll + sounding->roll_offset;
	
				/* translate to takeoff coordinates */
				double theta, phi;
				mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
				phi += sounding->heading_offset;    // TODO fix rotation w/ Giancarlo's functions
	
				/* get lateral and vertical components of range */
				double xx = sounding->range * sin(DTR * theta);
				sounding->depth = sounding->range * cos(DTR * theta)
													+ sounding->sensordepth_offset;
				sounding->acrosstrack = xx * cos(DTR * phi) + sounding->acrosstrack_offset;
				sounding->alongtrack = xx * sin(DTR * phi) + sounding->alongtrack_offset;

				/* calculate amplitude range factor */
				double altitude_factor = 1.0;
				if (target_altitude > 0.0)
					{
					double target_range = target_altitude /
													cos(DTR * (sounding->angle_az - mbarirange->roll - sounding->roll_offset));
					double scaled_range_diff = (sounding->range - target_range) / target_range;
					altitude_factor = exp(-4.0 * scaled_range_diff * scaled_range_diff);
					}
				double flagging_factor = altitude_factor * sounding->intensity * sounding->range * sounding->range / 1000.0;
				
				/* get beamflag */
        //if (sounding->intensity * amplitude_factor >= amplitude_threshold)
        if (sounding->sounding_id == 0 && flagging_factor > amplitude_threshold)
					{
          sounding->beamflag = MB_FLAG_NONE;
					}
				else 
					{
					sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_SONAR;
					}
//fprintf(stderr, " sounding->intensity:%d amplitude_threshold:%f *:%f  beamflag:%d\n",
//sounding->intensity, amplitude_threshold, sounding->intensity*amplitude_factor, sounding->beamflag);
  			}
  		else
  			{
				sounding->depth = 0.0;
				sounding->acrosstrack = 0.0;
				sounding->alongtrack = 0.0;
				sounding->beamflag = MB_FLAG_NULL;
  			}
  		}

		if (verbose >= 5) 
			{
			for (int isounding=0; isounding < mbarirange->num_soundings; isounding++) 
				{
				struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
				fprintf(stderr, "dbg5       sdg:%5.5d:%4.4d:%d | %8.6f %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f | %7.4f %8.3f %8.3f %4u %3u | %8.4f %8.4f %8.4f %u\n", 
					isounding, sounding->pulse_id, sounding->sounding_id, sounding->time_offset,
					sounding->acrosstrack_offset, sounding->alongtrack_offset, sounding->sensordepth_offset,
					sounding->heading_offset, sounding->roll_offset, sounding->pitch_offset,
					sounding->range, sounding->angle_az, sounding->angle_el, sounding->intensity, sounding->class,
					sounding->acrosstrack, sounding->alongtrack, sounding->depth, sounding->beamflag);
				}
			}

    /* set flag indicating that bathymetry has been calculated */
    store->bathymetry_calculated = true;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
}  /* mbsys_3ddwissl2_calculatebathymetry */
/*--------------------------------------------------------------------*/
