/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2100.h	2/4/94
 *	$Id: mbsys_sb2100.h,v 5.3 2002-07-20 20:42:40 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mbsys_sb2100.h defines the data structures used by MBIO functions
 * to store data from SeaBeam 2100 and 1000 multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam 1000/2100
 * data in files include
 *      MBF_SB2100RW : MBIO ID 41
 *      MBF_SB2100BN : MBIO ID 42
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1994
 * $Log: not supported by cvs2svn $
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.6  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.5  1995/06/07  20:39:59  caress
 * Fixed some typos.
 *
 * Revision 4.4  1995/05/08  21:26:28  caress
 * Made changes consistent with new i/o spec for SB2100 data.
 *
 * Revision 4.3  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.2  1994/06/21  22:54:21  caress
 * Added #ifdef statements to handle byte swapping.
 *
 * Revision 4.1  1994/04/09  15:49:21  caress
 * Altered to fit latest iteration of SeaBeam 2100 vendor format.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  02:12:07  caress
 * First cut for SeaBeam 2100 i/o.
 *
 *
 */
/*
 * Notes on the MBSYS_SB2100 data structure:
 *   1. SeaBeam 2100 multibeam sonars output raw data in two
 *      formats,  with a third created in post processing. 
 *      The older format is mostly ascii with binary
 *      sidescan (format 41).  The newer formats are entirely 
 *      binary. Format 42 contains all information; format 43
 *      is identical to format 42 except that the sidescan
 *      data has been removed.
 *   2. The SeaBeam 2100 sonars output up to 151 beams of 
 *      bathymetry and 2000 pixels of sidescan, along with a 
 *      plethora of other information.
 *   3. The records all include navigation and time stamp information.
 *      The record types are for format 41 are:
 *        PR:  sonar parameter record (roll bias, pitch bias, sound velocity profile)
 *        TR:  sonar text record (comments)
 *        DR:  bathymetry data record (bathymetry and per-beam amplitudes)
 *        SS:  side scan data record
 *      The record types are for format 42 are:
 *        PR:  sonar parameter record (roll bias, pitch bias, sound velocity profile)
 *        TR:  sonar text record (comments)
 *        DH:  ping data header (one per ping)
 *        BR:  bathymetry data record (bathymetry and per-beam amplitudes)
 *        SR:  side scan data record
 *   4. For format 41 a single ping usually results in both DR and SS 
 *      records.  The PR record occurs every 30 minutes or when 
 *      the sound velocity profile or bias parameters are changed.
 *   5. For format 42 a single ping results in DH, BR, and SR records.
 *      Format 42 files created directly by the sonars will have
 *      PR records at the beginning; format 42 files created by
 *      translating format 41 files will have the same frequency of
 *      PR records as in the original files.
 *   5. The kind value in the mbsys_sb2k_struct indicates whether the
 *      mbsys_sb2k_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a ping 
 *                   (DR + SS) or (DH + BR + SR)
 *        kind = 2 : comment (TR)
 *        kind = 8 : sonar parameter (PR)
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2100 records.
 */

/* maximum number of depth-velocity pairs */
#define MBSYS_SB2100_MAXVEL 30

/* maximum line length in characters */
#define MBSYS_SB2100_MAXLINE 1944

/* maximum number of formed beams for SeaBeam 1000/2100 */
#define MBSYS_SB2100_BEAMS 151

/* maximum number of sidescan pixels for SeaBeam 1000/2100 */
#define MBSYS_SB2100_PIXELS 2000
	
struct mbsys_sb2100_svp_struct
	{
	float	depth;			/* m */
	float	velocity;		/* m/sec */	   
	};
	
struct mbsys_sb2100_beam_struct
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
	
struct mbsys_sb2100_ss_struct
	{
	float	amplitude;		/* sidescan value */
	float	alongtrack;		/* m */	   
	};

struct mbsys_sb2100_struct
	{
	/* type of data record */
	int	kind;

	/* sonar parameters (SB21BIPR) */
	float	roll_bias_port;			/* deg */
	float	roll_bias_starboard;		/* deg */
	float	pitch_bias;			/* deg */
	float	ship_draft;			/* m */
	float	offset_x;			/* m */
	float	offset_y;			/* m */
	float	offset_z;			/* m */
	int	num_svp;
	struct mbsys_sb2100_svp_struct  svp[MBSYS_SB2100_MAXVEL];
	
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
	struct mbsys_sb2100_beam_struct beams[MBSYS_SB2100_BEAMS];
	
	/* sidescan record (SB21BISR) */
	struct mbsys_sb2100_ss_struct pixels[MBSYS_SB2100_PIXELS];
	
	/* parameters unique to MBF_SB2100RW format */
	char	range_scale; 		/* D = m; I = 0.1 m; S = 0.01 m */
	char	spare_dr[2];
	int	num_algorithms;		/* If 1 then only "best" algorithm 
						recorded, else multiple 
						algorithm results recorded */
	char	algorithm_order[4];	/* blank if num_algorithms=1; 
						W=WMT and B=BDI */
	char	svp_corr_ss;		/* 0=off; 1=on */
	int	ss_data_length;		/* number of bytes of sidescan data */
	char	pixel_size_scale;
	char	spare_ss;

	/* comment (SB21BITR) - comments are stored by
	    recasting pointer to roll_bias_port to a char ptr 
	    and writing over up to MBSYS_SB2100_MAXLINE
	    bytes in structure */
	char	*comment;
};

	
/* system specific function prototypes */
int mbsys_sb2100_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_sb2100_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_sb2100_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb2100_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb2100_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_sb2100_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_sb2100_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_sb2100_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_sb2100_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_sb2100_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_sb2100_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_sb2100_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

