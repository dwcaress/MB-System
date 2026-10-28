/*--------------------------------------------------------------------
 *    The MB-system:  mb_cube_options.c  10/28/2026
 *
 *    Copyright (c); 2026 by
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
 * mb_cube_options.c holds the parts of CUBE processing shared by the
 * programs that build CUBE grids (mbgrid -F9 and mbclean), so that they
 * build identical grids from identical options:
 *   - the --cube-* command line options, their defaults and checks,
 *   - the resolution of the IHO S-44 edition and order,
 *   - the sounding uncertainty models,
 *   - the predicted surface from a grid file or from the median of the
 *     soundings nearest to each node.
 * The CUBE algorithm itself is in mb_cube.c.
 *
 * Author:  C. dos Santos Ferreira
 * Date:  October 28, 2026
 */

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_cube.h"
#include "mb_define.h"
#include "mb_status.h"

#define MB_CUBE_CONF_95PC 1.96 /* unit normal scale for a 95% confidence interval */
#define MB_CUBE_MEDIAN_ALLOC_MIN (1024 * 1024) /* minimum number of median bins allocated */

/* sounding binned to its nearest node of a median predicted surface */
struct mb_cube_median_bin {
  int node;
  float depth;
};

/* soundings collected for a median predicted surface */
struct mb_cube_median_struct {
  int nx; /* CUBE grid dimensions */
  int ny;
  double dx; /* CUBE node spacing (m) */
  double dy;
  int factor; /* median grid spacing in node spacings */
  int mxdim;  /* median grid dimensions */
  int mydim;
  double max_bytes; /* memory limit of the bins */
  struct mb_cube_median_bin *bins;
  int nbins;
  int nbins_alloc;
};

/*--------------------------------------------------------------------------*/
int mb_cube_default_options(int verbose, struct mb_cube_options_struct *options, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       options:      %p\n", options);
  }

  memset(options, 0, sizeof(struct mb_cube_options_struct));
  int status = mb_cube_default_param(verbose, MB_CUBE_IHO_ORDER_2, &options->param, error);
  options->iho_edition = MB_CUBE_IHO_EDITION_6;
  options->iho_edition_set = 0;
  options->iho_ccom = 0;
  snprintf(options->iho_order, sizeof(options->iho_order), "2");
  options->model = MB_CUBE_MODEL_IHO;
  options->sigma_v = 0.25;
  options->sigma_h = 1.0;
  options->model_a = 0.25;
  options->model_b = 0.0066;
  options->model_c = 0.0;
  options->depth_below_sonar = 1;
  options->prediction = MB_CUBE_PREDICTION_NONE;
  options->prediction_factor = 1;
  options->prediction_file[0] = '\0';
  options->prediction_pct = 5.0;
  options->prediction_hypotheses = 1;
  options->memory_limit = 16.0;
  options->thu_fixed = 0.0;
  options->thu_fraction = 0.0;
  options->iho_label[0] = '\0';

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Parse one --cube-* long option, given by its name without the leading
   dashes and its value (NULL for options without a value). Unknown
   options and invalid values return MB_FAILURE and MB_ERROR_BAD_PARAMETER. */
