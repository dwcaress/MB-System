/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_invertnav.c
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
 * mbnavadjust_invertnav.c holds the network-adjustment navigation solver,
 * the reference bathymetry grid regeneration, and the corrected-navigation
 * output logic shared, GUI-free, by mbnavadjust and mbnavadjustmerge. This
 * file contains no do_*() GUI callback calls - progress is instead reported
 * via fprintf(stderr, ...) gated on mbna_verbose, matching the convention
 * used throughout the rest of MB-System's command-line tools and mirroring
 * mbnavadjust_autopick.c.
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
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

#include "mbnavadjust_core.h"

/* status variables - private to this file (mbnavadjust_prog.c and
    mbnavadjust_autopick.c each have their own, used only for their own
    functions' internal error/message scratch space) */
static int error = MB_ERROR_NO_ERROR;
static mb_pathplusplus message;
static mb_path error1;
static mb_pathplus error2;
static mb_path error3;

/*--------------------------------------------------------------------*/

void mb_aprod(int mode, int m, int n, double x[], double y[], void *UsrWrk) {
  (void)n;  // Unused parameter
  // mode == 1 : compute y = y + A*x
  // mode == 2 : compute x = x + A(transpose)*y
  struct mbna_matrix *matrix;
  int k;

  matrix = (struct mbna_matrix *)UsrWrk;

  if (mode == 1) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < matrix->nia[i]; j++) {
        k = matrix->ia[matrix->ia_dim * i + j];
        y[i] += matrix->a[matrix->ia_dim * i + j] * x[k];
      }
    }
  }

  else if (mode == 2) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < matrix->nia[i]; j++) {
        k = matrix->ia[matrix->ia_dim * i + j];
        x[k] += matrix->a[matrix->ia_dim * i + j] * y[i];
      }
    }
  }
}
/*--------------------------------------------------------------------*/

int mbnavadjust_invertnav(int verbose, struct mbna_project *project_ptr) {
  mbna_verbose = verbose;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* operate on the caller's project via the shared global (matches the
      mbnavadjust_autopick precedent) - a shallow copy is sufficient since
      the mutable file/section/crossing arrays are heap blocks referenced by
      pointer, so mutations through the global are visible through
      *project_ptr as well; scalar/summary fields are copied back explicitly
      below. */
  project = *project_ptr;

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_file *file1;
  struct mbna_file *file2;
  struct mbna_section *section;
  struct mbna_section *section1;
  struct mbna_section *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_matrix matrix = { 0, 0, 0, NULL, NULL, NULL };
  bool *x_continuity = NULL;
  int *x_quality = NULL;
  int *x_num_ties = NULL;
  int *x_chunk = NULL;
  double *x_time_d = NULL;
  int *chunk_center = NULL;
  bool *chunk_continuity = NULL;
  int *global_ties_xy_files = NULL;
  int *global_ties_xy_sections = NULL;
  int *global_ties_z_files = NULL;
  int *global_ties_z_sections = NULL;
  double *u = NULL;
  double *v = NULL;
  double *w = NULL;
  double *x = NULL;
  int *nx = NULL;
  double *se = NULL;
  double *b = NULL;
  int *nbxy = NULL;
  int *nbz = NULL;
  double *bxavg = NULL;
  double *byavg = NULL;
  double *bzavg = NULL;
  bool *bpoornav = NULL;
  int *bxfixstatus = NULL;
  int *byfixstatus = NULL;
  int *bzfixstatus = NULL;
  double *bxfix = NULL;
  double *byfix = NULL;
  double *bzfix = NULL;
  double matrix_scale = 1000.0;
  double rms_solution, rms_solution_total, rms_misfit_initial, rms_misfit_previous, rms_misfit_current;
  int nrms;

  int nnav = 0;
  int nblock = 0;
  int nglobaltiexy = 0;
  int nglobaltiez = 0;
  int nsmooth = 0;
  int ntie = 0;
  int nglobal = 0;
  int nfixed = 0;
  int nrows, ncols;
  int nblockties = 0;
  int nblockglobalties = 0;
  int nrows_ba = 0;
  int ncols_ba = 0;
  int nrows_alloc = 0;
  int ncols_alloc = 0;

  int nchunk, nchunk_start;
  double distance_sum, chunk_distance;
  double damping;

  int n_iteration;
  double convergence;
  double convergence_prior;
  double convergence_threshold;
  double offset_x, offset_y, offset_z, projected_offset;
  double weight, zweight;
  double smooth_exp;
  double smoothweight;
  bool ok_to_invert;
  bool found;
  double factor;
  int itielast, itienext;
  double damp;
  double atol;
  double btol;
  double relpr;
  double conlim;
  int itnlim;
  int istop_out;
  int itn_out;
  double anorm_out;
  double acond_out;
  double rnorm_out;
  double arnorm_out;
  double xnorm_out;

  /* check if it is ok to invert
      - if there is a project
      - enough crossings have been analyzed
      - no problems with offsets and offset uncertainties */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings))

  {
    /* check that all uncertainty magnitudes are nonzero */
    ok_to_invert = true;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigmar1 <= 0.0 || tie->sigmar2 <= 0.0 || tie->sigmar3 <= 0.0) {
            ok_to_invert = false;
            fprintf(stderr,
                    "PROBLEM WITH CROSSING TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
                    "%8.2f %8.2f %8.2f\n",
                    icrossing, j, project.files[crossing->file_id_1].survey, crossing->file_id_1, crossing->section_1,
                    tie->snav_1, project.files[crossing->file_id_2].survey, crossing->file_id_2, crossing->section_2,
                    tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1, tie->sigmar2,
                    tie->sigmar3);
          }
        }
      }
    }

    /* print out warning */
    if (!ok_to_invert) {
      fprintf(stderr, "\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
      fprintf(stderr, "Please fix the ties with problems noted above before trying again.\n\n");
    }
  }

  /* invert if there is a project and enough crossings have been analyzed */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      ok_to_invert)

  {
    fprintf(stderr, "\nInverting for navigation adjustment model...\n");

    /* set message dialog on */
    snprintf(message, sizeof(message), "Setting up navigation inversion...");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s\n", message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

    /* zero solution across all navigation */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          section->snav_lon_offset[isnav] = 0.0;
          section->snav_lat_offset[isnav] = 0.0;
          section->snav_z_offset[isnav] = 0.0;
        }
      }
    }

    /* zero the fixed tie structures for all file sections */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        section->fixedtie.status = MBNA_TIE_NONE;
      }
    }

    /* loop over all crossings looking for those in which one of the files
       involved is fixed and the other isn't - copy these to the fixedtie structure
       of the section structure. */
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        file1 = &project.files[crossing->file_id_1];
        file2 = &project.files[crossing->file_id_2];
        if ((file1->status == MBNA_FILE_FIXEDNAV || file1->status == MBNA_FILE_FIXEDXYNAV || file1->status == MBNA_FILE_FIXEDZNAV)
            && (file2->status == MBNA_FILE_POORNAV || file2->status == MBNA_FILE_GOODNAV)) {
          section2 = &file2->sections[crossing->section_2];
          if (file1->status == MBNA_FILE_FIXEDNAV)
            section2->fixedtie.status = MBNA_TIE_XYZ_FIXED;
          else if (file1->status == MBNA_FILE_FIXEDXYNAV)
            section2->fixedtie.status = MBNA_TIE_XY_FIXED;
          else if (file1->status == MBNA_FILE_FIXEDZNAV)
            section2->fixedtie.status = MBNA_TIE_Z_FIXED;
          else
            section2->fixedtie.status = MBNA_TIE_NONE;
          tie = &crossing->ties[0];
          section2->fixedtie.snav = tie->snav_2;
          section2->fixedtie.refgrid_id = 0;
          section2->fixedtie.snav_time_d = tie->snav_2_time_d;
          section2->fixedtie.offset_x = tie->offset_x;
          section2->fixedtie.offset_y = tie->offset_y;
          section2->fixedtie.offset_x_m = tie->offset_x_m;
          section2->fixedtie.offset_y_m = tie->offset_y_m;
          section2->fixedtie.offset_z_m = tie->offset_z_m;
          section2->fixedtie.sigmar1 = tie->sigmar1;
          section2->fixedtie.sigmax1[0] = tie->sigmax1[0];
          section2->fixedtie.sigmax1[1] = tie->sigmax1[1];
          section2->fixedtie.sigmax1[2] = tie->sigmax1[2];
          section2->fixedtie.sigmar2 = tie->sigmar2;
          section2->fixedtie.sigmax2[0] = tie->sigmax2[0];
          section2->fixedtie.sigmax2[1] = tie->sigmax2[1];
          section2->fixedtie.sigmax2[2] = tie->sigmax2[2];
          section2->fixedtie.sigmar3 = tie->sigmar3;
          section2->fixedtie.sigmax3[0] = tie->sigmax3[0];
          section2->fixedtie.sigmax3[1] = tie->sigmax3[1];
          section2->fixedtie.sigmax3[2] = tie->sigmax3[2];
          section2->fixedtie.inversion_status = tie->inversion_status;
          section2->fixedtie.inversion_offset_x = tie->inversion_offset_x;
          section2->fixedtie.inversion_offset_y = tie->inversion_offset_y;
          section2->fixedtie.inversion_offset_x_m = tie->inversion_offset_x_m;
          section2->fixedtie.inversion_offset_y_m = tie->inversion_offset_y_m;
          section2->fixedtie.inversion_offset_z_m = tie->inversion_offset_z_m;
          section2->fixedtie.dx_m = tie->dx_m;
          section2->fixedtie.dy_m = tie->dy_m;
          section2->fixedtie.dz_m = tie->dz_m;
          section2->fixedtie.sigma_m = tie->sigma_m;
          section2->fixedtie.dr1_m = tie->dr1_m;
          section2->fixedtie.dr2_m = tie->dr2_m;
          section2->fixedtie.dr3_m = tie->dr3_m;
          section2->fixedtie.rsigma_m = tie->rsigma_m;
          section2->fixedtie.isurveyplotindex  = tie->isurveyplotindex;
        }
        else if ((file2->status == MBNA_FILE_FIXEDNAV || file2->status == MBNA_FILE_FIXEDXYNAV)
            && (file1->status != MBNA_FILE_FIXEDNAV && file1->status != MBNA_FILE_FIXEDXYNAV)) {
          section1 = &file1->sections[crossing->section_1];
          if (file2->status == MBNA_FILE_FIXEDNAV)
            section1->fixedtie.status = MBNA_TIE_XYZ_FIXED;
          else if (file2->status == MBNA_FILE_FIXEDXYNAV)
            section1->fixedtie.status = MBNA_TIE_XY_FIXED;
          else if (file2->status == MBNA_FILE_FIXEDZNAV)
            section1->fixedtie.status = MBNA_TIE_Z_FIXED;
          else
            section1->fixedtie.status = MBNA_TIE_NONE;
          tie = &crossing->ties[0];
          section1->fixedtie.snav = tie->snav_1;
          section1->fixedtie.refgrid_id = 0;
          section1->fixedtie.snav_time_d = tie->snav_1_time_d;
          section1->fixedtie.offset_x = -tie->offset_x;
          section1->fixedtie.offset_y = -tie->offset_y;
          section1->fixedtie.offset_x_m = -tie->offset_x_m;
          section1->fixedtie.offset_y_m = -tie->offset_y_m;
          section1->fixedtie.offset_z_m = -tie->offset_z_m;
          section1->fixedtie.sigmar1 = tie->sigmar1;
          section1->fixedtie.sigmax1[0] = tie->sigmax1[0];
          section1->fixedtie.sigmax1[1] = tie->sigmax1[1];
          section1->fixedtie.sigmax1[2] = tie->sigmax1[2];
          section1->fixedtie.sigmar2 = tie->sigmar2;
          section1->fixedtie.sigmax2[0] = tie->sigmax2[0];
          section1->fixedtie.sigmax2[1] = tie->sigmax2[1];
          section1->fixedtie.sigmax2[2] = tie->sigmax2[2];
          section1->fixedtie.sigmar3 = tie->sigmar3;
          section1->fixedtie.sigmax3[0] = tie->sigmax3[0];
          section1->fixedtie.sigmax3[1] = tie->sigmax3[1];
          section1->fixedtie.sigmax3[2] = tie->sigmax3[2];
          section1->fixedtie.inversion_status = tie->inversion_status;
          section1->fixedtie.inversion_offset_x = -tie->inversion_offset_x;
          section1->fixedtie.inversion_offset_y = -tie->inversion_offset_y;
          section1->fixedtie.inversion_offset_x_m = -tie->inversion_offset_x_m;
          section1->fixedtie.inversion_offset_y_m = -tie->inversion_offset_y_m;
          section1->fixedtie.inversion_offset_z_m = -tie->inversion_offset_z_m;
          section1->fixedtie.dx_m = -tie->dx_m;
          section1->fixedtie.dy_m = -tie->dy_m;
          section1->fixedtie.dz_m = -tie->dz_m;
          section1->fixedtie.sigma_m = tie->sigma_m;
          section1->fixedtie.dr1_m = tie->dr1_m;
          section1->fixedtie.dr2_m = tie->dr2_m;
          section1->fixedtie.dr3_m = tie->dr3_m;
          section1->fixedtie.rsigma_m = tie->rsigma_m;
          section1->fixedtie.isurveyplotindex  = tie->isurveyplotindex;
        }
      }
    }

    /* count number of nav points, discontinuities, blocks, and global ties */
    nnav = 0;
    nblock = 0;
    nglobaltiexy = 0;
    nglobaltiez = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiexy++;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiez++;
          }
        }
        else if (section->fixedtie.status != MBNA_TIE_NONE) {
          if (section->fixedtie.status == MBNA_TIE_XY || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_XY_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiexy++;
          }
          if (section->fixedtie.status == MBNA_TIE_Z || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_Z_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiez++;
          }
        }
      }
      file->block = nblock - 1;
      file->block_offset_x = 0.0;
      file->block_offset_y = 0.0;
      file->block_offset_z = 0.0;
    }

    /* allocate nav time and continuity arrays */
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&x_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_quality, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_num_ties, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_chunk, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&x_time_d, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&chunk_center, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&chunk_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiexy * sizeof(int), (void **)&global_ties_xy_files, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiexy * sizeof(int), (void **)&global_ties_xy_sections, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiez * sizeof(int), (void **)&global_ties_z_files, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiez * sizeof(int), (void **)&global_ties_z_sections, &error);
    if (status != MB_SUCCESS) {
      strcpy(error1, "Unable to invert navigation!");
      strcpy(error2, "Failed to allocate memory for navigation control point arrays.");
      strcpy(error3, "The project may be too large for available memory.");
      fprintf(stderr, "%s\n%s\n%s\n", error1, error2, error3);
      snprintf(message, sizeof(message), "%s\n > %s\n", error1, error2);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      return (MB_FAILURE);
    }
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_quality, 0, nnav * sizeof(int));
    memset(x_num_ties, 0, nnav * sizeof(int));
    memset(x_chunk, 0, nnav * sizeof(int));
    memset(x_time_d, 0, nnav * sizeof(double));
    memset(chunk_center, 0, nnav * sizeof(int));
    memset(chunk_continuity, 0, nnav * sizeof(bool));
    memset(global_ties_xy_files, 0, nglobaltiexy * sizeof(int));
    memset(global_ties_xy_sections, 0, nglobaltiexy * sizeof(int));
    memset(global_ties_z_files, 0, nglobaltiez * sizeof(int));
    memset(global_ties_z_sections, 0, nglobaltiez * sizeof(int));

    /* loop over all files getting tables of time, continuity and global ties */
    int inav = 0;
    nchunk = 0;
    nchunk_start = 0;
    chunk_distance = 25 * project.section_length;
    distance_sum = 0.0;
    nglobaltiexy = 0;
    nglobaltiez = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      chunk_distance = 10 * file->sections[0].distance;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_xy_files[nglobaltiexy] = ifile;
            global_ties_xy_sections[nglobaltiexy] = isection;
            nglobaltiexy++;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_z_files[nglobaltiez] = ifile;
            global_ties_z_sections[nglobaltiez] = isection;
            nglobaltiez++;
          }
        }
        else if (section->fixedtie.status != MBNA_TIE_NONE) {
          if (section->fixedtie.status == MBNA_TIE_XY || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_XY_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_xy_files[nglobaltiexy] = ifile;
            global_ties_xy_sections[nglobaltiexy] = isection;
            nglobaltiexy++;
          }
          if (section->fixedtie.status == MBNA_TIE_Z || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_Z_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_z_files[nglobaltiez] = ifile;
            global_ties_z_sections[nglobaltiez] = isection;
            nglobaltiez++;
          }
        }
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (isnav == 0 && section->continuity) {
            section->snav_invert_id[isnav] = inav - 1;
          }
          else {
            section->snav_invert_id[isnav] = inav;
            if (isnav == 0) {
              x_continuity[inav] = false;
              distance_sum = 0.0;
            }
            else {
              x_continuity[inav] = true;
            }
            x_time_d[inav] = section->snav_time_d[isnav];
            x_quality[inav] = file->status;
            x_num_ties[inav] = section->snav_num_ties[isnav];
            distance_sum += section->snav_distance[isnav];
            if ((!x_continuity[inav] && inav > 0) || distance_sum > chunk_distance) {
                chunk_center[nchunk] = (nchunk_start + inav - 1) / 2;
//fprintf(stderr, "---chunk_center[%d]: %d\n", nchunk, chunk_center[nchunk]);
                nchunk++;
                chunk_continuity[nchunk] = x_continuity[inav];
                nchunk_start = inav;
                distance_sum = 0.0;
            }
            x_chunk[inav] = nchunk;
//fprintf(stderr,"inav:%d   %2.2d:%3.3d:%3.3d:%2.2d distance:  %f %f %f  chunk:%d:%d continuity:%d\n",
//        inav, file->block, ifile, isection, isnav, section->snav_distance[isnav], distance_sum, chunk_distance,
//        nchunk,x_chunk[inav],chunk_continuity[nchunk]);
            inav++;
          }
        }
      }
    }
    nchunk++;

    /* count first derivative smoothing points */
    nsmooth = 0;
    for (int inav = 0; inav < nnav - 1; inav++) {
      if (x_continuity[inav + 1]) {
        nsmooth += 3;
      }
    }

    /* count second derivative smoothing points */
    for (int inav = 0; inav < nnav - 2; inav++) {
        if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
            nsmooth += 3;
        }
    }

    /*----------------------------------------------------------------*/
    /* Generate starting adjustment model by applying global ties. If a block
       has a single global tie then that will be applied uniformly to all nav
       points in that block. If a block has more than one global tie then the
       offsets are applied by linear interpolation in time over the block. */
    /*----------------------------------------------------------------*/

