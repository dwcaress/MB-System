/*--------------------------------------------------------------------
 *    The MB-system:	mb_ttimes.c	4/9/94
 *    $Id: mb_ttimes.c,v 4.9 1996-08-05 15:21:58 caress Exp $

 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_ttimes.c calls the appropriate mbsys_ routine for 
 * extracting travel times, beam angles, and bad data flags from
 * a stored survey data ping.
 * 
 * The coordinates of the beam angles can be a bit confusing.
 * The angles are returned in "takeoff angle coordinates"
 * appropriate for raytracing. The array angles contains the
 * angle from vertical (theta below) and the array angles_forward
 * contains the angle from acrosstrack (phi below). This 
 * coordinate system is distinct from the roll-pitch coordinates
 * appropriate for correcting roll and pitch values. The following
 * is a description of these relevent coordinate systems:
 * 
 * Notes on Coordinate Systems used in MB-System
 * 
 * David W. Caress
 * April 22, 1996
 * R/V Maurice Ewing, EW9602
 * 
 * I. Introduction
 * ---------------
 * The coordinate systems described below are used
 * within MB-System for calculations involving
 * the location in space of depth, amplitude, or
 * sidescan data. In all cases the origin of the
 * coordinate system is at the center of the sonar 
 * transducers.
 * 
 * II. Cartesian Coordinates
 * -------------------------
 * The cartesian coordinate system used in MB-System
 * is a bit odd because it is left-handed, as opposed
 * to the right-handed x-y-z space conventionally
 * used in most circumstances. With respect to the
 * sonar (or the ship on which the sonar is mounted),
 * the x-axis is athwartships with positive to starboard
 * (to the right if facing forward), the y-axis is
 * fore-aft with positive forward, and the z-axis is
 * positive down.
 * 
 * III. Spherical Coordinates
 * --------------------------
 * There are two non-traditional spherical coordinate 
 * systems used in MB-System. The first, referred to here 
 * as takeoff angle coordinates, is useful for raytracing.
 * The second, referred to here as roll-pitch 
 * coordinates, is useful for taking account of 
 * corrections to roll and pitch angles.
 * 
 * 1. Takeoff Angle Coordinates
 * ----------------------------
 * The three parameters are r, theta, and phi, where
 * r is the distance from the origin, theta is the
 * angle from vertical down (that is, from the 
 * positive z-axis), and phi is the angle from 
 * acrosstrack (the positive x-axis) in the x-y plane.
 * Note that theta is always positive; the direction
 * in the x-y plane is given by phi.
 * Raytracing is simple in these coordinates because
 * the ray takeoff angle is just theta. However,
 * applying roll or pitch corrections is complicated because
 * roll and pitch have components in both theta and phi.
 * 
 * 	0 <= theta <= PI/2
 * 	-PI/2 <= phi <= 3*PI/2
 * 
 * 	x = r * SIN(theta) * COS(phi) 
 * 	y = r * SIN(theta) * SIN(phi)
 * 	z = r * COS(theta) 
 * 	
 * 	theta = 0    ---> vertical, along positive z-axis
 * 	theta = PI/2 ---> horizontal, in x-y plane
 * 	phi = -PI/2  ---> aft, in y-z plane with y negative
 * 	phi = 0      ---> port, in x-z plane with x positive
 * 	phi = PI/2   ---> forward, in y-z plane with y positive
 * 	phi = PI     ---> starboard, in x-z plane with x negative
 * 	phi = 3*PI/2 ---> aft, in y-z plane with y negative
 * 
 * 2. Roll-Pitch Coordinates
 * -------------------------
 * The three parameters are r, alpha, and beta, where
 * r is the distance from the origin, alpha is the angle 
 * forward (effectively pitch angle), and beta is the
 * angle from vertical down in the x-z plane (effectively
 * roll angle). Applying a roll or pitch correction is 
 * simple in these coordinates because pitch is just alpha 
 * and roll is just beta. However, raytracing is complicated 
 * because deflection from vertical has components in both 
 * alpha and beta.
 * 
 * 	-PI/2 <= alpha <= PI/2
 * 	0 <= beta <= PI
 * 	
 * 	x = r * COS(alpha) * SIN(beta) 
 * 	y = r * SIN(alpha)
 * 	z = r * COS(alpha) * COS(beta) 
 * 	
 * 	alpha = -PI/2 ---> horizontal, in x-y plane with y negative
 * 	alpha = 0     ---> ship level, zero pitch, in x-z plane
 * 	alpha = PI/2  ---> horizontal, in x-y plane with y positive
 * 	beta = 0      ---> starboard, along positive x-axis
 * 	beta = PI/2   ---> in y-z plane rotated by alpha
 * 	beta = PI     ---> port, along negative x-axis
 * 
 * IV. SeaBeam Coordinates
 * ----------------------
 * The per-beam parameters in the SB2100 data format include
 * angle-from-vertical and angle-forward. Angle-from-vertical
 * is the same as theta except that it is signed based on
 * the acrosstrack direction (positive to starboard, negative 
 * to port). The angle-forward values are also defined 
 * slightly differently from phi, in that angle-forward is 
 * signed differently on the port and starboard sides. The 
 * SeaBeam 2100 External Interface Specifications document 
 * includes both discussion and figures illustrating the 
 * angle-forward value. To summarize:
 * 
 *     Port:
 *     
 * 	theta = absolute value of angle-from-vertical
 * 	
 * 	-PI/2 <= phi <= PI/2  
 * 	is equivalent to 
 * 	-PI/2 <= angle-forward <= PI/2
 * 	
 * 	phi = -PI/2 ---> angle-forward = -PI/2 (aft)
 * 	phi = 0     ---> angle-forward = 0     (starboard)
 * 	phi = PI/2  ---> angle-forward = PI/2  (forward)
 * 
 *     Starboard:
 * 	
 * 	theta = angle-from-vertical
 *     
 * 	PI/2 <= phi <= 3*PI/2 
 * 	is equivalent to 
 * 	-PI/2 <= angle-forward <= PI/2
 * 	
 * 	phi = PI/2   ---> angle-forward = -PI/2 (forward)
 * 	phi = PI     ---> angle-forward = 0     (port)
 * 	phi = 3*PI/2 ---> angle-forward = PI/2  (aft)
 * 
 * V. Usage of Coordinate Systems in MB-System
 * ------------------------------------------
 * Some sonar data formats provide angle values along with
 * travel times. The angles are converted to takoff-angle 
 * coordinates regardless of the  storage form of the 
 * particular data format. Currently, most data formats
 * do not contain an alongtrack component to the position
 * values; in these cases the conversion is trivial since
 * phi = beta = 0 and theta = alpha. The angle and travel time 
 * values can be accessed using the MBIO function mb_ttimes.
 * All angle values passed by MB-System functions are in
 * degrees rather than radians.
 * 
 * The programs mbbath and mbvelocitytool use angles in
 * take-off angle coordinates to do the raytracing. If roll
 * and/or pitch corrections are to be made, the angles are
 * converted to roll-pitch coordinates, corrected, and then
 * converted back prior to raytracing.
 * 
 * The SeaBeam patch test tool SeaPatch calculates angles
 * in roll-pitch coordinates from the initial bathymetry
 * and then applies whatever roll and pitch corrections are
 * set interactively by the user.
 * 
 *
 * Author:	D. W. Caress
 * Date:	April 9, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/03/12  17:21:55  caress
 * Added format 63, short HMR1 processing format.
 *
 * Revision 4.6  1995/11/27  21:49:01  caress
 * New version of mb_ttimes with ssv and angles_null.
 *
 * Revision 4.5  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1994/11/09  21:40:34  caress
 * Changed ttimes extraction routines to handle forward beam angles
 * so that alongtrack distances can be calculated.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/04/11  23:37:14  caress
 * Setting version number properly.
 *
 * Revision 1.1  1994/04/11  23:34:41  caress
 * Initial revision
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/
int mb_ttimes(verbose,mbio_ptr,store_ptr,kind,nbeams,ttimes,
	angles,angles_forward,angles_null,flags,
	depthadd,ssv,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	*nbeams;
double	*ttimes;
double	*angles;
double	*angles_forward;
double	*angles_null;
int	*flags;
double	*depthadd;
double	*ssv;
int	*error;
{
  static char rcs_id[]="$Id: mb_ttimes.c,v 4.9 1996-08-05 15:21:58 caress Exp $";
	char	*function_name = "mb_ttimes";
	int	status;
	int	system;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call the appropriate mbsys_ extraction routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_MR1B)
		{
		status = mbsys_mr1b_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_HSMD)
		{
		status = mbsys_hsmd_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		fprintf(stderr,"dbg2       depthadd:   %f\n",*depthadd);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
