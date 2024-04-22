#include <iostream>
#include <QFileDialog>
#include <QQuickItem>
#include <QDebug>
#include <QDir>
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
#include <QMetaObject>
#include <QQmlProperty>
#include "MainWindow.h"
#include "GuiNames.h"

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

/// using namespace mb_system;

MainWindow::MainWindow(QObject *rootObject, int argc, char **argv) {

  ui_ = rootObject;
  dataPlotted_ = false;
  
  // Dummy first argument to drawing functions
  dummy_ = nullptr;
  
  const int width = 600;
  const int height = 600;
  canvasPixmap_ = new QPixmap(width, height);

  painter_ = new QPainter(canvasPixmap_);

  // Keep static reference to painter for use by static member functions
  staticPainter_ = painter_;

  // Find PixmapImage in QML object tree
  swathPixmapImage_ = 
    ui_->findChild<PixmapImage*>(SWATH_PIXMAP_NAME);

  if (!swathPixmapImage_) {
    qCritical() << "Couldn't find " << SWATH_PIXMAP_NAME << " in QML";
  }
  
  pixmapContainer_.pixmap = canvasPixmap_;
  swathPixmapImage_->setImage(&pixmapContainer_);
  
  staticFontMetrics_ = new QFontMetrics(painter_->font());

  int cSize[4] = {0, 0, 0, 0};
  canvasSize(&cSize[1], &cSize[3]);
  int inputSpecd = 0;
  mbedit_set_scaling(cSize, NO_ANCILL);
  
  mbedit_init(argc, argv, &inputSpecd,
	      nullptr,
	      &MainWindow::drawLine,
	      &MainWindow::drawRect,
	      &MainWindow::fillRect,
	      &MainWindow::drawString,
	      &MainWindow::justifyString,
	      &MainWindow::parseDataList,
	      &MainWindow::showError,
	      &MainWindow::showMessage,
	      &MainWindow::hideMessage,
	      &MainWindow::enableFileButton,
	      &MainWindow::disableFileButton,
	      &MainWindow::enableNextButton,
	      &MainWindow::disableNextButton,
	      &MainWindow::resetScaleX);
  
  mbedit_get_defaults(&maxPingsShown_, &nPingsShown_, &soundColorCoding_,
		      &showFlagSounding_, &showFlagProfile_,
		      &plotAncillData_, &buffSizeMax_, &buffSize_, &holdSize_,
		      &format_, &xTrackWidth_,  &verticalExagg_,
		      &xInterval_, &yInterval_,
		      firstDataTime_, &outMode_);

  qDebug() << "format_: " << format_;

  // Set GUI items to default values
  qDebug() << "TBD: Set sliders to default values";
  /* ***
  ui->vertExaggSlider->setSliderPosition(verticalExagg_/100);  
  ui->nPingsShowSlider->setSliderPosition(nPingsShown_);
  ui->xtrackWidthSlider->setSliderPosition(xTrackWidth_);  
  *** */
  char *swathFile = nullptr;
  // Parse command line options
  for (int i = 1; i < argc; i++) {
    // Input swath file is last argument
    if (i == argc-1) {
      swathFile = argv[i];
    }
  }

  if (swathFile) {
    if (!processSwathFile(swathFile)) {
      qWarning() << "Couldn't process " << swathFile;
    }
  }
  else {
    plotTest();
  }
  
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
}



void MainWindow::testSlot() {
  qDebug() << "*** testSlot() *****";
}


void MainWindow::onAncillDataChanged(QString msg) {
  qDebug() << "*** onAncillDataChanged() msg: " << msg;

  if (msg == NONE_ANCILLDATA) {
    plotAncillData_ = NO_ANCILL;
  }
  else if (msg == TIME_ANCILLDATA) {
    plotAncillData_ = TIME;
  }
  else if (msg == INTERVAL_ANCILLDATA) {
    plotAncillData_ = INTERVAL;
  }
  else if (msg == LATITUDE_ANCILLDATA) {
    plotAncillData_ = LATITUDE;
  }
  else if (msg == LONGITUDE_ANCILLDATA) {
    plotAncillData_ = LONGITUDE;
  }
  else if (msg == HEADING_ANCILLDATA) {
    plotAncillData_ = HEADING;
  }
  else if (msg == SPEED_ANCILLDATA) {
    plotAncillData_ = SPEED;
  }
  else if (msg == DEPTH_ANCILLDATA) {
    plotAncillData_ = DEPTH;
  }
  else if (msg == ALTITUDE_ANCILLDATA) {
    plotAncillData_ = ALTITUDE;
  }
  else if (msg == SENSORDEPTH_ANCILLDATA) {
    plotAncillData_ = SENSORDEPTH;
  }
  else if (msg == ROLL_ANCILLDATA) {
    plotAncillData_ = ROLL;
  }
  else if (msg == PITCH_ANCILLDATA) {
    plotAncillData_ = PITCH;
  }
  else if (msg == HEAVE_ANCILLDATA) {
    plotAncillData_ = HEAVE;
  }
  else {
    qWarning() << "Unknown ancillary data selected: " << msg;
  }

  plotSwath();
}


void MainWindow::onSliceChanged(QString slice) {
  qDebug() << "onSliceChanged(): " << slice;
  if (slice == ALONGTRACK_SLICE) {
    sliceMode_ = ALONGTRACK;
  }
  else if (slice == CROSSTRACK_SLICE) {
    sliceMode_ = ACROSSTRACK;
  }
  else if (slice == WATERFALL_SLICE) {
    sliceMode_ = WATERFALL;
  }
  else {
    qWarning() << "Unknown slice option: " << slice;
  }

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
  swathPixmapImage_->update();  

  return true;
}

