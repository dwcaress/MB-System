/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_controller.cc  10/11/2026
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

#include "mbnaq_controller.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QThread>

#include "mbnaq_plots.h"

namespace mbnaq {

ProjectController *ProjectController::create(QObject *parent) {
  Session *session = Session::create();
  if (session == nullptr)
    return nullptr;
  return new ProjectController(session, parent);
}

ProjectController::ProjectController(Session *session, QObject *parent)
    : QObject(parent), session_(session), plots_(new PlotSet(*session)) {
  Callbacks callbacks;
  callbacks.messageOn = [this](const std::string &message) {
    onGuiThread([this, message] { emit statusMessage(QString::fromStdString(message)); }, false);
  };
  callbacks.messageUpdate = callbacks.messageOn;
  callbacks.messageOff = [this] {
    onGuiThread([this] { emit statusMessage(QString()); }, false);
  };
  callbacks.info = [this](const std::string &text) {
    onGuiThread([this, text] { emit messageLogged(QString::fromStdString(text)); }, false);
  };
  callbacks.error = [this](const std::string &a, const std::string &b, const std::string &c) {
    onGuiThread([this, a, b, c] {
      emit errorReported(QString::fromStdString(a), QString::fromStdString(b), QString::fromStdString(c));
    }, false);
  };
  /* The background step waits here, so that the widgets read a stable project. */
  callbacks.statusChanged = [this] {
    onGuiThread([this] { emit statusChanged(); }, true);
  };
  callbacks.crossingChanged = [this] {
    onGuiThread([this] { emit crossingChanged(); }, true);
  };
  session_->setCallbacks(callbacks);
  session_->updateStatus();
}

ProjectController::~ProjectController() {
  waitUntilIdle();
  plots_.reset();
  delete session_;
}

void ProjectController::onGuiThread(const std::function<void()> &call, bool wait) {
  if (QThread::currentThread() == thread())
    call();
  else
    QMetaObject::invokeMethod(this, call, wait ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
}

bool ProjectController::projectOpen() const { return session_->selection().projectIsOpen; }

QString ProjectController::projectName() const { return QString::fromStdString(session_->projectName()); }

void ProjectController::updateTitle() {
  const QString name = projectName();
  emit titleChanged(name.isEmpty() ? QStringLiteral("mbnavadjust-qt") : QStringLiteral("mbnavadjust-qt: ") + name);
}

void ProjectController::runJob(const QString &message, const std::function<void()> &job, const std::function<void()> &done) {
  if (busy_)
    return;
  busy_ = true;
  plots_->freeze();
  emit busyChanged(true);
  if (!message.isEmpty())
    emit statusMessage(message);
  worker_ = QThread::create(job);
  connect(worker_, &QThread::finished, this, [this, done] {
    worker_->deleteLater();
    worker_ = nullptr;
    plots_->unfreeze();
    busy_ = false;
    if (done)
      done();
    emit statusMessage(QString());
    emit statusChanged();
    emit busyChanged(false);
    emit jobFinished();
  });
  worker_->start();
}

void ProjectController::waitUntilIdle() {
  while (worker_ != nullptr)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
}

void ProjectController::openProject(const QString &path) {
  auto ok = std::make_shared<bool>(false);
  runJob(QStringLiteral("Opening project..."), [this, path, ok] { *ok = session_->openProject(path.toStdString()); },
         [this, path, ok] {
           updateTitle();
           if (*ok)
             emit projectOpened();
           else
             emit projectFailed(path);
         });
}

void ProjectController::newProject(const QString &path) {
  auto ok = std::make_shared<bool>(false);
  runJob(QStringLiteral("Creating project..."), [this, path, ok] { *ok = session_->newProject(path.toStdString()); },
         [this, path, ok] {
           updateTitle();
           if (*ok)
             emit projectOpened();
           else
             emit projectFailed(path);
         });
}

void ProjectController::importData(const QString &path, int format, bool singleSurvey) {
  runJob(QStringLiteral("Importing data..."),
         [this, path, format, singleSurvey] { session_->importData(path.toStdString(), format, singleSurvey); }, nullptr);
}

void ProjectController::importReference(const QString &path) {
  runJob(QStringLiteral("Importing reference grid..."), [this, path] { session_->importReference(path.toStdString()); },
         nullptr);
}

void ProjectController::findCrossings() {
  runJob(QStringLiteral("Checking for crossings..."), [this] { session_->findCrossings(); }, nullptr);
}

void ProjectController::autoPick(bool withVertical) {
  const int tiesBefore = session_->selection().numTies;
  runJob(QStringLiteral("Auto-picking..."), [this, withVertical] { session_->autoPick(withVertical); },
         [this, tiesBefore] {
           session_->reloadCrossingWindow();
           const int added = session_->selection().numTies - tiesBefore;
           QString text = QStringLiteral("Auto-Pick added %1 ties.\n").arg(added);
           if (added == 0)
             text += QStringLiteral("Auto-Pick skips crossings whose sections are shorter than a quarter of the project section "
                                    "length (see Controls), and crossings with too little overlap.\n");
           emit messageLogged(text);
         });
}

void ProjectController::autoSetVertical() {
  runJob(QStringLiteral("Setting vertical offsets..."), [this] { session_->autoSetVertical(); },
         [this] { session_->reloadCrossingWindow(); });
}

void ProjectController::invert() {
  runJob(QStringLiteral("Inverting for navigation adjustment model..."), [this] { session_->invert(); }, nullptr);
}

void ProjectController::updateGrids() {
  runJob(QStringLiteral("Updating bathymetry grids..."), [this] { session_->updateGrids(); }, nullptr);
}

void ProjectController::applyNavigation() {
  runJob(QStringLiteral("Applying adjusted navigation..."), [this] { session_->applyNavigation(); }, nullptr);
}

void ProjectController::closeProject() {
  if (busy_)
    return;
  session_->closeProject(true);
  updateTitle();
  emit projectClosed();
}

void ProjectController::undo() {
  if (!busy_)
    session_->undo();
}

void ProjectController::redo() {
  if (!busy_)
    session_->redo();
}

}  // namespace mbnaq
