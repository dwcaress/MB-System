/*--------------------------------------------------------------------
 *    The MB-system:  mbnavedit-qt.cc  9/11/2026
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
 * MBnavedit-qt is an interactive navigation editor for swath sonar data. This
 * file holds the command line and the modes without a window: --dump prints what
 * a file holds, and --replay runs a script of editing actions, for tests and for
 * comparing with the Motif program mbnavedit.
 */

#include <getopt.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPalette>

#include "mb_define.h"
#include "mb_status.h"
#include "mbne_canvas.h"
#include "mbne_core.h"

using mbne::Record;
using mbne::Session;

namespace {

constexpr char program_name[] = "MBNAVEDIT-QT";
constexpr char help_message[] =
    "MBnavedit-qt is an interactive navigation editor for swath sonar data.\n"
    "It can work with any data format supported by the MBIO library.\n";
constexpr char usage_message[] =
    "mbnavedit-qt\n"
    "\t--begin-time=yr/mo/da/hr/mn/sc {-Byr/mo/da/hr/mn/sc}\n"
    "\t--browse {-D}\n"
    "\t--end-time=yr/mo/da/hr/mn/sc {-Eyr/mo/da/hr/mn/sc}\n"
    "\t--format=format_id {-Fformat_id}\n"
    "\t--gui-mode {-G}\n"
    "\t--help {-H}\n"
    "\t--input=file {-Ifile}\n"
    "\t--strip-comments {-N}\n"
    "\t--use-ping-data {-P}\n"
    "\t--run-mbprocess {-X}\n"
    "\t--verbose {-V}\n"
    "\t--dump\n"
    "\t--export=image.png --size=widthxheight\n"
    "\t--replay=script\n\n";

/* drawing without a window needs an application, on the offscreen platform if none is asked for */
void ensure_application() {
  static int argc = 1;
  static char name[] = "mbnavedit-qt";
  static char *argv[] = {name, nullptr};
  if (QCoreApplication::instance() == nullptr) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
      qputenv("QT_QPA_PLATFORM", "offscreen");
    new QApplication(argc, argv);
  }
}

unsigned long hash_bytes(unsigned long h, const void *p, size_t n) {
  const unsigned char *c = (const unsigned char *)p;
  for (size_t i = 0; i < n; i++)
    h = h * 33 + c[i];
  return h;
}

void print_state(const Session &s) {
  unsigned long data = 5381, model = 5381, pixels = 5381;
  long nsel = 0, nflag = 0, nchanged = 0;
  for (const Record &r : s.records()) {
    data = hash_bytes(data, r.time_i, sizeof(r.time_i));
    data = hash_bytes(data, &r.time_d, sizeof(double));
    data = hash_bytes(data, &r.tint, sizeof(double));
    data = hash_bytes(data, &r.lon, sizeof(double));
    data = hash_bytes(data, &r.lat, sizeof(double));
    data = hash_bytes(data, &r.speed, sizeof(double));
    data = hash_bytes(data, &r.heading, sizeof(double));
    data = hash_bytes(data, &r.draft, sizeof(double));
    const int flag = r.lonlat_flag ? 1 : 0;
    data = hash_bytes(data, &flag, sizeof(int));
    model = hash_bytes(model, &r.lon_dr, sizeof(double));
    model = hash_bytes(model, &r.lat_dr, sizeof(double));
    model = hash_bytes(model, &r.speed_made_good, sizeof(double));
    model = hash_bytes(model, &r.course_made_good, sizeof(double));
    for (int t = 0; t < mbne::EDITABLE_PLOTS; t++)
      nsel += r.isSelected(t) ? 1 : 0;
    nflag += flag;
    if (r.lon != r.lon_org || r.lat != r.lat_org || r.speed != r.speed_org || r.heading != r.heading_org || r.draft != r.draft_org ||
        r.time_d != r.time_d_org)
      nchanged++;
  }
  for (int i = std::max(s.current(), 0); i < s.current() + s.nplot() && i < s.nbuff(); i++)
    pixels = hash_bytes(pixels, s.records()[i].pixel, sizeof(s.records()[i].pixel));
  printf("state: nbuff %d current %d nplot %d plots %d span %d loaded %d dumped %d selected %ld flagged %ld changed %ld tsproblem %d data %lx model %lx "
         "pixels %lx\n",
         s.nbuff(), s.current(), s.nplot(), s.numberPlots(), s.data_show_size, s.loadedTotal(), s.dumpedTotal(), nsel, nflag, nchanged,
         s.timestampProblem() ? 1 : 0, data, model, pixels);
}

