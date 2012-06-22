/*--------------------------------------------------------------------
 *    The MB-system:	MBF_SB2100B1.h	1/16/94
 *	$Id$
 *
 *    Copyright (c) 1997, 2000, 2002, 2003 by
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
 * mbf_sb2100b1.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_SB2100B1 format (MBIO id 42).  
 *
 * Author:	D. W. Caress
 * Date:	January 16, 1997
 * $Log: not supported by cvs2svn $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.1  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.0  1997/04/21  16:59:50  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1997/04/17  15:11:34  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 1.1  1997/04/17  15:07:36  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBF_SB2100B1 data format:
 *   1. SeaBeam 2100 multibeam sonars currently generate 
 *      raw data in an hybrid ascii/binary format (41). 
 *      This is a replacement fully binary (excepting file header) 
 *      format which has significantly faster i/o during processing.
 *   2. The SeaBeam 2100 sonars output up to 151 beams of bathymetry 
 *      and 2000 pixels of sidescan measurements, along with a plethora 
 *      of other information.
 *   3. The record types are:
 *        SB21BIFH:  file header with data format description 
 *                   (beginning of file only)
 *        SB21BIPR:  sonar parameter record 
 *                   (roll bias, pitch bias, sound velocity profile)
 *        SB21BITR:  sonar text record (comments)
 *        SB21BIDH:  sonar data header (one for each ping)
 *        SB21BIBR:  bathymetry data record (one for each ping)
 *        SB21BISR:  sidescan data record (one for each ping)
 *   4. The file header record occurs at the beginning of each file.
 *      this is a fully ASCII record with line feeds and null
 *      termination so that uninformed users can figure out the
 *      contents of the file without additional documentation.
 *      There is no analog to this header in format 41.
 *   5. The parameter record should be generated at the beginning of
 *      every file (after the header); new files with new parameter
 *      records should be generated any time the roll bias, pitch
 *      bias, or sound velocity profile values change. 
 *      The existing SeaBeam 2100 sonars output parameter records when
 *      the sonar begins logging and every 30 minutes thereafter,
 *      regardless of where it appears in files.
 *      The parameter also includes values for navigation offsets
 *      due to the offset between the transducers and the GPS antenna.
 *      SeaBeam sonars do not presently make use of such parameters.
 *   6. Individual comment records are limited to lengths of
 *      1920 characters.
 *      Each file should begin with comment records stating
 *      the sonar and sonar control software version used to
 *      generate the data. This does not occur at present.
 *   7. Each ping generates three data records in the following
 *      order:
 *        SB21BIDH:  sonar data header
 *        SB21BIBR:  bathymetry data record
 *        SB21BISR:  sidescan data record
 *   8. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2100 records.
 * 
 * SeaBeam 2100 MBF_SB2100B1 data format definition:
 * 
 * 
 *    
 * 
 */

/* maximum number of depth-velocity pairs */
#define MBF_SB2100B1_MAXVEL 30

/* maximum comment line length in characters */
#define MBF_SB2100B1_MAXLINE 1944

/* maximum number of formed beams for SeaBeam 2100 */
#define MBF_SB2100B1_BEAMS 151

/* maximum number of sidescan pixels for SeaBeam 2100 */
#define MBF_SB2100B1_PIXELS 2000

/* define id's for the different types of raw records */
#define	MBF_SB2100B1_RECORDS	6
#define	MBF_SB2100B1_NONE	0
#define	MBF_SB2100B1_FH		1
#define	MBF_SB2100B1_TR		2
#define	MBF_SB2100B1_PR		3
#define	MBF_SB2100B1_DH		4
#define	MBF_SB2100B1_BR		5
#define	MBF_SB2100B1_SR		6
char *mbf_sb2100b1_labels[] = {
	"NONE    ", "SB21BIFH", "SB21BITR", "SB21BIPR", 
	"SB21BIDH", "SB21BIBR", "SB21BISR"};
	
#define MBF_SB2100B1_PR_WRITE_LEN   284
#define MBF_SB2100B1_DH_WRITE_LEN   80
#define MBF_SB2100B1_BR_WRITE_LEN   32
#define MBF_SB2100B1_SR_WRITE_LEN   4
#define MBF_SB2100B1_LABEL_LEN	    8

/* define end of record label */
char	mbf_sb2100b1_eor[2] = {'\r', '\n'};
	
struct mbf_sb2100b1_svp_struct
	{
	float	depth;			/* m */
	float	velocity;		/* m/sec */	   
	};
	
struct mbf_sb2100b1_beam_struct
	{
	float	depth;			/* m */
	float	acrosstrack;		/* m */
	float	alongtrack;		/* m */
	float	range;			/* seconds */
	float	angle_across;		/* degrees */
	float	angle_forward;		/* degrees */
	short	amplitude;		/* 0.25 dB */
	short	signal_to_noise;	/* dB */
	short	echo_length;		/* samples */
	char	quality;		/* 0=no data, 
						Q=poor quality, 
						blank otherwise */
	char	source;			/* B=BDI, W=WMT */
	};
	
struct mbf_sb2100b1_ss_struct
	{
	unsigned short	amplitude;	/* sidescan value */
	short		alongtrack;	/* 0.1 m */	   
	};

