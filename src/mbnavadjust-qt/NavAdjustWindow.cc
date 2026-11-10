/*--------------------------------------------------------------------
 *    The MB-system:  NavAdjustWindow.cc  10/11/2026
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

#include "NavAdjustWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QProgressBar>
#include <QStatusBar>
#include <QVBoxLayout>

#include "mbnaq_controller.h"
#include "mbnaq_plots.h"
#include "mbnaq_terminal.h"
#include "mbnaq_widgets.h"

namespace mbnaq {

NavAdjustWindow::NavAdjustWindow(ProjectController &controller, bool forceDark, QWidget *parent)
    : QMainWindow(parent), controller_(controller), forceDark_(forceDark), browser_(new ProjectBrowser(controller)), status_(new ProjectStatus(controller)),
      log_(new MessageLog(controller)), terminal_(new TerminalLog), editor_(new CrossingEditor(controller)), modelPlot_(new ModelPlot(controller)), modelDock_(new QDockWidget("Model Plot")),
      viewGroup_(new QActionGroup(this)), message_(new QLabel), progress_(new QProgressBar) {
  setWindowTitle("mbnavadjust-qt");
  setCentralWidget(editor_);

  auto *projectPanel = new QWidget;
  auto *projectLayout = new QVBoxLayout(projectPanel);
  projectLayout->setContentsMargins(4, 4, 4, 4);
  projectLayout->addWidget(status_);
  projectLayout->addWidget(browser_, 1);
  auto *projectDock = new QDockWidget("Project");
  projectDock->setObjectName("projectDock");
  projectDock->setWidget(projectPanel);
  addDockWidget(Qt::LeftDockWidgetArea, projectDock);

  auto *logDock = new QDockWidget("Messages");
  logDock->setObjectName("logDock");
  logDock->setWidget(log_);
  addDockWidget(Qt::BottomDockWidgetArea, logDock);

  auto *terminalDock = new QDockWidget("Terminal");
  terminalDock->setObjectName("terminalDock");
  terminalDock->setWidget(terminal_);
  addDockWidget(Qt::BottomDockWidgetArea, terminalDock);
  splitDockWidget(logDock, terminalDock, Qt::Horizontal);

  modelDock_->setObjectName("modelDock");
  modelDock_->setWidget(modelPlot_);
  modelDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::BottomDockWidgetArea, modelDock_);
  modelDock_->hide();
  connect(modelPlot_, &ModelPlot::dismissed, modelDock_, &QDockWidget::hide);

  progress_->setRange(0, 0);
  progress_->setMaximumWidth(160);
  progress_->hide();
  statusBar()->addWidget(message_, 1);
  statusBar()->addPermanentWidget(progress_);

  createMenus();

  connect(&controller_, &ProjectController::statusChanged, this, &NavAdjustWindow::refreshActions);
  connect(&controller_, &ProjectController::crossingChanged, this, &NavAdjustWindow::refreshActions);
  connect(&controller_, &ProjectController::busyChanged, this, [this](bool busy) {
    progress_->setVisible(busy);
    browser_->setEnabled(!busy);
    editor_->setEnabled(!busy);
    modelPlot_->setEnabled(!busy);
    refreshActions();
  });
  connect(&controller_, &ProjectController::statusMessage, message_, &QLabel::setText);
  connect(&controller_, &ProjectController::titleChanged, this, &QWidget::setWindowTitle);
  connect(&controller_, &ProjectController::errorReported, this, [this](const QString &a, const QString &b, const QString &c) {
    QMessageBox::warning(this, "mbnavadjust-qt", a + "\n" + b + "\n" + c);
  });
  connect(&controller_, &ProjectController::projectFailed, this, [this](const QString &path) {
    QMessageBox::warning(this, "mbnavadjust-qt", "Unable to open the project\n" + path);
  });
  connect(&controller_, &ProjectController::projectClosed, modelDock_, &QDockWidget::hide);
  connect(editor_, &CrossingEditor::biasesRequested, this, &NavAdjustWindow::showBiases);
  connect(editor_, &CrossingEditor::controlsRequested, this, &NavAdjustWindow::showControls);

  followSystemColors();
  refreshActions();
  resize(1500, 950);
  resizeDocks({projectDock}, {560}, Qt::Horizontal);
  resizeDocks({logDock, terminalDock}, {1, 1}, Qt::Horizontal);
  resizeDocks({logDock}, {180}, Qt::Vertical);
}

QAction *NavAdjustWindow::addMenuAction(QMenu *menu, const char *name, const QString &label, const std::function<void()> &call) {
  QAction *action = menu->addAction(label);
  actions_.insert(name, action);
  connect(action, &QAction::triggered, this, call);
  return action;
}

void NavAdjustWindow::createMenus() {
  Session &session = controller_.session();

  QMenu *file = menuBar()->addMenu("File");
  addMenuAction(file, "pushButton_new", "New...", [this] {
    const QString path = QFileDialog::getSaveFileName(this, "New project", QString(), "Projects (*.nvh)");
    if (!path.isEmpty())
      controller_.newProject(path);
  });
  addMenuAction(file, "pushButton_open", "Open...", [this] {
    const QString path = QFileDialog::getOpenFileName(this, "Open project", QString(), "Projects (*.nvh);;All files (*)");
    if (!path.isEmpty())
      controller_.openProject(path);
  });
  addMenuAction(file, "pushButton_close", "Close", [this] { controller_.closeProject(); });
  file->addSeparator();
  addMenuAction(file, "pushButton_importdata", "Import Swath Data...", [this] { askImportData(false); });
  addMenuAction(file, "pushButton_importsurvey", "Import Swath Data as Single Survey...", [this] { askImportData(true); });
  addMenuAction(file, "pushButton_importreference", "Import Reference Grid...", [this] {
    const QString path = QFileDialog::getOpenFileName(this, "Import reference grid", QString(), "Grids (*.grd);;All files (*)");
    if (!path.isEmpty())
      controller_.importReference(path);
  });
  file->addSeparator();
  QAction *quit = addMenuAction(file, "pushButton_quit", "Quit", [this] { close(); });
  quit->setShortcut(QKeySequence::Quit);

  QMenu *edit = menuBar()->addMenu("Edit");
  undo_ = edit->addAction("Undo");
  undo_->setShortcut(QKeySequence::Undo);
  connect(undo_, &QAction::triggered, &controller_, &ProjectController::undo);
  redo_ = edit->addAction("Redo");
  redo_->setShortcut(QKeySequence::Redo);
  connect(redo_, &QAction::triggered, &controller_, &ProjectController::redo);

  QMenu *option = menuBar()->addMenu("Option");
  addMenuAction(option, "pushButton_controls", "Controls...", [this] { showControls(); });

  QMenu *list = menuBar()->addMenu("List");
  const struct {
    const char *name;
    const char *label;
    ListKind kind;
  } lists[] = {
      {"pushButton_showreferencegrids", "Reference Grids", ListReferenceGrids},
      {"pushButton_showsections", "Data File Sections", ListSections},
      {"pushButton_showglobalties", "Global Ties", ListGlobalTies},
      {"pushButton_showglobaltiessorted", "Global Ties Sorted", ListGlobalTiesSorted},
      {"pushButton_showdata", "Data Files", ListFiles},
      {"pushButton_showsurveys", "Surveys", ListSurveys},
      {"pushButton_showblocks", "Survey-vs-Survey Blocks", ListBlocks},
      {"pushButton_showcrossings", "All Crossings", ListCrossings},
      {"pushButton_showmediocrecrossings", ">10% Overlap Crossings", ListMediocreCrossings},
      {"pushButton_showgoodcrossings", ">25% Overlap Crossings", ListGoodCrossings},
      {"pushButton_showbettercrossings", ">50% Overlap Crossings", ListBetterCrossings},
      {"pushButton_showtruecrossings", "True Crossings", ListTrueCrossings},
      {"pushButton_showcrossingties", "Crossing Ties", ListTies},
      {"pushButton_showcrossingtiessortedall", "All Crossing Ties Sorted", ListTiesSortedAll},
      {"pushButton_showcrossingtiessortedbad", "Crossing Ties Sorted, 5% Worst", ListTiesSortedBad},
      {"pushButton_showcrossingtiessortedworst", "Crossing Ties Sorted, 1% Worst", ListTiesSortedWorst},
  };
  for (const auto &item : lists) {
    const ListKind kind = item.kind;
    addMenuAction(list, item.name, item.label, [&session, kind] { session.showList(kind); });
  }

  QMenu *view = menuBar()->addMenu("View");
  const struct {
    const char *name;
    const char *label;
    ViewMode mode;
  } views[] = {
      {"toggleButton_showallsurveys", "Show All Surveys", ViewAll},
      {"toggleButton_showselectedsurvey", "Show Only Selected Survey", ViewSurvey},
      {"toggleButton_showselectedblock", "Show Only Selected Survey-vs-Survey Block", ViewBlock},
      {"toggleButton_showselectedfile", "Show Only Selected File", ViewFile},
      {"toggleButton_showwithselectedsurvey", "Show With Selected Survey", ViewWithSurvey},
      {"toggleButton_showwithselectedfile", "Show With Selected File", ViewWithFile},
      {"toggleButton_showselectedsection", "Show With Selected Section", ViewWithSection},
  };
  for (const auto &item : views) {
    const ViewMode mode = item.mode;
    QAction *action = addMenuAction(view, item.name, item.label, [this, &session, mode] {
      session.setViewMode(mode);
      refreshActions();
    });
    action->setCheckable(true);
    action->setData(static_cast<int>(mode));
    viewGroup_->addAction(action);
  }
  view->addSeparator();
  QAction *visualize = addMenuAction(view, "pushButton_visualize", "Visualize Survey", [] {});
  visualize->setToolTip("The map view needs mbview-qt, which is added in a later step");
  addMenuAction(view, "pushButton_showmodelplot", "Show Modelplot", [this] { showModelPlot(); });

  QMenu *action = menuBar()->addMenu("Action");
  addMenuAction(action, "pushButton_poornav", "Set File Poor Navigation", [&session] { session.setFileNav(FileNav::Poor); });
  addMenuAction(action, "pushButton_goodnav", "Set File Good Navigation", [&session] { session.setFileNav(FileNav::Good); });
  addMenuAction(action, "pushButton_fixednav", "Set File Fixed Navigation", [&session] { session.setFileNav(FileNav::Fixed); });
  addMenuAction(action, "pushButton_fixedxynav", "Set File Fixed XY Navigation", [&session] { session.setFileNav(FileNav::FixedXY); });
  addMenuAction(action, "pushButton_fixedznav", "Set File Fixed Z Navigation", [&session] { session.setFileNav(FileNav::FixedZ); });
  action->addSeparator();
  addMenuAction(action, "pushButton_tie_xy", "Set Tie to XY Only", [&session] { session.setTieSetting(TieSetting::XY); });
  addMenuAction(action, "pushButton_tie_z", "Set Tie to Z Only", [&session] { session.setTieSetting(TieSetting::Z); });
  addMenuAction(action, "pushButton_tie_xyz", "Set Tie to Full XYZ", [&session] { session.setTieSetting(TieSetting::XYZ); });
  addMenuAction(action, "pushButton_tie_unfixed", "Set Tie to Unfixed", [&session] { session.setTieSetting(TieSetting::Unfixed); });
  addMenuAction(action, "pushButton_tie_fixed", "Set Tie to Fixed", [&session] { session.setTieSetting(TieSetting::Fixed); });
  action->addSeparator();
  addMenuAction(action, "pushButton_autopick", "Auto-Pick Offsets", [this] { controller_.autoPick(true); });
  addMenuAction(action, "pushButton_autopickhorizontal", "Auto-Pick Horizontal Offsets", [this] { controller_.autoPick(false); });
  addMenuAction(action, "pushButton_autosetsvsvertical", "Auto-Set Survey-vs-Survey Vertical Offsets", [this] { controller_.autoSetVertical(); });
  addMenuAction(action, "pushButton_newcrossings", "Check For New Crossings", [this] { controller_.findCrossings(); });
  addMenuAction(action, "pushButton_analyzecrossings", "Analyze Crossings", [this] { analyzeCrossings(); });
  addMenuAction(action, "pushButton_zerozoffsets", "Zero All Z Offsets", [&session] { session.zeroZOffsets(); });
  addMenuAction(action, "pushButton_unsetskipped", "Unset Skipped Crossings in List", [&session] { session.unsetSkipped(); });
  action->addSeparator();
  addMenuAction(action, "pushButton_invertnav", "Invert Navigation", [this] { controller_.invert(); });
  addMenuAction(action, "pushButton_updategrids", "Update Grids", [this] { controller_.updateGrids(); });
  addMenuAction(action, "pushButton_applynav", "Apply Adjusted Navigation", [this] { controller_.applyNavigation(); });

  QMenu *help = menuBar()->addMenu("Help");
  addMenuAction(help, "pushButton_about", "About", [this, &session] {
    QMessageBox::about(this, "About mbnavadjust-qt",
                       QString("mbnavadjust-qt adjusts the navigation of swath sonar surveys.\nPart of MB-System %1")
                           .arg(QString::fromStdString(session.versionText())));
  });
}

void NavAdjustWindow::refreshActions() {
  Session &session = controller_.session();
  const bool busy = controller_.busy();
  const bool open = session.selection().projectIsOpen;

  /* these follow only the project being open */
  static const char *const whenOpen[] = {"pushButton_importsurvey", "pushButton_importreference", "pushButton_controls"};
  for (const char *name : whenOpen)
    actions_.value(name)->setEnabled(open && !busy);
  for (auto it = actions_.begin(); it != actions_.end(); ++it) {
    const QByteArray &name = it.key();
    if (name == "pushButton_importsurvey" || name == "pushButton_importreference" || name == "pushButton_controls")
      continue;
    if (name == "pushButton_quit" || name == "pushButton_about") {
      it.value()->setEnabled(true);
      continue;
    }
    it.value()->setEnabled(!busy && session.isEnabled(name.toStdString()));
  }
  actions_.value("pushButton_visualize")->setEnabled(false);
  undo_->setEnabled(!busy && session.canUndo());
  redo_->setEnabled(!busy && session.canRedo());

  const int mode = session.selection().mode;
  for (QAction *action : viewGroup_->actions()) {
    if (action->data().toInt() == mode)
      action->setChecked(true);
  }
}

