/*--------------------------------------------------------------------
 *    The MB-system:  NavAdjustWindow.h  10/11/2026
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
 * The main window of mbnavadjust-qt: the widgets of one ProjectController in
 * docks, with the menus of the Motif program.
 */

#ifndef MBNAQ_NAVADJUSTWINDOW_H_
#define MBNAQ_NAVADJUSTWINDOW_H_

#include <QMainWindow>
#include <QMap>

#include <functional>

#include "mbnaq_core.h"

class QAction;
class QActionGroup;
class QDockWidget;
class QLabel;
class QProgressBar;

namespace mbnaq {

class BiasesPanel;
class ControlsPanel;
class CrossingEditor;
class MessageLog;
class ModelPlot;
class ProjectBrowser;
class ProjectController;
class ProjectStatus;
class TerminalLog;

class NavAdjustWindow : public QMainWindow {
  Q_OBJECT

 public:
  /* A dark background chosen on the command line overrides the system setting. */
  explicit NavAdjustWindow(ProjectController &controller, bool forceDark = false, QWidget *parent = nullptr);

  ProjectBrowser *browser() { return browser_; }
  CrossingEditor *editor() { return editor_; }
  ModelPlot *modelPlot() { return modelPlot_; }
  QAction *action(const char *name) const { return actions_.value(name, nullptr); }

 signals:
  /* the window has closed, and with it the project */
  void windowClosed();

 protected:
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;

 private:
  QAction *addMenuAction(QMenu *menu, const char *name, const QString &label, const std::function<void()> &call);
  void createMenus();
  void refreshActions();
  void showBiases();
  void showControls();
  void showModelPlot();
  void analyzeCrossings();
  void askImportData(bool singleSurvey);
  void followSystemColors();

  ProjectController &controller_;
  bool forceDark_;
  ProjectBrowser *browser_;
  ProjectStatus *status_;
  MessageLog *log_;
  TerminalLog *terminal_;
  CrossingEditor *editor_;
  ModelPlot *modelPlot_;
  QDockWidget *modelDock_;
  bool modelPlotShown_ = false;
  QWidget *controlsWindow_ = nullptr;
  ControlsPanel *controls_ = nullptr;
  QWidget *biasesWindow_ = nullptr;
  BiasesPanel *biases_ = nullptr;
  QMap<QByteArray, QAction *> actions_;
  QActionGroup *viewGroup_;
  QAction *undo_;
  QAction *redo_;
  QLabel *message_;
  QProgressBar *progress_;
};

}  // namespace mbnaq

#endif
