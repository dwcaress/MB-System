/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_mr1v2001.h	3/6/2003
 *	$Id: mbsys_mr1v2001.h,v 5.0 2003-03-10 20:03:59 caress Exp $
 *
 *    Copyright (c) 2003 by
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
 * mbsys_mr1v2001.h defines the data structures used by MBIO functions
 * to store interferometry sonard data processed by the Hawaii Mapping
 * Research Group. This includes data from the MR1, SCAMP, and WHOI
 * DSL 120
 *.
 * The data formats associated with mbsys_mr1v2001 are:
 *      MBF_MR1PRVR2 : MBIO ID 63
 *
 * Author:	D. W. Caress
 * Date:	March 6, 2003
 * $Log: not supported by cvs2svn $
 *
 *
 */
/*
 * Notes on the MBSYS_MR1V2001 data structure:
 *   1. The MR1 post processing format uses the xdr external
 *      data representation for portability.
 *   2. The data stream consists of a file header followed
 *      by individual pings.
 *   3. The file header contains a comment string and the
 *      number of pings. The comment string is broken up
 *      into multiple comments by MBIO on reading; the comments
 *      are concatenated into a single string on writing.
 *   4. The pings each contain a header plus the bathymetry and/or
 *      sidescan data.
 *   6. The data structure defined below includes all of the values
 *      which are passed in the MR1 post processing format.
 *   7. This code pertains to the post-2001 version of the MR1 format.
 *   8. This format is used for data processed by HMRG, including 
 *      data from the MR1, SCAMP, and WHOI DSL 120.
 */

/* get MR1PR library header file */
#ifndef __MR1PR__
#include "mr1pr.h"
#endif

/* maximum number of bathymetry beams per side for MR1 */
#define MBSYS_MR1V2001_BEAMS_SIDE 1500

/* maximum number of sidescan pixels per side for MR1 */
#define MBSYS_MR1V2001_PIXELS_SIDE 3500

/* maximum number of bathymetry beams for MR1 */
#define MBSYS_MR1V2001_BEAMS (2*MBSYS_MR1V2001_BEAMS_SIDE + 3)

/* maximum number of sidescan pixels output for MR1 */
#define MBSYS_MR1V2001_PIXELS (2*MBSYS_MR1V2001_PIXELS_SIDE + 3)

/* maximum length of comment */
#define	MBSYS_MR1V2001_MAXLINE 256

/* angle from vertical of MR1 transducers */
#define	MBSYS_MR1V2001_XDUCER_ANGLE 50.0

struct mbsys_mr1v2001_struct
	{
	/* type of data record */
	int	kind;

	/* file header info */
	MR1File header;

	/* ping */
	Ping 	ping;
	
	/* data buffer */
	unsigned int	mr1buffersize;
	float 	*mr1buffer;
	
	/* sensors */
	float 	*compass;
	float 	*depth;
	float 	*pitch;
	float 	*roll;
	float 	*pbty;
	float 	*sbty;
	float 	*pss;
	float 	*sss;

	/* comment */
	char	comment[MBSYS_MR1V2001_MAXLINE];
};

	
/* system specific function prototypes */
int mbsys_mr1v2001_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_mr1v2001_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_mr1v2001_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_mr1v2001_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_mr1v2001_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_mr1v2001_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_mr1v2001_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_mr1v2001_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_mr1v2001_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

