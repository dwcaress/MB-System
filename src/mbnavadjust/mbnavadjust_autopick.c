/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_autopick.c
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
 * mbnavadjust_autopick.c holds the crossing-analysis / autopick logic
 * shared, GUI-free, by mbnavadjust and mbnavadjustmerge: loading a
 * crossing's swath data, computing the misfit grid between the two
 * overlapping sections, and picking (or auto-picking) navigation ties.
 * This file contains no do_*() GUI callback calls - progress is instead
 * reported via fprintf(stderr, ...) gated on mbna_verbose, matching the
 * convention used throughout the rest of MB-System's command-line tools.
 *
 * This file used to be part of mbnavadjust_prog.c.
 *
 * Author:  D. W. Caress
 * Date:  July 23, 2026
 */

/*--------------------------------------------------------------------*/

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

#define MBNAVADJUST_CORE_DECLARE_GLOBALS
#include "mbnavadjust_core.h"

/* status variables - private to this file (mbnavadjust_prog.c has its own,
    used only for its own remaining functions' internal error/message
    scratch space, not read by any function in this file) */
static int error = MB_ERROR_NO_ERROR;
static mb_pathplusplus message;

/* real initializer for the tentative definition mbnavadjust_core.h created
    above via MBNAVADJUST_CORE_DECLARE_GLOBALS */
