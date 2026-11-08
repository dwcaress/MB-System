/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_main.cc  11/8/2026
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
 * mbeditviz-qt: the Qt6 version of mbeditviz. As the Motif mbeditviz_main.c
 * it creates the main window, starts mbview and the 3D soundings view, and
 * passes the command line to mbeditviz_init() of the unchanged
 * mbeditviz_prog.c, which reads the options and opens the input.
 */

#include <QApplication>

#include "mbeditviz_qt_bridge.h"
#include "mbeditviz_qt_window.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
}

int main(int argc, char **argv) {
  /* the options are for mbeditviz_init(), not for Qt */
  const int argcsave = argc;
  int qt_argc = 1;

  /* one renderer per process, chosen before the QApplication exists */
  mbvq::set_backend(mbvq::Backend::LegacyGL);
  QApplication app(qt_argc, argv);
  QApplication::setApplicationName(QStringLiteral("mbeditviz-qt"));

  MbeditvizWindow window;
  window.show();
  mbeditviz_qt_start(&window);

  /* show the window before reading the input, as do_wait_until_viewed */
  QApplication::processEvents();

  static char program_name[] = "mbeditviz-qt";
  static char help_message[] = "MBeditviz is a bathymetry editor and ptch test tool";
  static char usage_message[] =
      "mbeditviz-qt\n"
      "\t--delete-input-file {-R}\n"
      "\t--format=format_id {-Fformat_id}\n"
      "\t--help {-H}\n"
      "\t--input=file {-Ifile}\n"
      "\t--simple-mean-grid {-G}\n"
      "\t--verbose {-V}\n";
  mbeditviz_init(argcsave, argv, program_name, help_message, usage_message, &do_mbeditviz_message_on,
                 &do_mbeditviz_message_off, &do_mbeditviz_update_gui, &do_error_dialog);
  window.show_mode();

  return app.exec();
}