int plot_type(const char *n) {
  const char *names[] = {"tint", "lon", "lat", "speed", "heading", "draft", "roll", "pitch", "heave"};
  for (int i = 0; i < 9; i++)
    if (strcmp(n, names[i]) == 0)
      return i;
  return -1;
}

/* one line of a script: the same commands as the harness around the Motif code */
int run_line(Session &s, const char *line) {
  char c[64] = "", a[1100] = "", b[64] = "";
  double d0 = 0.0, d1 = 0.0;
  int i0 = 0, i1 = 0;
  sscanf(line, "%63s", c);
  if (c[0] == 0 || c[0] == '#')
    return 0;
  const char *rest = line + strspn(line, " \t") + strlen(c);
  int status = MB_SUCCESS;
  bool quit = false;
  const auto pixel_of = [&s](int type, int i, int &x, int &y) {
    const Record &r = s.records()[s.current() + i];
    x = r.pixel[2 * type];
    y = r.pixel[2 * type + 1];
  };

  if (strcmp(c, "compat") == 0) {
    sscanf(rest, "%d", &i0);
    s.compatibility = i0 != 0;
  }
  else if (strcmp(c, "buffer") == 0) {
    sscanf(rest, "%d %d", &s.buffer_size, &s.hold_size);
  }
  else if (strcmp(c, "open") == 0) {
    sscanf(rest, "%1099s %d %d %63s", a, &i0, &i1, b);
    status = s.open(a, i0, i1 != 0, strcmp(b, "browse") == 0 ? mbne::OUTPUT_BROWSE : mbne::OUTPUT_EDIT);
    if (status == MB_SUCCESS)
      printf("sources: nav %d sensordepth %d heading %d attitude %d\n", s.navSource(), s.sensordepthSource(), s.headingSource(), s.attitudeSource());
  }
  else if (strcmp(c, "useping") == 0) {
    sscanf(rest, "%d", &i0);
    s.use_ping_data = i0 != 0;
  }
  else if (strcmp(c, "span") == 0) {
    sscanf(rest, "%d", &s.data_show_size);
    status = s.plotAll();
  }
  else if (strcmp(c, "stepsize") == 0) {
    sscanf(rest, "%d", &s.data_step_size);
  }
  else if (strcmp(c, "step") == 0) {
    sscanf(rest, "%d", &i0);
    status = s.step(i0);
  }
  else if (strcmp(c, "start") == 0) {
    status = s.start();
  }
  else if (strcmp(c, "end") == 0) {
    status = s.end();
  }
  else if (strcmp(c, "showall") == 0) {
    status = s.showAll();
  }
  else if (strcmp(c, "plot") == 0) {
    status = s.plotAll();
  }
  else if (strcmp(c, "show") == 0) {
    sscanf(rest, "%63s %d", b, &i0);
    const bool on = i0 != 0;
    const struct {
      const char *n;
      bool *v;
    } t[] = {{"tint", &s.show.tint},           {"lon", &s.show.lon},         {"lat", &s.show.lat},         {"speed", &s.show.speed},
             {"heading", &s.show.heading},     {"draft", &s.show.draft},     {"tint_org", &s.show.tint_org}, {"lon_org", &s.show.lon_org},
             {"lat_org", &s.show.lat_org},     {"speed_org", &s.show.speed_org}, {"heading_org", &s.show.heading_org},
             {"draft_org", &s.show.draft_org}, {"lon_dr", &s.show.lon_dr},   {"lat_dr", &s.show.lat_dr},   {"smg", &s.show.smg},
             {"cmg", &s.show.cmg}};
    for (const auto &e : t)
      if (strcmp(b, e.n) == 0)
        *e.v = on;
    if (strcmp(b, "vru") == 0)
      s.show.roll = s.show.pitch = s.show.heave = on;
    s.recountPlots();
    status = s.plotAll();
  }
  else if (strcmp(c, "model") == 0) {
    sscanf(rest, "%63s", b);
    s.model_mode = strcmp(b, "mean") == 0  ? mbne::MODEL_MEAN
                   : strcmp(b, "dr") == 0  ? mbne::MODEL_DR
                   : strcmp(b, "inv") == 0 ? mbne::MODEL_INVERT
                                           : mbne::MODEL_OFF;
    if (s.model_mode != mbne::MODEL_OFF)
      s.show.lon_dr = s.show.lat_dr = true;
    s.getModel();
    status = s.plotAll();
  }
  else if (strcmp(c, "window") == 0) {
    sscanf(rest, "%d", &s.mean_time_window);
    s.getModel();
    status = s.plotAll();
  }
  else if (strcmp(c, "driftlon") == 0) {
    sscanf(rest, "%d", &s.drift_lon);
    s.getModel();
    status = s.plotAll();
  }
  else if (strcmp(c, "driftlat") == 0) {
    sscanf(rest, "%d", &s.drift_lat);
    s.getModel();
    status = s.plotAll();
  }
  else if (strcmp(c, "weights") == 0) {
    sscanf(rest, "%lf %lf", &s.weight_speed, &s.weight_acceleration);
    s.getModel();
    status = s.plotAll();
  }
  else if (strcmp(c, "pick") == 0 || strcmp(c, "where") == 0) {
    sscanf(rest, "%63s %d", b, &i0);
    int x, y;
    pixel_of(plot_type(b), i0, x, y);
    if (c[0] == 'p')
      status = s.mousePick(x, y);
    else
      printf("where: %d %d\n", x, y);
  }
  else if (strcmp(c, "select") == 0 || strcmp(c, "deselect") == 0) {
    sscanf(rest, "%63s %d %d", b, &i0, &i1);
    for (int i = i0; i <= i1; i++) {
      int x, y;
      pixel_of(plot_type(b), i, x, y);
      status = c[0] == 's' ? s.mouseSelect(x, y) : s.mouseDeselect(x, y);
    }
  }
  else if (strcmp(c, "selectall") == 0) {
    sscanf(rest, "%63s", b);
    int x, y;
    pixel_of(plot_type(b), 0, x, y);
    status = s.mouseSelectAll(x, y);
  }
  else if (strcmp(c, "deselectall") == 0) {
    status = s.mouseDeselectAll();
  }
  else if (strcmp(c, "interval") == 0) {
    sscanf(rest, "%lf %lf", &d0, &d1);
    const mbne::Plot &p = s.plots().front();
    const int w = p.ixmax - p.ixmin;
    s.setInterval(p.ixmin + (int)(d0 * w), p.iymax + 5, 0);
    s.setInterval(p.ixmin + (int)(d1 * w), p.iymax + 5, 1);
    status = s.setInterval(0, 0, 2);
    s.setInterval(0, 0, 3);
  }
  else if (strcmp(c, "interpolate") == 0) {
    status = s.interpolate();
  }
  else if (strcmp(c, "repeats") == 0) {
    status = s.interpolateRepeats();
  }
  else if (strcmp(c, "revert") == 0) {
    status = s.revert();
  }
  else if (strcmp(c, "flag") == 0) {
    status = s.flag();
  }
  else if (strcmp(c, "unflag") == 0) {
    status = s.unflag();
  }
  else if (strcmp(c, "usedr") == 0) {
    status = s.useDr();
  }
  else if (strcmp(c, "usesmg") == 0) {
    status = s.useSmg();
  }
  else if (strcmp(c, "usecmg") == 0) {
    status = s.useCmg();
  }
  else if (strcmp(c, "fixtime") == 0) {
    status = s.fixTime();
  }
  else if (strcmp(c, "deletebad") == 0) {
    status = s.deleteBadTime();
  }
  else if (strcmp(c, "offset") == 0) {
    sscanf(rest, "%lf %lf", &d0, &d1);
    status = s.applyOffset(d0, d1);
  }
  else if (strcmp(c, "undo") == 0) {
    status = s.undo();
  }
  else if (strcmp(c, "redo") == 0) {
    status = s.redo();
  }
  else if (strcmp(c, "save") == 0) {
    status = s.save();
  }
  else if (strcmp(c, "discard") == 0) {
    status = s.closeDiscard();
  }
  else if (strcmp(c, "unsaved") == 0) {
    printf("unsaved: %d undo '%s' redo '%s'\n", s.unsaved() ? 1 : 0, s.undoLabel().c_str(), s.redoLabel().c_str());
  }
  else if (strcmp(c, "next") == 0) {
    status = s.nextBuffer(&quit);
    printf("next: quit %d\n", quit ? 1 : 0);
  }
  else if (strcmp(c, "done") == 0) {
    status = s.done(&quit);
    printf("done: quit %d\n", quit ? 1 : 0);
  }
  else if (strcmp(c, "quit") == 0) {
    status = s.closeSave();
  }
  else if (strcmp(c, "state") == 0) {
    print_state(s);
  }
  else if (strcmp(c, "row") == 0) {
    sscanf(rest, "%d", &i0);
    const Record &r = s.records()[i0];
    printf("row %d: t %.6f tint %.6f torg %.6f lon %.9f lat %.9f spd %.3f hdg %.3f dft %.3f smg %.3f cmg %.3f flag %d\n", i0, r.time_d, r.tint, r.tint_org,
           r.lon, r.lat, r.speed, r.heading, r.draft, r.speed_made_good, r.course_made_good, r.lonlat_flag ? 1 : 0);
  }
  else if (strcmp(c, "set") == 0) {
    sscanf(rest, "%63s %d %lf", b, &i0, &d0);
    Record &r = s.recordsForTest()[i0];
    if (strcmp(b, "heading") == 0)
      r.heading = d0;
    else if (strcmp(b, "time") == 0)
      r.time_d = d0;
    else if (strcmp(b, "tint") == 0)
      r.tint = d0;
    else if (strcmp(b, "lon") == 0)
      r.lon = d0;
  }
  else if (strcmp(c, "drawcount") == 0) {
    ensure_application();
    QImage image(1000, 1000, QImage::Format_RGB32);
    QPainter painter(&image);
    mbne::DrawCounts counts;
    s.plotAll();
    mbne::draw_plots(painter, s, image.size(), QApplication::palette(), &counts);
    printf("drawcount: fill_black %ld rect_red %ld rect_purple %ld rect_orange %ld line_green %ld line_blue %ld\n", counts.fill_black, counts.rect_red,
           counts.rect_purple, counts.rect_orange, counts.line_green, counts.line_blue);
  }
  else if (strcmp(c, "abort") == 0) {
    fflush(stdout);
    abort();
  }
  else {
    printf("unknown: %s\n", line);
    return 1;
  }
  printf("%s -> status %d\n", c, status);
  return 0;
}

