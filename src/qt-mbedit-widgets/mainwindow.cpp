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

/* edit outbounds defines */
#define MBEDIT_OUTBOUNDS_NONE 0
#define MBEDIT_OUTBOUNDS_FLAGGED 1
#define MBEDIT_OUTBOUNDS_UNFLAGGED 2

QPainter *MainWindow::staticPainter_ = nullptr;
QFontMetrics *MainWindow::staticFontMetrics_ = nullptr;
QString MainWindow::staticTextBuf_;

// using namespace mb_system;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  dataPlotted_ = false;
  
  // Dummy first argument to drawing functions
  dummy_ = nullptr;
  
  ui->setupUi(this);
  canvas_ = new QPixmap(ui->swathCanvas->width(),
			ui->swathCanvas->height());

  painter_ = new QPainter(canvas_);

  // Keep static reference to painter for use by static member functions
  staticPainter_ = painter_;
  
  staticFontMetrics_ = new QFontMetrics(painter_->font());
  
  qDebug() << "CTR: swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();    

  /* ***
     qtdesigner for Qt Widgets cannot add mutually exclusive menu items 
     in a submenu!!! LAME.
     C++ code must add actions to a QActionGroup after the actions have been 
     created by qtdesigner

     In the MainWindow constructor, create ana action group, add the actions 
     to it, and set it exclusive:

      groupTool = new QActionGroup(this);
      groupTool.addAction(ui->actionUsePointer);
      ...
      groupTool.setExclusive(true);

     In Qt Designer:

     * set the checkable property to true for each of the actions
     * set the checked property to true for one of the actions
 *** */
  QActionGroup *group = new QActionGroup(this);
  group->addAction(ui->actionNone);
  group->addAction(ui->actionTime);
  group->addAction(ui->actionInterval);
  group->addAction(ui->actionLatitude);
  group->addAction(ui->actionLongitude);
  group->addAction(ui->actionHeading);
  group->addAction(ui->actionSpeed);
  group->addAction(ui->actionDepth);
  group->addAction(ui->actionAltitude);
  group->addAction(ui->actionSensor_depth);
  group->addAction(ui->actionRoll);
  group->addAction(ui->actionPitch);
  group->addAction(ui->actionHeave);    
  group->setExclusive(true);

  group = new QActionGroup(this);
  group->addAction(ui->actionWaterfall_2);
  group->addAction(ui->actionAlong_track_2);
  group->addAction(ui->actionAcross_track_2);
  group->setExclusive(true);

  group = new QActionGroup(this);
  group->addAction(ui->actionBottom_detect_algorithm);
  group->addAction(ui->actionPulse_source);
  group->addAction(ui->actionFlag_state);
  group->setExclusive(true);
  
  mbedit_get_defaults(&maxPingsShown_, &nPingsShown_, &soundColorCoding_,
		      &showFlagSounding_, &showFlagProfile_,
		      &plotAncillData_, &buffSizeMax_, &buffSize_, &holdSize_,
		      &format_, &xTrackWidth_,  &verticalExagg_,
		      &xInterval_, &yInterval_,
		      firstDataTime_, &outMode_);

  qDebug() << "format_: " << format_;

  // Set sliders to default values
  ui->vertExaggSlider->setSliderPosition(verticalExagg_/100);  
  ui->nPingsShowSlider->setSliderPosition(nPingsShown_);
  ui->xtrackWidthSlider->setSliderPosition(xTrackWidth_);  

  plotTest();
  
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
  delete ui;
}



void MainWindow::on_xtrackWidthSlider_sliderReleased()
{
  std::cerr << "xtrackWidth released\n";
  xTrackWidth_ = ui->xtrackWidthSlider->sliderPosition();
  std::cerr << "xTrackWidth = " << xTrackWidth_ << "\n";
  plotSwath();
}


void MainWindow::on_nPingsShowSlider_sliderReleased()
{
  std::cerr << "nPingsShown released\n";
  nPingsShown_ = ui->nPingsShowSlider->sliderPosition();
  std::cerr << "read nPingsShow = " << nPingsShown_ << "\n";
  plotSwath();
}


void MainWindow::on_vertExaggSlider_sliderReleased()
{
  std::cerr << "vertExagg Released!\n";

  // Get slider position
  int position = ui->vertExaggSlider->sliderPosition();
  std::cerr << "read vertExagg = " << position << "\n";

  // NOTE: scaled by 100x
  verticalExagg_ = position * 100;

  std::cerr << "dataPlotted_ = " << dataPlotted_ << "\n";

  plotSwath();

}



