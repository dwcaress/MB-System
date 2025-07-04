/*--------------------------------------------------------------------
 *    The MB-system:  mb_angle.c  1/21/93
 *
 *    Copyright (c) 1998-2025 by
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
 * mb_angle.c includes the "mb_" functions used to translate between
 * the two coordinate systems that are relevant to the calculation
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
 *   0 <= theta <= PI/2
 *   -PI/2 <= phi <= 3*PI/2
 *
 *   x = r * SIN(theta) * COS(phi)
 *   y = r * SIN(theta) * SIN(phi)
 *   z = r * COS(theta)
 *
 *   theta = 0    ---> vertical, along positive z-axis
 *   theta = PI/2 ---> horizontal, in x-y plane
 *   phi = -PI/2  ---> aft, in y-z plane with y negative
 *   phi = 0      ---> port, in x-z plane with x positive
 *   phi = PI/2   ---> forward, in y-z plane with y positive
 *   phi = PI     ---> starboard, in x-z plane with x negative
 *   phi = 3*PI/2 ---> aft, in y-z plane with y negative
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
 *   -PI/2 <= pitch <= PI/2
 *   0 <= roll <= PI
 *
 *   x = r * COS(pitch) * COS(roll)
 *   y = r * SIN(pitch)
 *   z = r * COS(pitch) * SIN(roll)
 *
 *   pitch = -PI/2 ---> horizontal, in x-y plane with y negative
 *   pitch = 0     ---> ship level, zero pitch, in x-z plane
 *   pitch = PI/2  ---> horizontal, in x-y plane with y positive
 *   roll = 0      ---> starboard, along positive x-axis
 *   roll = PI/2   ---> in y-z plane rotated by pitch
 *   roll = PI     ---> port, along negative x-axis
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
 *   theta = absolute value of angle-from-vertical
 *
 *   -PI/2 <= phi <= PI/2
 *   is equivalent to
 *   -PI/2 <= angle-forward <= PI/2
 *
 *   phi = -PI/2 ---> angle-forward = -PI/2 (aft)
 *   phi = 0     ---> angle-forward = 0     (starboard)
 *   phi = PI/2  ---> angle-forward = PI/2  (forward)
 *
 *     Starboard:
 *
 *   theta = angle-from-vertical
 *
 *   PI/2 <= phi <= 3*PI/2
 *   is equivalent to
 *   -PI/2 <= angle-forward <= PI/2
 *
 *   phi = PI/2   ---> angle-forward = -PI/2 (forward)
 *   phi = PI     ---> angle-forward = 0     (port)
 *   phi = 3*PI/2 ---> angle-forward = PI/2  (aft)
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
 * Author:  D. W. Caress
 * Date:  December 30, 1998
 */

#include <math.h>
#include <stdio.h>

