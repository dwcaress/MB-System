#ifndef BACKEND_H
#define BACKEND_H
#include <iostream>
#include <QObject>
#include <QString>
#include <QQuickItem>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include "GuiNames.h"
#include "PixmapImage.h"
#include "PixmapDrawer.h"
#include "Emitter.h"

/** Backend application logic; QObject subclass, so it exchanges
info with QML  */
class Backend : public QObject  {

  /// This macro needed to enable info exchange with QML
  Q_OBJECT

public:
  Backend(int argc, char **argv);
  ~Backend();

  /// Complete Backend initialization, load/display swath file if
  /// specified on command line
  bool initialize(QObject *loadedRoot, int argc, char **argv);

  /// Prepare for input of one or more specified swath data files
  static void parseDataList(char *file, int format) {
    std::cout << "parseDataList() not implemented\n";
    return;
  }

  static int showError(char *s1, char *s2, char *s3) {
    std::cerr << "showError(): " << s1 << "\n" << s2 << "\n" << s3 << "\n";
    char msg[256];
    sprintf(msg, "%s\n%s\n%s\n", s1, s2, s3);
    emit staticEmitter_.showMessage(QVariant(msg));
    return 0;
  }

  static int showMessage(char *message) {
    std::cerr << "showMessage(): " << message << "\n";
    emit staticEmitter_.showMessage(QVariant(message));
    
    return 0;
  }

  static int hideMessage(void) {
    std::cerr << "hideMessage()\n";
    return 0;
  }

  static void enableFileInput(void) {
    std::cerr << "enableFileInput\n";
  }

  static void disableFileInput(void) {
    std::cerr << "disableFileInput\n";
  }

  static void setUiElements() {
    std::cerr << "setUiElements() not implemented!\n";
  }

  /// Emit signals on behalf of static member functions
  static mb_system::Emitter staticEmitter_;


protected:

  /// Process specified swath file
  bool processSwathFile(QUrl fileUrl);

  /// Plot current swath file
  bool plotSwath(void);
  
  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  
  
  /// UI root object
  QObject *ui_;

  /// QML-declared PixmapImage
  mb_system::PixmapImage *swathPixmapImage_;
  
  /// Test drawing to canvas
  bool plotTest(void);

  /// Do appropriate swath edit action at specified location, depending
  /// on edit-mode
  bool edit(double x, double y);

  /// Dummy first argument to canvas-drawing member funtions
  void *dummy_;

  /// Input swath file name
  char inputFilename_[256];

  /// Pixmap representation of swath data graph
  QPixmap *canvasPixmap_;

  /// painter_ draws into canvasPixmap_
  QPainter *painter_;

  /// Indicates if data is plotted
  bool dataPlotted_;
  
  // Display parameters


public slots:
  // These 'slots' can be invoked directly by QML code, i.e. not connected by
  // signals
  
  /// Methods called by QML code

  /// Invoked when main window is destroyed
  void onMainWindowDestroyed(void);

};


#endif // BACKEND_H
