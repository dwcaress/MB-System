/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_soundspeed.c  5 November 2026
 *
 *    Copyright (c) 2026-2026 by
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
 * Sound speed formulas and the deep part of a profile.
 *
 * Below the deepest sample of a profile only pressure changes the speed of
 * sound in a useful way: deep water keeps its potential temperature and
 * salinity closely. Full depth CTD casts from hadal trenches confirm this
 * (Kolbusz and Jamieson 2024, https://doi.org/10.1594/PANGAEA.969460). The
 * temperature that gives the last sample at salinity 35 is therefore warmed by
 * the adiabatic lapse rate on the way down, and the speed is computed with the
 * same formula as the profile.
 *
 * Formulas: Chen and Millero 1977, Leroy et al. 2008 (depth and latitude form),
 * Leroy and Parthiot 1998 (depth to pressure), Fofonoff and Millard 1983
 * (adiabatic lapse rate).
 */

#include <math.h>
#include <stdio.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

#define MBSVP_ASSUMED_SALINITY 35.0
#define MBSVP_LAPSE_STEP 50.0

/*--------------------------------------------------------------------*/
/* Pressure in bar at a depth in metres. */
double mbsvp_depth_to_pressure(double depth, double latitude) {
  const double z2 = depth * depth;
  const double h = 1.00818e-2 * depth + 2.465e-8 * z2 - 1.25e-13 * z2 * depth + 2.8e-19 * z2 * z2;
  const double sinlat = sin(DTR * latitude);
  const double g = 9.7803 * (1.0 + 5.3e-3 * sinlat * sinlat);
  const double k = (g - 2.0e-5 * depth) / (9.80612 - 2.0e-5 * depth);
  return 10.0 * h * k;
}
/*--------------------------------------------------------------------*/
double mbsvp_speed_chen(double t, double s, double p) {
  const double t2 = t * t;
  const double t3 = t2 * t;
  const double t4 = t3 * t;
  const double t5 = t4 * t;
  const double p2 = p * p;
  const double p3 = p2 * p;
  const double cw = 1402.388 + 5.03830 * t - 5.81090e-2 * t2 + 3.3432e-4 * t3 - 1.47797e-6 * t4 + 3.1419e-9 * t5 +
                    (0.153563 + 6.8999e-4 * t - 8.1829e-6 * t2 + 1.3632e-7 * t3 - 6.1260e-10 * t4) * p +
                    (3.1260e-5 - 1.7111e-6 * t + 2.5986e-8 * t2 - 2.5353e-10 * t3 + 1.0415e-12 * t4) * p2 +
                    (-9.7729e-9 + 3.8513e-10 * t - 2.3654e-12 * t2) * p3;
  const double a = 1.389 - 1.262e-2 * t + 7.166e-5 * t2 + 2.008e-6 * t3 - 3.21e-8 * t4 +
                   (9.4742e-5 - 1.2583e-5 * t - 6.4928e-8 * t2 + 1.0515e-8 * t3 - 2.0142e-10 * t4) * p +
                   (-3.9064e-7 + 9.1061e-9 * t - 1.6009e-10 * t2 + 7.994e-12 * t3) * p2 +
                   (1.100e-10 + 6.651e-12 * t - 3.391e-13 * t2) * p3;
  const double b = -1.922e-2 - 4.42e-5 * t + (7.3637e-5 + 1.7950e-7 * t) * p;
  const double d = 1.727e-3 - 7.9836e-6 * p;
  return cw + a * s + b * pow(s, 1.5) + d * s * s;
}
/*--------------------------------------------------------------------*/
double mbsvp_speed_leroy(double t, double s, double z, double latitude) {
  const double t2 = t * t;
  const double z2 = z * z;
  const double z3 = z2 * z;
  return 1402.5 + 5.0 * t - 5.44e-2 * t2 + 2.1e-4 * t2 * t + 1.33 * s - 1.23e-2 * s * t + 8.7e-5 * s * t2 + 1.56e-2 * z +
         2.55e-7 * z2 - 7.3e-12 * z3 + 1.2e-6 * z * (latitude - 45.0) - 9.5e-13 * t * z3 + 3.0e-7 * t2 * z + 1.43e-5 * s * z;
}
/*--------------------------------------------------------------------*/
/* Adiabatic lapse rate in degrees C per dbar, with pressure in dbar. */
double mbsvp_adiabatic_lapse(double s, double t, double p) {
  const double ds = s - 35.0;
  return (((-2.1687e-16 * t + 1.8676e-14) * t - 4.6206e-13) * p +
          ((2.7759e-12 * t - 1.1351e-10) * ds + ((-5.4481e-14 * t + 8.733e-12) * t - 6.7795e-10) * t + 1.8741e-8)) *
             p +
         (-4.2393e-8 * t + 1.8932e-6) * ds + ((6.6228e-10 * t - 6.836e-8) * t + 8.5258e-6) * t + 3.5803e-5;
}
/*--------------------------------------------------------------------*/
static double mbsvp_speed(enum mbsvp_formula formula, double t, double s, double z, double latitude) {
  if (formula == MBSVP_LEROY)
    return mbsvp_speed_leroy(t, s, z, latitude);
  return mbsvp_speed_chen(t, s, mbsvp_depth_to_pressure(z, latitude));
}
/*--------------------------------------------------------------------*/
/* Speeds at the increasing depths below a sample, with potential temperature
   and salinity held at their values in that sample. */
int mbsvp_physics_tail(int verbose, enum mbsvp_formula formula, double z0, double c0, double latitude, int n, const double *depth,
                       double *speed, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       formula:    %d\n", formula);
    fprintf(stderr, "dbg2       z0 c0:      %f %f\n", z0, c0);
    fprintf(stderr, "dbg2       latitude:   %f\n", latitude);
    fprintf(stderr, "dbg2       n:          %d\n", n);
  }

  const double s = MBSVP_ASSUMED_SALINITY;
  double t = 3.0;
  for (int i = 0; i < 30; i++) {
    const double e = mbsvp_speed(formula, t, s, z0, latitude) - c0;
    const double slope = (mbsvp_speed(formula, t + 0.01, s, z0, latitude) - mbsvp_speed(formula, t, s, z0, latitude)) / 0.01;
    t -= e / slope;
  }
  if (!isfinite(t) || fabs(mbsvp_speed(formula, t, s, z0, latitude) - c0) > 1.0e-3) {
    mbsvp_set_message(message, "Cannot find a temperature for speed %.2f m/s at %.0f m", c0, z0);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }

  double zc = z0;
  for (int k = 0; k < n; k++) {
    const double span = depth[k] - zc;
    const int steps = MAX(1, (int)(span / MBSVP_LAPSE_STEP));
    for (int i = 0; i < steps; i++) {
      const double za = zc + span * i / steps;
      const double zb = zc + span * (i + 1) / steps;
      const double pmid = 10.0 * mbsvp_depth_to_pressure(0.5 * (za + zb), latitude);
      const double dp = 10.0 * (mbsvp_depth_to_pressure(zb, latitude) - mbsvp_depth_to_pressure(za, latitude));
      t += mbsvp_adiabatic_lapse(s, t, pmid) * dp;
    }
    zc = depth[k];
    speed[k] = mbsvp_speed(formula, t, s, zc, latitude);
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
