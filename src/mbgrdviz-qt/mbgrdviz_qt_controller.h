/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_controller.h  11/10/2026
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
 * mbgrdviz without a window of its own: the menus of the Motif main window
 * (File, Realtime, Help), opening grids and the other files, the file
 * dialogs, the survey and realtime dialogs, and starting other tools. A
 * program shows the menus where it likes; the standalone mbgrdviz-qt shows
 * them in its small launcher window (mbgrdviz_qt_window.h). The maps
 * themselves are mbview-qt widgets, placed by mbvq::set_map_placement().
 * One controller per program, as the mbgrdviz core keeps global state.
 */

#ifndef MBGRDVIZ_QT_CONTROLLER_H
#define MBGRDVIZ_QT_CONTROLLER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <cstddef>
#include <functional>
#include <vector>

class QAction;
class QMenu;
class QWidget;

namespace mbgvq {

class Controller final : public QObject {
  Q_OBJECT

 public:
  explicit Controller(QObject *parent = nullptr);
  ~Controller() override;

  /* the controller of the program, nullptr if there is none */
  static Controller *current();

  /* File, Realtime and Help, as the Motif main window */
  QList<QMenu *> menus() const { return menus_; }
  /* the window the dialogs belong to */
  void setDialogParent(QWidget *parent) { dialog_parent_ = parent; }
  QWidget *dialogParent() const { return dialog_parent_; }

  /* open a grid in a new map; an empty path opens the internal test grid.
     Returns the mbview instance, or MBV_NO_WINDOW (999) if none opened. */
  size_t openPrimary(const QString &path);
  /* the overlay grid of a map; an empty path opens the internal test overlay */
  bool openOverlay(size_t instance, const QString &path);

  /* Starts another MB-System tool for the Action menu of a map (mbedit,
     mbeditviz, mbnavedit, mbvelocitytool). Without an opener the tool is
     started as a program of its own, the Qt version where one exists. */
  void setToolOpener(std::function<bool(const QString &tool, const QStringList &arguments)> open) {
    tool_opener_ = std::move(open);
  }
  /* the program of a tool: in directory, next to it in a CMake build tree,
     then on PATH; empty if not found */
  static QString findTool(const QString &tool, const QString &directory);

  /* closes all maps; true, nothing has to wait */
  bool prepareToClose();

  /* for the library (mbgrdviz_qt_ui.c through the bridge) */
  void setSensitivity(bool open_primary, bool open_other, bool save_site, bool save_route);
  void fileDialog(int mode, size_t instance, const QString &title, const QString &pattern);
  void launch(const QStringList &argv);
  void about();

 signals:
  void mapOpened(size_t instance);
  void mapClosed(size_t instance);
  void statusMessage(const QString &text);
  /* File > Quit */
  void closeRequested();

 private:
  void followMaps();

  QList<QMenu *> menus_;
  QWidget *dialog_parent_ = nullptr;
  QAction *open_primary_ = nullptr;
  QAction *open_other_[8] = {};
  QAction *save_site_ = nullptr;
  QAction *save_route_ = nullptr;
  QString directory_;
  std::vector<bool> open_maps_;
  std::function<bool(const QString &, const QStringList &)> tool_opener_;
};

}  // namespace mbgvq

#endif /* MBGRDVIZ_QT_CONTROLLER_H */
