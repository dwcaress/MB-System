/*--------------------------------------------------------------------
 *    The MB-system:  mb_otps_predict.c  9/2/2026
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
 * See mb_otps_predict.h for an overview and for attribution of the
 * numerical methods implemented here to Oregon State University's OSU
 * Tidal Prediction Software (OTPS), authored by Gary Egbert and Lana
 * Erofeeva (https://www.tpxo.net/), with equilibrium-argument, nodal-
 * correction, and minor-constituent-inference formulas supplied by
 * Richard Ray (NASA/GSFC). This file is a from-scratch C transcription
 * of the numerical methods used by OTPS's predict_tide (predict_tide.f90
 * / subs.f90), restricted to the single code path that MB-System's
 * mbotps actually uses: ocean (not geocentric) elevation prediction from
 * a global "atlas" format model, using all constituents present in the
 * model and inferring minor constituents. Current/transport prediction,
 * regional (non-atlas) models, load-tide correction, and the
 * uniform-grid-in-km map projections used only by regional models are
 * intentionally not implemented, since mbotps never exercises them.
 *
 * OTPS binary model file format (see the OTPS distribution's README):
 * each atlas file is a sequence of Fortran unformatted sequential
 * records, written big-endian with 4-byte record-length markers before
 * and after each record. The single header record holds, as 4-byte
 * fields: grid size n (longitude) and m (latitude), constituent count
 * (always 1 for atlas files), latitude limits (2 floats), longitude
 * limits (2 floats), and the constituent name (nc*4 characters). The
 * data record that follows holds n*m complex (real,imag) 4-byte-float
 * pairs in row-major (longitude-fastest) order; a (0.0,0.0) value marks
 * a land grid node. This module reads that data record directly via
 * computed byte offsets (a C translation of OTPS's own direct-access
 * "rd_mod_value_da" record arithmetic), rather than loading whole
 * (400MB-1.4GB) files into memory.
 *
 * Author:  D. W. Caress
 * Date:    September 2, 2026
 */

#include <dirent.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#include "mb_otps_predict.h"

/*--------------------------------------------------------------------*/
/* Tidal constituent constants, transcribed from OTPS's constit.h.
   Only the fields predict_tide's ocean-elevation path actually uses
   (equilibrium-argument frequency and reference phase) are carried;
   constit.h's alpha/amplitude/solid-earth-beta tables are not used by
   that code path and are omitted (see mb_otps_make_a() below for why
   the solid-earth beta correction in particular does not apply here). */

#define MBOTPS_ARG_MAX 53

static const char *const mbotps_constit_name[MBOTPS_CONSTITUENT_MAX] = {
    "m2",  "s2",  "k1",  "o1",  "n2",  "p1",   "k2",   "q1",
    "2n2", "mu2", "nu2", "l2",  "t2",  "j1",   "m1",   "oo1",
    "rho1","mf",  "mm",  "ssa", "m4",  "ms4",  "mn4",  "m6",
    "m8",  "mk3", "s6",  "msf", "2mk3","s1",   "2q1",  "m3"};

/* 1-based index into the 53-entry Richard Ray "arguments"/"astrol"
   equilibrium-argument table (arg/f/u below); 0 means this constituent
   has no corresponding formula (OTPS's constit.h "index" array). */
static const int mbotps_constit_argindex[MBOTPS_CONSTITUENT_MAX] = {
    30, 35, 19, 12, 27, 17, 37, 10, 25, 26, 28, 33, 34, 23, 14, 24,
    11, 5,  3,  2,  45, 46, 44, 50, 0,  42, 51, 40, 0,  18, 8,  41};

/* equilibrium tidal argument angular frequency, radians/second.
   NOTE: OTPS's constit.h declares omega_d as single-precision "real",
   not real*8; since this value is multiplied by a time span of order
   10^9 seconds (decades, in seconds, from the phase epoch below) to
   form a phase angle, its storage precision measurably affects the
   result and must be reproduced here as "float" rather than "double"
   to match OTPS's own output. */
static const float mbotps_constit_omega[MBOTPS_CONSTITUENT_MAX] = {
    1.405189e-04, 1.454441e-04, 7.292117e-05, 6.759774e-05,
    1.378797e-04, 7.252295e-05, 1.458423e-04, 6.495854e-05,
    1.352405e-04, 1.355937e-04, 1.382329e-04, 1.431581e-04,
    1.452450e-04, 7.556036e-05, 7.028195e-05, 7.824458e-05,
    6.531174e-05, 0.053234e-04, 0.026392e-04, 0.003982e-04,
    2.810377e-04, 2.859630e-04, 2.783984e-04, 4.215566e-04,
    5.620755e-04, 2.134402e-04, 4.363323e-04, 4.925200e-06,
    2.081166e-04, 7.2722e-05,   0.6231934e-04, 2.107783523e-04};

/* equilibrium tidal argument reference phase at 1992/1/1 00:00 UTC,
   radians (OTPS constit.h "phase_mkB"). Single precision "float" for
   the same reason as mbotps_constit_omega above (OTPS declares
   phase_mkB as single-precision "real"). */
static const float mbotps_constit_phase[MBOTPS_CONSTITUENT_MAX] = {
    1.731557546, 0.000000000, 0.173003674, 1.558553872,
    6.050721243, 6.110181633, 3.487600001, 5.877717569,
    4.086699633, 3.463115091, 5.427136701, 0.553986502,
    0.052841931, 2.137025284, 2.436575100, 1.929046130,
    5.254133027, 1.756042456, 1.964021610, 3.487600001,
    3.463115091, 1.731557546, 1.499093481, 5.194672637,
    6.926230184, 1.904561220, 0.000000000, 4.551613000,
    3.809122439, 0.000000000, 3.913707000, 5.738991000};

