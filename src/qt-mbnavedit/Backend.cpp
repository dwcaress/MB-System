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
#include "mb_define.h"
  
#define MBNAVEDIT_DECLARE_GLOBALS
#include "mbnavedit_prog.h"
}

mb_system::Emitter Backend::staticEmitter_;

using namespace mb_system;

Backend::Backend(int argc, char **argv) {

  dataPlotted_ = false;
  
  // Dummy first argument to drawing functions
  dummy_ = nullptr;
  
  const int width = 600;
  const int height = 600;
  canvasPixmap_ = new QPixmap(width, height);

  painter_ = new QPainter(canvasPixmap_);
  QFont myFont("Helvetica [Cronyx]", 9);
  painter_->setFont(myFont);

  // Set PixmapDrawer painter
  PixmapDrawer::setPainter(painter_);

  /// Hard-coded in mbedit_callbacks; should loosen this restriction
  int cSize[4] = {0, 1016, 0, 525};  
  canvasSize(&cSize[1], &cSize[3]);
  bool inputSpecd = false;

  mbnavedit_init_globals();
  
  mbnavedit_init(argc, argv, &inputSpecd,
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
		 &Backend::setUiElements);


  // Set default values here
  /// mbnavedit_get_defaults();
  qWarning("Get member defaults here");
}


Backend::~Backend()
{
  // Free unneeded memory
}


bool Backend::initialize(QObject *loadedRoot, int argc, char **argv) {
  ui_ = loadedRoot;

  // Find PixmapImage in QML object tree
  qDebug() << "Find PixmapImage " << SWATH_PIXMAP_NAME;
  
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


bool Backend::plotSwath(void) {
  if (!dataPlotted_) {
    std::cerr << "No data plotted yet\n";
    return false;
  }

  qDebug() << "invoke mbnavedit function to plot data";

  int status = MB_SUCCESS;
  
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
  int format;
  int formatErr;
  if (!mb_get_format(0, swathFile, NULL, &format, &formatErr)) {
    std::cerr << "Couldn't determine sonar format of " << swathFile
	      << "\n";

    return false;
  }

  qDebug() << "Invoke mbnavedit_prog functions to open and plot data";

  // Open swath file and plot data
  mbnavedit_set_inputfile(swathFile);
  int status = mbnavedit_action_open(false);
  
  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_action_open() failed\n";
    return false;
  }
  
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


