/*--------------------------------------------------------------------
 *    The MB-system:  mbvelocitytool-qt.cc  2/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBVELOCITYTOOL-QT is the Qt6 port of the interactive water velocity profile
 * editor MBVELOCITYTOOL. It also shows the profiles stored in the swath file, and
 * any display profile can become the edit profile. Without a window, --dump loads
 * a swath file and the profiles, ray traces the data through the edit profile and
 * prints the beam residuals; the --save options write what the File menu saves;
 * and --export writes an image of the plots.
 *
 * Author:  C. dos Santos Ferreira
 * Date:    November 2, 2026
 */

#include <getopt.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <QApplication>
#include <QString>

#include "MainWindow.h"
#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbvt_core.h"

constexpr char help_message[] =
    "MBVELOCITYTOOL-QT is an interactive water velocity profile editor used to examine\n"
    "water velocity profiles and to create new ones for the processing of swath sonar data.\n"
    "Without a window, --dump loads the data, ray traces it through the edit profile and\n"
    "prints the beam residuals, the --save options write the output files, and --export\n"
    "writes an image of the plots.";
constexpr char usage_message[] =
    "mbvelocitytool-qt [--input=file -Ifile --format=format -Fformat\n"
    "           --begin-time=yr/mo/dy/hr/mn/sc -Byr/mo/dy/hr/mn/sc --end-time=yr/mo/dy/hr/mn/sc -Eyr/mo/dy/hr/mn/sc\n"
    "           --display-svp-file=file -Sfile --edit-svp-file=file -Wfile\n"
    "           --dump --raypaths --angle-mode=mode --threads=number\n"
    "           --export=image.png --size=widthxheight\n"
    "           --save-edit-svp=file --save-swath-svp --save-residuals\n"
    "           --verbose -V --help -H]";

/* the long options without a short form, numbered clear of the letters of the short options */
enum LongOption {
  OPT_DUMP = 1000,
  OPT_RAYPATHS,
  OPT_ANGLE_MODE,
  OPT_THREADS,
  OPT_SAVE_EDIT,
  OPT_SAVE_SWATH,
  OPT_SAVE_RESIDUALS,
  OPT_EXPORT,
  OPT_SIZE
};

/* same layout as the verbose output of mbvelocitytool */
static void print_residuals(const mbvt::Residuals &r) {
  printf("\nCurrent Bathymetry Depth Range:\n\tminimum depth: %f\n\tmaximum depth: %f\n", r.bath_min, r.bath_max);
  printf("\nSwath Bathymetry Beam Residuals:\n");
  printf(" beam   angle   acrosstrack   altitude   residual     sigma  calculations\n");
  for (int i = 0; i < r.nbeams; i++)
    printf(" %4d  %7.3f  %9.3f   %9.3f  %9.3f  %9.3f  %5d\n", i, r.angle[i], r.acrosstrack[i], r.altitude[i], r.residual[i],
           r.sigma[i], r.count[i]);
}

