/*--------------------------------------------------------------------
 *    The MB-system:  mb_truecont.c  4/21/94
 *
 *    Copyright (c) 1994-2025 by
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
 * mb_truecontour.c contours a block of multibeam bathymetry data,
 * dealing correctly with beams in arbitrary locations by forming
 * a delauney triangle network and then contouring that network.
 *
 * Author:  D. W. Caress
 * Date:  April, 1994
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

#define IMOVE 3
#define IDRAW 2
#define ISTROKE -2
#define IOR -3
const double EPS = 0.0001;
#define NUM_BEAMS_ALLOC_MIN 16

/*--------------------------------------------------------------------------*/
/*   function mb_contour_init initializes the memory required to
    contour multibeam bathymetry data.
    if mbio_ptr is null, the arrays are allocated using mb_mallocd. If
    mbio_ptr is a valid mbio structure, then the arrays tied to
    beams_bath will be registered using mb_register_array */
int mb_contour_init(int verbose, struct swath **data, int npings_max, int beams_bath, int contour_algorithm, int plot_contours,
                    int plot_triangles, int plot_track, int plot_name, int plot_pingnumber, double contour_int, double color_int,
                    double tick_int, double label_int, double tick_len, double label_hgt, double label_spacing, int ncolor,
                    int nlevel, double *level_list, int *label_list, int *tick_list, double time_tick_int, double time_annot_int,
                    double date_annot_int, double time_tick_len, double name_hgt, int pingnumber_tick_int,
                    int pingnumber_annot_int, double pingnumber_tick_len, void (*contour_plot)(double, double, int),
                    void (*contour_newpen)(int), void (*contour_setline)(int),
                    void (*contour_justify_string)(double, char *, double *),
                    void (*contour_plot_string)(double, double, double, double, char *), int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       data:                 %p\n", data);
    fprintf(stderr, "dbg2       npings_max:           %d\n", npings_max);
    fprintf(stderr, "dbg2       beams_bath:           %d\n", beams_bath);
    fprintf(stderr, "dbg2       contour algorithm:    %d\n", contour_algorithm);
    fprintf(stderr, "dbg2       plot contours:        %d\n", plot_contours);
    fprintf(stderr, "dbg2       plot triangles:       %d\n", plot_triangles);
    fprintf(stderr, "dbg2       plot track:           %d\n", plot_track);
    fprintf(stderr, "dbg2       plot name:            %d\n", plot_name);
    fprintf(stderr, "dbg2       plot pingnumber:      %d\n", plot_pingnumber);
    fprintf(stderr, "dbg2       contour interval:     %f\n", contour_int);
    fprintf(stderr, "dbg2       color interval:       %f\n", color_int);
    fprintf(stderr, "dbg2       tick interval:        %f\n", tick_int);
    fprintf(stderr, "dbg2       label interval:       %f\n", label_int);
    fprintf(stderr, "dbg2       tick length:          %f\n", tick_len);
    fprintf(stderr, "dbg2       label height:         %f\n", label_hgt);
    fprintf(stderr, "dbg2       label spacing:        %f\n", label_spacing);
    fprintf(stderr, "dbg2       number of colors:     %d\n", ncolor);
    fprintf(stderr, "dbg2       number of levels:     %d\n", nlevel);
    for (int i = 0; i < nlevel; i++)
      fprintf(stderr, "dbg2       level %d: %f %d %d\n", i, level_list[i], label_list[i], tick_list[i]);
    fprintf(stderr, "dbg2       time tick int:        %f\n", time_tick_int);
    fprintf(stderr, "dbg2       time interval:        %f\n", time_annot_int);
    fprintf(stderr, "dbg2       date interval:        %f\n", date_annot_int);
    fprintf(stderr, "dbg2       time tick length:     %f\n", time_tick_len);
    fprintf(stderr, "dbg2       name height:          %f\n", name_hgt);
    fprintf(stderr, "dbg2       pingnumber tick int:  %d\n", pingnumber_tick_int);
    fprintf(stderr, "dbg2       pingnumber annot int: %d\n", pingnumber_annot_int);
    fprintf(stderr, "dbg2       pingnumber tick len:  %f\n", pingnumber_tick_len);
    fprintf(stderr, "dbg2       contour_plot():       %p\n", contour_plot);
    fprintf(stderr, "dbg2       contour_newpen():     %p\n", contour_newpen);
    fprintf(stderr, "dbg2       contour_setline():    %p\n", contour_setline);
    fprintf(stderr, "dbg2       contour_justify_string():     %p\n", contour_justify_string);
    fprintf(stderr, "dbg2       contour_plot_string():     %p\n", contour_plot_string);
  }

  /* allocate memory for swath structure */
  int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct swath), (void **)data, error);
  memset(*data, 0, sizeof(struct swath));

  /* set variables and allocate memory for bathymetry data */
  struct swath *dataptr = *data;
  dataptr->npings = 0;
  dataptr->npings_max = npings_max;
  dataptr->beams_bath = beams_bath;
  status &= mb_mallocd(verbose, __FILE__, __LINE__, npings_max * sizeof(struct ping), (void **)&(dataptr->pings), error);
  memset(dataptr->pings, 0, npings_max * sizeof(struct ping));
  for (int i = 0; i < npings_max; i++) {
    struct ping *ping = &dataptr->pings[i];
    ping->beams_bath = 0;
    ping->beams_bath_alloc = MAX(beams_bath, NUM_BEAMS_ALLOC_MIN);
    ping->beamflag = NULL;
    ping->bath = NULL;
    ping->bathlon = NULL;
    ping->bathlat = NULL;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(char), (void **)&(ping->beamflag), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(double), (void **)&(ping->bath), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(double), (void **)&(ping->bathlon), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(double), (void **)&(ping->bathlat), error);
    if (contour_algorithm == MB_CONTOUR_TRIANGLES) {
      ping->bflag[0] = NULL;
      ping->bflag[1] = NULL;
    }
    else {
      ping->bflag[0] = NULL;
      ping->bflag[1] = NULL;
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(int), (void **)&(ping->bflag[0]), error);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ping->beams_bath_alloc * sizeof(int), (void **)&(ping->bflag[1]), error);
    }
  }

  /* set controls on what gets plotted */
  dataptr->contour_algorithm = contour_algorithm;
  dataptr->plot_contours = plot_contours;
  dataptr->plot_triangles = plot_triangles;
  dataptr->plot_track = plot_track;
  dataptr->plot_name = plot_name;
  dataptr->plot_pingnumber = plot_pingnumber;

  /* set variables and allocate memory for contour controls */
  dataptr->contour_int = contour_int;
  dataptr->color_int = color_int;
  dataptr->tick_int = tick_int;
  dataptr->label_int = label_int;
  dataptr->tick_len = tick_len;
  dataptr->label_hgt = label_hgt;
  if (label_spacing > 0.0)
    dataptr->label_spacing = label_spacing;
  else
    dataptr->label_spacing = label_hgt;
  dataptr->ncolor = ncolor;
  dataptr->nlevel = nlevel;
  dataptr->nlevelset = false;
  dataptr->level_list = NULL;
  dataptr->label_list = NULL;
  dataptr->tick_list = NULL;
  dataptr->color_list = NULL;
  if (nlevel > 0) {
    dataptr->nlevelset = true;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(double), (void **)&(dataptr->level_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(int), (void **)&(dataptr->label_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(int), (void **)&(dataptr->tick_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(int), (void **)&(dataptr->color_list), error);
    for (int i = 0; i < nlevel; i++) {
      dataptr->level_list[i] = level_list[i];
      dataptr->label_list[i] = label_list[i];
      dataptr->tick_list[i] = tick_list[i];
      dataptr->color_list[i] = i;
    }
  }

  /* set variables and allocate memory for track controls */
  dataptr->time_tick_int = time_tick_int;
  dataptr->time_annot_int = time_annot_int;
  dataptr->date_annot_int = date_annot_int;
  dataptr->time_tick_len = time_tick_len;
  dataptr->name_hgt = name_hgt;

  /* set variables and allocate memory for pingnumber control parameters */
  dataptr->pingnumber_tick_int = pingnumber_tick_int;
  dataptr->pingnumber_annot_int = pingnumber_annot_int;
  dataptr->pingnumber_tick_len = pingnumber_tick_len;

  /* set variables and allocate memory for triangle network */
  dataptr->npts = 0;
  dataptr->edge = NULL;
  dataptr->pingid = NULL;
  dataptr->beamid = NULL;
  dataptr->ntri = 0;
  dataptr->ntri_alloc = 0;
  for (int i = 0; i < 3; i++) {
    dataptr->iv[i] = NULL;
    dataptr->ct[i] = NULL;
    dataptr->cs[i] = NULL;
    dataptr->ed[i] = NULL;
    dataptr->flag[i] = NULL;
  }
  dataptr->bath_min = 0.0;
  dataptr->bath_max = 0.0;
  dataptr->triangle_scale = 0.0;
  dataptr->x = NULL;
  dataptr->y = NULL;
  dataptr->z = NULL;
  dataptr->ndelaun_alloc = 0;
  dataptr->v1 = NULL;
  dataptr->v2 = NULL;
  dataptr->v3 = NULL;
  dataptr->istack = NULL;
  dataptr->kv1 = NULL;
  dataptr->kv2 = NULL;
  int ntri_max = 0;
  if (contour_algorithm == MB_CONTOUR_TRIANGLES) {
    dataptr->npts = 0;
    dataptr->npts_alloc = npings_max * beams_bath + 3;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(int), (void **)&(dataptr->edge), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(int), (void **)&(dataptr->pingid), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(int), (void **)&(dataptr->beamid), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(double), (void **)&(dataptr->x), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(double), (void **)&(dataptr->y), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(double), (void **)&(dataptr->z), error);
    ntri_max = 3 * npings_max * beams_bath + 1;
    dataptr->ntri = 0;
    dataptr->ntri_alloc = ntri_max;
    for (int i = 0; i < 3; i++) {
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ntri_max * sizeof(int), (void **)&(dataptr->iv[i]), error);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ntri_max * sizeof(int), (void **)&(dataptr->ct[i]), error);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ntri_max * sizeof(int), (void **)&(dataptr->cs[i]), error);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ntri_max * sizeof(int), (void **)&(dataptr->ed[i]), error);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, ntri_max * sizeof(int), (void **)&(dataptr->flag[i]), error);
    }
  }

  /* allocate memory for contour positions */
  dataptr->nsave = 0;
  dataptr->xsave = NULL;
  dataptr->ysave = NULL;
  dataptr->isave = NULL;
  dataptr->jsave = NULL;
  if (contour_algorithm == MB_CONTOUR_TRIANGLES) {
  	dataptr->nsave_alloc = (4 * ntri_max + 1);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->nsave_alloc * sizeof(double), (void **)&(dataptr->xsave), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->nsave_alloc * sizeof(double), (void **)&(dataptr->ysave), error);
  }
  else {
    dataptr->npts = 0;
    dataptr->npts_alloc = npings_max * beams_bath;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(double), (void **)&(dataptr->xsave), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(double), (void **)&(dataptr->ysave), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(int), (void **)&(dataptr->isave), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, dataptr->npts_alloc * sizeof(int), (void **)&(dataptr->jsave), error);
  }

  /* allocate memory for contour labels */
  dataptr->nlabel = 0;
  dataptr->xlabel = NULL;
  dataptr->ylabel = NULL;
  dataptr->angle = NULL;
  dataptr->justify = NULL;
  status &= mb_mallocd(verbose, __FILE__, __LINE__, (5 * npings_max) * sizeof(double), (void **)&(dataptr->xlabel), error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, (5 * npings_max) * sizeof(double), (void **)&(dataptr->ylabel), error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, (5 * npings_max) * sizeof(double), (void **)&(dataptr->angle), error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, (5 * npings_max) * sizeof(int), (void **)&(dataptr->justify), error);

  /* set plotting function pointers */
  dataptr->contour_plot = contour_plot;
  dataptr->contour_newpen = contour_newpen;
  dataptr->contour_setline = contour_setline;
  dataptr->contour_justify_string = contour_justify_string;
  dataptr->contour_plot_string = contour_plot_string;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------------*/
