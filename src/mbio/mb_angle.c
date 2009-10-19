/*--------------------------------------------------------------------
 *    The MB-system:	mb_angle.c	1/21/93
 *    $Id: mb_angle.c,v 5.9 2009/02/06 19:12:43 caress Exp $
 *
 *    Copyright (c) 1998-2008 by
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
 * mb_angle.c includes the "mb_" functions used to translate between
 * the two coordinate systems that are relevent to the calculation
 * of bathymetry data. The following
 * is a description of these coordinate systems:
 * 
 * Notes on Coordinate Systems used in MB-System
 * 
 * David W. Caress
 * April 22, 1996
 * Updated January 16, 2009
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
 * The three parameters are r, pitch, and roll, where
 * r is the distance from the origin, pitch is the angle 
 * forward (effectively pitch angle), and roll is the
 * angle from horizontal in the x-z plane (effectively
 * roll angle). Applying a roll or pitch correction is 
 * simple in these coordinates because pitch is just pitch 
 * and roll is just roll. However, raytracing is complicated 
 * because deflection from vertical has components in both 
 * pitch and roll.
 * 
 * 	-PI/2 <= pitch <= PI/2
 * 	0 <= roll <= PI
 * 	
 * 	x = r * COS(pitch) * COS(roll) 
 * 	y = r * SIN(pitch)
 * 	z = r * COS(pitch) * SIN(roll) 
 * 	
 * 	pitch = -PI/2 ---> horizontal, in x-y plane with y negative
 * 	pitch = 0     ---> ship level, zero pitch, in x-z plane
 * 	pitch = PI/2  ---> horizontal, in x-y plane with y positive
 * 	roll = 0      ---> starboard, along positive x-axis
 * 	roll = PI/2   ---> in y-z plane rotated by pitch
 * 	roll = PI     ---> port, along negative x-axis
 * 
 * IV. An Example of Vendor-Specific Coordinates: SeaBeam 2100
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
 * particular data format. Older data formats often
 * do not contain an alongtrack component to the position
 * values; in these cases the conversion is trivial since
 * phi = roll = 0 and theta = pitch. The angle and travel time 
 * values can be accessed using the MBIO function mb_ttimes.
 * All angle values passed by MB-System functions are in
 * degrees rather than radians.
 * 
 * The programs mbprocess and mbvelocitytool use angles in
 * take-off angle coordinates to do the raytracing. If roll
 * and/or pitch corrections are to be made, the angles are
 * converted to roll-pitch coordinates, corrected, and then
 * converted back prior to raytracing.
 * 
 * When used for the patch test function, MBeditviz can apply
 * roll and pitch bias corrections interactively. This is 
 * accomplished by first calculating the effective angles in 
 * roll-pitch coordinates from each beam (sounding) using the depth
 * (relative to the sonar), the acrosstrack distance, and the
 * alongtrack distance, The corrections in roll and pitch are
 * then applied, and the depth and distances recalculated from
 * from the corrected angles.
 * 
 *
 * Author:	D. W. Caress
 * Date:	December 30, 1998
 *
 * $Log: mb_angle.c,v $
 * Revision 5.9  2009/02/06 19:12:43  caress
 * Fixed description in mb_angle.c and angle extraction in mb_simrad3.c
 *
 * Revision 5.8  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.7  2006/12/15 21:35:31  caress
 * Fixed longstanding bug in coordinate translation code. Previously, the application of roll and pitch angles was done in the wrong order (pitch first). The errors due to this bug only became significant with large pitch values. The rotations are now done properly.
 *
 * Revision 5.6  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2003/01/15 20:51:48  caress
 * Release 5.0.beta28
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.1  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/01/01  23:38:01  caress
 * MB-System version 4.6beta6
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/
int mb_takeoff_to_rollpitch(int verbose,
		double theta, double phi,
		double *pitch, double *roll,
		int *error)
{
	char	*function_name = "mb_takeoff_to_rollpitch";
	int	status = MB_SUCCESS;
	double	x, y, z;
	double	asinroll;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       theta:      %f\n",theta);
		fprintf(stderr,"dbg2       phi:        %f\n",phi);
		}
		
	/* convert to cartesian coordinates */
	x = sin(DTR * theta) * cos(DTR * phi);
	y = sin(DTR * theta) * sin(DTR * phi);
	z = cos(DTR * theta);

	/* convert to roll-pitch coordinates */
	*roll = acos(x);
	*pitch = asin(y / sin(*roll));
	*pitch *= RTD;
	*roll *= RTD;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pitch:           %f\n",*pitch);
		fprintf(stderr,"dbg2       roll:            %f\n",*roll);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_rollpitch_to_takeoff(int verbose,
		double pitch, double roll,
		double *theta, double *phi,
		int *error)
{
	char	*function_name = "mb_rollpitch_to_takeoff";
	int	status = MB_SUCCESS;
	double	x, y, z;
	double	sintheta;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		}
		
	/* convert to cartesian coordinates */
	x = cos(DTR * roll);
	y = sin(DTR * pitch) * sin(DTR * roll);
	z = cos(DTR * pitch) * sin(DTR * roll);

	/* convert to takeoff angle coordinates */
	*theta = acos(z);
	sintheta = sin(*theta);
	if (fabs(sintheta) < 0.00001)
		{
		*phi = 0.0;
		}
	else
		{
		*phi = atan2(y,x);
		}
	*theta *= RTD;
	*phi *= RTD;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       theta:           %f\n",*theta);
		fprintf(stderr,"dbg2       phi:             %f\n",*phi);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_xyz_to_takeoff(int verbose,
		double x, double y, double z,
		double *theta, double *phi,
		int *error)
{
	char	*function_name = "mb_xyz_to_takeoff";
	int	status = MB_SUCCESS;
	double	aa, xx, yy, zz, rr;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       x:          %f\n",x);
		fprintf(stderr,"dbg2       y:          %f\n",y);
		fprintf(stderr,"dbg2       z:          %f\n",z);
		}
		
	/* normalize cartesian coordinates */
	rr = sqrt(x * x + y * y + z * z);
	xx = x / rr;
	yy = y / rr;
	zz = z / rr;

	/* convert to takeoff angle coordinates */
	*theta = acos(zz);
	if (zz < 1.0)
	    aa = yy / sin(*theta);
	else 
	    aa = 0.0;
	if (aa > 1.0)
	    *phi = 0.5 * M_PI;
	else if (aa < -1.0)
	    *phi = -0.5 * M_PI;
	else
	    *phi = asin(aa);
	*theta *= RTD;
	*phi *= RTD;
	if (xx < 0.0)
		*phi = 180.0 - *phi;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       theta:           %f\n",*theta);
		fprintf(stderr,"dbg2       phi:             %f\n",*phi);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_lever(int verbose,
		double sonar_offset_x,
		double sonar_offset_y,
		double sonar_offset_z,
		double nav_offset_x,
		double nav_offset_y,
		double nav_offset_z,
		double vru_offset_x,
		double vru_offset_y,
		double vru_offset_z,
		double vru_pitch,
		double vru_roll,
		double *lever_x,
		double *lever_y,
		double *lever_z,
		int *error)
{
	char	*function_name = "mb_lever";
	int	status = MB_SUCCESS;
	double	x, y, z;
	double	xx, yy, zz, r;
	double	pitch, roll;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       sonar_offset_x: %f\n",sonar_offset_x);
		fprintf(stderr,"dbg2       sonar_offset_y: %f\n",sonar_offset_y);
		fprintf(stderr,"dbg2       sonar_offset_z: %f\n",sonar_offset_z);
		fprintf(stderr,"dbg2       nav_offset_x:   %f\n",nav_offset_x);
		fprintf(stderr,"dbg2       nav_offset_y:   %f\n",nav_offset_y);
		fprintf(stderr,"dbg2       nav_offset_z:   %f\n",nav_offset_z);
		fprintf(stderr,"dbg2       vru_offset_x:   %f\n",vru_offset_x);
		fprintf(stderr,"dbg2       vru_offset_y:   %f\n",vru_offset_y);
		fprintf(stderr,"dbg2       vru_offset_z:   %f\n",vru_offset_z);
		fprintf(stderr,"dbg2       vru_pitch:      %f\n",vru_pitch);
		fprintf(stderr,"dbg2       vru_roll:       %f\n",vru_roll);
		}

	/* do lever calculation to find heave implied by roll and pitch
	   for a sonar displaced from the vru:
		x = r * COS(pitch) * COS(roll) 
		y = r * SIN(pitch)
		z = r * COS(pitch) * SIN(roll) */
	/* get net offset between sonar and vru */
	xx = sonar_offset_x - vru_offset_x;
	yy = sonar_offset_y - vru_offset_y;
	zz = sonar_offset_z - vru_offset_z;
	r = sqrt(xx * xx + yy * yy + zz * zz);
	
	/* lever arm only matters if offset is nonzero */
	if (r > 0.0)
	    {
	    /* get initial angles */
	    pitch = RTD * asin(yy / r);
	    if (cos(DTR * pitch) != 0.0)
		roll = RTD * acos(xx / (r * cos(DTR * pitch)));
	    else
		roll = 0.0;
		
  	    /* apply angle change */
	    pitch += vru_pitch;
	    roll += vru_roll;
	    
	    /* calculate new offsets */
	    x = r * cos(DTR * roll);
	    y = r * sin(DTR * pitch) * sin(DTR * roll);
	    z = r * cos(DTR * pitch) * sin(DTR * roll);
	    
	    /* get heave change due to lever arm */
	    *lever_z =  z - zz;
	    }
	else
	    {
	    *lever_z = 0.0;
	    }

	/* do lever calculation to find position shift implied by roll and pitch
	   for a sonar displaced from the nav sensor:
		x = r * COS(pitch) * COS(roll) 
		y = r * SIN(pitch)
		z = r * COS(pitch) * SIN(roll) */
	/* get net offset between sonar and nav sensor */
	xx = sonar_offset_x - nav_offset_x;
	yy = sonar_offset_y - nav_offset_y;
	zz = sonar_offset_z - nav_offset_z;
	r = sqrt(xx * xx + yy * yy + zz * zz);
	
	/* lever arm only matters if offset is nonzero */
	if (r > 0.0)
	    {
	    /* get initial angles */
	    pitch = RTD * asin(yy / r);
	    if (cos(DTR * pitch) != 0.0)
		roll = RTD * acos(xx / (r * cos(DTR * pitch)));
	    else
		roll = 0.0;
		
  	    /* apply angle change */
	    pitch += vru_pitch;
	    roll += vru_roll;
	    
	    /* calculate new offsets */
	    x = r * cos(DTR * roll);
	    y = r * sin(DTR * pitch) * sin(DTR * roll);
	    z = r * cos(DTR * pitch) * sin(DTR * roll);
	    
	    /* get position change due to lever arm */
	    *lever_x =  x - xx;
	    *lever_y =  y - yy;
	    }
	else
	    {
	    *lever_x = 0.0;
	    *lever_y = 0.0;
	    }

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lever_x:         %f\n",*lever_x);
		fprintf(stderr,"dbg2       lever_y:         %f\n",*lever_y);
		fprintf(stderr,"dbg2       lever_z:         %f\n",*lever_z);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

