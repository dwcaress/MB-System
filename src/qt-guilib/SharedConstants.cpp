#include <iostream>
#include "SharedConstants.h"
#include "TopoColorMap.h"

using namespace mb_system;

const QString SharedConstants::testString_ = QString("Hello sailor!");

SharedConstants::SharedConstants() {
  
    // Get colormap names to be displayed by QML GUI
    std::vector<const char *> colorMapNames;

    // Load colormap names into vector
    TopoColorMap::schemeNames(&colorMapNames);

    std::cout << "ColorMaps:\n";
    // Copy colormap names to QStringList
    for (int i = 0; i < colorMapNames.size(); i++) {
      std::cout << colorMapNames[i] << "\n";
      // Append name to QStringList
      colorMapsList_.append(colorMapNames[i]);
    }

    mouseModes_ += MousePanAndZoom;
    mouseModes_ += MouseRotateModel;
    mouseModes_ += MouseRotateView;
    mouseModes_ += MouseShading;
    mouseModes_ += MousePickArea;
    mouseModes_ += MouseEditSites;
    mouseModes_ += MouseEditRoutes;
    mouseModes_ += MousePickNav;
    mouseModes_ += MousePickNavFile;
      
}