int mb_cube_parse_option(int verbose, const char *name, const char *value, struct mb_cube_options_struct *options,
                         int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       name:         %s\n", name);
    fprintf(stderr, "dbg2       value:        %s\n", value != NULL ? value : "(null)");
    fprintf(stderr, "dbg2       options:      %p\n", options);
  }

  bool ok = false;
  bool done = false;
  int n = 0;
  char extra;
  if (strcmp(name, "cube-iho-ccom") == 0) {
    options->iho_ccom = 1;
    ok = true;
    done = true;
  }
  else if (value == NULL) {
    done = true;
  }
  else if (strcmp(name, "cube-order") == 0) {
    /* any order name of a known edition; checked against the selected
       edition by mb_cube_resolve_options() once all options are read */
    const int editions[] = {MB_CUBE_IHO_EDITION_4, MB_CUBE_IHO_EDITION_5, MB_CUBE_IHO_EDITION_6, MB_CUBE_IHO_CCOM};
    double tvu_a, tvu_b, thu_fixed, thu_fraction;
    int order_error = MB_ERROR_NO_ERROR;
    for (int k = 0; k < 4 && !ok; k++)
      ok = mb_cube_iho_edition_limits(verbose, editions[k], value, &tvu_a, &tvu_b, &thu_fixed, &thu_fraction,
                                      &order_error) == MB_SUCCESS;
    if (ok)
      snprintf(options->iho_order, sizeof(options->iho_order), "%s", value);
    done = true;
  }
  else if (strcmp(name, "cube-iho-edition") == 0) {
    int edition = 0;
    n = sscanf(value, "%d%c", &edition, &extra);
    if (n == 1 && (edition == MB_CUBE_IHO_EDITION_4 || edition == MB_CUBE_IHO_EDITION_5 || edition == MB_CUBE_IHO_EDITION_6)) {
      options->iho_edition = edition;
      options->iho_edition_set = 1;
      ok = true;
    }
    done = true;
  }
  else if (strcmp(name, "cube-model") == 0) {
    ok = true;
    if (strcmp(value, "iho") == 0)
      options->model = MB_CUBE_MODEL_IHO;
    else if (strcmp(value, "constant") == 0)
      options->model = MB_CUBE_MODEL_CONSTANT;
    else if (strcmp(value, "depth") == 0)
      options->model = MB_CUBE_MODEL_DEPTH;
    else
      ok = false;
    done = true;
  }
  else if (strcmp(name, "cube-extractor") == 0) {
    ok = true;
    if (strcmp(value, "prior") == 0)
      options->param.extractor = MB_CUBE_EXTRACT_PRIOR;
    else if (strcmp(value, "likelihood") == 0)
      options->param.extractor = MB_CUBE_EXTRACT_LIKELIHOOD;
    else if (strcmp(value, "posterior") == 0)
      options->param.extractor = MB_CUBE_EXTRACT_POSTERIOR;
    else if (strcmp(value, "predicted") == 0)
      options->param.extractor = MB_CUBE_EXTRACT_PREDICTED;
    else
      ok = false;
    done = true;
  }
  else if (strcmp(name, "cube-prediction") == 0) {
    if (strcmp(value, "median") == 0 || strncmp(value, "median/", 7) == 0) {
      options->prediction = MB_CUBE_PREDICTION_MEDIAN;
      options->prediction_factor = 1;
      ok = true;
      if (value[6] == '/') {
        n = sscanf(&value[7], "%d%c", &options->prediction_factor, &extra);
        ok = n == 1 && options->prediction_factor >= 1;
      }
    }
    else {
      options->prediction = MB_CUBE_PREDICTION_FILE;
      snprintf(options->prediction_file, sizeof(options->prediction_file), "%s", value);
      ok = value[0] != '\0';
    }
    done = true;
  }
  else if (strcmp(name, "cube-prediction-hypotheses") == 0) {
    ok = true;
    if (strcmp(value, "on") == 0)
      options->prediction_hypotheses = 1;
    else if (strcmp(value, "off") == 0)
      options->prediction_hypotheses = 0;
    else
      ok = false;
    done = true;
  }
  else if (strcmp(name, "cube-blunder") == 0) {
    if (strcmp(value, "off") == 0) {
      options->param.blunder = 0;
      ok = true;
    }
    else {
      options->param.blunder = 1;
      n = sscanf(value, "%lf/%lf/%lf%c", &options->param.blunder_min, &options->param.blunder_fraction,
                 &options->param.blunder_scalar, &extra);
      ok = n == 3;
    }
    done = true;
  }
  else if (strcmp(name, "cube-uncertainty") == 0) {
    ok = true;
    if (strcmp(value, "sample") == 0)
      options->param.uncertainty_mode = MB_CUBE_UNCT_SAMPLE;
    else if (strcmp(value, "posterior") == 0)
      options->param.uncertainty_mode = MB_CUBE_UNCT_POSTERIOR;
    else if (strcmp(value, "max") == 0)
      options->param.uncertainty_mode = MB_CUBE_UNCT_MAX;
    else
      ok = false;
    done = true;
  }
  else if (strcmp(name, "cube-depth-reference") == 0) {
    ok = true;
    if (strcmp(value, "sonar") == 0)
      options->depth_below_sonar = 1;
    else if (strcmp(value, "surface") == 0)
      options->depth_below_sonar = 0;
    else
      ok = false;
    done = true;
  }
  else if (strcmp(name, "cube-context") == 0) {
    n = sscanf(value, "%lf/%lf%c", &options->param.min_context, &options->param.max_context, &extra);
    ok = n == 2;
    done = true;
  }
  else if (strcmp(name, "cube-median-length") == 0) {
    n = sscanf(value, "%d%c", &options->param.median_length, &extra);
    ok = n == 1;
    done = true;
  }
  else if (strcmp(name, "cube-runlength") == 0) {
    n = sscanf(value, "%d%c", &options->param.runlength_t, &extra);
    ok = n == 1;
    done = true;
  }

  /* options with one real value */
  if (!done) {
    double *target = NULL;
    if (strcmp(name, "cube-bayes-factor") == 0)
      target = &options->param.bayes_fac_t;
    else if (strcmp(name, "cube-capture-min") == 0)
      target = &options->param.capture_min;
    else if (strcmp(name, "cube-capture-scale") == 0)
      target = &options->param.capture_scale;
    else if (strcmp(name, "cube-dist-exp") == 0)
      target = &options->param.dist_exp;
    else if (strcmp(name, "cube-est-offset") == 0)
      target = &options->param.est_offset;
    else if (strcmp(name, "cube-horiz-scale") == 0)
      target = &options->param.horiz_scale;
    else if (strcmp(name, "cube-influence-scale") == 0)
      target = &options->param.influence_scale;
    else if (strcmp(name, "cube-memory-limit") == 0)
      target = &options->memory_limit;
    else if (strcmp(name, "cube-model-a") == 0)
      target = &options->model_a;
    else if (strcmp(name, "cube-model-b") == 0)
      target = &options->model_b;
    else if (strcmp(name, "cube-model-c") == 0)
      target = &options->model_c;
    else if (strcmp(name, "cube-prediction-uncertainty") == 0)
      target = &options->prediction_pct;
    else if (strcmp(name, "cube-quotient-limit") == 0)
      target = &options->param.quotient_limit;
    else if (strcmp(name, "cube-sigma-h") == 0)
      target = &options->sigma_h;
    else if (strcmp(name, "cube-sigma-v") == 0)
      target = &options->sigma_v;
    else if (strcmp(name, "cube-uncertainty-scale") == 0)
      target = &options->param.sd2conf;
    if (target != NULL) {
      n = sscanf(value, "%lf%c", target, &extra);
      ok = n == 1;
    }
  }

  const int status = ok ? MB_SUCCESS : MB_FAILURE;
  *error = ok ? MB_ERROR_NO_ERROR : MB_ERROR_BAD_PARAMETER;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Look up the IHO S-44 limits of the selected edition (or the CCOM
   errmod_iho table) and order, once all options are read: they give the
   sounding uncertainties of the iho model and the TVU bound of the
   influence radius. On failure the reason is written to message. */
