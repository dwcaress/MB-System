/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_core.h
 *
 *    Copyright (c) 2000-2026 by
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
 * mbnavadjust_core.h holds the mbnavadjust global control parameters and
 * the prototypes of the crossing-analysis/autopick functions that are
 * shared, GUI-free, between the mbnavadjust GUI and the mbnavadjustmerge
 * CLI. Exactly one translation unit (mbnavadjust_autopick.c) defines
 * MBNAVADJUST_CORE_DECLARE_GLOBALS before including this file, giving
 * these variables real storage; every other file gets them as extern.
 *
 * Author:	D. W. Caress
 * Date:	July 23, 2026
 */

/*--------------------------------------------------------------------*/

#ifndef MBNAVADJUST_CORE_H_
#define MBNAVADJUST_CORE_H_

#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

#ifndef MB_YES
#include "mb_status.h"
#endif

#include "mbnavadjust_io.h"

#ifdef MBNAVADJUST_CORE_DECLARE_GLOBALS
#define MBNAVADJUST_CORE_EXTERNAL
#else
#define MBNAVADJUST_CORE_EXTERNAL extern
#endif

/* swath bathymetry raw data structures - shared between the crossing-load
    logic (mbnavadjust_autopick.c) and the GUI's naverr plotting code
    (mbnavadjust_prog.c), which draws the raw pings crossing_load loads */
struct pingraw {
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double draft;
  double beams_bath;
  char *beamflag;
  double *bath;
  double *bathacrosstrack;
  double *bathalongtrack;
};

struct swathraw {
  /* raw swath data */
  int file_id;
  int npings;
  int npings_max;
  int beams_bath;
  struct pingraw *pingraws;
};

/* mb_contour / misfit grid parameters - shared for the same reason: computed
    by mbnavadjust_get_misfit()/mbnavadjust_crossing_load() (in
    mbnavadjust_autopick.c) and displayed by the GUI's naverr plotting code
    (in mbnavadjust_prog.c) */
MBNAVADJUST_CORE_EXTERNAL struct swathraw *swathraw1;
MBNAVADJUST_CORE_EXTERNAL struct swathraw *swathraw2;
MBNAVADJUST_CORE_EXTERNAL struct swath *swath1;
MBNAVADJUST_CORE_EXTERNAL struct swath *swath2;
MBNAVADJUST_CORE_EXTERNAL int grid_nx;
MBNAVADJUST_CORE_EXTERNAL int grid_ny;
MBNAVADJUST_CORE_EXTERNAL int grid_nxy;
MBNAVADJUST_CORE_EXTERNAL int grid_nxyzeq;
MBNAVADJUST_CORE_EXTERNAL double grid_dx;
MBNAVADJUST_CORE_EXTERNAL double grid_dy;
MBNAVADJUST_CORE_EXTERNAL double grid_olon;
MBNAVADJUST_CORE_EXTERNAL double grid_olat;
MBNAVADJUST_CORE_EXTERNAL double misfit_min;
MBNAVADJUST_CORE_EXTERNAL double misfit_max;
MBNAVADJUST_CORE_EXTERNAL int gridm_nx;
MBNAVADJUST_CORE_EXTERNAL int gridm_ny;
MBNAVADJUST_CORE_EXTERNAL int gridm_nxyz;
MBNAVADJUST_CORE_EXTERNAL double *grid1;
MBNAVADJUST_CORE_EXTERNAL double *grid2;
MBNAVADJUST_CORE_EXTERNAL double *gridm;
MBNAVADJUST_CORE_EXTERNAL double *gridmeq;
MBNAVADJUST_CORE_EXTERNAL int *gridn1;
MBNAVADJUST_CORE_EXTERNAL int *gridn2;
MBNAVADJUST_CORE_EXTERNAL int *gridnm;
#define MBNA_NINTERVALS_MISFIT 80
MBNAVADJUST_CORE_EXTERNAL int nmisfit_intervals;
MBNAVADJUST_CORE_EXTERNAL double misfit_intervals[MBNA_NINTERVALS_MISFIT];
MBNAVADJUST_CORE_EXTERNAL int nzmisfitcalc;
MBNAVADJUST_CORE_EXTERNAL double zoff_dz;
MBNAVADJUST_CORE_EXTERNAL double zmin;
MBNAVADJUST_CORE_EXTERNAL double zmax;

