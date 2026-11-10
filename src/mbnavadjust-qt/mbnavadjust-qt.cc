/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust-qt.cc  9/11/2026
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
 * mbnavadjust-qt is the Qt6 port of mbnavadjust. --dump prints what each list
 * shows and --replay runs a script of user actions; both are used to compare
 * the port with the Motif code.
 */

#include <getopt.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <QGuiApplication>
#include <QString>

#include "mbnaq_core.h"
#include "mbnaq_plots.h"

using namespace mbnaq;

static const char program_name[] = "mbnavadjust-qt";
static const char usage_message[] =
    "mbnavadjust-qt [--input=project --reset-crossings --dark-background --verbose --help\n"
    "                --dump --replay=script --export=prefix --crossing=number]";

static const std::map<std::string, ListKind> list_names = {{"referencegrids", ListReferenceGrids},
                                                           {"surveys", ListSurveys},
                                                           {"blocks", ListBlocks},
                                                           {"files", ListFiles},
                                                           {"sections", ListSections},
                                                           {"crossings", ListCrossings},
                                                           {"mediocrecrossings", ListMediocreCrossings},
                                                           {"goodcrossings", ListGoodCrossings},
                                                           {"bettercrossings", ListBetterCrossings},
                                                           {"truecrossings", ListTrueCrossings},
                                                           {"ties", ListTies},
                                                           {"tiessortedall", ListTiesSortedAll},
                                                           {"tiessortedworst", ListTiesSortedWorst},
                                                           {"tiessortedbad", ListTiesSortedBad},
                                                           {"globalties", ListGlobalTies},
                                                           {"globaltiessorted", ListGlobalTiesSorted}};

static const std::map<std::string, ViewMode> view_names = {{"all", ViewAll},
                                                           {"survey", ViewSurvey},
                                                           {"withsurvey", ViewWithSurvey},
                                                           {"block", ViewBlock},
                                                           {"file", ViewFile},
                                                           {"withfile", ViewWithFile},
                                                           {"withsection", ViewWithSection}};

static void print_state(const Session &session) {
  printf("--- state\n");
  for (const std::string &line : session.statusLines())
    printf("status: %s\n", line.c_str());
  const ListView &list = session.list();
  printf("list-title: %s\n", list.title.c_str());
  printf("list-rows: %zu\n", list.rows.size());
  for (const std::string &row : list.rows)
    printf("row: %s\n", row.c_str());
  if (list.selected >= 0)
    printf("list-selected: %d\n", list.selected + 1);
  else
    printf("list-selected: none\n");
  for (const std::string &line : session.crossingStatusLines())
    printf("crossing: %s\n", line.c_str());
  for (const std::string &line : session.crossingOffsetLines())
    printf("offsets: %s\n", line.c_str());
  const Selection s = session.selection();
  printf("selection: list %d mode %d survey %d %d %d file %d section %d crossing %d tie %d\n", s.list, s.mode, s.survey,
         s.survey1, s.survey2, s.file, s.section, s.crossing, s.tie);
  printf("window: mode %d status %d current %d %d %d %d\n", s.naverrMode, s.status, s.currentCrossing, s.currentTie,
         s.currentFile, s.currentSection);
  printf("working: %.9f %.9f %.6f threshold %.6f saves %d\n", s.offsetX, s.offsetY, s.offsetZ, s.sortedThreshold, s.saveCount);
  printf("plots: %s\n", session.drawCounts().c_str());
  for (const std::string &line : session.modelPlotLines())
    printf("model: %s\n", line.c_str());
  for (const auto &item : session.enabledStates())
    printf("enabled: %s %d\n", item.first.c_str(), item.second ? 1 : 0);
}

