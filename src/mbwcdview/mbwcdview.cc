/*--------------------------------------------------------------------
 *    The MB-system:  mbwcdview.cc  1/11/2026
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
 * MBWCDVIEW displays water column data, one swath at a time, read through MBIO.
 * Opening a Kongsberg *.kmwcd or *.kmall finds the other file of the line, so the
 * soundings, navigation and transducer depth of the *.kmall can be shown with the
 * water column of the *.kmwcd. The samples are ray traced through the logged sound
 * velocity profile. Without a window, --dump prints a summary of a line and
 * --export writes an image of one swath.
 *
 * Author:  C. dos Santos Ferreira
 * Date:    November 1, 2026
 */

#include <getopt.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <QApplication>
#include <QImage>
#include <QString>

#include "MainWindow.h"
#include "mb_define.h"
#include "mb_status.h"
#include "wcd_data.h"
#include "wcd_dump.h"
#include "wcd_plot.h"
#include "wcd_scene.h"

constexpr char program_name[] = "MBWCDVIEW";
constexpr char help_message[] =
    "MBWCDVIEW displays water column data, one swath at a time, read through MBIO.\n"
    "Opening a Kongsberg *.kmwcd or *.kmall finds the other file of the survey line,\n"
    "so that the soundings and navigation of the *.kmall are shown with the water column.\n"
    "--dump prints a summary of the line and --export writes an image of one swath,\n"
    "both without opening a window.";
constexpr char usage_message[] =
    "mbwcdview [--input=file -Ifile --format=format -Fformat --ping=index --swath=index\n"
    "           --bottom --wc-bottom --straight --export=image.png --size=widthxheight\n"
    "           --dump --verbose -V --help -H]";

int main(int argc, char **argv) {
  static struct option options[] = {{"input", required_argument, nullptr, 'I'},
                                    {"format", required_argument, nullptr, 'F'},
                                    {"ping", required_argument, nullptr, 'P'},
                                    {"swath", required_argument, nullptr, 'S'},
                                    {"bottom", no_argument, nullptr, 'b'},
                                    {"wc-bottom", no_argument, nullptr, 'w'},
                                    {"straight", no_argument, nullptr, 's'},
                                    {"export", required_argument, nullptr, 'E'},
                                    {"size", required_argument, nullptr, 'Z'},
                                    {"dump", no_argument, nullptr, 'D'},
                                    {"verbose", no_argument, nullptr, 'V'},
                                    {"help", no_argument, nullptr, 'H'},
                                    {nullptr, 0, nullptr, 0}};

  std::string input, export_file;
  int format = 0, ping = 0, swath = -1, verbose = 0, width = 1400, height = 900;
  bool help = false, dump = false, errflg = false;
  wcd::SceneOptions scene;
  int c, option_index;
  while ((c = getopt_long(argc, argv, "VvHhI:i:F:f:P:p:S:", options, &option_index)) != -1) {
    switch (c) {
    case 'I':
    case 'i':
      input = optarg;
      break;
    case 'F':
    case 'f':
      format = atoi(optarg);
      break;
    case 'P':
    case 'p':
      ping = atoi(optarg);
      break;
    case 'S':
      swath = atoi(optarg);
      break;
    case 'b':
      scene.bottom = true;
      break;
    case 'w':
      scene.wc_bottom = true;
      break;
    case 's':
      scene.raytrace = false;
      break;
    case 'E':
      export_file = optarg;
      break;
    case 'Z':
      if (sscanf(optarg, "%dx%d", &width, &height) != 2 || width < 200 || height < 150)
        errflg = true;
      break;
    case 'D':
      dump = true;
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
  if (input.empty() && optind < argc)
    input = argv[optind];

  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    return MB_ERROR_BAD_USAGE;
  }
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    return MB_ERROR_NO_ERROR;
  }

  if (dump) {
    if (input.empty()) {
      fprintf(stderr, "usage: %s\n", usage_message);
      return MB_ERROR_BAD_USAGE;
    }
    return wcd::dump(input, format, ping);
  }

  QApplication app(argc, argv);

  /* write one swath to an image without a window */
  if (!export_file.empty()) {
    wcd::Line line;
    std::string message;
    if (input.empty() || !line.load(input, wcd::Progress(), &message, format) || !line.has_wc) {
      fprintf(stderr, "%s: no water column data in %s %s\n", program_name, input.c_str(), message.c_str());
      return MB_ERROR_OPEN_FAIL;
    }
    const size_t i = (size_t)std::min(std::max(ping, 0), (int)line.wc.size() - 1);
    const std::vector<int> swaths = line.wc.swaths(i);
    const int s = (swath >= 0 && std::find(swaths.begin(), swaths.end(), swath) != swaths.end()) ? swath
                                                                                                    : swaths.front();
    scene.raytrace = scene.raytrace && line.svp(line.wc.pings[i].time_d) != nullptr;
    wcd::PlotData data;
    wcd::SceneInfo info;
    wcd::build_scene(line, i, s, scene, data, info);
    wcd::WcdPlot plot;
    plot.setColorRange(line.wc.amp_min, line.wc.amp_max);
    plot.setDepthWindow(0.0, 1.0e5);
    plot.setData(std::move(data));
    const QString caption = QString::fromStdString(input.substr(input.find_last_of("/\\") + 1)) + " | " +
                            wcd::metadata_text(line, i, s);
    if (!plot.render(QSize(width, height), 1.0, caption).save(QString::fromStdString(export_file))) {
      fprintf(stderr, "%s: cannot write %s\n", program_name, export_file.c_str());
      return MB_ERROR_WRITE_FAIL;
    }
    if (verbose > 0)
      fprintf(stderr, "%s: ping %zu swath %d %s%s written to %s\n", program_name, i, s,
              info.traced ? "ray traced" : "straight rays", info.status.isEmpty() ? "" : (", " +
              info.status.toStdString()).c_str(), export_file.c_str());
    return MB_ERROR_NO_ERROR;
  }

  MainWindow window;
  window.show();
  if (!input.empty()) {
    window.openFile(QString::fromStdString(input));
    window.showPing(ping);
  }
  return app.exec();
}