/* time origin used with mbotps_constit_omega/phase above: seconds since
   1992/1/1 00:00 UTC, expressed as a Modified Julian Day */
#define MBOTPS_PHASE_EPOCH_MJD 48622.0

/* the 8 "major" constituents from which OTPS's minor-constituent
   inference (perth2-style) is computed, and the 18 minor constituents
   it produces; see mbotps_infer_minor() below. Order matches OTPS's
   subs.f90 infer_minor exactly. */
#define MBOTPS_MINOR_ANCHOR_N 8
#define MBOTPS_MINOR_N 18
static const char *const mbotps_minor_anchor_name[MBOTPS_MINOR_ANCHOR_N] = {
    "q1", "o1", "p1", "k1", "n2", "m2", "s2", "k2"};
static const char *const mbotps_minor_name[MBOTPS_MINOR_N] = {
    "2q1", "sig1", "rho1", "m1",  "m1",  "chi1", "pi1", "phi1", "the1",
    "j1",  "oo1",  "2n2",  "mu2", "nu2", "lam2", "l2",  "l2",   "t2"};

/*--------------------------------------------------------------------*/
/* Read a big-endian 4-byte int/float at an absolute byte offset. OTPS
   atlas files are always big-endian regardless of host byte order, so
   mb_get_binary_int/float() are always called with swapped=false: that
   call swaps on a little-endian host and leaves the value alone on a
   big-endian host, which is exactly the "always treat the source bytes
   as big-endian" behavior wanted here (see mb_get_value.c). */
static int mbotps_read_be_int(FILE *fp, long long offset, int *value) {
  unsigned char buf[4];
  if (fseeko(fp, (off_t)offset, SEEK_SET) != 0)
    return MB_FAILURE;
  if (fread(buf, 1, 4, fp) != 4)
    return MB_FAILURE;
  mb_get_binary_int(false, buf, value);
  return MB_SUCCESS;
}

static int mbotps_read_be_float(FILE *fp, long long offset, float *value) {
  unsigned char buf[4];
  if (fseeko(fp, (off_t)offset, SEEK_SET) != 0)
    return MB_FAILURE;
  if (fread(buf, 1, 4, fp) != 4)
    return MB_FAILURE;
  mb_get_binary_float(false, buf, value);
  return MB_SUCCESS;
}

/* Byte offset (0-based) of the real part of the complex grid value at
   1-based grid indices (i0,j0) in a single-constituent ("ic=1", so no
   skip for other constituents) atlas file whose own header records
   ncmod constituents (always 1 in practice) and n longitude points per
   row. The imaginary part immediately follows at offset+4. This is a
   direct translation of OTPS's rd_mod_value_da() direct-access record
   arithmetic (subs.f90), converting its 1-based, 4-byte "record number"
   (irec = 10+ncmod+((j0-1)*n+i0)*2-1) into a 0-based byte offset. */
static long long mbotps_value_offset(int ncmod, int n, int i0, int j0) {
  return 4LL * (8 + ncmod) + 8LL * ((long long)(j0 - 1) * n + i0);
}

