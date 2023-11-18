/*--------------------------------------------------------------------
 *    The MB-system:  mbprocess->c  3/31/93
 *
 *    Copyright (c) 2000-2023 by
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
/** @file
 * mbprocess is a tool for processing swath sonar bathymetry data.
 * This program performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit save files.
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter file syntax is documented by
 * comments in the source file mbsystem/src/mbio/mb_process->h
 * and the manual pages for mbprocess and mbset. The program
 * mbset is used to create and modify parameter files.
 * The data format and the input and output data files can be
 * specified using command line options. If no parameter file is
 * specified (using the -P option) but an input file is specified
 * (with the -I option), then mbprocess will look for a parameter
 * file with the path inputfile.par, where inputfile is the input
 * file path.\n";
 *
 * Author:  D. W. Caress
 * Date:  January 4, 2000
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_atlas.h"
#include "mbsys_ldeoih.h"

/* define sidescan correction table structure */
struct mbprocess_sscorr_struct {
  double time_d;
  int nangle;
  double *angle;
  double *amplitude;
  double *sigma;
};

/* define grid structure */
struct mbprocess_grid_struct {
// avoid compiler bug found in gcc 4.8.1 and 4.8.5 (Caress 2020.01.14)
#if __GNUC__ > 4 || __clang__
  mbprocess_grid_struct() :
      file(""),
      projectionname(""),
      projection_mode(0),
      projection_id(""),
      nodatavalue(0.0f),
      nxy(0),
      n_columns(0),
      n_rows(0),
      min(0.0),
      max(0.0),
      xmin(0.0),
      xmax(0.0),
      ymin(0.0),
      ymax(0.0),
      dx(0.0),
      dy(0.0),
      data(nullptr) {}
#endif

 public:
  mb_path file;
  mb_path projectionname;
  int projection_mode;
  mb_path projection_id;
  float nodatavalue;
  int nxy;
  int n_columns;
  int n_rows;
  double min;
  double max;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double dx;
  double dy;
  float *data;
};

constexpr char program_name[] = "mbprocess";
constexpr char help_message[] =
    "mbprocess is a tool for processing swath sonar bathymetry data.\n"
    "This program performs a number of functions, including:\n"
    "  - merging navigation\n"
    "  - recalculating bathymetry from travel time and angle data\n"
    "    by raytracing through a layered water sound velocity model.\n"
    "  - applying changes to ship draft, roll bias and pitch bias\n"
    "  - applying bathymetry edits from edit save files.\n"
    "The parameters controlling mbprocess are included in an ascii\n"
    "parameter file. The parameter file syntax is documented by\n"
    "the manual pages for mbprocess and mbset. The program\n"
    "mbset is used to create and modify parameter files.\n"
    "The input file \"infile\"  must be specified with the -I option. The\n"
    "data format can also be specified, thought the program can\n"
    "infer the format if the standard MB-System suffix convention\n"
    "is used (*.mbXXX where XXX is the MB-System format id number).\n"
    "The program will look for and use a parameter file with the \n"
    "name \"infile.par\". If no parameter file exists, the program \n"
    "will infer a reasonable processing path by looking for navigation\n"
    "and mbedit edit save files.\n";