#include "mb_define.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_takeoff_to_rollpitch(int verbose, double theta, double phi, double *pitch, double *roll, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       theta:      %f\n", theta);
    fprintf(stderr, "dbg2       phi:        %f\n", phi);
  }

  /* convert to cartesian coordinates */
  const double x = sin(DTR * theta) * cos(DTR * phi);
  const double y = sin(DTR * theta) * sin(DTR * phi);
  //const double z = cos(DTR * theta);

  /* convert to roll-pitch coordinates */
  *roll = acos(x);
  *pitch = asin(y / sin(*roll));
  *pitch *= RTD;
  *roll *= RTD;

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pitch:           %f\n", *pitch);
    fprintf(stderr, "dbg2       roll:            %f\n", *roll);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_rollpitch_to_takeoff(int verbose, double pitch, double roll, double *theta, double *phi, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
    fprintf(stderr, "dbg2       roll:       %f\n", roll);
  }

  /* convert to cartesian coordinates */
  const double x = cos(DTR * roll);
  const double y = sin(DTR * pitch) * sin(DTR * roll);
  const double z = cos(DTR * pitch) * sin(DTR * roll);

  /* convert to takeoff angle coordinates */
  *theta = acos(z);
  const double sintheta = sin(*theta);
  if (fabs(sintheta) < 0.00001) {
    *phi = 0.0;
  }
  else {
    *phi = atan2(y, x);
  }
  *theta *= RTD;
  *phi *= RTD;

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       theta:           %f\n", *theta);
    fprintf(stderr, "dbg2       phi:             %f\n", *phi);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_xyz_to_takeoff(int verbose, double x, double y, double z, double *theta, double *phi, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       x:          %f\n", x);
    fprintf(stderr, "dbg2       y:          %f\n", y);
    fprintf(stderr, "dbg2       z:          %f\n", z);
  }

  /* normalize cartesian coordinates */
  const double rr = sqrt(x * x + y * y + z * z);
  const double xx = x / rr;
  const double yy = y / rr;
  const double zz = z / rr;

  /* convert to takeoff angle coordinates */
  *theta = acos(zz);
  double aa;
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
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       theta:           %f\n", *theta);
    fprintf(stderr, "dbg2       phi:             %f\n", *phi);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_lever(int verbose, double sonar_offset_x, double sonar_offset_y, double sonar_offset_z, double nav_offset_x,
             double nav_offset_y, double nav_offset_z, double vru_offset_x, double vru_offset_y, double vru_offset_z,
             double vru_pitch, double vru_roll, double *lever_x, double *lever_y, double *lever_z, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       sonar_offset_x: %f\n", sonar_offset_x);
    fprintf(stderr, "dbg2       sonar_offset_y: %f\n", sonar_offset_y);
    fprintf(stderr, "dbg2       sonar_offset_z: %f\n", sonar_offset_z);
    fprintf(stderr, "dbg2       nav_offset_x:   %f\n", nav_offset_x);
    fprintf(stderr, "dbg2       nav_offset_y:   %f\n", nav_offset_y);
    fprintf(stderr, "dbg2       nav_offset_z:   %f\n", nav_offset_z);
    fprintf(stderr, "dbg2       vru_offset_x:   %f\n", vru_offset_x);
    fprintf(stderr, "dbg2       vru_offset_y:   %f\n", vru_offset_y);
    fprintf(stderr, "dbg2       vru_offset_z:   %f\n", vru_offset_z);
    fprintf(stderr, "dbg2       vru_pitch:      %f\n", vru_pitch);
    fprintf(stderr, "dbg2       vru_roll:       %f\n", vru_roll);
  }

  /* do lever calculation to find heave implied by roll and pitch
     for a sonar displaced from the vru:
      x = r * COS(pitch) * COS(roll)
      y = r * SIN(pitch)
      z = r * COS(pitch) * SIN(roll) */
  /* get net offset between sonar and vru */
  double xx = sonar_offset_x - vru_offset_x;
  double yy = sonar_offset_y - vru_offset_y;
  double zz = sonar_offset_z - vru_offset_z;
  double r = sqrt(xx * xx + yy * yy + zz * zz);

  /* lever arm only matters if offset is nonzero */
  if (r > 0.0) {
    /* get initial angles */
    double roll = RTD * acos(xx / r);
    if (zz < 0.0)
      roll = -roll;
    double pitch;
    if (sin(DTR * roll) != 0.0)
      pitch = RTD * asin(yy / (r * sin(DTR * roll)));
    else
      pitch = 0.0;

    /* apply angle change */
    pitch += vru_pitch;
    roll += vru_roll;

    /* calculate new offsets */
    // const double x = r * cos(DTR * roll);
    // const double y = r * sin(DTR * pitch) * sin(DTR * roll);
    const double z = r * cos(DTR * pitch) * sin(DTR * roll);

    /* get heave change due to lever arm */
    *lever_z = z - zz;
  }
  else {
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
  if (r > 0.0) {
    /* get initial angles */
    double roll = RTD * acos(xx / r);
    double pitch;
    if (sin(DTR * roll) != 0.0)
      pitch = RTD * asin(yy / (r * sin(DTR * roll)));
    else
      pitch = 0.0;

    /* apply angle change */
    pitch += vru_pitch;
    roll += vru_roll;

    /* calculate new offsets */
    const double x = r * cos(DTR * roll);
    const double y = r * sin(DTR * pitch) * sin(DTR * roll);
    // const double z = r * cos(DTR * pitch) * sin(DTR * roll);

    /* get position change due to lever arm */
    *lever_x = x - xx;
    *lever_y = y - yy;
  }
  else {
    *lever_x = 0.0;
    *lever_y = 0.0;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       lever_x:         %f\n", *lever_x);
    fprintf(stderr, "dbg2       lever_y:         %f\n", *lever_y);
    fprintf(stderr, "dbg2       lever_z:         %f\n", *lever_z);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
 * Calculate multibeam beam angles for bathymetry calculation by raytracing
 * from the transducer array orientations and beam steering angles.
 * This code was written by Jonathan Beaudoin and John Hughes Clarke,
 * provided to the MB-System team by Jonathan Beaudoin, and
 * derives from the paper:
 *
 *   Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
 *          Surface Sound Speed Measurements in Post-Processing for
 *          Multi-Sector Multibeam Echosounders : International
 *          Hydrographic Review, v.5, no.3, p.26-31.
 *      (available for download at
 *       http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
 *
 * The input consists of 14 angles:
 *   Transmit array installation angles: roll, pitch, heading
 *   Receive array installation angles: roll, pitch, heading
 *   Roll, pitch, and heading at transmit time
 *   Roll, pitch, and heading at receive time
 *   Transmit fore-aft pitch steering angle, can be unique by sector
 *       with Kongsberg systems (+ve angle is forward)
 *   RX steer angle (+ve angle is to port, this is consistent with
 *       the Roll sign convention but opposite of what most people expect)
 *
 * The output consists of the beam azimuthal and depression angles.
 *
 * Several Kongsberg multisector multibeams have arrays that can be reverse mounted.
 * The EM710 and EM2040 can have TX and RX independently mounted so one or both of
 * the TX and RX can be reverse mounted. If a transmit or receive array is reverse
 * mounted, the code handles this by:
 *      1) subtracting 180 degrees from the heading mount angle of the array
 *      2) flipping the sign of the pitch and roll mount offsets of the array
 *      3) flipping the sign of the beam steering angle from that array
 *         (reverse TX means flip sign of TX steer, reverse RX means flip sign of RX steer)
 */

