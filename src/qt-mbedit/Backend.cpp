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
#include <QFont>
#include "PixmapDrawer.h"
#include "Backend.h"
#include "GuiNames.h"

extern "C" {
#include "mb_status.h"
#include "mbedit_prog.h"
}

/* edit outbounds defines */
#define MBEDIT_OUTBOUNDS_NONE 0
#define MBEDIT_OUTBOUNDS_FLAGGED 1
#define MBEDIT_OUTBOUNDS_UNFLAGGED 2

mb_system::Emitter Backend::staticEmitter_;

using namespace mb_system;

Backend::Backend(int argc, char **argv) {

  dataPlotted_ = false;
  
  // Dummy first argument to drawing functions
  dummy_ = nullptr;
  
  const int width = 600;
  const int height = 600;
  canvasPixmap_ = new QPixmap(width, height);

  // mbedit uses this font:
  // "-*-fixed-bold-r-normal-*-13-*-75-75-c-70-iso8859-1");  
  painter_ = new QPainter(canvasPixmap_);
  QFont myFont("Helvetica [Cronyx]", 9);
  //  QFont myFont("-*-fixed-bold-r-normal-*-13-*-75-75-c-70-iso8859-1");  
  painter_->setFont(myFont);

  // Keep static reference to painter for use by static member functions
  PixmapDrawer::setPainter(painter_);

  /// Hard-coded in mbedit_callbacks; should loosen this restriction
  int cSize[4] = {0, 1016, 0, 525};  
  canvasSize(&cSize[1], &cSize[3]);
  mbedit_set_scaling(cSize, NO_ANCILL);
  int inputSpecd = 0;
  
  mbedit_init(argc, argv, &inputSpecd,
	      nullptr,
	      &PixmapDrawer::drawLine,
	      &PixmapDrawer::drawRect,
	      &PixmapDrawer::fillRect,
	      &PixmapDrawer::drawString,
	      &PixmapDrawer::justifyString,
	      &Backend::parseDataList,
	      &Backend::showError,
	      &Backend::showMessage,
	      &Backend::hideMessage,
	      &Backend::enableFileInput,
	      &Backend::disableFileInput,
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
  
}


Backend::~Backend()
{
  // Free unneeded memory
}


bool Backend::initialize(QObject *loadedRoot, int argc, char **argv) {
  ui_ = loadedRoot;

  // Find PixmapImage in QML object tree
  swathPixmapImage_ = 
    ui_->findChild<mb_system::PixmapImage*>(SWATH_PIXMAP_NAME);

  if (!swathPixmapImage_) {
    qCritical() << "Couldn't find " << SWATH_PIXMAP_NAME << " in QML";
    return false;
  }

  // Set the pixmap of QML-declared PixmapImage
  swathPixmapImage_->setImage(canvasPixmap_);  
  
  char *swathFile = nullptr;
  // Parse command line options
  for (int i = 1; i < argc; i++) {
    // Input swath file is last argument
    if (i == argc-1) {
      swathFile = argv[i];
    }
  }

  if (swathFile) {
    char *fullPath = realpath(swathFile, nullptr);
    if (!fullPath) {
      qWarning() << "swath file \"%s\" not found: " <<  swathFile;
      return false;
    }
    
    QString urlString("file://" + QString(fullPath));    
    if (!processSwathFile(urlString)) {
      qWarning() << "Couldn't process " << swathFile;
    }
  }
  else {
    plotTest();
  }

  return true;
}



void Backend::onMainWindowDestroyed() {
  qDebug() << "*** onMainWindowDestroyed() *****";
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

  // display data from selected file
  int status = mbedit_action_plot(xTrackWidth_,
				  verticalExagg_, xInterval_, yInterval_,
				  nPingsShown_, soundColorCoding_,
				  showFlagSounding_, showFlagProfile_,
				  plotAncillData_, &nBuffer_, &nGood_,
				  &iCurrent_, &mnPlot_);
  
  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_plot() failed\n";
    return false;
  }

  // Update GUI
  swathPixmapImage_->update();  

  return true;
}


bool Backend::processSwathFile(QUrl fileUrl) {

  qDebug() << "processSwathFile() " <<  fileUrl;
  char *swathFile = strdup(fileUrl.toLocalFile().toLatin1().data());  
  if (mbedit_get_format(swathFile, &format_) != MB_SUCCESS) {
    std::cerr << "Couldn't determine sonar format of " << swathFile
	      << "\n";

    return false;
  }

  qDebug() << "format_ #2: " << format_;
  int fileID = 0;
  int numFiles = 1;
  int saveMode = 0;
  int nDumped = 0;
  int nLoaded = 0;
  
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
				  &nBuffer_, &nGood_,
				  &iCurrent_, &mnPlot_);

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
  PixmapDrawer::fillRect(dummy_, 0, 0, canvasPixmap_->width(),
			 canvasPixmap_->height(),
			 WHITE, SOLID_LINE);

  PixmapDrawer::fillRect(dummy_, 100, 100,
			 canvasPixmap_->width()-200,
			 canvasPixmap_->height()-200,
			 RED, SOLID_LINE);  

  PixmapDrawer::drawLine(dummy_, 0, 0, canvasPixmap_->width(),
			 canvasPixmap_->height(),
			 BLACK, SOLID_LINE);

  PixmapDrawer::drawLine(dummy_, canvasPixmap_->width(), 0, 0,
			 canvasPixmap_->height(),
			 GREEN, DASH_LINE);  

  PixmapDrawer::drawString(dummy_, 100, 100, (char *)"this is coral",
			   CORAL, SOLID_LINE);

  PixmapDrawer::drawString(dummy_, 300, 100, (char *)"BLUE!",
			   BLUE, SOLID_LINE);


  PixmapDrawer::drawString(dummy_, 400, 100, (char *)"PURPLE",
			   PURPLE, SOLID_LINE);    

  // Update GUI
  qDebug() << "TBD: Update GUI";
  
  swathPixmapImage_->update();
  
  
  return true;
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
  else if (mode == INFO_EDIT_MODE) {
    editMode_ = INFO;
  }
  
  else {
    qWarning() << "Unknown edit mode: " << mode;
  }
}