/* plot-canvas pixel bounds for the contour/misfit windows - set from real
    Motif widget geometry by mbnavadjust_set_borders() (mbnavadjust_prog.c)
    for the GUI. mbnavadjust_autopick() initializes these to a placeholder
    square if they are still unset (all zero, as in the CLI, which never
    calls mbnavadjust_set_borders) - see the comment there for why any
    non-zero, self-consistent aspect ratio works. */
MBNAVADJUST_CORE_EXTERNAL int cont_borders[4];
MBNAVADJUST_CORE_EXTERNAL int corr_borders[4];

/* mbnavadjust global control parameters */
MBNAVADJUST_CORE_EXTERNAL int mbna_verbose;
MBNAVADJUST_CORE_EXTERNAL int mbna_status;
MBNAVADJUST_CORE_EXTERNAL int mbna_view_list;
MBNAVADJUST_CORE_EXTERNAL int mbna_view_mode;
MBNAVADJUST_CORE_EXTERNAL int mbna_invert_mode;
MBNAVADJUST_CORE_EXTERNAL int mbna_save_frequency;
MBNAVADJUST_CORE_EXTERNAL int mbna_color_foreground;
MBNAVADJUST_CORE_EXTERNAL int mbna_color_background;
MBNAVADJUST_CORE_EXTERNAL int mbna_survey_select;
MBNAVADJUST_CORE_EXTERNAL int mbna_survey_select1;
MBNAVADJUST_CORE_EXTERNAL int mbna_survey_select2;
MBNAVADJUST_CORE_EXTERNAL int mbna_file_select;
MBNAVADJUST_CORE_EXTERNAL int mbna_section_select;
MBNAVADJUST_CORE_EXTERNAL int mbna_crossing_select;
MBNAVADJUST_CORE_EXTERNAL int mbna_tie_select;
MBNAVADJUST_CORE_EXTERNAL int mbna_current_crossing;
MBNAVADJUST_CORE_EXTERNAL int mbna_current_tie;
MBNAVADJUST_CORE_EXTERNAL int mbna_current_file;
MBNAVADJUST_CORE_EXTERNAL int mbna_current_section;
MBNAVADJUST_CORE_EXTERNAL int mbna_naverr_mode;
MBNAVADJUST_CORE_EXTERNAL int mbna_file_id_1;
MBNAVADJUST_CORE_EXTERNAL int mbna_section_1;
MBNAVADJUST_CORE_EXTERNAL int mbna_file_id_2;
MBNAVADJUST_CORE_EXTERNAL int mbna_section_2;
MBNAVADJUST_CORE_EXTERNAL int mbna_snav_1;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_1_time_d;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_1_lon;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_1_lat;
MBNAVADJUST_CORE_EXTERNAL int mbna_snav_2;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_2_time_d;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_2_lon;
MBNAVADJUST_CORE_EXTERNAL double mbna_snav_2_lat;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_x;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_y;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_z;
MBNAVADJUST_CORE_EXTERNAL double mbna_invert_offset_x;
MBNAVADJUST_CORE_EXTERNAL double mbna_invert_offset_y;
MBNAVADJUST_CORE_EXTERNAL double mbna_invert_offset_z;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_x_old;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_y_old;
MBNAVADJUST_CORE_EXTERNAL double mbna_offset_z_old;
MBNAVADJUST_CORE_EXTERNAL double mbna_lon_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_lon_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_lat_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_lat_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_mtodeglon;
MBNAVADJUST_CORE_EXTERNAL double mbna_mtodeglat;
MBNAVADJUST_CORE_EXTERNAL double mbna_ox;
MBNAVADJUST_CORE_EXTERNAL double mbna_oy;
MBNAVADJUST_CORE_EXTERNAL int mbna_bin_beams_bath;
MBNAVADJUST_CORE_EXTERNAL double mbna_bin_swathwidth;
MBNAVADJUST_CORE_EXTERNAL double mbna_bin_pseudobeamwidth;
MBNAVADJUST_CORE_EXTERNAL double mbna_plot_lon_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_plot_lon_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_plot_lat_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_plot_lat_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_overlap_lon_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_overlap_lon_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_overlap_lat_min;
MBNAVADJUST_CORE_EXTERNAL double mbna_overlap_lat_max;
MBNAVADJUST_CORE_EXTERNAL double mbna_plotx_scale;
MBNAVADJUST_CORE_EXTERNAL double mbna_ploty_scale;
MBNAVADJUST_CORE_EXTERNAL int mbna_misfit_center;
MBNAVADJUST_CORE_EXTERNAL double mbna_misfit_xscale;
MBNAVADJUST_CORE_EXTERNAL double mbna_misfit_yscale;
MBNAVADJUST_CORE_EXTERNAL double mbna_misfit_offset_x;
MBNAVADJUST_CORE_EXTERNAL double mbna_misfit_offset_y;
MBNAVADJUST_CORE_EXTERNAL double mbna_misfit_offset_z;
MBNAVADJUST_CORE_EXTERNAL int mbna_minmisfit_nthreshold;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit;
MBNAVADJUST_CORE_EXTERNAL int mbna_minmisfit_n;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_x;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_y;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_z;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_xh;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_yh;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_zh;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sr1;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sx1[4];
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sr2;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sx2[4];
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sr3;
MBNAVADJUST_CORE_EXTERNAL double mbna_minmisfit_sx3[4];
MBNAVADJUST_CORE_EXTERNAL double mbna_zoff_scale_x;
MBNAVADJUST_CORE_EXTERNAL double mbna_zoff_scale_y;

