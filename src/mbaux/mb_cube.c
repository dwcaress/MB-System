/*--------------------------------------------------------------------
 *    The MB-system:  mb_cube.c  10/26/2026
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
 * mb_cube.c implements the CUBE (Combined Uncertainty and Bathymetric
 * Estimator) algorithm on a regular grid of estimation nodes:
 *
 *   Calder, B. R., and L. A. Mayer, 2003, Automatic processing of
 *     high-rate, high-density multibeam echosounder data, Geochem.
 *     Geophys. Geosyst., 4(6), 1048, doi:10.1029/2002GC000486.
 *   Calder, B. R., and D. Wells, 2007, CUBE User's Manual, version 1.13,
 *     Center for Coastal and Ocean Mapping & NOAA/UNH Joint Hydrographic
 *     Center, University of New Hampshire.
 *
 * This is an independent implementation. Its behaviour was checked
 * against the CCOM/JHC reference implementation of CUBE (MIT license);
 * the deliberate differences are:
 *   - depths are positive down,
 *   - the capture distance and the influence radius use the depth below
 *     the sonar passed by the caller, which equals the depth for surface
 *     vessels but not for AUVs and ROVs,
 *   - the minimum capture distance is a parameter,
 *   - a guide node in the likelihood and posterior rules contributes the
 *     larger of its sample and posterior variances, so the comparison never
 *     divides by zero,
 *   - the outlier quotient of the median queue is computed in double
 *     precision about the mean,
 *   - ties are broken in a fixed, documented order.
 * The predicted (initialisation) surface, the blunder test and user
 * nomination of hypotheses are not implemented.
 *
 * Each sounding is offered to the nodes within an influence radius. At
 * each node it passes through a median pre-filter queue and then updates
 * the closest of the node's depth hypotheses with a Kalman (dynamic linear
 * model) step, unless Bayes factor monitoring indicates that it does not
 * belong to that hypothesis, in which case it starts a new one. When the
 * grid is extracted, one hypothesis per node is selected.
 *
 * Author:  C. dos Santos Ferreira
 * Date:  October 26, 2026
 */

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_cube.h"
#include "mb_define.h"
#include "mb_status.h"

#define MB_CUBE_CONF_95PC 1.96            /* unit normal scale for a 95% confidence interval */
#define MB_CUBE_CONF_99PC 2.56            /* unit normal scale for a 99% confidence interval */
#define MB_CUBE_MAX_HYPOTHESIS_RATIO 5.0  /* ceiling of the hypothesis strength */
#define MB_CUBE_ALLOC_MIN 1024            /* minimum number of pool entries allocated */
#define MB_CUBE_NONE -1                   /* no queue or hypothesis */

/* entry of a median pre-filter queue */
struct mb_cube_queue_entry {
  float depth;
  float var;
};

/* depth hypothesis tracked at a node */
struct mb_cube_hyp {
  double cur_estimate;  /* current depth estimate (m) */
  double cur_variance;  /* posterior variance of the estimate (m^2) */
  double pred_estimate; /* one step prediction of depth (m) */
  double pred_variance; /* one step prediction variance (m^2) */
  double cum_bayes_fac; /* cumulative Bayes factor for monitoring */
  float var_estimate;   /* running variance of the assimilated samples (m^2) */
  int seq_length;       /* run length of samples favouring an alternative */
  int num_samples;      /* number of samples assimilated */
  int next;             /* next (older) hypothesis of the node or MB_CUBE_NONE */
};

/* estimation node */
struct mb_cube_node {
  int queue;    /* queue slot or MB_CUBE_NONE */
  int n_queued; /* number of entries in the queue */
  int hyp;      /* newest hypothesis or MB_CUBE_NONE */
  int nhyp;     /* number of hypotheses */
};

struct mb_cube_struct {
  struct mb_cube_param_struct param;
  int nx;
  int ny;
  double dx;
  double dy;
  double dist_scale; /* smaller node spacing (m) */
  double var_scale;  /* dist_scale^(-dist_exp) */
  int min_context;   /* guide search range (nodes) */
  int max_context;
  bool failed; /* a pool allocation failed, the grid is unusable */
  struct mb_cube_node *nodes;
  struct mb_cube_queue_entry *queues; /* pool of queue slots of median_length entries */
  int *queue_free;                    /* stack of free queue slots */
  int nqueue_alloc;
  int nqueue_free;
  struct mb_cube_hyp *hyps; /* pool of hypotheses */
  int nhyp_alloc;
  int nhyp_used;
};

/*--------------------------------------------------------------------------*/
/* IHO S-44 Edition 6.1.0 (October 2022) Table 1: maximum allowable TVU
   sqrt(a^2 + (b * d)^2) and THU fixed + fraction * d, both at 95% confidence. */
