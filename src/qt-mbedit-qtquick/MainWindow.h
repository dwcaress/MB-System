#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QObject>
#include <QString>
#include <QQuickItem>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QQmlApplicationEngine>
#include "GuiNames.h"
#include "PixmapImage.h"

extern "C" {
#include "mbedit_prog.h"
}

class MainWindow : public QObject  {
  
    Q_OBJECT

public:
  MainWindow(QObject *ui);
  ~MainWindow();

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

  /// Read value of slider named sliderName; return false if error
  bool sliderValue(QString sliderName, double *value);
  
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
  
  
  /// static members are referenced by static functions whose pointers
  /// are passed to mbedit 
  static QPainter *staticPainter_;
  static QFontMetrics *staticFontMetrics_;
					 

public slots:

  /// TEST SLOT
  void cppSlot(const QString &msg) {
    qDebug() << "cppSlot() called with message: " << msg;
  }

  /// GUI item callbacks/slots
  void onXtrackSliderChanged();
  void onPingsShownSliderChanged();
  void onVerticalExaggSliderChanged();
  void onPingStepSliderChanged();				     

    
				  
private slots:

  void on_xtrackWidthSlider_sliderReleased(void);

  void on_nPingsShowSlider_sliderReleased(void);

  void on_vertExaggSlider_sliderReleased(void);  

  void on_actionOpen_swath_file_triggered(void);

  /// Ancillary data option slots
  void on_actionNone_triggered(void);
  void on_actionTime_triggered(void);
  void on_actionInterval_triggered(void);
  void on_actionLatitude_triggered(void);
  void on_actionLongitude_triggered(void);
  void on_actionHeading_triggered(void);
  void on_actionSpeed_triggered(void);
  void on_actionDepth_triggered(void);
  void on_actionAltitude_triggered(void);
  void on_actionSensor_depth_triggered(void);
  void on_actionRoll_triggered(void);
  void on_actionPitch_triggered(void);
  void on_actionHeave_triggered(void);

  /// Acoustic color coding slots
  void on_actionBottom_detect_algorithm_triggered(void);
  void on_actionPulse_source_triggered(void);
  void on_actionFlag_state_triggered(void);
  
  /// Plot-slice slots
  void on_actionWaterfall_2_triggered();  
  void on_actionAcross_track_2_triggered();
  void on_actionAlong_track_2_triggered();  
  
};
#endif // MAINWINDOW_H
