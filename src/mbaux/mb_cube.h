/*--------------------------------------------------------------------
 *    The MB-system:  mb_cube.h  10/26/2026
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
 * mb_cube.h defines the parameters and functions of the CUBE (Combined
 * Uncertainty and Bathymetric Estimator) gridding algorithm implemented
 * in mb_cube.c.
 *
 * Conventions: depths are positive down and in meters, variances are
 * one sigma and in square meters, and node positions are in meters
 * from node (0,0) with x positive east and y positive north. Node (i,j)
 * is located at (i * dx, j * dy) and is stored at index i * ny + j, the
 * same layout as the grids of mbgrid.
 *
 * Author:  C. dos Santos Ferreira
 * Date:  October 26, 2026
 */

#ifndef MB_CUBE_H_
#define MB_CUBE_H_

/* hypothesis selection (disambiguation) rules */
#define MB_CUBE_EXTRACT_PRIOR 0      /* hypothesis with the most samples */
#define MB_CUBE_EXTRACT_LIKELIHOOD 1 /* closest to a nearby single hypothesis node */
#define MB_CUBE_EXTRACT_POSTERIOR 2  /* likelihood weighted by number of samples */
#define MB_CUBE_EXTRACT_PREDICTED 3  /* closest to the predicted surface */

/* variance reported as the uncertainty of a hypothesis */
#define MB_CUBE_UNCT_SAMPLE 0    /* running variance of the assimilated samples */
#define MB_CUBE_UNCT_POSTERIOR 1 /* posterior variance of the estimate */
#define MB_CUBE_UNCT_MAX 2       /* larger of the two */

/* IHO S-44 (Edition 6.1.0, Table 1) survey orders */
#define MB_CUBE_IHO_EXCLUSIVE 0
#define MB_CUBE_IHO_SPECIAL 1
#define MB_CUBE_IHO_ORDER_1A 2
#define MB_CUBE_IHO_ORDER_1B 3
#define MB_CUBE_IHO_ORDER_2 4

/* IHO S-44 editions for mb_cube_iho_edition_limits(), whose survey orders
   are given by name: edition 4 special|1|2|3, edition 5 special|1a|1b|2,
   edition 6 exclusive|special|1a|1b|2, CCOM special|1|2|3 */
#define MB_CUBE_IHO_CCOM 0      /* table of the CCOM/JHC CUBE errmod_iho */
#define MB_CUBE_IHO_EDITION_4 4 /* 4th edition, April 1998 */
#define MB_CUBE_IHO_EDITION_5 5 /* 5th edition, February 2008 */
#define MB_CUBE_IHO_EDITION_6 6 /* Editions 6.0.0 (2020) and 6.1.0 (2022) */

/* CUBE algorithm parameters */
struct mb_cube_param_struct {
  int extractor;         /* MB_CUBE_EXTRACT_PRIOR (default), _LIKELIHOOD, _POSTERIOR or _PREDICTED */
  int uncertainty_mode;  /* MB_CUBE_UNCT_SAMPLE (default), _POSTERIOR or _MAX */
  int median_length;     /* length of median pre-filter queue, odd, 3 - 101 */
  double quotient_limit; /* outlier quotient limit of the queue, 0.1 - 255 */
  double discount;       /* discount factor for evolution noise variance, 0.8 - 1 */
  double est_offset;     /* significant offset of the estimate (normalized), 0.1 - 10 */
  double bayes_fac_t;    /* Bayes factor threshold for an intervention, 0.001 - 10 */
  int runlength_t;       /* run length threshold for an intervention, 1 - 10 */
  double dist_exp;       /* exponent on distance for variance scaling, 1 - 10 */
  double horiz_scale;    /* scale of horizontal sigma added to the distance, 0 - 10 */
  double influence_scale; /* scale of horizontal sigma limiting the influence radius, 0 - 10 */
  double capture_scale;  /* capture distance as a fraction of depth, 0 - 1 */
  double capture_min;    /* minimum capture distance (m), 0 - 100 */
  double min_context;    /* minimum search distance for a guide node (m) */
  double max_context;    /* maximum search distance for a guide node (m) */
  double tvu_a;          /* 95% TVU fixed term (m) bounding the influence radius */
  double tvu_b;          /* 95% TVU depth fraction bounding the influence radius */
  double sd2conf;        /* scale applied to reported sigma (1.96 = 95%) */
  int blunder;             /* reject deep blunders below a predicted surface (0 or 1) */
  double blunder_min;      /* minimum blunder margin below the predicted depth (m), 1 - 1000 */
  double blunder_fraction; /* blunder margin as a fraction of depth, 0 - 1 */
  double blunder_scalar;   /* blunder margin in predicted surface sigmas, 0 - 10 */
};

/* sounding uncertainty models of the programs using CUBE */
#define MB_CUBE_MODEL_IHO 0      /* TVU and THU of the selected IHO S-44 edition and order */
#define MB_CUBE_MODEL_CONSTANT 1 /* constant vertical and horizontal sigmas */
#define MB_CUBE_MODEL_DEPTH 2    /* sigmas growing with depth */

/* predicted surfaces */
#define MB_CUBE_PREDICTION_NONE 0   /* no predicted surface */
#define MB_CUBE_PREDICTION_MEDIAN 1 /* median of the soundings nearest to each node */
#define MB_CUBE_PREDICTION_FILE 2   /* grid file */

#define MB_CUBE_PATH_MAXLINE 1024

/* options shared by the programs using CUBE (mbgrid -F9, mbclean), set by
   the --cube-* command line options; see mb_cube_options.c */