int nmisfit_intervals = MBNA_NINTERVALS_MISFIT;

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_snavpoints(int ix, int iy) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ix:           %d\n", ix);
    fprintf(stderr, "dbg2       iy:           %d\n", iy);
  }

  int status = MB_SUCCESS;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    /* get position in lon and lat */
    double x = ix / mbna_plotx_scale + mbna_plot_lon_min;
    double y = (cont_borders[3] - iy) / mbna_ploty_scale + mbna_plot_lat_min;
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];

    /* get closest snav point in swath 1 */
    struct mbna_section *section = &project.files[crossing->file_id_1].sections[crossing->section_1];
    double distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      double dx = (section->snav_lon[i] - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_1 = i;
        mbna_snav_1_time_d = section->snav_time_d[i];
        mbna_snav_1_lon = section->snav_lon[i];
        mbna_snav_1_lat = section->snav_lat[i];
      }
    }

    /* get closest snav point in swath 2 */
    section = &project.files[crossing->file_id_2].sections[crossing->section_2];
    distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      double dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_2 = i;
        mbna_snav_2_time_d = section->snav_time_d[i];
        mbna_snav_2_lon = section->snav_lon[i];
        mbna_snav_2_lat = section->snav_lat[i];
      }
    }
  }

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    /* get position in lon and lat */
    double x = ix / mbna_plotx_scale + mbna_plot_lon_min;
    double y = (cont_borders[3] - iy) / mbna_ploty_scale + mbna_plot_lat_min;

    /* get closest snav point in swath 2 */
    struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    mb_coor_scale(mbna_verbose, 0.5 * (section->latmin + section->latmax), &mbna_mtodeglon, &mbna_mtodeglat);
    double distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      double dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_2 = i;
        mbna_snav_2_time_d = section->snav_time_d[i];
        mbna_snav_2_lon = section->snav_lon[i];
        mbna_snav_2_lat = section->snav_lat[i];
      }
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_addtie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* deal with crossings */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    /* retrieve crossing parameters */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0
      && project.crossings[mbna_current_crossing].num_ties < MBNA_SNAV_NUM) {
      /* add tie and set number */
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      struct mbna_file *file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      struct mbna_file *file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      struct mbna_section *section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      struct mbna_section *section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      mbna_current_tie = crossing->num_ties;
      crossing->num_ties++;
      project.num_ties++;
      struct mbna_tie *tie = &crossing->ties[mbna_current_tie];

      if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
        project.num_crossings_analyzed++;
        if (crossing->truecrossing)
          project.num_truecrossings_analyzed++;
      }
      crossing->status = MBNA_CROSSING_STATUS_SET;

      /* look for unused pair of nav points */
      tie->snav_1 = -1;
      bool found = false;
      while (!found) {
        found = true;
        tie->snav_1++;
        for (int i = 0; i < crossing->num_ties - 1; i++) {
          if (crossing->ties[i].snav_1 == tie->snav_1)
            found = false;
        }
      }
      tie->snav_2 = -1;
      found = false;
      while (!found) {
        found = true;
        tie->snav_2++;
        for (int i = 0; i < crossing->num_ties - 1; i++) {
          if (crossing->ties[i].snav_2 == tie->snav_2)
            found = false;
        }
      }

      /* get rest of tie parameters */
      tie->status = MBNA_TIE_XYZ;
      tie->icrossing = mbna_current_crossing;
      tie->itie = mbna_current_tie;
      tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
      tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      tie->offset_x = mbna_offset_x;
      tie->offset_y = mbna_offset_y;
      tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      tie->offset_z_m = mbna_offset_z;
      tie->sigmar1 = mbna_minmisfit_sr1;
      tie->sigmar2 = mbna_minmisfit_sr2;
      tie->sigmar3 = mbna_minmisfit_sr3;
      for (int i = 0; i < 3; i++) {
        tie->sigmax1[i] = mbna_minmisfit_sx1[i];
        tie->sigmax2[i] = mbna_minmisfit_sx2[i];
        tie->sigmax3[i] = mbna_minmisfit_sx3[i];
      }
      if (tie->sigmar1 < MBNA_SMALL) {
        tie->sigmar1 = MBNA_SMALL;
        tie->sigmax1[0] = 1.0;
        tie->sigmax1[1] = 0.0;
        tie->sigmax1[2] = 0.0;
      }
      if (tie->sigmar2 < MBNA_SMALL) {
        tie->sigmar2 = MBNA_SMALL;
        tie->sigmax2[0] = 0.0;
        tie->sigmax2[1] = 1.0;
        tie->sigmax2[2] = 0.0;
      }
      if (tie->sigmar3 < MBNA_ZSMALL) {
        tie->sigmar3 = MBNA_ZSMALL;
        tie->sigmax3[0] = 0.0;
        tie->sigmax3[1] = 0.0;
        tie->sigmax3[2] = 1.0;
      }

      file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      tie->inversion_status = MBNA_INVERSION_NONE;
      tie->inversion_offset_x = mbna_invert_offset_x;
      tie->inversion_offset_y = mbna_invert_offset_y;
      tie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
      tie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
      tie->inversion_offset_z_m = mbna_invert_offset_z;
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* now put tie in center of plot */
      const int ix = (int)(0.5 * (mbna_plot_lon_max - mbna_plot_lon_min) * mbna_plotx_scale);
      const int iy = (int)(cont_borders[3] - (0.5 * (mbna_plot_lat_max - mbna_plot_lat_min) * mbna_ploty_scale));
      mbnavadjust_naverr_snavpoints(ix, iy);
      tie->snav_1 = mbna_snav_1;
      tie->snav_2 = mbna_snav_2;
      tie->snav_1_time_d = mbna_snav_1_time_d;
      tie->snav_2_time_d = mbna_snav_2_time_d;

      /* reset tie counts for snavs */
      section1->snav_num_ties[tie->snav_1]++;
      section2->snav_num_ties[tie->snav_2]++;

      /* set flag to update model plot */
      project.modelplot_uptodate = false;

      /* write updated project */
      project.save_count++;
      project.modelplot_uptodate = false;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
        project.save_count = 0;
      }

      /* add info text */
      snprintf(message, sizeof(message), "Add Tie Point %d of Crossing %d\n > Nav points: %2.2d:%4.4d:%2.2d:%2.2d %2.2d:%4.4d:%2.2d:%2.2d\n > Offsets: %f %f %f m\n",
              mbna_current_tie, mbna_current_crossing,
              project.files[crossing->file_id_1].survey, crossing->file_id_1, crossing->section_1, tie->snav_1,
              project.files[crossing->file_id_2].survey, crossing->file_id_2, crossing->section_2, tie->snav_2,
              tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
    }

    /* set mbna_crossing_select */
    if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
      mbna_crossing_select = mbna_current_crossing;
      if (mbna_current_tie >= 0)
        mbna_tie_select = mbna_current_tie;
      else
        mbna_tie_select = MBNA_SELECT_NONE;
    }
    else {
      mbna_crossing_select = MBNA_SELECT_NONE;
      mbna_tie_select = MBNA_SELECT_NONE;
    }
  }

  /* deal with sections */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    /* retrieve crossing parameters */
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      /* add tie and set number */
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status != MBNA_CROSSING_STATUS_SET)
        project.num_globalties++;
      section->status = MBNA_CROSSING_STATUS_SET;

      /* set global tie parameters */
      globaltie->status = MBNA_TIE_XY;
      globaltie->snav = mbna_snav_2;
      globaltie->refgrid_id = project.refgrid_select;
      globaltie->snav_time_d = section->snav_time_d[globaltie->snav];
      mbna_snav_1 = -1;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      globaltie->offset_x = mbna_offset_x;
      globaltie->offset_y = mbna_offset_y;
      globaltie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      globaltie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      globaltie->offset_z_m = mbna_offset_z;
      globaltie->sigmar1 = mbna_minmisfit_sr1;
      globaltie->sigmar2 = mbna_minmisfit_sr2;
      globaltie->sigmar3 = mbna_minmisfit_sr3;
      for (int i = 0; i < 3; i++) {
        globaltie->sigmax1[i] = mbna_minmisfit_sx1[i];
        globaltie->sigmax2[i] = mbna_minmisfit_sx2[i];
        globaltie->sigmax3[i] = mbna_minmisfit_sx3[i];
      }
      if (globaltie->sigmar1 < MBNA_SMALL) {
        globaltie->sigmar1 = MBNA_SMALL;
        globaltie->sigmax1[0] = 1.0;
        globaltie->sigmax1[1] = 0.0;
        globaltie->sigmax1[2] = 0.0;
      }
      if (globaltie->sigmar2 < MBNA_SMALL) {
        globaltie->sigmar2 = MBNA_SMALL;
        globaltie->sigmax2[0] = 0.0;
        globaltie->sigmax2[1] = 1.0;
        globaltie->sigmax2[2] = 0.0;
      }
      if (globaltie->sigmar3 < MBNA_ZSMALL) {
        globaltie->sigmar3 = MBNA_ZSMALL;
        globaltie->sigmax3[0] = 0.0;
        globaltie->sigmax3[1] = 0.0;
        globaltie->sigmax3[2] = 1.0;
      }

      mbna_invert_offset_x = section->snav_lon_offset[mbna_snav_2];
      mbna_invert_offset_y = section->snav_lat_offset[mbna_snav_2];
      mbna_invert_offset_z = section->snav_z_offset[mbna_snav_2];
      globaltie->inversion_status = MBNA_INVERSION_NONE;
      globaltie->inversion_offset_x = mbna_invert_offset_x;
      globaltie->inversion_offset_y = mbna_invert_offset_y;
      globaltie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
      globaltie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
      globaltie->inversion_offset_z_m = mbna_invert_offset_z;
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* set flag to update model plot */
      project.modelplot_uptodate = false;

      /* write updated project */
      project.save_count++;
      project.modelplot_uptodate = false;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
        project.save_count = 0;
      }

      /* add info text */
      snprintf(message, sizeof(message), "Add Global Tie of file %d section %d\n > Nav point: %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_file, mbna_current_section, mbna_current_file, mbna_current_section, globaltie->snav,
              globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_load() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;

  /* unload loaded crossing or section */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    status = mbnavadjust_crossing_unload();
  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    status = mbnavadjust_referenceplussection_unload();
  }
  mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  /* load current crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && project.open &&
      project.num_crossings > 0 && mbna_current_crossing >= 0) {
    /* retrieve crossing parameters */
    crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
    file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
    section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
    section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
    if (crossing->num_ties > 0 && mbna_current_tie >= 0) {
      /* get basic crossing parameters */
      tie = &crossing->ties[mbna_current_tie];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
    }
    else if (project.inversion_status != MBNA_INVERSION_NONE) {
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = section1->snav_time_d[mbna_snav_1];
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    }
    else {
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = section1->snav_time_d[mbna_snav_1];
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = 0.0;
      mbna_invert_offset_y = 0.0;
      mbna_invert_offset_z = 0.0;
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    }
    mbna_lon_min = MIN(section1->lonmin, section2->lonmin + mbna_offset_x);
    mbna_lon_max = MAX(section1->lonmax, section2->lonmax + mbna_offset_x);
    mbna_lat_min = MIN(section1->latmin, section2->latmin + mbna_offset_y);
    mbna_lat_max = MAX(section1->latmax, section2->latmax + mbna_offset_y);
    mbna_plot_lon_min = mbna_lon_min;
    mbna_plot_lon_max = mbna_lon_max;
    mbna_plot_lat_min = mbna_lat_min;
    mbna_plot_lat_max = mbna_lat_max;
    mb_coor_scale(mbna_verbose, 0.5 * (mbna_lat_min + mbna_lat_max), &mbna_mtodeglon, &mbna_mtodeglat);

    /* load sections */
    snprintf(message, sizeof(message), "Loading section 1 of crossing %d...", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_1, mbna_section_1,
                                          (void **)&swathraw1, (void **)&swath1, &error);
    snprintf(message, sizeof(message), "Loading section 2 of crossing %d...", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_2, mbna_section_2,
                                          (void **)&swathraw2, (void **)&swath2, &error);

    /* get lon lat positions for soundings */
    snprintf(message, sizeof(message), "Transforming section 1 of crossing %d...", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    snprintf(message, sizeof(message), "Transforming section 2 of crossing %d...", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      snprintf(message, sizeof(message), "Contouring section 1 of crossing %d...", mbna_current_crossing);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      mbna_contour = &mbna_contour1;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      snprintf(message, sizeof(message), "Contouring section 2 of crossing %d...", mbna_current_crossing);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
    }

    /* set loaded flag */
    mbna_naverr_mode = MBNA_NAVERR_MODE_CROSSING;

    /* generate misfit grids */
    snprintf(message, sizeof(message), "Getting misfit for crossing %d...", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    status = mbnavadjust_get_misfit();

    /* get overlap region */
    mbnavadjust_crossing_overlap(mbna_verbose, &project, mbna_current_crossing, &error);

    /* set flag to update model plot */
    project.modelplot_uptodate = false;
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_unload() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* unload loaded crossing */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    status = mbnavadjust_section_unload(mbna_verbose, (void **)&swathraw1, (void **)&swath1, &error);
    status = mbnavadjust_section_unload(mbna_verbose, (void **)&swathraw2, (void **)&swath2, &error);

    if (mbna_contour1.vector != NULL && mbna_contour1.nvector_alloc > 0) {
      free(mbna_contour1.vector);
    }
    if (mbna_contour2.vector != NULL && mbna_contour2.nvector_alloc > 0) {
      free(mbna_contour2.vector);
    }
    mbna_contour1.vector = NULL;
    mbna_contour1.nvector = 0;
    mbna_contour1.nvector_alloc = 0;
    mbna_contour2.vector = NULL;
    mbna_contour2.nvector = 0;
    mbna_contour2.nvector_alloc = 0;
    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;
    grid_nx = 0;
    grid_ny = 0;
    grid_nxy = 0;
    grid_nxyzeq = 0;
    gridm_nx = 0;
    gridm_ny = 0;
    gridm_nxyz = 0;
    if (grid1 != NULL) {
      free(grid1);
    }
    if (grid2 != NULL) {
      free(grid2);
    }
    if (gridm != NULL) {
      free(gridm);
    }
    if (gridmeq != NULL) {
      free(gridmeq);
    }
    if (gridn1 != NULL) {
      free(gridn1);
    }
    if (gridn2 != NULL) {
      free(gridn2);
    }
    if (gridnm != NULL) {
      free(gridnm);
    }
    grid1 = NULL;
    grid2 = NULL;
    gridm = NULL;
    gridmeq = NULL;
    gridn1 = NULL;
    gridn2 = NULL;
    gridnm = NULL;

    /* set flag to update model plot */
    project.modelplot_uptodate = false;

    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    status = mbnavadjust_referenceplussection_unload();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_replot() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* replot loaded crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    /* get lon lat positions for soundings */
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      mbna_contour = &mbna_contour1;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_referenceplussection_unload() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* unload loaded section */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (swath1 != NULL)
      status = mbnavadjust_reference_unload(mbna_verbose, (void **)&swath1, &error);
    if (swathraw2 != NULL && swath2 != NULL)
      status = mbnavadjust_section_unload(mbna_verbose, (void **)&swathraw2, (void **)&swath2, &error);

    if (mbna_contour1.vector != NULL && mbna_contour1.nvector_alloc > 0) {
      free(mbna_contour1.vector);
    }
    if (mbna_contour2.vector != NULL && mbna_contour2.nvector_alloc > 0) {
      free(mbna_contour2.vector);
    }
    mbna_contour1.vector = NULL;
    mbna_contour1.nvector = 0;
    mbna_contour1.nvector_alloc = 0;
    mbna_contour2.vector = NULL;
    mbna_contour2.nvector = 0;
    mbna_contour2.nvector_alloc = 0;
    project.refgrid_status = MBNA_REFGRID_UNLOADED;
    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;
    grid_nx = 0;
    grid_ny = 0;
    grid_nxy = 0;
    grid_nxyzeq = 0;
    gridm_nx = 0;
    gridm_ny = 0;
    gridm_nxyz = 0;
    if (grid1 != NULL) {
      free(grid1);
    }
    if (grid2 != NULL) {
      free(grid2);
    }
    if (gridm != NULL) {
      free(gridm);
    }
    if (gridmeq != NULL) {
      free(gridmeq);
    }
    if (gridn1 != NULL) {
      free(gridn1);
    }
    if (gridn2 != NULL) {
      free(gridn2);
    }
    if (gridnm != NULL) {
      free(gridnm);
    }
    grid1 = NULL;
    grid2 = NULL;
    gridm = NULL;
    gridmeq = NULL;
    gridn1 = NULL;
    gridn2 = NULL;
    gridnm = NULL;

    /* set flag to update model plot */
    project.modelplot_uptodate = false;

    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    status = mbnavadjust_crossing_unload();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfit() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  double dinterval;
  double zoff;
  double minmisfitthreshold, dotproduct;
  double x, y, z, r;
  double dotproductsave2;
  double rsave2;
  double dotproductsave3;
  double rsave3;
  bool found;
  int igx, igy;
  int lc;
  int ioff, joff, istart, iend, jstart, jend;
  int i2, j2, k1, k2;
  int k, ll;
  void *tptr;

  if (project.open
      && ((mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && project.num_crossings > 0 && mbna_current_crossing >= 0)
          || (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && project.refgrid_status == MBNA_REFGRID_LOADED))) {

    /* set message on */
    if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
      snprintf(message, sizeof(message), "Making misfit grid for crossing %d", mbna_current_crossing);
    else
      snprintf(message, sizeof(message), "Making misfit grid for file %d section %d vs reference bathymetry",
              mbna_file_select, mbna_section_select);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /* reset sounding density threshold for misfit calculation
        - will be tuned down if necessary */
    mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;

    /* figure out lateral extent of grids */
    grid_nx = MBNA_MISFIT_DIMXY;
    grid_ny = MBNA_MISFIT_DIMXY;
    if ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon > (mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat) {
      grid_dx = (mbna_plot_lon_max - mbna_plot_lon_min) / (grid_nx - 1);
      grid_dy = grid_dx * mbna_mtodeglat / mbna_mtodeglon;
    }
    else {
      grid_dy = (mbna_plot_lat_max - mbna_plot_lat_min) / (grid_ny - 1);
      grid_dx = grid_dy * mbna_mtodeglon / mbna_mtodeglat;
    }
    grid_nxy = grid_nx * grid_ny;
    grid_olon = 0.5 * (mbna_plot_lon_min + mbna_plot_lon_max) - (grid_nx / 2 + 0.5) * grid_dx;
    grid_olat = 0.5 * (mbna_plot_lat_min + mbna_plot_lat_max) - (grid_ny / 2 + 0.5) * grid_dy;

    /* get 3d misfit grid */
    nzmisfitcalc = MBNA_MISFIT_DIMZ;
    gridm_nx = grid_nx / 2 + 1;
    gridm_ny = gridm_nx;
    gridm_nxyz = gridm_nx * gridm_ny * nzmisfitcalc;
    if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER) {
      mbna_misfit_offset_x = 0.0;
      mbna_misfit_offset_y = 0.0;
      mbna_misfit_offset_z = 0.0;
    }
    else {
      mbna_misfit_offset_x = mbna_offset_x;
      mbna_misfit_offset_y = mbna_offset_y;
      mbna_misfit_offset_z = mbna_offset_z;
    }

    /* figure out range of z offsets */
    zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
    zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
    zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);

    /* allocate and initialize grids and arrays */
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(grid1, sizeof(double) * (grid_nxy));
      if (tptr != NULL) {
        grid1 = tptr;
        memset(grid1, 0, sizeof(double) * (grid_nxy));
      }
      else {
        free(grid1);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(grid2, sizeof(double) * (grid_nxy));
      if (tptr != NULL) {
        grid2 = tptr;
        memset(grid2, 0, sizeof(double) * (grid_nxy));
      }
      else {
        free(grid2);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(gridm, sizeof(double) * (gridm_nxyz));
      if (tptr != NULL) {
        gridm = tptr;
        memset(gridm, 0, sizeof(double) * (gridm_nxyz));
      }
      else {
        free(gridm);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(gridmeq, sizeof(double) * (gridm_nxyz));
      if (tptr != NULL) {
        gridmeq = tptr;
        memset(gridmeq, 0, sizeof(double) * (gridm_nxyz));
      }
      else {
        free(gridmeq);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridn1, sizeof(int) * (grid_nxy));
      if (tptr != NULL) {
        gridn1 = tptr;
        memset(gridn1, 0, sizeof(int) * (grid_nxy));
      }
      else {
        free(gridn1);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridn2, sizeof(int) * (grid_nxy));
      if (tptr != NULL) {
        gridn2 = tptr;
        memset(gridn2, 0, sizeof(int) * (grid_nxy));
      }
      else {
        free(gridn2);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridnm, sizeof(int) * (gridm_nxyz));
      if (tptr != NULL) {
        gridnm = tptr;
        memset(gridnm, 0, sizeof(int) * (gridm_nxyz));
      }
      else {
        free(gridnm);
        status = MB_FAILURE;
      }
    }

    /* loop over all beams */
    for (int i = 0; i < swath1->npings; i++) {
      for (int j = 0; j < swath1->pings[i].beams_bath; j++) {
        if (mb_beam_ok(swath1->pings[i].beamflag[j])) {
          x = (swath1->pings[i].bathlon[j] - grid_olon);
          y = (swath1->pings[i].bathlat[j] - grid_olat);
          igx = (int)(x / grid_dx);
          igy = (int)(y / grid_dy);
          k = igx + igy * grid_nx;
          if (igx >= 0 && igx < grid_nx && igy >= 0 && igy < grid_ny) {
            grid1[k] += swath1->pings[i].bath[j];
            gridn1[k]++;
          }
        }
      }
    }

    /* loop over all beams */
    for (int i = 0; i < swath2->npings; i++) {
      for (int j = 0; j < swath2->pings[i].beams_bath; j++) {
        if (mb_beam_ok(swath2->pings[i].beamflag[j])) {
          x = (swath2->pings[i].bathlon[j] + mbna_misfit_offset_x - grid_olon);
          y = (swath2->pings[i].bathlat[j] + mbna_misfit_offset_y - grid_olat);
          igx = (int)(x / grid_dx);
          igy = (int)(y / grid_dy);
          k = igx + igy * grid_nx;
          if (igx >= 0 && igx < grid_nx && igy >= 0 && igy < grid_ny) {
            grid2[k] += swath2->pings[i].bath[j];
            gridn2[k]++;
          }
        }
      }
    }

    /* calculate gridded bath */
    for (int k = 0; k < grid_nxy; k++) {
      if (gridn1[k] > 0) {
        grid1[k] = (grid1[k] / gridn1[k]);
      }
      if (gridn2[k] > 0) {
        grid2[k] = (grid2[k] / gridn2[k]);
      }
    }

    /* calculate gridded misfit over lateral and z offsets */
    for (int ic = 0; ic < gridm_nx; ic++)
      for (int jc = 0; jc < gridm_ny; jc++)
        for (int kc = 0; kc < nzmisfitcalc; kc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          gridm[lc] = 0.0;
          gridnm[lc] = 0;

          ioff = (gridm_nx / 2) - ic;
          joff = (gridm_ny / 2) - jc;
          zoff = zmin + zoff_dz * kc;

          istart = MAX(-ioff, 0);
          iend = grid_nx - MAX(0, ioff);
          jstart = MAX(-joff, 0);
          jend = grid_ny - MAX(0, joff);
          for (int i1 = istart; i1 < iend; i1++)
            for (int j1 = jstart; j1 < jend; j1++) {
              i2 = i1 + ioff;
              j2 = j1 + joff;
              k1 = i1 + j1 * grid_nx;
              k2 = i2 + j2 * grid_nx;
              if (gridn1[k1] > 0 && gridn2[k2] > 0) {
                gridm[lc] += (grid2[k2] - grid1[k1] + zoff - mbna_offset_z) *
                             (grid2[k2] - grid1[k1] + zoff - mbna_offset_z);
                gridnm[lc]++;
              }
            }
        }
    misfit_min = 0.0;
    misfit_max = 0.0;
    mbna_minmisfit = 0.0;
    mbna_minmisfit_n = 0;
    mbna_minmisfit_x = 0.0;
    mbna_minmisfit_y = 0.0;
    mbna_minmisfit_z = 0.0;
    found = false;
    for (int ic = 0; ic < gridm_nx; ic++)
      for (int jc = 0; jc < gridm_ny; jc++)
        for (int kc = 0; kc < nzmisfitcalc; kc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          if (gridnm[lc] > 0) {
            gridm[lc] = sqrt(gridm[lc]) / gridnm[lc];
            if (misfit_max == 0.0) {
              misfit_min = gridm[lc];
            }
            misfit_min = MIN(misfit_min, gridm[lc]);
            misfit_max = MAX(misfit_max, gridm[lc]);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit)) {
              mbna_minmisfit = gridm[lc];
              mbna_minmisfit_n = gridnm[lc];
              mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_z = zmin + zoff_dz * kc;
              found = true;
            }
          }
        }
    if (!found) {
      mbna_minmisfit_nthreshold /= 10.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold / 10 &&
                (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit)) {
              mbna_minmisfit = gridm[lc];
              mbna_minmisfit_n = gridnm[lc];
              mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_z = zmin + zoff_dz * kc;
              found = true;
            }
          }
    }
    misfit_min = 0.99 * misfit_min;
    misfit_max = 1.01 * misfit_max;

    /* set message on */
    if (mbna_verbose > 1)
      fprintf(stderr, "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
    snprintf(message, sizeof(message), "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /* sort the misfit to get histogram equalization */
    grid_nxyzeq = 0;
    for (int l = 0; l < gridm_nxyz; l++) {
      if (gridm[l] > 0.0) {
        gridmeq[grid_nxyzeq] = gridm[l];
        grid_nxyzeq++;
      }
    }

    if (grid_nxyzeq > 0) {
      qsort((char *)gridmeq, grid_nxyzeq, sizeof(double), mb_double_compare);
      dinterval = ((double)grid_nxyzeq) / ((double)(nmisfit_intervals - 1));
      if (dinterval < 1.0) {
        for (int l = 0; l < grid_nxyzeq; l++)
          misfit_intervals[l] = gridmeq[l];
        for (int l = grid_nxyzeq; l < nmisfit_intervals; l++)
          misfit_intervals[l] = gridmeq[grid_nxyzeq - 1];
      }
      else {
        misfit_intervals[0] = misfit_min;
        misfit_intervals[nmisfit_intervals - 1] = misfit_max;
        for (int l = 1; l < nmisfit_intervals - 1; l++) {
          ll = (int)(l * dinterval);
          misfit_intervals[l] = gridmeq[ll];
        }
      }

      /* get minimum misfit in 2D plane at current z offset */
      mbnavadjust_get_misfitxy();

      /* set message on */
      if (mbna_verbose > 1)
        fprintf(stderr, "Estimating 3D uncertainty for crossing %d\n", mbna_current_crossing);
      snprintf(message, sizeof(message), "Estimating 3D uncertainty for crossing %d\n", mbna_current_crossing);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);

      /* estimating 3 component uncertainty vector at minimum misfit point */
      /* first get the longest vector to a misfit value <= 2 times minimum misfit */
      minmisfitthreshold = mbna_minmisfit * 3.0;
      mbna_minmisfit_sr1 = 0.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold) {
              x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
              y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
              z = zmin + zoff_dz * kc - mbna_minmisfit_z;
              r = sqrt(x * x + y * y + z * z);
              if (r > mbna_minmisfit_sr1) {
                mbna_minmisfit_sx1[0] = x;
                mbna_minmisfit_sx1[1] = y;
                mbna_minmisfit_sx1[2] = z;
                mbna_minmisfit_sr1 = r;
              }
            }
          }
      mbna_minmisfit_sx1[0] /= mbna_minmisfit_sr1;
      mbna_minmisfit_sx1[1] /= mbna_minmisfit_sr1;
      mbna_minmisfit_sx1[2] /= mbna_minmisfit_sr1;

      /* now get a horizontal unit vector perpendicular to the the longest vector
          and then find the largest r associated with that vector */
      mbna_minmisfit_sr2 =
          sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
      if (mbna_minmisfit_sr2 < MBNA_SMALL) {
        mbna_minmisfit_sx2[0] = 0.0;
        mbna_minmisfit_sx2[1] = 1.0;
        mbna_minmisfit_sx2[2] = 0.0;
        mbna_minmisfit_sr2 = MBNA_SMALL;
      }
      else {
        mbna_minmisfit_sx2[0] = mbna_minmisfit_sx1[1] / mbna_minmisfit_sr2;
        mbna_minmisfit_sx2[1] = -mbna_minmisfit_sx1[0] / mbna_minmisfit_sr2;
        mbna_minmisfit_sx2[2] = 0.0;
        mbna_minmisfit_sr2 =
            sqrt(mbna_minmisfit_sx2[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx2[1] * mbna_minmisfit_sx2[1] +
                 mbna_minmisfit_sx2[2] * mbna_minmisfit_sx2[2]);
      }

      /* now get a near-vertical unit vector perpendicular to the the longest vector
          and then find the largest r associated with that vector */
      mbna_minmisfit_sr3 =
          sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
      if (mbna_minmisfit_sr3 < MBNA_ZSMALL) {
        mbna_minmisfit_sx3[0] = 0.0;
        mbna_minmisfit_sx3[1] = 0.0;
        mbna_minmisfit_sx3[2] = 1.0;
        mbna_minmisfit_sr3 = MBNA_ZSMALL;
      }
      else {
        if (mbna_minmisfit_sx1[2] >= 0.0) {
          mbna_minmisfit_sx3[0] =
              -mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
          mbna_minmisfit_sx3[1] =
              -mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
        }
        else {
          mbna_minmisfit_sx3[0] =
              mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
          mbna_minmisfit_sx3[1] =
              mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
        }
        mbna_minmisfit_sx3[2] = mbna_minmisfit_sr3;
        mbna_minmisfit_sr3 =
            sqrt(mbna_minmisfit_sx3[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx3[1] * mbna_minmisfit_sx3[1] +
                 mbna_minmisfit_sx3[2] * mbna_minmisfit_sx3[2]);
      }

      /* now get the longest r values to a misfit value <= 2 times minimum misfit
          for both secondary vectors */
      mbna_minmisfit_sr2 = 0.0;
      mbna_minmisfit_sr3 = 0.0;
      dotproductsave2 = 0.0;
      rsave2 = 0.0;
      dotproductsave3 = 0.0;
      rsave3 = 0.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold) {
              x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
              y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
              z = zmin + zoff_dz * kc - mbna_minmisfit_z;
              r = sqrt(x * x + y * y + z * z);
              if (r > mbna_minmisfit_sr2) {
                dotproduct =
                    (x * mbna_minmisfit_sx2[0] + y * mbna_minmisfit_sx2[1] + z * mbna_minmisfit_sx2[2]) / r;
                if (fabs(dotproduct) > 0.8) {
                  mbna_minmisfit_sr2 = r;
                }
                if (fabs(dotproduct) > dotproductsave2) {
                  dotproductsave2 = fabs(dotproduct);
                  rsave2 = r;
                }
              }
              if (r > mbna_minmisfit_sr3) {
                dotproduct =
                    (x * mbna_minmisfit_sx3[0] + y * mbna_minmisfit_sx3[1] + z * mbna_minmisfit_sx3[2]) / r;
                if (fabs(dotproduct) > 0.8) {
                  mbna_minmisfit_sr3 = r;
                }
                if (fabs(dotproduct) > dotproductsave3) {
                  dotproductsave3 = fabs(dotproduct);
                  rsave3 = r;
                }
              }
            }
          }
      if (mbna_minmisfit_sr2 < MBNA_SMALL)
        mbna_minmisfit_sr2 = rsave2;
      if (mbna_minmisfit_sr3 < MBNA_ZSMALL)
        mbna_minmisfit_sr3 = rsave3;
    }
    else {
      mbna_minmisfit_sx1[0] = 1.0;
      mbna_minmisfit_sx1[1] = 0.0;
      mbna_minmisfit_sx1[2] = 0.0;
      mbna_minmisfit_sr1 = 100.0;
      mbna_minmisfit_sx2[0] = 0.0;
      mbna_minmisfit_sx2[1] = 1.0;
      mbna_minmisfit_sx2[2] = 0.0;
      mbna_minmisfit_sr2 = 100.0;
      mbna_minmisfit_sx3[0] = 0.0;
      mbna_minmisfit_sx3[1] = 0.0;
      mbna_minmisfit_sx3[2] = 1.0;
      mbna_minmisfit_sr3 = 100.0;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfitxy() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  int kc, lc;

  if (project.open
      && ((mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && project.num_crossings > 0 && mbna_current_crossing >= 0)
          || (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && project.refgrid_status == MBNA_REFGRID_LOADED))) {
    if (grid_nxyzeq > 0) {
      /* get closest to current zoffset in existing 3d grid */
      misfit_max = 0.0;
      misfit_min = 0.0;
      kc = (int)((mbna_offset_z - zmin) / zoff_dz);
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          if (gridnm[lc] > mbna_minmisfit_nthreshold) {
            if (misfit_max == 0.0) {
              misfit_min = gridm[lc];
              misfit_max = gridm[lc];
            }
            else if (gridm[lc] < misfit_min) {
              misfit_min = gridm[lc];
              mbna_minmisfit_xh = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_yh = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_zh = zmin + zoff_dz * kc;
            }
            else if (gridm[lc] > misfit_max) {
              misfit_max = gridm[lc];
            }
          }
        }
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
void mbnavadjust_naverr_scale() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  double xscale, yscale;

  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    /* set scaling for contour window */
    xscale = (cont_borders[1] - cont_borders[0]) / ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon);
    yscale = (cont_borders[3] - cont_borders[2]) / ((mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat);
    if (xscale < yscale) {
      mbna_plotx_scale = xscale / mbna_mtodeglon;
      mbna_ploty_scale = xscale / mbna_mtodeglat;
      mbna_plot_lat_min =
          0.5 * (mbna_plot_lat_min + mbna_plot_lat_max) - 0.5 * (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
      mbna_plot_lat_max = mbna_plot_lat_min + (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
    }
    else {
      mbna_plotx_scale = yscale / mbna_mtodeglon;
      mbna_ploty_scale = yscale / mbna_mtodeglat;
      mbna_plot_lon_min =
          0.5 * (mbna_plot_lon_min + mbna_plot_lon_max) - 0.5 * (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
      mbna_plot_lon_max = mbna_plot_lon_min + (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
    }

    /* set scaling for misfit window */
    mbna_misfit_xscale = (corr_borders[1] - corr_borders[0]) / (grid_dx * (gridm_nx - 1));
    mbna_misfit_yscale = (corr_borders[3] - corr_borders[2]) / (grid_dy * (gridm_ny - 1));
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
  }
}
/*--------------------------------------------------------------------*/
int mbnavadjust_autopick(int verbose, struct mbna_project *project_ptr, int crossing_type, int scope_mode,
                         int survey_select, int survey_select1, int survey_select2,
                         int file_select, int section_select,
                         double overlap_threshold, bool do_vertical, int *error) {
  mbna_verbose = verbose;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       do_vertical: %d\n", do_vertical);
  }

  /* operate on the caller's project via the shared global (crossing_load,
      get_misfit, etc. all still reference the global directly) - a shallow
      copy is sufficient since the mutable file/section/crossing arrays are
      heap blocks referenced by pointer, so mutations through the global
      are visible through *project_ptr as well; scalar/summary fields are
      copied back explicitly below. */
  project = *project_ptr;

  /* mbnavadjust_crossing_load()'s inner logic only runs when mbna_status is
      MBNA_STATUS_NAVERR or MBNA_STATUS_AUTOPICK - the GUI's do_action_autopick
      callback sets this before calling autopick and restores it afterward;
      replicate that here since this is now the one place both the GUI and
      the CLI call through. */
  const int mbna_status_save = mbna_status;
  mbna_status = MBNA_STATUS_AUTOPICK;

  /* If nothing has ever called mbnavadjust_set_borders() (true for the CLI,
      which has no GUI canvas at all), cont_borders/corr_borders are still
      all zero and mbnavadjust_naverr_scale() would divide by zero computing
      its pixels-per-degree scale factors. Any non-zero, self-consistent
      placeholder works: the scale factors it produces cancel out exactly in
      the pixel<->lon/lat round-trips this cluster performs internally (see
      mbnavadjust_core.h), and a square aspect ratio keeps that neutral. */
  if (cont_borders[1] == cont_borders[0]) {
    cont_borders[0] = 0;
    cont_borders[1] = 999;
    cont_borders[2] = 0;
    cont_borders[3] = 999;
  }
  if (corr_borders[1] == corr_borders[0]) {
    corr_borders[0] = 0;
    corr_borders[1] = 999;
    corr_borders[2] = 0;
    corr_borders[3] = 999;
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;
  double dlon, dlat, overlap_scale;
  bool process;
  int nprocess;
  int isnav1_focus, isnav2_focus;
  double lon_focus, lat_focus;

  // loop over all crossings, autopick those that are in the requested
  // scope, unanalyzed, have sufficient overlap, and for which both sections
  // are sufficiently long (track length >=0.25 * project.section_length)
  if (project.open && project.num_crossings > 0) {
    snprintf(message, sizeof(message), "Autopicking offsets...\n");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s\n", message);

    /* loop over all crossings */
    nprocess = 0;
    for (int i = 0; i < project.num_crossings; i++) {
      /* get structure */
      crossing = &(project.crossings[i]);

      // check crossing is in the requested scope and has sufficient overlap
      process = false;
      if (crossing->status == MBNA_CROSSING_STATUS_NONE && crossing->overlap >= overlap_threshold) {
        if (crossing_type == MBNA_VIEW_LIST_CROSSINGS) {
          if ((scope_mode == MBNA_VIEW_MODE_ALL) ||
              (scope_mode == MBNA_VIEW_MODE_SURVEY &&
               survey_select == project.files[crossing->file_id_1].survey &&
               survey_select == project.files[crossing->file_id_2].survey) ||
              (scope_mode == MBNA_VIEW_MODE_FILE && file_select == crossing->file_id_1 &&
               file_select == crossing->file_id_2) ||
              (scope_mode == MBNA_VIEW_MODE_WITHSURVEY &&
               (survey_select == project.files[crossing->file_id_1].survey ||
                survey_select == project.files[crossing->file_id_2].survey)) ||
              (scope_mode == MBNA_VIEW_MODE_BLOCK &&
               ((survey_select1 == project.files[crossing->file_id_1].survey &&
                 survey_select2 == project.files[crossing->file_id_2].survey) ||
                (survey_select2 == project.files[crossing->file_id_1].survey &&
                 survey_select1 == project.files[crossing->file_id_2].survey))) ||
              (scope_mode == MBNA_VIEW_MODE_WITHFILE &&
               (file_select == crossing->file_id_1 || file_select == crossing->file_id_2)) ||
              (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_1 &&
               section_select == crossing->section_1) ||
              (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_2 &&
               section_select == crossing->section_2))
            process = true;
        }
        else if (crossing_type == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
          if (crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
            if ((scope_mode == MBNA_VIEW_MODE_ALL) ||
                (scope_mode == MBNA_VIEW_MODE_SURVEY &&
                 survey_select == project.files[crossing->file_id_1].survey &&
                 survey_select == project.files[crossing->file_id_2].survey) ||
                (scope_mode == MBNA_VIEW_MODE_FILE && file_select == crossing->file_id_1 &&
                 file_select == crossing->file_id_2) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (survey_select == project.files[crossing->file_id_1].survey ||
                  survey_select == project.files[crossing->file_id_2].survey)) ||
                (scope_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((survey_select1 == project.files[crossing->file_id_1].survey &&
                   survey_select2 == project.files[crossing->file_id_2].survey) ||
                  (survey_select2 == project.files[crossing->file_id_1].survey &&
                   survey_select1 == project.files[crossing->file_id_2].survey))) ||
                (scope_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (file_select == crossing->file_id_1 || file_select == crossing->file_id_2)) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_1 &&
                 section_select == crossing->section_1) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_2 &&
                 section_select == crossing->section_2))
              process = true;
          }
        }
        else if (crossing_type == MBNA_VIEW_LIST_GOODCROSSINGS) {
          if (crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
            if ((scope_mode == MBNA_VIEW_MODE_ALL) ||
                (scope_mode == MBNA_VIEW_MODE_SURVEY &&
                 survey_select == project.files[crossing->file_id_1].survey &&
                 survey_select == project.files[crossing->file_id_2].survey) ||
                (scope_mode == MBNA_VIEW_MODE_FILE && file_select == crossing->file_id_1 &&
                 file_select == crossing->file_id_2) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (survey_select == project.files[crossing->file_id_1].survey ||
                  survey_select == project.files[crossing->file_id_2].survey)) ||
                (scope_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((survey_select1 == project.files[crossing->file_id_1].survey &&
                   survey_select2 == project.files[crossing->file_id_2].survey) ||
                  (survey_select2 == project.files[crossing->file_id_1].survey &&
                   survey_select1 == project.files[crossing->file_id_2].survey))) ||
                (scope_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (file_select == crossing->file_id_1 || file_select == crossing->file_id_2)) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_1 &&
                 section_select == crossing->section_1) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_2 &&
                 section_select == crossing->section_2))
              process = true;
          }
        }
        else if (crossing_type == MBNA_VIEW_LIST_BETTERCROSSINGS) {
          if (crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
            if ((scope_mode == MBNA_VIEW_MODE_ALL) ||
                (scope_mode == MBNA_VIEW_MODE_SURVEY &&
                 survey_select == project.files[crossing->file_id_1].survey &&
                 survey_select == project.files[crossing->file_id_2].survey) ||
                (scope_mode == MBNA_VIEW_MODE_FILE && file_select == crossing->file_id_1 &&
                 file_select == crossing->file_id_2) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (survey_select == project.files[crossing->file_id_1].survey ||
                  survey_select == project.files[crossing->file_id_2].survey)) ||
                (scope_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((survey_select1 == project.files[crossing->file_id_1].survey &&
                   survey_select2 == project.files[crossing->file_id_2].survey) ||
                  (survey_select2 == project.files[crossing->file_id_1].survey &&
                   survey_select1 == project.files[crossing->file_id_2].survey))) ||
                (scope_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (file_select == crossing->file_id_1 || file_select == crossing->file_id_2)) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_1 &&
                 section_select == crossing->section_1) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_2 &&
                 section_select == crossing->section_2))
              process = true;
          }
        }
        else if (crossing_type == MBNA_VIEW_LIST_TRUECROSSINGS) {
          if (crossing->truecrossing) {
            if ((scope_mode == MBNA_VIEW_MODE_ALL) ||
                (scope_mode == MBNA_VIEW_MODE_SURVEY &&
                 survey_select == project.files[crossing->file_id_1].survey &&
                 survey_select == project.files[crossing->file_id_2].survey) ||
                (scope_mode == MBNA_VIEW_MODE_FILE && file_select == crossing->file_id_1 &&
                 file_select == crossing->file_id_2) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (survey_select == project.files[crossing->file_id_1].survey ||
                  survey_select == project.files[crossing->file_id_2].survey)) ||
                (scope_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((survey_select1 == project.files[crossing->file_id_1].survey &&
                   survey_select2 == project.files[crossing->file_id_2].survey) ||
                  (survey_select2 == project.files[crossing->file_id_1].survey &&
                   survey_select1 == project.files[crossing->file_id_2].survey))) ||
                (scope_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (file_select == crossing->file_id_1 || file_select == crossing->file_id_2)) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_1 &&
                 section_select == crossing->section_1) ||
                (scope_mode == MBNA_VIEW_MODE_WITHSECTION && file_select == crossing->file_id_2 &&
                 section_select == crossing->section_2))
              process = true;
          }
        }
        else
          process = true;
      }

      // check if section track lengths are long enough (at least 0.25 of the
      // current desired section length for the project) - this excludes trying to
      // match short sections at the end of a file */
      if (process) {
        file1 = &project.files[crossing->file_id_1];
        section1 = &file1->sections[crossing->section_1];
        file2 = &project.files[crossing->file_id_2];
        section2 = &file2->sections[crossing->section_2];
        if (section1->distance < 0.25 * project.section_length
            || section2->distance < 0.25 * project.section_length) {
          process = false;
        }
      }

      /* load the crossing */
      if (process) {
        mbna_current_crossing = i;
        mbna_file_id_1 = crossing->file_id_1;
        mbna_section_1 = crossing->section_1;
        mbna_file_id_2 = crossing->file_id_2;
        mbna_section_2 = crossing->section_2;
        mbna_current_tie = -1;

        /* reset survey file and section selections (GUI bookkeeping only;
            harmless for the CLI, which has no live selection to reflect) */
        if (scope_mode == MBNA_VIEW_MODE_SURVEY || scope_mode == MBNA_VIEW_MODE_WITHSURVEY) {
          if (survey_select == project.files[crossing->file_id_1].survey) {
            mbna_file_select = crossing->file_id_1;
            mbna_section_select = crossing->section_1;
          }
          else if (survey_select == project.files[crossing->file_id_2].survey) {
            mbna_file_select = crossing->file_id_2;
            mbna_section_select = crossing->section_2;
          }
          else {
            mbna_file_select = crossing->file_id_1;
            mbna_section_select = crossing->section_1;
          }
        }
        else if (scope_mode == MBNA_VIEW_MODE_FILE || scope_mode == MBNA_VIEW_MODE_WITHFILE) {
          if (file_select == crossing->file_id_1) {
            mbna_survey_select = project.files[crossing->file_id_1].survey;
            mbna_section_select = crossing->section_1;
          }
          else if (file_select == crossing->file_id_2) {
            mbna_survey_select = project.files[crossing->file_id_2].survey;
            mbna_section_select = crossing->section_2;
          }
          else {
            mbna_survey_select = project.files[crossing->file_id_1].survey;
            mbna_section_select = crossing->section_1;
          }
        }
        else if (scope_mode == MBNA_VIEW_MODE_WITHSECTION) {
          if (file_select == crossing->file_id_1 && section_select == crossing->section_1) {
            mbna_survey_select = project.files[crossing->file_id_1].survey;
            mbna_file_select = crossing->file_id_1;
          }
          else if (file_select == crossing->file_id_2 && section_select == crossing->section_2) {
            mbna_survey_select = project.files[crossing->file_id_2].survey;
            mbna_file_select = crossing->file_id_2;
          }
          else {
            mbna_survey_select = project.files[crossing->file_id_1].survey;
            mbna_file_select = crossing->file_id_1;
          }
        }
        else if (file_select == crossing->file_id_1) {
          mbna_survey_select = project.files[crossing->file_id_1].survey;
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }
        else if (file_select == crossing->file_id_2) {
          mbna_survey_select = project.files[crossing->file_id_2].survey;
          mbna_file_select = crossing->file_id_2;
          mbna_section_select = crossing->section_2;
        }
        else {
          mbna_survey_select = project.files[crossing->file_id_1].survey;
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }

        snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s: %s\n", __func__, message);

        /* load crossing */
        mbnavadjust_crossing_load();
        nprocess++;

        /* if this is a >50% overlap crossing then first set offsets to
            minimum misfit and then recalculate misfit */
        if (crossing->overlap > 50) {
          /* set offsets to minimum misfit */
          if (do_vertical) {
            mbna_offset_x = mbna_minmisfit_x;
            mbna_offset_y = mbna_minmisfit_y;
            mbna_offset_z = mbna_minmisfit_z;
          }
          else {
            mbna_offset_x = mbna_minmisfit_xh;
            mbna_offset_y = mbna_minmisfit_yh;
            mbna_offset_z = mbna_minmisfit_zh;
          }
          mbna_misfit_offset_x = mbna_offset_x;
          mbna_misfit_offset_y = mbna_offset_y;
          mbna_misfit_offset_z = mbna_offset_z;
          mbnavadjust_crossing_replot();

          /* get misfit */
          mbnavadjust_get_misfit();
        }

        /* set plot bounds to overlap region and recalculate misfit */
        mbnavadjust_crossing_overlapbounds(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                                   &mbna_overlap_lon_min, &mbna_overlap_lon_max,
                                                   &mbna_overlap_lat_min, &mbna_overlap_lat_max,
                                                   error);
        mbna_plot_lon_min = mbna_overlap_lon_min;
        mbna_plot_lon_max = mbna_overlap_lon_max;
        mbna_plot_lat_min = mbna_overlap_lat_min;
        mbna_plot_lat_max = mbna_overlap_lat_max;

        /* get characteristic scale of the overlap region */
        overlap_scale = MIN((mbna_overlap_lon_max - mbna_overlap_lon_min) / mbna_mtodeglon,
                            (mbna_overlap_lat_max - mbna_overlap_lat_min) / mbna_mtodeglat);

        /* get naverr plot scaling */
        mbnavadjust_naverr_scale();

        /* get misfit */
        mbnavadjust_get_misfit();

        /* The overlap focus point is currently the center of the line
         * connecting the two closest approach nav points. It could also
         * be the centroid of the overlap regions. */
        mbnavadjust_crossing_focuspoint(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                           &isnav1_focus, &isnav2_focus, &lon_focus, &lat_focus,
                                           error);

        /* If nonzero overlap region and focus point inside the overlap region,
         * set plot bounds to cover one-quarter
         * of overlap region centered on the focus point and
         * recalculate misfit. */
        if (mbna_overlap_lon_max > mbna_overlap_lon_min && mbna_overlap_lat_max > mbna_overlap_lat_min
          && lon_focus >= mbna_overlap_lon_min && lon_focus <= mbna_overlap_lon_max
          && lat_focus >= mbna_overlap_lat_min && lat_focus <= mbna_overlap_lat_max) {
          dlon =  0.25 * (mbna_overlap_lon_max - mbna_overlap_lon_min);
          dlat =  0.25 * (mbna_overlap_lat_max - mbna_overlap_lat_min);
          mbna_plot_lon_min = MAX((lon_focus - dlon), mbna_overlap_lon_min);
          mbna_plot_lon_max = MIN((lon_focus + dlon), mbna_overlap_lon_max);
          mbna_plot_lat_min = MAX((lat_focus - dlat), mbna_overlap_lat_min);
          mbna_plot_lat_max = MIN((lat_focus + dlat), mbna_overlap_lat_max);

          /* get naverr plot scaling */
          mbnavadjust_naverr_scale();

          /* get misfit */
          mbnavadjust_get_misfit();
        }

        /* check uncertainty estimate for a good pick */
        if (mbna_verbose > 0)
          fprintf(stderr, "Long misfit axis:%.3f Threshold:%.3f", MAX(mbna_minmisfit_sr1, mbna_minmisfit_sr2),
                  0.5 * overlap_scale);

        if (MAX(mbna_minmisfit_sr1, mbna_minmisfit_sr2) < 0.5 * overlap_scale &&
            MIN(mbna_minmisfit_sr1, mbna_minmisfit_sr2) > 0.0) {
          if (mbna_verbose > 0)
            fprintf(stderr, " AUTOPICK SUCCEEDED\n");

          /* set offsets to minimum misfit */
          if (do_vertical) {
            mbna_offset_x = mbna_minmisfit_x;
            mbna_offset_y = mbna_minmisfit_y;
            mbna_offset_z = mbna_minmisfit_z;
          }
          else {
            mbna_offset_x = mbna_minmisfit_xh;
            mbna_offset_y = mbna_minmisfit_yh;
            mbna_offset_z = mbna_minmisfit_zh;
          }

          /* add tie */
          mbnavadjust_naverr_addtie();
        } else {
          if (mbna_verbose > 0)
            fprintf(stderr, " AUTOPICK FAILED\n");
        }

        /* unload crossing */
        mbnavadjust_crossing_unload();

        if (mbna_verbose > 0)
          fprintf(stderr, "mbna_file_select:%d mbna_survey_select:%d mbna_section_select:%d\n", mbna_file_select,
                  mbna_survey_select, mbna_section_select);
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, error);
    project.save_count = 0;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  mbna_status = mbna_status_save;

  /* copy back scalar/summary fields to the caller's project (the array
      fields are already shared, in-place-mutated heap blocks) */
  *project_ptr = project;

  return (status);
}