int replay(const std::string &path, int verbose) {
  FILE *fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    fprintf(stderr, "%s: cannot read %s\n", program_name, path.c_str());
    return MB_ERROR_OPEN_FAIL;
  }
  Session session;
  session.verbose = verbose;
  session.on_error = [](const std::string &a, const std::string &b, const std::string &c) {
    printf("dialog: %s | %s | %s\n", a.c_str(), b.c_str(), c.c_str());
  };
  char line[2048];
  int failures = 0;
  while (fgets(line, sizeof(line), fp) != nullptr) {
    line[strcspn(line, "\r\n")] = 0;
    printf("> %s\n", line);
    failures += run_line(session, line);
    fflush(stdout);
  }
  fclose(fp);
  if (session.isOpen())
    session.closeDiscard();
  return failures == 0 ? MB_ERROR_NO_ERROR : MB_ERROR_BAD_USAGE;
}

int dump(Session &session, const std::string &input, int format) {
  session.on_error = [](const std::string &a, const std::string &b, const std::string &c) {
    fprintf(stderr, "%s\n%s\n%s\n", a.c_str(), b.c_str(), c.c_str());
  };
  if (session.open(input, format, false, mbne::OUTPUT_BROWSE) != MB_SUCCESS)
    return MB_ERROR_OPEN_FAIL;
  const auto &rec = session.records();
  printf("Swath file: %s\n", session.file());
  printf("Format: %d, navigation source %d, sensor depth source %d, heading source %d, attitude source %d\n", session.format(), session.navSource(),
         session.sensordepthSource(), session.headingSource(), session.attitudeSource());
  printf("Records loaded: %d\n", session.nbuff());
  if (!rec.empty()) {
    const Record &f = rec.front(), &l = rec.back();
    printf("First: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d lon %.9f lat %.9f\n", f.time_i[0], f.time_i[1], f.time_i[2], f.time_i[3], f.time_i[4],
           f.time_i[5], f.time_i[6], f.lon, f.lat);
    printf("Last:  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d lon %.9f lat %.9f\n", l.time_i[0], l.time_i[1], l.time_i[2], l.time_i[3], l.time_i[4],
           l.time_i[5], l.time_i[6], l.lon, l.lat);
    long repeated = 0;
    for (const Record &r : rec)
      repeated += r.lonlat_flag ? 1 : 0;
    printf("Repeated positions: %ld\n", repeated);
    printf("Duplicate or reverse order time stamps: %s\n", session.timestampProblem() ? "yes" : "no");
    printf("Span: %.3f s\n", l.time_d - f.time_d);
  }
  session.closeDiscard();
  return MB_ERROR_NO_ERROR;
}

