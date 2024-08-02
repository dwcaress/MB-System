#include <unistd.h>
#include <QDebug>
#include <QQmlContext>
#include "BackEnd.h"
#include "QVtkItem.h"
#include "SharedConstants.h"
#include "TopoColorMap.h"

using namespace sharedQmlCpp;
using namespace mb_system;

/// Initialize singleton to null
BackEnd *BackEnd::singleInstance_ = nullptr;

BackEnd::BackEnd(QQmlApplicationEngine *engine,
		 QObject *parent) : QObject(parent),
  qVtkItem_(nullptr)
{
    QObject *rootObject = engine->rootObjects().first();

    QObject::connect(rootObject, SIGNAL(sig(int, QString)),
                     this, SLOT(sigSlot(int, QString)));
    
    
    qVtkItem_ = rootObject->findChild<mb_system::QVtkItem *>("qVtkItem");
    if (!qVtkItem_) {
        qCritical() << "Could not find \"qVtkItem\" in QML";
        exit(1);
    }

    selectedFileItem_ = rootObject->findChild<QObject *>("selFile");    
    if (!selectedFileItem_) {
        qCritical() << "Could not find \"selectedFile\" in QML";
        exit(1);      
    }

    // Get colormap names to be displayed by QML GUI
    std::vector<const char *> colorMapNames;
    
    TopoColorMap::schemeNames(&colorMapNames);

    qDebug() << "ColorMaps:";
    for (int i = 0; i < colorMapNames.size(); i++) {
      qDebug() << colorMapNames[i];
      // Append name to QStringList
      colorMapsList_.append(colorMapNames[i]);
    }

    // Set color map names in model
    /// colorMapsModel_.setStringList(colorMapsList_);
    
}

bool BackEnd::registerSingleton(int argc, char **argv,
                                QQmlApplicationEngine *qmlEngine) {
    if (singleInstance_) {
        qInfo() << "BackEnd::registerSingleton(): Delete existing instance";
        delete singleInstance_;
    }
    singleInstance_ = new BackEnd(qmlEngine);

    bool error = false;
    for (int i = 1; i < argc; i++) {
        if ((!strcmp(argv[i], "-I") && i < argc-1) ||
                (i == argc -1 && argv[i][0] != '-')) {
            char *filename;
            if (i == argc-1) {
                // Last argument is grid file
                filename = argv[i];
            }
            else {
                // Argument following '-I' is grid file
                filename = argv[++i];
            }

            char *fullPath = realpath(argv[i], nullptr);
            if (!fullPath) {
                fprintf(stderr, "Grid file \"%s\" not found\n", filename);
                error = true;
                break;
            }

            QString urlstring("file://" + QString(fullPath));
            QUrl qUrl(urlstring);
            qDebug() << "registerSingleton(): urlstring - " << urlstring
                     << ", qUrl - " << qUrl;

            singleInstance_->setGridFile(qUrl);
            free((void *)fullPath);
        }
        else {
            fprintf(stderr, "Unknown/incomplete option: %s\n", argv[i]);
            error = true;
        }
    }
    if (error) {
        delete singleInstance_;
        singleInstance_ = nullptr;
        fprintf(stderr, "usage: %s [-I gridfile]\n", argv[0]);
        return false;
    }
    QQmlContext *rootContext = qmlEngine->rootContext();
    rootContext->setContextProperty("BackEnd", singleInstance_);
    return true;
}


bool BackEnd::setGridFile(QUrl fileURL) {

    qDebug() << "*** setGridFile() - " << fileURL;

    char *gridFilename = strdup(fileURL.toLocalFile().toLatin1().data());
    qVtkItem_->setGridFilename(gridFilename);
    qVtkItem_->update();

    selectedFileItem_->setProperty("text", fileURL.toLocalFile());

    return true;
}


void BackEnd::sigSlot(const int param, const QString &qval) {
  qDebug() << "sigSlot(): param=" << param << ", value=" << qval;
  QByteArray a;
  a.append(qval.toUtf8());
  char *value = a.data();

  switch (param) {

  case (int )Const::Cmd::VerticalExag:
    {
      float verticalExag = atof(value);
      std::cout << "vertical exagg: " << verticalExag << std::endl;
      qVtkItem_->setVerticalExagg(verticalExag);
      qVtkItem_->update();
    }
    break;

  case (int )Const::Cmd::ShowAxes:
    if (strstr(value, "true")) {
      qVtkItem_->showAxes(true);
    }
    else {
      qVtkItem_->showAxes(false);
    }
    qVtkItem_->update();        
    break;
    
  case (int )Const::Cmd::ColorMap:
    {
      char *schemeName = value;
      qDebug() << "colormap schemeName: " << schemeName;
      if (!qVtkItem_->setColorMapScheme(schemeName)) {
	qCritical() << "Unknown colormap scheme: " << schemeName;
      }
      else {
	qDebug() << "Set colormap scheme to " << schemeName;
      }
      qVtkItem_->update();
    }
    break;

  case (int )Const::Cmd::SiteFile:
    {
      qDebug() << "open site file " << value;
      char *siteFile = value;
      // Remove file URL prefix      
      siteFile += strlen("file://"); 
      qVtkItem_->setSiteFile(siteFile);
      qVtkItem_->update();
    }
    break;

  case (int )Const::Cmd::RouteFile:
    {
      qDebug() << "open route file " << value;
      char *routeFile = value;
      // Remove file URL prefix      
      routeFile += strlen("file://"); 
      /// qVtkItem_->setRouteFile(routeFile);
      qVtkItem_->update();
    }
    break;

  default:
    qCritical() << "Unhandled param: " << param;
  }

  return;
  
}

