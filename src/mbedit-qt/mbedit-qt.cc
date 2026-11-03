/*--------------------------------------------------------------------
 *    The MB-system:  mbedit-qt.cc  2/11/2026
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
 * mbedit-qt is an interactive editor used to identify and flag artifacts in
 * swath sonar bathymetry data: a Qt6 port of the Motif program mbedit. This
 * file holds the command line and the modes without a window: --replay runs a
 * script of editing actions, --dump lists the pings of a file and --export
 * writes an image of the plot.
 */

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <string>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <QApplication>
#include <QImage>
#include <QTimer>

#include "MainWindow.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mbe_canvas.h"
#include "mbe_core.h"

namespace {

constexpr char help_message[] =
    "mbedit-qt is an interactive editor used to identify and flag\n"
    "artifacts in swath sonar bathymetry data. Once a file has been read\n"
    "in, it displays the bathymetry profiles from several pings, allowing\n"
    "the user to identify and flag anomalous beams. It is a Qt6 port of\n"
    "mbedit. The edits are saved in an edit save file that mbprocess applies.";
constexpr char usage_message[] =
    "mbedit-qt\n"
    "\t--begin-time=yr/mo/da/hr/mn/sc {-Byr/mo/da/hr/mn/sc}\n"
    "\t--browse {-D}\n"
    "\t--end-time=yr/mo/da/hr/mn/sc {-Eyr/mo/da/hr/mn/sc}\n"
    "\t--format=format_id {-Fformat_id}\n"
    "\t--gui-mode {-G}\n"
    "\t--help {-H}\n"
    "\t--input=file {-Ifile}\n"
    "\t--reference=grid\n"
    "\t--run-mbprocess {-X}\n"
    "\t--use-edit-save {-S}\n"
    "\t--verbose {-V}\n"
    "\t--dump\n"
    "\t--export=image {with --size=WxH}\n"
    "\t--replay=script\n\n";

enum { OPT_DUMP = 1000, OPT_REPLAY, OPT_EXPORT, OPT_SIZE, OPT_REFERENCE };

/*--------------------------------------------------------------------*/
/* Print the same lines as the test harness around the old code, so that a session can be compared line by line. */
void print_flags(const mbe::Session &s) {
  unsigned long crc = 5381;
  long total = 0, ok = 0, nul = 0, manual = 0, filter = 0, filter2 = 0, sonar = 0, other = 0, changed = 0;
  int beams_max = 0;
  for (const mbe::Ping &p : s.pings()) {
    if (p.beams_bath > beams_max)
      beams_max = p.beams_bath;
    for (int j = 0; j < p.beams_bath; j++) {
      const char f = p.beamflag[j];
      crc = crc * 33 + (unsigned char)f;
      total++;
      if (f != p.beamflagorg[j])
        changed++;
      if (mb_beam_ok(f))
        ok++;
      else if (mb_beam_check_flag_null(f))
        nul++;
      else if (mb_beam_check_flag_filter2(f))
        filter2++;
      else if (mb_beam_check_flag_filter(f))
        filter++;
      else if (mb_beam_check_flag_manual(f))
        manual++;
      else if (mb_beam_check_flag_sonar(f))
        sonar++;
      else
        other++;
    }
  }
  printf("flags: nbuff %d current %d beams_max %d total %ld ok %ld null %ld manual %ld filter %ld filter2 %ld sonar %ld other %ld changed %ld crc "
         "%lu\n",
         s.nbuff(), s.current(), beams_max, total, ok, nul, manual, filter, filter2, sonar, other, changed, crc);
}

/* a checksum of the positions of the soundings and ping labels in the view */
void print_layout(const mbe::Session &s) {
  unsigned long crc = 5381;
  auto add = [&crc](int value) { crc = crc * 33 + (unsigned long)(unsigned int)value; };
  if (s.isOpen()) {
    for (int i = s.current(); i < s.current() + s.nplot(); i++) {
      const mbe::Ping &p = s.pings()[i];
      for (int j = 0; j < p.beams_bath; j++) {
        add(p.bath_x[j]);
        add(p.bath_y[j]);
      }
      add(p.record);
      add(p.outbounds);
      add(p.label_x);
      add(p.label_y);
      if (p.outbounds == mbe::OUTBOUNDS_UNFLAGGED) {
        add(p.zap_x1);
        add(p.zap_x2);
        add(p.zap_y1);
        add(p.zap_y2);
      }
    }
  }
  printf("layout: crc %lu width %d xint %d yint %d\n", crc, s.settings.plot_width, s.settings.x_interval, s.settings.y_interval);
}

void print_pingstats(const mbe::Session &s) {
  long dual = 0, multi = 0, usable = 0, total = 0;
  int beams_min = 1 << 30, beams_max = 0;
  for (const mbe::Ping &p : s.pings()) {
    if (p.dualprofile)
      dual++;
    if (p.multiplicity > 0)
      multi++;
    if (p.beams_bath < beams_min)
      beams_min = p.beams_bath;
    if (p.beams_bath > beams_max)
      beams_max = p.beams_bath;
    for (int j = 0; j < p.beams_bath; j++) {
      total++;
      if (!mb_beam_check_flag_unusable2(p.beamflag[j]))
        usable++;
    }
  }
  printf("pingstats: nbuff %d beams_bath min %d max %d, dualprofile pings %ld, multiplicity>0 pings %ld, beams %ld usable %ld, global beams_bath %d\n",
         s.nbuff(), beams_min, beams_max, dual, multi, total, usable, s.beamsBath());
}

void print_beam(const mbe::Session &s, int i, int j) {
  if (i >= 0 && i < s.nbuff() && j >= 0 && j < s.pings()[i].beams_bath) {
    const mbe::Ping &p = s.pings()[i];
    const int x = (int)p.bath_x.size() > j ? p.bath_x[j] : 0;
    const int y = (int)p.bath_y.size() > j ? p.bath_y[j] : 0;
    printf("beam: ping %d beam %d flag 0x%02x org 0x%02x x %d y %d depth %.3f xtrack %.3f ltrack %.3f amp %.3f det %d pri %d pulse %d mult %d\n", i,
           j, (unsigned char)p.beamflag[j], (unsigned char)p.beamflagorg[j], x, y, p.bath[j], p.bathacrosstrack[j], p.bathalongtrack[j], p.amp[j],
           p.detect[j], p.priority[j], p.pulses[j], p.multiplicity);
  }
}

/*--------------------------------------------------------------------*/
/* Run a script of actions without a window. The commands are those of the
   harness that runs the old code, and "legacy", "save", "discard", "changes",
   "recover", "unlock", "undo", "redo", "stroke", "endstroke", "lasso",
   "previewfilter", "previewesf", "writeedits", "unflagreason", "suspects", "reference"
   and "residual", which
   the old code does not have. */
int replay(const std::string &path, int verbose) {
  FILE *fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    fprintf(stderr, "Unable to read script %s\n", path.c_str());
    return MB_ERROR_OPEN_FAIL;
  }
  mbe::Session s;
  s.verbose = verbose;
  s.on_error = [](const std::string &a, const std::string &b, const std::string &c) { printf("dialog: %s | %s | %s\n", a.c_str(), b.c_str(), c.c_str()); };
  bool checkall = false;
  char line[2048];
  while (fgets(line, sizeof(line), fp) != nullptr) {
    printf("> %s", line);
    char cmd[64] = "", a1[1024] = "", a2[64] = "";
    int v[16] = {0};
    double d[8] = {0};
    if (sscanf(line, "%63s", cmd) != 1 || cmd[0] == '#')
      continue;
    const char *rest = line + strspn(line, " \t") + strlen(cmd);
    int status = MB_SUCCESS;
    bool quit = false;
    const std::string c = cmd;

    if (c == "buffer") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      s.setBuffer(v[0], v[1]);
      continue;
    }
    if (c == "canvas") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      s.setCanvas(v[0], v[1]);
      continue;
    }
    if (c == "legacy") {
      sscanf(rest, "%d", &v[0]);
      s.compatibility = v[0] != 0;
      continue;
    }
    if (c == "checkall") {
      sscanf(rest, "%d", &v[0]);
      checkall = v[0] != 0;
      continue;
    }
    if (c == "stepsize")
      continue;
    if (c == "flags") {
      print_flags(s);
      continue;
    }
    if (c == "layout") {
      print_layout(s);
      continue;
    }
    if (c == "pingstats") {
      print_pingstats(s);
      continue;
    }
    if (c == "histo") {
      long h[256] = {0};
      for (const mbe::Ping &p : s.pings())
        for (int j = 0; j < p.beams_bath; j++)
          h[(unsigned char)p.beamflag[j]]++;
      for (int i = 0; i < 256; i++)
        if (h[i] > 0)
          printf("flag 0x%02x: %ld\n", i, h[i]);
      continue;
    }
    if (c == "beam") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      print_beam(s, v[0], v[1]);
      continue;
    }
    if (c == "ts") {
      sscanf(rest, "%d", &v[0]);
      double tsmin, tsmax;
      s.tsMinMax(s.current(), s.nplot(), v[0], &tsmin, &tsmax);
      printf("ts: %d %s min %.9g max %.9g first %.9g slope %.9g\n", v[0], mbe::Session::tsLabel(v[0]), tsmin, tsmax, s.tsValue(s.current(), v[0]),
             s.xtrackSlope(s.current()));
      continue;
    }
    if (c == "limits") {
      int beams_max;
      double distance_max;
      s.filterLimits(&beams_max, &distance_max);
      printf("limits: %d %.3f\n", beams_max, distance_max);
      continue;
    }
    if (c == "changes") {
      printf("changes: %ld unsaved %d\n", s.changes(), s.unsaved() ? 1 : 0);
      continue;
    }
    if (c == "sh") {
      fflush(stdout);
      if (system(rest) != 0)
        printf("(command returned non zero)\n");
      continue;
    }
