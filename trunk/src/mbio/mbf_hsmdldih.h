/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsmdldih.h	9/26/95
 *	$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbf_hsmdldih.h,v 5.3 2003/04/17 21:05:23 caress Exp $
 *
 *    Copyright (c) 1995-2016 by
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
 * mbf_hsmdldih.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_HSMDLDIH format (MBIO id 102).
 *
 * Author:	David W. Caress
 * Date:	September 26, 1995
 *
 *
 *
 *
 * Notes on the MBSYS_HSMD data structure:
 *   1. The Atlas Hydrosweep MD generates several types of data files
 *      including:
 *           "*.P"    Profile data files that contain bathy data
 *           "*.W"    Wreck data files with info about obstructions
 *           "*.R"    Raw data files. At present, this is the only
 *                    data file format that is supported by MB. These
 *                    files are written in XDR format. Code to assist
 *                    in parsing these files was provided by STN Atlas.
 *
 *   2. The MD systems output 40 beams of bathymetry on one side and
 *      160 beams of amplitude measurements, along with a moras of other
 *      information.  MD systems ping to one side (making 40 beams & 160
 *      sidescan measurements), wait for all the returns and then ping
 *      to the other side. The "Raw.beamside" variable indicates which side
 *      the current ping is pointed.
 *
 *   3. The records all include navigation and time stamp information.
 *      There is a Header block in front of every data record. The
 *      record types are:
 *        - Raw Event (RAW in the case statement) which contains the
 *          travel time and amplitude data along with other interesting
 *          parameters.
 *        - Navigation (NAV -------"------------) which contains one
 *          input record from the NAV system.
 *        - MD Event
 *        - Beam angle data (ANG) which contains the pointing angles
 *          of the beams. The MD system is supposed to be able to operate
 *          in a beam hopping mode where the beam angles shift from ping
 *          to ping, but this is not yet enabled. The Raw.noho variable
 *          indicated the "hopping" state.
 *        - Sound Velocity (SVP) can contain a sound speed profile.
 *        - Raw Event (REV) which seems to happen at the beginging of
 *          each data file.
 *
 *   4. A single ping usually insonifies one side of the track and results
 *      in RAW data record. Sequential RAW records occur for alternating
 *      sides. NAV records appear to happen at the rate at which they are
 *      supplied by either of the possible Navigation inputs. In early legs
 *      (1994) on the Alliance, two different nav systems were inserting
 *      records, one flagged with an "I" for Integrated Nav and the other
 *      using a "G" to signify GPS. Some time early in 1995, the onboard
 *      nav system was changed and now there appears to be only one type
 *      "I" of input nav data. It appears to happen once per second.
 *
 *   5. The data structure defined below includes all of the values
 *      which are passed in raw Hydrosweep MD records. It also
 *      includes depth and acrosstrack distances calculated from
 *      the travel times, which are stored in the MBF_HSMDLDIH format.
 *
 *   6. Comment records have been implemented for use with MB-System
 *      programs. Comment records are NOT part of the vendor format
 *      definition and are NOT supported by Atlas software. Using
 *      mbcopy with the -N option will remove all comments and make
 *      the data compatible with Atlas software.
 */

/* HSMD defines */

/* maximum number of depth/sound speed data pairs allowed */
#define MBF_HSMDLDIH_MAXVEL		20

/* maximum number of nonzero beams per ping */
#define MBF_HSMDLDIH_BEAMS_PING		40

/* Stores both sides of swath, either starboard or port is nonzero in a ping. */
#define MBF_HSMDLDIH_BEAMS		79

/* maximum number of sidescan pixels in a single ping */
#define MBF_HSMDLDIH_PIXELS_PING		160

/* Stores both sides of swath, either starboard or port is nonzero in a ping. */
#define MBF_HSMDLDIH_PIXELS		319

/* length of a comment string */
#define MBF_HSMDLDIH_COMMENT (128)

/* define id's for the different types of raw Hydrosweep records */

#define MBF_HSMDLDIH_RECORDS	8 /* Zero, plus 6 from Atlas, plus comment */

 /* For HSMD, these (the 1-6 at least) are the "transid" fields */
#define	MBF_HSMDLDIH_NONE	0
#define	MBF_HSMDLDIH_RAW 	1 /* Raw data */
#define MBF_HSMDLDIH_NAV        2 /* Navigation data */
#define	MBF_HSMDLDIH_MDE 	3 /* MD Event */
#define	MBF_HSMDLDIH_ANG 	4 /* Beam angle data */
#define	MBF_HSMDLDIH_SVP 	5 /* Sound speed profile */
#define	MBF_HSMDLDIH_REV 	6 /* Raw event eg. start of file */
#define MBF_HSMDLDIH_COM        7 /* LDEO comment */
#define MBF_HSMDLDIH_BAT        8 /* LDEO bathymetry + raw data */

char *mbf_hsmdldih_labels[] = {
  "NONE",
  "RAW",			/* a Raw data record */
  "NAV",			/* Navigation data */
  "MDE",			/* Poke of the "Event button?" */
  "ANG",			/* Beam angle data */
  "SVP",			/* Sound Speed Profile */
  "REV",			/* Raw Event (like start and stop) */
  "COM"				/* an LDEO comment */
};

#define HEADER_ADJUST 12        /* magic offset to subract from scslng to
                                 * account for the header record
                                 */

/*********** HSMD Raw data file record types **************************/
#define RAW    1                /* Raw data */
#define NAV    2                /* Nav data */
#define MDE    3                /* MD Event */
#define ANG    4                /* Beam Angle */
#define SVP    5                /* Sound Velocity Profile */
#define REV    6                /* Raw EVent */
#define COM    7		/* LDEO comment */

/* This structure is an amalgamation of the individual strucutres used in
 * the Atlas example code and propageted into scan_md. MB requires (works
 * better with) a single structure. */

struct  mbf_hsmdldih_struct {
	int	kind;

	/* ------------------------------- Header data */
	char	scsid[4];		/* Typically "DXT" */
	char	scsart[4];		/* Typically "REI" or "RMM" */
	int	scslng;			/* length of subsequent data */
	int	scsext;			/* seems to be 0 */
	int	scsblcnt;		/* seems to be ping number */
	double	scsres1;		/* seems to be 0 */
	int	transid;		/* ID of the type of data to follow */
	double	reftime;		/* Internal time of day reference */

					/* data from a "Raw Event" */
	double	datuhr;			/* Unix epoch time + decimal seconds */
	char	mksysint[8];		/* Text message eg. "START" */
	char	mktext[84];		/* Text message eg.  "KAE HMS Start-Marke" */

	/* -------------------------------- Navigation data */
	int	navid;			/* Which nav input  */
	int	year;			/* "Date" type */
	int	month;			/* month */
	int	day;			/* day of the month */
	int	hour;			/* hour of the day */
	int	minute;			/* minute of the hour */
	int	second;			/* seconds of the minute */
	double	secf;			/* seconds of the minute */
	int	millisecond;
	double	PingTime;		/* floating point time */

	double	lat;			/* Position, decimal degrees */
	double	lon;			/* North and East are positive */
	char	pos_sens[2];		/* "G"== GPS,  "I"== Integrated */

	/* ------------------------------- From "Raw" data record */
	double	ckeel;			/* sound speed at the keel */
	double	cmean;			/* mean sound speed for water column */
	int	Port;			/* Port or Starboard ping (beamside)*/
	int	noho;			/* Indicates beam "hopping" mode */
	int	skals;			/* scale factor flag (0 -> .1, else .01 */
	int	spfb[MBF_HSMDLDIH_BEAMS_PING];	/* unscaled travel times */
	double	depth[MBF_HSMDLDIH_BEAMS_PING];		/* cross track depths */
	double	distance[MBF_HSMDLDIH_BEAMS_PING];	/* cross track distances */
	double	ss_range;				/* distance to outermost
								sidescan
								sample (meters) */
	mb_u_char	ss[MBF_HSMDLDIH_PIXELS_PING];	/* sidescan pixels */
	double	heading_tx;		/* Heading at transmit */
	double	heading_rx[5];		/* Heading during the receive window */
	double	roll_tx;		/* Roll at transmit */
	double	roll_rx[5];		/* Roll during receive window */
	double	pitch_tx;		/* Pitch at transmit */
	double	pitch_rx[5];		/* Pitch during receive window */

	/* ------------------------------- From the Angle data record */
	double	angle[MBF_HSMDLDIH_BEAMS_PING];	/* Table of beam angles */

	int	evid;			/* from MD Events */
	char	evtext[84];

	/* ------------------------------- Theoretical data from Sound Speed record */

	int	num_vel;		/* number of Depth/Sound Speed pairs */
	double	vdepth[MBF_HSMDLDIH_MAXVEL];		/* array of depths */
	double	velocity[MBF_HSMDLDIH_MAXVEL];		/* array of sound speeds */

	/* ----------------- derived data --------------------------------  */

	char	comment[MBSYS_HSMD_COMMENT]; /* I guess we will have comments
						    for MB records some day? */
	double	heave;
	double	speed;			/* not provided in HSMD */
	};

/* Atlas has expressed an intention to support "beam hopping" in the
future but all existing systems and data do not exhibit "hopping",
so we use a simple table of angles */
double mbf_hsmdldih_beamangle[] = {
   0.000,
   4.395,
   8.740,
  12.991,
  17.095,
  21.028,
  24.769,
  28.295,
  31.597,
  34.684,
  37.562,
  40.226,
  42.698,
  44.989,
  47.115,
  49.076,
  50.900,
  52.586,
  54.152,
  55.613,
  56.970,
  58.233,
  59.414,
  60.518,
  61.551,
  62.518,
  63.430,
  65.028,
  66.462,
  67.742,
  68.901,
  69.950,
  70.900,
  71.768,
  72.565,
  73.295,
  73.965,
  74.592,
  75.168,
  75.701
};
