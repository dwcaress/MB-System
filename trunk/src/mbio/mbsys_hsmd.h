/* --------------------------------------------------------------------
 *    The MB-system:	mbsys_hsmd.h	8/12/95
 *	$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbsys_hsmd.h,v 5.2 2001-07-20 00:32:54 caress Exp $
 *
 *    Copyright (c) 1995, 2000 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 * --------------------------------------------------------------------
 *
 * mbsys_hsmd.h defines the data structures used by MBIO functions
 * to store data from Hydrosweep MD (Medium Depth) multibeam sonar systems.
 * The data formats which are commonly used to store Hydrosweep MD (Medium
 * Depth data in files include
 *      MBF_HSMBARAW : MBIO ID 101
 *      MBF_HSMBLDIH : MBIO ID 102
 *
 * Based on mbsys_hsds.h
 *
 * Author:	Dale Chayes
 * 		David W. Caress
 * Date:	August 10, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.2  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.2  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1995/09/28  18:14:11  caress
 * First cut.
 *
 * Revision 4.0  1995/09/28  18:14:11  caress
 * First cut.
 *
 * Revision 1.1  1995/09/28  18:10:48  caress
 * Initial revision
 *
 * Revision 4.2  95/08/16  07:08:50  07:08:50  dale (Dale Chayes)
 * Works but not quite right.
 * 
 * Revision 4.1  95/08/14  20:56:19  20:56:19  dale (Dale Chayes)
 * HSMD is sort of working.... needs work and testing.
 * 
 * Revision 4.0  95/08/10  15:58:27  15:58:27  dale (Dale Chayes)
 * first crack, with formatting and "hs" ds" -> "md" conversions but
 * not with substantial functional changes.
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
 *      amplitude measurements), wait for all the returns and then ping
 *      to the other side. The "Raw.beamside" variable indicates which side
 *      the current ping is pointed. I've elected to call it "Pirt" for now.
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
 *      which are passed in Hydrosweep MD records.
 *
 *   6. Comment records have been implemented for use with MB-System
 *      programs. Comment records are NOT part of the vendor format
 *      definition and are NOT supported by Atlas software. Using
 *      mbcopy with the -N option will remove all comments and make
 *      the data compatible with Atlas software.
 */


/* maximum number of depth-velocity pairs */
#define MBSYS_HSMD_MAXVEL 20	/* As dimensioned in the Atlas code */

/* array sizes */
#define MBSYS_HSMD_BEAMS	79
#define MBSYS_HSMD_PIXELS	319 
#define MBSYS_HSMD_BEAMS_PING	40
#define MBSYS_HSMD_PIXELS_PING	160 
#define MBSYS_HSMD_COMMENT	128	/* length of a comment string */

struct mbsys_hsmd_struct
	{
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
	int	spfb[MBSYS_HSMD_BEAMS_PING];		/* unscaled travel times */
	double	depth[MBSYS_HSMD_BEAMS_PING];		/* cross track depths */
	double	distance[MBSYS_HSMD_BEAMS_PING];	/* cross track distances */
	double	ss_range;				/* distance to outermost 
								sidescan 
								sample (meters) */
	mb_u_char	ss[MBSYS_HSMD_PIXELS_PING];	/* sidescan pixels */
	double	heading_tx;		/* Heading at transmit */
	double	heading_rx[5];		/* Heading during the receive window */
	double	roll_tx;		/* Roll at transmit */
	double	roll_rx[5];		/* Roll during receive window */
	double	pitch_tx;		/* Pitch at transmit */
	double	pitch_rx[5];		/* Pitch during receive window */

	/* ------------------------------- From the Angle data record */
	double	angle[MBSYS_HSMD_BEAMS_PING];	/* Table of beam angles */

	int	evid;			/* from MD Events */
	char	evtext[84];

	/* ------------------------------- Theoretical data from Sound Speed record */

	int	num_vel;		/* number of Depth/Sound Speed pairs */
	double	vdepth[20];		/* array of depths */
	double	velocity[20];		/* array of sound speeds */

	/* ----------------- derived data --------------------------------  */

	char	comment[MBSYS_HSMD_COMMENT]; /* I guess we will have comments
						    for MB records some day? */
	double	heave;
	double	speed;			/* not provided in HSMD */
	};	
	
/* system specific function prototypes */
int mbsys_hsmd_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_hsmd_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_hsmd_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hsmd_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hsmd_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_hsmd_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_hsmd_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_hsmd_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_hsmd_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