#ifndef _WIN32
    if (c == "abort") {
      printf("abort: leaving without closing anything, as a crash would\n");
      fflush(stdout);
      _exit(9);
    }
#endif
    if (c == "unlock") {
      sscanf(rest, "%1023s", a1);
      printf("unlock: %d\n", mbe::Session::removeLock(a1) ? 1 : 0);
      continue;
    }

    if (c == "open" || c == "recover") {
      sscanf(rest, "%1023s %d %d %63s", a1, &v[0], &v[1], a2);
      status = s.open(a1, v[0], v[1] != 0, strcmp(a2, "browse") == 0 ? mbe::OUTPUT_BROWSE : mbe::OUTPUT_EDIT, c == "recover");
    }
    else if (c == "pings") {
      sscanf(rest, "%d", &s.settings.plot_size);
      status = s.replot();
    }
    else if (c == "view") {
      sscanf(rest, "%d", &v[0]);
      s.setViewMode(v[0]);
      status = s.replot();
    }
    else if (c == "show") {
      sscanf(rest, "%d", &s.settings.show_mode);
      status = s.replot();
    }
    else if (c == "time") {
      sscanf(rest, "%d", &v[0]);
      s.setPlotMode(v[0]);
      status = s.replot();
    }
    else if (c == "flagged") {
      sscanf(rest, "%d", &v[0]);
      s.settings.show_flaggedsoundings = v[0] != 0;
      status = s.replot();
    }
    else if (c == "flaggedprof") {
      sscanf(rest, "%d", &v[0]);
      s.settings.show_flaggedprofiles = v[0] != 0;
      status = s.replot();
    }
    else if (c == "plot") {
      status = s.replot();
    }
    else if (c == "step") {
      sscanf(rest, "%d", &v[0]);
      status = s.step(v[0]);
    }
    else if (c == "next") {
      status = s.nextBuffer(&quit);
    }
    else if (c == "goto") {
      int t[7] = {0};
      sscanf(rest, "%d %d %d %d %d %d", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
      status = s.gotoTime(t);
    }
    else if (c == "done") {
      status = s.done(&quit);
    }
    else if (c == "quit") {
      status = s.isOpen() ? s.closeSave() : MB_SUCCESS;
    }
    else if (c == "previewfilter") {
      /* the flags a filter run would give, in the same form as the line "flags" */
      sscanf(rest, "%d %d %d %d %d %d %d %d %d %d %lf %lf %d %lf %lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &d[0],
             &d[1], &v[10], &d[2], &d[3]);
      mbe::Filters f;
      f.medianspike = v[0] != 0;
      f.medianspike_threshold = v[1];
      f.medianspike_xtrack = v[2];
      f.medianspike_ltrack = v[3];
      f.wrongside = v[4] != 0;
      f.wrongside_threshold = v[5];
      f.cutbeam = v[6] != 0;
      f.cutbeam_begin = v[7];
      f.cutbeam_end = v[8];
      f.cutdistance = v[9] != 0;
      f.cutdistance_begin = d[0];
      f.cutdistance_end = d[1];
      f.cutangle = v[10] != 0;
      f.cutangle_begin = d[2];
      f.cutangle_end = d[3];
      mbe::Session::FlagCopy flags = s.copyFlags();
      const mbe::FlagPreview preview = s.previewFilters(f, flags, s.current());
      for (const mbe::FlagPreview::Item &item : preview.items)
        flags[item.ping][item.beam] = item.after;
      unsigned long crc = 5381;
      for (size_t i = 0; i < flags.size(); i++)
        for (const char flag : flags[i])
          crc = crc * 33 + (unsigned char)flag;
      printf("preview: flag %ld unflag %ld pings %d crc %lu\n", preview.nflag, preview.nunflag, preview.npings, crc);
      continue;
    }
    else if (c == "previewesf") {
      sscanf(rest, "%1023s", a1);
      std::string message;
      mbe::FlagPreview preview;
      const bool ok = s.previewEditFile(a1, preview, message);
      printf("previewesf: ok %d flag %ld unflag %ld pings %d complete %d\n", ok ? 1 : 0, preview.nflag, preview.nunflag, preview.npings, preview.complete ? 1 : 0);
      if (ok && strstr(rest, "accept") != nullptr)
        status = s.applyPreview(preview, "Preview");
      else
        continue;
    }
    else if (c == "reference") {
      sscanf(rest, "%1023s", a1);
      std::string message;
      status = s.loadReference(a1, message) ? MB_SUCCESS : MB_FAILURE;
      if (!message.empty())
        printf("reference: %s\n", message.c_str());
    }
    else if (c == "residual") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      std::vector<double> values;
      s.residuals(v[0], nullptr, values);
      printf("residual: %.3f\n", v[1] >= 0 && v[1] < (int)values.size() ? values[v[1]] : std::nan(""));
      continue;
    }
    else if (c == "writeedits") {
      sscanf(rest, "%1023s", a1);
      status = s.writeCurrentEdits(a1) ? MB_SUCCESS : MB_FAILURE;
    }
    else if (c == "unflagreason") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      printf("count: %ld\n", s.countReason(v[0], v[1]));
      status = s.unflagReason(v[0], v[1]);
    }
    else if (c == "suspects") {
      const std::vector<mbe::Suspect> suspects = s.findSuspects(s.copyFlags());
      for (const mbe::Suspect &sus : suspects)
        printf("suspect: %d %.2f spikes %d deficit %.2f attitude %d gap %d\n", sus.ping, sus.score, sus.spikes, sus.deficit, sus.attitude_jump ? 1 : 0, sus.time_gap ? 1 : 0);
      continue;
    }
    else if (c == "undo") {
      status = s.undo();
    }
    else if (c == "redo") {
      status = s.redo();
    }
    else if (c == "stroke") {
      s.beginStroke();
      continue;
    }
    else if (c == "endstroke") {
      s.endStroke("Stroke");
      continue;
    }
    else if (c == "lasso") {
      sscanf(rest, "%d %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]);
      status = s.mouseLasso({{v[0], v[1]}, {v[2], v[3]}, {v[4], v[5]}, {v[6], v[7]}});
    }
    else if (c == "save") {
      status = s.save();
    }
    else if (c == "discard") {
      status = s.closeDiscard();
    }
    else if (c == "toggle" || c == "pick" || c == "erase" || c == "restore" || c == "info") {
      sscanf(rest, "%d %d", &v[0], &v[1]);
      status = c == "toggle"    ? s.mouseToggle(v[0], v[1])
               : c == "pick"    ? s.mousePick(v[0], v[1])
               : c == "erase"   ? s.mouseErase(v[0], v[1])
               : c == "restore" ? s.mouseRestore(v[0], v[1])
                                : s.mouseInfo(v[0], v[1]);
    }
    else if (c == "grab") {
      sscanf(rest, "%d %d %d %d", &v[0], &v[1], &v[2], &v[3]);
      s.mouseGrab(mbe::GRAB_START, v[0], v[1]);
      s.mouseGrab(mbe::GRAB_MOVE, (v[0] + v[2]) / 2, (v[1] + v[3]) / 2);
      status = s.mouseGrab(mbe::GRAB_END, v[2], v[3]);
    }
    else if (c == "bad") {
      status = s.badPing();
    }
    else if (c == "good") {
      status = s.goodPing();
    }
    else if (c == "left") {
      status = s.leftPing();
    }
    else if (c == "right") {
      status = s.rightPing();
    }
    else if (c == "zero") {
      status = s.zeroPing();
    }
    else if (c == "flagview") {
      status = s.flagView();
    }
    else if (c == "unflagview") {
      status = s.unflagView();
    }
    else if (c == "unflagall") {
      status = s.unflagForward();
    }
    else if (c == "filter") {
      sscanf(rest, "%d %d %d %d %d %d %d %d %d %d %lf %lf %d %lf %lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &d[0],
             &d[1], &v[10], &d[2], &d[3]);
      mbe::Filters &f = s.filters;
      f.medianspike = v[0] != 0;
      f.medianspike_threshold = v[1];
      f.medianspike_xtrack = v[2];
      f.medianspike_ltrack = v[3];
      f.wrongside = v[4] != 0;
      f.wrongside_threshold = v[5];
      f.cutbeam = v[6] != 0;
      f.cutbeam_begin = v[7];
      f.cutbeam_end = v[8];
      f.cutdistance = v[9] != 0;
      f.cutdistance_begin = d[0];
      f.cutdistance_end = d[1];
      f.cutangle = v[10] != 0;
      f.cutangle_begin = d[2];
      f.cutangle_end = d[3];
      status = s.filterForward();
    }
    else {
      printf("unknown command: %s\n", line);
      continue;
    }

    printf("%-10s status %d  %10.2f ms  draw: fill 0 rect 0 line 0 text 0  beamsave %d ping %d beam %d width %d nplot %d\n", cmd, status, 0.0,
           s.beamSaved() ? 1 : 0, s.savedPing(), s.savedBeam(), s.settings.plot_width, s.nplot());
    if (checkall) {
      print_flags(s);
      print_layout(s);
    }
    fflush(stdout);
  }
  fclose(fp);
  return MB_ERROR_NO_ERROR;
}