int main(int argc, char **argv) {
  static struct option options[] = {{"input", required_argument, nullptr, 'I'},
                                    {"format", required_argument, nullptr, 'F'},
                                    {"begin-time", required_argument, nullptr, 'B'},
                                    {"end-time", required_argument, nullptr, 'E'},
                                    {"display-svp-file", required_argument, nullptr, 'S'},
                                    {"edit-svp-file", required_argument, nullptr, 'W'},
                                    {"dump", no_argument, nullptr, OPT_DUMP},
                                    {"raypaths", no_argument, nullptr, OPT_RAYPATHS},
                                    {"angle-mode", required_argument, nullptr, OPT_ANGLE_MODE},
                                    {"threads", required_argument, nullptr, OPT_THREADS},
                                    {"save-edit-svp", required_argument, nullptr, OPT_SAVE_EDIT},
                                    {"save-swath-svp", no_argument, nullptr, OPT_SAVE_SWATH},
                                    {"save-residuals", no_argument, nullptr, OPT_SAVE_RESIDUALS},
                                    {"export", required_argument, nullptr, OPT_EXPORT},
                                    {"size", required_argument, nullptr, OPT_SIZE},
                                    {"verbose", no_argument, nullptr, 'V'},
                                    {"help", no_argument, nullptr, 'H'},
                                    {nullptr, 0, nullptr, 0}};

  std::string input, display_file, edit_file, save_edit, export_file;
  int format = 0, verbose = 0, anglemode = MBP_ANGLES_SNELL, nthreads = 0, width = 1016, height = 668;
  int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
  int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
  bool help = false, dump = false, raypaths = false, save_swath = false, save_res = false, errflg = false;
  int c, option_index;
  while ((c = getopt_long(argc, argv, "VvHhI:i:F:f:B:b:E:e:S:s:W:w:", options, &option_index)) != -1) {
    switch (c) {
    case 'I':
    case 'i':
      input = optarg;
      break;
    case 'F':
    case 'f':
      format = atoi(optarg);
      break;
    case 'B':
    case 'b':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
      btime_i[6] = 0;
      break;
    case 'E':
    case 'e':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
      etime_i[6] = 0;
      break;
    case 'S':
    case 's':
      display_file = optarg;
      break;
    case 'W':
    case 'w':
      edit_file = optarg;
      break;
    case OPT_DUMP:
      dump = true;
      break;
    case OPT_RAYPATHS:
      raypaths = true;
      break;
    case OPT_ANGLE_MODE:
      anglemode = atoi(optarg);
      break;
    case OPT_THREADS:
      nthreads = atoi(optarg);
      break;
    case OPT_SAVE_EDIT:
      save_edit = optarg;
      break;
    case OPT_SAVE_SWATH:
      save_swath = true;
      break;
    case OPT_SAVE_RESIDUALS:
      save_res = true;
      break;
    case OPT_EXPORT:
      export_file = optarg;
      break;
    case OPT_SIZE:
      if (sscanf(optarg, "%dx%d", &width, &height) != 2 || width < 200 || height < 150)
        errflg = true;
      break;
    case 'V':
    case 'v':
      verbose++;
      break;
    case 'H':
    case 'h':
      help = true;
      break;
    default:
      errflg = true;
    }
  }

  if (anglemode != MBP_ANGLES_OK && anglemode != MBP_ANGLES_SNELL && anglemode != MBP_ANGLES_SNELLNULL) {
    fprintf(stderr, "Angle mode %d is not one of %d, %d, %d\n", anglemode, MBP_ANGLES_OK, MBP_ANGLES_SNELL, MBP_ANGLES_SNELLNULL);
    errflg = true;
  }
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", mbvt::PROGRAM_NAME);
    return MB_ERROR_BAD_USAGE;
  }
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", mbvt::PROGRAM_NAME);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    return MB_ERROR_NO_ERROR;
  }

  const bool saving = !save_edit.empty() || save_swath || save_res;
  if (!dump && !saving) {
    QApplication app(argc, argv);
    const bool quiet = !export_file.empty();
    MainWindow window(verbose, nthreads, anglemode, btime_i, etime_i, quiet);
    if (!quiet)
      window.show();
    if (!edit_file.empty())
      window.openEditProfile(QString::fromStdString(edit_file));
    if (!display_file.empty())
      window.openDisplayProfile(QString::fromStdString(display_file));
    if (!input.empty())
      window.openSwath(QString::fromStdString(input), format);
    if (!quiet)
      return app.exec();

    /* write the plots to an image without showing a window */
    window.waitUntilIdle();
    if (window.failed())
      return MB_ERROR_OPEN_FAIL;
    if (!window.saveImage(QString::fromStdString(export_file), width, height)) {
      fprintf(stderr, "%s: cannot write %s\n", mbvt::PROGRAM_NAME, export_file.c_str());
      return MB_ERROR_WRITE_FAIL;
    }
    return MB_ERROR_NO_ERROR;
  }

  std::string warning, error;
  mbvt::SwathFile swath;
  if (!input.empty()) {
    if (!mbvt::load_swath(input, format, btime_i, etime_i, verbose, swath, warning, error)) {
      fprintf(stderr, "%s\n", error.c_str());
      return MB_ERROR_OPEN_FAIL;
    }
    if (!warning.empty())
      fprintf(stderr, "%s\n", warning.c_str());
  }

  /* the edit profile is set before the first Reprocess, so the residuals belong to it */
  mbvt::Profile edit = mbvt::default_profile(swath.bath_min, swath.bath_max);
  if (!edit_file.empty() && !mbvt::read_profile(edit_file, mbvt::ProfileSource::File, edit, error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return MB_ERROR_OPEN_FAIL;
  }

  std::vector<mbvt::Profile> display;
  if (!swath.pings.empty()) {
    display = mbvt::swath_display_profiles(swath, error);
    if (!error.empty()) {
      fprintf(stderr, "%s\n", error.c_str());
      error.clear();
    }
  }
  if (!display_file.empty()) {
    mbvt::Profile p;
    if (mbvt::read_profile(display_file, mbvt::ProfileSource::File, p, error))
      display.push_back(std::move(p));
    else
      fprintf(stderr, "%s\n", error.c_str());
  }

  mbvt::Residuals residuals;
  double process_ms = 0.0;
  if (!swath.pings.empty()) {
    const auto start = std::chrono::steady_clock::now();
    if (!mbvt::process(swath, edit, anglemode, nthreads, residuals, error)) {
      fprintf(stderr, "%s\n", error.c_str());
      return MB_ERROR_OTHER;
    }
    process_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  }

  if (dump) {
    if (!swath.pings.empty()) {
      printf("Swath file: %s\n", swath.path.c_str());
      printf("Format: %d, travel times %s\n", swath.format, swath.traveltime ? "read" : "estimated");
      printf("Pings loaded: %d%s, beams per ping %d, ESF edits %d\n", (int)swath.pings.size(),
             swath.truncated ? " (limit reached, the rest of the file is not used)" : "", swath.beams_bath, swath.esf_edits);
      printf("First navigation: %.6f %.6f\n", swath.first_lon, swath.first_lat);
      printf("Profiles stored in the file: %d\n", (int)swath.profiles.size());
    }
    printf("Edit profile: %s, %d nodes\n", edit.name.c_str(), edit.size());
    printf("Display profiles: %d\n", (int)display.size());
    for (const mbvt::Profile &p : display)
      printf("  %s, %d nodes\n", p.name.c_str(), p.size());
    if (!swath.pings.empty()) {
      printf("Reprocess: %.1f ms, rays traced %ld, failed %ld, stopped at the bottom of the profile %ld\n", process_ms,
             residuals.rays_traced, residuals.rays_failed, residuals.rays_at_bottom);
      print_residuals(residuals);
    }
    if (raypaths && !swath.pings.empty()) {
      printf("\nRaypaths: xmax %f depth %f .. %f\n", residuals.ray_x_max, residuals.ray_depth_min, residuals.ray_depth_max);
      for (int i = 0; i < (int)residuals.raypaths.size(); i++)
        for (size_t j = 0; j < residuals.raypaths[i].x.size(); j++)
          printf(" %4d %4d %f %f\n", i, (int)j, residuals.raypaths[i].x[j], residuals.raypaths[i].z[j]);
    }
  }

  std::string saved;
  if (!save_edit.empty()) {
    if (!mbvt::write_profile(save_edit, edit, "", error)) {
      fprintf(stderr, "%s\n", error.c_str());
      return MB_ERROR_WRITE_FAIL;
    }
  }
  if (save_swath) {
    if (!mbvt::save_swath_profile(swath, edit, anglemode, saved, error)) {
      fprintf(stderr, "%s\n", error.c_str());
      return MB_ERROR_WRITE_FAIL;
    }
  }
  if (save_res) {
    if (!mbvt::save_residuals(swath, edit, residuals, saved, error)) {
      fprintf(stderr, "%s\n", error.c_str());
      return MB_ERROR_WRITE_FAIL;
    }
  }
  return MB_ERROR_NO_ERROR;
}