int export_image(Session &session, const std::string &input, int format, const std::string &path, int width, int height) {
  session.on_error = [](const std::string &a, const std::string &b, const std::string &c) {
    fprintf(stderr, "%s\n%s\n%s\n", a.c_str(), b.c_str(), c.c_str());
  };
  if (session.open(input, format, false, mbne::OUTPUT_BROWSE) != MB_SUCCESS)
    return MB_ERROR_OPEN_FAIL;
  ensure_application();
  const QSize size(width, height);
  mbne::layout_plots(session, size, QApplication::font());
  QImage image(size, QImage::Format_RGB32);
  QPainter painter(&image);
  painter.setFont(QApplication::font());
  mbne::draw_plots(painter, session, size, QApplication::palette());
  painter.end();
  session.closeDiscard();
  if (!image.save(QString::fromStdString(path))) {
    fprintf(stderr, "%s: cannot write %s\n", program_name, path.c_str());
    return MB_ERROR_WRITE_FAIL;
  }
  return MB_ERROR_NO_ERROR;
}

}  // namespace

int main(int argc, char **argv) {
  enum { OPT_DUMP = 1000, OPT_REPLAY, OPT_EXPORT, OPT_SIZE };
  static struct option options[] = {{"verbose", no_argument, nullptr, 'V'},
                                    {"help", no_argument, nullptr, 'H'},
                                    {"begin-time", required_argument, nullptr, 'B'},
                                    {"browse", no_argument, nullptr, 'D'},
                                    {"end-time", required_argument, nullptr, 'E'},
                                    {"format", required_argument, nullptr, 'F'},
                                    {"gui-mode", no_argument, nullptr, 'G'},
                                    {"input", required_argument, nullptr, 'I'},
                                    {"strip-comments", no_argument, nullptr, 'N'},
                                    {"use-ping-data", no_argument, nullptr, 'P'},
                                    {"run-mbprocess", no_argument, nullptr, 'X'},
                                    {"dump", no_argument, nullptr, OPT_DUMP},
                                    {"replay", required_argument, nullptr, OPT_REPLAY},
                                    {"export", required_argument, nullptr, OPT_EXPORT},
                                    {"size", required_argument, nullptr, OPT_SIZE},
                                    {nullptr, 0, nullptr, 0}};

  Session session;
  int verbose = 0, format = 0, help = 0, errflg = 0;
  bool do_dump = false;
  int width = 1000, height = 1200;
  std::string input, replay_script, export_file;
  int c;
  while ((c = getopt_long(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:NnPpXx", options, nullptr)) != -1) {
    switch (c) {
    case 'H':
    case 'h':
      help++;
      break;
    case 'V':
    case 'v':
      verbose++;
      break;
    case 'B':
    case 'b':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d", &session.btime_i[0], &session.btime_i[1], &session.btime_i[2], &session.btime_i[3], &session.btime_i[4],
             &session.btime_i[5]);
      session.btime_i[6] = 0;
      break;
    case 'E':
    case 'e':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d", &session.etime_i[0], &session.etime_i[1], &session.etime_i[2], &session.etime_i[3], &session.etime_i[4],
             &session.etime_i[5]);
      session.etime_i[6] = 0;
      break;
    case 'D':
    case 'd':
      break;
    case 'F':
    case 'f':
      sscanf(optarg, "%d", &format);
      break;
    case 'G':
    case 'g':
      session.gui_mode = true;
      break;
    case 'I':
    case 'i':
      input = optarg;
      break;
    case 'N':
    case 'n':
    case 'X':
    case 'x':
      break;
    case 'P':
    case 'p':
      session.use_ping_data = true;
      break;
    case OPT_DUMP:
      do_dump = true;
      break;
    case OPT_REPLAY:
      replay_script = optarg;
      break;
    case OPT_EXPORT:
      export_file = optarg;
      break;
    case OPT_SIZE:
      sscanf(optarg, "%dx%d", &width, &height);
      break;
    default:
      errflg++;
    }
  }

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
  session.verbose = verbose;

  if (!replay_script.empty())
    return replay(replay_script, verbose);
  if (do_dump) {
    if (input.empty()) {
      fprintf(stderr, "--dump needs an input file\n");
      return MB_ERROR_BAD_USAGE;
    }
    return dump(session, input, format);
  }
  if (!export_file.empty()) {
    if (input.empty() || width <= 0 || height <= 0) {
      fprintf(stderr, "--export needs an input file and a size\n");
      return MB_ERROR_BAD_USAGE;
    }
    return export_image(session, input, format, export_file, width, height);
  }
  fprintf(stderr, "%s: this version has no window yet: use --dump, --export or --replay\n", program_name);
  return MB_ERROR_BAD_USAGE;
}
