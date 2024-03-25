#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QMainWindow>
#include <QPixmap>
#include <QPainter>

#include "/home/oreilly/MB-System/src/qt-guilib/SwathGridData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  /// Initialize app
  int mbedit_init(int argc, char **argv, int *startupFile);


protected:

  /// Get input file name and format 
  void parseDatalist(char *file, int form);
  
  /// Plot swath data
  bool plot(QPainter *painter, mb_system::SwathGridData *grid);

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
