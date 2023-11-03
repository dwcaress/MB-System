/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2100bi.c	12/23/2004
 *
 *    Copyright (c) 1997-2023 by
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
 * mbr_sb2100bi.c contains the functions for reading and writing
 * multibeam data in the SB2100BI format.
 * These functions include:
 *   mbr_alm_sb2100bi	- allocate read/write memory
 *   mbr_dem_sb2100bi	- deallocate read/write memory
 *   mbr_rt_sb2100bi	- read and translate data
 *   mbr_wt_sb2100bi	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 *              (Original files:
 *                 mbr_sb2100b1.c
 *                 mbf_sb2100b1.h
 *                 mbr_sb2100b2.c
 *                 mbf_sb2100b2.h)
 *
 *
 * Date:	December 23, 2003
 *              (New file:
 *                 mbr_sb2100bi.c)
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_sb2100.h"

/* define id's for the different types of raw records */
static const int MBF_SB2100BI_RECORDS = 6;
static const int MBF_SB2100BI_NONE = 0;
static const int MBF_SB2100BI_FH = 1;
static const int MBF_SB2100BI_TR = 2;
static const int MBF_SB2100BI_PR = 3;
static const int MBF_SB2100BI_DH = 4;
static const int MBF_SB2100BI_BR = 5;
static const int MBF_SB2100BI_SR = 6;
char *mbf_sb2100bi_labels[] = {"NONE    ", "SB21BIFH", "SB21BITR", "SB21BIPR", "SB21BIDH", "SB21BIBR", "SB21BISR"};

static const int MBF_SB2100BI_PR_WRITE_LEN = 284;
static const int MBF_SB2100BI_DH_WRITE_LEN = 80;
static const int MBF_SB2100BI_BR_WRITE_LEN = 32;
static const int MBF_SB2100BI_SR_WRITE_LEN = 4;
static const int MBF_SB2100BI_LABEL_LEN = 8;

/* define end of record label */
static const char mbf_sb2100bi_eor[2] = {'\r', '\n'};

/* text for ascii file header */
static const char *mbf_sb2100bi_file_header_text_1 = {"\
\nSeaBeam 2100 multibeam sonar binary data format\n\
MB-System formats 42 and 43\n\
Format specification 1.2 defined March 20, 1997\n\
David W. Caress\n\
SeaBeam Instruments, Inc.\n\
\n\
Format specification 1.3 updated December 23, 2003\n\
David W. Caress\n\
Monterey Bay Aquarium Research Institute.\n\
\n\
Format 42 is a binary data format for storing all \n\
bathymetry and sidescan data obtained from a SeaBeam\n\
2100 multibeam sonar. Each file consists of an\n\
ASCII file header followed by a series of binary data records.\n\
All binary integer and float values are \"big-endian\" ordered.\n\
All floating point values (float and double) are in the\n\
IEEE standard format.\n\
\n\
Format 43 is identical to format 42 except that the\n\
number of sidescan pixels is always set to zero.\n\
\n\
The data records are:\n\
        Sonar Text Record (comments)\n\
        Sonar Parameter Record (roll bias, pitch bias, SVP)\n\
        Sonar Data Header\n\
        Sonar Bathymetry Record\n\
        Sonar Sidescan Record\n\
\n\
All data files will begin with the ascii File Header Record.\n\
\n\
All data files created by a sonar should include a Sonar\n\
Parameter Record before any ping data. Data files originating\n\
in the original SeaBeam 2100 format may not have a Sonar\n\
Parameter Record. Sonar Text Records may occur between the \n\
File Header Record and any other data records.\n\
\n\
Each sonar ping produces three data records in the following\n\
order:\n\
        Sonar Data Header\n\
        Sonar Bathymetry Record\n\
        Sonar Sidescan Record\n\
The Sonar Bathymetry Record and Sonar Sidescan Record will\n\
appear even if the numbers of beams and/or pixels are zero.\n\
The Sonar Bathymetry Record and Sonar Sidescan Record are\n\
variable in length, depending on the number of bathymetry\n\
beams and sidescan pixels, respectively.\n\
\n\
The structure of this format is designed to maximize i/o\n\
throughput with MB-System programs. Most numeric parameters,\n\
except for the sidescan amplitude and alongtrack values,\n\
are stored as IEEE float or double values. The data records\n\
are constructed so that the records can be read directly into\n\
C structures on machines that enforce 4-byte boundaries in\n\
memory. \n\
\n\
The data record definitions follow:\n\
----------------------------------------------------------------------------\n\
\n\
File Header Record (variable length ASCII, at start of each file):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     SB21            4       0       ASCII\n\
  >>Note: binary equivalent: 1396847153\n\
Record ID 2                     BIFH            4       4       ASCII\n\
  >>Note: binary equivalent: 1112098376\n\
Record Length   bytes           0 - 999999      6       8       ASCII\n\
  >>Note: Length of header text plus Record End in bytes Header\n\
\n\
Text                                            varies  14      ASCII Record\n\
\n\
Record End                      [CR][LF]        2       varies  ASCII\n\
----------------------------------------------------------------------------\n\
\n\
Sonar Text Record (variable length - comments derived from sonar or in processing):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     1396847153      4       0       unsigned int\n\
  >>Note: ASCII equivalent: \"SB21\"\n\
Record ID 2                     1112101970      4       4       unsigned int\n\
  >>Note: ASCII equivalent: \"BITR\"\n\
Record Length   bytes           6 - 1926        2       8       int\n\
  >>Note: Length of the rest of the record in bytes\n\
\n\
Comment text                                    varies  10      ASCII\n\
  >>Note: The comment string is null terminated unless it is 1920 bytes long.\n\
\n\
Checksum                                        4       varies  unsigned int\n\
Record End                      03338           2       varies  unsigned short\n\
  >>Note: ASCII equivalent: \"[CR][LF]\"\n\
----------------------------------------------------------------------------\n\
\n\
Sonar Parameter Record (300 bytes - roll bias, pitch bias, SVP):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     1396847153      4       0       unsigned int\n\
  >>Note: ASCII equivalent: \"SB21\"\n\
Record ID 2                     1112100946      4       4       unsigned int\n\
  >>Note: ASCII equivalent: \"BIPR\"\n\
Record Length   bytes           290             2       8       short\n\
  >>Note: Length of the rest of the record in bytes\n\
\n\
Year                            1994 -          2       10      short\n\
Day                             1 - 366         2       12      short\n\
  >>Note: Day of year\n\
Hour                            0 - 23          2       14      short\n\
Minute                          0 - 59          2       16      short\n\
Second                          0 - 59          2       18      short\n\
Millisecond                     0 - 999         2       20      short\n\
\n\
Roll bias port  degree          -5.0 - +5.0     4       22      float\n\
  >>Note: signed so + is port up\n\
Roll bias stbd  degree          -5.0 - +5.0     4       26      float\n\
  >>Note: signed so + is port up\n\
Pitch bias      degree          -5.0 - +5.0     4       30      float\n\
  >>Note: signed so + is stern up\n\
\n\
Ship's draft    m               0.0 - 10.0      4       34      float\n\
Nav X offset    m               -50.0 - 50.0    4       38      float\n\
Nav Y offset    m               -100.0 - 100.0  4       42      float\n\
Nav Z offset    m               0.0 - 20.0      4       46      float\n\
\n\
# of SVP points                 2 - 30          4       50      int\n\
\n\
SVP depth[0]    m               0 - 12000.0     4       54      float\n\
SVP depth[1]    m               0 - 12000.0     4       58      float\n\
.........\n\
SVP depth[29]   m               0 - 12000.0     4       170     float\n\
\n\
SVP velocity[0] m/s             1300.0 - 1700.0 4       174      float\n\
SVP velocity[1] m/s             1300.0 - 1700.0 4       178      float\n\
.........\n\
SVP velocity[29] m/s            1300.0 - 1700.0 4       290     float\n\
\n\
Checksum                                        4       294     unsigned int\n\
Record End                      03338           2       298     unsigned short\n\
  >>Note: ASCII equivalent: \"[CR][LF]\"\n\
----------------------------------------------------------------------------\n\
\n\
"};