/*--------------------------------------------------------------------*/
int check_ss_for_bath(int verbose, int nbath, char *beamflag, double *bath, double *bathacrosstrack, int nss, double *ss,
                      double *ssacrosstrack, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
    fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
    fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
    fprintf(stderr, "dbg2       bath:\n");
    for (int i = 0; i < nbath; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, bath[i], bathacrosstrack[i]);
  }

  /* find limits of good bathy */
  int ifirst = -1;
  int ilast = -1;
  for (int i = 0; i < nbath; i++) {
    if (mb_beam_ok(beamflag[i])) {
      if (ifirst < 0)
        ifirst = i;
      ilast = i;
    }
  }

  /* loop over sidescan looking for bathy on either side
     - zero sidescan if bathy lacking */
  if (ifirst < ilast) {
    int ibath = ifirst;
    for (int iss = 0; iss < nss; iss++) {
      /* make sure ibath sets right interval for ss */
      while (ibath < ilast - 1 && (!mb_beam_ok(beamflag[ibath]) || !mb_beam_ok(beamflag[ibath + 1]) ||
                                   (mb_beam_ok(beamflag[ibath + 1]) && ssacrosstrack[iss] > bathacrosstrack[ibath + 1])))
        ibath++;

      /* now zero sidescan if not surrounded by good bathy */
      if (!mb_beam_ok(beamflag[ibath]) || !mb_beam_ok(beamflag[ibath + 1]))
        ss[iss] = 0.0;
      else if (ssacrosstrack[iss] < bathacrosstrack[ibath])
        ss[iss] = 0.0;
      else if (ssacrosstrack[iss] > bathacrosstrack[ibath + 1])
        ss[iss] = 0.0;
    }
  }

  /* else if no good bathy zero all sidescan */
  else {
    for (int iss = 0; iss < nss; iss++) {
      ss[iss] = 0.0;
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int get_corrtable(int verbose, double time_d, int ncorrtable, int ncorrangle, struct mbprocess_sscorr_struct *corrtable,
                  struct mbprocess_sscorr_struct *corrtableuse, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
    fprintf(stderr, "dbg2       time_d:      %f\n", time_d);
    fprintf(stderr, "dbg2       ncorrtable:  %d\n", ncorrtable);
    fprintf(stderr, "dbg2       ncorrangle:  %d\n", ncorrangle);
    fprintf(stderr, "dbg2       corrtable:   %p\n", (void *)corrtable);
  }

  /* find the correction table */
  if (ncorrtable == 1 || time_d <= corrtable[0].time_d) {
    corrtableuse->time_d = corrtable[0].time_d;
    corrtableuse->nangle = corrtable[0].nangle;
    for (int i = 0; i < ncorrangle; i++) {
      corrtableuse->angle[i] = corrtable[0].angle[i];
      corrtableuse->amplitude[i] = corrtable[0].amplitude[i];
      corrtableuse->sigma[i] = corrtable[0].sigma[i];
    }
  }
  else if (time_d > corrtable[ncorrtable - 1].time_d) {
    corrtableuse->time_d = corrtable[ncorrtable - 1].time_d;
    corrtableuse->nangle = corrtable[ncorrtable - 1].nangle;
    for (int i = 0; i < ncorrangle; i++) {
      corrtableuse->angle[i] = corrtable[ncorrtable - 1].angle[i];
      corrtableuse->amplitude[i] = corrtable[ncorrtable - 1].amplitude[i];
      corrtableuse->sigma[i] = corrtable[ncorrtable - 1].sigma[i];
    }
  }
  else {
    int itable = 0;
    for (int i = 0; i < ncorrtable - 1; i++) {
      if (corrtable[i].time_d <= time_d && corrtable[i + 1].time_d > time_d)
        itable = i;
    }
    const double factor = (time_d - corrtable[itable].time_d) / (corrtable[itable + 1].time_d - corrtable[itable].time_d);
    corrtableuse->time_d = time_d;
    corrtableuse->nangle = std::min(corrtable[itable+1].nangle, corrtable[itable].nangle);
    for (int i = 0; i < corrtableuse->nangle; i++) {
      corrtableuse->angle[i] =
          corrtable[itable].angle[i] + factor * (corrtable[itable + 1].angle[i] - corrtable[itable].angle[i]);
      if (corrtable[itable].amplitude[i] != 0.0 && corrtable[itable + 1].amplitude[i] != 0.0) {
        corrtableuse->amplitude[i] = corrtable[itable].amplitude[i] +
                                     factor * (corrtable[itable + 1].amplitude[i] - corrtable[itable].amplitude[i]);
        corrtableuse->sigma[i] =
            corrtable[itable].sigma[i] + factor * (corrtable[itable + 1].sigma[i] - corrtable[itable].sigma[i]);
      }
      else if (corrtable[itable].amplitude[i] != 0.0) {
        corrtableuse->amplitude[i] = corrtable[itable].amplitude[i];
        corrtableuse->sigma[i] = corrtable[itable].sigma[i];
      }
      else {
        corrtableuse->amplitude[i] = corrtable[itable + 1].amplitude[i];
        corrtableuse->sigma[i] = corrtable[itable + 1].sigma[i];
      }
    }
  }

  /* now interpolate or extrapolate any zero values */
  int ifirst = ncorrangle;
  int ilast = -1;
  for (int i = 0; i < ncorrangle; i++) {
    if (corrtableuse->amplitude[i] != 0.0) {
      ifirst = std::min(i, ifirst);
      ilast = std::max(i, ilast);
    }
  }

  int irecent = 0;
  int inext;
  for (int i = 0; i < ncorrangle; i++) {
    if (corrtableuse->amplitude[i] != 0.0)
      irecent = i;
    if (i < ifirst) {
      corrtableuse->amplitude[i] = corrtableuse->amplitude[ifirst];
      corrtableuse->sigma[i] = corrtableuse->sigma[ifirst];
    }
    else if (i > ilast) {
      corrtableuse->amplitude[i] = corrtableuse->amplitude[ilast];
      corrtableuse->sigma[i] = corrtableuse->sigma[ilast];
    }
    else if (corrtableuse->amplitude[i] == 0.0) {
      inext = -1;
      for (int ii = i + 1; ii < ilast; ii++) {
        if (corrtableuse->amplitude[ii] != 0.0 && inext < 0)
          inext = ii;
      }
      if (irecent < i && inext > i) {
        const double factor = ((double)(i - irecent)) / ((double)(inext - irecent));
        corrtableuse->amplitude[i] = corrtableuse->amplitude[irecent] +
                                     factor * (corrtableuse->amplitude[inext] - corrtableuse->amplitude[irecent]);
        corrtableuse->sigma[i] =
            corrtableuse->sigma[irecent] + factor * (corrtableuse->sigma[inext] - corrtableuse->sigma[irecent]);
      }
    }
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ncorrangle:      %d\n", ncorrangle);
    for (int i = 0; i < ncorrangle; i++)
      fprintf(stderr, "dbg2       correction[%d]: %f %f %f\n", i, corrtableuse->angle[i], corrtableuse->amplitude[i],
              corrtableuse->sigma[i]);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int get_anglecorr(int verbose, int nangle, double *angles, double *corrs, double angle, double *corr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       nangle:      %d\n", nangle);
    fprintf(stderr, "dbg2       angles:      %p\n", (void *)angles);
    fprintf(stderr, "dbg2       corrs:       %p\n", (void *)corrs);
    for (int i = 0; i < nangle; i++)
      fprintf(stderr, "dbg2           angle[%d]:%f corrs[%d]:%f\n", i, angles[i], i, corrs[i]);
    fprintf(stderr, "dbg2       angle:       %f\n", angle);
  }

  int iangle;
  /* search for the specified angle */
  bool found = false;
  for (int i = 0; i < nangle - 1; i++) {
    if (angle >= angles[i] && angle <= angles[i + 1]) {
      found = true;
      iangle = i;
    }
  }

  /* interpolate the correction */
  if (found) {
    *corr = corrs[iangle] +
            (corrs[iangle + 1] - corrs[iangle]) * (angle - angles[iangle]) / (angles[iangle + 1] - angles[iangle]);
  }   else if (angle < angles[0]) {
    // iangle = 0;
    *corr = corrs[0];
  }   else if (angle > angles[nangle - 1]) {
    // iangle = nangle - 1;
    *corr = corrs[nangle - 1];
  } else {
    *corr = 0.0;
  }

  /* use outermost value if angle outside nonzero range */
  if (*corr == 0.0) {
    int ifirst = nangle - 1;
    int ilast = 0;
    for (int i = 0; i < nangle; i++) {
      if (corr[i] != 0.0) {
        if (ifirst > i)
          ifirst = i;
        if (ilast < i)
          ilast = i;
      }
    }
    if (angle < 0.0)
      *corr = corrs[ifirst];
    if (angle > 0.0)
      *corr = corrs[ilast];
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBPROCESS function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       corr:            %f\n", *corr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbprocess_save_edit(int verbose, FILE *esffp, double time_d, int beam, int action, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");

    fprintf(stderr, "dbg2       esffp:           %p\n", (void *)esffp);
    fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
    fprintf(stderr, "dbg2       beam:            %d\n", beam);
    fprintf(stderr, "dbg2       action:          %d\n", action);
  }

  int status = MB_SUCCESS;

  /* write out the edit */
  if (esffp != nullptr) {
#ifdef BYTESWAPPED
    mb_swap_double(&time_d);
    beam = mb_swap_int(beam);
    action = mb_swap_int(action);
#endif
    if (fwrite(&time_d, sizeof(double), 1, esffp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
    if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, esffp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
    if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, esffp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
void process_file(int verbose, int thread_id, struct mb_process_struct *process,
                  struct mbprocess_grid_struct *grid, int *status, int *error)
{

  /* MBIO read and write control parameters */
  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  void *imbio_ptr = nullptr;
  void *ombio_ptr = nullptr;
  int platform_source;
  int nav_source;
  int sensordepth_source;
  int heading_source;
  int attitude_source;
  int svp_source;

  /* mbio read and write values */
  void *store_ptr = nullptr;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  double draft;
  double roll;
  double pitch;
  double heave;
  int nbath;
  int namp;
  int nss;
  char *beamflag = nullptr;
  char *beamflagorg = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  int idata = 0;
  int inav = 0;
  int icomment = 0;
  int iother = 0;
  int odata = 0;
  int onav = 0;
  int ocomment = 0;
  int oother = 0;
  char comment[MB_COMMENT_MAXLINE];

  /* sidescan recalculation */
  int pixel_size_set;
  int swath_width_set;
  int pixel_int;
  double pixel_size = 0;
  double swath_width;

  /* processing variables */
  int variable_beams = false;
  int traveltime = false;
  int beam_flagging = false;
  char mbp_pfile[MBP_FILENAMESIZE];
  FILE *tfp;
  int nnav = 0;
  int nanav = 0;
  int nattitude = 0;
  int nsensordepth = 0;
  int ntide = 0;
  int nstatic = 0;
  int size, nchar;
  int time_j[5], stime_i[7], ftime_i[7];
  int ihr;
  double sec, hr;
  int quality, nsatellite, dilution, gpsheight;
  char *bufftmp;
  char NorS[2], EorW[2];
  double mlon, llon, mlat, llat;
  int degree;
  double dminute;
  double splineflag;
  double *ntime = nullptr;
  double *nlon = nullptr;
  double *nlat = nullptr;
  double *nheading = nullptr;
  double *nspeed = nullptr;
  double *ndraft = nullptr;
  double *nroll = nullptr;
  double *npitch = nullptr;
  double *nheave = nullptr;
  double *natime = nullptr;
  double *nalon = nullptr;
  double *nalat = nullptr;
  double *naz = nullptr;
  double zoffset;
  double *nlonspl = nullptr;
  double *nlatspl = nullptr;
  double *nalonspl = nullptr;
  double *nalatspl = nullptr;
  double *nazspl = nullptr;
  double *attitudetime = nullptr;
  double *attituderoll = nullptr;
  double *attitudepitch = nullptr;
  double *attitudeheave = nullptr;
  double *fsensordepthtime = nullptr;
  double *fsensordepth = nullptr;
  double *tidetime = nullptr;
  double *tide = nullptr;
  double tideval;
  int *staticbeam = nullptr;
  double *staticangle;
  double *staticoffset = nullptr;
  double headingx, headingy;
  double mtodeglon, mtodeglat;
  double del_time, dx, dy, dist;
  double headingcalc, speedcalc;
  double lever_x = 0.0;
  double lever_y = 0.0;
  double lever_heave = 0.0;
  double time_d_old = 0.0;
  double navlon_old = 0.0;
  double navlat_old = 0.0;
  double speed_old = 0.0;
  double heading_old = 0.0;
  double *depth = nullptr;
  double *velocity = nullptr;
  double *velocity_sum = nullptr;
  void *rt_svp = nullptr;
  double ssv;
  int sensorhead = 0;
  int sensortype = 0;

  /* edit save file control variables */
  struct mb_esf_struct esf;
  memset(&esf, 0, sizeof(struct mb_esf_struct));
  int neditnull;
  int neditduplicate;
  int neditnotused;
  int neditused;

  /* output reverse edit save file control variables */
  char resf_file[MB_PATH_MAXLINE+10];
  FILE *resf_fp = nullptr;
  mb_path resf_header;
  int action;

  double draft_org, depth_offset_use, depth_offset_change, depth_offset_org, static_shift;
  double roll_org, pitch_org, heave_org, heading_org;
  double ttime, range;
  double xx, zz, rr, vsum, vavg;
  double alpha, beta;
  double alphar, betar;
  int ray_stat;
  double *ttimes = nullptr;
  double *angles = nullptr;
  double *angles_forward = nullptr;
  double *angles_null = nullptr;
  double *bheave = nullptr;
  double *alongtrack_offset = nullptr;

  /* ssv handling variables */
  bool ssv_prelimpass = false;
  double ssv_default;
  double ssv_start;

  /* sidescan correction */
  double altitude_default = 1000.0;
  int nsmooth = 5;
  double reference_amp;
  double reference_amp_port;
  double reference_amp_stbd;
  int itable;
  int nsscorrtable = 0;
  int nsscorrangle = 0;
  struct mbprocess_sscorr_struct *sscorrtable = nullptr;
  struct mbprocess_sscorr_struct sscorrtableuse;
  int nampcorrtable = 0;
  int nampcorrangle = 0;
  struct mbprocess_sscorr_struct *ampcorrtable = nullptr;
  struct mbprocess_sscorr_struct ampcorrtableuse;
  int ndepths;
  double *depths = nullptr;
  double *depthsmooth = nullptr;
  double *depthacrosstrack = nullptr;
  int nslopes;
  double *slopes = nullptr;
  double *slopeacrosstrack = nullptr;
  double r[3];
  double v1[3], v2[3], v[3], vv;
  double slope;
  double bathy;
  double altitude_use;
  double angle;
  double correction;

  int pings;
  int format;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  *status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  char buffer[MBP_FILENAMESIZE];
  char dummy[MBP_FILENAMESIZE];
  double factor;
  int pingmultiplicity;
  int nbeams;
  int istart, iend, icut;
  int ioff;
  int mm;

  /* check for nav format with heading, speed, and draft merge */
  if (process->mbp_nav_mode == MBP_NAV_ON &&
      (process->mbp_nav_heading == MBP_NAV_ON || process->mbp_nav_speed == MBP_NAV_ON ||
       process->mbp_nav_draft == MBP_NAV_ON || process->mbp_nav_attitude == MBP_NAV_ON) &&
      process->mbp_nav_format != 9) {
    fprintf(stderr, "\nWarning:\n\tNavigation format <%d> does not include \n", process->mbp_nav_format);
    fprintf(stderr, "\theading, speed, draft, roll, pitch and heave values.\n");
    if (process->mbp_nav_heading == MBP_NAV_ON) {
      fprintf(stderr, "Merging of heading data disabled.\n");
      process->mbp_nav_heading = MBP_NAV_OFF;
    }
    if (process->mbp_nav_speed == MBP_NAV_ON) {
      fprintf(stderr, "Merging of speed data disabled.\n");
      process->mbp_nav_speed = MBP_NAV_OFF;
    }
    if (process->mbp_nav_draft == MBP_NAV_ON) {
      fprintf(stderr, "Merging of draft data disabled.\n");
      process->mbp_nav_draft = MBP_NAV_OFF;
    }
    if (process->mbp_nav_attitude == MBP_NAV_ON) {
      fprintf(stderr, "Merging of roll, pitch, and heave data disabled.\n");
      process->mbp_nav_attitude = MBP_NAV_OFF;
    }
  }

  /* check for format with travel time data */  // TODO(schwehr): Make mb_format_flags take bools.
  *status = mb_format_flags(verbose, &process->mbp_format, &variable_beams, &traveltime, &beam_flagging, error);
  if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE && !traveltime) {
    fprintf(stderr, "\nWarning:\n\tFormat %d does not include travel time data.\n", process->mbp_format);
    fprintf(stderr, "\tTravel times and angles estimated assuming\n");
    fprintf(stderr, "\t1500 m/s water sound speed.\n");
  }

  /* check for right format if recalculating sidescan is on */
  if (process->mbp_ssrecalc_mode == MBP_SSRECALC_ON
      && process->mbp_format != MBF_EM300MBA
      && process->mbp_format != MBF_EM710MBA
      && process->mbp_format != MBF_KEMKMALL
      && process->mbp_format != MBF_RESON7KR
      && process->mbp_format != MBF_RESON7K3) {
    fprintf(stderr, "\nProgram <%s> does not recalculate sidescan for format %d\n", program_name, process->mbp_format);
    fprintf(stderr, "Sidescan recalculation disabled\n");
    process->mbp_ssrecalc_mode = MBP_SSRECALC_OFF;
  }

  if (verbose == 1) {
    fprintf(stderr, "\nInput and Output Files:\n");
    if (process->mbp_format_specified)
      fprintf(stderr, "  Format:                        %d\n", process->mbp_format);
    fprintf(stderr, "  Input file:                    %s\n", process->mbp_ifile);
    fprintf(stderr, "  Output file:                   %s\n", process->mbp_ofile);
    if (process->mbp_strip_comments)
      fprintf(stderr, "  Comments in output:            OFF\n");
    else
      fprintf(stderr, "  Comments in output:            ON\n");

    fprintf(stderr, "\nNavigation Merging:\n");
    if (process->mbp_nav_mode == MBP_NAV_ON) {
      fprintf(stderr, "  Navigation merged from navigation file.\n");
      fprintf(stderr, "  Navigation file:               %s\n", process->mbp_navfile);
      fprintf(stderr, "  Navigation format:             %d\n", process->mbp_nav_format);
      if (process->mbp_nav_heading == MBP_NAV_ON)
        fprintf(stderr, "  Heading merged from navigation file.\n");
      else
        fprintf(stderr, "  Heading not merged from navigation file.\n");
      if (process->mbp_nav_speed == MBP_NAV_ON)
        fprintf(stderr, "  Speed merged from navigation file.\n");
      else
        fprintf(stderr, "  Speed not merged from navigation file.\n");
      if (process->mbp_nav_draft == MBP_NAV_ON)
        fprintf(stderr, "  Draft merged from navigation file.\n");
      else
        fprintf(stderr, "  Draft not merged from navigation file.\n");
      if (process->mbp_nav_attitude == MBP_NAV_ON)
        fprintf(stderr, "  Roll, pitch, and heave merged from navigation file.\n");
      else
        fprintf(stderr, "  Roll, pitch, and heave not merged from navigation file.\n");
      if (process->mbp_nav_algorithm == MBP_NAV_LINEAR)
        fprintf(stderr, "  Navigation algorithm:          linear interpolation\n");
      else if (process->mbp_nav_algorithm == MBP_NAV_SPLINE)
        fprintf(stderr, "  Navigation algorithm:          spline interpolation\n");
      fprintf(stderr, "  Navigation time shift:         %f\n", process->mbp_nav_timeshift);
    }
    else
      fprintf(stderr, "  Navigation not merged from navigation file.\n");

    fprintf(stderr, "\nNavigation Offsets and Shifts:\n");
    if (process->mbp_nav_shift == MBP_NAV_ON) {
      fprintf(stderr, "  Navigation positions shifted.\n");
      fprintf(stderr, "  Navigation offset x:       %f\n", process->mbp_nav_offsetx);
      fprintf(stderr, "  Navigation offset y:       %f\n", process->mbp_nav_offsety);
      fprintf(stderr, "  Navigation offset z:       %f\n", process->mbp_nav_offsetz);
      fprintf(stderr, "  Navigation shift longitude:%f\n", process->mbp_nav_shiftlon);
      fprintf(stderr, "  Navigation shift latitude: %f\n", process->mbp_nav_shiftlat);
    }
    else
      fprintf(stderr, "  Navigation positions not shifted.\n");

    fprintf(stderr, "\nAdjusted Navigation Merging:\n");
    if (process->mbp_navadj_mode >= MBP_NAVADJ_LL) {
      fprintf(stderr, "  Navigation merged from adjusted navigation file.\n");
      fprintf(stderr, "  Adjusted navigation file:      %s\n", process->mbp_navadjfile);
      if (process->mbp_navadj_mode == MBP_NAVADJ_LL)
        fprintf(stderr, "  Adjusted navigation applied to: lon lat only\n");
      else if (process->mbp_navadj_mode == MBP_NAVADJ_LLZ)
        fprintf(stderr, "  Adjusted navigation applied to: lon lat depth_offset\n");
      if (process->mbp_navadj_algorithm == MBP_NAV_LINEAR)
        fprintf(stderr, "  Adjusted navigation algorithm: linear interpolation\n");
      else if (process->mbp_navadj_algorithm == MBP_NAV_SPLINE)
        fprintf(stderr, "  Adjusted navigation algorithm: spline interpolation\n");
    }
    else
      fprintf(stderr, "  Navigation not merged from adjusted navigation file.\n");

    fprintf(stderr, "\nAttitude Merging:\n");
    if (process->mbp_attitude_mode == MBP_ATTITUDE_ON) {
      fprintf(stderr, "  Attitude merged from attitude file.\n");
      fprintf(stderr, "  Attitude file:                 %s\n", process->mbp_attitudefile);
      fprintf(stderr, "  Attitude format:               %d\n", process->mbp_attitude_format);
    }
    else
      fprintf(stderr, "  Attitude not merged from attitude file.\n");

    fprintf(stderr, "\nSensordepth Merging:\n");
    if (process->mbp_sensordepth_mode == MBP_SENSORDEPTH_ON) {
      fprintf(stderr, "  Sensordepth merged from sensordepth file.\n");
      fprintf(stderr, "  Sensordepth file:                 %s\n", process->mbp_sensordepthfile);
      fprintf(stderr, "  Sensordepth format:               %d\n", process->mbp_sensordepth_format);
    }
    else
      fprintf(stderr, "  Sensordepth not merged from sensordepth file.\n");

    fprintf(stderr, "\nData Cutting:\n");
    if (process->mbp_cut_num > 0)
      fprintf(stderr, "  Data cutting enabled (%d commands).\n", process->mbp_cut_num);
    else
      fprintf(stderr, "  Data cutting disabled.\n");
    for (int i = 0; i < process->mbp_cut_num; i++) {
      if (process->mbp_cut_kind[i] == MBP_CUT_DATA_BATH)
        fprintf(stderr, "  Cut[%d]: bathymetry", i);
      else if (process->mbp_cut_kind[i] == MBP_CUT_DATA_AMP)
        fprintf(stderr, "  Cut[%d]: amplitude ", i);
      else if (process->mbp_cut_kind[i] == MBP_CUT_DATA_SS)
        fprintf(stderr, "  Cut[%d]: sidescan  ", i);
      if (process->mbp_cut_mode[i] == MBP_CUT_MODE_NUMBER)
        fprintf(stderr, "  number   ");
      else if (process->mbp_cut_kind[i] == MBP_CUT_MODE_DISTANCE)
        fprintf(stderr, "  distance ");
      else if (process->mbp_cut_kind[i] == MBP_CUT_MODE_SPEED)
        fprintf(stderr, "  speed    ");
      fprintf(stderr, "  %f %f\n", process->mbp_cut_min[i], process->mbp_cut_max[i]);
    }

    fprintf(stderr, "\nBathymetry Editing:\n");
    if (process->mbp_edit_mode == MBP_EDIT_ON)
      fprintf(stderr, "  Bathymetry edits applied from file.\n");
    else
      fprintf(stderr, "  Bathymetry edits not applied from file.\n");
    fprintf(stderr, "  Bathymetry edit file:          %s\n", process->mbp_editfile);

    fprintf(stderr, "\nBathymetry Recalculation:\n");
    if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
      fprintf(stderr, "  Bathymetry not recalculated.\n");
    else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
      fprintf(stderr, "  Bathymetry recalculated by raytracing.\n");
    else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
      fprintf(stderr, "  Bathymetry recalculated by rigid rotation.\n");
    else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
      fprintf(stderr, "  Bathymetry recalculated by sonar depth shift.\n");
    fprintf(stderr, "  SVP file:                      %s\n", process->mbp_svpfile);
    if (process->mbp_ssv_mode == MBP_SSV_OFF)
      fprintf(stderr, "  SSV not modified.\n");
    else if (process->mbp_ssv_mode == MBP_SSV_OFFSET)
      fprintf(stderr, "  SSV offset by constant.\n");
    else
      fprintf(stderr, "  SSV set to constant.\n");
    fprintf(stderr, "  SSV offset/constant:           %f m/s\n", process->mbp_ssv);
    fprintf(stderr, "  Travel time mode:              %d\n", process->mbp_tt_mode);
    fprintf(stderr, "  Travel time multiplier:        %f\n", process->mbp_tt_mult);
    fprintf(stderr, "  Raytrace angle mode:           %d\n", process->mbp_angle_mode);

    fprintf(stderr, "\nStatic Beam Bathymetry Corrections:\n");
    if (process->mbp_static_mode == MBP_STATIC_BEAM_ON) {
      fprintf(stderr, "  Static beam corrections applied to bathymetry.\n");
      fprintf(stderr, "  Static file:                   %s m\n", process->mbp_staticfile);
    }
    else if (process->mbp_static_mode == MBP_STATIC_ANGLE_ON) {
      fprintf(stderr, "  Static angle corrections applied to bathymetry.\n");
      fprintf(stderr, "  Static file:                   %s m\n", process->mbp_staticfile);
    }
    else
      fprintf(stderr, "  Static beam corrections off.\n");

    fprintf(stderr, "\nBathymetry Water Sound Speed Reference:\n");
    if (process->mbp_corrected)
      fprintf(stderr, "  Output bathymetry reference:   CORRECTED\n");
    else
      fprintf(stderr, "  Output bathymetry reference:   UNCORRECTED\n");
    if (process->mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF) {
      if (process->mbp_corrected)
        fprintf(stderr, "  Depths modified from uncorrected to corrected\n");
      else
        fprintf(stderr, "  Depths modified from corrected to uncorrected\n");
    }
    else if (process->mbp_svp_mode == MBP_SVP_ON) {
      if (process->mbp_corrected)
        fprintf(stderr, "  Depths recalculated as corrected\n");
      else
        fprintf(stderr, "  Depths recalculated as uncorrected\n");
    }
    else {
      fprintf(stderr, "  Depths unmodified with respect to water sound speed reference\n");
    }

    fprintf(stderr, "\nDraft Correction:\n");
    if (process->mbp_draft_mode == MBP_DRAFT_OFF)
      fprintf(stderr, "  Draft not modified.\n");
    else if (process->mbp_draft_mode == MBP_DRAFT_SET)
      fprintf(stderr, "  Draft set to constant.\n");
    else if (process->mbp_draft_mode == MBP_DRAFT_OFFSET)
      fprintf(stderr, "  Draft offset by constant.\n");
    else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLY)
      fprintf(stderr, "  Draft multiplied by constant.\n");
    else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
      fprintf(stderr, "  Draft multiplied and offset by constants.\n");
    fprintf(stderr, "  Draft constant:                %f m\n", process->mbp_draft);
    fprintf(stderr, "  Draft offset:                  %f m\n", process->mbp_draft_offset);
    fprintf(stderr, "  Draft multiplier:              %f m\n", process->mbp_draft_mult);

    fprintf(stderr, "\nHeave Correction:\n");
    if (process->mbp_heave_mode == MBP_HEAVE_OFF)
      fprintf(stderr, "  Heave not modified.\n");
    else if (process->mbp_heave_mode == MBP_HEAVE_OFFSET)
      fprintf(stderr, "  Heave offset by constant.\n");
    else if (process->mbp_heave_mode == MBP_HEAVE_MULTIPLY)
      fprintf(stderr, "  Heave multiplied by constant.\n");
    else if (process->mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
      fprintf(stderr, "  Heave multiplied and offset by constants.\n");
    fprintf(stderr, "  Heave offset:                  %f m\n", process->mbp_heave);
    fprintf(stderr, "  Heave multiplier:              %f m\n", process->mbp_heave_mult);

    fprintf(stderr, "\nLever Correction:\n");
    if (process->mbp_lever_mode == MBP_LEVER_OFF)
      fprintf(stderr, "  Lever calculation off.\n");
    else {
      fprintf(stderr, "  Lever calculation used to calculate heave correction.\n");
      fprintf(stderr, "  Heave offset:                  %f m\n", process->mbp_heave);
      fprintf(stderr, "  VRU offset x:                  %f m\n", process->mbp_vru_offsetx);
      fprintf(stderr, "  VRU offset y:                  %f m\n", process->mbp_vru_offsety);
      fprintf(stderr, "  VRU offset z:                  %f m\n", process->mbp_vru_offsetz);
      fprintf(stderr, "  Sonar offset x:                %f m\n", process->mbp_sonar_offsetx);
      fprintf(stderr, "  Sonar offset y:                %f m\n", process->mbp_sonar_offsety);
      fprintf(stderr, "  Sonar offset z:                %f m\n", process->mbp_sonar_offsetz);
    }

    fprintf(stderr, "\nTide Correction:\n");
    if (process->mbp_tide_mode == MBP_TIDE_OFF)
      fprintf(stderr, "  Tide calculation off.\n");
    else {
      fprintf(stderr, "  Tide correction applied to bathymetry.\n");
      fprintf(stderr, "  Tide file:                     %s\n", process->mbp_tidefile);
      fprintf(stderr, "  Tide format:                   %d\n", process->mbp_tide_format);
    }

    fprintf(stderr, "\nRoll Correction:\n");
    if (process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
      fprintf(stderr, "  Roll not modified.\n");
    else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
      fprintf(stderr, "  Roll offset by bias.\n");
    else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
      fprintf(stderr, "  Roll offset by separate port and starboard biases.\n");
    fprintf(stderr, "  Roll bias:                     %f deg\n", process->mbp_rollbias);
    fprintf(stderr, "  Port roll bias:                %f deg\n", process->mbp_rollbias_port);
    fprintf(stderr, "  Starboard roll bias:           %f deg\n", process->mbp_rollbias_stbd);

    fprintf(stderr, "\nPitch Correction:\n");
    if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
      fprintf(stderr, "  Pitch not modified.\n");
    else
      fprintf(stderr, "  Pitch offset by bias.\n");
    fprintf(stderr, "  Pitch bias:                    %f deg\n", process->mbp_pitchbias);

    fprintf(stderr, "\nHeading Correction:\n");
    if (process->mbp_heading_mode == MBP_HEADING_OFF)
      fprintf(stderr, "  Heading not modified.\n");
    else if (process->mbp_heading_mode == MBP_HEADING_CALC)
      fprintf(stderr, "  Heading replaced by course-made-good.\n");
    else if (process->mbp_heading_mode == MBP_HEADING_OFFSET)
      fprintf(stderr, "  Heading offset by bias.\n");
    else if (process->mbp_heading_mode == MBP_HEADING_CALCOFFSET)
      fprintf(stderr, "  Heading replaced by course-made-good and then offset by bias.\n");
    fprintf(stderr, "  Heading offset:                %f deg\n", process->mbp_headingbias);

    fprintf(stderr, "\nAmplitude Corrections:\n");
    if (process->mbp_ampcorr_mode == MBP_AMPCORR_ON) {
      fprintf(stderr, "  Amplitude vs grazing angle corrections applied to amplitudes.\n");
      fprintf(stderr, "  Amplitude correction file:      %s m\n", process->mbp_ampcorrfile);
      if (process->mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION)
        fprintf(stderr, "  Amplitude correction by subtraction (dB scale)\n");
      else
        fprintf(stderr, "  Amplitude correction by division (linear scale)\n");
      if (process->mbp_ampcorr_symmetry == MBP_AMPCORR_SYMMETRIC)
        fprintf(stderr, "  AVGA tables forced to be symmetric\n");
      else
        fprintf(stderr, "  AVGA tables allowed to be asymmetric\n");
      fprintf(stderr, "  Reference grazing angle:       %f deg\n", process->mbp_ampcorr_angle);
      if (process->mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE || process->mbp_ampcorr_slope == MBP_AMPCORR_USESLOPE)
        fprintf(stderr, "  Amplitude correction uses swath bathymetry in file\n");
      else {
        fprintf(stderr, "  Amplitude correction uses topography grid\n");
        fprintf(stderr, "  Topography grid file:      %s m\n", process->mbp_ampsscorr_topofile);
      }
      if (process->mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE || process->mbp_ampcorr_slope == MBP_SSCORR_USETOPO)
        fprintf(stderr, "  Amplitude correction ignores seafloor slope\n");
      else
        fprintf(stderr, "  Amplitude correction uses seafloor slope\n");
    }
    else
      fprintf(stderr, "  Amplitude correction off.\n");

    fprintf(stderr, "\nSidescan Corrections:\n");
    if (process->mbp_sscorr_mode == MBP_SSCORR_ON) {
      fprintf(stderr, "  Amplitude vs grazing angle corrections applied to sidescan.\n");
      fprintf(stderr, "  Sidescan correction file:      %s m\n", process->mbp_sscorrfile);
      if (process->mbp_sscorr_type == MBP_SSCORR_SUBTRACTION)
        fprintf(stderr, "  Sidescan correction by subtraction (dB scale)\n");
      else
        fprintf(stderr, "  Sidescan correction by division (linear scale)\n");
      if (process->mbp_sscorr_symmetry == MBP_SSCORR_SYMMETRIC)
        fprintf(stderr, "  AVGA tables forced to be symmetric\n");
      else
        fprintf(stderr, "  AVGA tables allowed to be asymmetric\n");
      fprintf(stderr, "  Reference grazing angle:       %f deg\n", process->mbp_sscorr_angle);
      if (process->mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE || process->mbp_sscorr_slope == MBP_SSCORR_USESLOPE)
        fprintf(stderr, "  Sidescan correction uses swath bathymetry in file\n");
      else {
        fprintf(stderr, "  Sidescan correction uses topography grid\n");
        fprintf(stderr, "  Topography grid file:      %s m\n", process->mbp_ampsscorr_topofile);
      }
      if (process->mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE || process->mbp_sscorr_slope == MBP_SSCORR_USETOPO)
        fprintf(stderr, "  Sidescan correction ignores seafloor slope\n");
      else
        fprintf(stderr, "  Sidescan correction uses seafloor slope\n");
    }
    else
      fprintf(stderr, "  Sidescan correction off.\n");

    fprintf(stderr, "\nSidescan Recalculation:\n");
    if (process->mbp_ssrecalc_mode == MBP_SSRECALC_ON) {
      fprintf(stderr, "  Sidescan recalculated.\n");
      fprintf(stderr, "  Sidescan pixel size:           %f\n", process->mbp_ssrecalc_pixelsize);
      fprintf(stderr, "  Sidescan swath width:          %f\n", process->mbp_ssrecalc_swathwidth);
      fprintf(stderr, "  Sidescan interpolation:        %d\n", process->mbp_ssrecalc_interpolate);
    }
    else
      fprintf(stderr, "  Sidescan not recalculated.\n");

    fprintf(stderr, "\nMetadata Insertion:\n");
    fprintf(stderr, "  Metadata vessel:               %s\n", process->mbp_meta_vessel);
    fprintf(stderr, "  Metadata institution:          %s\n", process->mbp_meta_institution);
    fprintf(stderr, "  Metadata platform:             %s\n", process->mbp_meta_platform);
    fprintf(stderr, "  Metadata sonar:                %s\n", process->mbp_meta_sonar);
    fprintf(stderr, "  Metadata sonarversion:         %s\n", process->mbp_meta_sonarversion);
    fprintf(stderr, "  Metadata cruiseid:             %s\n", process->mbp_meta_cruiseid);
    fprintf(stderr, "  Metadata cruisename:           %s\n", process->mbp_meta_cruisename);
    fprintf(stderr, "  Metadata pi:                   %s\n", process->mbp_meta_pi);
    fprintf(stderr, "  Metadata piinstitution:        %s\n", process->mbp_meta_piinstitution);
    fprintf(stderr, "  Metadata client:               %s\n", process->mbp_meta_client);
    fprintf(stderr, "  Metadata svcorrected:          %d\n", process->mbp_meta_svcorrected);
    fprintf(stderr, "  Metadata tidecorrected         %d\n", process->mbp_meta_tidecorrected);
    fprintf(stderr, "  Metadata batheditmanual        %d\n", process->mbp_meta_batheditmanual);
    fprintf(stderr, "  Metadata batheditauto:         %d\n", process->mbp_meta_batheditauto);
    fprintf(stderr, "  Metadata rollbias:             %f\n", process->mbp_meta_rollbias);
    fprintf(stderr, "  Metadata pitchbias:            %f\n", process->mbp_meta_pitchbias);
    fprintf(stderr, "  Metadata headingbias:          %f\n", process->mbp_meta_headingbias);
    fprintf(stderr, "  Metadata draft:                %f\n", process->mbp_meta_draft);

    fprintf(stderr, "\nProcessing Kluges:\n");
    fprintf(stderr, "  Kluge001:                      %d\n", process->mbp_kluge001);
    fprintf(stderr, "  Kluge002:                      %d\n", process->mbp_kluge002);
    fprintf(stderr, "  Kluge003:                      %d\n", process->mbp_kluge003);
    fprintf(stderr, "  Kluge004:                      %d\n", process->mbp_kluge004);
    fprintf(stderr, "  Kluge005:                      %d\n", process->mbp_kluge005);
    fprintf(stderr, "  Kluge006:                      %d\n", process->mbp_kluge006);
    fprintf(stderr, "  Kluge007:                      %d\n", process->mbp_kluge007);
    fprintf(stderr, "  Kluge008:                      %d\n", process->mbp_kluge008);
    fprintf(stderr, "  Kluge009:                      %d\n", process->mbp_kluge009);
    fprintf(stderr, "  Kluge010:                      %d\n", process->mbp_kluge010);
  }

  /*--------------------------------------------
    rationalize topography grid bounds and lonflip
    --------------------------------------------*/
  if ((process->mbp_ampcorr_mode == MBP_AMPCORR_ON &&
       (process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPO || process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPOSLOPE)) ||
      (process->mbp_sscorr_mode == MBP_SSCORR_ON &&
       (process->mbp_sscorr_slope == MBP_SSCORR_USETOPO || process->mbp_sscorr_slope == MBP_SSCORR_USETOPOSLOPE))) {
    if (grid->data != nullptr) {
      if (grid->xmax > 180.0) {
        lonflip = 1;
      }
      else if (grid->xmin < -180.0) {
        lonflip = -1;
      }
      else {
        lonflip = 0;
      }
    }
  }

  /*--------------------------------------------
    get svp
    --------------------------------------------*/

  /* if raytracing or correction/uncorrection to be done get svp */
  int nsvp = 0;
  if (process->mbp_svp_mode != MBP_SVP_OFF) {
    /* count the data points in the svp file */
    nsvp = 0;
    if ((tfp = fopen(process->mbp_svpfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Velocity Profile File <%s> for reading\n", process->mbp_svpfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer)
      if (buffer[0] != '#')
        nsvp++;
    fclose(tfp);

    /* allocate arrays for svp */
    if (nsvp > 1) {
      size = (nsvp + 2) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&depth, error);
      if (*error == MB_ERROR_NO_ERROR)
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&velocity, error);
      if (*error == MB_ERROR_NO_ERROR)
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&velocity_sum, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no svp data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from SVP file <%s>\n", process->mbp_svpfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the svp file */
    nsvp = 0;
    if ((tfp = fopen(process->mbp_svpfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Velocity Profile File <%s> for reading\n", process->mbp_svpfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer) {
      if (buffer[0] != '#') {
        /* read the depth & sound speed pair */
        mm = sscanf(buffer, "%lf %lf", &depth[nsvp], &velocity[nsvp]);

        /* check for validity */
        if (mm == 2) {
          /* output some debug values */
          if (verbose >= 5) {
            fprintf(stderr, "\ndbg5  New velocity value read in program <%s>\n", program_name);
            fprintf(stderr, "dbg5       depth[%d]: %f  velocity[%d]: %f\n", nsvp, depth[nsvp], nsvp,
                    velocity[nsvp]);
          }

          /* set initial depth to zero if needed */
          if (nsvp == 0) {
            if (depth[0] < 0.0) {
              /* output some info */
              fprintf(stderr, "Warning:\n\tProblem with svp value read in program <%s>\n", program_name);
              fprintf(stderr,
                      "\t\tdepth[%d]: %f  velocity[%d]: %f reset so that first entry has zero depth\n",
                      nsvp, depth[0], nsvp, velocity[0]);

              depth[0] = 0.0;
              nsvp++;
            }
            else if (depth[0] > 0.0) {
              depth[1] = depth[0];
              depth[0] = 0.0;
              velocity[1] = velocity[0];
              nsvp += 2;

              /* output some info */
              fprintf(stderr, "Warning:\n\tProblem with svp value read in program <%s>\n", program_name);
              fprintf(stderr,
                      "\t\tdepth[%d]: %f  velocity[%d]: %f added so that first entry has zero depth\n",
                      nsvp, depth[0], nsvp, velocity[0]);
              fprintf(stderr, "\t\tdepth[%d]: %f  velocity[%d]: %f did not have zero depth\n", nsvp,
                      depth[1], nsvp, velocity[1]);
            }
            else {
              nsvp++;
            }
          }

          /* increment counter if all is ok */
          else if (depth[nsvp] > depth[nsvp - 1]) {
            nsvp++;
          }

          /* ignore sound speed value with duplicate or decreasing depth */
          else {
            /* output some info */
            fprintf(stderr, "Warning:\n\tProblem with svp value read in program <%s>\n", program_name);
            fprintf(stderr,
                    "\t\tdepth[%d]: %f  velocity[%d]: %f ignored due to duplicate or decreasing depth\n",
                    nsvp, depth[nsvp], nsvp, velocity[nsvp]);
          }
        }
      }
    }
    fclose(tfp);

    /* set ssv_default */
    ssv_default = velocity[0];

    /* if velocity profile doesn't extend to 12000 m depth
        extend it to that depth */
    if (depth[nsvp - 1] < 12000.0) {
      depth[nsvp] = 12000.0;
      velocity[nsvp] = velocity[nsvp - 1];
      nsvp++;
    }

    /* get velocity sums */
    velocity_sum[0] = 0.5 * (velocity[1] + velocity[0]) * (depth[1] - depth[0]);
    for (int i = 1; i < nsvp - 1; i++) {
      velocity_sum[i] = velocity_sum[i - 1] + 0.5 * (velocity[i + 1] + velocity[i]) * (depth[i + 1] - depth[i]);
    }
  }

  /*--------------------------------------------
    get nav
    --------------------------------------------*/

  /* if nav merging to be done get nav */
  if (process->mbp_nav_mode == MBP_NAV_ON) {
    /* set max number of characters to be read at a time */
    if (process->mbp_nav_format == 8)
      nchar = 96;
    else
      nchar = MBP_FILENAMESIZE - 1;

    /* count the data points in the nav file */
    nnav = 0;
    if ((tfp = fopen(process->mbp_navfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Navigation File <%s> for reading\n", process->mbp_navfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      nnav++;
    fclose(tfp);

    /* allocate arrays for nav */
    if (nnav > 1) {
      // size = (nnav + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&ntime, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlon, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlat, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nheading, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nspeed, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&ndraft, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nroll, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&npitch, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nheave, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlonspl, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlatspl, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no nav data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from navigation file <%s>\n", process->mbp_navfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the nav file */
    nnav = 0;
    if ((tfp = fopen(process->mbp_navfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open navigation File <%s> for reading\n", process->mbp_navfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }

    bool time_set = false;
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      bool nav_ok = false;

      /* deal with nav in form: time_d lon lat */
      if (process->mbp_nav_format == 1) {
        const int nget = sscanf(buffer, "%lf %lf %lf", &ntime[nnav], &nlon[nnav], &nlat[nnav]);
        if (nget == 3)
          nav_ok = true;
      }

      /* deal with nav in form: yr mon day hour min sec lon lat */
      else if (process->mbp_nav_format == 2) {
        const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                      &time_i[4], &sec, &nlon[nnav], &nlat[nnav]);
        time_i[5] = (int)sec;
        time_i[6] = 1000000 * (sec - time_i[5]);
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;
        if (nget == 8)
          nav_ok = true;
      }

      /* deal with nav in form: yr jday hour min sec lon lat */
      else if (process->mbp_nav_format == 3) {
        const int nget = sscanf(buffer, "%d %d %d %d %lf %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
                      &nlon[nnav], &nlat[nnav]);
        time_j[2] = time_j[2] + 60 * ihr;
        time_j[3] = (int)sec;
        time_j[4] = 1000000 * (sec - time_j[3]);
        mb_get_itime(verbose, time_j, time_i);
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;
        if (nget == 7)
          nav_ok = true;
      }

      /* deal with nav in form: yr jday daymin sec lon lat */
      else if (process->mbp_nav_format == 4) {
        const int nget = sscanf(buffer, "%d %d %d %lf %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &nlon[nnav],
                      &nlat[nnav]);
        time_j[3] = (int)sec;
        time_j[4] = 1000000 * (sec - time_j[3]);
        mb_get_itime(verbose, time_j, time_i);
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;
        if (nget == 6)
          nav_ok = true;
      }

      /* deal with nav in L-DEO processed nav format */
      else if (process->mbp_nav_format == 5) {
        strncpy(dummy, "", 128);
        if (buffer[2] == '+') {
          time_j[0] = (int)strtol(strncpy(dummy, buffer, 2), NULL, 10);
          mb_fix_y2k(verbose, time_j[0], &time_j[0]);
          ioff = 3;
        }
        else {
          time_j[0] = (int)strtol(strncpy(dummy, buffer, 4), NULL, 10);
          ioff = 5;
        }
        strncpy(dummy, "", 128);
        time_j[1] = (int)strtol(strncpy(dummy, buffer + ioff, 3), NULL, 10);
        strncpy(dummy, "", 128);
        ioff += 4;
        hr = (int)strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10);
        strncpy(dummy, "", 128);
        ioff += 3;
        time_j[2] = (int)strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10) + 60 * hr;
        strncpy(dummy, "", 128);
        ioff += 3;
        time_j[3] = (int)strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10);
        time_j[4] = 0;
        mb_get_itime(verbose, time_j, time_i);
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;

        strncpy(NorS, "", sizeof(NorS));
        ioff += 7;
        NorS[0] = buffer[ioff];
        ioff += 1;
        strncpy(dummy, "", 128);
        mlat = atof(strncpy(dummy, buffer + ioff, 3));
        strncpy(dummy, "", 128);
        ioff += 3;
        llat = atof(strncpy(dummy, buffer + ioff, 8));
        strncpy(EorW, "", sizeof(EorW));
        ioff += 9;
        EorW[0] = buffer[ioff];
        strncpy(dummy, "", 128);
        ioff += 1;
        mlon = atof(strncpy(dummy, buffer + ioff, 4));
        strncpy(dummy, "", 128);
        ioff += 4;
        llon = atof(strncpy(dummy, buffer + ioff, 8));
        nlon[nnav] = mlon + llon / 60.;
        if (strncmp(EorW, "W", 1) == 0)
          nlon[nnav] = -nlon[nnav];
        nlat[nnav] = mlat + llat / 60.;
        if (strncmp(NorS, "S", 1) == 0)
          nlat[nnav] = -nlat[nnav];
        nav_ok = true;
      }

      /* deal with nav in real and pseudo NMEA 0183 format */
      else if (process->mbp_nav_format == 6 || process->mbp_nav_format == 7) {
        /* check if real sentence */
        int len = strlen(buffer);
        if (strncmp(buffer, "$", 1) == 0) {
          if (strncmp(&buffer[3], "DAT", 3) == 0 && len > 15) {
            time_set = false;
            strncpy(dummy, "", 128);
            time_i[0] = (int)strtol(strncpy(dummy, buffer + 7, 4), NULL, 10);
            time_i[1] = (int)strtol(strncpy(dummy, buffer + 11, 2), NULL, 10);
            time_i[2] = (int)strtol(strncpy(dummy, buffer + 13, 2), NULL, 10);
          }
          else if ((strncmp(&buffer[3], "ZDA", 3) == 0 || strncmp(&buffer[3], "UNX", 3) == 0) && len > 14) {
            time_set = false;
            /* find start of ",hhmmss.ss" */
            if ((bufftmp = strchr(buffer, ',')) != nullptr) {
              strncpy(dummy, "", 128);
              time_i[3] = (int)strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
              strncpy(dummy, "", 128);
              time_i[4] = (int)strtol(strncpy(dummy, bufftmp + 3, 2), NULL, 10);
              strncpy(dummy, "", 128);
              time_i[5] = (int)strtol(strncpy(dummy, bufftmp + 5, 2), NULL, 10);
              if (bufftmp[7] == '.') {
                strncpy(dummy, "", 128);
                time_i[6] = 10000 * (int)strtol(strncpy(dummy, bufftmp + 8, 2), NULL, 10);
              }
              else
                time_i[6] = 0;
              /* find start of ",dd,mm,yyyy" */
              if ((bufftmp = strchr(&bufftmp[1], ',')) != nullptr) {
                strncpy(dummy, "", 128);
                time_i[2] = (int)strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
                strncpy(dummy, "", 128);
                time_i[1] = (int)strtol(strncpy(dummy, bufftmp + 4, 2), NULL, 10);
                strncpy(dummy, "", 128);
                time_i[0] = (int)strtol(strncpy(dummy, bufftmp + 7, 4), NULL, 10);
                time_set = true;
              }
            }
          }
          else if (((process->mbp_nav_format == 6 && strncmp(&buffer[3], "GLL", 3) == 0) ||
                    (process->mbp_nav_format == 7 && strncmp(&buffer[3], "GGA", 3) == 0)) &&
                   time_set && len > 26) {
            time_set = false;
            /* find start of ",ddmm.mm,N,ddmm.mm,E" */
            if ((bufftmp = strchr(buffer, ',')) != nullptr) {
              if (process->mbp_nav_format == 7)
                bufftmp = strchr(&bufftmp[1], ',');
              strncpy(dummy, "", 128);
              degree = (int)strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
              strncpy(dummy, "", 128);
              dminute = atof(strncpy(dummy, bufftmp + 3, 5));
              strncpy(NorS, "", sizeof(NorS));
              bufftmp = strchr(&bufftmp[1], ',');
              strncpy(NorS, bufftmp + 1, 1);
              nlat[nnav] = degree + dminute / 60.;
              if (strncmp(NorS, "S", 1) == 0)
                nlat[nnav] = -nlat[nnav];
              bufftmp = strchr(&bufftmp[1], ',');
              strncpy(dummy, "", 128);
              degree = (int)strtol(strncpy(dummy, bufftmp + 1, 3), NULL, 10);
              strncpy(dummy, "", 128);
              dminute = atof(strncpy(dummy, bufftmp + 4, 5));
              bufftmp = strchr(&bufftmp[1], ',');
              strncpy(EorW, "", sizeof(EorW));
              strncpy(EorW, bufftmp + 1, 1);
              nlon[nnav] = degree + dminute / 60.;
              if (strncmp(EorW, "W", 1) == 0)
                nlon[nnav] = -nlon[nnav];
              mb_get_time(verbose, time_i, &time_d);
              ntime[nnav] = time_d;
              nav_ok = true;
            }
          }
        }
      }

      /* deal with nav in Simrad 90 format */
      else if (process->mbp_nav_format == 8) {
        mb_get_int(&(time_i[2]), buffer + 2, 2);
        mb_get_int(&(time_i[1]), buffer + 4, 2);
        mb_get_int(&(time_i[0]), buffer + 6, 2);
        mb_fix_y2k(verbose, time_i[0], &time_i[0]);
        mb_get_int(&(time_i[3]), buffer + 9, 2);
        mb_get_int(&(time_i[4]), buffer + 11, 2);
        mb_get_int(&(time_i[5]), buffer + 13, 2);
        mb_get_int(&(time_i[6]), buffer + 15, 2);
        time_i[6] = 10000 * time_i[6];
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;

        mb_get_double(&mlat, buffer + 18, 2);
        mb_get_double(&llat, buffer + 20, 7);
        NorS[0] = buffer[27];
        nlat[nnav] = mlat + llat / 60.0;
        if (NorS[0] == 'S' || NorS[0] == 's')
          nlat[nnav] = -nlat[nnav];
        mb_get_double(&mlon, buffer + 29, 3);
        mb_get_double(&llon, buffer + 32, 7);
        EorW[0] = buffer[39];
        nlon[nnav] = mlon + llon / 60.0;
        if (EorW[0] == 'W' || EorW[0] == 'w')
          nlon[nnav] = -nlon[nnav];
        nav_ok = true;
      }

      /* deal with nav in form: yr mon day hour min sec time_d lon lat heading speed draft*/
      else if (process->mbp_nav_format == 9) {
        const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1],
                      &time_i[2], &time_i[3], &time_i[4], &sec, &ntime[nnav], &nlon[nnav], &nlat[nnav],
                      &nheading[nnav], &nspeed[nnav], &ndraft[nnav], &nroll[nnav], &npitch[nnav], &nheave[nnav]);
        if (nget >= 9)
          nav_ok = true;
        if (nnav > 0 && ntime[nnav] <= ntime[nnav - 1])
          nav_ok = false;
        if (nav_ok) {
          if (process->mbp_nav_heading == MBP_NAV_ON && nget < 10) {
            fprintf(stderr, "\nHeading data missing from nav file.\nMerging of heading data disabled.\n");
            process->mbp_nav_heading = MBP_NAV_OFF;
          }
          if (process->mbp_nav_speed == MBP_NAV_ON && nget < 11) {
            fprintf(stderr, "Speed data missing from nav file.\nMerging of speed data disabled.\n");
            process->mbp_nav_speed = MBP_NAV_OFF;
          }
          if (process->mbp_nav_draft == MBP_NAV_ON && nget < 12) {
            fprintf(stderr, "Draft data missing from nav file.\nMerging of draft data disabled.\n");
            process->mbp_nav_draft = MBP_NAV_OFF;
          }
          if (process->mbp_nav_attitude == MBP_NAV_ON && nget < 15) {
            fprintf(stderr, "Roll, pitch, and heave data missing from nav file.\nMerging of roll, pitch, and "
                            "heave data disabled.\n");
            process->mbp_nav_attitude = MBP_NAV_OFF;
          }
          if (process->mbp_nav_heading == MBP_NAV_OFF) {
            nheading[nnav] = 0.0;
          }
          if (process->mbp_nav_speed == MBP_NAV_OFF) {
            nspeed[nnav] = 0.0;
          }
          if (process->mbp_nav_draft == MBP_NAV_OFF) {
            ndraft[nnav] = 0.0;
          }
          if (process->mbp_nav_attitude == MBP_NAV_OFF) {
            nroll[nnav] = 0.0;
            npitch[nnav] = 0.0;
            nheave[nnav] = 0.0;
          }
        }
      }

      /* deal with nav in r2rnav form:
          yyyy-mm-ddThh:mm:ss.sssZ decimalLongitude decimalLatitude quality nsat dilution height */
      else if (process->mbp_nav_format == 10) {
        const int nget = sscanf(buffer, "%d-%d-%dT%d:%d:%lfZ %lf %lf %d %d %d %d", &time_i[0], &time_i[1], &time_i[2],
                      &time_i[3], &time_i[4], &sec, &nlon[nnav], &nlat[nnav], &quality, &nsatellite, &dilution,
                      &gpsheight);
        if (nget != 12) {
          quality = 0;
          nsatellite = 0;
          dilution = 0;
          gpsheight = 0;
        }
        time_i[5] = (int)floor(sec);
        time_i[6] = (int)((sec - time_i[5]) * 1000000);
        mb_get_time(verbose, time_i, &time_d);
        ntime[nnav] = time_d;
        nheading[nnav] = 0.0;
        nspeed[nnav] = 0.0;
        ndraft[nnav] = 0.0;
        nroll[nnav] = 0.0;
        npitch[nnav] = 0.0;
        nheave[nnav] = 0.0;
        if (nget >= 8)
          nav_ok = true;
      }

      /* make sure longitude is defined according to lonflip */
      if (nav_ok) {
        if (lonflip == -1 && nlon[nnav] > 0.0)
          nlon[nnav] = nlon[nnav] - 360.0;
        else if (lonflip == 0 && nlon[nnav] < -180.0)
          nlon[nnav] = nlon[nnav] + 360.0;
        else if (lonflip == 0 && nlon[nnav] > 180.0)
          nlon[nnav] = nlon[nnav] - 360.0;
        else if (lonflip == 1 && nlon[nnav] < 0.0)
          nlon[nnav] = nlon[nnav] + 360.0;
      }

      /* output some debug values */
      if (verbose >= 5 && nav_ok) {
        fprintf(stderr, "\ndbg5  New navigation point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav, ntime[nnav], nlon[nnav], nlat[nnav]);
      }
      else if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Error parsing line in navigation file in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       line: %s\n", buffer);
      }

      /* check for reverses or repeats in time */
      if (nav_ok) {
        if (nnav == 0)
          nnav++;
        else if (ntime[nnav] > ntime[nnav - 1])
          nnav++;
        else if (nnav > 0 && ntime[nnav] <= ntime[nnav - 1] && verbose >= 5) {
          fprintf(stderr, "\ndbg5  Navigation time error in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav - 1, ntime[nnav - 1], nlon[nnav - 1],
                  nlat[nnav - 1]);
          fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav, ntime[nnav], nlon[nnav], nlat[nnav]);
        }
      }
      strncpy(buffer, "", sizeof(buffer));
    }
    fclose(tfp);

    /* check for nav */
    if (nnav < 2) {
      fprintf(stderr, "\nNo navigation read from file <%s>\n", process->mbp_navfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* apply time shift if needed */
    if (process->mbp_nav_timeshift != 0.0)
      for (int i = 0; i < nnav; i++)
        ntime[i] += process->mbp_nav_timeshift;

    /* set up spline interpolation of nav points */
    splineflag = 1.0e30;
    mb_spline_init(verbose, ntime - 1, nlon - 1, nnav, splineflag, splineflag, nlonspl - 1, error);
    mb_spline_init(verbose, ntime - 1, nlat - 1, nnav, splineflag, splineflag, nlatspl - 1, error);

    /* get start and finish times of nav */
    mb_get_date(verbose, ntime[0], stime_i);
    mb_get_date(verbose, ntime[nnav - 1], ftime_i);

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d navigation records read\n", nnav);
      fprintf(stderr, "Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", stime_i[0], stime_i[1],
              stime_i[2], stime_i[3], stime_i[4], stime_i[5], stime_i[6]);
      fprintf(stderr, "Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ftime_i[0], ftime_i[1],
              ftime_i[2], ftime_i[3], ftime_i[4], ftime_i[5], ftime_i[6]);
    }
  }

  /*--------------------------------------------
    get adjusted nav
    --------------------------------------------*/

  /* if adjusted nav merging to be done get adjusted nav */
  if (process->mbp_navadj_mode >= MBP_NAVADJ_LL) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the adjusted nav file */
    nanav = 0;
    if ((tfp = fopen(process->mbp_navadjfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Adjusted Navigation File <%s> for reading\n", process->mbp_navadjfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      if (buffer[0] != '#')
        nanav++;
    fclose(tfp);

    /* allocate arrays for adjusted nav */
    if (nanav > 1) {
      // size = (nanav + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&natime, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&nalon, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&nalat, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&naz, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&nalonspl, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&nalatspl, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nanav * sizeof(double), (void **)&nazspl, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no adjusted nav data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from adjusted navigation file <%s>\n", process->mbp_navadjfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the nav file */
    nanav = 0;
    if ((tfp = fopen(process->mbp_navadjfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open navigation File <%s> for reading\n", process->mbp_navadjfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      bool nav_ok = false;

      /* deal with nav in form: yr mon day hour min sec time_d lon lat */
      if (buffer[0] != '#') {
        const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0],
                      &time_i[1], &time_i[2], &time_i[3], &time_i[4], &sec, &natime[nanav], &nalon[nanav],
                      &nalat[nanav], &heading, &speed, &draft, &roll, &pitch, &heave, &naz[nanav]);
        if (process->mbp_navadj_mode == MBP_NAVADJ_LL && nget >= 9)
          nav_ok = true;
        else if (process->mbp_navadj_mode == MBP_NAVADJ_LLZ && nget >= 16)
          nav_ok = true;
      }

      /* make sure longitude is defined according to lonflip */
      if (nav_ok) {
        if (lonflip == -1 && nalon[nanav] > 0.0)
          nalon[nanav] = nalon[nanav] - 360.0;
        else if (lonflip == 0 && nalon[nanav] < -180.0)
          nalon[nanav] = nalon[nanav] + 360.0;
        else if (lonflip == 0 && nalon[nanav] > 180.0)
          nalon[nanav] = nalon[nanav] - 360.0;
        else if (lonflip == 1 && nalon[nanav] < 0.0)
          nalon[nanav] = nalon[nanav] + 360.0;
      }

      /* output some debug values */
      if (verbose >= 5 && nav_ok) {
        fprintf(stderr, "\ndbg5  New adjusted navigation point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nanav, natime[nanav], nalon[nanav], nalat[nanav]);
      }
      else if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Error parsing line in navigation file in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       line: %s\n", buffer);
      }

      /* check for reverses or repeats in time */
      if (nav_ok) {
        if (nanav == 0)
          nanav++;
        else if (natime[nanav] > natime[nanav - 1])
          nanav++;
        else if (nanav > 0 && natime[nanav] <= natime[nanav - 1] && verbose >= 5) {
          fprintf(stderr, "\ndbg5  Navigation time error in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       adjusted nav[%d]: %f %f %f\n", nanav - 1, natime[nanav - 1],
                  nalon[nanav - 1], nalat[nanav - 1]);
          fprintf(stderr, "dbg5       adjusted nav[%d]: %f %f %f\n", nanav, natime[nanav], nalon[nanav],
                  nalat[nanav]);
        }
      }
      strncpy(buffer, "", sizeof(buffer));
    }
    fclose(tfp);

    /* check for adjusted nav */
    if (nanav < 2) {
      fprintf(stderr, "\nNo adjusted navigation read from file <%s>\n", process->mbp_navadjfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* set up spline interpolation of adjusted nav points */
    splineflag = 1.0e30;
    mb_spline_init(verbose, natime - 1, nalon - 1, nanav, splineflag, splineflag, nalonspl - 1, error);
    mb_spline_init(verbose, natime - 1, nalat - 1, nanav, splineflag, splineflag, nalatspl - 1, error);
    mb_spline_init(verbose, natime - 1, naz - 1, nanav, splineflag, splineflag, nazspl - 1, error);

    /* get start and finish times of nav */
    mb_get_date(verbose, natime[0], stime_i);
    mb_get_date(verbose, natime[nanav - 1], ftime_i);

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d adjusted navigation records read\n", nanav);
      fprintf(stderr, "Adjusted nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", stime_i[0],
              stime_i[1], stime_i[2], stime_i[3], stime_i[4], stime_i[5], stime_i[6]);
      fprintf(stderr, "Adjusted nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ftime_i[0],
              ftime_i[1], ftime_i[2], ftime_i[3], ftime_i[4], ftime_i[5], ftime_i[6]);
    }
  }

  /*--------------------------------------------
    get attitude
    --------------------------------------------*/

  /* if attitude merging to be done get attitude */
  if (process->mbp_attitude_mode == MBP_ATTITUDE_ON) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the attitude file */
    nattitude = 0;
    if ((tfp = fopen(process->mbp_attitudefile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Attitude File <%s> for reading\n", process->mbp_attitudefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      nattitude++;
    fclose(tfp);

    /* allocate arrays for attitude */
    if (nattitude > 1) {
      // size = (nattitude + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nattitude * sizeof(double), (void **)&attitudetime, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nattitude * sizeof(double), (void **)&attituderoll, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nattitude * sizeof(double), (void **)&attitudepitch, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nattitude * sizeof(double), (void **)&attitudeheave, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no attitude data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from attitude file <%s>\n", process->mbp_attitudefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the attitude file */
    nattitude = 0;
    if ((tfp = fopen(process->mbp_attitudefile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Attitude File <%s> for reading\n", process->mbp_attitudefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      bool attitude_ok = false;

      /* ignore comments */
      if (buffer[0] != '#') {

        /* deal with attitude in form: time_d roll pitch heave */
        if (process->mbp_attitude_format == 1) {
          const int nget = sscanf(buffer, "%lf %lf %lf %lf", &attitudetime[nattitude], &attituderoll[nattitude],
                        &attitudepitch[nattitude], &attitudeheave[nattitude]);
          if (nget == 4)
            attitude_ok = true;
        }

        /* deal with attitude in form: yr mon day hour min sec roll pitch heave */
        else if (process->mbp_attitude_format == 2) {
          const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2],
                        &time_i[3], &time_i[4], &sec, &attituderoll[nattitude], &attitudepitch[nattitude],
                        &attitudeheave[nattitude]);
          time_i[5] = (int)sec;
          time_i[6] = 1000000 * (sec - time_i[5]);
          mb_get_time(verbose, time_i, &time_d);
          attitudetime[nattitude] = time_d;
          if (nget == 9)
            attitude_ok = true;
        }

        /* deal with attitude in form: yr jday hour min sec roll pitch heave */
        else if (process->mbp_attitude_format == 3) {
          const int nget = sscanf(buffer, "%d %d %d %d %lf %lf %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
                        &attituderoll[nattitude], &attitudepitch[nattitude], &attitudeheave[nattitude]);
          time_j[2] = time_j[2] + 60 * ihr;
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          attitudetime[nattitude] = time_d;
          if (nget == 9)
            attitude_ok = true;
        }

        /* deal with attitude in form: yr jday daymin sec roll pitch heave */
        else if (process->mbp_attitude_format == 4) {
          const int nget = sscanf(buffer, "%d %d %d %lf %lf %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec,
                        &attituderoll[nattitude], &attitudepitch[nattitude], &attitudeheave[nattitude]);
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          attitudetime[nattitude] = time_d;
          if (nget == 7)
            attitude_ok = true;
        }
      }

      /* output some debug values */
      if (verbose >= 5 && attitude_ok) {
        fprintf(stderr, "\ndbg5  New attitude point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nattitude, attitudetime[nattitude],
                attituderoll[nattitude], attitudepitch[nattitude], attitudeheave[nattitude]);
      }
      else if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Error parsing line in attitude file in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       line: %s\n", buffer);
      }

      /* check for reverses or repeats in time */
      if (attitude_ok) {
        if (nattitude == 0)
          nattitude++;
        else if (attitudetime[nattitude] > attitudetime[nattitude - 1])
          nattitude++;
        else if (nattitude > 0 && attitudetime[nattitude] <= attitudetime[nattitude - 1] && verbose >= 5) {
          fprintf(stderr, "\ndbg5  Attitude time error in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nattitude - 1, attitudetime[nattitude - 1],
                  attituderoll[nattitude - 1], attitudepitch[nattitude - 1], attitudeheave[nattitude - 1]);
          fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nattitude, attitudetime[nattitude],
                  attituderoll[nattitude - 1], attitudepitch[nattitude - 1], attitudeheave[nattitude - 1]);
        }
      }
      strncpy(buffer, "", sizeof(buffer));
    }
    fclose(tfp);

    /* check for attitude */
    if (nattitude < 2) {
      fprintf(stderr, "\nNo attitude read from file <%s>\n", process->mbp_attitudefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* get start and finish times of attitude */
    mb_get_date(verbose, attitudetime[0], stime_i);
    mb_get_date(verbose, attitudetime[nattitude - 1], ftime_i);

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d attitude records read\n", nattitude);
      fprintf(stderr, "Attitude start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", stime_i[0], stime_i[1],
              stime_i[2], stime_i[3], stime_i[4], stime_i[5], stime_i[6]);
      fprintf(stderr, "Attitude end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ftime_i[0], ftime_i[1],
              ftime_i[2], ftime_i[3], ftime_i[4], ftime_i[5], ftime_i[6]);
    }
  }

  /*--------------------------------------------
    get sensordepth
    --------------------------------------------*/

  /* if sensordepth merging to be done get sensordepth */
  if (process->mbp_sensordepth_mode == MBP_SENSORDEPTH_ON) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the sensordepth file */
    nsensordepth = 0;
    if ((tfp = fopen(process->mbp_sensordepthfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Sensordepth File <%s> for reading\n", process->mbp_sensordepthfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      nsensordepth++;
    fclose(tfp);

    /* allocate arrays for sensordepth */
    if (nsensordepth > 1) {
      // size = (nsensordepth + 1) * sizeof(double);
      /* status = */
      mb_mallocd(verbose, __FILE__, __LINE__, nsensordepth * sizeof(double), (void **)&fsensordepthtime, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsensordepth * sizeof(double), (void **)&fsensordepth, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no sensordepth data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from sensordepth file <%s>\n", process->mbp_sensordepthfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the sensordepth file */
    nsensordepth = 0;
    if ((tfp = fopen(process->mbp_sensordepthfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open sensordepth File <%s> for reading\n", process->mbp_sensordepthfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
       bool sensordepth_ok = false;

      /* ignore comments */
      if (buffer[0] != '#') {

        /* deal with sensordepth in form: time_d sensordepth */
        if (process->mbp_sensordepth_format == 1) {
          const int nget = sscanf(buffer, "%lf %lf", &fsensordepthtime[nsensordepth], &fsensordepth[nsensordepth]);
          if (nget == 2)
            sensordepth_ok = true;
        }

        /* deal with sensordepth in form: yr mon day hour min sec sensordepth */
        else if (process->mbp_sensordepth_format == 2) {
          const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                        &time_i[4], &sec, &fsensordepth[nsensordepth]);
          time_i[5] = (int)sec;
          time_i[6] = 1000000 * (sec - time_i[5]);
          mb_get_time(verbose, time_i, &time_d);
          fsensordepthtime[nsensordepth] = time_d;
          if (nget == 7)
            sensordepth_ok = true;
        }

        /* deal with sensordepth in form: yr jday hour min sec sensordepth */
        else if (process->mbp_sensordepth_format == 3) {
          const int nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
                        &fsensordepth[nsensordepth]);
          time_j[2] = time_j[2] + 60 * ihr;
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          fsensordepthtime[nsensordepth] = time_d;
          if (nget == 7)
            sensordepth_ok = true;
        }

        /* deal with sensordepth in form: yr jday daymin sec sensordepth */
        else if (process->mbp_sensordepth_format == 4) {
          const int nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec,
                        &fsensordepth[nsensordepth]);
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          fsensordepthtime[nsensordepth] = time_d;
          if (nget == 5)
            sensordepth_ok = true;
        }
      }

      /* output some debug values */
      if (verbose >= 5 && sensordepth_ok) {
        fprintf(stderr, "\ndbg5  New sensordepth point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nsensordepth, fsensordepthtime[nsensordepth],
                fsensordepth[nsensordepth]);
      }
      else if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Error parsing line in sensordepth file in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       line: %s\n", buffer);
      }

      /* check for reverses or repeats in time */
      if (sensordepth_ok) {
        if (nsensordepth == 0)
          nsensordepth++;
        else if (fsensordepthtime[nsensordepth] > fsensordepthtime[nsensordepth - 1])
          nsensordepth++;
        else if (nsensordepth > 0 && fsensordepthtime[nsensordepth] <= fsensordepthtime[nsensordepth - 1] &&
                 verbose >= 5) {
          fprintf(stderr, "\ndbg5  sensordepth time error in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nsensordepth - 1,
                  fsensordepthtime[nsensordepth - 1], fsensordepth[nsensordepth - 1]);
          fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nsensordepth, fsensordepthtime[nsensordepth],
                  fsensordepth[nsensordepth - 1]);
        }
      }
      strncpy(buffer, "", sizeof(buffer));
    }
    fclose(tfp);

    /* check for sensordepth */
    if (nsensordepth < 2) {
      fprintf(stderr, "\nNo sensordepth read from file <%s>\n", process->mbp_sensordepthfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* get start and finish times of sensordepth */
    mb_get_date(verbose, fsensordepthtime[0], stime_i);
    mb_get_date(verbose, fsensordepthtime[nsensordepth - 1], ftime_i);

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d sensordepth records read\n", nsensordepth);
      fprintf(stderr, "sensordepth start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", stime_i[0], stime_i[1],
              stime_i[2], stime_i[3], stime_i[4], stime_i[5], stime_i[6]);
      fprintf(stderr, "sensordepth end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ftime_i[0], ftime_i[1],
              ftime_i[2], ftime_i[3], ftime_i[4], ftime_i[5], ftime_i[6]);
    }
  }

  /*--------------------------------------------
    get tide
    --------------------------------------------*/

  /* if tide correction to be done get tide */
  if (process->mbp_tide_mode == MBP_TIDE_ON) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the tide file */
    ntide = 0;
    if ((tfp = fopen(process->mbp_tidefile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Tide File <%s> for reading\n", process->mbp_tidefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      ntide++;
    fclose(tfp);

    /* allocate arrays for tide */
    if (ntide > 1) {
      // size = (ntide + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, ntide * sizeof(double), (void **)&tidetime, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, ntide * sizeof(double), (void **)&tide, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no tide data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from tide file <%s>\n", process->mbp_tidefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the tide file */
    ntide = 0;
    if ((tfp = fopen(process->mbp_tidefile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Tide File <%s> for reading\n", process->mbp_tidefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      bool tide_ok = false;

      /* ignore comments */
      if (buffer[0] != '#') {

        /* deal with tide in form: time_d tide */
        if (process->mbp_tide_format == 1) {
          const int nget = sscanf(buffer, "%lf %lf", &tidetime[ntide], &tide[ntide]);
          if (nget == 2)
            tide_ok = true;
        }

        /* deal with tide in form: yr mon day hour min sec tide */
        else if (process->mbp_tide_format == 2) {
          const int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                        &time_i[4], &sec, &tide[ntide]);
          time_i[5] = (int)sec;
          time_i[6] = 1000000 * (sec - time_i[5]);
          mb_get_time(verbose, time_i, &time_d);
          tidetime[ntide] = time_d;
          if (nget == 7)
            tide_ok = true;
        }

        /* deal with tide in form: yr jday hour min sec tide */
        else if (process->mbp_tide_format == 3) {
          const int nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
                        &tide[ntide]);
          time_j[2] = time_j[2] + 60 * ihr;
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          tidetime[ntide] = time_d;
          if (nget == 6)
            tide_ok = true;
        }

        /* deal with tide in form: yr jday daymin sec tide */
        else if (process->mbp_tide_format == 4) {
          const int nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &tide[ntide]);
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          tidetime[ntide] = time_d;
          if (nget == 5)
            tide_ok = true;
        }
      }

      /* output some debug values */
      if (verbose >= 5 && tide_ok) {
        fprintf(stderr, "\ndbg5  New tide point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       tide[%d]: %f %f\n", ntide, tidetime[ntide], tide[ntide]);
      }
      else if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Error parsing line in tide file in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       line: %s\n", buffer);
      }

      /* check for reverses or repeats in time */
      if (tide_ok) {
        if (ntide == 0)
          ntide++;
        else if (tidetime[ntide] > tidetime[ntide - 1])
          ntide++;
        else if (ntide > 0 && tidetime[ntide] <= tidetime[ntide - 1] && verbose >= 5) {
          fprintf(stderr, "\ndbg5  Tide time error in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       tide[%d]: %f %f\n", ntide - 1, tidetime[ntide - 1], tide[ntide - 1]);
          fprintf(stderr, "dbg5       tide[%d]: %f %f\n", ntide, tidetime[ntide], tide[ntide]);
        }
      }
      strncpy(buffer, "", sizeof(buffer));
    }
    fclose(tfp);

    /* check for tide */
    if (ntide < 1) {
      fprintf(stderr, "\nNo tide read from file <%s>\n", process->mbp_tidefile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* get start and finish times of tide */
    mb_get_date(verbose, tidetime[0], stime_i);
    mb_get_date(verbose, tidetime[ntide - 1], ftime_i);

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d tide records read\n", ntide);
      fprintf(stderr, "Tide start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", stime_i[0], stime_i[1],
              stime_i[2], stime_i[3], stime_i[4], stime_i[5], stime_i[6]);
      fprintf(stderr, "Tide end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ftime_i[0], ftime_i[1],
              ftime_i[2], ftime_i[3], ftime_i[4], ftime_i[5], ftime_i[6]);
    }
  }

  /*--------------------------------------------
    get edits
    --------------------------------------------*/

  /* get edits */
  if (process->mbp_edit_mode == MBP_EDIT_ON) {
    *status = mb_esf_open(verbose, program_name, process->mbp_editfile, true, false, &esf, error);
    if (*status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to read from Edit Save File <%s>\n", process->mbp_editfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d bathymetry edits read\n", esf.nedit);
    }
  }

  /*--------------------------------------------
    get beam static corrections
    --------------------------------------------*/

  /* if static correction to be done get statics */
  /* Static file is beam number vs correction */
  if (process->mbp_static_mode == MBP_STATIC_BEAM_ON) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the static file */
    nstatic = 0;
    if ((tfp = fopen(process->mbp_staticfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Static File <%s> for reading\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      nstatic++;
    fclose(tfp);

    /* allocate arrays for static */
    if (nstatic > 0) {
      // size = (nstatic + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nstatic * sizeof(int), (void **)&staticbeam, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nstatic * sizeof(double), (void **)&staticoffset, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no static data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from static file <%s>\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the static file */
    nstatic = 0;
    if ((tfp = fopen(process->mbp_staticfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Static File <%s> for reading\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      /* deal with static in form: beam_# offset */
      if (buffer[0] != '#') {
        bool static_ok = false;
        const int nget = sscanf(buffer, "%d %lf", &staticbeam[nstatic], &staticoffset[nstatic]);
        if (nget == 2) {
          static_ok = true;
          nstatic++;
        }

        /* output some debug values */
        if (verbose >= 5 && static_ok) {
          fprintf(stderr, "\ndbg5  New static beam correction read in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       beam:%d offset:%f\n", staticbeam[nstatic], staticoffset[nstatic]);
        }
        else if (verbose >= 5) {
          fprintf(stderr, "\ndbg5  Error parsing line in static beam correction file in program <%s>\n",
                  program_name);
          fprintf(stderr, "dbg5       line: %s\n", buffer);
        }
      }
    }
    fclose(tfp);

    /* check for good static data */
    if (nstatic < 1) {
      fprintf(stderr, "\nNo static beam corrections read from file <%s>\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d static beam corrections read\n", nstatic);
    }
  }

  /* if static correction to be done get statics */
  /* Static file is grazing angle vs correction */
  if (process->mbp_static_mode == MBP_STATIC_ANGLE_ON) {
    /* set max number of characters to be read at a time */
    nchar = 128;

    /* count the data points in the static file */
    nstatic = 0;
    if ((tfp = fopen(process->mbp_staticfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Static File <%s> for reading\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, nchar, tfp)) == buffer)
      nstatic++;
    fclose(tfp);

    /* allocate arrays for static */
    if (nstatic > 0) {
      // size = (nstatic + 1) * sizeof(double);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nstatic * sizeof(double), (void **)&staticoffset, error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nstatic * sizeof(double), (void **)&staticangle, error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no static data then quit */
    else {
      fprintf(stderr, "\nUnable to read data from static file <%s>\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the static file */
    nstatic = 0;
    if ((tfp = fopen(process->mbp_staticfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Static File <%s> for reading\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, nchar, tfp)) == buffer) {
      /* deal with static in form: angle offset */
      if (buffer[0] != '#') {
        const int nget = sscanf(buffer, "%lf %lf", &staticangle[nstatic], &staticoffset[nstatic]);
        bool static_ok = false;
        if (nget == 2) {
          static_ok = true;
          nstatic++;
        }

        /* output some debug values */
        if (verbose >= 5 && static_ok) {
          fprintf(stderr, "\ndbg5  New static angle correction read in program <%s>\n", program_name);
          fprintf(stderr, "dbg5       angle:%f offset:%f\n", staticangle[nstatic], staticoffset[nstatic]);
        }
        else if (verbose >= 5) {
          fprintf(stderr, "\ndbg5  Error parsing line in static angle correction file in program <%s>\n",
                  program_name);
          fprintf(stderr, "dbg5       line: %s\n", buffer);
        }
      }
    }
    fclose(tfp);

    /* check for good static data */
    if (nstatic < 1) {
      fprintf(stderr, "\nNo static angle corrections read from file <%s>\n", process->mbp_staticfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d static angle corrections read\n", nstatic);
    }
  }

  /*--------------------------------------------
    get amplitude corrections
    --------------------------------------------*/
  nampcorrtable = 0;
  nampcorrangle = 0;
  if (process->mbp_ampcorr_mode == MBP_AMPCORR_ON) {
    /* count the data points in the amplitude correction file */
    if ((tfp = fopen(process->mbp_ampcorrfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Amplitude Correction File <%s> for reading\n", process->mbp_ampcorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer) {
      if (strncmp(buffer, "# table:", 8) == 0)
        nampcorrtable++;
      else if (strncmp(buffer, "# nangles:", 10) == 0)
        sscanf(buffer, "# nangles:%d", &nampcorrangle);
    }
    fclose(tfp);

    /* allocate arrays for amplitude correction tables */
    if (nampcorrtable > 0) {
      size = nampcorrtable * sizeof(struct mbprocess_sscorr_struct);
      ampcorrtable = nullptr;
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&ampcorrtable, error);
      for (int i = 0; i < nampcorrtable; i++) {
        ampcorrtable[i].angle = nullptr;
        ampcorrtable[i].amplitude = nullptr;
        ampcorrtable[i].sigma = nullptr;
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                            (void **)&(ampcorrtable[i].angle), error);
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                            (void **)&(ampcorrtable[i].amplitude), error);
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                            (void **)&(ampcorrtable[i].sigma), error);
      }
      ampcorrtableuse.angle = nullptr;
      ampcorrtableuse.amplitude = nullptr;
      ampcorrtableuse.sigma = nullptr;
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                          (void **)&(ampcorrtableuse.angle), error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                          (void **)&(ampcorrtableuse.amplitude), error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nampcorrangle * sizeof(double),
                          (void **)&(ampcorrtableuse.sigma), error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no amplitude correction file then quit */
    else {
      fprintf(stderr, "\nUnable to read data from amplitude correction file <%s>\n", process->mbp_ampcorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the amplitude correction file */
    nampcorrtable = 0;
    if ((tfp = fopen(process->mbp_ampcorrfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Amplitude Correction File <%s> for reading\n", process->mbp_ampcorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer) {
      /* deal with amplitude correction tables */
      if (strncmp(buffer, "# table:", 8) == 0) {
        /* nget = */ sscanf(buffer, "# table:%d", &itable);
        nampcorrtable++;
        ampcorrtable[itable].nangle = 0;
      }
      else if (strncmp(buffer, "# time:", 7) == 0)
        /* nget = */ sscanf(buffer, "# time: %d/%d/%d %d:%d:%d.%d %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                      &time_i[4], &time_i[5], &time_i[6], &(ampcorrtable[itable].time_d));
      else if (buffer[0] != '#') {
        const int nget = sscanf(buffer, "%lf %lf %lf", &(ampcorrtable[itable].angle[ampcorrtable[itable].nangle]),
                      &(ampcorrtable[itable].amplitude[ampcorrtable[itable].nangle]),
                      &(ampcorrtable[itable].sigma[ampcorrtable[itable].nangle]));
        (ampcorrtable[itable].nangle)++;
        if (nget != 3) {
          fprintf(stderr, "\ndbg5  Error parsing line in sidescan correction file in program <%s>\n",
                  program_name);
          fprintf(stderr, "dbg5       line: %s\n", buffer);
        }
      }
    }
    fclose(tfp);

    /* force amplitude correction tables to be symmetric if desired */
    if (process->mbp_ampcorr_symmetry == MBP_AMPCORR_SYMMETRIC) {
      for (itable = 0; itable < nampcorrtable; itable++) {
        for (int i = 0; i < ampcorrtable[itable].nangle / 2; i++) {
          const int j = ampcorrtable[itable].nangle - 1 - i;
          if (ampcorrtable[itable].amplitude[i] != 0.0 && ampcorrtable[itable].amplitude[j] != 0.0)
            factor = 0.5;
          else
            factor = 1.0;
          ampcorrtable[itable].amplitude[i] =
              factor * (ampcorrtable[itable].amplitude[i] + ampcorrtable[itable].amplitude[j]);
          ampcorrtable[itable].sigma[i] = std::max(ampcorrtable[itable].sigma[i], ampcorrtable[itable].sigma[j]);
          ampcorrtable[itable].amplitude[j] = ampcorrtable[itable].amplitude[i];
          ampcorrtable[itable].sigma[j] = ampcorrtable[itable].sigma[i];
        }
      }
    }

    /* check for good amplitude correction data */
    if (nampcorrtable < 1) {
      fprintf(stderr, "\nNo amplitude correction tables read from file <%s>\n", process->mbp_ampcorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d amplitude correction tables with %d angles read\n", nampcorrtable, nampcorrangle);
    }
  }

  /*--------------------------------------------
    get sidescan corrections
    --------------------------------------------*/
  nsscorrtable = 0;
  nsscorrangle = 0;
  if (process->mbp_sscorr_mode == MBP_SSCORR_ON) {
    /* count the data points in the sidescan correction file */
    if ((tfp = fopen(process->mbp_sscorrfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Sidescan Correction File <%s> for reading\n", process->mbp_sscorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer) {
      if (strncmp(buffer, "# table:", 8) == 0)
        nsscorrtable++;
      else if (strncmp(buffer, "# nangles:", 10) == 0)
        sscanf(buffer, "# nangles:%d", &nsscorrangle);
    }
    fclose(tfp);

    /* allocate arrays for sidescan correction tables */
    if (nsscorrtable > 0) {
      size = nsscorrtable * sizeof(struct mbprocess_sscorr_struct);
      sscorrtable = nullptr;
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&sscorrtable, error);
      for (int i = 0; i < nsscorrtable; i++) {
        sscorrtable[i].angle = nullptr;
        sscorrtable[i].amplitude = nullptr;
        sscorrtable[i].sigma = nullptr;
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                            (void **)&(sscorrtable[i].angle), error);
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                            (void **)&(sscorrtable[i].amplitude), error);
        /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                            (void **)&(sscorrtable[i].sigma), error);
      }
      sscorrtableuse.angle = nullptr;
      sscorrtableuse.amplitude = nullptr;
      sscorrtableuse.sigma = nullptr;
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                          (void **)&(sscorrtableuse.angle), error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                          (void **)&(sscorrtableuse.amplitude), error);
      /* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nsscorrangle * sizeof(double),
                          (void **)&(sscorrtableuse.sigma), error);

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* if no sidescan correction file then quit */
    else {
      fprintf(stderr, "\nUnable to read data from sidescan correction file <%s>\n", process->mbp_sscorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_DATA);
    }

    /* read the data points in the sidescan correction file */
    nsscorrtable = 0;
    if ((tfp = fopen(process->mbp_sscorrfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Sidescan Correction File <%s> for reading\n", process->mbp_sscorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(buffer, MBP_FILENAMESIZE, tfp)) == buffer) {
      /* deal with sidescan correction tables */
      if (strncmp(buffer, "# table:", 8) == 0) {
        /* nget = */ sscanf(buffer, "# table:%d", &itable);
        nsscorrtable++;
        sscorrtable[itable].nangle = 0;
      }
      else if (strncmp(buffer, "# time:", 7) == 0)
        /* nget = */ sscanf(buffer, "# time: %d/%d/%d %d:%d:%d.%d %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                      &time_i[4], &time_i[5], &time_i[6], &(sscorrtable[itable].time_d));
      else if (buffer[0] != '#') {
        const int nget = sscanf(buffer, "%lf %lf %lf", &(sscorrtable[itable].angle[sscorrtable[itable].nangle]),
                      &(sscorrtable[itable].amplitude[sscorrtable[itable].nangle]),
                      &(sscorrtable[itable].sigma[sscorrtable[itable].nangle]));
        (sscorrtable[itable].nangle)++;
        if (nget != 3) {
          fprintf(stderr, "\ndbg5  Error parsing line in sidescan correction file in program <%s>\n",
                  program_name);
          fprintf(stderr, "dbg5       line: %s\n", buffer);
        }
      }
    }
    fclose(tfp);

    /* force sidescan correction tables to be symmetric if desired */
    if (process->mbp_sscorr_symmetry == MBP_SSCORR_SYMMETRIC) {
      for (itable = 0; itable < nsscorrtable; itable++) {
        for (int i = 0; i < sscorrtable[itable].nangle / 2; i++) {
          const int j = sscorrtable[itable].nangle - 1 - i;
          if (sscorrtable[itable].amplitude[i] != 0.0 && sscorrtable[itable].amplitude[j] != 0.0)
            factor = 0.5;
          else
            factor = 1.0;
          sscorrtable[itable].amplitude[i] =
              factor * (sscorrtable[itable].amplitude[i] + sscorrtable[itable].amplitude[j]);
          sscorrtable[itable].amplitude[j] = sscorrtable[itable].amplitude[i];
          sscorrtable[itable].sigma[i] = std::max(sscorrtable[itable].sigma[i], sscorrtable[itable].sigma[j]);
          sscorrtable[itable].sigma[j] = sscorrtable[itable].sigma[i];
        }
      }
    }

    /* check for good sidescan correction data */
    if (nsscorrtable < 1) {
      fprintf(stderr, "\nNo sidescan correction tables read from file <%s>\n", process->mbp_sscorrfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* give the statistics */
    if (verbose >= 1) {
      fprintf(stderr, "\n%d sidescan correction tables with %d angles read\n", nsscorrtable, nsscorrangle);
    }
  }

  /*--------------------------------------------
    now open the swath files
    --------------------------------------------*/

  /* reset all defaults */
  pings = 1;
  bounds[0] = -360.;
  bounds[1] = 360.;
  bounds[2] = -90.;
  bounds[3] = 90.;
  btime_i[0] = 1962;
  btime_i[1] = 2;
  btime_i[2] = 21;
  btime_i[3] = 10;
  btime_i[4] = 30;
  btime_i[5] = 0;
  btime_i[6] = 0;
  etime_i[0] = 2062;
  etime_i[1] = 2;
  etime_i[2] = 21;
  etime_i[3] = 10;
  etime_i[4] = 30;
  etime_i[5] = 0;
  etime_i[6] = 0;
  speedmin = 0.0;
  timegap = 1000000000.0;

  /* initialize reading the input swath sonar file */
  if (mb_read_init(verbose, process->mbp_ifile, process->mbp_format, pings, lonflip, bounds, btime_i, etime_i,
                   speedmin, timegap, &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
                   error) != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, *error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
    fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", process->mbp_ifile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
  }

  /* initialize writing the output swath sonar file */
  if (mb_write_init(verbose, process->mbp_ofile, process->mbp_format,
                    &ombio_ptr, &beams_bath, &beams_amp, &pixels_ss,
                    error) != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, *error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
    fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", process->mbp_ofile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
  }

  /* initialize writing the output fast bathymetry *fbt file */
  bool make_fbt = false;
  void *fmbio_ptr = nullptr;
  void *fstore_ptr = nullptr;
  struct mb_io_struct *fmb_io_ptr = nullptr;
  struct mbsys_ldeoih_struct *fstore = nullptr;
  if (mb_should_make_fbt(verbose, process->mbp_format)) {
    char fbtfile[MB_PATH_MAXLINE+10];

    snprintf(fbtfile, sizeof(fbtfile), "%s.fbt", process->mbp_ofile);
    int fbeams_bath = 0;
    int fbeams_amp = 0;
    int fpixels_ss = 0;
    if (mb_write_init(verbose, fbtfile, MBF_MBLDEOIH,
                      &fmbio_ptr, &fbeams_bath, &fbeams_amp, &fpixels_ss,
                      error) != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", process->mbp_ofile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }
    fmb_io_ptr = (struct mb_io_struct *)fmbio_ptr;
    fstore = (struct mbsys_ldeoih_struct *) fmb_io_ptr->store_data;
    fstore_ptr = (void *) fstore;
    make_fbt = true;
  }

  /* initialize writing the output fast navigation *.fnv file */
  bool make_fnv = false;
  FILE *nfp = nullptr;
  if (mb_should_make_fnv(verbose, process->mbp_format)) {
    char fnvfile[MB_PATH_MAXLINE+10];
    snprintf(fnvfile, sizeof(fnvfile), "%s.fnv", process->mbp_ofile);
    if ((nfp = fopen(fnvfile, "w")) == nullptr) {
        fprintf(stderr, "\nUnable to open output *.fnv file <%s> for reading\n",
        fnvfile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(MB_ERROR_OPEN_FAIL);
    }
    make_fnv = true;
    fprintf(nfp,  "## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
                  "<longitude (deg)> <latitude (deg)> <heading (deg)> <speed (km/hr)> "
                  "<draft (m)> <roll (deg)> <pitch (deg)> <heave (m)> <portlon (deg)> "
                  "<portlat (deg)> <stbdlon (deg)> <stbdlat (deg)>\n");
  }

  /* initialize bounds that will be used in call to mbinfo to generate the *.inf file */
  bool mask_bounds_init = false;
  double mask_bounds[4];

  /* allocate memory for data arrays */
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflagorg, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack,
                               error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack,
                               error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */
        mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */
        mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ttimes, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_forward,
                               error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */
        mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_null, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bheave, error);
  if (*error == MB_ERROR_NO_ERROR)
    /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                               (void **)&alongtrack_offset, error);

  /* if error initializing memory then quit */
  if (*error != MB_ERROR_NO_ERROR) {
    char *message = nullptr;
    mb_error(verbose, *error, &message);
    fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
  }

  /* get data kind sources for input format */
  mb_format_source(verbose, &(process->mbp_format), &platform_source, &nav_source, &sensordepth_source, &heading_source,
                   &attitude_source, &svp_source, error);

  /*--------------------------------------------
    read the input file to get first ssv if necessary
    --------------------------------------------*/
  /* read input file until a surface sound velocity value
      is obtained, then close and reopen the file
      this provides the starting surface sound velocity
      for recalculating the bathymetry */
  if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE && traveltime &&
      process->mbp_ssv_mode != MBP_SSV_SET) {
    ssv_start = 0.0;
    ssv_prelimpass = true;
    *error = MB_ERROR_NO_ERROR;
    while (*error <= MB_ERROR_NO_ERROR && ssv_start <= 0.0) {
      /* read some data */
      *error = MB_ERROR_NO_ERROR;
      *status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                          &heading, &distance, &altitude, &sensordepth, &nbath, &namp, &nss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);

      /* time gaps do not matter to mbprocess */
      if (*error == MB_ERROR_TIME_GAP) {
        *status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
      }

      /* out of bounds do not matter to mbprocess */
      if (*error == MB_ERROR_OUT_BOUNDS) {
        *status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
      }

      /* non-survey data do not matter to mbprocess */
      if (*error == MB_ERROR_OTHER) {
        *status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
      }

      if (kind == MB_DATA_DATA && *error <= MB_ERROR_NO_ERROR) {
        /* extract travel times */
        *status = mb_ttimes(verbose, imbio_ptr, store_ptr, &kind, &nbeams, ttimes, angles, angles_forward,
                           angles_null, bheave, alongtrack_offset, &draft, &ssv, error);

        /* check surface sound velocity */
        if (ssv > 0.0)
          ssv_start = ssv;
      }
    }

    /* close and reopen the input file */
    *status = mb_close(verbose, &imbio_ptr, error);
    if (mb_read_init(verbose, process->mbp_ifile, process->mbp_format, pings, lonflip, bounds, btime_i,
                     etime_i, speedmin, timegap, &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp,
                     &pixels_ss, error) != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", process->mbp_ifile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }

    /* reallocate memory for data arrays */
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflagorg, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ttimes, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_forward, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_null, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bheave, error);
    if (*error == MB_ERROR_NO_ERROR)
      /* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&alongtrack_offset, error);

    /* if error initializing memory then quit */
    if (*error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(*error);
    }
  }
  if (ssv_start <= 0.0) {
    ssv_start = ssv_default;
  }

  if (*status == MB_FAILURE) {
    fprintf(stderr, "WARNING: status is MB_FAILURE.\n");
  }

  char user[256], host[256], date[32];
  *status = mb_user_host_date(verbose, user, host, date, error);

  /* reset error */
  *error = MB_ERROR_NO_ERROR;
  *status = MB_SUCCESS;

  /* open reverse edit save file (*.resf) */
  snprintf(resf_file, sizeof(resf_file), "%s.resf", process->mbp_ifile);
  if ((resf_fp = fopen(resf_file, "w")) == nullptr) {
    *error = MB_ERROR_OPEN_FAIL;
    char *message = nullptr;
    mb_error(verbose, *error, &message);
    fprintf(stderr, "\nReverse edit save file <%s> not initialized for writing\n", resf_file);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
  } else {
    /* put version header at beginning */
    memset(resf_header, 0, MB_PATH_MAXLINE);
    const int resf_mode = MB_ESF_MODE_EXPLICIT;
    snprintf(resf_header, sizeof(resf_header),
        "ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n",
        resf_mode, MB_VERSION, program_name, user, host, date);
    if (fwrite(resf_header, MB_PATH_MAXLINE, 1, resf_fp) != 1) {
      *status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
  }

  /* allocate memory for amplitude and sidescan correction arrays */
  /*  */
  if (process->mbp_sscorr_mode == MBP_SSCORR_ON || process->mbp_ampcorr_mode == MBP_AMPCORR_ON ||
      process->mbp_static_mode == MBP_STATIC_ANGLE_ON) {
    if (*error == MB_ERROR_NO_ERROR)
      *status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depths, error);
    if (*error == MB_ERROR_NO_ERROR)
      *status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthsmooth,
                                 error);
    if (*error == MB_ERROR_NO_ERROR)
      *status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                 (void **)&depthacrosstrack, error);
    if (*error == MB_ERROR_NO_ERROR)
      *status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopes,
                                 error);
    if (*error == MB_ERROR_NO_ERROR)
      *status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double),
                                 (void **)&slopeacrosstrack, error);
  }

  // TODO(schwehr): What if there was an error in the register process?

  /*--------------------------------------------
    output comments
    --------------------------------------------*/

  /* write comments to beginning of output file */
  if (!process->mbp_strip_comments) {
    /* insert metadata */
    if (strlen(process->mbp_meta_vessel) > 0) {
      snprintf(comment, sizeof(comment), "METAVESSEL:%s", process->mbp_meta_vessel);
      // TODO(schwehr): Don't set "status =" for all the mb_put_comment.
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_institution) > 0) {
      snprintf(comment, sizeof(comment), "METAINSTITUTION:%s", process->mbp_meta_institution);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_platform) > 0) {
      snprintf(comment, sizeof(comment), "METAPLATFORM:%s", process->mbp_meta_platform);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_sonar) > 0) {
      snprintf(comment, sizeof(comment), "METASONAR:%s", process->mbp_meta_sonar);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_sonarversion) > 0) {
      snprintf(comment, sizeof(comment), "METASONARVERSION:%s", process->mbp_meta_sonarversion);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_cruiseid) > 0) {
      snprintf(comment, sizeof(comment), "METACRUISEID:%s", process->mbp_meta_cruiseid);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_cruisename) > 0) {
      snprintf(comment, sizeof(comment), "METACRUISENAME:%s", process->mbp_meta_cruisename);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_pi) > 0) {
      snprintf(comment, sizeof(comment), "METAPI:%s", process->mbp_meta_pi);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_piinstitution) > 0) {
      snprintf(comment, sizeof(comment), "METAPIINSTITUTION:%s", process->mbp_meta_piinstitution);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (strlen(process->mbp_meta_client) > 0) {
      snprintf(comment, sizeof(comment), "METACLIENT:%s", process->mbp_meta_client);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_svcorrected > -1) {
      snprintf(comment, sizeof(comment), "METASVCORRECTED:%d", process->mbp_meta_svcorrected);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_tidecorrected > -1) {
      snprintf(comment, sizeof(comment), "METATIDECORRECTED:%d", process->mbp_meta_tidecorrected);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_batheditmanual > -1) {
      snprintf(comment, sizeof(comment), "METABATHEDITMANUAL:%d", process->mbp_meta_batheditmanual);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_batheditauto > -1) {
      snprintf(comment, sizeof(comment), "METABATHEDITAUTO:%d", process->mbp_meta_batheditauto);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_rollbias < MBP_METANOVALUE) {
      snprintf(comment, sizeof(comment), "METAROLLBIAS:%f", process->mbp_meta_rollbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_pitchbias < MBP_METANOVALUE) {
      snprintf(comment, sizeof(comment), "METAPITCHBIAS:%f", process->mbp_meta_pitchbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_headingbias < MBP_METANOVALUE) {
      snprintf(comment, sizeof(comment), "METAHEADINGBIAS:%f", process->mbp_meta_headingbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_meta_draft < MBP_METANOVALUE) {
      snprintf(comment, sizeof(comment), "METADRAFT:%f", process->mbp_meta_draft);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    kind = MB_DATA_COMMENT;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Swath data modified by program %s", program_name);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "MB-system Version %s", MB_VERSION);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Run by user <%s> on cpu <%s> at <%s>", user, host, date);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;

    if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "Depths and crosstrack distances recalculated from travel times");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  by raytracing through a water velocity profile specified");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  by the user.  The depths have been saved in units of");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      if (!process->mbp_corrected)
        snprintf(comment, sizeof(comment), "  uncorrected meters (the depth values are adjusted to be");
      else
        snprintf(comment, sizeof(comment), "  corrected meters (the depth values obtained by");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      if (!process->mbp_corrected)
        snprintf(comment, sizeof(comment), "  consistent with a vertical water velocity of 1500 m/s).");
      else
        snprintf(comment, sizeof(comment), "  raytracing are not adjusted further).");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "Depths and crosstrack distances adjusted for roll bias");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  and pitch bias.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "Depths and crosstrack distances adjusted for ");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  change in transducer depth and/or heave.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Control Parameters:");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "  MBIO data format:   %d", process->mbp_format);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "  Input file:         %s", process->mbp_ifile);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "  Output file:        %s", process->mbp_ofile);
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;

    if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE) {
      if (process->mbp_angle_mode == MBP_ANGLES_OK) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Angle mode:         angles not altered");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else if (process->mbp_angle_mode == MBP_ANGLES_SNELL) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Angle mode:         angles corrected using Snell's Law");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else if (process->mbp_angle_mode == MBP_ANGLES_SNELLNULL) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Angle mode:         angles corrected using Snell's Law and array geometry");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Default SSV:        %f", ssv_default);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      if (ssv_prelimpass) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  SSV initial pass:   on");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  SSV initial pass:   off");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }

      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  SVP file:               %s", process->mbp_svpfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Input water sound velocity profile:");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "    depth (m)   velocity (m/s)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      for (int i = 0; i < nsvp; i++) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "     %10.2f     %10.2f", depth[i], velocity[i]);
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
    }
    if (process->mbp_svp_mode != MBP_SVP_OFF) {
      if (process->mbp_corrected) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Output bathymetry reference:   CORRECTED");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else if (!process->mbp_corrected) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Output bathymetry reference:   UNCORRECTED");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
    }
    if (process->mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF) {
      if (process->mbp_corrected) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Depths modified from uncorrected to corrected.");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Depths modified from corrected to uncorrected.");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
    }

    if (process->mbp_rollbias_mode == MBP_ROLLBIAS_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Roll bias:       OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Roll bias:       %f degrees (starboard: -, port: +)", process->mbp_rollbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Port roll bias:  %f degrees (starboard: -, port: +)", process->mbp_rollbias_port);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Starboard roll bias:  %f degrees (starboard: -, port: +)", process->mbp_rollbias_stbd);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Pitch bias:      OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Pitch bias:      %f degrees (aft: -, forward: +)", process->mbp_pitchbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    if (process->mbp_draft_mode == MBP_DRAFT_SET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft set:      %f meters", process->mbp_draft);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_draft_mode == MBP_DRAFT_OFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft offset:    %f meters", process->mbp_draft_offset);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLY) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft multiplier: %f", process->mbp_draft_mult);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft offset:    %f meters", process->mbp_draft_offset);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft multiplier: %f", process->mbp_draft_mult);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_draft_mode == MBP_DRAFT_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Draft:           not modified");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_heave_mode == MBP_HEAVE_OFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heave offset: %f meters", process->mbp_heave);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_heave_mode == MBP_HEAVE_MULTIPLY) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heave multiplier: %f", process->mbp_heave_mult);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heave offset: %f meters", process->mbp_heave);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heave multiplier: %f", process->mbp_heave_mult);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_heave_mode == MBP_HEAVE_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heave:           not modified");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_tt_mode == MBP_TT_MULTIPLY) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Travel time multiplier: %f", process->mbp_tt_mult);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_tt_mode == MBP_TT_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Travel time:     not modified");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_lever_mode == MBP_LEVER_OFF) {
      snprintf(comment, sizeof(comment), "  Lever calculation off.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else {
      snprintf(comment, sizeof(comment), "  Lever calculation used to calculate heave correction.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  VRU offset x:                  %f m", process->mbp_vru_offsetx);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  VRU offset y:                  %f m", process->mbp_vru_offsety);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  VRU offset z:                  %f m", process->mbp_vru_offsetz);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Sonar offset x:                %f m", process->mbp_sonar_offsetx);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Sonar offset y:                %f m", process->mbp_sonar_offsety);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Sonar offset z:                %f m", process->mbp_sonar_offsetz);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_tide_mode == MBP_TIDE_OFF) {
      snprintf(comment, sizeof(comment), "  Tide calculation off.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else {
      snprintf(comment, sizeof(comment), "  Tide correction applied to bathymetry.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Tide file:                     %s", process->mbp_tidefile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Tide format:                   %d", process->mbp_tide_format);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_nav_mode == MBP_NAV_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Merge navigation:          OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_nav_mode == MBP_NAV_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Merged navigation file:    %s", process->mbp_navfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;

      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Merged navigation format:  %d", process->mbp_nav_format);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;

      if (process->mbp_nav_heading == MBP_NAV_ON) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Heading merge:         ON");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Heading merge:         OFF");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      if (process->mbp_nav_speed == MBP_NAV_ON) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Speed merge:           ON");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Speed merge:           OFF");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      if (process->mbp_nav_draft == MBP_NAV_ON) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Draft merge:           ON");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Draft merge:           OFF");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      if (process->mbp_nav_attitude == MBP_NAV_ON) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Attitude merge:        ON");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Attitude merge:        OFF");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      if (process->mbp_nav_algorithm == MBP_NAV_LINEAR) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Navigation algorithm: linear interpolation");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      else if (process->mbp_nav_algorithm == MBP_NAV_SPLINE) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Navigation algorithm: spline interpolation");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
      }
      snprintf(comment, sizeof(comment), "  Navigation time shift:         %f", process->mbp_nav_timeshift);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_nav_shift == MBP_NAV_ON) {
      snprintf(comment, sizeof(comment), "  Navigation positions shifted.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Navigation offset x:       %f", process->mbp_nav_offsetx);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Navigation offset y:       %f", process->mbp_nav_offsety);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Navigation offset z:       %f", process->mbp_nav_offsetz);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Navigation shift longitude:%f", process->mbp_nav_shiftlon);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Navigation shift latitude: %f", process->mbp_nav_shiftlat);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else {
      snprintf(comment, sizeof(comment), "  Navigation positions not shifted.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_navadj_mode == MBP_NAVADJ_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Merge adjusted navigation: OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_navadj_mode >= MBP_NAVADJ_LL) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Adjusted navigation file: %s", process->mbp_navadjfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      if (process->mbp_navadj_mode == MBP_NAVADJ_LL) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Adjusted navigation applied to lon lat only");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
        strncpy(comment, "", MB_COMMENT_MAXLINE);
      }
      else if (process->mbp_navadj_mode == MBP_NAVADJ_LLZ) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Adjusted navigation applied to lon lat depth");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        if (*error == MB_ERROR_NO_ERROR)
          ocomment++;
        strncpy(comment, "", MB_COMMENT_MAXLINE);
      }
      if (process->mbp_navadj_algorithm == MBP_NAV_LINEAR)
        snprintf(comment, sizeof(comment), "  Adjusted navigation algorithm: linear interpolation");
      else if (process->mbp_navadj_algorithm == MBP_NAV_SPLINE)
        snprintf(comment, sizeof(comment), "  Adjusted navigation algorithm: spline interpolation");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_attitude_mode == MBP_ATTITUDE_OFF) {
      snprintf(comment, sizeof(comment), "  Attitude merging:              OFF.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else {
      snprintf(comment, sizeof(comment), "  Attitude merging:              ON.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Attitude file:                 %s", process->mbp_attitudefile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Attitude format:               %d", process->mbp_attitude_format);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_sensordepth_mode == MBP_SENSORDEPTH_OFF) {
      snprintf(comment, sizeof(comment), "  Sensordepth merging:              OFF.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else {
      snprintf(comment, sizeof(comment), "  Sensordepth merging:              ON.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Sensordepth file:                 %s", process->mbp_sensordepthfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
      snprintf(comment, sizeof(comment), "  Sensordepth format:               %d", process->mbp_sensordepth_format);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_heading_mode == MBP_HEADING_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heading modify:       OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_heading_mode == MBP_HEADING_CALC || process->mbp_heading_mode == MBP_HEADING_CALCOFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heading modify:       COURSE MADE GOOD");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    if (process->mbp_heading_mode == MBP_HEADING_OFFSET || process->mbp_heading_mode == MBP_HEADING_CALCOFFSET) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Heading offset:       %f deg", process->mbp_headingbias);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Amplitude Corrections:");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (process->mbp_ampcorr_mode == MBP_AMPCORR_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Amplitude vs grazing angle corrections applied to amplitudes.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Amplitude correction file:      %s m", process->mbp_ampcorrfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (process->mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction by subtraction (dB scale)");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction by division (linear scale)");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      if (process->mbp_ampcorr_symmetry == MBP_AMPCORR_SYMMETRIC) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  AVGA tables forced to be symmetric");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  AVGA tables allowed to be asymmetric");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Reference grazing angle:       %f deg", process->mbp_ampcorr_angle);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (process->mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE ||
          process->mbp_ampcorr_slope == MBP_AMPCORR_USESLOPE) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction uses swath bathymetry in file");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction uses topography grid");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Topography grid file:      %s m", process->mbp_ampsscorr_topofile);
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      if (process->mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction ignores seafloor slope");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Amplitude correction uses seafloor slope");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
    }
    else {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Amplitude correction off.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    }

    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Sidescan Corrections:");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (process->mbp_sscorr_mode == MBP_SSCORR_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Amplitude vs grazing angle corrections applied to sidescan.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan correction file:      %s m", process->mbp_sscorrfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (process->mbp_sscorr_type == MBP_SSCORR_SUBTRACTION) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction by subtraction (dB scale)");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction by division (linear scale)");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      if (process->mbp_sscorr_symmetry == MBP_SSCORR_SYMMETRIC) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  AVGA tables forced to be symmetric");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      } else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  AVGA tables allowed to be asymmetric");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Reference grazing angle:       %f deg", process->mbp_sscorr_angle);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (process->mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE || process->mbp_sscorr_slope == MBP_SSCORR_USESLOPE) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction uses swath bathymetry in file");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      } else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction uses topography grid");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Topography grid file:      %s m", process->mbp_ampsscorr_topofile);
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
      if (process->mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE) {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction ignores seafloor slope");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      } else {
        strncpy(comment, "", MB_COMMENT_MAXLINE);
        snprintf(comment, sizeof(comment), "  Sidescan correction uses seafloor slope");
        *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      }
    } else {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan correction off.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    }

    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), "Sidescan Recalculation:");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (process->mbp_ssrecalc_mode == MBP_SSRECALC_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan recalculated.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan pixel size:           %f", process->mbp_ssrecalc_pixelsize);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan swath width:          %f", process->mbp_ssrecalc_swathwidth);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan interpolation:        %d", process->mbp_ssrecalc_interpolate);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    } else {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Sidescan not recalculated.");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    }

    strncpy(comment, "", MB_COMMENT_MAXLINE);
    if (process->mbp_cut_num > 0)
      snprintf(comment, sizeof(comment), "  Data cutting enabled (%d commands).", process->mbp_cut_num);
    else
      snprintf(comment, sizeof(comment), "  Data cutting disabled.");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
    for (int i = 0; i < process->mbp_cut_num; i++) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Cut[%d]: %d %d %f %f", i, process->mbp_cut_kind[i], process->mbp_cut_mode[i],
              process->mbp_cut_min[i], process->mbp_cut_max[i]);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      snprintf(comment, sizeof(comment), "  %f %f", process->mbp_cut_min[i], process->mbp_cut_max[i]);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    if (process->mbp_edit_mode == MBP_EDIT_OFF) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Merge bath edit:      OFF");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_edit_mode == MBP_EDIT_ON) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Bathy edit file:      %s", process->mbp_editfile);
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    if (process->mbp_kluge001) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge001 applied (travel time correction to HSDS2 data)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge002) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge002 applied (heave correction to Simrad data)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge003) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge003 applied (roll correction for USCG Healy SB2112 data)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge004) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge004 applied (remove data with overlapping time stamps)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge005) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge005 applied (replaces survey record timestamps withtimestamps of "
                       "corresponding merged navigation records)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge006) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment),
          "  Processing Kluge006 applied (changes sonar depth / draft values without changing bathymetry values)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge007) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge007 applied (zero alongtrack values > half altitude)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge008) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge008 applied (undefined)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge009) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge009 applied (undefined)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    else if (process->mbp_kluge010) {
      strncpy(comment, "", MB_COMMENT_MAXLINE);
      snprintf(comment, sizeof(comment), "  Processing Kluge010 applied (undefined)");
      *status = mb_put_comment(verbose, ombio_ptr, comment, error);
      if (*error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    strncpy(comment, "", MB_COMMENT_MAXLINE);
    snprintf(comment, sizeof(comment), " ");
    *status = mb_put_comment(verbose, ombio_ptr, comment, error);
    if (*error == MB_ERROR_NO_ERROR)
      ocomment++;
  }

  /* set up the raytracing */
  if (process->mbp_svp_mode != MBP_SVP_OFF)
    *status = mb_rt_init(verbose, nsvp, depth, velocity, &rt_svp, error);

  /* set up the sidescan recalculation */
  if (process->mbp_ssrecalc_mode == MBP_SSRECALC_ON) {
    if (process->mbp_ssrecalc_pixelsize != 0.0) {
      pixel_size_set = true;
      pixel_size = process->mbp_ssrecalc_pixelsize;
    }
    else {
      pixel_size_set = false;
      pixel_size = 0.0;
    }
    if (process->mbp_ssrecalc_swathwidth != 0.0) {
      swath_width_set = true;
      swath_width = process->mbp_ssrecalc_swathwidth;
    }
    else {
      swath_width_set = false;
      swath_width = 0.0;
    }
    pixel_int = process->mbp_ssrecalc_interpolate;
  }

  double time_d_lastping = 0.0;
  int inavtime = 0;
  int iattitudetime = 0;
  int isensordepthtime = 0;
  int inavadjtime = 0;
  int itidetime = 0;

  /*--------------------------------------------
    loop over reading input
    --------------------------------------------*/

  /* read and write */
  while (*error <= MB_ERROR_NO_ERROR) {
    /* read some data */
    *error = MB_ERROR_NO_ERROR;
    *status = MB_SUCCESS;
    *status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                        &distance, &altitude, &sensordepth, &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack,
                        bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);

    /* time gaps do not matter to mbprocess */
    if (*error == MB_ERROR_TIME_GAP) {
      *status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
    }

    /* out of bounds do not matter to mbprocess */
    if (*error == MB_ERROR_OUT_BOUNDS) {
      *status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
    }

    /* non-survey data do not matter to mbprocess */
    if (*error == MB_ERROR_OTHER) {
      *status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
    }

    /* compare and save survey data timestamps */
    if (process->mbp_kluge004 && error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      if (time_d <= time_d_lastping) {
        *error = MB_ERROR_UNINTELLIGIBLE;
        *status = MB_FAILURE;
      }
    }

    /* save the original beamflag states */
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      for (int i = 0; i < nbath; i++) {
        beamflagorg[i] = beamflag[i];
      }
    }

    /* detect multiple pings with the same time stamps */
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      int sensorhead_error = MB_ERROR_NO_ERROR;
      const int sensorhead_status = mb_sensorhead(verbose, imbio_ptr, store_ptr, &sensorhead, &sensorhead_error);
      mb_sonartype(verbose, imbio_ptr, store_ptr, &sensortype, &sensorhead_error);
      if (sensorhead_status == MB_SUCCESS) {
        pingmultiplicity = sensorhead;
      }
      else if (fabs(time_d - time_d_lastping) < MB_ESF_MAXTIMEDIFF) {
        pingmultiplicity++;
      }
      else {
        pingmultiplicity = 0;
      }
      time_d_lastping = time_d;
    }

    /* increment counter */
    if (*error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
      idata++;
    else if (*error <= MB_ERROR_NO_ERROR && kind == nav_source)
      inav++;
    else if (*error <= MB_ERROR_NO_ERROR && kind == MB_DATA_COMMENT)
      icomment++;
    else if (*error <= MB_ERROR_NO_ERROR)
      iother++;

    /* output error messages */
    if (verbose >= 1 && *error == MB_ERROR_COMMENT) {
      if (icomment == 1)
        fprintf(stderr, "\nComments in Input:\n");
      fprintf(stderr, "%s\n", comment);
    }
    else if (verbose >= 1 && *error < MB_ERROR_NO_ERROR && *error > MB_ERROR_OTHER) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
      fprintf(stderr, "Input Record: %d\n", idata);
      fprintf(stderr, "Time: %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
              time_i[5]);
    }
    else if (verbose >= 1 && *error < MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
      fprintf(stderr, "Input Record: %d\n", idata);
    }
    else if (verbose >= 1 && *error != MB_ERROR_NO_ERROR && *error != MB_ERROR_EOF) {
      char *message = nullptr;
      mb_error(verbose, *error, &message);
      fprintf(stderr, "\nFatal MBIO Error:\n%s\n", message);
      fprintf(stderr, "Last Good Time: %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
              time_i[5]);
    }

    /*--------------------------------------------
      handle kluges 1 and 7
      --------------------------------------------*/

    /* apply kluge001 - enables correction of travel times in
                Hydrosweep DS2 data from the R/V Maurice
                Ewing in 2001 and 2002. */
    if (process->mbp_kluge001 && kind == MB_DATA_DATA && (format == 182 || format == 183))
      *status = mbsys_atlas_ttcorr(verbose, imbio_ptr, store_ptr, error);

    /* apply kluge007 - zero alongtrack distances > half the altitude */
    if (process->mbp_kluge007 && kind == MB_DATA_DATA) {
      for (int i = 0; i < nbath; i++) {
        if (fabs(bathalongtrack[i]) > 0.5 * altitude)
          bathalongtrack[i] = 0.0;
      }
      for (int i = 0; i < nss; i++) {
        if (fabs(ssalongtrack[i]) > 0.5 * altitude)
          ssalongtrack[i] = 0.0;
      }
    }

    /*--------------------------------------------
      handle navigation merging
      --------------------------------------------*/

    /* extract the navigation if available and set scaling that may be needed many times */
    if (*error == MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA || kind == nav_source)) {
      *status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                              &heading_org, &draft_org, &roll_org, &pitch_org, &heave_org, error);
      heading = heading_org;
      draft = draft_org;
      roll = roll_org;
      pitch = pitch_org;
      heave = heave_org;

      mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
      headingx = sin(heading * DTR);
      headingy = cos(heading * DTR);

      /* apply kluge002 - enables correction of draft values in Simrad data
                 - some Simrad multibeam data has had an
                   error in which the heave has bee added
                   to the sonar depth (draft for hull
                   mounted sonars)
                 - this correction subtracts the heave
                   value from the sonar depth */
      if (process->mbp_kluge002 && kind == MB_DATA_DATA)
        draft -= heave;
    }

    /* apply kluge005 - replaces survey record timestamps with
            timestamps of corresponding merged navigation
            records
            - this feature allows users to fix
                  timestamp errors using MBnavedit and
                  then insert the corrected timestamps
                  into processed data */
    if (process->mbp_kluge005 && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && nnav > 0) {
      time_d = ntime[idata - 1];
      mb_get_date(verbose, time_d, time_i);
    }

    /* interpolate the navigation if desired */
    if (*error == MB_ERROR_NO_ERROR && process->mbp_nav_mode == MBP_NAV_ON &&
        (kind == MB_DATA_DATA || kind == nav_source)) {
      /* interpolate navigation */
      if (process->mbp_nav_algorithm == MBP_NAV_SPLINE && time_d >= ntime[0] && time_d <= ntime[nnav - 1]) {
        mb_spline_interp(verbose, ntime - 1, nlon - 1, nlonspl - 1, nnav, time_d, &navlon, &inavtime, error);
        mb_spline_interp(verbose, ntime - 1, nlat - 1, nlatspl - 1, nnav, time_d, &navlat, &inavtime, error);
      }
      else {
        mb_linear_interp_longitude(verbose, ntime - 1, nlon - 1, nnav, time_d, &navlon, &inavtime, error);
        mb_linear_interp_latitude(verbose, ntime - 1, nlat - 1, nnav, time_d, &navlat, &inavtime, error);
      }

      /* interpolate heading */
      if (process->mbp_nav_heading == MBP_NAV_ON) {
        mb_linear_interp_heading(verbose, ntime - 1, nheading - 1, nnav, time_d, &heading, &inavtime, error);
        if (heading < 0.0)
          heading += 360.0;
        else if (heading > 360.0)
          heading -= 360.0;
      }

      /* interpolate speed */
      if (process->mbp_nav_speed == MBP_NAV_ON) {
        mb_linear_interp(verbose, ntime - 1, nspeed - 1, nnav, time_d, &speed, &inavtime, error);
      }

      /* interpolate draft */
      if (process->mbp_nav_draft == MBP_NAV_ON) {
        mb_linear_interp(verbose, ntime - 1, ndraft - 1, nnav, time_d, &draft, &inavtime, error);
      }

      /* interpolate attitude */
      if (process->mbp_nav_attitude == MBP_NAV_ON) {
        mb_linear_interp(verbose, ntime - 1, nroll - 1, nnav, time_d, &roll, &inavtime, error);
        mb_linear_interp(verbose, ntime - 1, npitch - 1, nnav, time_d, &pitch, &inavtime, error);
        mb_linear_interp(verbose, ntime - 1, nheave - 1, nnav, time_d, &heave, &inavtime, error);
      }
    }

    /*--------------------------------------------
      handle attitude merging
      --------------------------------------------*/

    /* interpolate the attitude if desired */
    if (*error == MB_ERROR_NO_ERROR && process->mbp_attitude_mode == MBP_ATTITUDE_ON &&
        (kind == MB_DATA_DATA || kind == nav_source)) {
      /* interpolate adjusted navigation */
      mb_linear_interp(verbose, attitudetime - 1, attituderoll - 1,
                        nattitude, time_d, &roll, &iattitudetime, error);
      mb_linear_interp(verbose, attitudetime - 1, attitudepitch - 1,
                        nattitude, time_d, &pitch, &iattitudetime, error);
      mb_linear_interp(verbose, attitudetime - 1, attitudeheave - 1,
                        nattitude, time_d, &heave, &iattitudetime, error);
    }

    /*--------------------------------------------
      handle sensor depth merging
      --------------------------------------------*/

    /* interpolate the sensordepth if desired */
    if (*error == MB_ERROR_NO_ERROR && process->mbp_sensordepth_mode == MBP_SENSORDEPTH_ON &&
        (kind == MB_DATA_DATA || kind == nav_source)) {
      /* interpolate adjusted navigation */
      mb_linear_interp(verbose, fsensordepthtime - 1, fsensordepth - 1, nsensordepth, time_d, &draft,
                                 &isensordepthtime, error);
    }

    /*--------------------------------------------
      handle position shifts
      --------------------------------------------*/

    /* apply position shifts if needed */
    if (process->mbp_nav_shift == MBP_NAV_ON) {
      navlon -= (headingy * mtodeglon * process->mbp_nav_offsetx + headingx * mtodeglon * process->mbp_nav_offsety -
                 mtodeglon * process->mbp_nav_shiftx - process->mbp_nav_shiftlon);
      navlat -= (-headingx * mtodeglat * process->mbp_nav_offsetx + headingy * mtodeglat * process->mbp_nav_offsety -
                 mtodeglat * process->mbp_nav_shifty - process->mbp_nav_shiftlat);
    }

    /*--------------------------------------------
      handle draft correction
      --------------------------------------------*/
    /* add user specified draft correction if desired */
    if (*error == MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA || kind == nav_source)) {
      if (process->mbp_draft_mode == MBP_DRAFT_OFFSET)
        draft = draft + process->mbp_draft_offset;
      else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLY)
        draft = draft * process->mbp_draft_mult;
      else if (process->mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
        draft = draft * process->mbp_draft_mult + process->mbp_draft_offset;
      else if (process->mbp_draft_mode == MBP_DRAFT_SET)
        draft = process->mbp_draft;
    }

    /*--------------------------------------------
      handle adjusted navigation merging
      --------------------------------------------*/

    /* interpolate the adjusted navigation if desired */
    if (*error == MB_ERROR_NO_ERROR && process->mbp_navadj_mode >= MBP_NAVADJ_LL &&
        (kind == MB_DATA_DATA || kind == nav_source)) {
      /* interpolate adjusted navigation */
      if (process->mbp_navadj_algorithm == MBP_NAV_SPLINE && time_d >= natime[0] && time_d <= natime[nanav - 1]) {
        mb_spline_interp(verbose, natime - 1, nalon - 1, nalonspl - 1, nanav, time_d, &navlon, &inavadjtime, error);
        mb_spline_interp(verbose, ntime - 1, nalat - 1, nalatspl - 1, nanav, time_d, &navlat, &inavadjtime, error);
      }
      else {
        mb_linear_interp_longitude(verbose, natime - 1, nalon - 1, nanav, time_d, &navlon, &inavadjtime, error);
        mb_linear_interp_latitude(verbose, natime - 1, nalat - 1, nanav, time_d, &navlat, &inavadjtime, error);
      }
    }

    /*--------------------------------------------
      apply z offset from navigation adjustment correction
      --------------------------------------------*/

    /* apply z offset from navigation adjustment correction */
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && process->mbp_navadj_mode == MBP_NAVADJ_LLZ &&
        nanav > 1) {
      /* interpolate z offset */
      if (process->mbp_navadj_algorithm == MBP_NAV_SPLINE && time_d >= natime[0] && time_d <= natime[nanav - 1]) {
        mb_spline_interp(verbose, natime - 1, naz - 1, nazspl - 1, nanav, time_d, &zoffset, &inavadjtime, error);
      }
      else {
        mb_linear_interp(verbose, natime - 1, naz - 1, nanav, time_d, &zoffset, &inavadjtime, error);
      }

      /* apply z offset to draft / sonar depth */
      draft += zoffset;
    }

    /*--------------------------------------------
      apply tide correction
      --------------------------------------------*/

    /* apply tide corrections */
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA
      && process->mbp_tide_mode == MBP_TIDE_ON && ntide > 0) {
      /* interpolate tide */
      mb_linear_interp(verbose, tidetime - 1, tide - 1, ntide, time_d, &tideval, &itidetime, error);

      /* apply tide to to draft / sonar depth */
      draft -= tideval;
    }

    /*--------------------------------------------
      handle lever arm correction
      --------------------------------------------*/

    /* do lever calculation to find heave implied by roll and pitch
       for a sonar displaced from the vru - this will be added to the
       bathymetry */
    if (*error == MB_ERROR_NO_ERROR && process->mbp_lever_mode == MBP_LEVER_ON && kind == MB_DATA_DATA) {
      alpha = pitch;
      beta = roll;
      if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
        alpha += process->mbp_pitchbias;
      if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
        beta += process->mbp_rollbias;
      else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
        beta += 0.5 * (process->mbp_rollbias_port + process->mbp_rollbias_stbd);
      mb_lever(verbose, process->mbp_sonar_offsetx, process->mbp_sonar_offsety, process->mbp_sonar_offsetz,
               (double)0.0, (double)0.0, (double)0.0, process->mbp_vru_offsetx, process->mbp_vru_offsety,
               process->mbp_vru_offsetz, alpha, beta, &lever_x, &lever_y, &lever_heave, error);
    }

    /*--------------------------------------------
      handle speed and heading calculation
      --------------------------------------------*/

    /* make up heading and speed if required */
    bool calculatespeedheading = false;
    if (process->mbp_heading_mode == MBP_HEADING_CALC || process->mbp_heading_mode == MBP_HEADING_CALCOFFSET)
      calculatespeedheading = true;
    for (icut = 0; icut < process->mbp_cut_num; icut++) {
      if (process->mbp_cut_mode[icut] == MBP_CUT_MODE_SPEED)
        calculatespeedheading = true;
    }
    if (*error == MB_ERROR_NO_ERROR
        && (kind == MB_DATA_DATA || kind == nav_source)
        && calculatespeedheading) {
      if (process->mbp_nav_mode == MBP_NAV_ON && inavtime > 0) {
        mb_coor_scale(verbose, nlat[inavtime - 1], &mtodeglon, &mtodeglat);
        del_time = ntime[inavtime] - ntime[inavtime - 1];
        dx = (nlon[inavtime] - nlon[inavtime - 1]) / mtodeglon;
        dy = (nlat[inavtime] - nlat[inavtime - 1]) / mtodeglat;
      }
      else if (process->mbp_navadj_mode >= MBP_NAVADJ_LL && inavadjtime > 0) {
        mb_coor_scale(verbose, nalat[inavadjtime - 1], &mtodeglon, &mtodeglat);
        del_time = natime[inavadjtime] - natime[inavadjtime - 1];
        dx = (nalon[inavadjtime] - nalon[inavadjtime - 1]) / mtodeglon;
        dy = (nalat[inavadjtime] - nalat[inavadjtime - 1]) / mtodeglat;
      }
      else if ((kind == MB_DATA_DATA && idata > 1) || (kind == nav_source && inav > 1)) {
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        del_time = time_d - time_d_old;
        dx = (navlon - navlon_old) / mtodeglon;
        dy = (navlat - navlat_old) / mtodeglat;
      }
      if ((process->mbp_nav_mode == MBP_NAV_ON) || (process->mbp_navadj_mode >= MBP_NAVADJ_LL) ||
          ((kind == MB_DATA_DATA && idata > 1) || (kind == nav_source && inav > 1))) {
        dist = sqrt(dx * dx + dy * dy);
        if (del_time > 0.0) {
          speedcalc = 3.6 * dist / del_time;
        }
        else
          speedcalc = speed_old;
        if (dist > 0.0 && del_time > 0.0) {
          headingcalc = RTD * atan2(dx / dist, dy / dist);
          if (headingcalc < 0.0)
            headingcalc += 360.0;
        }
        else
          headingcalc = heading_old;
      }
      else {
        speedcalc = speed;
        headingcalc = heading;
      }
      if (process->mbp_heading_mode == MBP_HEADING_CALC || process->mbp_heading_mode == MBP_HEADING_CALCOFFSET) {
        heading = headingcalc;
      }
      else {
        speed = speedcalc;
      }
      time_d_old = time_d;
      navlon_old = navlon;
      navlat_old = navlat;
      heading_old = headingcalc;
      speed_old = speedcalc;
    }

    /* adjust heading if required */
    if (*error == MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA || kind == nav_source) &&
        (process->mbp_heading_mode == MBP_HEADING_OFFSET || process->mbp_heading_mode == MBP_HEADING_CALCOFFSET)) {
      heading += process->mbp_headingbias;
      if (heading >= 360.0)
        heading -= 360.0;
      else if (heading < 0.0)
        heading += 360.0;
    }

    /*--------------------------------------------
      deal with bathymetry
      --------------------------------------------*/

    /* if survey data encountered,
        get the bathymetry */
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {

      /*--------------------------------------------
        get travel time values
        --------------------------------------------*/

      /* extract travel times if they exist */
      if (traveltime) {
        *status = mb_ttimes(verbose, imbio_ptr, store_ptr, &kind, &nbeams, ttimes, angles, angles_forward,
                           angles_null, bheave, alongtrack_offset, &draft_org, &ssv, error);
      }

      /* estimate travel times if they don't exist */
      else {
        draft_org = sensordepth - heave;
        ssv = 1500.0;
        nbeams = nbath;
        for (int i = 0; i < nbath; i++) {
          if (beamflag[i] != MB_FLAG_NULL) {
            zz = bath[i] - sensordepth;
            rr = sqrt(zz * zz + bathacrosstrack[i] * bathacrosstrack[i] +
                      bathalongtrack[i] * bathalongtrack[i]);
            ttimes[i] = rr / 750.0;
            mb_xyz_to_takeoff(verbose, bathacrosstrack[i], bathalongtrack[i], (bath[i] - sensordepth),
                              &angles[i], &angles_forward[i], error);
          }
          else {
            angles[i] = 0.0;
            angles_forward[i] = 0.0;
          }
          angles_null[i] = 0.0;
          bheave[i] = 0.0;
          alongtrack_offset[i] = 0.0;
        }
      }

      /*--------------------------------------------
        handle adjustments to ssv, heave, and travel times
        --------------------------------------------*/

      /* set surface sound speed to default if needed */
      if (ssv <= 0.0)
        ssv = ssv_start;
      else
        ssv_start = ssv;

      /* if heave adjustment specified do it */
      if (process->mbp_heave_mode != MBP_HEAVE_OFF) {
        if (process->mbp_heave_mode == MBP_HEAVE_MULTIPLY || process->mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET) {
          for (int i = 0; i < nbath; i++)
            bheave[i] *= process->mbp_heave_mult;
        }
        if (process->mbp_heave_mode == MBP_HEAVE_OFFSET || process->mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET) {
          for (int i = 0; i < nbath; i++)
            bheave[i] += process->mbp_heave;
        }
      }

      /* if tt adjustment specified do it */
      if (process->mbp_tt_mode == MBP_TT_MULTIPLY) {
        for (int i = 0; i < nbath; i++)
          ttimes[i] *= process->mbp_tt_mult;
      }

      /* if ssv adjustment specified do it */
      if (process->mbp_ssv_mode == MBP_SSV_SET) {
        ssv = process->mbp_ssv;
      }
      else if (process->mbp_ssv_mode == MBP_SSV_OFFSET) {
        ssv += process->mbp_ssv;
      }

      /*--------------------------------------------
        recalculate the bathymetry
        --------------------------------------------*/

      /* apply kluge006 - resets draft without changing bathymetry */
      if (process->mbp_kluge006 && kind == MB_DATA_DATA) {
        draft_org = draft;
      }

      /* if svp specified recalculate bathymetry
          by raytracing  */
      if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE) {
        /* loop over the beams */
        for (int i = 0; i < nbeams; i++) {
          if (ttimes[i] > 0.0) {
            /* if needed, translate angles from takeoff
                angle coordinates to roll-pitch
                coordinates, apply roll and pitch
                corrections, and translate back */
            if (process->mbp_rollbias_mode != MBP_ROLLBIAS_OFF ||
                process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON || process->mbp_nav_attitude == MBP_NAV_ON ||
                process->mbp_attitude_mode == MBP_ATTITUDE_ON || process->mbp_kluge003) {
              mb_takeoff_to_rollpitch(verbose, angles[i], angles_forward[i], &alpha, &beta, error);
              /* apply kluge_003 - enables correction of beam angles in
                   SeaBeam 2112 data
                   - a data sample from the SeaBeam 2112 on
                         the USCG Icebreaker Healy (collected on
                         23 July 2003) was found to have an error
                         in which the beam angles had 0.25 times
                         the roll added
                   - this correction subtracts 0.25 * roll
                         from the beam angles before the bathymetry
                         is recalculated by raytracing through a
                         water sound velocity profile
                   - the mbprocess parameter files must be
                         set to enable bathymetry recalculation
                         by raytracing in order to apply this
                         correction */
              if (process->mbp_kluge003)
                beta -= 0.25 * roll;
              if (process->mbp_nav_attitude == MBP_NAV_ON || process->mbp_attitude_mode == MBP_ATTITUDE_ON) {
                beta += roll - roll_org;
                alpha += pitch - pitch_org;
              }
              if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
                alpha += process->mbp_pitchbias;
              if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
                beta += process->mbp_rollbias;
              else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE && angles[i] >= 0.0)
                beta += process->mbp_rollbias_stbd;
              else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
                beta += process->mbp_rollbias_port;
              mb_rollpitch_to_takeoff(verbose, alpha, beta, &angles[i], &angles_forward[i], error);
            }

            /* add heave and draft */
            depth_offset_use = bheave[i] + draft + lever_heave;

            /* check depth_offset - use static shift if depth_offset negative */
            if (depth_offset_use >= depth[0]) {
              static_shift = 0.0;
            }
            else {
              static_shift = depth_offset_use - depth[0];

              if (verbose > 0) {
                fprintf(stderr, "\nWarning: Sonar depth is shallower than the top\n");
                fprintf(stderr, "of the SVP - transducers above water?!\n");
                fprintf(stderr, "Raytracing performed from top of SVP followed by static shift.\n");
                fprintf(stderr, "Sonar depth is sum of heave + draft (or transducer depth).\n");
                fprintf(stderr, "Draft from data:       %f\n", draft);
                fprintf(stderr, "Heave from data:       %f\n", bheave[i]);
                fprintf(stderr, "Heave from lever calc: %f\n", lever_heave);
                fprintf(stderr, "User specified draft:  %f\n", process->mbp_draft);
                fprintf(stderr, "Depth offset used:     %f\n", depth_offset_use);
                fprintf(stderr, "Data Record: %d\n", odata);
                fprintf(stderr, "Ping time:  %4d %2d %2d %2d:%2d:%2d.%6d\n", time_i[0], time_i[1],
                        time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
              }
            }

            /* raytrace */
            *status = mb_rt(verbose, rt_svp, (depth_offset_use - static_shift), angles[i], 0.5 * ttimes[i],
                           process->mbp_angle_mode, ssv, angles_null[i], 0, nullptr, nullptr, nullptr, nullptr, &xx, &zz, &ttime,
                           &ray_stat, error);

            /* apply static shift if any */
            zz += static_shift;

            /* get alongtrack and acrosstrack distances and depth */
            bathacrosstrack[i] = xx * cos(DTR * angles_forward[i]);
            bathalongtrack[i] = xx * sin(DTR * angles_forward[i]) + alongtrack_offset[i];
            bath[i] = zz;

            if (verbose >= 5) {
              fprintf(stderr, "dbg5       %3d %3d %6.3f %6.3f %6.3f %8.2f %8.2f %8.2f\n", idata, i,
                      0.5 * ttimes[i], angles[i], angles_forward[i], bathacrosstrack[i], bathalongtrack[i],
                      bath[i]);
            }
            if (verbose >= 5) {
              fprintf(stderr, "\ndbg5  Depth value calculated in program <%s>:\n", program_name);
              fprintf(stderr, "dbg5       kind:  %d\n", kind);
              fprintf(stderr, "dbg5       beam:  %d\n", i);
              fprintf(stderr, "dbg5       tt:     %f\n", ttimes[i]);
              fprintf(stderr, "dbg5       xx:     %f\n", xx);
              fprintf(stderr, "dbg5       zz:     %f\n", zz);
              fprintf(stderr, "dbg5       xtrack: %f\n", bathacrosstrack[i]);
              fprintf(stderr, "dbg5       ltrack: %f\n", bathalongtrack[i]);
              fprintf(stderr, "dbg5       depth:  %f\n", bath[i]);
            }
          }

          /* else if no travel time no data */
          else
            beamflag[i] = MB_FLAG_NULL;
        }
      }

      /* recalculate bathymetry by rigid rotations  */
      else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE) {
        /* loop over the beams */
        for (int i = 0; i < nbath; i++) {
          if (beamflag[i] != MB_FLAG_NULL) {
            /* output some debug messages */
            if (verbose >= 5) {
              fprintf(stderr, "\ndbg5  Depth value to be calculated in program <%s>:\n", program_name);
              fprintf(stderr, "dbg5       kind:  %d\n", kind);
              fprintf(stderr, "dbg5       beam:  %d\n", i);
              fprintf(stderr, "dbg5       xtrack: %f\n", bathacrosstrack[i]);
              fprintf(stderr, "dbg5       ltrack: %f\n", bathalongtrack[i]);
              fprintf(stderr, "dbg5       depth:  %f\n", bath[i]);
            }

            /* add heave and draft */
            depth_offset_use = bheave[i] + draft + lever_heave;
            depth_offset_org = bheave[i] + draft_org;

            /* strip off heave + draft */
            bath[i] -= depth_offset_org;

            /* get range and angles in
                roll-pitch frame */
            range = sqrt(bath[i] * bath[i] + bathacrosstrack[i] * bathacrosstrack[i] +
                         bathalongtrack[i] * bathalongtrack[i]);
            if (fabs(range) < 0.001) {
              alphar = 0.0;
              betar = 0.5 * M_PI;
            }
            else {
              alphar = asin(std::max(-1.0, std::min(1.0, (bathalongtrack[i] / range))));
              betar = acos(std::max(-1.0, std::min(1.0, (bathacrosstrack[i] / range / cos(alphar)))));
            }
            if (bath[i] < 0.0)
              betar = 2.0 * M_PI - betar;

            /* apply roll pitch corrections */
            if (process->mbp_nav_attitude == MBP_NAV_ON || process->mbp_attitude_mode == MBP_ATTITUDE_ON) {
              betar += DTR * (roll - roll_org);
              alphar += DTR * (pitch - pitch_org);
            }
            if (process->mbp_pitchbias_mode == MBP_PITCHBIAS_ON)
              alphar += DTR * process->mbp_pitchbias;
            if (process->mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
              betar += DTR * process->mbp_rollbias;
            else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE && betar <= M_PI * 0.5)
              betar += DTR * process->mbp_rollbias_stbd;
            else if (process->mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
              betar += DTR * process->mbp_rollbias_port;

            /* recalculate bathymetry */
            bath[i] = range * cos(alphar) * sin(betar);
            bathalongtrack[i] = range * sin(alphar);
            bathacrosstrack[i] = range * cos(alphar) * cos(betar);

            /* add heave and draft back in */
            bath[i] += depth_offset_use;

            /* output some debug values */
            if (verbose >= 5)
              fprintf(stderr, "dbg5       %3d beam:%3d bath:%8.2f %8.2f %8.2f\n", idata, i,
                      bathacrosstrack[i], bathalongtrack[i], bath[i]);
          }
        }
      }

      /* recalculate bathymetry by changes to transducer depth  */
      else if (process->mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET || process->mbp_tide_mode == MBP_TIDE_ON ||
               process->mbp_lever_mode == MBP_LEVER_ON || process->mbp_navadj_mode == MBP_NAVADJ_LLZ) {
        /* get draft change */
        depth_offset_change = draft - draft_org + lever_heave;

        /* loop over the beams */
        for (int i = 0; i < nbath; i++) {
          if (beamflag[i] != MB_FLAG_NULL) {
            /* apply transducer depth change to depths */
            bath[i] += depth_offset_change;

            if (verbose >= 5) {
              fprintf(stderr, "dbg5       %3d %3d %8.2f %8.2f %8.2f\n", idata, i, bathacrosstrack[i],
                      bathalongtrack[i], bath[i]);
              fprintf(stderr, "\ndbg5  Depth value calculated in program <%s>:\n", program_name);
              fprintf(stderr, "dbg5       kind:  %d\n", kind);
              fprintf(stderr, "dbg5       beam:  %d\n", i);
              fprintf(stderr, "dbg5       xtrack: %f\n", bathacrosstrack[i]);
              fprintf(stderr, "dbg5       ltrack: %f\n", bathalongtrack[i]);
              fprintf(stderr, "dbg5       depth:  %f\n", bath[i]);
            }
          }
        }
      }

      /*--------------------------------------------
        change water sound reference if needed
        --------------------------------------------*/

      /* change bathymetry water sound reference if required */
      if (process->mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF ||
          (process->mbp_svp_mode == MBP_SVP_ON && !process->mbp_corrected)) {
        for (int i = 0; i < nbath; i++) {
          if (beamflag[i] != MB_FLAG_NULL) {
            /* calculate average water sound speed
            for current depth value */
            depth_offset_use = bheave[i] + draft + lever_heave;
            zz = bath[i] - depth_offset_use;
            int k = -1;
            for (int j = 0; j < nsvp - 1; j++) {
              if ((depth[j] < zz) && (depth[j + 1] >= zz))
                k = j;
            }
            if (k > 0)
              vsum = velocity_sum[k - 1];
            else
              vsum = 0.0;
            if (k >= 0) {
              vsum += 0.5 *
                      (2 * velocity[k] +
                       (zz - depth[k]) * (velocity[k + 1] - velocity[k]) / (depth[k + 1] - depth[k])) *
                      (zz - depth[k]);
              vavg = vsum / zz;
            }
            if (vavg <= 0.0)
              vavg = 1500.0;

            /* if uncorrected value desired */
            if (!process->mbp_corrected)
              bath[i] = zz * 1500.0 / vavg + depth_offset_use;
            else
              bath[i] = zz * vavg / 1500.0 + depth_offset_use;
          }
        }
      }

      /*--------------------------------------------
        apply per-beam static offsets
        --------------------------------------------*/

      /* apply static corrections */
      if (process->mbp_static_mode == MBP_STATIC_BEAM_ON && nstatic > 0 && nstatic <= nbath) {
        for (int i = 0; i < nstatic; i++) {
          if (staticbeam[i] >= 0 && staticbeam[i] < nbath) {
            if (beamflag[staticbeam[i]] != MB_FLAG_NULL)
              bath[staticbeam[i]] -= staticoffset[i];
          }
        }
      }

      /*--------------------------------------------
        apply per-angle static offsets
        --------------------------------------------*/

      /* apply static corrections */
      if (process->mbp_static_mode == MBP_STATIC_ANGLE_ON && nstatic > 0) {
        int istatic = 0;
        mb_pr_set_bathyslope(verbose, nsmooth, nbath, beamflag, bath, bathacrosstrack, &ndepths, depths,
                             depthacrosstrack, &nslopes, slopes, slopeacrosstrack, depthsmooth, error);
        for (int i = 0; i < nbath; i++) {
          if (mb_beam_ok(beamflag[i])) {
            bathy = 0.0;
            if (ndepths > 1) {
              *status = mb_pr_get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                            slopeacrosstrack, bathacrosstrack[i], &bathy, &slope, error);
              if (bathy <= 0.0) {
                if (altitude > 0.0)
                  bathy = altitude + sensordepth;
                else
                  bathy = altitude_default + sensordepth;
                slope = 0.0;
              }
              if (bathy > 0.0) {
                altitude_use = bathy - sensordepth;
                angle = RTD * atan(bathacrosstrack[i] / altitude_use);

                /* Get offset from SBO file */
                *status = mb_linear_interp(verbose, staticangle - 1, staticoffset - 1, nstatic, angle,
                                          &correction, &istatic, error);
                bath[i] -= correction;
              }
            }
          }
        }
      }

      /* output some debug messages */
      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Depth values calculated in program <%s>:\n", program_name);
        fprintf(stderr, "dbg5       kind:  %d\n", kind);
        fprintf(stderr, "dbg5      beam    ttime      depth        xtrack    ltrack      flag\n");
        for (int i = 0; i < nbath; i++)
          fprintf(stderr, "dbg5       %2d   %f   %f   %f   %f   %d\n", i, ttimes[i], bath[i],
                  bathacrosstrack[i], bathalongtrack[i], beamflag[i]);
      }
    }

    /*--------------------------------------------
      apply beam edits
      --------------------------------------------*/

    /* apply the saved edits */
    if (process->mbp_edit_mode == MBP_EDIT_ON && esf.nedit > 0 && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      /* apply edits for this ping */
      *status = mb_esf_apply(verbose, &esf, time_d, pingmultiplicity, nbath, beamflag, error);
    }

    /*--------------------------------------------
      apply data cutting to bathymetry
      --------------------------------------------*/

    /* apply data cutting to bathymetry if specified */
    if (process->mbp_cut_num > 0 && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      for (icut = 0; icut < process->mbp_cut_num; icut++) {
        /* flag data according to beam number range */
        if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_BATH &&
            process->mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER) {
          istart = std::max((int)process->mbp_cut_min[icut], 0);
          iend = std::min((int)process->mbp_cut_max[icut], nbath - 1);
          for (int i = istart; i <= iend; i++) {
            if (mb_beam_ok(beamflag[i]))
              beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
          }
        }

        /* flag data according to beam
            acrosstrack distance */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_BATH &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE) {
          for (int i = 0; i < nbath; i++) {
            if (mb_beam_ok(beamflag[i]) && bathacrosstrack[i] >= process->mbp_cut_min[icut] &&
                bathacrosstrack[i] <= process->mbp_cut_max[icut])
              beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
          }
        }

        /* flag data according to speed */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_BATH &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_SPEED) {
          if (speed < process->mbp_cut_min[icut] || speed > process->mbp_cut_max[icut]) {
            for (int i = 0; i < nbath; i++) {
              if (mb_beam_ok(beamflag[i]))
                beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
            }
          }
        }
      }
    }

    /*--------------------------------------------
      insert data as altered so far (not done yet)
      --------------------------------------------*/

    /* insert the altered navigation if available */
    if (*error == MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA || kind == nav_source)) {
      if (heading >= 360.0)
        heading -= 360.0;
      else if (heading < 0.0)
        heading += 360.0;
      *status = mb_insert_nav(verbose, imbio_ptr, store_ptr, time_i, time_d, navlon, navlat, speed, heading, draft,
                             roll, pitch, heave, error);
    }

    /* insert the altered bathymetry, recalculate the sidescan (if that is defined),
        and extract the results if desired */
    if (process->mbp_ssrecalc_mode == MBP_SSRECALC_ON && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      *status = mb_insert(verbose, imbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading, nbath,
                         namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack,
                         ssalongtrack, comment, error);
      *status = mb_makess(verbose, imbio_ptr, store_ptr, pixel_size_set, &pixel_size, swath_width_set,
                                      &swath_width, pixel_int, error);
      *status = mb_extract(verbose, imbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &nbath, &namp, &nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                          ssacrosstrack, ssalongtrack, comment, error);
    }

    /*--------------------------------------------
      apply data cutting to amplitude and sidescan
      --------------------------------------------*/

    /* apply data cutting to sidescan and amplitude if specified */
    if (process->mbp_cut_num > 0 && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      for (icut = 0; icut < process->mbp_cut_num; icut++) {

        /* flag data according to beam number range */
        if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_AMP && process->mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER) {
          istart = std::max((int)process->mbp_cut_min[icut], 0);
          iend = std::min((int)process->mbp_cut_max[icut], namp - 1);
          for (int i = istart; i <= iend; i++) {
            if (mb_beam_ok(beamflag[i]))
              beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
          }
        }

        /* flag data according to beam
            acrosstrack distance */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_AMP &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE) {
          for (int i = 0; i < namp; i++) {
            if (mb_beam_ok(beamflag[i]) && bathacrosstrack[i] >= process->mbp_cut_min[icut] &&
                bathacrosstrack[i] <= process->mbp_cut_max[icut])
              beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
          }
        }

        /* flag data according to speed */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_AMP &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_SPEED) {
          if (speed < process->mbp_cut_min[icut] || speed > process->mbp_cut_max[icut]) {
            for (int i = 0; i < namp; i++) {
              amp[i] = 0.0;
            }
          }
        }

        /* flag data according to pixel number range */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_SS &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_NUMBER) {
          istart = std::max((int)process->mbp_cut_min[icut], 0);
          iend = std::min((int)process->mbp_cut_max[icut], nss - 1);
          for (int i = istart; i <= iend; i++) {
            ss[i] = MB_SIDESCAN_NULL;
          }
        }

        /* flag data according to pixel
            acrosstrack distance */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_SS &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_DISTANCE) {
          for (int i = 0; i < nss; i++) {
            if (ssacrosstrack[i] >= process->mbp_cut_min[icut] &&
                ssacrosstrack[i] <= process->mbp_cut_max[icut])
              ss[i] = MB_SIDESCAN_NULL;
          }
        }

        /* flag data according to speed */
        else if (process->mbp_cut_kind[icut] == MBP_CUT_DATA_SS &&
                 process->mbp_cut_mode[icut] == MBP_CUT_MODE_SPEED) {
          if (speed < process->mbp_cut_min[icut] || speed > process->mbp_cut_max[icut]) {
            for (int i = 0; i < nss; i++) {
              ss[i] = MB_SIDESCAN_NULL;
            }
          }
        }
      }
    }

    /*--------------------------------------------
      apply grazing angle corrections to amplitude and sidescan
      --------------------------------------------*/

    /* correct amplitude and sidescan using slopes from multibeam swath data */
    if ((process->mbp_ampcorr_mode == MBP_AMPCORR_ON && (process->mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE ||
                                                        process->mbp_ampcorr_slope == MBP_AMPCORR_USESLOPE)) ||
        (process->mbp_sscorr_mode == MBP_SSCORR_ON &&
         (process->mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE || process->mbp_sscorr_slope == MBP_SSCORR_USESLOPE))) {
      /* get seafloor slopes if needed for amplitude or sidescan correction */
      if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
          ((process->mbp_ampcorr_mode == MBP_AMPCORR_ON && nampcorrtable > 0 && nampcorrangle > 0) ||
           (process->mbp_sscorr_mode == MBP_SSCORR_ON && nsscorrtable > 0 && nsscorrangle > 0))) {
        mb_pr_set_bathyslope(verbose, nsmooth, nbath, beamflag, bath, bathacrosstrack, &ndepths, depths,
                             depthacrosstrack, &nslopes, slopes, slopeacrosstrack, depthsmooth, error);
      }

      /* correct the amplitude if desired */
      if (process->mbp_ampcorr_mode == MBP_AMPCORR_ON && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
          nampcorrtable > 0 && nampcorrangle > 0) {
        /* calculate the correction table */
        *status = get_corrtable(verbose, time_d, nampcorrtable, nampcorrangle, ampcorrtable, &ampcorrtableuse, error);

        /* set the reference amplitudes */
        *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle, ampcorrtableuse.amplitude,
                               (-process->mbp_ampcorr_angle), &reference_amp_port, error);
        *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle, ampcorrtableuse.amplitude,
                               process->mbp_ampcorr_angle, &reference_amp_stbd, error);
        reference_amp = 0.5 * (reference_amp_port + reference_amp_stbd);

        /* get seafloor slopes */
        for (int i = 0; i < namp; i++) {
          if (mb_beam_ok(beamflag[i])) {
            bathy = 0.0;
            if (ndepths > 1) {
              *status = mb_pr_get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                            slopeacrosstrack, bathacrosstrack[i], &bathy, &slope, error);
              if (*status != MB_SUCCESS) {
                bathy = 0.0;
                slope = 0.0;
                *status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
              }
            }
            if (bathy <= 0.0) {
              if (altitude > 0.0)
                bathy = altitude + sensordepth;
              else
                bathy = altitude_default + sensordepth;
              slope = 0.0;
            }

            if (bathy > 0.0) {
              altitude_use = bathy - sensordepth;
              angle = RTD * atan(bathacrosstrack[i] / altitude_use);
              if (process->mbp_ampcorr_slope != MBP_AMPCORR_IGNORESLOPE)
                angle += RTD * atan(slope);
              *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle,
                                     ampcorrtableuse.amplitude, angle, &correction, error);
              if (process->mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION)
                amp[i] = amp[i] - correction + reference_amp;
              else
                amp[i] = amp[i] / correction * reference_amp;
            }
          }
        }
      }

      /* correct the sidescan if desired */
      if (process->mbp_sscorr_mode == MBP_SSCORR_ON && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
          nsscorrtable > 0 && nsscorrangle > 0) {
        /* calculate the correction table */
        *status = get_corrtable(verbose, time_d, nsscorrtable, nsscorrangle, sscorrtable, &sscorrtableuse, error);

        /* set the reference amplitudes */
        *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle, sscorrtableuse.amplitude,
                               (-process->mbp_sscorr_angle), &reference_amp_port, error);
        *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle, sscorrtableuse.amplitude,
                               process->mbp_sscorr_angle, &reference_amp_stbd, error);
        reference_amp = 0.5 * (reference_amp_port + reference_amp_stbd);

        /* get seafloor slopes */
        for (int i = 0; i < nss; i++) {
          if (ss[i] > MB_SIDESCAN_NULL) {
            bathy = 0.0;
            if (ndepths > 1) {
              *status = mb_pr_get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                            slopeacrosstrack, ssacrosstrack[i], &bathy, &slope, error);
              if (*status != MB_SUCCESS) {
                bathy = 0.0;
                slope = 0.0;
                *status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
              }
            }
            if (bathy <= 0.0) {
              if (altitude > 0.0)
                bathy = altitude + sensordepth;
              else
                bathy = altitude_default + sensordepth;
              slope = 0.0;
            }

            if (bathy > 0.0) {
              altitude_use = bathy - sensordepth;
              angle = RTD * atan(ssacrosstrack[i] / altitude_use);
              if (process->mbp_sscorr_slope != MBP_SSCORR_IGNORESLOPE) {
                angle += RTD * atan(slope);
              }
              *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle,
                                     sscorrtableuse.amplitude, angle, &correction, error);
              if (process->mbp_sscorr_type == MBP_SSCORR_SUBTRACTION) {
                ss[i] = ss[i] - correction + reference_amp;
              }
              else {
                ss[i] = ss[i] / correction * reference_amp;
              }
            }
          }
        }
      }
    }

    /* correct amplitude and sidescan using slopes from topography grid */
    else if ((process->mbp_ampcorr_mode == MBP_AMPCORR_ON &&
              (process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPO ||
               process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPOSLOPE)) ||
             (process->mbp_sscorr_mode == MBP_SSCORR_ON && (process->mbp_sscorr_slope == MBP_SSCORR_USETOPO ||
                                                           process->mbp_sscorr_slope == MBP_SSCORR_USETOPOSLOPE))) {
      /* get distance scaling and heading vector */
      mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
      headingx = sin(heading * DTR);
      headingy = cos(heading * DTR);

      /* correct the amplitude if desired */
      if (process->mbp_ampcorr_mode == MBP_AMPCORR_ON && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
          nampcorrtable > 0 && nampcorrangle > 0) {
        /* calculate the correction table */
        *status = get_corrtable(verbose, time_d, nampcorrtable, nampcorrangle, ampcorrtable, &ampcorrtableuse, error);

        /* set the reference amplitudes */
        *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle, ampcorrtableuse.amplitude,
                               (-process->mbp_ampcorr_angle), &reference_amp_port, error);
        *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle, ampcorrtableuse.amplitude,
                               process->mbp_ampcorr_angle, &reference_amp_stbd, error);
        reference_amp = 0.5 * (reference_amp_port + reference_amp_stbd);

        /* get seafloor slopes */
        for (int i = 0; i < namp; i++) {
          if (mb_beam_ok(beamflag[i])) {
            /* get position in grid */
            r[0] = headingy * bathacrosstrack[i] + headingx * bathalongtrack[i];
            r[1] = -headingx * bathacrosstrack[i] + headingy * bathalongtrack[i];
            const int ix = (navlon + r[0] * mtodeglon - grid->xmin + 0.5 * grid->dx) / grid->dx;
            const int jy = (navlat + r[1] * mtodeglat - grid->ymin + 0.5 * grid->dy) / grid->dy;
            const int kgrid = ix * grid->n_rows + jy;
            const int kgrid00 = (ix - 1) * grid->n_rows + jy - 1;
            const int kgrid01 = (ix - 1) * grid->n_rows + jy + 1;
            const int kgrid10 = (ix + 1) * grid->n_rows + jy - 1;
            const int kgrid11 = (ix + 1) * grid->n_rows + jy + 1;
            if (ix > 0 && ix < grid->n_columns - 1 && jy > 0 && jy < grid->n_rows - 1 &&
                grid->data[kgrid] > grid->nodatavalue && grid->data[kgrid00] > grid->nodatavalue &&
                grid->data[kgrid01] > grid->nodatavalue && grid->data[kgrid10] > grid->nodatavalue &&
                grid->data[kgrid11] > grid->nodatavalue) {
              /* get look vector for data */
              bathy = -grid->data[kgrid];
              r[2] = grid->data[kgrid] + sensordepth;
              rr = -sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
              r[0] /= rr;
              r[1] /= rr;
              r[2] /= rr;

              /* get normal vector to grid surface */
              if (process->mbp_ampcorr_slope == MBP_SSCORR_USETOPOSLOPE) {
                v1[0] = 2.0 * grid->dx / mtodeglon;
                v1[1] = 2.0 * grid->dy / mtodeglat;
                v1[2] = grid->data[kgrid11] - grid->data[kgrid00];
                v2[0] = -2.0 * grid->dx / mtodeglon;
                v2[1] = 2.0 * grid->dy / mtodeglat;
                v2[2] = grid->data[kgrid01] - grid->data[kgrid10];
                v[0] = v1[1] * v2[2] - v2[1] * v1[2];
                v[1] = v2[0] * v1[2] - v1[0] * v2[2];
                v[2] = v1[0] * v2[1] - v2[0] * v1[1];
                vv = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
                v[0] /= vv;
                v[1] /= vv;
                v[2] /= vv;
              }
              else {
                v[0] = 0.0;
                v[1] = 0.0;
                v[2] = 1.0;
              }

              /* angle between look vector and surface normal
                  is the acos(r dot v) */
              angle = RTD * acos(r[0] * v[0] + r[1] * v[1] + r[2] * v[2]);
              if (bathacrosstrack[i] < 0.0)
                angle = -angle;

            }
            else {
              if (ix >= 0 && ix < grid->n_columns && jy >= 0 && jy < grid->n_rows && grid->data[kgrid] > grid->nodatavalue)
                bathy = -grid->data[kgrid];
              else
                bathy = bath[i];
              angle = RTD * atan(bathacrosstrack[i] / (bathy - sensordepth));
              slope = 0.0;
            }

            /* apply correction */
            *status = get_anglecorr(verbose, ampcorrtableuse.nangle, ampcorrtableuse.angle,
                                   ampcorrtableuse.amplitude, angle, &correction, error);
            if (process->mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION)
              amp[i] = amp[i] - correction + reference_amp;
            else
              amp[i] = amp[i] / correction * reference_amp;
          }
        }
      }

      /* correct the sidescan if desired */
      if (process->mbp_sscorr_mode == MBP_SSCORR_ON && *error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
          nsscorrtable > 0 && nsscorrangle > 0) {
        /* calculate the correction table */
        *status = get_corrtable(verbose, time_d, nsscorrtable, nsscorrangle, sscorrtable, &sscorrtableuse, error);

        /* set the reference amplitudes */
        *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle, sscorrtableuse.amplitude,
                               (-process->mbp_sscorr_angle), &reference_amp_port, error);
        *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle, sscorrtableuse.amplitude,
                               process->mbp_sscorr_angle, &reference_amp_stbd, error);
        reference_amp = 0.5 * (reference_amp_port + reference_amp_stbd);

        /* get seafloor slopes */
        for (int i = 0; i < nss; i++) {
          if (ss[i] > MB_SIDESCAN_NULL) {
            /* get position in grid */
            r[0] = headingy * ssacrosstrack[i] + headingx * ssalongtrack[i];
            r[1] = -headingx * ssacrosstrack[i] + headingy * ssalongtrack[i];
            const int ix = (navlon + r[0] * mtodeglon - grid->xmin + 0.5 * grid->dx) / grid->dx;
            const int jy = (navlat + r[1] * mtodeglat - grid->ymin + 0.5 * grid->dy) / grid->dy;
            const int kgrid = ix * grid->n_rows + jy;
            const int kgrid00 = (ix - 1) * grid->n_rows + jy - 1;
            const int kgrid01 = (ix - 1) * grid->n_rows + jy + 1;
            const int kgrid10 = (ix + 1) * grid->n_rows + jy - 1;
            const int kgrid11 = (ix + 1) * grid->n_rows + jy + 1;
            if (ix > 0 && ix < grid->n_columns - 1 && jy > 0 && jy < grid->n_rows - 1 &&
                grid->data[kgrid] > grid->nodatavalue && grid->data[kgrid00] > grid->nodatavalue &&
                grid->data[kgrid01] > grid->nodatavalue && grid->data[kgrid10] > grid->nodatavalue &&
                grid->data[kgrid11] > grid->nodatavalue) {
              /* get look vector for data */
              bathy = -grid->data[kgrid];
              r[2] = grid->data[kgrid] + sensordepth;
              rr = -sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
              r[0] /= rr;
              r[1] /= rr;
              r[2] /= rr;

              /* get normal vector to grid surface */
              if (process->mbp_sscorr_slope == MBP_SSCORR_USETOPOSLOPE) {
                v1[0] = 2.0 * grid->dx / mtodeglon;
                v1[1] = 2.0 * grid->dy / mtodeglat;
                v1[2] = grid->data[kgrid11] - grid->data[kgrid00];
                v2[0] = -2.0 * grid->dx / mtodeglon;
                v2[1] = 2.0 * grid->dy / mtodeglat;
                v2[2] = grid->data[kgrid01] - grid->data[kgrid10];
                v[0] = v1[1] * v2[2] - v2[1] * v1[2];
                v[1] = v2[0] * v1[2] - v1[0] * v2[2];
                v[2] = v1[0] * v2[1] - v2[0] * v1[1];
                vv = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
                v[0] /= vv;
                v[1] /= vv;
                v[2] /= vv;
              }
              else {
                v[0] = 0.0;
                v[1] = 0.0;
                v[2] = 1.0;
              }

              /* angle between look vector and surface normal
                  is the acos(r dot v) */
              angle = RTD * acos(r[0] * v[0] + r[1] * v[1] + r[2] * v[2]);
              if (ssacrosstrack[i] < 0.0)
                angle = -angle;
            }
            else {
              if (ix >= 0 && ix < grid->n_columns && jy >= 0 && jy < grid->n_rows && grid->data[kgrid] > grid->nodatavalue)
                bathy = -grid->data[kgrid];
              else if (altitude > 0.0)
                bathy = altitude + sensordepth;
              else
                bathy = altitude_default + sensordepth;
              angle = RTD * atan(bathacrosstrack[i] / (bathy - sensordepth));
              slope = 0.0;
            }

            /* apply correction */
            *status = get_anglecorr(verbose, sscorrtableuse.nangle, sscorrtableuse.angle,
                                   sscorrtableuse.amplitude, angle, &correction, error);
            if (process->mbp_sscorr_type == MBP_SSCORR_SUBTRACTION) {
              ss[i] = ss[i] - correction + reference_amp;
            }
            else {
              ss[i] = ss[i] / correction * reference_amp;
            }
          }
        }
      }
    }

    /*--------------------------------------------
      insert the altered data (now done)
      --------------------------------------------*/

    /* insert the altered data if available */
    if (*error == MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA || kind == MB_DATA_COMMENT)) {
      *status = mb_insert(verbose, imbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading, nbath,
                         namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack,
                         ssalongtrack, comment, error);
    }

    /*--------------------------------------------
      output any changed beamflags to the reverse
      edit save file (saving the change required
      to get back to the original flag state from
      the processed flag state)
      --------------------------------------------*/
    if (*error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      for (int i = 0; i < nbath; i++) {
        if (beamflag[i] != beamflagorg[i]) {
          if (mb_beam_ok(beamflagorg[i])) {
            action = MBP_EDIT_UNFLAG;
          }
          else if (mb_beam_check_flag_unusable(beamflagorg[i])) {
            action = MBP_EDIT_ZERO;
          }
          else if (mb_beam_check_flag_manual(beamflagorg[i])) {
            action = MBP_EDIT_FLAG;
          }
          else if (mb_beam_check_flag_filter(beamflagorg[i])) {
            action = MBP_EDIT_FILTER;
          }
          else if (mb_beam_check_flag_sonar(beamflagorg[i])) {
            action = MBP_EDIT_SONAR;
          }
        *status = mbprocess_save_edit(verbose, resf_fp, time_d,
                                                 i + pingmultiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                                 action, error);
        }
      }
    }

    /*--------------------------------------------
      write the processed data
      --------------------------------------------*/

    /* write some data */
    if (*error == MB_ERROR_NO_ERROR || (kind == MB_DATA_COMMENT && !process->mbp_strip_comments)) {
      *status = mb_put_all(verbose, ombio_ptr, store_ptr, false, kind, time_i, time_d, navlon, navlat, speed,
                          heading, nbath, namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                          ssacrosstrack, ssalongtrack, comment, error);
      if (*status == MB_SUCCESS) {
        if (kind == MB_DATA_DATA)
          odata++;
        else if (kind == nav_source)
          onav++;
        else if (kind == MB_DATA_COMMENT)
          ocomment++;
        else
          oother++;
      }
      else {
        char *message = nullptr;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error returned from function <mb_put>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam Data Not Written To File <%s>\n", process->mbp_ofile);
        fprintf(stderr, "Output Record: %d\n", odata + 1);
        fprintf(stderr, "Time: %4d %2d %2d %2d:%2d:%2d.%6d\n", time_i[0], time_i[1], time_i[2], time_i[3],
                time_i[4], time_i[5], time_i[6]);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }

      if (*status == MB_SUCCESS && kind == MB_DATA_DATA) {

        /* output fbt */
        if (make_fbt) {
          fstore->sensorhead = sensorhead;
          fstore->topo_type = sensortype;
          struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
          fstore->beam_xwidth = imb_io_ptr->beamwidth_xtrack;
          fstore->beam_lwidth = imb_io_ptr->beamwidth_ltrack;
          fstore->kind = kind;
          mb_insert_nav(verbose, fmbio_ptr, fstore_ptr, time_i, time_d,
                        navlon, navlat, speed, heading, draft,
                        roll, pitch, heave, error);
          mb_insert_altitude(verbose, fmbio_ptr, fstore_ptr, draft, altitude, error);
          *status = mb_insert(verbose, fmbio_ptr, fstore_ptr, kind, time_i, time_d,
                              navlon, navlat, speed, heading, nbath, namp, nss,
                              beamflag, bath, amp, bathacrosstrack, bathalongtrack,
                              ss, ssacrosstrack, ssalongtrack, comment, error);
          *status = mb_put_all(verbose, fmbio_ptr, fstore_ptr, false,
                              kind, time_i, time_d, navlon, navlat, speed,
                              heading, nbath, 0, 0,
                              beamflag, bath, nullptr, bathacrosstrack, bathalongtrack,
                              nullptr, nullptr, nullptr, comment, error);
        }

        // get scaling for both fnv and inf calculations
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        headingx = sin(heading * DTR);
        headingy = cos(heading * DTR);

        /* output fnv */
        /* mblist output: tMXYHScRPr=X=Y+X+Y */
        if (make_fnv) {
          double seconds = time_i[5] + 1e-6 * time_i[6];
          int beam_port, beam_vertical, beam_stbd;
          int pixel_port, pixel_vertical, pixel_stbd;
          *status = mb_swathbounds(verbose, true, nbath, 0,
                              beamflag, bathacrosstrack, nullptr, nullptr,
                              &beam_port, &beam_vertical, &beam_stbd,
                              &pixel_port, &pixel_vertical, &pixel_stbd, error);
          double portlon = navlon
                            + headingy * mtodeglon * bathacrosstrack[beam_port]
                            + headingx * mtodeglon * bathalongtrack[beam_port];
          double portlat = navlat
                            - headingx * mtodeglat * bathacrosstrack[beam_port]
                            + headingy * mtodeglat * bathalongtrack[beam_port];
          double stbdlon = navlon
                            + headingy * mtodeglon * bathacrosstrack[beam_stbd]
                            + headingx * mtodeglon * bathalongtrack[beam_stbd];
          double stbdlat = navlat
                            - headingx * mtodeglat * bathacrosstrack[beam_stbd]
                            + headingy * mtodeglat * bathalongtrack[beam_stbd];

          fprintf(nfp, "%.4d %.2d %.2d %.2d %.2d %09.6f\t%.6f\t"
                        "%15.10f\t%15.10f\t%7.3f\t%6.3f\t%.4f\t%6.3f\t%6.3f\t%7.4f\t"
                        "%15.10f\t%15.10f\t%15.10f\t%15.10f\n",
                  time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds,
                  time_d, navlon, navlat, heading, speed, draft, roll, pitch, heave,
                  portlon, portlat, stbdlon, stbdlat);
        }

        /* get bounds for mbinfo call to generate the *.inf file
            - use only data with good navigation and valid soundings or pixels */
        if (fabs(navlon) >= 0.005 || fabs(navlat) >= 0.005) {
          if (mask_bounds_init) {
            mask_bounds[0] = std::min(mask_bounds[0], navlon);
            mask_bounds[1] = std::max(mask_bounds[1], navlon);
            mask_bounds[2] = std::min(mask_bounds[2], navlat);
            mask_bounds[3] = std::max(mask_bounds[3], navlat);
          } else {
            mask_bounds[0] = navlon;
            mask_bounds[1] = navlon;
            mask_bounds[2] = navlat;
            mask_bounds[3] = navlat;
            mask_bounds_init = true;
          }
          for (int i=0; i<nbath; i++) {
            if (mb_beam_ok(beamflag[i])) {
              double bathlon = navlon
                          + headingy * mtodeglon * bathacrosstrack[i]
                          + headingx * mtodeglon * bathalongtrack[i];
              double bathlat = navlat
                          - headingx * mtodeglat * bathacrosstrack[i]
                          + headingy * mtodeglat * bathalongtrack[i];

              mask_bounds[0] = std::min(mask_bounds[0], bathlon);
              mask_bounds[1] = std::max(mask_bounds[1], bathlon);
              mask_bounds[2] = std::min(mask_bounds[2], bathlat);
              mask_bounds[3] = std::max(mask_bounds[3], bathlat);
            }
          }
          for (int i=0; i<nss; i++) {
            if (ss[i] > MB_SIDESCAN_NULL) {
              double sslon = navlon
                          + headingy * mtodeglon * ssacrosstrack[i]
                          + headingx * mtodeglon * ssalongtrack[i];
              double sslat = navlat
                          - headingx * mtodeglat * ssacrosstrack[i]
                          + headingy * mtodeglat * ssalongtrack[i];
              mask_bounds[0] = std::min(mask_bounds[0], sslon);
              mask_bounds[1] = std::max(mask_bounds[1], sslon);
              mask_bounds[2] = std::min(mask_bounds[2], sslat);
              mask_bounds[3] = std::max(mask_bounds[3], sslat);
            }
          }
        }

      }
    }
  }

  /* output beam flagging success info */
  neditnull = 0;
  neditduplicate = 0;
  neditnotused = 0;
  neditused = 0;
  if (process->mbp_edit_mode == MBP_EDIT_ON) {
    for (int i = 0; i < esf.nedit; i++) {
      if (esf.edit[i].use == 1000) {
        neditnull++;
        if (verbose >= 2)
          fprintf(stderr, "BEAM FLAG TIED TO NULL BEAM: i:%d edit: %f %d %d   %d\n", i, esf.edit[i].time_d,
              esf.edit[i].beam, esf.edit[i].action, esf.edit[i].use);
      } else if (esf.edit[i].use == 100) {
        neditduplicate++;
        if (verbose >= 2)
          fprintf(stderr, "DUPLICATE BEAM FLAG:         i:%d edit: %f %d %d   %d\n", i, esf.edit[i].time_d,
              esf.edit[i].beam, esf.edit[i].action, esf.edit[i].use);
      } else if (esf.edit[i].use != 1) {
        neditnotused++;
        if (verbose >= 2)
          fprintf(stderr, "BEAM FLAG NOT USED:          i:%d edit: %f %d %d   %d\n", i, esf.edit[i].time_d,
              esf.edit[i].beam, esf.edit[i].action, esf.edit[i].use);
      } else /* if (esf.edit[i].use == 1) */ {
        neditused++;
        if (verbose >= 2)
          fprintf(stderr, "BEAM FLAG USED:              i:%d edit: %f %d %d   %d\n", i, esf.edit[i].time_d,
              esf.edit[i].beam, esf.edit[i].action, esf.edit[i].use);
      }
    }
  }
  if (verbose >= 1) {
    fprintf(stderr, "          %d flags used\n", neditused);
    fprintf(stderr, "          %d flags not used\n", neditnotused);
    fprintf(stderr, "          %d flags tied to null beams\n", neditnull);
    fprintf(stderr, "          %d duplicate flags\n", neditduplicate);
  }

  /*--------------------------------------------
    reset status/error, close files and deallocate memory
    --------------------------------------------*/
  if (*status == MB_FAILURE && *error != MB_ERROR_EOF) {
    char *message = nullptr;
    mb_error(verbose, *error, &message);
    fprintf(stderr, "WARNING: exited read loop with error[%d]: %s\n", *error, message);
  }
  *status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  // close the input raw file
  *status = mb_close(verbose, &ombio_ptr, error);

  // close the output processed file
  *status = mb_close(verbose, &imbio_ptr, error);

  // close the output fbt file
  if (make_fbt)
    *status = mb_close(verbose, &fmbio_ptr, error);

  //close the output fnv file
  if (make_fnv)
    fclose(nfp);

  // use mbinfo to generate the inf file - specify the mask bounds so that
  // only one read pass is necessary
  mb_command command;
  snprintf(command, sizeof(command), "mbinfo -F %d -I %s -G -N -O -M10/10/%.9f/%.9f/%.9f/%.9f",
          process->mbp_format, process->mbp_ofile,
          mask_bounds[0], mask_bounds[1], mask_bounds[2], mask_bounds[3]);
  system(command);

  // close the *.resf file
  if (resf_fp != nullptr) {
    fclose(resf_fp);
  }

  /* deallocate arrays for amplitude correction tables */
  if (nampcorrtable > 0) {
    *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtableuse.angle), error);
    *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtableuse.amplitude), error);
    *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtableuse.sigma), error);
    for (int i = 0; i < nampcorrtable; i++) {
      *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtable[i].angle), error);
      *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtable[i].amplitude), error);
      *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(ampcorrtable[i].sigma), error);
    }
    *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&ampcorrtable, error);
  }

  /* deallocate arrays for sidescan correction tables */
  if (nsscorrtable > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtableuse.angle), error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtableuse.amplitude), error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtableuse.sigma), error);
    for (int i = 0; i < nsscorrtable; i++) {
      mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtable[i].angle), error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtable[i].amplitude), error);
      mb_freed(verbose, __FILE__, __LINE__, (void **)&(sscorrtable[i].sigma), error);
    }
    *status = mb_freed(verbose, __FILE__, __LINE__, (void **)&sscorrtable, error);
  }

  /* deallocate arrays for navigation */
  if (nnav > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ntime, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nlon, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nlat, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nheading, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nspeed, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&ndraft, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nroll, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&npitch, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nheave, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nlonspl, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nlatspl, error);
  }

  /* deallocate arrays for adjusted navigation */
  if (nanav > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&natime, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nalon, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nalat, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&naz, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nalonspl, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nalatspl, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&nazspl, error);
  }

  /* deallocate arrays for attitude merging */
  if (nattitude > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&attitudetime, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&attituderoll, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&attitudepitch, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&attitudeheave, error);
  }

  /* deallocate arrays for sensordepth merging */
  if (nsensordepth > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&fsensordepthtime, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&fsensordepth, error);
  }

  /* deallocate arrays for tide */
  if (ntide > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&tidetime, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&tide, error);
  }

  /* deallocate arrays for beam edits */
  if (process->mbp_edit_mode == MBP_EDIT_ON) {
    mb_esf_close(verbose, &esf, error);
  }

  /* deallocate memory for svp arrays and raytracing */
  if (process->mbp_svp_mode != MBP_SVP_OFF) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&depth, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&velocity, error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&velocity_sum, error);
    if (rt_svp != nullptr)
      *status = mb_rt_deall(verbose, &rt_svp, error);
  }

  /* check memory */
  if (verbose >= 4)
    *status = mb_memory_list(verbose, error);

  /* give the statistics */
  if (verbose >= 1) {
    fprintf(stderr, "\n%d input data records\n", idata);
    fprintf(stderr, "%d input nav records\n", inav);
    fprintf(stderr, "%d input comment records\n", icomment);
    fprintf(stderr, "%d input other records\n", iother);
    fprintf(stderr, "%d output data records\n", odata);
    fprintf(stderr, "%d output nav records\n", onav);
    fprintf(stderr, "%d output comment records\n", ocomment);
    fprintf(stderr, "%d output other records\n", oother);
  }

}
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  constexpr char usage_message[] =
      "mbprocess -Iinfile [-C -Fformat -N -Ooutfile -P -S -T -V -H]";

  int verbose = 0;
  int status = MB_SUCCESS;
  int error = MB_ERROR_NO_ERROR;
  int mbp_format;

  bool uselockfiles;
  status = mb_uselockfiles(verbose, &uselockfiles);

  /* set default input and output */
  bool mbp_ifile_specified = false;
  char mbp_ifile[MBP_FILENAMESIZE] = "";
  char mbp_pfile[MBP_FILENAMESIZE+10] = "";

  bool mbp_ofile_specified = false;
  char mbp_ofile[MBP_FILENAMESIZE] = "";
  bool mbp_format_specified = false;
  bool strip_comments = false;
  int format = 0;
  char read_file[MB_PATH_MAXLINE];
  bool checkuptodate = true;
  bool printfilestatus = false;
  bool testonly = false;

  unsigned int n_threads = 1;

  /* disable keeping a list of allocated memory because the memory list
      functionality in mb_mem.c is not thread safe */
  // TODO: add mutex locking/unlocking to allow testing with memory list
  //       enabled - we will still want it disabled most of the time for
  //       performance, but it would be good to be able to check for memory
  //       leaks again.  DWC December 27 2020
  mb_mem_list_disable(verbose, &error);

  /* process argument list */
  {
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt(argc, argv, "VvHhC:c:F:f:I:i:NnO:o:PpSsTt")) != -1)
      switch (c) {
      case 'H':
      case 'h':
        help = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        break;
      case 'C':
      case 'c':
        sscanf(optarg, "%d", &n_threads);
        break;
      case 'F':
      case 'f':
        sscanf(optarg, "%d", &format);
        mbp_format_specified = true;
        break;
      case 'I':
      case 'i':
        mbp_ifile_specified = true;
        sscanf(optarg, "%1023s", read_file);
        break;
      case 'N':
      case 'n':
        strip_comments = true;
        break;
      case 'O':
      case 'o':
        mbp_ofile_specified = true;
        sscanf(optarg, "%1023s", mbp_ofile);
        break;
      case 'P':
      case 'p':
        checkuptodate = false;
        break;
      case 'S':
      case 's':
        printfilestatus = true;
        break;
      case 'T':
      case 't':
        testonly = true;
        break;
      case '?':
        errflg = true;
      }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (help) {
      fprintf(stderr, "\nProgram %s\n", program_name);
      fprintf(stderr, "MB-System Version %s\n", MB_VERSION);
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  /* try datalist.mb-1 as input */
  struct stat file_status;
  if (!mbp_ifile_specified) {
    const int fstat = stat("datalist.mb-1", &file_status);
    if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
      strcpy(read_file, "datalist.mb-1");
      mbp_ifile_specified = true;
    }
  }

  /* quit if no input file specified */
  if (!mbp_ifile_specified) {
    fprintf(stderr, "\nProgram <%s> requires an input data file.\n", program_name);
    fprintf(stderr, "The input file may be specified with the -I option.\n");
    fprintf(stderr, "The default input file is \"datalist.mb-1\".\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_OPEN_FAIL);
  }

  /* get format if required */
  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data = false;

  /* open file list */
  void *datalist;
  char mbp_dfile[MBP_FILENAMESIZE];
  double file_weight;
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_NO;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = (mb_datalist_read(verbose, datalist, mbp_ifile, mbp_dfile, &mbp_format, &file_weight, &error) == MB_SUCCESS);
  } else {
    // else copy single filename to be read
    strcpy(mbp_ifile, read_file);
    mbp_format = format;
    read_data = true;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "\ndbg2  MB-System Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       read_file:       %s\n", read_file);
    fprintf(stderr, "dbg2       format:          %d\n", format);
    fprintf(stderr, "dbg2       strip_comments:  %d\n", strip_comments);
    fprintf(stderr, "dbg2       checkuptodate:   %d\n", checkuptodate);
    fprintf(stderr, "dbg2       printfilestatus: %d\n", printfilestatus);
    fprintf(stderr, "dbg2       testonly:        %d\n", testonly);
    fprintf(stderr, "dbg2       n_threads:       %d\n", n_threads);
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
  }

  else if (verbose > 0) {
    fprintf(stderr, "\nProgram <%s>\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "\nProgram Operation:\n");
    fprintf(stderr, "  Input file:      %s\n", read_file);
    fprintf(stderr, "  Format:          %d\n", format);
    if (checkuptodate)
      fprintf(stderr, "  Files processed only if out of date.\n");
    else
      fprintf(stderr, "  All files processed.\n");
    if (!strip_comments)
      fprintf(stderr, "  Comments embedded in output.\n\n");
    else
      fprintf(stderr, "  Comments stripped from output.\n\n");
    fprintf(stderr, "  Using %d threads\n\n", n_threads);
  }

  /* swath file locking variables */
  int lock_status;
  int lock_error;
  int lock_purpose = MBP_LOCK_NONE;
  mb_path lock_program;
  mb_path lock_cpu;
  mb_path lock_user;
  mb_path lock_date;
  bool proceedprocess = false;
  char str_process_yes[] = "**: Data processed";
  char str_process_no[] = "--: Data not processed";
  char str_process_yes_test[] = "Data processed (test-only mode)";
  char str_process_no_test[] = "Data not processed (test-only mode)";
  char str_outofdate_yes[] = "out of date";
  char str_outofdate_overridden[] = "up to date but overridden";
  char str_outofdate_no[] = "up to date";
  char str_locked_yes[] = "locked";
  char str_locked_ignored[] = "locked but lock ignored";
  char str_locked_fail[] = "unlocked but set lock failed";
  char str_locked_no[] = "unlocked";
  bool locked;

  /* get number of threads to use */
  unsigned int n_concurrency = std::thread::hardware_concurrency();
  n_threads = MIN(n_threads, MIN(n_concurrency, MB_THREAD_MAX));
  unsigned int n_thread_set = 0;
  std::thread mbprocessThreads[MB_THREAD_MAX];
  int thread_status[MB_THREAD_MAX];
  int thread_error[MB_THREAD_MAX];

  /* parameter controls */
  struct mb_process_struct processPars[MB_THREAD_MAX];

  /* topography grids for backscatter correction */
  struct mbprocess_grid_struct grids[MB_PR_TOPOGRID_NUM_MAX];
  bool grids_read[MB_PR_TOPOGRID_NUM_MAX];
  memset(grids_read, 0, sizeof(bool) * MB_PR_TOPOGRID_NUM_MAX);
  unsigned int grids_countSinceUsed[MB_PR_TOPOGRID_NUM_MAX];
  memset(grids_countSinceUsed, 0, sizeof(unsigned int) * MB_PR_TOPOGRID_NUM_MAX);

  /* loop over all files to be read */
  while (read_data) {
    /* load parameters */
    struct mb_process_struct *process = &processPars[n_thread_set];
    status = mb_pr_readpar(verbose, mbp_ifile, false, process, &error);

    /* set strip_comments */
    process->mbp_strip_comments = strip_comments;

    /* reset output file and format if not reading from datalist */
    if (!read_datalist) {
      if (mbp_ofile_specified) {
        strcpy(process->mbp_ofile, mbp_ofile);
      }
      if (mbp_format_specified) {
        process->mbp_format = mbp_format;
      }
    }

    /* make output file path global if needed */
    int len;
    if (status == MB_SUCCESS && !mbp_ofile_specified && process->mbp_ofile[0] != '/' && process->mbp_ofile[1] != ':' &&
        strrchr(process->mbp_ifile, '/') != nullptr && (len = strrchr(process->mbp_ifile, '/') - process->mbp_ifile + 1) > 1) {
      strcpy(mbp_ofile, process->mbp_ofile);
      strncpy(process->mbp_ofile, process->mbp_ifile, len);
      process->mbp_ofile[len] = '\0';
      strcat(process->mbp_ofile, mbp_ofile);
    }

    /* get mod time for the input file */
    int ifilemodtime = 0;
    int fstat = stat(mbp_ifile, &file_status);
    if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
      ifilemodtime = file_status.st_mtime;
    }

    /* check for existing parameter file */
    int pfilemodtime = 0;
    snprintf(mbp_pfile, sizeof(mbp_pfile), "%s.par", mbp_ifile);
    if ((fstat = stat(mbp_pfile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
      pfilemodtime = file_status.st_mtime;
    }

    /* skip if processing cannot be inferred */
    if (status == MB_FAILURE) {
      proceedprocess = false;
      if (verbose > 0 || testonly)
        fprintf(stderr, "Data skipped - processing unknown: %s\n", mbp_ifile);
    }

    /* skip if input file can't be read */
    else if (ifilemodtime == 0) {
      proceedprocess = false;
      if (verbose > 0 || testonly)
        fprintf(stderr, "Data skipped - input file cannot be read: %s\n", mbp_ifile);
    }

    /* skip if parameter file can't be read */
    else if (pfilemodtime == 0) {
      proceedprocess = false;
      if (verbose > 0 || testonly)
        fprintf(stderr, "Data skipped - parameter file cannot be read: %s\n", mbp_pfile);
    }
    /* check for up to date */
    else {
      /* get mod time for the output file */
      const int ofilemodtime =
          stat(process->mbp_ofile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the navigation file if needed */
      const int navfilemodtime =
          process->mbp_nav_mode != MBP_NAV_OFF &&
          stat(process->mbp_navfile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the navigation adjustment file if needed */
      const int navadjfilemodtime =
          process->mbp_navadj_mode != MBP_NAVADJ_OFF &&
          stat(process->mbp_navadjfile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the attitude file if needed */
      const int attitudefilemodtime =
          process->mbp_attitude_mode != MBP_ATTITUDE_OFF &&
          stat(process->mbp_attitudefile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the sensordepth file if needed */
      const int sensordepthfilemodtime =
          process->mbp_sensordepth_mode != MBP_SENSORDEPTH_OFF &&
          stat(process->mbp_sensordepthfile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the edit save file if needed */
      const int esfmodtime =
          process->mbp_edit_mode != MBP_EDIT_OFF &&
          stat(process->mbp_editfile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* get mod time for the svp file if needed */
      const int svpmodtime =
          process->mbp_svp_mode != MBP_SVP_OFF &&
          stat(process->mbp_svpfile, &file_status) == 0 &&
          (file_status.st_mode & S_IFMT) != S_IFDIR
          ? file_status.st_mtime : 0;

      /* now check if processed file is out of date */
      const bool outofdate =
          !(ofilemodtime > 0 && ofilemodtime >= ifilemodtime && ofilemodtime >= pfilemodtime &&
            ofilemodtime >= navfilemodtime && ofilemodtime >= navadjfilemodtime && ofilemodtime >= attitudefilemodtime &&
            ofilemodtime >= sensordepthfilemodtime && ofilemodtime >= esfmodtime && ofilemodtime >= svpmodtime);

      /* deal with information */
      if (outofdate || !checkuptodate) {
        /* not testing - do it for real */
        if (!testonly) {
          /* want to process, now try to set a lock of the file to be processed */
          if (uselockfiles) {
            lock_status = mb_pr_lockswathfile(verbose, process->mbp_ifile,
                                              MBP_LOCK_PROCESS, program_name,
                                              &lock_error);
            if (lock_status == MB_SUCCESS) {
              proceedprocess = true;
              locked = false;
            }
            else if (lock_error == MB_ERROR_FILE_LOCKED) {
              proceedprocess = false;
              mb_pr_lockinfo(verbose, process->mbp_ifile, &locked, &lock_purpose, lock_program,
                                           lock_user, lock_cpu, lock_date, &lock_error);
            }
            else if (lock_error == MB_ERROR_OPEN_FAIL) {
              proceedprocess = false;
              locked = false;
            }
          }

          /* want to process, but lock files are disabled */
          else {
            mb_pr_lockinfo(verbose, process->mbp_ifile, &locked, &lock_purpose, lock_program, lock_user,
                                         lock_cpu, lock_date, &lock_error);
            proceedprocess = true;
          }
        }

        /* else only testing */
        else {
          /* want to process, check lock status of the file to be processed */
          mb_pr_lockinfo(verbose, process->mbp_ifile, &locked, &lock_purpose, lock_program, lock_user,
                                       lock_cpu, lock_date, &lock_error);
          if (!locked || !uselockfiles) {
            proceedprocess = true;
          }
          else {
            proceedprocess = false;
          }
        }
      }
      else {
        proceedprocess = false;
        mb_pr_lockinfo(verbose, process->mbp_ifile, &locked, &lock_purpose,
                       lock_program, lock_user, lock_cpu, lock_date,
                       &lock_error);
      }

      /* write out information */
      char *string1, *string2, *string3;
      char dummy[MBP_FILENAMESIZE] = "";
      if (testonly) {
        if (proceedprocess)
          string1 = str_process_yes_test;
        else
          string1 = str_process_no_test;
      }
      else {
        if (proceedprocess)
          string1 = str_process_yes;
        else
          string1 = str_process_no;
      }
      if (outofdate)
        string2 = str_outofdate_yes;
      else if (!outofdate && !checkuptodate)
        string2 = str_outofdate_overridden;
      else
        string2 = str_outofdate_no;
      if (locked && !uselockfiles)
        string3 = str_locked_ignored;
      else if (locked)
        string3 = str_locked_yes;
      else if (!locked && lock_error == MB_ERROR_OPEN_FAIL)
        string3 = str_locked_fail;
      else
        string3 = str_locked_no;
      fprintf(stderr, "%s - %s - %s: \n\tInput:  %s\n\tOutput: %s\n", string1, string2, string3, process->mbp_ifile,
              process->mbp_ofile);
      if (locked)
        fprintf(stderr, "\tLocked by program <%s> run by <%s> on <%s> at <%s>\n", lock_program, lock_user, lock_cpu,
                lock_date);
      if (testonly || verbose > 0 || printfilestatus) {
        if (outofdate)
          fprintf(stderr, "\tFile Status: out of date\n");
        else
          fprintf(stderr, "\tFile Status: up to date\n");
        fprintf(stderr, "\t\tModification times and ages relative to the output file in seconds:\n");
        mb_get_date_string(verbose, (double)ifilemodtime, dummy);
        fprintf(stderr, "\t\t\tInput file:                 %s %12d <%s>\n", dummy, ofilemodtime - ifilemodtime,
                mbp_ifile);
        if (pfilemodtime > 0) {
          mb_get_date_string(verbose, (double)pfilemodtime, dummy);
          fprintf(stderr, "\t\t\tParameter file:             %s %12d <%s>\n", dummy, ofilemodtime - pfilemodtime,
                  mbp_pfile);
        }
        else {
          fprintf(stderr, "\t\t\tParameter file:             None\n");
        }
        if (navfilemodtime > 0) {
          mb_get_date_string(verbose, (double)navfilemodtime, dummy);
          fprintf(stderr, "\t\t\tNavigation file:            %s %12d <%s>\n", dummy, ofilemodtime - navfilemodtime,
                  process->mbp_navfile);
        }
        else {
          fprintf(stderr, "\t\t\tNavigation file:            None\n");
        }
        if (navadjfilemodtime > 0) {
          mb_get_date_string(verbose, (double)navadjfilemodtime, dummy);
          fprintf(stderr, "\t\t\tNavigation adjustment file: %s %12d <%s>\n", dummy, ofilemodtime - navadjfilemodtime,
                  process->mbp_navadjfile);
        }
        else {
          fprintf(stderr, "\t\t\tNavigation adjustment file: None\n");
        }
        if (attitudefilemodtime > 0) {
          mb_get_date_string(verbose, (double)attitudefilemodtime, dummy);
          fprintf(stderr, "\t\t\tSonar depth file:           %s %12d <%s>\n", dummy, ofilemodtime - attitudefilemodtime,
                  process->mbp_attitudefile);
        }
        else {
          fprintf(stderr, "\t\t\tSonar depth file:           None\n");
        }
        if (sensordepthfilemodtime > 0) {
          mb_get_date_string(verbose, (double)sensordepthfilemodtime, dummy);
          fprintf(stderr, "\t\t\tAttitude file:              %s %12d <%s>\n", dummy,
                  ofilemodtime - sensordepthfilemodtime, process->mbp_sensordepthfile);
        }
        else {
          fprintf(stderr, "\t\t\tAttitude file:              None\n");
        }
        if (esfmodtime > 0) {
          mb_get_date_string(verbose, (double)esfmodtime, dummy);
          fprintf(stderr, "\t\t\tEdit save file:             %s %12d <%s>\n", dummy, ofilemodtime - esfmodtime,
                  process->mbp_editfile);
        }
        else {
          fprintf(stderr, "\t\t\tEdit save file:             None\n");
        }
        if (svpmodtime > 0) {
          mb_get_date_string(verbose, (double)svpmodtime, dummy);
          fprintf(stderr, "\t\t\tSVP file:                   %s %12d <%s>\n", dummy, ofilemodtime - svpmodtime,
                  process->mbp_svpfile);
        }
        else {
          fprintf(stderr, "\t\t\tSVP file:                   None\n");
        }
        if (ofilemodtime > 0) {
          mb_get_date_string(verbose, (double)ofilemodtime, dummy);
          fprintf(stderr, "\t\t\tOutput file:                %s              <%s>\n", dummy, process->mbp_ofile);
        }
        else {
          fprintf(stderr, "\t\t\tOutput file:                None\n");
        }
      }

      /* reset proceedprocess if only testing */
      if (testonly)
        proceedprocess = false;
    }

    /* now start the processing thread for the input file */
    if (proceedprocess) {

      // if needed read in the specified topography grid for backscatter correction
      // - if this has already been read in then use the existing structure
      struct mbprocess_grid_struct *grid_use = nullptr;
      if ((process->mbp_ampcorr_mode == MBP_AMPCORR_ON &&
           (process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPO || process->mbp_ampcorr_slope == MBP_AMPCORR_USETOPOSLOPE)) ||
          (process->mbp_sscorr_mode == MBP_SSCORR_ON &&
           (process->mbp_sscorr_slope == MBP_SSCORR_USETOPO || process->mbp_sscorr_slope == MBP_SSCORR_USETOPOSLOPE))) {

        // Check if this grid has already been read
        bool found = false;
        for (int i = 0; i < MB_PR_TOPOGRID_NUM_MAX; i++) {
          if (grids_read[i]) {
            if (strcmp(process->mbp_ampsscorr_topofile, grids[i].file) == 0) {
              found = true;
              grid_use = &grids[i];
              grids_countSinceUsed[i] = 0;
            } else {
              grids_countSinceUsed[i]++;
            }
          }
        }

        // Delete any grids in memory that haven't been used recently
        for (int i = 0; i < MB_PR_TOPOGRID_NUM_MAX; i++) {
          if (grids_read[i] && grids_countSinceUsed[i] > MB_PR_TOPOGRID_NONUSE_MAX) {
            mb_freed(verbose, __FILE__, __LINE__, (void **)&grids[i].data, &error);
            memset(&grids[i], 0, sizeof(struct mbprocess_grid_struct));
            grids_read[i] = false;
            grids_countSinceUsed[i] = 0;
          }
        }

        // If necessary read new grid
        if (!found) {
          // find the first available grid slot or delete a grid to make room
          int igrid_use = -1;
          int igrid_delete = -1;
          int largest_count_since_used = -1;
          for (int i = 0; i < MB_PR_TOPOGRID_NUM_MAX && igrid_use == -1; i++) {
            if (!grids_read[i]) {
              igrid_use = i;
            } else {
              if (grids_countSinceUsed[i] > largest_count_since_used) {
                largest_count_since_used = grids_countSinceUsed[i];
                igrid_delete = i;
              }
            }
          }
          if (igrid_use < 0 && igrid_delete >= 0) {
            mb_freed(verbose, __FILE__, __LINE__, (void **)&grids[igrid_delete].data, &error);
            memset(&grids[igrid_delete], 0, sizeof(struct mbprocess_grid_struct));
            grids_read[igrid_delete] = false;
            grids_countSinceUsed[igrid_delete] = 0;
            igrid_use = igrid_delete;
          }

          // read the grid
          if (igrid_use >= 0) {
            grids[igrid_use].data = nullptr;
            strcpy(grids[igrid_use].file, process->mbp_ampsscorr_topofile);
            status = mb_read_gmt_grd(verbose, grids[igrid_use].file, &grids[igrid_use].projection_mode,
                                    grids[igrid_use].projection_id, &grids[igrid_use].nodatavalue,
                                    &grids[igrid_use].nxy, &grids[igrid_use].n_columns, &grids[igrid_use].n_rows,
                                    &grids[igrid_use].min, &grids[igrid_use].max,
                                    &grids[igrid_use].xmin, &grids[igrid_use].xmax,
                                    &grids[igrid_use].ymin, &grids[igrid_use].ymax,
                                    &grids[igrid_use].dx, &grids[igrid_use].dy,
                                    &grids[igrid_use].data, nullptr, nullptr, &error);
            if (status == MB_SUCCESS) {
              grids_read[igrid_use] = true;
              grids_countSinceUsed[igrid_use] = 0;
              grid_use = &grids[igrid_use];
            } else {
              fprintf(stderr, "\nUnable to read topography grid file: %s\n", grids[igrid_use].file);
              fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
              exit(MB_ERROR_OPEN_FAIL);
            }
          } else {
            fprintf(stderr, "\nUnable to clear memory to read topography grid file: %s\n", process->mbp_ampsscorr_topofile);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(MB_ERROR_OPEN_FAIL);
          }
        }
      }

      // Start next processing thread
      thread_status[n_thread_set] = MB_SUCCESS;
      thread_error[n_thread_set] = MB_ERROR_NO_ERROR;
      mbprocessThreads[n_thread_set]
          = std::thread(process_file, verbose, n_thread_set, &processPars[n_thread_set], grid_use,
                        &thread_status[n_thread_set], &thread_error[n_thread_set]);
      n_thread_set++;

    } /* end starting processing thread */

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = mb_datalist_read(verbose, datalist, mbp_ifile, mbp_dfile, &format, &file_weight, &error) == MB_SUCCESS;
    } else {
      read_data = false;
    }

    /* if the full number of processing threads have been started or there are no
       more files to process, join all of the existing threads in turn until all are completed */
    if (n_thread_set == n_threads || (!read_data && n_thread_set > 0)) {
      for (unsigned int ithread = 0; ithread < n_thread_set; ithread++) {
        /* join the thread (wait until it completes) */
        mbprocessThreads[ithread].join();

        // unlock the raw swath file
        if (uselockfiles) {
          thread_status[ithread] = mb_pr_unlockswathfile(verbose, processPars[ithread].mbp_ifile, MBP_LOCK_PROCESS,
                                program_name, &thread_error[ithread]);
        }
      }
      n_thread_set = 0;
    }

  } /* end loop over datalist */

  /* release any grids still in memory */
  for (int i = 0; i < MB_PR_TOPOGRID_NUM_MAX; i++) {
    if (grids_read[i]) {
      mb_freed(verbose, __FILE__, __LINE__, (void **)&grids[i].data, &error);
      memset(&grids[i], 0, sizeof(struct mbprocess_grid_struct));
      grids_read[i] = false;
      grids_countSinceUsed[i] = 0;
    }
  }

  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