void NavAdjustWindow::askImportData(bool singleSurvey) {
  const QString path = QFileDialog::getOpenFileName(this, "Import swath data", QString(), "Datalists (*.mb-1);;All files (*)");
  if (path.isEmpty())
    return;
  bool ok = false;
  const int format = QInputDialog::getInt(this, "Import swath data", "MBIO Format ID:", -1, -999, 9999, 1, &ok);
  if (ok)
    controller_.importData(path, format, singleSurvey);
}

void NavAdjustWindow::analyzeCrossings() {
  const int list = controller_.session().selection().list;
  const bool sections = list == ListSections || list == ListGlobalTies || list == ListGlobalTiesSorted;
  controller_.session().openCrossingWindow(sections ? CrossingWindowMode::Section : CrossingWindowMode::Crossing);
}

void NavAdjustWindow::showControls() {
  if (controlsWindow_ == nullptr) {
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("Controls");
    controls_ = new ControlsPanel(controller_);
    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(controls_);
    controlsWindow_ = dialog;
  }
  controls_->load();
  controlsWindow_->show();
  controlsWindow_->raise();
}

void NavAdjustWindow::showBiases() {
  if (biasesWindow_ == nullptr) {
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("Biases");
    biases_ = new BiasesPanel(controller_);
    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(biases_);
    biasesWindow_ = dialog;
  }
  biases_->load();
  biasesWindow_->show();
  biasesWindow_->raise();
}

void NavAdjustWindow::showModelPlot() {
  /* opens as its own window, large enough to read; it can still be docked */
  if (!modelPlotShown_) {
    modelPlotShown_ = true;
    modelDock_->setFloating(true);
    modelDock_->resize(1100, 720);
  }
  modelDock_->show();
  modelDock_->raise();
  controller_.plots().openModelPlot();
}

void NavAdjustWindow::followSystemColors() {
  if (forceDark_) {
    controller_.plots().setDark(true);
    return;
  }
  const QColor base = QApplication::palette().color(QPalette::Base);
  controller_.plots().setDark(base.lightness() < 128);
}

void NavAdjustWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange)
    followSystemColors();
  QMainWindow::changeEvent(event);
}

void NavAdjustWindow::closeEvent(QCloseEvent *event) {
  if (controller_.busy()) {
    QMessageBox::information(this, "mbnavadjust-qt", "Please wait until the current step has finished.");
    event->ignore();
    return;
  }
  if (controller_.session().selection().projectIsOpen)
    controller_.closeProject();
  event->accept();
  emit windowClosed();
}

}  // namespace mbnaq
