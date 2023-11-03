/*--------------------------------------------------------------------
 *    The MB-system:  mbr_image83p.c  5/5/2008
 *
 *    Copyright (c) 2008-2023 by
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
 * mbr_image83p.c contains the functions for reading and writing
 * multibeam data in the IMAGE83P format.
 * These functions include:
 *   mbr_alm_image83p  - allocate read/write memory
 *   mbr_dem_image83p  - deallocate read/write memory
 *   mbr_rt_image83p  - read and translate data
 *   mbr_wt_image83p  - translate and write data
 *
 * Author: Vivek Reddy, Santa Clara University (initial version)
 *         D.W. Caress (many revisions since)
 * Date:  May 5, 2008
 *
 */
/*
 * Notes on the MBSYS_IMAGE83P data structure:
 *   1. Imagenex DeltaT multibeam systems output raw data in a format
 *      combining ascii and binary values.
 *   2. These systems output up to 480 beams of bathymetry
 *   3. The data structure defined below includes all of the values
 *      which are passed in the 83P Imagenex data format records plus many
 *      values calculated from the raw data.
 *   4. The initial 83P format version was labeled 1.xx but is coded as 1.00. The
 *      second format version is 1.10. As of November 2022, versions through 1.10
 *      are supported as format MBF_IMAGE83 (191).
 *   5. Support for comment records is specific to MB-System.
 *   6. The MBF_IMAGE83P format does not support beam flags. Support for beam
 *      flags is specific to the extended MB-System format MBF_IMAGEMBA (id=192).
 *      Format MBF_IMAGEMBA records also include the bathymetry soundings
 *      calculated as arrays of bathymetry values and the acrosstrack and
 *      alongtrack positions of the soundings.
 *   7. Both formats have two spaces for recording heading, roll, and pitch. If the
 *      multibeam has its own attitude sensor then these values are recorded with
 *      0.1 degree precision. There are other spaces in the header for heading,
 *      roll and pitch stored as floats so that there are several digits of
 *      precision available. In some installations the logged files include
 *      attitude data in those secondary fields from an external sensor (and in
 *      that case can also include heave). MB-System uses the float attitude values
 *      in processing. When reading a file, if the internal integer values are
 *      nonzero and the external float values are flagged as undefined, then the
 *      former values (converted to degrees) are copied to the latter.
 *      Subsequently the external float fields are used as the source for heading
 *      and attitude data.
 *   8. The vendor MBF_IMAGE83P format does not include a field for sonar depth, but does
 *      include a field for heave. The extended MBF_IMAGEMBA format includes separate
 *      float fields for both heave and sonar depth - the sonar depth is typically used
 *      either as a static draft on a surface vessel or a pressure depth on a
 *      submerged AUV or ROV platform. Heave is positive up and sonar depth is
 *      positive down. In some cases on submerged platforms the pressure depth is
 *      recorded into the heave field. In that case the --kluge-sensordepth-from-heave
 *      argument to mbpreprocess will cause the heave value to be moved to the
 *      sonar_depth field in the output MBF_IMAGEMBA format files.
 *   9. Comment records are supported for both formats - this is specific to MB-System.
 */

