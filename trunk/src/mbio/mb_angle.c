/*--------------------------------------------------------------------
 *    The MB-system:	mb_angle.c	1/21/93
 *    $Id: mb_angle.c,v 4.1 2000-09-30 06:26:58 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * angle from horizontal in the x-z plane (effectively
 * roll angle). Applying a roll or pitch correction is 
 * simple in these coordinates because pitch is just alpha 
 * and roll is just beta. However, raytracing is complicated 
 * because deflection from vertical has components in both 
 * alpha and beta.
 * 
 * 	-PI/2 <= alpha <= PI/2
 * 	0 <= beta <= PI
 * 	
 * 	x = r * COS(alpha) * COS(beta) 
 * 	y = r * SIN(alpha)
 * 	z = r * COS(alpha) * SIN(beta) 
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
 * Date:	December 30, 1998
 *
 * $Log: not supported by cvs2svn $
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
int mb_takeoff_to_rollpitch(verbose,theta,phi,alpha,beta,error)
int	verbose;
double	theta;
double	phi;
double	*alpha;
double	*beta;
int	*error;
{
	char	*function_name = "mb_takeoff_to_rollpitch";
	int	status = MB_SUCCESS;
	double	x, y, z;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> called\n",
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
	*alpha = asin(y);
	*beta = acos(x / cos(*alpha));
	*alpha *= RTD;
	*beta *= RTD;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       alpha:           %f\n",*alpha);
		fprintf(stderr,"dbg2       beta:            %f\n",*beta);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_rollpitch_to_takeoff(verbose,alpha,beta,theta,phi,error)
int	verbose;
double	alpha;
double	beta;
double	*theta;
double	*phi;
int	*error;
{
	char	*function_name = "mb_rollpitch_to_takeoff";
	int	status = MB_SUCCESS;
	double	x, y, z;
	double	aa;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       alpha:      %f\n",alpha);
		fprintf(stderr,"dbg2       beta:       %f\n",beta);
		}
		
	/* convert to cartesian coordinates */
	x = cos(DTR * alpha) * cos(DTR * beta);
	y = sin(DTR * alpha);
	z = cos(DTR * alpha) * sin(DTR * beta);

	/* convert to takeoff angle coordinates */
	*theta = acos(z);
	aa = y / sin(*theta);
	if (aa > 1.0)
	    *phi = 0.5 * M_PI;
	else if (aa < -1.0)
	    *phi = -0.5 * M_PI;
	else
	    *phi = asin(aa);
	*theta *= RTD;
	*phi *= RTD;
	if (x < 0.0)
		*phi = 180.0 - *phi;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBATH function <%s> completed\n",
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