/*   function mb_contour_deall deallocates the memory required to
    contour multibeam bathymetry data. */
int mb_contour_deall(int verbose, struct swath *data, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       data:                    %p\n", data);
  }

  int status = MB_SUCCESS;

  /* deallocate memory for bathymetry data */
  if (data->npings_max > 0) {
    for (int i = 0; i < data->npings_max; i++) {
      struct ping *ping = &data->pings[i];
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->beamflag, error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->bath, error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->bathlon, error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->bathlat, error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->bflag[0], error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ping->bflag[1], error);
    }
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->pings, error);
    data->npings_max = 0;
  }
  data->npings = 0;
  data->beams_bath = 0;

  /* deallocate memory for contour controls */
  data->contour_algorithm = 0;
  data->plot_contours = 0;
  data->plot_triangles = 0;
  data->plot_track = 0;
  data->plot_name = 0;
  data->plot_pingnumber = 0;
  data->contour_int = 0.0;
  data->color_int = 0.0;
  data->tick_int = 0.0;
  data->label_int = 0.0;
  data->tick_len = 0.0;
  data->label_hgt = 0.0;
  data->label_spacing = 0.0;
  if (data->nlevel > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->level_list, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->label_list, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->tick_list, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->color_list, error);
    data->nlevel = 0;
  }
  data->ncolor = 0;
  data->nlevelset = 0;

  data->time_tick_int = 0.0;
  data->time_annot_int = 0.0;
  data->date_annot_int = 0.0;
  data->time_tick_len = 0.0;
  data->name_hgt = 0.0;
  data->pingnumber_tick_int = 0;
  data->pingnumber_annot_int = 0;
  data->pingnumber_tick_len = 0.0;

  /* deallocate memory for triangle network */
  if (data->ntri_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->edge, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->pingid, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->beamid, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->x, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->y, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->z, error);
    data->npts_alloc = 0;
  }
  data->npts = 0;

  if (data->ntri_alloc > 0) {
    for (int i = 0; i < 3; i++) {
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->iv[i], error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->ct[i], error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->cs[i], error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->ed[i], error);
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->flag[i], error);
    }
    data->ntri_alloc = 0;
  }
  data->ntri = 0;
  data->bath_min = 0.0;
  data->bath_max = 0.0;

  if (data->ndelaun_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->v1, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->v2, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->v3, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->istack, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->kv1, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->kv2, error);
    data->ndelaun_alloc = 0;
  }

  /* deallocate memory for contour positions */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->xsave, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->ysave, error);
  if (data->contour_algorithm != MB_CONTOUR_TRIANGLES) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->isave, error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->jsave, error);
  }

  /* deallocate memory for contour labels */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->xlabel, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->ylabel, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->angle, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->justify, error);

  /* deallocate memory for swath structure */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (MB_SUCCESS);
}
/*--------------------------------------------------------------------------*/
/*   function get_start_tri finds next contour starting point. */
int get_start_tri(struct swath *data, int *itri, int *iside1, int *iside2, int *closed) {
  /* search triangles */
  *closed = false;
  for (int i = 0; i < data->ntri; i++)
    for (int j = 0; j < 3; j++) {
      if (data->flag[j][i] > 0) {
        /* find two flagged sides */
        *itri = i;
        *iside1 = j;
        *iside2 = -1;
        for (int jj = 0; jj < 3; jj++)
          if (jj != j && data->flag[jj][i] > 0)
            *iside2 = jj;
        if (*iside2 == -1) {
          fprintf(stderr, "no flagged side in get_start_tri???\n");
          fprintf(stderr, "noflag: itri:%d flags: %d %d %d\n", *itri, data->flag[0][*itri], data->flag[1][*itri],
                  data->flag[2][*itri]);
        }

        /* check if contour continues on both sides */
        if (data->ct[*iside1][i] > -1 && data->ct[*iside2][i] > -1)
          *closed = true;

        /* else make sure contour starts at dead end */
        else if (data->ct[*iside1][i] > -1) {
          const int isave = *iside1;
          *iside1 = *iside2;
          *iside2 = isave;
          *closed = false;
        }
        else
          *closed = false;
        return (true);
      }
    }

  /* nothing found */
  return (false);
}
/*--------------------------------------------------------------------------*/
/*   function get_next_tri finds next contour component if it exists */
int get_next_tri(struct swath *data, int *itri, int *iside1, int *iside2, int *closed, int *itristart, int *isidestart) {
  /* check if contour ends where it began */
  if (*closed && data->ct[*iside2][*itri] == *itristart && data->cs[*iside2][*itri] == *isidestart)
    return (false);

  /* check if current triangle side connects to another */
  else if (data->ct[*iside2][*itri] > -1) {
    *iside1 = data->cs[*iside2][*itri];
    *itri = data->ct[*iside2][*itri];
    *iside2 = -1;
    for (int j = 0; j < 3; j++)
      if (j != *iside1 && data->flag[j][*itri] != 0)
        *iside2 = j;
    if (*iside2 == -1) {
      fprintf(stderr, "no flagged side in get_next_tri???\n");
      fprintf(stderr, "noflag: itri:%d flags: %d %d %d\n", *itri, data->flag[0][*itri], data->flag[1][*itri],
              data->flag[2][*itri]);
      return (false);
    }
    return (true);
  }

  /* else if contour ends but closed set true then
      turn contour around and continue in other direction */
  else if (*closed) {
    for (int i = 0; i < data->nsave / 2; i++) {
      const double xs = data->xsave[i];
      const double ys = data->ysave[i];
      data->xsave[i] = data->xsave[data->nsave - i - 1];
      data->ysave[i] = data->ysave[data->nsave - i - 1];
      data->xsave[data->nsave - i - 1] = xs;
      data->ysave[data->nsave - i - 1] = ys;
    }
    *closed = false;
    data->nsave--;
    const int itrisave = *itristart;
    const int isidesave = *isidestart;
    *itristart = *itri;
    *isidestart = *iside2;
    *itri = itrisave;
    *iside2 = isidesave;
    *iside1 = -1;
    for (int j = 0; j < 3; j++)
      if (j != *iside2 && data->flag[j][*itri] != 0)
        *iside1 = j;

    /* if next side not found end contour */
    if (*iside1 == -1)
      return (false);

    /* else keep going */
    return (true);
  }

  /* else contour ends and is not closed */
  else
    return (false);
}
/*--------------------------------------------------------------------------*/
/*   function get_pos_tri finds position of contour crossing point */
int get_pos_tri(const struct swath *data, double eps, int itri, int iside, double value, double *x, double *y) {
  const int v1 = iside;
  int v2 = iside + 1;
  if (v2 == 3)
    v2 = 0;
  const int ipt1 = data->iv[v1][itri];
  const int ipt2 = data->iv[v2][itri];
  double factor;
  if (fabs(data->z[ipt2] - data->z[ipt1]) > eps)
    factor = (value - data->z[ipt1]) / (data->z[ipt2] - data->z[ipt1]);
  else
    factor = 0.5;
  *x = data->x[ipt1] + factor * (data->x[ipt2] - data->x[ipt1]);
  *y = data->y[ipt1] + factor * (data->y[ipt2] - data->y[ipt1]);

  return (true);
}
/*--------------------------------------------------------------------------*/
/*   function get_azimuth_tri gets azimuth across track for a label */
int get_azimuth_tri(const struct swath *data, int itri, int iside, double *angle) {
  *angle = -data->pings[data->pingid[data->iv[iside][itri]]].heading;
  if (*angle > 180.0)
    *angle = *angle - 360.0;
  if (*angle < -180.0)
    *angle = *angle + 360.0;

  return (true);
}
/*--------------------------------------------------------------------------*/
/*   function check_label checks if new label will overwrite any recent
 *  labels. */