static int mbotps_read_value(FILE *fp, int ncmod, int n, int i0, int j0,
                              double *re, double *im) {
  const long long offset = mbotps_value_offset(ncmod, n, i0, j0);
  float fre = 0.0f, fim = 0.0f;
  if (mbotps_read_be_float(fp, offset, &fre) != MB_SUCCESS)
    return MB_FAILURE;
  if (mbotps_read_be_float(fp, offset + 4, &fim) != MB_SUCCESS)
    return MB_FAILURE;
  *re = (double)fre;
  *im = (double)fim;
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* ASTROL: basic astronomical mean longitudes s (moon), h (sun), p (lunar
   perigee), omega (ascending lunar node), in degrees, valid for roughly
   1990-2010 (formulas by David Cartwright, as supplied to OTPS by
   Richard Ray). time is UTC expressed as a Modified Julian Day. */
static void mbotps_astrol(double time, double shpn[5]) {
  const double circle = 360.0;
  const double t = time - 51544.4993;

  shpn[1] = 218.3164 + 13.17639648 * t; /* s: mean longitude of moon */
  shpn[2] = 280.4661 + 0.98564736 * t;  /* h: mean longitude of sun */
  shpn[3] = 83.3535 + 0.11140353 * t;   /* p: mean longitude of lunar perigee */
  shpn[4] = 125.0445 - 0.05295377 * t;  /* omega (N): ascending lunar node */

  for (int i = 1; i <= 4; i++) {
    shpn[i] = fmod(shpn[i], circle);
    if (shpn[i] < 0.0)
      shpn[i] += circle;
  }
}

/* ARGUMENTS: equilibrium tidal arguments arg(1:53) (degrees) and nodal
   correction factors f(1:53) (amplitude) and u(1:53) (degrees), for
   Richard Ray's 53-term Doodson-numbered constituent table. time1 is
   UTC expressed as a Modified Julian Day. Transcribed term-for-term
   from OTPS's subs.f90 "arguments" subroutine. */
static void mbotps_arguments(double time1, double arg[MBOTPS_ARG_MAX + 1],
                              double f[MBOTPS_ARG_MAX + 1],
                              double u[MBOTPS_ARG_MAX + 1]) {
  const double pi = 3.141592654;
  const double rad = pi / 180.0;
  const double pp = 282.94; /* solar perigee at epoch 2000 */

  double shpn[5];
  mbotps_astrol(time1, shpn);
  const double s = shpn[1], h = shpn[2], p = shpn[3], omega = shpn[4];

  const double hour = (time1 - floor(time1)) * 24.0;
  const double t1 = 15.0 * hour;
  const double t2 = 30.0 * hour;

  arg[1] = h - pp;
  arg[2] = 2.0 * h;
  arg[3] = s - p;
  arg[4] = 2.0 * s - 2.0 * h;
  arg[5] = 2.0 * s;
  arg[6] = 3.0 * s - p;
  arg[7] = t1 - 5.0 * s + 3.0 * h + p - 90.0;
  arg[8] = t1 - 4.0 * s + h + 2.0 * p - 90.0;
  arg[9] = t1 - 4.0 * s + 3.0 * h - 90.0;
  arg[10] = t1 - 3.0 * s + h + p - 90.0;
  arg[11] = t1 - 3.0 * s + 3.0 * h - p - 90.0;
  arg[12] = t1 - 2.0 * s + h - 90.0;
  arg[13] = t1 - 2.0 * s + 3.0 * h + 90.0;
  arg[14] = t1 - s + h + 90.0;
  arg[15] = t1 - s + 3.0 * h - p + 90.0;
  arg[16] = t1 - 2.0 * h + pp - 90.0;
  arg[17] = t1 - h - 90.0;
  arg[18] = t1 + 90.0;
  arg[19] = t1 + h + 90.0;
  arg[20] = t1 + 2.0 * h - pp + 90.0;
  arg[21] = t1 + 3.0 * h + 90.0;
  arg[22] = t1 + s - h + p + 90.0;
  arg[23] = t1 + s + h - p + 90.0;
  arg[24] = t1 + 2.0 * s + h + 90.0;
  arg[25] = t2 - 4.0 * s + 2.0 * h + 2.0 * p;
  arg[26] = t2 - 4.0 * s + 4.0 * h;
  arg[27] = t2 - 3.0 * s + 2.0 * h + p;
  arg[28] = t2 - 3.0 * s + 4.0 * h - p;
  arg[29] = t2 - 2.0 * s + h + pp;
  arg[30] = t2 - 2.0 * s + 2.0 * h;
  arg[31] = t2 - 2.0 * s + 3.0 * h - pp;
  arg[32] = t2 - s + p + 180.0;
  arg[33] = t2 - s + 2.0 * h - p + 180.0;
  arg[34] = t2 - h + pp;
  arg[35] = t2;
  arg[36] = t2 + h - pp + 180.0;
  arg[37] = t2 + 2.0 * h;
  arg[38] = t2 + s + 2.0 * h - pp;
  arg[39] = t2 - 5.0 * s + 4.0 * h + p;
  arg[40] = t2 + 2.0 * s - 2.0 * h;
  arg[41] = 1.5 * arg[30];
  arg[42] = arg[19] + arg[30];
  arg[43] = 3.0 * t1;
  arg[44] = arg[27] + arg[30];
  arg[45] = 2.0 * arg[30];
  arg[46] = arg[30] + arg[35];
  arg[47] = arg[30] + arg[37];
  arg[48] = 4.0 * t1;
  arg[49] = 5.0 * t1;
  arg[50] = 3.0 * arg[30];
  arg[51] = 3.0 * t2;
  arg[52] = 7.0 * t1;
  arg[53] = 4.0 * t2;

  const double sinn = sin(omega * rad);
  const double cosn = cos(omega * rad);
  const double sin2n = sin(2.0 * omega * rad);
  const double cos2n = cos(2.0 * omega * rad);
  const double sin3n = sin(3.0 * omega * rad);

  f[1] = 1.0;
  f[2] = 1.0;
  f[3] = 1.0 - 0.130 * cosn;
  f[4] = 1.0;
  f[5] = 1.043 + 0.414 * cosn;
  f[6] = sqrt(pow(1.0 + 0.203 * cosn + 0.040 * cos2n, 2) +
              pow(0.203 * sinn + 0.040 * sin2n, 2));
  f[7] = 1.0;
  f[8] = sqrt(pow(1.0 + 0.188 * cosn, 2) + pow(0.188 * sinn, 2));
  f[9] = f[8];
  f[10] = f[8];
  f[11] = f[8];
  f[12] = sqrt(pow(1.0 + 0.189 * cosn - 0.0058 * cos2n, 2) +
               pow(0.189 * sinn - 0.0058 * sin2n, 2));
  f[13] = 1.0;
  const double tmp1 = 1.36 * cos(p * rad) + 0.267 * cos((p - omega) * rad);
  const double tmp2 = 0.64 * sin(p * rad) + 0.135 * sin((p - omega) * rad);
  f[14] = sqrt(tmp1 * tmp1 + tmp2 * tmp2);
  f[15] = sqrt(pow(1.0 + 0.221 * cosn, 2) + pow(0.221 * sinn, 2));
  f[16] = 1.0;
  f[17] = 1.0;
  f[18] = 1.0;
  f[19] = sqrt(pow(1.0 + 0.1158 * cosn - 0.0029 * cos2n, 2) +
               pow(0.1554 * sinn - 0.0029 * sin2n, 2));
  f[20] = 1.0;
  f[21] = 1.0;
  f[22] = 1.0;
  f[23] = sqrt(pow(1.0 + 0.169 * cosn, 2) + pow(0.227 * sinn, 2));
  f[24] = sqrt(pow(1.0 + 0.640 * cosn + 0.134 * cos2n, 2) +
               pow(0.640 * sinn + 0.134 * sin2n, 2));
  f[25] = sqrt(pow(1.0 - 0.03731 * cosn + 0.00052 * cos2n, 2) +
               pow(0.03731 * sinn - 0.00052 * sin2n, 2));
  f[26] = f[25];
  f[27] = f[25];
  f[28] = f[25];
  f[29] = 1.0;
  f[30] = f[25];
  f[31] = 1.0;
  f[32] = 1.0;
  const double temp1 = 1.0 - 0.25 * cos(2.0 * p * rad) -
                        0.11 * cos((2.0 * p - omega) * rad) - 0.04 * cosn;
  const double temp2 = 0.25 * sin(2.0 * p * rad) +
                        0.11 * sin((2.0 * p - omega) * rad) + 0.04 * sinn;
  f[33] = sqrt(temp1 * temp1 + temp2 * temp2);
  f[34] = 1.0;
  f[35] = 1.0;
  f[36] = 1.0;
  f[37] = sqrt(pow(1.0 + 0.2852 * cosn + 0.0324 * cos2n, 2) +
               pow(0.3108 * sinn + 0.0324 * sin2n, 2));
  f[38] = sqrt(pow(1.0 + 0.436 * cosn, 2) + pow(0.436 * sinn, 2));
  f[39] = f[30] * f[30];
  f[40] = f[30];
  f[41] = 1.0;
  f[42] = f[19] * f[30];
  f[43] = 1.0;
  f[44] = f[30] * f[30];
  f[45] = f[44];
  f[46] = f[44];
  f[47] = f[30] * f[37];
  f[48] = 1.0;
  f[49] = 1.0;
  f[50] = f[30] * f[30] * f[30];
  f[51] = 1.0;
  f[52] = 1.0;
  f[53] = 1.0;

  u[1] = 0.0;
  u[2] = 0.0;
  u[3] = 0.0;
  u[4] = 0.0;
  u[5] = -23.7 * sinn + 2.7 * sin2n - 0.4 * sin3n;
  u[6] = atan2(-(0.203 * sinn + 0.040 * sin2n),
               1.0 + 0.203 * cosn + 0.040 * cos2n) / rad;
  u[7] = 0.0;
  u[8] = atan2(0.189 * sinn, 1.0 + 0.189 * cosn) / rad;
  u[9] = u[8];
  u[10] = u[8];
  u[11] = u[8];
  u[12] = 10.8 * sinn - 1.3 * sin2n + 0.2 * sin3n;
  u[13] = 0.0;
  u[14] = atan2(tmp2, tmp1) / rad;
  u[15] = atan2(-0.221 * sinn, 1.0 + 0.221 * cosn) / rad;
  u[16] = 0.0;
  u[17] = 0.0;
  u[18] = 0.0;
  u[19] = atan2(-0.1554 * sinn + 0.0029 * sin2n,
                1.0 + 0.1158 * cosn - 0.0029 * cos2n) / rad;
  u[20] = 0.0;
  u[21] = 0.0;
  u[22] = 0.0;
  u[23] = atan2(-0.227 * sinn, 1.0 + 0.169 * cosn) / rad;
  u[24] = atan2(-(0.640 * sinn + 0.134 * sin2n),
                1.0 + 0.640 * cosn + 0.134 * cos2n) / rad;
  u[25] = atan2(-0.03731 * sinn + 0.00052 * sin2n,
                1.0 - 0.03731 * cosn + 0.00052 * cos2n) / rad;
  u[26] = u[25];
  u[27] = u[25];
  u[28] = u[25];
  u[29] = 0.0;
  u[30] = u[25];
  u[31] = 0.0;
  u[32] = 0.0;
  u[33] = atan2(-temp2, temp1) / rad;
  u[34] = 0.0;
  u[35] = 0.0;
  u[36] = 0.0;
  u[37] = atan2(-(0.3108 * sinn + 0.0324 * sin2n),
                1.0 + 0.2852 * cosn + 0.0324 * cos2n) / rad;
  u[38] = atan2(-0.436 * sinn, 1.0 + 0.436 * cosn) / rad;
  u[39] = u[30] * 2.0;
  u[40] = u[30];
  u[41] = 1.5 * u[30];
  u[42] = u[30] + u[19];
  u[43] = 0.0;
  u[44] = u[30] * 2.0;
  u[45] = u[44];
  u[46] = u[30];
  u[47] = u[30] + u[37];
  u[48] = 0.0;
  u[49] = 0.0;
  u[50] = u[30] * 3.0;
  u[51] = 0.0;
  u[52] = 0.0;
  u[53] = 0.0;
}

/* NODAL: nodal amplitude correction factor pf[] and phase correction
   pu[] (radians), indexed 0..31 to match mbotps_constit_* above (i.e.
   already remapped from Richard Ray's 53-term table via
   mbotps_constit_argindex, matching the I/O contract of OTPS's own
   "nodal" subroutine, whose pu/pf outputs are likewise indexed by
   constit.h position rather than by Richard's numbering). dtime is UTC
   as a Modified Julian Day. */
static void mbotps_nodal(double dtime, double pu[MBOTPS_CONSTITUENT_MAX],
                          double pf[MBOTPS_CONSTITUENT_MAX]) {
  const double pi = 3.14159265358979;
  double arg[MBOTPS_ARG_MAX + 1], f[MBOTPS_ARG_MAX + 1], u[MBOTPS_ARG_MAX + 1];
  mbotps_arguments(dtime, arg, f, u);

  for (int i = 0; i < MBOTPS_CONSTITUENT_MAX; i++) {
    const int idx = mbotps_constit_argindex[i];
    if (idx > 0) {
      pu[i] = u[idx] * pi / 180.0;
      pf[i] = f[idx];
    } else {
      pu[i] = 0.0;
      pf[i] = 1.0;
    }
  }
}

/*--------------------------------------------------------------------*/
/* Infer 16 minor tidal constituents (perth2-style linear admittance,
   after Richard Ray) from the complex harmonic constants already
   interpolated for the 8 major "anchor" constituents in zmaj[]/names[],
   returning their combined correction *dh (meters) at the given time.
   Faithful to OTPS's subs.f90 infer_minor, including: (a) skipping any
   minor constituent already explicitly present in the model's own
   constituent list, to avoid double-counting, and (b) refusing to infer
   (returning MB_FAILURE, *dh unchanged) if fewer than 6 of the anchor
   constituents (excluding p1 and k2, per the original) are present. */
static int mbotps_infer_minor(int ncon, const char names[][8],
                               const double zmaj_re[], const double zmaj_im[],
                               double time_mjd, double *dh) {
  double z8_re[MBOTPS_MINOR_ANCHOR_N] = {0}, z8_im[MBOTPS_MINOR_ANCHOR_N] = {0};
  int ni = 0;
  for (int i = 0; i < MBOTPS_MINOR_ANCHOR_N; i++) {
    for (int j = 0; j < ncon; j++) {
      if (strcmp(names[j], mbotps_minor_anchor_name[i]) == 0) {
        z8_re[i] = zmaj_re[j];
        z8_im[i] = zmaj_im[j];
        if (i != 2 && i != 7) /* exclude p1 (index 2) and k2 (index 7) from the count */
          ni++;
      }
    }
  }

  double msk[MBOTPS_MINOR_N];
  for (int i = 0; i < MBOTPS_MINOR_N; i++) {
    msk[i] = 1.0;
    for (int j = 0; j < ncon; j++) {
      if (strcmp(names[j], mbotps_minor_name[i]) == 0)
        msk[i] = 0.0;
    }
  }

  if (ni < 6)
    return MB_FAILURE;

  double zmin_re[MBOTPS_MINOR_N], zmin_im[MBOTPS_MINOR_N];
#define Z8RE(k) z8_re[(k) - 1]
#define Z8IM(k) z8_im[(k) - 1]
#define ZMINSET(k, c1, k1, c2, k2)                                           \
  zmin_re[(k) - 1] = (c1) * Z8RE(k1) + (c2) * Z8RE(k2);                      \
  zmin_im[(k) - 1] = (c1) * Z8IM(k1) + (c2) * Z8IM(k2)

  ZMINSET(1, 0.263, 1, -0.0252, 2);
  ZMINSET(2, 0.297, 1, -0.0264, 2);
  ZMINSET(3, 0.164, 1, 0.0048, 2);
  ZMINSET(4, 0.0140, 2, 0.0101, 4);
  ZMINSET(5, 0.0389, 2, 0.0282, 4);
  ZMINSET(6, 0.0064, 2, 0.0060, 4);
  ZMINSET(7, 0.0030, 2, 0.0171, 4);
  ZMINSET(8, -0.0015, 2, 0.0152, 4);
  ZMINSET(9, -0.0065, 2, 0.0155, 4);
  ZMINSET(10, -0.0389, 2, 0.0836, 4);
  ZMINSET(11, -0.0431, 2, 0.0613, 4);
  ZMINSET(12, 0.264, 5, -0.0253, 6);
  ZMINSET(13, 0.298, 5, -0.0264, 6);
  ZMINSET(14, 0.165, 5, 0.00487, 6);
  ZMINSET(15, 0.0040, 6, 0.0074, 7);
  ZMINSET(16, 0.0131, 6, 0.0326, 7);
  ZMINSET(17, 0.0033, 6, 0.0082, 7);
  zmin_re[17] = 0.0585 * Z8RE(7);
  zmin_im[17] = 0.0585 * Z8IM(7);
#undef ZMINSET
#undef Z8RE
#undef Z8IM

  const double rad = 3.141592654 / 180.0;
  const double pp = 282.8;
  double shpn[5];
  const double hour = (time_mjd - floor(time_mjd)) * 24.0;
  const double t1 = 15.0 * hour;
  const double t2 = 30.0 * hour;
  mbotps_astrol(time_mjd, shpn);
  const double s = shpn[1], h = shpn[2], p = shpn[3], omega = shpn[4];

  double arg[MBOTPS_MINOR_N];
  arg[0] = t1 - 4.0 * s + h + 2.0 * p - 90.0;
  arg[1] = t1 - 4.0 * s + 3.0 * h - 90.0;
  arg[2] = t1 - 3.0 * s + 3.0 * h - p - 90.0;
  arg[3] = t1 - s + h - p + 90.0;
  arg[4] = t1 - s + h + p + 90.0;
  arg[5] = t1 - s + 3.0 * h - p + 90.0;
  arg[6] = t1 - 2.0 * h + pp - 90.0;
  arg[7] = t1 + 3.0 * h + 90.0;
  arg[8] = t1 + s - h + p + 90.0;
  arg[9] = t1 + s + h - p + 90.0;
  arg[10] = t1 + 2.0 * s + h + 90.0;
  arg[11] = t2 - 4.0 * s + 2.0 * h + 2.0 * p;
  arg[12] = t2 - 4.0 * s + 4.0 * h;
  arg[13] = t2 - 3.0 * s + 4.0 * h - p;
  arg[14] = t2 - s + p + 180.0;
  arg[15] = t2 - s + 2.0 * h - p + 180.0;
  arg[16] = t2 - s + 2.0 * h + p;
  arg[17] = t2 - h + pp;

  const double sinn = sin(omega * rad);
  const double cosn = cos(omega * rad);
  const double sin2n = sin(2.0 * omega * rad);
  const double cos2n = cos(2.0 * omega * rad);

  double f[MBOTPS_MINOR_N];
  for (int i = 0; i < MBOTPS_MINOR_N; i++)
    f[i] = 1.0;
  f[0] = sqrt(pow(1.0 + 0.189 * cosn - 0.0058 * cos2n, 2) +
              pow(0.189 * sinn - 0.0058 * sin2n, 2));
  f[1] = f[0];
  f[2] = f[0];
  f[3] = sqrt(pow(1.0 + 0.185 * cosn, 2) + pow(0.185 * sinn, 2));
  f[4] = sqrt(pow(1.0 + 0.201 * cosn, 2) + pow(0.201 * sinn, 2));
  f[5] = sqrt(pow(1.0 + 0.221 * cosn, 2) + pow(0.221 * sinn, 2));
  f[9] = sqrt(pow(1.0 + 0.198 * cosn, 2) + pow(0.198 * sinn, 2));
  f[10] = sqrt(pow(1.0 + 0.640 * cosn + 0.134 * cos2n, 2) +
               pow(0.640 * sinn + 0.134 * sin2n, 2));
  f[11] = sqrt(pow(1.0 - 0.0373 * cosn, 2) + pow(0.0373 * sinn, 2));
  f[12] = f[11];
  f[13] = f[11];
  f[15] = f[11];
  f[16] = sqrt(pow(1.0 + 0.441 * cosn, 2) + pow(0.441 * sinn, 2));

  double u[MBOTPS_MINOR_N];
  for (int i = 0; i < MBOTPS_MINOR_N; i++)
    u[i] = 0.0;
  u[0] = atan2(0.189 * sinn - 0.0058 * sin2n,
               1.0 + 0.189 * cosn - 0.0058 * sin2n) / rad;
  u[1] = u[0];
  u[2] = u[0];
  u[3] = atan2(0.185 * sinn, 1.0 + 0.185 * cosn) / rad;
  u[4] = atan2(-0.201 * sinn, 1.0 + 0.201 * cosn) / rad;
  u[5] = atan2(-0.221 * sinn, 1.0 + 0.221 * cosn) / rad;
  u[9] = atan2(-0.198 * sinn, 1.0 + 0.198 * cosn) / rad;
  u[10] = atan2(-0.640 * sinn - 0.134 * sin2n,
                1.0 + 0.640 * cosn + 0.134 * cos2n) / rad;
  u[11] = atan2(-0.0373 * sinn, 1.0 - 0.0373 * cosn) / rad;
  u[12] = u[11];
  u[13] = u[11];
  u[15] = u[11];
  u[16] = atan2(-0.441 * sinn, 1.0 + 0.441 * cosn) / rad;

  double sum = 0.0;
  for (int i = 0; i < MBOTPS_MINOR_N; i++) {
    const double ang = (arg[i] + u[i]) * rad;
    sum += msk[i] * (zmin_re[i] * f[i] * cos(ang) - zmin_im[i] * f[i] * sin(ang));
  }
  *dh = sum;
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* strip a trailing '!' comment (matching OTPS's rmCom) and trailing
   whitespace/newline from a control-file line, in place */
static void mbotps_trim_line(char *line) {
  char *bang = strchr(line, '!');
  if (bang != NULL)
    *bang = '\0';
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                      line[len - 1] == ' ' || line[len - 1] == '\t')) {
    line[--len] = '\0';
  }
}

/* true if constituent table index constit_i is already recorded in model */
static bool mbotps_have_constituent(const struct mbotps_model *model, int constit_i) {
  for (int j = 0; j < model->ncon; j++) {
    if (model->con[j].conidx == constit_i)
      return true;
  }
  return false;
}

/* Open fullpath as constituent constit_i's elevation ("h_") atlas file,
   reading and validating its header; on success this is recorded as one
   more entry in model (capturing the model's overall grid geometry from
   whichever constituent is recorded first - any one will do, since every
   constituent in a real atlas shares the identical land/sea grid). Guards
   against exceeding the fixed-size constituent table and against the
   same constituent being matched twice (e.g. a stray duplicate file). */
static void mbotps_add_constituent(struct mbotps_model *model, int constit_i,
                                    const char *fullpath) {
  if (model->ncon >= MBOTPS_CONSTITUENT_MAX || mbotps_have_constituent(model, constit_i))
    return;

  FILE *fp = fopen(fullpath, "rb");
  if (fp == NULL)
    return;

  int n = 0, m = 0, nc = 0;
  float lat0 = 0.0f, lat1 = 0.0f, lon0 = 0.0f, lon1 = 0.0f;
  if (mbotps_read_be_int(fp, 4, &n) != MB_SUCCESS ||
      mbotps_read_be_int(fp, 8, &m) != MB_SUCCESS ||
      mbotps_read_be_int(fp, 12, &nc) != MB_SUCCESS ||
      mbotps_read_be_float(fp, 16, &lat0) != MB_SUCCESS ||
      mbotps_read_be_float(fp, 20, &lat1) != MB_SUCCESS ||
      mbotps_read_be_float(fp, 24, &lon0) != MB_SUCCESS ||
      mbotps_read_be_float(fp, 28, &lon1) != MB_SUCCESS) {
    fclose(fp);
    return;
  }

  if (model->ncon == 0) {
    model->n = n;
    model->m = m;
    model->lat_lim[0] = lat0;
    model->lat_lim[1] = lat1;
    model->lon_lim[0] = lon0;
    model->lon_lim[1] = lon1;
  }

  struct mbotps_confile *cf = &model->con[model->ncon];
  strncpy(cf->name, mbotps_constit_name[constit_i], sizeof(cf->name) - 1);
  cf->conidx = constit_i;
  cf->ncmod = nc;
  cf->fp = fp;
  model->ncon++;
}

/* Strategy 1 (the standard OTPS convention): "<otps_path>/DATA/Model_
   <otps_model>" names the elevation ("h_") atlas files via a single '*'
   standing in for the constituent name, e.g.
   "DATA/TPXO10_atlas_v2/h_*_tpxo10_atlas_30_v2". */
static void mbotps_open_via_control_file(struct mbotps_model *model,
                                          const char *otps_path, const char *otps_model) {
  mb_pathplus ctrlfile = "";
  snprintf(ctrlfile, sizeof(ctrlfile), "%s/DATA/Model_%s", otps_path, otps_model);
  FILE *cfp = fopen(ctrlfile, "r");
  if (cfp == NULL)
    return;
  mb_path hline = "";
  const bool got_line = (fgets(hline, sizeof(hline), cfp) != NULL);
  fclose(cfp);
  if (!got_line)
    return;
  mbotps_trim_line(hline);

  char *star = strchr(hline, '*');
  if (star == NULL)
    return;
  const int prefix_len = (int)(star - hline);
  const char *suffix = star + 1;

  for (int i = 0; i < MBOTPS_CONSTITUENT_MAX; i++) {
    mb_path relname = "";
    snprintf(relname, sizeof(relname), "%.*s%s%s", prefix_len, hline,
             mbotps_constit_name[i], suffix);

    mb_pathplus fullpath = "";
    if (relname[0] == '/')
      snprintf(fullpath, sizeof(fullpath), "%s", relname);
    else
      snprintf(fullpath, sizeof(fullpath), "%s/%s", otps_path, relname);

    mbotps_add_constituent(model, i, fullpath);
  }
}

/* Strategy 2 (fallback): no "Model_<otps_model>" control file was found or
   usable, so instead scan "<otps_path>/DATA/<otps_model>/" directly for
   elevation ("h_") atlas files - this lets a model directory obtained
   from OSU (e.g. TPXO10_atlas_v2/) be used by naming the directory itself
   as --otps-model, with no accompanying "Model_" control file needed at
   all. Each directory entry is matched by taking the token immediately
   after "h_" up to the next '_' (or end of name) and comparing it, exact
   length and all, against the known constituent codes; this does not
   assume any fixed suffix, unlike the wildcard template of Strategy 1. */
static void mbotps_open_via_directory(struct mbotps_model *model,
                                       const char *otps_path, const char *otps_model) {
  mb_pathplus dirpath = "";
  snprintf(dirpath, sizeof(dirpath), "%s/DATA/%s", otps_path, otps_model);
  DIR *dp = opendir(dirpath);
  if (dp == NULL)
    return;

  struct dirent *entry;
  while ((entry = readdir(dp)) != NULL) {
    const char *name = entry->d_name;
    if (strncmp(name, "h_", 2) != 0)
      continue;
    const char *rest = name + 2;
    const size_t toklen = strcspn(rest, "_");

    for (int i = 0; i < MBOTPS_CONSTITUENT_MAX; i++) {
      if (strlen(mbotps_constit_name[i]) != toklen ||
          strncmp(rest, mbotps_constit_name[i], toklen) != 0)
        continue;

      mb_pathplus fullpath = "";
      snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, name);
      mbotps_add_constituent(model, i, fullpath);
      break;
    }
  }
  closedir(dp);
}