struct mbf_sb2100b1_struct
	{
	/* type of data record */
	int	kind;

	/* sonar parameters (SB21BIPR) */
	short	pr_year;
	short	pr_jday;
	short	pr_hour;
	short	pr_minute;
	short	pr_sec;
	short	pr_msec;
	float	roll_bias_port;			/* deg */
	float	roll_bias_starboard;		/* deg */
	float	pitch_bias;			/* deg */
	float	ship_draft;			/* m */
	float	offset_x;			/* m */
	float	offset_y;			/* m */
	float	offset_z;			/* m */
	int	num_svp;
	struct mbf_sb2100b1_svp_struct  svp[MBF_SB2100B1_MAXVEL];
	
	/* sonar data header (SB21BIDH) */
	short	year;
	short	jday;
	short	hour;
	short	minute;
	short	sec;
	short	msec;
	double	longitude;		/* degrees */
	double	latitude;		/* degrees */
	float	heading;		/* degrees */
	float	speed;			/* m/sec */
	float	roll;			/* degrees */
	float	pitch;			/* degrees */
	float	heave;			/* m */
	float	ssv;			/* m/sec */
	char	frequency;		/* L=12kHz; H=36kHz */
	char	depth_gate_mode;	/* A=Auto, M=Manual */
	char	ping_gain;		/* dB */
	char	ping_pulse_width;	/* msec */
	char	transmitter_attenuation;    /* dB */
	char	ssv_source;		/* V=Velocimeter, M=Manual, 
						T=Temperature */
	char	svp_correction;		/* 0=None; A=True Xtrack 
						and Apparent Depth;
						T=True Xtrack and True Depth */
	char	pixel_algorithm;	/* pixel intensity algorithm
						D = logarithm, L = linear */
	float	pixel_size;		/* m */
	int	nbeams;			/* up to 151 */
	int	npixels;		/* up to 2000 */
	short	spare1;
	short	spare2;
	short	spare3;
	short	spare4;
	short	spare5;
	short	spare6;
	
	/* bathymetry record (SB21BIBR) */
	struct mbf_sb2100b1_beam_struct beams[MBF_SB2100B1_BEAMS];
	
	/* sidescan record (SB21BISR) */
	struct mbf_sb2100b1_ss_struct pixels[MBF_SB2100B1_PIXELS];

	/* comment (SB21BITR) - comments are stored by
	    recasting pointer to pr_year to a char ptr 
	    and writing over up to MBF_SB2100B1_MAXLINE
	    bytes in structure */
	char	*comment;
};

/* text for ascii file header */
char	*mbf_sb2100b1_file_header_text_1 = 
{"\
\nSeaBeam 2100 multibeam sonar binary data format\n\
MB-System format 42\n\
Format specification 1.2 defined March 20, 1997\n\
\n\
David W. Caress\n\
SeaBeam Instruments, Inc.\n\
\n\
This is a binary data format for storing all \n\
bathymetry and sidescan data obtained from a SeaBeam\n\
2100 multibeam sonar. Each file consists of an\n\
ASCII file header followed by a series of binary data records.\n\
All binary integer and float values are \"big-endian\" ordered.\n\
All floating point values (float and double) are in the\n\
IEEE standard format.\n\
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
Paramater Record before any ping data. Data files originating\n\
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

char	*mbf_sb2100b1_file_header_text_2 = 
{"\
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
Longitude       degree E of 0E  0.0 - 359.99999 8       22      double\n\
Latitude        degree N of 0N  -90.0 - +90.0   8       30      double\n\
\n\
Heading at ping degree          0.0 - 359.999   4       38      float\n\
Speed           knot            0.0 - 100.0     4       42      float\n\
Roll at ping    degree          -45.0 - +45.0   4       46      float\n\
  >>Note: + = port up\n\
Pitch at ping   degree          -10.0 - +10.0   4       50      float\n\
  >>Note: + = stern up\n\
Heave at ping   m               -10.0 - +10.0   4       54      float\n\
  >>Note: + = above mean level\n\
Surface sound   m/s             1435.0 - 1565.0 4       58      float\n\
  velocity\n\
\n\
Frequency       kHz             L, H            1       62      char\n\
  >>Note: L = 12 kHz, H = 36 kHz\n\
Depth gate mode                 A, M            1       63      char\n\
  >>Note: A = auto, M = manual\n\
Ping gain       dB              0 - 45          1       64      unsigned char\n\
Ping pulse      0.001 s         1 - 20          1       65      unsigned char\n\
  width\n\
Transmitter     dB              0 - 18          1       66      unsigned char\n\
  attenuation\n\
SSV source                      V, M, T, E, U   1       67      char\n\
  >>Note: V = velocimeter, M = manual, T = temperature, \n\
          E = external, U = unknown\n\
SVP correction                  0, T            1       68      char\n\
  >>Note: 0 = None, T = true depth and true position\n\
\n\
Pixel intensity                 D, L            1       69      char\n\
  algorithm\n\
  >>Note: D = logarithm, L = linear; should always be linear for new data\n\
Pixel size      m               0.125 - 20.0    4       70      float\n\
\n\
Number of beams                 0 - 151         2       74      short\n\
Number of pixels                0 - 2000        2       76      short\n\
\n\
Spare                           0               2       78      short\n\
Spare                           0               2       80      short\n\
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
