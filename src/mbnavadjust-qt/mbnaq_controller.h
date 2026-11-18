/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_controller.h  10/11/2026
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
 * mbnaq::ProjectController is the object that the mbnavadjust-qt widgets share:
 * one project, its selections and its plots. It runs the long project steps on a
 * background thread and tells the widgets what has changed. Because the program
 * logic keeps its state in global variables, there can be one controller.
 */

#ifndef MBNAQ_CONTROLLER_H_
#define MBNAQ_CONTROLLER_H_

#include <QObject>
#include <QString>

#include <functional>
#include <memory>

#include "mbnaq_core.h"

class QThread;

namespace mbnaq {

class PlotSet;

class ProjectController : public QObject {
  Q_OBJECT

 public:
  /* The controller of this process, or nullptr if there is one already. */
  static ProjectController *create(QObject *parent = nullptr);
  ~ProjectController() override;

  Session &session() { return *session_; }
  PlotSet &plots() { return *plots_; }
  bool busy() const { return busy_; }
  bool projectOpen() const;
  QString projectName() const;

  /* These run on a background thread; the result arrives with busyChanged(false). */
  void openProject(const QString &path);
  void newProject(const QString &path);
  void importData(const QString &path, int format, bool singleSurvey);
  void importReference(const QString &path);
  void findCrossings();
  void autoPick(bool withVertical);
  void autoSetVertical();
  void invert();
  void updateGrids();
  void applyNavigation();

  /* Closing writes the outstanding edits; a project is saved every 10 edits anyway. */
  void closeProject();
  /* true when the host may close: nothing is running, and the project has been closed */
  bool prepareToClose();

  /* for a host: these return false if the program is busy or there is no such item */
  bool showList(ListKind kind);
  bool setViewMode(ViewMode mode);
  bool openCrossing(int crossing, int tie = -1);
  bool openSection(int file, int section);
  void dismissCrossing();

  void undo();
  void redo();

  /* waits for a running background step, keeping the events going */
  void waitUntilIdle();

 signals:
  void busyChanged(bool busy);
  void statusChanged();
  void crossingChanged();
  void projectOpened();
  void projectClosed();
  void projectFailed(const QString &path);
  void statusMessage(const QString &message);
  void messageLogged(const QString &text);
  void errorReported(const QString &line1, const QString &line2, const QString &line3);
  void titleChanged(const QString &title);
  /* the selected list, view, survey, file, section, crossing or tie has changed */
  void selectionChanged();
  void tiesChanged(int numberOfTies);
  /* 0 none, 1 out of date, 2 current */
  void inversionChanged(int status);
  void jobFinished();

 private:
  explicit ProjectController(Session *session, QObject *parent);
  void runJob(const QString &message, const std::function<void()> &job, const std::function<void()> &done);
  void onGuiThread(const std::function<void()> &call, bool wait);
  void updateTitle();
  void reportChanges();

  Session *session_;
  std::unique_ptr<PlotSet> plots_;
  QThread *worker_ = nullptr;
  bool busy_ = false;
  Selection last_;
};

}  // namespace mbnaq

#endif
