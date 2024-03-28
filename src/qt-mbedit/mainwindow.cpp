#include <iostream>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <QTextStream>

extern "C" {
#include "mb_status.h"
#include "mbedit_prog.h"
}

#define XG_SOLIDLINE 0
#define XG_DASHLINE 1

/* plot modes */
#define MBEDIT_PLOT_WIDE 0
#define MBEDIT_PLOT_TIME 1
#define MBEDIT_PLOT_INTERVAL 2
#define MBEDIT_PLOT_LON 3
#define MBEDIT_PLOT_LAT 4
#define MBEDIT_PLOT_HEADING 5
#define MBEDIT_PLOT_SPEED 6
#define MBEDIT_PLOT_DEPTH 7
#define MBEDIT_PLOT_ALTITUDE 8
#define MBEDIT_PLOT_SENSORDEPTH 9
#define MBEDIT_PLOT_ROLL 10
#define MBEDIT_PLOT_PITCH 11
#define MBEDIT_PLOT_HEAVE 12

/* view modes */
#define MBEDIT_VIEW_WATERFALL 0
#define MBEDIT_VIEW_ALONGTRACK 1
#define MBEDIT_VIEW_ACROSSTRACK 2
#define MBEDIT_SHOW_FLAG 0
#define MBEDIT_SHOW_DETECT 1
#define MBEDIT_SHOW_PULSE 2

/* edit outbounds defines */
#define MBEDIT_OUTBOUNDS_NONE 0
#define MBEDIT_OUTBOUNDS_FLAGGED 1
#define MBEDIT_OUTBOUNDS_UNFLAGGED 2

QPainter *MainWindow::staticPainter_ = nullptr;
QFontMetrics *MainWindow::staticFontMetrics_ = nullptr;
QString MainWindow::staticTextBuf_;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  dataPlotted_ = false;
  
  // Dummy first argument to drawing functions
  dummy_ = nullptr;
  
  ui->setupUi(this);
  canvas_ = new QPixmap(ui->swathCanvas->width(),
			ui->swathCanvas->height())
    ;

  painter_ = new QPainter(canvas_);

  // Keep static reference to painter for use by static member functions
  staticPainter_ = painter_;
  
  staticFontMetrics_ = new QFontMetrics(painter_->font());
  
  qDebug() << "CTR: swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();    


  int plotWidth = 0;
  
  mbedit_get_defaults(&plotSizeMax_, &plotSize_, &showMode_,
		      &showFlagSounding_, &showFlagProfile_,
		      &showTime_, &buffSizeMax_, &buffSize_, &holdSize_,
		      &format_, &plotWidth,  &verticalExagg_,
		      &xInterval_, &yInterval_,
		      firstDataTime_, &outMode_);

  qDebug() << "format_: " << format_;
  
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
  delete ui;
}



void MainWindow::on_xtrackWidthSlider_sliderMoved(int position)
{
  std::cerr << "xtrackWidth = " << position << "\n";
}


void MainWindow::on_nPingsShowSlider_sliderMoved(int position)
{
  std::cerr << "nPingsShown = " << position << "\n";
}

void MainWindow::on_vertExaggSlider_sliderReleased()
{
  std::cerr << "vertExagg Released!\n";

  // Can we get slider position?
  int position = ui->vertExaggSlider->sliderPosition();
  std::cerr << "read vertExagg = " << position << "\n";

  verticalExagg_ = position;

  if (dataPlotted_) {
    int nBuffer = 0;
    int nGood = 0;
    int iCurrent = 0;
    int nPlot = 0;
  
    // display data from selected file
    int status = mbedit_action_plot(canvas_->width(),
				    verticalExagg_, xInterval_, yInterval_,
				    plotSize_, showMode_,
				    showFlagSounding_, showFlagProfile_,
				    showTime_, &nBuffer, &nGood,
				    &iCurrent, &nPlot);
    if (status != MB_SUCCESS) {
      std::cerr << "mbedit_action_plot() failed\n";
      return;
    }
  }
}


void MainWindow::on_vertExaggSlider_sliderMoved(int position)
{
  std::cerr << "vertExagg = " << position << "\n";
  verticalExagg_ = position;

  // Can we get slider position?
  int pos = ui->vertExaggSlider->sliderPosition();
  std::cerr << "read vertExagg = " << pos << "\n";
  
}