/*
fprintf(stderr, "\nGlobal ties XY %d:\n", nglobaltiexy);
    for (int igtie = 0; igtie < nglobaltiexy; igtie++) {
      if (project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.status != MBNA_TIE_NONE) {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Global Tie\n",
                  igtie, project.files[global_ties_xy_files[igtie]].survey,
                  global_ties_xy_files[igtie],
                  global_ties_xy_sections[igtie],
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.snav,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.snav_time_d,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_x_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_y_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_z_m);
      }
      else {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Fixed Tie\n",
                  igtie, project.files[global_ties_xy_files[igtie]].survey,
                  global_ties_xy_files[igtie],
                  global_ties_xy_sections[igtie],
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.snav,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.snav_time_d,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_x_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_y_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_z_m);

      }
    }
fprintf(stderr, "\nGlobal ties Z %d:\n", nglobaltiez);
    for (int igtie = 0; igtie < nglobaltiez; igtie++) {
      if (project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.status != MBNA_TIE_NONE) {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Global Tie\n",
                  igtie, project.files[global_ties_z_files[igtie]].survey,
                  global_ties_z_files[igtie],
                  global_ties_z_sections[igtie],
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.snav,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.snav_time_d,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_x_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_y_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_z_m);
      }
      else {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Fixed Tie\n",
                  igtie, project.files[global_ties_z_files[igtie]].survey,
                  global_ties_z_files[igtie],
                  global_ties_z_sections[igtie],
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.snav,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.snav_time_d,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_x_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_y_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_z_m);
      }
    }
*/

    /* deal with xy global and/or fixed ties */
    for (int igtie = 0; igtie < nglobaltiexy; igtie++) {

      /* Note a conflict if the file for this global tie has xy navigation fixed */
      if (project.files[global_ties_xy_files[igtie]].status == MBNA_FILE_FIXEDNAV
          || project.files[global_ties_xy_files[igtie]].status == MBNA_FILE_FIXEDXYNAV) {
        fprintf(stdout, "MBnavadjust warning: An xy global tie has been defined for a file with xy navigation fixed.\n");
        fprintf(stdout, "  File: %2.2d:%5.5d %s   Section: %d  Offset: %f m east  %f m north  %f m vertical\n",
                project.files[global_ties_xy_sections[igtie]].survey,
                global_ties_xy_files[igtie],
                project.files[global_ties_xy_sections[igtie]].file,
                global_ties_xy_sections[igtie],
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_x_m,
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_y_m,
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_z_m);
        fprintf(stdout, "  This global tie will be ignored because the solution offset is constrained to be zero.\n\n");
      }

      /* deal with this global or fixed tie (global takes precedence if both exist) */
      int iblock_gtie = project.files[global_ties_xy_files[igtie]].survey;
      int ifile_gtie = global_ties_xy_files[igtie];
      int isection_gtie = global_ties_xy_sections[igtie];
      int isnav_gtie;
      double global_offset_time_d;
      double global_offset_x_m;
      double global_offset_y_m;
      if (project.files[ifile_gtie].sections[isection_gtie].globaltie.status != MBNA_TIE_NONE) {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav_time_d;
        global_offset_x_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_x_m;
        global_offset_y_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_y_m;
      }
      else /* if (project.files[ifile_gtie].sections[isection_gtie].fixedtie.status != MBNA_TIE_NONE) */ {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav_time_d;
        global_offset_x_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_x_m;
        global_offset_y_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_y_m;
      }
      int ifile_gtie0 = -1;
      int isection_gtie0 = -1;
      int isnav_gtie0 = -1;
      double global_offset0_time_d = 0.0;
      double global_offset0_x_m = 0.0;
      double global_offset0_y_m = 0.0;
      int iblock_gtie1 = -1;
      int ifile_gtie1 = -1;
      if (igtie > 0) {
        ifile_gtie0 = global_ties_xy_files[igtie-1];
        isection_gtie0 = global_ties_xy_sections[igtie-1];
        if (project.files[ifile_gtie0].sections[isection_gtie0].globaltie.status != MBNA_TIE_NONE) {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav_time_d;
          global_offset0_x_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_x_m;
          global_offset0_y_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_y_m;
        }
        else /* if (project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.status != MBNA_TIE_NONE) */ {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav_time_d;
          global_offset0_x_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_x_m;
          global_offset0_y_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_y_m;
        }
      }
      if (igtie < nglobaltiexy - 1) {
        iblock_gtie1 = project.files[global_ties_xy_files[igtie+1]].survey;
        ifile_gtie1 = global_ties_xy_files[igtie+1];
      }

      /* if this is the first global tie in a survey/block then set all previous nav
          in this block to the same offsets */
      if (igtie == 0 || project.files[global_ties_xy_files[igtie-1]].survey != iblock_gtie) {
        /* loop over all files and sections up to this point - any in the same block
            will have the offsets set */
        for (int ifile = 0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = 0; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = 0; isnav <= isnav_max; isnav++) {
                section->snav_lon_offset[isnav] = global_offset_x_m * project.mtodeglon;
                section->snav_lat_offset[isnav] = global_offset_y_m * project.mtodeglat;
              }
            }
          }
        }
      }

      /* else if the previous global tie is in the same block linearly interpolate
          the offsets to the current global tie */
      else {
        for (int ifile = ifile_gtie0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie0)
              isectionmin = isection_gtie0;
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              double fraction = 0.0;
              int isnav_min = 0;
              if (ifile == ifile_gtie0 && isection == isection_gtie0)
                isnav_min = isnav_gtie0;
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                if (global_offset_time_d > global_offset0_time_d)
                  fraction = (section->snav_time_d[isnav] - global_offset0_time_d)
                                    / (global_offset_time_d - global_offset0_time_d);
                section->snav_lon_offset[isnav] = (global_offset0_x_m
                      + fraction * (global_offset_x_m - global_offset0_x_m)) * project.mtodeglon;
                section->snav_lat_offset[isnav] = (global_offset0_y_m
                      + fraction * (global_offset_y_m - global_offset0_y_m)) * project.mtodeglat;
              }
            }
          }
        }
      }

      /* if this is the last global tie in a survey/block then set all following nav
          in this block to the same offsets */
      if (igtie == nglobaltiexy - 1 || iblock_gtie != iblock_gtie1) {
        /* loop over all files and sections following this point - any in the same block
            will have the offsets set */
        int ifilemax = project.num_files - 1;
        if (iblock_gtie1 > 0 && ifile_gtie1 > ifile_gtie)
          ifilemax = ifile_gtie1 - 1;
        for (int ifile = ifile_gtie; ifile <= ifilemax; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie)
              isectionmin = isection_gtie;
            int isectionmax = file->num_sections - 1;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_min = 0;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_min = isnav_gtie;
              int isnav_max = section->num_snav - 1;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                section->snav_lon_offset[isnav] = global_offset_x_m * project.mtodeglon;
                section->snav_lat_offset[isnav] = global_offset_y_m * project.mtodeglat;
              }
            }
          }
        }
      }
    } // end nglobaltiexy

    /* deal with z global ties */
    for (int igtie = 0; igtie < nglobaltiez; igtie++) {

      /* Note a conflict if the file for this global tie has z navigation fixed */
      if (project.files[global_ties_z_files[igtie]].status == MBNA_FILE_FIXEDNAV
          || project.files[global_ties_z_files[igtie]].status == MBNA_FILE_FIXEDZNAV) {
        fprintf(stdout, "MBnavadjust warning: A z global tie has been defined for a file with z navigation fixed.\n");
        fprintf(stdout, "  File: %2.2d:%5.5d %s   Section: %d  Offset: %f m east  %f m north  %f m vertical\n",
                project.files[global_ties_z_sections[igtie]].survey,
                global_ties_z_files[igtie],
                project.files[global_ties_z_sections[igtie]].file,
                global_ties_z_sections[igtie],
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_x_m,
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_y_m,
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_z_m);
        fprintf(stdout, "  This global tie will be ignored because the solution offset is constrained to be zero.\n\n");
      }

      /* deal with this global or fixed tie (global takes precedence if both exist) */
      int iblock_gtie = project.files[global_ties_xy_files[igtie]].survey;
      int ifile_gtie = global_ties_xy_files[igtie];
      int isection_gtie = global_ties_xy_sections[igtie];
      int isnav_gtie = -1;
      double global_offset_time_d = 0.0;
      double global_offset_z_m = 0.0;
      if (project.files[ifile_gtie].sections[isection_gtie].globaltie.status != MBNA_TIE_NONE) {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav_time_d;
        global_offset_z_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_z_m;
      }
      else /* if (project.files[ifile_gtie].sections[isection_gtie].fixedtie.status != MBNA_TIE_NONE) */ {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav_time_d;
        global_offset_z_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_z_m;
      }
      int ifile_gtie0 = -1;
      int isection_gtie0 = -1;
      int isnav_gtie0 = -1;
      double global_offset0_time_d = 0.0;
      double global_offset0_z_m = 0.0;
      int iblock_gtie1 = -1;
      int ifile_gtie1 = -1;
      if (igtie > 0) {
        ifile_gtie0 = global_ties_z_files[igtie-1];
        isection_gtie0 = global_ties_z_sections[igtie-1];
        if (project.files[ifile_gtie0].sections[isection_gtie0].globaltie.status != MBNA_TIE_NONE) {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav_time_d;
          global_offset0_z_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_z_m;
        }
        else /* if (project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.status != MBNA_TIE_NONE) */ {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav_time_d;
          global_offset0_z_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_z_m;
        }
      }
      if (igtie < nglobaltiez - 1) {
        iblock_gtie1 = project.files[global_ties_z_files[igtie+1]].survey;
        ifile_gtie1 = global_ties_z_files[igtie+1];
      }

      /* if this is the first global tie in a survey/block then set all previous nav
          in this block to the same offsets */
      if (igtie == 0 || project.files[global_ties_z_files[igtie-1]].survey != iblock_gtie) {
        /* loop over all files and sections up to this point - any in the same block
            will have the offsets set */
        for (int ifile = 0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = 0; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              for (int isnav = 0; isnav < section->num_snav; isnav++) {
                section->snav_z_offset[isnav] = global_offset_z_m;
              }
            }
          }
        }
      }

      /* else if the previous global tie is in the same block linearly interpolate
          the offsets to the current global tie */
      else {
        for (int ifile = global_ties_z_files[igtie-1]; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie0)
              isectionmin = isection_gtie0;
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              double fraction = 0.0;
              int isnav_min = 0;
              if (ifile == ifile_gtie0 && isection == isection_gtie0)
                isnav_min = isnav_gtie0;
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                if (global_offset_time_d > global_offset0_time_d)
                  fraction = (section->snav_time_d[isnav] - global_offset0_time_d)
                                    / (global_offset_time_d - global_offset0_time_d);
                section->snav_z_offset[isnav] = (global_offset0_z_m
                      + fraction * (global_offset_z_m - global_offset0_z_m));
              }
            }
          }
        }
      }

      /* if this is the last global tie in a survey/block then set all following nav
          in this block to the same offsets */
      if (igtie == nglobaltiexy - 1 || iblock_gtie != iblock_gtie1) {
        /* loop over all files and sections following this point - any in the same block
            will have the offsets set */
        int ifilemax = project.num_files - 1;
        if (iblock_gtie1 > 0 && ifile_gtie1 > ifile_gtie)
          ifilemax = ifile_gtie1 - 1;
        for (int ifile = ifile_gtie; ifile <= ifilemax; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie)
              isectionmin = isection_gtie;
            int isectionmax = file->num_sections - 1;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_min = 0;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_min = isnav_gtie;
              int isnav_max = section->num_snav - 1;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                section->snav_z_offset[isnav] = global_offset_z_m;
              }
            }
          }
        }
      }
    } // end nglobaltiez

    fprintf(stderr, "\nApplied global ties to initial adjustment model:\n\tnglobaltiexy:%d\n\tnglobaltiez:%d\n",
            nglobaltiexy, nglobaltiez);

    /*
    fprintf(stderr, "\nInitial adjustment model:\n");
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (section->globaltie.status != MBNA_TIE_NONE && isnav == 0) {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f    %d %9.3f %9.3f %7.3f\n",
                  file->block, ifile, isection, isnav,
                  section->snav_lon_offset[isnav] / project.mtodeglon,
                  section->snav_lat_offset[isnav] / project.mtodeglat,
                  section->snav_z_offset[isnav],
                  section->globaltie.status,
                  section->globaltie.offset_x_m,
                  section->globaltie.offset_y_m,
                  section->globaltie.offset_z_m);
          } else {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f\n",
                    file->block, ifile, isection, isnav,
                    section->snav_lon_offset[isnav] / project.mtodeglon,
                    section->snav_lat_offset[isnav] / project.mtodeglat,
                    section->snav_z_offset[isnav]);
          }
        }
      }
    }
    */


    /*----------------------------------------------------------------*/
    /* Modify starting adjustment model by applying any fixed         */
    /* navigation. Note that global ties and fixed files can be in    */
    /* conflict. This stage could overwrite global tie constraints    */
    /* applied above.                                                 */
    /*----------------------------------------------------------------*/

    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
        for (int isection = 0; isection < file->num_sections; isection++) {
          section = &file->sections[isection];
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            section->snav_lon_offset[isnav] = 0.0;
            section->snav_lat_offset[isnav] = 0.0;
          }
        }
      }
      if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
        for (int isection = 0; isection < file->num_sections; isection++) {
          section = &file->sections[isection];
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            section->snav_z_offset[isnav] = 0.0;
          }
        }
      }
    }

    /*----------------------------------------------------------------*/
    /* Modify starting adjustment model by solving for average adjustments
       between the survey/blocks using the offset signal that remains after
       applying the current adjustment model */
    /*----------------------------------------------------------------*/

    /* get dimensions of inversion problem and initial misfit */
    ntie = 0;
    nrms = 0;
    nglobal = 0;
    nfixed = 0;
    rms_misfit_initial = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks - calculate the original rms  */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XYZ
              || tie->status == MBNA_TIE_XY_FIXED || tie->status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (tie->offset_x_m * tie->offset_x_m) + (tie->offset_y_m * tie->offset_y_m);
            nrms += 2;
            //ntie += 2;
          }
          if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_XYZ
              || tie->status == MBNA_TIE_Z_FIXED || tie->status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (tie->offset_z_m * tie->offset_z_m);
            nrms += 1;
            //ntie += 1;
          }
          ntie += 3;
        }
      }
    }
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      /* get file */
      file = &project.files[ifile];

      /* count fixed and global ties for full inversion */
      for (int isection = 0; isection < file->num_sections; isection++) {
        /* get section */
        section = &file->sections[isection];

        /* count global ties for full inversion */
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (section->globaltie.offset_x_m * section->globaltie.offset_x_m)
                                    + (section->globaltie.offset_y_m * section->globaltie.offset_y_m);
            nrms += 2;
            nglobal += 2;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (section->globaltie.offset_z_m * section->globaltie.offset_z_m);
            nrms += 1;
            nglobal += 1;
          }
        }

        /* count fixed sections for full inversion */
        if (file->status == MBNA_FILE_FIXEDNAV)
          nfixed += 3 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDXYNAV)
          nfixed += 2 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDZNAV)
          nfixed += 1 * section->num_snav;
      }
    }
    if (nrms > 0) {
      rms_misfit_initial /= nrms;
      rms_misfit_previous = rms_misfit_initial;
      rms_misfit_current = rms_misfit_initial;
    }

    /* only do block average solution if there is more than one block */
    if (nblock > 1) {

      /* allocate block average offset arrays */
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbxy, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbz, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bxavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&byavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bzavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(bool), (void **)&bpoornav, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bxfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&byfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bzfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bxfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&byfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bzfix, &error);
      if (status != MB_SUCCESS) {
        strcpy(error1, "Unable to invert navigation!");
        strcpy(error2, "Failed to allocate memory for block-average arrays.");
        strcpy(error3, "The project may be too large for available memory.");
        fprintf(stderr, "%s\n%s\n%s\n", error1, error2, error3);
        snprintf(message, sizeof(message), "%s\n > %s\n", error1, error2);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", message);
        return (MB_FAILURE);
      }
      memset(nbxy, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(nbz, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(bxavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(byavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bzavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bpoornav, 0, nblock * sizeof(bool));
      memset(bxfixstatus, 0, nblock * sizeof(int));
      memset(byfixstatus, 0, nblock * sizeof(int));
      memset(bzfixstatus, 0, nblock * sizeof(int));
      memset(bxfix, 0, nblock * sizeof(double));
      memset(byfix, 0, nblock * sizeof(double));
      memset(bzfix, 0, nblock * sizeof(double));

      /* count ties for all block vs block pairs and calculate average offsets
       * and count dimensions of full inversion problem */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];

        /* for block vs block averages use only set crossings between
         * different blocks */
        if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          int jbvb1 = 0;
          int jbvb2 = 0;
          int jbvb = 0;
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            /* get tie */
            tie = (struct mbna_tie *)&crossing->ties[itie];

            /* if blocks differ get id for block vs block */
            if (project.files[crossing->file_id_1].survey != project.files[crossing->file_id_2].survey) {
              if (project.files[crossing->file_id_2].survey > project.files[crossing->file_id_1].survey) {
                jbvb1 = project.files[crossing->file_id_1].survey;
                jbvb2 = project.files[crossing->file_id_2].survey;
              }
              else {
                jbvb1 = project.files[crossing->file_id_2].survey;
                jbvb2 = project.files[crossing->file_id_1].survey;
              }
              jbvb = (jbvb2) * (jbvb2 + 1) / 2 + jbvb1;

              file1 = &project.files[crossing->file_id_1];
              section1 = &file1->sections[crossing->section_1];
              file2 = &project.files[crossing->file_id_2];
              section2 = &file2->sections[crossing->section_2];

              if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                bxavg[jbvb] += tie->offset_x_m
                            - (section2->snav_lon_offset[tie->snav_2]
                                - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                byavg[jbvb] += tie->offset_y_m
                            - (section2->snav_lat_offset[tie->snav_2]
                                - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                bzavg[jbvb] += tie->offset_z_m
                            - (section2->snav_z_offset[tie->snav_2]
                                - section1->snav_z_offset[tie->snav_1]);
                nbz[jbvb]++;
              }
            }
          }
        }
      }

      /* calculate block vs block tie averages */
      fprintf(stderr, "Survey vs Survey tie counts and average offsets:\n");
      nblockties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            bxavg[jbvb] /= nbxy[jbvb];
            byavg[jbvb] /= nbxy[jbvb];
            nblockties += 2;
          }
          if (nbz[jbvb] > 0) {
            bzavg[jbvb] /= nbz[jbvb];
            nblockties++;
          }
          fprintf(stderr, "%2d vs %2d: %5d xy ties  %5d z ties  Avg offsets: %8.3f %8.3f %8.3f\n", jblock, iblock,
                  nbxy[jbvb], nbz[jbvb], bxavg[jbvb], byavg[jbvb], bzavg[jbvb]);
        }
      }

      /* get fixed blocks and average global ties for blocks */
      mbna_global_tie_influence = 6000;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed and global ties for full inversion */
        for (int isection = 0; isection < file->num_sections; isection++) {
          /* get section */
          section = &file->sections[isection];

          /* count global ties for block offset inversion */
          if (section->globaltie.status != MBNA_TIE_NONE) {
            if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
              byfixstatus[file->block]++;
              byfix[file->block] += section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
            }
            if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
            }
          }
        }
      }

      /* count fixed sections for block average inversion,
       * overwriting global ties if they conflict */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed sections for block average inversion,
         * overwriting global ties if they conflict */
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
          bxfixstatus[file->block] = 1;
          bxfix[file->block] = 0.0;
          byfixstatus[file->block] = 1;
          byfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
          bzfixstatus[file->block] = 1;
          bzfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_POORNAV) {
          bpoornav[file->block] = true;
        }
      }
      nblockglobalties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          bxfix[iblock] /= (double)bxfixstatus[iblock];
          nblockglobalties++;
        }
        if (byfixstatus[iblock] > 0) {
          byfix[iblock] /= (double)byfixstatus[iblock];
          nblockglobalties++;
        }
        if (bzfixstatus[iblock] > 0) {
          bzfix[iblock] /= (double)bzfixstatus[iblock];
          nblockglobalties++;
        }
      }
    }

    /* We do a three stage inversion: first for block averages, then a slow relaxation
     * towards a coarse solution, and finally an overdetermined least squares
     * solution for an additional perturbation to satisfy the remaining signal.
     * Make sure arrays are allocated large enough for both stages. */
    nrows = nfixed + ntie + nglobal + nsmooth;
    ncols = 3 * nnav;
    nrows_ba = nblockties + nblockglobalties + 3;
    ncols_ba = 3 * nblock;
    nrows_alloc = MAX(nrows, nrows_ba);
    ncols_alloc = MAX(ncols, ncols_ba);
    fprintf(stderr, "\nMBnavadjust block average inversion preparation:\n");
    fprintf(stderr, "     nblock:            %d\n", nblock);
    fprintf(stderr, "     nblockties:        %d\n", nblockties);
    fprintf(stderr, "     nblockglobalties:  %d\n", nblockglobalties);
    fprintf(stderr, "     nrows_ba:          %d\n", nrows_ba);
    fprintf(stderr, "     ncols_ba:          %d\n", ncols_ba);
    fprintf(stderr, "\nMBnavadjust full inversion preparation:\n");
    fprintf(stderr, "     nnav:              %d\n", nnav);
    fprintf(stderr, "     ntie:              %d\n", ntie);
    fprintf(stderr, "     nglobal:           %d\n", nglobal);
    fprintf(stderr, "     nfixed:            %d\n", nfixed);
    fprintf(stderr, "     nsmooth:           %d\n", nsmooth);
    fprintf(stderr, "     nrows:             %d\n", nrows);
    fprintf(stderr, "     ncols:             %d\n", ncols);
    fprintf(stderr, "\nMBnavadjust inversion array allocation dimensions:\n");
    fprintf(stderr, "     nrows_alloc:       %d\n", nrows_alloc);
    fprintf(stderr, "     ncols_alloc:       %d\n", ncols_alloc);

    /* allocate solution vector x, perturbation vector xx, and average solution vector xa */
    matrix.nia = NULL;
    matrix.ia = NULL;
    matrix.a = NULL;
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&u, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&v, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&w, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&x, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(int), (void **)&nx, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&se, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&b, &error);
    /* matrix.ia/matrix.a are indexed by mb_aprod() as [matrix.ia_dim * row + col],
        and ia_dim is set below to ncols_ba (3 * nblock) for the preliminary
        block-offset solution - which can exceed 6 whenever nblock > 2 - so the
        allocation must cover ia_dim per row, not a fixed 6 */
    const int ia_dim_alloc = MAX(6, ncols_alloc);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(int), (void **)&matrix.nia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, (size_t)ia_dim_alloc * (size_t)nrows_alloc * sizeof(int), (void **)&matrix.ia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, (size_t)ia_dim_alloc * (size_t)nrows_alloc * sizeof(double), (void **)&matrix.a, &error);
    if (status != MB_SUCCESS) {
      strcpy(error1, "Unable to invert navigation!");
      strcpy(error2, "Failed to allocate memory for the sparse least-squares matrix.");
      strcpy(error3, "The project may be too large for available memory.");
      fprintf(stderr, "%s\n%s\n%s\n", error1, error2, error3);
      snprintf(message, sizeof(message), "%s\n > %s\n", error1, error2);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      return (MB_FAILURE);
    }
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(nx, 0, ncols_alloc * sizeof(int));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, (size_t)ia_dim_alloc * (size_t)nrows_alloc * sizeof(int));
    memset(matrix.a, 0, (size_t)ia_dim_alloc * (size_t)nrows_alloc * sizeof(double));

    /*----------------------------------------------------------------*/
    /* Create block offset inversion matrix problem                   */
    /*----------------------------------------------------------------*/
    if (nblock > 1) {
      matrix.m = nrows_ba;
      matrix.n = ncols_ba;
      matrix.ia_dim = ncols_ba;

      /* loop over each crossing, applying offsets evenly to both points
          for all ties that involve different blocks
          - weight inversely by number of ties for each block vs block pair
          so that each has same importance whether connected by one tie
          or many */

      /* set up inversion for block offsets
       * - start with average offsets between all block vs block pairs for
       *   x y and z wherever defined by one or more ties
       * - next apply average global ties for each block if they exist
       * - finally add a constraint for x y and z that the sum of all
       *   block offsets must be zero (ignoring blocks tagged as having
       *   poor navigation) */
      int irow = 0;

      /* start with average block vs block offsets */
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int index_m;
          int index_n;
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bxavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;

            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = byavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
          if (nbz[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bzavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
        }
      }

      /* next apply average global offsets for each block */
      mbna_global_tie_influence = 6000.0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m;
        int index_n;
        if (bxfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bxfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix X block %d to %f\n", iblock, bxfix[iblock]);
        }
        if (byfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * byfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix Y block %d to %f\n", iblock, byfix[iblock]);
        }
        if (bzfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bzfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix Z block %d to %f\n", iblock, bzfix[iblock]);
        }
      }

      /* add constraint that overall average offset must be zero, ignoring
       * blocks with poor navigation */
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 1;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 2;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;

      fprintf(stderr,
              "\nAbout to call LSQR for preliminary block solution   rows: %d cols: %d  (expected rows:%d cols:%d)\n", irow,
              nblock * 3, nrows_ba, ncols_ba);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < nrows_ba; irow++)
        u[irow] = b[irow];
      damp = 0.0;
      atol = 5.0e-7;   // releative precision of A matrix
      btol = 5.0e-7;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;
      // fprintf(stderr,"damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      //    damp,atol,btol,conlim,itnlim);

      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    int k = i * ncols_ba + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(nrows_ba, ncols_ba, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr,
                  &istop_out, &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

      /* save solution */
      double rms_solution = 0.0;
      double rms_solution_total = 0.0;
      int nrms = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          file->block_offset_x = x[3 * file->block];
          file->block_offset_y = x[3 * file->block + 1];
          file->block_offset_z = x[3 * file->block + 2];
          for (int isection = 0; isection < file->num_sections; isection++) {
              section = &file->sections[isection];
              for (int isnav = 0; isnav < section->num_snav; isnav++) {
                  section->snav_lon_offset[isnav] += file->block_offset_x * project.mtodeglon;
                  section->snav_lat_offset[isnav] += file->block_offset_y * project.mtodeglat;
                  section->snav_z_offset[isnav] += file->block_offset_z;
                  rms_solution += file->block_offset_x * file->block_offset_x;
                  rms_solution += file->block_offset_y * file->block_offset_y;
                  rms_solution += file->block_offset_z * file->block_offset_z;
                  nrms += 3;
              }
          }
      }
      if (nrms > 0) {
          rms_solution = sqrt(rms_solution);
          rms_solution_total = rms_solution;
      }

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out,
              sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
      fprintf(stderr, "\nBlock offsets (meters):\n");
      for (int i = 0; i < nblock; i++) {
        fprintf(stderr, "block[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n", i, x[3 * i], x[3 * i + 1],
                x[3 * i + 2]);
      }

      /* calculate final misfit */
      nrms = 0;
      rms_misfit_current = 0.0;
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          if (crossing->status == MBNA_CROSSING_STATUS_SET)
              for (int itie = 0; itie < crossing->num_ties; itie++) {
                  /* get tie */
                  tie = (struct mbna_tie *)&crossing->ties[itie];

                  /* get absolute id for first snav point */
                  file1 = &project.files[crossing->file_id_1];
                  section1 = &file1->sections[crossing->section_1];
                  // int nc1 = section1->snav_invert_id[tie->snav_1];

                  /* get absolute id for second snav point */
                  file2 = &project.files[crossing->file_id_2];
                  section2 = &file2->sections[crossing->section_2];
                  // int nc2 = section2->snav_invert_id[tie->snav_2];

                  /* get offset vector for this tie */
                  if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                      offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                      offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                      rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                      nrms += 2;
                  }
                  if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                      offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                      rms_misfit_current += offset_z * offset_z;
                      nrms += 1;
                  }
              }
      }
      for (int ifile = 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          for (int isection = 0; isection < file->num_sections; isection++) {
              section = &file->sections[isection];
              if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                  offset_x =
                      section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                  offset_y =
                      section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                  rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                  nrms += 2;
              }
              if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status == MBNA_TIE_XY_FIXED) {
                  offset_z = section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
                  rms_misfit_current += offset_z * offset_z;
                  nrms += 1;
              }
          }
      }
      if (nrms > 0) {
          rms_misfit_current = sqrt(rms_misfit_current) / nrms;
      }

      fprintf(stderr, "\nBlock inversion:\n > Solution size:        %12g\n"
              " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
              " > Previous misfit:      %12g\n > Final misfit:         %12g\n",
              rms_solution, rms_solution_total, rms_misfit_initial,
              rms_misfit_previous, rms_misfit_current);

      /* deallocate arrays used only for block inversion */
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbxy, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbz, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bpoornav, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfix, &error);
    }

    /*
    fprintf(stderr, "\nAdjustment model after block inversion stage:\n");
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (section->globaltie.status != MBNA_TIE_NONE && isnav == 0) {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f    %d %9.3f %9.3f %7.3f\n",
                  file->block, ifile, isection, isnav,
                  section->snav_lon_offset[isnav] / project.mtodeglon,
                  section->snav_lat_offset[isnav] / project.mtodeglat,
                  section->snav_z_offset[isnav],
                  section->globaltie.status,
                  section->globaltie.offset_x_m,
                  section->globaltie.offset_y_m,
                  section->globaltie.offset_z_m);
          } else {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f\n",
                    file->block, ifile, isection, isnav,
                    section->snav_lon_offset[isnav] / project.mtodeglon,
                    section->snav_lat_offset[isnav] / project.mtodeglat,
                    section->snav_z_offset[isnav]);
          }
        }
      }
    }
    */


        /* Stage 2 - Iteratively relax towards a coarse offset model in which
         * nav specified as poor is downweighted relative to good nav. The
         * coarseness is to solve for a navigation offset that is large scale
         * using a coarseness defined as 10 times the section length (which is
         * taken from the first section of the current file, as it can vary amongst
         * surveys in a project). The nav offsets of this coarse model will be
         * added to the global tie offsets and the block offsets to provide the
         * starting model for the the final inversion. Only the portions of the
         * crossing tie offsets not fit by this stage 2 model will be incorporated
         * into the inversion. The point of this multi-stage approach is to use
         * the least squares inversion to solve for zero mean, Gaussian distributed
         * offsets rather than the large scale offsets and drift characterizing
         * many multi-survey navigation adjustment problems.
         */

        /* loop over all ties applying the offsets to the chunks partitioned according to survey quality */
        n_iteration = 100000;
        convergence = 1000.0;
        convergence_prior = 1000.0;
        convergence_threshold = 0.000005;
        damping = 0.02;
        for (int iteration=0;
            iteration < n_iteration
                && convergence > convergence_threshold
                && convergence <= convergence_prior;
            iteration ++) {
            fprintf(stderr,"\nStage 2 relaxation iteration %d\n", iteration);

            /* zero the working average offset array */
            convergence_prior = convergence;
            memset(x, 0, ncols_alloc * sizeof(double));
            memset(nx, 0, ncols_alloc * sizeof(int));
            rms_misfit_previous = 0.0;
            nrms = 0;

            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];

                /* apply crossing ties */
                if (crossing->status == MBNA_CROSSING_STATUS_SET) {
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        int k1 = x_chunk[section1->snav_invert_id[tie->snav_1]];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        int k2 = x_chunk[section2->snav_invert_id[tie->snav_2]];

                        /* count tie impact on chunks */
                        nx[k1]++;
                        nx[k2]++;

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                            offset_x = tie->offset_x_m
                                        - (section2->snav_lon_offset[tie->snav_2]
                                            - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m
                                        - (section2->snav_lat_offset[tie->snav_2]
                                            - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;

                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        else {
                            offset_x = 0.0;
                            offset_y = 0.0;
                        }
                        if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                            offset_z = tie->offset_z_m
                                        - (section2->snav_z_offset[tie->snav_2]
                                            - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                        }
                        else {
                            offset_z = 0.0;
                        }

                        /* apply offsets to relevant chunks partitioned according to
                         * relative survey quality */
                        if (file1->status == MBNA_FILE_GOODNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_POORNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_FIXEDNAV
                                || file2->status == MBNA_FILE_FIXEDXYNAV
                                || file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDXYNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDZNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                    }
                }
            }

            /* apply global ties */
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->globaltie.status != MBNA_TIE_NONE) {

                        /* get absolute id for snav point */
                        int k = x_chunk[section->snav_invert_id[section->globaltie.snav]];

                        /* count global tie impact on chunks */
                        nx[k]++;

                        /* get and apply offset vector for this tie */
                        if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                            offset_x = section->globaltie.offset_x_m
                                - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                            offset_y = section->globaltie.offset_y_m
                                - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                            x[3*k]   += -offset_x;
                            x[3*k+1] += -offset_y;
                        }
                        if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
                            offset_z = section->globaltie.offset_z_m
                                - section->snav_z_offset[section->globaltie.snav];
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                            x[3*k+2] += -offset_z;
                        }
                    }
                }
            }

            /* linearly interpolate over gaps between impacted chunks */
            int klast = 0;
            for (int k=0; k < nchunk; k++) {
              if (nx[k] > 0) {
                if (k - klast > 1) {
                  if (chunk_continuity[klast+1] && chunk_continuity[k]) {
                    double factor0 = (x[3*k]   - x[3*klast])   / ((double)(k - klast));
                    double factor1 = (x[3*k+1] - x[3*klast+1]) / ((double)(k - klast));
                    double factor2 = (x[3*k+2] - x[3*klast+2]) / ((double)(k - klast));
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*klast]   + factor0 * ((double)(kk - klast));
                      x[3*kk+1] = x[3*klast+1] + factor1 * ((double)(kk - klast));
                      x[3*kk+2] = x[3*klast+2] + factor2 * ((double)(kk - klast));
                    }
                  }
                  else if (chunk_continuity[klast+1]) {
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*klast];
                      x[3*kk+1] = x[3*klast+1];
                      x[3*kk+2] = x[3*klast+2];
                    }
                  }
                  else if (chunk_continuity[k]) {
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*k];
                      x[3*kk+1] = x[3*k+1];
                      x[3*kk+2] = x[3*k+2];
                    }
                  }
                }
                klast = k;
              }
            }

            /* apply damping to solution vector */
            for (int k=0; k<3 * nchunk; k++) {
                x[k] *= damping;
            }

            /* penalize change between continuous chunks using the w work array */
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                w[3*k] = x[3*k] - x[3*(k-1)];
                w[3*k+1] = x[3*k+1] - x[3*(k-1)+1];
                w[3*k+2] = x[3*k+2] - x[3*(k-1)+2];
              }
            }
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                x[3*(k-1)] += 10.0 * damping * 0.5 * w[3*k];
                x[3*(k-1)+1] += 10.0 * damping * 0.5 * w[3*k+1];
                x[3*(k-1)+2] += 10.0 * damping * 0.5 * w[3*k+2];
                x[3*k] -= 10.0 * damping * 0.5 * w[3*k];
                x[3*k+1] -= 10.0 * damping * 0.5 * w[3*k+1];
                x[3*k+2] -= 10.0 * damping * 0.5 * w[3*k+2];
              }
            }

            /* get previous misfit measure */
            rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

            /* add average offsets back into the model */
            rms_solution = 0.0;
            rms_solution_total = 0.0;
            nrms = 0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    for (int isnav = 0; isnav < section->num_snav; isnav++) {
                        inav = section->snav_invert_id[isnav];
                        int k = x_chunk[inav];
                        if (inav == chunk_center[k]
                            || (k == 0 && inav <= chunk_center[k])
                            ||  (k == nchunk - 1 && inav >= chunk_center[k])) {
                            offset_x = x[3 * k];
                            offset_y = x[3 * k + 1];
                            offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                        }
                        else if  (inav <= chunk_center[k]) {
                            if (chunk_continuity[k]) {
                                factor = ((double)(inav - chunk_center[k-1])) / ((double)(chunk_center[k] - chunk_center[k-1]));
                                offset_x = x[3 * (k - 1)] + factor * (x[3 * k] - x[3 * (k - 1)]);
                                offset_y = x[3 * (k - 1) + 1] + factor * (x[3 * k + 1] - x[3 * (k - 1) + 1]);
                                offset_z = x[3 * (k - 1) + 2] + factor * (x[3 * k + 2] - x[3 * (k - 1) + 2]);
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            } else {
                                offset_x = x[3 * k];
                                offset_y = x[3 * k + 1];
                                offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            }
                        }
                        else if (inav >= chunk_center[k]) {
                            if (chunk_continuity[k+1]) {
                                factor = ((double)(inav - chunk_center[k])) / ((double)(chunk_center[k+1] - chunk_center[k]));
                                offset_x = x[3 * k] + factor * (x[3 * (k + 1)] - x[3 * k]);
                                offset_y = x[3 * k + 1] + factor * (x[3 * (k + 1) + 1] - x[3 * k + 1]);
                                offset_z = x[3 * k + 2] + factor * (x[3 * (k + 1) + 2] - x[3 * k + 2]);
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            } else {
                                offset_x = x[3 * k];
                                offset_y = x[3 * k + 1];
                                offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            }
                        }
//fprintf(stderr,"inav:%d %2.2d:%4.4d:%2.2d:%2.2d chunk:%d of %d cont:%d offsets:%f %f %f\n",
//inav,file->block, ifile, isection, isnav, k,nchunk,chunk_continuity[k],offset_x,offset_y,offset_z);

                        section->snav_lon_offset[isnav] += offset_x * project.mtodeglon;
                        section->snav_lat_offset[isnav] += offset_y * project.mtodeglat;
                        section->snav_z_offset[isnav] += offset_z;
                        rms_solution += offset_x * offset_x;
                        rms_solution += offset_y * offset_y;
                        rms_solution += offset_z * offset_z;
                        rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                        rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                        rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                        nrms += 3;
                    }
                }
            }
            if (nrms > 0) {
                rms_solution = sqrt(rms_solution);
                rms_solution_total = sqrt(rms_solution_total);
            }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                // int nc1;
                // int nc2;
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // const int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // const int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                        offset_x =
                            section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                        offset_y =
                            section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
                        offset_z = section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = fabs(rms_misfit_previous - rms_misfit_current) / rms_misfit_previous;
            }

            fprintf(stderr, " > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);
        } // iteration

    /* set message dialog on */
    snprintf(message, sizeof(message), "Completed chunk inversion...");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /*-------------------------------------------------------------------------*/
    /* Create complete inversion matrix problem to solve with LSQR             */
    /* - this is solving for a perturbation in addition to the model           */
    /* already constructed by the block inversion and then the chunk           */
    /* relaxation. Do this inversion with smoothing set by                     */
    /* project.smoothing using all crossing and global ties.                   */
    /*                                                                         */
    /* Invert each of the surveys separately first using only the ties within  */
    /* each survey and adding the result to the solution model. Then invert    */
    /* the whole project using all ties to fit the remmaining misfit.          */
    /*-------------------------------------------------------------------------*/

    for (int isurvey = -1; isurvey <= project.num_surveys; isurvey++) {
      matrix_scale = 1000.0;
      convergence = 1000.0;
      smooth_exp = project.smoothing;
      smoothweight = pow(10.0, smooth_exp) / 100.0;

      bool full_inversion = false;
      int inavstart = 0;
      int inavend = nnav - 1;
      if (isurvey == -1 || isurvey == project.num_surveys) {
        full_inversion = true;
        matrix.m = nrows;
        matrix.n = ncols;

        /* set message dialog on */
        if (isurvey == -1)
          snprintf(message, sizeof(message), "Performing initial navigation inversion using all crossing and global ties...");
        else
          snprintf(message, sizeof(message), "Performing final navigation inversion using all crossing and global ties...");
        if (mbna_verbose == 0)
          fprintf(stderr, "%s\n", message);
        fprintf(stderr, "\n------------------------------\n\nPreparing inversion of all surveys with smoothing %f ==> %f\n\t\tnfixed: %d  ntie: %d  nglobal: %d  nsmooth: %d\n\t\trows: %d  cols: %d\n",
              smooth_exp, smoothweight, nfixed, ntie, nglobal, nsmooth, matrix.m, matrix.n);
      }
      else {
        full_inversion = false;
        bool first = true;
        int ntie_surveyonly = 0;
        int nsmooth_surveyonly = 0;
        int nfixed_surveyonly = 0;
        int nglobal_surveyonly = 0;
        for (int ifile= 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          if (file->block == isurvey) {
            /* count fixed and global ties */
            for (int isection = 0; isection < file->num_sections; isection++) {
              section = &file->sections[isection];

              if (first) {
                inavstart = section->snav_invert_id[0];
                first = false;
              }
              inavend = section->snav_invert_id[section->num_snav-1];

              if (section->globaltie.status != MBNA_TIE_NONE) {
                if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
                    || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
                  rms_misfit_initial += (section->globaltie.offset_x_m * section->globaltie.offset_x_m)
                                          + (section->globaltie.offset_y_m * section->globaltie.offset_y_m);
                  nglobal_surveyonly += 2;
                }
                if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
                    || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
                  rms_misfit_initial += (section->globaltie.offset_z_m * section->globaltie.offset_z_m);
                  nglobal_surveyonly += 1;
                }
              }

              /* count fixed sections for full inversion */
              if (file->status == MBNA_FILE_FIXEDNAV)
                nfixed_surveyonly += 3 * section->num_snav;
              else if (file->status == MBNA_FILE_FIXEDXYNAV)
                nfixed_surveyonly += 2 * section->num_snav;
              else if (file->status == MBNA_FILE_FIXEDZNAV)
                nfixed_surveyonly += 1 * section->num_snav;
            }
          }
        }

        /* count first derivative smoothing points */
        for (int inav = inavstart; inav < inavend - 1; inav++) {
          if (x_continuity[inav + 1]) {
            nsmooth_surveyonly += 3;
          }
        }

        /* count second derivative smoothing points */
        for (int inav = inavstart; inav < inavend - 2; inav++) {
            if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
                nsmooth_surveyonly += 3;
            }
        }

        for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          if (crossing->status == MBNA_CROSSING_STATUS_SET
            && project.files[crossing->file_id_1].survey == isurvey
            && project.files[crossing->file_id_2].survey == isurvey) {
            for (int itie = 0; itie < crossing->num_ties; itie++) {
              ntie_surveyonly += 3;
            }
          }
        }

        matrix.m = ntie_surveyonly + nsmooth_surveyonly;
        matrix.n = 3 * (inavend - inavstart + 1);

        /* set message dialog on */
        snprintf(message, sizeof(message), "Performing navigation inversion for survey %d crossing ties only...", isurvey);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s\n", message);
        fprintf(stderr, "\n------------------------------\n\nPreparing inversion of survey %d with smoothing %f ==> %f\n\t\tnfixed: %d  ntie: %d  nglobal: %d  nsmooth: %d\n\t\trows: %d  cols: %d\n",
              isurvey, smooth_exp, smoothweight, nfixed_surveyonly, ntie_surveyonly, nglobal_surveyonly, nsmooth_surveyonly, matrix.m, matrix.n);
      }

      int irow = 0;
      nrms = 0;
      rms_misfit_previous = 0.0;
      matrix.ia_dim = 6;
      memset(u, 0, nrows_alloc * sizeof(double));
      memset(v, 0, ncols_alloc * sizeof(double));
      memset(w, 0, ncols_alloc * sizeof(double));
      memset(x, 0, ncols_alloc * sizeof(double));
      memset(se, 0, ncols_alloc * sizeof(double));
      memset(b, 0, nrows_alloc * sizeof(double));
      memset(matrix.nia, 0, nrows_alloc * sizeof(int));
      memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
      memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));

      fprintf(stderr, "\n----------\n\nPreparing inversion of survey %d with smoothing %f ==> %f\n\t\trows: %d  cols: %d\n",
              isurvey, smooth_exp, smoothweight, matrix.m, matrix.n);

      /* loop over each crossing, applying offsets evenly to both points */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          int nc1;
          int nc2;

          /* use only set crossings */
          if (crossing->status == MBNA_CROSSING_STATUS_SET
              && (full_inversion || (project.files[crossing->file_id_1].survey == isurvey
                  && project.files[crossing->file_id_2].survey == isurvey)))
              for (int itie = 0; itie < crossing->num_ties; itie++) {
                  /* A: get tie */
                  tie = (struct mbna_tie *)&crossing->ties[itie];
                  int index_m;
                  int index_n;

                  /* A1: get absolute id for first snav point */
                  file1 = &project.files[crossing->file_id_1];
                  section1 = &file1->sections[crossing->section_1];
                  nc1 = section1->snav_invert_id[tie->snav_1] - inavstart;

                  /* A2: get absolute id for second snav point */
                  file2 = &project.files[crossing->file_id_2];
                  section2 = &file2->sections[crossing->section_2];
                  nc2 = section2->snav_invert_id[tie->snav_2] - inavstart;

                  /* get uncertainty ellipsoid component magnitudes,
                      make them small if tie is set fixed so that
                      the solution actually closely matches the tie */
                  double sigmar1 = tie->sigmar1;
                  double sigmar2 = tie->sigmar2;
                  double sigmar3 = tie->sigmar3;
                  if (tie->status == MBNA_TIE_XY_FIXED
                      || tie->status == MBNA_TIE_Z_FIXED
                      || tie->status == MBNA_TIE_XYZ_FIXED) {
                      sigmar1 = 0.01;
                      sigmar2 = 0.01;
                      sigmar3 = 0.01;
                      }

                  if (section1->snav_time_d[tie->snav_1] ==
                      section2->snav_time_d[tie->snav_2])
                      fprintf(stderr, "ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
                              crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2, crossing->section_2,
                              tie->snav_2,
                              (section1->snav_time_d[tie->snav_1] -
                               section2->snav_time_d[tie->snav_2]));

                  /* A3: get offset vector for this tie */
                  if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                      offset_x = tie->offset_x_m
                                  - (section2->snav_lon_offset[tie->snav_2]
                                      - section1->snav_lon_offset[tie->snav_1])
                                      / project.mtodeglon;
                      offset_y = tie->offset_y_m
                                  - (section2->snav_lat_offset[tie->snav_2]
                                      - section1->snav_lat_offset[tie->snav_1])
                                      / project.mtodeglat;

                      rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                      nrms += 2;
                      //offset_x = tie->offset_x_m - (file2->block_offset_x - file1->block_offset_x);
                      //offset_y = tie->offset_y_m - (file2->block_offset_y - file1->block_offset_y);
                  }
                  else {
                      offset_x = 0.0;
                      offset_y = 0.0;
                  }
                  if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                      offset_z = tie->offset_z_m
                                  - (section2->snav_z_offset[tie->snav_2]
                                      - section1->snav_z_offset[tie->snav_1]);
                      rms_misfit_previous += offset_z * offset_z;
                      nrms += 1;
                      //offset_z = tie->offset_z_m - (file2->block_offset_z - file1->block_offset_z);
                  }
                  else {
                      offset_z = 0.0;
                  }

                  /* deal with each component of the error ellipse
                      - project offset vector onto each component by dot-product
                  - weight inversely by size of error for that component */

                  /* B1: deal with long axis */
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                      projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1];
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1] + offset_z * tie->sigmax1[2];
                  if (fabs(sigmar1) > 0.0)
                      weight = 1.0 / sigmar1;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax1[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax1[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax1[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax1[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY
                      || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax1[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY
                      || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax1[2];

                  b[irow] = weight * projected_offset;
                  matrix.nia[irow] = 6;
                  irow++;

                  /* B2: deal with horizontal axis */
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                      projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1];
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1] + offset_z * tie->sigmax2[2];
                  if (fabs(sigmar2) > 0.0)
                      weight = 1.0 / sigmar2;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax2[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax2[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax2[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax2[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax2[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax2[2];

                  b[irow] = weight * projected_offset;
                  matrix.nia[irow] = 6;
                  irow++;

                  /* B3:  deal with semi-vertical axis */
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                      projected_offset = offset_z * tie->sigmax3[2];
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      projected_offset = offset_x * tie->sigmax3[0] + offset_y * tie->sigmax3[1] + offset_z * tie->sigmax3[2];
                  if (fabs(sigmar3) > 0.0)
                      weight = 1.0 / sigmar3;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  matrix.a[index_m] = -weight * tie->sigmax3[0];
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax3[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax3[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax3[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax3[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax3[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax3[2];

                  b[irow] = weight * projected_offset;
                  matrix.nia[irow] = 6;
                  irow++;
              }
      }

      /* C1: loop over all files applying any global ties */
      //weight = 10.0;
      if (full_inversion) {
          for (int ifile = 0; ifile < project.num_files; ifile++) {
              file = &project.files[ifile];
              for (int isection = 0; isection < file->num_sections; isection++) {
                  section = &file->sections[isection];
                  int index_m;
                  int index_n;
                  struct mbna_globaltie *globaltie = &section->globaltie;

                  /* get uncertainty ellipsoid component magnitudes,
                      make them small if tie is set fixed so that
                      the solution actually closely matches the tie */
                  double sigmar1 = globaltie->sigmar1;
                  double sigmar2 = globaltie->sigmar2;
                  double sigmar3 = globaltie->sigmar3;
                  if (globaltie->status == MBNA_TIE_XY_FIXED
                      || globaltie->status == MBNA_TIE_Z_FIXED
                      || globaltie->status == MBNA_TIE_XYZ_FIXED) {
                      sigmar1 = 0.01;
                      sigmar2 = 0.01;
                      sigmar3 = 0.01;
                      }

                  if (globaltie->status == MBNA_TIE_XYZ
                      || globaltie->status == MBNA_TIE_XY
                      || globaltie->status == MBNA_TIE_XYZ_FIXED
                      || globaltie->status == MBNA_TIE_XY_FIXED) {
                      offset_x = globaltie->offset_x_m - section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                      weight = 1.0 / sigmar1;
                      weight *= matrix_scale;
      //fprintf(stderr,"APPLYING WEIGHT: %f  ifile:%d isection:%d\n",weight,ifile,isection);

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_x;
                      //b[irow] = weight * (globaltie->offset_x_m - file->block_offset_x);
                      matrix.nia[irow] = 1;
                      irow++;

                      offset_y = globaltie->offset_y_m - section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                      weight = 1.0 / sigmar2;
                      weight *= matrix_scale;

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3 + 1;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_y;
                      //b[irow] = weight * (globaltie->offset_y_m - file->block_offset_y);
                      matrix.nia[irow] = 1;
                      irow++;

                      rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                      nrms += 2;
                  }

                  if (globaltie->status == MBNA_TIE_XYZ
                      || globaltie->status == MBNA_TIE_Z
                      || globaltie->status == MBNA_TIE_XYZ_FIXED
                      || globaltie->status == MBNA_TIE_Z_FIXED) {
                      offset_z = globaltie->offset_z_m - section->snav_z_offset[globaltie->snav];
                      weight = 1.0 / sigmar3;
                      weight *= matrix_scale;

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3 + 2;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_z;
                      //b[irow] = weight * (globaltie->offset_z_m - file->block_offset_z);
                      matrix.nia[irow] = 1;
                      irow++;

                      rms_misfit_previous += offset_z * offset_z;
                      nrms += 1;
                  }
              }
          }
          rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

          /* D1: loop over all files applying ties for any fixed files */
          weight = 1000.0;
          weight *= matrix_scale;
          for (int ifile = 0; ifile < project.num_files; ifile++) {
              file = &project.files[ifile];
              int index_m;
              int index_n;
              if (file->status == MBNA_FILE_FIXEDNAV
                  || file->status == MBNA_FILE_FIXEDXYNAV
                  || file->status == MBNA_FILE_FIXEDZNAV) {
                  for (int isection = 0; isection < file->num_sections; isection++) {
                      section = &file->sections[isection];
                      for (int isnav = 0; isnav < section->num_snav; isnav++) {
                          if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3;
                              matrix.ia[index_m] = index_n;
                              matrix.a[index_m] = weight;
                              b[irow] = -file->block_offset_x;
                              matrix.nia[irow] = 1;
                              irow++;

                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3 + 1;
                              matrix.ia[index_m] = index_n;
                              matrix.a[index_m] = weight;
                              b[irow] = -file->block_offset_y;
                              matrix.nia[irow] = 1;
                              irow++;
                          }

                          if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3 + 2;
                              matrix.ia[index_m] = index_n;
                              matrix.a[index_m] = weight;
                              b[irow] = -file->block_offset_z;
                              matrix.nia[irow] = 1;
                              irow++;
                          }
                      }
                  }
              }
          }
      }

      /* E1: loop over all navigation applying first derivative smoothing */
      for (int inav = inavstart; inav < inavend; inav++) {
          int index_m;
          int index_n;
          if (x_continuity[inav + 1]) {
              if (x_time_d[inav + 1] - x_time_d[inav] > 0.0) {
                  weight = smoothweight / (x_time_d[inav + 1] - x_time_d[inav]);
                  if (x_quality[inav] == MBNA_FILE_POORNAV || x_quality[inav+1] == MBNA_FILE_POORNAV){
                      weight *= 0.25;
                  }
              }
              else {
                  weight = 0.0000001;
              }
              weight *= matrix_scale;
              zweight = 10.0 * weight;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -zweight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;
          }
      }

      /* E1: loop over all navigation applying second derivative smoothing */
      for (int inav = inavstart; inav < inavend - 1; inav++) {
          int index_m;
          int index_n;
          if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
              if (x_time_d[inav + 2] - x_time_d[inav] > 0.0) {
                  weight = smoothweight / (x_time_d[inav + 2] - x_time_d[inav]);
                  if (x_quality[inav] == MBNA_FILE_POORNAV
                      || x_quality[inav+1] == MBNA_FILE_POORNAV
                      || x_quality[inav+2] == MBNA_FILE_POORNAV) {
                      weight *= 0.25;
                  }
              }
              else {
                  weight = 0.0000001;
              }
              weight *= matrix_scale;
              zweight = 10.0 * weight;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * weight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * weight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * zweight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;
          }
      }

      fprintf(stderr, "\nAbout to call LSQR rows: %d  cols: %d\n", matrix.m, matrix.n);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < matrix.m; irow++)
          u[irow] = b[irow];
      damp = 0.0;
      atol = 5.0e-7;   // releative precision of A matrix
      btol = 5.0e-7;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;
      // fprintf(stderr, "damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      // damp, atol, btol, conlim, itnlim);

      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    k = i * 6 + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(matrix.m, matrix.n, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr, &istop_out,
                  &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out, sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);

      /* interpolate solution */
      itielast = -1;
      itienext = -1;
      for (int inav = inavstart; inav <= inavend; inav++) {
          int iinv = inav - inavstart;

          if (x_num_ties[inav] > 0) {
              itielast = inav;
          }
          else {
              /* look for the next tied point or the next discontinuity */
              found = false;
              itienext = -1;
              for (int iinav=inav+1; iinav < nnav && !found; iinav++) {
                  if (!x_continuity[iinav]) {
                      found = true;
                      itienext = -1;
                  }
                  else if (x_num_ties[iinav] > 0) {
                      found = true;
                      itienext = iinav;
                  }
              }
              if (!x_continuity[inav]) {
                  itielast = -1;
              }

              /* now interpolate or extrapolate */
              if (itielast >= 0 && itienext > itielast) {
                  factor = (x_time_d[inav] - x_time_d[itielast] ) / (x_time_d[itienext] - x_time_d[itielast]);
                  x[iinv * 3] = x[(itielast - inavstart) * 3] + factor * (x[(itienext - inavstart) * 3] - x[(itielast - inavstart) * 3]);
                  x[iinv * 3 + 1] = x[(itielast - inavstart) * 3 + 1] + factor * (x[(itienext - inavstart) * 3 + 1] - x[(itielast - inavstart) * 3 + 1]);
                  x[iinv * 3 + 2] = x[(itielast - inavstart) * 3 + 2] + factor * (x[(itienext - inavstart) * 3 + 2] - x[(itielast - inavstart) * 3 + 2]);
              }
              else if (itielast >= 0) {
                  x[iinv * 3] = x[(itielast - inavstart) * 3];
                  x[iinv * 3 + 1] = x[(itielast - inavstart) * 3 + 1];
                  x[iinv * 3 + 2] = x[(itielast - inavstart) * 3 + 2];
              }
              else if (itienext >= 0) {
                  x[iinv * 3] = x[(itienext - inavstart) * 3];
                  x[iinv * 3 + 1] = x[(itienext - inavstart) * 3 + 1];
                  x[iinv * 3 + 2] = x[(itienext - inavstart) * 3 + 2];
              }
          }
      }

      /* save solution */
      rms_solution = 0.0;
      rms_solution_total = 0.0;
      nrms = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          if (full_inversion || file->block == isurvey) {
              for (int isection = 0; isection < file->num_sections; isection++) {
                  section = &file->sections[isection];
                  for (int isnav = 0; isnav < section->num_snav; isnav++) {
                      int k = section->snav_invert_id[isnav] - inavstart;
                      section->snav_lon_offset[isnav] += x[3 * k] * project.mtodeglon;
                      section->snav_lat_offset[isnav] += x[3 * k + 1] * project.mtodeglat;
                      section->snav_z_offset[isnav] += x[3 * k + 2];
                      rms_solution += x[3 * k] * x[3 * k];
                      rms_solution += x[3 * k + 1] * x[3 * k + 1];
                      rms_solution += x[3 * k + 2] * x[3 * k + 2];
                      rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                      rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                      rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                      nrms += 3;
                  }
              }
          }
      }
      if (nrms > 0) {
          rms_solution = sqrt(rms_solution);
          rms_solution_total = sqrt(rms_solution_total);
      }
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          if (crossing->status == MBNA_CROSSING_STATUS_SET)
              for (int itie = 0; itie < crossing->num_ties; itie++) {
                  /* get tie */
                  tie = (struct mbna_tie *)&crossing->ties[itie];

                  /* get absolute id for first snav point */
                  file1 = &project.files[crossing->file_id_1];
                  section1 = &file1->sections[crossing->section_1];
                  // int nc1 = section1->snav_invert_id[tie->snav_1];

                  /* get absolute id for second snav point */
                  file2 = &project.files[crossing->file_id_2];
                  section2 = &file2->sections[crossing->section_2];
                  // int nc2 = section2->snav_invert_id[tie->snav_2];

                  if (full_inversion || (file1->block == isurvey && file2->block == isurvey)) {
                      offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                      offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                      offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
/* fprintf(stderr, "Tie Offset: %5d:%1d  %4.4d:%2.2d:%2.2d:%2.2d  %4.4d:%2.2d:%2.2d:%2.2d  %10.3f %10.3f %10.3f\n",
icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, section1->snav_invert_id[tie->snav_1],
file2->block, crossing->file_id_2, crossing->section_2, section2->snav_invert_id[tie->snav_2],
offset_x, offset_y, offset_z); */
                  }
            }
      }


    } /* end loop over surveys */

    /* calculate final misfit */
    nrms = 0;
    rms_misfit_current = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];
        if (crossing->status == MBNA_CROSSING_STATUS_SET)
            for (int itie = 0; itie < crossing->num_ties; itie++) {
                /* get tie */
                tie = (struct mbna_tie *)&crossing->ties[itie];

                /* get absolute id for first snav point */
                file1 = &project.files[crossing->file_id_1];
                section1 = &file1->sections[crossing->section_1];
                // int nc1 = section1->snav_invert_id[tie->snav_1];

                /* get absolute id for second snav point */
                file2 = &project.files[crossing->file_id_2];
                section2 = &file2->sections[crossing->section_2];
                // int nc2 = section2->snav_invert_id[tie->snav_2];

                /* get offset vector for this tie */
                if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                    offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                    offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                    rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                    nrms += 2;
                }
                if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                    offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                    rms_misfit_current += offset_z * offset_z;
                    nrms += 1;
                }
            }
    }
    for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        for (int isection = 0; isection < file->num_sections; isection++) {
            section = &file->sections[isection];
            struct mbna_globaltie *globaltie = &section->globaltie;
            if (globaltie->status != MBNA_TIE_NONE) {
                if (globaltie->status != MBNA_TIE_Z && globaltie->status != MBNA_TIE_Z_FIXED) {
                    offset_x =
                        globaltie->offset_x_m - section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                    offset_y =
                        globaltie->offset_y_m - section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                    rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                    nrms += 2;
                }
                if (globaltie->status != MBNA_TIE_XY || globaltie->status != MBNA_TIE_XY_FIXED) {
                    offset_z = globaltie->offset_z_m - section->snav_z_offset[globaltie->snav];
                    rms_misfit_current += offset_z * offset_z;
                    nrms += 1;
                }
            }
        }
    }
    if (nrms > 0) {
        rms_misfit_current = sqrt(rms_misfit_current) / nrms;
        convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous;
    }

    fprintf(stderr, "\nInversion %d:\n > Solution size:        %12g\n"
            " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
            " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
            " > Convergence:          %12g\n",
            1, rms_solution, rms_solution_total, rms_misfit_initial,
            rms_misfit_previous, rms_misfit_current, convergence);

    /*-------------------------------------------------------------------------*/

    /* set message dialog on */
    snprintf(message, sizeof(message), "Completed inversion...");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /* now output inverse solution */
    snprintf(message, sizeof(message), "Outputting navigation solution...");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    snprintf(message, sizeof(message), " > Final misfit:%12g\n > Initial misfit:%12g\n", rms_misfit_current, rms_misfit_initial);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /* get crossing offset results */
    snprintf(message, sizeof(message), " > Nav Tie Offsets (m):  id  observed  solution  error\n");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    mb_path tie_file;
    strcpy(tie_file, project.path);
    strcat(tie_file, project.name);
    strcat(tie_file, "_tiesoln.txt");
    FILE *ofp = fopen(tie_file, "w");
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* check only set ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          offset_x = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon_offset[tie->snav_1];
          offset_y = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat_offset[tie->snav_1];
          offset_z = project.files[crossing->file_id_2].sections[crossing->section_2].snav_z_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_z_offset[tie->snav_1];

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(offset_x) > 10000.0 || fabs(offset_y) > 10000.0 || fabs(offset_z) > 10000.0) {
            tie->inversion_status = MBNA_INVERSION_OLD;
            tie->inversion_offset_x = 0.0;
            tie->inversion_offset_y = 0.0;
            tie->inversion_offset_x_m = 0.0;
            tie->inversion_offset_y_m = 0.0;
            tie->inversion_offset_z_m = 0.0;
            tie->dx_m = 0.0;
            tie->dy_m = 0.0;
            tie->dz_m = 0.0;
            tie->sigma_m = 0.0;
            tie->dr1_m = 0.0;
            tie->dr2_m = 0.0;
            tie->dr3_m = 0.0;
            tie->rsigma_m = 0.0;
          }
          else {
            tie->inversion_status = MBNA_INVERSION_CURRENT;
            tie->inversion_offset_x = offset_x;
            tie->inversion_offset_y = offset_y;
            tie->inversion_offset_x_m = offset_x / project.mtodeglon;
            tie->inversion_offset_y_m = offset_y / project.mtodeglat;
            tie->inversion_offset_z_m = offset_z;
            tie->dx_m = tie->offset_x_m - tie->inversion_offset_x_m;
            tie->dy_m = tie->offset_y_m - tie->inversion_offset_y_m;
            tie->dz_m = tie->offset_z_m - tie->inversion_offset_z_m;
                        tie->sigma_m = sqrt(tie->dx_m * tie->dx_m + tie->dy_m * tie->dy_m + tie->dz_m * tie->dz_m);
            tie->dr1_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax1[0] +
                         (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax1[1] +
                         (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax1[2]) /
                         tie->sigmar1;
            tie->dr2_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax2[0] +
                         (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax2[1] +
                         (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax2[2]) /
                         tie->sigmar2;
            tie->dr3_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax3[0] +
                         (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax3[1] +
                         (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax3[2]) /
                         tie->sigmar3;
            tie->rsigma_m = sqrt(tie->dr1_m * tie->dr1_m + tie->dr2_m * tie->dr2_m + tie->dr3_m * tie->dr3_m);
          }

          snprintf(message, sizeof(message), " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f\n",
                  icrossing, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                            tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                            tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", message);
          int snav_1_time_i[7], snav_2_time_i[7];
          double snav_1_time_d = project.files[crossing->file_id_1].sections[crossing->section_1].snav_time_d[tie->snav_1];
          double snav_2_time_d = project.files[crossing->file_id_2].sections[crossing->section_2].snav_time_d[tie->snav_2];
          mb_get_date(mbna_verbose, snav_1_time_d, snav_1_time_i);
          mb_get_date(mbna_verbose, snav_2_time_d, snav_2_time_i);
          double avg_tie_lon = 0.5 * (project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon[tie->snav_1]
                                      + project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon[tie->snav_2]);
          double avg_tie_lat = 0.5 * (project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat[tie->snav_1]
                                      + project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat[tie->snav_2]);
          fprintf(ofp,  "%2.2d:%4.4d:%3.3d:%2.2d %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d  %.6f "
                        "%2.2d:%4.4d:%3.3d:%2.2d %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d  %.6f "
                        "%14.9f %14.9f "
                        "%8.2f %8.2f %8.2f   %8.2f %8.2f %8.2f   %8.2f %8.2f %8.2f\n",
                      project.files[crossing->file_id_1].survey, crossing->file_id_1, crossing->section_1, tie->snav_1,
                      snav_1_time_i[0], snav_1_time_i[1], snav_1_time_i[2], snav_1_time_i[3], snav_1_time_i[4], snav_1_time_i[5], snav_1_time_i[6],
                      snav_1_time_d,
                      project.files[crossing->file_id_2].survey, crossing->file_id_2, crossing->section_2, tie->snav_2,
                      snav_2_time_i[0], snav_2_time_i[1], snav_2_time_i[2], snav_2_time_i[3], snav_2_time_i[4], snav_2_time_i[5], snav_2_time_i[6],
                      snav_2_time_d,
                      avg_tie_lon, avg_tie_lat,
                      tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                      tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                      tie->dx_m, tie->dy_m, tie->dz_m);
        }
      }
    }
    fclose(ofp);

    /* get global tie results */
    snprintf(message, sizeof(message), " > Global Tie Offsets (m):  id  observed  solution  error\n");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        struct mbna_globaltie *globaltie = &section->globaltie;
        if (globaltie->status != MBNA_TIE_NONE) {

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(section->snav_lon_offset[globaltie->snav]) > 10000.0
                        || fabs(section->snav_lat_offset[globaltie->snav]) > 10000.0
                        || fabs(section->snav_z_offset[globaltie->snav]) > 10000.0) {
            globaltie->inversion_status = MBNA_INVERSION_OLD;
            globaltie->inversion_offset_x = 0.0;
            globaltie->inversion_offset_y = 0.0;
            globaltie->inversion_offset_x_m = 0.0;
            globaltie->inversion_offset_y_m = 0.0;
            globaltie->inversion_offset_z_m = 0.0;
            globaltie->dx_m = 0.0;
            globaltie->dy_m = 0.0;
            globaltie->dz_m = 0.0;
            globaltie->sigma_m = 0.0;
            globaltie->dr1_m = 0.0;
            globaltie->dr2_m = 0.0;
            globaltie->dr3_m = 0.0;
            globaltie->rsigma_m = 0.0;
          }
          else {
            globaltie->inversion_status = MBNA_INVERSION_CURRENT;
                        globaltie->inversion_offset_x = section->snav_lon_offset[globaltie->snav];
                        globaltie->inversion_offset_y = section->snav_lat_offset[globaltie->snav];
                        globaltie->inversion_offset_x_m = section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                        globaltie->inversion_offset_y_m = section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                        globaltie->inversion_offset_z_m = section->snav_z_offset[globaltie->snav];
                        globaltie->dx_m = globaltie->offset_x_m - globaltie->inversion_offset_x_m;
                        globaltie->dy_m = globaltie->offset_y_m - globaltie->inversion_offset_y_m;
                        globaltie->dz_m = globaltie->offset_z_m - globaltie->inversion_offset_z_m;
                        globaltie->sigma_m = sqrt(globaltie->dx_m * globaltie->dx_m + globaltie->dy_m * globaltie->dy_m + globaltie->dz_m * globaltie->dz_m);
                        globaltie->dr1_m = globaltie->inversion_offset_x_m / globaltie->sigmar1;
                        globaltie->dr2_m = globaltie->inversion_offset_y_m / globaltie->sigmar2;
                        globaltie->dr3_m = globaltie->inversion_offset_z_m / globaltie->sigmar3;
                        globaltie->rsigma_m = sqrt(globaltie->dr1_m * globaltie->dr1_m + globaltie->dr2_m * globaltie->dr2_m + globaltie->dr3_m * globaltie->dr3_m);
          }
          snprintf(message, sizeof(message), 
                  " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
                  ifile, isection, globaltie->snav, globaltie->status,
                  globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m,
                  globaltie->inversion_offset_x_m, globaltie->inversion_offset_y_m, globaltie->inversion_offset_z_m,
                  globaltie->dx_m, globaltie->dy_m, globaltie->dz_m);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* write updated project */
    project.inversion_status = MBNA_INVERSION_CURRENT;
        project.modelplot_uptodate = false;
    project.grid_status = MBNA_GRID_OLD;
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
    project.save_count = 0;

    /* deallocate arrays */
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_quality, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_num_ties, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_chunk, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_time_d, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_center, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&u, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&v, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&se, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&b, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.nia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.ia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.a, &error);

    /* turn off message dialog */
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  /* copy back scalar/summary fields to the caller's project (the array
      fields are already shared, in-place-mutated heap blocks) */
  *project_ptr = project;

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_updategrid(int verbose, struct mbna_project *project_ptr) {
  mbna_verbose = verbose;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* operate on the caller's project via the shared global (matches the
      mbnavadjust_autopick precedent) - a shallow copy is sufficient since
      the mutable file/section/crossing arrays are heap blocks referenced by
      pointer, so mutations through the global are visible through
      *project_ptr as well; scalar/summary fields are copied back explicitly
      below. */
  project = *project_ptr;

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  mb_pathplus npath;
  mb_pathplus apath;
  mb_pathplus spath;
  mb_pathplus command;
  FILE *nfp, *afp;
  char *result;
  mb_command buffer;
  int nscan;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double speed;
  double draft;
  double roll;
  double pitch;
  double heave;
  double factor;
  double zoffset;
  mb_pathplus ostring;
  int isection, isnav;
  double seconds;
  double lon_min, lon_max, lat_min, lat_max;

  /* generate current topography grid */
  if (project.open && project.num_files > 0 && error == MB_ERROR_NO_ERROR) {
    /* set message */
    snprintf(message, sizeof(message), "Setting up to generate current topography grid...");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s\n", message);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);

    /* update datalist files and mbgrid commands */
    snprintf(apath, sizeof(apath), "%s/datalist.mb-1", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        for (int j = 0; j < file->num_sections; j++) {
          fprintf(afp, "nvs_%4.4d_%4.4d.mb71 71\n", file->id, j);
        }
      }
      fclose(afp);
    }
    for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
      snprintf(apath, sizeof(apath), "%s/datalist_%4.4d.mb-1", project.datadir, isurvey);
      if ((afp = fopen(apath, "w")) != NULL) {
        for (int ifile = 0; ifile < project.num_files; ifile++) {
          if (project.files[ifile].survey == isurvey) {
            file = &project.files[ifile];
            for (int j = 0; j < file->num_sections; j++) {
              fprintf(afp, "nvs_%4.4d_%4.4d.mb71 71\n", file->id, j);
            }
          }
        }
        fclose(afp);
      }
    }

    double dlon = 0.1 * (project.lon_max - project.lon_min);
    double dlat = 0.1 * (project.lat_max - project.lat_min);
    lon_min = project.lon_min - dlon;
    lon_max = project.lon_max + dlon;
    lat_min = project.lat_min - dlat;
    lat_max = project.lat_max + dlat;
    snprintf(apath, sizeof(apath), "%s/mbgrid_adj.cmd", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      fprintf(afp, "mbgrid -I datalistp.mb-1 \\\n\t-R%.8f/%.8f/%.8f/%.8f \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopoAdj\n\n",
              lon_min, lon_max, lat_min, lat_max);

      for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
        bool first_file = true;
        for (int ifile = 0; ifile < project.num_files; ifile++) {
          if (project.files[ifile].survey == isurvey) {
            for (int isection=0; isection < project.files[ifile].num_sections; isection++) {
              if (first_file && isection == 0) {
                first_file = false;
                lon_min = project.files[ifile].sections[isection].lonmin;
                lon_max = project.files[ifile].sections[isection].lonmax;
                lat_min = project.files[ifile].sections[isection].latmin;
                lat_max = project.files[ifile].sections[isection].latmax;
              } else {
                lon_min = MIN(project.files[ifile].sections[isection].lonmin, lon_min);
                lon_max = MAX(project.files[ifile].sections[isection].lonmax, lon_max);
                lat_min = MIN(project.files[ifile].sections[isection].latmin, lat_min);
                lat_max = MAX(project.files[ifile].sections[isection].latmax, lat_max);
              }
            }
          }
        }
        lon_min -= dlon;
        lon_max += dlon;
        lat_min -= dlat;
        lat_max += dlat;
        fprintf(afp, "mbgrid -I datalist_%4.4dp.mb-1 \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopoAdj_%4.4d\n\n",
                isurvey, isurvey);
      }
      fclose(afp);
    }

    snprintf(command, sizeof(command), "chmod +x %s/mbgrid_adj.cmd", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);

    /* run mbdatalist to create datalistp.mb-1, update ancillary files,
        and clear any processing lock files */
    snprintf(message, sizeof(message), " > Running mbdatalist in project\n");
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
    snprintf(command, sizeof(command), "cd %s ; mbdatalist -Idatalist.mb-1 -O -Y -Z -V", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);
    for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
      snprintf(command, sizeof(command), "cd %s ; mbdatalist -Idatalist_%4.4d.mb-1 -Z -V", project.datadir, isurvey);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
    }

    if (project.inversion_status != MBNA_INVERSION_NONE) {
      /* set message */
      snprintf(message, sizeof(message), "Applying navigation solution within the project...");
      if (mbna_verbose == 0)
        fprintf(stderr, "%s\n", message);
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);

      /* generate new nav files */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        snprintf(npath, sizeof(npath), "%s/nvs_%4.4d.mb166", project.datadir, ifile);
        snprintf(apath, sizeof(apath), "%s/nvs_%4.4d.na0", project.datadir, ifile);
        if ((nfp = fopen(npath, "r")) == NULL) {
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          snprintf(message, sizeof(message), " > Unable to read initial nav file %s\n", npath);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", message);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else if ((afp = fopen(apath, "w")) == NULL) {
          fclose(nfp);
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          snprintf(message, sizeof(message), " > Unable to open output nav file %s\n", apath);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", message);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else {
          snprintf(message, sizeof(message), " > Output updated nav to %s\n", apath);
          if (mbna_verbose > 0)
            fprintf(stderr, "%s\n", message);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);

          /* write file header */
          char user[256], host[256], date[32];
          status = mb_user_host_date(mbna_verbose, user, host, date, &error);
          gethostname(host, 256);
          snprintf(ostring, sizeof(ostring), "# Adjusted navigation generated using MBnavadjust\n");
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MB-System version:        %s\n", MB_VERSION);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MB-System build data:     %s\n", MB_VERSION_DATE);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project name: %s\n", project.name);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project path: %s\n", project.path);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project home: %s\n", project.home);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
          fprintf(afp, "%s", ostring);

          /* read the input nav */
          isection = 0;
          section = &file->sections[isection];
          isnav = 0;
          bool done = false;
          while (!done) {
            if ((result = fgets(buffer, sizeof(buffer), nfp)) != buffer) {
              done = true;
            }
            else if ((nscan = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0],
                                     &time_i[1], &time_i[2], &time_i[3], &time_i[4], &seconds, &time_d, &navlon,
                                     &navlat, &heading, &speed, &draft, &roll, &pitch, &heave)) >= 11) {
              /* get integer seconds and microseconds */
              time_i[5] = (int)floor(seconds);
              time_i[6] = (int)((seconds - (double)time_i[5]) * 1000000);

              /* fix nav from early version */
              if (nscan < 15) {
                draft = 0.0;
                roll = 0.0;
                pitch = 0.0;
                heave = 0.0;
              }

              /* get next snav interval if needed */
              while (time_d > section->snav_time_d[isnav + 1] &&
                     !(isection == file->num_sections - 1 && isnav == section->num_snav - 2)) {
                if (isnav < section->num_snav - 2) {
                  isnav++;
                }
                else if (isection < file->num_sections - 1) {
                  isection++;
                  section = &file->sections[isection];
                  isnav = 0;
                }
              }

              /* update the nav if possible (and it should be...) */
              if (time_d < section->snav_time_d[isnav]) {
                factor = 0.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
//ifile, isection, isnav);
              }
              else if (time_d > section->snav_time_d[isnav + 1]) {
                factor = 1.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
//ifile, isection, isnav+1);
              }
              else {
                if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                  factor = (time_d - section->snav_time_d[isnav]) /
                           (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
                }
                else {
                  factor = 0.0;
                }
              }

              /* update and output only nonzero navigation */
              if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001) {
                navlon += section->snav_lon_offset[isnav] +
                          factor * (section->snav_lon_offset[isnav + 1] - section->snav_lon_offset[isnav]);
                navlat += section->snav_lat_offset[isnav] +
                          factor * (section->snav_lat_offset[isnav + 1] - section->snav_lat_offset[isnav]);
                zoffset = section->snav_z_offset[isnav] +
                          factor * (section->snav_z_offset[isnav + 1] - section->snav_z_offset[isnav]);

                /* write the updated nav out */
                snprintf(ostring, sizeof(ostring), 
                        "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f "
                        "%.2f %.3f\r\n",
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d,
                        navlon, navlat, heading, speed, draft, roll, pitch, heave, zoffset);
                fprintf(afp, "%s", ostring);
                /* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring);
                 */
              }
            }
          }
          fclose(nfp);
          fclose(afp);

          /* update output file in mbprocess parameter file for each section file */
          for (int isection = 0; isection < file->num_sections; isection++) {
            section = &(file->sections[isection]);

            snprintf(spath, sizeof(spath), "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, isection);

            status = mb_pr_update_format(mbna_verbose, spath, true, 71, &error);
            status = mb_pr_update_navadj(mbna_verbose, spath, MBP_NAVADJ_LLZ, apath, MBP_NAV_LINEAR, &error);
          }
        }
      }

      /* run mbprocess */
      snprintf(message, sizeof(message), " > Running mbprocess in project\n");
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      snprintf(command, sizeof(command), "cd %s ; mbprocess -C4", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
    }

    if (project.grid_status != MBNA_GRID_CURRENT) {
      /* run mbgrid */
      snprintf(message, sizeof(message), " > Running mbgrid_adj\n");
      if (mbna_verbose > 0)
        fprintf(stderr, "%s\n", message);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      snprintf(command, sizeof(command), "cd %s ; ./mbgrid_adj.cmd", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
      project.grid_status = MBNA_GRID_CURRENT;

      /* write current project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;

      /* turn off message dialog */
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  /* copy back scalar/summary fields to the caller's project (the array
      fields are already shared, in-place-mutated heap blocks) */
  *project_ptr = project;

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_applynav(int verbose, struct mbna_project *project_ptr) {
  mbna_verbose = verbose;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* operate on the caller's project via the shared global (matches the
      mbnavadjust_autopick precedent) - a shallow copy is sufficient since
      the mutable file/section/crossing arrays are heap blocks referenced by
      pointer, so mutations through the global are visible through
      *project_ptr as well; scalar/summary fields are copied back explicitly
      below. */
  project = *project_ptr;

  int status = MB_SUCCESS;

  /* output results from navigation solution */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      error == MB_ERROR_NO_ERROR) {

    mb_pathplus npath;
    mb_pathplus opath;
    mb_path ppath;
    FILE *nfp, *ofp;

    /* now output inverse solution */
    snprintf(message, sizeof(message), "Applying navigation solution...");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s\n", message);

    /* generate new nav files */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      struct mbna_file *file = &project.files[ifile];

      snprintf(npath, sizeof(npath), "%s/nvs_%4.4d.mb166", project.datadir, ifile);
      if (project.use_mode == MBNA_USE_MODE_PRIMARY) {
        snprintf(opath, sizeof(opath), "%s.na%d", file->path, 0);
      }
      else {
        status = mb_pr_get_output(mbna_verbose, &file->format, file->path, ppath, &error);
        if (project.use_mode == MBNA_USE_MODE_SECONDARY) {
          snprintf(opath, sizeof(ppath), "%s.na%d", ppath, 1);
        }
        else {
          snprintf(opath, sizeof(ppath), "%s.na%d", ppath, 2);
        }
      }
      if ((nfp = fopen(npath, "r")) == NULL) {
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        snprintf(message, sizeof(message), " > Unable to read initial nav file %s\n", npath);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", message);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else if ((ofp = fopen(opath, "w")) == NULL) {
        fclose(nfp);
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        snprintf(message, sizeof(message), " > Unable to open output nav file %s\n", opath);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", message);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else {
        snprintf(message, sizeof(message), " > Output updated nav to %s\n", opath);
        if (mbna_verbose > 0)
          fprintf(stderr, "%s\n", message);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);

        /* write file header */
        char user[256], host[256], date[32];
        mb_pathplus ostring;
        status = mb_user_host_date(mbna_verbose, user, host, date, &error);
        snprintf(ostring, sizeof(ostring), "# Adjusted navigation generated using MBnavadjust\n");
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MB-System version:        %s\n", MB_VERSION);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MB-System build data:     %s\n", MB_VERSION_DATE);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project name: %s\n", project.name);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project path: %s\n", project.path);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project home: %s\n", project.home);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(ofp, "%s", ostring);

        /* read the input nav */
        int isection = 0;
        struct mbna_section *section = &file->sections[isection];
        int isnav = 0;
        bool done = false;
        char *result = NULL;
        mb_command buffer;
        int nscan;
        int time_i[7];
        double seconds;
        double time_d;
        double navlon;
        double navlat;
        double heading;
        double speed;
        double draft;
        double roll;
        double pitch;
        double heave;
        double zoffset;
        while (!done) {
          if ((result = fgets(buffer, sizeof(buffer), nfp)) != buffer) {
            done = true;
          }
          else if ((nscan = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0],
                                   &time_i[1], &time_i[2], &time_i[3], &time_i[4], &seconds, &time_d, &navlon, &navlat,
                                   &heading, &speed, &draft, &roll, &pitch, &heave)) >= 11) {
            /* get integer seconds and microseconds */
            time_i[5] = (int)floor(seconds);
            time_i[6] = (int)((seconds - (double)time_i[5]) * 1000000);

            /* fix nav from early version */
            if (nscan < 15) {
              draft = 0.0;
              roll = 0.0;
              pitch = 0.0;
              heave = 0.0;
            }

            /* get next snav interval if needed */
            while (time_d > section->snav_time_d[isnav + 1] &&
                   !(isection == file->num_sections - 1 && isnav == section->num_snav - 2)) {
              if (isnav < section->num_snav - 2) {
                isnav++;
              }
              else if (isection < file->num_sections - 1) {
                isection++;
                section = &file->sections[isection];
                isnav = 0;
              }
            }

            /* update the nav if possible (and it should be...) */
            double factor;
            if (time_d < section->snav_time_d[isnav]) {
              factor = 0.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
//ifile, isection, isnav);
            }
            else if (time_d > section->snav_time_d[isnav + 1]) {
              factor = 1.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
//ifile, isection, isnav+1);
            }
            else {
              if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                factor = (time_d - section->snav_time_d[isnav]) /
                         (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
              }
              else {
                factor = 0.0;
              }
            }

            /* update and output only nonzero navigation */
            if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001) {
              navlon += section->snav_lon_offset[isnav] +
                        factor * (section->snav_lon_offset[isnav + 1] - section->snav_lon_offset[isnav]);
              navlat += section->snav_lat_offset[isnav] +
                        factor * (section->snav_lat_offset[isnav + 1] - section->snav_lat_offset[isnav]);
              zoffset = section->snav_z_offset[isnav] +
                        factor * (section->snav_z_offset[isnav + 1] - section->snav_z_offset[isnav]);

              /* write the updated nav out */
              snprintf(ostring, sizeof(ostring), 
                      "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f %.2f "
                      "%.3f\r\n",
                      time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d, navlon,
                      navlat, heading, speed, draft, roll, pitch, heave, zoffset);
              fprintf(ofp, "%s", ostring);
              /* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring); */
            }
          }
        }
        fclose(nfp);
        fclose(ofp);

        if (project.use_mode == MBNA_USE_MODE_PRIMARY) {
          int mbp_heading_mode;
          double mbp_headingbias;
          int mbp_rollbias_mode;
          double mbp_rollbias;
          double mbp_rollbias_port;
          double mbp_rollbias_stbd;

          /* get bias values */
          mb_pr_get_heading(mbna_verbose, file->path, &mbp_heading_mode, &mbp_headingbias, &error);
          mb_pr_get_rollbias(mbna_verbose, file->path, &mbp_rollbias_mode, &mbp_rollbias, &mbp_rollbias_port,
                             &mbp_rollbias_stbd, &error);

          /* update output file in mbprocess parameter file */
          status = mb_pr_update_format(mbna_verbose, file->path, true, file->format, &error);
          status = mb_pr_update_navadj(mbna_verbose, file->path, MBP_NAVADJ_LLZ, opath, MBP_NAV_LINEAR, &error);

          /* update heading bias in mbprocess parameter file */
          mbp_headingbias = file->heading_bias + file->heading_bias_import;
          if (mbp_headingbias == 0.0) {
            if (mbp_heading_mode == MBP_HEADING_OFF || mbp_heading_mode == MBP_HEADING_OFFSET)
              mbp_heading_mode = MBP_HEADING_OFF;
            else if (mbp_heading_mode == MBP_HEADING_CALC || mbp_heading_mode == MBP_HEADING_CALCOFFSET)
              mbp_heading_mode = MBP_HEADING_CALC;
          }
          else {
            if (mbp_heading_mode == MBP_HEADING_OFF || mbp_heading_mode == MBP_HEADING_OFFSET)
              mbp_heading_mode = MBP_HEADING_OFFSET;
            else if (mbp_heading_mode == MBP_HEADING_CALC || mbp_heading_mode == MBP_HEADING_CALCOFFSET)
              mbp_heading_mode = MBP_HEADING_CALCOFFSET;
          }
          status = mb_pr_update_heading(mbna_verbose, file->path, mbp_heading_mode, mbp_headingbias, &error);

          /* update roll bias in mbprocess parameter file */
          mbp_rollbias = file->roll_bias + file->roll_bias_import;
          if (mbp_rollbias == 0.0) {
            if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
              mbp_rollbias_port = mbp_rollbias + mbp_rollbias_port - file->roll_bias_import;
              mbp_rollbias_stbd = mbp_rollbias + mbp_rollbias_stbd - file->roll_bias_import;
            }
            else
              mbp_rollbias_mode = MBP_ROLLBIAS_OFF;
          }
          else {
            if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
              mbp_rollbias_port = mbp_rollbias + mbp_rollbias_port - file->roll_bias_import;
              mbp_rollbias_stbd = mbp_rollbias + mbp_rollbias_stbd - file->roll_bias_import;
            }
            else {
              mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
            }
          }
          status = mb_pr_update_rollbias(mbna_verbose, file->path, mbp_rollbias_mode, mbp_rollbias, mbp_rollbias_port,
                                         mbp_rollbias_stbd, &error);
        }
      }
    }

    /* turn off message dialog */
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  /* copy back scalar/summary fields to the caller's project (the array
      fields are already shared, in-place-mutated heap blocks) */
  *project_ptr = project;

  return (status);
}