int check_label(struct swath *data, int nlab) {
#define MAXHIS 30
  static double xlabel_his[MAXHIS];
  static double ylabel_his[MAXHIS];
  static int nlabel_his = 0;

  int good = 1;
  int ilab = 0;
  double rad_label_his = data->label_spacing;
  while (good && ilab < nlabel_his) {
    const double dx = xlabel_his[ilab] - data->xlabel[nlab];
    const double dy = ylabel_his[ilab] - data->ylabel[nlab];
    const double rr = sqrt(dx * dx + dy * dy);
    if (rr < rad_label_his)
      good = 0;
    ilab++;
  }
  ilab--;
  if (good) {
    nlabel_his++;
    if (nlabel_his >= MAXHIS)
      nlabel_his = MAXHIS - 1;
    for (int i = nlabel_his; i > 0; i--) {
      xlabel_his[i] = xlabel_his[i - 1];
      ylabel_his[i] = ylabel_his[i - 1];
    }
    xlabel_his[0] = data->xlabel[nlab];
    ylabel_his[0] = data->ylabel[nlab];
  }
  return (good);
}
/*--------------------------------------------------------------------------*/
/*   function dump_contour dumps the contour stored in xsave and ysave
 *  to the plotting routines */
int dump_contour(struct swath *data, double value) {
  /* plot the contours */
  if (data->nsave < 2)
    return (false);
  data->contour_plot(data->xsave[0], data->ysave[0], IMOVE);
  for (int i = 1; i < data->nsave - 1; i++)
    data->contour_plot(data->xsave[i], data->ysave[i], IDRAW);
  data->contour_plot(data->xsave[data->nsave - 1], data->ysave[data->nsave - 1], ISTROKE);
  data->nsave = 0;

  /* plot the labels */
  char label[25];
  sprintf(label, "  %d", (int)value);
  for (int i = 0; i < data->nlabel; i++) {
    if (data->justify[i] == 1) {
      double mtodeglon;
      double mtodeglat;
      mb_coor_scale(0, data->ylabel[i], &mtodeglon, &mtodeglat);
      double s[4];
      data->contour_justify_string(data->label_hgt, label, s);
      const double dx = 1.5 * s[2] * cos(DTR * data->angle[i]);
      const double dy = 1.5 * mtodeglat / mtodeglon * s[2] * sin(DTR * data->angle[i]);
      const double x = data->xlabel[i] - dx;
      const double y = data->ylabel[i] - dy;
      data->contour_plot_string(x, y, data->label_hgt, data->angle[i], label);
    }
    else {
      data->contour_plot_string(data->xlabel[i], data->ylabel[i], data->label_hgt, data->angle[i], label);
    }
  }
  data->nlabel = 0;

  return (true);
}
/*--------------------------------------------------------------------------*/
/*  function mb_triangulate calculates a delauney triangulization of the
    swath bathymetry in data */
