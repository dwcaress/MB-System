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

extern "C" {
#include "mbedit_prog.h"
}

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

  static void prepareForDataInput(char *file, int format) {
    std::cout << "prepareForDataInput() not implemented\n";
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

  static void enableFileButton(void) {
    std::cerr << "enableFileButton\n";
  }

  static void disableFileButton(void) {
    std::cerr << "disableFileButton\n";
  }

  static void enableNextButton(void) {
    std::cerr << "enableNextButton\n";
  }

  static void disableNextButton(void) {
    std::cerr << "disableNextButton\n";
  }

  static int resetScaleX(int pwidth, int maxx,
			 int xInterval, int yInterval) {
    std::cerr << "resetScaleX() - TBD!!!\n";
    return 0;
  }

  /// Emit signals on behalf of static member functions
  static mb_system::Emitter staticEmitter_;


protected:

  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  
  /// GUI item names
  GuiNames *guiNames_;
  
  /// UI root object
  QObject *ui_;

  /// QML-declared PixmapImage
  mb_system::PixmapImage *swathPixmapImage_;
  
  /// Test drawing to canvas
  bool plotTest(void);

  /// Plot swath data
  bool plotSwath(void);

  /// Do appropriate swath edit action at specified location, depending
  /// on edit-mode
  bool edit(double x, double y);

  /// Reset x-scale slider min/max values
  static void resetScaleXSlider(int width, int xMax,
				int xInterval, int yInterval);

  /// Return color name corresponding to input mbedit_color
  static const char *colorName(DrawingColor color);
  
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
  int maxPingsShown_;
  int nPingsShown_;
  int xTrackWidth_;
  PlotSliceMode sliceMode_;
  
  SoundColorCoding soundColorCoding_;
  bool showFlagSounding_;
  bool showFlagProfile_;
  PlotAncillData plotAncillData_;
  int buffSizeMax_;
  int buffSize_;
  int holdSize_;
  int format_;
  int verticalExagg_;
  int xInterval_;
  int yInterval_;
  int outMode_;
  int firstDataTime_[7];

  MouseEditMode editMode_;

  /// What do the following members mean?
  int nGood_;     /// ????
  int nBuffer_;   /// ???
  int iCurrent_;  /// ???
  int mnPlot_;    /// ???


public slots:
  // These 'slots' can be invoked directly by QML code, i.e. not connected by
  // signals
  
  /// Open and process swath file
  bool processSwathFile(QUrl swathFile);
  
  /// Methods called by QML code
  void onXtrackChanged(double value);
  void onPingsShownChanged(double value);
  void onVerticalExaggChanged(double value);
  void onPingStepChanged(double value);				     

  void onEditModeChanged(QString msg);
  void onAncillDataChanged(QString msg);
  void onSliceChanged(QString msg);
  void onColorCodeChanged(QString msg);

  void onLeftMouseButtonClicked(double x, double y);
  void onRightMouseButtonClicked(double x, double y);

  void onLeftMouseButtonDown(double x, double y);
  void onLeftMouseButtonUp(double x, double y);
  void onMouseMove(double x, double y);

  /// Invoked when main window is destroyed
  void onMainWindowDestroyed(void);

};


#endif // BACKEND_H