/*--------------------------------------------------------------------*/
/* One line per ping of each file, read as in browse mode. */
int dump(mbe::Session &s, const std::vector<mbe::InputFile> &files, bool use_saved) {
  s.setBuffer(mbe::DEFAULT_BUFFER_SIZE, 0);
  printf("# file record time_d beams good null manual filter sonar other\n");
  for (size_t ifile = 0; ifile < files.size(); ifile++) {
    if (s.open(files[ifile].path, files[ifile].format, use_saved && ifile == 0, mbe::OUTPUT_BROWSE) != MB_SUCCESS) {
      if (s.isOpen())
        s.closeDiscard();
      return MB_ERROR_BAD_DATA;
    }
    while (s.isOpen()) {
      for (const mbe::Ping &p : s.pings()) {
        int good = 0, nul = 0, manual = 0, filter = 0, sonar = 0, other = 0;
        for (int j = 0; j < p.beams_bath; j++) {
          const char f = p.beamflag[j];
          if (mb_beam_ok(f))
            good++;
          else if (mb_beam_check_flag_null(f))
            nul++;
          else if (mb_beam_check_flag_filter2(f) || mb_beam_check_flag_filter(f))
            filter++;
          else if (mb_beam_check_flag_manual(f))
            manual++;
          else if (mb_beam_check_flag_sonar(f))
            sonar++;
          else
            other++;
        }
        printf("%zu %d %.6f %d %d %d %d %d %d %d\n", ifile, p.record + 1, p.time_d, p.beams_bath, good, nul, manual, filter, sonar, other);
      }
      bool quit = false;
      s.nextBuffer(&quit);
    }
  }
  return MB_ERROR_NO_ERROR;
}

}  // namespace