static const char *mbf_sb2100bi_file_header_text_2 = {"\
Sonar Data Header Record (96 bytes - navigation and sonar parameters):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     1396847153      4       0       unsigned int\n\
  >>Note: ASCII equivalent: \"SB21\"\n\
Record ID 2                     1112097864      4       4       unsigned int\n\
  >>Note: ASCII equivalent: \"BIDH\"\n\
Record Length    bytes          86              2       8       short\n\
  >>Note: Length of the rest of the record in bytes\n\
\n\
Year                            1994 -          2       10      short\n\
Day                             1 - 366         2       12      short\n\
  >>Note: Day of year\n\
Hour                            0 - 23          2       14      short\n\
Minute                          0 - 59          2       16      short\n\
Second                          0 - 59          2       18      short\n\
Millisecond                     0 - 999         2       20      short\n\
\n\
Spare                           0               2       22      short\n\
Spare                           0               2       24      short\n\
Longitude       degree E of 0E  0.0 - 359.99999 8       26      double\n\
Latitude        degree N of 0N  -90.0 - +90.0   8       34      double\n\
\n\
Heading at ping degree          0.0 - 359.999   4       42      float\n\
Speed           knot            0.0 - 100.0     4       46      float\n\
Roll at ping    degree          -45.0 - +45.0   4       50      float\n\
  >>Note: + = port up\n\
Pitch at ping   degree          -10.0 - +10.0   4       54      float\n\
  >>Note: + = stern up\n\
Heave at ping   m               -10.0 - +10.0   4       58      float\n\
  >>Note: + = above mean level\n\
Surface sound   m/s             1435.0 - 1565.0 4       62      float\n\
  velocity\n\
\n\
Frequency       kHz             L, H            1       66      char\n\
  >>Note: L = 12 kHz, H = 36 kHz\n\
Depth gate mode                 A, M            1       67      char\n\
  >>Note: A = auto, M = manual\n\
Ping gain       dB              0 - 45          1       68      unsigned char\n\
Ping pulse      0.001 s         1 - 20          1       69      unsigned char\n\
  width\n\
Transmitter     dB              0 - 18          1       70      unsigned char\n\
  attenuation\n\
SSV source                      V, M, T, E, U   1       71      char\n\
  >>Note: V = velocimeter, M = manual, T = temperature, \n\
          E = external, U = unknown\n\
SVP correction                  0, T            1       72      char\n\
  >>Note: 0 = None, T = true depth and true position\n\
\n\
Pixel intensity                 D, L            1       73      char\n\
  algorithm\n\
  >>Note: D = logarithm, L = linear; should always be linear for new data\n\
Pixel size      m               0.125 - 20.0    4       74      float\n\
\n\
Number of beams                 0 - 151         2       78      short\n\
Number of pixels                0 - 2000        2       80      short\n\
\n\
Spare                           0               2       82      short\n\
Spare                           0               2       84      short\n\
Spare                           0               2       86      short\n\
Spare                           0               2       88      short\n\
\n\
Checksum                                        4       90      unsigned int\n\
Record End                      03338           2       94      unsigned short\n\
  >>Note: ASCII equivalent: \"[CR][LF]\"\n\
----------------------------------------------------------------------------\n\
\n\
Sonar Data Bathymetry Record (variable length - (16 + nbeams * 32) bytes \n\
                              - bathymetry and amplitude):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     1396847153      4       0       unsigned int\n\
  >>Note: ASCII equivalent: \"SB21\"\n\
Record ID 2                     1112097362      4       4       unsigned int\n\
  >>Note: ASCII equivalent: \"BIBR\"\n\
Record Length   bytes           6 - 4838        2       8       short\n\
  >>Note: Length of the rest of the record in bytes\n\
\n\
  >>Note: This record contains \"number of beams\" instances \n\
          of the following structure, where the \"number of beams\" \n\
          value is found near the end of the preceding \n\
          SB21BIDH header record\n\
Depth           m                               4       10      float\n\
Acrosstrack     m                               4       14      float\n\
  >>Note: + is starboard\n\
Alongtrack      m                               4       18      float\n\
  >>Note: + is forward\n\
Range           sec                             4       22      float\n\
  >>Note: raw round trip echo time\n\
Angle from      degree          -100.0 - +100.0 4       26      float\n\
  vertical\n\
  >>Note: see SeaBeam documentation figure for meaning of signs\n\
Angle forward   degree          -100.0 - +100.0 4       30      float\n\
  >>Note: see SeaBeam documentation figure for meaning of signs\n\
Beam amplitude  0.25 dB         0 - 400         2       34      short\n\
Signal to noise dB              0 - 99          2       36      short\n\
Echo length     sample interval 0 - 999         2       38      short\n\
Signal quality                  0, Q, F, G      1       40      char\n\
  >>Note: 0 = no data, Q = sonar flagged, \n\
          F = processing flagged, G = good data\n\
Beam algorithm                  W, B            1       41      char\n\
  >>Note: W = weighted mean time, B = BDI\n\
\n\
  >>Note: The usual checksum and record end tag are placed after\n\
          the last instance of the per-beam data\n\
Checksum                                        4       varies  unsigned int\n\
Record End                      03338           2       varies  unsigned short\n\
  >>Note: ASCII equivalent: \"[CR][LF]\"\n\
----------------------------------------------------------------------------\n\
\n\
Sonar Data Sidescan Record (variable length - (16 + 4 * npixels) bytes \n\
                            - 2000 pixels : 8016 bytes\n\
                            - sidescan):\n\
----------------------------------------------------------------------------\n\
Item            Units           Valid           # of    Byte    Coding\n\
                                Range           Bytes   Offset  \n\
----------------------------------------------------------------------------\n\
Record ID 1                     1396847153      4       0       unsigned int\n\
  >>Note: ASCII equivalent: \"SB21\"\n\
Record ID 2                     1112101714      4       4       unsigned int\n\
  >>Note: ASCII equivalent: \"BISR\"\n\
Record Length   bytes           6 - 16006       2       8       short\n\
  >>Note: Length of the rest of the record in bytes\n\
\n\
  >>Note: This record contains \"number of pixels\" instances \n\
          of the following structure, where the \"number of pixels\" \n\
          value is found near the end of the preceding \n\
          SB21BIDH header record\n\
  >>Note: The \"number of pixels\" is always 0 for format 43.\n\
\n\
Amplitude                       0 - 65535       2       10      unsigned short\n\
Alongtrack      0.1 m           -32767 - 32767  2       12      short\n\
  >>Note: + is forward\n\
\n\
  >>Note: The usual checksum and record end tag are placed after\n\
          the last instance of the per-pixel data\n\
Checksum                                        4       varies  unsigned int\n\
Record End                      03338           2       varies  unsigned short\n\
  >>Note: ASCII equivalent: \"[CR][LF]\"\n\
----------------------------------------------------------------------------\n\
\n\r\n\
"};