struct mb_cube_options_struct {
  struct mb_cube_param_struct param; /* algorithm parameters */
  int iho_edition;                   /* IHO S-44 edition (MB_CUBE_IHO_EDITION_*) of the order limits */
  int iho_edition_set;               /* edition given with --cube-iho-edition */
  int iho_ccom;                      /* CCOM/JHC errmod_iho limits instead of an edition */
  char iho_order[16];                /* IHO S-44 order of the iho model and influence radius */
  int model;                         /* sounding uncertainty model (MB_CUBE_MODEL_*) */
  double sigma_v;                    /* constant model vertical sigma (m) */
  double sigma_h;                    /* constant and depth models horizontal sigma (m) */
  double model_a;                    /* depth model fixed vertical sigma (m) */
  double model_b;                    /* depth model vertical sigma fraction of depth */
  double model_c;                    /* depth model horizontal sigma fraction of depth */
  int depth_below_sonar;             /* depth below the sonar (1) or the sea surface (0) */
  int prediction;                    /* predicted surface (MB_CUBE_PREDICTION_*) */
  int prediction_factor;             /* median surface spacing in node spacings */
  char prediction_file[MB_CUBE_PATH_MAXLINE]; /* predicted surface grid file */
  double prediction_pct;             /* predicted surface 95% uncertainty, percent of depth */
  int prediction_hypotheses;         /* initial hypotheses at the predicted depths */
  double memory_limit;               /* maximum memory for the CUBE grid (GB) */
  double thu_fixed;                  /* IHO THU fixed term (m), set by mb_cube_resolve_options */
  double thu_fraction;               /* IHO THU depth fraction, set by mb_cube_resolve_options */
  char iho_label[64];                /* IHO limits description, set by mb_cube_resolve_options */
};

#ifdef __cplusplus
extern "C" {
#endif

int mb_cube_iho_limits(int verbose, int iho_order, double *tvu_a, double *tvu_b, double *thu_fixed, double *thu_fraction,
                       int *error);
int mb_cube_iho_edition_limits(int verbose, int edition, const char *order, double *tvu_a, double *tvu_b,
                               double *thu_fixed, double *thu_fraction, int *error);
int mb_cube_iho_edition_orders(int verbose, int edition, char *orders, int size, int *error);
int mb_cube_default_param(int verbose, int iho_order, struct mb_cube_param_struct *param, int *error);
int mb_cube_check_param(int verbose, const struct mb_cube_param_struct *param, int *error);
int mb_cube_estimate_memory(int verbose, const struct mb_cube_param_struct *param, int nx, int ny, double *bytes,
                            int *error);
int mb_cube_init(int verbose, const struct mb_cube_param_struct *param, int nx, int ny, double dx, double dy,
                 void **cube_ptr, int *error);
int mb_cube_set_prediction(int verbose, void *cube_ptr, const float *depth, const float *variance, float nodata,
                           double sonar_fraction, int initial_hypotheses, int *error);
int mb_cube_insert(int verbose, void *cube_ptr, double x, double y, double depth, double depth_sonar, double var_depth,
                   double var_horiz, int *nused, int *error);
int mb_cube_prediction_sign(int verbose, void *cube_ptr, double *nsame, double *nopposite, int *error);
int mb_cube_flush(int verbose, void *cube_ptr, int *error);
int mb_cube_extract(int verbose, void *cube_ptr, float nodata, float *depth, float *uncertainty, float *nhyp,
                    float *strength, int *nsamples, int *error);
int mb_cube_node_hypotheses(int verbose, void *cube_ptr, int i, int j, int nhyp_max, int *nhyp, double *depth,
                            double *uncertainty, int *nsamples, int *error);
int mb_cube_classify(int verbose, void *cube_ptr, double x, double y, double depth, double depth_sonar,
                     double var_depth, double var_horiz, int *node, int *nhyp, int *is_selected, int *is_blunder,
                     double *error_selected, double *strength, int *error);
int mb_cube_deall(int verbose, void **cube_ptr, int *error);

/* mb_cube_options.c */
int mb_cube_default_options(int verbose, struct mb_cube_options_struct *options, int *error);
int mb_cube_parse_option(int verbose, const char *name, const char *value, struct mb_cube_options_struct *options,
                         int *error);
int mb_cube_resolve_options(int verbose, struct mb_cube_options_struct *options, char *message, int size, int *error);
int mb_cube_check_options(int verbose, const char *program_name, const struct mb_cube_options_struct *options,
                          char *message, int size, int *error);
int mb_cube_sounding_variance(int verbose, const struct mb_cube_options_struct *options, double h, double *var_depth,
                              double *var_horiz, int *error);
int mb_cube_sample_grid(int verbose, const float *grid, int grid_nx, int grid_ny, double grid_x0, double grid_y0,
                        double grid_dx, double grid_dy, float grid_nodata, double sign, int nx, int ny, double x0,
                        double y0, double dx, double dy, float nodata, float *depth, int *error);
int mb_cube_apply_prediction(int verbose, void *cube_ptr, const struct mb_cube_options_struct *options, int nx, int ny,
                             const float *depth, float nodata, int *npred, int *error);
int mb_cube_median_init(int verbose, int nx, int ny, double dx, double dy, int factor, double max_bytes,
                        void **median_ptr, int *error);
int mb_cube_median_add(int verbose, void *median_ptr, double x, double y, double depth, double *bytes, int *error);
int mb_cube_median_surface(int verbose, void *median_ptr, float nodata, float *depth, int *nused, int *error);
int mb_cube_median_deall(int verbose, void **median_ptr, int *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MB_CUBE_H_ */