void MainWindow::on_actionNone_triggered() {
  std::cerr << "actionNone triggered!\n";
  plotAncillData_ = NO_ANCILL;
  plotSwath();  
}

void MainWindow::on_actionTime_triggered() {
  std::cerr << "actionTime triggered!\n";
  plotAncillData_ = TIME;
  plotSwath();  
}

void MainWindow::on_actionInterval_triggered() {
  std::cerr << "actionInterval triggered!\n";
  plotAncillData_ = INTERVAL;
  plotSwath();  
}

void MainWindow::on_actionLatitude_triggered() {
  std::cerr << "actionLatitude triggered!\n";
  plotAncillData_ = LATITUDE;
  plotSwath();  
}

void MainWindow::on_actionLongitude_triggered() {
  std::cerr << "actionLongitude triggered!\n";
  plotAncillData_ = LONGITUDE;
  plotSwath();  
}

void MainWindow::on_actionHeading_triggered() {
  std::cerr << "actionHeading triggered!\n";
  plotAncillData_ = HEADING;
  plotSwath();  
}

void MainWindow::on_actionSpeed_triggered() {
  std::cerr << "actionSpeed triggered!\n";
  plotAncillData_ = SPEED;
  plotSwath();  
}

void MainWindow::on_actionDepth_triggered() {
  std::cerr << "actionDepth triggered!\n";
  plotAncillData_ = DEPTH;
  plotSwath();  
}

void MainWindow::on_actionAltitude_triggered() {
  std::cerr << "actionAltitude triggered!\n";
  plotAncillData_ = ALTITUDE;
  plotSwath();
}

void MainWindow::on_actionSensor_depth_triggered() {
  std::cerr << "actionSensor_depth triggered!\n";
  plotAncillData_ = SPEED;
  plotSwath();  
}

void MainWindow::on_actionRoll_triggered() {
  std::cerr << "actionRoll triggered!\n";
  plotAncillData_ = ROLL;
  plotSwath();
}

void MainWindow::on_actionPitch_triggered() {
  std::cerr << "actionPitch triggered!\n";
  plotAncillData_ = PITCH;
  plotSwath();  
}

void MainWindow::on_actionHeave_triggered() {
  std::cerr << "actionHeave triggered!\n";
  plotAncillData_ = HEAVE;
  plotSwath();  
}


                 
void MainWindow::on_actionBottom_detect_algorithm_triggered() {
  std::cerr << "action bottom detected triggered\n";
  soundColorCoding_ = DETECT;
  plotSwath();
}


void MainWindow::on_actionPulse_source_triggered() {
  std::cerr << "action pulse source detected triggered\n";  
  soundColorCoding_ = PULSE;
  plotSwath();
}


void MainWindow::on_actionFlag_state_triggered() {
  std::cerr << "action flag state detected triggered\n";    
  soundColorCoding_ = FLAG;
  plotSwath();
}


void MainWindow::on_swathCanvas_labelMouseEvent(QMouseEvent *event) {
  std::cerr << "mouse event on swathCanvas: ";
  switch (event->type()) {
  case QEvent::MouseButtonPress:
    std::cerr << "mouse button pressed\n";
    break;

  case QEvent::MouseButtonRelease:
    std::cerr << "mouse button released\n";
    break;

  case QEvent::MouseMove:
    std::cerr << "mouse moved\n";
    break;

  default:
    std::cerr << "unhandled mouse event\n";
  }
}


