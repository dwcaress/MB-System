/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_dsl.h	8/5/94
 *	$Id: mbsys_dsl.h,v 5.5 2003-04-17 21:05:23 caress Exp $
 *
 *    Copyright (c) 1996, 2000, 2002, 2003 by
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
 * mbsys_dsl.h defines the data structures used by MBIO functions
 * to store data from the DSL AMS-120 sonar system. This is a 120 khz 
 * deep-towed sonar which produces both sidescan and bathymetry.
 * The data formats used to store the DSL AMS-120 data are: 
 *      MBF_DSL120PF : MBIO ID 111
 *      MBF_DSL120SF : MBIO ID 112
 *
 *
 * Author:	D. W. Caress
 * Date:	August 5, 1996
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.0  1996/08/26  17:29:56  caress
 * Release 4.4 revision.
 *
 * Revision 1.1  1996/08/26  17:24:56  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBSYS_DSL data structure:
 *   1. The DSL processing system used a parallel file scheme
 *      in which bathymetry, sidescan, and navigation are kept
 *      in separate files. Some examples of the filenames are:
 *         DSL120.940630_1100.bat.dat	- bathymetry
 *         DSL120.940630_1100.amp.dat	- sidescan
 *         DSL120.940630_1100.nav	- navigation
 *   2. The DSL parallel file scheme is supported under MB-System
 *      data format 111 (MBF_DSL120PF); a single file scheme is
 *      supported under data format 112 (MBF_DSL120SF). The 
 *      single file scheme is within the DSL format specification.
 *   3. The bathymetry and sidescan data are stored in binary
 *      data structures; the navigation is stored in ASCII.
 *   4. The DSL format supports arbitrary numbers of bathymetry
 *      and sidescan values. The MB-System implementation has
 *      maximum numbers of values hardwired in the #defines
 *      below.
 *   5. The bathymetry and sidescan data have navigation 
 *      fields,  but these navigation values typically repeat
 *      for many pings, often being the same for entire files.
 *      The separate navigation files contain the post-processed
 *      navigation with distinct values for each ping.
 *   6. The original navigation values supplied by WHOI/DSL
 *      so far are in projected eastings and northings rather
 *      than in longitude and latitude. Since MB-System only
 *      works with longitude and latitude, a special program
 *      will need to be used to translate the navigation to
 *      longitude and latitude.
 *   7. Due to the above problem, the following scheme will
 *      be used to handle MBF_DSL120DT data in MB-System:
 *        a) Run a perl macro called mbdslnavfix on the
 *           DSL120.*.nav files,  producing DSL.*.mbnav
 *           files in a format understood by mbmerge.
 *        b) Run mbmerge on the DSL120.*.bat.dat and DSL120.*.amp.dat
 *           files to merge the navigation from the DSL120.*.mbnav
 *           in with the bathymetry and sidescan data.
 *   8. The parallel file structure is difficult to deal with
 *      in the MB-System world where programs expect a single
 *      input and a single output file name. Handling the two
 *      files (bat.dat and amp.dat) will be handled in low level
 *      i/o routines. If the input file name has a "bat" in it, 
 *      the code will attempt to open a second file with the
 *      same name except that "amp" is substituted for "bat".
 *      If the specified input file has "amp" in it,  then the
 *      second file will have "bat". The same will be true for
 *      the output files.
 *   9. The comment records reside only in the "bat" files in
 *      the parallel file scheme of format 111.
 *      The comment records are an MB-System extension to the
 *      DSL format. The program mbcopy can be used to strip
 *      the comments out of the data files prior to reusing
 *      DSL processing software.
 *
 */

/* maximum number of beams and pixels */
#define	MBSYS_DSL_MAXBEAMS_SIDE	    1024
#define	MBSYS_DSL_MAXBEAMS	    2 * MBSYS_DSL_MAXBEAMS_SIDE
#define	MBSYS_DSL_MAXPIXELS_SIDE    4096
#define	MBSYS_DSL_MAXPIXELS	    2 * MBSYS_DSL_MAXPIXELS_SIDE
#define	MBSYS_DSL_COMMENT_LENGTH	80

/* datagram types */
#define	DSL_NONE		0x00000000L
#define	DSL_HEADER		0x44534C20L	/* "DSL " */
#define	DSL_BATH		0x42415448L	/* "BATH" */
#define	DSL_AMP			0x414D5020L	/* "AMP " */
#define	DSL_COMMENT		0x434f4d4dL	/* "COMM" */

/* internal data structure */
struct mbsys_dsl_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* record header */
	int	rec_type;		/* always "DSL " */
	int	rec_len;
	int	rec_hdr_len;
	unsigned int p_flags;		/* processing flags */
	int	num_data_types;		/* number of data types in rec */
   	int	ping;			/* ping number */
   	char	sonar_cmd[4];		/* sonar parameters */
   	char	time_stamp[24];		/* ascii event time */
  	float	nav_x;			/* x position */
   	float	nav_y;			/* y position */
   	float	depth;			/* depth - meters */
   	float	heading;		/* heading of vehicle - degrees */
   	float	pitch;			/* pitch - degrees */
   	float	roll;			/* roll - degrees */
   	float	alt;			/* altitude - meters */
   	float	ang_offset;	 	/* pointing ang relative to nose - deg*/
   	int	transmit_pwr;  		/* transmit power decibels */
   	int	gain_port;		/* db - not sure if belongs here */ 
   	int	gain_starbd;		/* db - not sure if belongs here */
   	float	pulse_width;		/* pulse width */
   	int	swath_width;		/* meters */
   	char	side;			/* 0 - port, 1 - stbd for fwd scan */
   	char	swapped;		/* data,header: 00-PC 01-SunHdr 11-Sun*/
	int	tv_sec;			/* seconds */
	int	tv_usec;		/* and microseconds */
   	short	interface;	        /* digital interface: 0,1,or 2 -
					 * must be specified in config file */
    	short reserved[5];
	
	/* bathymetry record */
	int	bat_type;		/* always "BATH" */
	int	bat_len;
	int	bat_hdr_len;
	int	bat_num_bins;      
	float	bat_sampleSize;
	unsigned int bat_p_flags;
	float	bat_max_range; /* meters */
	int	bat_future[9];
	float	bat_port[MBSYS_DSL_MAXBEAMS_SIDE];
	float	bat_stbd[MBSYS_DSL_MAXBEAMS_SIDE];
	
	/* amplitude record */
	int	amp_type;		/* always "AMP " */
	int	amp_len;
	int	amp_hdr_len;
	int	amp_num_samp;      
	float	amp_sampleSize;
	unsigned int amp_p_flags; /* offset/slr, ... */
	float	amp_max_range; /* meters */
	int	amp_channel;     /* 1-upper/0-lower */
	int	amp_future[8];
	float	amp_port[MBSYS_DSL_MAXPIXELS];
	float	amp_stbd[MBSYS_DSL_MAXPIXELS_SIDE];
	
	/* comment */
	char	comment[MBSYS_DSL_COMMENT_LENGTH];
	};
	
/* system specific function prototypes */
int mbsys_dsl_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_dsl_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_dsl_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_dsl_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_dsl_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_dsl_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_dsl_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_dsl_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_dsl_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_dsl_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