int mb_beaudoin(int verbose, mb_3D_orientation tx_align, mb_3D_orientation tx_orientation, double tx_steer,
                mb_3D_orientation rx_align, mb_3D_orientation rx_orientation, double rx_steer, double reference_heading,
                double *beamAzimuth, double *beamDepression, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       tx_align.roll:           %f\n", tx_align.roll);
    fprintf(stderr, "dbg2       tx_align.pitch:          %f\n", tx_align.pitch);
    fprintf(stderr, "dbg2       tx_align.heading:        %f\n", tx_align.heading);
    fprintf(stderr, "dbg2       tx_orientation.roll:     %f\n", tx_orientation.roll);
    fprintf(stderr, "dbg2       tx_orientation.pitch:    %f\n", tx_orientation.pitch);
    fprintf(stderr, "dbg2       tx_orientation.heading:  %f\n", tx_orientation.heading);
    fprintf(stderr, "dbg2       tx_steer:                %f\n", tx_steer);
    fprintf(stderr, "dbg2       rx_align.roll:           %f\n", rx_align.roll);
    fprintf(stderr, "dbg2       rx_align.pitch:          %f\n", rx_align.pitch);
    fprintf(stderr, "dbg2       rx_align.heading:        %f\n", rx_align.heading);
    fprintf(stderr, "dbg2       rx_orientation.roll:     %f\n", rx_orientation.roll);
    fprintf(stderr, "dbg2       rx_orientation.pitch:    %f\n", rx_orientation.pitch);
    fprintf(stderr, "dbg2       rx_orientation.heading:  %f\n", rx_orientation.heading);
    fprintf(stderr, "dbg2       rx_steer:                %f\n", rx_steer);
    fprintf(stderr, "dbg2       reference_heading:       %f\n", reference_heading);
  }
  mb_3D_vector txIdeal;

  txIdeal.x = 1.0;
  txIdeal.y = 0.0;
  txIdeal.z = 0.0;

  // All of these in degrees, these are the transmitter array mount angles.
  mb_3D_orientation datt;
  datt.roll = tx_align.roll;
  datt.pitch = tx_align.pitch;
  datt.heading = tx_align.heading;
  mb_3D_vector txMount;
  mb_beaudoin_unrotate(verbose, txIdeal, datt, &txMount, error);

  // All of these in degrees, these are the motion measurements at the time of transmit.
  datt.roll = tx_orientation.roll;
  datt.pitch = tx_orientation.pitch;
  datt.heading = tx_orientation.heading;
  mb_3D_vector txGeo;
  mb_beaudoin_unrotate(verbose, txMount, datt, &txGeo, error);

  if (verbose >= 4)
    fprintf(stderr, "dbg4      TX array x %f y %f z %f in geographic reference frame\n", txGeo.x, txGeo.y, txGeo.z);

  mb_3D_vector rxIdeal;
  rxIdeal.x = 0.0;
  rxIdeal.y = 1.0;
  rxIdeal.z = 0.0;

  // Degrees, these are the receiver array mount angles.
  datt.roll = rx_align.roll;
  datt.pitch = rx_align.pitch;
  datt.heading = rx_align.heading;
  mb_3D_vector rxMount;
  mb_beaudoin_unrotate(verbose, rxIdeal, datt, &rxMount, error);

  // Degrees, these are the motion measurements at the time of reception (this is unique per beam)
  datt.roll = rx_orientation.roll;
  datt.pitch = rx_orientation.pitch;
  datt.heading = rx_orientation.heading;
  mb_3D_vector rxGeo;
  mb_beaudoin_unrotate(verbose, rxMount, datt, &rxGeo, error);

  if (verbose >= 4)
    fprintf(stderr, "dbg4     RX array x %f y %f z %f in geographic reference frame\n", rxGeo.x, rxGeo.y, rxGeo.z);

  /* Have to negate it so signs work out...(was 90 - acos(...) before)  */
  /* acos of dotproduct of rxGeo and txGeo yields angle from between vectors */
  /* ...subtract 90 to bring into required units */
  double non_ortho = acos(rxGeo.x * txGeo.x + rxGeo.y * txGeo.y + rxGeo.z * txGeo.z) * 180.0 / M_PI - 90.0;

  if (verbose >= 4)
    fprintf(stderr, "dbg4     TX/RX are non-orthogonal by %f degrees\n", non_ortho);
  mb_3D_vector beamVectGeo;

  const double y1 = sin(-rx_steer * DTR) / cos(non_ortho * DTR);
  const double y2 = sin(tx_steer * DTR) * tan(non_ortho * DTR);
  const double radial = sqrt((y1 + y2) * (y1 + y2) + sin(tx_steer * DTR) * sin(tx_steer * DTR));

  mb_3D_vector beamVectRel;
  if (radial <= 1.0) {
    beamVectRel.x = sin(tx_steer * DTR);
    beamVectRel.y = y1 + y2;
    beamVectRel.z = sqrt(1.0 - radial * radial);
  }
  else {
    beamVectRel.x = sin(tx_steer * DTR);
    beamVectRel.y = sqrt(1.0 - beamVectRel.x * beamVectRel.x);
    beamVectRel.z = 0.0;
  }

  if (verbose >= 4)
  if (radial > 1.0)
    fprintf(stderr, "dbg4     Got y1, y2, radial: %lf %lf %lf     beamVectRel:%f %f %f\n", y1, y2, radial, beamVectRel.x,
            beamVectRel.y, beamVectRel.z);

  /* Build ortho-normal basis */
  mb_3D_vector xPrime;
  xPrime = txGeo;

  /* Crossproduct of txGeo and rxGeo yields zPrime */
  /* i.e. normal of plane containing txGeo and rxGeo */
  mb_3D_vector zPrime;
  zPrime.x = txGeo.y * rxGeo.z - txGeo.z * rxGeo.y;
  zPrime.y = txGeo.z * rxGeo.x - txGeo.x * rxGeo.z;
  zPrime.z = txGeo.x * rxGeo.y - txGeo.y * rxGeo.x;

  /* Crossproduct of zPrime and xPrime yields yPrime */
  mb_3D_vector yPrime;
  yPrime.x = zPrime.y * xPrime.z - zPrime.z * xPrime.y;
  yPrime.y = zPrime.z * xPrime.x - zPrime.x * xPrime.z;
  yPrime.z = zPrime.x * xPrime.y - zPrime.y * xPrime.x;

  if (verbose >= 4) {
    fprintf(stderr, "dbg4     x': %f, %f, %f\n", xPrime.x, xPrime.y, xPrime.z);
    fprintf(stderr, "dbg4     y': %f, %f, %f\n", yPrime.x, yPrime.y, yPrime.z);
    fprintf(stderr, "dbg4     z': %f, %f, %f\n", zPrime.x, zPrime.y, zPrime.z);
  }

  /* Columns of equivalent rotation matrix are coordinates of */
  /* primed unit vectors in original coordinate system */
  /* Inverse (transpose) of this matrix will bring relative vector into */
  /* geographic coordinates */
  beamVectGeo.x = beamVectRel.x * xPrime.x + beamVectRel.y * yPrime.x + beamVectRel.z * zPrime.x;
  beamVectGeo.y = beamVectRel.x * xPrime.y + beamVectRel.y * yPrime.y + beamVectRel.z * zPrime.y;
  beamVectGeo.z = beamVectRel.x * xPrime.z + beamVectRel.y * yPrime.z + beamVectRel.z * zPrime.z;

  if (verbose >= 4) {
    fprintf(stderr, "dbg4     Beam vector is %f %f %f in transducer reference frame\n", beamVectRel.x, beamVectRel.y,
            beamVectRel.z);
    fprintf(stderr, "dbg4     Beam vector is %f %f %f in geographic reference frame\n", beamVectGeo.x, beamVectGeo.y,
            beamVectGeo.z);
  }

  *beamAzimuth = atan2(beamVectGeo.y, beamVectGeo.x) * 180.0 / M_PI;

  /* Reduce the beam azimuth relative to the ship's heading ...  */
  /* Hmmmm, this can do some funny stuff with multi-sector systems */
  /* as the ship's heading in the math below is different for each beam */
  /* but then we use the last sector as the reference azimuth -- JDB, Feb. 28, 2008 */

  // Taking care of this by now using reference_heading, which specifically
  // is taken as the heading at the ping time associated with the first sector.
  *beamAzimuth = *beamAzimuth - reference_heading;

  /* and then make sure she's positive, eh? */
  if (*beamAzimuth < 0)
    *beamAzimuth = 360.0 + *beamAzimuth;

  *beamDepression = atan(beamVectGeo.z / sqrt(beamVectGeo.x * beamVectGeo.x + beamVectGeo.y * beamVectGeo.y)) * 180.0 / M_PI;

  if (verbose >= 4)
    fprintf(stderr, "dbg4     Got beam azimuth (re: ship's heading) and depression %.2f %.2f\n", *beamAzimuth,
            *beamDepression);

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       beamAzimuth:     %f\n", *beamAzimuth);
    fprintf(stderr, "dbg2       beamDepression:  %f\n", *beamDepression);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mb_beaudoin_unrotate(int verbose, mb_3D_vector orig, mb_3D_orientation rotate, mb_3D_vector *final, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       orig.x:          %f\n", orig.x);
    fprintf(stderr, "dbg2       orig.y:          %f\n", orig.y);
    fprintf(stderr, "dbg2       orig.z:          %f\n", orig.z);
    fprintf(stderr, "dbg2       rotate.roll:     %f\n", rotate.roll);
    fprintf(stderr, "dbg2       rotate.pitch:    %f\n", rotate.pitch);
    fprintf(stderr, "dbg2       rotate.heading:  %f\n", rotate.heading);
  }

  const double sinr = sin(rotate.roll * DTR);
  const double cosr = cos(rotate.roll * DTR);
  const double sinp = sin(rotate.pitch * DTR);
  const double cosp = cos(rotate.pitch * DTR);
  const double siny = sin(rotate.heading * DTR);
  const double cosy = cos(rotate.heading * DTR);

  final->x = cosp * cosy * orig.x + (sinr * sinp * cosy - cosr * siny) * orig.y + (cosr * sinp * cosy + sinr * siny) * orig.z;

  final->y = cosp * siny * orig.x + (sinr * sinp * siny + cosr * cosy) * orig.y + (cosr * sinp * siny - sinr * cosy) * orig.z;

  final->z = -sinp * orig.x + sinr * cosp * orig.y + cosr * cosp * orig.z;

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       final->x:        %f\n", final->x);
    fprintf(stderr, "dbg2       final->y:        %f\n", final->y);
    fprintf(stderr, "dbg2       final->z:        %f\n", final->z);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
