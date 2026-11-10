/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_main.cc  11/7/2026
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
 * mbgrdviz-qt: the Qt6 version of mbgrdviz. The options and the opening of
 * the files on the command line are those of the Motif mbgrdviz
 * (src/mbgrdviz/mbgrdviz_main.c); the program logic is the shared
 * src/mbgrdviz/mbgrdviz_core.c.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QApplication>

#include "mbgrdviz_qt_window.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"

/* mbgrdviz_state.h is for C only */
extern int mbgrdviz_verbose;
extern bool mbgrdviz_mbview_id[MBV_MAX_WINDOWS];
}

namespace {

/* the name in messages; the files the program writes keep "MBgrdviz"
   (mbgrdviz_program_name), as the Motif mbgrdviz writes them */
const char program_name[] = "mbgrdviz-qt";
const char help_message[] = "MBgrdviz provides simple interactive 2D/3Dvizualization of GMT grids.";
const char usage_message[] =
    "mbgrdviz-qt\n"
    "\t--grid-file=grdfile {-Igrdfile}\n"
    "\t--help {-H}\n"
    "\t--overlay-file=grdfile {-Jgrdfile}\n"
    "\t--test {-T}\n"
    "\t--verbose {-V}\n\n";

}  // namespace

int main(int argc, char **argv) {
  int error = MB_ERROR_NO_ERROR;
  int verbose = 0;
  int help = 0;
  int errflg = 0;
  int ifileflag = 0;
  int jfileflag = 0;
  int testflag = 0;
  char ifile[MB_PATH_MAXLINE] = "";
  char jfile[MB_PATH_MAXLINE] = "";

  static struct option options[] = {{"verbose", no_argument, nullptr, 0},
                                    {"help", no_argument, nullptr, 0},
                                    {"grid-file", required_argument, nullptr, 0},
                                    {"overlay-file", required_argument, nullptr, 0},
                                    {"test", no_argument, nullptr, 0},
                                    {nullptr, 0, nullptr, 0}};
  int option_index;
  int c;

  /* process argument list */
  while ((c = getopt_long(argc, argv, "VvHhI:i:J:j:Tt", options, &option_index)) != -1) {
    switch (c) {
    /* long options all return c=0 */
    case 0:
      if (strcmp("verbose", options[option_index].name) == 0) {
        verbose++;
      }
      else if (strcmp("help", options[option_index].name) == 0) {
        help++;
      }
      else if (strcmp("grid-file", options[option_index].name) == 0) {
        sscanf(optarg, "%1023s", ifile);
        ifileflag++;
      }
      else if (strcmp("overlay-file", options[option_index].name) == 0) {
        sscanf(optarg, "%1023s", jfile);
        jfileflag++;
      }
      else if (strcmp("test", options[option_index].name) == 0) {
        testflag++;
      }
      break;
    case 'H':
    case 'h':
      help++;
      break;
    case 'V':
    case 'v':
      verbose++;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%1023s", ifile);
      ifileflag++;
      break;
    case 'J':
    case 'j':
      sscanf(optarg, "%1023s", jfile);
      jfileflag++;
      break;
    case 'T':
    case 't':
      testflag++;
      break;
    case '?':
      errflg++;
    }
  }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }

  /* print starting message */
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting message */
  if (help) {
    fprintf(stderr, "\n%s\n\nUsage: %s\n", help_message, usage_message);
    error = MB_ERROR_NO_ERROR;
    exit(error);
  }

  /* one renderer per process, chosen before the QApplication exists */
  mbvq::set_backend(mbvq::Backend::LegacyGL);
  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("mbgrdviz-qt"));

  MbgrdvizWindow window;
  window.show();

  /* initialize the vizualization code */
  mbgrdviz_verbose = verbose;
  mbview_startup(verbose, nullptr, nullptr, &error);
  for (int i = 0; i < MBV_MAX_WINDOWS; i++)
    mbgrdviz_mbview_id[i] = false;
  do_mbgrdviz_sensitivity();

  /* open the files specified on the command line */
  if (ifileflag > 0) {
    do_mbgrdviz_openprimary(ifile);
    if (jfileflag > 0) {
      do_mbgrdviz_openoverlay(0, jfile);
    }
  }
  else if (testflag > 0) {
    do_mbgrdviz_openprimary(nullptr);
  }

  return app.exec();
}