int mb_triangulate(int verbose, struct swath *data, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       data:                    %p\n", data);
    fprintf(stderr, "dbg2       data->contour_algorithm: %d\n", data->contour_algorithm);
    fprintf(stderr, "dbg2       data->plot_contours:     %d\n", data->plot_contours);
    fprintf(stderr, "dbg2       data->plot_triangles:    %d\n", data->plot_triangles);
    fprintf(stderr, "dbg2       data->plot_track:        %d\n", data->plot_track);
    fprintf(stderr, "dbg2       data->plot_name:         %d\n", data->plot_name);
    fprintf(stderr, "dbg2       data->contour_int:       %f\n", data->contour_int);
    fprintf(stderr, "dbg2       data->color_int:         %f\n", data->color_int);
    fprintf(stderr, "dbg2       data->tick_int:          %f\n", data->tick_int);
    fprintf(stderr, "dbg2       data->label_int:         %f\n", data->label_int);
    fprintf(stderr, "dbg2       data->tick_len:          %f\n", data->tick_len);
    fprintf(stderr, "dbg2       data->label_hgt:         %f\n", data->label_hgt);
    fprintf(stderr, "dbg2       data->label_spacing:     %f\n", data->label_spacing);
    fprintf(stderr, "dbg2       data->ncolor:            %d\n", data->ncolor);
    fprintf(stderr, "dbg2       data->nlevel:            %d\n", data->nlevel);
    fprintf(stderr, "dbg2       data->nlevelset:         %d\n", data->nlevelset);
    if (data->nlevelset)
      for (int i = 0; i < data->nlevel; i++) {
        fprintf(stderr, "dbg2          level[%3d]:  %f %d %d %d\n", i, data->level_list[i], data->label_list[i],
                data->tick_list[i], data->color_list[i]);
      }
    fprintf(stderr, "dbg2       data->npings:     %d\n", data->npings);
    fprintf(stderr, "dbg2       data->npings_max: %d\n", data->npings_max);
    fprintf(stderr, "dbg2       data->beams_bath: %d\n", data->beams_bath);
    for (int i = 0; i < data->npings; i++) {
      fprintf(stderr, "dbg2          ping[%4d]: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d:%6.6d %f %f %f %f %d\n", i,
              data->pings[i].time_i[0], data->pings[i].time_i[1], data->pings[i].time_i[2], data->pings[i].time_i[3],
              data->pings[i].time_i[4], data->pings[i].time_i[5], data->pings[i].time_i[6], data->pings[i].time_d,
              data->pings[i].navlon, data->pings[i].navlat, data->pings[i].heading, data->pings[i].beams_bath);
      for (int j = 0; j < data->pings[i].beams_bath; j++) {
        if (mb_beam_ok(data->pings[i].beamflag[j]))
          fprintf(stderr, "dbg2          beam[%4d:%3d]:  %2d %f %f %f\n", i, j, data->pings[i].beamflag[j],
                  data->pings[i].bath[j], data->pings[i].bathlon[j], data->pings[i].bathlat[j]);
      }
    }
  }

  /* count number of points and verify that enough memory is allocated */
  int npt_cnt = 0;
  for (int i = 0; i < data->npings; i++) {
    struct ping *ping = &data->pings[i];
    for (int j = 0; j < ping->beams_bath; j++) {
      if (mb_beam_ok(ping->beamflag[j]))
        npt_cnt++;
    }
  }

  // allocate memory as needed
  int status = MB_SUCCESS;
  const int ntri_cnt = 3 * npt_cnt + 1;
  if (npt_cnt > data->npts_alloc) {
    data->npts_alloc = npt_cnt;
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(int), (void **)&data->edge, error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(int), (void **)&data->pingid, error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(int), (void **)&data->beamid, error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(double), (void **)&data->x, error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(double), (void **)&data->y, error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->npts_alloc * sizeof(double), (void **)&data->z, error);
    memset(data->edge, 0, data->npts_alloc * sizeof(int));
    memset(data->pingid, 0, data->npts_alloc * sizeof(int));
    memset(data->beamid, 0, data->npts_alloc * sizeof(int));
    memset(data->x, 0, data->npts_alloc * sizeof(double));
    memset(data->y, 0, data->npts_alloc * sizeof(double));
    memset(data->z, 0, data->npts_alloc * sizeof(double));
  }
  if (ntri_cnt > data->ntri_alloc) {
    data->ntri_alloc = ntri_cnt;
    for (int i = 0; i < 3; i++) {
      status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->iv[i]), error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->ct[i]), error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->cs[i]), error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->ed[i]), error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->flag[i]), error);
      memset(data->iv[i], 0, ntri_cnt * sizeof(int));
      memset(data->ct[i], 0, ntri_cnt * sizeof(int));
      memset(data->cs[i], 0, ntri_cnt * sizeof(int));
      memset(data->ed[i], 0, ntri_cnt * sizeof(int));
      memset(data->flag[i], 0, ntri_cnt * sizeof(int));
    }
    status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(double), (void **)&(data->v1), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(double), (void **)&(data->v2), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(double), (void **)&(data->v3), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, ntri_cnt * sizeof(int), (void **)&(data->istack), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, 3 * ntri_cnt * sizeof(int), (void **)&(data->kv1), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, 3 * ntri_cnt * sizeof(int), (void **)&(data->kv2), error);
    data->nsave_alloc = (4 * ntri_cnt + 1);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->xsave), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->ysave), error);
    memset(data->v1, 0, ntri_cnt * sizeof(double));
    memset(data->v2, 0, ntri_cnt * sizeof(double));
    memset(data->v3, 0, ntri_cnt * sizeof(double));
    memset(data->istack, 0, ntri_cnt * sizeof(int));
    memset(data->kv1, 0, 3 * ntri_cnt * sizeof(int));
    memset(data->kv2, 0, 3 * ntri_cnt * sizeof(int));
    memset(data->xsave, 0, data->nsave_alloc * sizeof(double));
    memset(data->ysave, 0, data->nsave_alloc * sizeof(double));
  }

  /* construct list of good soundings */
  data->npts = 0;
  for (int i = 0; i < data->npings; i++) {
    struct ping *ping = &data->pings[i];

    /* find edges of ping */
    int left = ping->beams_bath / 2;
    int right = left;
    for (int j = 0; j < ping->beams_bath; j++) {
      if (j < left && mb_beam_ok(ping->beamflag[j]))
        left = j;
      if (j > right && mb_beam_ok(ping->beamflag[j]))
        right = j;
    }

    /* add valid points to list */
    for (int j = 0; j < ping->beams_bath; j++) {
      if (mb_beam_ok(ping->beamflag[j])) {
        data->pingid[data->npts] = i;
        data->beamid[data->npts] = j;
        if (j == right)
          data->edge[data->npts] = 1;
        else if (j == left)
          data->edge[data->npts] = -1;
        else
          data->edge[data->npts] = 0;
        data->x[data->npts] = ping->bathlon[j];
        data->y[data->npts] = ping->bathlat[j];
        data->z[data->npts] = ping->bath[j];
        data->npts++;
      }
    }
  }

  // get extrema of preliminary soundings and the minimum  distance allowed
  // between points submitted for triangulization
  double mtodeglon = 0.0;
  double mtodeglat = 0.0;
  double rr_threshold = 0.0;
  double xmin = 0.0;
  double xmax = 0.0;
  double ymin = 0.0;
  double ymax = 0.0;
  double zmin = 0.0;
  double zmax = 0.0;
  double dlon = 0.0;
  double dlat = 0.0;
  if (data->npts > 0) {
    xmin = data->x[0];
    xmax = data->x[0];
    ymin = data->y[0];
    ymax = data->y[0];
    zmin = data->z[0];
    zmax = data->z[0];
    for (int ipt = 1; ipt < data->npts; ipt++) {
      xmin = MIN(xmin, data->x[ipt]);
      xmax = MAX(xmax, data->x[ipt]);
      ymin = MIN(ymin, data->y[ipt]);
      ymax = MAX(ymax, data->y[ipt]);
      zmin = MIN(zmin, data->z[ipt]);
      zmax = MAX(zmax, data->z[ipt]);
    }
    mb_coor_scale(verbose, 0.5 * (ymin + ymax), &mtodeglon, &mtodeglat);
    if (data->triangle_scale > 0.001) {
      rr_threshold = data->triangle_scale;
    } else {
      rr_threshold = MAX(0.01 * (xmax - xmin) / mtodeglon, 0.01 * (ymax - ymin) / mtodeglat);
// fprintf(stderr, "%s:%d:%s: rr_threshold: %f m\n", __FILE__, __LINE__, __FUNCTION__, rr_threshold);
    }
    dlon = rr_threshold * mtodeglon;
    dlat = rr_threshold * mtodeglat;
  }

  // delete all but one of points with close x-y positions where close is 1/100
  // of the long dimension of the area covered by the section
  for (int ipt0 = 0; ipt0 < data->npts; ipt0++) {
    if (data->pingid[ipt0] >= 0) {
      int ii0 = (int)floor((data->x[ipt0] - xmin) / dlon);
      int jj0 = (int)floor((data->y[ipt0] - ymin) / dlat);
      for (int ipt1 = data->npts - 1; ipt1 > ipt0; ipt1--) {
        if (data->pingid[ipt1] >= 0) {
          int ii1 = (int)floor((data->x[ipt1] - xmin) / dlon);
          int jj1 = (int)floor((data->y[ipt1] - ymin) / dlat);
          if (ii0 == ii1 && jj0 == jj1) {
            if (data->z[ipt0] > data->z[ipt1]) {
              data->pingid[ipt0] = -1;
            } else {
              data->pingid[ipt1] = -1;
            }
          }
        }
      }
    }
  }
  for (int ipt = data->npts - 1; ipt >= 0; ipt--) {
    if (data->pingid[ipt] < 0) {
      for (int ipt0=ipt; ipt0<data->npts-1; ipt0++) {
        data->pingid[ipt0] = data->pingid[ipt0+1];
        data->beamid[ipt0] = data->beamid[ipt0+1];
        data->edge[ipt0] = data->edge[ipt0+1];
        data->x[ipt0] = data->x[ipt0+1];
        data->y[ipt0] = data->y[ipt0+1];
        data->z[ipt0] = data->z[ipt0+1];
      }
      data->npts--;
    }
  }

  /* get extrema of remaining soundings */
  if (data->npts > 0) {
    data->bath_min = data->z[0];
    data->bath_max = data->z[0];
    for (int ipt = 1; ipt < data->npts; ipt++) {
      data->bath_min = MIN(data->bath_min, data->z[ipt]);
      data->bath_max = MAX(data->bath_max, data->z[ipt]);
    }
  }

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg4  Data points to be used for triangulization:\n");
    fprintf(stderr, "dbg4       npts:             %d\n", data->npts);
    fprintf(stderr, "dbg4       bath_min:         %f\n", data->bath_min);
    fprintf(stderr, "dbg4       bath_max:         %f\n", data->bath_max);
    for (int ipt = 0; ipt < data->npts; ipt++)
      fprintf(stderr, "dbg4       %4d %4d %4d %d  %f %f %f\n",
              ipt, data->pingid[ipt], data->beamid[ipt], data->edge[ipt],
              data->x[ipt], data->y[ipt], data->z[ipt]);
  }

  /* get triangle network */
  if (data->npts > 2) {
    status = mb_delaun(verbose, data->npts, data->x, data->y, data->edge,
                      &data->ntri, data->iv[0], data->iv[1], data->iv[2],
                      data->ct[0], data->ct[1], data->ct[2], data->cs[0], data->cs[1], data->cs[2],
                      data->v1, data->v2, data->v3, data->istack, data->kv1, data->kv2, error);
  }
  if (verbose > 1)
    fprintf(stderr, "\n");
  if (verbose > 0)
    fprintf(stderr, "-->Obtained %d triangles of scale %f meters from %d points in %d pings...\n",
                    data->ntri, rr_threshold, data->npts, data->npings);

  /* figure out which triangle sides are on the swath edge */
  for (int itri = 0; itri < data->ntri; itri++) {
    for (int j = 0; j < 3; j++) {
      int jj = j + 1;
      if (jj > 2)
        jj = 0;
      if (data->edge[data->iv[j][itri]] == -1 && data->edge[data->iv[jj][itri]] == -1)
        data->ed[j][itri] = -1;
      else if (data->edge[data->iv[j][itri]] == 1 && data->edge[data->iv[jj][itri]] == 1)
        data->ed[j][itri] = 1;
      else
        data->ed[j][itri] = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       data:             %p\n", data);
    fprintf(stderr, "dbg2       data->npts:       %d\n", data->npts);
    fprintf(stderr, "dbg2       data->npts_alloc: %d\n", data->npts_alloc);
    for (int ipt = 0; ipt < data->npts; ipt++) {
      fprintf(stderr, "dbg2          pt[%4d]: %4d %4d  %f %f %f\n",
                      ipt, data->pingid[ipt], data->beamid[ipt],
                      data->x[ipt], data->y[ipt], data->z[ipt]);
    }
    fprintf(stderr, "dbg2       data->ntri:       %d\n", data->ntri);
    fprintf(stderr, "dbg2       data->ntri_alloc: %d\n", data->ntri_alloc);
    fprintf(stderr, "dbg2       triangle[i of %d]: <vertices> <connecting triangles> <connecting sides> <on edge?>\n",
                      data->ntri);
    for (int i = 0; i < data->ntri; i++) {
      fprintf(stderr, "dbg2          tri[%4d]: v: %d %d %d  t: %d %d %d  s: %d %d %d  e: %d %d %d\n",
                      i, data->iv[0][i], data->iv[1][i], data->iv[2][i],
                      data->ct[0][i], data->ct[1][i], data->ct[2][i],
                      data->cs[0][i], data->cs[1][i], data->cs[2][i],
                      data->ed[0][i], data->ed[0][i], data->ed[0][i]);
    }
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
/*   function mb_tcontour contours multibeam data that has been triangulated */
int mb_tcontour(int verbose, struct swath *data, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       data:                    %p\n", data);
    fprintf(stderr, "dbg2       data->contour_algorithm: %d\n", data->contour_algorithm);
    fprintf(stderr, "dbg2       data->plot_contours:     %d\n", data->plot_contours);
    fprintf(stderr, "dbg2       data->plot_triangles:    %d\n", data->plot_triangles);
    fprintf(stderr, "dbg2       data->plot_track:        %d\n", data->plot_track);
    fprintf(stderr, "dbg2       data->plot_name:         %d\n", data->plot_name);
    fprintf(stderr, "dbg2       data->contour_int:       %f\n", data->contour_int);
    fprintf(stderr, "dbg2       data->color_int:         %f\n", data->color_int);
    fprintf(stderr, "dbg2       data->tick_int:          %f\n", data->tick_int);
    fprintf(stderr, "dbg2       data->label_int:         %f\n", data->label_int);
    fprintf(stderr, "dbg2       data->tick_len:          %f\n", data->tick_len);
    fprintf(stderr, "dbg2       data->label_hgt:         %f\n", data->label_hgt);
    fprintf(stderr, "dbg2       data->label_spacing:     %f\n", data->label_spacing);
    fprintf(stderr, "dbg2       data->ncolor:            %d\n", data->ncolor);
    fprintf(stderr, "dbg2       data->nlevel:            %d\n", data->nlevel);
    fprintf(stderr, "dbg2       data->nlevelset:         %d\n", data->nlevelset);
    if (data->nlevelset)
      for (int i = 0; i < data->nlevel; i++) {
        fprintf(stderr, "dbg2          level[%3d]:  %f %d %d %d\n", i, data->level_list[i], data->label_list[i],
                data->tick_list[i], data->color_list[i]);
      }
    fprintf(stderr, "dbg2       data->npings:     %d\n", data->npings);
    fprintf(stderr, "dbg2       data->npings_max: %d\n", data->npings_max);
    fprintf(stderr, "dbg2       data->beams_bath: %d\n", data->beams_bath);
    for (int i = 0; i < data->npings; i++) {
      fprintf(stderr, "dbg2          ping[%4d]: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d:%6.6d %f %f %f %f %d\n", i,
              data->pings[i].time_i[0], data->pings[i].time_i[1], data->pings[i].time_i[2], data->pings[i].time_i[3],
              data->pings[i].time_i[4], data->pings[i].time_i[5], data->pings[i].time_i[6], data->pings[i].time_d,
              data->pings[i].navlon, data->pings[i].navlat, data->pings[i].heading, data->pings[i].beams_bath);
      for (int j = 0; j < data->pings[i].beams_bath; j++) {
        if (mb_beam_ok(data->pings[i].beamflag[j]))
          fprintf(stderr, "dbg2          beam[%4d:%3d]:  %2d %f %f %f\n", i, j, data->pings[i].beamflag[j],
                  data->pings[i].bath[j], data->pings[i].bathlon[j], data->pings[i].bathlat[j]);
      }
    }
    fprintf(stderr, "dbg2       data->npts:       %d\n", data->npts);
    fprintf(stderr, "dbg2       data->npts_alloc: %d\n", data->npts_alloc);
    fprintf(stderr, "dbg2       data->bath_min:   %f\n", data->bath_min);
    fprintf(stderr, "dbg2       data->bath_max:   %f\n", data->bath_max);
    for (int ipt = 0; ipt < data->npts; ipt++) {
      fprintf(stderr, "dbg2          pt[%4d]: %4d %4d  %f %f %f\n",
                      ipt, data->pingid[ipt], data->beamid[ipt],
                      data->x[ipt], data->y[ipt], data->z[ipt]);
    }
    fprintf(stderr, "dbg2       data->ntri:       %d\n", data->ntri);
    fprintf(stderr, "dbg2       data->ntri_alloc: %d\n", data->ntri_alloc);
    fprintf(stderr, "dbg2       triangle[i of %d]: <vertices> <connecting triangles> <connecting sides> <on edge?>\n",
                      data->ntri);
    for (int i = 0; i < data->ntri; i++) {
      fprintf(stderr, "dbg2          tri[%4d]: v: %d %d %d  t: %d %d %d  s: %d %d %d  e: %d %d %d\n",
                      i, data->iv[0][i], data->iv[1][i], data->iv[2][i],
                      data->ct[0][i], data->ct[1][i], data->ct[2][i],
                      data->cs[0][i], data->cs[1][i], data->cs[2][i],
                      data->ed[0][i], data->ed[0][i], data->ed[0][i]);
    }
  }

  /* if no depth variation don't bother */
  int status = MB_SUCCESS;
  if ((data->bath_max - data->bath_min) < EPS)
    return (status);

  /* get number of contour intervals */
  if (!data->nlevelset) {
    if (data->nlevel > 0) {
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->level_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->color_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->label_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->tick_list, error);
    }
    const int nci = data->bath_min / data->contour_int + 1;
    const int ncf = data->bath_max / data->contour_int + 1;
    data->nlevel = ncf - nci;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(double), (void **)&(data->level_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->color_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->label_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->tick_list), error);
    if (*error != MB_ERROR_NO_ERROR)
      return (status);
    for (int i = 0; i < data->nlevel; i++) {
      const int k = nci + i;
      data->level_list[i] = k * data->contour_int;
      data->color_list[i] = (int)(data->level_list[i] / data->color_int) % data->ncolor;
      if (data->tick_int <= 0.0)
        data->tick_list[i] = 0;

      else {
        const double ratio = data->level_list[i] / data->tick_int;
        if (fabs(round(ratio) - ratio) < 0.005 * data->contour_int)
          data->tick_list[i] = 1;
        else
          data->tick_list[i] = 0;
      }
      if (data->label_int <= 0.0)
        data->label_list[i] = 0;
      else {
        const double ratio = data->level_list[i] / data->label_int;
        if (fabs(round(ratio) - ratio) < 0.005 * data->contour_int)
          data->label_list[i] = 1;
        else
          data->label_list[i] = 0;
      }
    }
  }

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg4  Contour levels:\n");
    fprintf(stderr, "dbg4       nlevel:           %d\n", data->nlevel);
    fprintf(stderr, "dbg4       i level color tick label:\n");
    for (int i = 0; i < data->nlevel; i++)
      fprintf(stderr, "dbg4       %d %f %d %d %d\n", i, data->level_list[i], data->color_list[i], data->tick_list[i],
              data->label_list[i]);
  }

  /* make sure that no depths are exact contour values */
  const double eps = EPS * (data->bath_max - data->bath_min);
  for (int klevel = 0; klevel < data->nlevel; klevel++) {
    for (int ipt = 0; ipt < data->npts; ipt++) {
      if (fabs(data->z[ipt] - data->level_list[klevel]) < eps)
        data->z[ipt] = data->level_list[klevel] + eps;
    }
  }

  /* plot the triangles if desired */
  if (data->plot_triangles) {
    data->contour_newpen(0);
    for (int itri = 0; itri < data->ntri; itri++) {
      const int ipt0 = data->iv[0][itri];
      const int ipt1 = data->iv[1][itri];
      const int ipt2 = data->iv[2][itri];
      data->contour_plot(data->x[ipt0], data->y[ipt0], IMOVE);
      data->contour_plot(data->x[ipt1], data->y[ipt1], IDRAW);
      data->contour_plot(data->x[ipt2], data->y[ipt2], IDRAW);
      data->contour_plot(data->x[ipt0], data->y[ipt0], ISTROKE);
    }
  }

  /* loop over all of the contour values */
  data->nsave = 0;
  data->nlabel = 0;
  if (status == MB_SUCCESS && data->plot_contours)
    for (int ival = 0; ival < data->nlevel; ival++) {
      const double value = data->level_list[ival];
      data->contour_newpen(data->color_list[ival]);
      const int tick = data->tick_list[ival];
      const int label = data->label_list[ival];

      if (verbose >= 4) {
        fprintf(stderr, "\ndbg4  About to contour level in function <%s>\n", __func__);
        fprintf(stderr, "dbg4       value:         %f\n", value);
        fprintf(stderr, "dbg4       tick:          %d\n", tick);
        fprintf(stderr, "dbg4       label:         %d\n", label);
      }

      /* flag all triangle sides crossed by the current contour */
      for (int itri = 0; itri < data->ntri; itri++) {
        for (int j = 0; j < 3; j++) {
          int jj = j + 1;
          if (jj == 3)
            jj = 0;
          if ((data->z[data->iv[j][itri]] > value && data->z[data->iv[jj][itri]] < value) ||
              (data->z[data->iv[jj][itri]] > value && data->z[data->iv[j][itri]] < value))
            data->flag[j][itri] = 1;
          else
            data->flag[j][itri] = 0;
        }
      }

      /* do the contouring */
      data->nsave = 0;
      int itri;
      int iside1;
      int iside2;
      int closed;
      while (get_start_tri(data, &itri, &iside1, &iside2, &closed)) {
        
        /* check that large enough xsave and ysave arrays have been allocated */
        if (data->nsave >= data->nsave_alloc - 4) {
			data->nsave_alloc += 8192;
			status &= mb_reallocd(2, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->xsave), error);
			status &= mb_reallocd(2, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->ysave), error);
        }

        /* if not closed remove flags */
        data->flag[iside1][itri] = -1;
        data->flag[iside2][itri] = -1;

        /* get position of start of contour */
        get_pos_tri(data, eps, itri, iside1, value, &data->xsave[data->nsave], &data->ysave[data->nsave]);
        data->nsave++;
        get_pos_tri(data, eps, itri, iside2, value, &data->xsave[data->nsave], &data->ysave[data->nsave]);
        data->nsave++;
        int itristart = itri;
        int isidestart = iside1;
        int itriend = itri;
        int isideend = iside2;

        /* set tick flag */
        bool tick_last = false;

        /* look for next segment */
        while (get_next_tri(data, &itri, &iside1, &iside2, &closed, &itristart, &isidestart)) {
		  
		  /* check that large enough xsave and ysave arrays have been allocated */
		  if (data->nsave >= data->nsave_alloc - 4) {
			  data->nsave_alloc += 8192;
			  status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->xsave), error);
			  status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->ysave), error);
		  }

          /* get position */
          double x;
          double y;
          get_pos_tri(data, eps, itri, iside2, value, &x, &y);

          /* deal with tick as needed */
          if (tick && !tick_last) {
            int hand = 0;
            if (data->z[data->iv[iside1][itri]] > data->z[data->iv[iside2][itri]])
              hand = -1;
            else
              hand = 1;
            data->xsave[data->nsave] = 0.5 * (x + data->xsave[data->nsave - 1]);
            data->ysave[data->nsave] = 0.5 * (y + data->ysave[data->nsave - 1]);
            const double magdis =
                sqrt(pow((x - data->xsave[data->nsave - 1]), 2.0) + pow((y - data->ysave[data->nsave - 1]), 2.0));
            data->xsave[data->nsave + 1] =
                data->xsave[data->nsave] - hand * data->tick_len * (y - data->ysave[data->nsave - 1]) / magdis;
            data->ysave[data->nsave + 1] =
                data->ysave[data->nsave] + hand * data->tick_len * (x - data->xsave[data->nsave - 1]) / magdis;
            data->xsave[data->nsave + 2] = data->xsave[data->nsave];
            data->ysave[data->nsave + 2] = data->ysave[data->nsave];
            data->xsave[data->nsave + 3] = x;
            data->ysave[data->nsave + 3] = y;
            data->flag[iside1][itri] = -1;
            data->flag[iside2][itri] = -1;
            data->nsave = data->nsave + 4;
            tick_last = true;
          }
          else {
            data->xsave[data->nsave] = x;
            data->ysave[data->nsave] = y;
            data->flag[iside1][itri] = -1;
            data->flag[iside2][itri] = -1;
            data->nsave++;
            tick_last = false;
          }

          /* set latest point */
          itriend = itri;
          isideend = iside2;
        }

        /* set label if needed */
        if (label && !closed && data->ed[isidestart][itristart] != 0) {
          data->xlabel[data->nlabel] = data->xsave[0];
          data->ylabel[data->nlabel] = data->ysave[0];
          get_azimuth_tri(data, itristart, isidestart, &data->angle[data->nlabel]);
          if (data->ed[isidestart][itristart] == -1)
            data->justify[data->nlabel] = 1;
          else
            data->justify[data->nlabel] = 0;
          if (check_label(data, data->nlabel))
            data->nlabel++;
        }
        if (label && !closed && data->ed[isideend][itriend] != 0) {
          data->xlabel[data->nlabel] = data->xsave[data->nsave - 1];
          data->ylabel[data->nlabel] = data->ysave[data->nsave - 1];
          get_azimuth_tri(data, itriend, isideend, &data->angle[data->nlabel]);
          if (data->ed[isideend][itriend] == -1)
            data->justify[data->nlabel] = 1;
          else
            data->justify[data->nlabel] = 0;
          if (check_label(data, data->nlabel))
            data->nlabel++;
        }

        /* dump the contour */
        dump_contour(data, value);
      }

      /* done with contouring this level */
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
/*   function get_start_old finds next contour starting point.
 *  the borders are searched first and then the interior */