int mb_cube_iho_limits(int verbose, int iho_order, double *tvu_a, double *tvu_b, double *thu_fixed, double *thu_fraction,
                       int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       iho_order:    %d\n", iho_order);
  }

  static const double limits[5][4] = {
      {0.15, 0.0075, 1.0, 0.0},   /* Exclusive Order */
      {0.25, 0.0075, 2.0, 0.0},   /* Special Order */
      {0.5, 0.013, 5.0, 0.05},    /* Order 1a */
      {0.5, 0.013, 5.0, 0.05},    /* Order 1b */
      {1.0, 0.023, 20.0, 0.10}};  /* Order 2 */

  int status = MB_SUCCESS;
  if (iho_order < MB_CUBE_IHO_EXCLUSIVE || iho_order > MB_CUBE_IHO_ORDER_2) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }
  else {
    *tvu_a = limits[iho_order][0];
    *tvu_b = limits[iho_order][1];
    *thu_fixed = limits[iho_order][2];
    *thu_fraction = limits[iho_order][3];
    *error = MB_ERROR_NO_ERROR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    if (status == MB_SUCCESS) {
      fprintf(stderr, "dbg2       tvu_a:        %f\n", *tvu_a);
      fprintf(stderr, "dbg2       tvu_b:        %f\n", *tvu_b);
      fprintf(stderr, "dbg2       thu_fixed:    %f\n", *thu_fixed);
      fprintf(stderr, "dbg2       thu_fraction: %f\n", *thu_fraction);
    }
    fprintf(stderr, "dbg2       error:        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:       %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_cube_default_param(int verbose, int iho_order, struct mb_cube_param_struct *param, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       iho_order:    %d\n", iho_order);
    fprintf(stderr, "dbg2       param:        %p\n", param);
  }

  double thu_fixed;
  double thu_fraction;
  const int status = mb_cube_iho_limits(verbose, iho_order, &param->tvu_a, &param->tvu_b, &thu_fixed, &thu_fraction, error);
  if (status == MB_SUCCESS) {
    param->extractor = MB_CUBE_EXTRACT_PRIOR;
    param->uncertainty_mode = MB_CUBE_UNCT_SAMPLE;
    param->median_length = 11;
    param->quotient_limit = 30.0;
    param->discount = 1.0;
    param->est_offset = 4.0;
    param->bayes_fac_t = 0.135;
    param->runlength_t = 5;
    param->dist_exp = 2.0;
    param->horiz_scale = MB_CUBE_CONF_95PC;
    param->capture_scale = 0.05;
    param->capture_min = 0.5;
    param->min_context = 5.0;
    param->max_context = 10.0;
    param->sd2conf = MB_CUBE_CONF_95PC;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_cube_check_param(int verbose, const struct mb_cube_param_struct *param, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       extractor:        %d\n", param->extractor);
    fprintf(stderr, "dbg2       uncertainty_mode: %d\n", param->uncertainty_mode);
    fprintf(stderr, "dbg2       median_length:    %d\n", param->median_length);
    fprintf(stderr, "dbg2       quotient_limit:   %f\n", param->quotient_limit);
    fprintf(stderr, "dbg2       discount:         %f\n", param->discount);
    fprintf(stderr, "dbg2       est_offset:       %f\n", param->est_offset);
    fprintf(stderr, "dbg2       bayes_fac_t:      %f\n", param->bayes_fac_t);
    fprintf(stderr, "dbg2       runlength_t:      %d\n", param->runlength_t);
    fprintf(stderr, "dbg2       dist_exp:         %f\n", param->dist_exp);
    fprintf(stderr, "dbg2       horiz_scale:      %f\n", param->horiz_scale);
    fprintf(stderr, "dbg2       capture_scale:    %f\n", param->capture_scale);
    fprintf(stderr, "dbg2       capture_min:      %f\n", param->capture_min);
    fprintf(stderr, "dbg2       min_context:      %f\n", param->min_context);
    fprintf(stderr, "dbg2       max_context:      %f\n", param->max_context);
    fprintf(stderr, "dbg2       tvu_a:            %f\n", param->tvu_a);
    fprintf(stderr, "dbg2       tvu_b:            %f\n", param->tvu_b);
    fprintf(stderr, "dbg2       sd2conf:          %f\n", param->sd2conf);
  }

  /* the comparisons are written so that NaN values fail them */
  const bool ok = (param->extractor >= MB_CUBE_EXTRACT_PRIOR && param->extractor <= MB_CUBE_EXTRACT_POSTERIOR)
    && (param->uncertainty_mode >= MB_CUBE_UNCT_SAMPLE && param->uncertainty_mode <= MB_CUBE_UNCT_MAX)
    && (param->median_length >= 3 && param->median_length <= 101 && param->median_length % 2 == 1)
    && (param->quotient_limit >= 0.1 && param->quotient_limit <= 255.0)
    && (param->discount >= 0.8 && param->discount <= 1.0)
    && (param->est_offset >= 0.1 && param->est_offset <= 10.0)
    && (param->bayes_fac_t >= 0.001 && param->bayes_fac_t <= 1.0)
    && (param->runlength_t >= 1 && param->runlength_t <= 10)
    && (param->dist_exp >= 1.0 && param->dist_exp <= 10.0)
    && (param->horiz_scale >= 0.0 && param->horiz_scale <= 10.0)
    && (param->capture_scale >= 0.01 && param->capture_scale <= 1.0)
    && (param->capture_min >= 0.0 && param->capture_min <= 100.0)
    && (param->min_context >= 0.0 && param->min_context <= 1000.0)
    && (param->max_context >= param->min_context && param->max_context <= 3000.0)
    && (param->tvu_a > 0.0 && param->tvu_a <= 1000.0)
    && (param->tvu_b >= 0.0 && param->tvu_b <= 1.0)
    && (param->sd2conf > 0.0 && param->sd2conf <= 10.0);

  int status = MB_SUCCESS;
  if (ok) {
    *error = MB_ERROR_NO_ERROR;
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Upper estimate of the memory needed, assuming every node receives data
   and holds on average 1.5 hypotheses. */
int mb_cube_estimate_memory(int verbose, const struct mb_cube_param_struct *param, int nx, int ny, double *bytes,
                            int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       median_length: %d\n", param->median_length);
    fprintf(stderr, "dbg2       nx:            %d\n", nx);
    fprintf(stderr, "dbg2       ny:            %d\n", ny);
  }

  const double nnodes = (double)nx * (double)ny;
  *bytes = nnodes * (sizeof(struct mb_cube_node) + param->median_length * sizeof(struct mb_cube_queue_entry) + sizeof(int)
                     + 1.5 * sizeof(struct mb_cube_hyp));
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bytes:      %f\n", *bytes);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_cube_init(int verbose, const struct mb_cube_param_struct *param, int nx, int ny, double dx, double dy,
                 void **cube_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       param:      %p\n", param);
    fprintf(stderr, "dbg2       nx:         %d\n", nx);
    fprintf(stderr, "dbg2       ny:         %d\n", ny);
    fprintf(stderr, "dbg2       dx:         %f\n", dx);
    fprintf(stderr, "dbg2       dy:         %f\n", dy);
    fprintf(stderr, "dbg2       cube_ptr:   %p\n", cube_ptr);
  }

  *cube_ptr = NULL;
  int status = mb_cube_check_param(verbose, param, error);
  if (status == MB_SUCCESS && (nx < 1 || ny < 1 || (double)nx * (double)ny > (double)INT_MAX || !(dx > 0.0) || !(dy > 0.0))) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }

  struct mb_cube_struct *cube = NULL;
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mb_cube_struct), (void **)&cube, error);
  if (status == MB_SUCCESS) {
    memset(cube, 0, sizeof(struct mb_cube_struct));
    cube->param = *param;
    cube->nx = nx;
    cube->ny = ny;
    cube->dx = dx;
    cube->dy = dy;
    cube->dist_scale = MIN(dx, dy);
    cube->var_scale = pow(cube->dist_scale, -param->dist_exp);
    cube->min_context = (int)(param->min_context / cube->dist_scale);
    cube->max_context = (int)(param->max_context / cube->dist_scale);
    cube->failed = false;
    status = mb_mallocd(verbose, __FILE__, __LINE__, (size_t)nx * (size_t)ny * sizeof(struct mb_cube_node),
                        (void **)&cube->nodes, error);
    if (status == MB_SUCCESS) {
      for (size_t k = 0; k < (size_t)nx * (size_t)ny; k++) {
        cube->nodes[k].queue = MB_CUBE_NONE;
        cube->nodes[k].n_queued = 0;
        cube->nodes[k].hyp = MB_CUBE_NONE;
        cube->nodes[k].nhyp = 0;
      }
      *cube_ptr = (void *)cube;
    }
    else {
      int tmp_error = MB_ERROR_NO_ERROR;
      mb_freed(verbose, __FILE__, __LINE__, (void **)&cube, &tmp_error);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       cube_ptr:   %p\n", *cube_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Get a free queue slot, growing the pool if needed. */
static int mb_cube_queue_alloc(int verbose, struct mb_cube_struct *cube, int *slot, int *error) {
  int status = MB_SUCCESS;
  if (cube->nqueue_free == 0) {
    const int nalloc = MAX(MB_CUBE_ALLOC_MIN, 2 * cube->nqueue_alloc);
    status = mb_reallocd(verbose, __FILE__, __LINE__,
                         (size_t)nalloc * (size_t)cube->param.median_length * sizeof(struct mb_cube_queue_entry),
                         (void **)&cube->queues, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)nalloc * sizeof(int), (void **)&cube->queue_free, error);
    if (status != MB_SUCCESS) {
      cube->failed = true;
      return (status);
    }
    for (int k = nalloc - 1; k >= cube->nqueue_alloc; k--)
      cube->queue_free[cube->nqueue_free++] = k;
    cube->nqueue_alloc = nalloc;
  }
  *slot = cube->queue_free[--cube->nqueue_free];
  return (status);
}
/*--------------------------------------------------------------------------*/
/* Add a new hypothesis to the front of the node's list. */
static int mb_cube_new_hyp(int verbose, struct mb_cube_struct *cube, struct mb_cube_node *node, float mean, float var,
                           int *error) {
  int status = MB_SUCCESS;
  if (cube->nhyp_used == cube->nhyp_alloc) {
    const int nalloc = MAX(MB_CUBE_ALLOC_MIN, 2 * cube->nhyp_alloc);
    status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)nalloc * sizeof(struct mb_cube_hyp), (void **)&cube->hyps,
                         error);
    if (status != MB_SUCCESS) {
      cube->failed = true;
      return (status);
    }
    cube->nhyp_alloc = nalloc;
  }
  const int ihyp = cube->nhyp_used++;
  struct mb_cube_hyp *hyp = &cube->hyps[ihyp];
  hyp->cur_estimate = mean;
  hyp->pred_estimate = mean;
  hyp->cur_variance = var;
  hyp->pred_variance = var;
  hyp->cum_bayes_fac = 1.0;
  hyp->var_estimate = 0.0f;
  hyp->seq_length = 0;
  hyp->num_samples = 1;
  hyp->next = node->hyp;
  node->hyp = ihyp;
  node->nhyp++;
  return (status);
}
/*--------------------------------------------------------------------------*/
/* Bayes factor monitoring of a hypothesis (West & Harrison, Bayesian
   Forecasting and Dynamic Models, 2nd ed., 1997, sec. 11.5.1). Returns
   true if the sample is consistent with the hypothesis. */
