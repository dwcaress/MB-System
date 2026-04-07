#include <unistd.h>
#include <QDebug>
#include <QQmlContext>
#include "BackEnd.h"
#include "TopoColorMap.h"

using namespace sharedQmlCpp;
using namespace mb_system;

/// Initialize singleton to null
BackEnd *BackEnd::singleInstance_ = nullptr;

BackEnd::BackEnd(QQmlApplicationEngine *engine,
		 QObject *parent) : QObject(parent),
  topoGridItem_(nullptr)
{

    QObject *rootObject = engine->rootObjects().first();

    QObject::connect(rootObject, SIGNAL(sig(int, QString)),
                     this, SLOT(sigSlot(int, QString)));
    
    
    topoGridItem_ =
      rootObject->findChild<mb_system::TopoGridItem *>("topoGridItem");
    if (!topoGridItem_) {
        qCritical() << "Could not find \"topoGridItem\" in QML";
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
      topoGridItem_->setVerticalExagg(verticalExag);
      topoGridItem_->update();
    }
    break;

  case (int )Const::Cmd::ShowAxes:
    if (strstr(value, "true")) {
      topoGridItem_->showAxes(true);
    }
    else {
      topoGridItem_->showAxes(false);
    }
    topoGridItem_->update();        
    break;
    
  case (int )Const::Cmd::ColorMap:
    {
      if (!topoGridItem_->setColormap(qval)) {
	qCritical() << "Unknown colormap scheme: " << qval;
      }
      else {
	qDebug() << "Set colormap scheme to " << qval;
      }
      topoGridItem_->update();
    }
    break;

  case (int )Const::Cmd::SiteFile:
    {
      qDebug() << "open site file " << value;
      char *siteFile = value;
      // Remove file URL prefix      
      siteFile += strlen("file://"); 
      //      topoGridItem_->setSiteFile(siteFile);
	qDebug() << "setSiteFile() not yet implemented";
      topoGridItem_->update();
    }
    break;

  case (int )Const::Cmd::RouteFile:
    {
      qDebug() << "open route file " << value;
      char *routeFile = value;
      // Remove file URL prefix      
      routeFile += strlen("file://"); 
      /// topoGridItem_->setRouteFile(routeFile);
      topoGridItem_->update();
    }
    break;

  default:
    qCritical() << "Unhandled param: " << param;
  }

  return;
  
}