/* Runs one line of a script; false if the line is not understood. */
static bool run_project_command(Session &session, const std::string &command, const std::string &arg) {
  if (command == "open")
    session.openProject(arg);
  else if (command == "importref")
    session.importReference(arg);
  else if (command == "close")
    session.closeProject(false);
  else if (command == "write")
    session.writeProject();
  else if (command == "state")
    print_state(session);
  else if (command == "list" && list_names.count(arg) != 0)
    session.showList(list_names.at(arg));
  else if (command == "view" && view_names.count(arg) != 0)
    session.setViewMode(view_names.at(arg));
  else if (command == "click")
    session.clickListRow(atoi(arg.c_str()) - 1);
  else if (command == "reclick")
    session.clickSelectedRow();
  else if (command == "analyze")
    session.openCrossingWindow(arg == "section" ? CrossingWindowMode::Section : CrossingWindowMode::Crossing);
  else if (command == "next")
    session.nextCrossing();
  else if (command == "prev")
    session.previousCrossing();
  else if (command == "nextunset")
    session.nextUnsetCrossing();
  else if (command == "addtie")
    session.addTie();
  else if (command == "deletetie")
    session.deleteTie();
  else if (command == "selecttie")
    session.selectTie();
  else if (command == "settie")
    session.setTie();
  else if (command == "resettie")
    session.resetTie();
  else if (command == "unset")
    session.unsetCrossing();
  else if (command == "skip")
    session.skipCrossing();
  else if (command == "dismiss")
    session.dismissCrossingWindow();
  else if (command == "fullsize")
    session.fullSize();
  else if (command == "zerooffset")
    session.zeroOffset();
  else if (command == "zerozoffset")
    session.zeroZOffset();
  else if (command == "applyzoffset")
    session.applyZOffset();
  else if (command == "minmisfit")
    session.moveToMinimumMisfit();
  else if (command == "minxymisfit")
    session.moveToMinimumXYMisfit();
  else if (command == "misfitcenter")
    session.setMisfitCenter(arg == "zero");
  else if (command == "file" && arg == "poor")
    session.setFileNav(FileNav::Poor);
  else if (command == "file" && arg == "good")
    session.setFileNav(FileNav::Good);
  else if (command == "file" && arg == "fixed")
    session.setFileNav(FileNav::Fixed);
  else if (command == "file" && arg == "fixedxy")
    session.setFileNav(FileNav::FixedXY);
  else if (command == "file" && arg == "fixedz")
    session.setFileNav(FileNav::FixedZ);
  else if (command == "tie" && arg == "xy")
    session.setTieSetting(TieSetting::XY);
  else if (command == "tie" && arg == "z")
    session.setTieSetting(TieSetting::Z);
  else if (command == "tie" && arg == "xyz")
    session.setTieSetting(TieSetting::XYZ);
  else if (command == "tie" && arg == "unfixed")
    session.setTieSetting(TieSetting::Unfixed);
  else if (command == "tie" && arg == "fixed")
    session.setTieSetting(TieSetting::Fixed);
  else if (command == "autopick")
    session.autoPick(true);
  else if (command == "autopickhorizontal")
    session.autoPick(false);
  else if (command == "autosetvertical")
    session.autoSetVertical();
  else if (command == "findcrossings")
    session.findCrossings();
  else if (command == "zerozoffsets")
    session.zeroZOffsets();
  else if (command == "unsetskipped")
    session.unsetSkipped();
  else if (command == "invert")
    session.invert();
  else if (command == "updategrids")
    session.updateGrids();
  else if (command == "applynav")
    session.applyNavigation();
  else
    return false;
  return true;
}