/* **
void MainWindow::on_actionOpen_swath_file_triggered()
{
  qDebug() << "select swath file\n";

  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open swath file"),
						  QDir::homePath(),
						  tr("swath files (*.m*)"));
  QString fileName("dummy");
  
  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *swathFile = (char *)utf8_text.c_str();
  std::cerr << "swathFile: " << swathFile << "\n";
  if (!processSwathFile(swathFile)) {
    qWarning() << "Couldn't process " << swathFile;
  }
  return;
}
*** */

bool MainWindow::processSwathFile(char *swathFile) {
  
  if (mbedit_get_format(swathFile, &format_) != MB_SUCCESS) {
    std::cerr << "Couldn't determine sonar format of " << swathFile
	      << "\n";

    return false;
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
				  outMode_, canvasPixmap_->width(),
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
    return false;
  }
  
  // Add pixmap to UI label
  qDebug() << "TBD: Draw on GUI\n";

  
  // Update GUI
  swathPixmapImage_->update();
  
  dataPlotted_ = true;
  return true;
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
  qDebug() << "plotTest(): canvas width: " << canvasPixmap_->width() <<
    ", canvas height: " << canvasPixmap_->height();

  painter_->eraseRect(0, 0, canvasPixmap_->width(), canvasPixmap_->height());

  //// TEST TEST TEST
  fillRect(dummy_, 0, 0, canvasPixmap_->width(), canvasPixmap_->height(),
	   WHITE, XG_SOLIDLINE);

  fillRect(dummy_, 100, 100,
	   canvasPixmap_->width()-200, canvasPixmap_->height()-200,
	   RED, XG_SOLIDLINE);  

  drawLine(dummy_, 0, 0, canvasPixmap_->width(), canvasPixmap_->height(),
	   BLACK, XG_SOLIDLINE);

  drawLine(dummy_, canvasPixmap_->width(), 0, 0, canvasPixmap_->height(),
	   GREEN, XG_DASHLINE);  

  drawString(dummy_, 100, 100, (char *)"hello sailor!",
	     BLACK, XG_SOLIDLINE);

  drawString(dummy_, 300, 100, (char *)"BLUE!",
	     BLUE, XG_SOLIDLINE);


  drawString(dummy_, 400, 100, (char *)"GREEN",
	     GREEN, XG_SOLIDLINE);    

  // Update GUI
  qDebug() << "TBD: Update GUI";
  
  swathPixmapImage_->update();
  
  
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

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  staticPainter_->drawText(x, y, textBuf);
}


void MainWindow::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

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

const char *MainWindow::colorName(mbedit_color_t color) {
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



void MainWindow::setPenColorAndStyle(mbedit_color_t color, int style) {


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


void MainWindow::onXtrackSliderChanged() {
  qDebug() << "onXtrackSliderChanged() slot";
  double value;
  if (!sliderValue(XTRACK_SLIDER_NAME, &value)) {
    qWarning() << "Couldn't get value of slider " << XTRACK_SLIDER_NAME;
    return;
  }
  qDebug() << "onXtrackSliderChanged(): value=" << value;
  xTrackWidth_ = (int )value;
  plotSwath();
  return;
}


void MainWindow::onPingsShownSliderChanged() {
  qDebug() << "onPingsShownSliderChanged() slot";
  double value;
  if (!sliderValue(PINGS_SHOWN_SLIDER_NAME, &value)) {
    qWarning() << "Couldn't get value of slider "
	       << PINGS_SHOWN_SLIDER_NAME;
    return;
  }
  qDebug() << "onPingsShownSliderChanged(): value=" << value;
  nPingsShown_ = (int )value;
  
  plotSwath();
  return;
}


void MainWindow::onVerticalExaggSliderChanged() {
  qDebug() << "onVerticalExaggSliderChanged() slot";
  double value;
  if (!sliderValue(VERTICAL_EXAGG_SLIDER_NAME, &value)) {
    qWarning() << "Couldn't get value of slider "
	       << VERTICAL_EXAGG_SLIDER_NAME;
    return;
  }
  qDebug() << "onVerticalExaggSliderChanged(): value=" << value;
  // Note: scaled by x100
  verticalExagg_ = (int )value * 100;
  plotSwath();
  return;
}



void MainWindow::onPingStepSliderChanged() {
  qDebug() << "onPingStepSliderChanged() slot";
  double value;
  if (!sliderValue(PING_STEP_SLIDER_NAME, &value)) {
    qWarning() << "Couldn't get value of slider "
	       << PING_STEP_SLIDER_NAME;
    return;
  }
  qDebug() << "onPingStepSliderChanged(): value=" << value;
  plotSwath();
  return;
}


bool MainWindow::sliderValue(QString sliderName, double *value) {

    QQuickItem *slider =
      ui_->findChild<QQuickItem*>(sliderName);

    if (!slider) {
      qWarning() << "sliderValue() couldn't find slider named " << sliderName;
      return false;
    }
    
    bool ok;
    
    QVariant qvar = slider->property("position");
    double position = qvar.toDouble(&ok);

    if (!ok) {
      qDebug() << "sliderValue(): couldn't get position from QVariant";
      return false;
    }
    qDebug() << sliderName << " sliderValue() position = " << qvar <<
      " (" << position << ")";

    QMetaObject::invokeMethod(slider, "valueAt",
			      Q_RETURN_ARG(double, *value),
			      Q_ARG(double, position));


    return true;
}


void MainWindow::onEditModeChanged(QString msg) {
  qDebug() << "onEditModeChanged(): " << msg;
}