int mb_cube_resolve_options(int verbose, struct mb_cube_options_struct *options, char *message, int size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       options:      %p\n", options);
    fprintf(stderr, "dbg2       iho_edition:  %d\n", options->iho_edition);
    fprintf(stderr, "dbg2       iho_ccom:     %d\n", options->iho_ccom);
    fprintf(stderr, "dbg2       iho_order:    %s\n", options->iho_order);
    fprintf(stderr, "dbg2       size:         %d\n", size);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  if (size > 0)
    message[0] = '\0';
  const int edition = options->iho_ccom ? MB_CUBE_IHO_CCOM : options->iho_edition;
  double tvu_a, tvu_b, thu_fixed, thu_fraction;
  int lookup_error = MB_ERROR_NO_ERROR;
  if (options->iho_ccom && options->iho_edition_set) {
    snprintf(message, (size_t)size, "\nThe options --cube-iho-ccom and --cube-iho-edition cannot be combined\n");
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }
  else if (mb_cube_iho_edition_limits(verbose, edition, options->iho_order, &tvu_a, &tvu_b, &thu_fixed, &thu_fraction,
                                      &lookup_error) != MB_SUCCESS) {
    char orders[MB_PATH_MAXLINE];
    mb_cube_iho_edition_orders(verbose, edition, orders, sizeof(orders), &lookup_error);
    if (options->iho_ccom)
      snprintf(message, (size_t)size, "\nThe CCOM errmod_iho table (--cube-iho-ccom) has no order %s; its orders are %s\n",
               options->iho_order, orders);
    else
      snprintf(message, (size_t)size, "\nIHO S-44 edition %d has no order %s; its orders are %s\n", edition,
               options->iho_order, orders);
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }
  else {
    options->param.tvu_a = tvu_a;
    options->param.tvu_b = tvu_b;
    options->thu_fixed = thu_fixed;
    options->thu_fraction = thu_fraction;
    char order_label[32];
    if (strcmp(options->iho_order, "special") == 0)
      snprintf(order_label, sizeof(order_label), "Special Order");
    else if (strcmp(options->iho_order, "exclusive") == 0)
      snprintf(order_label, sizeof(order_label), "Exclusive Order");
    else
      snprintf(order_label, sizeof(order_label), "Order %s", options->iho_order);
    if (options->iho_ccom)
      snprintf(options->iho_label, sizeof(options->iho_label), "CCOM errmod_iho %s", order_label);
    else
      snprintf(options->iho_label, sizeof(options->iho_label), "IHO S-44 Ed. %d %s", edition, order_label);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       tvu_a:        %f\n", options->param.tvu_a);
    fprintf(stderr, "dbg2       tvu_b:        %f\n", options->param.tvu_b);
    fprintf(stderr, "dbg2       thu_fixed:    %f\n", options->thu_fixed);
    fprintf(stderr, "dbg2       thu_fraction: %f\n", options->thu_fraction);
    fprintf(stderr, "dbg2       iho_label:    %s\n", options->iho_label);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Check the algorithm, uncertainty model and predicted surface options;
   every problem found is appended to message. */
int mb_cube_check_options(int verbose, const char *program_name, const struct mb_cube_options_struct *options,
                          char *message, int size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       program_name: %s\n", program_name);
    fprintf(stderr, "dbg2       options:      %p\n", options);
    fprintf(stderr, "dbg2       size:         %d\n", size);
  }

  int len = 0;
  if (size > 0)
    message[0] = '\0';
  int param_error = MB_ERROR_NO_ERROR;
  if (mb_cube_check_param(verbose, &options->param, &param_error) != MB_SUCCESS && len < size)
    len += snprintf(message + len, (size_t)(size - len),
                    "\nCUBE algorithm parameters out of range, see the %s manual page\n", program_name);
  /* the largest Bayes factor of a single sample, at zero error, is
     exp(h^2 / 2): from there on the monitor rejects every sample */
  const double bayes_factor_max = exp(0.5 * options->param.est_offset * options->param.est_offset);
  if (!(options->param.bayes_fac_t < bayes_factor_max) && len < size)
    len += snprintf(message + len, (size_t)(size - len),
                    "\nThe Bayes factor threshold (--cube-bayes-factor) must be below exp(h^2 / 2) = %g for the\n"
                    "estimate offset h = %g (--cube-est-offset); otherwise every sounding is an outlier\n",
                    bayes_factor_max, options->param.est_offset);
  if (options->param.extractor == MB_CUBE_EXTRACT_PREDICTED && options->prediction == MB_CUBE_PREDICTION_NONE && len < size)
    len += snprintf(message + len, (size_t)(size - len),
                    "\nThe predicted hypothesis selection rule needs a predicted surface (--cube-prediction)\n");
  if (options->prediction != MB_CUBE_PREDICTION_NONE && !(options->prediction_pct > 0.0 && options->prediction_pct < 100.0)
      && len < size)
    len += snprintf(message + len, (size_t)(size - len),
                    "\nThe predicted surface uncertainty must be between 0 and 100 percent of depth\n");
  if ((!(options->sigma_v > 0.0) || !(options->sigma_h >= 0.0) || !(options->model_a > 0.0) || !(options->model_b >= 0.0)
       || !(options->model_c >= 0.0) || !(options->memory_limit > 0.0))
      && len < size)
    len += snprintf(message + len, (size_t)(size - len),
                    "\nCUBE uncertainty model parameters and memory limit must be positive\n");

  const int status = len > 0 ? MB_FAILURE : MB_SUCCESS;
  *error = len > 0 ? MB_ERROR_BAD_PARAMETER : MB_ERROR_NO_ERROR;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Vertical and horizontal variances (m^2, one sigma) of a sounding from
   the selected uncertainty model, with h the depth below the sonar or the
   sea surface (the sign of h is ignored). The IHO limits must have been
   resolved by mb_cube_resolve_options(). */
int mb_cube_sounding_variance(int verbose, const struct mb_cube_options_struct *options, double h, double *var_depth,
                              double *var_horiz, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       options:      %p\n", options);
    fprintf(stderr, "dbg2       h:            %f\n", h);
  }

  h = fabs(h);
  double sigma_v;
  double sigma_h;
  if (options->model == MB_CUBE_MODEL_IHO) {
    /* IHO S-44 limits are 95% values */
    sigma_v = sqrt(options->param.tvu_a * options->param.tvu_a + options->param.tvu_b * options->param.tvu_b * h * h)
              / MB_CUBE_CONF_95PC;
    sigma_h = (options->thu_fixed + options->thu_fraction * h) / MB_CUBE_CONF_95PC;
  }
  else if (options->model == MB_CUBE_MODEL_CONSTANT) {
    sigma_v = options->sigma_v;
    sigma_h = options->sigma_h;
  }
  else {
    sigma_v = sqrt(options->model_a * options->model_a + options->model_b * options->model_b * h * h);
    sigma_h = sqrt(options->sigma_h * options->sigma_h + options->model_c * options->model_c * h * h);
  }
  *var_depth = sigma_v * sigma_v;
  *var_horiz = sigma_h * sigma_h;
  const int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       var_depth:    %f\n", *var_depth);
    fprintf(stderr, "dbg2       var_horiz:    %f\n", *var_horiz);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Sample a grid (layout k = i * grid_ny + j, node (i,j) at
   (grid_x0 + i * grid_dx, grid_y0 + j * grid_dy)) bilinearly at the nodes of
   an nx x ny grid with origin (x0, y0) and spacing (dx, dy) in the same
   coordinates, multiplying the values by sign. Nodes with a missing
   surrounding grid value are set to nodata. */
int mb_cube_sample_grid(int verbose, const float *grid, int grid_nx, int grid_ny, double grid_x0, double grid_y0,
                        double grid_dx, double grid_dy, float grid_nodata, double sign, int nx, int ny, double x0,
                        double y0, double dx, double dy, float nodata, float *depth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       grid:         %p\n", grid);
    fprintf(stderr, "dbg2       grid_nx:      %d\n", grid_nx);
    fprintf(stderr, "dbg2       grid_ny:      %d\n", grid_ny);
    fprintf(stderr, "dbg2       grid_x0:      %f\n", grid_x0);
    fprintf(stderr, "dbg2       grid_y0:      %f\n", grid_y0);
    fprintf(stderr, "dbg2       grid_dx:      %f\n", grid_dx);
    fprintf(stderr, "dbg2       grid_dy:      %f\n", grid_dy);
    fprintf(stderr, "dbg2       grid_nodata:  %f\n", grid_nodata);
    fprintf(stderr, "dbg2       sign:         %f\n", sign);
    fprintf(stderr, "dbg2       nx:           %d\n", nx);
    fprintf(stderr, "dbg2       ny:           %d\n", ny);
    fprintf(stderr, "dbg2       x0:           %f\n", x0);
    fprintf(stderr, "dbg2       y0:           %f\n", y0);
    fprintf(stderr, "dbg2       dx:           %f\n", dx);
    fprintf(stderr, "dbg2       dy:           %f\n", dy);
    fprintf(stderr, "dbg2       nodata:       %f\n", nodata);
  }

  for (int i = 0; i < nx; i++)
    for (int j = 0; j < ny; j++) {
      const int k = i * ny + j;
      depth[k] = nodata;
      const double gx = (x0 + i * dx - grid_x0) / grid_dx;
      const double gy = (y0 + j * dy - grid_y0) / grid_dy;
      const int gi = (int)floor(gx);
      const int gj = (int)floor(gy);
      if (gi < 0 || gi >= grid_nx - 1 || gj < 0 || gj >= grid_ny - 1)
        continue;
      const float z00 = grid[gi * grid_ny + gj];
      const float z01 = grid[gi * grid_ny + gj + 1];
      const float z10 = grid[(gi + 1) * grid_ny + gj];
      const float z11 = grid[(gi + 1) * grid_ny + gj + 1];
      if (z00 == grid_nodata || z01 == grid_nodata || z10 == grid_nodata || z11 == grid_nodata)
        continue;
      const double fx = gx - gi;
      const double fy = gy - gj;
      depth[k] = (float)(sign * (z00 * (1.0 - fx) * (1.0 - fy) + z10 * fx * (1.0 - fy) + z01 * (1.0 - fx) * fy
                                 + z11 * fx * fy));
    }
  const int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Set a predicted surface given as depths (positive down) at the nodes of
   the CUBE grid, nodata where there is none. Its uncertainty is the
   percentage of depth given as a 95% value, as in the reference initsheet
   -p: of the depth below the sonar of the first sounding reaching each
   node, or of the predicted depth itself with the sea surface as depth
   reference. */
int mb_cube_apply_prediction(int verbose, void *cube_ptr, const struct mb_cube_options_struct *options, int nx, int ny,
                             const float *depth, float nodata, int *npred, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:     %p\n", cube_ptr);
    fprintf(stderr, "dbg2       options:      %p\n", options);
    fprintf(stderr, "dbg2       nx:           %d\n", nx);
    fprintf(stderr, "dbg2       ny:           %d\n", ny);
    fprintf(stderr, "dbg2       depth:        %p\n", depth);
    fprintf(stderr, "dbg2       nodata:       %f\n", nodata);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *npred = 0;
  for (int k = 0; k < nx * ny; k++)
    if (depth[k] != nodata)
      (*npred)++;
  if (options->depth_below_sonar) {
    status = mb_cube_set_prediction(verbose, cube_ptr, depth, NULL, nodata, 0.01 * options->prediction_pct,
                                    options->prediction_hypotheses ? 1 : 0, error);
  }
  else {
    float *variance = NULL;
    status = mb_mallocd(verbose, __FILE__, __LINE__, (size_t)nx * ny * sizeof(float), (void **)&variance, error);
    if (status == MB_SUCCESS) {
      for (int k = 0; k < nx * ny; k++) {
        const double sd = 0.01 * options->prediction_pct * fabs(depth[k]) / MB_CUBE_CONF_95PC;
        variance[k] = depth[k] != nodata ? (float)(sd * sd) : 0.0f;
      }
      status = mb_cube_set_prediction(verbose, cube_ptr, depth, variance, nodata, 0.0,
                                      options->prediction_hypotheses ? 1 : 0, error);
      int free_error = MB_ERROR_NO_ERROR;
      mb_freed(verbose, __FILE__, __LINE__, (void **)&variance, &free_error);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       npred:        %d\n", *npred);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Start collecting soundings for a median predicted surface on a grid
   factor times coarser than the nx x ny CUBE grid with node spacing
   (dx, dy) in meters; the soundings use at most max_bytes of memory. */
int mb_cube_median_init(int verbose, int nx, int ny, double dx, double dy, int factor, double max_bytes,
                        void **median_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       nx:           %d\n", nx);
    fprintf(stderr, "dbg2       ny:           %d\n", ny);
    fprintf(stderr, "dbg2       dx:           %f\n", dx);
    fprintf(stderr, "dbg2       dy:           %f\n", dy);
    fprintf(stderr, "dbg2       factor:       %d\n", factor);
    fprintf(stderr, "dbg2       max_bytes:    %f\n", max_bytes);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *median_ptr = NULL;
  if (nx < 1 || ny < 1 || !(dx > 0.0) || !(dy > 0.0) || factor < 1) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }
  else {
    status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mb_cube_median_struct), median_ptr, error);
  }
  if (status == MB_SUCCESS) {
    struct mb_cube_median_struct *median = (struct mb_cube_median_struct *)*median_ptr;
    median->nx = nx;
    median->ny = ny;
    median->dx = dx;
    median->dy = dy;
    median->factor = factor;
    median->mxdim = (nx - 1) / factor + 1;
    median->mydim = (ny - 1) / factor + 1;
    median->max_bytes = max_bytes;
    median->bins = NULL;
    median->nbins = 0;
    median->nbins_alloc = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       median_ptr:   %p\n", *median_ptr);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Bin one sounding, at (x, y) in meters from node (0,0), to its nearest
   median grid node; soundings outside the grid are ignored. If the memory
   of the bins would exceed the limit, or cannot be allocated, or more than
   INT_MAX soundings would be binned, MB_FAILURE and MB_ERROR_MEMORY_FAIL
   are returned with the memory wanted in bytes. */
int mb_cube_median_add(int verbose, void *median_ptr, double x, double y, double depth, double *bytes, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       median_ptr:   %p\n", median_ptr);
    fprintf(stderr, "dbg2       x:            %f\n", x);
    fprintf(stderr, "dbg2       y:            %f\n", y);
    fprintf(stderr, "dbg2       depth:        %f\n", depth);
  }

  struct mb_cube_median_struct *median = (struct mb_cube_median_struct *)median_ptr;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *bytes = (double)median->nbins_alloc * sizeof(struct mb_cube_median_bin);
  const int i = (int)floor(x / (median->factor * median->dx) + 0.5);
  const int j = (int)floor(y / (median->factor * median->dy) + 0.5);
  if (i >= 0 && i < median->mxdim && j >= 0 && j < median->mydim) {
    if (median->nbins == median->nbins_alloc) {
      /* double the bins, in size_t so that the count cannot overflow, up to
         the INT_MAX bins an int can index */
      size_t nbins_alloc = MAX((size_t)MB_CUBE_MEDIAN_ALLOC_MIN, 2 * (size_t)median->nbins_alloc);
      if (nbins_alloc > INT_MAX)
        nbins_alloc = INT_MAX;
      *bytes = (double)nbins_alloc * sizeof(struct mb_cube_median_bin);
      if (median->nbins == INT_MAX || *bytes > median->max_bytes) {
        status = MB_FAILURE;
        *error = MB_ERROR_MEMORY_FAIL;
      }
      else {
        status = mb_reallocd(verbose, __FILE__, __LINE__, nbins_alloc * sizeof(struct mb_cube_median_bin),
                             (void **)&median->bins, error);
        if (status == MB_SUCCESS)
          median->nbins_alloc = (int)nbins_alloc;
        else {
          median->nbins = 0;
          median->nbins_alloc = 0;
        }
      }
    }
    if (status == MB_SUCCESS) {
      median->bins[median->nbins].node = i * median->mydim + j;
      median->bins[median->nbins].depth = (float)depth;
      median->nbins++;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bytes:        %f\n", *bytes);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
static int mb_cube_median_compare(const void *a, const void *b) {
  const struct mb_cube_median_bin *ba = (const struct mb_cube_median_bin *)a;
  const struct mb_cube_median_bin *bb = (const struct mb_cube_median_bin *)b;
  if (ba->node != bb->node)
    return ba->node < bb->node ? -1 : 1;
  return ba->depth < bb->depth ? -1 : (ba->depth > bb->depth ? 1 : 0);
}
/*--------------------------------------------------------------------------*/
/* Compute the median predicted surface at the nx x ny CUBE nodes: the
   lower median of the depths at each median grid node, as the median of
   heights in the reference (nodes without soundings get no prediction),
   and a coarser median grid interpolated bilinearly to the nodes, as the
   reference interpolates its initialisation grid (nodes need valid median
   values at the corners of their cell). nused returns the number of
   soundings used; the soundings are released. */
int mb_cube_median_surface(int verbose, void *median_ptr, float nodata, float *depth, int *nused, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       median_ptr:   %p\n", median_ptr);
    fprintf(stderr, "dbg2       nodata:       %f\n", nodata);
    fprintf(stderr, "dbg2       depth:        %p\n", depth);
  }

  struct mb_cube_median_struct *median = (struct mb_cube_median_struct *)median_ptr;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  *nused = median->nbins;
  const int mf = median->factor;
  const int mxdim = median->mxdim;
  const int mydim = median->mydim;
  if (median->nbins > 0)
    qsort(median->bins, (size_t)median->nbins, sizeof(struct mb_cube_median_bin), mb_cube_median_compare);
  float *mgrid = depth;
  if (mf > 1)
    status = mb_mallocd(verbose, __FILE__, __LINE__, (size_t)mxdim * mydim * sizeof(float), (void **)&mgrid, error);
  if (status == MB_SUCCESS) {
    for (int k = 0; k < mxdim * mydim; k++)
      mgrid[k] = nodata;
    for (int ib0 = 0; ib0 < median->nbins;) {
      int ib1 = ib0;
      while (ib1 < median->nbins && median->bins[ib1].node == median->bins[ib0].node)
        ib1++;
      mgrid[median->bins[ib0].node] = median->bins[ib0 + (ib1 - ib0 - 1) / 2].depth;
      ib0 = ib1;
    }
    if (mf > 1) {
      for (int i = 0; i < median->nx; i++)
        for (int j = 0; j < median->ny; j++) {
          const int k = i * median->ny + j;
          depth[k] = nodata;
          const int mi = i / mf;
          const int mj = j / mf;
          const double fx = (double)(i - mi * mf) / mf;
          const double fy = (double)(j - mj * mf) / mf;
          const int mi1 = fx > 0.0 ? mi + 1 : mi;
          const int mj1 = fy > 0.0 ? mj + 1 : mj;
          if (mi1 >= mxdim || mj1 >= mydim)
            continue;
          const float z00 = mgrid[mi * mydim + mj];
          const float z10 = mgrid[mi1 * mydim + mj];
          const float z01 = mgrid[mi * mydim + mj1];
          const float z11 = mgrid[mi1 * mydim + mj1];
          if (z00 == nodata || z10 == nodata || z01 == nodata || z11 == nodata)
            continue;
          depth[k] = (float)(z00 * (1.0 - fx) * (1.0 - fy) + z10 * fx * (1.0 - fy) + z01 * (1.0 - fx) * fy
                             + z11 * fx * fy);
        }
      int free_error = MB_ERROR_NO_ERROR;
      mb_freed(verbose, __FILE__, __LINE__, (void **)&mgrid, &free_error);
    }
  }
  if (median->bins != NULL) {
    int free_error = MB_ERROR_NO_ERROR;
    mb_freed(verbose, __FILE__, __LINE__, (void **)&median->bins, &free_error);
  }
  median->nbins = 0;
  median->nbins_alloc = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       nused:        %d\n", *nused);
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_cube_median_deall(int verbose, void **median_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       median_ptr:   %p\n", *median_ptr);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  struct mb_cube_median_struct *median = (struct mb_cube_median_struct *)*median_ptr;
  if (median != NULL) {
    if (median->bins != NULL)
      mb_freed(verbose, __FILE__, __LINE__, (void **)&median->bins, error);
    status = mb_freed(verbose, __FILE__, __LINE__, median_ptr, error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
