#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QString>
#include <QColor>


#include "/home/oreilly/MB-System/src/qt-guilib/SwathGridData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



/// Colors
typedef enum {
  WHITE = 0,
  BLACK = 1,
  RED = 2,
  GREEN = 3,
  BLUE = 4,
  CORAL = 5,
  LIGHTGREY = 6,
} Color;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  /// Initialize app
  int mbedit_init(int argc, char **argv, bool *inputFileSpecd);


protected:

  /// Process and plot specified swath file
  bool processSwathfile(char *filename);
  
  /// Plot swath data
  bool plot(mb_system::SwathGridData *grid);

  /// Adaptation of mbedit_plot_all (mbedit_prog.c)
  int mbedit_plot(int plotWidth, int vExagg, int xInterval, int yInterval,
		  int plotSize, int showMode, int showTime,
		  bool autoScale);
  
  // Replacement for X11-based MB graphics functions
  void xg_drawline(void *dummy, int x1, int y1, int x2, int y2,
		   Color color, int style);
  
  void xg_drawrectangle(void *dummy, int x, int y, int width, int height,
			Color color, int style);
  
  void xg_fillrectangle(void *dummy, int x, int y, int width, int height,
			Color color, int style);

  void xg_drawstring(void *dummy, int x, int y, char *string,
		     Color color, int style);
  
  void xg_justify(void *dummy, char *string, int *width,
		  int *ascent, int *descent);


  /// Set QPainter pen color and style
  void setPenColorAndStyle(Color color, int style);


  /// Dummy first argument to xg_ member funtions
  void *dummy_;

  /// Used within xg_ member functions
  QString qTextBuf_;

  /// Used witin xg_ member functions
  QColor qColorBuf_;
    
  int verbose_;
  int lonflip_;
  bool uselockfiles_;
  int output_mode_;
  bool gui_mode_;
  bool run_mbprocess_;
  int error_;

  
  // MBIO control parameters
  int format_;
  int pings_;
  double bounds_[4];
  int btime_i_[7];
  int etime_i_[7];
  double btime_d_;
  double etime_d_;
  double speedmin_;
  double timegap_;

  char inputFilename_[256];

  
  mb_system::SwathGridData grid_;
  
  QPixmap *canvas_;
  QPainter *painter_;

  static const char help_message_[];
  
  static const char usage_message_[];
		   
		      
private slots:

    void on_xtrackWidthSlider_sliderMoved(int position);

    void on_nPingsShowSlider_sliderMoved(int position);

    void on_vertExaggSlider_sliderMoved(int position);

    void on_actionOpen_swath_file_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