/* The following .83P format specification information is taken from a
 *      16 March 2010 document from Imagenex Technology Corporation.
 *
 *  DeltaT - 83P PROFILE POINT OUTPUT
 *  ( 83P UDP/IP Ethernet Datagram, .83P File Format )
 *
 *  For each ping, the following bytes are output during the 83P UDP datagram. If
 *  recording to a .83P file, the following bytes are appended and saved to the file
 *  for each ping. The total number of bytes ‘N’ for each ping will vary depending
 *  on the number of beams selected.
 *
 *      Byte #          Byte Description
 *      ------          ----------------
 *      0-255       File Header (256 bytes)
 *      256-nnn     Profile Ranges for current ping (2 range bytes / beam)
 *                    nnn = 256 + (2*number_of_beams) – 1
 *                  If Intensity Bytes are included (Byte 117 = 1),
 *                    nnn = 256 + (4*number_of_beams) – 1
 *
 *  FILE HEADER (note -this is really a record header present for each survey record)
 *  Bytes 0 through 255 contain the following File Header information:
 *
 *  0       ASCII '8'
 *  1       ASCII '3'
 *  2       ASCII 'P'
 *
 *  3       .83P File Version
 *          0 = v1.00
 *          10 = v1.10
 *
 *  4-5     Total Bytes ‘N’ - number of bytes that are written to the disk for this ping
 *          Byte 117 = 0 (No Intensity) ==> N = 256 + (2*number_of_beams)
 *          Byte 117 = 1 (Intensity)    ==> N = 256 + (4*number_of_beams)
 *
 *  6       Reserved - always 0
 *  7       Reserved - always 0
 *
 *  8-19    Sonar Ping Interrogation Timestamp
 *            Date – system date, null terminated string (12 bytes) "DD-MMM-YYYY"
 *
 *  20-28   Sonar Ping Interrogation Timestamp
 *            Time – system time, null terminated string (9 bytes) "HH:MM:SS"
 *
 *  29-32   Sonar Ping Interrogation Timestamp
 *            Hundredths of Seconds – system time, null terminated string (4 bytes) ".hh"
 *            Note: see Bytes 112-116 for Milliseconds.
 *
 *  33-46   GNSS Ships Position Latitude – text string (14 bytes) “_dd.mm.xxxxx_N”
 *          dd = Degrees
 *          mm = Minutes
 *          xxxxx = Decimal Minutes
 *          _ = Space
 *          N = North or S = South
 *
 *  47-60   GNSS Ships Position Longitude – text string (14 bytes) “ddd.mm.xxxxx_E”
 *          ddd = Degrees
 *          mm = Minutes
 *          xxxxx = Decimal Minutes
 *          _ = Space
 *          E = East or W = West
 *
 *  61 GNSS Ships Speed
 *          Speed = (Byte 61)/10 in knots
 *
 *  62-63 GNSS Ships Course
 *          |        byte 62         |        byte 63         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |           Course * 10 (in degrees)              |
 *
 *  64-65 Pitch Angle (from Internal Sensor)
 *          |        byte 64         |        byte 65         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | P |       (Pitch Angle*10) + 900                |
 *                If 'P' = 0, Pitch Angle = 0 degrees
 *                If 'P' = 1, Pitch Angle = [[((Byte 64 & 0x7F)<<8) | (Byte 65)]-900]/10
 *
 *  66-67 Roll Angle (from Internal Sensor)
 *          |        byte 66         |        byte 67         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | R |       (Roll Angle*10) + 900                 |
 *                If 'R' = 0, Roll Angle = 0 degrees
 *                If 'R' = 1, Roll Angle = [[((Byte 66 & 0x7F)<<8) | (Byte 67)]-900]/10
 *
 *  68-69 Heading Angle (from Internal Sensor)
 *          |        byte 68         |        byte 69         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | H |       Heading Angle*10                      |
 *                If 'H' = 0, Heading Angle = 0 degrees
 *                If 'H' = 1, Heading Angle = [((Byte 68 & 0x7F)<<8) | (Byte 69)]/10
 *
 *  70-71 Beams
 *          |        byte 70         |        byte 71         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |                    Number of Beams              |
 *
 *  72-73 Samples Per Beam
 *          |        byte 72         |        byte 73         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Number of Samples Per Beam         |
 *
 *  74-75 Sector Size
 *          |        byte 74         |        byte 75         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Sector Size (in degrees)           |
 *
 *  76-77 Start Angle (Beam 0 angle)
 *          |        byte 76         |        byte 77         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |    [Start Angle (in degrees) + 180] * 100       |
 *
 *  78 Angle Increment
 *          Angle spacing per beam = (Byte 78)/100 in degrees
 *
 *  79-80 Acoustic Range
 *          |        byte 79         |        byte 80         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Acoustic Range (in meters)         |
 *
 *  81-82 Acoustic Frequency
 *          |        byte 81         |        byte 82         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |             Acoustic Frequency (in kHz)         |
 *
 *  83-84 Sound Velocity
 *          |        byte 83         |        byte 84         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | V |  Sound Velocity (in meters/second) * 10     |
 *                If 'V' = 0, Sound Velocity = 1500.0 m/s
 *                If 'V' = 1, Sound Velocity = [((Byte 83 & 0x7F)<<8) | (Byte 84)]/10.0
 *
 *  85-86 Range Resolution
 *          |        byte 85         |        byte 86         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |       Range Resolution (in millimeters)         |
 *
 *  87-88 Reserved
 *          Reserved – always 0
 *
 *  89-90 Profile Tilt Angle (mounting offset)
 *          |        byte 89         |        byte 90         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |     Profile Tilt Angle (in degrees) + 180       |
 *
 *  91-92 Repetition Rate – Time between pings
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |       Repetition Rate (in milliseconds)         |
 *
 *  93-96 Ping Number – increment for every ping
 *          |  byte 93  |  byte 94   |   byte 93  |  byte 94  |
 *          |                   Ping Number                   |
 *
 *  Version 1.00 - end of nonzero parameters in the header
 *  97-255  Reserved – always 0
 *
 *  Version 1.10 - bytes 100-154 now used for additional parameters
 *
 *  97-98  Reserved – always 0
 *
 *  100-103 Sonar X-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar X-Offset (in meters)           |
 *
 *  104-107 Sonar Y-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar Y-Offset (in meters)           |
 *
 *  108-111 Sonar Z-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar Z-Offset (in meters)           |
 *
 *  112-116 Sonar Ping Interrogation Timestamp
 *          Milliseconds – system time, null terminated string (5 bytes) ".mmm"
 *
 *  117 Intensity Bytes Included
 *          0 = No
 *          1 = Yes
 *
 *  118-119 Ping Latency – Time from sonar ping interrogation to actual ping
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Ping Latency (in units of 100 microseconds)   |
 *
 *  120-121 Data Latency – Time from sonar ping interrogation to 83P UDP datagram
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Data Latency (in units of 100 microseconds)   |
 *          Time Since Ping = Data Latency – Ping Latency
 *          Note: Data Latency is not available during file playback.
 *
 *  122 Sample Rate
 *          0 = Standard Resolution (1 in 500)
 *          1 = High Resolution (1 in 5000)
 *
 *  123 Option Flags
 *          Bit 0 – 1 = data is corrected for roll
 *          Bit 1 – 1 = data is corrected for ray bending
 *          Bit 2 – 1 = sonar is operating in overlapped mode Bit 3 – 0
 *          Bit 4 – 0
 *          Bit 5 – 0
 *          Bit 6 – 0
 *          Bit 7 – 0
 *
 *  124 Reserved - always 0
 *
 *  125 Number of Pings Averaged
 *          0 to 25
 *
 *  126-127 Center Ping Time Offset
 *          The Sonar Ping Interrogation Timestamp (Bytes 8-19, 20-28 and 112-116)
 *          is the timestamp for the current ping. But due to ping averaging,
 *          the ping time of the center ping (of a group of averaged pings)
 *          may be required (i.e. for roll stabilization). The Center Ping Time
 *          Offset is the time difference between the center ping interrogation
 *          and the current ping interrogation.
 *          |        byte 126        |        byte 127        |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Center Ping Time Offset (in units of 100 microseconds)   |
 *          Center Ping Time = Sonar Ping Interrogation Timestamp
 *                             – Center Ping Time Offset + Ping Latency
 *          Note: Profile data from the current ping should be used when
 *          subtracting the Center Ping Time Offset.
 *
 *  128-131 Heave (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 128 |  byte 129  |   byte 130 |  byte 131 |
 *          |                 Heave (in meters)               |
 *
 *  132     User Defined Byte – this is a copy of the 837 User Defined Byte
           (Byte 45 from the .837 File Header)
 *
 *  133-136 Altitude
 *          4-byte single precision floating point number
 *          |  byte 133 |  byte 134  |   byte 135 |  byte 136 |
 *          |               Altitude (in meters)              |
 *
 *  137     External Sensor Flags
 *          Bit 0 – 1 = external heading angle available
 *          Bit 1 – 1 = external roll angle available
 *          Bit 2 – 1 = external pitch angle available
 *          Bit 3 – 1 = external heave available
 *          Bit 4 – 0
 *          Bit 5 – 0
 *          Bit 6 – 0
 *          Bit 7 – 0
 *
 *  138-141 Pitch Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 138 |  byte 139  |   byte 140 |  byte 141 |
 *          |                 Pitch (in degrees)              |
 *
 *  142-145 Roll Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 142 |  byte 143  |   byte 144 |  byte 145 |
 *          |                  Roll (in degrees)              |
 *
 *  146-149 Heading Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 146 |  byte 147  |   byte 148 |  byte 149 |
 *          |               Heading (in degrees)              |
 *
 *  150     Transmit Scan Flag
 *          0 = manual scan
 *          1 = auto-scan
 *
 *  151-154 Transmit Scan Angle
 *          4-byte single precision floating point number
 *          |  byte 151 |  byte 152  |   byte 153 |  byte 154 |
 *          |        Transmit Scan Angle (in degrees)         |
 *
 *  155-255 Reserved
 *          always 0
 *
 *  BEAM DATA
 *  Version 1.00: Only the beam ranges are provided, as an array of 2-byte
 *  unsigned integers in units of range resolution (bytes 85-86) assuming
 *  a 1500 m/s water sound speed. The number of beams is set in the 2-byte
 *  integer in bytes 70-71.
 *
 *  Version 1.10: If byte 117 is true, then following the array of beam ranges
 *  is an array of beam intensity (per-beam amplitude in MB-System terminology)
 *  as 2-byte unsigned integers described as "normalized amplitude". The number
 *  of beams is set in the 2-byte integer in bytes 70-71.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_image83p.h"

#define MBF_IMAGE83P_BUFFER_SIZE (MBSYS_IMAGE83P_HEADERLEN + MBSYS_IMAGE83P_BEAMS * 4)

/*--------------------------------------------------------------------*/
int mbr_info_image83p(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* set format info parameters */
  *error = MB_ERROR_NO_ERROR;
  *system = MB_SYS_IMAGE83P;
  *beams_bath_max = MBSYS_IMAGE83P_BEAMS;
  *beams_amp_max = MBSYS_IMAGE83P_BEAMS;
  *pixels_ss_max = 0;
  strncpy(format_name, "IMAGE83P", MB_NAME_LENGTH);
  strncpy(system_name, "IMAGE83P", MB_NAME_LENGTH);
  strncpy(format_description,
          "Format name:          MBF_IMAGE83P\nInformal Description: Imagenex DeltaT Multibeam\nAttributes:           "
          "Multibeam, bathymetry, 480 beams, ascii + binary, Imagenex.\n",
          MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_NORMAL;
  *variable_beams = false;
  *traveltime = false;
  *beam_flagging = true;
  *platform_source = MB_DATA_NONE;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_NONE;
  *svp_source = MB_DATA_NONE;
  *beamwidth_xtrack = 0.75;
  *beamwidth_ltrack = 0.75;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", *system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
    fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_image83p(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_image83p_struct), &mb_io_ptr->store_data, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_image83p(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* deallocate memory for data descriptor */
  const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  char buffer[MBF_IMAGE83P_BUFFER_SIZE] = "";
  int index = 0;
  short short_val = 0;
  int int_val = 0;
  float float_val = 0.0;
  int numberbytes = 0;

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

  /* set file position */
  mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

  /* read next record header from file */
  memset((void *) buffer, (int)0, (size_t)MBF_IMAGE83P_BUFFER_SIZE);
  int status = MB_SUCCESS;
  bool done = false;
  if ((status = fread(buffer, 1, 6, mb_io_ptr->mbfp)) == 6) {
    /* check for valid header */
    if (strncmp(buffer, "83P", 3) == 0) {
      done = true;
      status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
    }
    else {
      /* loop over reading bytes until valid header is found */
      while (!done) {
        for (int i = 0; i < 5; i++)
          buffer[i] = buffer[i + 1];
        status = fread(&buffer[5], 1, 1, mb_io_ptr->mbfp);
        if (status != 1) {
          mb_io_ptr->file_bytes += status;
          status = MB_FAILURE;
          *error = MB_ERROR_EOF;
          done = true;
        }
        else if (strncmp(buffer, "83P", 3) == 0) {
          done = true;
        }
      }
    }
  }
  else {
    mb_io_ptr->file_bytes += status;
    status = MB_FAILURE;
    *error = MB_ERROR_EOF;
  }

  const bool swap = false;
  bool swap_external = false;

  /* read rest of record from file */
  if (status == MB_SUCCESS) {
    index = 3;
    store->version = (int)buffer[index];
    index += 1;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    numberbytes = (int)((unsigned short)short_val);

    if ((status = fread(&buffer[6], 1, numberbytes - 6, mb_io_ptr->mbfp)) != numberbytes - 6) {
      mb_io_ptr->file_bytes += status;
      status = MB_FAILURE;
      *error = MB_ERROR_EOF;
      store->kind = MB_DATA_NONE;
    }
    else {
      status = MB_SUCCESS;
      mb_io_ptr->file_bytes += status;
    }
  }

  /* if success then parse the buffer */
  if (status == MB_SUCCESS && buffer[6] == '#') {
    /* type of data record */
    store->kind = MB_DATA_COMMENT;

    /* copy comment */
    index = 8;
    strncpy(store->comment, &buffer[index], MBSYS_IMAGE83P_COMMENTLEN);
  }

  /* if success then parse the buffer */
  else if (status == MB_SUCCESS) {
    /* type of data record */
    store->kind = MB_DATA_DATA;

    /* parse year */
    index = 8;
    mb_get_int(&store->time_i[0], &buffer[index + 7], 4);

    /* parse month */
    if (buffer[index + 3] == 'J') {
      if (buffer[index + 4] == 'A') {
        store->time_i[1] = 1;
      }
      else if (buffer[index + 5] == 'N') {
        store->time_i[1] = 6;
      }
      else {
        store->time_i[1] = 7;
      }
    }
    else if (buffer[index + 3] == 'F') {
      store->time_i[1] = 2;
    }
    else if (buffer[index + 3] == 'M') {
      if (buffer[index + 5] == 'R') {
        store->time_i[1] = 3;
      }
      else {
        store->time_i[1] = 5;
      }
    }
    else if (buffer[index + 3] == 'A') {
      if (buffer[index + 4] == 'P') {
        store->time_i[1] = 4;
      }
      else {
        store->time_i[1] = 8;
      }
    }
    else if (buffer[index + 3] == 'S') {
      store->time_i[1] = 9;
    }
    else if (buffer[index + 3] == 'O') {
      store->time_i[1] = 10;
    }
    else if (buffer[index + 3] == 'N') {
      store->time_i[1] = 11;
    }
    else if (buffer[index + 3] == 'D') {
      store->time_i[1] = 12;
    }

    mb_get_int(&store->time_i[2], &buffer[index + 0], 2);
    index += 12; /*to time*/

    /* parse time */
    mb_get_int(&store->time_i[3], &buffer[index], 2);
    mb_get_int(&store->time_i[4], &buffer[index + 3], 2);
    mb_get_int(&store->time_i[5], &buffer[index + 6], 2);
    int seconds_hundredths;
    mb_get_int(&seconds_hundredths, &buffer[index + 10], 2);
    store->time_i[6] = 10000 * seconds_hundredths;
    mb_get_time(verbose, store->time_i, &store->time_d);
    for (int i = 0; i < 7; i++) {
      mb_io_ptr->new_time_i[i] = store->time_i[i];
    }
    mb_io_ptr->new_time_d = store->time_d;
    index += 13; /*to navigation latitude*/

    /* parse gps navigation string latitude */
    double degrees, minutes, dec_minutes;
    mb_get_double(&degrees, &buffer[index + 1], 2);
    mb_get_double(&minutes, &buffer[index + 4], 2);
    mb_get_double(&dec_minutes, &buffer[index + 7], 5);
    store->nav_lat = degrees + (((dec_minutes / 100000) + minutes) / 60);
    if (buffer[index + 13] == 'S' || buffer[index + 13] == 's') {
      store->nav_lat = -store->nav_lat;
    }
    index += 14; /*to navigation longtitude*/

    /* parse gps navigation string longtitude */
    mb_get_double(&degrees, &buffer[index], 3);
    mb_get_double(&minutes, &buffer[index + 4], 2);
    mb_get_double(&dec_minutes, &buffer[index + 7], 5);
    store->nav_long = degrees + (((dec_minutes / 100000) + minutes) / 60);
    if (buffer[index + 13] == 'W' || buffer[index + 13] == 'w') {
      store->nav_long = -store->nav_long;
    }
    index += 14;

    /* parse gps speed and heading */
    store->nav_speed = (int)((mb_u_char)buffer[index]);
    index += 1;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->course = (int)((unsigned short)short_val);
    index += 2;

    /* parse dvl attitude and heading */
    /*    if (((mb_u_char)buffer[index]) >> 7)
                {
                store->pitch = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
                }
            else
                {
                store->pitch = 0;
                }*/
    store->pitch = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index + 1]);
    index += 2;

    /*    if (((mb_u_char)buffer[index]) >> 7)
                {
                store->roll = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
                }
            else
                {
                store->roll = 0;
                }*/
    store->roll = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index + 1]);
    index += 2;

    /*    if (((mb_u_char)buffer[index]) >> 7)
                {
                store->heading = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
                }
            else
                {
                store->heading = 0;
                }*/
    store->heading = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index + 1]);
    index += 2;

    /* parse beam info */
    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->num_beams = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->samples_per_beam = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->sector_size = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->start_angle = (int)((unsigned short)short_val);
    index += 2;

    store->angle_increment = (int)((mb_u_char)buffer[index]);
    index += 1;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->acoustic_range = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->acoustic_frequency = (int)((unsigned short)short_val);
    index += 2;

    if (buffer[index] >> 7) {
      store->sound_velocity = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index + 1]);
    }
    else {
      store->sound_velocity = 15000;
    }
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->range_resolution = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->pulse_length = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->profile_tilt_angle = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_short(swap, &buffer[index], &short_val);
    store->rep_rate = (int)((unsigned short)short_val);
    index += 2;

    mb_get_binary_int(swap, &buffer[index], &int_val);
    store->ping_number = int_val;
    index += 4;

    /* if version 1.10 read the rest of the header parameters */
    if (store->version >= 10) {
      index += 3;

      mb_get_binary_float(swap, &buffer[index], &float_val);
      store->sonar_x_offset = float_val;
      index += 4;

      mb_get_binary_float(swap, &buffer[index], &float_val);
      store->sonar_y_offset = float_val;
      index += 4;

      mb_get_binary_float(swap, &buffer[index], &float_val);
      store->sonar_z_offset = float_val;
      index += 4;

      /* replace centiseconds of timestamp already parsed with higher resolution milliseconds encoded here */
      int milliseconds;
      mb_get_int(&milliseconds, &buffer[113], 3);
      store->time_i[6] = 1000 * milliseconds;
      mb_get_time(verbose, store->time_i, &store->time_d);
      mb_io_ptr->new_time_i[6] = store->time_i[6];
      mb_io_ptr->new_time_d = store->time_d;
      index += 5;

      store->has_intensity = (int)buffer[index];
      index += 1;

      mb_get_binary_short(swap, &buffer[index], &short_val);
      store->ping_latency = (int)((unsigned short)short_val);
      index += 2;

      mb_get_binary_short(swap, &buffer[index], &short_val);
      store->data_latency = (int)((unsigned short)short_val);
      index += 2;

      store->sample_rate = (int)buffer[index];
      index += 1;

      store->option_flags = (mb_u_char)buffer[index];
      index += 1;

      index += 1;

      store->number_averaged = (int)buffer[index];
      index += 1;

      mb_get_binary_short(swap, &buffer[index], &short_val);
      store->center_time_offset = (int)((unsigned short)short_val);
      index += 2;

      mb_get_binary_float(swap_external, &buffer[index], &float_val);
      store->heave_external = float_val;
      index += 4;

      store->user_defined_byte = (mb_u_char)buffer[index];
      index += 1;

      mb_get_binary_float(swap_external, &buffer[index], &float_val);
      store->altitude = float_val;
      index += 4;

      store->external_sensor_flags = (mb_u_char)buffer[index];
      index += 1;

      mb_get_binary_float(swap_external, &buffer[index], &float_val);
      store->pitch_external = float_val;
      index += 4;

      mb_get_binary_float(swap_external, &buffer[index], &float_val);
      store->roll_external = float_val;
      index += 4;

      mb_get_binary_float(swap_external, &buffer[index], &float_val);
      store->heading_external = float_val;
      index += 4;

      store->transmit_scan_flag = (mb_u_char)buffer[index];
      index += 1;

      mb_get_binary_float(swap, &buffer[index], &float_val);
      store->transmit_scan_angle = float_val;
      index += 4;

      /* check for case that external sensor values have little endian byte
          order, in which case reverse the swap bool and recalculate the
          values for this ping */
      bool reverse_swap = false;
      if ((store->external_sensor_flags & 0x01 && store->heading_external < 0.0) || store->heading_external > 360.0)
        reverse_swap = true;
      if ((store->external_sensor_flags & 0x02 && store->roll_external < -90.0) || store->roll_external > 90.0)
        reverse_swap = true;
      if ((store->external_sensor_flags & 0x04 && store->pitch_external < 0.0) || store->pitch_external > 360.0)
        reverse_swap = true;
      if ((store->external_sensor_flags & 0x08 && store->heave_external < -1000.0) || store->heave_external > 1000.0)
        reverse_swap = true;
      if (reverse_swap) {
        if (swap_external)
          swap_external = false;
        else
          swap_external = true;
        mb_get_binary_float(swap_external, &buffer[128], &float_val);
        store->altitude = float_val;
        mb_get_binary_float(swap_external, &buffer[133], &float_val);
        store->heave_external = float_val;
        mb_get_binary_float(swap_external, &buffer[138], &float_val);
        store->pitch_external = float_val;
        mb_get_binary_float(swap_external, &buffer[142], &float_val);
        store->roll_external = float_val;
        mb_get_binary_float(swap_external, &buffer[146], &float_val);
        store->heading_external = float_val;
      }
    }

    /* get ranges */
    index = 256;
    for (int i = 0; i < store->num_beams; i++) {
      mb_get_binary_short(swap, &buffer[index], &short_val);
      store->range[i] = (int)((unsigned short)short_val);
      index += 2;
    }

    /* get amplitudes */
    if (store->has_intensity) {
      for (int i = 0; i < store->num_beams; i++) {
        mb_get_binary_short(swap, &buffer[index], &short_val);
        store->intensity[i] = (int)((unsigned short)short_val);
        index += 2;
      }
    }
    else {
      memset(store->intensity, 0, store->num_beams * sizeof(short));
    }

    /* fix unexpected zero values */
    if (store->pitch == 0)
      store->pitch = 900;
    if (store->roll == 0)
      store->roll = 900;
    if (store->profile_tilt_angle == 0)
      store->profile_tilt_angle = 180;

    /* if external attitude not included, copy from internal values */
    if (!(store->external_sensor_flags & 0x01)) {
      store->heading_external = store->heading / 10.0;
      store->external_sensor_flags = store->external_sensor_flags | 0x01;
    }
    if (!(store->external_sensor_flags & 0x02)) {
      store->roll_external = 0.1 * (store->roll - 900);
      store->external_sensor_flags = store->external_sensor_flags | 0x02;
    }
    if (!(store->external_sensor_flags & 0x04)) {
      store->pitch_external = 0.1 * (store->pitch - 900);
      store->external_sensor_flags = store->external_sensor_flags | 0x04;
    }
    if (!(store->external_sensor_flags & 0x08)) {
      store->heave_external = 0.0;
      store->external_sensor_flags = store->external_sensor_flags | 0x08;
    }
  }
  mb_io_ptr->new_kind = store->kind;
  mb_io_ptr->new_error = *error;

  /* if success then calculate bathymetry */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
    double soundspeed;
    if (store->sound_velocity > 13000 && store->sound_velocity < 17000)
      soundspeed = 0.1 * store->sound_velocity;
    else
      soundspeed = 1500.0;
    store->sonar_depth = 0.0;
    double heading = (double) store->heading_external;
    double roll = (double) store->roll_external;
    double pitch = (double) store->pitch_external;

    mb_3D_orientation tx_align = {0.0, 0.0, 0.0};
    mb_3D_orientation tx_orientation = {0.0, 0.0, 0.0};
    double tx_steer = 0.0;
    mb_3D_orientation rx_align = {0.0, 0.0, 0.0};
    mb_3D_orientation rx_orientation = {0.0, 0.0, 0.0};
    double rx_steer = 0.0;
    int rx_sign = 1;
    store->num_proc_beams = store->num_beams;
    for (int i = 0; i < store->num_proc_beams; i++) {
      if (store->range[i] > 0) {
        /* calculate beam angles for raytracing using Jon Beaudoin's code based on:
            Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
            Surface Sound Speed Measurements in Post-Processing for Multi-Sector
            Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
            p.26-31.
            (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
           note complexity if transducer arrays are reverse mounted, as determined
           by a mount heading angle of about 180 degrees rather than about 0 degrees.
           If a receive array or a transmit array are reverse mounted then:
            1) subtract 180 from the heading mount angle of the array
            2) flip the sign of the pitch and roll mount offsets of the array
            3) flip the sign of the beam steering angle from that array
                (reverse TX means flip sign of TX steer, reverse RX
                means flip sign of RX steer) */
        tx_steer = 0.0;
        tx_orientation.roll = roll;
        tx_orientation.pitch = pitch + ((double)store->profile_tilt_angle - 180.0);;
        tx_orientation.heading = heading;
        rx_steer = rx_sign * (180.0 - 0.01 * (store->start_angle + i * store->angle_increment));
        rx_orientation.roll = roll;
        rx_orientation.pitch = pitch + ((double)store->profile_tilt_angle - 180.0);;
        rx_orientation.heading = heading;
        double reference_heading = heading;
        double beamAzimuth;
        double beamDepression;
        status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                             reference_heading, &beamAzimuth, &beamDepression, error);
        const double theta = 90.0 - beamDepression;
        double phi = 90.0 - beamAzimuth;
        if (phi < 0.0)
          phi += 360.0;

        /* calculate Bathymetry */
        double rr = (soundspeed / 1500.0) * 0.001 * store->range_resolution * store->range[i];
        const double xx = rr * sin(DTR * theta);
        const double zz = rr * cos(DTR * theta);
        store->beamrange[i] = rr;
        store->angles[i] = theta;
        store->angles_forward[i] = phi;
        store->beamflag[i] = MB_FLAG_NONE;
        store->bath[i] = zz + store->sonar_depth - store->heave_external;
        store->bathacrosstrack[i] = xx * cos(DTR * phi);
        store->bathalongtrack[i] = xx * sin(DTR * phi);
        store->amp[i] = (float) store->intensity[i];
      }
      else {
        store->beamrange[i] = 0.0;
        store->angles[i] = 0.0;
        store->angles_forward[i] = 0.0;
        store->beamflag[i] = MB_FLAG_NULL;
        store->bath[i] = 0.0;
        store->bathacrosstrack[i] = 0.0;
        store->bathalongtrack[i] = 0.0;
        store->amp[i] = 0.0;
      }
    }
  }

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg2  Record read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg4  Data values:\n");
    fprintf(stderr, "dbg4       kind:                    %d\n", store->kind);
    fprintf(stderr, "dbg4       version:                 %d\n", store->version);
    fprintf(stderr, "dbg4       time_i[0]:               %d\n", store->time_i[0]);
    fprintf(stderr, "dbg4       time_i[1]:               %d\n", store->time_i[1]);
    fprintf(stderr, "dbg4       time_i[2]:               %d\n", store->time_i[2]);
    fprintf(stderr, "dbg4       time_i[3]:               %d\n", store->time_i[3]);
    fprintf(stderr, "dbg4       time_i[4]:               %d\n", store->time_i[4]);
    fprintf(stderr, "dbg4       time_i[5]:               %d\n", store->time_i[5]);
    fprintf(stderr, "dbg4       time_i[6]:               %d\n", store->time_i[6]);
    fprintf(stderr, "dbg4       time_d:                  %f\n", store->time_d);
    fprintf(stderr, "dbg4       nav_lat:                 %f\n", store->nav_lat);    /* degrees */
    fprintf(stderr, "dbg4       nav_long:                %f\n", store->nav_long);   /* degrees */
    fprintf(stderr, "dbg4       nav_speed:               %d\n", store->nav_speed);  /* 0.1 knots */
    fprintf(stderr, "dbg4       course:                  %d\n", store->course);     /* 0.1 degrees */
    fprintf(stderr, "dbg4       pitch:                   %d\n", store->pitch);      /* 0.1 degrees */
    fprintf(stderr, "dbg4       roll:                    %d\n", store->roll);       /* 0.1 degrees */
    fprintf(stderr, "dbg4       heading:                 %d\n", store->heading);    /* 0.1 degrees */
    fprintf(stderr, "dbg4       num_beams:               %d\n", store->num_beams);
    fprintf(stderr, "dbg4       samples_per_beam:        %d\n", store->samples_per_beam);
    fprintf(stderr, "dbg4       sector_size:             %d\n", store->sector_size);        /* degrees */
    fprintf(stderr, "dbg4       start_angle:             %d\n", store->start_angle);        /* 0.01 degrees + 180.0 */
    fprintf(stderr, "dbg4       angle_increment:         %d\n", store->angle_increment);    /* 0.01 degrees */
    fprintf(stderr, "dbg4       acoustic_range:          %d\n", store->acoustic_range);     /* meters */
    fprintf(stderr, "dbg4       acoustic_frequency:      %d\n", store->acoustic_frequency); /* kHz */
    fprintf(stderr, "dbg4       sound_velocity:          %d\n", store->sound_velocity);     /* 0.1 m/sec */
    fprintf(stderr, "dbg4       range_resolution:        %d\n", store->range_resolution);   /* 0.001 meters */
    fprintf(stderr, "dbg4       pulse_length:            %d\n", store->pulse_length);       /* usec */
    fprintf(stderr, "dbg4       profile_tilt_angle:      %d\n", store->profile_tilt_angle); /* degrees + 180.0 */
    fprintf(stderr, "dbg4       rep_rate:                %d\n", store->rep_rate);           /* msec */
    fprintf(stderr, "dbg4       ping_number:             %d\n", store->ping_number);
    fprintf(stderr, "dbg4       sonar_x_offset:          %f\n", store->sonar_x_offset);
    fprintf(stderr, "dbg4       sonar_y_offset:          %f\n", store->sonar_y_offset);
    fprintf(stderr, "dbg4       sonar_z_offset:          %f\n", store->sonar_z_offset);
    fprintf(stderr, "dbg4       has_intensity:           %d\n", store->has_intensity);
    fprintf(stderr, "dbg4       ping_latency:            %d\n", store->ping_latency);
    fprintf(stderr, "dbg4       data_latency:            %d\n", store->data_latency);
    fprintf(stderr, "dbg4       sample_rate:             %d\n", store->sample_rate);
    fprintf(stderr, "dbg4       option_flags:            %u\n", store->option_flags);
    fprintf(stderr, "dbg4       number_averaged:         %d\n", store->number_averaged);
    fprintf(stderr, "dbg4       center_time_offset:      %u\n", store->center_time_offset);
    fprintf(stderr, "dbg4       heave_external:          %f\n", store->heave_external);
    fprintf(stderr, "dbg4       user_defined_byte:       %u\n", store->user_defined_byte);
    fprintf(stderr, "dbg4       altitude:                %f\n", store->altitude);
    fprintf(stderr, "dbg4       external_sensor_flags:   %u\n", store->external_sensor_flags);
    fprintf(stderr, "dbg4       pitch_external:          %f\n", store->pitch_external);
    fprintf(stderr, "dbg4       roll_external:           %f\n", store->roll_external);
    fprintf(stderr, "dbg4       heading_external:        %f\n", store->heading_external);
    fprintf(stderr, "dbg4       transmit_scan_flag:      %u\n", store->transmit_scan_flag);
    fprintf(stderr, "dbg4       transmit_scan_angle:     %f\n", store->transmit_scan_angle);
    for (int i = 0; i < store->num_beams; i++)
      fprintf(stderr, "dbg4       %d range: %d intensity: %d\n", i, store->range[i], store->intensity[i]);
    fprintf(stderr, "dbg4       sonar_depth:        %f\n", store->sonar_depth);
    fprintf(stderr, "dbg4       num_proc_beams:     %d\n", store->num_proc_beams);
    for (int i = 0; i < store->num_proc_beams; i++)
      fprintf(stderr, "dbg4       tt[%d]: %f angles:%f %f   bath: %f %f %f %d\n", i, store->beamrange[i], store->angles[i],
              store->angles_forward[i], store->bath[i], store->bathacrosstrack[i], store->bathalongtrack[i],
              store->beamflag[i]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  char buffer[MBF_IMAGE83P_BUFFER_SIZE] = "";
  int seconds_hundredths;
  int degrees;
  double minutes;
  char NorS;
  int write_len = 0;
  int index;

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_image83p_struct *store = (struct mbsys_image83p_struct *)store_ptr;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Status at beginning of MBIO function <%s>\n", __func__);
    if (store != NULL)
      fprintf(stderr, "dbg5       store->kind:    %d\n", store->kind);
    fprintf(stderr, "dbg5       new_kind:       %d\n", mb_io_ptr->new_kind);
    fprintf(stderr, "dbg5       new_error:      %d\n", mb_io_ptr->new_error);
    fprintf(stderr, "dbg5       error:          %d\n", *error);
  }

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg2  Record read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg4  Data values:\n");
    fprintf(stderr, "dbg4       kind:                    %d\n", store->kind);
    fprintf(stderr, "dbg4       version:                 %d\n", store->version);
    fprintf(stderr, "dbg4       time_i[0]:               %d\n", store->time_i[0]);
    fprintf(stderr, "dbg4       time_i[1]:               %d\n", store->time_i[1]);
    fprintf(stderr, "dbg4       time_i[2]:               %d\n", store->time_i[2]);
    fprintf(stderr, "dbg4       time_i[3]:               %d\n", store->time_i[3]);
    fprintf(stderr, "dbg4       time_i[4]:               %d\n", store->time_i[4]);
    fprintf(stderr, "dbg4       time_i[5]:               %d\n", store->time_i[5]);
    fprintf(stderr, "dbg4       time_i[6]:               %d\n", store->time_i[6]);
    fprintf(stderr, "dbg4       time_d:                  %f\n", store->time_d);
    fprintf(stderr, "dbg4       nav_lat:                 %f\n", store->nav_lat);
    fprintf(stderr, "dbg4       nav_long:                %f\n", store->nav_long);
    fprintf(stderr, "dbg4       nav_speed:               %d\n", store->nav_speed);          /* 0.1 knots */
    fprintf(stderr, "dbg4       course:                  %d\n", store->course);             /* 0.1 degrees */
    fprintf(stderr, "dbg4       pitch:                   %d\n", store->pitch);              /* 0.1 degrees */
    fprintf(stderr, "dbg4       roll:                    %d\n", store->roll);               /* 0.1 degrees */
    fprintf(stderr, "dbg4       heading:                 %d\n", store->heading);            /* 0.1 degrees */
    fprintf(stderr, "dbg4       num_beams:               %d\n", store->num_beams);
    fprintf(stderr, "dbg4       samples_per_beam:        %d\n", store->samples_per_beam);
    fprintf(stderr, "dbg4       sector_size:             %d\n", store->sector_size);        /* degrees */
    fprintf(stderr, "dbg4       start_angle:             %d\n", store->start_angle);        /* 0.01 degrees + 180.0 */
    fprintf(stderr, "dbg4       angle_increment:         %d\n", store->angle_increment);    /* 0.01 degrees */
    fprintf(stderr, "dbg4       acoustic_range:          %d\n", store->acoustic_range);     /* meters */
    fprintf(stderr, "dbg4       acoustic_frequency:      %d\n", store->acoustic_frequency); /* kHz */
    fprintf(stderr, "dbg4       sound_velocity:          %d\n", store->sound_velocity);     /* 0.1 m/sec */
    fprintf(stderr, "dbg4       range_resolution:        %d\n", store->range_resolution);   /* 0.001 meters */
    fprintf(stderr, "dbg4       pulse_length:            %d\n", store->pulse_length);       /* usec */
    fprintf(stderr, "dbg4       profile_tilt_angle:      %d\n", store->profile_tilt_angle); /* degrees + 180.0 */
    fprintf(stderr, "dbg4       rep_rate:                %d\n", store->rep_rate);           /* msec */
    fprintf(stderr, "dbg4       ping_number:             %d\n", store->ping_number);
    fprintf(stderr, "dbg4       sonar_x_offset:          %f\n", store->sonar_x_offset);
    fprintf(stderr, "dbg4       sonar_y_offset:          %f\n", store->sonar_y_offset);
    fprintf(stderr, "dbg4       sonar_z_offset:          %f\n", store->sonar_z_offset);
    fprintf(stderr, "dbg4       has_intensity:           %d\n", store->has_intensity);
    fprintf(stderr, "dbg4       ping_latency:            %d\n", store->ping_latency);
    fprintf(stderr, "dbg4       data_latency:            %d\n", store->data_latency);
    fprintf(stderr, "dbg4       sample_rate:             %d\n", store->sample_rate);
    fprintf(stderr, "dbg4       option_flags:            %u\n", store->option_flags);
    fprintf(stderr, "dbg4       number_averaged:         %d\n", store->number_averaged);
    fprintf(stderr, "dbg4       center_time_offset:      %u\n", store->center_time_offset);
    fprintf(stderr, "dbg4       heave_external:          %f\n", store->heave_external);
    fprintf(stderr, "dbg4       user_defined_byte:       %u\n", store->user_defined_byte);
    fprintf(stderr, "dbg4       altitude:                %f\n", store->altitude);
    fprintf(stderr, "dbg4       external_sensor_flags:   %u\n", store->external_sensor_flags);
    fprintf(stderr, "dbg4       pitch_external:          %f\n", store->pitch_external);
    fprintf(stderr, "dbg4       roll_external:           %f\n", store->roll_external);
    fprintf(stderr, "dbg4       heading_external:        %f\n", store->heading_external);
    fprintf(stderr, "dbg4       transmit_scan_flag:      %u\n", store->transmit_scan_flag);
    fprintf(stderr, "dbg4       transmit_scan_angle:     %f\n", store->transmit_scan_angle);
    for (int i = 0; i < store->num_beams; i++)
      fprintf(stderr, "dbg4       %d range: %d intensity: %d\n", i, store->range[i], store->intensity[i]);
    fprintf(stderr, "dbg4       sonar_depth:        %f\n", store->sonar_depth);
    fprintf(stderr, "dbg4       num_proc_beams:     %d\n", store->num_proc_beams);
    for (int i = 0; i < store->num_proc_beams; i++)
      fprintf(stderr, "dbg4       tt[%d]: %f angles:%f %f   bath: %f %f %f %d\n", i, store->beamrange[i], store->angles[i],
              store->angles_forward[i], store->bath[i], store->bathacrosstrack[i], store->bathalongtrack[i],
              store->beamflag[i]);
  }

  int status = MB_SUCCESS;

  const bool swap = false;

  /*  translate values from image83p data storage structure */
  if (store != NULL) {
    if (store->kind == MB_DATA_DATA) {
      /* header */
      index = 0;
      buffer[index] = '8';
      index += 1;
      buffer[index] = '3';
      index += 1;
      buffer[index] = 'P';
      index += 1;
      buffer[index] = (char)store->version;
      index += 1;
      write_len = 256 + 2 * store->num_beams;
      mb_put_binary_short(swap, (unsigned short)write_len, (void *)&buffer[index]);
      index += 2;
      buffer[index] = 0;
      index += 1;
      buffer[index] = 0;
      index += 1; /* index = 8 */

      /* date */
      sprintf(&buffer[index], "%2.2d-", store->time_i[2]);
      index += 3;
      switch (store->time_i[1]) {
      case (1):
        sprintf(&buffer[index], "%s", "JAN-");
        break;
      case (2):
        sprintf(&buffer[index], "%s", "FEB-");
        break;
      case (3):
        sprintf(&buffer[index], "%s", "MAR-");
        break;
      case (4):
        sprintf(&buffer[index], "%s", "APR-");
        break;
      case (5):
        sprintf(&buffer[index], "%s", "MAY-");
        break;
      case (6):
        sprintf(&buffer[index], "%s", "JUN-");
        break;
      case (7):
        sprintf(&buffer[index], "%s", "JUL-");
        break;
      case (8):
        sprintf(&buffer[index], "%s", "AUG-");
        break;
      case (9):
        sprintf(&buffer[index], "%s", "SEP-");
        break;
      case (10):
        sprintf(&buffer[index], "%s", "OCT-");
        break;
      case (11):
        sprintf(&buffer[index], "%s", "NOV-");
        break;
      case (12):
        sprintf(&buffer[index], "%s", "DEC-");
        break;
      }
      index += 4;
      sprintf(&buffer[index], "%4.4d", store->time_i[0]);
      index += 4;
      buffer[index] = 0;
      index += 1; /* index = 20 */

      /* time */
      sprintf(&buffer[index], "%2.2d:%2.2d:%2.2d", store->time_i[3], store->time_i[4], store->time_i[5]);
      index += 8;
      buffer[index] = 0;
      index += 1; /* index = 29 */

      /* hundredths of seconds */
      seconds_hundredths = store->time_i[6] / 10000;
      sprintf(&buffer[index], ".%2.2d", seconds_hundredths);
      index += 3;
      buffer[index] = 0;
      index += 1; /* index = 33 */

      /* latitude*/
      if (store->nav_lat > 0.0)
        NorS = 'N';
      else
        NorS = 'S';
      degrees = (int)fabs(store->nav_lat);
      minutes = (fabs(store->nav_lat) - (double)degrees) * 60.0;
      sprintf(&buffer[index], "_%2.2d.%8.5f_%c", degrees, minutes, NorS);
      index += 14; /* index = 47 */

      /* longitude*/
      if (store->nav_long > 0.0)
        NorS = 'E';
      else
        NorS = 'W';
      degrees = (int)fabs(store->nav_long);
      minutes = (fabs(store->nav_long) - (double)degrees) * 60.0;
      sprintf(&buffer[index], "%3.3d.%8.5f_%c", degrees, minutes, NorS);
      index += 14; /* index = 61 */

      /* speed */
      buffer[index] = store->nav_speed;
      index += 1; /* index = 62 */

      /* heading*/
      mb_put_binary_short(swap, (unsigned short)store->course, (void *)&buffer[index]);
      index += 2; /* index = 64 */

      /* pitch */
      mb_put_binary_short(swap, (unsigned short)store->pitch, (void *)&buffer[index]);
      if (store->pitch != 0)
        buffer[index] = buffer[index] | 0x80;
      index += 2; /* index = 66 */

      /* roll */
      mb_put_binary_short(swap, (unsigned short)store->roll, (void *)&buffer[index]);
      if (store->roll != 0)
        buffer[index] = buffer[index] | 0x80;
      index += 2; /* index = 68 */

      /* heading */
      mb_put_binary_short(swap, (unsigned short)store->heading, (void *)&buffer[index]);
      if (store->heading != 0)
        buffer[index] = buffer[index] | 0x80;
      index += 2; /* index = 70 */

      /* beams */
      mb_put_binary_short(swap, (unsigned short)store->num_beams, (void *)&buffer[index]);
      index += 2; /* index = 72 */
      mb_put_binary_short(swap, (unsigned short)store->samples_per_beam, (void *)&buffer[index]);
      index += 2; /* index = 74 */
      mb_put_binary_short(swap, (unsigned short)store->sector_size, (void *)&buffer[index]);
      index += 2; /* index = 76 */
      mb_put_binary_short(swap, (unsigned short)store->start_angle, (void *)&buffer[index]);
      index += 2; /* index = 78 */
      buffer[index] = store->angle_increment;
      index += 1; /* index = 79 */
      mb_put_binary_short(swap, (unsigned short)store->acoustic_range, (void *)&buffer[index]);
      index += 2; /* index = 81 */
      mb_put_binary_short(swap, (unsigned short)store->acoustic_frequency, (void *)&buffer[index]);
      index += 2; /* index = 83 */
      mb_put_binary_short(swap, (unsigned short)store->sound_velocity, (void *)&buffer[index]);
      if (store->sound_velocity != 0)
        buffer[index] = buffer[index] | 0x80;
      index += 2; /* index = 85 */
      mb_put_binary_short(swap, (unsigned short)store->range_resolution, (void *)&buffer[index]);
      index += 2; /* index = 87 */
      mb_put_binary_short(swap, (unsigned short)store->pulse_length, (void *)&buffer[index]);
      index += 2; /* index = 89 */
      mb_put_binary_short(swap, (unsigned short)store->profile_tilt_angle, (void *)&buffer[index]);
      index += 2; /* index = 91 */
      mb_put_binary_short(swap, (unsigned short)store->rep_rate, (void *)&buffer[index]);
      index += 2; /* index = 93 */
      mb_put_binary_int(swap, store->ping_number, (void *)&buffer[index]);
      index += 4; /* index = 97 */

      /* if version 1.10 write the rest of the header parameters */
      if (store->version >= 10) {
        index = 100;

        mb_put_binary_float(swap, (float)store->sonar_x_offset, (void *)&buffer[index]);
        index += 4; /* index = 104 */

        mb_put_binary_float(swap, (float)store->sonar_y_offset, (void *)&buffer[index]);
        index += 4; /* index = 108 */

        mb_put_binary_float(swap, (float)store->sonar_z_offset, (void *)&buffer[index]);
        index += 4; /* index = 112 */

        /* replace centiseconds of timestamp already parsed with higher resolution milliseconds encoded here */
        int milliseconds = store->time_i[6] / 1000;
        sprintf(&buffer[index], ".%3.3d", milliseconds);
        index += 4;
        buffer[index] = 0;
        index += 1; /* index = 117 */

        buffer[index] = (char) store->has_intensity;
        index += 1; /* index = 118 */

        mb_put_binary_short(swap, (unsigned short)store->ping_latency, (void *)&buffer[index]);
        index += 2; /* index = 120 */

        mb_put_binary_short(swap, (unsigned short) store->data_latency, (void *)&buffer[index]);
        index += 2; /* index = 122 */

        buffer[index] = (char)store->sample_rate;
        index += 1; /* index = 123 */

        buffer[index] = (char)store->option_flags;
        index += 1; /* index = 124 */

        index += 1; /* index = 125 */

        buffer[index] = (char)store->number_averaged;
        index += 1; /* index = 126 */

        mb_put_binary_short(swap, (unsigned short)store->center_time_offset, (void *)&buffer[index]);
        index += 2; /* index = 128 */

        mb_put_binary_float(swap, (float)(store->heave_external), (void *)&buffer[index]);
        index += 4; /* index = 132 */

        buffer[index] = (char)store->user_defined_byte;
        index += 1; /* index = 133 */

        mb_put_binary_float(swap, (float)store->altitude, (void *)&buffer[index]);
        index += 4; /* index = 137 */

        buffer[index] = (char)store->external_sensor_flags;
        index += 1; /* index = 138 */

        mb_put_binary_float(swap, (float)store->pitch_external, (void *)&buffer[index]);
        index += 4; /* index = 142 */

        mb_put_binary_float(swap, (float)store->roll_external, (void *)&buffer[index]);
        index += 4; /* index = 146 */

        mb_put_binary_float(swap, (float)store->heading_external, (void *)&buffer[index]);
        index += 4; /* index = 150 */

        buffer[index] = store->transmit_scan_flag;
        index += 1; /* index = 151 */

        mb_put_binary_float(swap, (float)store->transmit_scan_angle, (void *)&buffer[index]);
        index += 4; /* index = 155 */
      }

      /* fill remainder of header with zeros */
      for (int i = index; i <= 255; i++)
        buffer[i] = 0;
      index = 256;

      /* put ranges */
      for (int i = 0; i < store->num_beams; i++) {
        mb_put_binary_short(swap, (unsigned short)store->range[i], &buffer[index]);
        index += 2;
      }

      /* put amplitudes */
      if (store->has_intensity) {
        for (int i = 0; i < store->num_beams; i++) {
          mb_put_binary_short(swap, (unsigned short)store->intensity[i], &buffer[index]);
          index += 2;
        }
      }
    }

    else if (store->kind == MB_DATA_COMMENT) {
      /* header */
      index = 0;
      buffer[index] = '8';
      index += 1;
      buffer[index] = '3';
      index += 1;
      buffer[index] = 'P';
      index += 1;
      buffer[index] = (char)store->version;
      index += 1;
      write_len = 256;
      mb_put_binary_short(swap, (unsigned short)write_len, (void *)&buffer[index]);
      index += 2;
      buffer[index] = '#';
      index += 1;
      buffer[index] = '#';
      index += 1; /* index = 8 */

      /* write comment */
      strncpy(&buffer[index], store->comment, MBSYS_IMAGE83P_COMMENTLEN);
      for (int i = 8 + strlen(store->comment); i < 8 + MBSYS_IMAGE83P_COMMENTLEN; i++)
        buffer[i] = 0;
    }

    /* write next record to file */
    if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_COMMENT) {
      if ((status = fwrite(buffer, 1, write_len, mb_io_ptr->mbfp)) == write_len) {
        status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
      }
      else {
        status = MB_FAILURE;
        *error = MB_ERROR_WRITE_FAIL;
      }
    }
    else {
      status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
      if (verbose >= 5)
        fprintf(stderr, "\ndbg5  No data written in MBIO function <%s>\n", __func__);
    }
  }

  else {
    status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    if (verbose >= 5)
      fprintf(stderr, "\ndbg5  No data written in MBIO function <%s>\n", __func__);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_register_image83p(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_image83p(
      verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
      mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
      &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
      &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
      &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_image83p;
  mb_io_ptr->mb_io_format_free = &mbr_dem_image83p;
  mb_io_ptr->mb_io_store_alloc = &mbsys_image83p_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_image83p_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_image83p;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_image83p;
  mb_io_ptr->mb_io_dimensions = &mbsys_image83p_dimensions;
  mb_io_ptr->mb_io_pingnumber = &mbsys_image83p_pingnumber;
  mb_io_ptr->mb_io_sonartype = &mbsys_image83p_sonartype;
  mb_io_ptr->mb_io_sidescantype = NULL;
  mb_io_ptr->mb_io_preprocess = &mbsys_image83p_preprocess;
  mb_io_ptr->mb_io_extract_platform = &mbsys_image83p_extract_platform;
  mb_io_ptr->mb_io_extract = &mbsys_image83p_extract;
  mb_io_ptr->mb_io_insert = &mbsys_image83p_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_image83p_extract_nav;
  mb_io_ptr->mb_io_extract_nnav = NULL;
  mb_io_ptr->mb_io_insert_nav = &mbsys_image83p_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_image83p_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = NULL;
  mb_io_ptr->mb_io_insert_svp = NULL;
  mb_io_ptr->mb_io_ttimes = &mbsys_image83p_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_image83p_detects;
  mb_io_ptr->mb_io_gains = NULL;
  mb_io_ptr->mb_io_copyrecord = &mbsys_image83p_copy;
  mb_io_ptr->mb_io_makess = NULL;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;
  mb_io_ptr->mb_io_extract_segytraceheader = NULL;
  mb_io_ptr->mb_io_extract_segy = NULL;
  mb_io_ptr->mb_io_insert_segy = NULL;
  mb_io_ptr->mb_io_ctd = NULL;
  mb_io_ptr->mb_io_ancilliarysensor = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
    fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
    fprintf(stderr, "dbg2       format_alloc:       %p\n", (void *)mb_io_ptr->mb_io_format_alloc);
    fprintf(stderr, "dbg2       format_free:        %p\n", (void *)mb_io_ptr->mb_io_format_free);
    fprintf(stderr, "dbg2       store_alloc:        %p\n", (void *)mb_io_ptr->mb_io_store_alloc);
    fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
    fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
    fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
    fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
    fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
    fprintf(stderr, "dbg2       extract_nav:        %p\n", (void *)mb_io_ptr->mb_io_extract_nav);
    fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
    fprintf(stderr, "dbg2       extract_altitude:   %p\n", (void *)mb_io_ptr->mb_io_extract_altitude);
    fprintf(stderr, "dbg2       insert_altitude:    %p\n", (void *)mb_io_ptr->mb_io_insert_altitude);
    fprintf(stderr, "dbg2       extract_svp:        %p\n", (void *)mb_io_ptr->mb_io_extract_svp);
    fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
    fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
    fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
    fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
