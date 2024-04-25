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
#include "Backend.h"
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

QPainter *Backend::staticPainter_ = nullptr;
QFontMetrics *Backend::staticFontMetrics_ = nullptr;
QString Backend::staticTextBuf_;

/// using namespace mb_system;

Backend::Backend(QObject *rootObject, int argc, char **argv) {

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
	      &Backend::drawLine,
	      &Backend::drawRect,
	      &Backend::fillRect,
	      &Backend::drawString,
	      &Backend::justifyString,
	      &Backend::parseDataList,
	      &Backend::showError,
	      &Backend::showMessage,
	      &Backend::hideMessage,
	      &Backend::enableFileButton,
	      &Backend::disableFileButton,
	      &Backend::enableNextButton,
	      &Backend::disableNextButton,
	      &Backend::resetScaleX);
  
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

Backend::~Backend()
{
  // Free unneeded memory
}



void Backend::testSlot() {
  qDebug() << "*** testSlot() *****";
}


void Backend::onAncillDataChanged(QString msg) {
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


void Backend::onSliceChanged(QString slice) {
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


void Backend::onColorCodeChanged(QString code) {
  if (code == BOTTOM_DETECT_COLOR) {
    soundColorCoding_ = DETECT;
  }
  else if (code == PULSE_SOURCE_COLOR) {
    soundColorCoding_ = PULSE;
  }
  else if (code == FLAG_STATE_COLOR) {
    soundColorCoding_ = FLAG;    
  }
  else {
    qWarning() << "Unknown color code option: " << code;    
  }

  plotSwath();
}


bool Backend::plotSwath(void) {
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


bool Backend::processSwathFile(char *swathFile) {
  
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


bool Backend::plotTest() {
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




void Backend::drawLine(void *dummy,
			  int x1, int y1, int x2, int y2,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);
  
  staticPainter_->drawLine(x1, y1, x2, y2);
}


void Backend::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  staticPainter_->drawRect(x, y, width, height);
}


void Backend::drawString(void *dummy, int x, int y, char *string,
			    mbedit_color_t color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  staticPainter_->drawText(x, y, textBuf);
}


void Backend::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  staticPainter_->fillRect(x, y, width, height, colorName(color));
}



void Backend::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  *width = staticFontMetrics_->width(string);
  *ascent = staticFontMetrics_->ascent();
  *descent = staticFontMetrics_->descent();
}

const char *Backend::colorName(mbedit_color_t color) {
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



void Backend::setPenColorAndStyle(mbedit_color_t color, int style) {


  if (style == XG_DASHLINE) {
    staticPainter_->setPen(Qt::DashLine);
  }
  else {
    staticPainter_->setPen(Qt::SolidLine);    
  }
  staticPainter_->setPen(colorName(color));
  
}





void Backend::resetScaleXSlider(int width, int xMax,
				   int xInterval, int yInterval) {
  return;
}


void Backend::onXtrackChanged(double value) {
  qDebug() << "onXtrackChanged() value: " << value;
  xTrackWidth_ = (int )value;
  plotSwath();
  return;
}

void Backend::onPingsShownChanged(double value) {
  qDebug() << "onPingsShownChanged() value: " << value;  
  nPingsShown_ = (int )value;
  plotSwath();
  return;
}

void Backend::onVerticalExaggChanged(double value) {
  qDebug() << "onVerticalExaggChanged() value: " << value;
  verticalExagg_ = (int )value * 100;
  plotSwath();
  return;
}

void Backend::onPingStepChanged(double value) {
  qDebug() << "onPingStepChanged() value: " << value <<
    " TBD: set member variable!";
  plotSwath();
  return;
}


void Backend::onEditModeChanged(QString mode) {
  qDebug() << "onEditModeChanged(): " << mode;

  if (mode == TOGGLE_EDIT_MODE) {
    editMode_ = TOGGLE;
  }
  else if (mode == PICK_EDIT_MODE) {
    editMode_ = PICK;
  }
  else if (mode == ERASE_EDIT_MODE) {
    editMode_ = ERASE;
  }
  else if (mode == RESTORE_EDIT_MODE) {
    editMode_ = RESTORE;
  }
  else if (mode == GRAB_EDIT_MODE) {
    editMode_ = GRAB;
  }
  else {
    qWarning() << "Unknown edit mode: " << mode;
  }
}


void Backend::onLeftMouseButtonClicked() {
  qDebug() << "onLeftMouseButtonClicked()";
}

void Backend::onRightMouseButtonClicked() {
  qDebug() << "onRightMouseButtonClicked()";  
}

