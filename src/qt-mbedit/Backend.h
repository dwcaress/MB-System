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
#include "CPixmapDrawer.h"
#include "Emitter.h"

extern "C" {
#include "mbedit_prog.h"
}

/** mbedit Backend application logic; QObject subclass, so it can exchange
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
    emit staticEmitter_.showMessage(QVariant((const char *)msg));
    return 0;
  }

  static int showMessage(char *message) {
    std::cerr << "showMessage(): " << message << "\n";
    emit staticEmitter_.showMessage(QVariant((const char *)message));
    
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

  // IVOKABLE methods can be invoked directly by QML code
  
  /// Open and process swath file
  Q_INVOKABLE bool processSwathFile(QUrl swathFile);
  
  /// Methods called by QML code
  Q_INVOKABLE void onXtrackChanged(double value);
  Q_INVOKABLE void onPingsShownChanged(double value);
  Q_INVOKABLE void onVerticalExaggChanged(double value);
  Q_INVOKABLE void onPingStepChanged(double value); 

  Q_INVOKABLE void onEditModeChanged(QString msg);
  Q_INVOKABLE void onAncillDataChanged(QString msg);
  Q_INVOKABLE void onSliceChanged(QString msg);
  Q_INVOKABLE void onColorCodeChanged(QString msg);

  Q_INVOKABLE void onLeftMouseButtonClicked(double x, double y);
  Q_INVOKABLE void onRightMouseButtonClicked(double x, double y);

  Q_INVOKABLE void onLeftMouseButtonDown(double x, double y);
  Q_INVOKABLE void onLeftMouseButtonUp(double x, double y);
  Q_INVOKABLE void onMouseMove(double x, double y);

  /// Invoked by QML when resize occurs; sets xScale_ and yScale member values
  Q_INVOKABLE void onPixmapImageResize(int width, int height);
  
  /// Invoked when main window is destroyed
  Q_INVOKABLE void onMainWindowDestroyed(void);
  

protected:

  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  
  /// GUI item names
  GuiNames *guiNames_;

  /// scaling between device and world x-coordinate
  double xScale_ = 1;

  /// scaling between device and world y-coordinate
  double yScale_ = 1;
  
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

  /// Dummy first argument to canvas-drawing member funtions
  void *dummy_;

  /// Input swath file name
  char *swathFileName_ = nullptr;

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
  int nDumped_ = 0;
  int nLoaded_ = 0;
  int nGood_;     /// ????
  int nBuffer_;   /// ???
  int iCurrent_;  /// ???
  int mnPlot_;    /// ???
	     

};


#endif // BACKEND_H