bool MainWindow::plotSwath(void) {
  if (!dataPlotted_) {
    std::cerr << "No data plotted yet\n";
    return false;
  }

  int nBuffer = 0;
  int nGood = 0;
  int iCurrent = 0;
  int nPlot = 0;
  
  // display data from selected file
  int status = mbedit_action_plot(xTrackWidth_,
				  verticalExagg_, xInterval_, yInterval_,
				  nPingsShown_, soundColorCoding_,
				  showFlagSounding_, showFlagProfile_,
				  plotAncillData_, &nBuffer, &nGood,
				  &iCurrent, &nPlot);
  
  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_plot() failed\n";
    return false;
  }

  // Update GUI
  ui->swathCanvas->setPixmap(*canvas_);  

  return true;
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
  char *swathFile = (char *)utf8_text.c_str();
  std::cerr << "swathFile: " << swathFile << "\n";

  if (mbedit_get_format(swathFile, &format_) != MB_SUCCESS) {
    std::cerr << "Couldn't determine sonar format of " << swathFile
	      << "\n";

    return;
  }

  qDebug() << "format_ #2: " << format_;
  int currentFile = 0;
  int fileID = 0;
  int numFiles = 1;
  int saveMode = 0;
  int nDumped = 0;
  int nLoaded = 0;
  int nBuffer = 0;
  int nGood = 0;
  int iCurrent = 0;
  int nPlot = 0;

  
  // Open swath file and plot data
  int status = mbedit_action_open(swathFile,
				  format_,
				  fileID, numFiles, saveMode,
				  outMode_, canvas_->width(),
				  verticalExagg_, xInterval_, yInterval_,
				  nPingsShown_, soundColorCoding_,
				  showFlagSounding_,
				  showFlagProfile_, plotAncillData_,
				  &buffSize_, &buffSizeMax_,
				  &holdSize_, &nDumped, &nLoaded,
				  &nBuffer, &nGood,
				  &iCurrent, &nPlot);

  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_open() failed\n";
    return;
  }
  
  // Add pixmap to UI label
  qDebug() << "Draw on GUI\n";
  // Update GUI
  ui->swathCanvas->setPixmap(*canvas_);
  dataPlotted_ = true;
}



void MainWindow::on_actionWaterfall_2_triggered() {
  qDebug() << "on_actionWaterfall_triggered()";
  mbedit_set_viewmode(WATERFALL);
  plotSwath();
}

void MainWindow::on_actionAcross_track_2_triggered() {
  qDebug() << "on_actionAcross_track_triggered()";
  mbedit_set_viewmode(ACROSSTRACK);
  plotSwath();
}

void MainWindow::on_actionAlong_track_2_triggered() {
  qDebug() << "on_actionAlong_track_triggered()";
  mbedit_set_viewmode(ALONGTRACK);
  plotSwath();
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

  drawLine(dummy_, canvas_->width(), 0, 0, canvas_->height(),
	   GREEN, XG_DASHLINE);  

  drawString(dummy_, 100, 100, (char *)"hello sailor!",
	     BLACK, XG_SOLIDLINE);

  drawString(dummy_, 300, 100, (char *)"BLUE!",
	     BLUE, XG_SOLIDLINE);


  drawString(dummy_, 400, 100, (char *)"GREEN",
	     GREEN, XG_SOLIDLINE);    

  // Update GUI
  ui->swathCanvas->setPixmap(*canvas_);
  
  return true;
}




void MainWindow::drawLine(void *dummy,
			  int x1, int y1, int x2, int y2,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);
  
  staticPainter_->drawLine(x1, y1, x2, y2);
}


void MainWindow::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  staticPainter_->drawRect(x, y, width, height);
}


void MainWindow::drawString(void *dummy, int x, int y, char *string,
			    DrawingColor color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  /// staticPainter_->setPen(colorName(color));  //// TEST TEST TEST
  staticPainter_->drawText(x, y, textBuf);
}


void MainWindow::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  staticPainter_->fillRect(x, y, width, height, colorName(color));
}



void MainWindow::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  *width = staticFontMetrics_->width(string);
  *ascent = staticFontMetrics_->ascent();
  *descent = staticFontMetrics_->descent();
}

const char *MainWindow::colorName(DrawingColor color) {
  switch (color) {
  case WHITE:
    return "white";

  case BLACK:
    return "black";

  case RED:
    return "red";

  case GREEN:
    return "green";
    
  case BLUE:
    return "blue";

  case CORAL:
    return "coral";

  case LIGHTGREY:
    return "lightGray";

  default:
    std::cerr << "colorName(): unknown fill color!\n";
    return "black";
  }  

}



void MainWindow::setPenColorAndStyle(DrawingColor color, int style) {


  if (style == XG_DASHLINE) {
    staticPainter_->setPen(Qt::DashLine);
  }
  else {
    staticPainter_->setPen(Qt::SolidLine);    
  }
  staticPainter_->setPen(colorName(color));
  
}





void MainWindow::resetScaleXSlider(int width, int xMax,
				   int xInterval, int yInterval) {
  return;
}