static bool mb_cube_monitor(const struct mb_cube_param_struct *param, struct mb_cube_hyp *hyp, float yn, float vn) {
  const double forecast_var = hyp->pred_variance + vn;
  const double error = (yn - hyp->pred_estimate) / sqrt(forecast_var);
  double bayes_factor;
  if (error >= 0.0)
    bayes_factor = exp(0.5 * (param->est_offset * param->est_offset - 2.0 * param->est_offset * error));
  else
    bayes_factor = exp(0.5 * (param->est_offset * param->est_offset + 2.0 * param->est_offset * error));

  /* single sample outlier */
  if (bayes_factor < param->bayes_fac_t)
    return (false);

  /* cumulative Bayes factor and run length */
  if (hyp->cum_bayes_fac < 1.0)
    hyp->seq_length++;
  else
    hyp->seq_length = 1;
  hyp->cum_bayes_fac = bayes_factor * MIN(1.0, hyp->cum_bayes_fac);
  if (hyp->cum_bayes_fac < param->bayes_fac_t || hyp->seq_length > param->runlength_t)
    return (false);

  return (true);
}
/*--------------------------------------------------------------------------*/
/* Assimilate one sample into the node: update the best matching hypothesis
   or start a new one. */
static int mb_cube_update_node(int verbose, struct mb_cube_struct *cube, struct mb_cube_node *node, float depth, float var,
                               int *error) {
  /* find the hypothesis with the smallest normalized forecast error,
     the newest one on ties */
  struct mb_cube_hyp *best = NULL;
  double min_error = FLT_MAX;
  for (int ihyp = node->hyp; ihyp != MB_CUBE_NONE; ihyp = cube->hyps[ihyp].next) {
    struct mb_cube_hyp *hyp = &cube->hyps[ihyp];
    const double error_n = fabs((depth - hyp->pred_estimate) / sqrt(hyp->pred_variance + var));
    if (error_n < min_error) {
      min_error = error_n;
      best = hyp;
    }
  }

  if (best == NULL)
    return (mb_cube_new_hyp(verbose, cube, node, depth, var, error));

  if (!mb_cube_monitor(&cube->param, best, depth, var)) {
    /* intervention: reset the monitor and start a new hypothesis */
    best->cum_bayes_fac = 1.0;
    best->seq_length = 0;
    return (mb_cube_new_hyp(verbose, cube, node, depth, var, error));
  }

  /* running variance of the samples, from the estimate before the update */
  const int n = best->num_samples;
  best->var_estimate = (float)((n - 1) * best->var_estimate / n
                               + (depth - best->cur_estimate) * (depth - best->cur_estimate) / n);

  /* Kalman update with discounted evolution variance */
  const double sys_variance = best->cur_variance * (1.0 - cube->param.discount) / cube->param.discount;
  const double gain = best->pred_variance / (var + best->pred_variance);
  best->pred_estimate += gain * (depth - best->pred_estimate);
  best->cur_estimate = best->pred_estimate;
  best->cur_variance = var * best->pred_variance / (var + best->pred_variance);
  best->pred_variance = best->cur_variance + sys_variance;
  best->num_samples++;

  return (MB_SUCCESS);
}
/*--------------------------------------------------------------------------*/
/* Remove outliers from a queue sorted from shallow to deep: with N entries,
   n = N - 1, overall sum of squared deviations S and S_k = S - d_k^2 N / n
   the sum of squared deviations of the other entries about their own mean,
   entry k is removed if its quotient d_k^2 (n^2 - 1) / (n S_k) reaches the
   quotient limit. This is the reference quotient, computed in double
   precision about the mean (the single precision sum of squares of the
   reference loses all significant digits at typical depths); if all the
   other entries are identical (S_k = 0) a differing entry is removed.
   Queues shorter than three entries are left as they are. */