void Backend::onLeftMouseButtonClicked(double x, double y) {
  qDebug() << "onLeftMouseButtonClicked()";
  int status = 0;
  /// bool ok = edit(x, y);
}

void Backend::onRightMouseButtonClicked(double x, double y) {
  qDebug() << "onRightMouseButtonClicked()";

}


void Backend::onLeftMouseButtonDown(double x, double y) {
  qDebug() << "onLeftMouseButtonDown()  x: " << x << ", y: " << y;

  if (editMode_ == GRAB) {
    // Start grabbing selected points
    int status =
      mbedit_action_mouse_grab(GRAB_START,
			       (int )x, (int )y, canvasPixmap_->width(),
			       verticalExagg_, xInterval_, yInterval_,
			       nPingsShown_, soundColorCoding_,
			       showFlagSounding_, showFlagProfile_,
			       plotAncillData_,
			       &nBuffer_,
			       &nGood_, &iCurrent_, &mnPlot_);
    plotSwath();
    return;
  }

  bool ok = edit(x, y);
}


void Backend::onLeftMouseButtonUp(double x, double y) {
  qDebug() << "onLeftMouseButtonUp()  x: " << x << ", y: " << y;  
  if (editMode_ == GRAB) {
    // Done grabbing points
    int status =
      mbedit_action_mouse_grab(GRAB_END,
			       (int )x, (int )y, canvasPixmap_->width(),
			       verticalExagg_, xInterval_, yInterval_,
			       nPingsShown_, soundColorCoding_,
			       showFlagSounding_, showFlagProfile_,
			       plotAncillData_,
			       &nBuffer_,
			       &nGood_, &iCurrent_, &mnPlot_);
  
    plotSwath();

    return;
  }
  
  bool ok = edit(x, y);
}

void Backend::onMouseMove(double x, double y) {
  qDebug() << "onMouseMove()";

  bool ok = edit(x, y);
}


bool Backend::edit(double x, double y) {
  int status = 0;

  qDebug() << "edit(): editMode_ = " << editMode_;

  switch (editMode_) {

  case TOGGLE:
    status = 
      mbedit_action_mouse_toggle((int )x, (int )y, canvasPixmap_->width(),
				 verticalExagg_, xInterval_, yInterval_,
				 nPingsShown_, soundColorCoding_,
				 showFlagSounding_, showFlagProfile_,
				 plotAncillData_,
				 &nBuffer_,
				 &nGood_, &iCurrent_, &mnPlot_);	    
    break;

  case INFO:
    status = 
      mbedit_action_mouse_info((int )x, (int )y, canvasPixmap_->width(),
			       verticalExagg_, xInterval_, yInterval_,
			       nPingsShown_, soundColorCoding_,
			       showFlagSounding_, showFlagProfile_,
			       plotAncillData_,
			       &nBuffer_,
			       &nGood_, &iCurrent_, &mnPlot_);
    break;

  case PICK:
    break;

  case GRAB:
    status = 
      mbedit_action_mouse_grab(GRAB_MOVE,
			       (int )x, (int )y, canvasPixmap_->width(),
			       verticalExagg_, xInterval_, yInterval_,
			       nPingsShown_, soundColorCoding_,
			       showFlagSounding_, showFlagProfile_,
			       plotAncillData_,
			       &nBuffer_,
			       &nGood_, &iCurrent_, &mnPlot_);

    break;

  case ERASE:
    status =
      mbedit_action_mouse_erase((int )x, (int )y, canvasPixmap_->width(),
				verticalExagg_, xInterval_, yInterval_,
				nPingsShown_, soundColorCoding_,
				showFlagSounding_, showFlagProfile_,
				plotAncillData_,
				&nBuffer_,
				&nGood_, &iCurrent_, &mnPlot_);
    break;

  case RESTORE:
    status = 
      mbedit_action_mouse_restore((int )x, (int )y,
				  canvasPixmap_->width(),
				  verticalExagg_, xInterval_, yInterval_,
				  nPingsShown_, soundColorCoding_,
				  showFlagSounding_, showFlagProfile_,
				  plotAncillData_, &nBuffer_,
				  &nGood_, &iCurrent_, &mnPlot_);    
      break;

  default:
    qWarning() << "Unsupported edit mode: " << editMode_;
  
  }
  // Update display (don't call plotSwath(), it will erase intemediate
  // graphics...)
  swathPixmapImage_->update();    
  
  if (status != MB_SUCCESS) {
    qWarning() << "Error while editing: " << status;
    return false;
  }
  return true;

}

