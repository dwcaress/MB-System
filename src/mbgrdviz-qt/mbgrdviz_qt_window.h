/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_window.h  11/7/2026
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
 * The main window of mbgrdviz-qt, as the Motif mbgrdviz main window: the
 * File, Realtime and Help menus.
 */

#ifndef MBGRDVIZ_QT_WINDOW_H
#define MBGRDVIZ_QT_WINDOW_H

#include <QMainWindow>
#include <QString>

class QAction;

class MbgrdvizWindow final : public QMainWindow {
 public:
  MbgrdvizWindow();
  static MbgrdvizWindow *instance();

  void set_sensitivity(bool open_primary, bool open_other, bool save_site, bool save_route);
  /* a file dialog, then the core opens or saves the file */
  void file_dialog(int mode, size_t instance, const QString &title, const QString &pattern);
  void about();

  /* the executable to start for an MB-System tool: the Qt version where one
     exists (mbedit-qt, mbeditviz-qt, mbvelocitytool-qt), else the tool itself; looked for
     in directory (the directory of this program), next to it in a CMake
     build tree, then on PATH; empty if not found */
  static QString find_tool(const QString &tool, const QString &directory);

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  QAction *open_primary_ = nullptr;
  QAction *open_other_[8] = {};
  QAction *save_site_ = nullptr;
  QAction *save_route_ = nullptr;
  QString directory_;
};

#endif /* MBGRDVIZ_QT_WINDOW_H */