int get_start_old(const struct swath *data, int *k, int *i, int *j, int *d, int *closed) {
  /* search edges */
  *closed = 0;

  /* search bottom (i = 0) */
  for (int jj = 0; jj < data->pings[0].beams_bath - 1; jj++)
    if (data->pings[0].bflag[0][jj]) {
      *k = 0;
      *i = 0;
      *j = jj;
      *d = 0;
      return (1);
    }

  /* search top (i = npings-1) */
  for (int jj = 0; jj < data->pings[data->npings - 1].beams_bath - 1; jj++)
    if (data->pings[data->npings - 1].bflag[0][jj]) {
      *k = 0;
      *i = data->npings - 1;
      *j = jj;
      *d = 1;
      return (1);
    }

  /* search left (j = 0) */
  for (int ii = 0; ii < data->npings - 1; ii++)
    if (data->pings[ii].beams_bath > 0 && data->pings[ii].bflag[1][0]) {
      *k = 1;
      *i = ii;
      *j = 0;
      *d = 0;
      return (1);
    }

  /* search right (j = beams_bath-1) */
  for (int ii = 0; ii < data->npings - 1; ii++)
    if (data->pings[ii].beams_bath > 0 && data->pings[ii].bflag[1][data->pings[ii].beams_bath - 1]) {
      *k = 1;
      *i = ii;
      *j = data->pings[ii].beams_bath - 1;
      *d = 1;
      return (1);
    }

  /* search interior */
  *closed = 1;
  for (int ii = 0; ii < data->npings - 1; ii++)
    for (int jj = 0; jj < data->pings[ii].beams_bath - 1; jj++) {
      if (data->pings[ii].bflag[0][jj]) {
        *k = 0;
        *i = ii;
        *j = jj;
        *d = 0;
        return (1);
      }
      if (data->pings[ii].bflag[1][jj]) {
        *k = 1;
        *i = ii;
        *j = jj;
        *d = 0;
        return (1);
      }
    }

  /* nothing found */
  return (0);
}
/*--------------------------------------------------------------------------*/
/*   function get_next_old finds next contour component if it exists */
int get_next_old(struct swath *data, int *nk, int *ni, int *nj, int *nd, int k, int i, int j, int d, int kbeg, int ibeg, int jbeg,
                 int dbeg, int *closed) {
  static const int ioff[3][2][2] = {{{0, -1}, {1, 0}}, {{1, -1}, {0, 0}}, {{0, -1}, {0, 1}}};
  static const int joff[3][2][2] = {{{0, 1}, {0, -1}}, {{0, 0}, {1, -1}}, {{1, 0}, {0, -1}}};
  static const int koff[3][2][2] = {{{1, 1}, {0, 0}}, {{0, 0}, {1, 1}}, {{1, 1}, {0, 0}}};
  static const int doff[3][2][2] = {{{1, 0}, {0, 1}}, {{0, 1}, {0, 1}}, {{0, 1}, {1, 0}}};
  int kt[3];
  int it[3];
  int jt[3];
  int dt[3];
  int ifedge[3];

  /* there are three possible edges for the contour to go to */
  /* (left = 0, across = 1, right = 2) */
  /* find out which edges have unflagged crossing points */
  for (int edge = 0; edge < 3; edge++) {
    kt[edge] = koff[edge][k][d];
    it[edge] = i + ioff[edge][k][d];
    jt[edge] = j + joff[edge][k][d];
    dt[edge] = doff[edge][k][d];
    if (it[edge] < 0 || it[edge] >= data->npings || jt[edge] < 0
            || jt[edge] >= data->pings[i].beams_bath || data->pings[it[edge]].beams_bath <= 0)
      ifedge[edge] = 0;
    else
      ifedge[edge] = data->pings[it[edge]].bflag[kt[edge]][jt[edge]];
  }

  /* if the across edge exists, use it */
  if (ifedge[1]) {
    *nk = kt[1];
    *ni = it[1];
    *nj = jt[1];
    *nd = dt[1];
    return (1);
  }

  /* else if edge 0 exists, use it */
  else if (ifedge[0]) {
    *nk = kt[0];
    *ni = it[0];
    *nj = jt[0];
    *nd = dt[0];
    return (1);
  }

  /* else if edge 2 exists, use it */
  else if (ifedge[2]) {
    *nk = kt[2];
    *ni = it[2];
    *nj = jt[2];
    *nd = dt[2];
    return (1);
  }

  /* if no edge is found and contour is closed and closes then */
  /* contour ends */
  else if (*closed && kbeg == k && ibeg == i && jbeg == j)
    return (0);

  /* if no edge is found and contour is closed but doesn't close then */
  /* reverse order of points and start over */
  else if (*closed) {
    for (int ii = 0; ii < data->nsave / 2; ii++) {
      const double xs = data->xsave[ii];
      const double ys = data->ysave[ii];
      data->xsave[ii] = data->xsave[data->nsave - ii - 1];
      data->ysave[ii] = data->ysave[data->nsave - ii - 1];
      data->xsave[data->nsave - ii - 1] = xs;
      data->ysave[data->nsave - ii - 1] = ys;
    }
    *closed = 0;
    *nk = kbeg;
    *ni = ibeg;
    *nj = jbeg;
    if (dbeg)
      *nd = 0;
    else
      *nd = 1;
    data->nsave--;
    return (1);
  }

  /* else if no edge is found and contour is not closed */
  /* then contour ends */
  else
    return (0);
}
/*--------------------------------------------------------------------------*/
/*   function get_pos_old finds position of contour crossing point */
int get_pos_old(const struct swath *data, double eps, double *x, double *y, int k, int i, int j, double value) {

  /* get grid positions and values */
  const double x1 = data->pings[i].bathlon[j];
  const double y1 = data->pings[i].bathlat[j];
  const double v1 = data->pings[i].bath[j];
  double x2;
  double y2;
  double v2;
  if (k == 0) {
    x2 = data->pings[i].bathlon[j + 1];
    y2 = data->pings[i].bathlat[j + 1];
    v2 = data->pings[i].bath[j + 1];
  }
  else {
    x2 = data->pings[i + 1].bathlon[j];
    y2 = data->pings[i + 1].bathlat[j];
    v2 = data->pings[i + 1].bath[j];
  }

  /* interpolate the position */
  double factor;
  if (fabs(v2 - v1) > eps)
    factor = (value - v1) / (v2 - v1);
  else
    factor = 0.5;
  if (factor < 0.0)
    factor = 0.0;
  if (factor > 1.0)
    factor = 1.0;
  *x = factor * (x2 - x1) + x1;
  *y = factor * (y2 - y1) + y1;

  return (true);
}
/*--------------------------------------------------------------------------*/
/*   function get_hand_old finds handedness of contour */
int get_hand_old(const struct swath *data, int *hand, int k, int i, int j, int d) {
  if (k == 0 && d == 0) {
    if (data->pings[i].bath[j] > data->pings[i].bath[j + 1])
      *hand = 1;
    else
      *hand = -1;
  }
  else if (k == 0 && d == 1) {
    if (data->pings[i].bath[j] > data->pings[i].bath[j + 1])
      *hand = -1;
    else
      *hand = 1;
  }
  else if (k == 1 && d == 0) {
    if (data->pings[i].bath[j] > data->pings[i + 1].bath[j])
      *hand = -1;
    else
      *hand = 1;
  }
  else if (k == 1 && d == 1) {
    if (data->pings[i].bath[j] > data->pings[i + 1].bath[j])
      *hand = 1;
    else
      *hand = -1;
  }
  return (true);
}
/*--------------------------------------------------------------------------*/
/*   function get_azimuth_old gets azimuth across shiptrack at ping iping */
int get_azimuth_old(const struct swath *data, int iping, double *angle) {

  *angle = -data->pings[iping].heading;
  if (*angle > 180.0)
    *angle = *angle - 360.0;
  if (*angle < -180.0)
    *angle = *angle + 360.0;

  return (true);
}
/*--------------------------------------------------------------------------*/
/*  function mb_ocontour contours multibeam data connecting soundings
    from one ping to the next, without delauney triangulization */
