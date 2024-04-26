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

extern "C" {
#include "mbedit_prog.h"
}

  

class Backend : public QObject  {
  
    Q_OBJECT

public:
  Backend(QObject *ui, int argc, char **argv);
  ~Backend();

  /// Get canvas width and height
  void canvasSize(int *width, int *height) {
    *width = canvasPixmap_->width();
    *height = canvasPixmap_->height();
  }
  
  static void drawLine(void *dummy, int x1, int y1, int x2, int y2,
		       mbedit_color_t color, int style);
  
  static void drawRect(void *dummy, int x, int y, int width, int height,
		       mbedit_color_t color, int style);
  
  static void fillRect(void *dummy, int x, int y, int width, int height,
		       mbedit_color_t color, int style);

  static void drawString(void *dummy, int x, int y, char *string,
			 mbedit_color_t color, int style);
  
  static void justifyString(void *dummy, char *string, int *width,
			    int *ascent, int *descent);


  static void parseDataList(char *file, int format) {
    return;
  }

  static int showError(char *s1, char *s2, char *s3) {
    std::cerr << "showError(): " << s1 << "\n" << s2 << "\n" << s3 << "\n";
    return 0;
  }

  static int showMessage(char *message) {
    std::cerr << "showMessage(): " << message << "\n";
    /* **
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
    *** */
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
  
protected:

  /// Open and process swath file
  bool processSwathFile(char *swathFile);
  
  /// GUI item names
  GuiNames *guiNames_;
  
  /// UI root object
  QObject *ui_;

  /// QML-declared PixmapImage
  PixmapImage *swathPixmapImage_;
  PixmapContainer pixmapContainer_;
  
  /// Test drawing to canvas
  bool plotTest(void);

  /// Plot swath data
  bool plotSwath(void);

  /// Do appropriate swath edit action at specified location, depending
  /// on edit-mode
  bool edit(double x, double y);
  
  /// Set QPainter pen color and style
  static void setPenColorAndStyle(mbedit_color_t color, int style);

  /// Reset x-scale slider min/max values
  static void resetScaleXSlider(int width, int xMax,
				int xInterval, int yInterval);

  /// Return color name corresponding to input mbedit_color
  static const char *colorName(mbedit_color_t color);
  
  /// Dummy first argument to canvas-drawing member funtions
  void *dummy_;

  /// Used within static member functions
  static QString staticTextBuf_;

  /// Input swath file name
  char inputFilename_[256];

  QPixmap *canvasPixmap_;
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
  
  /// static members are referenced by static functions whose pointers
  /// are passed to mbedit 
  static QPainter *staticPainter_;
  static QFontMetrics *staticFontMetrics_;
					 

public slots:

  /// GUI item callbacks/slots
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
  
  void testSlot(void);

};


#endif // BACKEND_H