void MainWindow::on_actionOpen_swath_file_triggered()
{
  std::cerr << "select swath file\n";
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open swath file"),
						  QDir::homePath(),
						  tr("swath files (*.m*)"));

  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *fname = (char *)utf8_text.c_str();
  std::cerr << "fname: " << fname << "\n";

  int currentFile = 0;
  int fileID = 0;
  int numFiles = 1;
  int saveMode = 1;
  int nDumped;
  int nLoaded;
  int nBuffer;
  int nGood;
  int iCurrent;
  int nPlot;

  qDebug() << "format_ #2: " << format_;
  
  int status = mbedit_action_open(fname,
				  format_,
				  fileID, numFiles, saveMode,
				  outMode_, canvas_->width(),
				  verticalExagg_, xInterval_, yInterval_,
				  plotSize_, showMode_,
				  showFlagSounding_,
				  showFlagProfile_, showTime_,
				  &buffSize_, &buffSizeMax_,
				  &holdSize_, &nDumped, &nLoaded,
				  &nBuffer, &nGood,
				  &iCurrent, &nPlot);

  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_open() failed\n";
    return;
  }

  // display data from selected file
  status = mbedit_action_plot(canvas_->width(),
			      verticalExagg_, xInterval_, yInterval_,
			      plotSize_, showMode_,
			      showFlagSounding_, showFlagProfile_,
			      showTime_, &nBuffer, &nGood,
			      &iCurrent, &nPlot);
  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_plot() failed\n";
    return;
  }
  
  // Add pixmap to UI label
  qDebug() << "Draw on GUI\n";
  ui->swathCanvas->setPixmap(*canvas_);
  dataPlotted_ = true;

  
}



bool MainWindow::plotTest() {
  qDebug() << "plot(): canvas width: " << canvas_->width() <<
    ", canvas height: " << canvas_->height();

  qDebug() << "plot(): swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();  

  painter_->eraseRect(0, 0, canvas_->width(), canvas_->height());

  //// TEST TEST TEST
  fillRect(dummy_, 0, 0, canvas_->width(), canvas_->height(),
	   WHITE, XG_SOLIDLINE);

  fillRect(dummy_, 100, 100,
	   canvas_->width()-200, canvas_->height()-200,
	   RED, XG_SOLIDLINE);  

  drawLine(dummy_, 0, 0, canvas_->width(), canvas_->height(),
	   BLACK, XG_SOLIDLINE);

  drawString(dummy_, 100, 100, (char *)"hello sailor!",
	     BLACK, XG_SOLIDLINE);
  
  return true;
}




void MainWindow::drawLine(void *dummy,
			  int x1, int y1, int x2, int y2,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);
  
  staticPainter_->drawLine(x1, y1, x2, y2);
}


void MainWindow::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  staticPainter_->drawRect(x, y, width, height);
}


void MainWindow::drawString(void *dummy, int x, int y, char *string,
			    mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);
  QTextStream(&staticTextBuf_) << string;
  staticPainter_->drawText(x, y, staticTextBuf_);
}


void MainWindow::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  const char *fillColor;
  
  switch (color) {
  case WHITE:
    fillColor = "white";
    break;

  case BLACK:
    fillColor = "black";
    break;

  case RED:
    fillColor = "red";
    break;

  case BLUE:
    fillColor = "blue";
    break;

  case CORAL:
    fillColor = "coral";
    break;

  case LIGHTGREY:
    fillColor = "lightGray";
    break;

  default:
    std::cerr << "xg_fillrectange(): unknown fill color!\n";
    fillColor = "white";
  }  
  staticPainter_->fillRect(x, y, width, height, fillColor);
}



void MainWindow::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  *width = staticFontMetrics_->width(string);
  *ascent = staticFontMetrics_->ascent();
  *descent = staticFontMetrics_->descent();
}


void MainWindow::setPenColorAndStyle(mbedit_color_t color, int style) {

  QPen pen = staticPainter_->pen();
  
  switch (color) {
  case WHITE:
    pen.setColor(Qt::white);
    break;

  case BLACK:
    pen.setColor(Qt::black);
    break;

  case RED:
    pen.setColor(Qt::red);
    break;

  case BLUE:
    pen.setColor(Qt::blue);
    break;

  case GREEN:
    pen.setColor(Qt::green);
    break;    

  case CORAL:
    pen.setColor("coral");
    break;

  case LIGHTGREY:
    pen.setColor("lightGray");
    break;

  default:
    std::cerr << "setPenColorAndStyle(): hey! unknown color: " << (int )color
	      << "\n";
  }


  if (style == XG_DASHLINE) {
    pen.setStyle(Qt::DashLine);
  }
  else {
    pen.setStyle(Qt::SolidLine);    
  }
  
}





void MainWindow::resetScaleXSlider(int width, int xMax,
				   int xInterval, int yInterval) {
  return;
}