int mb_ocontour(int verbose, struct swath *data, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       data:                    %p\n", data);
    fprintf(stderr, "dbg2       data->contour_algorithm: %d\n", data->contour_algorithm);
    fprintf(stderr, "dbg2       data->plot_contours:     %d\n", data->plot_contours);
    fprintf(stderr, "dbg2       data->plot_triangles:    %d\n", data->plot_triangles);
    fprintf(stderr, "dbg2       data->plot_track:        %d\n", data->plot_track);
    fprintf(stderr, "dbg2       data->plot_name:         %d\n", data->plot_name);
    fprintf(stderr, "dbg2       data->contour_int:       %f\n", data->contour_int);
    fprintf(stderr, "dbg2       data->color_int:         %f\n", data->color_int);
    fprintf(stderr, "dbg2       data->tick_int:          %f\n", data->tick_int);
    fprintf(stderr, "dbg2       data->label_int:         %f\n", data->label_int);
    fprintf(stderr, "dbg2       data->tick_len:          %f\n", data->tick_len);
    fprintf(stderr, "dbg2       data->label_hgt:         %f\n", data->label_hgt);
    fprintf(stderr, "dbg2       data->label_spacing:     %f\n", data->label_spacing);
    fprintf(stderr, "dbg2       data->ncolor:            %d\n", data->ncolor);
    fprintf(stderr, "dbg2       data->nlevel:            %d\n", data->nlevel);
    fprintf(stderr, "dbg2       data->nlevelset:         %d\n", data->nlevelset);
    if (data->nlevelset)
      for (int i = 0; i < data->nlevel; i++) {
        fprintf(stderr, "dbg2          level[%3d]:  %f %d %d %d\n", i, data->level_list[i], data->label_list[i],
                data->tick_list[i], data->color_list[i]);
      }
    fprintf(stderr, "dbg2       data->npings:     %d\n", data->npings);
    fprintf(stderr, "dbg2       data->npings_max: %d\n", data->npings_max);
    fprintf(stderr, "dbg2       data->beams_bath: %d\n", data->beams_bath);
    for (int i = 0; i < data->npings; i++) {
      fprintf(stderr, "dbg2          ping[%4d]: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d:%6.6d %f %f %f %f %d\n", i,
              data->pings[i].time_i[0], data->pings[i].time_i[1], data->pings[i].time_i[2], data->pings[i].time_i[3],
              data->pings[i].time_i[4], data->pings[i].time_i[5], data->pings[i].time_i[6], data->pings[i].time_d,
              data->pings[i].navlon, data->pings[i].navlat, data->pings[i].heading, data->pings[i].beams_bath);
      for (int j = 0; j < data->pings[i].beams_bath; j++) {
        if (mb_beam_ok(data->pings[i].beamflag[j]))
          fprintf(stderr, "dbg2          beam[%4d:%3d]:  %2d %f %f %f\n", i, j, data->pings[i].beamflag[j],
                  data->pings[i].bath[j], data->pings[i].bathlon[j], data->pings[i].bathlat[j]);
      }
    }
  }


  /* count number of points and verify that enough memory is allocated */
  int nsave_cnt = 0;
  for (int i = 0; i < data->npings; i++) {
    struct ping *ping = &data->pings[i];
    for (int j = 0; j < ping->beams_bath; j++) {
      if (mb_beam_ok(ping->beamflag[j]))
        nsave_cnt++;
    }
  }

  int status = MB_SUCCESS;
  if (nsave_cnt > data->nsave_alloc) {
    data->nsave_alloc = nsave_cnt;
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->xsave), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->ysave), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(int), (void **)&(data->isave), error);
    status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(int), (void **)&(data->jsave), error);
  }

  /* zero flags */
  for (int i = 0; i < data->npings; i++) {
    struct ping *ping = &data->pings[i];
    for (int j = 0; j < ping->beams_bath; j++) {
      ping->bflag[0][j] = 0;
      ping->bflag[1][j] = 0;
    }
  }

  /* get min max of bathymetry */
  // TODO(schwehr): Better to set min to DBL_MAX and max to -DBL_MAX?
  double bath_min = 0.0;  // -Wmaybe-uninitialized
  double bath_max = 0.0;  // -Wmaybe-uninitialized
  bool extreme_start = false;
  for (int i = 0; i < data->npings; i++) {
    struct ping *ping = &data->pings[i];
    for (int j = 0; j < ping->beams_bath; j++) {
      if (!extreme_start && mb_beam_ok(ping->beamflag[j])) {
        bath_min = ping->bath[j];
        bath_max = ping->bath[j];
        extreme_start = true;
      }
      if (mb_beam_ok(ping->beamflag[j])) {
        bath_min = MIN(bath_min, ping->bath[j]);
        bath_max = MAX(bath_max, ping->bath[j]);
      }
    }
  }

  /* if no depth variation don't bother */
  if ((bath_max - bath_min) < EPS)
    return (status);

  /* get number of contour intervals */
  if (!data->nlevelset) {
    if (data->nlevel > 0) {
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->level_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->color_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->label_list, error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&data->tick_list, error);
    }
    const int nci = bath_min / data->contour_int + 1;
    const int ncf = bath_max / data->contour_int + 1;
    data->nlevel = ncf - nci;
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(double), (void **)&(data->level_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->color_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->label_list), error);
    status &= mb_mallocd(verbose, __FILE__, __LINE__, data->nlevel * sizeof(int), (void **)&(data->tick_list), error);
    if (*error != MB_ERROR_NO_ERROR)
      return (status);
    for (int i = 0; i < data->nlevel; i++) {
      const int k = nci + i;
      data->level_list[i] = k * data->contour_int;
      data->color_list[i] = (int)(data->level_list[i] / data->color_int) % data->ncolor;
      if (data->tick_int <= 0.0)
        data->tick_list[i] = 0;
      else {
        const double ratio = data->level_list[i] / data->tick_int;
        if (fabs(round(ratio) - ratio) < 0.005 * data->contour_int)
          data->tick_list[i] = 1;
        else
          data->tick_list[i] = 0;
      }
      if (data->label_int <= 0.0)
        data->label_list[i] = 0;
      else {
        const double ratio = data->level_list[i] / data->label_int;
        if (fabs(round(ratio) - ratio) < 0.005 * data->contour_int)
          data->label_list[i] = 1;
        else
          data->label_list[i] = 0;
      }
    }
  }

  if (verbose >= 4) {
    fprintf(stderr, "\ndbg4  Data points:\n");
    fprintf(stderr, "dbg4       nlevel:           %d\n", data->nlevel);
    fprintf(stderr, "dbg4       i level color tick label:\n");
    for (int i = 0; i < data->nlevel; i++)
      fprintf(stderr, "dbg4       %d %f %d %d %d\n", i, data->level_list[i], data->color_list[i], data->tick_list[i],
              data->label_list[i]);
  }

  /* make sure that no depths are exact contour values */
  const double eps = EPS * (bath_max - bath_min);
  for (int k = 0; k < data->nlevel; k++) {
    for (int i = 0; i < data->npts; i++) {
      if (fabs(data->z[i] - data->level_list[k]) < eps)
        data->z[i] = data->level_list[k] + eps;
    }
  }

  /* loop over all of the contour values */
  data->nsave = 0;
  data->nlabel = 0;
  if (status == MB_SUCCESS && data->plot_contours)
    for (int ival = 0; ival < data->nlevel; ival++) {
      const double value = data->level_list[ival];
      data->contour_newpen(data->color_list[ival]);
      const int tick = data->tick_list[ival];
      const int label = data->label_list[ival];

      if (verbose >= 4) {
        fprintf(stderr, "\ndbg4  About to contour level in function <%s>\n", __func__);
        fprintf(stderr, "dbg4       value:         %f\n", value);
        fprintf(stderr, "dbg4       tick:          %d\n", tick);
        fprintf(stderr, "dbg4       label:         %d\n", label);
      }

      /* flag all grid sides crossed by the current contour */
      char *beamflag2 = NULL;
      double *bath2 = NULL;
      for (int i = 0; i < data->npings; i++) {
        char *beamflag1 = data->pings[i].beamflag;
        beamflag2 = NULL;
        double *bath1 = data->pings[i].bath;
        bath2 = NULL;
        int beams_bath_use = data->pings[i].beams_bath;
        if (i < data->npings - 1) {
          beamflag2 = data->pings[i + 1].beamflag;
          bath2 = data->pings[i + 1].bath;
          beams_bath_use = MIN(beams_bath_use, data->pings[i + 1].beams_bath);
        }
        for (int j = 0; j < beams_bath_use; j++) {
          /* check for across track intersection */
          if (j < beams_bath_use - 1)
            if ((mb_beam_ok(beamflag1[j]) && mb_beam_ok(beamflag1[j + 1])) &&
                ((bath1[j] < value && bath1[j + 1] > value) || (bath1[j] > value && bath1[j + 1] < value)))
              data->pings[i].bflag[0][j] = 1;

          /* check for along track intersection */
          if (i < data->npings - 1)
            if ((mb_beam_ok(beamflag1[j]) && mb_beam_ok(beamflag2[j])) &&
                ((bath1[j] < value && bath2[j] > value) || (bath1[j] > value && bath2[j] < value)))
              data->pings[i].bflag[1][j] = 1;
        }
      }

      /* loop until all flagged points have been unflagged */
      int i = 0;
      int k = 0;
      int j = 0;
      int d = 0;
      int closed = 0;
      while (get_start_old(data, &k, &i, &j, &d, &closed)) {
        /* if not closed remove from flag list */
        if (closed == 0)
          data->pings[i].bflag[k][j] = 0;

        /* get position and handedness */
        double x;
        double y;
        get_pos_old(data, eps, &x, &y, k, i, j, value);
        data->xsave[0] = x;
        data->ysave[0] = y;
        data->isave[0] = i;
        data->jsave[0] = j;
        data->nsave = 1;
        data->nlabel = 0;
        const int ibeg = i;
        const int jbeg = j;
        const int kbeg = k;
        const int dbeg = d;

        /* set tick flag */
        bool tick_last = false;

        /* look for next component */
        int hand;
        int nk;
        int ni;
        int nj;
        int nd;
        while (get_next_old(data, &nk, &ni, &nj, &nd, k, i, j, d, kbeg, ibeg, jbeg, dbeg, &closed)) {
		  
		  /* check that large enough xsave and ysave arrays have been allocated */
		  if (data->nsave >= data->nsave_alloc - 4) {
			  data->nsave_alloc += 8192;
			  status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->xsave), error);
			  status &= mb_reallocd(verbose, __FILE__, __LINE__, data->nsave_alloc * sizeof(double), (void **)&(data->ysave), error);
		  }

          /* get position */
          get_pos_old(data, eps, &x, &y, nk, ni, nj, value);
          get_hand_old(data, &hand, k, i, j, d);
          if (tick && !tick_last) {
            data->xsave[data->nsave] = 0.5 * (x + data->xsave[data->nsave - 1]);
            data->ysave[data->nsave] = 0.5 * (y + data->ysave[data->nsave - 1]);
            const double magdis =
                sqrt(pow((x - data->xsave[data->nsave - 1]), 2.0) + pow((y - data->ysave[data->nsave - 1]), 2.0));
            if (magdis > 0.0) {
              data->xsave[data->nsave + 1] =
                  data->xsave[data->nsave] - hand * data->tick_len * (y - data->ysave[data->nsave - 1]) / magdis;
              data->ysave[data->nsave + 1] =
                  data->ysave[data->nsave] + hand * data->tick_len * (x - data->xsave[data->nsave - 1]) / magdis;
            }
            else {
              data->xsave[data->nsave + 1] = data->xsave[data->nsave];
              data->ysave[data->nsave + 1] = data->ysave[data->nsave];
            }
            data->xsave[data->nsave + 2] = data->xsave[data->nsave];
            data->ysave[data->nsave + 2] = data->ysave[data->nsave];
            data->xsave[data->nsave + 3] = x;
            data->ysave[data->nsave + 3] = y;
            data->isave[data->nsave] = ni;
            data->jsave[data->nsave] = nj;
            data->isave[data->nsave + 1] = ni;
            data->jsave[data->nsave + 1] = nj;
            data->isave[data->nsave + 2] = ni;
            data->jsave[data->nsave + 2] = nj;
            data->isave[data->nsave + 3] = ni;
            data->jsave[data->nsave + 3] = nj;
            data->nsave = data->nsave + 4;
            tick_last = true;
          }
          else {
            data->xsave[data->nsave] = x;
            data->ysave[data->nsave] = y;
            data->isave[data->nsave] = ni;
            data->jsave[data->nsave] = nj;
            data->nsave++;
            tick_last = false;
          }
          i = ni;
          j = nj;
          k = nk;
          d = nd;
          data->pings[i].bflag[k][j] = 0;
        }

        /* clean up if not a full contour */
        if (data->nsave < 2) {
          data->nsave = 0;
          data->pings[i].bflag[k][j] = 0;
        }

        /* set labels if needed */
        if (data->nsave > 0 && label && !closed) {
          /* check beginning of contour */
          int left = data->pings[data->isave[0]].beams_bath / 2;
          int right = data->pings[data->isave[0]].beams_bath / 2;
          for (int jj = 0; jj < data->beams_bath; jj++) {
            if (mb_beam_ok(data->pings[data->isave[0]].beamflag[jj])) {
              if (jj < left)
                left = jj;
              if (jj > right)
                right = jj;
            }
          }
          if (data->jsave[0] == left || data->jsave[0] == left + 1) {
            data->xlabel[data->nlabel] = data->xsave[0];
            data->ylabel[data->nlabel] = data->ysave[0];
            get_azimuth_old(data, data->isave[0], &data->angle[data->nlabel]);
            data->justify[data->nlabel] = 1;
            if (check_label(data, data->nlabel))
              data->nlabel++;
          }
          else if (data->jsave[0] == right || data->jsave[0] == right - 1) {
            data->xlabel[data->nlabel] = data->xsave[0];
            data->ylabel[data->nlabel] = data->ysave[0];
            get_azimuth_old(data, data->isave[0], &data->angle[data->nlabel]);
            data->justify[data->nlabel] = 0;
            if (check_label(data, data->nlabel))
              data->nlabel++;
          }

          /* check end of contour */
          left = data->pings[data->isave[data->nsave - 1]].beams_bath / 2;
          right = data->pings[data->isave[data->nsave - 1]].beams_bath / 2;
          for (int jj = 0; jj < data->pings[data->isave[data->nsave - 1]].beams_bath; jj++) {
            if (mb_beam_ok(data->pings[data->isave[data->nsave - 1]].beamflag[jj])) {
              if (jj < left)
                left = jj;
              if (jj > right)
                right = jj;
            }
          }
          if ((data->nlabel == 0 || data->nsave > 10) &&
              (data->jsave[data->nsave - 1] == left || data->jsave[data->nsave - 1] == left + 1)) {
            data->xlabel[data->nlabel] = data->xsave[data->nsave - 1];
            data->ylabel[data->nlabel] = data->ysave[data->nsave - 1];
            get_azimuth_old(data, data->isave[data->nsave - 1], &data->angle[data->nlabel]);
            data->justify[data->nlabel] = 1;
            if (check_label(data, data->nlabel))
              data->nlabel++;
          }
          else if ((data->nlabel == 0 || data->nsave > 10) &&
                   (data->jsave[data->nsave - 1] == right || data->jsave[data->nsave - 1] == right - 1)) {
            data->xlabel[data->nlabel] = data->xsave[data->nsave - 1];
            data->ylabel[data->nlabel] = data->ysave[data->nsave - 1];
            get_azimuth_old(data, data->isave[data->nsave - 1], &data->angle[data->nlabel]);
            data->justify[data->nlabel] = 0;
            if (check_label(data, data->nlabel))
              data->nlabel++;
          }
        }

        /* dump the contour */
        dump_contour(data, value);
      }

      /* done with contouring this level */
    }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (MB_SUCCESS);
}
/*--------------------------------------------------------------------------*/
/*   function mb_contour calls the appropriate contouring routine. */
int mb_contour(int verbose, struct swath *data, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       data:                    %p\n", data);
    fprintf(stderr, "dbg2       data->contour_alg:       %d\n", data->contour_algorithm);
  }

  /* call the appropriate contouring routine */
  int status = MB_SUCCESS;
  if (data->contour_algorithm == MB_CONTOUR_TRIANGLES) {
    if (data->ntri <= 0) {
      status &= mb_triangulate(verbose, data, error);
    }
    status &= mb_tcontour(verbose, data, error);
    }
  else
    status &= mb_ocontour(verbose, data, error);

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