static void mb_cube_queue_truncate(const struct mb_cube_param_struct *param, struct mb_cube_queue_entry *queue,
                                   int *n_queued) {
  if (*n_queued < 3)
    return;

  const int nq = *n_queued;
  const int n = nq - 1;
  double mean = 0.0;
  for (int k = 0; k < nq; k++)
    mean += queue[k].depth;
  mean /= nq;
  double ssd = 0.0;
  for (int k = 0; k < nq; k++)
    ssd += (queue[k].depth - mean) * (queue[k].depth - mean);

  int nkeep = 0;
  for (int k = 0; k < nq; k++) {
    const double diff_sq = (queue[k].depth - mean) * (queue[k].depth - mean);
    const double ssd_k = ssd - diff_sq * nq / n;
    double quotient;
    if (ssd_k > 1.0e-12 * ssd)
      quotient = diff_sq * ((double)n * n - 1.0) / (n * ssd_k);
    else
      quotient = diff_sq > 0.0 ? DBL_MAX : 0.0;
    if (quotient < param->quotient_limit)
      queue[nkeep++] = queue[k];
  }
  *n_queued = nkeep;
}
/*--------------------------------------------------------------------------*/
/* Add a sample to the node's median queue. Once the queue is full, each new
   sample pushes the median out to the estimator. */
