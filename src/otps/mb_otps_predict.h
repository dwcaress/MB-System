/*--------------------------------------------------------------------
 *    The MB-system:  mb_otps_predict.h  9/2/2026
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
 * mb_otps_predict provides a native C implementation of ocean tidal
 * elevation prediction from an OSU Tidal Prediction Software (OTPS)
 * "atlas" format global tide model (e.g. TPXO9_atlas, TPXO10_atlas).
 * It reads the same binary model files used by the OTPS Fortran program
 * predict_tide directly, so no OTPS executable is required at run time -
 * only the OTPS model data files themselves (obtained separately from
 * https://www.tpxo.net/) are needed, at a location outside the MB-System
 * installation, exactly as before.
 *
 * The algorithms implemented here - harmonic tidal reconstruction using
 * equilibrium tidal arguments and nodal corrections, inference of minor
 * constituents from major ones, and bilinear spatial interpolation of
 * atlas grids - are derived from the OSU Tidal Prediction Software (OTPS),
 * whose Fortran source (predict_tide.f90 / subs.f90) is:
 *
 *    Copyright (c) Oregon State University, 2010
 *    Authors: Gary Egbert & Lana Erofeeva
 *             College of Earth, Ocean, and Atmospheric Sciences
 *             Oregon State University, Corvallis, OR 97331-5503
 *             https://www.tpxo.net/
 *
 *    The equilibrium tidal argument and nodal correction formulas
 *    ("arguments"/"astrol") were supplied by Richard Ray (NASA/GSFC),
 *    and the minor-constituent inference formulas are based on
 *    Richard Ray's "perth2" code, both as incorporated into OTPS by
 *    Lana Erofeeva.
 *
 * OTPS is distributed by OSU under a royalty-free, nonexclusive license
 * for academic, research, and other similar noncommercial use, which
 * permits modification and redistribution of the software subject to
 * conditions including attribution of the original copyright (see the
 * OTPS distribution's COPYRIGHT file for the complete license text).
 * This module is a derivative work under that license: it is a
 * transcription/reimplementation in C of the numerical methods used by
 * OTPS's predict_tide, not a copy of the Fortran source code.
 *
 * Author:  D. W. Caress
 * Date:    September 2, 2026
 */

#ifndef MB_OTPS_PREDICT_H_
#define MB_OTPS_PREDICT_H_

#include <stdio.h>

/* maximum number of tidal constituents recognized (matches OTPS constit.h) */
#define MBOTPS_CONSTITUENT_MAX 32

/* one open, single-constituent OTPS atlas elevation ("h_") file */
struct mbotps_confile {
  char name[8]; /* lower case constituent name, e.g. "m2" */
  int conidx;   /* 0-based index of this constituent in the internal
                    32-constituent tidal constant table (equilibrium
                    argument frequency/phase and nodal correction) */
  int ncmod;    /* number of constituents recorded in this file's own
                    header (always 1 for atlas-format files); cached here
                    to reproduce OTPS's direct-access byte offset formula */
  FILE *fp;     /* left open for the life of the model for direct-access reads */
};

/* an open OTPS atlas tide model (elevation prediction only) */
struct mbotps_model {
  int n;                              /* grid size in longitude */
  int m;                              /* grid size in latitude */
  float lat_lim[2];                   /* latitude limits (deg), low to high */
  float lon_lim[2];                   /* longitude limits (deg), low to high */
  int ncon;                           /* number of constituent files found */
  struct mbotps_confile con[MBOTPS_CONSTITUENT_MAX];
};

/* Open an OTPS atlas model given the OTPS installation/data path and model
   name (e.g. otps_path="/usr/local/src/otps", otps_model="tpxo9_atlas_v5").
   Two ways of locating the model's elevation ("h_") atlas files are tried,
   in order:
     1) the "<otps_path>/DATA/Model_<otps_model>" control file convention
        used by OTPS and by the original mbotps, whose first line names
        the "h_" files via a single '*' standing in for the constituent
        name; or, if that file does not exist or is not usable,
     2) treating otps_model as the name of a model directory directly
        under DATA/ (e.g. otps_model="TPXO10_atlas_v2") and scanning it
        for "h_<constituent>..." files - this lets a model directory
        obtained from OSU be used with no "Model_" control file at all.
   Either way, only the elevation files are opened; current/transport
   ("u_") files and the bathymetry grid file are not needed for elevation
   prediction and are not touched. */
int mb_otps_model_open(int verbose, const char *otps_path, const char *otps_model,
                        struct mbotps_model *model, int *error);

/* Close an OTPS atlas model, releasing all open file handles. */
int mb_otps_model_close(int verbose, struct mbotps_model *model, int *error);

/* Predict the ocean tidal elevation (meters) at the given longitude (degrees,
   either sign convention accepted), latitude (degrees), and time (time_d,
   seconds since 1970/1/1 00:00:00 UTC, MB-System's standard epoch).
   On return, *ok is 1 if the position is inside the model grid and not on
   land (in which case *tide is valid), or 0 if the position is outside the
   grid or on land (in which case *tide is undefined). */
int mb_otps_predict(int verbose, struct mbotps_model *model, double lon, double lat,
                     double time_d, int *ok, double *tide, int *error);

#endif /* MB_OTPS_PREDICT_H_ */