static bool run_command(Session &session, PlotSet &plots, const std::string &line) {
  std::istringstream in(line);
  std::string command, arg;
  in >> command >> arg;
  if (command.empty() || command[0] == '#')
    return true;
  int a = 0, b = 0, c = 0;
  {
    std::istringstream numbers(line);
    std::string skip;
    numbers >> skip >> a >> b >> c;
  }
  if (command == "contpress")
    session.contourPress(a, b, c);
  else if (command == "contmove")
    session.contourDrag(a, b);
  else if (command == "contrelease")
    session.contourRelease(a, b, c);
  else if (command == "corrpress")
    session.misfitPress(a, b);
  else if (command == "corrmove")
    session.misfitDrag(a, b);
  else if (command == "corrrelease")
    session.misfitRelease();
  else if (command == "zoffpress")
    session.zOffsetPress(a, b);
  else if (command == "zoffmove")
    session.zOffsetDrag(a, b);
  else if (command == "zoffrelease")
    session.zOffsetRelease();
  else if (command == "modelshow")
    plots.openModelPlot();
  else if (command == "modeldismiss")
    session.closeModelPlot();
  else if (command == "modelstyle" && arg == "timeseries")
    session.setModelPlotStyle(ModelPlotStyle::TimeSeries);
  else if (command == "modelstyle" && arg == "perturbation")
    session.setModelPlotStyle(ModelPlotStyle::Perturbation);
  else if (command == "modelstyle" && arg == "tieoffsets")
    session.setModelPlotStyle(ModelPlotStyle::TieOffsets);
  else if (command == "modelfullsize")
    session.modelPlotFullSize();
  else if (command == "modelclearblock")
    session.modelPlotClearBlock();
  else if (command == "modelpress")
    session.modelPlotPress(a, b, c);
  else if (command == "modelmove")
    session.modelPlotDrag(a, b);
  else if (command == "modelrelease")
    session.modelPlotRelease(a, b, c);
  else if (command == "export")
    plots.exportImages(QString::fromStdString(arg));
  else if (command == "contourcheck")
    printf("contour-difference: %ld\n", plots.compareContourWithShared());
  else if (command == "dark")
    plots.setDark(arg == "on");
  else
    return run_project_command(session, command, arg);
  return true;
}
int main(int argc, char **argv) {
  static struct option options[] = {{"verbose", no_argument, nullptr, 'V'},
                                    {"help", no_argument, nullptr, 'H'},
                                    {"dark-background", no_argument, nullptr, 'D'},
                                    {"input", required_argument, nullptr, 'I'},
                                    {"reset-crossings", no_argument, nullptr, 'R'},
                                    {"dump", no_argument, nullptr, 1},
                                    {"replay", required_argument, nullptr, 2},
                                    {"export", required_argument, nullptr, 3},
                                    {"crossing", required_argument, nullptr, 4},
                                    {nullptr, 0, nullptr, 0}};

  int verbose = 0;
  bool help = false, error = false, dark = false, reset = false, dump = false;
  int crossing = -1;
  std::string input, replay, export_prefix;
  int c;
  while ((c = getopt_long(argc, argv, "VvHhDdI:i:Rr", options, nullptr)) != -1) {
    switch (c) {
    case 'V':
    case 'v':
      verbose++;
      break;
    case 'H':
    case 'h':
      help = true;
      break;
    case 'D':
    case 'd':
      dark = true;
      break;
    case 'I':
    case 'i':
      input = optarg;
      break;
    case 'R':
    case 'r':
      reset = true;
      break;
    case 1:
      dump = true;
      break;
    case 2:
      replay = optarg;
      break;
    case 3:
      export_prefix = optarg;
      break;
    case 4:
      crossing = atoi(optarg);
      break;
    default:
      error = true;
      break;
    }
  }

  if (error) {
    fprintf(stderr, "usage: %s\n", usage_message);
    return EXIT_FAILURE;
  }
  if (help) {
    printf("Program %s\nusage: %s\n", program_name, usage_message);
    return EXIT_SUCCESS;
  }

  /* the plots draw on images, which needs no display */
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  int qt_argc = 1;
  QGuiApplication app(qt_argc, argv);

  Session *session = Session::create();
  Callbacks callbacks;
  callbacks.info = [](const std::string &text) {
    /* the time tag lines differ between runs */
    if (text.rfind(" > User", 0) != 0)
      printf("LOG %s", text.c_str());
  };
  callbacks.error = [](const std::string &s1, const std::string &s2, const std::string &s3) {
    printf("ERROR %s | %s | %s\n", s1.c_str(), s2.c_str(), s3.c_str());
  };
  session->setCallbacks(callbacks);
  session->setVerbose(verbose);
  session->setResetCrossings(reset);
  session->updateStatus();
  PlotSet *plots = new PlotSet(*session);
  plots->setDark(dark);

  if (!input.empty() && !session->openProject(input)) {
    fprintf(stderr, "%s: could not open project %s\n", program_name, input.c_str());
    delete plots;
    delete session;
    return EXIT_FAILURE;
  }

  int status = EXIT_SUCCESS;
  if (!replay.empty()) {
    /* the programs that the project steps start also write to this output */
    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::ifstream script(replay);
    if (!script) {
      fprintf(stderr, "%s: could not open script %s\n", program_name, replay.c_str());
      status = EXIT_FAILURE;
    }
    std::string line;
    while (status == EXIT_SUCCESS && std::getline(script, line)) {
      if (!run_command(*session, *plots, line)) {
        fprintf(stderr, "%s: bad script line: %s\n", program_name, line.c_str());
        status = EXIT_FAILURE;
      }
    }
  } else if (dump) {
    for (const auto &item : list_names) {
      printf("=== %s\n", item.first.c_str());
      session->showList(item.second);
      print_state(*session);
    }
  } else if (!export_prefix.empty()) {
    session->showList(ListCrossings);
    if (!session->list().rows.empty())
      session->clickListRow(crossing >= 0 ? crossing : 0);
    if (session->isEnabled("pushButton_showmodelplot"))
      plots->openModelPlot();
    if (!plots->exportImages(QString::fromStdString(export_prefix))) {
      fprintf(stderr, "%s: could not write the images %s\n", program_name, export_prefix.c_str());
      status = EXIT_FAILURE;
    }
  } else {
    printf("%s: the windows are not part of this build step; use --dump, --replay or --export\n", program_name);
  }

  fflush(stdout);
  delete plots;
  delete session;
  return status;
}