static int mb_cube_queue_add(int verbose, struct mb_cube_struct *cube, struct mb_cube_node *node, float depth, float var,
                             int *error) {
  const int length = cube->param.median_length;
  if (node->queue == MB_CUBE_NONE) {
    if (mb_cube_queue_alloc(verbose, cube, &node->queue, error) != MB_SUCCESS)
      return (MB_FAILURE);
    node->n_queued = 0;
  }
  struct mb_cube_queue_entry *queue = &cube->queues[(size_t)node->queue * (size_t)length];

  /* fill the queue in sorted order, shallowest first; a new sample goes
     before existing samples of equal depth */
  if (node->n_queued < length) {
    int i = 0;
    while (i < node->n_queued && queue[i].depth < depth)
      i++;
    if (i < node->n_queued)
      memmove(&queue[i + 1], &queue[i], (node->n_queued - i) * sizeof(struct mb_cube_queue_entry));
    queue[i].depth = depth;
    queue[i].var = var;
    node->n_queued++;
    return (MB_SUCCESS);
  }

  /* queue is full: take out the median and insert the new sample */
  const int c = length / 2;
  const struct mb_cube_queue_entry median = queue[c];
  if (depth < median.depth) {
    int i = c - 1;
    while (i >= 0 && depth < queue[i].depth)
      i--;
    if (i != c - 1)
      memmove(&queue[i + 2], &queue[i + 1], (c - i - 1) * sizeof(struct mb_cube_queue_entry));
    queue[i + 1].depth = depth;
    queue[i + 1].var = var;
  }
  else {
    int i = c + 1;
    while (i < length && depth > queue[i].depth)
      i++;
    if (i != c + 1)
      memmove(&queue[c], &queue[c + 1], (i - c - 1) * sizeof(struct mb_cube_queue_entry));
    queue[i - 1].depth = depth;
    queue[i - 1].var = var;
  }

  /* if the 99% bounds of the shallowest and deepest samples do not overlap,
     there may be outliers in the queue */
  if (queue[length - 1].depth - queue[0].depth
      >= MB_CUBE_CONF_99PC * (sqrt(queue[0].var) + sqrt(queue[length - 1].var)))
    mb_cube_queue_truncate(&cube->param, queue, &node->n_queued);

  return (mb_cube_update_node(verbose, cube, node, median.depth, median.var, error));
}
/*--------------------------------------------------------------------------*/
int mb_cube_insert(int verbose, void *cube_ptr, double x, double y, double depth, double depth_sonar, double var_depth,
                   double var_horiz, int *nused, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:    %p\n", cube_ptr);
    fprintf(stderr, "dbg2       x:           %f\n", x);
    fprintf(stderr, "dbg2       y:           %f\n", y);
    fprintf(stderr, "dbg2       depth:       %f\n", depth);
    fprintf(stderr, "dbg2       depth_sonar: %f\n", depth_sonar);
    fprintf(stderr, "dbg2       var_depth:   %f\n", var_depth);
    fprintf(stderr, "dbg2       var_horiz:   %f\n", var_horiz);
  }

  struct mb_cube_struct *cube = (struct mb_cube_struct *)cube_ptr;
  const struct mb_cube_param_struct *param = &cube->param;
  int status = MB_SUCCESS;
  *nused = 0;
  *error = MB_ERROR_NO_ERROR;

  if (cube->failed) {
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }
  else if (!isfinite(x) || !isfinite(y) || !isfinite(depth) || !isfinite(depth_sonar) || !(var_depth > 0.0)
           || !isfinite(var_depth) || !(var_horiz >= 0.0) || !isfinite(var_horiz)) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }

  if (status == MB_SUCCESS) {
    const double h = fabs(depth_sonar);

    /* influence radius: soundings much better than the allowed TVU spread
       further, but never beyond the 99% bound of their horizontal
       uncertainty or below the node spacing */
    const double max_var = (param->tvu_a * param->tvu_a + param->tvu_b * param->tvu_b * h * h)
                           / (MB_CUBE_CONF_95PC * MB_CUBE_CONF_95PC);
    double ratio = max_var / var_depth;
    if (ratio <= 2.0)
      ratio = 2.0;
    const double max_radius = MB_CUBE_CONF_99PC * sqrt(var_horiz);
    double radius = cube->dist_scale * pow(ratio - 1.0, 1.0 / param->dist_exp) - max_radius;
    if (radius < 0.0)
      radius = cube->dist_scale;
    if (radius > max_radius)
      radius = max_radius;
    if (radius < cube->dist_scale)
      radius = cube->dist_scale;
    const double radius_sq = radius * radius;
    const double capture = MAX(param->capture_scale * h, param->capture_min);

    /* nodes within the radius, clipped to the grid */
    const double fimin = ceil((x - radius) / cube->dx);
    const double fimax = floor((x + radius) / cube->dx);
    const double fjmin = ceil((y - radius) / cube->dy);
    const double fjmax = floor((y + radius) / cube->dy);
    if (fimax >= 0.0 && fimin <= cube->nx - 1 && fjmax >= 0.0 && fjmin <= cube->ny - 1) {
      const int imin = (int)MAX(fimin, 0.0);
      const int imax = (int)MIN(fimax, cube->nx - 1.0);
      const int jmin = (int)MAX(fjmin, 0.0);
      const int jmax = (int)MIN(fjmax, cube->ny - 1.0);
      for (int i = imin; i <= imax && status == MB_SUCCESS; i++) {
        for (int j = jmin; j <= jmax && status == MB_SUCCESS; j++) {
          const double ddx = i * cube->dx - x;
          const double ddy = j * cube->dy - y;
          const double dist_sq = ddx * ddx + ddy * ddy;
          if (dist_sq >= radius_sq)
            continue;
          double dist = sqrt(dist_sq);
          if (dist > capture)
            continue;

          /* propagate the vertical variance to the node */
          dist += param->horiz_scale * sqrt(var_horiz);
          const double variance = var_depth * (1.0 + cube->var_scale * pow(dist, param->dist_exp));
          status = mb_cube_queue_add(verbose, cube, &cube->nodes[(size_t)i * cube->ny + j], (float)depth, (float)variance,
                                     error);
          if (status == MB_SUCCESS)
            (*nused)++;
        }
      }
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       nused:      %d\n", *nused);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Feed all queued samples to the estimators, from the median outwards
   (shallower side first when the count is even), and release the queues.
   Flushing and then adding more data does not give the same result as
   adding all data and then flushing. */
int mb_cube_flush(int verbose, void *cube_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:   %p\n", cube_ptr);
  }

  struct mb_cube_struct *cube = (struct mb_cube_struct *)cube_ptr;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  if (cube->failed) {
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }

  const size_t nnodes = (size_t)cube->nx * (size_t)cube->ny;
  for (size_t k = 0; k < nnodes && status == MB_SUCCESS; k++) {
    struct mb_cube_node *node = &cube->nodes[k];
    if (node->queue == MB_CUBE_NONE)
      continue;
    struct mb_cube_queue_entry *queue = &cube->queues[(size_t)node->queue * (size_t)cube->param.median_length];
    mb_cube_queue_truncate(&cube->param, queue, &node->n_queued);
    int ex_pt;
    int direction;
    if (node->n_queued % 2 == 0) {
      ex_pt = node->n_queued / 2 - 1;
      direction = 1;
    }
    else {
      ex_pt = node->n_queued / 2;
      direction = -1;
    }
    for (int scale = 1; ex_pt >= 0 && status == MB_SUCCESS; scale++) {
      status = mb_cube_update_node(verbose, cube, node, queue[ex_pt].depth, queue[ex_pt].var, error);
      ex_pt += direction * scale;
      direction = -direction;
    }
    cube->queue_free[cube->nqueue_free++] = node->queue;
    node->queue = MB_CUBE_NONE;
    node->n_queued = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Variance reported for a hypothesis. */
static double mb_cube_hyp_variance(const struct mb_cube_param_struct *param, const struct mb_cube_hyp *hyp) {
  if (param->uncertainty_mode == MB_CUBE_UNCT_POSTERIOR)
    return (hyp->cur_variance);
  else if (param->uncertainty_mode == MB_CUBE_UNCT_MAX)
    return (MAX(hyp->var_estimate, hyp->cur_variance));
  return (hyp->var_estimate);
}
/*--------------------------------------------------------------------------*/
/* Hypothesis with the most samples (the newest on ties) and the strength
   5 - n_best / n_second, at least 0. */
static const struct mb_cube_hyp *mb_cube_select_prior(const struct mb_cube_struct *cube, const struct mb_cube_node *node,
                                                      double *strength) {
  const struct mb_cube_hyp *best = NULL;
  for (int ihyp = node->hyp; ihyp != MB_CUBE_NONE; ihyp = cube->hyps[ihyp].next)
    if (best == NULL || cube->hyps[ihyp].num_samples > best->num_samples)
      best = &cube->hyps[ihyp];
  int n_second = 0;
  for (int ihyp = node->hyp; ihyp != MB_CUBE_NONE; ihyp = cube->hyps[ihyp].next)
    if (&cube->hyps[ihyp] != best && cube->hyps[ihyp].num_samples > n_second)
      n_second = cube->hyps[ihyp].num_samples;
  *strength = 0.0;
  if (n_second > 0)
    *strength = MAX(0.0, MB_CUBE_MAX_HYPOTHESIS_RATIO - (double)best->num_samples / n_second);
  return (best);
}
/*--------------------------------------------------------------------------*/
/* Nearest node with exactly one hypothesis in the square rings from
   min_context to max_context nodes around node (i,j); on equal distance
   the last one found in the scan wins. Returns NULL if there is none. */
static const struct mb_cube_hyp *mb_cube_find_guide(const struct mb_cube_struct *cube, int i, int j) {
  const struct mb_cube_hyp *guide = NULL;
  for (int offset = cube->min_context; offset <= cube->max_context && guide == NULL; offset++) {
    double dist_min = DBL_MAX;
    for (int ii = i - offset; ii <= i + offset; ii++) {
      if (ii < 0 || ii >= cube->nx)
        continue;
      /* rows j +/- offset in full, columns i +/- offset between them */
      const int jstep = (ii == i - offset || ii == i + offset) ? 1 : MAX(1, 2 * offset);
      for (int jj = j - offset; jj <= j + offset; jj += jstep) {
        if (jj < 0 || jj >= cube->ny)
          continue;
        const struct mb_cube_node *node = &cube->nodes[(size_t)ii * cube->ny + jj];
        if (node->nhyp != 1)
          continue;
        const double ddx = (ii - i) * cube->dx;
        const double ddy = (jj - j) * cube->dy;
        const double dist = ddx * ddx + ddy * ddy;
        if (dist <= dist_min) {
          dist_min = dist;
          guide = &cube->hyps[node->hyp];
        }
      }
    }
  }
  return (guide);
}
/*--------------------------------------------------------------------------*/
int mb_cube_extract(int verbose, void *cube_ptr, float nodata, float *depth, float *uncertainty, float *nhyp,
                    float *strength, int *nsamples, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:    %p\n", cube_ptr);
    fprintf(stderr, "dbg2       nodata:      %f\n", nodata);
    fprintf(stderr, "dbg2       depth:       %p\n", depth);
    fprintf(stderr, "dbg2       uncertainty: %p\n", uncertainty);
    fprintf(stderr, "dbg2       nhyp:        %p\n", nhyp);
    fprintf(stderr, "dbg2       strength:    %p\n", strength);
    fprintf(stderr, "dbg2       nsamples:    %p\n", nsamples);
  }

  const struct mb_cube_struct *cube = (const struct mb_cube_struct *)cube_ptr;
  const struct mb_cube_param_struct *param = &cube->param;
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  if (cube->failed) {
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }

  for (int i = 0; i < cube->nx && status == MB_SUCCESS; i++) {
    for (int j = 0; j < cube->ny; j++) {
      const size_t k = (size_t)i * cube->ny + j;
      const struct mb_cube_node *node = &cube->nodes[k];
      const struct mb_cube_hyp *selected = NULL;
      double strength_k = 0.0;

      if (node->nhyp == 1) {
        selected = &cube->hyps[node->hyp];
      }
      else if (node->nhyp > 1) {
        const struct mb_cube_hyp *guide = NULL;
        if (param->extractor != MB_CUBE_EXTRACT_PRIOR)
          guide = mb_cube_find_guide(cube, i, j);
        if (guide == NULL) {
          selected = mb_cube_select_prior(cube, node, &strength_k);
        }
        else {
          /* closest hypothesis to the guide depth, weighted by the
             number of samples for the posterior rule; the newest wins ties */
          const double guide_depth = guide->cur_estimate;
          const double guide_var = MAX(guide->var_estimate, guide->cur_variance);
          double best_score = -DBL_MAX;
          int n_total = 0;
          for (int ihyp = node->hyp; ihyp != MB_CUBE_NONE; ihyp = cube->hyps[ihyp].next) {
            const struct mb_cube_hyp *hyp = &cube->hyps[ihyp];
            const double diff = hyp->cur_estimate - guide_depth;
            double score;
            if (param->extractor == MB_CUBE_EXTRACT_POSTERIOR)
              score = -diff * diff / (2.0 * guide_var) + log((double)hyp->num_samples);
            else
              score = -fabs(diff) / sqrt(guide_var);
            if (score > best_score) {
              best_score = score;
              selected = hyp;
            }
            n_total += hyp->num_samples;
          }
          strength_k = MAX(0.0, MB_CUBE_MAX_HYPOTHESIS_RATIO
                                     - (double)selected->num_samples / (n_total - selected->num_samples));
        }
      }

      if (selected != NULL) {
        if (depth != NULL)
          depth[k] = (float)selected->cur_estimate;
        if (uncertainty != NULL)
          uncertainty[k] = (float)(param->sd2conf * sqrt(mb_cube_hyp_variance(param, selected)));
        if (strength != NULL)
          strength[k] = (float)strength_k;
        if (nsamples != NULL)
          nsamples[k] = selected->num_samples;
      }
      else {
        if (depth != NULL)
          depth[k] = nodata;
        if (uncertainty != NULL)
          uncertainty[k] = nodata;
        if (strength != NULL)
          strength[k] = nodata;
        if (nsamples != NULL)
          nsamples[k] = 0;
      }
      if (nhyp != NULL)
        nhyp[k] = (float)node->nhyp;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/* Return up to nhyp_max hypotheses of node (i,j), newest first; nhyp is
   set to the number of hypotheses at the node. */
int mb_cube_node_hypotheses(int verbose, void *cube_ptr, int i, int j, int nhyp_max, int *nhyp, double *depth,
                            double *uncertainty, int *nsamples, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:   %p\n", cube_ptr);
    fprintf(stderr, "dbg2       i:          %d\n", i);
    fprintf(stderr, "dbg2       j:          %d\n", j);
    fprintf(stderr, "dbg2       nhyp_max:   %d\n", nhyp_max);
  }

  const struct mb_cube_struct *cube = (const struct mb_cube_struct *)cube_ptr;
  int status = MB_SUCCESS;
  *nhyp = 0;
  *error = MB_ERROR_NO_ERROR;
  if (cube->failed) {
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }
  else if (i < 0 || i >= cube->nx || j < 0 || j >= cube->ny) {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_PARAMETER;
  }
  else {
    const struct mb_cube_node *node = &cube->nodes[(size_t)i * cube->ny + j];
    *nhyp = node->nhyp;
    int n = 0;
    for (int ihyp = node->hyp; ihyp != MB_CUBE_NONE && n < nhyp_max; ihyp = cube->hyps[ihyp].next, n++) {
      const struct mb_cube_hyp *hyp = &cube->hyps[ihyp];
      depth[n] = hyp->cur_estimate;
      uncertainty[n] = cube->param.sd2conf * sqrt(mb_cube_hyp_variance(&cube->param, hyp));
      nsamples[n] = hyp->num_samples;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       nhyp:       %d\n", *nhyp);
    for (int n = 0; n < MIN(*nhyp, nhyp_max); n++)
      fprintf(stderr, "dbg2       hypothesis %d: depth:%f uncertainty:%f nsamples:%d\n", n, depth[n], uncertainty[n],
              nsamples[n]);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_cube_deall(int verbose, void **cube_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       cube_ptr:   %p\n", *cube_ptr);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  struct mb_cube_struct *cube = (struct mb_cube_struct *)*cube_ptr;
  if (cube != NULL) {
    if (cube->nodes != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&cube->nodes, error);
    if (cube->queues != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&cube->queues, error);
    if (cube->queue_free != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&cube->queue_free, error);
    if (cube->hyps != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&cube->hyps, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, cube_ptr, error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
