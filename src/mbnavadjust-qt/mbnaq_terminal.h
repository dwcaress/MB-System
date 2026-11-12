/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_terminal.h  10/11/2026
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
 * What the program logic and the programs it starts print on the terminal.
 * OutputCapture takes over the standard output and standard error of the
 * process, and still passes everything on to the terminal. Only a standalone
 * program should start it, because it changes the whole process. TerminalLog
 * shows the text.
 */

#ifndef MBNAQ_TERMINAL_H_
#define MBNAQ_TERMINAL_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <mutex>
#include <string>
#include <thread>

class QPlainTextEdit;

namespace mbnaq {

class OutputCapture : public QObject {
  Q_OBJECT

 public:
  static OutputCapture &instance();
  /* false if the output could not be taken over */
  bool start();
  void stop();
  bool active() const { return active_; }

 signals:
  void textCaptured(const QString &text);

 private:
  OutputCapture();
  void reader();
  void flush();

  bool active_ = false;
  int pipeRead_ = -1;
  int savedOut_ = -1;
  int savedErr_ = -1;
  std::thread thread_;
  std::mutex mutex_;
  std::string pending_;
  QTimer timer_;
};

class TerminalLog : public QWidget {
  Q_OBJECT

 public:
  explicit TerminalLog(QWidget *parent = nullptr);
  QString text() const;

 private:
  void append(const QString &text);

  QPlainTextEdit *text_;
};

}  // namespace mbnaq

#endif