/*--------------------------------------------------------------------*/
int mbr_info_sb2100b1(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SB2100;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 2000;
	strncpy(format_name, "SB2100B1", MB_NAME_LENGTH);
	strncpy(system_name, "SB2100", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SB2100B1\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           "
	        "SeaBeam 2100, bathymetry, amplitude \n                      and sidescan, 151 beams bathymetry,\n                   "
	        "   2000 pixels sidescan, binary,\n                      SeaBeam Instruments and L-DEO.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

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
int mbr_info_sb2100b2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SB2100;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 0;
	strncpy(format_name, "SB2100B2", MB_NAME_LENGTH);
	strncpy(system_name, "SB2100", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SB2100B2\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           "
	        "SeaBeam 2100, bathymetry and amplitude,  \n                      151 beams bathymetry,\n                      "
	        "binary,\n                      SeaBeam Instruments and L-DEO.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

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
int mbr_zero_sb2100bi(int verbose, char *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store_ptr: %p\n", (void *)store_ptr);
	}

	/* get pointer to data descriptor */
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)store_ptr;

	/* initialize everything to zeros */
	if (store != NULL) {
		/* type of data record */
		store->kind = MB_DATA_NONE;

		/* set comment pointer */
		store->comment = (char *)&store->roll_bias_port;

		/* sonar parameters (SB21BIPR) */
		store->roll_bias_port = 0.0;      /* deg */
		store->roll_bias_starboard = 0.0; /* deg */
		store->pitch_bias = 0.0;          /* deg */
		store->ship_draft = 0.0;          /* m */
		store->offset_x = 0.0;            /* m */
		store->offset_y = 0.0;            /* m */
		store->offset_z = 0.0;            /* m */
		store->num_svp = 0;
		for (int i = 0; i < MBSYS_SB2100_MAXVEL; i++) {
			store->svp[i].depth = 0.0;
			store->svp[i].velocity = 0.0;
		}

		/* sonar data header (SB21BIDH) */
		store->year = 0;
		store->jday = 0;
		store->hour = 0;
		store->minute = 0;
		store->sec = 0;
		store->msec = 0;
		store->spare1 = 0;
		store->spare2 = 0;
		store->longitude = 0.0;             /* degrees */
		store->latitude = 0.0;              /* degrees */
		store->heading = 0.0;               /* degrees */
		store->speed = 0.0;                 /* m/sec */
		store->roll = 0.0;                  /* degrees */
		store->pitch = 0.0;                 /* degrees */
		store->heave = 0.0;                 /* m */
		store->ssv = 0.0;                   /* m/sec */
		store->frequency = 'L';             /* L=12kHz; H=36kHz; 2=20kHz */
		store->depth_gate_mode = 'A';       /* A=Auto, M=Manual */
		store->ping_gain = 0;               /* dB */
		store->ping_pulse_width = 0;        /* msec */
		store->transmitter_attenuation = 0; /* dB */
		store->ssv_source = 'M';            /* V=Velocimeter, M=Manual,
		                        T=Temperature */
		store->svp_correction = 'T'; /* 0=None; A=True Xtrack
		                 and Apparent Depth;
		                 T=True Xtrack and True Depth */
		store->pixel_algorithm = 'L'; /* pixel intensity algorithm
		                  D = logarithm, L = linear */
		store->pixel_size = 0.0;      /* m */
		store->nbeams = 0;            /* up to 151 */
		store->npixels = 0;           /* up to 2000 */
		store->spare3 = 0;
		store->spare4 = 0;
		store->spare5 = 0;
		store->spare6 = 0;

		/* bathymetry record (SB21BIBR) */
		for (int i = 0; i < MBSYS_SB2100_BEAMS; i++) {
			store->beams[i].depth = 0.0;         /* m */
			store->beams[i].acrosstrack = 0.0;   /* m */
			store->beams[i].alongtrack = 0.0;    /* m */
			store->beams[i].range = 0.0;         /* seconds */
			store->beams[i].angle_across = 0.0;  /* degrees */
			store->beams[i].angle_forward = 0.0; /* degrees */
			store->beams[i].amplitude = 0;       /* 0.25 dB */
			store->beams[i].signal_to_noise = 0; /* dB */
			store->beams[i].echo_length = 0;     /* samples */
			store->beams[i].quality = '0'; /* 0=no data,
			               Q=poor quality,
			               blank otherwise */
			store->beams[i].source = 'W'; /* B=BDI, W=WMT */
		}

		/* sidescan record (SB21BISR) */
		for (int i = 0; i < MBSYS_SB2100_PIXELS; i++) {
			store->pixels[i].amplitude = 0;
			store->pixels[i].alongtrack = 0; /* 0.1 m */
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_alm_sb2100bi(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbsys_sb2100_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_sb2100_struct), &mb_io_ptr->store_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, 4 * MBSYS_SB2100_PIXELS, &mb_io_ptr->saveptr1, error);

	/* get store structure pointer */
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)mb_io_ptr->store_data;

	/* set comment pointer */
	store->comment = (char *)&(store->roll_bias_port);

	/* initialize everything to zeros */
	mbr_zero_sb2100bi(verbose, mb_io_ptr->raw_data, error);

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
int mbr_dem_sb2100bi(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->saveptr1, error);

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
int mbr_sb2100bi_rd_fh(int verbose, FILE *mbfp, char *buffer, int record_length, int *error) {
	int status = MB_SUCCESS;
	int nread;
	int nlast;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length > 100000) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read data into buffer */
		nread = record_length / 100;
		nlast = record_length % 100;
		for (int i = 0; i < nread; i++)
			if ((status = fread(buffer, 100, 1, mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		if (nlast > 0)
			if ((status = fread(buffer, nlast, 1, mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
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
int mbr_sb2100bi_rd_pr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, short record_length, int *error) {
	int status = MB_SUCCESS;
	int read_length = 0;
	unsigned int checksum_read = 0;
	unsigned int checksum = 0;
	char eor_read[6];
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length != MBF_SB2100BI_PR_WRITE_LEN + 6) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read data into structure */
		read_length = MBF_SB2100BI_PR_WRITE_LEN;
		if ((status = fread(buffer, 1, MBF_SB2100BI_PR_WRITE_LEN, mbfp)) != MBF_SB2100BI_PR_WRITE_LEN) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		index = 0;
		mb_get_binary_short(false, &buffer[index], &store->year);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->jday);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->hour);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->minute);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->sec);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->msec);
		index += 2;
		mb_get_binary_float(false, &buffer[index], &store->roll_bias_port);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->roll_bias_starboard);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->pitch_bias);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->ship_draft);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->offset_x);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->offset_y);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->offset_z);
		index += 4;
		mb_get_binary_int(false, &buffer[index], &store->num_svp);
		index += 4;
		for (int i = 0; i < store->num_svp; i++) {
			mb_get_binary_float(false, &buffer[index], &(store->svp[i].depth));
			index += 4;
			mb_get_binary_float(false, &buffer[index], &(store->svp[i].velocity));
			index += 4;
		}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0], 6, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		mb_get_binary_int(false, &eor_read[0], (int *)&checksum_read);

		/* do checksum */
		if (verbose >= 2) {
			checksum = 0;
			for (int i = 0; i < read_length; i++)
				checksum += (unsigned int)buffer[i];

			/* check checksum and report */
			fprintf(stderr, "\ndbg5  Checksum test done in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       checksum read:       %d\n", checksum_read);
			fprintf(stderr, "dbg5       checksum calculated: %d\n", checksum);
			if (checksum != checksum_read) {
				fprintf(stderr, "dbg5       CHECKSUM ERROR!!\n");
			}
			else {
				fprintf(stderr, "dbg5       checksum ok\n");
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", store->year);
		fprintf(stderr, "dbg5       julian day:       %d\n", store->jday);
		fprintf(stderr, "dbg5       hour:             %d\n", store->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", store->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", store->sec);
		fprintf(stderr, "dbg5       msec:             %d\n", store->msec);
		fprintf(stderr, "dbg5       roll_bias_port:   %f\n", store->roll_bias_port);
		fprintf(stderr, "dbg5       roll_bias_strbrd: %f\n", store->roll_bias_starboard);
		fprintf(stderr, "dbg5       pitch_bias:       %f\n", store->pitch_bias);
		fprintf(stderr, "dbg5       ship_draft:       %f\n", store->ship_draft);
		fprintf(stderr, "dbg5       offset_x:         %f\n", store->offset_x);
		fprintf(stderr, "dbg5       offset_y:         %f\n", store->offset_y);
		fprintf(stderr, "dbg5       offset_z:         %f\n", store->offset_z);
		fprintf(stderr, "dbg5       num_svp:          %d\n", store->num_svp);
		fprintf(stderr, "dbg5       Sound Velocity Profile:\n");
		for (int i = 0; i < store->num_svp; i++)
			fprintf(stderr, "dbg5       %d  depth:%f  velocity:%f\n", i, store->svp[i].depth, store->svp[i].velocity);
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
int mbr_sb2100bi_rd_tr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, short record_length, int *error) {
	int status = MB_SUCCESS;
	int read_length = 0;
	unsigned int checksum_read = 0;
	unsigned int checksum = 0;
	char eor_read[6];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length > MBSYS_SB2100_MAXLINE + 6) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read data into structure */
		read_length = record_length - 6;
		if ((status = fread(store->comment, read_length, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0], 6, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		mb_get_binary_int(false, &eor_read[0], (int *)&checksum_read);

		/* do checksum */
		if (verbose >= 2) {
			checksum = 0;
			for (int i = 0; i < read_length; i++)
				checksum += (unsigned int)buffer[i];

			/* check checksum and report */
			fprintf(stderr, "\ndbg5  Checksum test done in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       checksum read:       %d\n", checksum_read);
			fprintf(stderr, "dbg5       checksum calculated: %d\n", checksum);
			if (checksum != checksum_read) {
				fprintf(stderr, "dbg5       CHECKSUM ERROR!!\n");
			}
			else {
				fprintf(stderr, "dbg5       checksum ok\n");
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Value read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", store->comment);
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
int mbr_sb2100bi_rd_dh(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, short record_length, int *error) {
	int status = MB_SUCCESS;
	int read_length = 0;
	unsigned int checksum_read = 0;
	unsigned int checksum = 0;
	char eor_read[6];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length != MBF_SB2100BI_DH_WRITE_LEN + 6) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read data into structure */
		read_length = MBF_SB2100BI_DH_WRITE_LEN;
		if ((status = fread(buffer, read_length, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		index = 0;
		mb_get_binary_short(false, &buffer[index], &store->year);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->jday);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->hour);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->minute);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->sec);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->msec);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->spare1);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->spare2);
		index += 2;
		mb_get_binary_double(false, &buffer[index], &store->longitude);
		index += 8;
		mb_get_binary_double(false, &buffer[index], &store->latitude);
		index += 8;
		mb_get_binary_float(false, &buffer[index], &store->heading);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->speed);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->roll);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->pitch);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->heave);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &store->ssv);
		index += 4;
		store->frequency = buffer[index];
		index++;
		store->depth_gate_mode = buffer[index];
		index++;
		store->ping_gain = buffer[index];
		index++;
		store->ping_pulse_width = buffer[index];
		index++;
		store->transmitter_attenuation = buffer[index];
		index++;
		store->ssv_source = buffer[index];
		index++;
		store->svp_correction = buffer[index];
		index++;
		store->pixel_algorithm = buffer[index];
		index++;
		mb_get_binary_float(false, &buffer[index], &store->pixel_size);
		index += 4;
		mb_get_binary_int(false, &buffer[index], &store->nbeams);
		index += 4;
		mb_get_binary_int(false, &buffer[index], &store->npixels);
		index += 4;
		mb_get_binary_short(false, &buffer[index], &store->spare3);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->spare4);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->spare5);
		index += 2;
		mb_get_binary_short(false, &buffer[index], &store->spare6);
		index += 2;

		/* read checksum and eor */
		if ((status = fread(&eor_read[0], 6, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		mb_get_binary_int(false, &eor_read[0], (int *)&checksum_read);

		/* do checksum */
		if (verbose >= 2) {
			checksum = 0;
			for (int i = 0; i < read_length; i++)
				checksum += (unsigned int)buffer[i];

			/* check checksum and report */
			fprintf(stderr, "\ndbg5  Checksum test done in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       checksum read:       %d\n", checksum_read);
			fprintf(stderr, "dbg5       checksum calculated: %d\n", checksum);
			if (checksum != checksum_read) {
				fprintf(stderr, "dbg5       CHECKSUM ERROR!!\n");
			}
			else {
				fprintf(stderr, "dbg5       checksum ok\n");
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", store->year);
		fprintf(stderr, "dbg5       julian day:       %d\n", store->jday);
		fprintf(stderr, "dbg5       hour:             %d\n", store->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", store->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", store->sec);
		fprintf(stderr, "dbg5       msec:             %d\n", store->msec);
		fprintf(stderr, "dbg5       longitude:        %f\n", store->longitude);
		fprintf(stderr, "dbg5       latitude:         %f\n", store->latitude);
		fprintf(stderr, "dbg5       heading:          %f\n", store->heading);
		fprintf(stderr, "dbg5       speed:            %f\n", store->speed);
		fprintf(stderr, "dbg5       roll:             %f\n", store->roll);
		fprintf(stderr, "dbg5       pitch:            %f\n", store->pitch);
		fprintf(stderr, "dbg5       heave:            %f\n", store->heave);
		fprintf(stderr, "dbg5       ssv:              %f\n", store->ssv);
		fprintf(stderr, "dbg5       frequency:        %c\n", store->frequency);
		fprintf(stderr, "dbg5       depth_gate_mode:  %c\n", store->depth_gate_mode);
		fprintf(stderr, "dbg5       ping_gain:        %d\n", store->ping_gain);
		fprintf(stderr, "dbg5       ping_pulse_width: %d\n", store->ping_pulse_width);
		fprintf(stderr, "dbg5       trans_atten:      %d\n", store->transmitter_attenuation);
		fprintf(stderr, "dbg5       ssv_source:       %c\n", store->ssv_source);
		fprintf(stderr, "dbg5       svp_correction:   %c\n", store->svp_correction);
		fprintf(stderr, "dbg5       pixel_algorithm:  %c\n", store->pixel_algorithm);
		fprintf(stderr, "dbg5       pixel_size:       %f\n", store->pixel_size);
		fprintf(stderr, "dbg5       nbeams:           %d\n", store->nbeams);
		fprintf(stderr, "dbg5       npixels:          %d\n", store->npixels);
		fprintf(stderr, "dbg5       spare1:           %d\n", store->spare1);
		fprintf(stderr, "dbg5       spare2:           %d\n", store->spare2);
		fprintf(stderr, "dbg5       spare3:           %d\n", store->spare3);
		fprintf(stderr, "dbg5       spare4:           %d\n", store->spare4);
		fprintf(stderr, "dbg5       spare5:           %d\n", store->spare5);
		fprintf(stderr, "dbg5       spare6:           %d\n", store->spare6);
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
int mbr_sb2100bi_rd_br(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, short record_length, int *error) {
	int status = MB_SUCCESS;
	int read_length = 0;
	unsigned int checksum_read = 0;
	unsigned int checksum = 0;
	char eor_read[6];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length != store->nbeams * MBF_SB2100BI_BR_WRITE_LEN + 6) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read data into structure */
		read_length = store->nbeams * MBF_SB2100BI_BR_WRITE_LEN;
		if (read_length > 0)
			if ((status = fread(buffer, read_length, 1, mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

		index = 0;
		for (int i = 0; i < store->nbeams; i++) {
			mb_get_binary_float(false, &buffer[index], &store->beams[i].depth);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &store->beams[i].acrosstrack);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &store->beams[i].alongtrack);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &store->beams[i].range);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &store->beams[i].angle_across);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &store->beams[i].angle_forward);
			index += 4;
			mb_get_binary_short(false, &buffer[index], &store->beams[i].amplitude);
			index += 2;
			mb_get_binary_short(false, &buffer[index], &store->beams[i].signal_to_noise);
			index += 2;
			mb_get_binary_short(false, &buffer[index], &store->beams[i].echo_length);
			index += 2;
			store->beams[i].quality = buffer[index];
			index++;
			store->beams[i].source = buffer[index];
			index++;
		}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0], 6, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		mb_get_binary_int(false, &eor_read[0], (int *)&checksum_read);

		/* do checksum */
		if (verbose >= 2) {
			checksum = 0;
			for (int i = 0; i < read_length; i++)
				checksum += (unsigned int)buffer[i];

			/* check checksum and report */
			fprintf(stderr, "\ndbg5  Checksum test done in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       checksum read:       %d\n", checksum_read);
			fprintf(stderr, "dbg5       checksum calculated: %d\n", checksum);
			if (checksum != checksum_read) {
				fprintf(stderr, "dbg5       CHECKSUM ERROR!!\n");
			}
			else {
				fprintf(stderr, "dbg5       checksum ok\n");
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (int i = 0; i < store->nbeams; i++) {
			fprintf(stderr, "dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n", i, store->beams[i].depth,
			        store->beams[i].acrosstrack, store->beams[i].alongtrack, store->beams[i].range, store->beams[i].angle_across,
			        store->beams[i].angle_forward, store->beams[i].amplitude, store->beams[i].signal_to_noise,
			        store->beams[i].echo_length, store->beams[i].source, store->beams[i].quality);
		}
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
int mbr_sb2100bi_rd_sr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, short record_length, int *error) {
	int status = MB_SUCCESS;
	int read_length = 0;
	unsigned int checksum_read = 0;
	unsigned int checksum = 0;
	char eor_read[6];
	short amplitude_short = 0;
	short alongtrack_short = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       record_len: %d\n", record_length);
	}

	/* check record size */
	if (record_length != store->npixels * MBF_SB2100BI_SR_WRITE_LEN + 6) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* if success read rest of record */
	if (status == MB_SUCCESS) {
		/* read store into structure */
		read_length = store->npixels * MBF_SB2100BI_SR_WRITE_LEN;
		if (read_length > 0)
			if ((status = fread(buffer, read_length, 1, mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

		index = 0;
		for (int i = 0; i < store->npixels; i++) {
			mb_get_binary_short(false, &buffer[index], &amplitude_short);
			index += 2;
			mb_get_binary_short(false, &buffer[index], &alongtrack_short);
			index += 2;
			store->pixels[i].amplitude = (float)amplitude_short;
			store->pixels[i].alongtrack = 0.1 * ((float)alongtrack_short);
		}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0], 6, 1, mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		mb_get_binary_int(false, &eor_read[0], (int *)&checksum_read);

		/* do checksum */
		if (verbose >= 2) {
			checksum = 0;
			for (int i = 0; i < read_length; i++)
				checksum += (unsigned int)buffer[i];

			/* check checksum and report */
			fprintf(stderr, "\ndbg5  Checksum test done in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       checksum read:       %d\n", checksum_read);
			fprintf(stderr, "dbg5       checksum calculated: %d\n", checksum);
			if (checksum != checksum_read) {
				fprintf(stderr, "dbg5       CHECKSUM ERROR!!\n");
			}
			else {
				fprintf(stderr, "dbg5       checksum ok\n");
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       pixel amplitude alongtrack\n");
		for (int i = 0; i < store->npixels; i++) {
			fprintf(stderr, "dbg5       %3d   %f   %f\n", i, store->pixels[i].amplitude, store->pixels[i].alongtrack);
		}
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
int mbr_sb2100bi_rd_data(int verbose, void *mbio_ptr, char *store_ptr, int *error) {
	int status = MB_SUCCESS;
	int type = 0;
	short record_length = 0;
	char record_length_fh_str[8];
	int record_length_fh = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)store_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;
	char *buffer = (char *) mb_io_ptr->saveptr1;

	/* get saved values */
	char *label = (char *)mb_io_ptr->save_label;
	int *label_save_flag = (int *)&mb_io_ptr->save_label_flag;
	// char *record_length_ptr = (char *)&record_length;

	/* initialize everything to zeros */
	mbr_zero_sb2100bi(verbose, store_ptr, error);

	bool done = false;
	int expect = MBF_SB2100BI_NONE;
	while (!done) {
		/* if no label saved get next record label */
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (!*label_save_flag) {
			/* get next 10 bytes */
			if ((status = fread(&label[0], 10, 1, mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

			/* if not a format 42 label read individual
			    bytes until label found or eof */
			while (status == MB_SUCCESS && strncmp(label, "SB21BI", 6) != 0) {
				for (int i = 0; i < 9; i++)
					label[i] = label[i + 1];
				if ((status = fread(&label[9], 1, 1, mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}
		}

		/* else use saved label */
		else
			*label_save_flag = false;

		/* get the label type */
		if (status == MB_SUCCESS) {
			/* get type */
			type = MBF_SB2100BI_NONE;
			for (int i = 1; i <= MBF_SB2100BI_RECORDS; i++)
				if (strncmp(label, mbf_sb2100bi_labels[i], 8) == 0)
					type = i;

			/* get the record length */
			if (type != MBF_SB2100BI_FH) {
				mb_get_binary_short(false, &label[8], &record_length);
			}
			else {
				record_length_fh_str[0] = label[8];
				record_length_fh_str[1] = label[9];
				if ((status = fread(&record_length_fh_str[2], 4, 1, mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
				record_length_fh_str[6] = 0;
				record_length_fh_str[7] = 0;
				sscanf(record_length_fh_str, "%d", &record_length_fh);
			}
		}

		/* read the appropriate data records */
		if ((status == MB_FAILURE || type == MBF_SB2100BI_NONE) && expect == MBF_SB2100BI_NONE) {
			done = true;
		}
		else if ((status == MB_FAILURE || type == MBF_SB2100BI_NONE) && expect != MBF_SB2100BI_NONE) {
			done = true;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else if (expect != MBF_SB2100BI_NONE && expect != type) {
			done = true;
			expect = MBF_SB2100BI_NONE;
			*label_save_flag = true;
		}
		else if (type == MBF_SB2100BI_FH) {
			status = mbr_sb2100bi_rd_fh(verbose, mbfp, buffer, record_length_fh, error);
			if (status == MB_SUCCESS) {
				done = false;
				expect = MBF_SB2100BI_NONE;
				store->kind = MB_DATA_NONE;
			}
		}
		else if (type == MBF_SB2100BI_PR) {
			status = mbr_sb2100bi_rd_pr(verbose, mbfp, buffer, store, record_length, error);
			if (status == MB_SUCCESS) {
				done = true;
				store->kind = MB_DATA_VELOCITY_PROFILE;
			}
		}
		else if (type == MBF_SB2100BI_TR) {
			status = mbr_sb2100bi_rd_tr(verbose, mbfp, buffer, store, record_length, error);
			if (status == MB_SUCCESS) {
				done = true;
				store->kind = MB_DATA_COMMENT;
			}
		}
		else if (type == MBF_SB2100BI_DH) {
			status = mbr_sb2100bi_rd_dh(verbose, mbfp, buffer, store, record_length, error);
			if (status == MB_SUCCESS) {
				done = false;
				store->kind = MB_DATA_DATA;
				expect = MBF_SB2100BI_BR;
			}
		}
		else if (type == MBF_SB2100BI_BR) {
			status = mbr_sb2100bi_rd_br(verbose, mbfp, buffer, store, record_length, error);
			if (status == MB_SUCCESS && expect == MBF_SB2100BI_BR) {
				done = false;
				store->kind = MB_DATA_DATA;
				expect = MBF_SB2100BI_SR;
			}
			else if (status == MB_SUCCESS) {
				done = true;
				expect = MBF_SB2100BI_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
			else if (status == MB_FAILURE) {
				done = true;
				expect = MBF_SB2100BI_NONE;
			}
		}
		else if (type == MBF_SB2100BI_SR) {
			status = mbr_sb2100bi_rd_sr(verbose, mbfp, buffer, store, record_length, error);
			if (status == MB_SUCCESS && expect == MBF_SB2100BI_SR) {
				done = true;
			}
			else if (status == MB_SUCCESS) {
				done = true;
				expect = MBF_SB2100BI_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
			else if (status == MB_FAILURE && *error == MB_ERROR_UNINTELLIGIBLE && expect == MBF_SB2100BI_SR) {
				/* this preserves the bathymetry
				   that has already been read */
				done = true;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
		}
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
int mbr_rt_sb2100bi(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)store_ptr;

	/* read next data from file */
	int status = mbr_sb2100bi_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* zero sidescan for format MBF_SB2100B2 (43) */
	if (status == MB_SUCCESS && store != NULL && store->kind != MB_DATA_COMMENT && mb_io_ptr->format == MBF_SB2100B2)
		store->npixels = 0;

	/* set unset parameters in sb2100 data storage structure */
	if (status == MB_SUCCESS && store != NULL && store->kind != MB_DATA_COMMENT) {
		/* parameters for MBF_SB2100RW format */
		store->range_scale = ' ';
		store->spare_dr[0] = ' ';
		store->spare_dr[1] = ' ';
		store->num_algorithms = 1;
		for (int i = 0; i < 4; i++)
			store->algorithm_order[i] = ' ';
		store->svp_corr_ss = 0;
		store->ss_data_length = 4 * MBSYS_SB2100_PIXELS;
		store->pixel_size_scale = 'D';
		store->spare_ss = ' ';
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
int mbr_sb2100bi_wr_fh(int verbose, FILE *mbfp, int *error) {
	int status = MB_SUCCESS;
	int record_length = 0;
	char record_length_str[8];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       file_header_text: \n%s%s\n", mbf_sb2100bi_file_header_text_1,
		        mbf_sb2100bi_file_header_text_2);
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_FH], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = strlen(mbf_sb2100bi_file_header_text_1) + strlen(mbf_sb2100bi_file_header_text_2);
		sprintf(record_length_str, "%6d", record_length);
		if (fwrite(record_length_str, 6, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the store */
	if (status == MB_SUCCESS) {
		/* write the data */
		if (fwrite(mbf_sb2100bi_file_header_text_1, strlen(mbf_sb2100bi_file_header_text_1), 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		if (fwrite(mbf_sb2100bi_file_header_text_2, strlen(mbf_sb2100bi_file_header_text_2), 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_pr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, int *error) {
	int status = MB_SUCCESS;
	short record_length = 0;
	int write_length = 0;
	unsigned int checksum = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", store->year);
		fprintf(stderr, "dbg5       julian day:       %d\n", store->jday);
		fprintf(stderr, "dbg5       hour:             %d\n", store->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", store->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", store->sec);
		fprintf(stderr, "dbg5       msec:             %d\n", store->msec);
		fprintf(stderr, "dbg5       roll_bias_port:   %f\n", store->roll_bias_port);
		fprintf(stderr, "dbg5       roll_bias_strbrd: %f\n", store->roll_bias_starboard);
		fprintf(stderr, "dbg5       pitch_bias:       %f\n", store->pitch_bias);
		fprintf(stderr, "dbg5       ship_draft:       %f\n", store->ship_draft);
		fprintf(stderr, "dbg5       offset_x:         %f\n", store->offset_x);
		fprintf(stderr, "dbg5       offset_y:         %f\n", store->offset_y);
		fprintf(stderr, "dbg5       offset_z:         %f\n", store->offset_z);
		fprintf(stderr, "dbg5       num_svp:          %d\n", store->num_svp);
		fprintf(stderr, "dbg5       Sound Velocity Profile:\n");
		for (int i = 0; i < store->num_svp; i++)
			fprintf(stderr, "dbg5       %d  depth:%f  velocity:%f\n", i, store->svp[i].depth, store->svp[i].velocity);
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_PR], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = MBF_SB2100BI_PR_WRITE_LEN + 6;
		mb_put_binary_short(false, record_length, &buffer[0]);
		if (fwrite(buffer, 2, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the data */
	if (status == MB_SUCCESS) {
		index = 0;
		mb_put_binary_short(false, store->year, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->jday, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->hour, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->minute, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->sec, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->msec, &buffer[index]);
		index += 2;
		mb_put_binary_float(false, store->roll_bias_port, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->roll_bias_starboard, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->pitch_bias, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->ship_draft, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->offset_x, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->offset_y, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->offset_z, &buffer[index]);
		index += 4;
		mb_put_binary_int(false, store->num_svp, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->num_svp; i++) {
			mb_put_binary_float(false, store->svp[i].depth, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->svp[i].velocity, &buffer[index]);
			index += 4;
		}

		/* do checksum */
		write_length = MBF_SB2100BI_PR_WRITE_LEN;
		checksum = 0;
		for (int i = 0; i < write_length; i++)
			checksum += (unsigned int)buffer[i];
		mb_put_binary_int(false, checksum, &buffer[index]);
		index += 4;
		buffer[index] = mbf_sb2100bi_eor[0];
		index++;
		buffer[index] = mbf_sb2100bi_eor[1];
		index++;
		write_length += 6;

		/* write the data */
		if (fwrite(buffer, write_length, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_tr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, int *error) {
	int status = MB_SUCCESS;
	short record_length = 0;
	int write_length = 0;
	unsigned int checksum = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", store->comment);
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_TR], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = strlen(store->comment) + 1;
		if (record_length >= MBSYS_SB2100_MAXLINE) {
			store->comment[MBSYS_SB2100_MAXLINE - 1] = '\0';
			record_length = MBSYS_SB2100_MAXLINE;
		}
		record_length += 6;
		mb_put_binary_short(false, record_length, &buffer[0]);
		if (fwrite(buffer, 2, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* do checksum */
		strcpy(buffer, store->comment);
		write_length = strlen(buffer) + 1;
		checksum = 0;
		for (int i = 0; i < write_length; i++)
			checksum += (unsigned int)buffer[i];
		index = write_length;
		mb_put_binary_int(false, checksum, &buffer[index]);
		index += 4;
		buffer[index] = mbf_sb2100bi_eor[0];
		index++;
		buffer[index] = mbf_sb2100bi_eor[1];
		index++;
		write_length += 6;

		/* write the data */
		if (fwrite(buffer, write_length, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_dh(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, int *error) {
	int status = MB_SUCCESS;
	short record_length = 0;
	int write_length = 0;
	unsigned int checksum = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", store->year);
		fprintf(stderr, "dbg5       julian day:       %d\n", store->jday);
		fprintf(stderr, "dbg5       hour:             %d\n", store->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", store->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", store->sec);
		fprintf(stderr, "dbg5       msec:             %d\n", store->msec);
		fprintf(stderr, "dbg5       longitude:        %f\n", store->longitude);
		fprintf(stderr, "dbg5       latitude:         %f\n", store->latitude);
		fprintf(stderr, "dbg5       heading:          %f\n", store->heading);
		fprintf(stderr, "dbg5       speed:            %f\n", store->speed);
		fprintf(stderr, "dbg5       roll:             %f\n", store->roll);
		fprintf(stderr, "dbg5       pitch:            %f\n", store->pitch);
		fprintf(stderr, "dbg5       heave:            %f\n", store->heave);
		fprintf(stderr, "dbg5       ssv:              %f\n", store->ssv);
		fprintf(stderr, "dbg5       frequency:        %c\n", store->frequency);
		fprintf(stderr, "dbg5       depth_gate_mode:  %d\n", store->depth_gate_mode);
		fprintf(stderr, "dbg5       ping_gain:        %d\n", store->ping_gain);
		fprintf(stderr, "dbg5       ping_pulse_width: %d\n", store->ping_pulse_width);
		fprintf(stderr, "dbg5       trans_atten:      %d\n", store->transmitter_attenuation);
		fprintf(stderr, "dbg5       ssv_source:       %c\n", store->ssv_source);
		fprintf(stderr, "dbg5       svp_correction:   %c\n", store->svp_correction);
		fprintf(stderr, "dbg5       pixel_algorithm:  %c\n", store->pixel_algorithm);
		fprintf(stderr, "dbg5       pixel_size:       %f\n", store->pixel_size);
		fprintf(stderr, "dbg5       nbeams:           %d\n", store->nbeams);
		fprintf(stderr, "dbg5       npixels:          %d\n", store->npixels);
		fprintf(stderr, "dbg5       spare1:           %d\n", store->spare1);
		fprintf(stderr, "dbg5       spare2:           %d\n", store->spare2);
		fprintf(stderr, "dbg5       spare3:           %d\n", store->spare3);
		fprintf(stderr, "dbg5       spare4:           %d\n", store->spare4);
		fprintf(stderr, "dbg5       spare5:           %d\n", store->spare5);
		fprintf(stderr, "dbg5       spare6:           %d\n", store->spare6);
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_DH], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = MBF_SB2100BI_DH_WRITE_LEN + 6;
		mb_put_binary_short(false, record_length, &buffer[0]);
		if (fwrite(buffer, 2, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the data */
	if (status == MB_SUCCESS) {
		index = 0;
		mb_put_binary_short(false, store->year, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->jday, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->hour, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->minute, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->sec, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->msec, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->spare1, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->spare2, &buffer[index]);
		index += 2;
		mb_put_binary_double(false, store->longitude, &buffer[index]);
		index += 8;
		mb_put_binary_double(false, store->latitude, &buffer[index]);
		index += 8;
		mb_put_binary_float(false, store->heading, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->speed, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->roll, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->pitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->heave, &buffer[index]);
		index += 4;
		mb_put_binary_float(false, store->ssv, &buffer[index]);
		index += 4;
		buffer[index] = store->frequency;
		index++;
		buffer[index] = store->depth_gate_mode;
		index++;
		buffer[index] = store->ping_gain;
		index++;
		buffer[index] = store->ping_pulse_width;
		index++;
		buffer[index] = store->transmitter_attenuation;
		index++;
		buffer[index] = store->ssv_source;
		index++;
		buffer[index] = store->svp_correction;
		index++;
		buffer[index] = store->pixel_algorithm;
		index++;
		mb_put_binary_float(false, store->pixel_size, &buffer[index]);
		index += 4;
		mb_put_binary_int(false, store->nbeams, &buffer[index]);
		index += 4;
		mb_put_binary_int(false, store->npixels, &buffer[index]);
		index += 4;
		mb_put_binary_short(false, store->spare3, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->spare4, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->spare5, &buffer[index]);
		index += 2;
		mb_put_binary_short(false, store->spare6, &buffer[index]);
		index += 2;

		/* do checksum */
		checksum = 0;
		write_length = MBF_SB2100BI_DH_WRITE_LEN;
		for (int i = 0; i < write_length; i++)
			checksum += (unsigned int)buffer[i];
		mb_put_binary_int(false, checksum, &buffer[index]);
		index += 4;
		buffer[index] = mbf_sb2100bi_eor[0];
		index++;
		buffer[index] = mbf_sb2100bi_eor[1];
		index++;
		write_length += 6;

		/* write the store */
		if (fwrite(buffer, write_length, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_br(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, int *error) {
	int status = MB_SUCCESS;
	short record_length = 0;
	int write_length = 0;
	unsigned int checksum = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (int i = 0; i < store->nbeams; i++) {
			fprintf(stderr, "dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n", i, store->beams[i].depth,
			        store->beams[i].acrosstrack, store->beams[i].alongtrack, store->beams[i].range, store->beams[i].angle_across,
			        store->beams[i].angle_forward, store->beams[i].amplitude, store->beams[i].signal_to_noise,
			        store->beams[i].echo_length, store->beams[i].source, store->beams[i].quality);
		}
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_BR], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = store->nbeams * MBF_SB2100BI_BR_WRITE_LEN + 6;
		mb_put_binary_short(false, record_length, &buffer[0]);
		if (fwrite(buffer, 2, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the data */
	if (status == MB_SUCCESS) {
		index = 0;
		for (int i = 0; i < store->nbeams; i++) {
			mb_put_binary_float(false, store->beams[i].depth, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->beams[i].acrosstrack, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->beams[i].alongtrack, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->beams[i].range, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->beams[i].angle_across, &buffer[index]);
			index += 4;
			mb_put_binary_float(false, store->beams[i].angle_forward, &buffer[index]);
			index += 4;
			mb_put_binary_short(false, store->beams[i].amplitude, &buffer[index]);
			index += 2;
			mb_put_binary_short(false, store->beams[i].signal_to_noise, &buffer[index]);
			index += 2;
			mb_put_binary_short(false, store->beams[i].echo_length, &buffer[index]);
			index += 2;
			buffer[index] = store->beams[i].quality;
			index++;
			buffer[index] = store->beams[i].source;
			index++;
		}

		/* do checksum */
		checksum = 0;
		write_length = store->nbeams * MBF_SB2100BI_BR_WRITE_LEN;
		for (int i = 0; i < write_length; i++)
			checksum += (unsigned int)buffer[i];
		mb_put_binary_int(false, checksum, &buffer[index]);
		index += 4;
		buffer[index] = mbf_sb2100bi_eor[0];
		index++;
		buffer[index] = mbf_sb2100bi_eor[1];
		index++;
		write_length += 6;

		/* write the data */
		if (fwrite(buffer, write_length, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_sr(int verbose, FILE *mbfp, char *buffer, struct mbsys_sb2100_struct *store, int *error) {
	int status = MB_SUCCESS;
	short record_length = 0;
	int write_length = 0;
	unsigned int checksum = 0;
	short amplitude_short = 0;
	short alongtrack_short = 0;
	int index = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       pixel amplitude alongtrack\n");
		for (int i = 0; i < store->npixels; i++) {
			fprintf(stderr, "dbg5       %3d   %f   %f\n", i, store->pixels[i].amplitude, store->pixels[i].alongtrack);
		}
	}

	/* write the record label */
	if (fwrite(mbf_sb2100bi_labels[MBF_SB2100BI_SR], MBF_SB2100BI_LABEL_LEN, 1, mbfp) != 1) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* write the record length */
	if (status == MB_SUCCESS) {
		record_length = store->npixels * MBF_SB2100BI_SR_WRITE_LEN + 6;
		mb_put_binary_short(false, record_length, &buffer[0]);
		if (fwrite(buffer, 2, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the data */
	if (status == MB_SUCCESS) {
		index = 0;
		for (int i = 0; i < store->npixels; i++) {
			amplitude_short = (short)store->pixels[i].amplitude;
			alongtrack_short = (short)(10 * store->pixels[i].alongtrack);
			mb_put_binary_short(false, amplitude_short, &buffer[index]);
			index += 2;
			mb_put_binary_short(false, alongtrack_short, &buffer[index]);
			index += 2;
		}

		/* do checksum */
		checksum = 0;
		write_length = store->npixels * MBF_SB2100BI_SR_WRITE_LEN;
		for (int i = 0; i < write_length; i++)
			checksum += (unsigned int)buffer[i];
		mb_put_binary_int(false, checksum, &buffer[index]);
		index += 4;
		buffer[index] = mbf_sb2100bi_eor[0];
		index++;
		buffer[index] = mbf_sb2100bi_eor[1];
		index++;
		write_length += 6;

		/* write the data */
		if (fwrite(buffer, write_length, 1, mbfp) != 1) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_sb2100bi_wr_data(int verbose, void *mbio_ptr, char *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)store_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;
	char *buffer = (char *) mb_io_ptr->saveptr1;

	int status = MB_SUCCESS;

	/* write file header if not written yet */
	if (!mb_io_ptr->save_flag) {
		status = mbr_sb2100bi_wr_fh(verbose, mbfp, error);
		mb_io_ptr->save_flag = true;
	}

	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		status = mbr_sb2100bi_wr_pr(verbose, mbfp, buffer, store, error);
	}
	else if (store->kind == MB_DATA_COMMENT) {
		status = mbr_sb2100bi_wr_tr(verbose, mbfp, buffer, store, error);
	}
	else if (store->kind == MB_DATA_DATA) {
		status = mbr_sb2100bi_wr_dh(verbose, mbfp, buffer, store, error);
		status = mbr_sb2100bi_wr_br(verbose, mbfp, buffer, store, error);
		status = mbr_sb2100bi_wr_sr(verbose, mbfp, buffer, store, error);
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  store record kind in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", store->kind);
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
int mbr_wt_sb2100bi(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_sb2100_struct *store = (struct mbsys_sb2100_struct *)store_ptr;

	/* make sure no sidescan is written for format MBF_SB2100B2 (43) */
	if (store != NULL && store->kind != MB_DATA_COMMENT && mb_io_ptr->format == MBF_SB2100B2)
		store->npixels = 0;

	/* write next data to file */
	const int status = mbr_sb2100bi_wr_data(verbose, mbio_ptr, store_ptr, error);

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
int mbr_register_sb2100b1(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_sb2100b1(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2100bi;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2100bi;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2100_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb2100_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2100bi;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2100bi;
	mb_io_ptr->mb_io_dimensions = &mbsys_sb2100_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_sb2100_extract;
	mb_io_ptr->mb_io_insert = &mbsys_sb2100_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2100_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2100_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2100_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_sb2100_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_sb2100_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2100_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_sb2100_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2100_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

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
int mbr_register_sb2100b2(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_sb2100b2(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2100bi;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2100bi;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2100_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb2100_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2100bi;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2100bi;
	mb_io_ptr->mb_io_dimensions = &mbsys_sb2100_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_sb2100_extract;
	mb_io_ptr->mb_io_insert = &mbsys_sb2100_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2100_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2100_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2100_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_sb2100_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_sb2100_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2100_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_sb2100_detects;
	mb_io_ptr->mb_io_gains = &mbsys_sb2100_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2100_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

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