/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
  mbe::Session session;
  std::vector<mbe::InputFile> files;
  int verbose = 0, format = 0, help = 0, errflg = 0;
  bool browse = false, gui_mode = false, run_mbprocess = false, use_esf = false, do_dump = false;
  std::string replay_script, export_file;
  int export_width = 1400, export_height = 900;

  std::string reference;
  static struct option options[] = {{"verbose", no_argument, nullptr, 'V'},
                                    {"help", no_argument, nullptr, 'H'},
                                    {"begin-time", required_argument, nullptr, 'B'},
                                    {"browse", no_argument, nullptr, 'D'},
                                    {"end-time", required_argument, nullptr, 'E'},
                                    {"format", required_argument, nullptr, 'F'},
                                    {"gui-mode", no_argument, nullptr, 'G'},
                                    {"input", required_argument, nullptr, 'I'},
                                    {"run-mbprocess", no_argument, nullptr, 'X'},
                                    {"use-edit-save", no_argument, nullptr, 'S'},
                                    {"dump", no_argument, nullptr, OPT_DUMP},
                                    {"replay", required_argument, nullptr, OPT_REPLAY},
                                    {"export", required_argument, nullptr, OPT_EXPORT},
                                    {"size", required_argument, nullptr, OPT_SIZE},
                                    {"reference", required_argument, nullptr, OPT_REFERENCE},
                                    {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:SsXx", options, nullptr)) != -1) {
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
    case 'D':
    case 'd':
      browse = true;
      break;
    case 'E':
    case 'e':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d", &session.etime_i[0], &session.etime_i[1], &session.etime_i[2], &session.etime_i[3], &session.etime_i[4],
             &session.etime_i[5]);
      session.etime_i[6] = 0;
      break;
    case 'F':
    case 'f':
      sscanf(optarg, "%d", &format);
      break;
    case 'G':
    case 'g':
      gui_mode = true;
      break;
    case 'I':
    case 'i': {
      std::string message;
      if (!mbe::parse_input(optarg, format, files, message))
        fprintf(stderr, "%s\n", message.c_str());
      break;
    }
    case 'X':
    case 'x':
      run_mbprocess = true;
      break;
    case 'S':
    case 's':
      use_esf = true;
      break;
    case OPT_DUMP:
      do_dump = true;
      break;
    case OPT_REPLAY:
      replay_script = optarg;
      break;
    case OPT_REFERENCE:
      reference = optarg;
      break;
    case OPT_EXPORT:
      export_file = optarg;
      break;
    case OPT_SIZE:
      sscanf(optarg, "%dx%d", &export_width, &export_height);
      break;
    case '?':
      errflg++;
      break;
    }
  }

  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", mbe::PROGRAM_NAME);
    return MB_ERROR_BAD_USAGE;
  }
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", mbe::PROGRAM_NAME);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    return MB_ERROR_NO_ERROR;
  }
  session.verbose = verbose;
  session.gui_mode = gui_mode;

  if (!replay_script.empty())
    return replay(replay_script, verbose);
  if (do_dump) {
    if (files.empty()) {
      fprintf(stderr, "--dump needs an input file\n");
      return MB_ERROR_BAD_USAGE;
    }
    return dump(session, files, use_esf);
  }

  /* an image of the first file, read as in browse mode */
  if (!export_file.empty()) {
    if (files.empty()) {
      fprintf(stderr, "--export needs an input file\n");
      return MB_ERROR_BAD_USAGE;
    }
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
      qputenv("QT_QPA_PLATFORM", "offscreen");
    int qt_argc = 1;
    QApplication app(qt_argc, argv);
    Canvas canvas(session);
    if (session.open(files[0].path, files[0].format, use_esf, mbe::OUTPUT_BROWSE) != MB_SUCCESS)
      return MB_ERROR_BAD_DATA;
    const bool ok = canvas.render(QSize(export_width, export_height), 2.0).save(QString::fromStdString(export_file));
    session.closeDiscard();
    if (!ok)
      fprintf(stderr, "Unable to write %s\n", export_file.c_str());
    return ok ? MB_ERROR_NO_ERROR : MB_ERROR_WRITE_FAIL;
  }

  int qt_argc = 1;
  QApplication app(qt_argc, argv);
  QApplication::setApplicationName(mbe::PROGRAM_NAME);
  MainWindow window;
  window.session().verbose = verbose;
  window.session().btime_i[0] = session.btime_i[0];
  std::copy(session.btime_i, session.btime_i + 7, window.session().btime_i);
  std::copy(session.etime_i, session.etime_i + 7, window.session().etime_i);
  window.setBrowse(browse);
  window.setGuiMode(gui_mode);
  window.setRunMbprocess(run_mbprocess);
  for (const mbe::InputFile &file : files)
    window.addInput(file.path, file.format);
  window.setStartReference(QString::fromStdString(reference));
  window.show();
  QTimer::singleShot(0, &window, [&window, use_esf] { window.loadFirstFile(use_esf); });
  return app.exec();
}
