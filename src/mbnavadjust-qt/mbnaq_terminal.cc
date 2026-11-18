/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_terminal.cc  10/11/2026
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

#include "mbnaq_terminal.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>

#include <cstdio>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace mbnaq {

namespace {

#ifdef _WIN32
int makePipe(int fds[2]) { return _pipe(fds, 65536, _O_BINARY); }
int dupFd(int fd) { return _dup(fd); }
int dup2Fd(int from, int to) { return _dup2(from, to); }
int closeFd(int fd) { return _close(fd); }
int readFd(int fd, char *buffer, unsigned int size) { return _read(fd, buffer, size); }
int writeFd(int fd, const char *buffer, unsigned int size) { return _write(fd, buffer, size); }
const int kOutFd = 1;
const int kErrFd = 2;
#else
int makePipe(int fds[2]) { return pipe(fds); }
int dupFd(int fd) { return dup(fd); }
int dup2Fd(int from, int to) { return dup2(from, to); }
int closeFd(int fd) { return close(fd); }
int readFd(int fd, char *buffer, unsigned int size) { return static_cast<int>(read(fd, buffer, size)); }
int writeFd(int fd, const char *buffer, unsigned int size) { return static_cast<int>(write(fd, buffer, size)); }
const int kOutFd = STDOUT_FILENO;
const int kErrFd = STDERR_FILENO;
#endif

const size_t kMaximumPending = 2000000;

}  // namespace

OutputCapture &OutputCapture::instance() {
  static OutputCapture capture;
  return capture;
}

OutputCapture::OutputCapture() {
  timer_.setInterval(100);
  connect(&timer_, &QTimer::timeout, this, &OutputCapture::flush);
}

bool OutputCapture::start() {
  if (active_)
    return true;
  fflush(stdout);
  fflush(stderr);
  int fds[2];
  if (makePipe(fds) != 0)
    return false;
  savedOut_ = dupFd(kOutFd);
  savedErr_ = dupFd(kErrFd);
  if (savedOut_ < 0 || savedErr_ < 0 || dup2Fd(fds[1], kOutFd) < 0 || dup2Fd(fds[1], kErrFd) < 0) {
    if (savedOut_ >= 0)
      dup2Fd(savedOut_, kOutFd);
    if (savedErr_ >= 0)
      dup2Fd(savedErr_, kErrFd);
    closeFd(fds[0]);
    closeFd(fds[1]);
    return false;
  }
  closeFd(fds[1]);
  pipeRead_ = fds[0];
  /* a pipe would otherwise hold back the output until its buffer is full */
#ifdef _WIN32
  setvbuf(stdout, nullptr, _IONBF, 0);
#else
  setvbuf(stdout, nullptr, _IOLBF, 0);
#endif
  active_ = true;
  thread_ = std::thread([this] { reader(); });
  timer_.start();
  return true;
}

void OutputCapture::stop() {
  if (!active_)
    return;
  fflush(stdout);
  fflush(stderr);
  /* restoring the descriptors closes the last writer, which ends the reader */
  dup2Fd(savedOut_, kOutFd);
  dup2Fd(savedErr_, kErrFd);
  thread_.join();
  closeFd(pipeRead_);
  closeFd(savedOut_);
  closeFd(savedErr_);
  pipeRead_ = savedOut_ = savedErr_ = -1;
  timer_.stop();
  active_ = false;
  flush();
}

void OutputCapture::reader() {
  char buffer[8192];
  for (;;) {
    const int count = readFd(pipeRead_, buffer, sizeof(buffer));
    if (count <= 0)
      break;
    if (writeFd(savedOut_, buffer, count) < 0) {
      /* the terminal has gone away; the widget still gets the text */
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (pending_.size() < kMaximumPending)
      pending_.append(buffer, count);
  }
}

void OutputCapture::flush() {
  std::string text;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    text.swap(pending_);
  }
  if (!text.empty())
    emit textCaptured(QString::fromLocal8Bit(text.data(), static_cast<int>(text.size())));
}

/*--------------------------------------------------------------------*/

TerminalLog::TerminalLog(QWidget *parent) : QWidget(parent), text_(new QPlainTextEdit) {
  text_->setReadOnly(true);
  text_->setMaximumBlockCount(20000);
  text_->setLineWrapMode(QPlainTextEdit::NoWrap);
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPointSize(font.pointSize() - 1);
  text_->setFont(font);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(text_);
  connect(&OutputCapture::instance(), &OutputCapture::textCaptured, this, &TerminalLog::append);
}

QString TerminalLog::text() const { return text_->toPlainText(); }

void TerminalLog::append(const QString &text) {
  QString cleaned = text;
  cleaned.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  cleaned.replace(QLatin1Char('\r'), QLatin1Char('\n'));
  QScrollBar *bar = text_->verticalScrollBar();
  const bool atEnd = bar->value() >= bar->maximum() - 2;
  QTextCursor cursor = text_->textCursor();
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(cleaned);
  if (atEnd)
    bar->setValue(bar->maximum());
}

}  // namespace mbnaq