MBNAVADJUST_CORE_EXTERNAL int mbna_zoom_x1;
MBNAVADJUST_CORE_EXTERNAL int mbna_zoom_y1;
MBNAVADJUST_CORE_EXTERNAL int mbna_zoom_x2;
MBNAVADJUST_CORE_EXTERNAL int mbna_zoom_y2;
MBNAVADJUST_CORE_EXTERNAL double mbna_smoothweight;
MBNAVADJUST_CORE_EXTERNAL double mbna_offsetweight;
MBNAVADJUST_CORE_EXTERNAL double mbna_zweightfactor;
MBNAVADJUST_CORE_EXTERNAL double mbna_global_tie_influence;
MBNAVADJUST_CORE_EXTERNAL int mbna_bias_mode;
MBNAVADJUST_CORE_EXTERNAL int mbna_allow_set_tie;
MBNAVADJUST_CORE_EXTERNAL int mbna_allow_add_tie;

/* plot vector data */
MBNAVADJUST_CORE_EXTERNAL struct mbna_contour_vector *mbna_contour;
MBNAVADJUST_CORE_EXTERNAL struct mbna_contour_vector mbna_contour1;
MBNAVADJUST_CORE_EXTERNAL struct mbna_contour_vector mbna_contour2;

/* model plot parameters */
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_mode;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_width;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_height;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_count;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_start;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_end;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_xo;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_yo_lon;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_yo_lat;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_yo_z;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_yxmid;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_yymid;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_yzmid;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_xscale;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_yscale;
MBNAVADJUST_CORE_EXTERNAL double mbna_modelplot_yzscale;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_zoom_x1;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_zoom_x2;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_zoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_startzoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_endzoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_num_ties_plot;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_tiestart;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_tieend;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_tiezoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_tiestartzoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_tieendzoom;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_pickfile;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_picksection;
MBNAVADJUST_CORE_EXTERNAL int mbna_modelplot_picksnav;

/* mbnavadjust global project parameters */
MBNAVADJUST_CORE_EXTERNAL struct mbna_project project;

/* flag to reset all crossings to unanalyzed when a project is opened */
MBNAVADJUST_CORE_EXTERNAL int mbna_reset_crossings;

/* GUI-free crossing-analysis / autopick functions, shared by mbnavadjust
    and mbnavadjustmerge, implemented in mbnavadjust_autopick.c */
int mbnavadjust_naverr_snavpoints(int ix, int iy);
int mbnavadjust_naverr_addtie(void);
int mbnavadjust_crossing_load(void);
int mbnavadjust_crossing_unload(void);
int mbnavadjust_crossing_replot(void);
int mbnavadjust_referenceplussection_unload(void);
int mbnavadjust_get_misfit(void);
int mbnavadjust_get_misfitxy(void);
void mbnavadjust_naverr_scale(void);
int mbnavadjust_autopick(int verbose, struct mbna_project *project_ptr, int crossing_type, int scope_mode,
                         int survey_select, int survey_select1, int survey_select2,
                         int file_select, int section_select,
                         double overlap_threshold, bool do_vertical, int *error);

/* GUI-free network-adjustment navigation solver, reference grid
    regeneration, and corrected-navigation output, shared by mbnavadjust
    and mbnavadjustmerge, implemented in mbnavadjust_invertnav.c */
int mbnavadjust_invertnav(int verbose, struct mbna_project *project_ptr);
int mbnavadjust_updategrid(int verbose, struct mbna_project *project_ptr);
int mbnavadjust_applynav(int verbose, struct mbna_project *project_ptr);

#endif /* MBNAVADJUST_CORE_H_ */
/*--------------------------------------------------------------------*/