/*--------------------------------------------------------------------*/
int mb_otps_model_open(int verbose, const char *otps_path, const char *otps_model,
                        struct mbotps_model *model, int *error) {
  memset(model, 0, sizeof(*model));
  *error = MB_ERROR_NO_ERROR;

  /* prefer the standard OTPS "Model_<name>" control file if one exists
     and is usable; otherwise fall back to treating <name> as a model
     directory under DATA/ directly */
  mbotps_open_via_control_file(model, otps_path, otps_model);
  if (model->ncon == 0)
    mbotps_open_via_directory(model, otps_path, otps_model);

  if (model->ncon == 0) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }

  if (verbose > 0) {
    fprintf(stderr, "Opened OTPS atlas model '%s': %d x %d grid, %d constituents:",
            otps_model, model->n, model->m, model->ncon);
    for (int i = 0; i < model->ncon; i++)
      fprintf(stderr, " %s", model->con[i].name);
    fprintf(stderr, "\n");
  }

  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mb_otps_model_close(int verbose, struct mbotps_model *model, int *error) {
  for (int i = 0; i < model->ncon; i++) {
    if (model->con[i].fp != NULL) {
      fclose(model->con[i].fp);
      model->con[i].fp = NULL;
    }
  }
  model->ncon = 0;
  *error = MB_ERROR_NO_ERROR;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mb_otps_predict(int verbose, struct mbotps_model *model, double lon, double lat,
                     double time_d, int *ok, double *tide, int *error) {
  *ok = 0;
  *tide = 0.0;
  *error = MB_ERROR_NO_ERROR;

  /* bring longitude within the model's stated limits by a single +/-360
     degree shift, matching OTPS's own longitude-convention handling */
  double lonc = lon;
  if (lonc < model->lon_lim[0])
    lonc += 360.0;
  if (lonc > model->lon_lim[1])
    lonc -= 360.0;
  if (lonc < model->lon_lim[0] || lonc > model->lon_lim[1])
    return MB_SUCCESS; /* outside model grid: *ok stays 0 */
  if (lat < model->lat_lim[0] || lat > model->lat_lim[1])
    return MB_SUCCESS;

  /* OTPS computes the grid cell location and interpolation weights in
     single precision (subs.f90 BSI_indices/interp_da both declare these
     as plain "real"); reproduced here as float so that grid geometry
     rounds the same way OTPS's own executable does. */
  const float dphi = (model->lon_lim[1] - model->lon_lim[0]) / model->n;
  const float dtheta = (model->lat_lim[1] - model->lat_lim[0]) / model->m;

  /* locate the surrounding z-grid cell (OTPS BSI_indices, zuv='z' case) */
  float xi = ((float)lonc - model->lon_lim[0]) / dphi + 0.5f;
  float xj = ((float)lat - model->lat_lim[0]) / dtheta + 0.5f;
  if (xi < 1.0f)
    xi += model->n;

  const int i0 = (int)xi;
  const float x = xi - i0;
  const int j0 = (int)xj;
  const float y = xj - j0;

  if (i0 > model->n || i0 < 1 || j0 > model->m || j0 < 1)
    return MB_SUCCESS; /* out of range: *ok stays 0 */

  const int i1 = (i0 % model->n) + 1; /* periodic wrap in longitude */
  const int j1 = (j0 % model->m) + 1;

  /* z1_re/z1_im hold the atlas harmonic constants interpolated to this
     point, one complex value per constituent; OTPS stores these as
     single-precision complex (interp_da's "uv1"), so the interpolation
     below is done in float and the double-precision array here only
     widens the resulting (already single-precision-rounded) values for
     convenience in the double-precision code that follows */
  double z1_re[MBOTPS_CONSTITUENT_MAX], z1_im[MBOTPS_CONSTITUENT_MAX];
  float ww00 = 0.0f, ww01 = 0.0f, ww10 = 0.0f, ww11 = 0.0f;

  for (int ic = 0; ic < model->ncon; ic++) {
    struct mbotps_confile *cf = &model->con[ic];
    double v00red, v00imd, v10red, v10imd, v01red, v01imd, v11red, v11imd;
    if (mbotps_read_value(cf->fp, cf->ncmod, model->n, i0, j0, &v00red, &v00imd) != MB_SUCCESS ||
        mbotps_read_value(cf->fp, cf->ncmod, model->n, i1, j0, &v10red, &v10imd) != MB_SUCCESS ||
        mbotps_read_value(cf->fp, cf->ncmod, model->n, i0, j1, &v01red, &v01imd) != MB_SUCCESS ||
        mbotps_read_value(cf->fp, cf->ncmod, model->n, i1, j1, &v11red, &v11imd) != MB_SUCCESS) {
      *error = MB_ERROR_EOF;
      return MB_FAILURE;
    }
    /* the values on disk are already single precision; these casts are
       only to make that explicit for the arithmetic below */
    const float v00re = (float)v00red, v00im = (float)v00imd;
    const float v10re = (float)v10red, v10im = (float)v10imd;
    const float v01re = (float)v01red, v01im = (float)v01imd;
    const float v11re = (float)v11red, v11im = (float)v11imd;

    if (ic == 0) {
      /* the first constituent found (in whatever order the model was
         opened - any one will do, since every constituent in a real
         atlas shares the identical land/sea grid) defines the land/sea
         mask and interpolation weights used for all constituents, as
         in OTPS's interp_da: land nodes are stored as exactly (0,0) */
      const float mask00 = (v00re != 0.0f || v00im != 0.0f) ? 1.0f : 0.0f;
      const float mask01 = (v01re != 0.0f || v01im != 0.0f) ? 1.0f : 0.0f;
      const float mask10 = (v10re != 0.0f || v10im != 0.0f) ? 1.0f : 0.0f;
      const float mask11 = (v11re != 0.0f || v11im != 0.0f) ? 1.0f : 0.0f;
      const float w00 = (1.0f - x) * (1.0f - y) * mask00;
      const float w01 = (1.0f - x) * y * mask01;
      const float w10 = x * (1.0f - y) * mask10;
      const float w11 = x * y * mask11;
      const float wtot = w00 + w01 + w10 + w11;
      if (wtot == 0.0f)
        return MB_SUCCESS; /* land / no data here: *ok stays 0 */
      ww00 = w00 / wtot;
      ww01 = w01 / wtot;
      ww10 = w10 / wtot;
      ww11 = w11 / wtot;
    }

    z1_re[ic] = v00re * ww00 + v10re * ww10 + v01re * ww01 + v11re * ww11;
    z1_im[ic] = v00im * ww00 + v10im * ww10 + v01im * ww01 + v11im * ww11;
  }

  /* harmonic reconstruction: OTPS's make_a()+height(), called with
     l_sal=.true. (no solid-earth-tide beta correction applied - see
     ptide()'s call site in subs.f90) plus infer_minor(). The equilibrium
     argument angle is computed in double precision (matching OTPS's
     mixed-mode arithmetic promoting its single-precision omega/phase
     into the surrounding double-precision expression), but the
     resulting per-constituent phasor is then rounded to single
     precision - matching OTPS's make_a(), which stores it into a
     single-precision "complex" - before combining it with z1 and
     accumulating the height sum, since OTPS's height() function
     performs that entire combination in single precision. */
  const double time_mjd = time_d / 86400.0 + 40587.0; /* unix epoch -> MJD */
  double pu[MBOTPS_CONSTITUENT_MAX], pf[MBOTPS_CONSTITUENT_MAX];
  mbotps_nodal(time_mjd, pu, pf);
  const double phase_time = (time_mjd - MBOTPS_PHASE_EPOCH_MJD) * 86400.0;

  float height = 0.0f;
  char names[MBOTPS_CONSTITUENT_MAX][8];
  for (int ic = 0; ic < model->ncon; ic++) {
    const int conidx = model->con[ic].conidx;
    strncpy(names[ic], model->con[ic].name, sizeof(names[ic]) - 1);
    names[ic][sizeof(names[ic]) - 1] = '\0';
    const double ang = (double)mbotps_constit_omega[conidx] * phase_time +
                        (double)mbotps_constit_phase[conidx] + pu[conidx];
    const float a_re = (float)(pf[conidx] * cos(ang));
    const float a_im = (float)(pf[conidx] * sin(ang));
    height += (float)z1_re[ic] * a_re - (float)z1_im[ic] * a_im;
  }

  double dh = 0.0;
  mbotps_infer_minor(model->ncon, names, z1_re, z1_im, time_mjd, &dh);
  /* if inference is refused (too few anchor constituents), dh stays 0,
     matching OTPS's ptide(), which silently drops inference in that case */

  /* OTPS's ptide() adds the double-precision minor-constituent
     correction to the single-precision major-constituent height and
     stores the result back into a single-precision array */
  *tide = (float)((double)height + dh);
  *ok = 1;
  return MB_SUCCESS;
}
